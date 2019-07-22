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
#include "buttontext.h"
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
#include "graphics/ThemeTextures.h"
#include "sound.h"
#include "SwitchState.h"
#include "cheat.h"
#include "errorScreen.h"

#include "gbaswitch.h"
#include "nds_loader_arm9.h"

#include "common/inifile.h"
#include "common/flashcard.h"
#include "common/dsimenusettings.h"

#define SCREEN_COLS 32
#define ENTRIES_PER_SCREEN 15
#define ENTRIES_START_ROW 3
#define ENTRY_PAGE_LENGTH 10


extern int currentBg;
extern bool displayGameIcons;

const char* SDKnumbertext;

extern bool showdialogbox;
extern bool dbox_showIcon;

bool perGameSettingsChanged = false;

int perGameSettings_cursorPosition = 0;
bool perGameSettings_cursorSide = false;
bool perGameSettings_directBoot = false;	// Homebrew only
int perGameSettings_dsiMode = -1;
int perGameSettings_language = -2;
int perGameSettings_saveNo = 0;
int perGameSettings_ramDiskNo = -1;
int perGameSettings_boostCpu = -1;
int perGameSettings_boostVram = -1;
int perGameSettings_bootstrapFile = -1;

extern int file_count;

char pergamefilepath[256];

extern std::string ReplaceAll(std::string str, const std::string& from, const std::string& to);

extern mm_sound_effect snd_launch;
extern mm_sound_effect snd_select;
extern mm_sound_effect snd_stop;
extern mm_sound_effect snd_wrong;
extern mm_sound_effect snd_back;
extern mm_sound_effect snd_switch;

extern char usernameRendered[11];
extern bool usernameRenderedDone;

char fileCounter[8];
char gameTIDText[16];

void loadPerGameSettings (std::string filename) {
	snprintf(pergamefilepath, sizeof(pergamefilepath), "%s/_nds/TWiLightMenu/gamesettings/%s.ini", (ms().secondaryDevice ? "fat:" : "sd:"), filename.c_str());
	CIniFile pergameini( pergamefilepath );
	perGameSettings_directBoot = pergameini.GetInt("GAMESETTINGS", "DIRECT_BOOT", ms().secondaryDevice);	// Homebrew only
	if ((isDSiMode() && ms().useBootstrap) || !ms().secondaryDevice) {
		perGameSettings_dsiMode = pergameini.GetInt("GAMESETTINGS", "DSI_MODE", -1);
	} else {
		perGameSettings_dsiMode = -1;
	}
	perGameSettings_language = pergameini.GetInt("GAMESETTINGS", "LANGUAGE", -2);
	perGameSettings_saveNo = pergameini.GetInt("GAMESETTINGS", "SAVE_NUMBER", 0);
	perGameSettings_ramDiskNo = pergameini.GetInt("GAMESETTINGS", "RAM_DISK", -1);
	perGameSettings_boostCpu = pergameini.GetInt("GAMESETTINGS", "BOOST_CPU", -1);
	perGameSettings_boostVram = pergameini.GetInt("GAMESETTINGS", "BOOST_VRAM", -1);
    perGameSettings_bootstrapFile = pergameini.GetInt("GAMESETTINGS", "BOOTSTRAP_FILE", -1);
}

