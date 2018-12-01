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
#include "common/gl2d.h"

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
#include "flashcard.h"

#include "soundbank.h"
#include "soundbank_bin.h"

#define SCREEN_COLS 32
#define ENTRIES_PER_SCREEN 15
#define ENTRIES_START_ROW 3
#define ENTRY_PAGE_LENGTH 10

const char* SDKnumbertext;

extern bool showdialogbox;
extern bool dbox_showIcon;

bool perGameSettingsChanged = false;

int perGameSettings_cursorPosition = 0;
bool perGameSettings_directBoot = false;	// Homebrew only
int perGameSettings_dsiMode = -1;
int perGameSettings_language = -2;
int perGameSettings_boostCpu = -1;
int perGameSettings_boostVram = -1;
int perGameSettings_soundFix = -1;
int perGameSettings_asyncPrefetch = -1;

extern int cursorPosition[2];
extern int pagenum[2];
extern int file_count;

char pergamefilepath[256];

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
	snprintf(pergamefilepath, sizeof(pergamefilepath), "%s/_nds/TWiLightMenu/gamesettings/%s.ini", (secondaryDevice ? "fat:" : "sd:"), filename.c_str());
	CIniFile pergameini( pergamefilepath );
	perGameSettings_directBoot = pergameini.GetInt("GAMESETTINGS", "DIRECT_BOOT", secondaryDevice);	// Homebrew only
	perGameSettings_dsiMode = pergameini.GetInt("GAMESETTINGS", "DSI_MODE", -1);
	perGameSettings_language = pergameini.GetInt("GAMESETTINGS", "LANGUAGE", -2);
	perGameSettings_boostCpu = pergameini.GetInt("GAMESETTINGS", "BOOST_CPU", -1);
	perGameSettings_boostVram = pergameini.GetInt("GAMESETTINGS", "BOOST_VRAM", -1);
	perGameSettings_soundFix = pergameini.GetInt("GAMESETTINGS", "SOUND_FIX", -1);
	perGameSettings_asyncPrefetch = pergameini.GetInt("GAMESETTINGS", "ASYNC_PREFETCH", -1);
}

void savePerGameSettings (std::string filename) {
	snprintf(pergamefilepath, sizeof(pergamefilepath), "%s/_nds/TWiLightMenu/gamesettings/%s.ini", (secondaryDevice ? "fat:" : "sd:"), filename.c_str());
	CIniFile pergameini( pergamefilepath );
	if (isHomebrew[cursorPosition[secondaryDevice]] == 1) {
		pergameini.SetInt("GAMESETTINGS", "DIRECT_BOOT", perGameSettings_directBoot);
		if (isDSiMode()) {
			pergameini.SetInt("GAMESETTINGS", "DSI_MODE", perGameSettings_dsiMode);
			pergameini.SetInt("GAMESETTINGS", "BOOST_CPU", perGameSettings_boostCpu);
			pergameini.SetInt("GAMESETTINGS", "BOOST_VRAM", perGameSettings_boostVram);
		}
	} else if (isDSiMode()) {
		if (!secondaryDevice) pergameini.SetInt("GAMESETTINGS", "LANGUAGE", perGameSettings_language);
		pergameini.SetInt("GAMESETTINGS", "DSI_MODE", perGameSettings_dsiMode);
		pergameini.SetInt("GAMESETTINGS", "BOOST_CPU", perGameSettings_boostCpu);
		pergameini.SetInt("GAMESETTINGS", "BOOST_VRAM", perGameSettings_boostVram);
		if (!secondaryDevice) pergameini.SetInt("GAMESETTINGS", "SOUND_FIX", perGameSettings_soundFix);
	}
	pergameini.SaveIniFile( pergamefilepath );
}

