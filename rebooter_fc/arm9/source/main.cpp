#include <nds.h>

#include "autoboot.h"
#include "tonccpy.h"

static const char *unlaunchAutoLoadID = "AutoLoadInfo";
static char bootNdsPath[14] = {'s','d','m','c',':','/','b','o','o','t','.','n','d','s'};

//---------------------------------------------------------------------------------
int main(int argc, char **argv) {
//---------------------------------------------------------------------------------
	tonccpy((u8*)0x02000800, unlaunchAutoLoadID, 12);
	*(u16*)(0x0200080C) = 0x3F0;		// Unlaunch Length for CRC16 (fixed, must be 3F0h)
	*(u16*)(0x0200080E) = 0;			// Unlaunch CRC16 (empty)
	*(u32*)(0x02000810) = (BIT(0) | BIT(1));		// Load the title at 2000838h
													// Use colors 2000814h
	*(u16*)(0x02000814) = 0x7FFF;		// Unlaunch Upper screen BG color (0..7FFFh)
	*(u16*)(0x02000816) = 0x7FFF;		// Unlaunch Lower screen BG color (0..7FFFh)
	toncset((u8*)0x02000818, 0, 0x20+0x208+0x1C0);		// Unlaunch Reserved (zero)
	int i2 = 0;
	for (int i = 0; i < (int)sizeof(bootNdsPath); i++) {
		*(u8*)(0x02000838+i2) = bootNdsPath[i];				// Unlaunch Device:/Path/Filename.ext (16bit Unicode,end by 0000h)
		i2 += 2;
	}
	while (*(u16*)(0x0200080E) == 0) {	// Keep running, so that CRC16 isn't 0
		*(u16*)(0x0200080E) = swiCRC16(0xFFFF, (void*)0x02000810, 0x3F0);		// Unlaunch CRC16
	}

	tonccpy((u32*)0x02000300, autoboot_bin, 0x020);
	DC_FlushAll();					// Fix the throwback to System Menu bug
	fifoSendValue32(FIFO_USER_01, 1); // Reboot TWiLight Menu++ back in DSi mode

	for (int i = 0; i < 10; i++) {
		swiWaitForVBlank();
	}

	return 0;
}
