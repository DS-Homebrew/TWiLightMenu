#!/usr/bin/env python3
"""
deploy.py -- the one-stop installer for TWiLightMenuGRID's dsimenu.srldr and its game art
(logos + gameplay clips).

Pure Python, no shell/WSL/Git-Bash required -- this is meant to work out of the box on a
plain Windows install with just `python3` on PATH, as well as on macOS and Linux.

What it does, in order:
  1) LOCATE   the SD card (auto-detects it; asks you to pick or type a path otherwise).
  2) ASK      whether the menu is already installed on this SD -- if so, steps 3-4 are
              skipped entirely and it jumps straight to scraping art (also: --scrape-only).
  3) DEPLOY   dsimenu.srldr (+ a bundled themes/ folder, if this package ships one) onto
              the SD, backing up whatever was there before.
  4) SET      as the DSi menu's default theme any single theme folder (one containing a
              theme.ini) that ships next to this script -- e.g. "Default grid theme" --
              by installing it under dsimenu/themes/ and writing DSI_THEME in settings.ini.
  5) ASK      whether to scrape game LOGOS now (quick: no video download).
  6) ASK      whether to scrape gameplay CLIPS now (slow: downloads + converts video).

Steps 5 and 6 both call into assetbind/scan_and_bind.py, which does the actual scanning,
downloading (via fetch_ds_media.py -> Skyscraper/ScreenScraper) and binding by ROM hash.
Both steps are safe to run independently, any number of times, and in any order: already
-downloaded assets are never re-fetched unless you pass --force.

Requirements: just Python 3.8+ and a free ScreenScraper.fr account (credentials are asked
for once and cached locally) -- only if you say yes to steps 5/6. ffmpeg and Skyscraper are
NOT something you need to install by hand: if ffmpeg is missing, a portable copy is
downloaded automatically (no admin rights, nothing added to PATH); if Skyscraper is missing,
on macOS/Linux with git available it's built and installed for you (asks for your password
once, via sudo). Windows has no way around a manual Skyscraper build (upstream ships no
prebuilt binary for it) -- see the error message for exact steps if that's your case.
"""
import argparse
import os
import platform
import shutil
import string
import subprocess
import sys
import urllib.request

HERE = os.path.dirname(os.path.abspath(__file__))
ASSETBIND_DIR = os.path.join(HERE, "assetbind")
TOOLS_DIR = os.path.join(HERE, "tools")
sys.path.insert(0, HERE)
sys.path.insert(0, ASSETBIND_DIR)

DSIMENU_SUBPATH = os.path.join("_nds", "TWiLightMenu")
DSIMENU_ASSETS_SUBPATH = os.path.join(DSIMENU_SUBPATH, "dsimenu")
SETTINGS_INI_SUBPATH = os.path.join(DSIMENU_SUBPATH, "settings.ini")


def log(msg):
    print(f"\n\033[1;34m== {msg} ==\033[0m")


def info(msg):
    print(f">> {msg}")


def err(msg):
    sys.stderr.write(f"ERROR: {msg}\n")


# ---------------------------------------------------------------------------
# Small interactive helpers.
# ---------------------------------------------------------------------------
def ask_yes_no(question, default=False, assume_yes=None):
    """Prompts a yes/no question. `assume_yes` (True/False/None) skips the prompt when set,
    which is how --yes / --no-interactive-* flags short-circuit interactive runs."""
    if assume_yes is not None:
        return assume_yes
    hint = "[Y/n]" if default else "[y/N]"
    while True:
        reply = input(f"{question} {hint} ").strip().lower()
        if not reply:
            return default
        if reply in ("y", "yes"):
            return True
        if reply in ("n", "no"):
            return False
        print("Please answer 'y' or 'n'.")


def ask_choice(prompt, choices):
    """Lets the user pick one of `choices` (list of str) by number. Returns the chosen string."""
    for i, c in enumerate(choices, 1):
        print(f"  {i}) {c}")
    while True:
        reply = input(f"{prompt} [1-{len(choices)}]: ").strip()
        if reply.isdigit() and 1 <= int(reply) <= len(choices):
            return choices[int(reply) - 1]
        print("Invalid choice.")


