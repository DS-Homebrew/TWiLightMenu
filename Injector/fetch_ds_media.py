#!/usr/bin/env python3
"""
fetch_ds_media.py

Downloads the LOGO (artwork type "wheel") and the FOOTAGE (gameplay video) of Nintendo DS
game(s) from ScreenScraper, using Skyscraper, and saves them into a destination folder.

Pure Python / cross-platform (Windows, macOS, Linux) -- no shell required. The only
external dependencies are the `Skyscraper` and `ffmpeg` binaries, which must be on PATH
(or pointed to via SKYSCRAPER_BIN / FFMPEG_BIN).

Two modes:
  - Single ROM (the classic CLI):  fetch_ds_media.py <rom.nds> [dest]
  - Batch (many ROMs in as few Skyscraper invocations as possible; see --batch-file below).
    scan_and_bind.py uses this for a whole scrape pass: Skyscraper's own per-invocation
    overhead (process start, cache open, phase switch) dwarfs the actual per-ROM network
    time once you're doing this hundreds of times, and Skyscraper already scrapes several
    files with internal threading (-t, default 4) when given more than one filename in a
    single call -- something a "one ROM per invocation" loop can never benefit from.

Skyscraper works in two phases per invocation:
  1) Gather:   scrapes and populates the local cache  -> uses  -s screenscraper
  2) Generate: builds the media files from the cache  -> runs WITHOUT  -s
This script runs both phases, in that order, for whichever ROM(s) it was given.

Credentials:
  On first run the script prompts for the ScreenScraper username/password and caches them
  next to this script (.ss_credentials.json). Later runs reuse the cache. Environment
  variables SS_USER/SS_PASS, if set, take precedence over the cache.
"""
import argparse
import getpass
import json
import os
import re
import shutil
import subprocess
import sys
import tempfile
import time

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
CRED_FILE = os.path.join(SCRIPT_DIR, ".ss_credentials.json")

# split_ds_video.py lives next to this script; import it directly instead of shelling out
# to "python3" (whose name/availability varies across platforms).
sys.path.insert(0, SCRIPT_DIR)


def err(msg):
    sys.stderr.write("ERROR: " + msg + "\n")


def log(msg):
    print("==> " + msg)


# ---------------------------------------------------------------------------
# Credential handling.
#
# load_credentials() fills SS_USER/SS_PASS following this precedence:
#   1) Environment variables SS_USER and SS_PASS (if both set and non-empty).
#   2) The ".ss_credentials.json" cache file next to this script.
#   3) An interactive prompt (first run), which then writes the JSON cache.
#
# The cache is a JSON object {"username":..,"password":..}, chmod'd to 600 where the
# platform supports it (a no-op on Windows, which uses ACLs instead of POSIX bits).
# ---------------------------------------------------------------------------
def load_credentials():
    user = os.environ.get("SS_USER")
    passwd = os.environ.get("SS_PASS")
    if user and passwd:
        log("Using ScreenScraper credentials from the environment.")
        return user, passwd

    if os.path.isfile(CRED_FILE):
        try:
            with open(CRED_FILE, "r", encoding="utf-8") as fh:
                data = json.load(fh)
            user, passwd = data.get("username"), data.get("password")
            if user and passwd:
                log(f"Loaded cached ScreenScraper credentials from: {CRED_FILE}")
                return user, passwd
        except (OSError, ValueError):
            pass
        err("Cached credentials file is invalid; re-entering credentials.")

    if not sys.stdin.isatty():
        err("No cached credentials and no terminal available to prompt.")
        err("Set SS_USER and SS_PASS in the environment, or run interactively once.")
        sys.exit(1)

    log("First run: please enter your ScreenScraper credentials.")
    user = input("ScreenScraper username: ").strip()
    passwd = getpass.getpass("ScreenScraper password: ")

    if not user or not passwd:
        err("Username and password must not be empty.")
        sys.exit(1)

    fd = os.open(CRED_FILE, os.O_WRONLY | os.O_CREAT | os.O_TRUNC, 0o600)
    with os.fdopen(fd, "w", encoding="utf-8") as fh:
        json.dump({"username": user, "password": passwd}, fh, ensure_ascii=False, indent=2)
        fh.write("\n")
    try:
        os.chmod(CRED_FILE, 0o600)
    except OSError:
        pass
    log(f"Credentials cached in: {CRED_FILE} (delete this file to reset).")
    return user, passwd


