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
extern int dialogboxHeight;

bool perGameSettingsChanged = false;

int perGameSettings_cursorPosition = 0;
int perGameSettings_language = -2;
int perGameSettings_boostCpu = -1;
int perGameSettings_boostVram = -1;
int perGameSettings_soundFix = -1;
int perGameSettings_asyncPrefetch = -1;

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
char gameTIDText[16];

void loadPerGameSettings (std::string filename) {
	pergamefilepath = "sd:/_nds/dsimenuplusplus/gamesettings/"+filename+".ini";
	CIniFile pergameini( pergamefilepath );
	perGameSettings_language = pergameini.GetInt("GAMESETTINGS", "LANGUAGE", -2);
	perGameSettings_boostCpu = pergameini.GetInt("GAMESETTINGS", "BOOST_CPU", -1);
	perGameSettings_asyncPrefetch = pergameini.GetInt("GAMESETTINGS", "ASYNC_PREFETCH", -1);
}

void savePerGameSettings (std::string filename) {
	pergamefilepath = "sd:/_nds/dsimenuplusplus/gamesettings/"+filename+".ini";
	CIniFile pergameini( pergamefilepath );
	pergameini.SetInt("GAMESETTINGS", "LANGUAGE", perGameSettings_language);
	pergameini.SetInt("GAMESETTINGS", "BOOST_CPU", perGameSettings_boostCpu);
	pergameini.SetInt("GAMESETTINGS", "ASYNC_PREFETCH", perGameSettings_asyncPrefetch);
	pergameini.SaveIniFile( pergamefilepath );
}

