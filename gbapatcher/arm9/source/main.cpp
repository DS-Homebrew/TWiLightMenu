#include <nds.h>
#include <nds/arm9/dldi.h>
#include <stdio.h>
#include <fat.h>
#include <sys/stat.h>
#include <limits.h>

#include <string>
#include <string.h>
#include <unistd.h>

#include "io_m3_common.h"
#include "io_g6_common.h"
#include "io_sc_common.h"
#include "exptools.h"

#include "common/nitrofs.h"
#include "common/inifile.h"
#include "common/stringtool.h"
#include "common/tonccpy.h"
#include "fileCopy.h"
#include "save/Save.h"
#include "gbaswitch.h"

static u8 blankBuf[0x10000] = {0};
u8 borderData[0x30000] = {0};
std::string gbaBorder = "default.png";

u32 romFileSize = 0;
bool savingAllowed = true;

static const u32 greenSwapPatch[6] = {
	0xE59F0008,	// LDR  R0, =0x4000002
	0xE3A01001, // MOV  R1, #1
	0xE5C01000, // STRB R1, [R0]
	0xE59FF000, // LDR  PC, =0x9FFFFE4
	0x04000002,
	0x09FFFFE4
};

static u32 prefetchPatch[6] = {
	0xE59F0008,	// LDR  R0, =0x4000204
	0xE3A01901, // MOV  R1, #0x4000
	0xE4A01000, // STRT R1, [R0]
	0xE59FF000, // LDR  PC, =0x80000C0 (this changes, depending on the ROM)
	0x04000204,
	0x080000C0
};

static const u32 classicNesPatch[2] = {
	0xE8BD0003,
	0xE281F00C
};

static const u8 sDbzLoGUPatch1[0x24] = 
	{0x0A, 0x1C, 0x40, 0x0B, 0xE0, 0x21, 0x09, 0x05, 0x41, 0x18, 0x07, 0x31, 0x00, 0x23, 0x08, 0x78,
	 0x10, 0x70, 0x01, 0x33, 0x01, 0x32, 0x01, 0x39, 0x07, 0x2B, 0xF8, 0xD9, 0x00, 0x20, 0x70, 0xBC,
	 0x02, 0xBC, 0x08, 0x47
	};

static const u8 sDbzLoGUPatch2[0x28] = 
	{0x70, 0xB5, 0x00, 0x04, 0x0A, 0x1C, 0x40, 0x0B, 0xE0, 0x21, 0x09, 0x05, 0x41, 0x18, 0x07, 0x31,
	 0x00, 0x23, 0x10, 0x78, 0x08, 0x70, 0x01, 0x33, 0x01, 0x32, 0x01, 0x39, 0x07, 0x2B, 0xF8, 0xD9,
	 0x00, 0x20, 0x70, 0xBC, 0x02, 0xBC, 0x08, 0x47
	};

static const u8 wwTwistedPatch[0xF0] = 
{
	0x1F, 0x24, 0x1F, 0xB4, 0x33, 0x48, 0x01, 0x21, 0x01, 0x60, 0x33, 0x48, 0x01, 0x21, 0x01, 0x60,
	0x32, 0x49, 0x0A, 0x68, 0x10, 0x23, 0x1A, 0x40, 0x1E, 0xD1, 0x30, 0x49, 0x0A, 0x68, 0x02, 0x23,
	0x1A, 0x40, 0x0D, 0xD0, 0x2E, 0x48, 0x01, 0x68, 0x01, 0x22, 0x91, 0x42, 0x02, 0xDB, 0x09, 0x19,
	0x01, 0x60, 0x38, 0xE0, 0x2A, 0x48, 0x01, 0x22, 0x02, 0x60, 0x12, 0x19, 0x02, 0x60, 0x32, 0xE0,
	0x27, 0x48, 0x01, 0x68, 0x01, 0x22, 0x91, 0x42, 0x00, 0xDB, 0x01, 0xE0, 0x02, 0x60, 0x11, 0x1C,
	0x24, 0x4B, 0xC9, 0x18, 0x01, 0x60, 0x26, 0xE0, 0x20, 0x49, 0x0A, 0x68, 0x20, 0x23, 0x1A, 0x40,
	0x1E, 0xD1, 0x1E, 0x49, 0x0A, 0x68, 0x02, 0x23, 0x1A, 0x40, 0x0D, 0xD0, 0x1C, 0x48, 0x01, 0x68,
	0x1D, 0x4A, 0x91, 0x42, 0x02, 0xDC, 0x09, 0x1B, 0x01, 0x60, 0x14, 0xE0, 0x18, 0x48, 0x1A, 0x4A,
	0x02, 0x60, 0x12, 0x1B, 0x02, 0x60, 0x0E, 0xE0, 0x15, 0x48, 0x01, 0x68, 0x16, 0x4A, 0x91, 0x42,
	0x00, 0xDC, 0x01, 0xE0, 0x02, 0x60, 0x11, 0x1C, 0x12, 0x4B, 0xC9, 0x1A, 0x01, 0x60, 0x02, 0xE0,
	0x0F, 0x48, 0x01, 0x21, 0x01, 0x60, 0x1F, 0xBC, 0x0C, 0x48, 0x00, 0x88, 0x0F, 0x4A, 0x10, 0x47,
	0x00, 0x7F, 0x00, 0x03, 0xA0, 0x7F, 0x00, 0x03, 0x30, 0x01, 0x00, 0x04, 0x4B, 0x13, 0x00, 0x08,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x20, 0x10, 0x00, 0x03, 0x98, 0x0F, 0x00, 0x03, 0x30, 0x01, 0x00, 0x04,
	0x30, 0x10, 0x00, 0x03, 0x20, 0x01, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x4B, 0x13, 0x00, 0x08
};

