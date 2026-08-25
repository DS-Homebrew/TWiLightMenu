#!/usr/bin/env bash
#
# fetch_ds_media.sh
#
# Downloads the LOGO (artwork type "wheel") and the FOOTAGE (gameplay video) of
# ONE Nintendo DS game from ScreenScraper, using Skyscraper, and saves both
# files into a destination folder.
#
# Skyscraper works in two phases:
#   1) Gather:   scrapes and populates the local cache  -> uses  -s screenscraper
#   2) Generate: builds the media files from the cache  -> runs WITHOUT  -s
# This script runs both phases, in that order.
#
# Credentials:
#   On first run the script prompts for the ScreenScraper username/password and
#   caches them next to the script (.ss_credentials). Later runs reuse the cache.
#   Environment variables SS_USER/SS_PASS, if set, take precedence over the cache.

# ---------------------------------------------------------------------------
# 1) Strict mode
# ---------------------------------------------------------------------------
set -euo pipefail

# ---------------------------------------------------------------------------
# Resolve the directory where this script lives. Used as the default output
# folder and as the location of the cached credentials.
# ---------------------------------------------------------------------------
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"
CRED_FILE="$SCRIPT_DIR/.ss_credentials.json"

# ---------------------------------------------------------------------------
# Usage header
# ---------------------------------------------------------------------------
usage() {
    cat >&2 <<'EOF'
Usage: fetch_ds_media.sh <ROM.nds> [DESTINATION_DIR]

Downloads the logo (wheel) and the gameplay video of a Nintendo DS game via
Skyscraper/ScreenScraper and saves them into the destination directory.

Arguments:
  <ROM.nds>          Path to the DS ROM file (.nds). Required.
  [DESTINATION_DIR]  Folder to save the logo and video. Optional.
                     Defaults to the folder where this script is located.
                     Created if missing.

Credentials:
  On the first run you are prompted for your ScreenScraper username and
  password; they are cached as JSON in ".ss_credentials.json" next to this
  script and reused on later runs. Set SS_USER/SS_PASS in the environment to
  override the cache without prompting. Requires `jq` or `python3` to read/
  write the JSON cache.

Environment variables:
  SS_USER          (optional) ScreenScraper username; overrides the cache.
  SS_PASS          (optional) ScreenScraper password; overrides the cache.
  SKYSCRAPER_BIN   (optional) Path to the Skyscraper binary. Default: "Skyscraper".
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

Output:
  <dest>/<base>-logo.png
  <dest>/<base>-top.tgrv       (top screen, TGR2 128x96 PAL8 8bpp, via split_ds_video.py)
  <dest>/<base>-bottom.tgrv    (bottom screen)
  <dest>/<base>-video.<ext>    (stacked mp4; removed if KEEP_MP4=0)
  (where <base> is the ROM filename without extension)
EOF
}

# Helper: error messages always go to stderr.
err() { printf 'ERROR: %s\n' "$*" >&2; }
# Progress logs go to stdout.
log() { printf '==> %s\n' "$*"; }

# ---------------------------------------------------------------------------
# JSON helpers (used for the credentials cache).
# Prefer `jq`; fall back to `python3`. Both handle escaping of special
# characters correctly, which a hand-rolled parser would not.
# ---------------------------------------------------------------------------
JSON_TOOL=""
detect_json_tool() {
    if command -v jq >/dev/null 2>&1; then
        JSON_TOOL="jq"
    elif command -v python3 >/dev/null 2>&1; then
        JSON_TOOL="python3"
    else
        JSON_TOOL=""
    fi
}

# json_write <file> <username> <password> : writes {"username":..,"password":..}
json_write() {
    local file="$1" u="$2" p="$3"
    case "$JSON_TOOL" in
        jq)
            jq -n --arg u "$u" --arg p "$p" \
               '{username:$u, password:$p}' > "$file"
            ;;
        python3)
            python3 - "$file" "$u" "$p" <<'PY'
import json, sys
file, u, p = sys.argv[1], sys.argv[2], sys.argv[3]
with open(file, "w") as fh:
    json.dump({"username": u, "password": p}, fh, ensure_ascii=False, indent=2)
    fh.write("\n")
PY
            ;;
    esac
}

