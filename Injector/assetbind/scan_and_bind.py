#!/usr/bin/env python3
"""
scan_and_bind.py — varre um SD card, baixa a mídia de cada JOGO e binda por HASH.

Fluxo:
  1) Varre o SD (recursivo) por ROMs .nds.
  2) Filtra apps de sistema/homebrew via blocklist (por sha1 e/ou nome).
  3) Agrupa por sha1 (identidade de conteúdo) — duplicatas (ex.: "... - cópia.nds")
     colapsam em UMA entrada.
  4) Para cada jogo sem assets ainda, chama o fetch_ds_media.sh (Skyscraper +
     ScreenScraper) para baixar logo + vídeo. Idempotente: pula quem já tem.
  5) ORGANIZA os assets em <out>/assets/<sha1>/{logo.png, video.<ext>} — espelhando
     a estrutura por-hash dentro do SD (padrão: <out> = raiz do SD).
  6) Escreve <out>/manifest.yml (caminhos relativos à raiz do manifesto).

IDENTIDADE POR HASH (não por nome): mesmo que o arquivo esteja duplicado/renomeado,
ele resolve para o mesmo jogo e os mesmos assets. Ver rom_binder.py para o runtime.

Credenciais do ScreenScraper: lidas pelo próprio fetch_ds_media.sh (env SS_USER/SS_PASS
ou o cache .ss_credentials.json). Este script apenas repassa o ambiente.
"""
import argparse
import glob
import os
import shutil
import subprocess
import sys
import tempfile
import unicodedata

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from rom_hash import hash_file                       # noqa: E402
from yaml_io import dump_manifest, dump_assets_index  # noqa: E402

HERE = os.path.dirname(os.path.abspath(__file__))
DEFAULT_FETCH = os.path.normpath(os.path.join(HERE, "..", "fetch_ds_media.sh"))
DEFAULT_BLOCKLIST = os.path.join(HERE, "system_blocklist.txt")
DEFAULT_CACHE = os.path.normpath(os.path.join(HERE, "..", "cache"))

# Nomes canônicos dos assets dentro de assets/<sha1>/.
ASSET_NAMES = {"logo": "logo.png", "top": "top.tgrv", "bottom": "bottom.tgrv"}


# --------------------------- blocklist ---------------------------

def load_blocklist(path):
    """Lê a blocklist. Retorna (set de sha1 minúsculos, lista de substrings de nome)."""
    sha1s, names = set(), []
    if not path or not os.path.isfile(path):
        return sha1s, names
    with open(path, "r", encoding="utf-8") as f:
        for raw in f:
            line = raw.split("#", 1)[0].strip()
            if not line:
                continue
            low = line.lower()
            if low.startswith("sha1:"):
                sha1s.add(low[5:].strip())
            elif low.startswith("name:"):
                names.append(low[5:].strip())
            else:
                names.append(low)
    return sha1s, names


def name_blocked(filename, names):
    low = os.path.basename(filename).lower()
    return any(sub in low for sub in names)


# --------------------------- scan ---------------------------

def find_roms(sd_dir, exts):
    out = []
    for dirpath, _, files in os.walk(sd_dir):
        for fn in files:
            if os.path.splitext(fn)[1].lower() in exts:
                out.append(os.path.join(dirpath, fn))
    return sorted(out)


# --------------------------- download (fetch_ds_media.sh) ---------------------------

def fetch_media(fetch_script, rom_path, dest_dir):
    """
    Chama o fetch_ds_media.sh para baixar logo+vídeo e já dividir o vídeo em TGRV.
    O .sh grava '<base>-logo.png', '<base>-top.tgrv' e '<base>-bottom.tgrv'.
    Retorna dict {logo, top, bottom} (paths ou None). Repassa o ambiente
    (SS_USER/SS_PASS, SKYSCRAPER_BIN, TGRV_FPS, ...) intacto.
    """
    os.makedirs(dest_dir, exist_ok=True)
    proc = subprocess.run(
        ["bash", fetch_script, rom_path, dest_dir],
        stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True,
    )
    base = os.path.splitext(os.path.basename(rom_path))[0]

    def pick(name):
        p = os.path.join(dest_dir, base + name)
        return p if os.path.isfile(p) else None

    # rc != 0 do .sh pode significar "sem nenhum asset" (exit 4) — não é fatal aqui.
    if proc.returncode not in (0, 4):
        sys.stderr.write(f"   [fetch rc={proc.returncode}] {os.path.basename(rom_path)}\n")
        sys.stderr.write("   " + proc.stdout.strip().replace("\n", "\n   ") + "\n")
    return {
        "logo": pick("-logo.png"),
        "top": pick("-top.tgrv"),
        "bottom": pick("-bottom.tgrv"),
    }


def existing_assets(game_dir):
    """Retorna dict {logo, top, bottom} já presentes em assets/<sha1>/ (ou None)."""
    def pick(name):
        p = os.path.join(game_dir, name)
        return p if os.path.isfile(p) else None
    return {"logo": pick("logo.png"), "top": pick("top.tgrv"), "bottom": pick("bottom.tgrv")}


