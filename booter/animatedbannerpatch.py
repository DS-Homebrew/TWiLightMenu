# import os
from argparse import ArgumentParser
from struct import pack, unpack


# GBATEK swiCRC16 pseudocode
# https://problemkaputt.de/gbatek-bios-misc-functions.htm
def crc16(data):
    crc = 0xFFFF
    for byte in bytearray(data):
        crc ^= byte
        for i in range(8):
            carry = (crc & 0x0001) > 0
            crc = crc >> 1
            if carry:
                crc = crc ^ 0xA001
    return pack("<H", crc)


def patch(path, banner):
    with open(path, "rb+") as f:
        # get total TWL ROM size
        f.seek(0x210, 0)
        romsize = unpack("<I", f.read(0x4))[0]

        # we're going to make this look nice in a hex editor
        while (romsize % 16 != 0):
            romsize += 1

        # new animated banner location
        f.seek(0x68, 0)
        f.write(pack("<I", romsize))

        # total ROM size update, both DS and DSi header entry
        # NTR ROM size is supposed to be NTR data only, meaning TWL ROM size is different.
        # Ignore it, as new banner is added to EOF, so the whole thing has to be included anyway.
        f.seek(0x80, 0)
        f.write(pack("<I", romsize + 0x23c0))
        f.seek(0x210, 0)
        f.write(pack("<I", romsize + 0x23c0))

        # total icon size update in TWL header
        f.seek(0x208, 0)
        f.write(pack("<I", 0x23c0))

        # actually write the banner now
        f.seek(romsize, 0)
        infile = open(banner, "rb")
        f.write(infile.read())
        infile.close()

        # update header CRC
        f.seek(0, 0)
        crc = crc16(f.read(0x15E))
        f.seek(0x15E, 0)
        f.write(crc)
        f.close()
    return 0


if __name__ == "__main__":
    description = "Animated banner injector tool\n\
        This must have a prepared animated banner binary!\n\
        Only compatible with DSi-mode ROMs."
    parser = ArgumentParser(description=description)
    parser.add_argument("input", metavar="input.nds", type=str, nargs=1, help="DS ROM path")
    parser.add_argument("banner", metavar="banner.bin", type=str, nargs=1, help="Animated banner path")
    args = parser.parse_args()
    print(description)
    path = args.input[0]
    banner = args.banner[0]

    if (patch(path, banner)) == 0:
        print("\nSuccess.")