# ---------------------------------------------------------------------------
# SD card detection. Looks for a "_nds" folder, which every TWiLightMenu SD card has.
# ---------------------------------------------------------------------------
def list_mount_candidates():
    system = platform.system()
    candidates = []

    if system == "Darwin":
        base = "/Volumes"
        if os.path.isdir(base):
            candidates += [os.path.join(base, n) for n in sorted(os.listdir(base))]

    elif system == "Linux":
        user = os.environ.get("USER") or os.environ.get("LOGNAME") or ""
        bases = [f"/media/{user}", f"/run/media/{user}"] if user else []
        bases += ["/media", "/mnt"]
        seen = set()
        for base in bases:
            if base in seen or not os.path.isdir(base):
                continue
            seen.add(base)
            candidates += [os.path.join(base, n) for n in sorted(os.listdir(base))]

    elif system == "Windows":
        try:
            import ctypes
            mask = ctypes.windll.kernel32.GetLogicalDrives()  # type: ignore[attr-defined]
            for i, letter in enumerate(string.ascii_uppercase):
                if mask & (1 << i):
                    candidates.append(f"{letter}:\\")
        except Exception:
            # Fallback if ctypes/win32 access fails for any reason.
            candidates += [f"{letter}:\\" for letter in string.ascii_uppercase]

    return [c for c in candidates if os.path.isdir(c)]


def looks_like_twilightmenu_sd(path):
    return os.path.isdir(os.path.join(path, "_nds"))


def find_sd_card():
    """Auto-detects a mounted SD card that already has a TWiLightMenu "_nds" folder."""
    return [c for c in list_mount_candidates() if looks_like_twilightmenu_sd(c)]


def choose_sd(explicit, assume_yes):
    if explicit:
        if not os.path.isdir(explicit):
            err(f"Path does not exist: {explicit}")
            sys.exit(1)
        return os.path.abspath(explicit)

    log("Locating the SD card")
    matches = find_sd_card()
    if len(matches) == 1:
        info(f"Found: {matches[0]}")
        return os.path.abspath(matches[0])
    if len(matches) > 1:
        info("Multiple SD cards / drives with a TWiLightMenu folder were found:")
        choice = ask_choice("Which one is your DS/DSi SD card?", matches)
        return os.path.abspath(choice)

    info("No SD card with an existing _nds folder was auto-detected.")
    if assume_yes:
        err("Pass --sd <path> when running non-interactively.")
        sys.exit(1)
    while True:
        typed = input("Type the full path to your SD card (or its drive letter): ").strip()
        if not typed:
            continue
        if os.path.isdir(typed):
            return os.path.abspath(typed)
        print(f"'{typed}' is not a folder I can see. Try again (e.g. E:\\ or /Volumes/DSI).")


# ---------------------------------------------------------------------------
# Step 2: deploy dsimenu.srldr (+ an optional bundled themes/ folder) onto the SD.
# ---------------------------------------------------------------------------
def merge_copy_tree(src, dst):
    """Copies `src` on top of `dst`, folder by folder, WITHOUT deleting anything already in
    `dst` that isn't also in `src`. Safer than shutil.copytree(dirs_exist_ok=True) followed
    by nothing extra -- it behaves the same, this just documents the intent."""
    for root, _dirs, files in os.walk(src):
        rel = os.path.relpath(root, src)
        target_dir = os.path.join(dst, rel) if rel != "." else dst
        os.makedirs(target_dir, exist_ok=True)
        for name in files:
            if name in (".DS_Store",) or name.startswith("._"):
                continue
            shutil.copy2(os.path.join(root, name), os.path.join(target_dir, name))


def deploy_srldr_and_themes(sd_root, srldr_path, themes_path):
    log("Deploying dsimenu.srldr")
    dsimenu_dir = os.path.join(sd_root, DSIMENU_SUBPATH)
    os.makedirs(dsimenu_dir, exist_ok=True)

    if srldr_path and os.path.isfile(srldr_path):
        dst_srldr = os.path.join(dsimenu_dir, "dsimenu.srldr")
        if os.path.isfile(dst_srldr):
            shutil.copyfile(dst_srldr, dst_srldr + ".bak")
            info("Backed up the previous dsimenu.srldr -> dsimenu.srldr.bak")
        shutil.copyfile(srldr_path, dst_srldr)
        size_kb = os.path.getsize(srldr_path) / 1024
        info(f"Installed dsimenu.srldr ({size_kb:.0f} KB) -> {dst_srldr}")
    else:
        info("No dsimenu.srldr found next to this script (expected at "
             f"'{os.path.join(HERE, 'build', 'dsimenu.srldr')}') -- skipping this step.")
        info("If you only want to scrape art, that's fine; run this again with the file "
             "in place whenever you want to (re)install the menu itself.")

    if themes_path and os.path.isdir(themes_path):
        dst_themes = os.path.join(sd_root, DSIMENU_ASSETS_SUBPATH, "themes")
        os.makedirs(dst_themes, exist_ok=True)
        info(f"Copying themes -> {dst_themes}")
        merge_copy_tree(themes_path, dst_themes)

    os.makedirs(os.path.join(sd_root, DSIMENU_ASSETS_SUBPATH), exist_ok=True)


