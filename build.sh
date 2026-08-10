#!/usr/bin/env bash
# ---------------------------------------------------------------------------
# build.sh — compila o frontend (romsel_dsimenutheme) e gera o dsimenu.srldr
# pronto para instalar no console.
#
# Saída:
#   dist/dsimenu.srldr   -> copiar para sd:/_nds/TWiLightMenu/dsimenu.srldr no DSi
#
# Uso:
#   ./build.sh            # compila e gera dist/dsimenu.srldr  (padrão)
#   ./build.sh --preview  # também copia para o SD de preview (.preview/sdcard)
# ---------------------------------------------------------------------------
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$ROOT"

NDS="$ROOT/romsel_dsimenutheme/romsel_dsimenutheme.nds"
DIST="$ROOT/dist"
SD="$ROOT/.preview/sdcard"
MODE="${1:-}"

# Garante a imagem docker de build (a mesma do compile_docker.sh).
if ! docker image inspect twilightmenu >/dev/null 2>&1; then
	echo ">> Imagem docker 'twilightmenu' ausente; buildando (pode demorar)..."
	docker build -t twilightmenu --label twilightmenu ./
fi

echo ">> Compilando romsel_dsimenutheme..."
docker run --rm -v "$ROOT:/data" twilightmenu make romsel_dsimenutheme

if [ ! -f "$NDS" ]; then
	echo "!! Build falhou: $NDS não foi gerado" >&2
	exit 1
fi

echo ">> Gerando dist/dsimenu.srldr..."
mkdir -p "$DIST"
cp "$NDS" "$DIST/dsimenu.srldr"

if [ "$MODE" = "--preview" ] && [ -d "$SD/_nds/TWiLightMenu" ]; then
	echo ">> Copiando para o SD de preview..."
	cp "$NDS" "$SD/dsimenu.nds"
	cp "$NDS" "$SD/_nds/TWiLightMenu/dsimenu.srldr"
fi

echo ">> Pronto: $DIST/dsimenu.srldr"
echo "   Instale em: sd:/_nds/TWiLightMenu/dsimenu.srldr"
