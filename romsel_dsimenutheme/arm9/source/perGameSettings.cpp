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

#include "perGameSettings.h"
#include <vector>
#include <algorithm>
#include <unistd.h>
#include <string>
#include <sstream>
#include <stdio.h>
#include <dirent.h>

#include <nds.h>
#include <maxmod9.h>
#include <gl2d.h>

#include "date.h"

#include "ndsheaderbanner.h"
#include "iconTitle.h"
#include "graphics/fontHandler.h"
#include "graphics/graphics.h"
#include "graphics/FontGraphic.h"
#include "graphics/TextPane.h"
#include "SwitchState.h"

#include "gbaswitch.h"
#include "nds_loader_arm9.h"

#include "inifile.h"

#include "soundbank.h"
#include "soundbank_bin.h"

#define SCREEN_COLS 32
#define ENTRIES_PER_SCREEN 15
#define ENTRIES_START_ROW 3
#define ENTRY_PAGE_LENGTH 10

const char* SDKnumbertext;

extern bool showdialogbox;

bool perGameSettingsButtons = false;
bool perGameSettingsChanged = false;

int perGameSettings_cursorPosition = 0;
int perGameSettings_boostCpu = -1;

extern int cursorPosition;
extern int pagenum;
extern int file_count;

extern bool flashcardUsed;

std::string pergamefilepath;

extern std::string ReplaceAll(std::string str, const std::string& from, const std::string& to);

extern mm_sound_effect snd_launch;
extern mm_sound_effect snd_select;
extern mm_sound_effect snd_stop;
extern mm_sound_effect snd_wrong;
extern mm_sound_effect snd_back;
extern mm_sound_effect snd_switch;

extern char usernameRendered[10];
extern bool usernameRenderedDone;

char fileCounter[8];

void loadPerGameSettings (std::string filename) {
	pergamefilepath = "sd:/_nds/dsimenuplusplus/gamesettings/"+filename+".ini";
	CIniFile pergameini( pergamefilepath );
	perGameSettings_boostCpu = pergameini.GetInt("GAMESETTINGS", "BOOST_CPU", -1);
}

void savePerGameSettings (std::string filename) {
	pergamefilepath = "sd:/_nds/dsimenuplusplus/gamesettings/"+filename+".ini";
	CIniFile pergameini( pergamefilepath );
	pergameini.SetInt("GAMESETTINGS", "BOOST_CPU", perGameSettings_boostCpu);
	pergameini.SaveIniFile( pergamefilepath );
}

void perGameSettings (std::string filename, const char* username) {
	int pressed = 0;

	clearText();
	if (!flashcardUsed) perGameSettingsButtons = true;
	showdialogbox = true;
	
	snprintf (fileCounter, sizeof(fileCounter), "%i/%i", (cursorPosition+1)+pagenum*40, file_count);
	
	loadPerGameSettings(filename);

	FILE *f_nds_file = fopen(filename.c_str(), "rb");

	u32 SDKVersion = 0;
	char game_TID[5];
	grabTID(f_nds_file, game_TID);
	game_TID[4] = 0;
	game_TID[3] = 0;
	if(strcmp(game_TID, "###") != 0) SDKVersion = getSDKVersion(f_nds_file);
	fclose(f_nds_file);

	if((SDKVersion > 0x1000000) && (SDKVersion < 0x2000000)) {
		SDKnumbertext = "SDK ver: 1";
	} else if((SDKVersion > 0x2000000) && (SDKVersion < 0x3000000)) {
		SDKnumbertext = "SDK ver: 2";
	} else if((SDKVersion > 0x3000000) && (SDKVersion < 0x4000000)) {
		SDKnumbertext = "SDK ver: 3";
	} else if((SDKVersion > 0x4000000) && (SDKVersion < 0x5000000)) {
		SDKnumbertext = "SDK ver: 4";
	} else if((SDKVersion > 0x5000000) && (SDKVersion < 0x6000000)) {
		SDKnumbertext = "SDK ver: 5 (TWLSDK)";
	} else {
		SDKnumbertext = "SDK ver: ?";
	}
	for (int i = 0; i < 30; i++) swiWaitForVBlank();

	while (1) {
		clearText();
		titleUpdate(isDirectory[cursorPosition], filename.c_str());
		printSmall(false, 16, 64, filename.c_str());
		printSmall(false, 16, 166, fileCounter);
		if (flashcardUsed) {
			printSmallCentered(false, 120, SDKnumbertext);
			printSmall(false, 208, 166, "A: OK");
		} else {
			printSmall(false, 16, 80, SDKnumbertext);
			printSmall(false, 24, 112+(perGameSettings_cursorPosition*16), ">");
			printSmall(false, 32, 112, "ARM9 CPU Speed:");
			if (perGameSettings_boostCpu == -1) {
				printSmall(false, 180, 112, "Default");
			} else if (perGameSettings_boostCpu == 1) {
				printSmall(false, 144, 112, "133mhz (TWL)");
			} else {
				printSmall(false, 156, 112, "67mhz (NTR)");
			}
			printSmall(false, 200, 166, "B: Back");
		}
		do {
			consoleClear();
			if(!usernameRenderedDone) {
				for (int i = 0; i < 10; i++) {
					if (username[i] == 0)
						usernameRendered[i] = 0x20;
					else
						usernameRendered[i] = username[i];
				}
				usernameRenderedDone = true;
			}
			iprintf("\n   %s           %s", usernameRendered, RetTime().c_str());

			scanKeys();
			pressed = keysDownRepeat();
			swiWaitForVBlank();
		} while (!pressed);

		if (flashcardUsed) {
			if ((pressed & KEY_A) || (pressed & KEY_B)) {
				break;
			}
		} else {
			//if (pressed & KEY_UP) {
			//	perGameSettings_cursorPosition--;
			//}
			//if (pressed & KEY_DOWN) {
			//	perGameSettings_cursorPosition++;
			//}

			if (pressed & KEY_A) {
				switch (perGameSettings_cursorPosition) {
					case 0:
					default:
						perGameSettings_boostCpu++;
						if (perGameSettings_boostCpu > 1) perGameSettings_boostCpu = -1;
						break;
				}
				perGameSettingsChanged = true;
			}

			if (pressed & KEY_B) {
				if (perGameSettingsChanged) {
					savePerGameSettings(filename);
					perGameSettingsChanged = false;
				}
				break;
			}

			//if (perGameSettings_cursorPosition > 1) perGameSettings_cursorPosition = 0;
			//if (perGameSettings_cursorPosition < 0) perGameSettings_cursorPosition = 1;
		}
	}
	clearText();
	showdialogbox = false;
	for (int i = 0; i < 15; i++) swiWaitForVBlank();
	if (!flashcardUsed) perGameSettingsButtons = false;
}
