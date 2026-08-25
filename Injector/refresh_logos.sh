#!/usr/bin/env bash
#
# refresh_logos.sh — rebaixa SÓ os logos dos jogos do SD e substitui os antigos.
#
# Passo cirúrgico: para cada jogo no SD, baixa só o LOGO (sem vídeo) via
# fetch_ds_media.sh (LOGO_ONLY=1) e sobrescreve
# <SD>/_nds/TWiLightMenu/dsimenu/assets/<sha1>/logo.png. Vídeos (.tgrv) e
# manifest.yml NÃO são tocados. Útil após mudar o processamento de logo.
#
# O destino dos assets é derivado automaticamente da nossa estrutura no SD
# (<SD>/_nds/TWiLightMenu/dsimenu) — não é passado na linha de comando.
#
# Só um invólucro do assetbind/refresh_logos.py (que reaproveita a varredura por
# hash do scan_and_bind.py). As credenciais do ScreenScraper são tratadas pelo
# fetch_ds_media.sh (env SS_USER/SS_PASS, cache ou prompt no primeiro uso).

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"
REFRESH="$ROOT/assetbind/refresh_logos.py"
FETCH="$ROOT/fetch_ds_media.sh"

err() { printf 'ERROR: %s\n' "$*" >&2; }
log() { printf '\n\033[1;34m== %s ==\033[0m\n' "$*"; }

usage() {
    cat >&2 <<'EOF'
Uso: refresh_logos.sh <SD_CARD_DIR> [flags]

Rebaixa apenas o logo de cada jogo do SD e substitui o antigo em
<SD>/_nds/TWiLightMenu/dsimenu/assets/<sha1>/logo.png (destino automático,
conforme a estrutura do SD). Vídeos e manifest.yml permanecem intactos.

Argumentos:
  <SD_CARD_DIR>   Raiz do SD card a escanear. Obrigatório.

Flags (repassadas ao refresh_logos.py):
  --only-existing   Só atualiza jogos que JÁ têm assets/<sha1>/ (não cria novos).
  --dry-run         Lista o que faria, sem baixar nem escrever.

Variáveis de ambiente:
  SS_USER, SS_PASS   Credenciais do ScreenScraper (senão o fetch pergunta/usa cache).
  SKYSCRAPER_BIN     (opcional) caminho do binário Skyscraper.
  LOGO_MAX_WIDTH / LOGO_MAX_HEIGHT / LOGO_DOWNSCALE
                     (opcional) parâmetros do logo (ver fetch_ds_media.sh).

Exemplos:
  ./refresh_logos.sh /Volumes/DSI
  ./refresh_logos.sh /Volumes/DSI --only-existing
  ./refresh_logos.sh /Volumes/DSI --dry-run
EOF
}

case "${1:-}" in
    -h|--help|help) usage; exit 0 ;;
esac
if [ "$#" -lt 1 ]; then
    usage; exit 2
fi

SD="$1"; shift
# O que sobrar são flags repassadas ao Python (--only-existing, --dry-run).
EXTRA_ARGS=("$@")

command -v python3 >/dev/null 2>&1 || { err "python3 não encontrado."; exit 1; }
command -v ffmpeg  >/dev/null 2>&1 || { err "ffmpeg não encontrado (brew install ffmpeg)."; exit 1; }
[ -d "$SD" ]        || { err "SD card não encontrado: $SD"; exit 1; }
[ -f "$REFRESH" ]   || { err "refresh_logos.py não encontrado: $REFRESH"; exit 1; }
[ -f "$FETCH" ]     || { err "fetch_ds_media.sh não encontrado: $FETCH"; exit 1; }

# O destino (assets/) é resolvido pelo refresh_logos.py a partir de <SD>:
#   <SD>/_nds/TWiLightMenu/dsimenu/assets/<sha1>/logo.png
log "Refresh de logos no SD: $SD"
python3 "$REFRESH" --sd "$SD" --fetch-script "$FETCH" \
    ${EXTRA_ARGS[@]+"${EXTRA_ARGS[@]}"}
