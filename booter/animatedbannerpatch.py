import os
from argparse import ArgumentParser
from struct import pack


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
    # we're going to make this look nice
    filesize = os.path.getsize(path)
    while(filesize % 16 != 0):
        filesize += 1
    with open(path, "rb+") as outfile:
        # new animated banner location
        outfile.seek(0x68, 0)
        outfile.write(pack("<I", filesize))
        outfile.seek(0x208, 0)
        outfile.write(b'\xC0')
        outfile.seek(0x209, 0)
        outfile.write(b'\x23')
        outfile.seek(filesize, 0)
        infile = open(banner, "rb")
        outfile.write(infile.read())
        infile.close()
        # update header CRC
        outfile.seek(0, 0)
        crc = crc16(outfile.read(0x15E))
        outfile.seek(0x15E, 0)
        outfile.write(crc)
        outfile.close()
    return 0


if __name__ == "__main__":
    description = "Animated banner injector tool\n\
        This must have a prepared animated banner binary!"
    parser = ArgumentParser(description=description)
    parser.add_argument("input", metavar="input.nds", type=str, nargs=1, help="DS ROM path")
    parser.add_argument("banner", metavar="banner.bin", type=str, nargs=1, help="Animated banner path")
    args = parser.parse_args()
    print(description)
    path = args.input[0]
    banner = args.banner[0]

    if(patch(path, banner)) == 0:
        print("\nSuccess.")