# ---------------------------------------------------------------------------
# settings.ini read/write, matching TWiLightMenu's own CIniFile format exactly
# (universal/source/common/inifile.cpp): case-sensitive "[Section]" headers,
# "Key = Value" lines, CRLF line endings, lines starting with ';', '/' or '!' are
# comments. We only ever touch the specific keys we're given -- every other line in
# the file (other sections, other SRLOADER keys, the user's own settings) is left
# byte-for-byte alone.
# ---------------------------------------------------------------------------
def _read_ini_lines(path):
    if not os.path.isfile(path):
        return []
    with open(path, "rb") as fh:
        raw = fh.read()
    if raw[:3] == b"\xef\xbb\xbf":     # UTF-8 BOM, tolerated by CIniFile
        raw = raw[3:]
    text = raw.decode("utf-8", errors="replace")
    lines = []
    for line in text.replace("\r\n", "\n").replace("\r", "\n").split("\n"):
        s = line.strip(" \t")
        if s and s[0] not in (";", "/", "!"):
            lines.append(s)
    return lines


def _write_ini_lines(path, lines):
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "wb") as fh:
        for line in lines:
            fh.write(line.encode("utf-8"))
            fh.write(b"\r\n")


def set_ini_keys(path, section, keys):
    """Sets `keys` (dict, insertion order preserved) under [section] in an SRLoader-style
    .ini file, creating the file/section/keys if missing. Every other line is preserved."""
    lines = _read_ini_lines(path)
    pending = dict(keys)

    in_section = False
    for i, line in enumerate(lines):
        if line.startswith("["):
            end = line.find("]")
            in_section = end > 0 and line[1:end] == section
            continue
        if in_section and pending:
            eq = line.find("=")
            if eq > 0:
                item = line[:eq].strip(" \t")
                if item in pending:
                    lines[i] = f"{item} = {pending.pop(item)}"

    if pending:
        insert_at = None
        for idx, line in enumerate(lines):
            if line.startswith("[") and line[1:line.find("]")] == section:
                j = idx + 1
                while j < len(lines) and not lines[j].startswith("["):
                    j += 1
                insert_at = j
                break
        if insert_at is None:
            lines.append(f"[{section}]")
            lines.extend(f"{k} = {v}" for k, v in pending.items())
        else:
            for offset, (k, v) in enumerate(pending.items()):
                lines.insert(insert_at + offset, f"{k} = {v}")

    _write_ini_lines(path, lines)


# ---------------------------------------------------------------------------
# Step 3: install a bundled single-theme folder (one containing a theme.ini directly,
# e.g. "Default grid theme") and select it as the DSi menu's default theme.
# ---------------------------------------------------------------------------
def find_bundled_theme_dirs():
    """Immediate subfolders of this script's own folder that directly contain a
    theme.ini -- i.e. a single theme, as opposed to --themes (a folder of *several*
    theme subfolders, used for the developer preview-sync workflow)."""
    found = []
    for name in sorted(os.listdir(HERE)):
        path = os.path.join(HERE, name)
        if os.path.isdir(path) and os.path.isfile(os.path.join(path, "theme.ini")):
            found.append(path)
    return found


def install_default_theme(sd_root, theme_dir):
    log("Setting the default DSi theme")
    name = os.path.basename(os.path.normpath(theme_dir))
    dst = os.path.join(sd_root, DSIMENU_ASSETS_SUBPATH, "themes", name)
    os.makedirs(dst, exist_ok=True)
    info(f"Installing theme '{name}' -> {dst}")
    merge_copy_tree(theme_dir, dst)

    settings_path = os.path.join(sd_root, SETTINGS_INI_SUBPATH)
    if os.path.isfile(settings_path):
        shutil.copyfile(settings_path, settings_path + ".bak")
        info("Backed up the previous settings.ini -> settings.ini.bak")
    set_ini_keys(settings_path, "SRLOADER", {"THEME": "0", "DSI_THEME": name})
    info(f"settings.ini: [SRLOADER] THEME = 0, DSI_THEME = {name}")