# json_read_field <file> <field> : prints the field value (empty if missing).
json_read_field() {
    local file="$1" field="$2"
    case "$JSON_TOOL" in
        jq)
            jq -r --arg f "$field" '.[$f] // empty' "$file" 2>/dev/null
            ;;
        python3)
            python3 - "$file" "$field" <<'PY'
import json, sys
file, field = sys.argv[1], sys.argv[2]
try:
    with open(file) as fh:
        print(json.load(fh).get(field, ""))
except Exception:
    print("")
PY
            ;;
    esac
}

# ---------------------------------------------------------------------------
# Credential handling.
#
# load_credentials() populates SS_USER/SS_PASS following this precedence:
#   1) Environment variables SS_USER and SS_PASS (if both set and non-empty).
#   2) The JSON cache file ".ss_credentials.json" next to the script.
#   3) Interactive prompt (first run), which then writes the JSON cache.
#
# The cache is a JSON object {"username":..,"password":..}, written/read with
# jq or python3 so special characters are preserved. The file is chmod 600.
# ---------------------------------------------------------------------------
load_credentials() {
    # 1) Environment variables win — no JSON tool needed in this path.
    if [ -n "${SS_USER:-}" ] && [ -n "${SS_PASS:-}" ]; then
        log "Using ScreenScraper credentials from the environment."
        return 0
    fi

    # A JSON tool is required to read or create the cache.
    detect_json_tool
    if [ -z "$JSON_TOOL" ]; then
        err "The JSON credentials cache needs 'jq' or 'python3', but neither is installed."
        err "Install one of them, or set SS_USER and SS_PASS in the environment."
        exit 1
    fi

    # 2) JSON cache file, if present.
    if [ -f "$CRED_FILE" ]; then
        SS_USER="$(json_read_field "$CRED_FILE" username)"
        SS_PASS="$(json_read_field "$CRED_FILE" password)"
        if [ -n "${SS_USER:-}" ] && [ -n "${SS_PASS:-}" ]; then
            log "Loaded cached ScreenScraper credentials from: $CRED_FILE"
            return 0
        fi
        # Cache is present but incomplete/corrupted; fall through to prompt.
        err "Cached credentials file is invalid; re-entering credentials."
    fi

    # 3) Interactive prompt (first run). Requires a terminal.
    if [ ! -t 0 ] && [ ! -e /dev/tty ]; then
        err "No cached credentials and no terminal available to prompt."
        err "Set SS_USER and SS_PASS in the environment, or run interactively once."
        exit 1
    fi

    log "First run: please enter your ScreenScraper credentials."
    printf 'ScreenScraper username: ' > /dev/tty
    IFS= read -r SS_USER < /dev/tty
    printf 'ScreenScraper password: ' > /dev/tty
    IFS= read -r -s SS_PASS < /dev/tty
    printf '\n' > /dev/tty

    if [ -z "${SS_USER:-}" ] || [ -z "${SS_PASS:-}" ]; then
        err "Username and password must not be empty."
        exit 1
    fi

    # Persist the JSON cache with owner-only permissions.
    ( umask 077; json_write "$CRED_FILE" "$SS_USER" "$SS_PASS" )
    chmod 600 "$CRED_FILE" 2>/dev/null || true
    log "Credentials cached in: $CRED_FILE (delete this file to reset)."
}

# ---------------------------------------------------------------------------
# 2) Prerequisite validation (fail early with a helpful message)
# ---------------------------------------------------------------------------

# 2.1) At least the ROM argument is required.
if [ "$#" -lt 1 ]; then
    usage
    exit 2
fi

ROM="$1"
# Destination is optional: default to the script's own directory.
DEST="${2:-$SCRIPT_DIR}"

# 2.2) Skyscraper binary (overridable via SKYSCRAPER_BIN)
SKYSCRAPER_BIN="${SKYSCRAPER_BIN:-Skyscraper}"
if ! command -v "$SKYSCRAPER_BIN" >/dev/null 2>&1; then
    err "Skyscraper not found (looked for: '$SKYSCRAPER_BIN')."
    err "Install Skyscraper or point SKYSCRAPER_BIN to the correct binary."
    exit 1
