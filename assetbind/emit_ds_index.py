#!/usr/bin/env python3
"""
emit_ds_index.py — gera o ÍNDICE LEVE que o DS lê (o DS não hasheia em runtime).

Para cada ROM do cartão, resolve o game_id via HASH (usando o manifest/binder) e escreve
um mapa simples "nome-base-no-SD: game_id". O DS casa a ROM em foco por esse índice e então
carrega os assets de assets/<game_id>/ (logo.png, top.tgrv, bottom.tgrv).

Duplicatas/renomes: cada arquivo do SD (com seu nome) aponta para o mesmo game_id -> mesma arte.

Uso:
  python3 emit_ds_index.py --roms <dir_das_roms> --manifest <manifest.yml> --out <assets_index.yml>
"""
import argparse
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from rom_binder import load_manifest  # noqa: E402


def find_roms(roms_dir, exts):
    out = []
    for dp, _, files in os.walk(roms_dir):
        for fn in files:
            if os.path.splitext(fn)[1].lower() in exts:
                out.append(os.path.join(dp, fn))
    return sorted(out)


def yq(s):
    return '"' + str(s).replace("\\", "\\\\").replace('"', '\\"') + '"'


def main(argv):
    ap = argparse.ArgumentParser()
    ap.add_argument("--roms", required=True)
    ap.add_argument("--manifest", required=True)
    ap.add_argument("--out", required=True)
    ap.add_argument("--rom-ext", default=".nds")
    args = ap.parse_args(argv)

    exts = {e if e.startswith(".") else "." + e for e in args.rom_ext.lower().split(",")}
    binder = load_manifest(args.manifest)

    lines = ["# GERADO pelo host: nome-base-no-SD (sem extensão) -> game_id.",
             "# O DS casa a ROM em foco por aqui e carrega assets/<game_id>/.",
             "version: 1", "roms:"]
    n_hit = n_miss = 0
    for path in find_roms(args.roms, exts):
        base = os.path.splitext(os.path.basename(path))[0]
        res = binder.bind(path)
        if res is None:
            n_miss += 1
            continue
        lines.append(f"  {yq(base)}: {yq(res.game_id)}")
        n_hit += 1

    with open(args.out, "w", encoding="utf-8") as f:
        f.write("\n".join(lines) + "\n")
    print(f">> {args.out}: {n_hit} ROMs indexadas, {n_miss} sem entrada.", file=sys.stderr)
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
