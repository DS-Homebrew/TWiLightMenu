/*-----------------------------------------------------------------

 Copyright (C) 2005  Michael "Chishm" Chisholm

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

 If you use this code, please give due credit and email me about your
 project at chishm@hotmail.com
------------------------------------------------------------------*/
@---------------------------------------------------------------------------------
	.align	4
	.arm
	.global _dldi_start
	.global _io_dldi
@---------------------------------------------------------------------------------
.equ FEATURE_MEDIUM_CANREAD,		0x00000001
.equ FEATURE_MEDIUM_CANWRITE,		0x00000002
.equ FEATURE_SLOT_GBA,			0x00000010
.equ FEATURE_SLOT_NDS,			0x00000020


_dldi_start:
@---------------------------------------------------------------------------------
@ IO_INTERFACE data -- 32 bytes
_io_dldi:
	.ascii	"DLDI"				@ ioType
	.word	0x00000000			@ Features
	.word	_DLDI_startup			@
	.word	_DLDI_isInserted		@
	.word	_DLDI_readSectors		@   Function pointers to standard device driver functions
	.word	_DLDI_writeSectors		@
	.word	_DLDI_clearStatus		@
	.word	_DLDI_shutdown			@

_DLDI_startup:
	mov r12, #0x06000000
	ldr r12, [r12, #0x68]
	bx r12
_DLDI_isInserted:
	mov r12, #0x06000000
	ldr r12, [r12, #0x6C]
	bx r12
_DLDI_readSectors:
	mov r12, #0x06000000
	ldr r12, [r12, #0x70]
	bx r12
_DLDI_writeSectors:
	mov r12, #0x06000000
	ldr r12, [r12, #0x74]
	bx r12
_DLDI_clearStatus:
	mov r12, #0x06000000
	ldr r12, [r12, #0x78]
	bx r12
_DLDI_shutdown:
	mov r12, #0x06000000
	ldr r12, [r12, #0x7C]
	bx r12
