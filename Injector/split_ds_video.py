#!/usr/bin/env python3
"""
split_ds_video.py — converte o vídeo empilhado (2 telas) do ScreenScraper em DOIS
vídeos otimizados para a versão modificada do TWiLightMenu rodando no DS.

O vídeo baixado tem as duas telas do DS empilhadas verticalmente (ex.: 320x480 =
tela superior 320x240 + tela inferior 320x240). Este script:
  1) corta a metade de cima (tela superior) e a metade de baixo (tela inferior);
  2) redimensiona cada uma para uma resolução BAIXA (padrão 128x96, 4:3), pensada
     para ser reescalada por hardware (BG affine) até 256x192 no DS — a menor
     resolução armazenada é o maior fator de economia de leitura do cartão SD;
  3) grava cada tela como um arquivo .tgrv (magic "TGR2").

Como o .tgrv é vídeo CRU (sem codec), o "bitrate" é fixo:
    bytes/segundo = largura * altura * bytes_por_pixel * fps
e é a banda de leitura do SD que limita o FPS no DS. Por isso otimizamos os três
eixos: RESOLUÇÃO (quadrático), PROFUNDIDADE DE COR (16bpp->8bpp = metade) e FPS.

Dois modos de cor:
  * pal8  (padrão): 8 bpp indexado + paleta de 256 cores otimizada por vídeo.
    1 byte/pixel — metade dos dados do BGR555 e casa com o modo BG bitmap de
    256 cores do DS (a paleta vai para a palette RAM; o blit é só o índice).
  * bgr555 (legado): 16 bpp direto (u16 BGR555), maior qualidade e o dobro de dados.

Formato TGR2 (little-endian):
  offset 0 : magic   'TGR2'  (4 bytes ASCII)
  offset 4 : width   u16
  offset 6 : height  u16
  offset 8 : fps     u16
  offset 10: nframes u32
  offset 14: fmt     u8    (0 = BGR555 16bpp, 1 = PAL8 8bpp)
  offset 15: flags   u8    (bit0 = pixels já têm bit15/opaco setado)
  offset 16: pal_cnt u16   (0 no BGR555; 256 no PAL8)
  offset 18: paleta  pal_cnt x u16 BGR555 (bit15=1 opaco)   [só PAL8]
  offset ..: nframes x frame
      pal8  : width*height bytes    (1 índice por pixel)
      bgr555: width*height*2 bytes  (u16 BGR555, bit15=1 opaco)

Leitura no DS: leia o header; no pal8 carregue a paleta na palette RAM uma vez e
então `fread` (width*height) bytes por frame direto para o BG bitmap de 256 cores;
no bgr555 `fread` (width*height*2) bytes para um u16[width*height] e blite direto.

Requer ffmpeg no PATH.
"""
import argparse
import os
import struct
import subprocess
import sys
import tempfile

MAGIC = b"TGR2"
HEADER_FMT = "<4sHHHIBBH"          # magic,w,h,fps,nframes,fmt,flags,pal_cnt
HEADER_SIZE = struct.calcsize(HEADER_FMT)   # 18
NFRAMES_OFFSET = 10                 # posição do u32 nframes dentro do header

FMT_BGR555 = 0
FMT_PAL8 = 1

# Tabela de tradução C-speed: seta o bit alto (0x80) de um byte.
# Num u16 little-endian, o bit15 é o bit7 do 2º byte (índices ímpares do stream).
_HIGHBIT = bytes((i | 0x80) for i in range(256))


def err(msg):
    sys.stderr.write("ERROR: " + msg + "\n")


def log(msg):
    print("==> " + msg)


def set_alpha_bit(frame: bytes) -> bytes:
    """Seta bit15=1 (opaco) em cada pixel u16 do frame, a C-speed."""
    buf = bytearray(frame)
    odd = bytearray(buf[1::2])           # bytes altos de cada u16
    buf[1::2] = odd.translate(_HIGHBIT)  # OR 0x80 em todos
    return bytes(buf)