fi
# Store the resolved absolute path and confirm it is executable.
SKYSCRAPER_BIN="$(command -v "$SKYSCRAPER_BIN")"
if [ ! -x "$SKYSCRAPER_BIN" ]; then
    err "The Skyscraper binary exists but is not executable: $SKYSCRAPER_BIN"
    exit 1
fi

# 2.3) The ROM must exist...
if [ ! -f "$ROM" ]; then
    err "ROM not found: $ROM"
    exit 1
fi
# ...and have a .nds extension (case-insensitive comparison).
rom_lower="$(printf '%s' "$ROM" | tr '[:upper:]' '[:lower:]')"
case "$rom_lower" in
    *.nds) : ;;  # ok
    *)
        err "The ROM does not have a .nds extension: $ROM"
        exit 1
        ;;
esac

# 2.4) Obtain ScreenScraper credentials (env, cache, or interactive prompt).
load_credentials

# ---------------------------------------------------------------------------
# Derive the ROM base name and directory.
#   base name: "/roms/Super Game (USA).nds" -> "Super Game (USA)"
#   directory: the folder that Skyscraper needs as its input folder (-i).
# Skyscraper always scrapes an INPUT FOLDER (-i); a positional filename only
# filters which files inside that folder are processed. So we point -i at the
# ROM's own directory and pass the bare filename as the filter.
# ---------------------------------------------------------------------------
ROM_FILENAME="$(basename "$ROM")"
BASE="${ROM_FILENAME%.*}"
ROM_DIR="$(cd "$(dirname "$ROM")" >/dev/null 2>&1 && pwd)"

# ---------------------------------------------------------------------------
# 3) Create the destination directory (if it does not exist).
# ---------------------------------------------------------------------------
mkdir -p "$DEST"
log "Output directory: $DEST"

# ---------------------------------------------------------------------------
# 8) Temporary work directory + guaranteed cleanup on EXIT (trap).
# ---------------------------------------------------------------------------
WORKDIR="$(mktemp -d "${TMPDIR:-/tmp}/skyscraper_nds.XXXXXX")"
cleanup() {
    # Remove the temporary directory, even on error.
    rm -rf "$WORKDIR"
}
trap cleanup EXIT

# ---------------------------------------------------------------------------
# 4) GATHER PHASE: populate the local cache from ScreenScraper.
#    - Targets only this ROM (passed as the last argument).
#    - `videos` enables video gathering (not gathered by default).
#    - `unattend,unattendskip` avoid interactive prompts.
#    We capture the output to detect identification failures / API quota.
# ---------------------------------------------------------------------------
log "Gather phase for: $ROM_FILENAME"

# LOGO_ONLY=1 skips video entirely (gather + generate + split): a cheap path used
# by refresh_logos.sh to re-download just the logo. Default: full media.
if [ "${LOGO_ONLY:-0}" = "1" ]; then
    MEDIA_FLAGS="unattend,unattendskip"
    log "LOGO_ONLY=1 — skipping video (logo only)."
else
    MEDIA_FLAGS="videos,unattend,unattendskip"
fi

GATHER_LOG="$WORKDIR/gather.log"
set +e  # inspect the output/exit code without aborting immediately
"$SKYSCRAPER_BIN" -p nds -i "$ROM_DIR" -s screenscraper -u "$SS_USER:$SS_PASS" \
    --flags "$MEDIA_FLAGS" "$ROM_FILENAME" 2>&1 | tee "$GATHER_LOG"
gather_rc="${PIPESTATUS[0]}"
set -e

# 4.1) Detect known error conditions in the gather output.
if grep -qiE 'API closed for non-registered members|quota|too many requests|request limit|Wrong username/password' "$GATHER_LOG"; then
    err "ScreenScraper rejected the request (bad credentials, API closed, or quota exhausted)."
    err "If your credentials are wrong, delete $CRED_FILE and run again."
    exit 3
fi

# 4.2) If Skyscraper found no game, there is no point generating.
if grep -qiE 'Games found:\s*0|No entries to scrape|found 0 game' "$GATHER_LOG"; then
    err "The gather phase did not identify the game for this ROM: $ROM_FILENAME"
    err "Check the ROM name/hash or whether the title exists on ScreenScraper."
    exit 3
fi

# 4.3) A non-zero exit code from gather = hard failure.
if [ "$gather_rc" -ne 0 ]; then
    err "Gather phase failed (exit code $gather_rc). See the output above."
    exit 3
