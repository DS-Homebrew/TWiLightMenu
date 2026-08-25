#!/usr/bin/env python3
"""
Testes do binder (hash-based). Rodar: python3 -m unittest -v  (ou python3 test_binder.py)

Cobre:
  (a) arquivo renomeado resolve certo (mesmo conteúdo, nome diferente);
  (b) dois arquivos = mesmo jogo compartilham arte;
  (c) nomes iguais + hashes diferentes NÃO colidem;
  (d) asset faltante degrada com aviso;
  + manifesto inválido (mesmo hash em duas entradas) => erro;
  + round-trip do YAML próprio.
"""
import logging
import os
import sys
import tempfile
import unittest

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from rom_hash import hash_file           # noqa: E402
from yaml_io import (dump_manifest, load_manifest_text,               # noqa: E402
                     dump_assets_index, load_assets_index_text)
from rom_binder import Binder, ManifestError            # noqa: E402


def write(path, data: bytes):
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "wb") as f:
        f.write(data)
    return path


def entry_for(root, rom_path, game_id, with_logo=True, with_video=True, rom_name=None):
    """Cria uma entrada de manifesto p/ uma ROM, gravando os assets em assets/<game_id>/."""
    h = hash_file(rom_path)
    logo = video = None
    if with_logo:
        logo_rel = os.path.join("assets", game_id, "logo.png")
        write(os.path.join(root, logo_rel), b"PNG-logo")
        logo = logo_rel
    if with_video:
        video_rel = os.path.join("assets", game_id, "video.mp4")
        write(os.path.join(root, video_rel), b"MP4-video")
        video = video_rel
    return {
        "game_id": game_id,
        "identity": h,
        "rom_name": rom_name or os.path.basename(rom_path),
        "assets": {"logo": logo, "video": video},
    }


class BinderTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.mkdtemp()
        self.root = self.tmp  # raiz do manifesto (resolve assets relativos)

    def _binder(self, games, allow_name_match=False):
        m = {"version": 1, "games": games}
        if allow_name_match:
            m["allow_name_match"] = True
        return Binder(m, self.root)

    # (a) renomeado resolve certo
    def test_renamed_resolves(self):
        content = b"MARIOKART-CONTENT" * 100
        rom = write(os.path.join(self.tmp, "roms", "Mario Kart DS (USA).nds"), content)
        game = entry_for(self.root, rom, "gid-mkds")
        binder = self._binder([game])

        renamed = write(os.path.join(self.tmp, "roms", "totally-different-name.nds"), content)
        res = binder.bind(renamed)
        self.assertIsNotNone(res)
        self.assertEqual(res.game_id, "gid-mkds")
        self.assertEqual(res.matched_by, "sha1")
        self.assertTrue(res.logo and os.path.isfile(res.logo))

    # (b) dois arquivos = mesmo jogo compartilham arte
    def test_duplicates_share_art(self):
        content = b"SAME-GAME" * 200
        a = write(os.path.join(self.tmp, "roms", "copy A.nds"), content)
        b = write(os.path.join(self.tmp, "roms", "copy B.nds"), content)
        binder = self._binder([entry_for(self.root, a, "gid-1")])
        ra, rb = binder.bind(a), binder.bind(b)
        self.assertEqual(ra.game_id, rb.game_id)
        self.assertEqual(ra.logo, rb.logo)
        self.assertEqual(ra.video, rb.video)

    # (c) nomes iguais + hashes diferentes NÃO colidem
    def test_same_name_diff_content(self):
        r1 = write(os.path.join(self.tmp, "d1", "game.nds"), b"CONTENT-ONE" * 50)
        r2 = write(os.path.join(self.tmp, "d2", "game.nds"), b"CONTENT-TWO" * 50)
        binder = self._binder([
            entry_for(self.root, r1, "gid-one"),
            entry_for(self.root, r2, "gid-two"),
        ])
        self.assertEqual(binder.bind(r1).game_id, "gid-one")
        self.assertEqual(binder.bind(r2).game_id, "gid-two")

    # (d) asset faltante degrada com aviso
    def test_missing_asset_degrades(self):
        rom = write(os.path.join(self.tmp, "roms", "nolo.nds"), b"NOLOGO" * 30)
        game = entry_for(self.root, rom, "gid-nolo", with_logo=True, with_video=True)
        # apaga o logo do disco (referenciado no manifesto, mas ausente)
        os.remove(os.path.join(self.root, game["assets"]["logo"]))
        binder = self._binder([game])
        with self.assertLogs("assetbind", level="WARNING") as cm:
            res = binder.bind(rom)
        self.assertIsNone(res.logo)                 # ausente -> None
        self.assertIsNotNone(res.video)             # presente -> vinculado
        self.assertTrue(any("logo" in m for m in cm.output))

    # ROM sem entrada -> None + aviso
    def test_no_entry_returns_none(self):
        rom = write(os.path.join(self.tmp, "roms", "unknown.nds"), b"UNKNOWN" * 10)
        binder = self._binder([])
        with self.assertLogs("assetbind", level="WARNING"):
            self.assertIsNone(binder.bind(rom))

    # fallback por nome só quando habilitado
    def test_name_fallback_flag(self):
        rom = write(os.path.join(self.tmp, "roms", "ByName.nds"), b"BYNAME" * 10)
        game = entry_for(self.root, rom, "gid-name", rom_name="ByName.nds")
        # zera a identity p/ forçar cair no fallback de nome
        game["identity"] = {"sha1": "", "md5": "", "crc32": "", "size": 0}
        binder_off = self._binder([game], allow_name_match=False)
        self.assertIsNone(binder_off.bind(rom))                 # desligado -> não casa
        binder_on = self._binder([game], allow_name_match=True)
        self.assertEqual(binder_on.bind(rom).matched_by, "name")  # ligado -> casa por nome

    # manifesto inválido: mesmo hash em duas entradas
    def test_duplicate_hash_invalid(self):
        rom = write(os.path.join(self.tmp, "roms", "dup.nds"), b"DUP" * 40)
        g1 = entry_for(self.root, rom, "gid-A")
        g2 = entry_for(self.root, rom, "gid-B")  # mesmo conteúdo, game_id diferente
        with self.assertRaises(ManifestError):
            self._binder([g1, g2])

    # assets .tgrv (top/bottom) são bindados e sobrevivem ao round-trip do YAML
    def test_tgrv_assets_bind(self):
        rom = write(os.path.join(self.tmp, "roms", "tg.nds"), b"TGRV-GAME" * 40)
        h = hash_file(rom)
        top_rel = os.path.join("assets", "gid-tg", "top.tgrv")
        bot_rel = os.path.join("assets", "gid-tg", "bottom.tgrv")
        write(os.path.join(self.root, top_rel), b"TGRV-top")
        write(os.path.join(self.root, bot_rel), b"TGRV-bottom")
        game = {
            "game_id": "gid-tg", "identity": h, "rom_name": "tg.nds",
            "assets": {"logo": None, "video": None,
                       "video_top": top_rel, "video_bottom": bot_rel},
        }
        # bind resolve os dois tgrv para caminhos absolutos existentes
        res = self._binder([game]).bind(rom)
        self.assertTrue(res.video_top and os.path.isfile(res.video_top))
        self.assertTrue(res.video_bottom and os.path.isfile(res.video_bottom))
        self.assertIsNone(res.video)
        # round-trip: os campos tgrv persistem no YAML emitido
        loaded = load_manifest_text(dump_manifest({"version": 1, "games": [game]}))
        a = loaded["games"][0]["assets"]
        self.assertTrue(a["video_top"].endswith("top.tgrv"))
        self.assertTrue(a["video_bottom"].endswith("bottom.tgrv"))

    # assets_index.yml: nomes (com espaços/duplicatas) -> game_id, e volta certo
    def test_assets_index(self):
        roms = {
            "Mario Kart DS (USA) (En,Fr,De,Es,It)": "gid-mk",
            "Mario Kart DS (USA) (En,Fr,De,Es,It) - cópia": "gid-mk",  # duplicata -> mesmo id
            "Some Other Game (USA)": "gid-so",
        }
        text = dump_assets_index(roms)
        loaded = load_assets_index_text(text)
        self.assertEqual(loaded["version"], 1)
        self.assertEqual(loaded["roms"]["Mario Kart DS (USA) (En,Fr,De,Es,It)"], "gid-mk")
        self.assertEqual(loaded["roms"]["Mario Kart DS (USA) (En,Fr,De,Es,It) - cópia"], "gid-mk")
        self.assertEqual(loaded["roms"]["Some Other Game (USA)"], "gid-so")

    # round-trip do YAML próprio
    def test_yaml_roundtrip(self):
        rom = write(os.path.join(self.tmp, "roms", "rt.nds"), b"RT" * 100)
        m = {"version": 1, "allow_name_match": True,
             "games": [entry_for(self.root, rom, "gid-rt")]}
        loaded = load_manifest_text(dump_manifest(m))
        self.assertEqual(loaded["version"], 1)
        self.assertTrue(loaded["allow_name_match"])
        g = loaded["games"][0]
        self.assertEqual(g["game_id"], "gid-rt")
        self.assertEqual(g["identity"]["sha1"], m["games"][0]["identity"]["sha1"])
        self.assertEqual(g["identity"]["size"], m["games"][0]["identity"]["size"])
        self.assertTrue(g["assets"]["logo"].endswith("logo.png"))


if __name__ == "__main__":
    # Warnings esperados são capturados por assertLogs; evita ruído dos demais no stderr.
    logging.getLogger("assetbind").addHandler(logging.NullHandler())
    logging.getLogger("assetbind").propagate = False
    unittest.main(verbosity=2)
