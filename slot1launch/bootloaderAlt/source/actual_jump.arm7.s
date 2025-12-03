@ __attribute__((noreturn)) void arm7_actual_jump(void* fn)
@ Clears most registers and properly sets the other ones.
@ Then calls fn.
@ ARM9 and ARM7 versions of the function need to be separate
@ to prevent issues with cross-calling.

#include <nds/asminc.h>

#define DEFAULT_SP_DSI 0x03FFFF80

.arm

BEGIN_ASM_FUNC arm7_actual_jump
	mov lr, r0
	mov r0, #0
	mov r1, #0
	mov r2, #0
	mov r3, #0
	mov r4, #0
	mov r5, #0
	mov r6, #0
	mov r7, #0
	mov r8, #0
	mov r9, #0
	mov r10, #0
	mov r11, #0
	mov r12, #0
	ldr sp,=#DEFAULT_SP_DSI
	bx lr