# ---------------------------------------------------------------------------
# Locating generated media.
#
# Skyscraper's generate phase names each output file after the SOURCE ROM's own base name
# (extension replaced) -- e.g. "Mario Kart DS (USA).nds" -> ".../marquees/Mario Kart DS
# (USA).png" (confirmed straight from Skyscraper's scraperworker.cpp: it builds the path as
# `<mediaFolder>/<QFileInfo(romFile).completeBaseName()>.<ext>`, which is exactly Python's
# os.path.splitext(basename)[0]). That means, batch or not, we can look each ROM's media up
# by its OWN name instead of guessing from "whatever is newest in the folder" -- important
# once a single workdir can hold output for many ROMs at once.
# ---------------------------------------------------------------------------
def _find_named(dirs, base, exts=None):
    """Returns the path to <dir>/<base>.<ext> for the first matching (dir, ext) pair, or
    None. `exts=None` means "any extension" (globs for it)."""
    import glob
    for d in dirs:
        if not os.path.isdir(d):
            continue
        if exts is None:
            matches = sorted(glob.glob(os.path.join(glob.escape(d), base + ".*")))
            if matches:
                return matches[0]
        else:
            for ext in exts:
                p = os.path.join(d, base + ext)
                if os.path.isfile(p):
                    return p
    return None


def run_and_capture(cmd):
    """Runs `cmd`, streaming its output to our own stdout while also capturing it (like
    piping through `tee`). Returns (returncode, combined_output_text)."""
    proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
    lines = []
    for line in proc.stdout:
        sys.stdout.write(line)
        lines.append(line)
    proc.wait()
    return proc.returncode, "".join(lines)


