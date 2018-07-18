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

@ void arm7_clearmem (void* loc, size_t len);
@ Clears memory using an stmia loop

#include <nds/asminc.h>

.arm


BEGIN_ASM_FUNC arm7_clearmem
	add    r1, r0, r1
	stmfd  sp!, {r4-r9}
	mov    r2, #0
	mov    r3, #0
	mov    r4, #0
	mov    r5, #0
	mov    r6, #0
	mov    r7, #0
	mov    r8, #0
	mov    r9, #0

clearmem_loop:
	stmia  r0!, {r2-r9}
	cmp    r0, r1
	blt    clearmem_loop

	ldmfd  sp!, {r4-r9}
	bx     lr

