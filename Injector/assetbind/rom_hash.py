"""
rom_hash.py -- content identity of a ROM.

WHY HASH INSTEAD OF NAME: the file name is fragile (it can be renamed, duplicated, or two
different ROMs can share the same name). The correct identity is the CONTENT. For DS we hash
the WHOLE file (no header stripping). sha1, md5, crc32 and size are computed in a single read pass.
"""
import hashlib
import zlib


def hash_file(path, chunk=1 << 20):
    """Returns dict {sha1, md5, crc32, size} for the file's content (single pass)."""
    sha1 = hashlib.sha1()
    md5 = hashlib.md5()
    crc = 0
    size = 0
    with open(path, "rb") as f:
        while True:
            b = f.read(chunk)
            if not b:
                break
            sha1.update(b)
            md5.update(b)
            crc = zlib.crc32(b, crc)
            size += len(b)
    return {
        "sha1": sha1.hexdigest(),
        "md5": md5.hexdigest(),
        "crc32": format(crc & 0xFFFFFFFF, "08x"),
        "size": size,
    }