fi

# ---------------------------------------------------------------------------
# 5) GENERATE PHASE: build the media from the cache.
#    - Runs WITHOUT `-s`.
#    - `-o "$WORKDIR"` writes the media into the temporary work directory.
# ---------------------------------------------------------------------------
log "Generate phase into: $WORKDIR"
"$SKYSCRAPER_BIN" -p nds -i "$ROM_DIR" -o "$WORKDIR" \
    --flags "$MEDIA_FLAGS" "$ROM_FILENAME"

# ---------------------------------------------------------------------------
# 6) Locate the generated files.
#    The default frontend (emulationstation) writes media DIRECTLY under the
#    output folder given to `-o` (there is no per-platform "/nds/" level):
#      <WORKDIR>/marquees/ -> the wheel logo
#                             (default artwork.xml maps: <output type="marquee"
#                              resource="wheel"/>, i.e. the raw transparent logo)
#      <WORKDIR>/videos/   -> the gameplay video
#    We search a priority list of candidate folders so a customized
#    artwork.xml that emits a real "wheels/" folder also works. The file base
#    name derives from the ROM name; since this script targets ONE ROM per run,
#    we simply pick the most recent file in the first matching folder.
# ---------------------------------------------------------------------------

# find_latest <dir> : prints the path of the most recent file (or nothing).
# Uses `ls -t` (sorts by mtime, newest first) instead of `find -printf`,
# because `-printf` is GNU-only and would break on BSD find (macOS).
find_latest() {
    local dir="$1" name
    [ -d "$dir" ] || return 0
    # List only the direct contents of the folder, newest first, take the first.
    name="$(cd "$dir" 2>/dev/null && ls -1t 2>/dev/null | head -n1)"
    [ -n "$name" ] && printf '%s/%s\n' "$dir" "$name"
    return 0
}

# find_latest_in <dir> [dir...] : returns the newest file from the first
# candidate directory that contains one. Lets us try several output folders.
find_latest_in() {
    local d found
    for d in "$@"; do
        found="$(find_latest "$d")"
        if [ -n "$found" ]; then
            printf '%s\n' "$found"
            return 0
        fi
    done
    return 0
}

# Logo (wheel): default lands in "marquees/"; "wheels/" covers custom artwork.
LOGO_SRC="$(find_latest_in "$WORKDIR/wheels" "$WORKDIR/marquees")"
# Video (footage): always "videos/". Skipped entirely under LOGO_ONLY.
if [ "${LOGO_ONLY:-0}" = "1" ]; then
    VIDEO_SRC=""
else
    VIDEO_SRC="$(find_latest_in "$WORKDIR/videos")"
fi

# ---------------------------------------------------------------------------
# 7) Copy/rename into the destination using the ROM base name.
#    - Logo  -> <dest>/<base>-logo.png
#    - Video -> <dest>/<base>-video.<original_ext>  (preserve the extension)
# ---------------------------------------------------------------------------
got_logo=0
got_video=0

# 7.1) Logo (wheel) — saved, then downscaled to cut DS load time.
if [ -n "$LOGO_SRC" ] && [ -f "$LOGO_SRC" ]; then
    LOGO_DEST="$DEST/$BASE-logo.png"
    cp -f "$LOGO_SRC" "$LOGO_DEST"
    log "Logo saved to: $LOGO_DEST"
    got_logo=1

    # Downscale (only if larger) to reduce PNG decode time on the DS. Preserves the
    # aspect ratio and the alpha channel. Configurable via env:
    #   LOGO_DOWNSCALE=0     disable
    #   LOGO_MAX_WIDTH=<n>   max width  (default 256 = DS screen width)
    #   LOGO_MAX_HEIGHT=<n>  max height (default 128)
    if [ "${LOGO_DOWNSCALE:-1}" != "0" ] && command -v ffmpeg >/dev/null 2>&1; then
        lw="${LOGO_MAX_WIDTH:-256}"
        lh="${LOGO_MAX_HEIGHT:-128}"
        tmp_logo="$DEST/.logo_scaled_$$.png"
        if ffmpeg -y -v error -i "$LOGO_DEST" \
             -vf "scale='min($lw,iw)':'min($lh,ih)':force_original_aspect_ratio=decrease" \
             -pix_fmt rgba "$tmp_logo" 2>/dev/null; then
            mv -f "$tmp_logo" "$LOGO_DEST"
            log "Logo downscaled to fit ${lw}x${lh} (alpha preserved)."
        else
            rm -f "$tmp_logo"
            err "Logo downscale failed; kept the original size."
        fi
    fi
