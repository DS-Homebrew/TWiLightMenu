#!/usr/bin/env python3
"""
refresh_logos.py -- re-downloads ONLY the LOGO for each game on the SD and REPLACES the old
logo.png under assets/<sha1>/, leaving videos (.tgrv) and manifest.yml untouched.

What it's for: after changing how logos are processed, refresh the logos already written to
the SD without re-downloading the videos or rewriting the manifest -- a cheap, surgical pass.
Unlike scan_and_bind.py, this always re-fetches every game's logo (that's the point of a
"refresh"); use --only-existing to at least avoid creating brand new entries.

How it works (reuses scan_and_bind.py):
  1) scans the SD for ROMs (.nds), skips the system blocklist and groups by sha1
     (the same content identity used for binding -- duplicates collapse). Hashing reuses
     scan_and_bind.py's hash cache, so re-running this doesn't re-read every ROM's content;
  2) for each game, calls fetch_ds_media.py with LOGO_ONLY=1 (downloads only the logo,
     no video) into a temporary directory;
  3) copies the logo over <out>/assets/<sha1>/logo.png (creating the folder if it doesn't
     exist). NOTHING besides logo.png is touched.

ScreenScraper credentials: read by fetch_ds_media.py itself (env SS_USER/SS_PASS, the
.ss_credentials.json cache, or an interactive prompt).
"""
import argparse
import os
import shutil
import sys
import tempfile

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
# Reuses the already-tested scan/hash/fetch pipeline.
from scan_and_bind import (                       # noqa: E402
    find_roms, load_blocklist, name_blocked, fetch_media, ProgressBar,
    load_hash_cache, save_hash_cache, hash_file_cached,
    DEFAULT_FETCH, DEFAULT_BLOCKLIST, DEFAULT_CACHE,
)

DEFAULT_OUT_SUB = os.path.join("_nds", "TWiLightMenu", "dsimenu")


def main(argv):
    ap = argparse.ArgumentParser(
        description="Re-downloads only the logo for each game on the SD, replacing the old one.")
    ap.add_argument("--sd", required=True, help="SD card root to scan")
    ap.add_argument("--out", default=None,
                    help="where assets/ lives (default: <SD>/_nds/TWiLightMenu/dsimenu)")
    ap.add_argument("--cache", default=DEFAULT_CACHE,
                    help="working folder holding the ROM hash cache (shared with scan_and_bind.py)")
    ap.add_argument("--fetch-script", default=DEFAULT_FETCH,
                    help="path to fetch_ds_media.py")
    ap.add_argument("--blocklist", default=DEFAULT_BLOCKLIST,
                    help="system app blocklist")
    ap.add_argument("--rom-ext", default=".nds", help="ROM extensions (csv). Default .nds")
    ap.add_argument("--only-existing", action="store_true",
                    help="only update games that ALREADY have assets/<sha1>/ (don't create new ones).")
    ap.add_argument("--dry-run", action="store_true",
                    help="list what would be done, without downloading or writing anything.")
    ap.add_argument("--no-progress", action="store_true",
                    help="print one line per game instead of a live progress bar")
    args = ap.parse_args(argv)

    if not os.path.isdir(args.sd):
        sys.stderr.write(f"!! SD not found: {args.sd}\n")
        return 1
    out = args.out or os.path.join(args.sd, DEFAULT_OUT_SUB)
    out_assets = os.path.join(out, "assets")
    cache = os.path.abspath(args.cache)
    if not args.dry_run and not os.path.isfile(args.fetch_script):
        sys.stderr.write(f"!! fetch script not found: {args.fetch_script}\n")
        return 1

    exts = {e if e.startswith(".") else "." + e
            for e in (x.strip().lower() for x in args.rom_ext.split(",")) if e}
    sha1_block, name_block = load_blocklist(args.blocklist)

    roms = find_roms(args.sd, exts)
    if not roms:
        sys.stderr.write(f"!! no ROM {sorted(exts)} in {args.sd}\n")
        return 1

    # Groups by sha1 (same as scan_and_bind): skips entries blocked by name and by hash.
    # Hashing is cached by (path, size, mtime) so a re-run doesn't re-read every ROM.
    hash_cache = load_hash_cache(cache)
    hash_cache_dirty = False
    by_sha1 = {}
    scan_bar = ProgressBar(len(roms), prefix="Scanning ", enabled=not args.no_progress)
    for i, path in enumerate(roms, 1):
        scan_bar.update(i, os.path.basename(path))
        if name_blocked(path, name_block):
            continue
        h, was_cached = hash_file_cached(path, hash_cache)
        hash_cache_dirty = hash_cache_dirty or not was_cached
        if h["sha1"].lower() in sha1_block:
            continue
        by_sha1.setdefault(h["sha1"], []).append(path)
    scan_bar.finish()
    if hash_cache_dirty:
        save_hash_cache(cache, hash_cache)

    total = len(by_sha1)
    print(f">> {len(roms)} files, {total} unique games. Destination: {out_assets}",
          file=sys.stderr)

    # Forces logo-only mode on the fetch (no video).
    os.environ["LOGO_ONLY"] = "1"

    replaced = skipped = failed = 0
    bar = ProgressBar(total, prefix="Refreshing logos ", enabled=not args.no_progress)
    for i, (sha1, files) in enumerate(sorted(by_sha1.items()), 1):
        primary = files[0]
        game_dir = os.path.join(out_assets, sha1)
        dst_logo = os.path.join(game_dir, "logo.png")
        had = os.path.isfile(dst_logo)
        bar.update(i, os.path.basename(primary))
        tag = f"[{i}/{total}] {sha1[:12]}... {os.path.basename(primary)}"

        if args.only_existing and not had:
            bar.note(f">> {tag} -- no assets/ yet; skipped (--only-existing).")
            skipped += 1
            continue
        if args.dry_run:
            bar.note(f">> {tag} -- would {'replace' if had else 'create'} {dst_logo}")
            continue

        try:
            with tempfile.TemporaryDirectory(prefix="refresh_logo_") as tmp:
                src = fetch_media(args.fetch_script, primary, tmp)
                new_logo = src.get("logo")
                if not new_logo or not os.path.isfile(new_logo):
                    bar.note(f">> {tag} -- logo not found on ScreenScraper; kept as-is.")
                    skipped += 1
                    continue
                os.makedirs(game_dir, exist_ok=True)
                shutil.copyfile(new_logo, dst_logo)
            replaced += 1
        except Exception as e:                       # noqa: BLE001 (log and continue)
            bar.note(f"!! {tag} -- failed: {e}")
            failed += 1
    bar.finish()

    if args.dry_run:
        print(f">> dry-run: {total} game(s) evaluated (nothing changed).", file=sys.stderr)
    else:
        print(f">> Done: {replaced} logo(s) updated, {skipped} skipped, "
              f"{failed} failure(s).", file=sys.stderr)
    return 0 if failed == 0 or replaced else 1


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
