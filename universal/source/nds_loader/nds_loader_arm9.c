/*-----------------------------------------------------------------
 Copyright (C) 2005 - 2010
	Michael "Chishm" Chisholm
	Dave "WinterMute" Murphy

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

------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <nds.h>
#include <nds/arm9/dldi.h>
#include <sys/stat.h>
#include <limits.h>

#include <unistd.h>
#include <fat.h>

#include "common/tonccpy.h"
#include "load_bin.h"

#ifndef _NO_BOOTSTUB_
struct __my_bootstub {
	u64	bootsig;
	VoidFn arm9reboot;
	VoidFn arm7reboot;
	u32 bootaddr;
	u32 bootsize;
};

#include "bootstub_bin.h"
#endif

#include "common/nds_loader_arm9.h"
#define LCDC_BANK_C (u16*)0x06848000
#define STORED_FILE_CLUSTER (*(((u32*)LCDC_BANK_C) + 1))
#define INIT_DISC (*(((u32*)LCDC_BANK_C) + 2))
#define WANT_TO_PATCH_DLDI (*(((u32*)LCDC_BANK_C) + 3))


/*
	b	startUp

storedFileCluster:
	.word	0x0FFFFFFF		@ default BOOT.NDS
initDisc:
	.word	0x00000001		@ init the disc by default
wantToPatchDLDI:
	.word	0x00000001		@ by default patch the DLDI section of the loaded NDS
@ Used for passing arguments to the loaded app
argStart:
	.word	_end - _start
argSize:
	.word	0x00000000
dldiOffset:
	.word	_dldi_start - _start
dsiSD:
	.word	0
dsiMode:
	.word	0
*/

#define STORED_FILE_CLUSTER_OFFSET 4
#define INIT_DISC_OFFSET 8
#define WANT_TO_PATCH_DLDI_OFFSET 12
#define ARG_START_OFFSET 16
#define ARG_SIZE_OFFSET 20
#define HAVE_DSISD_OFFSET 28
#define DSIMODE_OFFSET 32
#define CLEAR_MASTER_BRIGHT_OFFSET 36
#define DSMODE_SWITCH_OFFSET 40
#define LOADFROMRAM_OFFSET 44
#define LANGUAGE_OFFSET 48
#define TSC_TGDS_OFFSET 52


typedef signed int addr_t;
typedef unsigned char data_t;

#define FIX_ALL	0x01
#define FIX_GLUE	0x02
#define FIX_GOT	0x04
#define FIX_BSS	0x08

enum DldiOffsets {
	DO_magicString = 0x00,			// "\xED\xA5\x8D\xBF Chishm"
	DO_magicToken = 0x00,			// 0xBF8DA5ED
	DO_magicShortString = 0x04,		// " Chishm"
	DO_version = 0x0C,
	DO_driverSize = 0x0D,
	DO_fixSections = 0x0E,
	DO_allocatedSpace = 0x0F,

	DO_friendlyName = 0x10,

	DO_text_start = 0x40,			// Data start
	DO_data_end = 0x44,				// Data end
	DO_glue_start = 0x48,			// Interworking glue start	-- Needs address fixing
	DO_glue_end = 0x4C,				// Interworking glue end
	DO_got_start = 0x50,			// GOT start					-- Needs address fixing
	DO_got_end = 0x54,				// GOT end
	DO_bss_start = 0x58,			// bss start					-- Needs setting to zero
	DO_bss_end = 0x5C,				// bss end

	// IO_INTERFACE data
	DO_ioType = 0x60,
	DO_features = 0x64,
	DO_startup = 0x68,
	DO_isInserted = 0x6C,
	DO_readSectors = 0x70,
	DO_writeSectors = 0x74,
	DO_clearStatus = 0x78,
	DO_shutdown = 0x7C,
	DO_code = 0x80
};

static addr_t readAddr (data_t *mem, addr_t offset) {
	return ((addr_t*)mem)[offset/sizeof(addr_t)];
}

static void writeAddr (data_t *mem, addr_t offset, addr_t value) {
	((addr_t*)mem)[offset/sizeof(addr_t)] = value;
}