# ---------------------------------------------------------------------------
# Shared gather+generate for one or many ROMs in the same folder (Skyscraper scrapes one
# input folder -i per invocation, but accepts several filenames from it in a single call --
# that's the whole point of batching: one process, one cache session, Skyscraper's own
# internal threading, instead of paying full process/API-session overhead per ROM).
# ---------------------------------------------------------------------------
def _gather_and_generate(skyscraper_bin, rom_dir, filenames, workdir, ss_user, ss_pass):
    """Returns None on success, or an error message string on a hard (whole-batch) failure.
    A single ROM in the batch not being found on ScreenScraper is NOT a hard failure --
    that's discovered per-ROM afterwards by _find_named() coming up empty."""
    logo_only = os.environ.get("LOGO_ONLY", "0") == "1"
    media_flags = "unattend,unattendskip" if logo_only else "videos,unattend,unattendskip"
    verbose = os.environ.get("FETCH_VERBOSE", "0") == "1"

    log(f"[1/2] GATHER  -- scraping {len(filenames)} ROM(s) from ScreenScraper "
        f"in: {rom_dir}")
    if len(filenames) > 1:
        log("       " + ", ".join(filenames[:5]) + (", ..." if len(filenames) > 5 else ""))
    gather_cmd = [skyscraper_bin, "-p", "nds", "-i", rom_dir, "-s", "screenscraper",
                  "-u", f"{ss_user}:{ss_pass}", "--flags", media_flags]
    if verbose:
        gather_cmd += ["--verbosity", "3"]
    gather_cmd += filenames
    if verbose:
        shown = [a if a != f"{ss_user}:{ss_pass}" else "***:***" for a in gather_cmd]
        log("       $ " + " ".join(shown))
    t0 = time.monotonic()
    gather_rc, gather_out = run_and_capture(gather_cmd)
    log(f"[1/2] GATHER  -- done in {time.monotonic() - t0:.0f}s (exit {gather_rc}).")

    if re.search(r"API closed for non-registered members|quota|too many requests|"
                 r"request limit|Wrong username/password", gather_out, re.IGNORECASE):
        return ("ScreenScraper rejected the request (bad credentials, API closed, or quota "
                f"exhausted). If your credentials are wrong, delete {CRED_FILE} and run again.")
    # A single ROM's "not found" is routine in a batch and handled per-ROM below; only a
    # non-zero exit code (Skyscraper itself giving up) is treated as a hard failure here.
    if len(filenames) == 1 and re.search(
            r"Games found:\s*0|No entries to scrape|found 0 game", gather_out, re.IGNORECASE):
        return f"The gather phase did not identify the game for: {filenames[0]}"
    if gather_rc != 0:
        return f"Gather phase failed (exit code {gather_rc}). See the output above."

    log(f"[2/2] GENERATE -- building media into: {workdir}")
    generate_cmd = [skyscraper_bin, "-p", "nds", "-i", rom_dir, "-o", workdir,
                    "--flags", media_flags]
    if verbose:
        generate_cmd += ["--verbosity", "3"]
    generate_cmd += filenames
    if verbose:
        log("       $ " + " ".join(generate_cmd))
    t0 = time.monotonic()
    proc = subprocess.run(generate_cmd)
    log(f"[2/2] GENERATE -- done in {time.monotonic() - t0:.0f}s (exit {proc.returncode}).")
    if proc.returncode != 0:
        return f"Generate phase failed (exit code {proc.returncode})."
    return None


