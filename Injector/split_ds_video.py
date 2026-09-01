#!/usr/bin/env python3
"""
split_ds_video.py -- converts the ScreenScraper stacked (2-screen) video into TWO videos
optimized for the modified TWiLightMenu build running on the DS.

The downloaded video has the DS's two screens stacked vertically (e.g. 320x480 = top screen
320x240 + bottom screen 320x240). This script:
  1) crops the top half (top screen) and the bottom half (bottom screen);
  2) resizes each to a LOW resolution (default 128x96, 4:3), meant to be hardware-upscaled
     (affine BG) to 256x192 on the DS -- the smaller the stored resolution, the bigger the
     saving on SD card read bandwidth;
  3) writes each screen as a .tgrv file (magic "TGR2").

Since .tgrv is RAW video (no codec), the "bitrate" is fixed:
    bytes/second = width * height * bytes_per_pixel * fps
and it's the DS's SD read bandwidth that caps the FPS. That's why we optimize all three
axes: RESOLUTION (quadratic), COLOR DEPTH (16bpp->8bpp = half), and FPS.

Two color modes:
  * pal8  (default): 8 bpp indexed + a 256-color palette optimized per video.
    1 byte/pixel -- half the data of BGR555 and matches the DS's 256-color bitmap BG mode
    (the palette goes into palette RAM; the blit is just the index).
  * bgr555 (legacy): direct 16 bpp (u16 BGR555), higher quality and twice the data.

TGR2 format (little-endian):
  offset 0 : magic   'TGR2'  (4 ASCII bytes)
  offset 4 : width   u16
  offset 6 : height  u16
  offset 8 : fps     u16
  offset 10: nframes u32
  offset 14: fmt     u8    (0 = BGR555 16bpp, 1 = PAL8 8bpp)
  offset 15: flags   u8    (bit0 = pixels already have bit15/opaque set)
  offset 16: pal_cnt u16   (0 for BGR555; 256 for PAL8)
  offset 18: palette pal_cnt x u16 BGR555 (bit15=1 opaque)   [PAL8 only]
  offset ..: nframes x frame
      pal8  : width*height bytes    (1 index per pixel)
      bgr555: width*height*2 bytes  (u16 BGR555, bit15=1 opaque)

Reading it on the DS: read the header; for pal8 load the palette into palette RAM once and
then `fread` (width*height) bytes per frame directly into the 256-color bitmap BG; for
bgr555 `fread` (width*height*2) bytes per frame into a u16[width*height] and blit directly.

Requires ffmpeg on PATH.
"""
import argparse
import os
import shutil
import struct
import subprocess
import sys
import tempfile

MAGIC = b"TGR2"
HEADER_FMT = "<4sHHHIBBH"          # magic,w,h,fps,nframes,fmt,flags,pal_cnt
HEADER_SIZE = struct.calcsize(HEADER_FMT)   # 18
NFRAMES_OFFSET = 10                 # position of the u32 nframes field inside the header

FMT_BGR555 = 0
FMT_PAL8 = 1

# C-speed translation table: sets the high bit (0x80) of a byte.
# In a little-endian u16, bit15 is bit7 of the 2nd byte (odd indexes in the stream).
_HIGHBIT = bytes((i | 0x80) for i in range(256))


def _ffmpeg():
    """Path/command for ffmpeg. deploy.py sets FFMPEG_BIN when it auto-downloads a portable
    copy; standalone use falls back to whatever "ffmpeg" resolves to on PATH."""
    return os.environ.get("FFMPEG_BIN", "ffmpeg")


def err(msg):
    sys.stderr.write("ERROR: " + msg + "\n")


def log(msg):
    print("==> " + msg)


def set_alpha_bit(frame: bytes) -> bytes:
    """Sets bit15=1 (opaque) on every u16 pixel of the frame, at C speed."""
    buf = bytearray(frame)
    odd = bytearray(buf[1::2])           # high byte of every u16
    buf[1::2] = odd.translate(_HIGHBIT)  # OR 0x80 on all of them
    return bytes(buf)


def rgb24_to_bgr555_palette(rgb: bytes, count: int = 256) -> bytes:
    """Converts `count` RGB24 triplets (in index order) into u16 BGR555 (bit15=1)."""
    out = bytearray(count * 2)
    for i in range(count):
        r, g, b = rgb[i * 3], rgb[i * 3 + 1], rgb[i * 3 + 2]
        val = 0x8000 | (r >> 3) | ((g >> 3) << 5) | ((b >> 3) << 10)
        struct.pack_into("<H", out, i * 2, val)
    return bytes(out)