/* static addr_t quickFind (const data_t* data, const data_t* search, size_t dataLen, size_t searchLen) {
	const int* dataChunk = (const int*) data;
	int searchChunk = ((const int*)search)[0];
	addr_t i;
	addr_t dataChunkEnd = (addr_t)(dataLen / sizeof(int));

	for ( i = 0; i < dataChunkEnd; i++) {
		if (dataChunk[i] == searchChunk) {
			if ((i*sizeof(int) + searchLen) > dataLen) {
				return -1;
			}
			if (memcmp (&data[i*sizeof(int)], search, searchLen) == 0) {
				return i*sizeof(int);
			}
		}
	}

	return -1;
} */

// Normal DLDI uses "\xED\xA5\x8D\xBF Chishm"
// Bootloader string is different to avoid being patched
// static const data_t dldiMagicLoaderString[] = "\xEE\xA5\x8D\xBF Chishm";	// Different to a normal DLDI file

#define DEVICE_TYPE_DLDI 0x49444C44

static bool dldiPatchLoader (void)
{
	addr_t memOffset;			// Offset of DLDI after the file is loaded into memory
	addr_t relocationOffset;	// Value added to all offsets within the patch to fix it properly
	addr_t ddmemOffset;			// Original offset used in the DLDI file
	addr_t ddmemStart;			// Start of range that offsets can be in the DLDI file
	addr_t ddmemEnd;			// End of range that offsets can be in the DLDI file
	addr_t ddmemSize;			// Size of range that offsets can be in the DLDI file

	addr_t addrIter;

	size_t dldiFileSize = 0;

	data_t *pDH = (data_t*)(io_dldi_data);

	data_t *pAH = (data_t*)0x02FF8000;

	if (*((u32*)(pDH + DO_ioType)) == DEVICE_TYPE_DLDI) {
		// No DLDI patch
		return false;
	}

	if (*(u32*)0x02FF8004 == 0x69684320 || *(u32*)0x02FF8000 == 0x53535A4C) {
		return true; // Skip patching for existing or compressed DLDI file
	}

	dldiFileSize = 1 << pDH[DO_driverSize];

	/* memOffset = readAddr (pAH, DO_text_start);
	if (memOffset == 0) {
			memOffset = readAddr (pAH, DO_startup) - DO_code;
	} */
	memOffset = 0x06000000;
	ddmemOffset = readAddr (pDH, DO_text_start);
	relocationOffset = memOffset - ddmemOffset;

	ddmemStart = readAddr (pDH, DO_text_start);
	ddmemSize = (1 << pDH[DO_driverSize]);
	ddmemEnd = ddmemStart + ddmemSize;

	// Remember how much space is actually reserved
	pDH[DO_allocatedSpace] = pAH[DO_allocatedSpace];
	// Copy the DLDI patch into the application
	tonccpy (pAH, pDH, (dldiFileSize > 0x4000) ? 0x4000 : dldiFileSize);

	// Fix the section pointers in the header
	writeAddr (pAH, DO_text_start, readAddr (pAH, DO_text_start) + relocationOffset);
	writeAddr (pAH, DO_data_end, readAddr (pAH, DO_data_end) + relocationOffset);
	writeAddr (pAH, DO_glue_start, readAddr (pAH, DO_glue_start) + relocationOffset);
	writeAddr (pAH, DO_glue_end, readAddr (pAH, DO_glue_end) + relocationOffset);
	writeAddr (pAH, DO_got_start, readAddr (pAH, DO_got_start) + relocationOffset);
	writeAddr (pAH, DO_got_end, readAddr (pAH, DO_got_end) + relocationOffset);
	writeAddr (pAH, DO_bss_start, readAddr (pAH, DO_bss_start) + relocationOffset);
	writeAddr (pAH, DO_bss_end, readAddr (pAH, DO_bss_end) + relocationOffset);
	// Fix the function pointers in the header
	writeAddr (pAH, DO_startup, readAddr (pAH, DO_startup) + relocationOffset);
	writeAddr (pAH, DO_isInserted, readAddr (pAH, DO_isInserted) + relocationOffset);
	writeAddr (pAH, DO_readSectors, readAddr (pAH, DO_readSectors) + relocationOffset);
	writeAddr (pAH, DO_writeSectors, readAddr (pAH, DO_writeSectors) + relocationOffset);
	writeAddr (pAH, DO_clearStatus, readAddr (pAH, DO_clearStatus) + relocationOffset);
	writeAddr (pAH, DO_shutdown, readAddr (pAH, DO_shutdown) + relocationOffset);

	if (pDH[DO_fixSections] & FIX_ALL) {
		// Search through and fix pointers within the data section of the file
		for (addrIter = (readAddr(pDH, DO_text_start) - ddmemStart); addrIter < (readAddr(pDH, DO_data_end) - ddmemStart); addrIter++) {
			if ((ddmemStart <= readAddr(pAH, addrIter)) && (readAddr(pAH, addrIter) < ddmemEnd)) {
				writeAddr (pAH, addrIter, readAddr(pAH, addrIter) + relocationOffset);
			}
		}
	}

	if (pDH[DO_fixSections] & FIX_GLUE) {
		// Search through and fix pointers within the glue section of the file
		for (addrIter = (readAddr(pDH, DO_glue_start) - ddmemStart); addrIter < (readAddr(pDH, DO_glue_end) - ddmemStart); addrIter++) {
			if ((ddmemStart <= readAddr(pAH, addrIter)) && (readAddr(pAH, addrIter) < ddmemEnd)) {
				writeAddr (pAH, addrIter, readAddr(pAH, addrIter) + relocationOffset);
			}
		}
	}

	if (pDH[DO_fixSections] & FIX_GOT) {
		// Search through and fix pointers within the Global Offset Table section of the file
		for (addrIter = (readAddr(pDH, DO_got_start) - ddmemStart); addrIter < (readAddr(pDH, DO_got_end) - ddmemStart); addrIter++) {
			if ((ddmemStart <= readAddr(pAH, addrIter)) && (readAddr(pAH, addrIter) < ddmemEnd)) {
				writeAddr (pAH, addrIter, readAddr(pAH, addrIter) + relocationOffset);
			}
		}
	}

	if (/* clearBSS && */ (pDH[DO_fixSections] & FIX_BSS) && dldiFileSize <= 0x4000) {
		// Initialise the BSS to 0, only if the disc is being re-inited
		toncset (&pAH[readAddr(pDH, DO_bss_start) - ddmemStart] , 0, readAddr(pDH, DO_bss_end) - readAddr(pDH, DO_bss_start));
	}

	return true;
}

