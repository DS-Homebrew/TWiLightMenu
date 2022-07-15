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
#ifndef NO_DLDI

@---------------------------------------------------------------------------------
@ Driver patch file standard header -- 16 bytes
#ifdef STANDARD_DLDI
	.word	0xBF8DA5ED		@ Magic number to identify this region
#else
	.word	0xBF8DA5EE		@ Magic number to identify this region
#endif
	.asciz	" Chishm"		@ Identifying Magic string (8 bytes with null terminator)
	.byte	0x01			@ Version number
	.byte	0x0e		@ 16KiB	@ Log [base-2] of the size of this driver in bytes.
	.byte	0x00			@ Sections to fix
	.byte 	0x0e		@ 16KiB	@ Log [base-2] of the allocated space in bytes.
	
@---------------------------------------------------------------------------------
@ Text identifier - can be anything up to 47 chars + terminating null -- 16 bytes
	.align	4
	.asciz "Loader (No interface)"

@---------------------------------------------------------------------------------
@ Offsets to important sections within the data	-- 32 bytes
	.align	6
	.word   _dldi_start		@ data start
	.word   _dldi_end		@ data end
	.word	0x00000000		@ Interworking glue start	-- Needs address fixing
	.word	0x00000000		@ Interworking glue end
	.word   0x00000000		@ GOT start			-- Needs address fixing
	.word   0x00000000		@ GOT end
	.word   0x00000000		@ bss start			-- Needs setting to zero
	.word   0x00000000		@ bss end
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

	
@---------------------------------------------------------------------------------

_DLDI_startup:
_DLDI_isInserted:
_DLDI_readSectors:
_DLDI_writeSectors:
_DLDI_clearStatus:
_DLDI_shutdown:
	mov		r0, #0x00		@ Return false for every function
	bx		lr



@---------------------------------------------------------------------------------
	.align
	.pool

	.space (_dldi_start + 16384) - .	@ Fill to 16KiB

_dldi_end:
	.end
@---------------------------------------------------------------------------------
#else
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
_DLDI_isInserted:
_DLDI_readSectors:
_DLDI_writeSectors:
_DLDI_clearStatus:
_DLDI_shutdown:
	mov		r0, #0x00		@ Return false for every function
	bx		lr


#endif
