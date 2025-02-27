/*-----------------------------------------------------------------
 boot.c

 BootLoader
 Loads a file into memory and runs it

 All resetMemory and startBinary functions are based
 on the MultiNDS loader by Darkain.
 Original source available at:
 http://cvs.sourceforge.net/viewcvs.py/ndslib/ndslib/examples/loader/boot/main.cpp

License:
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

Helpful information:
 This code runs from VRAM bank C on ARM7
------------------------------------------------------------------*/

#include <nds/ndstypes.h>
#include <nds/dma.h>
#include <nds/system.h>
#include <nds/interrupts.h>
#include <nds/timers.h>
#define ARM9
#undef ARM7
#include <nds/memory.h>
#include <nds/arm9/video.h>
#include <nds/arm9/input.h>
#undef ARM9
#define ARM7
#include <nds/arm7/audio.h>
#include <string.h> // memcmp
#include "common/tonccpy.h"
#include "sdmmc.h"
#include "fat.h"
#include "dldi_patcher.h"
#include "card.h"
#include "boot.h"

void arm7clearRAM();

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Important things
#define TEMP_MEM 0x02FFD000
#define TWL_HEAD 0x02FFE000
#define NDS_HEAD 0x02FFFE00
#define TEMP_ARM9_START_ADDRESS (*(vu32*)0x02FFFFF4)


const char* bootName = "BOOT.NDS";

extern unsigned long _start;
extern unsigned long storedFileCluster;
extern unsigned long initDisc;
extern unsigned long wantToPatchDLDI;
extern unsigned long argStart;
extern unsigned long argSize;
extern unsigned long dsiSD;
extern unsigned long dsiMode;
extern unsigned long loadFromRam;

bool sdRead = false;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Firmware stuff

#define FW_READ        0x03

void boot_readFirmware (uint32 address, uint8 * buffer, uint32 size) {
	uint32 index;

	// Read command
	while (REG_SPICNT & SPI_BUSY);
	REG_SPICNT = SPI_ENABLE | SPI_CONTINUOUS | SPI_DEVICE_NVRAM;
	REG_SPIDATA = FW_READ;
	while (REG_SPICNT & SPI_BUSY);

	// Set the address
	REG_SPIDATA = (address>>16) & 0xFF;
	while (REG_SPICNT & SPI_BUSY);
	REG_SPIDATA = (address>>8) & 0xFF;
	while (REG_SPICNT & SPI_BUSY);
	REG_SPIDATA = (address) & 0xFF;
	while (REG_SPICNT & SPI_BUSY);

	for (index = 0; index < size; index++) {
		REG_SPIDATA = 0;
		while (REG_SPICNT & SPI_BUSY);
		buffer[index] = REG_SPIDATA & 0xFF;
	}
	REG_SPICNT = 0;
}


static inline void copyLoop (u32* dest, const u32* src, u32 size) {
	size = (size +3) & ~3;
	do {
		*dest++ = *src++;
	} while (size -= 4);
}

//#define resetCpu() __asm volatile("\tswi 0x000000\n");

/*-------------------------------------------------------------------------
passArgs_ARM7
Copies the command line arguments to the end of the ARM9 binary,
then sets a flag in memory for the loaded NDS to use
--------------------------------------------------------------------------*/
void passArgs_ARM7 (void) {
	u32 ARM9_DST = *((u32*)(NDS_HEAD + 0x028));
	u32 ARM9_LEN = *((u32*)(NDS_HEAD + 0x02C));
	u32* argSrc;
	u32* argDst;

	if (!argStart || !argSize) return;

	if ( ARM9_DST == 0 && ARM9_LEN == 0) {
		ARM9_DST = *((u32*)(NDS_HEAD + 0x038));
		ARM9_LEN = *((u32*)(NDS_HEAD + 0x03C));
	}

	argSrc = (u32*)(argStart + (int)&_start);

	argDst = (u32*)((ARM9_DST + ARM9_LEN + 3) & ~3);		// Word aligned

	if (dsiMode && (*(u8*)(NDS_HEAD + 0x012) & BIT(1)))
	{
		u32 ARM9i_DST = *((u32*)(TWL_HEAD + 0x1C8));
		u32 ARM9i_LEN = *((u32*)(TWL_HEAD + 0x1CC));
		if (ARM9i_LEN)
		{
			u32* argDst2 = (u32*)((ARM9i_DST + ARM9i_LEN + 3) & ~3);		// Word aligned
			if (argDst2 > argDst)
				argDst = argDst2;
		}
	}

	copyLoop(argDst, argSrc, argSize);

	__system_argv->argvMagic = ARGV_MAGIC;
	__system_argv->commandLine = (char*)argDst;
	__system_argv->length = argSize;
}




