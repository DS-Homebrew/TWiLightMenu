#!/usr/bin/env python3
"""
scan_and_bind.py -- scans an SD card, downloads each GAME's media and binds it by HASH.

Flow:
  1) Scans the SD (recursively) for .nds ROMs.
  2) Filters out system/homebrew apps via the blocklist (by sha1 and/or name).
  3) Groups by sha1 (content identity) -- duplicates (e.g. "... - copy.nds")
     collapse into ONE entry.
  4) For each game that doesn't have assets yet, calls fetch_ds_media.py (Skyscraper +
     ScreenScraper) to download the logo + video. Idempotent: skips games that already have them.
  5) ORGANIZES the assets into <out>/assets/<sha1>/{logo.png, video.<ext>} -- mirroring
     the per-hash layout inside the SD (default: <out> = the SD root).
  6) Writes <out>/manifest.yml (paths relative to the manifest root).

IDENTITY BY HASH (not by name): even if the file is duplicated/renamed, it resolves to the
same game and the same assets. See rom_binder.py for the runtime side.

INCREMENTAL RE-SCANS: hashing every ROM (sha1+md5+crc32, a full read of the file) is the
expensive part of a re-scan, not the download check. A ROM's content never changes once it's
sitting on the SD, so we cache each file's hash keyed by (path, size, mtime) in
<cache>/rom_hash_cache.json and only re-hash a file when that fingerprint changes. Combined
with the existing-assets check below, re-running this on an SD that's already fully set up
costs a cheap directory walk + a stat() per ROM -- not a re-read of every ROM's content --
and only the games actually missing something get processed.

ScreenScraper credentials: read by fetch_ds_media.py itself (env SS_USER/SS_PASS or the
.ss_credentials.json cache). This script only forwards the environment.
"""
import argparse
import json
import os
import shutil
import subprocess
import sys
import tempfile
import unicodedata

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from rom_hash import hash_file                       # noqa: E402
from yaml_io import dump_manifest, dump_assets_index  # noqa: E402

HERE = os.path.dirname(os.path.abspath(__file__))
DEFAULT_FETCH = os.path.normpath(os.path.join(HERE, "..", "fetch_ds_media.py"))
DEFAULT_BLOCKLIST = os.path.join(HERE, "system_blocklist.txt")
DEFAULT_CACHE = os.path.normpath(os.path.join(HERE, "..", "cache"))
HASH_CACHE_FILENAME = "rom_hash_cache.json"

# Canonical asset names inside assets/<sha1>/.
ASSET_NAMES = {"logo": "logo.png", "top": "top.tgrv", "bottom": "bottom.tgrv"}


# --------------------------- progress bar ---------------------------

class ProgressBar:
    """A single in-place progress line (via \\r), safe to interleave with occasional log
    lines through .note() -- which cleanly breaks out of the bar first so messages never
    land in the middle of it, then lets the next .update() resume drawing."""

    def __init__(self, total, prefix="", width=30, stream=sys.stderr, enabled=True):
        self.total = max(total, 1)
        self.prefix = prefix
        self.width = width
        self.stream = stream
        self.enabled = enabled and stream.isatty()
        self._active = False

    def update(self, current, label=""):
        if not self.enabled:
            return
        pct = min(current, self.total) * 100 // self.total
        filled = self.width * min(current, self.total) // self.total
        bar = "#" * filled + "-" * (self.width - filled)
        text = f"\r{self.prefix}[{bar}] {pct:3d}% ({current}/{self.total}) {label}"
        self.stream.write(text + "\033[K")
        self.stream.flush()
        self._active = True

    def note(self, msg):
        """Prints a one-off line without corrupting the progress bar."""
        if self._active:
            self.stream.write("\n")
            self._active = False
        self.stream.write(msg + "\n")
        self.stream.flush()

    def finish(self):
        if self._active:
            self.stream.write("\n")
            self._active = False


# --------------------------- ROM hash cache ---------------------------

def load_hash_cache(cache_dir):
    path = os.path.join(cache_dir, HASH_CACHE_FILENAME)
    try:
        with open(path, "r", encoding="utf-8") as fh:
            return json.load(fh)
    except (OSError, ValueError):
        return {}


