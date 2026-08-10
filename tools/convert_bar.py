#!/usr/bin/env python3
"""
convert_bar.py — converte assets/Botton_bar.png (RGBA) para o pipeline de grit:
gera gfx/botton_bar.bmp (256x64, fundo magenta #FF00FF = transparente) + .grit.

Decodifica o PNG (color type 6, 8-bit, sem interlace) em Python puro (zlib),
faz o unfilter das scanlines, e escreve um BMP 24-bit onde pixels com alpha<128
viram magenta (cor-chave de transparência do grit "-gT FF00FF").

Saída: romsel_dsimenutheme/gfx/botton_bar.bmp + botton_bar.grit
"""
import os, struct, zlib

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SRC = os.path.join(ROOT, "assets", "Botton_bar.png")
GFX = os.path.join(ROOT, "romsel_dsimenutheme", "gfx")
TEX_W, TEX_H = 256, 64          # textura potência de 2 (barra ocupa o topo)
TRANSPARENT = (255, 0, 255)     # #FF00FF => índice 0 no grit


def decode_png_rgba(path):
    d = open(path, "rb").read()
    assert d[:8] == b"\x89PNG\r\n\x1a\n", "não é PNG"
    i = 8
    w = h = ct = 0
    idat = b""
    while i < len(d):
        ln = struct.unpack(">I", d[i:i+4])[0]
        typ = d[i+4:i+8]
        data = d[i+8:i+8+ln]
        if typ == b"IHDR":
            w, h, bd, ct, cm, fm, il = struct.unpack(">IIBBBBB", data[:13])
            assert bd == 8 and ct == 6 and il == 0, f"suporta só 8-bit RGBA não-interlaced (got bd={bd} ct={ct})"
        elif typ == b"IDAT":
            idat += data
        elif typ == b"IEND":
            break
        i += 12 + ln
    raw = zlib.decompress(idat)
    stride = w * 4
    out = bytearray(w * h * 4)
    prev = bytearray(stride)
    pos = 0
    for y in range(h):
        ft = raw[pos]; pos += 1
        line = bytearray(raw[pos:pos+stride]); pos += stride
        if ft == 1:      # Sub
            for x in range(stride):
                line[x] = (line[x] + (line[x-4] if x >= 4 else 0)) & 0xff
        elif ft == 2:    # Up
            for x in range(stride):
                line[x] = (line[x] + prev[x]) & 0xff
        elif ft == 3:    # Average
            for x in range(stride):
                a = line[x-4] if x >= 4 else 0
                line[x] = (line[x] + ((a + prev[x]) >> 1)) & 0xff
        elif ft == 4:    # Paeth
            for x in range(stride):
                a = line[x-4] if x >= 4 else 0
                b = prev[x]
                c = prev[x-4] if x >= 4 else 0
                p = a + b - c
                pa, pb, pc = abs(p-a), abs(p-b), abs(p-c)
                pr = a if (pa <= pb and pa <= pc) else (b if pb <= pc else c)
                line[x] = (line[x] + pr) & 0xff
        out[y*stride:(y+1)*stride] = line
        prev = line
    return w, h, out


def write_bmp(path, pix, w, h):
    pad = (4 - (w * 3) % 4) % 4
    rowsz = w * 3 + pad
    with open(path, "wb") as f:
        f.write(b"BM")
        f.write(struct.pack("<IHHI", 54 + rowsz * h, 0, 0, 54))
        f.write(struct.pack("<IiiHHIIiiII", 40, w, h, 1, 24, 0, rowsz * h, 0, 0, 0, 0))
        for y in range(h - 1, -1, -1):   # bottom-up
            for x in range(w):
                r, g, b = pix[y][x]
                f.write(bytes((b, g, r)))
            f.write(b"\x00" * pad)


def main():
    os.makedirs(GFX, exist_ok=True)
    w, h, rgba = decode_png_rgba(SRC)
    print(f">> {os.path.basename(SRC)} {w}x{h} RGBA -> gfx/botton_bar.bmp [{TEX_W}x{TEX_H}]")
    pix = [[TRANSPARENT] * TEX_W for _ in range(TEX_H)]
    for y in range(min(h, TEX_H)):
        for x in range(min(w, TEX_W)):
            i = (y * w + x) * 4
            r, g, b, a = rgba[i], rgba[i+1], rgba[i+2], rgba[i+3]
            if a >= 128:
                pix[y][x] = (r, g, b)
    write_bmp(os.path.join(GFX, "botton_bar.bmp"), pix, TEX_W, TEX_H)
    with open(os.path.join(GFX, "botton_bar.grit"), "w") as f:
        f.write("# GERADO por tools/convert_bar.py\n-gb\n-gB8\n-gT FF00FF\n")
    print(">> pronto (bmp + grit em gfx/)")


if __name__ == "__main__":
    main()
