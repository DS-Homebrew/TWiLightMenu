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

#include "blocks_codec.h"

// #include <nds/registers_alt.h>
// #include <nds/memory.h>
// #include <nds/card.h>
// #include <stdio.h>

#ifndef NULL
#define NULL 0
#endif

#define REG_GPIO_WIFI *(vu16*)0x4004C04

//#define FULL_DSI_MODE_ENABLED

#include "common.h"
#include "dmaTwl.h"
#include "common/tonccpy.h"
#include "read_card.h"
#include "module_params.h"
#include "cardengine_arm7_bin.h"
#include "hook.h"
#include "find.h"

#ifdef FULL_DSI_MODE_ENABLED
#include "gm9i/crypto.h"
#include "gm9i/f_xy.h"
#include "twltool/dsi.h"
#include "u128_math.h"
#endif

extern bool __dsimode;
extern u32 dsiMode;
extern u32 language;
extern u32 sdAccess;
extern u32 scfgUnlock;
extern u32 twlMode;
extern u32 twlClock;
extern u32 boostVram;
extern u32 twlTouch;
extern u32 soundFreq;
extern u32 sleepMode;
extern u32 runCardEngine;

extern bool arm9_runCardEngine;

u16 scfgRomBak = 0;

bool my_isDSiMode() {
	return ((vu8)scfgRomBak == 1);
}

bool useTwlCfg = false;
int twlCfgLang = 0;

bool gameSoftReset = false;

void arm7_clearmem (void* loc, size_t len);

static const u32 cheatDataEndSignature[2] = {0xCF000000, 0x00000000};

// Module params
static const u32 moduleParamsSignature[2] = {0xDEC00621, 0x2106C0DE};

// Sleep input write
static const u32 sleepInputWriteEndSignature1[2]     = {0x04000136, 0x027FFFA8};
static const u32 sleepInputWriteEndSignature5[2]     = {0x04000136, 0x02FFFFA8};
static const u32 sleepInputWriteSignature[1]         = {0x13A04902};
static const u16 sleepInputWriteBeqSignatureThumb[1] = {0xD000};

static u32 chipID;

const char* getRomTid(const tNDSHeader* ndsHeader) {
	//u32 ROM_TID = *(u32*)ndsHeader->gameCode;
	static char romTid[5];
	strncpy(romTid, ndsHeader->gameCode, 4);
	romTid[4] = '\0';
	return romTid;
}

static module_params_t* moduleParams;

u32* findModuleParamsOffset(const tNDSHeader* ndsHeader) {
	//dbg_printf("findModuleParamsOffset:\n");

	u32* moduleParamsOffset = findOffset(
			(u32*)ndsHeader->arm9destination, ndsHeader->arm9binarySize,
			moduleParamsSignature, 2
		);
	return moduleParamsOffset;
}

u32* findSleepInputWriteOffset(const tNDSHeader* ndsHeader, const module_params_t* moduleParams) {
	// dbg_printf("findSleepInputWriteOffset:\n");

	u32* offset = NULL;
	u32* endOffset = findOffset(
		(u32*)ndsHeader->arm7destination, ndsHeader->arm7binarySize,
		(moduleParams->sdk_version > 0x5000000) ? sleepInputWriteEndSignature5 : sleepInputWriteEndSignature1, 2
	);
	if (endOffset) {
		offset = findOffsetBackwards(
			endOffset, 0x38,
			sleepInputWriteSignature, 1
		);
		if (!offset) {
			u32 thumbOffset = (u32)findOffsetBackwardsThumb(
				(u16*)endOffset, 0x30,
				sleepInputWriteBeqSignatureThumb, 1
			);
			if (thumbOffset) {
				thumbOffset += 2;
				offset = (u32*)thumbOffset;
			}
		}
	}
	/* if (offset) {
		dbg_printf("Sleep input write found\n");
	} else {
		dbg_printf("Sleep input write not found\n");
	}

	dbg_printf("\n"); */
	return offset;
}

