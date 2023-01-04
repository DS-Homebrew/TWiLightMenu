# -*- coding: utf8 -*-
# Patch a homebrew or DS-Demo .nds file for use with make_cia
#
# 2016-02-28, Ahezard
#
# inspired by
# Apache Thunder .nds edited files and comments
# https://github.com/Relys/Project_CTR/blob/master/makerom/srl.h
# https://dsibrew.org/wiki/DSi_Cartridge_Header
# Patch header size of nds file to 0x4000 (normal ds/dsi header), 0x3E00 offset
# only if base header size = 0x200 (old homebrew)

from struct import *
from collections import namedtuple
# from collections import OrderedDict
from pprint import pprint
import os  # , sys
# import binascii
import argparse
from ctypes import c_ushort


parser = argparse.ArgumentParser(description='Patch an nds in order to be ready cia conversion via make_cia --srl=.')
parser.add_argument('file', metavar='file.nds', type=argparse.FileType('rb'), help='nds file to patch')
parser.add_argument('--verbose', help='verbose mode', action="store_true")
parser.add_argument('--out', help='output file [optionnal]')
parser.add_argument('--read', help='print only the header content, do not patch', action="store_true")
parser.add_argument('--extract', help='extract the content of the rom : header.bin,arm9.bin,arm7.bin,icon.bin,arm9i.bin,arm7i.bin, do not patch', action="store_true")  # Not yet implemented
parser.add_argument('--title', help='Game title')
parser.add_argument('--code', help='Game code')
parser.add_argument('--maker', help='Maker code')
parser.add_argument('--mode', help='target mode, default mode is ds [ds|dsi|dsinogba|nitrohax]')
parser.add_argument('--arm9', type=argparse.FileType('rb'), help='swap the ds arm9 binary by the one provided')
parser.add_argument('--arm7', type=argparse.FileType('rb'), help='swap the ds arm7 binary by the one provided')
parser.add_argument('--arm9EntryAddress', help='arm9 ram address of the binary provided')
parser.add_argument('--arm7EntryAddress', help='arm7 ram address of the binary provided')
parser.add_argument('--arm9i', type=argparse.FileType('rb'), help='add a dsi arm9i binary to the file')
parser.add_argument('--arm7i', type=argparse.FileType('rb'), help='add a dsi arm7i binary to the file')
parser.add_argument('--accessControl', help='access control field')
parser.add_argument('--ntrHb', help='NTR homebrew', action="store_true")
parser.add_argument('--ntrTouch', help='Toggle NTR Touch or not', action="store_true")
parser.add_argument('--twlTouch', help='Toggle TWL Touch or not', action="store_true")
parser.add_argument('--twlIcon', help='Toggle TWL Icon size or not', action="store_true")
parser.add_argument('--structParam', help='Set device list', action="store_true")
args = parser.parse_args()

if args.mode is None:
	args.mode = "dsi"

class CRC16(object):

	"""Source:  https://github.com/cristianav/PyCRC/blob/master/demo.py"""
	crc16_tab = []

	# The CRC's are computed using polynomials. Here is the most used
	# coefficient for CRC16
	crc16_constant = 0xA001  # 40961

	def __init__(self, modbus_flag=False):
		# initialize the precalculated tables
		if not len(self.crc16_tab):
			self.init_crc16()
		self.mdflag = bool(modbus_flag)

	def calculate(self, input_data=None):
		try:
			is_string = isinstance(input_data, str)
			is_bytes = isinstance(input_data, (bytes, bytearray))

			if not is_string and not is_bytes:
				raise Exception("Please provide a string or a byte sequence "
								"as argument for calculation.")

			crc_value = 0x0000 if not self.mdflag else 0xffff

			for c in input_data:
				d = ord(c) if is_string else c
				tmp = crc_value ^ d
				rotated = crc_value >> 8
				crc_value = rotated ^ self.crc16_tab[(tmp & 0x00ff)]

			return crc_value
		except Exception as e:
			print("EXCEPTION(calculate): {}".format(e))

	def init_crc16(self):
		"""The algorithm uses tables with precalculated values"""
		for i in range(0, 256):
			crc = c_ushort(i).value
			for j in range(0, 8):
				if crc & 0x0001:
					crc = c_ushort(crc >> 1).value ^ self.crc16_constant
				else:
					crc = c_ushort(crc >> 1).value
			self.crc16_tab.append(crc)


def getSize(fileobject):
	current = fileobject.tell()
	fileobject.seek(0, 2)  # move the cursor to the end of the file
	size = fileobject.tell()
	fileobject.seek(current, 0)
	return size

def skipUntilAddress(f_in, f_out, caddr, taddr):
	chunk = f_in.read(taddr - caddr)
	f_out.write(chunk)

