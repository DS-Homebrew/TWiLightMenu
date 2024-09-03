@---------------------------------------------------------------------------------
	.section ".init"
	.global _start
	.global dsiMode
	.global language
	.global sdAccess
	.global scfgUnlock
	.global twlMode
	.global twlClock
	.global boostVram
	.global twlTouch
	.global soundFreq
	.global sleepMode
	.global runCardEngine
@---------------------------------------------------------------------------------
	.align	4
	.arm
@---------------------------------------------------------------------------------
_start:
@---------------------------------------------------------------------------------
	b	startUp

dsiMode:
	.word	0x00000000
language:
	.word	0x00000000
sdAccess:
	.word	0x00000000
scfgUnlock:
	.word	0x00000000
twlMode:
	.word	0x00000000
twlClock:
	.word	0x00000000
boostVram:
	.word	0x00000000
twlTouch:
	.word	0x00000000
soundFreq:
	.word	0x00000000
sleepMode:
	.word	0x00000000
runCardEngine:
	.word	0x00000000

startUp:
	mov	r0, #0x04000000		@ IME = 0;
	add	r0, r0, #0x208
	strh	r0, [r0]

	mov	r0, #0x12		@ Switch to IRQ Mode
	msr	cpsr, r0
	ldr	sp, =__sp_irq		@ Set IRQ stack

	mov	r0, #0x13		@ Switch to SVC Mode
	msr	cpsr, r0
	ldr	sp, =__sp_svc		@ Set SVC stack

	mov	r0, #0x1F		@ Switch to System Mode
	msr	cpsr, r0
	ldr	sp, =__sp_usr		@ Set user stack

	ldr	r0, =__bss_start	@ Clear BSS section to 0x00
	ldr	r1, =__bss_end
	sub	r1, r1, r0
	bl	ClearMem
	
@ Load ARM9 region into main RAM
	ldr	r1, =__arm9_source_start
	ldr	r2, =__arm9_start	
	ldr	r3, =__arm9_source_end
	sub	r3, r3, r1
	bl	CopyMem

@ Start ARM9 binary
	ldr	r0, =0x02FFFE24	
	ldr	r1, =_arm9_start
	str	r1, [r0]

	mov	r0, #0			@ int argc
	mov	r1, #0			@ char *argv[]
	ldr	r3, =arm7_main
	bl	_blx_r3_stub		@ jump to user code
		
	@ If the user ever returns, restart
	b	_start

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

@---------------------------------------------------------------------------------
@ Copy memory if length	!= 0
@  r1 = Source Address
@  r2 = Dest Address
@  r4 = Dest Address + Length
@---------------------------------------------------------------------------------
CopyMemCheck:
@---------------------------------------------------------------------------------
	sub	r3, r4, r2		@ Is there any data to copy?
@---------------------------------------------------------------------------------
@ Copy memory
@  r1 = Source Address
@  r2 = Dest Address
@  r3 = Length
@---------------------------------------------------------------------------------
CopyMem:
@---------------------------------------------------------------------------------
	mov	r0, #3			@ These commands are used in cases where
	add	r3, r3, r0		@ the length is not a multiple of 4,
	bics	r3, r3, r0		@ even though it should be.
	bxeq	lr			@ Length is zero, so exit
CIDLoop:
	ldmia	r1!, {r0}
	stmia	r2!, {r0}
	subs	r3, r3, #4
	bne	CIDLoop
	bx	lr

@---------------------------------------------------------------------------------
	.align
	.pool
	.end
@---------------------------------------------------------------------------------