def rgb24_to_bgr555_palette(rgb: bytes, count: int = 256) -> bytes:
    """Converte `count` triplas RGB24 (ordem de índice) em u16 BGR555 (bit15=1)."""
    out = bytearray(count * 2)
    for i in range(count):
        r, g, b = rgb[i * 3], rgb[i * 3 + 1], rgb[i * 3 + 2]
        val = 0x8000 | (r >> 3) | ((g >> 3) << 5) | ((b >> 3) << 10)
        struct.pack_into("<H", out, i * 2, val)
    return bytes(out)


def build_vf_prefix(crop, width, height, fps):
    """Filtro comum: recorta a tela, reescala p/ WxH (lanczos) e fixa o fps."""
    return f"{crop},scale={width}:{height}:flags=lanczos,fps={fps}"


def gen_palette(input_mp4, crop, width, height, max_seconds):
    """Gera a paleta ótima de 256 cores e retorna 256 triplas RGB24 (ordem de índice)."""
    with tempfile.NamedTemporaryFile(suffix=".png", delete=False) as tf:
        pal_png = tf.name
    try:
        vf = (f"{crop},scale={width}:{height}:flags=lanczos,"
              f"palettegen=max_colors=256:reserve_transparent=0")
        cmd = ["ffmpeg", "-v", "error"]
        if max_seconds and max_seconds > 0:
            cmd += ["-t", str(max_seconds)]
        cmd += ["-i", input_mp4, "-vf", vf, "-y", pal_png]
        subprocess.run(cmd, check=True, capture_output=True)

        # Lê os pixels da paleta em ordem row-major (== ordem dos índices do pal8).
        cmd = ["ffmpeg", "-v", "error", "-i", pal_png,
               "-pix_fmt", "rgb24", "-f", "rawvideo", "-"]
        rgb = subprocess.run(cmd, check=True, capture_output=True).stdout
        if len(rgb) < 256 * 3:                 # completa se vier < 256 cores
            rgb = rgb + b"\x00" * (256 * 3 - len(rgb))
        return rgb[: 256 * 3]
    finally:
        try:
            os.unlink(pal_png)
        except OSError:
            pass


def _run_pal8(input_mp4, out_path, crop, width, height, fps, max_seconds,
              dither, palette, frame_bytes):
    """Gera a paleta em arquivo temporário e faz o streaming pal8 -> TGR2."""
    with tempfile.NamedTemporaryFile(suffix=".png", delete=False) as tf:
        pal_png = tf.name
    try:
        vf_pal = (f"{crop},scale={width}:{height}:flags=lanczos,"
                  f"palettegen=max_colors=256:reserve_transparent=0")
        cmd = ["ffmpeg", "-v", "error"]
        if max_seconds and max_seconds > 0:
            cmd += ["-t", str(max_seconds)]
        cmd += ["-i", input_mp4, "-vf", vf_pal, "-y", pal_png]
        subprocess.run(cmd, check=True, capture_output=True)

        lavfi = (f"{crop},scale={width}:{height}:flags=lanczos,fps={fps}[x];"
                 f"[x][1:v]paletteuse=dither={dither}")
        cmd = ["ffmpeg", "-v", "error"]
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
    """Executa o ffmpeg (stdout raw) e empacota frames no arquivo TGR2."""
    proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    nframes = 0
    # No pal8 o rawvideo do ffmpeg emite, DEPOIS dos width*height índices, a AVPALETTE
    # (256 x RGBA = 1024 bytes) a CADA frame. Precisamos ler esses 1024 bytes extras e
    # descartá-los; senão o leitor drifta 1024 bytes/frame (rolagem vertical estilo TV).
    pipe_bytes = frame_bytes + (1024 if fmt == FMT_PAL8 else 0)
    with open(out_path, "wb") as out:
        # Header com nframes provisório (0); corrigido no final via seek.
        out.write(struct.pack(HEADER_FMT, MAGIC, width, height, fps, 0,
                              fmt, flags, pal_cnt))
        if pal_cnt:
            out.write(palette)
        while True:
            chunk = proc.stdout.read(pipe_bytes)
            if not chunk or len(chunk) < pipe_bytes:
                break                          # fim do stream / frame parcial
            frame = chunk[:frame_bytes]        # só os índices; ignora a paleta por-frame
            if set_alpha:
                frame = set_alpha_bit(frame)
            out.write(frame)
            nframes += 1
        out.seek(NFRAMES_OFFSET)
        out.write(struct.pack("<I", nframes))

    _, stderr = proc.communicate()
    if proc.returncode != 0:
        raise RuntimeError(
            f"ffmpeg falhou ({proc.returncode}): "
            f"{stderr.decode(errors='replace').strip()}")
    return nframes


