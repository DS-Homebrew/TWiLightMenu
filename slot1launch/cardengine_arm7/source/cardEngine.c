/*
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

#include <nds/fifomessages.h>
#include <nds/ipc.h>
#include <nds/interrupts.h>
#include <nds/system.h>
#include <nds/input.h>
#include <nds/arm7/audio.h>

#include <string.h>
#include "cardEngine.h"
#include "i2c.h"

#include "sr_data_error.h"	// For showing an error screen
#include "sr_data_srloader.h"	// For rebooting into TWiLight Menu++
#include "sr_data_srllastran.h"	// For rebooting the game

static const char *unlaunchAutoLoadID = "AutoLoadInfo";
static const char bootNdsPath[15] = "sdmc:/boot.nds";
static const char resetGameSrldrPath[40] = "sdmc:/_nds/TWiLightMenu/main.srldr";

extern void cheat_engine_start(void);

extern u32 language;
extern u32 gameSoftReset;
static int softResetTimer = 0;

static void unlaunchSetFilename(bool boot) {
	memcpy((u8*)0x02000800, unlaunchAutoLoadID, 12);
	*(u16*)(0x0200080C) = 0x3F0;		// Unlaunch Length for CRC16 (fixed, must be 3F0h)
	*(u16*)(0x0200080E) = 0;			// Unlaunch CRC16 (empty)
	*(u32*)(0x02000810) = (BIT(0) | BIT(1));		// Load the title at 2000838h
													// Use colors 2000814h
	*(u16*)(0x02000814) = 0x7FFF;		// Unlaunch Upper screen BG color (0..7FFFh)
	*(u16*)(0x02000816) = 0x7FFF;		// Unlaunch Lower screen BG color (0..7FFFh)
	memset((u8*)0x02000818, 0, 0x20+0x208+0x1C0);		// Unlaunch Reserved (zero)
	if (boot) {
		for (unsigned int i = 0; i < sizeof(bootNdsPath)/sizeof(bootNdsPath[0]); i++) {
			((u16*)0x02000838)[i] = bootNdsPath[i];		// Unlaunch Device:/Path/Filename.ext (16bit Unicode,end by 0000h)
		}
	} else {
		for (unsigned int i = 0; i < sizeof(resetGameSrldrPath)/sizeof(resetGameSrldrPath[0]); i++) {
			((u16*)0x02000838)[i] = resetGameSrldrPath[i];		// Unlaunch Device:/Path/Filename.ext (16bit Unicode,end by 0000h)
		}
	}
	while (*(u16*)(0x0200080E) == 0) {	// Keep running, so that CRC16 isn't 0
		*(u16*)(0x0200080E) = swiCRC16(0xFFFF, (void*)0x02000810, 0x3F0);		// Unlaunch CRC16
	}
}

void myIrqHandlerVBlank(void) {
	if (language >= 0 && language <= 7) {
		// Change language
		*(u8*)(0x027FFCE4) = language;
	}

	if (0 == (REG_KEYINPUT & (KEY_L | KEY_R | KEY_DOWN | KEY_B))) {
		if (softResetTimer == 60*2) {
			REG_MASTER_VOLUME = 0;
			int oldIME = enterCriticalSection();
			memset((u32*)0x02000000, 0, 0x400);
			unlaunchSetFilename(true);
			memcpy((u32*)0x02000300, sr_data_srloader, 0x20);
			i2cWriteRegister(0x4a,0x70,0x01);
			i2cWriteRegister(0x4a,0x11,0x01);	// Reboot into TWiLight Menu++
			leaveCriticalSection(oldIME);
		}
		softResetTimer++;
	} else {
		softResetTimer = 0;
	}

	if ((0 == (REG_KEYINPUT & (KEY_L | KEY_R | KEY_START | KEY_SELECT))) && !gameSoftReset) {
		REG_MASTER_VOLUME = 0;
		int oldIME = enterCriticalSection();
		unlaunchSetFilename(false);
		memset((u32*)0x02000000, 0, 0x400);
		*(u32*)(0x02000000) = BIT(3);
		memcpy((u32*)0x02000300, sr_data_srllastran, 0x20);
		i2cWriteRegister(0x4a,0x70,0x01);
		i2cWriteRegister(0x4a,0x11,0x01);	// Reboot game
		leaveCriticalSection(oldIME);
	}

	if (REG_IE & IRQ_NETWORK) {
		REG_IE &= ~IRQ_NETWORK; // DSi RTC fix
	}

	#ifdef DEBUG
	nocashMessage("cheat_engine_start\n");
	#endif	
	
	cheat_engine_start();
}