void savePerGameSettings (std::string filename) {
	snprintf(pergamefilepath, sizeof(pergamefilepath), "%s/_nds/TWiLightMenu/gamesettings/%s.ini", (ms().secondaryDevice ? "fat:" : "sd:"), filename.c_str());
	CIniFile pergameini( pergamefilepath );
	if (isHomebrew[CURPOS] == 1) {
		pergameini.SetInt("GAMESETTINGS", "DIRECT_BOOT", perGameSettings_directBoot);
		if (isDSiMode()) {
			pergameini.SetInt("GAMESETTINGS", "DSI_MODE", perGameSettings_dsiMode);
		}
		if (!ms().secondaryDevice) pergameini.SetInt("GAMESETTINGS", "RAM_DISK", perGameSettings_ramDiskNo);
		if (REG_SCFG_EXT != 0) {
			pergameini.SetInt("GAMESETTINGS", "BOOST_CPU", perGameSettings_boostCpu);
			pergameini.SetInt("GAMESETTINGS", "BOOST_VRAM", perGameSettings_boostVram);
		}
	} else {
		if (ms().useBootstrap || !ms().secondaryDevice) pergameini.SetInt("GAMESETTINGS", "LANGUAGE", perGameSettings_language);
		if ((isDSiMode() && ms().useBootstrap) || !ms().secondaryDevice) {
			pergameini.SetInt("GAMESETTINGS", "DSI_MODE", perGameSettings_dsiMode);
		}
		if (ms().useBootstrap || !ms().secondaryDevice) pergameini.SetInt("GAMESETTINGS", "SAVE_NUMBER", perGameSettings_saveNo);
		if (REG_SCFG_EXT != 0) {
			pergameini.SetInt("GAMESETTINGS", "BOOST_CPU", perGameSettings_boostCpu);
			pergameini.SetInt("GAMESETTINGS", "BOOST_VRAM", perGameSettings_boostVram);
		}
		if (ms().useBootstrap || !ms().secondaryDevice) pergameini.SetInt("GAMESETTINGS", "BOOTSTRAP_FILE", perGameSettings_bootstrapFile);
	}
	pergameini.SaveIniFile( pergamefilepath );
}

bool checkIfShowAPMsg (std::string filename) {
	snprintf(pergamefilepath, sizeof(pergamefilepath), "%s/_nds/TWiLightMenu/gamesettings/%s.ini", (ms().secondaryDevice ? "fat:" : "sd:"), filename.c_str());
	CIniFile pergameini( pergamefilepath );
	if (pergameini.GetInt("GAMESETTINGS", "NO_SHOW_AP_MSG", 0) == 0) {
		return true;	// Show AP message
	}
	return false;	// Don't show AP message
}

void dontShowAPMsgAgain (std::string filename) {
	snprintf(pergamefilepath, sizeof(pergamefilepath), "%s/_nds/TWiLightMenu/gamesettings/%s.ini", (ms().secondaryDevice ? "fat:" : "sd:"), filename.c_str());
	CIniFile pergameini( pergamefilepath );
	pergameini.SetInt("GAMESETTINGS", "NO_SHOW_AP_MSG", 1);
	pergameini.SaveIniFile( pergamefilepath );
}

