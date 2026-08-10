#!/usr/bin/env bash
# ---------------------------------------------------------------------------
# deploy_sd.sh — instala a build no SD card real do DSi:
#   1. Substitui  <SD>/_nds/TWiLightMenu/dsimenu.srldr  por dist/dsimenu.srldr
#      (fazendo backup do anterior em dsimenu.srldr.bak).
#   2. Atualiza a pasta de temas do cartão com a nossa do preview
#      (.preview/sdcard/_nds/TWiLightMenu/dsimenu/themes).
#
# Uso:
#   ./deploy_sd.sh                # detecta /Volumes/DSI ou /Volumes/DSi
#   ./deploy_sd.sh /Volumes/OUTRO # aponta um mount manualmente
# ---------------------------------------------------------------------------
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SRLDR="$ROOT/dist/dsimenu.srldr"
THEMES_SRC="$ROOT/.preview/sdcard/_nds/TWiLightMenu/dsimenu/themes"

# ---- localizar o SD ----
SD="${1:-}"
if [ -z "$SD" ]; then
	for cand in /Volumes/DSI /Volumes/DSi /Volumes/DSISD; do
		[ -d "$cand/_nds/TWiLightMenu" ] && SD="$cand" && break
	done
fi
if [ -z "$SD" ] || [ ! -d "$SD/_nds/TWiLightMenu" ]; then
	echo "!! SD do DSi não encontrado (procure /Volumes/DSI). Passe o mount como argumento." >&2
	exit 1
fi
echo ">> SD: $SD"

# ---- validações ----
if [ ! -f "$SRLDR" ]; then
	echo "!! $SRLDR não existe. Rode ./build.sh antes." >&2
	exit 1
fi
if [ ! -d "$THEMES_SRC" ]; then
	echo "!! Pasta de temas do preview não encontrada: $THEMES_SRC" >&2
	exit 1
fi

# ---- 1) dsimenu.srldr ----
DST_SRLDR="$SD/_nds/TWiLightMenu/dsimenu.srldr"
if [ -f "$DST_SRLDR" ]; then
	cp -f "$DST_SRLDR" "$SD/_nds/TWiLightMenu/dsimenu.srldr.bak"
	echo ">> Backup: dsimenu.srldr.bak"
fi
cp -f "$SRLDR" "$DST_SRLDR"
echo ">> Copiado dsimenu.srldr ($(du -h "$SRLDR" | cut -f1))"

# ---- 2) pasta de temas ----
DST_THEMES="$SD/_nds/TWiLightMenu/dsimenu/themes"
mkdir -p "$DST_THEMES"
echo ">> Atualizando temas -> $DST_THEMES"
if command -v rsync >/dev/null 2>&1; then
	# flags FAT-friendly: sem perms/owner, tolera 1s de diferença de timestamp
	rsync -rt --modify-window=1 --no-perms --no-owner --no-group \
		--exclude='.DS_Store' --exclude='._*' \
		"$THEMES_SRC/" "$DST_THEMES/"
else
	cp -R "$THEMES_SRC/." "$DST_THEMES/"
fi

# ---- flush ----
sync
echo ">> Concluído. Ejete o cartão com segurança antes de remover:"
echo "   diskutil eject \"$SD\""
