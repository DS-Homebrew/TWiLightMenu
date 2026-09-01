"""
rom_binder.py -- binds ROMs to their assets ({logo, video}) by content HASH.

IDENTIFICATION KEY (non-negotiable): priority sha1 > md5 > crc32(+size). The file name is
NEVER the primary key -- it is only used as a fallback when allow_name_match=True (off by
default), because a name is ambiguous (rename/duplicate/two games sharing the same name).

Usage:
    from rom_binder import load_manifest
    binder = load_manifest("path/manifest.yml")
    res = binder.bind("/sd/roms/Some ROM.nds")
    if res:
        print(res.logo, res.video, res.matched_by)
"""
import logging
import os
from dataclasses import dataclass
from typing import Optional

from rom_hash import hash_file
from yaml_io import load_manifest_text

log = logging.getLogger("assetbind")


class ManifestError(Exception):
    """Invalid manifest (e.g. the same hash appears in two different entries)."""


@dataclass
class BindResult:
    game_id: str
    logo: Optional[str]           # absolute path to the logo (or None if missing)
    video: Optional[str]          # stacked mp4 (optional; not used by the DS)
    matched_by: str               # 'sha1' | 'md5' | 'crc32+size' | 'name'
    video_top: Optional[str] = None     # top screen .tgrv (DS)
    video_bottom: Optional[str] = None  # bottom screen .tgrv (DS)


class Binder:
    def __init__(self, manifest: dict, root: str):
        # root: base directory used to resolve relative asset paths -> absolute paths.
        self.root = os.path.abspath(root)
        self.games = manifest.get("games", []) or []
        self.allow_name_match_default = bool(manifest.get("allow_name_match", False))

        # O(1) indexes. The same hash appearing in TWO different entries = invalid manifest.
        self._sha1, self._md5, self._crcsize = {}, {}, {}
        self._name = {}  # name is only a fallback; duplicate names are tolerated (first wins)

        for g in self.games:
            idn = g.get("identity", {}) or {}
            self._index(self._sha1, idn.get("sha1"), g, "sha1")
            self._index(self._md5, idn.get("md5"), g, "md5")
            crc, size = idn.get("crc32"), idn.get("size")
            if crc and size is not None:
                self._index(self._crcsize, f"{str(crc).lower()}:{size}", g, "crc32+size")
            if g.get("rom_name"):
                self._name.setdefault(str(g["rom_name"]).lower(), g)

    def _index(self, idx, key, g, label):
        if not key:
            return
        k = str(key).lower()
        if k in idx and idx[k] is not g:
            raise ManifestError(
                f"duplicate {label} hash ({k}) between game_id "
                f"'{idx[k].get('game_id')}' and '{g.get('game_id')}'"
            )
        idx[k] = g

    # -------- asset resolution --------

    def _resolve(self, rel):
        """Path relative to the manifest root -> absolute; None if it doesn't exist on disk."""
        if not rel:
            return None
        p = rel if os.path.isabs(rel) else os.path.join(self.root, rel)
        return p if os.path.isfile(p) else None

    def _result(self, g, matched):
        a = g.get("assets", {}) or {}
        gid = g.get("game_id")

        def resolve_warn(key):
            ref = a.get(key)
            path = self._resolve(ref)
            # Asset referenced in the manifest but missing on disk: warn, don't break.
            if ref and path is None:
                log.warning("game %s: %s referenced but missing: %s", gid, key, ref)
            return path

        return BindResult(
            game_id=gid,
            logo=resolve_warn("logo"),
            video=resolve_warn("video"),
            video_top=resolve_warn("video_top"),
            video_bottom=resolve_warn("video_bottom"),
            matched_by=matched,
        )

    # -------- bind --------

    def bind(self, rom_path: str, allow_name_match: Optional[bool] = None) -> Optional[BindResult]:
        """Given a ROM path, returns a BindResult or None (no match)."""
        h = hash_file(rom_path)

        g = self._sha1.get(h["sha1"])
        if g is not None:
            return self._result(g, "sha1")
        g = self._md5.get(h["md5"])
        if g is not None:
            return self._result(g, "md5")
        g = self._crcsize.get(f"{h['crc32']}:{h['size']}")
        if g is not None:
            return self._result(g, "crc32+size")

        allow = self.allow_name_match_default if allow_name_match is None else allow_name_match
        if allow:
            g = self._name.get(os.path.basename(rom_path).lower())
            if g is not None:
                return self._result(g, "name")

        # No entry: leave it without art (placeholder) and log the sha1 to scrape later.
        log.warning("ROM has no manifest entry (sha1=%s): %s", h["sha1"], rom_path)
        return None


def load_manifest(path: str) -> Binder:
    """Loads the manifest and returns a Binder. Asset paths are relative to the manifest."""
    with open(path, "r", encoding="utf-8") as f:
        manifest = load_manifest_text(f.read())
    if not isinstance(manifest, dict) or "games" not in manifest:
        raise ManifestError(f"invalid manifest: {path}")
    return Binder(manifest, root=os.path.dirname(os.path.abspath(path)))
