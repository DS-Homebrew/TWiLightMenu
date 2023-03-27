#include <nds.h>
#include <sys/stat.h>
#include "common/twlmenusettings.h"

#ifndef __CARD_LAUNCH__
#define __CARD_LAUNCH__

const char *unlaunchAutoLoadID = "AutoLoadInfo";
const char16_t hiyaNdsPath[] = u"sdmc:/hiya.dsi";

void unlaunchSetHiyaBoot(void) {
	if (access("sd:/hiya.dsi", F_OK) != 0) return;

	memcpy((u8*)0x02000800, unlaunchAutoLoadID, 12);
	*(u16*)(0x0200080C) = 0x3F0;		// Unlaunch Length for CRC16 (fixed, must be 3F0h)
	*(u16*)(0x0200080E) = 0;			// Unlaunch CRC16 (empty)
	*(u32*)(0x02000810) |= BIT(0);		// Load the title at 2000838h
	*(u32*)(0x02000810) |= BIT(1);		// Use colors 2000814h
	*(u16*)(0x02000814) = 0x7FFF;		// Unlaunch Upper screen BG color (0..7FFFh)
	*(u16*)(0x02000816) = 0x7FFF;		// Unlaunch Lower screen BG color (0..7FFFh)
	memset((u8*)0x02000818, 0, 0x20+0x208+0x1C0);		// Unlaunch Reserved (zero)
	for (uint i = 0; i < sizeof(hiyaNdsPath)/sizeof(hiyaNdsPath[0]); i++) {
		((char16_t*)0x02000838)[i] = hiyaNdsPath[i];		// Unlaunch Device:/Path/Filename.ext (16bit Unicode,end by 0000h)
	}
	*(u16*)(0x0200080E) = swiCRC16(0xFFFF, (void*)0x02000810, 0x3F0);		// Unlaunch CRC16
}

void dsiSysMenuLaunch()
{
    *(u32 *)(0x02000300) = 0x434E4C54; // Set "CNLT" warmboot flag
    *(u16 *)(0x02000304) = 0x1801;
    *(u32 *)(0x02000310) = 0x4D454E55; // "MENU"
	unlaunchSetHiyaBoot();
	DC_FlushAll();						// Make reboot not fail
    fifoSendValue32(FIFO_USER_02, 1);  // ReturntoDSiMenu
}

void dsiLaunchSystemSettings()
{
    char tmdpath[256];
    u8 titleID[4] = {0, 0x42, 0x4E, 0x48};
	switch (ms().sysRegion) {
		case TWLSettings::ERegionJapan:
			titleID[0] = 0x4A;		// JPN
			break;
		case TWLSettings::ERegionUSA:
			titleID[0] = 0x45;		// USA
			break;
		case TWLSettings::ERegionEurope:
			titleID[0] = 0x50;		// EUR
			break;
		case TWLSettings::ERegionAustralia:
			titleID[0] = 0x55;		// AUS
			break;
		case TWLSettings::ERegionChina:
			titleID[0] = 0x43;		// CHN
			break;
		case TWLSettings::ERegionKorea:
			titleID[0] = 0x4B;		// KOR
			break;
		case TWLSettings::ERegionDefault:
			break;
	}
	if (ms().sysRegion == -1) {
		for (u8 i = 0x41; i <= 0x5A; i++) {
			snprintf(tmdpath, sizeof(tmdpath), "sd:/title/00030015/484e42%x/content/title.tmd", i);
			if (access(tmdpath, F_OK) == 0) {
				titleID[0] = i;
				break;
			}
		}
	}

    *(u32 *)(0x02000300) = 0x434E4C54; // Set "CNLT" warmboot flag
    *(u16 *)(0x02000304) = 0x1801;
    *(u8 *)(0x02000308) = titleID[0];
    *(u8 *)(0x02000309) = titleID[1];
    *(u8 *)(0x0200030A) = titleID[2];
    *(u8 *)(0x0200030B) = titleID[3];
    *(u32 *)(0x0200030C) = 0x00030015;
    *(u8 *)(0x02000310) = titleID[0];
    *(u8 *)(0x02000311) = titleID[1];
    *(u8 *)(0x02000312) = titleID[2];
    *(u8 *)(0x02000313) = titleID[3];
    *(u32 *)(0x02000314) = 0x00030015;
    *(u32 *)(0x02000318) = 0x00000017;
    *(u32 *)(0x0200031C) = 0x00000000;
    *(u16 *)(0x02000306) = swiCRC16(0xFFFF, (void *)0x02000308, 0x18);

	unlaunchSetHiyaBoot();
	DC_FlushAll();						// Make reboot not fail
    fifoSendValue32(FIFO_USER_02, 1); // Reboot into System Settings
}

#endif