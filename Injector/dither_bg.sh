#!/usr/bin/env bash
#
# dither_bg.sh — (re)gera os fundos "dithered" da tela superior dos temas DSi.
#
# Passo isolado do pipeline: para CADA tema em
#   <root>/_nds/TWiLightMenu/dsimenu/themes/<TEMA>/quickmenu/
# cria/substitui  topbg_dither.png  a partir do  topbg.png  original — uma cópia
# RGBA com alpha por dithering, escolhível entre vários padrões (bayer8/4/2,
# cluster, hlines, vlines, checker, noise). Pixels opacos mostram o brick; buracos
# deixam o vídeo aparecer atrás. O topbg.png original NUNCA é tocado; se o
# topbg_dither.png já existir, é REGERADO (overwrite).
#
# É só um invólucro fino do dither_topbg.py, para ajustar o fundo sem rodar o
# pipeline completo do run_sdcard.sh. Use para experimentar intensidade/padrão.

set -euo pipefail

# Diretório deste script (acha o dither_topbg.py ao lado).
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"
DITHER="$SCRIPT_DIR/dither_topbg.py"

err() { printf 'ERROR: %s\n' "$*" >&2; }
log() { printf '\n\033[1;34m== %s ==\033[0m\n' "$*"; }

usage() {
    cat >&2 <<'EOF'
Uso:
  dither_bg.sh <SD_CARD_DIR> [INTENSIDADE] [PADRAO]
  dither_bg.sh patterns          # lista os padrões de dithering (helper)
  dither_bg.sh help | -h         # esta ajuda

(Re)gera quickmenu/topbg_dither.png para cada tema DSi. Gera um novo se não
existir, ou substitui a versão antiga por uma nova (útil p/ testar variações).
O topbg.png original nunca é alterado.

Argumentos:
  <SD_CARD_DIR>   Raiz do SD (contém _nds/TWiLightMenu/dsimenu/themes/). Obrigatório.
  [INTENSIDADE]   1..100 = % de pixels OPACOS (visibilidade do brick). Padrão 40.
                    100 -> todo brick (sem buracos); 40 -> ~40% brick / 60% vídeo;
                      1 -> quase tudo transparente.
  [PADRAO]        Tipo de dithering. Padrão bayer8. Rode 'dither_bg.sh patterns'
                  para ver todos.

Exemplos:
  ./dither_bg.sh /Volumes/DSI 55
  ./dither_bg.sh /Volumes/DSI 40 cluster
  ./dither_bg.sh ./.preview/sdcard 30 noise
  ./dither_bg.sh patterns
EOF
}

# ---------------------------------------------------------------------------
# Pré-requisitos (necessários também para o helper 'patterns').
# ---------------------------------------------------------------------------
command -v python3 >/dev/null 2>&1 || { err "python3 não encontrado."; exit 1; }
command -v ffmpeg  >/dev/null 2>&1 || { err "ffmpeg não encontrado (brew install ffmpeg)."; exit 1; }
[ -f "$DITHER" ] || { err "dither_topbg.py não encontrado: $DITHER"; exit 1; }

# ---------------------------------------------------------------------------
# Subcomandos de ajuda / helper de comandos.
# ---------------------------------------------------------------------------
case "${1:-}" in
    -h|--help|help)     usage; exit 0 ;;
    patterns|--patterns|list|--list-patterns)
        python3 "$DITHER" --list-patterns; exit 0 ;;
esac

# ---------------------------------------------------------------------------
# Parse dos argumentos posicionais: <SD_CARD_DIR> [INTENSIDADE] [PADRAO].
# ---------------------------------------------------------------------------
if [ "$#" -lt 1 ]; then
    usage; exit 2
fi
ROOT="$1"
INTENSITY="${2:-40}"
PATTERN="${3:-bayer8}"

# Validação básica da intensidade (a checagem final é feita no Python).
case "$INTENSITY" in
    ''|*[!0-9]*) err "INTENSIDADE deve ser um inteiro entre 1 e 100."; exit 2 ;;
esac

# ---------------------------------------------------------------------------
# Executa. O dither_topbg.py resolve <root>, varre os temas e (re)gera os PNGs.
# O padrão é validado lá (choices); um nome inválido gera erro com a lista.
# ---------------------------------------------------------------------------
log "Ajustando fundos (dithering) — ${INTENSITY}% opacos, padrão '${PATTERN}'"
python3 "$DITHER" "$ROOT" --intensity "$INTENSITY" --pattern "$PATTERN"