# ---------------------------------------------------------------------------
# Steps 4/5: scrape logos, then clips. Both delegate to assetbind/scan_and_bind.py.
#
# Neither ffmpeg nor Skyscraper ship with Python, so a plain "pip install" can't pull
# them in. To keep this a real "no installation needed" tool for non-technical users:
#   - ffmpeg: we download an official portable static build ourselves (no admin rights,
#     no PATH changes -- we just point FFMPEG_BIN at the downloaded copy).
#   - Skyscraper: it has no portable binary at all (upstream only ships source, built via
#     their own installer script), so a real one-command install is only possible where
#     that installer runs unattended (macOS/Linux with git+build tools). We run it for the
#     user there; on Windows we fall back to clear manual instructions.
# ---------------------------------------------------------------------------
FFMPEG_BUILD_URLS = {
    "Windows": {None: "https://www.gyan.dev/ffmpeg/builds/ffmpeg-release-essentials.zip"},
    "Darwin": {None: "https://evermeet.cx/ffmpeg/getrelease/zip"},
    "Linux": {
        "x86_64": "https://johnvansickle.com/ffmpeg/releases/ffmpeg-release-amd64-static.tar.xz",
        "amd64": "https://johnvansickle.com/ffmpeg/releases/ffmpeg-release-amd64-static.tar.xz",
        "aarch64": "https://johnvansickle.com/ffmpeg/releases/ffmpeg-release-arm64-static.tar.xz",
        "arm64": "https://johnvansickle.com/ffmpeg/releases/ffmpeg-release-arm64-static.tar.xz",
        "armv7l": "https://johnvansickle.com/ffmpeg/releases/ffmpeg-release-armhf-static.tar.xz",
        "i686": "https://johnvansickle.com/ffmpeg/releases/ffmpeg-release-i686-static.tar.xz",
        "i386": "https://johnvansickle.com/ffmpeg/releases/ffmpeg-release-i686-static.tar.xz",
    },
}


def _local_ffmpeg_path():
    name = "ffmpeg.exe" if platform.system() == "Windows" else "ffmpeg"
    return os.path.join(TOOLS_DIR, "ffmpeg", name)


def _download_ffmpeg_url():
    system = platform.system()
    table = FFMPEG_BUILD_URLS.get(system)
    if not table:
        return None
    return table.get(None) or table.get(platform.machine().lower())


def _extract_ffmpeg(archive_bytes, dest_path):
    """Pulls just the ffmpeg binary out of the downloaded archive (zip or tar.xz), whatever
    folder it's nested under, and writes it to dest_path. The archive type is sniffed from
    its magic bytes rather than the URL -- e.g. evermeet.cx's URL has no file extension at
    all (the real name only shows up in a Content-Disposition header we don't bother
    reading)."""
    import io
    wanted = os.path.basename(dest_path)
    os.makedirs(os.path.dirname(dest_path), exist_ok=True)
    if archive_bytes[:2] == b"PK":                          # ZIP local-file-header magic
        import zipfile
        with zipfile.ZipFile(io.BytesIO(archive_bytes)) as zf:
            member = next(n for n in zf.namelist() if os.path.basename(n) == wanted)
            with zf.open(member) as src, open(dest_path, "wb") as dst:
                shutil.copyfileobj(src, dst)
    elif archive_bytes[:6] == b"\xfd7zXZ\x00":               # XZ magic
        import tarfile
        with tarfile.open(fileobj=io.BytesIO(archive_bytes), mode="r:xz") as tf:
            member = next(m for m in tf.getmembers()
                         if m.isfile() and os.path.basename(m.name) == wanted)
            with tf.extractfile(member) as src, open(dest_path, "wb") as dst:
                shutil.copyfileobj(src, dst)
    else:
        raise RuntimeError(f"Unrecognized archive format (first bytes: {archive_bytes[:8]!r})")
    os.chmod(dest_path, 0o755)


def ensure_ffmpeg(auto_download):
    """Returns a path/command usable as the ffmpeg binary, or None if unavailable.
    Order: PATH -> already-downloaded local copy -> download one now (if allowed)."""
    found = shutil.which("ffmpeg")
    if found:
        return found

    local = _local_ffmpeg_path()
    if os.path.isfile(local):
        return local

    if not auto_download:
        return None

    url = _download_ffmpeg_url()
    if not url:
        err(f"No portable ffmpeg build known for {platform.system()} {platform.machine()}; "
            "install ffmpeg yourself from https://ffmpeg.org/download.html.")
        return None

    size_hint = "~25-110 MB depending on your OS"
    info(f"ffmpeg isn't installed; downloading a portable copy ({size_hint}, one-time)...")
    info(f"Source: {url}")
    try:
        req = urllib.request.Request(url, headers={"User-Agent": "TWiLightMenuGRID-deploy"})
        with urllib.request.urlopen(req, timeout=120) as resp:
            data = resp.read()
        _extract_ffmpeg(data, local)
    except Exception as e:                             # noqa: BLE001 -- fall back to manual install
        err(f"Automatic ffmpeg download failed ({e}).")
        err("Install it yourself from https://ffmpeg.org/download.html and put it on PATH.")
        return None

    info(f"ffmpeg ready: {local}")
    return local


