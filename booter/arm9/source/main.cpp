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
#include <stdio.h>
#include <fat.h>
#include <sys/stat.h>
#include <limits.h>

#include <string.h>
#include <unistd.h>
#include <gl2d.h>

#include "graphics/graphics.h"

#include "nds_loader_arm9.h"

#include "graphics/fontHandler.h"

bool fadeType = false;		// false = out, true = in

using namespace std;

//---------------------------------------------------------------------------------
void stop (void) {
//---------------------------------------------------------------------------------
	while (1) {
		swiIntrWait(0, 1);
	}
}

char filePath[PATH_MAX];

//---------------------------------------------------------------------------------
void doPause(int x, int y) {
//---------------------------------------------------------------------------------
	// iprintf("Press start...\n");
	printSmall(false, x, y, "Press start...");
	while(1) {
		scanKeys();
		if(keysDown() & KEY_START)
			break;
	}
	scanKeys();
}

std::string ReplaceAll(std::string str, const std::string& from, const std::string& to) {
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
    }
    return str;
}

//---------------------------------------------------------------------------------
int main(int argc, char **argv) {
//---------------------------------------------------------------------------------

	// Turn on screen backlights if they're disabled
	powerOn(PM_BACKLIGHT_TOP);
	powerOn(PM_BACKLIGHT_BOTTOM);
	
	bool graphicsInited = false;

	if (!fatInitDefault()) {
		if(!graphicsInited) {
			graphicsInit();
			fontInit();
			graphicsInited = true;
			fadeType = true;
		}
		clearText();
		printSmall(false, 4, 4, "fatinitDefault failed!");
		stop();
	}

	int err = runNdsFile ("/_nds/dsimenuplusplus/main.srldr", 0, NULL);

	if(!graphicsInited) {
		graphicsInit();
		fontInit();
		graphicsInited = true;
		fadeType = true;
	}

	clearText();
	if (err == 1) {
		printSmall(false, 4, 4, "sd:/_nds/dsimenuplusplus/");
		printSmall(false, 4, 12, "main.srldr not found.");
	} else {
		char errorText[16];
		snprintf(errorText, sizeof(errorText), "Error %i", err);
		printSmall(false, 4, 4, "Unable to start main.srldr");
		printSmall(false, 4, 12, errorText);
	}
	printSmall(false, 4, 28, "Press B to return to menu.");
	
	while (1) {
		scanKeys();
		if (keysHeld() & KEY_B) fifoSendValue32(FIFO_USER_01, 1);	// Tell ARM7 to reboot into 3DS HOME Menu (power-off/sleep mode screen skipped)
	}

	return 0;
}
