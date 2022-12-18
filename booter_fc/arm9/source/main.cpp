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
#include <vector>

#include <string.h>
#include <unistd.h>

#include "nds_loader_arm9.h"
#include "common/tonccpy.h"

using namespace std;

//---------------------------------------------------------------------------------
int main(int argc, char **argv) {
//---------------------------------------------------------------------------------

	//defaultExceptionHandler();

	// overwrite reboot stub identifier
	extern char *fake_heap_end;
	*fake_heap_end = 0;

	REG_SCFG_CLK &= ~(BIT(1) | BIT(2));	// Disable DSP and Camera Interface clocks

	// Turn on screen backlights if they're disabled
	powerOn(PM_BACKLIGHT_TOP);
	powerOn(PM_BACKLIGHT_BOTTOM);
	
	REG_SCFG_CLK = 0x85;					// TWL clock speed
	REG_SCFG_EXT = 0x8307F100;				// Extended memory, 32-bit VRAM bus, etc.

	//consoleDemoInit();

	bool isRegularDS = fifoGetValue32(FIFO_USER_07);

	//iprintf("Initing flashcard...\n");
	bool flashcardFound = fatMountSimple("fat", dldiGetInternal());
	if (isDSiMode() || (!isRegularDS && REG_SCFG_EXT != 0)) {
		extern const DISC_INTERFACE __my_io_dsisd;
		//iprintf("Initing SD...\n");
		fatMountSimple("sd", &__my_io_dsisd);
	}
	bool primaryIsSd = (access("sd:/_nds/primary", F_OK) == 0 || access("fat:/_nds/TWiLightMenu/main.srldr", F_OK) != 0);
	if (REG_SCFG_EXT != 0) {
		FILE* twlCfgFile = fopen(primaryIsSd ? "sd:/_nds/TWiLightMenu/16KBcache.bin" : "fat:/_nds/TWiLightMenu/16KBcache.bin", "rb");
		fread((void*)0x02400000, 1, 0x4000, twlCfgFile);
		fclose(twlCfgFile);
	}

	if (/* !sdFound && */ !flashcardFound) {
		consoleDemoInit();
		printf ("FAT init failed!\n");
	} else {
		vector<char *> argarray;
		argarray.push_back((char*)(primaryIsSd ? "sd:/_nds/TWiLightMenu/main.srldr" : "fat:/_nds/TWiLightMenu/main.srldr"));

		//iprintf(sdFound ? "Running from SD...\n" : "Running from flashcard...\n");
		int err = runNdsFile (argarray[0], argarray.size(), (const char**)&argarray[0]);

		consoleDemoInit();

		if (err == 1) {
			iprintf ("fat:/_nds/TWiLightMenu/\nmain.srldr not found.\n");
		} else {
			iprintf ("Start failed. Error %i\n", err);
		}
	}

	iprintf(isRegularDS ? "\nPress B to power off." : "\nPress B to return to menu.");

	while (1) {
		scanKeys();
		if (keysDown() & KEY_B) {
			fifoSendValue32(FIFO_USER_01, 1);	// Tell ARM7 to reboot or poweroff depending on whether DS or DSi
			break;
		}
		swiWaitForVBlank();
	}

	for (int i = 0; i < 10; i++) {
		swiWaitForVBlank();
	}

	return 0;
}