# ---------------------------------------------------------------------------
# Per-ROM post-processing: pick up whatever generate() produced for THIS rom out of a
# (possibly shared) workdir, copy/downscale the logo, copy+split the video. Shared by both
# the single-ROM and the batch code paths.
# ---------------------------------------------------------------------------
def _postprocess_one(base, dest, workdir, dest_tag=None):
    """`base` is the ROM's own base name (no extension) -- also the output media's base
    name. Returns {"logo": path_or_None, "top": path_or_None, "bottom": path_or_None}."""
    tag = f"[{dest_tag}] " if dest_tag else ""
    logo_only = os.environ.get("LOGO_ONLY", "0") == "1"

    # Default frontend (emulationstation) writes media DIRECTLY under -o's folder (no
    # per-platform "/nds/" level): marquees/ for the wheel logo (default artwork.xml maps
    # <output type="marquee" resource="wheel"/>), videos/ for the gameplay clip. "wheels/"
    # is also checked in case of a customized artwork.xml.
    logo_src = _find_named([os.path.join(workdir, "wheels"), os.path.join(workdir, "marquees")],
                           base, [".png"])
    video_src = None if logo_only else _find_named([os.path.join(workdir, "videos")], base)

    result = {"logo": None, "top": None, "bottom": None, "video_saved": False}
    got_video = False
    video_dest = None

    # ---- Logo (wheel): saved, then downscaled to cut DS load time. ----
    if logo_src and os.path.isfile(logo_src):
        logo_dest = os.path.join(dest, base + "-logo.png")
        shutil.copyfile(logo_src, logo_dest)
        log(f"{tag}Logo saved to: {logo_dest}")
        result["logo"] = logo_dest

        # Downscale (only if larger) to reduce PNG decode time on the DS. Preserves the
        # aspect ratio and the alpha channel. Configurable via env:
        #   LOGO_DOWNSCALE=0     disable
        #   LOGO_MAX_WIDTH=<n>   max width  (default 256 = DS screen width)
        #   LOGO_MAX_HEIGHT=<n>  max height (default 128)
        ffmpeg_bin = os.environ.get("FFMPEG_BIN", "ffmpeg")
        if os.environ.get("LOGO_DOWNSCALE", "1") != "0" and shutil.which(ffmpeg_bin):
            lw = os.environ.get("LOGO_MAX_WIDTH", "256")
            lh = os.environ.get("LOGO_MAX_HEIGHT", "128")
            tmp_logo = os.path.join(dest, f".logo_scaled_{os.getpid()}_{base}.png")
            scale_cmd = [ffmpeg_bin, "-y", "-v", "error", "-i", logo_dest,
                        "-vf", f"scale='min({lw},iw)':'min({lh},ih)':force_original_aspect_ratio=decrease",
                        "-pix_fmt", "rgba", tmp_logo]
            if subprocess.run(scale_cmd, stdout=subprocess.DEVNULL,
                              stderr=subprocess.DEVNULL).returncode == 0:
                os.replace(tmp_logo, logo_dest)
            else:
                try:
                    os.remove(tmp_logo)
                except OSError:
                    pass
                err(f"{tag}Logo downscale failed; kept the original size.")
    else:
        err(f"{tag}Logo (wheel) not found -- the game may not have this asset on ScreenScraper.")

    # ---- Video (footage): preserve the original extension. Skipped under LOGO_ONLY. ----
    if not logo_only:
        if video_src and os.path.isfile(video_src):
            video_ext = os.path.splitext(video_src)[1].lstrip(".")
            video_dest = os.path.join(dest, f"{base}-video.{video_ext}")
            shutil.copyfile(video_src, video_dest)
            log(f"{tag}Video saved to: {video_dest}")
            got_video = True
        else:
            err(f"{tag}Video (footage) not found -- the game may not have this asset on ScreenScraper.")
    result["video_saved"] = got_video

    # ---- Split the stacked video into two DS-ready TGRV files (top/bottom screens). ----
    # Non-fatal: if it fails / is disabled / ffmpeg is missing, the mp4 is kept as-is.
    if got_video and os.environ.get("SPLIT_VIDEO", "1") != "0":
        if shutil.which(os.environ.get("FFMPEG_BIN", "ffmpeg")):
            try:
                import split_ds_video
                split_args = [
                    "--fps", os.environ.get("TGRV_FPS", "12"),
                    "--width", os.environ.get("TGRV_WIDTH", "128"),
                    "--height", os.environ.get("TGRV_HEIGHT", "96"),
                    "--mode", os.environ.get("TGRV_MODE", "pal8"),
                ]
                if os.environ.get("TGRV_DITHER"):
                    split_args += ["--dither", os.environ["TGRV_DITHER"]]
                if os.environ.get("TGRV_MAX_SECONDS"):
                    split_args += ["--max-seconds", os.environ["TGRV_MAX_SECONDS"]]
                split_rc = split_ds_video.main([video_dest, dest] + split_args)
                if split_rc == 0:
                    result["top"] = os.path.join(dest, base + "-top.tgrv")
                    result["bottom"] = os.path.join(dest, base + "-bottom.tgrv")
                    if not (os.path.isfile(result["top"]) and os.path.isfile(result["bottom"])):
                        result["top"] = result["bottom"] = None
                    elif os.environ.get("KEEP_MP4", "1") == "0":
                        os.remove(video_dest)
                else:
                    err(f"{tag}Video split failed; keeping the mp4 as-is.")
            except Exception as e:                    # noqa: BLE001 (non-fatal, log and continue)
                err(f"{tag}Video split failed; keeping the mp4 as-is. ({e})")
        else:
            err(f"{tag}Skipping split (ffmpeg not found on PATH). Kept the mp4.")

    if not result["logo"] and not result["video_saved"]:
        err(f"{tag}No asset (logo or video) was found for: {base}")
    return result


