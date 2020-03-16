/*
 main.arm7.c
 
 By Michael Chisholm (Chishm)
 
 All resetMemory and startBinary functions are based 
 on the MultiNDS loader by Darkain.
 Original source available at:
 http://cvs.sourceforge.net/viewcvs.py/ndslib/ndslib/examples/loader/boot/main.cpp

 License:
    NitroHax -- Cheat tool for the Nintendo DS
    Copyright (C) 2008  Michael "Chishm" Chisholm

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef ARM7
# define ARM7
#endif
#include <nds/ndstypes.h>
#include <nds/arm7/codec.h>
#include <nds/system.h>
#include <nds/interrupts.h>
#include <nds/timers.h>
#include <nds/dma.h>
#include <nds/arm7/audio.h>
#include <nds/ipc.h>
#include <string.h>

// #include <nds/registers_alt.h>
// #include <nds/memory.h>
// #include <nds/card.h>
// #include <stdio.h>

#ifndef NULL
#define NULL 0
#endif

#include "common.h"
#include "tonccpy.h"
#include "read_card.h"
#include "module_params.h"
#include "cardengine_arm7_bin.h"
#include "hook.h"
#include "find.h"


extern u32 dsiMode;
extern u32 language;
extern u32 sdAccess;
extern u32 scfgUnlock;
extern u32 twlMode;
extern u32 twlClock;
extern u32 boostVram;
extern u32 soundFreq;
extern u32 runCardEngine;

extern bool arm9_runCardEngine;

bool useTwlCfg = false;
int twlCfgLang = 0;

bool gameSoftReset = false;

void arm7_clearmem (void* loc, size_t len);
extern void ensureBinaryDecompressed(const tNDSHeader* ndsHeader, module_params_t* moduleParams);

static const u32 cheatDataEndSignature[2] = {0xCF000000, 0x00000000};

// Module params
static const u32 moduleParamsSignature[2] = {0xDEC00621, 0x2106C0DE};

static u32 chipID;

static module_params_t* moduleParams;

u32* findModuleParamsOffset(const tNDSHeader* ndsHeader) {
	//dbg_printf("findModuleParamsOffset:\n");

	u32* moduleParamsOffset = findOffset(
			(u32*)ndsHeader->arm9destination, ndsHeader->arm9binarySize,
			moduleParamsSignature, 2
		);
	return moduleParamsOffset;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Important things
#define NDS_HEADER         0x027FFE00
#define NDS_HEADER_SDK5    0x02FFFE00 // __NDSHeader
#define NDS_HEADER_POKEMON 0x027FF000

#define DSI_HEADER         0x027FE000
#define DSI_HEADER_SDK5    0x02FFE000 // __DSiHeader

#define ENGINE_LOCATION_ARM7  	0x037C0000

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Used for debugging purposes
/* Disabled for now. Re-enable to debug problems
static void errorOutput (u32 code) {
	// Wait until the ARM9 is ready
	while (arm9_stateFlag != ARM9_READY);
	// Set the error code, then tell ARM9 to display it
	arm9_errorCode = code;
	arm9_errorClearBG = true;
	arm9_stateFlag = ARM9_DISPERR;
	// Stop
	while(1);
}
*/