def main(argv):
    ap = argparse.ArgumentParser(
        description="Divide o vídeo DS empilhado em 2 .tgrv (TGR2) otimizados.")
    ap.add_argument("input", help="vídeo .mp4 empilhado (2 telas) do fetch_ds_media.sh")
    ap.add_argument("outdir", nargs="?", default=None,
                    help="pasta de saída (padrão: pasta do input)")
    ap.add_argument("--width", type=int, default=128,
                    help="largura armazenada por tela (padrão 128; DS reescala p/ 256)")
    ap.add_argument("--height", type=int, default=96,
                    help="altura armazenada por tela (padrão 96; DS reescala p/ 192)")
    ap.add_argument("--fps", type=int, default=30,
                    help="frames por segundo do TGR2 (padrão 30; divide o vsync de 60Hz)")
    ap.add_argument("--mode", choices=["pal8", "bgr555"], default="pal8",
                    help="cor: pal8 (8bpp, padrão) ou bgr555 (16bpp legado)")
    ap.add_argument("--dither", default="bayer:bayer_scale=3",
                    help="dither do paletteuse (pal8). Use 'none' p/ desligar.")
    ap.add_argument("--max-seconds", type=float, default=0,
                    help="limita a duração (0 = vídeo inteiro).")
    ap.add_argument("--no-alpha-bit", action="store_true",
                    help="[bgr555] NÃO setar bit15 (leitor seta o alpha ele mesmo)")
    args = ap.parse_args(argv)

    if args.width <= 0 or args.height <= 0:
        err("width/height devem ser > 0")
        return 1
    if not os.path.isfile(args.input):
        err(f"input não encontrado: {args.input}")
        return 1
    if subprocess.run(["bash", "-c", "command -v ffmpeg"],
                      stdout=subprocess.DEVNULL).returncode != 0:
        err("ffmpeg não encontrado no PATH. Instale (brew install ffmpeg).")
        return 1

    outdir = args.outdir or os.path.dirname(os.path.abspath(args.input))
    os.makedirs(outdir, exist_ok=True)

    # base do nome: remove extensão e um sufixo '-video' se houver.
    base = os.path.splitext(os.path.basename(args.input))[0]
    if base.endswith("-video"):
        base = base[: -len("-video")]

    fmt = FMT_PAL8 if args.mode == "pal8" else FMT_BGR555
    # No bgr555 o flag de alpha depende de --no-alpha-bit; no pal8 a paleta cuida disso.
    set_alpha = not args.no_alpha_bit
    top_path = os.path.join(outdir, base + "-top.tgrv")
    bot_path = os.path.join(outdir, base + "-bottom.tgrv")

    def do_screen(out_path, crop, label):
        log(f"{label} -> {out_path}")
        if fmt == FMT_BGR555:
            frame_bytes = args.width * args.height * 2
            vf = build_vf_prefix(crop, args.width, args.height, args.fps)
            cmd = ["ffmpeg", "-v", "error", "-i", args.input]
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

    n_top = do_screen(top_path, "crop=iw:ih/2:0:0", "Tela superior")
    n_bot = do_screen(bot_path, "crop=iw:ih/2:0:ih/2", "Tela inferior")

    bpp = 1 if fmt == FMT_PAL8 else 2
    fb = args.width * args.height * bpp
    rate = fb * args.fps / 1024.0
    log(f"Concluído: superior {n_top} frames "
        f"({(HEADER_SIZE + (512 if fmt == FMT_PAL8 else 0) + n_top*fb)/1048576:.2f} MB), "
        f"inferior {n_bot} frames "
        f"({(HEADER_SIZE + (512 if fmt == FMT_PAL8 else 0) + n_bot*fb)/1048576:.2f} MB) "
        f"@ {args.fps} fps, {args.width}x{args.height} "
        f"{'PAL8 8bpp' if fmt == FMT_PAL8 else 'BGR555 16bpp'} "
        f"(~{rate:.0f} KB/s por tela).")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
