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

	.arm
	.global arm7clearRAM
	.type	arm7clearRAM STT_FUNC
arm7clearRAM:

	push	{r0-r9}
	// clear exclusive IWRAM
	// 0380:0000 to 0380:FFFF, total 64KiB
	mov	r0, #0
	mov	r1, #0
	mov	r2, #0
	mov	r3, #0
	mov	r4, #0
	mov	r5, #0
	mov	r6, #0
	mov	r7, #0
	mov	r8, #0x03800000
	sub	r8, #0x00008000
	mov	r9, #0x03800000
	orr	r9, r9, #0x10000
clear_IWRAM_loop:
	stmia	r8!, {r0, r1, r2, r3, r4, r5, r6, r7}
	cmp	r8, r9
	blt	clear_IWRAM_loop

	pop	{r0-r9}
	
	bx	lr
  