def save_hash_cache(cache_dir, cache):
    os.makedirs(cache_dir, exist_ok=True)
    path = os.path.join(cache_dir, HASH_CACHE_FILENAME)
    tmp = path + ".tmp"
    with open(tmp, "w", encoding="utf-8") as fh:
        json.dump(cache, fh)
    os.replace(tmp, path)


def hash_file_cached(path, cache):
    """hash_file(path), reusing a cached result when the file's size+mtime still match what
    was recorded last time. ROM files aren't edited in place, so this turns a re-scan of an
    already-processed library into a cheap stat() per file instead of a full re-read.
    Returns (hash_dict, was_cached)."""
    st = os.stat(path)
    entry = cache.get(path)
    if entry and entry.get("size") == st.st_size and entry.get("mtime") == st.st_mtime:
        return entry["hash"], True
    h = hash_file(path)
    cache[path] = {"size": st.st_size, "mtime": st.st_mtime, "hash": h}
    return h, False


# --------------------------- blocklist ---------------------------

def load_blocklist(path):
    """Reads the blocklist. Returns (set of lowercase sha1s, list of name substrings)."""
    sha1s, names = set(), []
    if not path or not os.path.isfile(path):
        return sha1s, names
    with open(path, "r", encoding="utf-8") as f:
        for raw in f:
            line = raw.split("#", 1)[0].strip()
            if not line:
                continue
            low = line.lower()
            if low.startswith("sha1:"):
                sha1s.add(low[5:].strip())
            elif low.startswith("name:"):
                names.append(low[5:].strip())
            else:
                names.append(low)
    return sha1s, names


def name_blocked(filename, names):
    low = os.path.basename(filename).lower()
    return any(sub in low for sub in names)


# --------------------------- scan ---------------------------

# AppleDouble magic number (big-endian u32 0x00051607), at the start of every "._foo" sidecar
# file macOS writes next to "foo" whenever it touches a non-HFS+ filesystem (any FAT32/exFAT
# SD card qualifies). These aren't ROMs, but they DO match a ".nds" extension filter if the
# original file was itself an .nds ROM -- and worse, since their real (garbage) content never
# hashes to a known game, Skyscraper falls back to searching ScreenScraper BY NAME on them,
# which can return a real logo for a real game title... bound to the sidecar's bogus hash,
# never to the actual ROM. Filtered out on every OS, not just when running on a Mac, since a
# card that ever touched a Mac keeps these regardless of what's scanning it now.
_APPLEDOUBLE_MAGIC = b"\x00\x05\x16\x07"


def _is_appledouble(path):
    if os.path.basename(path).startswith("._"):
        return True
    try:
        with open(path, "rb") as fh:
            return fh.read(4) == _APPLEDOUBLE_MAGIC
    except OSError:
        return False


def find_roms(sd_dir, exts):
    out = []
    skipped = 0
    for dirpath, _, files in os.walk(sd_dir):
        for fn in files:
            if os.path.splitext(fn)[1].lower() not in exts:
                continue
            path = os.path.join(dirpath, fn)
            if _is_appledouble(path):
                skipped += 1
                continue
            out.append(path)
    if skipped:
        print(f">> ignored {skipped} macOS AppleDouble sidecar file(s) (._*) -- not ROMs",
              file=sys.stderr)
    return sorted(out)


# --------------------------- download (fetch_ds_media.py) ---------------------------

def fetch_media(fetch_script, rom_path, dest_dir):
    """
    Calls fetch_ds_media.py to download logo+video and already split the video into TGRV.
    The script writes '<base>-logo.png', '<base>-top.tgrv' and '<base>-bottom.tgrv'.
    Returns dict {logo, top, bottom} (paths or None). Forwards the environment
    (SS_USER/SS_PASS, SKYSCRAPER_BIN, TGRV_FPS, LOGO_ONLY, ...) unchanged.
    """
    os.makedirs(dest_dir, exist_ok=True)
    proc = subprocess.run(
        [sys.executable, fetch_script, rom_path, dest_dir],
        stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True,
    )
    base = os.path.splitext(os.path.basename(rom_path))[0]

    def pick(name):
        p = os.path.join(dest_dir, base + name)
        return p if os.path.isfile(p) else None

    result = {
        "logo": pick("-logo.png"),
        "top": pick("-top.tgrv"),
        "bottom": pick("-bottom.tgrv"),
    }
    # A non-zero rc from the script can mean "no asset at all" (exit 4) -- not fatal here.
    result["_failed"] = proc.returncode not in (0, 4)
    result["_output"] = proc.stdout
    result["_rc"] = proc.returncode
    return result


