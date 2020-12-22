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

#include "nitrofs.h"
#include "inifile.h"
#include "tonccpy.h"
#include "fileCopy.h"
#include "tool/stringtool.h"
#include "save/Save.h"
#include "gbaswitch.h"

static u8 blankBuf[0x10000] = {0};
u8 borderData[0x30000] = {0};
std::string gbaBorder = "default.png";

u32 romSize = 0;

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


ITCM_CODE void gptc_patchWait()
{
	// General fix for white screen crash
	// Patch out wait states
	for (u32 addr = 0x080000C0; addr < 0x08000000+romSize; addr+=4) {
		if ((*(u8*)(addr-1) == 0x00 || *(u8*)(addr-1) == 0x03 || *(u8*)(addr-1) == 0x04 || *(u8*)(addr+7) == 0x04
		  || *(u8*)(addr-1) == 0x47 || *(u8*)(addr-1) == 0x81 || *(u8*)(addr-1) == 0x85
		  || *(u8*)(addr-1) == 0xE0 || *(u8*)(addr-1) == 0xE7)
		&& *(u32*)addr == 0x04000204) {
			toncset((u16*)addr, 0, sizeof(u32));
		}
	}

	// Also check at 0x410
	if (*(u32*)0x08000410 == 0x04000204) {
		toncset((u16*)0x08000410, 0, sizeof(u32));
	}
}

void gptc_patchRom()
{
	gptc_patchWait();

	u32 nop = 0xE1A00000;

	u32 gameCode = *(u32*)(0x080000AC);
	if(gameCode == 0x50584C42)
	{
		//Astreix & Obelix XXL (Europe)
		//Fix white screen crash
		if (*(u16*)(0x08000000 + 0x50118) == 0x4014)
			*(u16*)(0x08000000 + 0x50118) = 0x4000;
	}
	else if(gameCode == 0x45474C41)
	{
		//Dragon Ball Z - The Legacy of Goku (USA)
		//Fix white screen crash
		if (*(u16*)(0x08000000 + 0x96E8) == 0x80A8)
			*(u16*)(0x08000000 + 0x96E8) = 0x46C0;

		//Fix "game cannot be played on hardware found" error
		if (*(u16*)(0x08000000 + 0x356) == 0x7002)
			*(u16*)(0x08000000 + 0x356) = 0;

		if (*(u16*)(0x08000000 + 0x35E) == 0x7043)
			*(u16*)(0x08000000 + 0x35E) = 0;

		if (*(u16*)(0x08000000 + 0x37E) == 0x7001)
			*(u16*)(0x08000000 + 0x37E) = 0;

		if (*(u16*)(0x08000000 + 0x382) == 0x7041)
			*(u16*)(0x08000000 + 0x382) = 0;

		if (*(u16*)(0x08000000 + 0xE27E) == 0xB0A2)
		{
			*(u16*)(0x08000000 + 0xE27E) = 0x400;

			for(int i = 0; i < (int)sizeof(sDbzLoGUPatch1); i += 2)
				*(u16*)(0x08000000 + 0xE280 + i) = *(u16*)&sDbzLoGUPatch1[i];

			for(int i = 0; i < (int)sizeof(sDbzLoGUPatch2); i += 2)
				*(u16*)(0x08000000 + 0xE32C + i) = *(u16*)&sDbzLoGUPatch2[i];
		}
	}
	else if(gameCode == 0x50474C41)
	{
		//Dragon Ball Z - The Legacy of Goku (Europe)
		//Fix white screen crash
		if (*(u16*)(0x08000000 + 0x9948) == 0x80B0)
			*(u16*)(0x08000000 + 0x9948) = 0x46C0;

		//Fix "game cannot be played on hardware found" error
		if (*(u16*)(0x08000000 + 0x33C) == 0x7119)
			*(u16*)(0x08000000 + 0x33C) = 0x46C0;

		if (*(u16*)(0x08000000 + 0x340) == 0x7159)
			*(u16*)(0x08000000 + 0x340) = 0x46C0;

		if (*(u16*)(0x08000000 + 0x356) == 0x705A)
			*(u16*)(0x08000000 + 0x356) = 0x46C0;

		if (*(u16*)(0x08000000 + 0x35A) == 0x7002)
			*(u16*)(0x08000000 + 0x35A) = 0x46C0;

		if (*(u16*)(0x08000000 + 0x35E) == 0x7042)
			*(u16*)(0x08000000 + 0x35E) = 0x46C0;

		if (*(u16*)(0x08000000 + 0x384) == 0x7001)
			*(u16*)(0x08000000 + 0x384) = 0x46C0;

		if (*(u16*)(0x08000000 + 0x388) == 0x7041)
			*(u16*)(0x08000000 + 0x388) = 0x46C0;

		if (*(u16*)(0x08000000 + 0x494C) == 0x7002)
			*(u16*)(0x08000000 + 0x494C) = 0x46C0;

		if (*(u16*)(0x08000000 + 0x4950) == 0x7042)
			*(u16*)(0x08000000 + 0x4950) = 0x46C0;

		if (*(u16*)(0x08000000 + 0x4978) == 0x7001)
			*(u16*)(0x08000000 + 0x4978) = 0x46C0;

		if (*(u16*)(0x08000000 + 0x497C) == 0x7041)
			*(u16*)(0x08000000 + 0x497C) = 0x46C0;

		if (*(u16*)(0x08000000 + 0x988E) == 0x7028)
			*(u16*)(0x08000000 + 0x988E) = 0x46C0;

		if (*(u16*)(0x08000000 + 0x9992) == 0x7068)
			*(u16*)(0x08000000 + 0x9992) = 0x46C0;
	}
	else if(gameCode == 0x45464C41)
	{
		//Dragon Ball Z - The Legacy of Goku II (USA)
		tonccpy((u16*)0x080000E0, &nop, sizeof(u32));	// Fix white screen crash

		//Fix "game will not run on the hardware found" error
		if (*(u16*)(0x08000000 + 0x3B8E9E) == 0x1102)
			*(u16*)(0x08000000 + 0x3B8E9E) = 0x1001;

		if (*(u16*)(0x08000000 + 0x3B8EAE) == 0x0003)
			*(u16*)(0x08000000 + 0x3B8EAE) = 0;
	}
	else if(gameCode == 0x4A464C41)
	{
		//Dragon Ball Z - The Legacy of Goku II International (Japan)
		tonccpy((u16*)0x080000E0, &nop, sizeof(u32));	// Fix white screen crash

		//Fix "game will not run on the hardware found" error
		if (*(u16*)(0x08000000 + 0x3FC8F6) == 0x1102)
			*(u16*)(0x08000000 + 0x3FC8F6) = 0x1001;

		if (*(u16*)(0x08000000 + 0x3FC906) == 0x0003)
			*(u16*)(0x08000000 + 0x3FC906) = 0;
	}
	else if(gameCode == 0x50464C41)
	{
		//Dragon Ball Z - The Legacy of Goku II (Europe)
		tonccpy((u16*)0x080000E0, &nop, sizeof(u32));	// Fix white screen crash

		//Fix "game will not run on the hardware found" error
		if (*(u16*)(0x08000000 + 0x6F42B2) == 0x1102)
			*(u16*)(0x08000000 + 0x6F42B2) = 0x1001;

		if (*(u16*)(0x08000000 + 0x6F42C2) == 0x0003)
			*(u16*)(0x08000000 + 0x6F42C2) = 0;
	}
	else if(gameCode == 0x45464C42)
	{
		//2 Games in 1 - Dragon Ball Z - The Legacy of Goku I & II (USA)
		tonccpy((u16*)0x080000E0, &nop, sizeof(u32));	// Fix white screen crash

		if (*(u16*)(0x08000000 + 0x49840) == 0x80A8)
			*(u16*)(0x08000000 + 0x49840) = 0x46C0;

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
		/*if (*(u16*)(0x08000000 + 0x4E316) == 0xB0A2)
		{
			*(u16*)(0x08000000 + 0x4E316) = 0x400;

			for(int i = 0; i < sizeof(sDbzLoGUPatch1); i += 2)
				*(u16*)(0x08000000 + 0x4E318 + i) = *(u16*)&sDbzLoGUPatch1[i];

			for(int i = 0; i < sizeof(sDbzLoGUPatch2); i += 2)
				*(u16*)(0x08000000 + 0x????? + i) = *(u16*)&sDbzLoGUPatch2[i];
		}*/

		//LoG2: Fix "game will not run on the hardware found" error
		if (*(u16*)(0x08000000 + 0xBB9016) == 0x1102)
			*(u16*)(0x08000000 + 0xBB9016) = 0x1001;

		if (*(u16*)(0x08000000 + 0xBB9026) == 0x0003)
			*(u16*)(0x08000000 + 0xBB9026) = 0;
	}
	else if(gameCode == 0x45424442)
	{
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
	}
	else if(gameCode == 0x50424442)
	{
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
	}
	else if(gameCode == 0x45334742)
	{
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
	}
	else if(gameCode == 0x45345442)
	{
		//Dragon Ball GT - Transformation (USA)
		tonccpy((u16*)0x080000E0, &nop, sizeof(u32));	// Fix white screen crash
	}
	else if(gameCode == 0x45465542)
	{
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
	}
	else if(gameCode == 0x45564442)
	{
		//Dragon Ball - Advanced Adventure (USA)
		//Fix white screen crash
		if (*(u16*)(0x08000000 + 0x10C240) == 0x8008)
			*(u16*)(0x08000000 + 0x10C240) = 0x46C0;
	}
	else if(gameCode == 0x50564442)
	{
		//Dragon Ball - Advanced Adventure (Europe)
		//Fix white screen crash
		if (*(u16*)(0x08000000 + 0x10CE3C) == 0x8008)
			*(u16*)(0x08000000 + 0x10CE3C) = 0x46C0;
	}
	else if(gameCode == 0x4A564442)
	{
		//Dragon Ball - Advanced Adventure (Japan)
		//Fix white screen crash
		if (*(u16*)(0x08000000 + 0x10B078) == 0x8008)
			*(u16*)(0x08000000 + 0x10B078) = 0x46C0;
	}
	else if(gameCode == 0x454B3842)
	{
		//Kirby and the Amazing Mirror (USA)
		//Fix white screen crash
		if (*(u16*)(0x08000000 + 0x1515A4) == 0x8008)
			*(u16*)(0x08000000 + 0x1515A4) = 0x46C0;
	}
	else if(gameCode == 0x504B3842)
	{
		//Kirby and the Amazing Mirror (Europe)
		//Fix white screen crash
		if (*(u16*)(0x08000000 + 0x151EE0) == 0x8008)
			*(u16*)(0x08000000 + 0x151EE0) = 0x46C0;
	}
	else if(gameCode == 0x4A4B3842)
	{
		//Hoshi no Kirby - Kagami no Daimeikyuu (Japan) (V1.1)
		//Fix white screen crash
		if (*(u16*)(0x08000000 + 0x151564) == 0x8008)
			*(u16*)(0x08000000 + 0x151564) = 0x46C0;
	}
	else if(gameCode == 0x45533342)
	{
		//Sonic Advance 3 (USA)
		//Fix white screen crash
		if (*(u16*)(0x08000000 + 0xBB67C) == 0x8008)
			*(u16*)(0x08000000 + 0xBB67C) = 0x46C0;
	}
	else if(gameCode == 0x50533342)
	{
		//Sonic Advance 3 (Europe)
		//Fix white screen crash
		if (*(u16*)(0x08000000 + 0xBBA04) == 0x8008)
			*(u16*)(0x08000000 + 0xBBA04) = 0x46C0;
	}
	else if(gameCode == 0x4A533342)
	{
		//Sonic Advance 3 (Japan)
		//Fix white screen crash
		if (*(u16*)(0x08000000 + 0xBB9F8) == 0x8008)
			*(u16*)(0x08000000 + 0xBB9F8) = 0x46C0;
	}
	else if(gameCode == 0x45415741 || gameCode == 0x4A415741)
	{
		//Wario Land 4/Advance (USA/Europe/Japan)
		//Fix white screen crash
		if (*(u16*)(0x08000000 + 0x726) == 0x8008)
			*(u16*)(0x08000000 + 0x726) = 0x46C0;
	}
	else if(gameCode == 0x43415741)
	{
		//Wario Land Advance (iQue)
		//Fix white screen crash
		if (*(u16*)(0x08000000 + 0xE92) == 0x8008)
			*(u16*)(0x08000000 + 0xE92) = 0x46C0;
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

	romSize = getFileSize(argv[1]);

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

	if (*(u16*)(0x020000C0) != 0x5A45) {
		gptc_patchRom();
		//iprintf("ROM patched\n");
	}

	const save_type_t* saveType = save_findTag();
	//iprintf("Save tag found\n");
	if (saveType != NULL && saveType->patchFunc != NULL)
	{
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
			} while(!(pressed & KEY_A));

			consoleClear();
		}
	}

	s2RamAccess(false);
	//iprintf("s2RamAccess(false)\n");

	if (saveType != NULL) {
		std::string savepath = replaceAll(argv[1], ".gba", ".sav");
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
