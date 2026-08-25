#!/usr/bin/env python3
"""
dither_topbg.py — gera uma cópia "dithered" (com transparência) do fundo da tela
superior de CADA tema do TWiLightMenu++ (tema DSi).

Objetivo: usar a cópia como camada por cima de um vídeo de gameplay. Os pixels
OPACOS mostram o brick do tema; os pixels TRANSPARENTES (buracos) deixam o vídeo
aparecer atrás — recria a translucidez SEM alpha blending em runtime.

Fluxo:
  <root>/_nds/TWiLightMenu/dsimenu/themes/<TEMA>/quickmenu/topbg.png
    -> <root>/_nds/TWiLightMenu/dsimenu/themes/<TEMA>/quickmenu/topbg_dither.png

A cópia tem as MESMAS dimensões e as MESMAS cores RGB do original; só o canal
ALPHA é reescrito por um PADRÃO DE DITHERING (--pattern, ver PATTERNS):
  - pixel "mantido": RGB original, alpha = 255 (opaco);
  - pixel "buraco":  alpha = 0   (totalmente transparente).
Saída em PNG RGBA 8-bit por canal (compatível com lodepng).

Intensidade (--intensity N, 1..100, default 40):
  N = porcentagem de pixels que ficam OPACOS (visibilidade do brick).
    100 -> todos opacos (sem buracos; = original, só que RGBA)
     40 -> ~40% opacos, ~60% transparentes (mais vídeo visível)
      1 -> quase tudo transparente
  Regra por pixel (igual p/ todos os padrões): opaco se tile[y%rows][x%cols] < N.

Padrão (--pattern NOME, default bayer8): bayer8/bayer4/bayer2, cluster (halftone),
  hlines, vlines, checker, noise. Use --list-patterns para a lista com descrições.

Requer ffmpeg no PATH (decode/encode do PNG). NÃO altera o topbg.png original.

Uso:
    dither_topbg.py [<root>] --intensity <1..100> --pattern <nome>
    dither_topbg.py --list-patterns
Exemplos:
    dither_topbg.py ./.preview/sdcard --intensity 40 --pattern cluster
    dither_topbg.py /Volumes/DSI --intensity 55 --pattern noise
"""
import argparse
import os
import struct
import subprocess
import sys

THEMES_SUBPATH = os.path.join("_nds", "TWiLightMenu", "dsimenu", "themes")
PNG_SIG = b"\x89PNG\r\n\x1a\n"


def err(msg):
    sys.stderr.write("ERROR: " + msg + "\n")


def log(msg):
    print("==> " + msg)


