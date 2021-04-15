#!/usr/bin/env python

import os
import struct
import sys

files = []
data = []

# Get list of files and data from them
dirlist = os.listdir("apfix")
dirlist += os.listdir(os.path.join("apfix", "cht"))
dirlist.sort()
for file in dirlist:
	if not file[-3:] in ["ips", "bin"]:
		continue

	files.append(file)

	if os.path.exists(os.path.join("apfix", file)):
		with open(os.path.join("apfix", file), "rb") as f:
			data.append(f.read())
	else:
		with open(os.path.join("apfix", "cht", file), "rb") as f:
			data.append(f.read())

# Write file
with open(os.path.join("..", "7zfile", "_nds", "TWiLightMenu", "extras", "apfix.pck"), "wb") as f:
	f.write(b".PCK") # Format magic
	f.write(struct.pack("<l", len(files))) # File count
	f.write(b"\x00" * 8) # Padding

	# Position of data, starts after the file list
	position = 16 + len(files) * 16

	# Write file list
	for i, file in enumerate(files):
		if sys.version_info[0] == 3:
			f.write(bytes(file[:4], "ASCII")) # TID
		else:
			f.write(file[:4]) # TID
		# Header CRC-16, Position of data, Size of data, Flags (bit 1 is cheat), Padding
		f.write(struct.pack("<Hllbx", int(file[5:file.find(".")], 16), position, len(data[i]), 1 if file[-3:] == "bin" else 0))
		position += len(data[i])

	# Write data
	for d in data:
		f.write(d)
