#define ARM9
#undef ARM7

#include <nds/ndstypes.h>
#include <nds/dma.h>
#include <nds/system.h>
#include <nds/ipc.h>
#include <nds/interrupts.h>
#include <nds/timers.h>

#include <nds/memory.h>
#include <nds/arm9/video.h>
#include <nds/arm9/input.h>

#include "boot.h"

/*-------------------------------------------------------------------------
resetMemory2_ARM9
Clears the ARM9's DMA channels and resets video memory
Written by Darkain.
Modified by Chishm:
 * Changed MultiNDS specific stuff
--------------------------------------------------------------------------*/
void __attribute__ ((long_call)) __attribute__((naked)) __attribute__((noreturn)) resetMemory2_ARM9 (void) 
{
 	register int i, reg;
  
	//clear out ARM9 DMA channels
	for (i=0; i<4; i++) {
		DMA_CR(i) = 0;
		DMA_SRC(i) = 0;
		DMA_DEST(i) = 0;
		TIMER_CR(i) = 0;
		TIMER_DATA(i) = 0;
		for (reg=0; reg<0x1c; reg+=4)*((vu32*)(0x04004104 + ((i*0x1c)+reg))) = 0;//Reset NDMA.
	}

	// Clear out FIFO
	REG_IPC_SYNC = 0;
	REG_IPC_FIFO_CR = IPC_FIFO_ENABLE | IPC_FIFO_SEND_CLEAR;
	REG_IPC_FIFO_CR = 0;

	VRAM_CR = (VRAM_CR & 0xffff0000) | 0x00008080 ;
	
	REG_DISPSTAT = 0;

	VRAM_A_CR = 0;
	VRAM_B_CR = 0;
// Don't mess with the ARM7's VRAM
//	VRAM_C_CR = 0;
	VRAM_D_CR = 0;
	VRAM_E_CR = 0;
	VRAM_F_CR = 0;
	VRAM_G_CR = 0;
	VRAM_H_CR = 0;
	VRAM_I_CR = 0;
	REG_POWERCNT  = 0x820F;

	//set shared ram to ARM7
	WRAM_CR = 0x03;

	// Return to passme loop
	*((vu32*)0x02FFFE04) = (u32)0xE59FF018;		// ldr pc, 0x02FFFE24
	*((vu32*)0x02FFFE24) = (u32)0x02FFFE04;		// Set ARM9 Loop address

	asm volatile(
		"\tbx %0\n"
		: : "r" (0x02FFFE04)
	);
	while (1);
}

void __attribute__ ((long_call)) __attribute__((naked)) __attribute__((noreturn)) clearMasterBright_ARM9 (void) 
{
	vu16 *mainregs = (vu16*)0x04000000;
	vu16 *subregs = (vu16*)0x04001000;
	
	for (register int i=0; i<43; i++) {
		mainregs[i] = 0;
		subregs[i] = 0;
	}
	
	u16 mode = 1 << 14;

	*(vu16*)(0x0400006C + (0x1000 * 0)) = 0 + mode;
	*(vu16*)(0x0400006C + (0x1000 * 1)) = 0 + mode;

	// Return to passme loop
	*((vu32*)0x02FFFE04) = (u32)0xE59FF018;		// ldr pc, 0x02FFFE24
	*((vu32*)0x02FFFE24) = (u32)0x02FFFE04;		// Set ARM9 Loop address

	asm volatile(
		"\tbx %0\n"
		: : "r" (0x02FFFE04)
	);
	while (1);
}

/*-------------------------------------------------------------------------
startBinary_ARM9
Jumps to the ARM9 NDS binary in sync with the display and ARM7
Written by Darkain.
Modified by Chishm:
 * Removed MultiNDS specific stuff
--------------------------------------------------------------------------*/
void __attribute__ ((long_call)) __attribute__((noreturn)) __attribute__((naked)) startBinary_ARM9 (void)
{
	REG_IME=0;
	REG_EXMEMCNT = 0xE880;
	// set ARM9 load address to 0 and wait for it to change again
	ARM9_START_FLAG = 0;
	while (REG_VCOUNT!=191);
	while (REG_VCOUNT==191);
	while (ARM9_START_FLAG != 1);
	VoidFn arm9code = *(VoidFn*)(0x2FFFE24);
	arm9code();
	while (1);
}