static void patchSleepInputWrite(const tNDSHeader* ndsHeader, const module_params_t* moduleParams) {
	if (sleepMode) {
		return;
	}

	u32* offset = findSleepInputWriteOffset(ndsHeader, moduleParams);
	if (!offset) {
		return;
	}

	if (*offset == 0x13A04902) {
		*offset = 0xE1A00000; // nop
	} else {
		u16* offsetThumb = (u16*)offset;
		*offsetThumb = 0x46C0; // nop
	}

	/* dbg_printf("Sleep input write location : ");
	dbg_hexa((u32)offset);
	dbg_printf("\n\n"); */
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
	while (1);
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

void initMBK() {
	// arm7 is master of WRAM-A, arm9 of WRAM-B & C
	REG_MBK9=0x3000000F;

	// WRAM-A fully mapped to arm7
	*((vu32*)REG_MBK1)=0x8D898581; // same as dsiware

	// WRAM-B fully mapped to arm9 // inverted order
	*((vu32*)REG_MBK2)=0x8C888480;
	*((vu32*)REG_MBK3)=0x9C989490;

	// WRAM-C fully mapped to arm9 // inverted order
	*((vu32*)REG_MBK4)=0x8C888480;
	*((vu32*)REG_MBK5)=0x9C989490;

	// WRAM mapped to the 0x3700000 - 0x37FFFFF area 
	// WRAM-A mapped to the 0x3000000 - 0x303FFFF area : 256k
	REG_MBK6=0x00403000;
	// WRAM-B mapped to the 0x3740000 - 0x37BFFFF area : 512k // why? only 256k real memory is there
	REG_MBK7=0x07C03740; // same as dsiware
	// WRAM-C mapped to the 0x3700000 - 0x373FFFF area : 256k
	REG_MBK8=0x07403700; // same as dsiware
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
	if (!my_isDSiMode()) {
		toncset((u32*)start, 0, ((int)end - (int)start));
		return;
	}
	dma_twlFill32(0, 0, (u32*)start, ((int)end - (int)start));
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
	int i, reg;

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

	// Clear out ARM7 DMA channels and timers
	for (i=0; i<4; i++) {
		DMA_CR(i) = 0;
		DMA_SRC(i) = 0;
		DMA_DEST(i) = 0;
		TIMER_CR(i) = 0;
		TIMER_DATA(i) = 0;
		if (my_isDSiMode()) {
			for (reg=0; reg<0x1c; reg+=4)*((u32*)(0x04004104 + ((i*0x1c)+reg))) = 0;//Reset NDMA.
		}
	}

	REG_RCNT = 0;

	// Clear out FIFO
	REG_IPC_SYNC = 0;
	REG_IPC_FIFO_CR = IPC_FIFO_ENABLE | IPC_FIFO_SEND_CLEAR;
	REG_IPC_FIFO_CR = 0;

	if (my_isDSiMode()) {
		memset_addrs_arm7(0x03000000, 0x0380FFC0);
		memset_addrs_arm7(0x0380FFD0, 0x03800000 + 0x10000);
	} else {
		memset_addrs_arm7(0x03800000 - 0x8000, 0x03800000 + 0x10000);
	}

	// clear most of EXRAM - except before 0x023F0000, which has the cheat data
	memset_addrs_arm7(0x02004000, 0x023DA000);
	memset_addrs_arm7(0x023DB000, 0x023F0000);

	// clear more of EXRAM, skipping the cheat data section
	toncset ((void*)0x023F8000, 0, 0x8000);

	if(my_isDSiMode() || swiIsDebugger())
		memset_addrs_arm7(0x02400000, 0x02800000); // Clear the rest of EXRAM

	if (my_isDSiMode()) {
		// clear last part of EXRAM
		memset_addrs_arm7(0x02800000, 0x02FFD7BC); // Leave eMMC CID intact
		memset_addrs_arm7(0x02FFD7CC, 0x03000000);
	}

	REG_IE = 0;
	REG_IF = ~0;
	REG_AUXIE = 0;
	REG_AUXIF = ~0;
	(*(vu32*)(0x04000000-4)) = 0;  //IRQ_HANDLER ARM7 version
	(*(vu32*)(0x04000000-8)) = ~0; //VBLANK_INTR_WAIT_FLAGS, ARM7 version
	REG_POWERCNT = 1;  //turn off power to stuffs

	useTwlCfg = (my_isDSiMode() && (*(u8*)0x02000400 & 0x0F) && (*(u8*)0x02000401 == 0) && (*(u8*)0x02000402 == 0) && (*(u8*)0x02000404 == 0));
	twlCfgLang = *(u8*)0x02000406;

	// Load FW header 
	//readFirmware((u32)0x000000, (u8*)0x027FF830, 0x20);
}

static void NDSTouchscreenMode(void) {
	//unsigned char * *(unsigned char*)0x40001C0=		(unsigned char*)0x40001C0;
	//unsigned char * *(unsigned char*)0x40001C0byte2=(unsigned char*)0x40001C1;
	//unsigned char * *(unsigned char*)0x40001C2=	(unsigned char*)0x40001C2;
	//unsigned char * I2C_DATA=	(unsigned char*)0x4004500;
	//unsigned char * I2C_CNT=	(unsigned char*)0x4004501;

	bool specialSetting = false;
	u8 volLevel;
	
	/*static const char list[][4] = {
		"ABX",	// NTR-ABXE Bomberman Land Touch!
		"YO9",	// NTR-YO9J Bokura no TV Game Kentei - Pikotto! Udedameshi
		"ALH",	// NTR-ALHE Flushed Away
		"ACC",	// NTR-ACCE Cooking Mama
		"YCQ",	// NTR-YCQE Cooking Mama 2 - Dinner with Friends
		"YYK",	// NTR-YYKE Trauma Center - Under the Knife 2
		"AZW",	// NTR-AZWE WarioWare - Touched!
		"AKA",	// NTR-AKAE Rub Rabbits!, The
		"AN9",	// NTR-AN9E Little Mermaid - Ariel's Undersea Adventure, The
		"AKE",	// NTR-AKEJ Keroro Gunsou - Enshuu da Yo! Zenin Shuugou Part 2
		"YFS",	// NTR-YFSJ Frogman Show - DS Datte, Shouganaijanai, The
		"YG8",	// NTR-YG8E Yu-Gi-Oh! World Championship 2008
		"AY7",	// NTR-AY7E Yu-Gi-Oh! World Championship 2007
		"YON",	// NTR-YONJ Minna no DS Seminar - Kantan Ongakuryoku
		"A5H",	// NTR-A5HE Interactive Storybook DS - Series 2
		"A5I",	// NTR-A5IE Interactive Storybook DS - Series 3
		"AMH",	// NTR-AMHE Metroid Prime Hunters
		"A3T",	// NTR-A3TE Tak - The Great Juju Challenge
		"YBO",	// NTR-YBOE Boogie
		"ADA",	// NTR-ADAE PKMN Diamond
		"APA",	// NTR-APAE PKMN Pearl
		"CPU",	// NTR-CPUE PKMN Platinum
		"APY",	// NTR-APYE Puyo Pop Fever
		"AWH",	// NTR-AWHE Bubble Bobble Double Shot
		"AXB",	// NTR-AXBJ Daigassou! Band Brothers DX
		"A4U",	// NTR-A4UJ Wi-Fi Taiou - Morita Shogi
		"A8N",	// NTR-A8NE Planet Puzzle League
		"ABJ",	// NTR-ABJE Harvest Moon DS - Island of Happiness
		"ABN",	// NTR-ABNE Bomberman Story DS
		"ACL",	// NTR-ACLE Custom Robo Arena
		"ART",	// NTR-ARTJ Shin Lucky Star Moe Drill - Tabidachi
		"AVT",	// NTR-AVTJ Kou Rate Ura Mahjong Retsuden Mukoubuchi - Goburei, Shuuryou desu ne
		"AWY",	// NTR-AWYJ Wi-Fi Taiou - Gensen Table Game DS
		"AXJ",	// NTR-AXJE Dungeon Explorer - Warriors of Ancient Arts
		"AYK",	// NTR-AYKJ Wi-Fi Taiou - Yakuman DS
		"YB2",	// NTR-YB2E Bomberman Land Touch! 2
		"YB3",	// NTR-YB3E Harvest Moon DS - Sunshine Islands
		"YCH",	// NTR-YCHJ Kousoku Card Battle - Card Hero
		"YFE",	// NTR-YFEE Fire Emblem - Shadow Dragon
		"YGD",	// NTR-YGDE Diary Girl
		"YKR",	// NTR-YKRJ Culdcept DS
		"YRM",	// NTR-YRME My Secret World by Imagine
		"YW2",	// NTR-YW2E Advance Wars - Days of Ruin
		"AJU",	// NTR-AJUJ Jump! Ultimate Stars
		"ACZ",	// NTR-ACZE Cars
		"AHD",	// NTR-AHDE Jam Sessions
		"ANR",	// NTR-ANRE Naruto - Saikyou Ninja Daikesshu 3
		"YT3",	// NTR-YT3E Tamagotchi Connection - Corner Shop 3
		"AVI",	// NTR-AVIJ Kodomo no Tame no Yomi Kikase - Ehon de Asobou 1-Kan
		"AV2",	// NTR-AV2J Kodomo no Tame no Yomi Kikase - Ehon de Asobou 2-Kan
		"AV3",	// NTR-AV3J Kodomo no Tame no Yomi Kikase - Ehon de Asobou 3-Kan
		"AV4",	// NTR-AV4J Kodomo no Tame no Yomi Kikase - Ehon de Asobou 4-Kan
		"AV5",	// NTR-AV5J Kodomo no Tame no Yomi Kikase - Ehon de Asobou 5-Kan
		"AV6",	// NTR-AV6J Kodomo no Tame no Yomi Kikase - Ehon de Asobou 6-Kan
		"YNZ",	// NTR-YNZE Petz - Dogz Fashion
	};

	for (unsigned int i = 0; i < sizeof(list) / sizeof(list[0]); i++) {
		if (memcmp(ndsHeader->gameCode, list[i], 3) == 0) {
			// Found a match.
			specialSetting = true; // Special setting (when found special gamecode)
			break;
		}
	}*/

	if (specialSetting) {
		// special setting (when found special gamecode)
		volLevel = 0xAC;
	} else {
		// normal setting (for any other gamecodes)
		volLevel = 0xA7;
	}

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

/* static void DSiTouchscreenMode(void) {
	if (strncmp((const char*)0x04FFFA00, "no$gba", 6) != 0) {
		return;
	}

	// Touchscreen
	cdcWriteReg(0, 0x01, 0x01);
	cdcWriteReg(0, 0x39, 0x66);
	cdcWriteReg(1, 0x20, 0x16);
	cdcWriteReg(0, 0x04, 0x00);
	cdcWriteReg(0, 0x12, 0x81);
	cdcWriteReg(0, 0x13, 0x82);
	cdcWriteReg(0, 0x51, 0x82);
	cdcWriteReg(0, 0x51, 0x00);
	cdcWriteReg(0, 0x04, 0x03);
	cdcWriteReg(0, 0x05, 0xA1);
	cdcWriteReg(0, 0x06, 0x15);
	cdcWriteReg(0, 0x0B, 0x87);
	cdcWriteReg(0, 0x0C, 0x83);
	cdcWriteReg(0, 0x12, 0x87);
	cdcWriteReg(0, 0x13, 0x83);
	cdcWriteReg(3, 0x10, 0x88);
	cdcWriteReg(4, 0x08, 0x7F);
	cdcWriteReg(4, 0x09, 0xE1);
	cdcWriteReg(4, 0x0A, 0x80);
	cdcWriteReg(4, 0x0B, 0x1F);
	cdcWriteReg(4, 0x0C, 0x7F);
	cdcWriteReg(4, 0x0D, 0xC1);
	cdcWriteReg(0, 0x41, 0x08);
	cdcWriteReg(0, 0x42, 0x08);
	cdcWriteReg(0, 0x3A, 0x00);
	cdcWriteReg(4, 0x08, 0x7F);
	cdcWriteReg(4, 0x09, 0xE1);
	cdcWriteReg(4, 0x0A, 0x80);
	cdcWriteReg(4, 0x0B, 0x1F);
	cdcWriteReg(4, 0x0C, 0x7F);
	cdcWriteReg(4, 0x0D, 0xC1);
	cdcWriteReg(1, 0x2F, 0x2B);
	cdcWriteReg(1, 0x30, 0x40);
	cdcWriteReg(1, 0x31, 0x40);
	cdcWriteReg(1, 0x32, 0x60);
	cdcWriteReg(0, 0x74, 0x82);
	cdcWriteReg(0, 0x74, 0x92);
	cdcWriteReg(0, 0x74, 0xD2);
	cdcWriteReg(1, 0x21, 0x20);
	cdcWriteReg(1, 0x22, 0xF0);
	cdcWriteReg(0, 0x3F, 0xD4);
	cdcWriteReg(1, 0x23, 0x44);
	cdcWriteReg(1, 0x1F, 0xD4);
	cdcWriteReg(1, 0x28, 0x4E);
	cdcWriteReg(1, 0x29, 0x4E);
	cdcWriteReg(1, 0x24, 0x9E);
	cdcWriteReg(1, 0x25, 0x9E);
	cdcWriteReg(1, 0x20, 0xD4);
	cdcWriteReg(1, 0x2A, 0x14);
	cdcWriteReg(1, 0x2B, 0x14);
	cdcWriteReg(1, 0x26, 0xA7);
	cdcWriteReg(1, 0x27, 0xA7);
	cdcWriteReg(0, 0x40, 0x00);
	cdcWriteReg(0, 0x3A, 0x60);

	// Finish up!
	cdcReadReg (CDC_TOUCHCNT, 0x02);
	cdcWriteReg(CDC_TOUCHCNT, 0x02, 0x00);
} */

// SDK 5
static bool ROMsupportsDsiMode(const tNDSHeader* ndsHeader) {
	return (ndsHeader->unitCode > 0);
}

#ifdef FULL_DSI_MODE_ENABLED
void decrypt_modcrypt_area(dsi_context* ctx, u8 *buffer, unsigned int size)
{
	uint32_t len = size / 0x10;
	u8 block[0x10];

	while (len>0) {
		toncset(block, 0, 0x10);
		dsi_crypt_ctr_block(ctx, buffer, block);
		tonccpy(buffer, block, 0x10);
		buffer+=0x10;
		len--;
	}
}
#endif

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
void arm7_startBinary (void) {	
	// Get the ARM9 to boot
	arm9_stateFlag = ARM9_BOOTBIN;

	while (REG_VCOUNT!=191);
	while (REG_VCOUNT==191);

	REG_IE = 0;
	REG_IF = ~0;
	REG_AUXIE = 0;
	REG_AUXIF = ~0;
	// Start ARM7
	VoidFn arm7code = (VoidFn)ndsHeader->arm7executeAddress;
	arm7code();
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
	toncset((char*)0x0C400000, 0xFF, 0xC0);
	toncset((u8*)0x0C4000B2, 0, 3);
	toncset((u8*)0x0C4000B5, 0x24, 3);
	*(u16*)0x0C4000BE = 0x7FFF;
	*(u16*)0x0C4000CE = 0x7FFF;

	// Opera RAM patch (ARM9)
	*(u32*)0x02003D48 = 0xC400000;
	*(u32*)0x02003D4C = 0xC400004;

	*(u32*)0x02010FF0 = 0xC400000;
	*(u32*)0x02010FF4 = 0xC4000CE;

	*(u32*)0x020112AC = 0xC400080;

	*(u32*)0x020402BC = 0xC4000C2;
	*(u32*)0x020402C0 = 0xC4000C0;
	*(u32*)0x020402CC = 0xCFFFFFE;
	*(u32*)0x020402D0 = 0xC800000;
	*(u32*)0x020402D4 = 0xC9FFFFF;
	*(u32*)0x020402D8 = 0xCBFFFFF;
	*(u32*)0x020402DC = 0xCFFFFFF;
	*(u32*)0x020402E0 = 0xD7FFFFF;	// ???
	toncset((char*)0xC800000, 0xFF, 0x800000);		// Fill fake MEP with FFs

	// Opera RAM patch (ARM7)
	*(u32*)0x0238C7BC = 0xC400000;
	*(u32*)0x0238C7C0 = 0xC4000CE;

	//*(u32*)0x0238C950 = 0xC400000;
}


static void setMemoryAddress(const tNDSHeader* ndsHeader) {
	if (ROMsupportsDsiMode(ndsHeader)) {
	//	u8* deviceListAddr = (u8*)((u8*)0x02FFE1D4);
	//	tonccpy(deviceListAddr, deviceList_bin, deviceList_bin_len);

	//	const char *ndsPath = "nand:/dsiware.nds";
	//	tonccpy(deviceListAddr+0x3C0, ndsPath, sizeof(ndsPath));

		//tonccpy((u32*)0x02FFC000, (u32*)DSI_HEADER_SDK5, 0x1000);		// Make a duplicate of DSi header (Already used by dsiHeaderTemp)
		tonccpy((u32*)0x02FFFA80, (u32*)NDS_HEADER_SDK5, 0x160);	// Make a duplicate of DS header

		*(u32*)(0x02FFA680) = 0x02FD4D80;
		*(u32*)(0x02FFA684) = 0x00000000;
		*(u32*)(0x02FFA688) = 0x00001980;

		*(u32*)(0x02FFF00C) = 0x0000007F;
		*(u32*)(0x02FFF010) = 0x550E25B8;
		*(u32*)(0x02FFF014) = 0x02FF4000;

		// Set region flag
		if (strncmp(getRomTid(ndsHeader)+3, "J", 1) == 0) {
			*(u8*)(0x02FFFD70) = 0;
		} else if (strncmp(getRomTid(ndsHeader)+3, "E", 1) == 0) {
			*(u8*)(0x02FFFD70) = 1;
		} else if (strncmp(getRomTid(ndsHeader)+3, "P", 1) == 0) {
			*(u8*)(0x02FFFD70) = 2;
		} else if (strncmp(getRomTid(ndsHeader)+3, "U", 1) == 0) {
			*(u8*)(0x02FFFD70) = 3;
		} else if (strncmp(getRomTid(ndsHeader)+3, "C", 1) == 0) {
			*(u8*)(0x02FFFD70) = 4;
		} else if (strncmp(getRomTid(ndsHeader)+3, "K", 1) == 0) {
			*(u8*)(0x02FFFD70) = 5;
		}

		if (dsiModeConfirmed) {
			i2cWriteRegister(I2C_PM, I2CREGPM_MMCPWR, 1);		// Have IRQ check for power button press
			i2cWriteRegister(I2C_PM, I2CREGPM_RESETFLAG, 1);		// SDK 5 --> Bootflag = Warmboot/SkipHealthSafety
		}
	}

    // Set memory values expected by loaded NDS
    // from NitroHax, thanks to Chism
	*((u32*)(isSdk5(moduleParams) ? 0x02fff800 : 0x027ff800)) = chipID;					// CurrentCardID
	*((u32*)(isSdk5(moduleParams) ? 0x02fff804 : 0x027ff804)) = chipID;					// Command10CardID
	*((u16*)(isSdk5(moduleParams) ? 0x02fff808 : 0x027ff808)) = ndsHeader->headerCRC16;	// Header Checksum, CRC-16 of [000h-15Dh]
	*((u16*)(isSdk5(moduleParams) ? 0x02fff80a : 0x027ff80a)) = ndsHeader->secureCRC16;	// Secure Area Checksum, CRC-16 of [ [20h]..7FFFh]

	*((u16*)(isSdk5(moduleParams) ? 0x02fff850 : 0x027ff850)) = 0x5835;

	// Copies of above
	*((u32*)(isSdk5(moduleParams) ? 0x02fffc00 : 0x027ffc00)) = chipID;					// CurrentCardID
	*((u32*)(isSdk5(moduleParams) ? 0x02fffc04 : 0x027ffc04)) = chipID;					// Command10CardID
	*((u16*)(isSdk5(moduleParams) ? 0x02fffc08 : 0x027ffc08)) = ndsHeader->headerCRC16;	// Header Checksum, CRC-16 of [000h-15Dh]
	*((u16*)(isSdk5(moduleParams) ? 0x02fffc0a : 0x027ffc0a)) = ndsHeader->secureCRC16;	// Secure Area Checksum, CRC-16 of [ [20h]..7FFFh]

	*((u16*)(isSdk5(moduleParams) ? 0x02fffc10 : 0x027ffc10)) = 0x5835;

	*((u16*)(isSdk5(moduleParams) ? 0x02fffc40 : 0x027ffc40)) = 0x1;						// Boot Indicator (Booted from card for SDK5) -- EXTREMELY IMPORTANT!!! Thanks to cReDiAr
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Main function

void arm7_main (void) {

	initMBK();

	__dsimode = dsiMode;
	int errorCode;

	// Wait for ARM9 to at least start
	while (arm9_stateFlag < ARM9_START);

	//debugOutput (ERR_STS_CLR_MEM);

	scfgRomBak = REG_SCFG_ROM;

	// Get ARM7 to clear RAM
	arm7_resetMemory();	

	//debugOutput (ERR_STS_LOAD_BIN);

	if (!twlMode) {
		REG_SCFG_ROM = 0x703;	// Needed for Golden Sun: Dark Dawn to work
	}

	tDSiHeader* dsiHeaderTemp = (tDSiHeader*)0x02FFC000;

	// Load the NDS file
	errorCode = arm7_loadBinary(dsiHeaderTemp);
	if (errorCode) {
		debugOutput(errorCode);
	}

	if (my_isDSiMode()) {
		if (twlMode == 2) {
			dsiModeConfirmed = twlMode;
		} else {
			#ifdef FULL_DSI_MODE_ENABLED
			dsiModeConfirmed = twlMode && ROMsupportsDsiMode(&dsiHeaderTemp->ndshdr);
			#else
			dsiModeConfirmed = 0;
			#endif
		}
	}

	if (dsiModeConfirmed) {
		if (dsiHeaderTemp->arm9ibinarySize > 0) {
			cardRead((u32)dsiHeaderTemp->arm9iromOffset, (u32*)dsiHeaderTemp->arm9idestination, dsiHeaderTemp->arm9ibinarySize);
		}
		if (dsiHeaderTemp->arm7ibinarySize > 0) {
			cardRead((u32)dsiHeaderTemp->arm7iromOffset, (u32*)dsiHeaderTemp->arm7idestination, dsiHeaderTemp->arm7ibinarySize);
		}

		#ifdef FULL_DSI_MODE_ENABLED
		uint8_t *target = (uint8_t *)0x02FFC000 ;

		if (target[0x01C] & 2) {
			u8 key[16] = {0} ;
			u8 keyp[16] = {0} ;
			if (target[0x01C] & 4) {
				// Debug Key
				tonccpy(key, target, 16) ;
			} else
			{
				//Retail key
				char modcrypt_shared_key[8] = {'N','i','n','t','e','n','d','o'};
				tonccpy(keyp, modcrypt_shared_key, 8) ;
				for (int i=0;i<4;i++) {
					keyp[8+i] = target[0x0c+i] ;
					keyp[15-i] = target[0x0c+i] ;
				}
				tonccpy(key, target+0x350, 16) ;
				
				u128_xor(key, keyp);
				u128_add(key, DSi_KEY_MAGIC);
		  u128_lrot(key, 42) ;
			}
			uint32_t modcryptLengths[2] ;
			modcryptLengths[0] = ((uint32_t *)(target+0x220))[1] ;
			modcryptLengths[1] = ((uint32_t *)(target+0x220))[3] ;

			uint32_t rk[4];
			tonccpy(rk, key, 16) ;
			
			dsi_context ctx;
			dsi_set_key(&ctx, key);
			dsi_set_ctr(&ctx, &target[0x300]);
			if (modcryptLengths[0]) {
				decrypt_modcrypt_area(&ctx, (u8*)dsiHeaderTemp->arm9idestination, modcryptLengths[0]);
			}

			dsi_set_key(&ctx, key);
			dsi_set_ctr(&ctx, &target[0x314]);
			if (modcryptLengths[1]) {
				decrypt_modcrypt_area(&ctx, (u8*)dsiHeaderTemp->arm7idestination, modcryptLengths[1]);
			}

			for (int i=0;i<4;i++) {
				((uint32_t *)(target+0x220))[i] = 0;
			}
		}
		#endif
	}

	ndsHeader = loadHeader(dsiHeaderTemp);

	bool isDSBrowser = (memcmp(ndsHeader->gameCode, "UBRP", 4) == 0);

	arm9_extendedMemory = (dsiModeConfirmed || isDSBrowser);
	if (!arm9_extendedMemory) {
		tonccpy((u32*)0x023FF000, (u32*)(isSdk5(moduleParams) ? 0x02FFF000 : 0x027FF000), 0x1000);
	}

	my_readUserSettings(ndsHeader); // Header has to be loaded first

	if (my_isDSiMode()) {
		if ((REG_SNDEXTCNT & SNDEXTCNT_ENABLE) && ((!soundFreq && (REG_SNDEXTCNT & BIT(13))) || (soundFreq && !(REG_SNDEXTCNT & BIT(13))))) {
			if (soundFreq) {
				*(vu16*)0x04004700 |= BIT(13);	// Set 48khz sound/mic frequency
			} else {
				*(vu16*)0x04004700 &= ~BIT(13);	// Set 32khz sound/mic frequency
			}
		}

		if (dsiModeConfirmed) {
			*(u32*)0x3FFFFC8 = 0x7884;	// Fix sound pitch table for DSi mode (works with SDK5 binaries)

			if (ndsHeader->unitCode == 0 || (ndsHeader->unitCode > 0 && !(*(u8*)0x02FFE1BF & BIT(0)))) {
				NDSTouchscreenMode();
				*(vu16*)0x4000500 = 0x807F;
			}
		} else {
			REG_SCFG_ROM = 0x703;

			NDSTouchscreenMode();
			*(vu16*)0x4000500 = 0x807F;

			REG_GPIO_WIFI |= BIT(8);	// Old NDS-Wifi mode

			if (twlClock) {
				REG_SCFG_CLK = 0x0181;
			} else {
				REG_SCFG_CLK = 0x0180;
			}
			if (!sdAccess) {
				REG_SCFG_EXT = 0x93FBFB06;
			}
			// Used by ARM7 binaries to determine DSi mode...
			toncset((u8*)0x0380FFC0, 0, 0x10);
		}
	}

	if (my_isDSiMode() && isDSBrowser) {
		fixDSBrowser();
	}

	patchSleepInputWrite(ndsHeader, moduleParams);

	if (memcmp(ndsHeader->gameCode, "NTR", 3) == 0		// Download Play ROMs
	 || memcmp(ndsHeader->gameCode, "ASM", 3) == 0		// Super Mario 64 DS
	 || memcmp(ndsHeader->gameCode, "AMC", 3) == 0		// Mario Kart DS
	 || memcmp(ndsHeader->gameCode, "A2D", 3) == 0		// New Super Mario Bros.
	 || memcmp(ndsHeader->gameCode, "ARZ", 3) == 0		// Rockman ZX/MegaMan ZX
	 || memcmp(ndsHeader->gameCode, "AKW", 3) == 0		// Kirby Squeak Squad/Mouse Attack
	 || memcmp(ndsHeader->gameCode, "YZX", 3) == 0		// Rockman ZX Advent/MegaMan ZX Advent
	 || memcmp(ndsHeader->gameCode, "B6Z", 3) == 0)	// Rockman Zero Collection/MegaMan Zero Collection
	{
		gameSoftReset = true;
	}

	if (memcmp(ndsHeader->gameTitle, "TOP TF/SD DS", 12) == 0) {
		runCardEngine = false;
	}

	if (runCardEngine) {
		// WRAM-A mapped to the 0x37C0000 - 0x37FFFFF area : 256k
		REG_MBK6=0x080037C0;

		copyLoop ((u32*)ENGINE_LOCATION_ARM7, (u32*)cardengine_arm7_bin, cardengine_arm7_bin_size);
		errorCode = hookNdsRetail(ndsHeader, (u32*)ENGINE_LOCATION_ARM7);
		if (errorCode == ERR_NONE) {
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
				tonccpy (cheatDataOffset, (u32*)0x023F0000, 0x8000);	// Copy cheat data
			}
		}
	}
	toncset ((void*)0x023F0000, 0, 0x8000);		// Clear cheat data from main memory

	//debugOutput (ERR_STS_START);

	arm9_boostVram = boostVram;
	arm9_scfgUnlock = scfgUnlock;
	arm9_isSdk5 = isSdk5(moduleParams);
	arm9_runCardEngine = runCardEngine;

	if (isSdk5(moduleParams) && ndsHeader->unitCode > 0 && dsiModeConfirmed) {
		initMBK_dsiMode();
		REG_SCFG_EXT = 0x93FFFB06;
		REG_SCFG_CLK = 0x187;
	}

	if (!scfgUnlock && !dsiModeConfirmed) {
		// lock SCFG
		REG_SCFG_EXT &= ~(1UL << 31);
	}

	arm9_stateFlag = ARM9_SETSCFG;
	while (arm9_stateFlag != ARM9_READY);

	setMemoryAddress(ndsHeader);

	arm7_startBinary();

	while (1);
}