DEFAULT_BATCH_SIZE = int(os.environ.get("SCRAPE_BATCH_SIZE", "15"))


def fetch_media_batch(fetch_script, rom_paths, dest_dir, cache_dir, bar=None, verbose=False):
    """
    Calls fetch_ds_media.py's batch mode for MANY ROMs in as few Skyscraper invocations as
    possible (grouped by folder internally) -- one process/API session instead of one per
    ROM, and Skyscraper's own multi-file scraping (threaded, -t 4 by default) actually gets
    something to parallelize. `dest_dir` is a scratch folder the CALLER owns and cleans up
    (mirrors fetch_media()'s single-ROM contract, just shared across the whole batch).

    Output streams live (not buffered) -- a batch covers many ROMs and can run for minutes
    with nothing to show for it otherwise; this is what actually prints Skyscraper's own
    per-ROM gather/generate progress in real time instead of dumping it all at the end.
    Returns {rom_path: {logo, top, bottom}}.
    """
    with tempfile.NamedTemporaryFile("w", suffix=".txt", dir=cache_dir, delete=False,
                                     encoding="utf-8") as bf:
        batch_file = bf.name
        bf.write("\n".join(rom_paths) + "\n")
    results_file = batch_file + ".json"
    try:
        if bar:
            bar.note(f">> Starting Skyscraper for {len(rom_paths)} ROM(s) in this batch "
                     "(gather + generate)...")
        cmd = [sys.executable, fetch_script, "--batch-file", batch_file,
               "--results-file", results_file, "--dest-dir", dest_dir]
        if verbose:
            if bar:
                bar.note(f"   $ {' '.join(cmd)}")
            os.environ["FETCH_VERBOSE"] = "1"
        # No output capture: let it stream straight to the terminal live, in real time.
        proc = subprocess.run(cmd)
        if proc.returncode not in (0, 4):
            sys.stderr.write(f"   [batch fetch rc={proc.returncode}] (see output above)\n")
        try:
            with open(results_file, "r", encoding="utf-8") as fh:
                return json.load(fh)
        except (OSError, ValueError):
            return {}
    finally:
        for f in (batch_file, results_file):
            try:
                os.remove(f)
            except OSError:
                pass


def existing_assets(game_dir):
    """Returns dict {logo, top, bottom} already present under assets/<sha1>/ (or None)."""
    def pick(name):
        p = os.path.join(game_dir, name)
        return p if os.path.isfile(p) else None
    return {"logo": pick("logo.png"), "top": pick("top.tgrv"), "bottom": pick("bottom.tgrv")}


# --------------------------- main ---------------------------