/*-------------------------------------------------------------------------
resetMemory_ARM7
Clears all of the NDS's RAM that is visible to the ARM7
Written by Darkain.
Modified by Chishm:
 * Added STMIA clear mem loop
--------------------------------------------------------------------------*/
void resetMemory_ARM7 (void)
{
	int i;
	u8 settings1, settings2;
	u32 settingsOffset = 0;

	REG_IME = 0;

	for (i=0; i<16; i++) {
		SCHANNEL_CR(i) = 0;
		SCHANNEL_TIMER(i) = 0;
		SCHANNEL_SOURCE(i) = 0;
		SCHANNEL_LENGTH(i) = 0;
	}

	REG_SOUNDCNT = 0;
	REG_SNDCAP0CNT = 0;
	REG_SNDCAP1CNT = 0;

	REG_SNDCAP0DAD = 0;
	REG_SNDCAP0LEN = 0;
	REG_SNDCAP1DAD = 0;
	REG_SNDCAP1LEN = 0;

	//clear out ARM7 DMA channels and timers
	for (i=0; i<4; i++) {
		DMA_CR(i) = 0;
		DMA_SRC(i) = 0;
		DMA_DEST(i) = 0;
		TIMER_CR(i) = 0;
		TIMER_DATA(i) = 0;
	}

	arm7clearRAM();
	// clear most of EWRAM - except after RAM end - 0xc000, which has the bootstub
	if (dsiMode && loadFromRam) {
		toncset((void*)0x02004000, 0, 0x7FC000);
		toncset((void*)0x02D00000, 0, 0x2F4000);
	} else {
		toncset((void*)0x02004000, 0, dsiMode ? 0xFF0000 : 0x3F0000);
	}
	*(u32*)(0x2FFFD9C) = 0;	// Clear exception handler

	REG_IE = 0;
	REG_IF = ~0;
	(*(vu32*)(0x04000000-4)) = 0;  //IRQ_HANDLER ARM7 version
	(*(vu32*)(0x04000000-8)) = ~0; //VBLANK_INTR_WAIT_FLAGS, ARM7 version
	REG_POWERCNT = 1;  //turn off power to stuff

	// Get settings location
	boot_readFirmware((u32)0x00020, (u8*)&settingsOffset, 0x2);
	settingsOffset *= 8;

	// Reload DS Firmware settings
	boot_readFirmware(settingsOffset + 0x070, &settings1, 0x1);
	boot_readFirmware(settingsOffset + 0x170, &settings2, 0x1);

	if ((settings1 & 0x7F) == ((settings2+1) & 0x7F)) {
		boot_readFirmware(settingsOffset + 0x000, (u8*)0x02FFFC80, 0x70);
	} else {
		boot_readFirmware(settingsOffset + 0x100, (u8*)0x02FFFC80, 0x70);
	}

	((vu32*)0x040044f0)[2] = 0x202DDD1D;
	((vu32*)0x040044f0)[3] = 0xE1A00005;
	while((*(vu32*)0x04004400) & 0x2000000);

}