static const u8 yoshiTopsyTurvyPatch[0x18C] = 
{
	0x0C, 0x20, 0x9F, 0xE5, 0x80, 0x30, 0xA0, 0xE3, 0x00, 0x30, 0xE2, 0xE4, 0x04, 0x30, 0x9F, 0xE5,
	0x13, 0xFF, 0x2F, 0xE1, 0xE0, 0x7F, 0x00, 0x03, 0x69, 0x51, 0x02, 0x08, 0x00, 0x00, 0x00, 0x00,
	0xFF, 0xB5, 0x01, 0x4F, 0x00, 0x00, 0x09, 0xE0, 0xE0, 0x7F, 0x00, 0x03, 0x02, 0x49, 0x09, 0x88,
	0x01, 0x23, 0x08, 0x40, 0x70, 0x47, 0x00, 0x00, 0x30, 0x01, 0x00, 0x04, 0x3D, 0x78, 0x7F, 0x1C,
	0x80, 0x26, 0x35, 0x42, 0x24, 0xD0, 0x33, 0x48, 0x00, 0x21, 0x00, 0x88, 0x88, 0x42, 0x04, 0xD1,
	0x31, 0x49, 0x20, 0x20, 0x00, 0x02, 0x02, 0x30, 0x08, 0x80, 0x30, 0x48, 0x03, 0x21, 0x00, 0x88,
	0x88, 0x42, 0x04, 0xD1, 0x2E, 0x49, 0x20, 0x20, 0x00, 0x02, 0x02, 0x30, 0x08, 0x80, 0x02, 0x20,
	0x00, 0x02, 0x00, 0x30, 0xFF, 0xF7, 0xDA, 0xFF, 0x02, 0xD1, 0x2A, 0x49, 0x00, 0x20, 0x08, 0x80,
	0x01, 0x20, 0xFF, 0x30, 0xFF, 0xF7, 0xD2, 0xFF, 0x02, 0xD1, 0x27, 0x49, 0x03, 0x20, 0x08, 0x80,
	0x76, 0x08, 0x35, 0x42, 0x02, 0xD0, 0x25, 0x49, 0x63, 0x20, 0x08, 0x70, 0x76, 0x08, 0x35, 0x42,
	0x04, 0xD0, 0x23, 0x49, 0x27, 0x20, 0x00, 0x02, 0x0F, 0x30, 0x08, 0x80, 0x76, 0x08, 0x35, 0x42,
	0x02, 0xD0, 0x20, 0x49, 0x03, 0x20, 0x08, 0x80, 0x76, 0x08, 0x35, 0x42, 0x22, 0xD0, 0x1E, 0x49,
	0xAA, 0x20, 0x00, 0x02, 0xAA, 0x30, 0x08, 0x80, 0x1C, 0x49, 0xAA, 0x20, 0x00, 0x02, 0xAA, 0x30,
	0x08, 0x80, 0x1B, 0x49, 0xAA, 0x20, 0x00, 0x02, 0xAA, 0x30, 0x08, 0x80, 0x19, 0x49, 0xAA, 0x20,
	0x00, 0x02, 0xAA, 0x30, 0x08, 0x80, 0x18, 0x49, 0xAA, 0x20, 0x00, 0x02, 0xAA, 0x30, 0x08, 0x80,
	0x16, 0x49, 0xAA, 0x20, 0x00, 0x02, 0xAA, 0x30, 0x08, 0x80, 0x15, 0x49, 0xAA, 0x20, 0x00, 0x02,
	0xAA, 0x30, 0x08, 0x80, 0x76, 0x08, 0x35, 0x42, 0x02, 0xD0, 0x12, 0x49, 0x0A, 0x20, 0x08, 0x80,
	0x00, 0x00, 0x21, 0xE0, 0xE0, 0x1D, 0x00, 0x03, 0xE0, 0x1D, 0x00, 0x03, 0xE0, 0x1D, 0x00, 0x03,
	0xE0, 0x1D, 0x00, 0x03, 0xE0, 0x1D, 0x00, 0x03, 0xE0, 0x1D, 0x00, 0x03, 0xD8, 0x03, 0x00, 0x03,
	0xF8, 0x03, 0x00, 0x03, 0x00, 0x05, 0x00, 0x03, 0xDA, 0x03, 0x00, 0x03, 0xDC, 0x03, 0x00, 0x03,
	0xDE, 0x03, 0x00, 0x03, 0xE0, 0x03, 0x00, 0x03, 0xE2, 0x03, 0x00, 0x03, 0xE4, 0x03, 0x00, 0x03,
	0xE6, 0x03, 0x00, 0x03, 0x48, 0x29, 0x00, 0x02, 0xFF, 0xBD, 0x00, 0x00, 0x00, 0xB5, 0x03, 0x48,
	0xFE, 0x46, 0x00, 0x47, 0x01, 0xBC, 0x86, 0x46, 0x01, 0xBC, 0x01, 0xE0, 0x01, 0x9C, 0x7B, 0x08,
	0x02, 0x48, 0x00, 0x88, 0xC0, 0x43, 0x80, 0x05, 0x81, 0x0D, 0x01, 0xE0, 0x30, 0x01, 0x00, 0x04,
	0x03, 0xB4, 0x01, 0x48, 0x01, 0x90, 0x01, 0xBD, 0x18, 0x1A, 0x00, 0x08
};


ITCM_CODE void gptc_patchWait()
{
	u32 entryPoint = *(u32*)0x08000000;
	entryPoint -= 0xEA000000;
	entryPoint += 2;
	prefetchPatch[5] = 0x08000000+(entryPoint*4);

	u32 patchOffset = 0x01FFFFE4;
	tonccpy((u8*)0x08000000+patchOffset, prefetchPatch, 6*sizeof(u32));

	u32 branchCode = 0xEA000000+(patchOffset/sizeof(u32))-2;
	tonccpy((u16*)0x08000000, &branchCode, sizeof(u32));

	u32 searchRange = 0x08000000+romFileSize;
	if (romFileSize > 0x01FFFFE4) searchRange = 0x09FFFFE4;

	// General fix for white screen crash
	// Patch out wait states
	for (u32 addr = 0x080000C0; addr < searchRange; addr+=4) {
		if (*(u32*)addr != 0x04000204) {
			continue;
		}
		const u8 data8_last = *(u8*)(addr-1);
		 if (data8_last == 0x00 || data8_last == 0x03 || data8_last == 0x04 || *(u8*)(addr+7) == 0x04 || *(u8*)(addr+0xB) == 0x04
		  || data8_last == 0x08 || data8_last == 0x09
		  || data8_last == 0x47 || data8_last == 0x81 || data8_last == 0x85
		  || data8_last == 0xE0 || data8_last == 0xE7 || *(u16*)(addr+4) == 0x4017 || *(u16*)(addr-2) == 0xFFFE)
		{
			toncset((u16*)addr, 0, sizeof(u32));
		}
	}

	scanKeys();
	int keys = keysHeld();

	if ((keys & KEY_LEFT) && (keys & KEY_R)) {
		// Activate green swap
		u32 gsPatchOffset = 0x01FFFFCC;
		tonccpy((u8*)0x08000000+gsPatchOffset, greenSwapPatch, 6*sizeof(u32));

		branchCode = 0xEA000000+(gsPatchOffset/sizeof(u32))-2;
		tonccpy((u16*)0x08000000, &branchCode, sizeof(u32));
	}
}

