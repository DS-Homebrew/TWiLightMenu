# TWiLightMenuGRID deploy tools

Install `dsimenu.srldr` on your DS/DSi SD card and scrape game logos + gameplay clips from
ScreenScraper, with a single interactive script. Pure Python -- no bash, WSL or Git Bash
needed, works the same on Windows, macOS and Linux.

## Quick start

1. Install **Python 3.8+** ([python.org](https://www.python.org/downloads/) -- on Windows,
   tick "Add python.exe to PATH" during install). That's the only thing you install by hand.
2. (Only if you also want to scrape art) Create a free account at
   [screenscraper.fr](https://www.screenscraper.fr/) -- you'll be asked for the
   username/password once; they're cached locally afterwards.
3. Plug in your DS/DSi SD card.
4. Run the tool:
   - **Windows:** double-click `Start Deploy.bat` (or run `python deploy.py` from a terminal).
   - **macOS:** double-click `Start Deploy.command` (or run `python3 deploy.py`).
   - **Linux:** run `python3 deploy.py` from a terminal.

You do **not** need to install ffmpeg or Skyscraper yourself:
- **ffmpeg** is downloaded automatically the first time it's needed (an official portable
  build for your OS, ~25-110 MB) -- no admin rights, nothing added to your system PATH.
- **Skyscraper** has no ready-to-run download from its own project (only source code), so
  the tool clones and compiles it for you automatically on macOS/Linux, into its own local
  folder -- **on macOS this needs no admin password at all** (Qt5 is fetched via Homebrew,
  which doesn't need root). On Linux, only the one-time Qt5 system package needs `sudo`
  (via apt/dnf/pacman, whichever you have). On Windows there is currently no way around a
  manual build -- if that's you, the tool prints the exact steps when it gets to that point.

The script will:
1. **Locate** your SD card automatically (or let you pick/type it).
2. **Ask** if the menu is already installed on this SD. Say yes and it skips straight to
   step 5 (scraping) -- nothing on the SD's `_nds/TWiLightMenu/` gets touched. Useful for
   re-running the tool later just to grab art for newly added games. (Non-interactive
   equivalent: `--scrape-only`.)
3. **Install `dsimenu.srldr`** onto it (backing up the previous one), if you placed a built
   copy at `build/dsimenu.srldr` next to this script.
4. **Install and select the default DSi theme**: any folder sitting next to this script
   that directly contains a `theme.ini` (e.g. `Default grid theme/`) is copied onto the SD
   under `dsimenu/themes/` and set as the active theme by writing `DSI_THEME` (and
   `THEME = 0`, DSi mode) into `_nds/TWiLightMenu/settings.ini` -- the same file and keys
   TWiLightMenu itself reads/writes. The previous `settings.ini` is backed up as
   `settings.ini.bak` first, and every other line/setting already in it is left untouched.
5. **Ask** whether to scrape game **logos** now (quick).
6. **Ask** whether to scrape gameplay **clips** now (slower -- downloads and converts video).

You can re-run it any time; already-downloaded assets are never re-fetched unless you pass
`--force`. Run `python3 deploy.py --help` for all the flags (`--sd`, `--yes`, `--scrape-only`,
`--no-logos`, `--no-clips`, `--default-theme`, `--no-default-theme`, `--force`,
`--no-auto-download`).

## What ends up on the SD card

```
<SD>/_nds/TWiLightMenu/
├── settings.ini                 # DSI_THEME + THEME=0 are set here (rest is untouched)
├── dsimenu.srldr                # the menu itself
└── dsimenu/
    ├── manifest.yml             # game_id -> identity + assets
    ├── assets_index.yml         # ROM name -> game_id (what the DS reads at runtime)
    ├── themes/
    │   └── Default grid theme/  # (or whichever bundled theme(s) shipped with this package)
    └── assets/
        └── <sha1>/              # one folder per game, keyed by ROM content hash
            ├── logo.png
            ├── top.tgrv         # top screen gameplay clip
            └── bottom.tgrv      # bottom screen gameplay clip
```

Games are identified by the **hash of the ROM file**, not its name -- renamed or duplicated
ROMs automatically resolve to the same entry and share the same art. See
[`docs/asset-structure-changes.md`](docs/asset-structure-changes.md) for the full format,
and [`assetbind/README.md`](assetbind/README.md) for the binding internals.

## Advanced / scripted usage

Each stage is also a standalone script if you want more control than `deploy.py`'s prompts
give you:

| Script | Purpose |
| --- | --- |
| `deploy.py` | The interactive all-in-one tool described above. |
| `assetbind/scan_and_bind.py` | Scans an SD, downloads logo+video for every game, writes the manifest. `--list-only` to preview, `--no-download` to just rebind existing assets, `--force` to re-download. Fast on repeat runs: ROM hashes are cached, and only games actually missing something get fetched, in batches (see `SCRAPE_BATCH_SIZE` below) rather than one Skyscraper call per ROM. |
| `assetbind/refresh_logos.py` | Re-downloads only the logo for every game already on the SD (leaves videos/manifest untouched). |
| `fetch_ds_media.py <rom.nds> [dest]` | Fetches logo+video for a single ROM. `--batch-file <list.txt> --results-file <out.json> [dest]` fetches many ROMs (one per line in the list) in as few Skyscraper invocations as possible -- what `scan_and_bind.py` uses internally. |
| `split_ds_video.py <video.mp4> [outdir]` | Splits a stacked ScreenScraper video into the two `.tgrv` files the DS reads. |
| `dither_topbg.py <SD> [--intensity N] [--pattern NAME]` | (Optional) regenerates each theme's `topbg_dither.png` overlay. `--list-patterns` shows the available patterns. |

Environment variables (all optional) that tune the scrape: `SS_USER` / `SS_PASS`
(ScreenScraper credentials, otherwise cached/prompted), `SKYSCRAPER_BIN`, `FFMPEG_BIN` (set
automatically by `deploy.py` when it downloads a portable copy), `SCRAPE_BATCH_SIZE` (ROMs
per Skyscraper invocation during a scrape, default 15 -- higher means fewer, larger batches;
lower it if a big batch call fails outright, since a whole batch shares one gather/generate
pair), `LOGO_ONLY`, `SPLIT_VIDEO`, `TGRV_FPS`, `TGRV_WIDTH`, `TGRV_HEIGHT`, `TGRV_MODE`,
`TGRV_MAX_SECONDS`, `KEEP_MP4`, `LOGO_DOWNSCALE`, `LOGO_MAX_WIDTH`, `LOGO_MAX_HEIGHT`. See
each script's `--help` for details.

## Troubleshooting

- **"ffmpeg is not available" after a download attempt** -- usually a network issue, or your
  OS/CPU combination has no known portable build (see `FFMPEG_BUILD_URLS` in `deploy.py`).
  Install it yourself from [ffmpeg.org](https://ffmpeg.org/download.html), put it on PATH,
  and run the tool again.
- **"Skyscraper is not available"** -- on macOS/Linux, make sure `git` is installed and that
  you're running the tool from a real terminal (the automatic build is skipped entirely for
  non-interactive/piped runs, since on Linux it needs to ask for a `sudo` password for the
  Qt5 package -- macOS doesn't need a password at all). If a build was attempted and failed,
  the error above it has the actual reason (missing Homebrew, missing package manager, a
  compile error, etc.). On Windows, follow the manual build steps the tool prints (there is
  no prebuilt binary).
- **"ScreenScraper rejected the request"** -- your credentials are wrong, or you've hit the
  API's rate limit for unregistered/free accounts; wait a bit and try again. Delete
  `.ss_credentials.json` next to the scripts to re-enter your credentials.
- **No SD card detected** -- make sure it's mounted (has a `_nds` folder from a prior
  TWiLightMenu install), or just type its path/drive letter when asked.
- **Nothing found for a game** -- not every ROM has art on ScreenScraper; it's skipped, with
  a warning, and simply won't have a logo/clip on the menu.
