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
#include "sr_data_srloader.h"	// For rebooting into DSiMenu++
#include "sr_data_srllastran.h"	// For rebooting the game

extern void* memcpy(const void * src0, void * dst0, int len0);	// Fixes implicit declaration

extern u32 language;
extern u32 gameSoftReset;
static int softResetTimer = 0;

void myIrqHandlerVBlank(void) {
	if (language >= 0 && language < 6) {
		*(u8*)(0x027FFCE4) = language;	// Change language
	}

	if(REG_KEYINPUT & (KEY_L | KEY_R | KEY_DOWN | KEY_B)) {
		softResetTimer = 0;
	} else { 
		if(softResetTimer == 60*2) {
			memcpy((u32*)0x02000300,sr_data_srloader,0x020);
			i2cWriteRegister(0x4a,0x70,0x01);
			i2cWriteRegister(0x4a,0x11,0x01);	// Reboot into DSiMenu++
		}
		softResetTimer++;
	}

	if(REG_KEYINPUT & (KEY_L | KEY_R | KEY_START | KEY_SELECT)) {
	} else if (!gameSoftReset) {
    	memcpy((u32*)0x02000300,sr_data_srllastran,0x020);
    	i2cWriteRegister(0x4a,0x70,0x01);
    	i2cWriteRegister(0x4a,0x11,0x01);	// Reboot game
	}

}
