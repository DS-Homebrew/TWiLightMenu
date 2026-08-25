#!/usr/bin/env bash
# ---------------------------------------------------------------------------
# deploy_sd.sh — compila o receptor fileQR e copia o .nds para o SD do DSi.
#
# Uso:
#   ./deploy_sd.sh                 # detecta /Volumes/DSI (ou DSi/DSISD) e faz build
#   ./deploy_sd.sh /Volumes/OUTRO  # aponta um mount manualmente
#   ./deploy_sd.sh --no-build      # pula a compilação, só copia o .nds atual
#
# Onde grava:
#   <SD>/fileQR.nds            (aparece na lista do TWiLightMenu)
#   <SD>/fileQR/               (pasta de destino sugerida p/ os arquivos recebidos)
# ---------------------------------------------------------------------------
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
NDS="$ROOT/nds/fileQR.nds"

# ---- ambiente devkitPro ----
export DEVKITPRO="${DEVKITPRO:-/opt/devkitpro}"
export DEVKITARM="${DEVKITARM:-$DEVKITPRO/devkitARM}"
export PATH="$PATH:$DEVKITARM/bin:$DEVKITPRO/tools/bin"

# ---- args ----
SD=""
DO_BUILD=1
for a in "$@"; do
	case "$a" in
		--no-build) DO_BUILD=0 ;;
		/*)         SD="$a" ;;
		*)          echo "!! argumento desconhecido: $a" >&2; exit 1 ;;
	esac
done

# ---- build ----
if [ "$DO_BUILD" -eq 1 ]; then
	if [ ! -x "$DEVKITARM/bin/arm-none-eabi-gcc" ]; then
		echo "!! devkitARM não encontrado em $DEVKITARM. Ajuste DEVKITPRO/DEVKITARM." >&2
		exit 1
	fi
	echo ">> Compilando fileQR.nds..."
	make -C "$ROOT/nds"
fi

if [ ! -f "$NDS" ]; then
	echo "!! $NDS não existe. Rode sem --no-build para compilar." >&2
	exit 1
fi

# ---- localizar o SD ----
if [ -z "$SD" ]; then
	for cand in /Volumes/DSI /Volumes/DSi /Volumes/DSISD; do
		[ -d "$cand" ] && SD="$cand" && break
	done
fi
if [ -z "$SD" ] || [ ! -d "$SD" ]; then
	echo "!! SD do DSi não encontrado. Monte o cartão (ex.: /Volumes/DSI) ou passe o caminho:" >&2
	echo "   ./deploy_sd.sh /Volumes/SEU_SD" >&2
	exit 1
fi
echo ">> SD: $SD"

# ---- copiar ----
cp "$NDS" "$SD/fileQR.nds"
mkdir -p "$SD/fileQR"
sync
echo ">> Copiado:"
echo "   $SD/fileQR.nds  ($(du -h "$NDS" | cut -f1))"
echo "   $SD/fileQR/      (pasta de destino sugerida)"
echo ">> Pronto. Ejete o cartão com segurança e rode 'fileQR' pelo TWiLightMenu."
