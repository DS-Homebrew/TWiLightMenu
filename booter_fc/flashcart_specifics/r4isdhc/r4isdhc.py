#
# ARM9 relocator
# Patch an ARM9 binary to change RAM location to 0x02000000
# and entrypoint to 0x02000450
# Inspired by Xenon++'s r4isdhc patcher
#
# Copyright (C) 2022-2023 DS-Homebrew
#
# SPDX-License-Identifier: 0BSD
#

from array import array
from os import stat
from sys import argv, exit

TARGET_SIZE = 0x69C70
HEADER_SIZE = 0x450


def main():

    argc = len(argv)
    if (argc < 3):
        print("r4isdhc.py <in> <out>")
        exit(1)

    buf = array("B")

    buf.extend([0] * HEADER_SIZE)
    buf[0x00] = 0x12
    buf[0x01] = 0x01
    buf[0x02] = 0x00
    buf[0x03] = 0xEA
    buf[0xEC] = 0x49
    buf[0xED] = 0x00
    buf[0xEE] = 0x00
    buf[0xEF] = 0xEB

    try:
        input_size = stat(argv[1]).st_size
        with open(argv[1], "rb") as i:
            buf.fromfile(i, input_size)
    except Exception:
        print("cannot open input")
        exit(2)

    remaining = TARGET_SIZE - HEADER_SIZE - input_size
    if remaining > 0:
        buf.extend([0] * remaining)

    try:
        with open(argv[2], "wb") as o:
            buf.tofile(o)
    except Exception:
        print("cannot open output")
        exit(3)

    print("Success.\n")
    exit(0)


main()