static void debugOutput (u32 code) {
	// Wait until the ARM9 is ready
	while (arm9_stateFlag != ARM9_READY);
	// Set the error code, then tell ARM9 to display it
	arm9_errorCode = code;
	arm9_errorClearBG = false;
	arm9_stateFlag = ARM9_DISPERR;
	// Wait for completion
	while (arm9_stateFlag != ARM9_READY);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Firmware stuff

static void my_readUserSettings(tNDSHeader* ndsHeader) {
	PERSONAL_DATA slot1;
	PERSONAL_DATA slot2;

	short slot1count, slot2count; //u8
	short slot1CRC, slot2CRC;

	u32 userSettingsBase;

	// Get settings location
	readFirmware(0x20, &userSettingsBase, 2);

	u32 slot1Address = userSettingsBase * 8;
	u32 slot2Address = userSettingsBase * 8 + 0x100;

	// Reload DS Firmware settings
	readFirmware(slot1Address, &slot1, sizeof(PERSONAL_DATA)); //readFirmware(slot1Address, personalData, 0x70);
	readFirmware(slot2Address, &slot2, sizeof(PERSONAL_DATA)); //readFirmware(slot2Address, personalData, 0x70);
	readFirmware(slot1Address + 0x70, &slot1count, 2); //readFirmware(slot1Address + 0x70, &slot1count, 1);
	readFirmware(slot2Address + 0x70, &slot2count, 2); //readFirmware(slot1Address + 0x70, &slot2count, 1);
	readFirmware(slot1Address + 0x72, &slot1CRC, 2);
	readFirmware(slot2Address + 0x72, &slot2CRC, 2);

	// Default to slot 1 user settings
	void *currentSettings = &slot1;

	short calc1CRC = swiCRC16(0xFFFF, &slot1, sizeof(PERSONAL_DATA));
	short calc2CRC = swiCRC16(0xFFFF, &slot2, sizeof(PERSONAL_DATA));

	// Bail out if neither slot is valid
	if (calc1CRC != slot1CRC && calc2CRC != slot2CRC) {
		return;
	}

	// If both slots are valid pick the most recent
	if (calc1CRC == slot1CRC && calc2CRC == slot2CRC) { 
		currentSettings = (slot2count == ((slot1count + 1) & 0x7f) ? &slot2 : &slot1); //if ((slot1count & 0x7F) == ((slot2count + 1) & 0x7F)) {
	} else {
		if (calc2CRC == slot2CRC) {
			currentSettings = &slot2;
		}
	}

	PERSONAL_DATA* personalData = (PERSONAL_DATA*)((u32)__NDSHeader - (u32)ndsHeader + (u32)PersonalData); //(u8*)((u32)ndsHeader - 0x180)

	tonccpy(PersonalData, currentSettings, sizeof(PERSONAL_DATA));

	if (useTwlCfg && (language == 0xFF || language == -1)) {
		language = twlCfgLang;
	}

	if (language >= 0 && language <= 7) {
		// Change language
		personalData->language = language; //*(u8*)((u32)ndsHeader - 0x11C) = language;
	}

	if (personalData->language != 6 && ndsHeader->reserved1[8] == 0x80) {
		ndsHeader->reserved1[8] = 0;	// Patch iQue game to be region-free
		ndsHeader->headerCRC16 = swiCRC16(0xFFFF, ndsHeader, 0x15E);	// Fix CRC
	}
}

/*-------------------------------------------------------------------------
arm7_resetMemory
Clears all of the NDS's RAM that is visible to the ARM7
Written by Darkain.
Modified by Chishm:
 * Added STMIA clear mem loop
--------------------------------------------------------------------------*/
void arm7_resetMemory (void)
{
	int i;

	REG_IME = 0;

	for (i=0; i<16; i++) {
		SCHANNEL_CR(i) = 0;
		SCHANNEL_TIMER(i) = 0;
		SCHANNEL_SOURCE(i) = 0;
		SCHANNEL_LENGTH(i) = 0;
	}
	REG_SOUNDCNT = 0;

	// Clear out ARM7 DMA channels and timers
	for (i=0; i<4; i++) {
		DMA_CR(i) = 0;
		DMA_SRC(i) = 0;
		DMA_DEST(i) = 0;
		TIMER_CR(i) = 0;
		TIMER_DATA(i) = 0;
	}

	// Clear out FIFO
	REG_IPC_SYNC = 0;
	REG_IPC_FIFO_CR = IPC_FIFO_ENABLE | IPC_FIFO_SEND_CLEAR;
	REG_IPC_FIFO_CR = 0;

	// clear IWRAM - 037F:8000 to 0380:FFFF, total 96KiB
	arm7_clearmem ((void*)0x037F8000, 96*1024);
	
	// clear most of EXRAM - except before 0x023F0000, which has the cheat data
	toncset ((void*)0x02004000, 0, 0x3EC000);

	// clear more of EXRAM, skipping the cheat data section
	toncset ((void*)0x023F8000, 0, 0x6000);

	// clear last part of EXRAM
	toncset ((void*)0x02400000, 0, 0xC00000);

	REG_IE = 0;
	REG_IF = ~0;
	(*(vu32*)(0x04000000-4)) = 0;  //IRQ_HANDLER ARM7 version
	(*(vu32*)(0x04000000-8)) = ~0; //VBLANK_INTR_WAIT_FLAGS, ARM7 version
	REG_POWERCNT = 1;  //turn off power to stuffs

	useTwlCfg = (dsiMode && (*(u8*)0x02000400 & 0x0F) && (*(u8*)0x02000404 == 0));
	twlCfgLang = *(u8*)0x02000406;

	// Load FW header 
	//arm7_readFirmware((u32)0x000000, (u8*)0x027FF830, 0x20);
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

	// Touchscreen
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
	cdcWriteReg(CDC_SOUND, 0x26, volLevel);
	cdcWriteReg(CDC_SOUND, 0x27, volLevel);
	cdcWriteReg(CDC_SOUND, 0x2E, 0x03);
	cdcWriteReg(CDC_TOUCHCNT, 0x03, 0x00);
	cdcWriteReg(CDC_SOUND, 0x21, 0x20);
	cdcWriteReg(CDC_SOUND, 0x22, 0xF0);
	cdcReadReg (CDC_SOUND, 0x22);
	cdcWriteReg(CDC_SOUND, 0x22, 0x00);
	cdcWriteReg(CDC_CONTROL, 0x52, 0x80);
	cdcWriteReg(CDC_CONTROL, 0x51, 0x00);
	
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

	// Finish up!
	cdcReadReg (CDC_TOUCHCNT, 0x02);
	cdcWriteReg(CDC_TOUCHCNT, 0x02, 0x98);
	cdcWriteReg(0xFF, 0x05, 0x00); //writeTSC(0x00, 0xFF);

	// Power management
	writePowerManagement(PM_READ_REGISTER, 0x00); //*(unsigned char*)0x40001C2 = 0x80, 0x00; // read PWR[0]   ;<-- also part of TSC !
	writePowerManagement(PM_CONTROL_REG, 0x0D); //*(unsigned char*)0x40001C2 = 0x00, 0x0D; // PWR[0]=0Dh    ;<-- also part of TSC !
}

// SDK 5
static bool ROMsupportsDsiMode(const tNDSHeader* ndsHeader) {
	return (ndsHeader->unitCode > 0);
}

// SDK 5
static bool ROMisDsiEnhanced(const tNDSHeader* ndsHeader) {
	return (ndsHeader->unitCode == 0x02);
}

// SDK 5
static bool ROMisDsiExclusive(const tNDSHeader* ndsHeader) {
	return (ndsHeader->unitCode == 0x03);
}

int arm7_loadBinary (const tDSiHeader* dsiHeaderTemp) {
	u32 errorCode;
	
	// Init card
	errorCode = cardInit((sNDSHeaderExt*)dsiHeaderTemp, &chipID);
	if (errorCode) {
		return errorCode;
	}

	// Fix Pokemon games needing header data.
	tonccpy((u32*)NDS_HEADER_POKEMON, (u32*)NDS_HEADER, 0x170);

	char* romTid = (char*)NDS_HEADER_POKEMON+0xC;
	if (
		memcmp(romTid, "ADA", 3) == 0    // Diamond
		|| memcmp(romTid, "APA", 3) == 0 // Pearl
		|| memcmp(romTid, "CPU", 3) == 0 // Platinum
		|| memcmp(romTid, "IPK", 3) == 0 // HG
		|| memcmp(romTid, "IPG", 3) == 0 // SS
	) {
		// Make the Pokemon game code ADAJ.
		const char gameCodePokemon[] = { 'A', 'D', 'A', 'J' };
		tonccpy((char*)NDS_HEADER_POKEMON+0xC, gameCodePokemon, 4);
	}

	cardRead(dsiHeaderTemp->ndshdr.arm9romOffset, (u32*)dsiHeaderTemp->ndshdr.arm9destination, dsiHeaderTemp->ndshdr.arm9binarySize);
	cardRead(dsiHeaderTemp->ndshdr.arm7romOffset, (u32*)dsiHeaderTemp->ndshdr.arm7destination, dsiHeaderTemp->ndshdr.arm7binarySize);

	moduleParams = (module_params_t*)findModuleParamsOffset(&dsiHeaderTemp->ndshdr);

	return ERR_NONE;
}

static tNDSHeader* loadHeader(tDSiHeader* dsiHeaderTemp) {
	tNDSHeader* ndsHeader = (tNDSHeader*)(isSdk5(moduleParams) ? NDS_HEADER_SDK5 : NDS_HEADER);

	*ndsHeader = dsiHeaderTemp->ndshdr;
	if (dsiModeConfirmed) {
		tDSiHeader* dsiHeader = (tDSiHeader*)(isSdk5(moduleParams) ? DSI_HEADER_SDK5 : DSI_HEADER); // __DSiHeader
		*dsiHeader = *dsiHeaderTemp;
	}

	return ndsHeader;
}

/*-------------------------------------------------------------------------
arm7_startBinary
Jumps to the ARM7 NDS binary in sync with the display and ARM9
Written by Darkain, modified by Chishm.
--------------------------------------------------------------------------*/
void arm7_startBinary (void)
{
	REG_IME = 0;

	while(REG_VCOUNT!=191);
	while(REG_VCOUNT==191);
	
	// Get the ARM9 to boot
	arm9_stateFlag = ARM9_BOOTBIN;

	while(REG_VCOUNT!=191);
	while(REG_VCOUNT==191);

	// Start ARM7
	VoidFn arm7code = (VoidFn)ndsHeader->arm7executeAddress;
	arm7code();
}


void initMBK() {
	// give all DSI WRAM to arm7 at boot
	// this function have no effect with ARM7 SCFG locked
	
	// arm7 is master of WRAM-A, arm9 of WRAM-B & C
	REG_MBK9=0x3000000F;
	
	// WRAM-A fully mapped to arm7
	*((vu32*)REG_MBK1)=0x8185898D; // same as dsiware
	
	// WRAM-B fully mapped to arm7 // inverted order
	*((vu32*)REG_MBK2)=0x9195999D;
	*((vu32*)REG_MBK3)=0x8185898D;
	
	// WRAM-C fully mapped to arm7 // inverted order
	*((vu32*)REG_MBK4)=0x9195999D;
	*((vu32*)REG_MBK5)=0x8185898D;
	
	// WRAM mapped to the 0x3700000 - 0x37FFFFF area 
	// WRAM-A mapped to the 0x37C0000 - 0x37FFFFF area : 256k
	REG_MBK6=0x080037C0; // same as dsiware
	// WRAM-B mapped to the 0x3740000 - 0x37BFFFF area : 512k // why? only 256k real memory is there
	REG_MBK7=0x07C03740; // same as dsiware
	// WRAM-C mapped to the 0x3700000 - 0x373FFFF area : 256k
	REG_MBK8=0x07403700; // same as dsiware
}


/*void fixFlashcardForDSiMode(void) {
	if ((memcmp(ndsHeader->gameTitle, "PASS", 4) == 0)
	&& (memcmp(ndsHeader->gameCode, "ASME", 4) == 0))		// CycloDS Evolution
	{
		*(u16*)(0x0200197A) = 0xDF02;	// LZ77UnCompReadByCallbackWrite16bit
		*(u16*)(0x020409FA) = 0xDF02;	// LZ77UnCompReadByCallbackWrite16bit
	}
}*/

void fixDSBrowser(void) {
	extern void patchMpu(const tNDSHeader* ndsHeader, const module_params_t* moduleParams);
	patchMpu(ndsHeader, moduleParams);

	toncset((char*)0x02400000, 0xFF, 0xC0);
	*(u8*)0x024000B2 = 0;
	*(u8*)0x024000B3 = 0;
	*(u8*)0x024000B4 = 0;
	*(u8*)0x024000B5 = 0x24;
	*(u8*)0x024000B6 = 0x24;
	*(u8*)0x024000B7 = 0x24;
	*(u16*)0x024000BE = 0x7FFF;
	*(u16*)0x024000CE = 0x7FFF;

	// Opera RAM patch (ARM9)
	*(u32*)0x02003D48 = 0x2400000;
	*(u32*)0x02003D4C = 0x2400004;

	*(u32*)0x02010FF0 = 0x2400000;
	*(u32*)0x02010FF4 = 0x24000CE;

	*(u32*)0x020112AC = 0x2400080;

	*(u32*)0x020402BC = 0x24000C2;
	*(u32*)0x020402C0 = 0x24000C0;
	*(u32*)0x020402CC = 0x2FFFFFE;
	*(u32*)0x020402D0 = 0x2800000;
	*(u32*)0x020402D4 = 0x29FFFFF;
	*(u32*)0x020402D8 = 0x2BFFFFF;
	*(u32*)0x020402DC = 0x2FFFFFF;
	*(u32*)0x020402E0 = 0xD7FFFFF;	// ???
	toncset((char*)0x2800000, 0xFF, 0x800000);		// Fill fake MEP with FFs

	// Opera RAM patch (ARM7)
	*(u32*)0x0238C7BC = 0x2400000;
	*(u32*)0x0238C7C0 = 0x24000CE;

	//*(u32*)0x0238C950 = 0x2400000;
}


static void setMemoryAddress(const tNDSHeader* ndsHeader) {
	if (ROMsupportsDsiMode(ndsHeader)) {
	//	u8* deviceListAddr = (u8*)((u8*)0x02FFE1D4);
	//	tonccpy(deviceListAddr, deviceList_bin, deviceList_bin_len);

	//	const char *ndsPath = "nand:/dsiware.nds";
	//	tonccpy(deviceListAddr+0x3C0, ndsPath, sizeof(ndsPath));

		//tonccpy((u32*)0x02FFC000, (u32*)DSI_HEADER_SDK5, 0x1000);		// Make a duplicate of DSi header
		tonccpy((u32*)0x02FFFA80, (u32*)NDS_HEADER_SDK5, 0x160);	// Make a duplicate of DS header

		*(u32*)(0x02FFA680) = 0x02FD4D80;
		*(u32*)(0x02FFA684) = 0x00000000;
		*(u32*)(0x02FFA688) = 0x00001980;

		*(u32*)(0x02FFF00C) = 0x0000007F;
		*(u32*)(0x02FFF010) = 0x550E25B8;
		*(u32*)(0x02FFF014) = 0x02FF4000;
	}

    // Set memory values expected by loaded NDS
    // from NitroHax, thanks to Chism
	*((u32*)(isSdk5(moduleParams) ? 0x02fff800 : 0x027ff800)) = chipID;					// CurrentCardID
	*((u32*)(isSdk5(moduleParams) ? 0x02fff804 : 0x027ff804)) = chipID;					// Command10CardID
	*((u16*)(isSdk5(moduleParams) ? 0x02fff808 : 0x027ff808)) = ndsHeader->headerCRC16;	// Header Checksum, CRC-16 of [000h-15Dh]
	*((u16*)(isSdk5(moduleParams) ? 0x02fff80a : 0x027ff80a)) = ndsHeader->secureCRC16;	// Secure Area Checksum, CRC-16 of [ [20h]..7FFFh]

	// Copies of above
	*((u32*)(isSdk5(moduleParams) ? 0x02fffc00 : 0x027ffc00)) = chipID;					// CurrentCardID
	*((u32*)(isSdk5(moduleParams) ? 0x02fffc04 : 0x027ffc04)) = chipID;					// Command10CardID
	*((u16*)(isSdk5(moduleParams) ? 0x02fffc08 : 0x027ffc08)) = ndsHeader->headerCRC16;	// Header Checksum, CRC-16 of [000h-15Dh]
	*((u16*)(isSdk5(moduleParams) ? 0x02fffc0a : 0x027ffc0a)) = ndsHeader->secureCRC16;	// Secure Area Checksum, CRC-16 of [ [20h]..7FFFh]

	*((u16*)(isSdk5(moduleParams) ? 0x02fffc40 : 0x027ffc40)) = 0x1;						// Boot Indicator (Booted from card for SDK5) -- EXTREMELY IMPORTANT!!! Thanks to cReDiAr
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Main function

void arm7_main (void) {
	
	if (runCardEngine) {
		arm9_runCardEngine = runCardEngine;
		initMBK();
	}
	
	int errorCode;
	
	// Wait for ARM9 to at least start
	while (arm9_stateFlag < ARM9_START);

	debugOutput (ERR_STS_CLR_MEM);
	
	// Get ARM7 to clear RAM
	arm7_resetMemory();	

	debugOutput (ERR_STS_LOAD_BIN);

	if (!twlMode) {
		REG_SCFG_ROM = 0x703;	// Not running this prevents (some?) flashcards from running
	}

	tDSiHeader* dsiHeaderTemp = (tDSiHeader*)0x02FFC000;

	// Load the NDS file
	errorCode = arm7_loadBinary(dsiHeaderTemp);
	if (errorCode) {
		debugOutput(errorCode);
	}

	if (dsiMode) {
		if (twlMode == 2) {
			dsiModeConfirmed = twlMode;
		} else {
			dsiModeConfirmed = twlMode && ROMsupportsDsiMode(&dsiHeaderTemp->ndshdr);
		}
	}

	if (dsiModeConfirmed) {
		if (dsiHeaderTemp->arm9ibinarySize > 0) {
			cardRead(dsiHeaderTemp->arm9iromOffset, (u32*)dsiHeaderTemp->arm9idestination, dsiHeaderTemp->arm9ibinarySize);
		}
		if (dsiHeaderTemp->arm7ibinarySize > 0) {
			cardRead(dsiHeaderTemp->arm7iromOffset, (u32*)dsiHeaderTemp->arm7idestination, dsiHeaderTemp->arm7ibinarySize);
		}
	}

	ndsHeader = loadHeader(dsiHeaderTemp);

	my_readUserSettings(ndsHeader); // Header has to be loaded first

	if (dsiMode && !dsiModeConfirmed) {
		NDSTouchscreenMode();
		*(u16*)0x4000500 = 0x807F;

		if (twlClock) {
			REG_SCFG_CLK = 0x0181;
		} else {
			REG_SCFG_CLK = 0x0180;
		}
		if (!sdAccess) {
			REG_SCFG_EXT = 0x93FBFB06;
		}
	}

	if (*(u32*)(NDS_HEADER+0xC) == 0x50524255) {
		fixDSBrowser();
	}

	if ((*(u32*)(NDS_HEADER+0xC) & 0x00FFFFFF) == 0x52544E	// Download Play ROMs
	|| (*(u32*)(NDS_HEADER+0xC) & 0x00FFFFFF) == 0x4D5341		// Super Mario 64 DS
	|| (*(u32*)(NDS_HEADER+0xC) & 0x00FFFFFF) == 0x434D41		// Mario Kart DS
	|| (*(u32*)(NDS_HEADER+0xC) & 0x00FFFFFF) == 0x443241		// New Super Mario Bros.
	|| (*(u32*)(NDS_HEADER+0xC) & 0x00FFFFFF) == 0x5A5241		// Rockman ZX/MegaMan ZX
	|| (*(u32*)(NDS_HEADER+0xC) & 0x00FFFFFF) == 0x574B41		// Kirby Squeak Squad/Mouse Attack
	|| (*(u32*)(NDS_HEADER+0xC) & 0x00FFFFFF) == 0x585A59		// Rockman ZX Advent/MegaMan ZX Advent
	|| (*(u32*)(NDS_HEADER+0xC) & 0x00FFFFFF) == 0x5A3642)	// Rockman Zero Collection/MegaMan Zero Collection
	{
		gameSoftReset = true;
	}

	if (runCardEngine) {
		copyLoop ((u32*)ENGINE_LOCATION_ARM7, (u32*)cardengine_arm7_bin, cardengine_arm7_bin_size);
		errorCode = hookNdsRetail(ndsHeader, (u32*)ENGINE_LOCATION_ARM7);
		if(errorCode == ERR_NONE) {
			nocashMessage("card hook Sucessfull");
		} else {
			nocashMessage("error during card hook");
			debugOutput(errorCode);
		}
		if (*(u32*)(0x023F0000) != 0xCF000000) {
			u32* cheatDataOffset = findOffset(
				(u32*)ENGINE_LOCATION_ARM7, cardengine_arm7_bin_size,
				cheatDataEndSignature, 2
			);
			if (cheatDataOffset) {
				copyLoop (cheatDataOffset, (u32*)0x023F0000, 0x8000);	// Copy cheat data
			}
		}
	}
	toncset ((void*)0x023F0000, 0, 0x8000);		// Clear cheat data from main memory

	debugOutput (ERR_STS_START);

	arm9_boostVram = boostVram;
	arm9_scfgUnlock = scfgUnlock;
	//arm9_isSdk5 = isSdk5(moduleParams);

	if (!scfgUnlock && !dsiModeConfirmed) {
		// lock SCFG
		REG_SCFG_EXT &= ~(1UL << 31);
	}

	while (arm9_stateFlag != ARM9_READY);
	arm9_stateFlag = ARM9_SETSCFG;
	while (arm9_stateFlag != ARM9_READY);

	setMemoryAddress(ndsHeader);

	arm7_startBinary();

	while (1);
}

