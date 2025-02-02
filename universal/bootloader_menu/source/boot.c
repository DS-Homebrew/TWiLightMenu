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
#include <nds/ipc.h>
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
#include <nds/arm7/codec.h>
#include <string.h> // memcmp
#include "dmaTwl.h"
#include "common/tonccpy.h"
#include "sdmmc.h"
#include "i2c.h"
#include "fat.h"
#include "dldi_patcher.h"
#include "card.h"
#include "boot.h"

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Important things
#define TEMP_MEM 0x02FFD000
#define TWL_HEAD 0x02FFE000
#define NDS_HEAD 0x02FFFE00
#define TEMP_ARM9_START_ADDRESS (*(vu32*)0x02FFFFF4)
#define REG_GPIO_WIFI *(vu16*)0x4004C04


const char* bootName = "BOOT.NDS";

extern unsigned long _start;
extern unsigned long storedFileCluster;
extern unsigned long initDisc;
extern unsigned long wantToPatchDLDI;
extern unsigned long argStart;
extern unsigned long argSize;
extern unsigned long dsiSD;
extern unsigned long dsiMode;
extern unsigned long clearMasterBright;
extern unsigned long dsMode;
extern unsigned long loadFromRam;
extern unsigned long language;
extern unsigned long tscTgds;

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

	if (ARM9_DST == 0 && ARM9_LEN == 0) {
		ARM9_DST = *((u32*)(NDS_HEAD + 0x038));
		ARM9_LEN = *((u32*)(NDS_HEAD + 0x03C));
	}

	argSrc = (u32*)(argStart + (int)&_start);

	argDst = (u32*)((ARM9_DST + ARM9_LEN + 3) & ~3);		// Word aligned

	if (ARM9_LEN > 0x380000) {
		argDst = (u32*)(TEMP_MEM - ((argSize/4)*4));
	} else
	if (dsiMode && (*(u8*)(NDS_HEAD + 0x012) & BIT(1))) {
		u32 ARM9i_DST = *((u32*)(TWL_HEAD + 0x1C8));
		u32 ARM9i_LEN = *((u32*)(TWL_HEAD + 0x1CC));
		if (ARM9i_LEN) {
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




static void initMBK_dsiMode(void) {
	// This function has no effect with ARM7 SCFG locked
	*(vu32*)REG_MBK1 = *(u32*)0x02FFE180;
	*(vu32*)REG_MBK2 = *(u32*)0x02FFE184;
	*(vu32*)REG_MBK3 = *(u32*)0x02FFE188;
	*(vu32*)REG_MBK4 = *(u32*)0x02FFE18C;
	*(vu32*)REG_MBK5 = *(u32*)0x02FFE190;
	REG_MBK6 = *(u32*)0x02FFE1A0;
	REG_MBK7 = *(u32*)0x02FFE1A4;
	REG_MBK8 = *(u32*)0x02FFE1A8;
	REG_MBK9 = *(u32*)0x02FFE1AC;
}

void memset_addrs_arm7(u32 start, u32 end)
{
	if (!dsiMode && !(REG_SCFG_EXT & BIT(16))) {
		toncset((u32*)start, 0, ((int)end - (int)start));
		return;
	}
	dma_twlFill32(0, 0, (u32*)start, ((int)end - (int)start));
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
	int i, reg;
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
		for (reg=0; reg<0x1c; reg+=4)*((vu32*)(0x04004104 + ((i*0x1c)+reg))) = 0;//Reset NDMA.
	}

	// Clear out FIFO
	REG_IPC_SYNC = 0;
	REG_IPC_FIFO_CR = IPC_FIFO_ENABLE | IPC_FIFO_SEND_CLEAR;
	REG_IPC_FIFO_CR = 0;

	memset_addrs_arm7(0x03800000 - 0x8000, 0x03800000 + (dsiMode ? 0xC000 : 0x10000)); // clear exclusive IWRAM
	// clear most of EWRAM - except after RAM end - 0xc000, which has the bootstub
	if (dsiMode && loadFromRam) {
		memset_addrs_arm7(0x02004000, 0x02800000);
		memset_addrs_arm7(0x02D00000, 0x02FF4000);
	} else {
		memset_addrs_arm7(0x02004000, dsiMode&&!dsMode ? 0x02FF4000 : 0x023F4000);
	}
	*(u32*)(0x2FFFD9C) = 0;	// Clear exception handler

	REG_IE = 0;
	REG_IF = ~0;
	REG_AUXIE = 0;
	REG_AUXIF = ~0;
	*(vu32*)0x0380FFFC = 0;  // IRQ_HANDLER ARM7 version
	*(vu32*)0x0380FFF8 = 0; // VBLANK_INTR_WAIT_FLAGS, ARM7 version
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

	if (language >= 0 && language <= 7) {
		// Change language
		*(u8*)((u32)NDS_HEAD - 0x11C) = language;
	}

	((vu32*)0x040044f0)[2] = 0x202DDD1D;
	((vu32*)0x040044f0)[3] = 0xE1A00005;
	while ((*(vu32*)0x04004400) & 0x2000000);

}


u32 ROM_TID;
u32 ARM9_SRC;
u8 dsiFlags;

void loadBinary_ARM7 (u32 fileCluster)
{
	if (loadFromRam) {
		ARM9_SRC = *(u32*)(TWL_HEAD+0x20);
		char* ARM9_DST = (char*)*(u32*)(TWL_HEAD+0x28);
		u32 ARM9_LEN = *(u32*)(TWL_HEAD+0x2C);
		//char* ARM7_SRC = (char*)*(u32*)(TWL_HEAD+0x30);
		char* ARM7_DST = (char*)*(u32*)(TWL_HEAD+0x38);
		u32 ARM7_LEN = *(u32*)(TWL_HEAD+0x3C);

		ROM_TID = *(u32*)(TWL_HEAD+0xC);

		tonccpy(ARM9_DST, (char*)0x02800000, ARM9_LEN);
		tonccpy(ARM7_DST, (char*)0x02B80000, ARM7_LEN);

		// first copy the header to its proper location, excluding
		// the ARM9 start address, so as not to start it
		TEMP_ARM9_START_ADDRESS = *(u32*)(TWL_HEAD+0x24);		// Store for later
		*(u32*)(TWL_HEAD+0x24) = 0;
		dmaCopyWords(3, (void*)TWL_HEAD, (void*)NDS_HEAD, 0x170);
		*(u32*)(TWL_HEAD+0x24) = TEMP_ARM9_START_ADDRESS;

		dsiFlags = *(u8*)(TWL_HEAD+0x1BF);
		if (!dsMode && dsiMode && (*(u8*)(TWL_HEAD+0x12) > 0)) {
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

			initMBK_dsiMode();
		}

		toncset((void*)0x02800000, 0, 0x500000);

		return;
	}

	u32 ndsHeader[0x170>>2];

	// read NDS header
	fileRead ((char*)ndsHeader, fileCluster, 0, 0x170);
	fileRead ((char*)&dsiFlags, fileCluster, 0x1BF, 1);
	// read ARM9 info from NDS header
	ARM9_SRC = ndsHeader[0x020>>2];
	char* ARM9_DST = (char*)ndsHeader[0x028>>2];
	u32 ARM9_LEN = ndsHeader[0x02C>>2];
	// read ARM7 info from NDS header
	u32 ARM7_SRC = ndsHeader[0x030>>2];
	char* ARM7_DST = (char*)ndsHeader[0x038>>2];
	u32 ARM7_LEN = ndsHeader[0x03C>>2];

	ROM_TID = ndsHeader[0x00C>>2];

	// Load binaries into memory
	fileRead(ARM9_DST, fileCluster, ARM9_SRC, ARM9_LEN);
	fileRead(ARM7_DST, fileCluster, ARM7_SRC, ARM7_LEN);

	// first copy the header to its proper location, excluding
	// the ARM9 start address, so as not to start it
	TEMP_ARM9_START_ADDRESS = ndsHeader[0x024>>2];		// Store for later
	ndsHeader[0x024>>2] = 0;
	dmaCopyWords(3, (void*)ndsHeader, (void*)NDS_HEAD, 0x170);

	if (!dsMode && dsiMode && (ndsHeader[0x10>>2]&BIT(16+1))) {
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

		initMBK_dsiMode();
	}
}

static void NDSTouchscreenMode(void) {
	//unsigned char * *(unsigned char*)0x40001C0=		(unsigned char*)0x40001C0;
	//unsigned char * *(unsigned char*)0x40001C0byte2=(unsigned char*)0x40001C1;
	//unsigned char * *(unsigned char*)0x40001C2=	(unsigned char*)0x40001C2;
	//unsigned char * I2C_DATA=	(unsigned char*)0x4004500;
	//unsigned char * I2C_CNT=	(unsigned char*)0x4004501;

	u8 volLevel;
	
	//if (fifoCheckValue32(FIFO_MAXMOD)) {
	//	// special setting (when found special gamecode)
	//	volLevel = 0xAC;
	//} else {
		// normal setting (for any other gamecodes)
		volLevel = 0xA7;
	//}

	const bool noSgba = (strncmp((const char*)0x04FFFA00, "no$gba", 6) == 0);

	// Touchscreen
	if (noSgba) {
		cdcReadReg (0x63, 0x00);
		cdcWriteReg(CDC_CONTROL, 0x3A, 0x00);
		cdcReadReg (CDC_CONTROL, 0x51);
		cdcReadReg (CDC_TOUCHCNT, 0x02);
		cdcReadReg (CDC_CONTROL, 0x3F);
		cdcReadReg (CDC_SOUND, 0x28);
		cdcReadReg (CDC_SOUND, 0x2A);
		cdcReadReg (CDC_SOUND, 0x2E);
		cdcWriteReg(CDC_CONTROL, 0x52, 0x80);
		cdcWriteReg(CDC_CONTROL, 0x40, 0x0C);
		cdcWriteReg(CDC_SOUND, 0x24, 0xFF);
		cdcWriteReg(CDC_SOUND, 0x25, 0xFF);
		cdcWriteReg(CDC_SOUND, 0x26, 0x7F);
		cdcWriteReg(CDC_SOUND, 0x27, 0x7F);
		cdcWriteReg(CDC_SOUND, 0x28, 0x4A);
		cdcWriteReg(CDC_SOUND, 0x29, 0x4A);
		cdcWriteReg(CDC_SOUND, 0x2A, 0x10);
		cdcWriteReg(CDC_SOUND, 0x2B, 0x10);
		cdcWriteReg(CDC_CONTROL, 0x51, 0x00);
		cdcReadReg (CDC_TOUCHCNT, 0x02);
		cdcWriteReg(CDC_TOUCHCNT, 0x02, 0x98);
		cdcWriteReg(CDC_SOUND, 0x23, 0x00);
		cdcWriteReg(CDC_SOUND, 0x1F, 0x14);
		cdcWriteReg(CDC_SOUND, 0x20, 0x14);
		cdcWriteReg(CDC_CONTROL, 0x3F, 0x00);
		cdcReadReg (CDC_CONTROL, 0x0B);
		cdcWriteReg(CDC_CONTROL, 0x05, 0x00);
		cdcWriteReg(CDC_CONTROL, 0x0B, 0x01);
		cdcWriteReg(CDC_CONTROL, 0x0C, 0x02);
		cdcWriteReg(CDC_CONTROL, 0x12, 0x01);
		cdcWriteReg(CDC_CONTROL, 0x13, 0x02);
		cdcWriteReg(CDC_SOUND, 0x2E, 0x00);
		cdcWriteReg(CDC_CONTROL, 0x3A, 0x60);
		cdcWriteReg(CDC_CONTROL, 0x01, 0x01);
		cdcWriteReg(CDC_CONTROL, 0x39, 0x66);
		cdcReadReg (CDC_SOUND, 0x20);
		cdcWriteReg(CDC_SOUND, 0x20, 0x10);
		cdcWriteReg(CDC_CONTROL, 0x04, 0x00);
		cdcWriteReg(CDC_CONTROL, 0x12, 0x81);
		cdcWriteReg(CDC_CONTROL, 0x13, 0x82);
		cdcWriteReg(CDC_CONTROL, 0x51, 0x82);
		cdcWriteReg(CDC_CONTROL, 0x51, 0x00);
		cdcWriteReg(CDC_CONTROL, 0x04, 0x03);
		cdcWriteReg(CDC_CONTROL, 0x05, 0xA1);
		cdcWriteReg(CDC_CONTROL, 0x06, 0x15);
		cdcWriteReg(CDC_CONTROL, 0x0B, 0x87);
		cdcWriteReg(CDC_CONTROL, 0x0C, 0x83);
		cdcWriteReg(CDC_CONTROL, 0x12, 0x87);
		cdcWriteReg(CDC_CONTROL, 0x13, 0x83);
		cdcReadReg (CDC_TOUCHCNT, 0x10);
		cdcWriteReg(CDC_TOUCHCNT, 0x10, 0x08);
		cdcWriteReg(0x04, 0x08, 0x7F);
		cdcWriteReg(0x04, 0x09, 0xE1);
		cdcWriteReg(0x04, 0x0A, 0x80);
		cdcWriteReg(0x04, 0x0B, 0x1F);
		cdcWriteReg(0x04, 0x0C, 0x7F);
		cdcWriteReg(0x04, 0x0D, 0xC1);
		cdcWriteReg(CDC_CONTROL, 0x41, 0x08);
		cdcWriteReg(CDC_CONTROL, 0x42, 0x08);
		cdcWriteReg(CDC_CONTROL, 0x3A, 0x00);
		cdcWriteReg(0x04, 0x08, 0x7F);
		cdcWriteReg(0x04, 0x09, 0xE1);
		cdcWriteReg(0x04, 0x0A, 0x80);
		cdcWriteReg(0x04, 0x0B, 0x1F);
		cdcWriteReg(0x04, 0x0C, 0x7F);
		cdcWriteReg(0x04, 0x0D, 0xC1);
		cdcWriteReg(CDC_SOUND, 0x2F, 0x2B);
		cdcWriteReg(CDC_SOUND, 0x30, 0x40);
		cdcWriteReg(CDC_SOUND, 0x31, 0x40);
		cdcWriteReg(CDC_SOUND, 0x32, 0x60);
		cdcReadReg (CDC_CONTROL, 0x74);
		cdcWriteReg(CDC_CONTROL, 0x74, 0x02);
		cdcReadReg (CDC_CONTROL, 0x74);
		cdcWriteReg(CDC_CONTROL, 0x74, 0x10);
		cdcReadReg (CDC_CONTROL, 0x74);
		cdcWriteReg(CDC_CONTROL, 0x74, 0x40);
		cdcWriteReg(CDC_SOUND, 0x21, 0x20);
		cdcWriteReg(CDC_SOUND, 0x22, 0xF0);
		cdcReadReg (CDC_CONTROL, 0x51);
		cdcReadReg (CDC_CONTROL, 0x3F);
		cdcWriteReg(CDC_CONTROL, 0x3F, 0xD4);
		cdcWriteReg(CDC_SOUND, 0x23, 0x44);
		cdcWriteReg(CDC_SOUND, 0x1F, 0xD4);
		cdcWriteReg(CDC_SOUND, 0x28, 0x4E);
		cdcWriteReg(CDC_SOUND, 0x29, 0x4E);
		cdcWriteReg(CDC_SOUND, 0x24, 0x9E);
		cdcWriteReg(CDC_SOUND, 0x25, 0x9E);
		cdcWriteReg(CDC_SOUND, 0x20, 0xD4);
		cdcWriteReg(CDC_SOUND, 0x2A, 0x14);
		cdcWriteReg(CDC_SOUND, 0x2B, 0x14);
		cdcWriteReg(CDC_SOUND, 0x26, 0xA7);
		cdcWriteReg(CDC_SOUND, 0x27, 0xA7);
		cdcWriteReg(CDC_CONTROL, 0x40, 0x00);
		cdcWriteReg(CDC_CONTROL, 0x3A, 0x60);
	}
	cdcWriteReg(CDC_SOUND, 0x26, volLevel);
	cdcWriteReg(CDC_SOUND, 0x27, volLevel);
	cdcWriteReg(CDC_SOUND, 0x2E, 0x03);
	cdcWriteReg(CDC_TOUCHCNT, 0x03, 0x00);
	cdcWriteReg(CDC_SOUND, 0x21, 0x20);
	cdcWriteReg(CDC_SOUND, 0x22, 0xF0);
	cdcWriteReg(CDC_SOUND, 0x22, 0x70);
	cdcWriteReg(CDC_CONTROL, 0x52, 0x80);
	cdcWriteReg(CDC_CONTROL, 0x51, 0x00);

	if (noSgba) {
		// Set remaining values
		cdcWriteReg(CDC_CONTROL, 0x03, 0x44);
		cdcWriteReg(CDC_CONTROL, 0x0D, 0x00);
		cdcWriteReg(CDC_CONTROL, 0x0E, 0x80);
		cdcWriteReg(CDC_CONTROL, 0x0F, 0x80);
		cdcWriteReg(CDC_CONTROL, 0x10, 0x08);
		cdcWriteReg(CDC_CONTROL, 0x14, 0x80);
		cdcWriteReg(CDC_CONTROL, 0x15, 0x80);
		cdcWriteReg(CDC_CONTROL, 0x16, 0x04);
		cdcWriteReg(CDC_CONTROL, 0x1A, 0x01);
		cdcWriteReg(CDC_CONTROL, 0x1E, 0x01);
		cdcWriteReg(CDC_CONTROL, 0x24, 0x80);
		cdcWriteReg(CDC_CONTROL, 0x33, 0x34);
		cdcWriteReg(CDC_CONTROL, 0x34, 0x32);
		cdcWriteReg(CDC_CONTROL, 0x35, 0x12);
		cdcWriteReg(CDC_CONTROL, 0x36, 0x03);
		cdcWriteReg(CDC_CONTROL, 0x37, 0x02);
		cdcWriteReg(CDC_CONTROL, 0x38, 0x03);
		cdcWriteReg(CDC_CONTROL, 0x3C, 0x19);
		cdcWriteReg(CDC_CONTROL, 0x3D, 0x05);
		cdcWriteReg(CDC_CONTROL, 0x44, 0x0F);
		cdcWriteReg(CDC_CONTROL, 0x45, 0x38);
		cdcWriteReg(CDC_CONTROL, 0x49, 0x00);
		cdcWriteReg(CDC_CONTROL, 0x4A, 0x00);
		cdcWriteReg(CDC_CONTROL, 0x4B, 0xEE);
		cdcWriteReg(CDC_CONTROL, 0x4C, 0x10);
		cdcWriteReg(CDC_CONTROL, 0x4D, 0xD8);
		cdcWriteReg(CDC_CONTROL, 0x4E, 0x7E);
		cdcWriteReg(CDC_CONTROL, 0x4F, 0xE3);
		cdcWriteReg(CDC_CONTROL, 0x58, 0x7F);
		cdcWriteReg(CDC_CONTROL, 0x74, 0xD2);
		cdcWriteReg(CDC_CONTROL, 0x75, 0x2C);
		cdcWriteReg(CDC_SOUND, 0x22, 0x70);
		cdcWriteReg(CDC_SOUND, 0x2C, 0x20);
	}

	// Finish up!
	cdcReadReg (CDC_TOUCHCNT, 0x02);
	cdcWriteReg(CDC_TOUCHCNT, 0x02, 0x98);
	cdcWriteReg(0xFF, 0x05, 0x00); //writeTSC(0x00, 0xFF);

	// Power management
	writePowerManagement(PM_READ_REGISTER, 0x00); //*(unsigned char*)0x40001C2 = 0x80, 0x00; // read PWR[0]   ;<-- also part of TSC !
	writePowerManagement(PM_CONTROL_REG, 0x0D); //*(unsigned char*)0x40001C2 = 0x00, 0x0D; // PWR[0]=0Dh    ;<-- also part of TSC !
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
	while (REG_VCOUNT!=191);
	while (REG_VCOUNT==191);
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
	sdRead = (dsiSD && dsiMode);
#endif
	toncset((u32*)0x06000000, 0, 0x8000);
	if (wantToPatchDLDI) {
		if (*(u32*)0x02FF4184 == 0x69684320) { // DLDI ' Chi' string in bootstub space + bootloader in DLDI driver space
			const u16 dldiFileSize = 1 << *(u8*)0x02FF418D;
			tonccpy((u32*)0x06000000, (u32*)0x02FF4180, dldiFileSize);
			dldiRelocateBinary();

			toncset((u32*)0x02FF4000, 0, 0x8180); // Clear bootstub + DLDI driver
		} else if (*(u32*)0x02FF8004 == 0x69684320) { // DLDI ' Chi' string
			const u16 dldiFileSize = 1 << *(u8*)0x02FF800D;
			tonccpy((u32*)0x06000000, (u32*)0x02FF8000, (dldiFileSize > 0x4000) ? 0x4000 : dldiFileSize);
			dldiClearBss();
		} else if (*(u32*)0x02FF8000 == 0x53535A4C) { // LZ77 flag
			dldiDecompressBinary();
		} else {
			return -1;
		}
	}

	u32 fileCluster = storedFileCluster;
	if (!loadFromRam) {
		// Init card
		if (!FAT_InitFiles(initDisc)) {
			return -1;
		}
		if ((fileCluster < CLUSTER_FIRST) || (fileCluster >= CLUSTER_EOF)) 	/* Invalid file cluster specified */
		{
			fileCluster = getBootFileCluster(bootName);
		}
		if (fileCluster == CLUSTER_FREE) {
			return -1;
		}
	}

	// ARM9 clears its memory part 2
	// copy ARM9 function to RAM, and make the ARM9 jump to it
	copyLoop((void*)TEMP_MEM, (void*)resetMemory2_ARM9, resetMemory2_ARM9_size);
	(*(vu32*)0x02FFFE24) = (u32)TEMP_MEM;	// Make ARM9 jump to the function
	// Wait until the ARM9 has completed its task
	while ((*(vu32*)0x02FFFE24) == (u32)TEMP_MEM);

	if (clearMasterBright) {
		// ARM9 clears master brightness
		// copy ARM9 function to RAM, and make the ARM9 jump to it
		copyLoop((void*)TEMP_MEM, (void*)clearMasterBright_ARM9, clearMasterBright_ARM9_size);
		(*(vu32*)0x02FFFE24) = (u32)TEMP_MEM;	// Make ARM9 jump to the function
		// Wait until the ARM9 has completed its task
		while ((*(vu32*)0x02FFFE24) == (u32)TEMP_MEM);
	}

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

	// Fix for Pictochat and DLP
	if (ROM_TID == 0x41444E48 || ROM_TID == 0x41454E48
	 || ROM_TID == 0x43444E48 || ROM_TID == 0x43454E48
	 || ROM_TID == 0x4B444E48 || ROM_TID == 0x4B454E48) {
		(*(vu16*)0x02FFFCFA) = 0x1041;	// NoCash: channel ch1+7+13
	}

	char ndsHeaderTitle[12];
	tonccpy(ndsHeaderTitle, (char*)0x02FFFE00, 12);

	if ((dsiMode && ((ARM9_SRC==0x4000 && !(dsiFlags & BIT(0))) || dsMode || tscTgds))
	|| (!dsiMode && REG_SCFG_EXT != 0 && memcmp(ndsHeaderTitle, "TWLMENUPP", 9) != 0 && memcmp(ndsHeaderTitle, "NDSBOOTSTRAP", 12) != 0)) {
		NDSTouchscreenMode();
		*(vu16*)0x4000500 = 0x807F;
		if (dsMode) REG_GPIO_WIFI |= BIT(8);	// Old NDS-Wifi mode
		i2cWriteRegister(I2C_PM, I2CREGPM_MMCPWR, 0);		// Press power button for auto-reset
		i2cWriteRegister(I2C_PM, I2CREGPM_RESETFLAG, 1);	// Bootflag = Warmboot/SkipHealthSafety
		if (dsMode || tscTgds) {
			REG_SCFG_ROM = 0x703;								// NTR BIOS
		}
		if (dsMode && REG_SCFG_ROM != 0x703) {
			*(u32*)0x3FFFFC8 = 0x7884;	// Fix sound pitch table
		}
		if (dsMode) {
			REG_SCFG_EXT = 0x12A03000;
		}
	}

#ifndef NO_DLDI
	// Patch with DLDI if desired
	if (wantToPatchDLDI) {
		dldiPatchBinary ((u8*)((u32*)NDS_HEAD)[0x0A], ((u32*)NDS_HEAD)[0x0B]);
		toncset((u32*)0x06000000, 0, 0x8000);
	}
#endif

#ifndef NO_SDMMC
	if (dsiSD && !dsMode && dsiMode) {
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