def writeBlankuntilAddress(f_out, caddr, taddr):
	f_out.write(b"\x00"*(taddr-caddr))

fname=args.file.name
args.file.close()

if not args.read:
	print("Patching file : "+fname)
else:
	print("Reading header of file : "+fname)

# offset of 0x4600 created

# File size compute
file = open(fname, 'rb')
fsize=getSize(file)
file.close()

# CRC header compute "CRC-16 (Modbus)"
file = open(fname, 'rb')
# 0x15E from https://github.com/devkitPro/ndstool/ ... source/header.cpp
hdr = file.read(0x15E)
hdrCrc=CRC16(modbus_flag=True).calculate(hdr)
if args.verbose:
	print("{:10s} {:20X}".format('HDR CRC-16 ModBus', hdrCrc))
# print("origin header cr c"+hdr[0x15E:0x15F])
# filew = open(fname+".hdr", "wb")
# filew.write(hdr);
# filew.close()
file.close()

if args.arm9 is not None:
	arm9Fname = args.arm9.name
	args.arm9.close()
	arm9File = open(arm9Fname, 'rb')
	arm9FileSize = getSize(arm9File)
	dataArm9 = arm9File.read(arm9FileSize)
	arm9File.close()

if args.arm7 is not None:
	arm7Fname = args.arm7.name
	args.arm7.close()
	arm7File = open(arm7Fname, 'rb')
	arm7FileSize = getSize(arm7File)
	dataArm7 = arm7File.read(arm7FileSize)
	arm7File.close()

filer = open(fname, 'rb')
data = filer.read(0xB0)
data += b'DoNotZeroFillMem'
filer.read(0x10)
data = data + filer.read(0xC0)
caddr = 0x180

# DS Data 180 bytes
SrlHeader = namedtuple('SrlHeader', 
	"gameTitle "
	"gameCode "
	"makerCode "
	"unitCode "
	"encryptionSeedSelect "
	"deviceCapacity "
	"reserved0 " 
	"dsiflags " 
	"romVersion "
	"internalFlag "
	"arm9RomOffset "
	"arm9EntryAddress "
	"arm9RamAddress "
	"arm9Size "
	"arm7RomOffset "
	"arm7EntryAddress "
	"arm7RamAddress "
	"arm7Size "
	"fntOffset "
	"fntSize "
	"fatOffset "
	"fatSize "
	"arm9OverlayOffset "
	"arm9OverlaySize "
	"arm7OverlayOffset "
	"arm7OverlaySize "
	"normalCardControlRegSettings "
	"secureCardControlRegSettings "
	"icon_bannerOffset "
	"secureAreaCrc "
	"secure_transfer_timeout "
	"arm9Autoload "
	"arm7Autoload "
	"secureDisable "
	"ntrRomSize "
	"headerSize "
	"reserved1 "
	"nintendoLogo "
	"nintendoLogoCrc "
	"headerCrc "
	"debugReserved ")
srlHeaderFormat = '<12s4s2scbb7s2sbcIIIIIIIIIIIIIIIIIIIHHII8sII56s156s2sH32s'
srlHeader = SrlHeader._make(unpack_from(srlHeaderFormat, data))
if args.verbose:
	print("origin header crc "+hex(srlHeader.headerCrc))
	print("origin secure crc "+hex(srlHeader.secureAreaCrc))

# SecureArea CRC compute "CRC-16 (Modbus)"
file = open(fname, 'rb')
# 0x15E from https://github.com/devkitPro/ndstool/ ... source/header.cpp
file.read(0x200)
sec = file.read(0x4000)
secCrc = CRC16(modbus_flag=True).calculate(sec)
if args.verbose:
	print("{:10s} {:20X}".format('SEC CRC-16 ModBus', secCrc))
file.close()

if srlHeader.arm7EntryAddress > 0x2400000 and not args.read and args.arm7 is None:
	print("WARNING: .nds arm7EntryAddress greater than 0x2400000 will not boot as cia")
	print("you need to recompile or swap the arm7 binary with a precompiled one with --arm7 and --arm7EntryAddress")

if "dsi" in args.mode:
	srlHeaderPatched = srlHeader._replace(
		dsiflags=					b'\x01\x00',  # disable modcrypt but enable twl
		unitCode=					b'\x03',
		)

data1 = pack(srlHeaderFormat, *srlHeaderPatched._asdict().values())
newHdrCrc = CRC16(modbus_flag = True).calculate(data1[0:0x15E])
srlHeaderPatched = srlHeaderPatched._replace(headerCrc = newHdrCrc)

if args.verbose:
	print("new header crc "+hex(newHdrCrc))
