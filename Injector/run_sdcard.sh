#!/usr/bin/env bash
#
# run_sdcard.sh — pipeline completo do GridFootage para um SD card de Nintendo DS.
#
# Amarra as peças numa única execução, em três fases visíveis:
#   1) LISTAR   — escaneia o SD e lista os jogos encontrados (ignora apps de sistema
#                 via blocklist; duplicatas por conteúdo colapsam em um jogo só).
#   2) BAIXAR   — para cada jogo do scan, baixa LOGO + VÍDEO chamando o
#                 fetch_ds_media.sh (Skyscraper + ScreenScraper).
#   3) BINDAR   — organiza os assets em assets/<sha1>/ (espelhado no SD) e escreve o
#                 manifest.yml, vinculando cada ROM pelo HASH do conteúdo.
#
# As fases 2 e 3 são executadas pelo assetbind/scan_and_bind.py (que já dedup por
# hash, é idempotente e chama o fetch_ds_media.sh por jogo). A fase 1 usa o modo
# --list-only do mesmo script, que apenas mostra o que será processado.
#
# Credenciais do ScreenScraper: tratadas pelo fetch_ds_media.sh (env SS_USER/SS_PASS,
# cache .ss_credentials.json, ou prompt no primeiro uso). Este .sh só as repassa.

set -euo pipefail

# ---------------------------------------------------------------------------
# Localiza a raiz do projeto (onde este script vive) e as ferramentas.
# ---------------------------------------------------------------------------
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"
FETCH="$ROOT/fetch_ds_media.sh"
SCAN="$ROOT/assetbind/scan_and_bind.py"
DITHER="$ROOT/dither_topbg.py"

err() { printf 'ERROR: %s\n' "$*" >&2; }
log() { printf '\n\033[1;34m== %s ==\033[0m\n' "$*"; }

usage() {
    cat >&2 <<EOF
Uso: run_sdcard.sh <SD_CARD_DIR> [OUT_DIR]

Escaneia o SD, baixa o logo+vídeo de cada jogo e binda os assets por hash.
Todo o download/processamento acontece numa pasta de trabalho local (cache/);
só ao final as pastas são montadas no SD e o conteúdo é movido para lá — o SD
é apenas LIDO durante a varredura, minimizando escrita/processamento no cartão.

Argumentos:
  <SD_CARD_DIR>   Raiz do SD card a escanear. Obrigatório.
  [OUT_DIR]       Destino final no SD (assets/ + manifest.yml + assets_index.yml).
                  Padrão: <SD>/_nds/TWiLightMenu/dsimenu (onde o TWiLightMenu lê).

Variáveis de ambiente:
  SS_USER, SS_PASS   Credenciais do ScreenScraper (senão o fetch pergunta/usa cache).
  SKYSCRAPER_BIN     (opcional) caminho do binário Skyscraper.
  TGRV_FPS           (opcional) fps dos .tgrv gerados (padrão 12).
  TGRV_WIDTH         (opcional) largura armazenada por tela (padrão 128; DS reescala).
  TGRV_HEIGHT        (opcional) altura armazenada por tela (padrão 96).
  TGRV_MODE          (opcional) cor: pal8 (8bpp, padrão) ou bgr555 (16bpp).
  TGRV_MAX_SECONDS   (opcional) limita a duração dos .tgrv (0 = vídeo inteiro).
  SPLIT_VIDEO=0      (opcional) desliga a divisão em .tgrv (mantém só o mp4).
  LOGO_MAX_WIDTH     (opcional) largura máx. do logo em px (padrão 256).
  LOGO_MAX_HEIGHT    (opcional) altura máx. do logo em px (padrão 128).
  LOGO_DOWNSCALE=0   (opcional) mantém o logo no tamanho original.
  DITHER_TOPBG=0     (opcional) desliga a geração dos topbg_dither.png dos temas.
  DITHER_INTENSITY   (opcional) 1..100 = %% de pixels opacos (brick). Padrão 40.

Flags (opcionais, após os argumentos):
  --fresh         limpa qualquer vestígio de execuções anteriores do run_sdcard.sh
                  (remove <OUT>/assets, <OUT>/manifest.yml, <OUT>/assets_index.yml e
                  esvazia o cache) ANTES de rodar, para receber as versões novas.
                  Preserva logos/, logos.yml e themes/.
  --no-download   só (re)organiza assets já existentes e reescreve o manifesto.
  --force         rebaixa mesmo se assets/<sha1>/ já existir.
EOF
}

# ---------------------------------------------------------------------------
# Pré-requisitos.
# ---------------------------------------------------------------------------
if [ "$#" -lt 1 ]; then
    usage
    exit 2
fi

SD="$1"; shift
OUT=""
# Se o próximo argumento não for uma flag (--...), é o OUT_DIR.
if [ "$#" -gt 0 ] && [ "${1#--}" = "$1" ]; then
    OUT="$1"; shift