# Skyscraper's own installer (update_skyscraper.sh) does `sudo make install` into
# /usr/local/{bin,etc}, needs wget+tar to fetch a release archive, and upstream's own docs
# admit macOS "is not officially supported". We avoid ALL of that: we already have the full
# source from `git clone`, so we build it directly (qmake && make, no root at all) and use
# the resulting binary straight out of the build folder via SKYSCRAPER_BIN -- exactly like
# ensure_ffmpeg() does for its own downloaded binary. The only thing `sudo make install`
# does that actually matters at runtime is seeding ~/.skyscraper/ with default config/
# artwork/resource files on first run (skyscraper.cpp's loadConfig() copies them there from
# /usr/local/etc/skyscraper, which is itself just a copy of files already sitting at the
# root of the very source tree we cloned) -- so we seed ~/.skyscraper/ ourselves, straight
# from the clone, and skip the system-wide copy step entirely.
SKYSCRAPER_HOME = os.path.join(os.path.expanduser("~"), ".skyscraper")
# (dest path relative to ~/.skyscraper, source path relative to the cloned repo). Mirrors
# skyscraper.cpp's own copyFile(..., "dest", false) calls -- i.e. only the files it treats
# as "don't overwrite if already there", which are the ones actually required to run instead
# of just reference/example docs.
SKYSCRAPER_SEED_FILES = [
    ("artwork.xml", "artwork.xml"),                  # required -- Skyscraper exits without it
    ("config.ini", "config.ini.example"),
    ("aliasMap.csv", "aliasMap.csv"),
    ("mameMap.csv", "mameMap.csv"),
    (os.path.join("resources", "maskexample.png"), os.path.join("resources", "maskexample.png")),
    (os.path.join("resources", "frameexample.png"), os.path.join("resources", "frameexample.png")),
    (os.path.join("resources", "scanlines1.png"), os.path.join("resources", "scanlines1.png")),
    (os.path.join("resources", "scanlines2.png"), os.path.join("resources", "scanlines2.png")),
    (os.path.join("import", "definitions.dat"), os.path.join("import", "definitions.dat.example2")),
]


def _seed_skyscraper_home(src_dir):
    """Populates ~/.skyscraper/ with the default config Skyscraper needs, straight from our
    git clone. Never overwrites a file the user (or a previous run) already put there."""
    for dest_rel, src_rel in SKYSCRAPER_SEED_FILES:
        src = os.path.join(src_dir, src_rel)
        dest = os.path.join(SKYSCRAPER_HOME, dest_rel)
        if os.path.isfile(dest) or not os.path.isfile(src):
            continue
        os.makedirs(os.path.dirname(dest), exist_ok=True)
        shutil.copyfile(src, dest)


def _ensure_qmake():
    """Returns the qmake binary to use, installing Qt5 first if needed. macOS: Homebrew,
    no root required (brew installs under its own user-owned prefix). Linux: the system
    package manager, which does need sudo -- that's an unavoidable, one-time OS-level
    dependency install, same as it would be for any other Qt application."""
    found = shutil.which("qmake") or shutil.which("qmake5")
    if found:
        return found

    system = platform.system()
    if system == "Darwin":
        if not shutil.which("brew"):
            err("Homebrew isn't installed, so I can't fetch Qt5 (needed to build "
                "Skyscraper) automatically. Install Homebrew from https://brew.sh first.")
            return None
        info("Installing Qt5 via Homebrew (needed to build Skyscraper; can take a while, "
             "no admin password needed)...")
        try:
            subprocess.run(["brew", "install", "qt5"], check=True, timeout=1800)
            subprocess.run(["brew", "link", "qt5", "--force"], timeout=60)
        except Exception as e:                          # noqa: BLE001 -- caller reports failure
            err(f"Installing Qt5 via Homebrew failed: {e}")
            return None
    elif system == "Linux":
        for manager, pkgs in (("apt-get", ["qtbase5-dev", "qt5-qmake"]),
                              ("dnf", ["qt5-qtbase-devel"]),
                              ("pacman", ["qt5-base"])):
            if shutil.which(manager):
                info(f"Installing Qt5 via {manager} (needed to build Skyscraper; will ask "
                     "for your sudo password)...")
                try:
                    subprocess.run(["sudo", manager, "install", "-y"] + pkgs,
                                   check=True, timeout=1800)
                except Exception as e:                  # noqa: BLE001 -- caller reports failure
                    err(f"Installing Qt5 via {manager} failed: {e}")
                    return None
                break
        else:
            err("No supported package manager (apt/dnf/pacman) found to install Qt5 "
                "automatically. Install a Qt5 dev package yourself, then run this again.")
            return None

    return shutil.which("qmake") or shutil.which("qmake5")


