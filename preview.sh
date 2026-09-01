#!/usr/bin/env bash
# ---------------------------------------------------------------------------
# preview.sh -- compiles the frontend (romsel_dsimenutheme), copies it to the
# preview SD, and (re)launches melonDS with our build.
#
# Usage:
#   ./preview.sh              # compile, copy and launch  (default)
#   ./preview.sh --no-build   # skip compiling, just copy the current build and launch
#   ./preview.sh --launch     # just relaunch melonDS (no compile, no copy)
# ---------------------------------------------------------------------------
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$ROOT"

NDS="$ROOT/romsel_dsimenutheme/romsel_dsimenutheme.nds"
SD="$ROOT/.preview/sdcard"
MELONDS="/Applications/melonDS.app/Contents/MacOS/melonDS"
MODE="${1:-}"

if [ ! -x "$MELONDS" ]; then
	echo "!! melonDS not found at: $MELONDS" >&2
	exit 1
fi

if [ "$MODE" != "--launch" ]; then
	if [ "$MODE" != "--no-build" ]; then
		# Ensures the build docker image exists (the same one compile_docker.sh uses).
		if ! docker image inspect twilightmenu >/dev/null 2>&1; then
			echo ">> Docker image 'twilightmenu' missing; building it (this can take a while)..."
			docker build -t twilightmenu --label twilightmenu ./
		fi
		echo ">> Compiling romsel_dsimenutheme..."
		docker run --rm -v "$ROOT:/data" twilightmenu make romsel_dsimenutheme
	fi

	echo ">> Copying the build to the preview SD..."
	cp "$NDS" "$SD/dsimenu.nds"
	cp "$NDS" "$SD/_nds/TWiLightMenu/dsimenu.srldr"
fi

echo ">> (Re)launching melonDS..."
pkill -x melonDS 2>/dev/null || true
sleep 1

# Clears melonDS's folder-sync cache (dsisd.bin). It grows on every launch and, once it gets
# large (hundreds of MB / a few GB), melonDS crashes when booting a game (nds-bootstrap
# chainload) => white screen. Clearing it here keeps boot working in preview.
rm -f "$HOME/Library/Preferences/melonDS/dsisd.bin" "$HOME/Library/Preferences/melonDS/dsisd.bin.idx"
rm -f "$SD/_nds/pagefile.sys" "$SD/NDSBTSRP.LOG"

"$MELONDS" "$SD/dsimenu.nds" >/tmp/melonds_preview.log 2>&1 &
echo ">> melonDS started (PID $!). Log: /tmp/melonds_preview.log"
