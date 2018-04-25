/*
	Copyright 2006 - 2015 Dave Murphy (WinterMute)

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include <nds/arm9/cache_asm.h>

	.text
	.align 4

	.arm

	.arch	armv5te
	.cpu	arm946e-s

@---------------------------------------------------------------------------------
	.global _start
	.type	_start STT_FUNC
@---------------------------------------------------------------------------------
_start:
@---------------------------------------------------------------------------------
	@ Switch off MPU
	mrc	p15, 0, r0, c1, c0, 0
	bic	r0, r0, #PROTECT_ENABLE
	mcr	p15, 0, r0, c1, c0, 0


	adr	r12, mpu_initial_data
	ldmia	r12, {r0-r10}

	mcr	p15, 0, r0, c2, c0, 0
	mcr	p15, 0, r0, c2, c0, 1
	mcr	p15, 0, r1, c3, c0, 0
	mcr	p15, 0, r2, c5, c0, 2
	mcr	p15, 0, r3, c5, c0, 3
	mcr	p15, 0, r4, c6, c0, 0
	mcr	p15, 0, r5, c6, c1, 0
	mcr	p15, 0, r6, c6, c3, 0
	mcr	p15, 0, r7, c6, c4, 0
	mcr	p15, 0, r8, c6, c6, 0
	mcr	p15, 0, r9, c6, c7, 0
	mcr	p15, 0, r10, c9, c1, 0

	mov	r0, #0
	mcr	p15, 0, r0, c6, c2, 0   @ PU Protection Unit Data/Unified Region 2
	mcr	p15, 0, r0, c6, c5, 0   @ PU Protection Unit Data/Unified Region 5

	mrc	p15, 0, r0, c9, c1, 0   @ DTCM
	mov	r0, r0, lsr #12         @ base
	mov	r0, r0, lsl #12         @ size
	add	r0, r0, #0x4000         @ dtcm top

	sub	r0, r0, #4              @ irq vector
	mov	r1, #0
	str 	r1, [r0]
	sub	r0, r0, #4              @ IRQ1 Check Bits
	str 	r1, [r0]

	sub	r0, r0, #128
	bic	r0, r0, #7

	msr	cpsr_c, #0xd3      @ svc mode
	mov	sp, r0
	sub	r0, r0, #128
	msr	cpsr_c, #0xd2      @ irq mode
	mov	sp, r0
	sub	r0, r0, #128
	msr	cpsr_c, #0xdf      @ system mode
	mov	sp, r0

	@ enable cache & tcm
	mrc	p15, 0, r0, c1, c0, 0
	ldr	r1,= ITCM_ENABLE | DTCM_ENABLE | ICACHE_ENABLE | DCACHE_ENABLE
	orr	r0,r0,r1
	mcr	p15, 0, r0, c1, c0, 0

	ldr	r10, =0x2FFFE04
	ldr	r0, =0xE59FF018
	str	r0, [r10]
	add	r1, r10, #0x20
	str	r10, [r1]
	bx	r10

	.pool

mpu_initial_data:
	.word 0x00000042  @ p15,0,c2,c0,0..1,r0 ;PU Cachability Bits for Data/Unified+Instruction Protection Region
	.word 0x00000002  @ p15,0,c3,c0,0,r1    ;PU Write-Bufferability Bits for Data Protection Regions
	.word 0x15111011  @ p15,0,c5,c0,2,r2    ;PU Extended Access Permission Data/Unified Protection Region
	.word 0x05100011  @ p15,0,c5,c0,3,r3    ;PU Extended Access Permission Instruction Protection Region
	.word 0x04000033  @ p15,0,c6,c0,0,r4    ;PU Protection Unit Data/Unified Region 0
	.word 0x0200002b  @ p15,0,c6,c1,0,r5    ;PU Protection Unit Data/Unified Region 1 4MB
	.word 0x08000035  @ p15,0,c6,c3,0,r6    ;PU Protection Unit Data/Unified Region 3
	.word 0x0300001b  @ p15,0,c6,c4,0,r7    ;PU Protection Unit Data/Unified Region 4
	.word 0xffff001d  @ p15,0,c6,c6,0,r8    ;PU Protection Unit Data/Unified Region 6
	.word 0x02fff017  @ p15,0,c6,c7,0,r9    ;PU Protection Unit Data/Unified Region 7 4KB
	.word 0x0300000a  @ p15,0,c9,c1,0,r10   ;TCM Data TCM Base and Virtual Size