def _resolve_skyscraper_bin():
    skyscraper_bin = os.environ.get("SKYSCRAPER_BIN", "Skyscraper")
    resolved = shutil.which(skyscraper_bin)
    if not resolved:
        err(f"Skyscraper not found (looked for: '{skyscraper_bin}').")
        err("Install Skyscraper or point SKYSCRAPER_BIN to the correct binary.")
        return None
    return resolved


# ---------------------------------------------------------------------------
# Batch mode: fetch media for MANY ROMs in as few Skyscraper invocations as possible.
# ---------------------------------------------------------------------------
def main_batch(batch_file, dest, results_file):
    with open(batch_file, "r", encoding="utf-8") as fh:
        rom_paths = [line.rstrip("\n") for line in fh if line.strip()]
    if not rom_paths:
        with open(results_file, "w", encoding="utf-8") as fh:
            json.dump({}, fh)
        return 0

    skyscraper_bin = _resolve_skyscraper_bin()
    if not skyscraper_bin:
        return 1

    ss_user, ss_pass = load_credentials()
    os.makedirs(dest, exist_ok=True)

    # Skyscraper scrapes one input folder (-i) per invocation, so ROMs living in different
    # folders still need separate calls -- but everything inside the SAME folder goes in one.
    groups = {}
    for rom in rom_paths:
        groups.setdefault(os.path.dirname(os.path.abspath(rom)), []).append(rom)

    results = {}
    overall_rc = 0
    for rom_dir, roms_in_dir in groups.items():
        filenames = [os.path.basename(r) for r in roms_in_dir]
        workdir = tempfile.mkdtemp(prefix="skyscraper_batch_")
        try:
            error = _gather_and_generate(skyscraper_bin, rom_dir, filenames, workdir,
                                         ss_user, ss_pass)
            if error:
                err(error)
                overall_rc = 3
                for rom in roms_in_dir:
                    results[rom] = {"logo": None, "top": None, "bottom": None}
                continue
            for rom in roms_in_dir:
                base = os.path.splitext(os.path.basename(rom))[0]
                results[rom] = _postprocess_one(base, dest, workdir, dest_tag=base)
        finally:
            shutil.rmtree(workdir, ignore_errors=True)

    with open(results_file, "w", encoding="utf-8") as fh:
        json.dump(results, fh)

    got_anything = any(any(r.values()) for r in results.values())
    if not got_anything and rom_paths:
        return 4
    return overall_rc