static int paddingLevel = 0;
static void fixRomPadding(void) {
	if (paddingLevel != 1) {
		return;
	}

	// Pad unused ROM area with 0xFFs (trimmed ROMs).
	// Smallest retail ROM chip is 8 Mbit (1 MiB).
	s32 romSize = 1;
	while (romSize < (s32)romFileSize) {
		romSize += romSize;
	}
	if(romSize < 0x100000) romSize = 0x100000;
	const uintptr_t romLoc = 0x08000000;
	toncset((void*)(romLoc + romFileSize), 0xFF, romSize - romFileSize);

	u32 mirroredSize = romSize;
	if(romSize == 0x100000) // 1 MiB.
	{
		// ROM mirroring for Classic NES Series/others with 8 Mbit ROM.
		// The ROM is mirrored exactly 4 times.
		// Thanks to endrift for discovering this.
		mirroredSize = 0x400000; // 4 MiB.
		uintptr_t mirrorLoc = romLoc + romSize;
		do
		{
			tonccpy((void*)mirrorLoc, (void*)romLoc, romSize);
			mirrorLoc += romSize;
		} while(mirrorLoc < romLoc + mirroredSize);
	}

	/* if (paddingLevel != 2) {
		return;
	}

	if (mirroredSize < 0x02000000) {
		// Fake "open bus" padding.
		u16 hword = 0;
		for(uintptr_t i = romLoc + mirroredSize; i < 0x09FFFFCC; i += 2)
		{
			toncset16((u16*)i, hword, 1);
			hword++;
		}
	} */
}