void loadBinary_ARM7 (u32 fileCluster)
{
	if (loadFromRam) {
		//u32 ARM9_SRC = *(u32*)(TWL_HEAD+0x20);
		char* ARM9_DST = (char*)*(u32*)(TWL_HEAD+0x28);
		u32 ARM9_LEN = *(u32*)(TWL_HEAD+0x2C);
		//char* ARM7_SRC = (char*)*(u32*)(TWL_HEAD+0x30);
		char* ARM7_DST = (char*)*(u32*)(TWL_HEAD+0x38);
		u32 ARM7_LEN = *(u32*)(TWL_HEAD+0x3C);

		tonccpy(ARM9_DST, (char*)0x02800000, ARM9_LEN);
		tonccpy(ARM7_DST, (char*)0x02B80000, ARM7_LEN);

		// first copy the header to its proper location, excluding
		// the ARM9 start address, so as not to start it
		TEMP_ARM9_START_ADDRESS = *(u32*)(TWL_HEAD+0x24);		// Store for later
		*(u32*)(TWL_HEAD+0x24) = 0;
		dmaCopyWords(3, (void*)TWL_HEAD, (void*)NDS_HEAD, 0x170);
		*(u32*)(TWL_HEAD+0x24) = TEMP_ARM9_START_ADDRESS;

		if (dsiMode && (*(u8*)(TWL_HEAD+0x12) > 0))
		{
			//char* ARM9i_SRC = (char*)*(u32*)(TWL_HEAD+0x1C0);
			char* ARM9i_DST = (char*)*(u32*)(TWL_HEAD+0x1C8);
			u32 ARM9i_LEN = *(u32*)(TWL_HEAD+0x1CC);
			//char* ARM7i_SRC = (char*)*(u32*)(TWL_HEAD+0x1D0);
			char* ARM7i_DST = (char*)*(u32*)(TWL_HEAD+0x1D8);
			u32 ARM7i_LEN = *(u32*)(TWL_HEAD+0x1DC);

			if (ARM9i_LEN)
				tonccpy(ARM9i_DST, (char*)0x02C00000, ARM9i_LEN);
			if (ARM7i_LEN)
				tonccpy(ARM7i_DST, (char*)0x02C80000, ARM7i_LEN);
		}

		toncset((void*)0x02800000, 0, 0x500000);

		return;
	}

	u32 ndsHeader[0x170>>2];

	// read NDS header
	fileRead ((char*)ndsHeader, fileCluster, 0, 0x170);
	// read ARM9 info from NDS header
	u32 ARM9_SRC = ndsHeader[0x020>>2];
	char* ARM9_DST = (char*)ndsHeader[0x028>>2];
	u32 ARM9_LEN = ndsHeader[0x02C>>2];
	// read ARM7 info from NDS header
	u32 ARM7_SRC = ndsHeader[0x030>>2];
	char* ARM7_DST = (char*)ndsHeader[0x038>>2];
	u32 ARM7_LEN = ndsHeader[0x03C>>2];

	// Load binaries into memory
	fileRead(ARM9_DST, fileCluster, ARM9_SRC, ARM9_LEN);
	fileRead(ARM7_DST, fileCluster, ARM7_SRC, ARM7_LEN);

	// first copy the header to its proper location, excluding
	// the ARM9 start address, so as not to start it
	TEMP_ARM9_START_ADDRESS = ndsHeader[0x024>>2];		// Store for later
	ndsHeader[0x024>>2] = 0;
	dmaCopyWords(3, (void*)ndsHeader, (void*)NDS_HEAD, 0x170);

	if (dsiMode && (ndsHeader[0x10>>2]&BIT(16+1)))
	{
		// Read full TWL header
		fileRead((char*)TWL_HEAD, fileCluster, 0, 0x1000);

		u32 ARM9i_SRC = *(u32*)(TWL_HEAD+0x1C0);
		char* ARM9i_DST = (char*)*(u32*)(TWL_HEAD+0x1C8);
		u32 ARM9i_LEN = *(u32*)(TWL_HEAD+0x1CC);
		u32 ARM7i_SRC = *(u32*)(TWL_HEAD+0x1D0);
		char* ARM7i_DST = (char*)*(u32*)(TWL_HEAD+0x1D8);
		u32 ARM7i_LEN = *(u32*)(TWL_HEAD+0x1DC);

		if (ARM9i_LEN)
			fileRead(ARM9i_DST, fileCluster, ARM9i_SRC, ARM9i_LEN);
		if (ARM7i_LEN)
			fileRead(ARM7i_DST, fileCluster, ARM7i_SRC, ARM7i_LEN);
	}
}