# --------------------------- main ---------------------------

def main(argv):
    ap = argparse.ArgumentParser(description="Varre o SD, baixa mídia e binda por hash.")
    ap.add_argument("--sd", required=True, help="raiz do SD card a escanear")
    ap.add_argument("--out", default=None,
                    help="destino FINAL no SD (manifest.yml + assets/). Padrão: a própria raiz do SD.")
    ap.add_argument("--cache", default=DEFAULT_CACHE,
                    help="pasta de trabalho onde tudo é baixado/processado antes do deploy no SD.")
    ap.add_argument("--fetch-script", default=DEFAULT_FETCH, help="caminho do fetch_ds_media.sh")
    ap.add_argument("--blocklist", default=DEFAULT_BLOCKLIST, help="arquivo de blocklist de sistema")
    ap.add_argument("--rom-ext", default=".nds", help="extensões de ROM (csv). Padrão .nds")
    ap.add_argument("--list-only", action="store_true",
                    help="apenas escaneia e LISTA os jogos encontrados (sem baixar nem escrever nada)")
    ap.add_argument("--no-download", action="store_true",
                    help="não baixa nada; só organiza assets já existentes e (re)escreve o manifesto")
    ap.add_argument("--force", action="store_true",
                    help="rebaixa a mídia mesmo se assets/<sha1>/ já existir")
    ap.add_argument("--allow-name-match", action="store_true",
                    help="grava allow_name_match: true no manifesto (fallback por nome no runtime)")
    args = ap.parse_args(argv)

    out = os.path.abspath(args.out or args.sd)        # destino FINAL (SD)
    cache = os.path.abspath(args.cache)               # área de trabalho local
    exts = {e if e.startswith(".") else "." + e for e in args.rom_ext.lower().split(",")}
    out_assets = os.path.join(out, "assets")
    cache_assets = os.path.join(cache, "assets")

    # --list-only não escreve nada; os demais modos preparam o cache de trabalho.
    if not args.list_only:
        os.makedirs(cache_assets, exist_ok=True)

    if not args.list_only and not args.no_download and not os.path.isfile(args.fetch_script):
        sys.stderr.write(f"!! fetch script não encontrado: {args.fetch_script}\n")
        sys.stderr.write("   Use --no-download para apenas (re)gerar o manifesto.\n")
        return 1

    sha1_block, name_block = load_blocklist(args.blocklist)
    print(f">> blocklist: {len(sha1_block)} hashes, {len(name_block)} nomes", file=sys.stderr)

    roms = find_roms(args.sd, exts)
    if not roms:
        sys.stderr.write(f"!! nenhuma ROM {sorted(exts)} em {args.sd}\n")
        return 1

    # Agrupa por sha1; pula os bloqueados (por nome antes de hashear, por hash depois).
    by_sha1 = {}
    n_blocked = 0
    for path in roms:
        if name_blocked(path, name_block):
            n_blocked += 1
            print(f"   ignorado (nome de sistema): {os.path.basename(path)}", file=sys.stderr)
            continue
        h = hash_file(path)
        if h["sha1"].lower() in sha1_block:
            n_blocked += 1
            print(f"   ignorado (hash de sistema): {os.path.basename(path)}", file=sys.stderr)
            continue
        by_sha1.setdefault(h["sha1"], {"hash": h, "files": []})["files"].append(path)

    print(f">> {len(roms)} arquivos, {n_blocked} bloqueados, "
          f"{len(by_sha1)} jogos únicos", file=sys.stderr)

    # --list-only: apenas imprime os jogos encontrados (nome + sha1 + nº de arquivos) e sai.
    if args.list_only:
        for sha1, info in sorted(by_sha1.items()):
            files = info["files"]
            dup = f"  (x{len(files)} duplicatas)" if len(files) > 1 else ""
            print(f"{sha1[:12]}…  {os.path.basename(files[0])}{dup}")
        print(f">> {len(by_sha1)} jogos listados (nada baixado/escrito).", file=sys.stderr)
        return 0

    # Assets já prontos de um jogo: procura primeiro no destino final (SD), senão no cache.
    def game_assets(sha1):
        for base in (out_assets, cache_assets):
            a = existing_assets(os.path.join(base, sha1))
            if any(a.values()):
                return a
        return {"logo": None, "top": None, "bottom": None}

    games = []
    roms_index = {}   # nome-base-da-ROM (sem extensão) -> game_id (para assets_index.yml)
    processed = []    # sha1 processados neste run (assets ficam no cache p/ deploy)
    n_logo = n_video = n_noasset = n_downloaded = 0
    total = len(by_sha1)

    # ---- PROCESSAMENTO (tudo no cache local; o SD só foi lido na varredura) ----
    for i, (sha1, info) in enumerate(sorted(by_sha1.items()), 1):
        pct = i * 100 // total
        h = info["hash"]
        files = info["files"]
        primary = files[0]  # representante do grupo de duplicatas
        dup = f"  (x{len(files)} dup)" if len(files) > 1 else ""
        print(f">> [{i}/{total}] {pct:3d}% | {os.path.basename(primary)}{dup}", file=sys.stderr)

        # Índice runtime por NOME: cada arquivo do grupo (inclui duplicatas) -> este game_id.
        # Normaliza o nome para NFC: o macOS lista arquivos em NFD (ó = o + acento),
        # mas o DS casa a string em forma canônica NFC (ó precomposto), como o índice de
        # referência. Sem isso, nomes acentuados não casariam no DS.
        for f in files:
            base = unicodedata.normalize("NFC", os.path.splitext(os.path.basename(f))[0])
            if base in roms_index and roms_index[base] != sha1:
                print(f"   aviso: nome '{base}' aponta para 2 jogos diferentes; "
                      f"o índice por nome fica ambíguo.", file=sys.stderr)
            roms_index[base] = sha1

        assets = game_assets(sha1)  # {logo, top, bottom} (final ou cache)

        # Baixa/processa (no CACHE) se ainda não tem nenhum asset (ou --force).
        need = args.force or not any(assets.values())
        if need and not args.no_download:
            cache_game_dir = os.path.join(cache_assets, sha1)
            # Baixa+split+downscale num tmp dentro do cache; move só os finais canônicos.
            with tempfile.TemporaryDirectory(prefix="scanbind_", dir=cache) as tmp:
                src = fetch_media(args.fetch_script, primary, tmp)  # {logo, top, bottom}
                if any(src.values()):
                    if os.path.isdir(cache_game_dir):
                        shutil.rmtree(cache_game_dir)
                    os.makedirs(cache_game_dir, exist_ok=True)
                    for k, name in ASSET_NAMES.items():
                        if src.get(k):
                            shutil.move(src[k], os.path.join(cache_game_dir, name))
            assets = existing_assets(cache_game_dir)
            if any(assets.values()):
                n_downloaded += 1
                processed.append(sha1)

        # Caminhos no manifesto: relativos ao destino FINAL (onde ficarão após o deploy).
        rel = {k: (f"assets/{sha1}/{ASSET_NAMES[k]}" if assets.get(k) else None)
               for k in ("logo", "top", "bottom")}
        if assets["logo"]:
            n_logo += 1
        if assets["top"] or assets["bottom"]:
            n_video += 1
        if not any(assets.values()):
            n_noasset += 1
            print(f"   aviso: sem assets para {os.path.basename(primary)} (sha1 {sha1[:8]}…)",
                  file=sys.stderr)

        games.append({
            "game_id": sha1,
            "identity": h,
            "rom_name": os.path.basename(primary),  # informativo/fallback
            "assets": {
                "logo": rel["logo"],
                "video": None,                 # mp4 empilhado não vai para o SD
                "video_top": rel["top"],
                "video_bottom": rel["bottom"],
            },
        })

    # ---- GERAÇÃO DOS .yml (no cache) ----
    manifest = {"version": 1, "games": games}
    if args.allow_name_match:
        manifest["allow_name_match"] = True

    os.makedirs(cache, exist_ok=True)
    cache_manifest = os.path.join(cache, "manifest.yml")
    cache_index = os.path.join(cache, "assets_index.yml")
    with open(cache_manifest, "w", encoding="utf-8") as fh:
        fh.write(dump_manifest(manifest))
    with open(cache_index, "w", encoding="utf-8") as fh:
        fh.write(dump_assets_index(roms_index))

    print(f">> processados no cache: {len(games)} jogos, {n_logo} logos, "
          f"{n_video} vídeos (tgrv), {n_noasset} sem assets, {n_downloaded} baixados agora.",
          file=sys.stderr)

    # ---- DEPLOY: monta as pastas no SD e move o conteúdo do cache p/ o SD ----
    print(">> Deploy: montando pastas no SD e movendo o conteúdo...", file=sys.stderr)
    os.makedirs(out_assets, exist_ok=True)
    ndep = len(processed)
    for j, sha1 in enumerate(processed, 1):
        dpct = j * 100 // max(1, ndep)
        src_dir = os.path.join(cache_assets, sha1)
        dst_dir = os.path.join(out_assets, sha1)
        if os.path.isdir(dst_dir):
            shutil.rmtree(dst_dir)
        shutil.move(src_dir, dst_dir)
        print(f">> [deploy {j}/{ndep}] {dpct:3d}% | {sha1[:12]}…", file=sys.stderr)
    # Os .yml por último (depois que os assets já estão no lugar).
    shutil.move(cache_manifest, os.path.join(out, "manifest.yml"))
    shutil.move(cache_index, os.path.join(out, "assets_index.yml"))

    print(f">> Deploy concluído em {out}: manifest.yml + assets_index.yml + "
          f"{ndep} jogo(s) movidos do cache.", file=sys.stderr)
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
