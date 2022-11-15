.thumb

.global biosRead16

biosRead16:
	mov r1, r0
	mov r2, #2
	swi #0xE
	mov r0, r3
	bx lr