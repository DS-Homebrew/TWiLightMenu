@ LZ77 decompression routine
@ v1.0 - by fincs

.arch armv5te
.arm

.align 2
.global LZ77_Decompress
.type LZ77_Decompress, %function

LZ77_Decompress:
	ldr r3, [r0], #4
	and r2, r3, #0xF0
	cmp r2, #0x10
	bxne lr

	mov r3, r3, lsr #8
	push {r4, lr}
.hdr:
	ldrb r2, [r0], #1
	bl .doblock
	mov r2, r2, lsl #1
	bl .doblock
	mov r2, r2, lsl #1
	bl .doblock
	mov r2, r2, lsl #1
	bl .doblock
	mov r2, r2, lsl #1
	bl .doblock
	mov r2, r2, lsl #1
	bl .doblock
	mov r2, r2, lsl #1
	bl .doblock
	mov r2, r2, lsl #1
	bl .doblock
	b .hdr

.doblock:
	tst r2, #0x80
	beq .uncomp

	ldrb r4, [r0], #1
	mov r4, r4, ror #4
	ldrb r12, [r0], #1
	orr r12, r4, lsr #20
	and r4, r4, #0xF
	add r4, r4, #3
	sub r12, r1, r12
	sub r12, r12, #1

.copyloop:
	swpb r4, r4, [r1]
	ldrb r4, [r12], #1
	swpb r4, r4, [r1]
	add r1, r1, #1
	subs r3, r3, #1
	beq .exit
	subs r4, r4, #1
	bne .copyloop
	bx lr

.uncomp:
	ldrb r4, [r0], #1
	strb r4, [r1], #1
	subs r3, r3, #1
	beq .exit
	bx lr

.exit:
	pop {r4, pc}