void perGameSettings (std::string filename) {
	int pressed = 0;

	clearText();
	dbox_showIcon = true;
	if (ms().theme == 4) {
		snd().playStartup();
		currentBg = 1;
		displayGameIcons = false;
	} else {
		showdialogbox = true;
	}
	
	snprintf (fileCounter, sizeof(fileCounter), "%i/%i", (CURPOS+1)+PAGENUM*40, file_count);
	
	perGameSettings_cursorPosition = 0;
	perGameSettings_cursorSide = false;
	loadPerGameSettings(filename);

	std::string filenameForInfo = filename;
	if((filenameForInfo.substr(filenameForInfo.find_last_of(".") + 1) == "argv")
	|| (filenameForInfo.substr(filenameForInfo.find_last_of(".") + 1) == "ARGV"))
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

	char game_TID[5];
	grabTID(f_nds_file, game_TID);
	game_TID[4] = 0;
	game_TID[3] = 0;
	
	bool showSDKVersion = false;
	u32 SDKVersion = 0;
	if (strcmp(game_TID, "HND") == 0 || strcmp(game_TID, "HNE") == 0) {
		SDKVersion = getSDKVersion(f_nds_file);
		showSDKVersion = true;
	} else if(isHomebrew[CURPOS] == 0) {
		SDKVersion = getSDKVersion(f_nds_file);
		showSDKVersion = true;
		if (!ms().useBootstrap && ms().secondaryDevice) {
			perGameSettings_cursorPosition = 2;
		}
	}
	
	bool showPerGameSettings =
		(!isDSiWare[CURPOS]
		&& isHomebrew[CURPOS] != 2
		&& strcmp(game_TID, "HND") != 0
		&& strcmp(game_TID, "HNE") != 0
		&& REG_SCFG_EXT != 0);
	if (!ms().useBootstrap && REG_SCFG_EXT == 0) {
		showPerGameSettings = false;
	}

	char gameTIDDisplay[5];
	grabTID(f_nds_file, gameTIDDisplay);
	gameTIDDisplay[4] = 0;
	fclose(f_nds_file);
	
	snprintf (gameTIDText, sizeof(gameTIDText), "TID: %s", gameTIDDisplay);

	char saveNoDisplay[16];

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
	for (int i = 0; i < 30; i++) { snd().updateStream(); swiWaitForVBlank(); }

	while (1) {
		clearText();
		titleUpdate(isDirectory[CURPOS], filename.c_str(), CURPOS);
		// About 38 characters fit in the box.
		std::string displayFilename = filename;
		if (strlen(displayFilename.c_str()) > 35) {
			// Truncate to 35, 35 + 3 = 38 (because we append "...").
			displayFilename.resize(32, ' ');
			size_t first = displayFilename.find_first_not_of(' ');
			size_t last = displayFilename.find_last_not_of(' ');
			displayFilename = displayFilename.substr(first, (last - first + 1));
			displayFilename.append("...");
		}


		printSmall(false, 16, 66, displayFilename.c_str());
		if (showSDKVersion) printSmall(false, 16, 80, SDKnumbertext);
		printSmall(false, 176, 80, gameTIDText);
		printSmall(false, 16, 160, fileCounter);
		if (isHomebrew[CURPOS] == 1) {		// Per-game settings for homebrew (no DSi-Extended header)
			if (perGameSettings_cursorPosition < 4) {
				if (perGameSettings_cursorSide) {
					printSmall(false, 154, 98, ">");
				} else {
					printSmall(false, 16, 98+(perGameSettings_cursorPosition*14), ">");
				}
				if (perGameSettings_directBoot) {
					printSmall(false, 24, 98, "Direct boot: Yes");
				} else {
					printSmall(false, 24, 98, "Direct boot: No");
				}
				if (!ms().secondaryDevice) {
					if (perGameSettings_ramDiskNo == -1) {
						printSmall(false, 162, 98, "RAM disk: No");
					} else {
						snprintf (saveNoDisplay, sizeof(saveNoDisplay), "RAM disk: %i", perGameSettings_ramDiskNo);
						printSmall(false, 162, 98, saveNoDisplay);
					}
				}
				if((isDSiMode() && ms().useBootstrap) || !ms().secondaryDevice) {
					printSmall(false, 24, 112, "Run in:");
					if (perGameSettings_dsiMode == -1) {
						printSmall(false, 188, 112, "Default");
					} else if (perGameSettings_dsiMode == 2) {
						printSmall(false, 128, 112, "DSi mode (Forced)");
					} else if (perGameSettings_dsiMode == 1) {
						printSmall(false, 184, 112, "DSi mode");
					} else {
						printSmall(false, 184, 112, "DS mode");
					}
				}
				if (REG_SCFG_EXT != 0) {
					printSmall(false, 24, 126, "ARM9 CPU Speed:");
					printSmall(false, 24, 140, "VRAM boost:");
					if (perGameSettings_dsiMode > 0) {
						printSmall(false, 146, 126, "133mhz (TWL)");
						printSmall(false, 188, 140, "On");
					} else {
						if (perGameSettings_boostCpu == -1) {
							printSmall(false, 188, 126, "Default");
						} else if (perGameSettings_boostCpu == 1) {
							printSmall(false, 146, 126, "133mhz (TWL)");
						} else {
							printSmall(false, 156, 126, "67mhz (NTR)");
						}
						if (perGameSettings_boostVram == -1) {
							printSmall(false, 188, 140, "Default");
						} else if (perGameSettings_boostVram == 1) {
							printSmall(false, 188, 140, "On");
						} else {
							printSmall(false, 188, 140, "Off");
						}
					}
				}
			} else {
				printSmall(false, 16, 98, ">");
				printSmall(false, 24, 98, "Bootstrap:");
				if (perGameSettings_bootstrapFile == -1) {
					printSmall(false, 188, 98, "Default");
				} else if (perGameSettings_bootstrapFile == 1) {
					printSmall(false, 188, 98, "Nightly");
				} else {
					printSmall(false, 188, 98, "Release");
				}
			}
			printSmall(false, 200, 160, BUTTON_B" Back");
		} else if (!showPerGameSettings) {
			printSmall(false, 208, 160, BUTTON_A" OK");
		} else {	// Per-game settings for retail/commercial games
			if (perGameSettings_cursorPosition < 4) {
				if (perGameSettings_cursorSide) {
					printSmall(false, 154, 98, ">");
				} else {
					printSmall(false, 16, 98+(perGameSettings_cursorPosition*14), ">");
				}
				if (ms().useBootstrap || !ms().secondaryDevice) {
					printSmall(false, 24, 98, "Language:");
					if (perGameSettings_language == -2) {
						printSmall(false, 88, 98, "Default");
					} else if (perGameSettings_language == -1) {
						printSmall(false, 88, 98, "System");
					} else if (perGameSettings_language == 0) {
						printSmall(false, 88, 98, "Japanese");
					} else if (perGameSettings_language == 1) {
						printSmall(false, 88, 98, "English");
					} else if (perGameSettings_language == 2) {
						printSmall(false, 88, 98, "French");
					} else if (perGameSettings_language == 3) {
						printSmall(false, 88, 98, "German");
					} else if (perGameSettings_language == 4) {
						printSmall(false, 88, 98, "Italian");
					} else if (perGameSettings_language == 5) {
						printSmall(false, 88, 98, "Spanish");
					}
					snprintf (saveNoDisplay, sizeof(saveNoDisplay), "Save no: %i", perGameSettings_saveNo);
					printSmall(false, 162, 98, saveNoDisplay);
				}
				if ((isDSiMode() && ms().useBootstrap) || !ms().secondaryDevice) {
					printSmall(false, 24, 112, "Run in:");
					if (perGameSettings_dsiMode == -1) {
						printSmall(false, 188, 112, "Default");
					} else if (perGameSettings_dsiMode == 2) {
						printSmall(false, 128, 112, "DSi mode (Forced)");
					} else if (perGameSettings_dsiMode == 1) {
						printSmall(false, 184, 112, "DSi mode");
					} else {
						printSmall(false, 184, 112, "DS mode");
					}
				}
				if (REG_SCFG_EXT != 0) {
					printSmall(false, 24, 126, "ARM9 CPU Speed:");
					printSmall(false, 24, 140, "VRAM boost:");
					if (perGameSettings_dsiMode > 0 && isDSiMode()) {
						printSmall(false, 146, 126, "133mhz (TWL)");
						printSmall(false, 188, 140, "On");
					} else {
						if (perGameSettings_boostCpu == -1) {
							printSmall(false, 188, 126, "Default");
						} else if (perGameSettings_boostCpu == 1) {
							printSmall(false, 146, 126, "133mhz (TWL)");
						} else {
							printSmall(false, 156, 126, "67mhz (NTR)");
						}
						if (perGameSettings_boostVram == -1) {
							printSmall(false, 188, 140, "Default");
						} else if (perGameSettings_boostVram == 1) {
							printSmall(false, 188, 140, "On");
						} else {
							printSmall(false, 188, 140, "Off");
						}
					}
				}
			} else {
				printSmall(false, 16, 98, ">");
				printSmall(false, 24, 98, "Bootstrap:");
				if (perGameSettings_bootstrapFile == -1) {
					printSmall(false, 188, 98, "Default");
				} else if (perGameSettings_bootstrapFile == 1) {
					printSmall(false, 188, 98, "Nightly");
				} else {
					printSmall(false, 188, 98, "Release");
				}
			}
			printSmall(false, 128, 160, BUTTON_X " Cheats  " BUTTON_B" Back");
		}
		do {
			scanKeys();
			pressed = keysDown();
			checkSdEject();
			tex().drawVolumeImageCached();
			tex().drawBatteryImageCached();
			drawCurrentTime();
			drawCurrentDate();
			drawClockColon();
			snd().updateStream();
			swiWaitForVBlank();
		} while (!pressed);

		if (isHomebrew[CURPOS] == 1) {
			if (ms().useBootstrap) {
				if (pressed & KEY_UP) {
					if (perGameSettings_cursorPosition == 0) {
						perGameSettings_cursorSide = false;
					}
					snd().playSelect();
					perGameSettings_cursorPosition--;
					if (perGameSettings_cursorPosition < 0) perGameSettings_cursorPosition = 4;
					if (!isDSiMode() && REG_SCFG_EXT != 0 && perGameSettings_cursorPosition == 1) perGameSettings_cursorPosition = 0;
					if (!isDSiMode() && REG_SCFG_EXT == 0 && perGameSettings_cursorPosition == 3) perGameSettings_cursorPosition = 0;
				}
				if ((!ms().secondaryDevice && (pressed & KEY_LEFT))
				|| (!ms().secondaryDevice && (pressed & KEY_RIGHT))) {
					if (perGameSettings_cursorPosition == 0) {
						snd().playSelect();
						perGameSettings_cursorSide = !perGameSettings_cursorSide;
					}
				}
				if (pressed & KEY_DOWN) {
					if (perGameSettings_cursorPosition == 0) {
						perGameSettings_cursorSide = false;
					}
					snd().playSelect();
					perGameSettings_cursorPosition++;
					if (perGameSettings_cursorPosition > 4) perGameSettings_cursorPosition = 0;
					if (!isDSiMode() && REG_SCFG_EXT != 0 && perGameSettings_cursorPosition == 1) perGameSettings_cursorPosition = 2;
					if (!isDSiMode() && REG_SCFG_EXT == 0 && perGameSettings_cursorPosition == 1) perGameSettings_cursorPosition = 4;
				}
			} else {
				if (pressed & KEY_UP) {
					snd().playSelect();
					perGameSettings_cursorPosition--;
					if (perGameSettings_cursorPosition < 2) perGameSettings_cursorPosition = 3;
				}
				if (pressed & KEY_DOWN) {
					snd().playSelect();
					perGameSettings_cursorPosition++;
					if (perGameSettings_cursorPosition > 3) perGameSettings_cursorPosition = 2;
				}
			}

			if (pressed & KEY_A) {
				switch (perGameSettings_cursorPosition) {
					case 0:
					default:
						if (perGameSettings_cursorSide) {
							perGameSettings_ramDiskNo++;
							if (perGameSettings_ramDiskNo > 9) perGameSettings_ramDiskNo = -1;
						} else {
							perGameSettings_directBoot = !perGameSettings_directBoot;
						}
						break;
					case 1:
						perGameSettings_dsiMode++;
						if (perGameSettings_dsiMode > 2) perGameSettings_dsiMode = -1;
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
						perGameSettings_bootstrapFile++;
						if (perGameSettings_bootstrapFile > 1) perGameSettings_bootstrapFile = -1;
						break;
				}
				(ms().theme == 4) ? snd().playLaunch() : snd().playSelect();
				perGameSettingsChanged = true;
			}

			if (pressed & KEY_B) {
				snd().playBack();
				if (perGameSettingsChanged) {
					savePerGameSettings(filename);
					perGameSettingsChanged = false;
				}
				break;
			}
		} else if (!showPerGameSettings) {
			if ((pressed & KEY_A) || (pressed & KEY_B)) {
				snd().playBack();
				break;
			}
		} else {
			if (ms().useBootstrap || !ms().secondaryDevice) {
				if (pressed & KEY_UP) {
					if (perGameSettings_cursorPosition == 0) {
						perGameSettings_cursorSide = false;
					}
					snd().playSelect();
					perGameSettings_cursorPosition--;
					if (perGameSettings_cursorPosition < 0) perGameSettings_cursorPosition = 4;
					if (!isDSiMode() && REG_SCFG_EXT != 0 && perGameSettings_cursorPosition == 1) perGameSettings_cursorPosition = 0;
					if (!isDSiMode() && REG_SCFG_EXT == 0 && perGameSettings_cursorPosition == 3) perGameSettings_cursorPosition = 0;
				}
				if (pressed & KEY_DOWN) {
					if (perGameSettings_cursorPosition == 0) {
						perGameSettings_cursorSide = false;
					}
					snd().playSelect();
					perGameSettings_cursorPosition++;
					if (perGameSettings_cursorPosition > 4) perGameSettings_cursorPosition = 0;
					if (!isDSiMode() && REG_SCFG_EXT != 0 && perGameSettings_cursorPosition == 1) perGameSettings_cursorPosition = 2;
					if (!isDSiMode() && REG_SCFG_EXT == 0 && perGameSettings_cursorPosition == 1) perGameSettings_cursorPosition = 4;
				}
				if ((pressed & KEY_LEFT) || (pressed & KEY_RIGHT)) {
					if (perGameSettings_cursorPosition == 0) {
						snd().playSelect();
						perGameSettings_cursorSide = !perGameSettings_cursorSide;
					}
				}
			} else {
				if (pressed & KEY_UP) {
					snd().playSelect();
					perGameSettings_cursorPosition--;
					if (perGameSettings_cursorPosition < 2) perGameSettings_cursorPosition = 3;
				}
				if (pressed & KEY_DOWN) {
					snd().playSelect();
					perGameSettings_cursorPosition++;
					if (perGameSettings_cursorPosition > 3) perGameSettings_cursorPosition = 2;
				}
			}

			if (pressed & KEY_A) {
				switch (perGameSettings_cursorPosition) {
					case 0:
					default:
						if (perGameSettings_cursorSide) {
							perGameSettings_saveNo++;
							if (perGameSettings_saveNo > 9) perGameSettings_saveNo = 0;
						} else {
							perGameSettings_language++;
							if (perGameSettings_language > 5) perGameSettings_language = -2;
						}
						break;
					case 1:
						perGameSettings_dsiMode++;
						if (perGameSettings_dsiMode > 2) perGameSettings_dsiMode = -1;
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
						perGameSettings_bootstrapFile++;
						if (perGameSettings_bootstrapFile > 1) perGameSettings_bootstrapFile = -1;
						break;
				}
				(ms().theme == 4) ? snd().playLaunch() : snd().playSelect();
				perGameSettingsChanged = true;
			}

			if (pressed & KEY_B) {
				snd().playBack();
				if (perGameSettingsChanged) {
					savePerGameSettings(filename);
					perGameSettingsChanged = false;
				}
				break;
			}
			if (pressed & KEY_X) {
				(ms().theme == 4) ? snd().playLaunch() : snd().playSelect();
				CheatCodelist codelist;
				codelist.selectCheats(filename);
			}
		}
	}
	clearText();
	showdialogbox = false;
	if (ms().theme == 4) {
		currentBg = 0;
		displayGameIcons = true;
	} else {
		for (int i = 0; i < 15; i++) { snd().updateStream(); swiWaitForVBlank(); }
	}
	dbox_showIcon = false;
}

std::string getSavExtension(void) {
	switch (perGameSettings_saveNo) {
		case 0:
		default:
			return ".sav";
			break;
		case 1:
			return ".sav1";
			break;
		case 2:
			return ".sav2";
			break;
		case 3:
			return ".sav3";
			break;
		case 4:
			return ".sav4";
			break;
		case 5:
			return ".sav5";
			break;
		case 6:
			return ".sav6";
			break;
		case 7:
			return ".sav7";
			break;
		case 8:
			return ".sav8";
			break;
		case 9:
			return ".sav9";
			break;
	}
}

std::string getImgExtension(int number) {
	switch (number) {
		case 0:
		default:
			return ".img";
			break;
		case 1:
			return ".img1";
			break;
		case 2:
			return ".img2";
			break;
		case 3:
			return ".img3";
			break;
		case 4:
			return ".img4";
			break;
		case 5:
			return ".img5";
			break;
		case 6:
			return ".img6";
			break;
		case 7:
			return ".img7";
			break;
		case 8:
			return ".img8";
			break;
		case 9:
			return ".img9";
			break;
	}
}
