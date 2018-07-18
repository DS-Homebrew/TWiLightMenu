@   NitroHax -- Cheat tool for the Nintendo DS
@   Copyright (C) 2008  Michael "Chishm" Chisholm
@
@   This program is free software: you can redistribute it and/or modify
@   it under the terms of the GNU General Public License as published by
@   the Free Software Foundation, either version 3 of the License, or
@   (at your option) any later version.
@
@   This program is distributed in the hope that it will be useful,
@   but WITHOUT ANY WARRANTY; without even the implied warranty of
@   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
@   GNU General Public License for more details.
@
@   You should have received a copy of the GNU General Public License
@   along with this program.  If not, see <http://www.gnu.org/licenses/>.

@ Clears ICache and Dcache, and resets the protection units
@ Originally written by Darkain, modified by Chishm

#include <nds/asminc.h>

.arm

BEGIN_ASM_FUNC arm9_clearCache
	@ Clean and flush cache
	mov r1, #0                   
	outer_loop:                  
		mov r0, #0                  
		inner_loop:                 
			orr r2, r1, r0             
			mcr p15, 0, r2, c7, c14, 2 
			add r0, r0, #0x20          
			cmp r0, #0x400             
		bne inner_loop              
		add r1, r1, #0x40000000     
		cmp r1, #0x0                
	bne outer_loop               

	mov r3, #0                  
	mcr p15, 0, r3, c7, c5, 0		@ Flush ICache
	mcr p15, 0, r3, c7, c6, 0		@ Flush DCache
	mcr p15, 0, r3, c7, c10, 4		@ empty write buffer

	mcr p15, 0, r3, c3, c0, 0		@ disable write buffer       (def = 0)

	mcr p15, 0, r3, c2, c0, 0		@ disable DTCM and protection unit

	mcr p15, 0, r3, c6, c0, 0		@ disable protection unit 0  (def = 0)
	mcr p15, 0, r3, c6, c1, 0		@ disable protection unit 1  (def = 0)
	mcr p15, 0, r3, c6, c2, 0 		@ disable protection unit 2  (def = 0)
	mcr p15, 0, r3, c6, c3, 0		@ disable protection unit 3  (def = 0)
	mcr p15, 0, r3, c6, c4, 0		@ disable protection unit 4  (def = ?)
	mcr p15, 0, r3, c6, c5, 0		@ disable protection unit 5  (def = ?)
	mcr p15, 0, r3, c6, c6, 0		@ disable protection unit 6  (def = ?)
	mcr p15, 0, r3, c6, c7, 0		@ disable protection unit 7  (def = ?)

	mcr p15, 0, r3, c5, c0, 3		@ IAccess
	mcr p15, 0, r3, c5, c0, 2		@ DAccess

	mov r3, #0x00800000
	add r3, r3, #0x00A
	mcr p15, 0, r3, c9, c1, 0		@ DTCM base  (def = 0x0080000A) ???

	mov r3, #0x0000000C
	mcr p15, 0, r3, c9, c1, 1		@ ITCM base  (def = 0x0000000C) ???

	bx lr