def ensure_skyscraper(assume_yes):
    """Returns a path/command usable as the Skyscraper binary, or None if unavailable.
    Skyscraper has no portable binary releases (source-only upstream), so we build it
    ourselves in a local folder -- no root needed on macOS, and only the OS package manager
    (for Qt5) needs sudo on Linux. Windows always needs a manual, documented install."""
    env_bin = os.environ.get("SKYSCRAPER_BIN", "Skyscraper")
    found = shutil.which(env_bin)
    if found:
        return found

    # A previously-built local copy, from an earlier run of this same tool.
    local_candidates = (os.path.join(TOOLS_DIR, "skyscraper-src", "Skyscraper"),
                       os.path.join(TOOLS_DIR, "skyscraper-src", "Skyscraper.app",
                                   "Contents", "MacOS", "Skyscraper"))
    for candidate in local_candidates:
        if os.path.isfile(candidate):
            return candidate
    if os.environ.get("SCAN_VERBOSE", "0") == "1":
        info("No cached Skyscraper build found at: " + " or ".join(local_candidates))

    system = platform.system()
    # No sudo is needed on macOS anymore, but Linux still shells out to the OS package
    # manager (via sudo) for Qt5, and a heavy multi-minute build isn't something to kick off
    # silently in a non-interactive/piped context either -- require a real terminal.
    can_auto_install = (system in ("Darwin", "Linux") and shutil.which("git")
                        and sys.stdin.isatty())
    if can_auto_install:
        do_install = assume_yes or ask_yes_no(
            "Skyscraper isn't installed. Build it now automatically? (clones the project "
            "and compiles it locally -- no system install, and on macOS no admin password "
            "either; can take several minutes, mostly for Qt5 if it isn't installed yet)",
            default=True)
        if do_install:
            qmake_bin = _ensure_qmake()
            if not qmake_bin:
                err("Couldn't set up Qt5; skipping the automatic Skyscraper build.")
            else:
                info("Building Skyscraper (this can take a while)...")
                src_dir = os.path.join(TOOLS_DIR, "skyscraper-src")
                try:
                    if not os.path.isdir(src_dir):
                        subprocess.run(
                            ["git", "clone", "--depth", "1",
                             "https://github.com/muldjord/skyscraper", src_dir],
                            check=True, timeout=180)
                    # No `git pull` on an existing checkout: we intentionally delete its
                    # VERSION file below (see comment), which a shallow clone's pull can
                    # trip over as a conflicting local change on a later run for no benefit
                    # -- Skyscraper is stable enough that reusing whatever was cloned once
                    # is a fine trade-off. Delete Injector/tools/skyscraper-src by hand to
                    # force a fresh clone if you ever want to pick up new upstream commits.
                    # The repo ships a "VERSION" file at its root, and qmake puts the repo
                    # root on the include path (-I.). On a case-insensitive filesystem (the
                    # macOS default), that file collides with the C++17 standard <version>
                    # header -- Skyscraper's own VERSION text gets fed to the compiler in
                    # its place, breaking every source file that (transitively) includes
                    # <version>. We remove the file (the .pro reads it into its own $$VERSION
                    # qmake variable) but keep the version string it held by passing it back
                    # in on the qmake command line instead -- purely cosmetic (--version
                    # banner text), but no reason to lose it just to dodge the header clash.
                    # Read it via `git show` (not the working-tree file) so a re-run still
                    # gets the right string even though a previous run already deleted it.
                    version_file = os.path.join(src_dir, "VERSION")
                    version_str = ""
                    try:
                        version_str = subprocess.run(
                            ["git", "-C", src_dir, "show", "HEAD:VERSION"], check=True,
                            timeout=15, capture_output=True, text=True).stdout.strip()
                    except Exception:                    # noqa: BLE001 -- cosmetic, non-fatal
                        pass
                    if os.path.isfile(version_file):
                        os.remove(version_file)
                    qmake_cmd = [qmake_bin, "CONFIG+=sdk_no_version_check"]
                    if version_str:
                        qmake_cmd.append(f"VERSION={version_str}")
                    subprocess.run(qmake_cmd, cwd=src_dir, check=True, timeout=120)
                    subprocess.run(["make", f"-j{os.cpu_count() or 2}"], cwd=src_dir,
                                   check=True, timeout=1800)
                    _seed_skyscraper_home(src_dir)
                except Exception as e:                  # noqa: BLE001 -- fall back to manual install
                    err(f"Automatic Skyscraper build failed ({e}).")
                else:
                    for candidate in (os.path.join(src_dir, "Skyscraper"),
                                     os.path.join(src_dir, "Skyscraper.app",
                                                 "Contents", "MacOS", "Skyscraper")):
                        if os.path.isfile(candidate):
                            os.chmod(candidate, 0o755)
                            info(f"Skyscraper built: {candidate}")
                            return candidate
                    err("The build finished, but the Skyscraper binary wasn't found "
                        "afterwards.")

    err("Skyscraper isn't installed. It has no ready-to-run download -- it has to be built:")
    if system == "Windows":
        err("  Windows: see https://github.com/muldjord/skyscraper#windows for the manual "
            "MSYS2/Qt build steps (there is no prebuilt .exe from upstream).")
    else:
        err("  Run: git clone https://github.com/muldjord/skyscraper && "
            "cd skyscraper && sudo ./update_skyscraper.sh")
    return None


