/*-----------------------------------------------------------------

 Copyright (C) 2010  Dave "WinterMute" Murphy

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

------------------------------------------------------------------*/
	.global	_start
	
//-----------------------------------------------------------------
_start:
//-----------------------------------------------------------------
	.ascii	"bootstub"
	.word	hook7from9 - _start
	.word	hook9from7 - _start
_loader_addr:
	.word	_loader - _start
_loader_size:
	.word	0

	.arch	armv4t
	.cpu	arm7tdmi

//-----------------------------------------------------------------
hook9from7:
//-----------------------------------------------------------------
	ldr	r0, arm9bootaddr
	adr	r1, hook7from9
	str	r1, [r0]

	mov	r3, #0x04000000
	ldr	r0, resetcode
	str	r0, [r3, #0x188]
	add	r3, r3, #0x180

	adr	r0, waitcode_start
	ldr	r1, arm7base
	adr	r2, waitcode_end
1:	ldr	r4, [r0],#4
	str	r4, [r1],#4
	cmp	r2, r0
	bne	1b
	
	ldr	r1, arm7base
	bx	r1

//-----------------------------------------------------------------
waitcode_start:
//-----------------------------------------------------------------
	push	{lr}
	mov	r2, #1
	bl	waitsync

	mov	r0, #0x100
	strh	r0, [r3]

	mov	r2, #0
	bl	waitsync

	mov	r0, #0
	strh	r0, [r3]
	pop	{lr}
	
	bx	lr
	
waitsync:
	ldrh	r0, [r3]
	and	r0, r0, #0x000f
	cmp	r0, r2
	bne	waitsync
	bx	lr
waitcode_end:

arm7base:
	.word	0x037f8000
arm7bootaddr:
	.word	0x02FFFE34
arm9bootaddr:
	.word	0x02FFFE24
tcmpudisable:
	.word	0x2078

resetcode:
	.word	0x0c04000c

//-----------------------------------------------------------------
hook7from9:
//-----------------------------------------------------------------
	mov	r12, #0x04000000
	strb	r12, [r12,#0x208]

	.arch	armv5te
	.cpu	arm946e-s

	ldr	r1, tcmpudisable		@ disable TCM and protection unit
	mcr	p15, 0, r1, c1, c0

	@ Disable cache
	mov	r0, #0
	mcr	p15, 0, r0, c7, c5, 0		@ Instruction cache
	mcr	p15, 0, r0, c7, c6, 0		@ Data cache
	mcr	p15, 0, r0, c3, c0, 0		@ write buffer

	@ Wait for write buffer to empty 
	mcr	p15, 0, r0, c7, c10, 4

	add	r3, r12, #0x180		@ r3 = 4000180

	mov	r0,#0x80
	strb	r0,[r3,#0x242-0x180]

	ldr	r0, _loader_addr
	ldr	r2, _loader_size
	mov	r1, #0x06800000
	add	r1, r1, #0x48000
	add	r2, r0, r2
_copyloader:
	ldr	r4, [r0], #4
	str	r4, [r1], #4
	cmp	r0, r2
	blt	_copyloader

	mov	r0,#0x82
	strb	r0,[r3,#0x242-0x180]

	ldrh	r0,[r3,#0x204-0x180]
	orr	r0,r0,#(1<<11) | (1<<7)
	strh	r0,[r3,#0x204-0x180]
	

	ldr	r0, arm7bootaddr
	mov	r1, #0x06000000
	add	r1, r1, #0x8000
	str	r1, [r0]
	
	ldr	r0, resetcode
	str	r0, [r12, #0x188]

	mov	r2, #1
	bl	waitsync

	mov	r0, #0x100
	strh	r0, [r3]

	mov	r2, #0
	bl	waitsync

	mov	r0, #0
	strh	r0, [r3]

// set up and enter passme loop

	ldr	r0,arm9branchaddr
	ldr	r1,branchinst
	str	r1,[r0]
	str	r0,[r0,#0x20]

	bx	r0

branchinst:
	.word 0xE59FF018

arm9branchaddr:
	.word 0x02fffe04


_loader:
