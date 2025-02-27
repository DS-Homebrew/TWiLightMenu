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
#include <string.h>
#include <nds.h>
#include <nds/arm9/dldi.h>
#include <sys/stat.h>
#include <stdio.h>
#include <limits.h>

#include <unistd.h>
#include <fat.h>

#include "common/lzss.h"
#include "common/tonccpy.h"
//#include "locations.h"

#include "common/nds_bootstrap_loader.h"
#define LCDC_BANK_C (u16*)0x06840000
#define LCDC_BANK_D (u16*)0x06860000
#define STORED_FILE_CLUSTER (*(((u32*)LCDC_BANK_D) + 1))
#define INIT_DISC (*(((u32*)LCDC_BANK_D) + 2))
#define WANT_TO_PATCH_DLDI (*(((u32*)LCDC_BANK_D) + 3))


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

#define STORED_FILE_CLUSTER_OFFSET 4 
#define INIT_DISC_OFFSET 8
#define WANT_TO_PATCH_DLDI_OFFSET 12
#define ARG_START_OFFSET 16
#define ARG_SIZE_OFFSET 20
#define HAVE_DSISD_OFFSET 28
#define LANGUAGE_OFFSET 32
#define DSIMODE_OFFSET 36
#define BOOSTVRAM_OFFSET 40
#define RAM_DISK_CLUSTER_OFFSET 44
#define RAM_DISK_SIZE_OFFSET 48
#define CFG_CLUSTER_OFFSET 52
#define CFG_SIZE_OFFSET 56
#define ROM_FILE_TYPE_OFFSET 60
#define ROM_IS_COMPRESSED_OFFSET 64
#define PATCHCACHE_FILE_OFFSET 68
#define SOFTRESET_FILE_OFFSET 72
#define SOUND_FREQ_OFFSET 76
#define PRELOADED_OFFSET 80


typedef signed int addr_t;
typedef unsigned char data_t;

static addr_t readAddr (data_t *mem, addr_t offset) {
	return ((addr_t*)mem)[offset/sizeof(addr_t)];
}

static void writeAddr (data_t *mem, addr_t offset, addr_t value) {
	((addr_t*)mem)[offset/sizeof(addr_t)] = value;
}

static bool bootloaderFound = false;

char* hbLoad_bin = (char*)0x02FC0000;
char* hbLoadInject_bin = (char*)0x02FD0000;
char* imgTemplateBuffer = (char*)0x02FB0000;