def check_scrape_requirements(assume_yes, auto_download_ffmpeg=True):
    ffmpeg_bin = ensure_ffmpeg(auto_download_ffmpeg)
    skyscraper_bin = ensure_skyscraper(assume_yes)
    if not ffmpeg_bin or not skyscraper_bin:
        if not ffmpeg_bin:
            err("ffmpeg is not available.")
        if not skyscraper_bin:
            err("Skyscraper is not available.")
        return False
    os.environ["FFMPEG_BIN"] = ffmpeg_bin
    os.environ["SKYSCRAPER_BIN"] = skyscraper_bin
    return True


def run_scrape(sd_root, logo_only, force, assume_yes, verbose=False):
    import scan_and_bind

    dest = os.path.join(sd_root, DSIMENU_ASSETS_SUBPATH)
    os.environ["LOGO_ONLY"] = "1" if logo_only else "0"

    argv = ["--sd", sd_root, "--out", dest]
    if force:
        argv.append("--force")
    if verbose:
        argv.append("--verbose")

    label = "logos" if logo_only else "gameplay clips"
    log(f"Scanning the SD for games (preview -- {label})")
    rc = scan_and_bind.main(argv + ["--list-only"])
    if rc != 0:
        err("Nothing to scrape (see the messages above).")
        return

    if not ask_yes_no(f"Proceed with downloading {label} for the game(s) listed above?",
                      default=True, assume_yes=(True if assume_yes else None)):
        info("Skipped.")
        return

    log(f"Downloading {label}")
    rc = scan_and_bind.main(argv)
    if rc != 0:
        err(f"The {label} scrape finished with errors (see the messages above).")


def run_dither(sd_root):
    """Best-effort, non-fatal regeneration of the theme quickmenu/topbg_dither.png overlays."""
    if shutil.which(os.environ.get("FFMPEG_BIN", "ffmpeg")) is None:
        return
    try:
        import dither_topbg
        log("Refreshing theme video overlays (topbg_dither.png)")
        dither_topbg.main([sd_root])
    except Exception as e:                            # noqa: BLE001 -- purely cosmetic, never fatal
        err(f"topbg_dither.png generation failed (continuing anyway): {e}")


# ---------------------------------------------------------------------------
# Safe-eject hint, tailored to the host OS.
# ---------------------------------------------------------------------------
def eject_hint(sd_root):
    system = platform.system()
    if system == "Darwin":
        return f'diskutil eject "{sd_root}"'
    if system == "Linux":
        return f'udisksctl unmount -b "{sd_root}"   # or use your file manager\'s "Eject"'
    if system == "Windows":
        return 'Use "Safely Remove Hardware" in the taskbar before pulling the card.'
    return "Safely unmount the SD card before removing it."