int runNds (const void* loader, u32 loaderSize, u32 cluster, bool initDisc, bool dldiPatchNds, bool loadFromRam, const char* filename, int argc, const char** argv, bool clearMasterBright, bool dsModeSwitch, bool lockScfg, bool boostCpu, bool boostVram, bool tscTgds, int language)
{
	char* argStart;
	u16* argData;
	u16 argTempVal = 0;
	int argSize;
	const char* argChar;

	irqDisable(IRQ_ALL);

	// Direct CPU access to VRAM bank C
	VRAM_C_CR = VRAM_ENABLE | VRAM_C_LCD;
	// Load the loader/patcher into the correct address
	tonccpy (LCDC_BANK_C, loader, loaderSize);
	toncset (LCDC_BANK_C+loaderSize, 0, 0x18000-loaderSize);

	// Set the parameters for the loader
	// STORED_FILE_CLUSTER = cluster;
	writeAddr ((data_t*) LCDC_BANK_C, STORED_FILE_CLUSTER_OFFSET, cluster);
	// INIT_DISC = initDisc;
	writeAddr ((data_t*) LCDC_BANK_C, INIT_DISC_OFFSET, initDisc);

	writeAddr ((data_t*) LCDC_BANK_C, DSIMODE_OFFSET, isDSiMode());
	if (filename[0]=='s' && filename[1]=='d') {
		writeAddr ((data_t*) LCDC_BANK_C, HAVE_DSISD_OFFSET, 1);
	}

	writeAddr ((data_t*) LCDC_BANK_C, CLEAR_MASTER_BRIGHT_OFFSET, clearMasterBright);
	if (isDSiMode()) {
		writeAddr ((data_t*) LCDC_BANK_C, DSMODE_SWITCH_OFFSET, dsModeSwitch);
	}
	writeAddr ((data_t*) LCDC_BANK_C, LOADFROMRAM_OFFSET, loadFromRam);
	writeAddr ((data_t*) LCDC_BANK_C, LANGUAGE_OFFSET, language);
	writeAddr ((data_t*) LCDC_BANK_C, TSC_TGDS_OFFSET, tscTgds);

	// WANT_TO_PATCH_DLDI = dldiPatchNds;
	writeAddr ((data_t*) LCDC_BANK_C, WANT_TO_PATCH_DLDI_OFFSET, dldiPatchNds);
	// Give arguments to loader
	argStart = (char*)LCDC_BANK_C + readAddr((data_t*)LCDC_BANK_C, ARG_START_OFFSET);
	argStart = (char*)(((int)argStart + 3) & ~3);	// Align to word
	argData = (u16*)argStart;
	argSize = 0;

	for (; argc > 0 && *argv; ++argv, --argc) {
		for (argChar = *argv; *argChar != 0; ++argChar, ++argSize) {
			if (argSize & 1) {
				argTempVal |= (*argChar) << 8;
				*argData = argTempVal;
				++argData;
			} else {
				argTempVal = *argChar;
			}
		}
		if (argSize & 1) {
			*argData = argTempVal;
			++argData;
		}
		argTempVal = 0;
		++argSize;
	}
	*argData = argTempVal;

	writeAddr ((data_t*) LCDC_BANK_C, ARG_START_OFFSET, (addr_t)argStart - (addr_t)LCDC_BANK_C);
	writeAddr ((data_t*) LCDC_BANK_C, ARG_SIZE_OFFSET, argSize);


	if (dldiPatchNds) {
		// Patch the loader with a DLDI for the card
		if (!dldiPatchLoader ()) {
			return 3;
		}
	}

	irqDisable(IRQ_ALL);

	if (REG_SCFG_EXT != 0 && dsModeSwitch) {
		tonccpy((u32*)0x023F4000, (u32*)0x02FF4000, 0x8000); // Relocate data to 4MB RAM area

		if (!boostCpu) {
			REG_SCFG_CLK = 0x80;
		}
		if (lockScfg) {
			REG_SCFG_EXT = (boostVram ? 0x03002000 : 0x03000000);		// 4MB memory mode, and lock SCFG
		} else {
			REG_SCFG_EXT = (boostVram ? 0x83002000 : 0x83000000);		// 4MB memory mode
		}
	}

	// Give the VRAM to the ARM7
	VRAM_C_CR = VRAM_ENABLE | VRAM_C_ARM7_0x06000000;
	// Reset into a passme loop
	REG_EXMEMCNT |= ARM7_OWNS_ROM | ARM7_OWNS_CARD;
	*((vu32*)0x02FFFFFC) = 0;
	*((vu32*)0x02FFFE04) = (u32)0xE59FF018;
	*((vu32*)0x02FFFE24) = (u32)0x02FFFE04;

	resetARM7(0x06008000);

	swiSoftReset();
	return true;
}