int bootstrapHbRunNds (const void* loader, u32 loaderSize, u32 cluster, u32 ramDiskCluster, u32 ramDiskSize, u32 srParamsCluster, u32 patchOffsetCacheCluster, u32 cfgCluster, u32 cfgSize, int romToRamDisk, bool romIsCompressed, bool initDisc, bool dldiPatchNds, int argc, const char** argv, int language, int dsiMode, bool boostCpu, bool boostVram, int consoleModel, bool soundFreq, u32 srTid1, u32 srTid2, bool ndsPreloaded)
{
	char* argStart;
	u16* argData;
	u16 argTempVal = 0;
	int argSize;
	const char* argChar;
	
	//nocashMessage("runNds");

	irqDisable(IRQ_ALL);

	// Direct CPU access to VRAM bank D
	VRAM_C_CR = VRAM_ENABLE | VRAM_C_LCD;
	VRAM_D_CR = VRAM_ENABLE | VRAM_D_LCD;
	// Load the loader/patcher into the correct address
	tonccpy (LCDC_BANK_C, hbLoadInject_bin, 0x8000);
	tonccpy (LCDC_BANK_D, loader, loaderSize);

	if (romToRamDisk != -1) {
		tonccpy ((u8*)LCDC_BANK_C+0x10000, imgTemplateBuffer, 0xEA00);
	}

	// Set the parameters for the loader
	// STORED_FILE_CLUSTER = cluster;
	writeAddr ((data_t*) LCDC_BANK_D, STORED_FILE_CLUSTER_OFFSET, cluster);
	// INIT_DISC = initDisc;
	writeAddr ((data_t*) LCDC_BANK_D, INIT_DISC_OFFSET, initDisc);

	/*if (argv[0][0]=='s' && argv[0][1]=='d') {
		dldiPatchNds = false;
		writeAddr ((data_t*) LCDC_BANK_D, HAVE_DSISD_OFFSET, 1);
	}*/

	// WANT_TO_PATCH_DLDI = dldiPatchNds;
	writeAddr ((data_t*) LCDC_BANK_D, WANT_TO_PATCH_DLDI_OFFSET, dldiPatchNds);
	// Give arguments to loader
	argStart = (char*)LCDC_BANK_D + readAddr((data_t*)LCDC_BANK_D, ARG_START_OFFSET);
	argStart = (char*)(((int)argStart + 3) & ~3);	// Align to word
	argData = (u16*)argStart;
	argSize = 0;
	
	for (; argc > 0 && *argv; ++argv, --argc) 
	{
		for (argChar = *argv; *argChar != 0; ++argChar, ++argSize) 
		{
			if (argSize & 1) 
			{
				argTempVal |= (*argChar) << 8;
				*argData = argTempVal;
				++argData;
			} 
			else 
			{
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
	
	writeAddr ((data_t*) LCDC_BANK_C, 0x24, consoleModel);
	writeAddr ((data_t*) LCDC_BANK_C, 0x28, srParamsCluster);
	writeAddr ((data_t*) LCDC_BANK_C, 0x2C, srTid1);
	writeAddr ((data_t*) LCDC_BANK_C, 0x30, srTid2);

	writeAddr ((data_t*) LCDC_BANK_D, ARG_START_OFFSET, (addr_t)argStart - (addr_t)LCDC_BANK_D);
	writeAddr ((data_t*) LCDC_BANK_D, ARG_SIZE_OFFSET, argSize);
	writeAddr ((data_t*) LCDC_BANK_D, LANGUAGE_OFFSET, language);
	writeAddr ((data_t*) LCDC_BANK_D, DSIMODE_OFFSET, dsiMode);
	writeAddr ((data_t*) LCDC_BANK_D, BOOSTVRAM_OFFSET, boostVram);
	writeAddr ((data_t*) LCDC_BANK_D, RAM_DISK_CLUSTER_OFFSET, ramDiskCluster);
	writeAddr ((data_t*) LCDC_BANK_D, RAM_DISK_SIZE_OFFSET, ramDiskSize);
	writeAddr ((data_t*) LCDC_BANK_D, CFG_CLUSTER_OFFSET, cfgCluster);
	writeAddr ((data_t*) LCDC_BANK_D, CFG_SIZE_OFFSET, cfgSize);
	writeAddr ((data_t*) LCDC_BANK_D, ROM_FILE_TYPE_OFFSET, romToRamDisk);
	writeAddr ((data_t*) LCDC_BANK_D, ROM_IS_COMPRESSED_OFFSET, romIsCompressed);
	writeAddr ((data_t*) LCDC_BANK_D, PATCHCACHE_FILE_OFFSET, patchOffsetCacheCluster);
	writeAddr ((data_t*) LCDC_BANK_D, SOFTRESET_FILE_OFFSET, srParamsCluster);
	writeAddr ((data_t*) LCDC_BANK_D, SOUND_FREQ_OFFSET, soundFreq);
	writeAddr ((data_t*) LCDC_BANK_D, PRELOADED_OFFSET, ndsPreloaded);


	//nocashMessage("irqDisable(IRQ_ALL);");

	irqDisable(IRQ_ALL);

	if (!boostCpu) {
		REG_SCFG_CLK = 0x80;
	}

	//nocashMessage("Give the VRAM to the ARM7");
	// Give the VRAM to the ARM7
	VRAM_C_CR = VRAM_ENABLE | VRAM_C_ARM7_0x06000000;
	VRAM_D_CR = VRAM_ENABLE | VRAM_D_ARM7_0x06020000;

	//nocashMessage("Reset into a passme loop");
	// Reset into a passme loop
	REG_EXMEMCNT |= ARM7_OWNS_ROM | ARM7_OWNS_CARD;

	*((vu32*)0x02FFFFFC) = 0;
	*((vu32*)0x02FFFE04) = (u32)0xE59FF018;
	*((vu32*)0x02FFFE24) = (u32)0x02FFFE04;
	
	//nocashMessage("resetARM7");

	resetARM7(0x06020000);

	//nocashMessage("swiSoftReset");

	swiSoftReset(); 
	return true;
}

int bootstrapHbRunNdsFile (const char* filename, const char* fatFilename, const char* ramDiskFilename, const char* cfgFilename, u32 ramDiskSize, const char* srParamsFilename, const char* patchOffsetCacheFilename, u32 cfgSize, int romToRamDisk, bool romIsCompressed, int argc, const char** argv, int language, int dsiMode, bool boostCpu, bool boostVram, int consoleModel, bool soundFreq, bool ndsPreloaded) {
	if (!bootloaderFound) {
		return 3;
	}

	u32 srBackendId[2] = {0};
	struct stat st;
	struct stat stRam;
	struct stat stCfg;
	struct stat stPatchCache;
	struct stat stSr;
	u32 clusterRam = 0;
	u32 clusterCfg = 0;
	u32 clusterPatchCache = 0;
	u32 clusterSr = 0;
	char filePath[PATH_MAX];
	int pathLen;
	const char* args[1];

	if (romIsCompressed) {
		FILE *ramDiskTemplate = fopen(ramDiskFilename, "rb");
		if (ramDiskTemplate) {
			fread((void*)0x02900000, 1, ramDiskSize, ramDiskTemplate);
			fclose(ramDiskTemplate);
			if (romToRamDisk == 1) {
				LZ77_Decompress((u8*)0x02900000, (u8*)0x02400000+0xEA00);
			} else if (romToRamDisk == 0 || romToRamDisk == 2 || romToRamDisk == 3 || romToRamDisk == 4) {
				LZ77_Decompress((u8*)0x02900000, (u8*)0x02400000+0xDE00);
			}
		}
	}

	if (stat (filename, &st) < 0) {
		return 1;
	}
	
	if (stat(ramDiskFilename, &stRam) >= 0) {
		clusterRam = stRam.st_ino;
	}

	if (stat(cfgFilename, &stCfg) >= 0) {
		clusterCfg = stCfg.st_ino;
	}

	if (access(patchOffsetCacheFilename, F_OK) != 0) {
		char buffer[0x200] = {0};

		FILE* patchOffsetCacheFile = fopen(patchOffsetCacheFilename, "wb");
		fwrite(buffer, 1, sizeof(buffer), patchOffsetCacheFile);
		fclose(patchOffsetCacheFile);
	}

	if (stat(patchOffsetCacheFilename, &stPatchCache) >= 0) {
		clusterPatchCache = stPatchCache.st_ino;
	}

	if (stat(srParamsFilename, &stSr) >= 0) {
		clusterSr = stSr.st_ino;
	}

	if (argc <= 0 || !argv) {
		// Construct a command line if we weren't supplied with one
		if (!getcwd (filePath, PATH_MAX)) {
			//free(hbLoad_bin);
			//free(hbLoadInject_bin);
			return 2;
		}
		pathLen = strlen (filePath);
		strcpy (filePath + pathLen, fatFilename);
		args[0] = filePath;
		argv = args;
	}

	FILE* srBackendBin = fopen("sd:/_nds/nds-bootstrap/srBackendId.bin", "rb");
	if (srBackendBin) {
		fread(&srBackendId, sizeof(u32), 2, srBackendBin);
		fclose(srBackendBin);
	}

	//bool havedsiSD = false;

	//if (argv[0][0]=='s' && argv[0][1]=='d') havedsiSD = true;
	
	//installBootStub(havedsiSD);

	return bootstrapHbRunNds (hbLoad_bin, 0x10000, st.st_ino, clusterRam, ramDiskSize, clusterSr, clusterPatchCache, clusterCfg, cfgSize, romToRamDisk, romIsCompressed, true, true, argc, argv, language, dsiMode, boostCpu, boostVram, consoleModel, soundFreq, srBackendId[0], srBackendId[1], ndsPreloaded);
}

void bootstrapHbRunPrep (int romToRamDisk) {
	FILE* ramDiskTemplate = fopen("boot:/load.bin", "rb");
	if (ramDiskTemplate) {
		//hbLoad_bin = (char*)malloc(0x10000);
		fread((void*)hbLoad_bin, 1, 0x10000, ramDiskTemplate);
		fclose(ramDiskTemplate);
		bootloaderFound = true;
	} else {
		return;
	}

	ramDiskTemplate = fopen("boot:/loadInject.bin", "rb");
	if (ramDiskTemplate) {
		//hbLoadInject_bin = (char*)malloc(0x8000);
		fread((void*)hbLoadInject_bin, 1, 0x8000, ramDiskTemplate);
		fclose(ramDiskTemplate);
	}

	if (romToRamDisk == 4) {
		ramDiskTemplate = fopen("boot:/imgTemplate_PCE.bin", "rb");
		if (ramDiskTemplate) fread(imgTemplateBuffer, 1, 0xEA00, ramDiskTemplate);
		fclose(ramDiskTemplate);
	} /*else if (romToRamDisk == 3) {
		ramDiskTemplate = fopen("boot:/imgTemplate_GG.bin", "rb");
		if (ramDiskTemplate) fread(imgTemplateBuffer, 1, 0xEA00, ramDiskTemplate);
		fclose(ramDiskTemplate);
	} else if (romToRamDisk == 2) {
		ramDiskTemplate = fopen("boot:/imgTemplate_SMS.bin", "rb");
		if (ramDiskTemplate) fread(imgTemplateBuffer, 1, 0xEA00, ramDiskTemplate);
		fclose(ramDiskTemplate);
	}*/ else if (romToRamDisk == 1) {
		ramDiskTemplate = fopen("boot:/imgTemplate_SNES.bin", "rb");
		if (ramDiskTemplate) fread(imgTemplateBuffer, 1, 0xEA00, ramDiskTemplate);
		fclose(ramDiskTemplate);
	} else if (romToRamDisk == 0) {
		ramDiskTemplate = fopen("boot:/imgTemplate_SegaMD.bin", "rb");
		if (ramDiskTemplate) fread(imgTemplateBuffer, 1, 0xEA00, ramDiskTemplate);
		fclose(ramDiskTemplate);
	}
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
/*bool installBootStub(bool havedsiSD) {
#ifndef _NO_BOOTSTUB_
	nocashMessage("installBootStub");
	extern char *fake_heap_end;
	struct __bootstub *bootcode = (struct __bootstub *)fake_heap_end;

	memcpy(fake_heap_end,bootstub_bin,bootstub_bin_size);
	memcpy(fake_heap_end+bootstub_bin_size,load_bin,load_bin_size);
	bool ret = false;

	if ( havedsiSD) {
		ret = true;
		u32 *bootcode = (u32*)(fake_heap_end+bootstub_bin_size);
		bootcode[3] = 0; // don't dldi patch
		bootcode[7] = 1; // use internal dsi SD code
	} else {
		ret = dldiPatchLoader((data_t*)(fake_heap_end+bootstub_bin_size), load_bin_size,false);
	}
	bootcode->arm9reboot = (VoidFn)(((u32)bootcode->arm9reboot)+fake_heap_end); 
	bootcode->arm7reboot = (VoidFn)(((u32)bootcode->arm7reboot)+fake_heap_end); 
	bootcode->bootsize = load_bin_size;
	
	DC_FlushAll();

	return ret;
#else
	return true;
#endif

}*/