def main(argv):
    ap = argparse.ArgumentParser(description="Scans the SD, downloads media and binds it by hash.")
    ap.add_argument("--sd", required=True, help="SD card root to scan")
    ap.add_argument("--out", default=None,
                    help="FINAL destination on the SD (manifest.yml + assets/). Default: the SD root itself.")
    ap.add_argument("--cache", default=DEFAULT_CACHE,
                    help="working folder where everything is downloaded/processed before deploying to the SD.")
    ap.add_argument("--fetch-script", default=DEFAULT_FETCH, help="path to fetch_ds_media.py")
    ap.add_argument("--blocklist", default=DEFAULT_BLOCKLIST, help="system blocklist file")
    ap.add_argument("--rom-ext", default=".nds", help="ROM extensions (csv). Default .nds")
    ap.add_argument("--list-only", action="store_true",
                    help="only scan and LIST the games found (downloads/writes nothing)")
    ap.add_argument("--no-download", action="store_true",
                    help="download nothing; only organize existing assets and (re)write the manifest")
    ap.add_argument("--force", action="store_true",
                    help="re-download media even if assets/<sha1>/ already exists")
    ap.add_argument("--allow-name-match", action="store_true",
                    help="write allow_name_match: true in the manifest (name fallback at runtime)")
    ap.add_argument("--no-progress", action="store_true",
                    help="print one line per file instead of a live progress bar "
                         "(used automatically when output isn't a terminal)")
    ap.add_argument("--verbose", "-v", action="store_true",
                    help="show the exact Skyscraper command line for each batch and raise "
                         "its own info verbosity (env SCAN_VERBOSE=1 does the same)")
    args = ap.parse_args(argv)
    verbose = args.verbose or os.environ.get("SCAN_VERBOSE", "0") == "1"

    out = os.path.abspath(args.out or args.sd)        # FINAL destination (SD)
    cache = os.path.abspath(args.cache)               # local working area
    exts = {e if e.startswith(".") else "." + e for e in args.rom_ext.lower().split(",")}
    out_assets = os.path.join(out, "assets")
    cache_assets = os.path.join(cache, "assets")
    show_bar = not args.no_progress

    # --list-only writes nothing; the other modes prepare the working cache.
    if not args.list_only:
        os.makedirs(cache_assets, exist_ok=True)

    if not args.list_only and not args.no_download and not os.path.isfile(args.fetch_script):
        sys.stderr.write(f"!! fetch script not found: {args.fetch_script}\n")
        sys.stderr.write("   Use --no-download to only (re)generate the manifest.\n")
        return 1

    sha1_block, name_block = load_blocklist(args.blocklist)
    print(f">> blocklist: {len(sha1_block)} hashes, {len(name_block)} names", file=sys.stderr)

    roms = find_roms(args.sd, exts)
    if not roms:
        sys.stderr.write(f"!! no ROM {sorted(exts)} in {args.sd}\n")
        return 1

    # Hash cache: lets a re-scan skip re-reading ROMs whose (path, size, mtime) haven't
    # changed since last time -- see hash_file_cached()'s docstring.
    hash_cache = load_hash_cache(cache)
    hash_cache_dirty = False

    # Group by sha1; skip blocked entries (by name before hashing, by hash afterwards).
    by_sha1 = {}
    n_blocked = 0
    n_reused_hash = 0
    scan_bar = ProgressBar(len(roms), prefix="Scanning ", enabled=show_bar)
    for i, path in enumerate(roms, 1):
        scan_bar.update(i, os.path.basename(path))
        if name_blocked(path, name_block):
            n_blocked += 1
            scan_bar.note(f"   ignored (system name): {os.path.basename(path)}")
            continue
        h, was_cached = hash_file_cached(path, hash_cache)
        if was_cached:
            n_reused_hash += 1
        else:
            hash_cache_dirty = True
        if h["sha1"].lower() in sha1_block:
            n_blocked += 1
            scan_bar.note(f"   ignored (system hash): {os.path.basename(path)}")
            continue
        by_sha1.setdefault(h["sha1"], {"hash": h, "files": []})["files"].append(path)
    scan_bar.finish()

    if hash_cache_dirty:
        save_hash_cache(cache, hash_cache)

    print(f">> {len(roms)} files ({n_reused_hash} already hashed, cached), {n_blocked} blocked, "
          f"{len(by_sha1)} unique games", file=sys.stderr)

    # --list-only: just prints the games found (name + sha1 + file count) and exits.
    if args.list_only:
        for sha1, info in sorted(by_sha1.items()):
            files = info["files"]
            dup = f"  (x{len(files)} duplicates)" if len(files) > 1 else ""
            print(f"{sha1[:12]}...  {os.path.basename(files[0])}{dup}")
        print(f">> {len(by_sha1)} game(s) listed (nothing downloaded/written).", file=sys.stderr)
        return 0

    # Assets already ready for a game: look first in the final destination (SD), then in the cache.
    def game_assets(sha1):
        for base in (out_assets, cache_assets):
            a = existing_assets(os.path.join(base, sha1))
            if any(a.values()):
                return a
        return {"logo": None, "top": None, "bottom": None}

    # Whether this run is a logo-only pass (env set by the caller, e.g. deploy.py's "scrape
    # logos" step). Determines what counts as "already have it" below.
    logo_only_mode = os.environ.get("LOGO_ONLY", "0") == "1"

    roms_index = {}   # ROM base name (no extension) -> game_id (for assets_index.yml)
    processed = []    # sha1s processed in this run (assets stay in the cache for deploy)
    n_logo = n_video = n_noasset = n_downloaded = n_skipped_existing = 0
    label = "logos" if logo_only_mode else "media"

    # ---- PASS 1: figure out what's already there, and what actually needs fetching. ----
    # Cheap -- no network/subprocess calls happen here, so this whole pass is instant even
    # for a large, already-fully-processed library.
    entries = {}   # sha1 -> {"hash", "primary", "assets"} -- "assets" gets updated in pass 2
    needed = []    # [(sha1, primary_rom_path), ...] that pass 2 must actually fetch
    for sha1, info in sorted(by_sha1.items()):
        h = info["hash"]
        files = info["files"]
        primary = files[0]  # representative of the duplicate group

        # Runtime index BY NAME: every file in the group (including duplicates) -> this game_id.
        # Normalizes the name to NFC: macOS lists files in NFD (e.g. an accented letter is
        # stored as base+combining-accent), but the DS matches the string in canonical form
        # (NFC, precomposed), like the reference index. Without this, accented names would
        # never match on the DS.
        for f in files:
            base = unicodedata.normalize("NFC", os.path.splitext(os.path.basename(f))[0])
            if base in roms_index and roms_index[base] != sha1:
                print(f"   warning: name '{base}' points at 2 different games; "
                      f"the name index becomes ambiguous.", file=sys.stderr)
            roms_index[base] = sha1

        assets = game_assets(sha1)  # {logo, top, bottom} (final or cache)
        entries[sha1] = {"hash": h, "primary": primary, "assets": assets}

        # A game "needs" downloading if the asset(s) relevant to the CURRENT mode are missing.
        # Logo-only pass: only the logo matters (a prior full pass may already have video-only
        # coverage, but that's not the concern here). Full pass: only the video matters -- if
        # a logo-only pass already ran, the logo alone must NOT be treated as "already done",
        # otherwise the video would never get fetched. The fetch downloads both together in
        # full mode, so the logo gets refreshed as a side effect, which is harmless.
        if logo_only_mode:
            need = args.force or not assets.get("logo")
        else:
            need = args.force or not (assets.get("top") or assets.get("bottom"))

        if need and not args.no_download:
            needed.append((sha1, primary))
        elif not need:
            n_skipped_existing += 1

    # ---- PASS 2: fetch, in batches -- one Skyscraper invocation covers up to
    # SCRAPE_BATCH_SIZE ROMs (grouped by folder) instead of one invocation per ROM. This is
    # the single biggest speedup available: Skyscraper's own per-invocation overhead (process
    # start, cache session, phase switch) otherwise dwarfs the actual per-ROM network time,
    # and a multi-ROM call lets Skyscraper use its own internal threaded scraping (-t,
    # default 4) instead of processing everything one connection at a time. ----
    if needed:
        fetch_bar = ProgressBar(len(needed), prefix=f"Fetching {label} ", enabled=show_bar)
        n_chunks = (len(needed) + DEFAULT_BATCH_SIZE - 1) // DEFAULT_BATCH_SIZE
        done = 0
        for chunk_i, start in enumerate(range(0, len(needed), DEFAULT_BATCH_SIZE), 1):
            chunk = needed[start:start + DEFAULT_BATCH_SIZE]
            chunk_roms = [primary for _, primary in chunk]
            fetch_bar.note(f">> Batch {chunk_i}/{n_chunks}: {len(chunk_roms)} ROM(s)")
            with tempfile.TemporaryDirectory(prefix="scanbind_batch_", dir=cache) as tmp:
                results = fetch_media_batch(args.fetch_script, chunk_roms, tmp, cache,
                                            bar=fetch_bar, verbose=verbose)
                for sha1, primary in chunk:
                    done += 1
                    fetch_bar.update(done, os.path.basename(primary))
                    cache_game_dir = os.path.join(cache_assets, sha1)
                    src = results.get(primary, {})
                    if any(src.get(k) for k in ASSET_NAMES):
                        os.makedirs(cache_game_dir, exist_ok=True)
                        for k, name in ASSET_NAMES.items():
                            if src.get(k) and os.path.isfile(src[k]):
                                shutil.move(src[k], os.path.join(cache_game_dir, name))
                    elif not src:
                        fetch_bar.note(f"   warning: no fetch result for "
                                       f"{os.path.basename(primary)} (batch error above?)")

                    # Re-check assets after the fetch: existing files not touched by this
                    # fetch (e.g. a logo from an earlier logo-only pass) must still be
                    # reported, so merge in whatever was already sitting in the destination.
                    merged = existing_assets(cache_game_dir)
                    for k in ASSET_NAMES:
                        if not merged.get(k) and entries[sha1]["assets"].get(k):
                            merged[k] = entries[sha1]["assets"][k]
                    entries[sha1]["assets"] = merged
                    if any(merged.values()):
                        n_downloaded += 1
                        processed.append(sha1)
        fetch_bar.finish()

    # ---- PASS 3: build the manifest entries from the (now up to date) per-game assets. ----
    games = []
    for sha1, info in sorted(by_sha1.items()):
        entry = entries[sha1]
        h, primary, assets = entry["hash"], entry["primary"], entry["assets"]

        # Paths in the manifest: relative to the FINAL destination (where they will live
        # after deploying). Assets that came from the SD (not the cache) don't need moving.
        rel = {k: (f"assets/{sha1}/{ASSET_NAMES[k]}" if assets.get(k) else None)
               for k in ("logo", "top", "bottom")}
        if assets["logo"]:
            n_logo += 1
        if assets["top"] or assets["bottom"]:
            n_video += 1
        if not any(assets.values()):
            n_noasset += 1
            print(f"   warning: no assets for {os.path.basename(primary)} (sha1 {sha1[:8]}...)",
                  file=sys.stderr)

        games.append({
            "game_id": sha1,
            "identity": h,
            "rom_name": os.path.basename(primary),  # informational/fallback
            "assets": {
                "logo": rel["logo"],
                "video": None,                 # stacked mp4 is not shipped to the SD
                "video_top": rel["top"],
                "video_bottom": rel["bottom"],
            },
        })

    # ---- .yml GENERATION (in the cache) ----
    manifest = {"version": 1, "games": games}
    if args.allow_name_match:
        manifest["allow_name_match"] = True

    os.makedirs(cache, exist_ok=True)
    cache_manifest = os.path.join(cache, "manifest.yml")
    cache_index = os.path.join(cache, "assets_index.yml")
    with open(cache_manifest, "w", encoding="utf-8") as fh:
        fh.write(dump_manifest(manifest))
    with open(cache_index, "w", encoding="utf-8") as fh:
        fh.write(dump_assets_index(roms_index))

    print(f">> processed in cache: {len(games)} games, {n_logo} logos, "
          f"{n_video} videos (tgrv), {n_noasset} with no assets, "
          f"{n_skipped_existing} already up to date, {n_downloaded} downloaded now.",
          file=sys.stderr)

    # ---- DEPLOY: builds the folders on the SD and moves the cache content there ----
    ndep = len(processed)
    if ndep:
        print(">> Deploy: creating folders on the SD and moving content over...", file=sys.stderr)
        os.makedirs(out_assets, exist_ok=True)
        deploy_bar = ProgressBar(ndep, prefix="Deploying ", enabled=show_bar)
        for j, sha1 in enumerate(processed, 1):
            deploy_bar.update(j, f"{sha1[:12]}...")
            src_dir = os.path.join(cache_assets, sha1)
            dst_dir = os.path.join(out_assets, sha1)
            if not os.path.isdir(src_dir):
                continue
            os.makedirs(dst_dir, exist_ok=True)
            for name in os.listdir(src_dir):
                shutil.move(os.path.join(src_dir, name), os.path.join(dst_dir, name))
            shutil.rmtree(src_dir, ignore_errors=True)
        deploy_bar.finish()
    else:
        os.makedirs(out_assets, exist_ok=True)
    # The .yml files go last (after the assets are already in place).
    shutil.move(cache_manifest, os.path.join(out, "manifest.yml"))
    shutil.move(cache_index, os.path.join(out, "assets_index.yml"))

    print(f">> Deploy complete in {out}: manifest.yml + assets_index.yml + "
          f"{ndep} game(s) moved from the cache.", file=sys.stderr)
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
