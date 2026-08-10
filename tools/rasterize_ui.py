#!/usr/bin/env python3
"""
rasterize_ui.py — rasteriza os SVGs de UI em assets/ para bitmaps do pipeline
de assets do TWiLightMenu (grit).

Sem dependências externas: faz o parse dos paths (M/L/H/V/C/Z, abs+rel), achata
as curvas de Bézier, preenche com regra even-odd via supersampling (AA) e escreve
um .bmp 24-bit por asset em gfx/, usando o fundo PURPLE/MAGENTA (#FF00FF) como
cor-chave de transparência — o mesmo esquema dos assets do TWiLightMenu
(grit "-gT FF00FF" => índice 0 = transparente).

Também gera o .grit correspondente (bitmap 4bpp, transparente FF00FF).

Cada bitmap é 64x64 (potência de 2, exigido pelo hardware 3D do DS). O background
(56x53) é centralizado nesse canvas mantendo a escala nativa, para que ao desenhar
os 3 layers no mesmo quad a composição fique correta.

Saída: romsel_dsimenutheme/gfx/<nome>.bmp  +  <nome>.grit
"""
import os
import re
import struct
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
ASSETS = os.path.join(ROOT, "assets")
GFX_DIR = os.path.join(ROOT, "romsel_dsimenutheme", "gfx")

TEX = 64          # tamanho do bitmap (potência de 2)
SS = 4            # supersampling por eixo (4x4 = 16 amostras/pixel)
BEZIER_STEPS = 24 # segmentos por curva cúbica

TRANSPARENT = (255, 0, 255)  # #FF00FF magenta/purple => índice 0 (transparente) no grit

NUM_RE = re.compile(r"[-+]?(?:\d*\.\d+|\d+\.?)(?:[eE][-+]?\d+)?")
CMD_RE = re.compile(r"[MmLlHhVvCcSsQqTtZzAa]")


def parse_svg(path):
    with open(path) as f:
        txt = f.read()
    m = re.search(r'viewBox="([^"]+)"', txt)
    if m:
        vb = [float(x) for x in m.group(1).replace(",", " ").split()]
        vw, vh = vb[2], vb[3]
    else:
        vw = float(re.search(r'width="([\d.]+)"', txt).group(1))
        vh = float(re.search(r'height="([\d.]+)"', txt).group(1))
    d = re.search(r'\bd="([^"]+)"', txt).group(1)
    fill = re.search(r'fill="(#[0-9A-Fa-f]{6})"', txt).group(1)
    return d, fill, vw, vh


def tokenize_path(d):
    tokens = []
    i = 0
    while i < len(d):
        c = d[i]
        if CMD_RE.match(c):
            tokens.append(c)
            i += 1
        elif c in " ,\t\n\r":
            i += 1
        else:
            m = NUM_RE.match(d, i)
            if not m:
                i += 1
                continue
            tokens.append(float(m.group()))
            i = m.end()
    return tokens


def cubic(p0, p1, p2, p3, steps):
    pts = []
    for s in range(1, steps + 1):
        t = s / steps
        mt = 1 - t
        x = (mt**3) * p0[0] + 3 * (mt**2) * t * p1[0] + 3 * mt * (t**2) * p2[0] + (t**3) * p3[0]
        y = (mt**3) * p0[1] + 3 * (mt**2) * t * p1[1] + 3 * mt * (t**2) * p2[1] + (t**3) * p3[1]
        pts.append((x, y))
    return pts


def flatten(d):
    """Retorna lista de subpaths; cada subpath é lista de vértices (fechados)."""
    toks = tokenize_path(d)
    subpaths = []
    cur = []
    cx = cy = sx = sy = 0.0
    i = 0
    cmd = None
    while i < len(toks):
        t = toks[i]
        if isinstance(t, str):
            cmd = t
            i += 1
        rel = cmd.islower()
        C = cmd.upper()
        if C == "M":
            x, y = toks[i], toks[i + 1]; i += 2
            if rel: x += cx; y += cy
            if cur:
                subpaths.append(cur)
            cur = [(x, y)]
            cx, cy = x, y
            sx, sy = x, y
            cmd = "l" if rel else "L"
        elif C == "L":
            x, y = toks[i], toks[i + 1]; i += 2
            if rel: x += cx; y += cy
            cur.append((x, y)); cx, cy = x, y
        elif C == "H":
            x = toks[i]; i += 1
            if rel: x += cx
            cur.append((x, cy)); cx = x
        elif C == "V":
            y = toks[i]; i += 1
            if rel: y += cy
            cur.append((cx, y)); cy = y
        elif C == "C":
            x1, y1, x2, y2, x, y = toks[i:i + 6]; i += 6
            if rel:
                x1 += cx; y1 += cy; x2 += cx; y2 += cy; x += cx; y += cy
            cur.extend(cubic((cx, cy), (x1, y1), (x2, y2), (x, y), BEZIER_STEPS))
            cx, cy = x, y
        elif C == "Z":
            if cur:
                cur.append((sx, sy))
                subpaths.append(cur)
            cur = []
            cx, cy = sx, sy
        else:
            raise ValueError(f"comando de path não suportado: {cmd}")
    if cur:
        subpaths.append(cur)
    return subpaths