void perGameSettings (std::string filename) {
	int pressed = 0;

	clearText();
	
	loadPerGameSettings(filename);

	std::string filenameForInfo = filename;
	if((filenameForInfo.substr(filenameForInfo.find_last_of(".") + 1) == "argv")
	|| (filenameForInfo.substr(filenameForInfo.find_last_of(".") + 1) == "launcharg"))
	{
		std::vector<char*> argarray;

		FILE *argfile = fopen(filenameForInfo.c_str(),"rb");
			char str[PATH_MAX], *pstr;
		const char seps[]= "\n\r\t ";

		while( fgets(str, PATH_MAX, argfile) ) {
			// Find comment and end string there
			if( (pstr = strchr(str, '#')) )
				*pstr= '\0';

			// Tokenize arguments
			pstr= strtok(str, seps);

			while( pstr != NULL ) {
				argarray.push_back(strdup(pstr));
				pstr= strtok(NULL, seps);
			}
		}
		fclose(argfile);
		filenameForInfo = argarray.at(0);
	}

	FILE *f_nds_file = fopen(filenameForInfo.c_str(), "rb");

	bool showSDKVersion = false;
	u32 SDKVersion = 0;
	char game_TID[5];
	grabTID(f_nds_file, game_TID);
	game_TID[4] = 0;
	game_TID[3] = 0;
	if(strcmp(game_TID, "###") != 0 || !isHomebrew) {
		SDKVersion = getSDKVersion(f_nds_file);
		showSDKVersion = true;
	}

	char gameTIDDisplay[5];
	grabTID(f_nds_file, gameTIDDisplay);
	gameTIDDisplay[4] = 0;
	fclose(f_nds_file);

	snprintf (gameTIDText, sizeof(gameTIDText), "TID: %s", gameTIDDisplay);

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
	if (isDSiWare || flashcardUsed) {
		dialogboxHeight = 0;
	} else {
		dialogboxHeight = 5;
	}
	showdialogbox = true;

	while (1) {
		clearText();
		titleUpdate(isDirectory, filename.c_str());
		if (isDSiWare || isHomebrew || flashcardUsed) {
			printLargeCentered(false, 84, "Info");
			if (showSDKVersion) printSmall(false, 24, 104, SDKnumbertext);
			printSmall(false, 172, 104, gameTIDText);
			printSmallCentered(false, 118, "A: OK");
		} else {
			printLargeCentered(false, 84, "Game settings");
			if (showSDKVersion) printSmall(false, 24, 98, SDKnumbertext);
			printSmall(false, 172, 98, gameTIDText);
			printSmall(false, 24, 112+(perGameSettings_cursorPosition*8), ">");
			printSmall(false, 32, 112, "Language:");
			printSmall(false, 32, 120, "ARM9 CPU Speed:");
			printSmall(false, 32, 128, "VRAM boost:");
			printSmall(false, 32, 136, "Sound fix:");
			printSmall(false, 32, 144, "Async prefetch:");
			if (perGameSettings_language == -2) {
				printSmall(false, 180, 112, "Default");
			} else if (perGameSettings_language == -1) {
				printSmall(false, 180, 112, "System");
			} else if (perGameSettings_language == 0) {
				printSmall(false, 172, 112, "Japanese");
			} else if (perGameSettings_language == 1) {
				printSmall(false, 180, 112, "English");
			} else if (perGameSettings_language == 2) {
				printSmall(false, 180, 112, "French");
			} else if (perGameSettings_language == 3) {
				printSmall(false, 180, 112, "German");
			} else if (perGameSettings_language == 4) {
				printSmall(false, 180, 112, "Italian");
			} else if (perGameSettings_language == 5) {
				printSmall(false, 180, 112, "Spanish");
			}
			if (perGameSettings_boostCpu == -1) {
				printSmall(false, 180, 120, "Default");
			} else if (perGameSettings_boostCpu == 1) {
				printSmall(false, 153, 120, "133mhz (TWL)");
			} else {
				printSmall(false, 156, 120, "67mhz (NTR)");
			}
			if (perGameSettings_boostVram == -1) {
				printSmall(false, 180, 128, "Default");
			} else if (perGameSettings_boostVram == 1) {
				printSmall(false, 180, 128, "On");
			} else {
				printSmall(false, 180, 128, "Off");
			}
			if (perGameSettings_soundFix == -1) {
				printSmall(false, 180, 136, "Default");
			} else if (perGameSettings_soundFix == 1) {
				printSmall(false, 180, 136, "On");
			} else {
				printSmall(false, 180, 136, "Off");
			}
			if (perGameSettings_asyncPrefetch == -1) {
				printSmall(false, 180, 144, "Default");
			} else if (perGameSettings_asyncPrefetch == 1) {
				printSmall(false, 180, 144, "On");
			} else {
				printSmall(false, 180, 144, "Off");
			}
			printSmallCentered(false, 142, "B: Back");
		}
		do {
			scanKeys();
			pressed = keysDownRepeat();
			swiWaitForVBlank();
		} while (!pressed);

		if (isDSiWare || isHomebrew || flashcardUsed) {
			if ((pressed & KEY_A) || (pressed & KEY_B)) {
				break;
			}
		} else {
			if (pressed & KEY_UP) {
				perGameSettings_cursorPosition--;
				if (perGameSettings_cursorPosition < 0) perGameSettings_cursorPosition = 4;
			}
			if (pressed & KEY_DOWN) {
				perGameSettings_cursorPosition++;
				if (perGameSettings_cursorPosition > 4) perGameSettings_cursorPosition = 0;
			}

			if (pressed & KEY_A) {
				switch (perGameSettings_cursorPosition) {
					case 0:
					default:
						perGameSettings_language++;
						if (perGameSettings_language > 5) perGameSettings_language = -2;
						break;
					case 1:
						perGameSettings_boostCpu++;
						if (perGameSettings_boostCpu > 1) perGameSettings_boostCpu = -1;
						break;
					case 2:
						perGameSettings_asyncPrefetch++;
						if (perGameSettings_asyncPrefetch > 1) perGameSettings_asyncPrefetch = -1;
						break;
					case 3:
						perGameSettings_soundFix++;
						if (perGameSettings_soundFix > 1) perGameSettings_soundFix = -1;
						break;
					case 4:
						perGameSettings_asyncPrefetch++;
						if (perGameSettings_asyncPrefetch > 1) perGameSettings_asyncPrefetch = -1;
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
		}
	}
	clearText();
	showdialogbox = false;
	dialogboxHeight = 0;
}