fi
# O que sobrar são flags. Interceptamos --fresh aqui (é do run_sdcard, não do Python);
# as demais (ex.: --no-download, --force) são repassadas ao scan_and_bind.py.
FRESH=0
EXTRA_ARGS=()
for a in "$@"; do
    if [ "$a" = "--fresh" ]; then
        FRESH=1
    else
        EXTRA_ARGS+=("$a")
    fi
done

command -v python3 >/dev/null 2>&1 || { err "python3 não encontrado."; exit 1; }
[ -d "$SD" ]        || { err "SD card não encontrado: $SD"; exit 1; }
[ -f "$SCAN" ]      || { err "scan_and_bind.py não encontrado: $SCAN"; exit 1; }
[ -f "$FETCH" ]     || { err "fetch_ds_media.sh não encontrado: $FETCH"; exit 1; }

# Destino padrão: a pasta que a versão modificada do TWiLightMenu lê no SD.
# (Se OUT_DIR não for informado, os assets vão para <SD>/_nds/TWiLightMenu/dsimenu.)
if [ -z "$OUT" ]; then
    OUT="$SD/_nds/TWiLightMenu/dsimenu"
fi
mkdir -p "$OUT"

# Pasta de trabalho local (mesma default do scan_and_bind.py).
CACHE="$ROOT/cache"

# ---------------------------------------------------------------------------
# --fresh: remove SÓ os artefatos produzidos pelo run_sdcard.sh (assets/,
# manifest.yml, assets_index.yml) e esvazia o cache. NÃO toca em logos/,
# logos.yml nem themes/ (são de outras features / do próprio TWiLightMenu).
# ---------------------------------------------------------------------------
if [ "$FRESH" -eq 1 ]; then
    log "FRESH — limpando vestígios de execuções anteriores"
    for stale in "$OUT/assets" "$OUT/manifest.yml" "$OUT/assets_index.yml"; do
        if [ -e "$stale" ]; then
            printf '   removendo: %s\n' "$stale"
            rm -rf "$stale"
        fi
    done
    if [ -d "$CACHE" ]; then
        printf '   esvaziando cache: %s\n' "$CACHE"
        rm -rf "${CACHE:?}/"* 2>/dev/null || true
    fi
fi

# Monta os argumentos comuns do scanner (sempre com --out e --cache explícitos).
SCAN_COMMON=(--sd "$SD" --out "$OUT" --cache "$CACHE" --fetch-script "$FETCH")

# ---------------------------------------------------------------------------
# FASE 1 — LISTAR os jogos presentes (sem baixar nem escrever nada).
# ---------------------------------------------------------------------------
log "FASE 1/3 — Listando jogos no SD"
python3 "$SCAN" "${SCAN_COMMON[@]}" --list-only

# ---------------------------------------------------------------------------
# FASE 2 + 3 — BAIXAR a mídia de cada jogo (via fetch_ds_media.sh) e BINDAR
# (organizar assets/<sha1>/ + escrever manifest.yml). Feito numa passada só pelo
# scan_and_bind.py, que é idempotente e dedup por hash.
# ---------------------------------------------------------------------------
log "FASE 2/3 — Baixando mídia (logo + vídeo) dos jogos encontrados"
log "FASE 3/3 — Bindando assets por hash e escrevendo o manifest.yml"
# Expansão segura de array possivelmente vazio (compatível com bash 3.2 + set -u).
python3 "$SCAN" "${SCAN_COMMON[@]}" ${EXTRA_ARGS[@]+"${EXTRA_ARGS[@]}"}

# ---------------------------------------------------------------------------
# FASE EXTRA — TEMAS: gera quickmenu/topbg_dither.png para cada tema DSi, uma
# cópia RGBA do fundo da tela superior com alpha por dithering (Bayer 8x8). Os
# pixels opacos mostram o brick; os buracos deixam o vídeo aparecer atrás. Não
# toca no topbg.png original. Não-fatal: falha aqui não aborta o pipeline.
#   DITHER_TOPBG=0      desliga este passo.
#   DITHER_INTENSITY=N  1..100 = %% de pixels opacos (brick). Padrão 40.
#   DITHER_PATTERN=NOME padrão de dithering (bayer8/…/noise). Padrão bayer8.
#                       Veja: python3 dither_topbg.py --list-patterns
# ---------------------------------------------------------------------------
if [ "${DITHER_TOPBG:-1}" != "0" ] && [ -f "$DITHER" ]; then
    log "FASE EXTRA — Gerando topbg_dither.png dos temas DSi"
    python3 "$DITHER" "$SD" --intensity "${DITHER_INTENSITY:-40}" \
        --pattern "${DITHER_PATTERN:-bayer8}" \
        || err "Geração dos topbg_dither.png falhou (seguindo mesmo assim)."
fi

DEST="$OUT"
log "Concluído"
printf 'Manifesto: %s/manifest.yml\n' "$DEST"
printf 'Assets:    %s/assets/<sha1>/{logo.png,top.tgrv,bottom.tgrv}\n' "$DEST"