def build_edges(subpaths):
    edges = []
    for sp in subpaths:
        for a, b in zip(sp, sp[1:]):
            x0, y0 = a; x1, y1 = b
            if y0 == y1:
                continue
            edges.append((x0, y0, x1, y1))
    return edges


def inside_evenodd(edges, px, py):
    crossings = 0
    for x0, y0, x1, y1 in edges:
        if (y0 <= py < y1) or (y1 <= py < y0):
            xint = x0 + (py - y0) * (x1 - x0) / (y1 - y0)
            if xint > px:
                crossings += 1
    return crossings & 1


def hex_to_rgb(h):
    return int(h[1:3], 16), int(h[3:5], 16), int(h[5:7], 16)


def rasterize(d, fill, vw, vh):
    """Retorna matriz TEXxTEX de (r,g,b): cor de preenchimento ou magenta (transparente)."""
    subpaths = flatten(d)
    edges = build_edges(subpaths)
    fr, fg, fb = hex_to_rgb(fill)
    offx = (TEX - vw) / 2.0
    offy = (TEX - vh) / 2.0
    pix = [[TRANSPARENT] * TEX for _ in range(TEX)]
    for py in range(TEX):
        for px in range(TEX):
            hit = 0
            for iy in range(SS):
                uy = (py + (iy + 0.5) / SS) - offy
                for ix in range(SS):
                    ux = (px + (ix + 0.5) / SS) - offx
                    if inside_evenodd(edges, ux, uy):
                        hit += 1
            if hit * 2 >= SS * SS:  # cobertura >= 0.5 -> preenchido
                pix[py][px] = (fr, fg, fb)
    return pix


def write_bmp(path, pix):
    """Escreve BMP 24-bit (bottom-up), sem compressão."""
    w = len(pix[0])
    h = len(pix)
    row_pad = (4 - (w * 3) % 4) % 4
    row_size = w * 3 + row_pad
    data_size = row_size * h
    file_size = 54 + data_size
    with open(path, "wb") as f:
        f.write(b"BM")
        f.write(struct.pack("<IHHI", file_size, 0, 0, 54))          # file header
        f.write(struct.pack("<IiiHHIIiiII", 40, w, h, 1, 24, 0,      # info header
                            data_size, 0, 0, 0, 0))
        for y in range(h - 1, -1, -1):  # BMP é bottom-up
            for x in range(w):
                r, g, b = pix[y][x]
                f.write(bytes((b, g, r)))
            f.write(b"\x00" * row_pad)


def write_grit(path):
    with open(path, "w") as f:
        f.write("# GERADO por tools/rasterize_ui.py\n")
        f.write("-gb\n")          # bitmap mode
        f.write("-gB4\n")         # 4bpp (16 cores) — assets são cor chapada + transparente
        f.write("-gT FF00FF\n")   # cor transparente (purple/magenta) => índice 0


ASSET_MAP = [
    ("ui_border_active",   "Rectangle Subtractborder_active.svg"),
    ("ui_border_inactive", "Rectangle Subtractborder_inactive.svg"),
    ("ui_background",      "Rectanglebackground.svg"),
]


def main():
    os.makedirs(GFX_DIR, exist_ok=True)
    for name, fn in ASSET_MAP:
        path = os.path.join(ASSETS, fn)
        if not os.path.exists(path):
            print(f"!! asset ausente: {path}", file=sys.stderr)
            sys.exit(1)
        d, fill, vw, vh = parse_svg(path)
        print(f">> {fn}  ({vw:g}x{vh:g}, fill {fill}) -> gfx/{name}.bmp [{TEX}x{TEX}]")
        pix = rasterize(d, fill, vw, vh)
        write_bmp(os.path.join(GFX_DIR, f"{name}.bmp"), pix)
        write_grit(os.path.join(GFX_DIR, f"{name}.grit"))
    print(">> pronto (bmp + grit em gfx/)")


if __name__ == "__main__":
    main()