else
    err "Logo (wheel) not found — the game may not have this asset on ScreenScraper."
fi

# 7.2) Video (footage) — preserve the original extension. Skipped under LOGO_ONLY.
if [ "${LOGO_ONLY:-0}" = "1" ]; then
    :  # logo-only run: no video expected
elif [ -n "$VIDEO_SRC" ] && [ -f "$VIDEO_SRC" ]; then
    video_filename="$(basename "$VIDEO_SRC")"
    video_ext="${video_filename##*.}"
    VIDEO_DEST="$DEST/$BASE-video.$video_ext"
    cp -f "$VIDEO_SRC" "$VIDEO_DEST"
    log "Video saved to: $VIDEO_DEST"
    got_video=1
else
    err "Video (footage) not found — the game may not have this asset on ScreenScraper."
fi

# ---------------------------------------------------------------------------
# 7.3) Split the stacked video into two DS-ready TGRV files (top/bottom screens).
#    Produces "<base>-top.tgrv" and "<base>-bottom.tgrv" via split_ds_video.py.
#    Non-fatal: if it fails / is disabled / ffmpeg is missing, we keep the mp4.
#    Configurable via env:
#      SPLIT_VIDEO=0        disable the split entirely
#      TGRV_FPS=<n>         frames per second (default 12)
#      TGRV_WIDTH=<n>       stored width per screen (default 128; DS upscales)
#      TGRV_HEIGHT=<n>      stored height per screen (default 96)
#      TGRV_MODE=<m>        color mode: pal8 (8bpp, default) or bgr555 (16bpp)
#      TGRV_DITHER=<d>      paletteuse dither for pal8 (default bayer:bayer_scale=3)
#      TGRV_MAX_SECONDS=<n> cap duration (default: full video)
#      KEEP_MP4=0           delete the intermediate mp4 after a successful split
# ---------------------------------------------------------------------------
SPLIT_SCRIPT="$SCRIPT_DIR/split_ds_video.py"
if [ "$got_video" -eq 1 ] && [ "${SPLIT_VIDEO:-1}" != "0" ]; then
    if command -v python3 >/dev/null 2>&1 && command -v ffmpeg >/dev/null 2>&1 \
       && [ -f "$SPLIT_SCRIPT" ]; then
        split_args=(--fps "${TGRV_FPS:-12}"
                    --width "${TGRV_WIDTH:-128}"
                    --height "${TGRV_HEIGHT:-96}"
                    --mode "${TGRV_MODE:-pal8}")
        [ -n "${TGRV_DITHER:-}" ] && split_args+=(--dither "$TGRV_DITHER")
        [ -n "${TGRV_MAX_SECONDS:-}" ] && split_args+=(--max-seconds "$TGRV_MAX_SECONDS")
        log "Splitting video into DS TGRV (top/bottom)..."
        if python3 "$SPLIT_SCRIPT" "$VIDEO_DEST" "$DEST" "${split_args[@]}"; then
            # Remove the intermediate mp4 unless the user wants to keep it.
            if [ "${KEEP_MP4:-1}" = "0" ]; then
                rm -f "$VIDEO_DEST"
                log "Removed intermediate mp4 (KEEP_MP4=0)."
            fi
        else
            err "Video split failed; keeping the mp4 as-is."
        fi
    else
        err "Skipping split (need python3 + ffmpeg + split_ds_video.py). Kept the mp4."
    fi
fi

# ---------------------------------------------------------------------------
# Final result:
#  - If NEITHER asset was found, exit with an error.
#  - If at least one was saved, success (warnings already went to stderr).
# ---------------------------------------------------------------------------
if [ "$got_logo" -eq 0 ] && [ "$got_video" -eq 0 ]; then
    err "No asset (logo or video) was found for: $ROM_FILENAME"
    exit 4
fi

log "Done."
exit 0