static void gptc_patchRom()
{
	gptc_patchWait();

	const u32 nop = 0xE1A00000;
	const u16 nopT = 0x46C0;

	const u32 gameCode = *(u32*)(0x080000AC);
	if (gameCode == 0x50584C42) {
		//Astreix & Obelix XXL (Europe)
		//Fix white screen crash
		if (*(u16*)(0x08000000 + 0x50118) == 0x4014)
			*(u16*)(0x08000000 + 0x50118) = 0x4000;
	} else if (gameCode == 0x454D4246) {
		//Classic NES Series: Bomberman (USA, Europe)
		paddingLevel = 1;
		//Fix white screen crash
		tonccpy((u16*)(0x08000000 + 0xF4), classicNesPatch, 8);

		//Patch out SRAM check
		//*(u8*)(0x08000000 + 0xB3D5) = 0;
	} else if (gameCode == 0x45444146) {
		//Classic NES Series: Castlevania (USA, Europe)
		paddingLevel = 1;
		//Fix white screen crash
		tonccpy((u16*)(0x08000000 + 0xFC), classicNesPatch, 8);

		//Patch out SRAM check
		//*(u8*)(0x08000000 + 0x1B5EF) = 0;
	} else if (gameCode == 0x454B4446) {
		//Classic NES Series: Donkey Kong (USA, Europe)
		paddingLevel = 1;
		//Fix white screen crash
		tonccpy((u16*)(0x08000000 + 0xF4), classicNesPatch, 8);

		//Patch out SRAM check
		//*(u8*)(0x08000000 + 0xCB5E) = 0;
	} else if (gameCode == 0x454D4446) {
		//Classic NES Series: Dr. Mario (USA, Europe)
		paddingLevel = 1;
		//Fix white screen crash
		tonccpy((u16*)(0x08000000 + 0xFC), classicNesPatch, 8);

		//Patch out SRAM check
		//*(u8*)(0x08000000 + 0x1D974) = 0;
	} else if (gameCode == 0x45424546) {
		//Classic NES Series: Excitebike (USA, Europe)
		paddingLevel = 1;
		//Fix white screen crash
		tonccpy((u16*)(0x08000000 + 0xF4), classicNesPatch, 8);

		//Patch out SRAM check
		//*(u8*)(0x08000000 + 0xADDD) = 0;
	} else if (gameCode == 0x45434946) {
		//Classic NES Series: Ice Climber (USA, Europe)
		paddingLevel = 1;
		//Fix white screen crash
		tonccpy((u16*)(0x08000000 + 0xF4), classicNesPatch, 8);

		//Patch out SRAM check
		//*(u8*)(0x08000000 + 0xCB3B) = 0;
	} else if (gameCode == 0x454C5A46) {
		//Classic NES Series: The Legend of Zelda (USA, Europe)
		paddingLevel = 1;
		//Fix white screen crash
		tonccpy((u16*)(0x08000000 + 0xF4), classicNesPatch, 8);

		//Patch out SRAM check
		//*(u8*)(0x08000000 + 0x19417) = 0;
	} else if (gameCode == 0x45524D46 || gameCode == 0x45524D46) {
		//Classic NES Series: Metroid (USA, Europe)
		//Classic NES Series: Zelda II: The Adventure of Link (USA, Europe)
		paddingLevel = 1;
		//Fix white screen crash
		tonccpy((u16*)(0x08000000 + 0xFC), classicNesPatch, 8);

		//Patch out SRAM check
		//*(u8*)(0x08000000 + 0x20631) = 0;
	} else if (gameCode == 0x45375046) {
		//Classic NES Series: Pac-Man (USA, Europe)
		paddingLevel = 1;
		//Fix white screen crash
		tonccpy((u16*)(0x08000000 + 0xF4), classicNesPatch, 8);

		//Patch out SRAM check
		//*(u8*)(0x08000000 + 0xCEF7) = 0;
	} else if (gameCode == 0x454D5346) {
		//Classic NES Series: Super Mario Bros. (USA, Europe)
		paddingLevel = 1;
		//Fix white screen crash
		tonccpy((u16*)(0x08000000 + 0xF4), classicNesPatch, 8);

		//Patch out SRAM check
		//*(u8*)(0x08000000 + 0xCB53) = 0;
	} else if (gameCode == 0x45565846) {
		//Classic NES Series: Xevious (USA, Europe)
		paddingLevel = 1;
		//Fix white screen crash
		tonccpy((u16*)(0x08000000 + 0xF4), classicNesPatch, 8);

		//Patch out SRAM check
		//*(u8*)(0x08000000 + 0x14E02) = 0;
	} else if (gameCode == 0x4A4D5346 || gameCode == 0x4A4B4446 || gameCode == 0x4A434946 || gameCode == 0x4A4D5046 || gameCode == 0x4A565846 || gameCode == 0x4A504D46 || gameCode == 0x4A444446 || gameCode == 0x4A575446) {
		//Famicom Mini 01: Super Mario Bros. (Japan)
		//Famicom Mini 02: Donkey Kong (Japan)
		//Famicom Mini 03: Ice Climber (Japan)
		//Famicom Mini 06: Pac-Man (Japan)
		//Famicom Mini 07: Xevious (Japan)
		//Famicom Mini 08: Mappy (Japan)
		//Famicom Mini 16: Dig Dug (Japan)
		//Famicom Mini 19: TwinBee (Japan)
		paddingLevel = 1;
		//Fix white screen crash
		tonccpy((u16*)(0x08000000 + 0xF4), classicNesPatch, 8);
	} else if (gameCode == 0x4A425A46 || gameCode == 0x4A4C5A46 || gameCode == 0x4A4D4246 || gameCode == 0x4A4F5346) {
		//Famicom Mini 04: Excitebike (Japan)
		//Famicom Mini 05: Zelda no Densetsu 1: The Hyrule Fantasy (Japan)
		//Famicom Mini 09: Bomberman (Japan)
		//Famicom Mini 10: Star Soldier (Japan)
		paddingLevel = 1;
		//Fix white screen crash
		tonccpy((u16*)(0x08000000 + 0x100), classicNesPatch, 8);
	} else if (gameCode == 0x4A424D46) {
		//Famicom Mini 11: Mario Bros. (Japan)
		paddingLevel = 1;
		//Fix white screen crash
		tonccpy((u16*)(0x08000000 + 0xF4), classicNesPatch, 8);

		//Patch out SRAM check
		//*(u8*)(0x08000000 + 0xD1C5) = 0;
	} else if (gameCode == 0x4A4C4346) {
		//Famicom Mini 12: Clu Clu Land (Japan)
		paddingLevel = 1;
		//Fix white screen crash
		tonccpy((u16*)(0x08000000 + 0xF4), classicNesPatch, 8);

		//Patch out SRAM check
		//*(u8*)(0x08000000 + 0xD1BE) = 0;
	} else if (gameCode == 0x4A464246) {
		//Famicom Mini 13: Balloon Fight (Japan)
		paddingLevel = 1;
		//Fix white screen crash
		tonccpy((u16*)(0x08000000 + 0xF4), classicNesPatch, 8);

		//Patch out SRAM check
		//*(u8*)(0x08000000 + 0xD1EE) = 0;
	} else if (gameCode == 0x4A574346) {
		//Famicom Mini 14: Wrecking Crew (Japan)
		paddingLevel = 1;
		//Fix white screen crash
		tonccpy((u16*)(0x08000000 + 0xF4), classicNesPatch, 8);

		//Patch out SRAM check
		//*(u8*)(0x08000000 + 0xD6BA) = 0;
	} else if (gameCode == 0x4A4D4446) {
		//Famicom Mini 15: Dr. Mario (Japan)
		paddingLevel = 1;
		//Fix white screen crash
		tonccpy((u16*)(0x08000000 + 0xF4), classicNesPatch, 8);

		//Patch out SRAM check
		//*(u8*)(0x08000000 + 0x1DF1E) = 0;
	} else if (gameCode == 0x4A425446) {
		//Famicom Mini 17: Takahashi Meijin no Bouken-jima (Japan)
		paddingLevel = 1;
		//Fix white screen crash
		tonccpy((u16*)(0x08000000 + 0xF4), classicNesPatch, 8);

		//Patch out SRAM check
		//*(u8*)(0x08000000 + 0x233CC) = 0;
	} else if (gameCode == 0x4A4B4D46) {
		//Famicom Mini 18: Makaimura (Japan)
		paddingLevel = 1;
		//Fix white screen crash
		tonccpy((u16*)(0x08000000 + 0xF4), classicNesPatch, 8);

		//Patch out SRAM check
		//*(u8*)(0x08000000 + 0xD3E0) = 0;
	} else if (gameCode == 0x4A474746) {
		//Famicom Mini 20: Ganbare Goemon!: Karakuri Douchuu (Japan)
		paddingLevel = 1;
		//Fix white screen crash
		tonccpy((u16*)(0x08000000 + 0xF4), classicNesPatch, 8);

		//Patch out SRAM check
		//*(u8*)(0x08000000 + 0x5B38D) = 0;
	} else if (gameCode == 0x45324D46 || gameCode == 0x4A445346) {
		//Famicom Mini 21: Super Mario Bros. 2 (Japan)
		//Famicom Mini 30: SD Gundam World: Gachapon Senshi Scramble Wars (Japan)
		paddingLevel = 1;
		//Fix white screen crash
		tonccpy((u16*)(0x08000000 + 0xFC), classicNesPatch, 8);
	} else if (gameCode == 0x4A4D4E46) {
		//Famicom Mini 22: Nazo no Murasame Jou (Japan)
		paddingLevel = 1;
		//Fix white screen crash
		tonccpy((u16*)(0x08000000 + 0xFC), classicNesPatch, 8);

		//Patch out SRAM check
		//*(u8*)(0x08000000 + 0x18614) = 0;
	} else if (gameCode == 0x4A524D46) {
		//Famicom Mini 23: Metroid (Japan)
		paddingLevel = 1;
		//Fix white screen crash
		tonccpy((u16*)(0x08000000 + 0xFC), classicNesPatch, 8);

		//Patch out SRAM check
		//*(u8*)(0x08000000 + 0x1B305) = 0;
	} else if (gameCode == 0x4A545046) {
		//Famicom Mini 24: Hikari Shinwa: Palthena no Kagami (Japan)
		paddingLevel = 1;
		//Fix white screen crash
		tonccpy((u16*)(0x08000000 + 0xFC), classicNesPatch, 8);

		//Patch out SRAM check
		//*(u8*)(0x08000000 + 0x1B8EC) = 0;
	} else if (gameCode == 0x4A424C46) {
		//Famicom Mini 25: The Legend of Zelda 2: Link no Bouken (Japan)
		paddingLevel = 1;
		//Fix white screen crash
		tonccpy((u16*)(0x08000000 + 0xFC), classicNesPatch, 8);

		//Patch out SRAM check
		//*(u8*)(0x08000000 + 0x1F068) = 0;
	} else if (gameCode == 0x4A4D4646) {
		//Famicom Mini 26: Famicom Mukashibanashi: Shin Onigashima: Zen, Kouhen (Japan)
		paddingLevel = 1;
		//Fix white screen crash
		tonccpy((u16*)(0x08000000 + 0xFC), classicNesPatch, 8);

		//Patch out SRAM check
		//*(u8*)(0x08000000 + 0x31D8F) = 0;
	} else if (gameCode == 0x4A4B5446) {
		//Famicom Mini 27: Famicom Tantei Club: Kieta Koukeisha: Zen, Kouhen (Japan)
		paddingLevel = 1;
		//Fix white screen crash
		tonccpy((u16*)(0x08000000 + 0xFC), classicNesPatch, 8);

		//Patch out SRAM check
		//*(u8*)(0x08000000 + 0x2D80C) = 0;
	} else if (gameCode == 0x4A555446) {
		//Famicom Mini 28: Famicom Tantei Club Part II: Ushiro ni Tatsu Shoujo: Zen, Kouhen (Japan)
		paddingLevel = 1;
		//Fix white screen crash
		tonccpy((u16*)(0x08000000 + 0xFC), classicNesPatch, 8);

		//Patch out SRAM check
		//*(u8*)(0x08000000 + 0x2ED9C) = 0;
	} else if (gameCode == 0x4A444146) {
		//Famicom Mini 29: Akumajou Dracula (Japan)
		paddingLevel = 1;
		//Fix white screen crash
		tonccpy((u16*)(0x08000000 + 0xFC), classicNesPatch, 8);

		//Patch out SRAM check
		//*(u8*)(0x08000000 + 0x1C81C) = 0;
	} else if (gameCode == 0x4A525346) {
		//Famicom Mini: Dai-2-ji Super Robot Taisen (Japan) (Promo)
		paddingLevel = 1;
		//Fix white screen crash
		tonccpy((u16*)(0x08000000 + 0xFC), classicNesPatch, 8);

		//Patch out SRAM check
		//*(u8*)(0x08000000 + 0xC7F00) = 0;
	} else if (gameCode == 0x4A5A4746) {
		//Famicom Mini: Kidou Senshi Z Gundam: Hot Scramble (Japan) (Promo)
		paddingLevel = 1;
		//Fix white screen crash
		tonccpy((u16*)(0x08000000 + 0xFC), classicNesPatch, 8);

		//Patch out SRAM check
		//*(u8*)(0x08000000 + 0x27F1F) = 0;
	} else if (gameCode == 0x454D4441) {
		//Doom (USA)
		//Fix black screen crash
		if (*(u16*)(0x08000000 + 0x51C) == 0x45B6)
			*(u16*)(0x08000000 + 0x51C) = 0x4002;
	} else if (gameCode == 0x45443941 || gameCode == 0x50443941) {
		//Doom II (USA/Europe)
		//Fix black screen crash
		if (*(u16*)(0x08000000 + 0x2856) == 0x5281)
			*(u16*)(0x08000000 + 0x2856) = nopT;
	} else if (gameCode == 0x45474C41) {
		//Dragon Ball Z - The Legacy of Goku (USA)
		//Fix white screen crash
		if (*(u16*)(0x08000000 + 0x96E8) == 0x80A8)
			*(u16*)(0x08000000 + 0x96E8) = nopT;

		//Fix "game cannot be played on hardware found" error
		if (*(u16*)(0x08000000 + 0x356) == 0x7002)
			*(u16*)(0x08000000 + 0x356) = 0;

		if (*(u16*)(0x08000000 + 0x35E) == 0x7043)
			*(u16*)(0x08000000 + 0x35E) = 0;

		if (*(u16*)(0x08000000 + 0x37E) == 0x7001)
			*(u16*)(0x08000000 + 0x37E) = 0;

		if (*(u16*)(0x08000000 + 0x382) == 0x7041)
			*(u16*)(0x08000000 + 0x382) = 0;

		if (*(u16*)(0x08000000 + 0xE27E) == 0xB0A2) {
			*(u16*)(0x08000000 + 0xE27E) = 0x400;

			for (int i = 0; i < (int)sizeof(sDbzLoGUPatch1); i += 2)
				*(u16*)(0x08000000 + 0xE280 + i) = *(u16*)&sDbzLoGUPatch1[i];

			for (int i = 0; i < (int)sizeof(sDbzLoGUPatch2); i += 2)
				*(u16*)(0x08000000 + 0xE32C + i) = *(u16*)&sDbzLoGUPatch2[i];
		}
	} else if (gameCode == 0x50474C41) {
		//Dragon Ball Z - The Legacy of Goku (Europe)
		//Fix white screen crash
		if (*(u16*)(0x08000000 + 0x9948) == 0x80B0)
			*(u16*)(0x08000000 + 0x9948) = nopT;

		//Fix "game cannot be played on hardware found" error
		if (*(u16*)(0x08000000 + 0x33C) == 0x7119)
			*(u16*)(0x08000000 + 0x33C) = nopT;

		if (*(u16*)(0x08000000 + 0x340) == 0x7159)
			*(u16*)(0x08000000 + 0x340) = nopT;

		if (*(u16*)(0x08000000 + 0x356) == 0x705A)
			*(u16*)(0x08000000 + 0x356) = nopT;

		if (*(u16*)(0x08000000 + 0x35A) == 0x7002)
			*(u16*)(0x08000000 + 0x35A) = nopT;

		if (*(u16*)(0x08000000 + 0x35E) == 0x7042)
			*(u16*)(0x08000000 + 0x35E) = nopT;

		if (*(u16*)(0x08000000 + 0x384) == 0x7001)
			*(u16*)(0x08000000 + 0x384) = nopT;

		if (*(u16*)(0x08000000 + 0x388) == 0x7041)
			*(u16*)(0x08000000 + 0x388) = nopT;

		if (*(u16*)(0x08000000 + 0x494C) == 0x7002)
			*(u16*)(0x08000000 + 0x494C) = nopT;

		if (*(u16*)(0x08000000 + 0x4950) == 0x7042)
			*(u16*)(0x08000000 + 0x4950) = nopT;

		if (*(u16*)(0x08000000 + 0x4978) == 0x7001)
			*(u16*)(0x08000000 + 0x4978) = nopT;

		if (*(u16*)(0x08000000 + 0x497C) == 0x7041)
			*(u16*)(0x08000000 + 0x497C) = nopT;

		if (*(u16*)(0x08000000 + 0x988E) == 0x7028)
			*(u16*)(0x08000000 + 0x988E) = nopT;

		if (*(u16*)(0x08000000 + 0x9992) == 0x7068)
			*(u16*)(0x08000000 + 0x9992) = nopT;
	} else if (gameCode == 0x45464C41) {
		//Dragon Ball Z - The Legacy of Goku II (USA)
		tonccpy((u16*)0x080000E0, &nop, sizeof(u32));	// Fix white screen crash

		//Fix "game will not run on the hardware found" error
		if (*(u16*)(0x08000000 + 0x3B8E9E) == 0x1102)
			*(u16*)(0x08000000 + 0x3B8E9E) = 0x1001;

		if (*(u16*)(0x08000000 + 0x3B8EAE) == 0x0003)
			*(u16*)(0x08000000 + 0x3B8EAE) = 0;
	} else if (gameCode == 0x4A464C41) {
		//Dragon Ball Z - The Legacy of Goku II International (Japan)
		tonccpy((u16*)0x080000E0, &nop, sizeof(u32));	// Fix white screen crash

		//Fix "game will not run on the hardware found" error
		if (*(u16*)(0x08000000 + 0x3FC8F6) == 0x1102)
			*(u16*)(0x08000000 + 0x3FC8F6) = 0x1001;

		if (*(u16*)(0x08000000 + 0x3FC906) == 0x0003)
			*(u16*)(0x08000000 + 0x3FC906) = 0;
	} else if (gameCode == 0x50464C41) {
		//Dragon Ball Z - The Legacy of Goku II (Europe)
		tonccpy((u16*)0x080000E0, &nop, sizeof(u32));	// Fix white screen crash

		//Fix "game will not run on the hardware found" error
		if (*(u16*)(0x08000000 + 0x6F42B2) == 0x1102)
			*(u16*)(0x08000000 + 0x6F42B2) = 0x1001;

		if (*(u16*)(0x08000000 + 0x6F42C2) == 0x0003)
			*(u16*)(0x08000000 + 0x6F42C2) = 0;
	} else if (gameCode == 0x45464C42) {
		//2 Games in 1 - Dragon Ball Z - The Legacy of Goku I & II (USA)
		tonccpy((u16*)0x080000E0, &nop, sizeof(u32));	// Fix white screen crash

		if (*(u16*)(0x08000000 + 0x49840) == 0x80A8)
			*(u16*)(0x08000000 + 0x49840) = nopT;

		tonccpy((u16*)0x088000E0, &nop, sizeof(u32));

		//LoG1: Fix "game cannot be played on hardware found" error
		if (*(u16*)(0x08000000 + 0x40356) == 0x7002)
			*(u16*)(0x08000000 + 0x40356) = 0;

		if (*(u16*)(0x08000000 + 0x4035E) == 0x7043)
			*(u16*)(0x08000000 + 0x4035E) = 0;

		if (*(u16*)(0x08000000 + 0x4037E) == 0x7001)
			*(u16*)(0x08000000 + 0x4037E) = 0;

		if (*(u16*)(0x08000000 + 0x40382) == 0x7041)
			*(u16*)(0x08000000 + 0x40382) = 0;

		//Do we need this?
		/*if (*(u16*)(0x08000000 + 0x4E316) == 0xB0A2) {
			*(u16*)(0x08000000 + 0x4E316) = 0x400;

			for (int i = 0; i < sizeof(sDbzLoGUPatch1); i += 2)
				*(u16*)(0x08000000 + 0x4E318 + i) = *(u16*)&sDbzLoGUPatch1[i];

			for (int i = 0; i < sizeof(sDbzLoGUPatch2); i += 2)
				*(u16*)(0x08000000 + 0x????? + i) = *(u16*)&sDbzLoGUPatch2[i];
		}*/

		//LoG2: Fix "game will not run on the hardware found" error
		if (*(u16*)(0x08000000 + 0xBB9016) == 0x1102)
			*(u16*)(0x08000000 + 0xBB9016) = 0x1001;

		if (*(u16*)(0x08000000 + 0xBB9026) == 0x0003)
			*(u16*)(0x08000000 + 0xBB9026) = 0;
	} else if (gameCode == 0x45424442) {
		//Dragon Ball Z - Taiketsu (USA)
		//Fix "game cannot be played on this hardware" error
		if (*(u16*)(0x08000000 + 0x2BD54) == 0x7818)
			*(u16*)(0x08000000 + 0x2BD54) = 0x2000;

		if (*(u16*)(0x08000000 + 0x2BD60) == 0x7810)
			*(u16*)(0x08000000 + 0x2BD60) = 0x2000;

		if (*(u16*)(0x08000000 + 0x2BD80) == 0x703A)
			*(u16*)(0x08000000 + 0x2BD80) = 0x1C00;

		if (*(u16*)(0x08000000 + 0x2BD82) == 0x7839)
			*(u16*)(0x08000000 + 0x2BD82) = 0x2100;

		if (*(u16*)(0x08000000 + 0x2BD8C) == 0x7030)
			*(u16*)(0x08000000 + 0x2BD8C) = 0x1C00;

		if (*(u16*)(0x08000000 + 0x2BD8E) == 0x7830)
			*(u16*)(0x08000000 + 0x2BD8E) = 0x2000;

		if (*(u16*)(0x08000000 + 0x2BDAC) == 0x7008)
			*(u16*)(0x08000000 + 0x2BDAC) = 0x1C00;

		if (*(u16*)(0x08000000 + 0x2BDB2) == 0x7008)
			*(u16*)(0x08000000 + 0x2BDB2) = 0x1C00;
	} else if (gameCode == 0x50424442) {
		//Dragon Ball Z - Taiketsu (Europe)
		//Fix "game cannot be played on this hardware" error
		if (*(u16*)(0x08000000 + 0x3FE08) == 0x7818)
			*(u16*)(0x08000000 + 0x3FE08) = 0x2000;

		if (*(u16*)(0x08000000 + 0x3FE14) == 0x7810)
			*(u16*)(0x08000000 + 0x3FE14) = 0x2000;

		if (*(u16*)(0x08000000 + 0x3FE34) == 0x703A)
			*(u16*)(0x08000000 + 0x3FE34) = 0x1C00;

		if (*(u16*)(0x08000000 + 0x3FE36) == 0x7839)
			*(u16*)(0x08000000 + 0x3FE36) = 0x2100;

		if (*(u16*)(0x08000000 + 0x3FE40) == 0x7030)
			*(u16*)(0x08000000 + 0x3FE40) = 0x1C00;

		if (*(u16*)(0x08000000 + 0x3FE42) == 0x7830)
			*(u16*)(0x08000000 + 0x3FE42) = 0x2000;

		if (*(u16*)(0x08000000 + 0x3FE58) == 0x7008)
			*(u16*)(0x08000000 + 0x3FE58) = 0x1C00;

		if (*(u16*)(0x08000000 + 0x3FE66) == 0x7008)
			*(u16*)(0x08000000 + 0x3FE66) = 0x1C00;
	} else if (gameCode == 0x45334742) {
		//Dragon Ball Z - Buu's Fury (USA)
		tonccpy((u16*)0x080000E0, &nop, sizeof(u32));	// Fix white screen crash

		//Fix "game will not run on this hardware" error
		if (*(u16*)(0x08000000 + 0x8B66) == 0x7032)
			*(u16*)(0x08000000 + 0x8B66) = 0;

		if (*(u16*)(0x08000000 + 0x8B6A) == 0x7072)
			*(u16*)(0x08000000 + 0x8B6A) = 0;

		if (*(u16*)(0x08000000 + 0x8B86) == 0x7008)
			*(u16*)(0x08000000 + 0x8B86) = 0;

		if (*(u16*)(0x08000000 + 0x8B8C) == 0x7031)
			*(u16*)(0x08000000 + 0x8B8C) = 0;

		if (*(u16*)(0x08000000 + 0x8B90) == 0x7071)
			*(u16*)(0x08000000 + 0x8B90) = 0;
	} else if (gameCode == 0x45345442) {
		//Dragon Ball GT - Transformation (USA)
		tonccpy((u16*)0x080000E0, &nop, sizeof(u32));	// Fix white screen crash
	} else if (gameCode == 0x45465542) {
		//2 Games in 1 - Dragon Ball Z - Buu's Fury & Dragon Ball GT - Transformation (USA)
		tonccpy((u16*)0x080000E0, &nop, sizeof(u32));	// Fix white screen crash
		tonccpy((u16*)0x080300E0, &nop, sizeof(u32));
		tonccpy((u16*)0x088000E0, &nop, sizeof(u32));

		//DBZ BF: Fix "game will not run on this hardware" error
		if (*(u16*)(0x08000000 + 0x38B66) == 0x7032)
			*(u16*)(0x08000000 + 0x38B66) = 0;

		if (*(u16*)(0x08000000 + 0x38B6A) == 0x7072)
			*(u16*)(0x08000000 + 0x38B6A) = 0;

		if (*(u16*)(0x08000000 + 0x38B86) == 0x7008)
			*(u16*)(0x08000000 + 0x38B86) = 0;

		if (*(u16*)(0x08000000 + 0x38B8C) == 0x7031)
			*(u16*)(0x08000000 + 0x38B8C) = 0;

		if (*(u16*)(0x08000000 + 0x38B90) == 0x7071)
			*(u16*)(0x08000000 + 0x38B90) = 0;
	} else if (gameCode == 0x45564442) {
		//Dragon Ball - Advanced Adventure (USA)
		//Fix white screen crash
		if (*(u16*)(0x08000000 + 0x10C240) == 0x8008)
			*(u16*)(0x08000000 + 0x10C240) = nopT;
	} else if (gameCode == 0x50564442) {
		//Dragon Ball - Advanced Adventure (Europe)
		//Fix white screen crash
		if (*(u16*)(0x08000000 + 0x10CE3C) == 0x8008)
			*(u16*)(0x08000000 + 0x10CE3C) = nopT;
	} else if (gameCode == 0x4A564442) {
		//Dragon Ball - Advanced Adventure (Japan)
		//Fix white screen crash
		if (*(u16*)(0x08000000 + 0x10B078) == 0x8008)
			*(u16*)(0x08000000 + 0x10B078) = nopT;
	} else if (gameCode == 0x45324941) {
		//Iridion II (USA)
		//Fix where the game freezes during the first boss battle (but the music continues)
		if (*(u16*)(0x08000000 + 0x1FA90) == 0xD3F4)
			*(u16*)(0x08000000 + 0x1FA90) = 0x1C00;

		if (*(u8*)(0x08000000 + 0x1FA95) == 0xD1)
			*(u8*)(0x08000000 + 0x1FA95) = 0xE0;
	} else if (gameCode == 0x50324941) {
		//Iridion II (Europe)
		//Fix where the game freezes during the first boss battle (but the music continues)
		if (*(u16*)(0x08000000 + 0x1FB50) == 0xD3F4)
			*(u16*)(0x08000000 + 0x1FB50) = 0x1C00;

		if (*(u8*)(0x08000000 + 0x1FB55) == 0xD1)
			*(u8*)(0x08000000 + 0x1FB55) = 0xE0;
	} else if (gameCode == 0x454B3842) {
		//Kirby and the Amazing Mirror (USA)
		//Fix white screen crash
		if (*(u16*)(0x08000000 + 0x1515A4) == 0x8008)
			*(u16*)(0x08000000 + 0x1515A4) = nopT;
	} else if (gameCode == 0x504B3842) {
		//Kirby and the Amazing Mirror (Europe)
		//Fix white screen crash
		if (*(u16*)(0x08000000 + 0x151EE0) == 0x8008)
			*(u16*)(0x08000000 + 0x151EE0) = nopT;
	} else if (gameCode == 0x4A4B3842) {
		//Hoshi no Kirby - Kagami no Daimeikyuu (Japan) (V1.1)
		//Fix white screen crash
		if (*(u16*)(0x08000000 + 0x151564) == 0x8008)
			*(u16*)(0x08000000 + 0x151564) = nopT;
	} else if (gameCode == 0x45533342) {
		//Sonic Advance 3 (USA)
		//Fix white screen crash
		if (*(u16*)(0x08000000 + 0xBB67C) == 0x8008)
			*(u16*)(0x08000000 + 0xBB67C) = nopT;
	} else if (gameCode == 0x50533342) {
		//Sonic Advance 3 (Europe)
		//Fix white screen crash
		if (*(u16*)(0x08000000 + 0xBBA04) == 0x8008)
			*(u16*)(0x08000000 + 0xBBA04) = nopT;
	} else if (gameCode == 0x4A533342) {
		//Sonic Advance 3 (Japan)
		//Fix white screen crash
		if (*(u16*)(0x08000000 + 0xBB9F8) == 0x8008)
			*(u16*)(0x08000000 + 0xBB9F8) = nopT;
	} else if (gameCode == 0x45593241) {
		//Top Gun - Combat Zones (USA)
		//Fix softlock when attempting to save (original cartridge does not have a save chip)
		if (*(u16*)(0x08000000 + 0x88816) == 0x3501)
			*(u16*)(0x08000000 + 0x88816) = 0x3401;

		savingAllowed = false;
	} else if (gameCode == 0x45415741 || gameCode == 0x4A415741) {
		//Wario Land 4/Advance (USA/Europe/Japan)
		//Fix white screen crash
		if (*(u16*)(0x08000000 + 0x726) == 0x8008)
			*(u16*)(0x08000000 + 0x726) = nopT;
	} else if (gameCode == 0x43415741) {
		//Wario Land Advance (iQue)
		//Fix white screen crash
		if (*(u16*)(0x08000000 + 0xE92) == 0x8008)
			*(u16*)(0x08000000 + 0xE92) = nopT;
	} else if (gameCode == 0x45575A52) {
		//WarioWare: Twisted! (USA)
		//Patch out tilt controls
		if (*(u16*)(0x08000000 + 0x1348) == 0x8800)
			*(u16*)(0x08000000 + 0x1348) = 0x4700;

		if (*(u16*)(0x08000000 + 0x1376) == 0x0400 && *(u16*)(0x08000000 + 0x1374) == 0x0130) {
			*(u16*)(0x08000000 + 0x1376) = 0x08E9;
			*(u16*)(0x08000000 + 0x1374) = 0x3C6D;

			tonccpy((u8*)0x08E93C6C, &wwTwistedPatch, 0xF0);
		}
	} else if (gameCode == 0x4547594B) {
		//Yoshi Topsy-Turvy (USA)
		//Fix white screen crash
		if (*(u16*)(0x08000000 + 0x16E4) == 0x8008)
			*(u16*)(0x08000000 + 0x16E4) = nopT;

		//Patch out tilt controls
		if (*(u16*)(0x08000000 + 0x1F2) == 0x0802 && *(u16*)(0x08000000 + 0x1F0) == 0x5169) {
			*(u16*)(0x08000000 + 0x1F2) = 0x087B;
			*(u16*)(0x08000000 + 0x1F0) = 0x9BE0;

			tonccpy((u8*)0x087B9BE0, &yoshiTopsyTurvyPatch, 0x18C);
		}

		if (*(u16*)(0x08000000 + 0x1A0E) == 0x4808)
			*(u16*)(0x08000000 + 0x1A0E) = 0xB401;

		if (*(u16*)(0x08000000 + 0x1A10) == 0x8800)
			*(u16*)(0x08000000 + 0x1A10) = 0x4800;

		if (*(u16*)(0x08000000 + 0x1A12) == 0x43C0)
			*(u16*)(0x08000000 + 0x1A12) = 0x4700;

		if (*(u16*)(0x08000000 + 0x1A14) == 0x0580)
			*(u16*)(0x08000000 + 0x1A14) = 0x9D3D;

		if (*(u16*)(0x08000000 + 0x1A16) == 0x0D81)
			*(u16*)(0x08000000 + 0x1A16) = 0x087B;
	}
}