/*-------------------------------------------------------------------------
startBinary_ARM7
Jumps to the ARM7 NDS binary in sync with the display and ARM9
Written by Darkain.
Modified by Chishm:
 * Removed MultiNDS specific stuff
--------------------------------------------------------------------------*/
void startBinary_ARM7 (void) {
	REG_IME=0;
	while(REG_VCOUNT!=191);
	while(REG_VCOUNT==191);
	// copy NDS ARM9 start address into the header, starting ARM9
	*((vu32*)0x02FFFE24) = TEMP_ARM9_START_ADDRESS;
	ARM9_START_FLAG = 1;
	// Start ARM7
	VoidFn arm7code = *(VoidFn*)(0x2FFFE34);
	arm7code();
}

void mpu_reset();
void mpu_reset_end();

int main (void) {
#ifdef NO_DLDI
	dsiSD = true;
	dsiMode = true;
#endif
#ifndef NO_SDMMC
	sdRead = dsiSD;
#endif
	u32 fileCluster = storedFileCluster;
	if (!loadFromRam) {
		// Init card
		if (!FAT_InitFiles(initDisc))
		{
			return -1;
		}
		if ((fileCluster < CLUSTER_FIRST) || (fileCluster >= CLUSTER_EOF)) 	/* Invalid file cluster specified */
		{
			fileCluster = getBootFileCluster(bootName);
		}
		if (fileCluster == CLUSTER_FREE)
		{
			return -1;
		}
	}

	if ((REG_SCFG_EXT != 0) && (memcmp((u8*)0x02000000, (u8*)0x02400000, 0x4000) != 0)
	 && (*(u8*)0x02400400 != 0) && (*(u8*)0x02400401 == 0) && (*(u8*)0x02400402 == 0) && (*(u8*)0x02400404 == 0) && (*(u8*)0x02400448 != 0)) {
		// Restore TWLCFG backup
		tonccpy((u8*)0x02000000, (u8*)0x02400000, 0x4000);
		toncset((u8*)0x02400000, 0, 0x4000);
	}

	// ARM9 clears its memory part 2
	// copy ARM9 function to RAM, and make the ARM9 jump to it
	copyLoop((void*)TEMP_MEM, (void*)resetMemory2_ARM9, resetMemory2_ARM9_size);
	(*(vu32*)0x02FFFE24) = (u32)TEMP_MEM;	// Make ARM9 jump to the function
	// Wait until the ARM9 has completed its task
	while ((*(vu32*)0x02FFFE24) == (u32)TEMP_MEM);

	// ARM9 sets up mpu
	// copy ARM9 function to RAM, and make the ARM9 jump to it
	copyLoop((void*)TEMP_MEM, (void*)mpu_reset, mpu_reset_end - mpu_reset);
	(*(vu32*)0x02FFFE24) = (u32)TEMP_MEM;	// Make ARM9 jump to the function
	// Wait until the ARM9 has completed its task
	while ((*(vu32*)0x02FFFE24) == (u32)TEMP_MEM);

	// Get ARM7 to clear RAM
	resetMemory_ARM7();

	// ARM9 enters a wait loop
	// copy ARM9 function to RAM, and make the ARM9 jump to it
	copyLoop((void*)TEMP_MEM, (void*)startBinary_ARM9, startBinary_ARM9_size);
	(*(vu32*)0x02FFFE24) = (u32)TEMP_MEM;	// Make ARM9 jump to the function

	// Load the NDS file
	loadBinary_ARM7(fileCluster);

	sdRead = false;

#ifndef NO_DLDI
	// Patch with DLDI if desired
	if (wantToPatchDLDI) {
		dldiPatchBinary ((u8*)((u32*)NDS_HEAD)[0x0A], ((u32*)NDS_HEAD)[0x0B]);
	}
#endif

#ifndef NO_SDMMC
	if (dsiSD) {
		sdmmc_controller_init(true);
		*(vu16*)(SDMMC_BASE + REG_SDDATACTL32) &= 0xFFFDu;
		*(vu16*)(SDMMC_BASE + REG_SDDATACTL) &= 0xFFDDu;
		*(vu16*)(SDMMC_BASE + REG_SDBLKLEN32) = 0;
	}
#endif
	// Pass command line arguments to loaded program
	passArgs_ARM7();

	startBinary_ARM7();

	return 0;
}

