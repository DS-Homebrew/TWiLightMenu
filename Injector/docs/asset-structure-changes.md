# Asset structure changes -- prompt for the DS side (modified TWiLightMenu)

> Ready-to-hand-off prompt for whoever updates the asset reader on the DS.
> Documents the layout produced by the host pipeline (GridFootage:
> `deploy.py` -> `scan_and_bind.py` -> `fetch_ds_media.py` -> `split_ds_video.py`).

---

## Context

The host pipeline (GridFootage) changed the asset layout that the modified TWiLightMenu
build consumes. Update the DS-side reader to match the layout below.

## Location (on the SD card)

Everything lives under: `<SD>/_nds/TWiLightMenu/dsimenu/`

```
<SD>/_nds/TWiLightMenu/dsimenu/
├── manifest.yml               # source of truth: game_id -> identity + assets
├── assets_index.yml           # runtime index: ROM name (no ext) -> game_id
└── assets/
    └── <sha1>/                 # one folder per game; <sha1> = SHA1 of the whole .nds file
        ├── logo.png            # transparent logo / wheel
        ├── top.tgrv            # TOP SCREEN video
        └── bottom.tgrv         # BOTTOM SCREEN video
```

(`logos/` + `logos.yml`, from the older name-based logo feature, still live alongside this --
they don't conflict.)

## What changed vs. the previous layout

- **BEFORE:** assets had `{ logo, video }` and the video was a stacked mp4 (2 screens).
- **NOW:** the stacked video was **SPLIT** into two screens and converted to the **TGRV**
  format (raw frames, direct blit). The `video` (mp4) field is **no longer used** (stays
  `null`) and the mp4 is not copied to the SD. Use `video_top` and `video_bottom`.
- **NEW:** `assets_index.yml` -- a name-based index the DS uses at runtime to match the
  focused ROM without hashing (base name without extension -> game_id).

## `manifest.yml` schema

```yaml
version: 1
games:
  - game_id: "<sha1>"
    identity:                      # identity by CONTENT (priority sha1 > md5 > crc32+size)
      sha1: "<hex>"
      md5:  "<hex>"
      crc32: "<hex>"
      size: <bytes>
    rom_name: "<file name>"        # informational / human fallback only
    assets:
      logo: "assets/<sha1>/logo.png"            # or null
      video: null                               # LEGACY: always null (don't use)
      video_top: "assets/<sha1>/top.tgrv"       # or null
      video_bottom: "assets/<sha1>/bottom.tgrv" # or null
```

Paths are **relative** to the `manifest.yml` folder. A game can have missing assets
(`null`): degrade with a placeholder, don't crash.

## `assets_index.yml` schema (runtime name-based index)

```yaml
version: 1
roms:
  "<ROM-base-name-without-extension>": "<game_id>"
  ...
```

- One line per **file** on the SD, including duplicates: `"Game"` and `"Game - copy"`
  both point at the **same** `game_id`.
- Keys (names) are in **Unicode NFC** (e.g. an accented letter is precomposed). When
  comparing against a name read from the filesystem, normalize it to NFC before matching.

## TGR2 format (per-screen video file) -- little-endian

`.tgrv` is **raw** video (no codec): the "bitrate" is fixed at
`width*height*bytes_per_pixel*fps`, and the **SD card's read bandwidth** is what limits the
FPS on the DS. That's why screens are recorded at a **low resolution** (default 128x96,
hardware-upscaled to 256x192) and, by default, at **8bpp indexed** (half the data of
BGR555). The header carries `width`/`height`/`fmt` -- the reader must **respect them** (don't
assume a fixed 256x192/98304 bytes).

```
offset 0 : magic   'TGR2'  (4 ASCII bytes)
offset 4 : width   u16     (default 128)
offset 6 : height  u16     (default 96)
offset 8 : fps     u16     (default 12)
offset 10: nframes u32
offset 14: fmt     u8       (0 = BGR555 16bpp, 1 = PAL8 8bpp)
offset 15: flags   u8       (bit0 = pixels/palette already have bit15/opaque set)
offset 16: pal_cnt u16      (0 for BGR555; 256 for PAL8)
offset 18: palette pal_cnt x u16 BGR555 (bit15=1 opaque)   [PAL8 only]
offset ..: nframes x frame
  PAL8   (fmt=1): each frame = width*height bytes (1 index per pixel).
                  Load the palette (256 x u16 BGR555) into palette RAM once and use the
                  256-color bitmap BG mode; the index is blitted directly.
  BGR555 (fmt=0): each frame = width*height*2 bytes, u16 BGR555
                  (bits 0-4=R, 5-9=G, 10-14=B, bit15=1 opaque). Same layout as the DS's
                  16-bit framebuffer -> fread straight into u16[width*height].
Upscale from width x height to 256x192 on screen (affine BG does x2 in hardware with
128x96). No audio (silent preview).
```

## Matching a ROM to its asset (runtime)

1. Take the focused ROM's **base name** (no extension) and normalize it to **NFC**.
2. Look it up in `assets_index.yml` -> get the `game_id`.
3. Load `assets/<game_id>/logo.png`, `top.tgrv`, `bottom.tgrv` (handling missing ones).
   (Optional: cross-reference `manifest.yml` via `game_id` for extra metadata.)

**IMPORTANT:** the index already resolves duplicates -- renamed/duplicated ROMs map to the
same `game_id` and share the same assets. Don't try to match by content on the DS (the host
already did that by SHA1); just use the name-based index.

## Tasks

- Read `assets_index.yml` and `manifest.yml` from `<SD>/_nds/TWiLightMenu/dsimenu/`.
- Resolve, for the focused game, `logo` + `top.tgrv` + `bottom.tgrv` (handling `null`).
- Play `top.tgrv` on the top screen and `bottom.tgrv` on the bottom, looping, respecting
  the header's `fps` and `width`/`height`/`fmt`. PAL8: load the palette once and `fread`
  `width*height` bytes/frame into the 256-color bitmap BG. BGR555: `fread`
  `width*height*2` bytes/frame -> `u16[width*height]`. Upscale to 256x192 (affine BG x2
  with 128x96).
- Handle missing assets (`null` / missing file) with a fallback (e.g. logo only).