def main(argv):
    ap = argparse.ArgumentParser(
        prog="fetch_ds_media.py",
        description="Downloads the logo (wheel) and gameplay video of Nintendo DS game(s) "
                    "via Skyscraper/ScreenScraper.",
        epilog="""
Environment variables:
  SS_USER          (optional) ScreenScraper username; overrides the cache.
  SS_PASS          (optional) ScreenScraper password; overrides the cache.
  SKYSCRAPER_BIN   (optional) Path to the Skyscraper binary. Default: "Skyscraper".
  FFMPEG_BIN       (optional) Path to the ffmpeg binary. Default: "ffmpeg" (set
                   automatically by deploy.py when it downloads a portable copy).
  LOGO_ONLY        (optional) 1 = fetch only the logo (skip video). Default: 0.
  SPLIT_VIDEO      (optional) 0 = skip splitting the video into TGRV. Default: 1.
  TGRV_FPS         (optional) fps of the generated .tgrv files. Default: 12.
  TGRV_WIDTH       (optional) stored width per screen. Default: 128 (DS upscales).
  TGRV_HEIGHT      (optional) stored height per screen. Default: 96.
  TGRV_MODE        (optional) color mode: pal8 (8bpp, default) or bgr555 (16bpp).
  TGRV_DITHER      (optional) paletteuse dither (pal8). Default: bayer:bayer_scale=3.
  TGRV_MAX_SECONDS (optional) cap the .tgrv duration. Default: full video.
  KEEP_MP4         (optional) 0 = delete the intermediate mp4 after a good split.
  LOGO_DOWNSCALE   (optional) 0 = keep the logo at original size. Default: 1.
  LOGO_MAX_WIDTH   (optional) max logo width in px. Default: 256.
  LOGO_MAX_HEIGHT  (optional) max logo height in px. Default: 128.

Output (per ROM):
  <dest>/<base>-logo.png
  <dest>/<base>-top.tgrv       (top screen, TGR2 128x96 PAL8 8bpp, via split_ds_video.py)
  <dest>/<base>-bottom.tgrv    (bottom screen)
  <dest>/<base>-video.<ext>    (stacked mp4; removed if KEEP_MP4=0)
  (where <base> is the ROM file name without extension)
""",
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    ap.add_argument("rom", nargs="?", help="path to the DS ROM file (.nds)")
    ap.add_argument("dest", nargs="?", default=None,
                    help="folder to save the logo and video into. Default: this script's folder.")
    ap.add_argument("--batch-file", default=None,
                    help="path to a text file listing one ROM path per line. Fetches media "
                         "for ALL of them grouped into as few Skyscraper invocations as "
                         "possible (much faster than calling this script once per ROM). "
                         "When given, the `rom`/`dest` positionals are ignored -- use "
                         "--dest-dir for the destination -- and --results-file is required.")
    ap.add_argument("--results-file", default=None,
                    help="[--batch-file only] where to write a JSON "
                         "{rom_path: {logo, top, bottom}} results map.")
    ap.add_argument("--dest-dir", default=None,
                    help="[--batch-file only] shared destination folder. A NAMED flag on "
                         "purpose: two optional positionals (`rom`, `dest`) can't tell a "
                         "single trailing path apart, so a bare positional would silently "
                         "bind to `rom` instead of `dest` here.")
    args = ap.parse_args(argv)

    if args.batch_file:
        if not args.results_file:
            err("--batch-file requires --results-file.")
            return 2
        return main_batch(args.batch_file, args.dest_dir or SCRIPT_DIR, args.results_file)

    if not args.rom:
        ap.error("rom is required (or use --batch-file)")

    rom = args.rom
    dest = args.dest or SCRIPT_DIR

    skyscraper_bin = _resolve_skyscraper_bin()
    if not skyscraper_bin:
        return 1

    # The ROM must exist and have a .nds extension (case-insensitive).
    if not os.path.isfile(rom):
        err(f"ROM not found: {rom}")
        return 1
    if os.path.splitext(rom)[1].lower() != ".nds":
        err(f"The ROM does not have a .nds extension: {rom}")
        return 1

    ss_user, ss_pass = load_credentials()

    # ROM base name: "/roms/Super Game (USA).nds" -> "Super Game (USA)"
    # ROM directory: the folder Skyscraper needs as its input folder (-i). Skyscraper always
    # scrapes an INPUT FOLDER (-i); a positional file name only filters which files inside
    # that folder get processed. So -i points at the ROM's own folder, filtered to its name.
    rom_filename = os.path.basename(rom)
    base = os.path.splitext(rom_filename)[0]
    rom_dir = os.path.dirname(os.path.abspath(rom))

    os.makedirs(dest, exist_ok=True)
    log(f"Output directory: {dest}")
    if os.environ.get("LOGO_ONLY", "0") == "1":
        log("LOGO_ONLY=1 -- skipping video (logo only).")

    workdir = tempfile.mkdtemp(prefix="skyscraper_nds_")
    try:
        error = _gather_and_generate(skyscraper_bin, rom_dir, [rom_filename], workdir,
                                     ss_user, ss_pass)
        if error:
            err(error)
            return 3
        result = _postprocess_one(base, dest, workdir)
    finally:
        shutil.rmtree(workdir, ignore_errors=True)

    if not result["logo"] and not result["video_saved"]:
        # _postprocess_one already logged the specific reason.
        return 4
    log("Done.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