//---------------------------------------------------------------------------------
void stop (void) {
//---------------------------------------------------------------------------------
	while (1) {
		swiWaitForVBlank();
	}
}

void s2RamAccess(bool open) {
	if (io_dldi_data->ioInterface.features & FEATURE_SLOT_NDS) return;

	if (open) {
		if (*(u16*)(0x020000C0) == 0x334D) {
			_M3_changeMode(M3_MODE_RAM);
		} else if (*(u16*)(0x020000C0) == 0x3647) {
			_G6_SelectOperation(G6_MODE_RAM);
		} else if (*(u16*)(0x020000C0) == 0x4353) {
			_SC_changeMode(SC_MODE_RAM);
		}
	} else {
		if (*(u16*)(0x020000C0) == 0x334D) {
			_M3_changeMode(M3_MODE_MEDIA);
		} else if (*(u16*)(0x020000C0) == 0x3647) {
			_G6_SelectOperation(G6_MODE_MEDIA);
		} else if (*(u16*)(0x020000C0) == 0x4353) {
			_SC_changeMode(SC_MODE_MEDIA);
		}
	}
}

void gbaSramAccess(bool open) {
	if (open) {
		if (*(u16*)(0x020000C0) == 0x334D) {
			_M3_changeMode(M3_MODE_RAM);
		} else if (*(u16*)(0x020000C0) == 0x3647) {
			_G6_SelectOperation(G6_MODE_RAM);
		} else if (*(u16*)(0x020000C0) == 0x4353) {
			_SC_changeMode(SC_MODE_RAM_RO);
		}
	} else {
		if (*(u16*)(0x020000C0) == 0x334D) {
			_M3_changeMode((io_dldi_data->ioInterface.features & FEATURE_SLOT_GBA) ? M3_MODE_MEDIA : M3_MODE_RAM);
		} else if (*(u16*)(0x020000C0) == 0x3647) {
			_G6_SelectOperation((io_dldi_data->ioInterface.features & FEATURE_SLOT_GBA) ? G6_MODE_MEDIA : G6_MODE_RAM);
		} else if (*(u16*)(0x020000C0) == 0x4353) {
			_SC_changeMode((io_dldi_data->ioInterface.features & FEATURE_SLOT_GBA) ? SC_MODE_MEDIA : SC_MODE_RAM);
		}
	}
}

