#include <nds/asminc.h>

#define ITCM_foo_base 0x00000004

.align	4
.arm

@ Writes a new value to the ARM9 SCFG_CLK.
@ Must be called from ITCM.
@ Must wait 8 clock cycles after changing the value before returning.
@ https://problemkaputt.de/gbatek.htm#dsicontrolregistersscfg
@ Not what I experimentally observed, but better safe than sorry...
@ void _arm9_write_to_scfg_clk(uint16_t new_val);
BEGIN_ASM_FUNC _arm9_write_to_scfg_clk
	mov r1, #0x04000000
	add r1, #0x4000
	strh r0,[r1, #4]
	@ Wait 8 clock cycles... Could be done in a better way.
	mov r0, #8
	wait_loop:
	subs r0, r0, #1
	bne wait_loop
	bx lr

_arm9_write_to_scfg_clk_end:

bx_r1_caller:
	bx r1

@ Writes a new value to the ARM9 SCFG_CLK.
@ Copies the right function to ITCM and calls it.
@ void arm9_write_to_scfg_clk(uint16_t new_val);
BEGIN_ASM_FUNC arm9_write_to_scfg_clk
	push {r4, lr}

	mrc p15, 0, r3, c1, c0, 0		@ Store current Control Register
	push {r3}
	ldr r2,=#0x00041000				@ Enable ITCM back
	orr r3, r3, r2
	mvn r2, #1						@ Disabe MMU/PU
	and r3, r3, r2
	mcr p15, 0, r3, c1, c0, 0		@ Update Control Register

	mov r3, #3
	mcr p15, 0, r3, c5, c0, 3		@ Enable RW IAccess

	mov r1, #ITCM_foo_base			@ ITCM foo_base
	ldr r2,=_arm9_write_to_scfg_clk
	ldr r3,=_arm9_write_to_scfg_clk_end-_arm9_write_to_scfg_clk
	itcm_copy_loop:
	ldr r4,[r2, #0]
	str r4,[r1, #0]
	add r1, #4
	add r2, #4
	subs r3, r3, #4
	bne itcm_copy_loop
	mov r1, #ITCM_foo_base			@ ITCM foo_base
	@ Call _arm9_write_to_scfg_clk
	bl bx_r1_caller

	@ Clear back the ITCM
	mov r0, #0
	mov r1, #0x00000000				@ ITCM
	ldr r3,=#0x00008000				@ Wipe ITCM back
	itcm_clear_loop:
	str r0,[r1, #0]
	add r1, #4
	subs r3, r3, #4
	bne itcm_clear_loop

	mov r3, #0                  
	mcr p15, 0, r3, c7, c5, 0		@ Flush ICache
	mcr p15, 0, r3, c5, c0, 3		@ Disable IAccess

	pop {r3}
	mcr p15, 0, r3, c1, c0, 0		@ Restore old Control Register

	pop {r4, pc}