def main(argv):
    ap = argparse.ArgumentParser(
        description="Locates your SD card, installs dsimenu.srldr, and optionally scrapes "
                    "game logos and gameplay clips.")
    ap.add_argument("--sd", default=None, help="SD card path (skip auto-detection)")
    ap.add_argument("--srldr", default=os.path.join(HERE, "build", "dsimenu.srldr"),
                    help="path to the dsimenu.srldr to install "
                         "(default: build/dsimenu.srldr next to this script)")
    ap.add_argument("--themes", default=os.path.join(HERE, "themes"),
                    help="optional folder of SEVERAL theme subfolders to copy onto the SD "
                         "(default: themes/ next to this script, if present)")
    ap.add_argument("--default-theme", default=None,
                    help="path to a SINGLE theme folder (containing theme.ini) to install "
                         "and select as the DSi menu's default theme. Default: "
                         "auto-detect any such folder next to this script (e.g. "
                         "'Default grid theme').")
    ap.add_argument("--no-default-theme", action="store_true",
                    help="don't install/select a default theme")
    ap.add_argument("--scrape-only", "--skip-deploy", dest="scrape_only", action="store_true",
                    help="skip installing dsimenu.srldr/theme entirely; only scrape art. "
                         "Use this on an SD that already has the menu installed. If not "
                         "given (and --yes isn't either), you'll be asked interactively.")
    ap.add_argument("--yes", action="store_true",
                    help="assume 'yes' to every prompt (non-interactive; scrapes both "
                         "logos and clips without asking)")
    ap.add_argument("--no-logos", action="store_true", help="don't scrape logos")
    ap.add_argument("--no-clips", action="store_true", help="don't scrape gameplay clips")
    ap.add_argument("--force", action="store_true",
                    help="re-download media even for games that already have assets")
    ap.add_argument("--no-auto-download", action="store_true",
                    help="don't auto-download a portable ffmpeg if it's missing "
                         "(fail instead, so you can install it yourself)")
    ap.add_argument("--verbose", "-v", action="store_true",
                    help="show real-time detail during scraping: the exact Skyscraper "
                         "command for each batch, its own raised verbosity, and timing "
                         "per phase (gather/generate)")
    args = ap.parse_args(argv)

    print("TWiLightMenuGRID deploy tool")
    print("============================")

    sd_root = choose_sd(args.sd, assume_yes=args.yes)
    info(f"Using SD card: {sd_root}")

    if not args.scrape_only and not args.yes:
        args.scrape_only = ask_yes_no(
            "\nIs TWiLightMenuGRID already installed on this SD? "
            "(skip straight to scraping game art)", default=False)

    if not args.scrape_only:
        deploy_srldr_and_themes(sd_root, args.srldr, args.themes)

        if not args.no_default_theme:
            theme_dir = args.default_theme
            if theme_dir and not os.path.isfile(os.path.join(theme_dir, "theme.ini")):
                err(f"--default-theme path has no theme.ini: {theme_dir}")
                sys.exit(1)
            if not theme_dir:
                candidates = find_bundled_theme_dirs()
                if len(candidates) == 1:
                    theme_dir = candidates[0]
                elif len(candidates) > 1:
                    if args.yes:
                        theme_dir = candidates[0]
                        info(f"Multiple bundled themes found; defaulting to "
                             f"'{os.path.basename(theme_dir)}' (--yes). Use "
                             "--default-theme to pick another.")
                    else:
                        log("Multiple bundled themes found")
                        theme_dir = ask_choice(
                            "Which one should be the default DSi theme?", candidates)
            if theme_dir:
                install_default_theme(sd_root, theme_dir)
    else:
        info("Scrape-only: skipping dsimenu.srldr/theme installation.")
        os.makedirs(os.path.join(sd_root, DSIMENU_ASSETS_SUBPATH), exist_ok=True)

    want_logos = False if args.no_logos else (True if args.yes else
                 ask_yes_no("\nScrape game LOGOS now? (quick, no video download)", default=True))
    want_clips = False if args.no_clips else (True if args.yes else
                 ask_yes_no("Scrape gameplay CLIPS now? (slower, downloads + converts video)",
                            default=False))

    if want_logos or want_clips:
        if not check_scrape_requirements(assume_yes=args.yes,
                                         auto_download_ffmpeg=not args.no_auto_download):
            sys.exit(1)
        if want_logos:
            run_scrape(sd_root, logo_only=True, force=args.force, assume_yes=args.yes,
                      verbose=args.verbose)
        if want_clips:
            run_scrape(sd_root, logo_only=False, force=args.force, assume_yes=args.yes,
                      verbose=args.verbose)
        run_dither(sd_root)

    log("Done")
    print(f"dsimenu.srldr + assets live under: {os.path.join(sd_root, DSIMENU_SUBPATH)}")
    print(f"Before removing the card: {eject_hint(sd_root)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
