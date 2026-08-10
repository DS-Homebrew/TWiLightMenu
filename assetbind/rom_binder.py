"""
rom_binder.py — vincula ROMs aos seus assets ({logo, video}) pelo HASH do conteúdo.

CHAVE DE IDENTIFICAÇÃO (não negociável): prioridade sha1 > md5 > crc32(+size). O nome do
arquivo NUNCA é chave primária — só é usado como fallback se allow_name_match=True (padrão
desligado), porque nome é ambíguo (renome/duplicata/nomes iguais para jogos diferentes).

Uso:
    from rom_binder import load_manifest
    binder = load_manifest("caminho/manifest.yml")
    res = binder.bind("/sd/roms/Alguma ROM.nds")
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
    """Manifesto inválido (ex.: mesmo hash em duas entradas diferentes)."""


@dataclass
class BindResult:
    game_id: str
    logo: Optional[str]      # caminho absoluto do logo (ou None se ausente)
    video: Optional[str]     # caminho absoluto do vídeo (ou None se ausente)
    matched_by: str          # 'sha1' | 'md5' | 'crc32+size' | 'name'


class Binder:
    def __init__(self, manifest: dict, root: str):
        # root: diretório-base para resolver caminhos de asset relativos -> absolutos.
        self.root = os.path.abspath(root)
        self.games = manifest.get("games", []) or []
        self.allow_name_match_default = bool(manifest.get("allow_name_match", False))

        # Índices O(1). Um mesmo hash em DUAS entradas diferentes = manifesto inválido.
        self._sha1, self._md5, self._crcsize = {}, {}, {}
        self._name = {}  # nome é só fallback; duplicatas de nome toleradas (primeiro vence)

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
                f"hash {label} duplicado ({k}) entre game_id "
                f"'{idx[k].get('game_id')}' e '{g.get('game_id')}'"
            )
        idx[k] = g

    # -------- resolução de asset --------

    def _resolve(self, rel):
        """Caminho relativo à raiz do manifesto -> absoluto; None se não existir em disco."""
        if not rel:
            return None
        p = rel if os.path.isabs(rel) else os.path.join(self.root, rel)
        return p if os.path.isfile(p) else None

    def _result(self, g, matched):
        a = g.get("assets", {}) or {}
        logo = self._resolve(a.get("logo"))
        video = self._resolve(a.get("video"))
        gid = g.get("game_id")
        # Asset referenciado no manifesto mas ausente em disco: avisa, não quebra.
        if a.get("logo") and logo is None:
            log.warning("game %s: logo referenciado mas ausente: %s", gid, a.get("logo"))
        if a.get("video") and video is None:
            log.warning("game %s: video referenciado mas ausente: %s", gid, a.get("video"))
        return BindResult(game_id=gid, logo=logo, video=video, matched_by=matched)

    # -------- bind --------

    def bind(self, rom_path: str, allow_name_match: Optional[bool] = None) -> Optional[BindResult]:
        """Dado o caminho de uma ROM, devolve BindResult ou None (sem correspondência)."""
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

        # Sem entrada: deixa sem arte (placeholder) e loga o sha1 p/ scrapear depois.
        log.warning("ROM sem entrada no manifesto (sha1=%s): %s", h["sha1"], rom_path)
        return None


def load_manifest(path: str) -> Binder:
    """Carrega o manifesto e devolve um Binder. Caminhos de asset são relativos ao manifesto."""
    with open(path, "r", encoding="utf-8") as f:
        manifest = load_manifest_text(f.read())
    if not isinstance(manifest, dict) or "games" not in manifest:
        raise ManifestError(f"manifesto inválido: {path}")
    return Binder(manifest, root=os.path.dirname(os.path.abspath(path)))
