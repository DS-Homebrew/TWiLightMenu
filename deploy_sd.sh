#!/usr/bin/env bash
# ---------------------------------------------------------------------------
# deploy_sd.sh -- installs the build onto a real DSi SD card (developer tool):
#   1. Replaces  <SD>/_nds/TWiLightMenu/dsimenu.srldr  with dist/dsimenu.srldr
#      (backing up the previous one as dsimenu.srldr.bak).
#   2. Updates the card's theme folder from our preview one
#      (.preview/sdcard/_nds/TWiLightMenu/dsimenu/themes).
#
# This is a developer convenience script: it expects a local build (dist/dsimenu.srldr,
# produced by build.sh) and the .preview/sdcard checkout. End users installing a release
# build should use Injector/deploy.py instead, which also handles game art scraping.
#
# Usage:
#   ./deploy_sd.sh                 # auto-detects the SD card mount point
#   ./deploy_sd.sh /Volumes/OTHER  # point at a mount manually
# ---------------------------------------------------------------------------
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SRLDR="$ROOT/dist/dsimenu.srldr"
THEMES_SRC="$ROOT/.preview/sdcard/_nds/TWiLightMenu/dsimenu/themes"

# ---- locate the SD ----
SD="${1:-}"
if [ -z "$SD" ]; then
	# macOS mount points.
	for cand in /Volumes/DSI /Volumes/DSi /Volumes/DSISD; do
		[ -d "$cand/_nds/TWiLightMenu" ] && SD="$cand" && break
	done
fi
if [ -z "$SD" ]; then
	# Linux auto-mount points (GNOME/KDE typically mount under one of these).
	for base in "/media/${USER:-}" "/run/media/${USER:-}"; do
		[ -d "$base" ] || continue
		for cand in "$base"/*; do
			[ -d "$cand/_nds/TWiLightMenu" ] && SD="$cand" && break 2
		done
	done
fi
if [ -z "$SD" ] || [ ! -d "$SD/_nds/TWiLightMenu" ]; then
	echo "!! DSi SD card not found (looked under /Volumes and /media). Pass the mount point as an argument." >&2
	exit 1
fi
echo ">> SD: $SD"

# ---- validation ----
if [ ! -f "$SRLDR" ]; then
	echo "!! $SRLDR does not exist. Run ./build.sh first." >&2
	exit 1
fi
if [ ! -d "$THEMES_SRC" ]; then
	echo "!! Preview themes folder not found: $THEMES_SRC" >&2
	exit 1
fi

# ---- 1) dsimenu.srldr ----
DST_SRLDR="$SD/_nds/TWiLightMenu/dsimenu.srldr"
if [ -f "$DST_SRLDR" ]; then
	cp -f "$DST_SRLDR" "$SD/_nds/TWiLightMenu/dsimenu.srldr.bak"
	echo ">> Backup: dsimenu.srldr.bak"
fi
cp -f "$SRLDR" "$DST_SRLDR"
echo ">> Copied dsimenu.srldr ($(du -h "$SRLDR" | cut -f1))"

# ---- 2) themes folder ----
DST_THEMES="$SD/_nds/TWiLightMenu/dsimenu/themes"
mkdir -p "$DST_THEMES"
echo ">> Updating themes -> $DST_THEMES"
if command -v rsync >/dev/null 2>&1; then
	# FAT-friendly flags: no perms/owner, tolerate a 1s timestamp difference.
	rsync -rt --modify-window=1 --no-perms --no-owner --no-group \
		--exclude='.DS_Store' --exclude='._*' \
		"$THEMES_SRC/" "$DST_THEMES/"
else
	cp -R "$THEMES_SRC/." "$DST_THEMES/"
fi

# ---- flush ----
sync
echo ">> Done. Safely eject the card before removing it:"
echo "   diskutil eject \"$SD\"   # macOS"
echo "   udisksctl unmount -b \"$SD\"   # Linux"