if not args.read:
	if args.verbose:
		pprint(dict(srlHeaderPatched._asdict()))
else:
	pprint(dict(srlHeader._asdict()))

data1 = pack(srlHeaderFormat, *srlHeaderPatched._asdict().values())

arm9isize = 0
arm7isize = 0

# TWL Only Data 384 bytes
SrlTwlExtHeader = namedtuple('SrlTwlExtHeader', 
	"MBK_1_5_Settings "
	"MBK_6_8_Settings_ARM9 "
	"MBK_6_8_Settings_ARM7 "
	"global_MBK_9_Setting "
	"regionFlags "
	"accessControl "
	"arm7ScfgExtMask "
	"reserved_flags "
	"arm9iRomOffset "
	"reserved2 "
	"arm9iLoadAddress "
	"arm9iSize "
	"arm7iRomOffset "
	"struct_param_baseAddress "
	"arm7iLoadAddress "
	"arm7iSize "
	"digest_ntrRegionOffset "
	"digest_ntrRegionSize "
	"digest_twlRegionOffset "
	"digest_twlRegionSize "
	"digestSectorHashtableOffset "
	"digestSectorHashtableSize "
	"digest_blockHashtableOffset "
	"digest_blockHashtableSize "
	"digestSectorSize "
	"digest_blockSectorcount "
	"iconSize "	#usually 0x23C0 or 2112 in homebrew
	"unknown1 "
	"twlRomSize "
	"unknown2 "
	"modcryptArea1Offset "
	"modcryptArea1Size "
	"modcryptArea2Offset "
	"modcryptArea2Size "
	"title_id "
	"pubSaveDataSize "
	"privSaveDataSize "
	"reserved4 "
	"parentalControl ")
srlTwlExtHeaderFormat = "<20s12s12s4s4sIIII4sIIIIIIIIIIIIIIIII4sI12sIIII8sII176s16s"
if srlHeader.headerSize < 0x300:
	# homebrew
	srlTwlExtHeader = SrlTwlExtHeader._make(unpack_from(srlTwlExtHeaderFormat, "\x00" * (0x300-0x180)))
else:
	data = filer.read(0x300-0x180)
	srlTwlExtHeader = SrlTwlExtHeader._make(unpack_from(srlTwlExtHeaderFormat, data))
	caddr = 0x300

# pprint(dict(srlTwlExtHeader._asdict()))

if not args.read:
	# Fix srlTwlExtHeader
	if "dsi" in args.mode:
		arm7iRomOffset = srlHeaderPatched.arm7RomOffset
		arm9iRomOffset = srlHeaderPatched.arm9RomOffset	
		arm7isize = srlHeaderPatched.arm7Size
		arm9isize = srlHeaderPatched.arm9Size
		totaldsisize = 0
		arm7iname = None
		arm9iname = None

		if args.ntrHb:
			if args.arm9i is not None:
				arm9iname = args.arm9i.name
				arm9isize = getSize(args.arm9i)
				arm9iRomOffset = srlHeaderPatched.ntrRomSize
				if args.verbose:
					print("arm9isize : "+hex(arm9isize))
					print("arm9ioffset : "+hex(srlHeaderPatched.ntrRomSize))
				args.arm9i.close()
				totaldsisize = arm9isize

			if args.arm7i is not None:
				arm7iname = args.arm7i.name
				arm7isize = getSize(args.arm7i)
				arm7iRomOffset = srlHeaderPatched.ntrRomSize+arm9isize
				if args.verbose:
					print("arm7isize : "+hex(arm7isize))
					print("arm9ioffset : "+hex(srlHeaderPatched.ntrRomSize+arm9isize))
				args.arm7i.close()
				totaldsisize = arm9isize + arm7isize

			srlTwlExtHeader = srlTwlExtHeader._replace(
				accessControl=			0x00000138,
				arm7ScfgExtMask=		0x80040000,
				reserved_flags=			0x00000000
				)

		if args.ntrTouch:
			srlTwlExtHeader = srlTwlExtHeader._replace(reserved_flags=0x00000000)

		if args.twlTouch:
			srlTwlExtHeader = srlTwlExtHeader._replace(reserved_flags=0x01000000)

		if args.twlIcon:
			srlTwlExtHeader = srlTwlExtHeader._replace(iconSize=0x23C0)

		if args.structParam:
			srlTwlExtHeader = srlTwlExtHeader._replace(struct_param_baseAddress=0x02800000)

		if args.accessControl is not None:
			srlTwlExtHeader = srlTwlExtHeader._replace(accessControl=int(args.accessControl, 0))

if args.verbose or args.read:	
	pprint(dict(srlTwlExtHeader._asdict()))

