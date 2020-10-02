@---------------------------------------------------------------------------------
	.global _arm9_start
@---------------------------------------------------------------------------------
	.align	4
	.arm
@---------------------------------------------------------------------------------
_arm9_start:
@---------------------------------------------------------------------------------
	mov	r0, #0x04000000		@ IME = 0;
	add	r0, r0, #0x208
	strh	r0, [r0]

	mov	r0, #0x12		@ Switch to IRQ Mode
	msr	cpsr, r0
	ldr	sp, =__arm9_sp_irq		@ Set IRQ stack

	mov	r0, #0x13		@ Switch to SVC Mode
	msr	cpsr, r0
	ldr	sp, =__arm9_sp_svc		@ Set SVC stack

	mov	r0, #0x1F		@ Switch to System Mode
	msr	cpsr, r0
	ldr	sp, =__arm9_sp_usr		@ Set user stack

	ldr	r0, =__arm9_bss_start	@ Clear BSS section to 0x00
	ldr	r1, =__arm9_bss_end
	sub	r1, r1, r0
	bl	ClearMem

	mov	r0, #0			@ int argc
	mov	r1, #0			@ char *argv[]
	ldr	r3, =arm9_main
	bl	_blx_r3_stub		@ jump to user code

	@ If the user ever returns, restart
	b	_arm9_start

@---------------------------------------------------------------------------------
_blx_r3_stub:
@---------------------------------------------------------------------------------
	bx	r3


@---------------------------------------------------------------------------------
@ Clear memory to 0x00 if length != 0
@  r0 = Start Address
@  r1 = Length
@---------------------------------------------------------------------------------
ClearMem:
@---------------------------------------------------------------------------------
	mov	r2, #3			@ Round down to nearest word boundary
	add	r1, r1, r2		@ Shouldn't be needed
	bics	r1, r1, r2		@ Clear 2 LSB (and set Z)
	bxeq	lr			@ Quit if copy size is 0

	mov	r2, #0
ClrLoop:
	stmia	r0!, {r2}
	subs	r1, r1, #4
	bne	ClrLoop
	bx	lr