bool runNds9 (const char* filename, bool dsModeSwitch) {
	if (isDSiMode() || (io_dldi_data->ioInterface.features & FEATURE_SLOT_GBA)) return false;
	const bool isDSi = (REG_SCFG_EXT != 0);

	if (isDSi) {
		if (dsModeSwitch) return false;
	} else {
		return false;
	}

	FILE* ndsFile = fopen(filename, "rb");
	fseek(ndsFile, 0, SEEK_SET);
	fread(__DSiHeader, 1, 0x1000, ndsFile);
	fseek(ndsFile, __DSiHeader->ndshdr.arm9romOffset, SEEK_SET);
	fread((void*)0x02800000, 1, __DSiHeader->ndshdr.arm9binarySize, ndsFile);
	fseek(ndsFile, __DSiHeader->ndshdr.arm7romOffset, SEEK_SET);
	fread((void*)0x02B80000, 1, __DSiHeader->ndshdr.arm7binarySize, ndsFile);
	fclose(ndsFile);

	return true;
}

int runUnlaunchDsi (const char* filename, u32 sector) {
	FILE* ndsFile = fopen(filename, "rb");
	fseek(ndsFile, 0, SEEK_SET);
	fread(__DSiHeader, 1, 0x1000, ndsFile);
	fseek(ndsFile, __DSiHeader->ndshdr.arm9romOffset, SEEK_SET);
	fread((void*)0x02800000, 1, __DSiHeader->ndshdr.arm9binarySize, ndsFile);
	fseek(ndsFile, __DSiHeader->ndshdr.arm7romOffset, SEEK_SET);
	fread((void*)0x02B80000, 1, __DSiHeader->ndshdr.arm7binarySize, ndsFile);
	fclose(ndsFile);

	extern int *removeLauncherPatchesPtr;
	if (!removeLauncherPatchesPtr)
		return 10;

	char ver19String[3];
	tonccpy(ver19String, (char*)0x0280A591, 3);
	char ver20String[3];
	tonccpy(ver20String, (char*)0x0280A616, 3);

	// Patch out splash and sound disable patches
	if (*removeLauncherPatchesPtr == 0) { // 'Off', change launcher TID to block all patches
		const char newID[3] = {'S','A','N'};

		if (memcmp(ver19String, "1.9", 3) == 0) {
			tonccpy((char*)0x02806E74, newID, 3);
		} else if (memcmp(ver20String, "2.0", 3) == 0) {
			tonccpy((char*)0x02806E91, newID, 3);
		} else {
			return 11;
		}
	} else if(*removeLauncherPatchesPtr == 2) { // 'Default', just remove the sound/splash patches
		void *patchSection = (void *)0x02804580;
		const long patchSectionSize = 0x67FD;
		const u32 sanityCheck = 0x00931C10; // First word of the section we're about to patch

		if (memcmp(ver20String, "2.0", 3) != 0)
			return 11;

		FILE *patch = fopen("nitro:/unlaunch-patch.bin", "rb");
		if (!patch)
			return 12;

		fseek(patch, 0, SEEK_END);
		long size = ftell(patch);
		fseek(patch, 0, SEEK_SET);

		if (size > patchSectionSize) // Ensure the patch isn't too big
			return 13;
		else if(*(u32 *)patchSection != sanityCheck) // Something's up, this should match
			return 14;

		toncset(patchSection, 0, patchSectionSize);
		if (fread(patchSection, 1, size, patch) != size)
			return 15;

		fclose(patch);
	}

	extern const char *charUnlaunchBg;
	char bgPath[256];
	sprintf(bgPath, "sd:/_nds/TWiLightMenu/unlaunch/backgrounds/%s", charUnlaunchBg);

	FILE* gifFile = fopen(bgPath, "rb");
	long fsize = 0;
	if (gifFile) {
		fseek(gifFile, 0, SEEK_END);
		fsize = ftell(gifFile); // Get file size
	}

	if (fsize > 0 && fsize <= 0x3C70) {
		// Check GIF
		u16 gifWidth;
		u16 gifHeight;
		fseek(gifFile, 6, SEEK_SET);
		fread(&gifWidth, 1, sizeof(u16), gifFile);
		fread(&gifHeight, 1, sizeof(u16), gifFile);

		if (gifWidth == 256 && gifHeight == 192) {
			// Replace Unlaunch background with custom one

			const u32 gifSignatureStart = 0x38464947;
			const u32 gifSignatureEnd = 0x3B000044;

			u32 iEnd = 0;
			for (u32 i = 0x02800000; i < 0x02810000; i += 4) {
				iEnd = i+0x3C6C;
				if (*(u32*)i == gifSignatureStart && *(u32*)iEnd == gifSignatureEnd) {
					fseek(gifFile, 0, SEEK_SET);
					fread((void*)i, 1, 0x3C70, gifFile);
					break;
				}
			}
		}
		fclose(gifFile);
	}

	return runNds (load_bin, load_bin_size, sector, true, false, true, filename, 0, NULL, true, false, false, true, true, false, -1);
}

