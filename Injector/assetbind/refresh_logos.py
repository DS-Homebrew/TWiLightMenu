#!/usr/bin/env python3
"""
refresh_logos.py — REBAIXA só o LOGO de cada jogo do SD e SUBSTITUI o logo.png
antigo em assets/<sha1>/, mantendo vídeos (.tgrv) e manifest.yml intactos.

Para que serve: depois de mudar o processamento de logo, atualizar os logos já
gravados no SD sem re-baixar os vídeos nem reescrever o manifesto — uma passada
barata e cirúrgica.

Como funciona (reaproveita o scan_and_bind.py):
  1) varre o SD por ROMs (.nds), pula a blocklist de sistema e agrupa por sha1
     (mesma identidade de conteúdo usada no bind — duplicatas colapsam);
  2) para cada jogo, chama o fetch_ds_media.sh com LOGO_ONLY=1 (baixa só o logo,
     sem vídeo) num diretório temporário;
  3) copia o logo por cima de <out>/assets/<sha1>/logo.png (cria a pasta se não
     existir). NADA além de logo.png é tocado.

Credenciais do ScreenScraper: lidas pelo próprio fetch_ds_media.sh (env
SS_USER/SS_PASS, cache .ss_credentials.json ou prompt).
"""
import argparse
import os
import shutil
import sys
import tempfile

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
# Reutiliza a varredura/hash/fetch já testados do pipeline.
from scan_and_bind import (                       # noqa: E402
    find_roms, load_blocklist, name_blocked, fetch_media,
    DEFAULT_FETCH, DEFAULT_BLOCKLIST,
)
from rom_hash import hash_file                     # noqa: E402

DEFAULT_OUT_SUB = os.path.join("_nds", "TWiLightMenu", "dsimenu")


def main(argv):
    ap = argparse.ArgumentParser(
        description="Rebaixa só o logo de cada jogo do SD e substitui o antigo.")
    ap.add_argument("--sd", required=True, help="raiz do SD card a escanear")
    ap.add_argument("--out", default=None,
                    help="onde estão os assets/ (padrão: <SD>/_nds/TWiLightMenu/dsimenu)")
    ap.add_argument("--fetch-script", default=DEFAULT_FETCH,
                    help="caminho do fetch_ds_media.sh")
    ap.add_argument("--blocklist", default=DEFAULT_BLOCKLIST,
                    help="blocklist de apps de sistema")
    ap.add_argument("--rom-ext", default=".nds", help="extensões de ROM (csv). Padrão .nds")
    ap.add_argument("--only-existing", action="store_true",
                    help="só atualiza jogos que JÁ têm assets/<sha1>/ (não cria novos).")
    ap.add_argument("--dry-run", action="store_true",
                    help="lista o que faria, sem baixar nem escrever.")
    args = ap.parse_args(argv)

    if not os.path.isdir(args.sd):
        sys.stderr.write(f"!! SD não encontrado: {args.sd}\n")
        return 1
    out = args.out or os.path.join(args.sd, DEFAULT_OUT_SUB)
    out_assets = os.path.join(out, "assets")
    if not args.dry_run and not os.path.isfile(args.fetch_script):
        sys.stderr.write(f"!! fetch script não encontrado: {args.fetch_script}\n")
        return 1

    exts = {e if e.startswith(".") else "." + e
            for e in (x.strip().lower() for x in args.rom_ext.split(",")) if e}
    sha1_block, name_block = load_blocklist(args.blocklist)

    roms = find_roms(args.sd, exts)
    if not roms:
        sys.stderr.write(f"!! nenhuma ROM {sorted(exts)} em {args.sd}\n")
        return 1

    # Agrupa por sha1 (igual ao scan_and_bind): pula bloqueados por nome e por hash.
    by_sha1 = {}
    for path in roms:
        if name_blocked(path, name_block):
            continue
        h = hash_file(path)
        if h["sha1"].lower() in sha1_block:
            continue
        by_sha1.setdefault(h["sha1"], []).append(path)

    total = len(by_sha1)
    print(f">> {len(roms)} arquivos, {total} jogos únicos. Destino: {out_assets}",
          file=sys.stderr)

    # Força logo-only no fetch (sem vídeo).
    os.environ["LOGO_ONLY"] = "1"

    replaced = skipped = failed = 0
    for i, (sha1, files) in enumerate(sorted(by_sha1.items()), 1):
        primary = files[0]
        game_dir = os.path.join(out_assets, sha1)
        dst_logo = os.path.join(game_dir, "logo.png")
        had = os.path.isfile(dst_logo)
        tag = f"[{i}/{total}] {sha1[:12]}… {os.path.basename(primary)}"

        if args.only_existing and not had:
            print(f">> {tag} — sem assets/ ainda; pulado (--only-existing).",
                  file=sys.stderr)
            skipped += 1
            continue
        if args.dry_run:
            print(f">> {tag} — {'substituiria' if had else 'criaria'} {dst_logo}",
                  file=sys.stderr)
            continue

        try:
            with tempfile.TemporaryDirectory(prefix="refresh_logo_") as tmp:
                src = fetch_media(args.fetch_script, primary, tmp)
                new_logo = src.get("logo")
                if not new_logo or not os.path.isfile(new_logo):
                    print(f">> {tag} — logo não encontrado no ScreenScraper; mantido.",
                          file=sys.stderr)
                    skipped += 1
                    continue
                os.makedirs(game_dir, exist_ok=True)
                shutil.copyfile(new_logo, dst_logo)
            print(f">> {tag} — logo {'substituído' if had else 'criado'}.",
                  file=sys.stderr)
            replaced += 1
        except Exception as e:                       # noqa: BLE001 (loga e segue)
            sys.stderr.write(f"!! {tag} — falhou: {e}\n")
            failed += 1

    if args.dry_run:
        print(f">> dry-run: {total} jogo(s) avaliados (nada alterado).", file=sys.stderr)
    else:
        print(f">> Concluído: {replaced} logo(s) atualizados, {skipped} pulados, "
              f"{failed} falhas.", file=sys.stderr)
    return 0 if failed == 0 or replaced else 1


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