data2=pack(srlTwlExtHeaderFormat, *srlTwlExtHeader._asdict().values())

#TWL and Signed NTR 3328 bytes
SrlSignedHeader = namedtuple('SrlSignedHeader', 
	"arm9WithSecAreaSha1Hmac "
	"arm7Sha1Hmac "
	"digestMasterSha1Hmac "
	"bannerSha1Hmac "
	"arm9iSha1Hmac "
	"arm7iSha1Hmac "
	"reserved5 "
	"arm9Sha1Hmac "
	"reserved6 "
	"reserved7 "
	"signature "
	)
srlSignedHeaderFormat = "<20s20s20s20s20s20s40s20s2636s384s128s"
if srlHeader.headerSize < 0x1100:
	# homebrew
	srlSignedHeader = SrlSignedHeader._make(unpack_from(srlSignedHeaderFormat, "\x00" * (3328)))
else:
	data = filer.read(3328)
	srlSignedHeader = SrlSignedHeader._make(unpack_from(srlSignedHeaderFormat, data))
	caddr = 0x300 + 3328
	filer.read(0x4000 - caddr)
	caddr = 0x4000

# pprint(dict(srlSignedHeader._asdict()))

# Fix srlSignedHeader
if not args.read:
	if args.ntrHb:
		srlSignedHeader=srlSignedHeader._replace(
			arm7Sha1Hmac=				'\xff'*20,
			arm9WithSecAreaSha1Hmac=	'\xff'*20,
			bannerSha1Hmac=				'\xff'*20,
			signature=					'\xff'*128
			)
		if "dsi" in args.mode :
			srlSignedHeader=srlSignedHeader._replace(
				arm7Sha1Hmac=				'\xff'*20,
				arm7iSha1Hmac=				'\xff'*20,
				arm9Sha1Hmac=				'\xff'*20,
				arm9WithSecAreaSha1Hmac=	'\xff'*20,
				arm9iSha1Hmac=				'\xff'*20,
				bannerSha1Hmac=				'\xff'*20,
				digestMasterSha1Hmac=		'\xff'*20,
				signature=					'\xff'*128
				)
if args.verbose or args.read:
	pprint(dict(srlSignedHeader._asdict()))

data3 = pack(srlSignedHeaderFormat, *srlSignedHeader._asdict().values())

# ARM9 footer 
# from https://github.com/devkitPro/ndstool/ ... source/header.cpp
# ARM9 footer size = 3*4
ARM9Footer = namedtuple('ARM9Footer',
	"nitrocode " #0xDEC00621
	"versionInfo "
	"reserved "
	)
ARM9FooterFormat = "<III"
file = open(fname, 'rb')
arm9FooterAddr = srlHeader.arm9RomOffset + srlHeader.arm9Size
file.read(arm9FooterAddr)
data = file.read(12)
arm9Footer = ARM9Footer._make(unpack_from(ARM9FooterFormat, data))
if args.verbose:
	print("footer addr " + hex(arm9FooterAddr))
if arm9Footer.nitrocode == 0xDEC00621:
	if args.verbose or args.read:
		print("ARM9 footer found.")
		print("no patch needed")
		print("nitrocode " + hex(arm9Footer.nitrocode))
		print("versionInfo " + hex(arm9Footer.versionInfo))
		print("reserved " + hex(arm9Footer.reserved))
		print("\n")
else:
	if args.verbose or args.read:
		print("ARM9 footer not found.\n")
	arm9FooterPatched = arm9Footer._replace(
		nitrocode=		0xDEC00621,
		versionInfo=	0xad8,
		reserved=		0
	)
	data4 = pack(ARM9FooterFormat, *arm9FooterPatched._asdict().values())
file.close()

if not args.read:
	# write the file
	if args.out is not None:
		filew = open(args.out, "wb")
	else:
		filew = open(fname + ".tmp", "wb")

	filew.write(data1)
	filew.write(data2)
	filew.write(data3[0:0xC80])
	filew.write(b'\xff'*16*8)
	writeBlankuntilAddress(filew, 0x1000, 0x4000)

	if args.ntrHb:
		if arm9Footer.nitrocode != 0xDEC00621:
			# patch ARM9 footer 
			skipUntilAddress(filer, filew, caddr, arm9FooterAddr)
			filew.write(data4)
			filer.read(12)
			caddr = arm9FooterAddr + 12

	skipUntilAddress(filer, filew, caddr, srlTwlExtHeader.twlRomSize)

	filew.close()
	filer.close()

	if args.out is None:
		if os.path.exists(fname + ".orig.nds"):
			os.remove(fname + ".orig.nds")
		os.rename(fname, fname + ".orig.nds")
		os.rename(fname + ".tmp", fname)	
	print("file patched")