//---------------------------------------------------------------------------------
int main(int argc, char **argv) {
//---------------------------------------------------------------------------------

	defaultExceptionHandler();

	if (!fatInitDefault()) {
		consoleDemoInit();
		printf("fatInitDefault failed!");
		stop();
	}

	if (argc < 2) {
		return 0;	// No arguments passed!
	}

	// overwrite reboot stub identifier
	extern char *fake_heap_end;
	*fake_heap_end = 0;

	//consoleDemoInit();

	FILE* file = fopen("/_nds/TWiLightMenu/gbaswitch.srldr", "rb");
	if (file) {
		fread((void*)0x02001000, 1, 0x1000, file);
		fclose(file);
	}
	//iprintf("Loaded GBA switcher\n");

	romFileSize = getFileSize(argv[1]);

	nitroFSInit("/_nds/TWiLightMenu/gbapatcher.srldr");

	CIniFile settingsini("/_nds/TWiLightMenu/settings.ini");
	gbaBorder = settingsini.GetString("SRLOADER", "GBA_BORDER", gbaBorder);

	char borderPath[256];
	sprintf(borderPath, "/_nds/TWiLightMenu/gbaborders/%s", gbaBorder.c_str());

	file = fopen((access(borderPath, F_OK)==0) ? borderPath : "nitro:/graphics/gbaborder.png", "rb");
	if (file) {
		fread(borderData, 1, sizeof(borderData), file);
		fclose(file);
	}
	//iprintf("Loaded GBA border\n");

	sysSetCartOwner(BUS_OWNER_ARM9); // Allow arm9 to access GBA ROM

	s2RamAccess(true);
	//iprintf("s2RamAccess(true)\n");

	if (*(u32*)0x080004AC == 0x4A424741) {
		// Make multiboot binary bootable
		file = fopen("nitro:/mb2gba.gba", "rb");
		if (file) {
			fread((void*)0x08000000, 1, 0x400, file);
			fclose(file);
		}
		s2RamAccess(false);
	} else if (*(u32*)0x080000AC != 0x4732424D) {
		if (*(u16*)(0x020000C0) != 0x5A45) {
			gptc_patchRom();
			//iprintf("ROM patched\n");
		}

		const save_type_t* saveType = savingAllowed ? save_findTag() : NULL;
		//iprintf("Save tag found\n");
		if (saveType != NULL && saveType->patchFunc != NULL) {
			if (saveType->patchFunc(saveType) && *(u16*)(0x020000C0) == 0x5A45) {
				consoleDemoInit();
				printf("\x1B[41mWARNING!\x1B[47m\n");
				printf("This game uses a save type\n");
				printf("other than SRAM.\n\n");
				printf("Please SRAM-patch your ROM\n");
				printf("in order to save your data.\n\n");
				printf("Press A to continue\nwithout saving\n");

				u16 pressed = 0;
				do {
					swiWaitForVBlank();
					scanKeys();
					pressed = keysDown();
				} while (!(pressed & KEY_A));

				consoleClear();
			}
		}

		if (*(u16*)(0x020000C0) != 0x5A45) {
			fixRomPadding();
		}

		s2RamAccess(false);
		//iprintf("s2RamAccess(false)\n");

		if (saveType != NULL) {
			std::string filename = argv[1];
			std::string typeToReplace = filename.substr(filename.rfind('.'));
			std::string savepath = replaceAll(argv[1], typeToReplace, ".sav");
			if (getFileSize(savepath.c_str()) == 0) {
				u32 size = (saveType->size > 0x10000 ? 0x10000 : saveType->size);
				for (u32 i = 0; i < size; i++) {
					blankBuf[i] = 0xFF;
				}
				//iprintf("Creating save file\n");
				gbaSramAccess(true);	// Switch to GBA SRAM
				cExpansion::WriteSram(0x0A000000, (u8*)blankBuf, size);
				gbaSramAccess(false);	// Switch out of GBA SRAM
				FILE *pFile = fopen(savepath.c_str(), "wb");
				if (pFile) {
					fseek(pFile, saveType->size - 1, SEEK_SET);
					fputc('\0', pFile);
					fclose(pFile);
				}
			}
		}
	} else {
		s2RamAccess(false);
	}

	// Lock write access to ROM region (and switch to GBA SRAM)
	if (*(u16*)(0x020000C0) == 0x334D) {
		_M3_changeMode(M3_MODE_RAM);
	} else 	if (*(u16*)(0x020000C0) == 0x4353) {
		_SC_changeMode(SC_MODE_RAM_RO);
	} else 	if (*(u16*)(0x020000C0) == 0x5A45) {
		cExpansion::CloseNorWrite();
	}

	//iprintf("Switching to GBA mode\n");
	gbaSwitch();

	return 0;
}
