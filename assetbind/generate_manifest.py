#!/usr/bin/env python3
"""
generate_manifest.py — gera o manifesto .yml e ORGANIZA os assets em pastas por jogo.

Entrada:
  --roms  <dir>   pasta com as ROMs (ex.: .nds)
  --media <dir>   pasta onde o Skyscraper/ScreenScraper baixou os assets, nomeados
                  "<base>-logo.png" e "<base>-video.<ext>" (base = nome da ROM sem extensão)
  --out   <dir>   raiz de saída: recebe manifest.yml + assets/<game_id>/{logo,video}

Estrutura de saída (pasta de assets + subpasta por jogo):
  <out>/
    manifest.yml
    assets/
      <game_id>/            # game_id = sha1 da ROM (identidade estável de conteúdo)
        logo.png
        video.<ext>

IDENTIDADE POR HASH (não por nome): a ROM é casada ao asset pelo conteúdo. Arquivos
duplicados (mesmo hash, nomes diferentes) viram UMA entrada e compartilham os assets.
"""
import argparse
import glob
import os
import shutil
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from rom_hash import hash_file          # noqa: E402
from yaml_io import dump_manifest        # noqa: E402


def find_roms(roms_dir, exts):
    out = []
    for dirpath, _, files in os.walk(roms_dir):
        for fn in files:
            if os.path.splitext(fn)[1].lower() in exts:
                out.append(os.path.join(dirpath, fn))
    return sorted(out)


def find_source_assets(media_dir, base, logo_suffix, video_suffix):
    """Acha logo e vídeo do Skyscraper para o 'base' da ROM. Retorna (logo_path, video_path)."""
    logo = os.path.join(media_dir, base + logo_suffix)
    logo = logo if os.path.isfile(logo) else None
    vids = sorted(glob.glob(glob.escape(os.path.join(media_dir, base + video_suffix)) + ".*"))
    video = vids[0] if vids else None
    return logo, video


def main(argv):
    ap = argparse.ArgumentParser(description="Gera manifest.yml + organiza assets por jogo.")
    ap.add_argument("--roms", required=True, help="pasta das ROMs")
    ap.add_argument("--media", required=True, help="pasta dos assets do Skyscraper")
    ap.add_argument("--out", required=True, help="raiz de saída (manifest.yml + assets/)")
    ap.add_argument("--rom-ext", default=".nds", help="extensões de ROM (csv). Padrão .nds")
    ap.add_argument("--logo-suffix", default="-logo.png")
    ap.add_argument("--video-suffix", default="-video")
    ap.add_argument("--allow-name-match", action="store_true",
                    help="grava allow_name_match: true no manifesto (fallback por nome no runtime)")
    args = ap.parse_args(argv)

    exts = {e if e.startswith(".") else "." + e for e in args.rom_ext.lower().split(",")}
    assets_root = os.path.join(args.out, "assets")
    os.makedirs(assets_root, exist_ok=True)

    roms = find_roms(args.roms, exts)
    if not roms:
        print(f"!! nenhuma ROM {sorted(exts)} em {args.roms}", file=sys.stderr)
        return 1

    # Agrupa por sha1 (conteúdo). Duplicatas (mesmo hash) => uma entrada só.
    by_sha1 = {}
    for path in roms:
        h = hash_file(path)
        by_sha1.setdefault(h["sha1"], {"hash": h, "files": []})["files"].append(path)

    games = []
    n_logo = n_video = n_noasset = 0
    for sha1, info in sorted(by_sha1.items()):
        h = info["hash"]
        files = info["files"]
        if len(files) > 1:
            print(f">> {len(files)} arquivos com o mesmo conteúdo (sha1 {sha1[:8]}…) "
                  f"-> 1 entrada: {[os.path.basename(f) for f in files]}", file=sys.stderr)

        # procura assets por qualquer um dos nomes do grupo (bases diferentes p/ mesmo jogo)
        src_logo = src_video = None
        for f in files:
            base = os.path.splitext(os.path.basename(f))[0]
            l, v = find_source_assets(args.media, base, args.logo_suffix, args.video_suffix)
            src_logo = src_logo or l
            src_video = src_video or v

        game_id = sha1  # identidade estável de conteúdo (pode ser o id do ScreenScraper)
        game_dir = os.path.join(assets_root, game_id)
        rel_logo = rel_video = None
        if src_logo or src_video:
            os.makedirs(game_dir, exist_ok=True)
        if src_logo:
            dst = os.path.join(game_dir, "logo.png")
            shutil.copy2(src_logo, dst)
            rel_logo = os.path.relpath(dst, args.out)
            n_logo += 1
        if src_video:
            ext = os.path.splitext(src_video)[1].lower()
            dst = os.path.join(game_dir, "video" + ext)
            shutil.copy2(src_video, dst)
            rel_video = os.path.relpath(dst, args.out)
            n_video += 1
        if not src_logo and not src_video:
            n_noasset += 1
            print(f"   aviso: sem assets para {os.path.basename(files[0])} (sha1 {sha1[:8]}…)",
                  file=sys.stderr)

        games.append({
            "game_id": game_id,
            "identity": h,               # sha1, md5, crc32, size
            "rom_name": os.path.basename(files[0]),  # informativo/fallback apenas
            "assets": {"logo": rel_logo, "video": rel_video},
        })

    manifest = {"version": 1, "games": games}
    if args.allow_name_match:
        manifest["allow_name_match"] = True

    out_path = os.path.join(args.out, "manifest.yml")
    with open(out_path, "w", encoding="utf-8") as fh:
        fh.write(dump_manifest(manifest))

    print(f">> {out_path}: {len(games)} jogos, {n_logo} logos, {n_video} vídeos, "
          f"{n_noasset} sem assets.", file=sys.stderr)
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
