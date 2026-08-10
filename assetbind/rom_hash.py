"""
rom_hash.py — identidade de conteúdo de uma ROM.

POR QUE HASH E NÃO NOME: o nome do arquivo é frágil (pode ser renomeado, duplicado, ou
duas ROMs diferentes terem o mesmo nome). A identidade correta é o CONTEÚDO. Para DS
usamos o arquivo INTEIRO (sem stripping de header). Calculamos sha1, md5, crc32 e size
numa única passada de leitura.
"""
import hashlib
import zlib


def hash_file(path, chunk=1 << 20):
    """Retorna dict {sha1, md5, crc32, size} do conteúdo do arquivo (uma passada)."""
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