int runNdsFile (const char* filename, int argc, const char** argv, bool isRunFromSD, bool dldiPatchNds, bool clearMasterBright, bool dsModeSwitch, bool boostCpu, bool boostVram, bool tscTgds, int language) {
	struct stat st;
	char filePath[PATH_MAX];
	int pathLen;
	const char* args[1];


	if (stat (filename, &st) < 0) {
		return 1;
	}

	if (argc <= 0 || !argv) {
		// Construct a command line if we weren't supplied with one
		if (!getcwd (filePath, PATH_MAX)) {
			return 2;
		}
		pathLen = strlen (filePath);
		strcpy (filePath + pathLen, filename);
		args[0] = filePath;
		argv = args;
	}

	if (strncmp(filename, "sd", 2) != 0 && strncmp(filename, "fat", 3) != 0) {
		filename = argv[0];
	}

	const bool havedsiSD = (access("sd:/", F_OK) == 0);

	if (REG_SCFG_EXT != 0 && havedsiSD) {
		// Check for Unlaunch
		char gameTitle[0xC];
		FILE* ndsFile = fopen(filename, "rb");
		fread(&gameTitle, 1, 0xC, ndsFile);
		fclose(ndsFile);

		if (memcmp(gameTitle, "UNLAUNCH.DSI", 0xC) == 0) {
			return runUnlaunchDsi (filename, st.st_ino);
		}
	}

	bool lockScfg = (strncmp(filename, "fat:/_nds/GBARunner2", 20) != 0
					&& strncmp(filename, "fat:/_nds/TWiLightMenu/emulators/gameyob", 40) != 0);

	bool loadFromRam = runNds9(filename, dsModeSwitch);

	#ifndef _NO_BOOTSTUB_
	installBootStub(havedsiSD, isRunFromSD, dsModeSwitch);
	#endif

	return runNds (load_bin, load_bin_size, st.st_ino, true, (dldiPatchNds && memcmp(io_dldi_data->friendlyName, "Default", 7) != 0), loadFromRam, filename, argc, argv, clearMasterBright, dsModeSwitch, lockScfg, boostCpu, boostVram, tscTgds, language);
}

