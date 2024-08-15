#
# DLDI auto-patch disabler
#
# Copyright (C) 2024 lifehackerhansol
# SPDX-License-Identifier: 0BSD
#

from array import array
from os import stat
from sys import argv, exit


def main():

    argc = len(argv)
    if (argc < 2):
        print("dldinoap.py in")
        exit(1)

    buf = array("B")

    try:
        input_size = stat(argv[1]).st_size
        with open(argv[1], "rb") as i:
            buf.fromfile(i, input_size)
    except Exception:
        print("cannot open input")
        exit(2)

    dldiFound = False

    for i in range(0, len(buf), 4):
        # DLDI header magic
        if buf[i+3] == 0xBF and buf[i+2] == 0x8D and buf[i+1] == 0xA5 and buf[i] == 0xED:
            dldiFound = True
            buf[i] = 0
            buf[i+1] = 0
            buf[i+2] = 0
            buf[i+3] = 0
            break

    if not dldiFound:
        print("DLDI section not found, exiting")
        exit(3)

    try:
        with open(argv[1], "wb") as o:
            buf.tofile(o)
    except Exception:
        print("cannot open output")
        exit(4)

    print("Success.\n")
    exit(0)


main()
