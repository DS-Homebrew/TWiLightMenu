/*-----------------------------------------------------------------
 Copyright (C) 2005 - 2013
	Michael "Chishm" Chisholm
	Dave "WinterMute" Murphy
	Claudio "sverx"

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
#include <nds.h>
#include <nds/arm9/dldi.h>
#include <stdio.h>
#include <fat.h>
#include <sys/stat.h>
#include <limits.h>

#include <string.h>
#include <unistd.h>

#include "nds_loader_arm9.h"
#include "tonccpy.h"

using namespace std;

//---------------------------------------------------------------------------------
int main(int argc, char **argv) {
//---------------------------------------------------------------------------------

	//defaultExceptionHandler();

	// overwrite reboot stub identifier
	extern char *fake_heap_end;
	*fake_heap_end = 0;

	REG_SCFG_CLK &= (BIT(1) | BIT(2));	// Disable DSP and Camera Interface clocks

	// Turn on screen backlights if they're disabled
	powerOn(PM_BACKLIGHT_TOP);
	powerOn(PM_BACKLIGHT_BOTTOM);
	
	REG_SCFG_CLK = 0x85;					// TWL clock speed
	REG_SCFG_EXT = 0x8307F100;				// Extended memory, extended VRAM, etc.

	if (*(u32*)0x02400000 == 1) {
		tonccpy((char*)0x02000000, (char*)0x02400000, 0x4000);	// Grab TWLCFG backup, including boot splash flag
	}

	bool isRegularDS = true;
	u16 arm7_SNDEXCNT = fifoGetValue32(FIFO_USER_07);
	if (arm7_SNDEXCNT != 0) isRegularDS = false;	// If sound frequency setting is found, then the console is not a DS Phat/Lite

	/*bool sdFound = false;
	#ifndef CYCLODSI
	if (isDSiMode() || (!isRegularDS && REG_SCFG_EXT != 0)) {
		extern const DISC_INTERFACE __my_io_dsisd;
		sdFound = fatMountSimple("sd", &__my_io_dsisd);
	}
	#endif*/
	bool flashcardFound = fatMountSimple("fat", dldiGetInternal());

	if (/* !sdFound && */ !flashcardFound) {
		consoleDemoInit();
		printf ("FAT init failed!\n");
	} else {
		int err = 0;
		err = runNdsFile ((/*sdFound ? "sd:/_nds/TWiLightMenu/main.srldr" :*/ "fat:/_nds/TWiLightMenu/main.srldr"), 0, NULL);

		consoleDemoInit();

		if (err == 1) {
			printf ("fat:/_nds/TWiLightMenu/\nmain.srldr not found.\n");
		} else {
			printf ("Start failed. Error %i\n", err);
		}
	}

	printf(isRegularDS ? "\nPress B to power off." : "\nPress B to return to menu.");

	while (1) {
		scanKeys();
		if (keysHeld() & KEY_B) {
			fifoSendValue32(FIFO_USER_01, 1);	// Tell ARM7 to reboot into System Menu (power-off/sleep mode screen skipped)
			break;
		}
	}

	for (int i = 0; i < 10; i++) {
		swiWaitForVBlank();
	}

	return 0;
}