# ---------------------------------------------------------------------------
# PADRÕES DE DITHERING (ordered dithering).
#
# Cada padrão é uma matriz "tile" de LIMIARES normalizados 0..99. A máscara de
# alpha se decide, por pixel, com a MESMA regra para todos:
#     opaco (brick)  se  tile[y % rows][x % cols] < intensity
#     buraco (vídeo)  caso contrário
# Como o tile se repete pela imagem, o padrão define a "textura" dos buracos.
# Todos são determinísticos (idempotentes) — inclusive o 'noise', que usa seed fixa.
# ---------------------------------------------------------------------------
def _normalize(mat, levels):
    """Escala uma matriz 0..levels-1 para limiares 0..99."""
    return [[(v * 100) // levels for v in row] for row in mat]


def _bayer(n):
    """Matriz de Bayer n×n (n potência de 2), gerada recursivamente."""
    if n == 1:
        return [[0]]
    half = _bayer(n // 2)
    m = [[0] * n for _ in range(n)]
    for y in range(n // 2):
        for x in range(n // 2):
            base = 4 * half[y][x]
            m[y][x] = base + 0
            m[y][x + n // 2] = base + 2
            m[y + n // 2][x] = base + 3
            m[y + n // 2][x + n // 2] = base + 1
    return m


# Clustered-dot 8×8 clássico (0..63): buracos crescem em "pontos" -> visual halftone.
_CLUSTER8 = [
    [24, 10, 12, 26, 35, 47, 49, 37],
    [8,   0,  2, 14, 45, 59, 61, 51],
    [22,  6,  4, 16, 43, 57, 63, 53],
    [30, 20, 18, 28, 33, 41, 55, 39],
    [34, 46, 48, 38, 25, 11, 13, 27],
    [44, 58, 60, 50,  9,  1,  3, 15],
    [42, 56, 62, 52, 23,  7,  5, 17],
    [32, 40, 54, 36, 31, 21, 19, 29],
]


def _lines(period, horizontal):
    """Linhas: limiar depende só de y (horizontais) ou só de x (verticais)."""
    steps = [(i * 100) // period for i in range(period)]
    if horizontal:                       # tile rows×1 -> constante ao longo da linha
        return [[s] for s in steps]
    return [steps]                       # tile 1×cols -> constante ao longo da coluna


def _noise(n=64, seed=1337):
    """Ruído ordenado determinístico: tile n×n com limiares pseudo-aleatórios."""
    import random
    rnd = random.Random(seed)
    return [[rnd.randint(0, 99) for _ in range(n)] for _ in range(n)]


# Registry: nome -> (descrição, tile de limiares 0..99). Ordem = ordem no helper.
PATTERNS = {
    "bayer8":  ("Bayer 8×8 — ordenado fino, buracos bem espalhados (padrão).",
                _normalize(_bayer(8), 64)),
    "bayer4":  ("Bayer 4×4 — ordenado médio, textura um pouco mais grossa.",
                _normalize(_bayer(4), 16)),
    "bayer2":  ("Bayer 2×2 — ordenado bem grosso (4 níveis).",
                _normalize(_bayer(2), 4)),
    "cluster": ("Clustered-dot 8×8 — visual halftone (pontos de meio-tom).",
                _normalize(_CLUSTER8, 64)),
    "hlines":  ("Linhas horizontais (scanlines).",
                _lines(8, horizontal=True)),
    "vlines":  ("Linhas verticais.",
                _lines(8, horizontal=False)),
    "checker": ("Xadrez — alternância diagonal (efeito ~50%).",
                [[0, 50], [50, 0]]),
    "noise":   ("Ruído determinístico — buracos aleatórios (seed fixa).",
                _noise()),
}
DEFAULT_PATTERN = "bayer8"


def parse_png_size(path):
    """Lê width/height do IHDR sem decodificar a imagem inteira."""
    with open(path, "rb") as fh:
        head = fh.read(24)
    if len(head) < 24 or head[:8] != PNG_SIG or head[12:16] != b"IHDR":
        raise ValueError("não é um PNG válido")
    width, height = struct.unpack(">II", head[16:24])
    if width == 0 or height == 0:
        raise ValueError("dimensões inválidas no IHDR")
    return width, height


def build_alpha_mask(width, height, intensity, tile):
    """
    Constrói o plano de alpha (bytes, len = width*height): 255 (opaco) ou 0 (buraco).
    Opaco se tile[y % rows][x % cols] < intensity. `tile` é uma matriz de limiares 0..99.
    """
    rows = len(tile)
    cols = len(tile[0])
    # Pré-calcula uma linha (largura cheia) de opacidade para cada fase vertical.
    row_patterns = [
        bytes(255 if tile[phase][x % cols] < intensity else 0 for x in range(width))
        for phase in range(rows)
    ]
    return b"".join(row_patterns[y % rows] for y in range(height))


def decode_rgba(src, width, height):
    """Decodifica o PNG para RGBA cru (width*height*4 bytes) via ffmpeg."""
    cmd = ["ffmpeg", "-v", "error", "-i", src,
           "-f", "rawvideo", "-pix_fmt", "rgba", "-"]
    proc = subprocess.run(cmd, capture_output=True)
    if proc.returncode != 0:
        raise RuntimeError("ffmpeg decode falhou: "
                           + proc.stderr.decode(errors="replace").strip())
    expected = width * height * 4
    if len(proc.stdout) != expected:
        raise RuntimeError(f"tamanho RGBA inesperado: {len(proc.stdout)} != {expected}")
    return proc.stdout


def encode_rgba_png(rgba, width, height, dst):
    """Codifica RGBA cru de volta para PNG RGBA 8-bit via ffmpeg (overwrite)."""
    cmd = ["ffmpeg", "-v", "error", "-y",
           "-f", "rawvideo", "-pix_fmt", "rgba", "-s", f"{width}x{height}",
           "-i", "-", "-frames:v", "1", "-pix_fmt", "rgba", dst]
    proc = subprocess.run(cmd, input=rgba, capture_output=True)
    if proc.returncode != 0:
        raise RuntimeError("ffmpeg encode falhou: "
                           + proc.stderr.decode(errors="replace").strip())


def dither_one(topbg, intensity, tile):
    """Gera topbg_dither.png ao lado de topbg.png. Retorna (width, height)."""
    width, height = parse_png_size(topbg)
    rgba = bytearray(decode_rgba(topbg, width, height))
    # Reescreve SÓ o canal alpha (bytes de índice 3,7,11,...).
    rgba[3::4] = build_alpha_mask(width, height, intensity, tile)
    dst = os.path.join(os.path.dirname(topbg), "topbg_dither.png")
    encode_rgba_png(bytes(rgba), width, height, dst)
    return width, height


def find_root(explicit):
    """Resolve <root>: usa o argumento se dado; senão tenta caminhos comuns."""
    candidates = [explicit] if explicit else [
        "sd:/", "./.preview/sdcard", "./.preview", "./sdcard",
        "/Volumes/DSI", "/Volumes/SD", "/Volumes/NDS",
    ]
    for c in candidates:
        if c and os.path.isdir(os.path.join(c, THEMES_SUBPATH)):
            return c
    return None


def print_patterns(prog="dither_topbg.py", examples=None):
    """Helper de comandos: lista os padrões de dithering disponíveis.

    `prog`/`examples` deixam cada ferramenta que reutilize esta lista mostrar
    exemplos com o seu próprio nome.
    """
    print("Padrões de dithering disponíveis (--pattern <nome>):\n")
    for name, (desc, _tile) in PATTERNS.items():
        star = "  (padrão)" if name == DEFAULT_PATTERN else ""
        print(f"  {name:<8} {desc}{star}")
    print("\nExemplos:")
    if examples is None:
        examples = [
            f"{prog} /Volumes/DSI --intensity 40 --pattern cluster",
            f"{prog} /Volumes/DSI --pattern noise",
        ]
    for ex in examples:
        print(f"  {ex}")


def main(argv):
    ap = argparse.ArgumentParser(
        description="Gera topbg_dither.png (RGBA, alpha por dithering) para cada tema.")
    ap.add_argument("root", nargs="?", default=None,
                    help="raiz do SD (contém _nds/...). Se omitido, tenta caminhos comuns.")
    ap.add_argument("--intensity", type=int, default=40,
                    help="1..100 = %% de pixels OPACOS (brick). Default 40.")
    ap.add_argument("--pattern", default=DEFAULT_PATTERN, choices=list(PATTERNS),
                    help=f"padrão de dithering (default {DEFAULT_PATTERN}). "
                         "Veja --list-patterns.")
    ap.add_argument("--list-patterns", action="store_true",
                    help="lista os padrões de dithering disponíveis e sai.")
    args = ap.parse_args(argv)

    if args.list_patterns:
        print_patterns()
        return 0
    if not (1 <= args.intensity <= 100):
        err("--intensity deve estar entre 1 e 100.")
        return 2
    if subprocess.run(["bash", "-c", "command -v ffmpeg"],
                      stdout=subprocess.DEVNULL).returncode != 0:
        err("ffmpeg não encontrado no PATH. Instale (brew install ffmpeg).")
        return 1

    tile = PATTERNS[args.pattern][1]
    root = find_root(args.root)
    if not root:
        err("pasta de temas não encontrada "
            f"(<root>/{THEMES_SUBPATH}). Passe <root> explicitamente.")
        return 1
    themes_dir = os.path.join(root, THEMES_SUBPATH)
    log(f"Temas em: {themes_dir}")
    log(f"Intensidade: {args.intensity}% opacos (brick), padrão '{args.pattern}'.")

    processed, skipped, failed = [], [], []
    for name in sorted(os.listdir(themes_dir)):
        theme_dir = os.path.join(themes_dir, name)
        if not os.path.isdir(theme_dir):
            continue
        topbg = os.path.join(theme_dir, "quickmenu", "topbg.png")
        if not os.path.isfile(topbg):
            log(f"[skip] {name}: sem quickmenu/topbg.png")
            skipped.append(name)
            continue
        try:
            w, h = dither_one(topbg, args.intensity, tile)
            log(f"[ok]   {name}: topbg_dither.png ({w}x{h} RGBA)")
            processed.append(name)
        except Exception as e:                       # noqa: BLE001 (log e continua)
            err(f"{name}: falhou — {e}")
            failed.append(name)

    print()
    log("Resumo:")
    print(f"    processados: {len(processed)}")
    print(f"    pulados (sem topbg.png): {len(skipped)}")
    print(f"    falhas: {len(failed)}")
    if failed:
        print(f"    -> temas com falha: {', '.join(failed)}")
    # Sucesso a menos que TODOS os candidatos com topbg tenham falhado.
    return 0 if not failed or processed else 1


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