/*
	b	startUp

storedFileCluster:
	.word	0x0FFFFFFF		@ default BOOT.NDS
initDisc:
	.word	0x00000001		@ init the disc by default
wantToPatchDLDI:
	.word	0x00000001		@ by default patch the DLDI section of the loaded NDS
@ Used for passing arguments to the loaded app
argStart:
	.word	_end - _start
argSize:
	.word	0x00000000
dldiOffset:
	.word	_dldi_start - _start
dsiSD:
	.word	0
*/

#ifndef _NO_BOOTSTUB_
void installBootStub(const bool havedsiSD, const bool isRunFromSD, const bool dsModeSwitch) {
	extern char *fake_heap_end;
	struct __my_bootstub *bootstub = (struct __my_bootstub *)fake_heap_end;
	u32 *bootloader = (u32*)(fake_heap_end+bootstub_bin_size);

	tonccpy(bootstub,bootstub_bin,bootstub_bin_size);
	tonccpy(bootloader,load_bin,load_bin_size);

	bootloader[8] = dsModeSwitch ? 0 : isDSiMode();
	if (havedsiSD && !dsModeSwitch) {
		if (memcmp(io_dldi_data->friendlyName, "Default", 7) != 0) {
			dldiPatchLoader ();
		} else {
			bootloader[3] = 0; // don't dldi patch
		}
		if (isRunFromSD) {
			bootloader[7] = 1; // use internal dsi SD code
		}
	} else {
		dldiPatchLoader ();
	}
	bootstub->arm9reboot = (VoidFn)(((u32)bootstub->arm9reboot)+fake_heap_end);
	bootstub->arm7reboot = (VoidFn)(((u32)bootstub->arm7reboot)+fake_heap_end);
	bootstub->bootaddr += (u32)fake_heap_end;
	bootstub->bootsize = load_bin_size;

	DC_FlushAll();
}
#endif