void perGameSettings (std::string filename) {
	int pressed = 0;

	clearText();
	dbox_showIcon = true;
	showdialogbox = true;
	
	snprintf (fileCounter, sizeof(fileCounter), "%i/%i", (cursorPosition[secondaryDevice]+1)+pagenum[secondaryDevice]*40, file_count);
	
	perGameSettings_cursorPosition = 0;
	loadPerGameSettings(filename);

	std::string filenameForInfo = filename;
	bool isLauncharg = ((filenameForInfo.substr(filenameForInfo.find_last_of(".") + 1) == "launcharg")
					|| (filenameForInfo.substr(filenameForInfo.find_last_of(".") + 1) == "LAUNCHARG"));
	if((filenameForInfo.substr(filenameForInfo.find_last_of(".") + 1) == "argv")
	|| (filenameForInfo.substr(filenameForInfo.find_last_of(".") + 1) == "ARGV")
	|| isLauncharg)
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

		if (isLauncharg) {
			char appPath[256];
			for (u8 appVer = 0; appVer <= 0xFF; appVer++)
			{
				if (appVer > 0xF) {
					snprintf(appPath, sizeof(appPath), "%scontent/000000%x.app", filenameForInfo.c_str(), appVer);
				} else {
					snprintf(appPath, sizeof(appPath), "%scontent/0000000%x.app", filenameForInfo.c_str(), appVer);
				}
				/*printSmall(false, 16, 64, appPath);
				printSmall(false, -128, 80, appPath);
				while (1) {
					swiWaitForVBlank();
				}*/
				if (access(appPath, F_OK) == 0)
				{
					break;
				}
			}
			filenameForInfo = appPath;
		}
	}

	FILE *f_nds_file = fopen(filenameForInfo.c_str(), "rb");

	bool showSDKVersion = false;
	u32 SDKVersion = 0;
	if(isHomebrew[cursorPosition[secondaryDevice]] == 0) {
		SDKVersion = getSDKVersion(f_nds_file);
		showSDKVersion = true;
		if (secondaryDevice) {
			perGameSettings_cursorPosition = 2;
		}
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
	for (int i = 0; i < 30; i++) swiIntrWait(0, 1);

	while (1) {
		clearText();
		titleUpdate(isDirectory[cursorPosition[secondaryDevice]], filename.c_str(), cursorPosition[secondaryDevice]);
		printSmall(false, 16, 64, filename.c_str());
		if (showSDKVersion) printSmall(false, 16, 80, SDKnumbertext);
		printSmall(false, 176, 80, gameTIDText);
		printSmall(false, 16, 166, fileCounter);
		if (isHomebrew[cursorPosition[secondaryDevice]] == 1) {		// Per-game settings for homebrew (no DSi-Extended header)
			printSmall(false, 16, 96+(perGameSettings_cursorPosition*16), ">");
			printSmall(false, 24, 96, "Direct boot:");
			if (perGameSettings_directBoot) {
				printSmall(false, 208, 96, "Yes");
			} else {
				printSmall(false, 208, 96, "No");
			}
			if(isDSiMode()) {
				printSmall(false, 24, 112, "Run in:");
				if (perGameSettings_boostVram == -1) {
					printSmall(false, 188, 112, "Default");
				} else if (perGameSettings_dsiMode == 1) {
					printSmall(false, 184, 112, "DSi mode");
				} else {
					printSmall(false, 184, 112, "DS mode");
				}
				printSmall(false, 24, 128, "ARM9 CPU Speed:");
				printSmall(false, 24, 144, "VRAM boost:");
				if (perGameSettings_dsiMode == 1) {
					printSmall(false, 146, 128, "133mhz (TWL)");
					printSmall(false, 188, 144, "On");
				} else {
					if (perGameSettings_boostCpu == -1) {
						printSmall(false, 188, 128, "Default");
					} else if (perGameSettings_boostCpu == 1) {
						printSmall(false, 146, 128, "133mhz (TWL)");
					} else {
						printSmall(false, 156, 128, "67mhz (NTR)");
					}
					if (perGameSettings_boostVram == -1) {
						printSmall(false, 188, 144, "Default");
					} else if (perGameSettings_boostVram == 1) {
						printSmall(false, 188, 144, "On");
					} else {
						printSmall(false, 188, 144, "Off");
					}
				}
			}
			printSmall(false, 200, 166, "B: Back");
		} else if (isLauncharg || isDSiWare[cursorPosition[secondaryDevice]] || isHomebrew[cursorPosition[secondaryDevice]] == 2 || !isDSiMode()) {
			printSmall(false, 208, 166, "A: OK");
		} else {	// Per-game settings for retail/commercial games
			if (perGameSettings_cursorPosition >= 0 && perGameSettings_cursorPosition < 4) {
				printSmall(false, 16, 96+(perGameSettings_cursorPosition*16), ">");
				if (!secondaryDevice) {
					printSmall(false, 24, 96, "Language:");
					printSmall(false, 24, 112, "Run in:");
					if (perGameSettings_dsiMode == -1) {
						printSmall(false, 188, 112, "Default");
					} else if (perGameSettings_dsiMode == 1) {
						printSmall(false, 184, 112, "DSi mode");
					} else {
						printSmall(false, 184, 112, "DS mode");
					}
				}
				printSmall(false, 24, 128, "ARM9 CPU Speed:");
				printSmall(false, 24, 144, "VRAM boost:");
				if (!secondaryDevice) {
					if (perGameSettings_language == -2) {
						printSmall(false, 188, 96, "Default");
					} else if (perGameSettings_language == -1) {
						printSmall(false, 188, 96, "System");
					} else if (perGameSettings_language == 0) {
						printSmall(false, 188, 96, "Japanese");
					} else if (perGameSettings_language == 1) {
						printSmall(false, 188, 96, "English");
					} else if (perGameSettings_language == 2) {
						printSmall(false, 188, 96, "French");
					} else if (perGameSettings_language == 3) {
						printSmall(false, 188, 96, "German");
					} else if (perGameSettings_language == 4) {
						printSmall(false, 188, 96, "Italian");
					} else if (perGameSettings_language == 5) {
						printSmall(false, 188, 96, "Spanish");
					}
				}
				if (perGameSettings_dsiMode == 1) {
					printSmall(false, 146, 128, "133mhz (TWL)");
					printSmall(false, 188, 144, "On");
				} else {
					if (perGameSettings_boostCpu == -1) {
						printSmall(false, 188, 128, "Default");
					} else if (perGameSettings_boostCpu == 1) {
						printSmall(false, 146, 128, "133mhz (TWL)");
					} else {
						printSmall(false, 156, 128, "67mhz (NTR)");
					}
					if (perGameSettings_boostVram == -1) {
						printSmall(false, 188, 144, "Default");
					} else if (perGameSettings_boostVram == 1) {
						printSmall(false, 188, 144, "On");
					} else {
						printSmall(false, 188, 144, "Off");
					}
				}
			} else {
				printSmall(false, 16, 96, ">");
				printSmall(false, 24, 96, "Sound fix:");
				if (perGameSettings_soundFix == -1) {
					printSmall(false, 188, 96, "Default");
				} else if (perGameSettings_soundFix == 1) {
					printSmall(false, 188, 96, "On");
				} else {
					printSmall(false, 188, 96, "Off");
				}
			}
			printSmall(false, 200, 166, "B: Back");
		}
		do {
			scanKeys();
			pressed = keysDownRepeat();
			swiIntrWait(0, 1);
		} while (!pressed);

		if (isHomebrew[cursorPosition[secondaryDevice]] == 1) {
			if (pressed & KEY_A) {
				switch (perGameSettings_cursorPosition) {
					case 0:
					default:
						perGameSettings_directBoot = !perGameSettings_directBoot;
						break;
					case 1:
						perGameSettings_dsiMode++;
						if (perGameSettings_dsiMode > 1) perGameSettings_dsiMode = -1;
						break;
					case 2:
						if (!perGameSettings_dsiMode) {
							perGameSettings_boostCpu++;
							if (perGameSettings_boostCpu > 1) perGameSettings_boostCpu = -1;
						}
						break;
					case 3:
						if (!perGameSettings_dsiMode) {
							perGameSettings_boostVram++;
							if (perGameSettings_boostVram > 1) perGameSettings_boostVram = -1;
						}
						break;
				}
				if(isDSiMode()) {
					perGameSettingsChanged = true;
				} else {
					perGameSettingsChanged = !perGameSettingsChanged;
				}
			}
			if(isDSiMode()) {
				if (pressed & KEY_UP) {
					perGameSettings_cursorPosition--;
					if (perGameSettings_cursorPosition < 0) perGameSettings_cursorPosition = 3;
				}
				if (pressed & KEY_DOWN) {
					perGameSettings_cursorPosition++;
					if (perGameSettings_cursorPosition > 3) perGameSettings_cursorPosition = 0;
				}
			}

			if (pressed & KEY_B) {
				if (perGameSettingsChanged) {
					savePerGameSettings(filename);
					perGameSettingsChanged = false;
				}
				break;
			}
		} else if (isLauncharg || isDSiWare[cursorPosition[secondaryDevice]] || isHomebrew[cursorPosition[secondaryDevice]] == 2 || !isDSiMode()) {
			if ((pressed & KEY_A) || (pressed & KEY_B)) {
				break;
			}
		} else {
			if (pressed & KEY_UP) {
				perGameSettings_cursorPosition--;
				if (secondaryDevice) {
					if (perGameSettings_cursorPosition < 2) perGameSettings_cursorPosition = 3;
				} else {
					if (perGameSettings_cursorPosition < 0) perGameSettings_cursorPosition = 4;
				}
			}
			if (pressed & KEY_DOWN) {
				perGameSettings_cursorPosition++;
				if (secondaryDevice) {
					if (perGameSettings_cursorPosition > 3) perGameSettings_cursorPosition = 2;
				} else {
					if (perGameSettings_cursorPosition > 4) perGameSettings_cursorPosition = 0;
				}
			}

			if (pressed & KEY_A) {
				switch (perGameSettings_cursorPosition) {
					case 0:
					default:
						perGameSettings_language++;
						if (perGameSettings_language > 5) perGameSettings_language = -2;
						break;
					case 1:
						perGameSettings_dsiMode++;
						if (perGameSettings_dsiMode > 1) perGameSettings_dsiMode = -1;
						break;
					case 2:
						if (perGameSettings_dsiMode < 1) {
							perGameSettings_boostCpu++;
							if (perGameSettings_boostCpu > 1) perGameSettings_boostCpu = -1;
						}
						break;
					case 3:
						if (perGameSettings_dsiMode < 1) {
							perGameSettings_boostVram++;
							if (perGameSettings_boostVram > 1) perGameSettings_boostVram = -1;
						}
						break;
					case 4:
						perGameSettings_soundFix++;
						if (perGameSettings_soundFix > 1) perGameSettings_soundFix = -1;
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
	for (int i = 0; i < 15; i++) swiIntrWait(0, 1);
	dbox_showIcon = false;
}
