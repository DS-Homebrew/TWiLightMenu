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
#include "cardEngine.h"
#include "i2c.h"

#include "sr_data_error.h"	// For showing an error screen
#include "sr_data_srloader.h"	// For rebooting into TWiLight Menu++
#include "sr_data_srllastran.h"	// For rebooting the game

static const char *unlaunchAutoLoadID = "AutoLoadInfo";
static char hiyaNdsPath[14] = {'s','d','m','c',':','/','h','i','y','a','.','d','s','i'};

extern void* memcpy(const void * src0, void * dst0, int len0);	// Fixes implicit declaration

extern u32 language;
extern u32 gameSoftReset;
static int softResetTimer = 0;

static void unlaunchSetHiyaBoot(void) {
	memcpy((u8*)0x02000800, unlaunchAutoLoadID, 12);
	*(u16*)(0x0200080C) = 0x3F0;		// Unlaunch Length for CRC16 (fixed, must be 3F0h)
	*(u16*)(0x0200080E) = 0;			// Unlaunch CRC16 (empty)
	*(u32*)(0x02000810) = (BIT(0) | BIT(1));		// Load the title at 2000838h
													// Use colors 2000814h
	*(u16*)(0x02000814) = 0x7FFF;		// Unlaunch Upper screen BG color (0..7FFFh)
	*(u16*)(0x02000816) = 0x7FFF;		// Unlaunch Lower screen BG color (0..7FFFh)
	memset((u8*)0x02000818, 0, 0x20+0x208+0x1C0);		// Unlaunch Reserved (zero)
	int i2 = 0;
	for (int i = 0; i < 14; i++) {
		*(u8*)(0x02000838+i2) = hiyaNdsPath[i];		// Unlaunch Device:/Path/Filename.ext (16bit Unicode,end by 0000h)
		i2 += 2;
	}
	while (*(u16*)(0x0200080E) == 0) {	// Keep running, so that CRC16 isn't 0
		*(u16*)(0x0200080E) = swiCRC16(0xFFFF, (void*)0x02000810, 0x3F0);		// Unlaunch CRC16
	}
}

void myIrqHandlerVBlank(void) {
	if (language >= 0 && language < 6) {
		*(u8*)(0x027FFCE4) = language;	// Change language
	}

	if (0 == (REG_KEYINPUT & (KEY_L | KEY_R | KEY_DOWN | KEY_B))) {
		if(softResetTimer == 60*2) {
			REG_MASTER_VOLUME = 0;
			int oldIME = enterCriticalSection();
			unlaunchSetHiyaBoot();
			memcpy((u32*)0x02000300,sr_data_srloader,0x020);
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
		unlaunchSetHiyaBoot();
    	memcpy((u32*)0x02000300,sr_data_srllastran,0x020);
    	i2cWriteRegister(0x4a,0x70,0x01);
    	i2cWriteRegister(0x4a,0x11,0x01);	// Reboot game
		leaveCriticalSection(oldIME);
	}

	#ifdef DEBUG
	nocashMessage("cheat_engine_start\n");
	#endif	
	
	cheat_engine_start();
}