def build_vf_prefix(crop, width, height, fps):
    """Common filter: crops the screen, resizes to WxH (lanczos) and fixes the fps."""
    return f"{crop},scale={width}:{height}:flags=lanczos,fps={fps}"


def gen_palette(input_mp4, crop, width, height, max_seconds):
    """Generates the optimal 256-color palette and returns 256 RGB24 triplets (index order)."""
    with tempfile.NamedTemporaryFile(suffix=".png", delete=False) as tf:
        pal_png = tf.name
    try:
        vf = (f"{crop},scale={width}:{height}:flags=lanczos,"
              f"palettegen=max_colors=256:reserve_transparent=0")
        cmd = [_ffmpeg(), "-v", "error"]
        if max_seconds and max_seconds > 0:
            cmd += ["-t", str(max_seconds)]
        cmd += ["-i", input_mp4, "-vf", vf, "-y", pal_png]
        subprocess.run(cmd, check=True, capture_output=True)

        # Reads the palette pixels in row-major order (== the pal8 index order).
        cmd = [_ffmpeg(), "-v", "error", "-i", pal_png,
               "-pix_fmt", "rgb24", "-f", "rawvideo", "-"]
        rgb = subprocess.run(cmd, check=True, capture_output=True).stdout
        if len(rgb) < 256 * 3:                 # pad if fewer than 256 colors came back
            rgb = rgb + b"\x00" * (256 * 3 - len(rgb))
        return rgb[: 256 * 3]
    finally:
        try:
            os.unlink(pal_png)
        except OSError:
            pass


def _run_pal8(input_mp4, out_path, crop, width, height, fps, max_seconds,
              dither, palette, frame_bytes):
    """Generates the palette into a temp file and streams pal8 -> TGR2."""
    with tempfile.NamedTemporaryFile(suffix=".png", delete=False) as tf:
        pal_png = tf.name
    try:
        vf_pal = (f"{crop},scale={width}:{height}:flags=lanczos,"
                  f"palettegen=max_colors=256:reserve_transparent=0")
        cmd = [_ffmpeg(), "-v", "error"]
        if max_seconds and max_seconds > 0:
            cmd += ["-t", str(max_seconds)]
        cmd += ["-i", input_mp4, "-vf", vf_pal, "-y", pal_png]
        subprocess.run(cmd, check=True, capture_output=True)

        lavfi = (f"{crop},scale={width}:{height}:flags=lanczos,fps={fps}[x];"
                 f"[x][1:v]paletteuse=dither={dither}")
        cmd = [_ffmpeg(), "-v", "error"]
        if max_seconds and max_seconds > 0:
            cmd += ["-t", str(max_seconds)]
        cmd += ["-i", input_mp4, "-i", pal_png, "-lavfi", lavfi,
                "-pix_fmt", "pal8", "-f", "rawvideo", "-"]
        return _stream_frames(cmd, out_path, width, height, fps, FMT_PAL8,
                              1, 256, palette, frame_bytes, set_alpha=False)
    finally:
        try:
            os.unlink(pal_png)
        except OSError:
            pass


def _stream_frames(cmd, out_path, width, height, fps, fmt, flags, pal_cnt,
                   palette, frame_bytes, set_alpha):
    """Runs ffmpeg (raw stdout) and packs the frames into the TGR2 file."""
    proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    nframes = 0
    # In pal8, ffmpeg's rawvideo output emits, right after the width*height indexes, the
    # AVPALETTE (256 x RGBA = 1024 bytes) on EVERY frame. We need to read those extra 1024
    # bytes and discard them; otherwise the reader drifts 1024 bytes/frame (TV-style vertical
    # roll).
    pipe_bytes = frame_bytes + (1024 if fmt == FMT_PAL8 else 0)
    with open(out_path, "wb") as out:
        # Header with a provisional nframes (0); fixed up at the end via seek.
        out.write(struct.pack(HEADER_FMT, MAGIC, width, height, fps, 0,
                              fmt, flags, pal_cnt))
        if pal_cnt:
            out.write(palette)
        while True:
            chunk = proc.stdout.read(pipe_bytes)
            if not chunk or len(chunk) < pipe_bytes:
                break                          # end of stream / partial frame
            frame = chunk[:frame_bytes]        # indexes only; discard the per-frame palette
            if set_alpha:
                frame = set_alpha_bit(frame)
            out.write(frame)
            nframes += 1
        out.seek(NFRAMES_OFFSET)
        out.write(struct.pack("<I", nframes))

    _, stderr = proc.communicate()
    if proc.returncode != 0:
        raise RuntimeError(
            f"ffmpeg failed ({proc.returncode}): "
            f"{stderr.decode(errors='replace').strip()}")
    return nframes


