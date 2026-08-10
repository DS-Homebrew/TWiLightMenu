#!/usr/bin/env bash
# ---------------------------------------------------------------------------
# preview.sh — compila o frontend (romsel_dsimenutheme), copia pro SD de
# preview e (re)lança o melonDS com a nossa build.
#
# Uso:
#   ./preview.sh              # compila, copia e lança  (padrão)
#   ./preview.sh --no-build   # pula a compilação, só copia a build atual e lança
#   ./preview.sh --launch     # só relança o melonDS (sem compilar nem copiar)
# ---------------------------------------------------------------------------
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$ROOT"

NDS="$ROOT/romsel_dsimenutheme/romsel_dsimenutheme.nds"
SD="$ROOT/.preview/sdcard"
MELONDS="/Applications/melonDS.app/Contents/MacOS/melonDS"
MODE="${1:-}"

if [ ! -x "$MELONDS" ]; then
	echo "!! melonDS não encontrado em: $MELONDS" >&2
	exit 1
fi

if [ "$MODE" != "--launch" ]; then
	if [ "$MODE" != "--no-build" ]; then
		# Garante a imagem docker de build (a mesma do compile_docker.sh).
		if ! docker image inspect twilightmenu >/dev/null 2>&1; then
			echo ">> Imagem docker 'twilightmenu' ausente; buildando (pode demorar)..."
			docker build -t twilightmenu --label twilightmenu ./
		fi
		echo ">> Compilando romsel_dsimenutheme..."
		docker run --rm -v "$ROOT:/data" twilightmenu make romsel_dsimenutheme
	fi

	echo ">> Copiando build pro SD de preview..."
	cp "$NDS" "$SD/dsimenu.nds"
	cp "$NDS" "$SD/_nds/TWiLightMenu/dsimenu.srldr"
fi

echo ">> (Re)lançando melonDS..."
pkill -x melonDS 2>/dev/null || true
sleep 1

# Limpa o cache de folder-sync do melonDS (dsisd.bin). Ele cresce a cada lançamento e, quando
# fica grande (centenas de MB / GB), o melonDS crasha ao bootar um jogo (chainload do
# nds-bootstrap) => tela branca. Limpar aqui mantém o boot funcionando no preview.
rm -f "$HOME/Library/Preferences/melonDS/dsisd.bin" "$HOME/Library/Preferences/melonDS/dsisd.bin.idx"
rm -f "$SD/_nds/pagefile.sys" "$SD/NDSBTSRP.LOG"

"$MELONDS" "$SD/dsimenu.nds" >/tmp/melonds_preview.log 2>&1 &
echo ">> melonDS iniciado (PID $!). Log: /tmp/melonds_preview.log"
