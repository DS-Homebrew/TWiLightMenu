#!/usr/bin/env bash
# ---------------------------------------------------------------------------
# build.sh -- compiles the frontend (romsel_dsimenutheme) and produces the
# dsimenu.srldr ready to install on the console.
#
# Output:
#   dist/dsimenu.srldr   -> copy to sd:/_nds/TWiLightMenu/dsimenu.srldr on the DSi
#
# Usage:
#   ./build.sh            # compiles and generates dist/dsimenu.srldr  (default)
#   ./build.sh --preview  # also copies it to the preview SD (.preview/sdcard)
# ---------------------------------------------------------------------------
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$ROOT"

NDS="$ROOT/romsel_dsimenutheme/romsel_dsimenutheme.nds"
DIST="$ROOT/dist"
SD="$ROOT/.preview/sdcard"
MODE="${1:-}"

# Ensures the build docker image exists (the same one compile_docker.sh uses).
if ! docker image inspect twilightmenu >/dev/null 2>&1; then
	echo ">> Docker image 'twilightmenu' missing; building it (this can take a while)..."
	docker build -t twilightmenu --label twilightmenu ./
fi

echo ">> Compiling romsel_dsimenutheme..."
docker run --rm -v "$ROOT:/data" twilightmenu make romsel_dsimenutheme

if [ ! -f "$NDS" ]; then
	echo "!! Build failed: $NDS was not generated" >&2
	exit 1
fi

echo ">> Generating dist/dsimenu.srldr..."
mkdir -p "$DIST"
cp "$NDS" "$DIST/dsimenu.srldr"

if [ "$MODE" = "--preview" ] && [ -d "$SD/_nds/TWiLightMenu" ]; then
	echo ">> Copying to the preview SD..."
	cp "$NDS" "$SD/dsimenu.nds"
	cp "$NDS" "$SD/_nds/TWiLightMenu/dsimenu.srldr"
fi

echo ">> Ready: $DIST/dsimenu.srldr"
echo "   Install it at: sd:/_nds/TWiLightMenu/dsimenu.srldr"