def main(argv):
    ap = argparse.ArgumentParser(
        description="Splits the stacked DS video into 2 optimized .tgrv (TGR2) files.")
    ap.add_argument("input", help="stacked (2-screen) .mp4 video from fetch_ds_media.py")
    ap.add_argument("outdir", nargs="?", default=None,
                    help="output folder (default: the input's folder)")
    ap.add_argument("--width", type=int, default=128,
                    help="stored width per screen (default 128; the DS upscales to 256)")
    ap.add_argument("--height", type=int, default=96,
                    help="stored height per screen (default 96; the DS upscales to 192)")
    ap.add_argument("--fps", type=int, default=30,
                    help="frames per second of the TGR2 file (default 30; divides the 60Hz vsync)")
    ap.add_argument("--mode", choices=["pal8", "bgr555"], default="pal8",
                    help="color: pal8 (8bpp, default) or bgr555 (16bpp, legacy)")
    ap.add_argument("--dither", default="bayer:bayer_scale=3",
                    help="paletteuse dither (pal8). Use 'none' to disable.")
    ap.add_argument("--max-seconds", type=float, default=0,
                    help="caps the duration (0 = the whole video).")
    ap.add_argument("--no-alpha-bit", action="store_true",
                    help="[bgr555] do NOT set bit15 (the reader sets the alpha bit itself)")
    args = ap.parse_args(argv)

    if args.width <= 0 or args.height <= 0:
        err("width/height must be > 0")
        return 1
    if not os.path.isfile(args.input):
        err(f"input not found: {args.input}")
        return 1
    if shutil.which(_ffmpeg()) is None:
        err("ffmpeg not found on PATH. Install it (e.g. https://ffmpeg.org/download.html).")
        return 1

    outdir = args.outdir or os.path.dirname(os.path.abspath(args.input))
    os.makedirs(outdir, exist_ok=True)

    # base name: strips the extension and a trailing '-video' suffix, if present.
    base = os.path.splitext(os.path.basename(args.input))[0]
    if base.endswith("-video"):
        base = base[: -len("-video")]

    fmt = FMT_PAL8 if args.mode == "pal8" else FMT_BGR555
    # In bgr555 the alpha flag depends on --no-alpha-bit; in pal8 the palette handles it.
    set_alpha = not args.no_alpha_bit
    top_path = os.path.join(outdir, base + "-top.tgrv")
    bot_path = os.path.join(outdir, base + "-bottom.tgrv")

    def do_screen(out_path, crop, label):
        log(f"{label} -> {out_path}")
        if fmt == FMT_BGR555:
            frame_bytes = args.width * args.height * 2
            vf = build_vf_prefix(crop, args.width, args.height, args.fps)
            cmd = [_ffmpeg(), "-v", "error", "-i", args.input]
            if args.max_seconds and args.max_seconds > 0:
                cmd += ["-t", str(args.max_seconds)]
            cmd += ["-an", "-vf", vf, "-pix_fmt", "bgr555le",
                    "-f", "rawvideo", "-"]
            return _stream_frames(cmd, out_path, args.width, args.height,
                                  args.fps, FMT_BGR555, 1 if set_alpha else 0,
                                  0, b"", frame_bytes, set_alpha=set_alpha)
        else:
            frame_bytes = args.width * args.height
            pal_rgb = gen_palette(args.input, crop, args.width, args.height,
                                  args.max_seconds)
            palette = rgb24_to_bgr555_palette(pal_rgb, 256)
            return _run_pal8(args.input, out_path, crop, args.width,
                             args.height, args.fps, args.max_seconds,
                             args.dither, palette, frame_bytes)

    n_top = do_screen(top_path, "crop=iw:ih/2:0:0", "Top screen")
    n_bot = do_screen(bot_path, "crop=iw:ih/2:0:ih/2", "Bottom screen")

    bpp = 1 if fmt == FMT_PAL8 else 2
    fb = args.width * args.height * bpp
    rate = fb * args.fps / 1024.0
    log(f"Done: top {n_top} frames "
        f"({(HEADER_SIZE + (512 if fmt == FMT_PAL8 else 0) + n_top*fb)/1048576:.2f} MB), "
        f"bottom {n_bot} frames "
        f"({(HEADER_SIZE + (512 if fmt == FMT_PAL8 else 0) + n_bot*fb)/1048576:.2f} MB) "
        f"@ {args.fps} fps, {args.width}x{args.height} "
        f"{'PAL8 8bpp' if fmt == FMT_PAL8 else 'BGR555 16bpp'} "
        f"(~{rate:.0f} KB/s per screen).")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
