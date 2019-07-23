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

using namespace std;

//---------------------------------------------------------------------------------
void stop (void) {
//---------------------------------------------------------------------------------
	while (1) {
		swiWaitForVBlank();
	}
}

char filePath[PATH_MAX];

//---------------------------------------------------------------------------------
void doPause(int x, int y) {
//---------------------------------------------------------------------------------
	printf("Press start...\n");
	while(1) {
		scanKeys();
		if(keysDown() & KEY_START)
			break;
	}
	scanKeys();
}

//---------------------------------------------------------------------------------
int main(int argc, char **argv) {
//---------------------------------------------------------------------------------

	// Turn on screen backlights if they're disabled
	powerOn(PM_BACKLIGHT_TOP);
	powerOn(PM_BACKLIGHT_BOTTOM);
	
	// overwrite reboot stub identifier
	//extern u64 *fake_heap_end;
	//*fake_heap_end = 0;

	//defaultExceptionHandler();
	
	// Go back into DSi mode, if possible
	REG_SCFG_CLK = 0x85;					// TWL clock speed
	//REG_SCFG_EXT = 0x8307F100;				// Extended memory, extended VRAM, etc.

	if (!fatMountSimple("fat", dldiGetInternal())) {
		consoleDemoInit();
		printf ("DLDI init failed!");
		stop();
	}

	bool srldrFound = (access("fat:/_nds/TWiLightMenu/main.srldr", F_OK) == 0);

	int err = 0;
	if (srldrFound) {
		err = runNdsFile ("fat:/_nds/TWiLightMenu/main.srldr", 0, NULL);
	}

	consoleDemoInit();

	if (!srldrFound) {
		iprintf ("fat:/_nds/TWiLightMenu/\nmain.srldr not found.");
	} else {
		iprintf ("Start failed. Error %i\n", err);
	}

	stop();

	return 0;
}
