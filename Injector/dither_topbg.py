#!/usr/bin/env python3
"""
dither_topbg.py -- generates a "dithered" (translucent) copy of the top-screen background
for EVERY theme of the modified TWiLightMenu++ (DSi theme).

Goal: use the copy as an overlay on top of a gameplay video. OPAQUE pixels show the theme's
brick; TRANSPARENT pixels (holes) let the video show through behind it -- recreating the
translucency WITHOUT runtime alpha blending.

Flow:
  <root>/_nds/TWiLightMenu/dsimenu/themes/<THEME>/quickmenu/topbg.png
    -> <root>/_nds/TWiLightMenu/dsimenu/themes/<THEME>/quickmenu/topbg_dither.png

The copy has the SAME dimensions and the SAME RGB colors as the original; only the ALPHA
channel is rewritten by a DITHER PATTERN (--pattern, see PATTERNS):
  - a "kept" pixel: original RGB, alpha = 255 (opaque);
  - a "hole" pixel:  alpha = 0   (fully transparent).
Output is 8-bit-per-channel RGBA PNG (compatible with lodepng).

Intensity (--intensity N, 1..100, default 40):
  N = percentage of pixels that stay OPAQUE (brick visibility).
    100 -> all opaque (no holes; same as the original, just RGBA)
     40 -> ~40% opaque, ~60% transparent (more of the video shows through)
      1 -> almost fully transparent
  Per-pixel rule (same for every pattern): opaque if tile[y%rows][x%cols] < N.

Pattern (--pattern NAME, default bayer8): bayer8/bayer4/bayer2, cluster (halftone),
  hlines, vlines, checker, noise. Use --list-patterns for the full list with descriptions.

Requires ffmpeg on PATH (PNG decode/encode). Does NOT modify the original topbg.png.

Usage:
    dither_topbg.py [<root>] --intensity <1..100> --pattern <name>
    dither_topbg.py --list-patterns
Examples:
    dither_topbg.py ./.preview/sdcard --intensity 40 --pattern cluster
    dither_topbg.py /Volumes/DSI --intensity 55 --pattern noise
"""
import argparse
import os
import shutil
import struct
import subprocess
import sys

THEMES_SUBPATH = os.path.join("_nds", "TWiLightMenu", "dsimenu", "themes")
PNG_SIG = b"\x89PNG\r\n\x1a\n"


def err(msg):
    sys.stderr.write("ERROR: " + msg + "\n")


def log(msg):
    print("==> " + msg)


def _ffmpeg():
    """Path/command for ffmpeg. deploy.py sets FFMPEG_BIN when it auto-downloads a portable
    copy; standalone use falls back to whatever "ffmpeg" resolves to on PATH."""
    return os.environ.get("FFMPEG_BIN", "ffmpeg")


# ---------------------------------------------------------------------------
# DITHER PATTERNS (ordered dithering).
#
# Each pattern is a "tile" matrix of normalized THRESHOLDS 0..99. The alpha mask is decided,
# per pixel, with the SAME rule for every pattern:
#     opaque (brick)  if  tile[y % rows][x % cols] < intensity
#     hole (video)    otherwise
# Since the tile repeats across the image, the pattern defines the "texture" of the holes.
# All of them are deterministic (idempotent) -- including 'noise', which uses a fixed seed.
# ---------------------------------------------------------------------------
def _normalize(mat, levels):
    """Scales a 0..levels-1 matrix to 0..99 thresholds."""
    return [[(v * 100) // levels for v in row] for row in mat]


def _bayer(n):
    """n x n Bayer matrix (n a power of 2), generated recursively."""
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


# Classic 8x8 clustered-dot (0..63): holes grow in "dots" -> halftone look.
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
    """Lines: threshold depends only on y (horizontal) or only on x (vertical)."""
    steps = [(i * 100) // period for i in range(period)]
    if horizontal:                       # rows x 1 tile -> constant along each row
        return [[s] for s in steps]
    return [steps]                       # 1 x cols tile -> constant along each column


def _noise(n=64, seed=1337):
    """Deterministic ordered noise: n x n tile of pseudo-random thresholds."""
    import random
    rnd = random.Random(seed)
    return [[rnd.randint(0, 99) for _ in range(n)] for _ in range(n)]


# Registry: name -> (description, threshold tile 0..99). Order = order in the helper output.
PATTERNS = {
    "bayer8":  ("Bayer 8x8 -- fine ordered dither, holes spread evenly (default).",
                _normalize(_bayer(8), 64)),
    "bayer4":  ("Bayer 4x4 -- medium ordered dither, slightly coarser texture.",
                _normalize(_bayer(4), 16)),
    "bayer2":  ("Bayer 2x2 -- very coarse ordered dither (4 levels).",
                _normalize(_bayer(2), 4)),
    "cluster": ("Clustered-dot 8x8 -- halftone look (midtone dots).",
                _normalize(_CLUSTER8, 64)),
    "hlines":  ("Horizontal lines (scanlines).",
                _lines(8, horizontal=True)),
    "vlines":  ("Vertical lines.",
                _lines(8, horizontal=False)),
    "checker": ("Checkerboard -- diagonal alternation (~50% effect).",
                [[0, 50], [50, 0]]),
    "noise":   ("Deterministic noise -- random-looking holes (fixed seed).",
                _noise()),
}
DEFAULT_PATTERN = "bayer8"


def parse_png_size(path):
    """Reads width/height from the IHDR chunk without decoding the whole image."""
    with open(path, "rb") as fh:
        head = fh.read(24)
    if len(head) < 24 or head[:8] != PNG_SIG or head[12:16] != b"IHDR":
        raise ValueError("not a valid PNG")
    width, height = struct.unpack(">II", head[16:24])
    if width == 0 or height == 0:
        raise ValueError("invalid dimensions in IHDR")
    return width, height


def build_alpha_mask(width, height, intensity, tile):
    """
    Builds the alpha plane (bytes, len = width*height): 255 (opaque) or 0 (hole).
    Opaque if tile[y % rows][x % cols] < intensity. `tile` is a matrix of 0..99 thresholds.
    """
    rows = len(tile)
    cols = len(tile[0])
    # Precomputes one full-width opacity row for each vertical phase.
    row_patterns = [
        bytes(255 if tile[phase][x % cols] < intensity else 0 for x in range(width))
        for phase in range(rows)
    ]
    return b"".join(row_patterns[y % rows] for y in range(height))


def decode_rgba(src, width, height):
    """Decodes the PNG into raw RGBA (width*height*4 bytes) via ffmpeg."""
    cmd = [_ffmpeg(), "-v", "error", "-i", src,
           "-f", "rawvideo", "-pix_fmt", "rgba", "-"]
    proc = subprocess.run(cmd, capture_output=True)
    if proc.returncode != 0:
        raise RuntimeError("ffmpeg decode failed: "
                           + proc.stderr.decode(errors="replace").strip())
    expected = width * height * 4
    if len(proc.stdout) != expected:
        raise RuntimeError(f"unexpected RGBA size: {len(proc.stdout)} != {expected}")
    return proc.stdout


def encode_rgba_png(rgba, width, height, dst):
    """Encodes raw RGBA back into an 8-bit RGBA PNG via ffmpeg (overwrite)."""
    cmd = [_ffmpeg(), "-v", "error", "-y",
           "-f", "rawvideo", "-pix_fmt", "rgba", "-s", f"{width}x{height}",
           "-i", "-", "-frames:v", "1", "-pix_fmt", "rgba", dst]
    proc = subprocess.run(cmd, input=rgba, capture_output=True)
    if proc.returncode != 0:
        raise RuntimeError("ffmpeg encode failed: "
                           + proc.stderr.decode(errors="replace").strip())


def dither_one(topbg, intensity, tile):
    """Generates topbg_dither.png next to topbg.png. Returns (width, height)."""
    width, height = parse_png_size(topbg)
    rgba = bytearray(decode_rgba(topbg, width, height))
    # Rewrites ONLY the alpha channel (bytes at indexes 3,7,11,...).
    rgba[3::4] = build_alpha_mask(width, height, intensity, tile)
    dst = os.path.join(os.path.dirname(topbg), "topbg_dither.png")
    encode_rgba_png(bytes(rgba), width, height, dst)
    return width, height


def find_root(explicit):
    """Resolves <root>: uses the argument if given; otherwise tries common mount points."""
    if explicit:
        return explicit if os.path.isdir(os.path.join(explicit, THEMES_SUBPATH)) else None

    candidates = ["./.preview/sdcard", "./.preview", "./sdcard"]

    # macOS: every folder under /Volumes is a mounted volume.
    if os.path.isdir("/Volumes"):
        candidates += [os.path.join("/Volumes", n) for n in sorted(os.listdir("/Volumes"))]

    # Linux: typical auto-mount locations for removable media.
    user = os.environ.get("USER") or os.environ.get("LOGNAME") or ""
    for base in (f"/media/{user}", f"/run/media/{user}", "/media"):
        if user and os.path.isdir(base):
            candidates += [os.path.join(base, n) for n in sorted(os.listdir(base))]

    # Windows: check every drive letter.
    if os.name == "nt":
        import string
        candidates += [f"{letter}:\\" for letter in string.ascii_uppercase]

    for c in candidates:
        if c and os.path.isdir(os.path.join(c, THEMES_SUBPATH)):
            return c
    return None


def print_patterns(prog="dither_topbg.py", examples=None):
    """Command helper: lists the available dither patterns.

    `prog`/`examples` let any tool reusing this list show examples under its own name.
    """
    print("Available dither patterns (--pattern <name>):\n")
    for name, (desc, _tile) in PATTERNS.items():
        star = "  (default)" if name == DEFAULT_PATTERN else ""
        print(f"  {name:<8} {desc}{star}")
    print("\nExamples:")
    if examples is None:
        examples = [
            f"{prog} /Volumes/DSI --intensity 40 --pattern cluster",
            f"{prog} /Volumes/DSI --pattern noise",
        ]
    for ex in examples:
        print(f"  {ex}")


def main(argv):
    ap = argparse.ArgumentParser(
        description="Generates topbg_dither.png (RGBA, dithered alpha) for every theme.")
    ap.add_argument("root", nargs="?", default=None,
                    help="SD root (contains _nds/...). If omitted, tries common mount points.")
    ap.add_argument("--intensity", type=int, default=40,
                    help="1..100 = %% of pixels that stay OPAQUE (brick). Default 40.")
    ap.add_argument("--pattern", default=DEFAULT_PATTERN, choices=list(PATTERNS),
                    help=f"dither pattern (default {DEFAULT_PATTERN}). "
                         "See --list-patterns.")
    ap.add_argument("--list-patterns", action="store_true",
                    help="lists the available dither patterns and exits.")
    args = ap.parse_args(argv)

    if args.list_patterns:
        print_patterns()
        return 0
    if not (1 <= args.intensity <= 100):
        err("--intensity must be between 1 and 100.")
        return 2
    if shutil.which(_ffmpeg()) is None:
        err("ffmpeg not found on PATH. Install it (e.g. https://ffmpeg.org/download.html).")
        return 1

    tile = PATTERNS[args.pattern][1]
    root = find_root(args.root)
    if not root:
        err("themes folder not found "
            f"(<root>/{THEMES_SUBPATH}). Pass <root> explicitly.")
        return 1
    themes_dir = os.path.join(root, THEMES_SUBPATH)
    log(f"Themes in: {themes_dir}")
    log(f"Intensity: {args.intensity}% opaque (brick), pattern '{args.pattern}'.")

    processed, skipped, failed = [], [], []
    for name in sorted(os.listdir(themes_dir)):
        theme_dir = os.path.join(themes_dir, name)
        if not os.path.isdir(theme_dir):
            continue
        topbg = os.path.join(theme_dir, "quickmenu", "topbg.png")
        if not os.path.isfile(topbg):
            log(f"[skip] {name}: no quickmenu/topbg.png")
            skipped.append(name)
            continue
        try:
            w, h = dither_one(topbg, args.intensity, tile)
            log(f"[ok]   {name}: topbg_dither.png ({w}x{h} RGBA)")
            processed.append(name)
        except Exception as e:                       # noqa: BLE001 (log and continue)
            err(f"{name}: failed -- {e}")
            failed.append(name)

    print()
    log("Summary:")
    print(f"    processed: {len(processed)}")
    print(f"    skipped (no topbg.png): {len(skipped)}")
    print(f"    failures: {len(failed)}")
    if failed:
        print(f"    -> failed themes: {', '.join(failed)}")
    # Success unless ALL candidates with a topbg failed.
    return 0 if not failed or processed else 1


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
