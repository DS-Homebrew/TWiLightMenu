/*---------------------------------------------------------------------------------

	Copyright (C) 2009
		Dave Murphy (WinterMute)

	This software is provided 'as-is', without any express or implied
	warranty.  In no event will the authors be held liable for any
	damages arising from the use of this software.

	Permission is granted to anyone to use this software for any
	purpose, including commercial applications, and to alter it and
	redistribute it freely, subject to the following restrictions:

	1.	The origin of this software must not be misrepresented; you
		must not claim that you wrote the original software. If you use
		this software in a product, an acknowledgment in the product
		documentation would be appreciated but is not required.
	2.	Altered source versions must be plainly marked as such, and
		must not be misrepresented as being the original software.
	3.	This notice may not be removed or altered from any source
		distribution.

---------------------------------------------------------------------------------*/

	.text
	.align 4

	.arm
@---------------------------------------------------------------------------------
	.global swiSoftResetarm9
	.type	swiSoftResetarm9 STT_FUNC
@---------------------------------------------------------------------------------
swiSoftResetarm9:
@---------------------------------------------------------------------------------
	REG_IME = 0;


	.arch	armv5te
	.cpu	arm946e-s
	ldr	r1, =0x00002078			@ disable TCM and protection unit
	mcr	p15, 0, r1, c1, c0
	@ Disable cache
	mov	r0, #0
	mcr	p15, 0, r0, c7, c5, 0		@ Instruction cache
	mcr	p15, 0, r0, c7, c6, 0		@ Data cache

	@ Wait for write buffer to empty 
	mcr	p15, 0, r0, c7, c10, 4

	ldr	r0,=0x2FFFE24


	ldr	r0,[r0]
	bx	r0

	.pool