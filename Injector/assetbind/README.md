# assetbind -- bind ROM <-> assets by content HASH

Layer that links each ROM to its assets (`logo`, `video`) through a `.yml` manifest,
using the ROM's **content identity** -- not the file name.

## Why hash and not name (identification decision)

The file name is fragile: it can be **renamed**, **duplicated**, and two different ROMs can
share the **same name**. The stable identity is the **content**. That's why the key is the hash,
with priority **`sha1` > `md5` > `crc32`+`size`**. The name is only used as an optional
*fallback* (`allow_name_match`, off by default). For DS the hash is of the **whole file**
(no header stripping).

Consequences (expected, not bugs):
- A renamed/duplicated ROM -> resolves to the same entry (same hash).
- N files with the same content -> share the same assets.
- Files with the same name but different content -> different entries, no collision.

## Folder layout

The host (generator) organizes the assets under a root folder, with **one subfolder per game**:

```
<root>/
  manifest.yml
  assets/                       # assets root folder
    <game_id>/                  # one folder per game (game_id = the ROM's sha1)
      logo.png
      video.<ext>
```

Paths in the `.yml` are **relative to the manifest root** (never absolute machine paths).

## `.yml` schema

```yaml
version: 1
# allow_name_match: true        # optional (off by default): enables the name fallback at runtime
games:
  - game_id: "<stable id (the ROM's sha1, or a ScreenScraper id)>"
    identity:                   # KEY = content (sha1 > md5 > crc32+size)
      sha1: "<hex>"
      md5:  "<hex>"
      crc32: "<hex>"
      size: <bytes>
    rom_name: "Cool Game (USA).nds"   # informational only / human fallback
    assets:
      logo:  "assets/<game_id>/logo.png"       # or null if missing
      video: null                              # stacked mp4 (not shipped to the SD)
      video_top:    "assets/<game_id>/top.tgrv"    # top screen TGR2 128x96 PAL8 (DS)
      video_bottom: "assets/<game_id>/bottom.tgrv" # bottom screen TGR2 128x96 PAL8 (DS)
```

`fetch_ds_media.py` already splits the stacked video from ScreenScraper into two `.tgrv`
files (via `../split_ds_video.py`): `top.tgrv` (top screen) and `bottom.tgrv` (bottom
screen), each in TGR2 128x96 8bpp indexed format (a 256-color palette per video), ready for
the modified TWiLightMenu. See the format in `../docs/asset-structure-changes.md`.
`deploy.py`/`scan_and_bind.py` move those `.tgrv` files into `assets/<game_id>/` and register
them in the manifest. The intermediate `mp4` is not copied to the SD.

See `manifest.example.yml`.

## Components

- **`scan_and_bind.py`** -- the SD card orchestrator. **Only READS the SD during the scan**;
  all the download/processing (via `../fetch_ds_media.py`, video splitting, logo downscaling)
  happens in a **local working folder** (`--cache`, default `../cache`). Generates
  `manifest.yml` + `assets_index.yml` in the cache and, at the end, **builds the folders on
  the SD and moves** the content to the destination (`--out`). Filters system apps
  (blocklist), dedups by hash, is idempotent (skips whatever is already on the SD), and shows
  a **live progress bar** (`--no-progress` for plain one-line-per-file output, e.g. when
  piping to a log file). Re-scanning an SD that's already fully set up is cheap: ROM hashes
  are cached by (path, size, mtime) in `<cache>/rom_hash_cache.json`, so unchanged ROMs are
  never re-read, and games that already have the assets a given pass cares about (logo for a
  logo-only pass, video for a full pass) are never re-downloaded -- only genuinely new ROMs
  get hashed and fetched. Whatever's left to fetch is scraped in **batches** of
  `SCRAPE_BATCH_SIZE` ROMs (default 15) per Skyscraper invocation instead of one invocation
  per ROM: Skyscraper accepts several filenames in a single call and, given more than one,
  uses its own internal multi-threaded scraping (`-t`, 4 threads by default) -- a "one ROM
  per process" loop can never benefit from that, and pays a full process/cache-session
  startup for every single game on top.
- **`system_blocklist.txt`** -- `.nds` files that are NOT games (dsimenu, pictochat,
  nds-bootstrap, GBARunner2, ...). Excludes by `sha1:` and/or `name:` -- the hash is robust to
  renaming.
- **`generate_manifest.py`** -- the standalone "app": scans the ROMs + the Skyscraper media
  folder, computes the hashes, **copies/organizes** the assets into `assets/<game_id>/`, and
  writes `manifest.yml`.
- **`rom_binder.py`** -- runtime bind: `load_manifest(path)` -> `Binder`; `binder.bind(rom_path)`
  returns `{game_id, logo, video, matched_by}` (absolute paths, validated on disk) or `None`.
- **`rom_hash.py`** -- content hash (sha1/md5/crc32/size) in a single pass.
- **`yaml_io.py`** -- `.yml` I/O (uses PyYAML if installed; otherwise a purpose-built reader/writer for this schema).
- **`test_binder.py`** -- tests (unittest).

## Usage

### Scan the SD card, download, and bind (the SD feature)

Scans the SD, ignores system apps (blocklist), downloads the logo+video for each game,
organizes them into `assets/<sha1>/` **inside the SD**, and writes `manifest.yml`. Duplicates
(`... - copy.nds`) collapse by hash into one entry and share the assets.

Most users should run this through `../deploy.py` instead (it also handles finding the SD
card and installing `dsimenu.srldr`). Calling it directly is only needed for scripting /
advanced use:

```bash
export SS_USER='...'; export SS_PASS='...'   # or let fetch_ds_media.py prompt/use its cache
python3 scan_and_bind.py --sd "/path/to/SD_CARD"
# options:
#   --out <dir>          output root (default: the SD root itself)
#   --no-download        only (re)organize existing assets and rewrite the manifest
#   --force               re-download even if assets/<sha1>/ already exists
#   --blocklist <file>    custom blocklist (default: system_blocklist.txt)
```

### Generate the manifest (host)

Skyscraper downloads, per ROM, `"<base>-logo.png"` and `"<base>-video.<ext>"`. Then:

```bash
python3 generate_manifest.py \
  --roms  "/path/to/roms/nds" \
  --media "/path/to/skyscraper/media" \
  --out   "/path/to/output"
# options: --rom-ext .nds  --logo-suffix -logo.png  --video-suffix -video  --allow-name-match
```

Generates `/path/to/output/manifest.yml` + `/path/to/output/assets/<game_id>/...`.

### Bind at runtime

```python
from rom_binder import load_manifest
binder = load_manifest("/path/to/output/manifest.yml")
res = binder.bind("/sd/roms/Some ROM.nds")
if res:
    print(res.logo, res.video, "via", res.matched_by)
else:
    print("no art yet (scrape it later)")
```

### Tests

```bash
python3 test_binder.py          # or: python3 -m unittest -v
```
Covers: (a) a renamed file resolves; (b) duplicates share art; (c) same names + different
hashes don't collide; (d) a missing asset degrades with a warning; + duplicate hash = invalid
manifest; + YAML round-trip.

## Error handling
- Asset missing on disk (but referenced): binds whatever exists, logs a warning, doesn't break.
- ROM with no entry: returns `None` and logs the `sha1` (to scrape later).
- Same hash in two `.yml` entries: `ManifestError` naming the conflicting `game_id`s.

## Note on DS integration (the bridge)

The DS **does not hash at runtime** (a 67 MHz ARM9 can't hash tens/hundreds of MB of ROM in
time). This manifest lives on the **host**: it is the robust source of truth. For the DS
frontend to consume it, the host resolves each ROM on the cartridge by hash and emits a light
index the DS can match cheaply (`assets_index.yml`, written directly by `scan_and_bind.py`).
