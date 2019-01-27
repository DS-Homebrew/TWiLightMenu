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
#include "cheat.h"

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

extern bool useBootstrap;

const char* SDKnumbertext;

extern bool showdialogbox;
extern int dialogboxHeight;

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
	perGameSettings_saveNo = pergameini.GetInt("GAMESETTINGS", "SAVE_NUMBER", 0);
	perGameSettings_ramDiskNo = pergameini.GetInt("GAMESETTINGS", "RAM_DISK", -1);
	perGameSettings_boostCpu = pergameini.GetInt("GAMESETTINGS", "BOOST_CPU", -1);
	perGameSettings_boostVram = pergameini.GetInt("GAMESETTINGS", "BOOST_VRAM", -1);
    perGameSettings_bootstrapFile = pergameini.GetInt("GAMESETTINGS", "BOOTSTRAP_FILE", -1);
}

void savePerGameSettings (std::string filename) {
	snprintf(pergamefilepath, sizeof(pergamefilepath), "%s/_nds/TWiLightMenu/gamesettings/%s.ini", (secondaryDevice ? "fat:" : "sd:"), filename.c_str());
	CIniFile pergameini( pergamefilepath );
	if (isHomebrew == 1) {
		pergameini.SetInt("GAMESETTINGS", "DIRECT_BOOT", perGameSettings_directBoot);
		if (isDSiMode()) {
			pergameini.SetInt("GAMESETTINGS", "DSI_MODE", perGameSettings_dsiMode);
		}
		if (!secondaryDevice) pergameini.SetInt("GAMESETTINGS", "RAM_DISK", perGameSettings_ramDiskNo);
		if (REG_SCFG_EXT != 0) {
			pergameini.SetInt("GAMESETTINGS", "BOOST_CPU", perGameSettings_boostCpu);
			pergameini.SetInt("GAMESETTINGS", "BOOST_VRAM", perGameSettings_boostVram);
		}
	} else {
		if (useBootstrap) pergameini.SetInt("GAMESETTINGS", "LANGUAGE", perGameSettings_language);
		if (isDSiMode()) {
			pergameini.SetInt("GAMESETTINGS", "DSI_MODE", perGameSettings_dsiMode);
		}
		if (useBootstrap) pergameini.SetInt("GAMESETTINGS", "SAVE_NUMBER", perGameSettings_saveNo);
		if (REG_SCFG_EXT != 0) {
			pergameini.SetInt("GAMESETTINGS", "BOOST_CPU", perGameSettings_boostCpu);
			pergameini.SetInt("GAMESETTINGS", "BOOST_VRAM", perGameSettings_boostVram);
		}
		if (useBootstrap) pergameini.SetInt("GAMESETTINGS", "BOOTSTRAP_FILE", perGameSettings_bootstrapFile);
	}
	pergameini.SaveIniFile( pergamefilepath );
}

bool checkIfShowAPMsg (std::string filename) {
	snprintf(pergamefilepath, sizeof(pergamefilepath), "%s/_nds/TWiLightMenu/gamesettings/%s.ini", (secondaryDevice ? "fat:" : "sd:"), filename.c_str());
	CIniFile pergameini( pergamefilepath );
	if (pergameini.GetInt("GAMESETTINGS", "NO_SHOW_AP_MSG", 0) == 0) {
		return true;	// Show AP message
	}
	return false;	// Don't show AP message
}

void dontShowAPMsgAgain (std::string filename) {
	snprintf(pergamefilepath, sizeof(pergamefilepath), "%s/_nds/TWiLightMenu/gamesettings/%s.ini", (secondaryDevice ? "fat:" : "sd:"), filename.c_str());
	CIniFile pergameini( pergamefilepath );
	pergameini.SetInt("GAMESETTINGS", "NO_SHOW_AP_MSG", 1);
	pergameini.SaveIniFile( pergamefilepath );
}

void perGameSettings (std::string filename) {
	int pressed = 0;

	clearText();
	
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

	bool showSDKVersion = false;
	u32 SDKVersion = 0;
	if(isHomebrew == 0) {
		SDKVersion = getSDKVersion(f_nds_file);
		showSDKVersion = true;
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
	if (isHomebrew == 1) {
		if (REG_SCFG_EXT != 0) {
			dialogboxHeight = 4+useBootstrap;
		} else {
			dialogboxHeight = 1;
		}
	} else if (isDSiWare || isHomebrew == 2) {
		dialogboxHeight = 0;
	} else {
		dialogboxHeight = 4+useBootstrap;
	}
	showdialogbox = true;

	while (1) {
		clearText();
		titleUpdate(isDirectory, filename.c_str());
		if (isHomebrew == 1) {
			printLargeCentered(false, 84, "Game settings");
			printSmall(false, 172, 104, gameTIDText);
			if (perGameSettings_cursorSide) {
				printSmall(false, 156, 112, ">");
			} else {
				printSmall(false, 24, 112+(perGameSettings_cursorPosition*8), ">");
			}
			if (perGameSettings_directBoot) {
				printSmall(false, 32, 112, "Direct boot: Yes");
			} else {
				printSmall(false, 32, 112, "Direct boot: No");
			}
			if (!secondaryDevice) {
				if (perGameSettings_ramDiskNo == -1) {
					printSmall(false, 162, 112, "RAM disk: No");
				} else {
					snprintf (saveNoDisplay, sizeof(saveNoDisplay), "RAM disk: %i", perGameSettings_ramDiskNo);
					printSmall(false, 164, 112, saveNoDisplay);
				}
			}
			if(isDSiMode()) {
				printSmall(false, 32, 120, "Run in:");
				if (perGameSettings_dsiMode == -1) {
					printSmall(false, 180, 120, "Default");
				} else if (perGameSettings_dsiMode == 2) {
					printSmall(false, 120, 120, "DSi mode (Forced)");
				} else if (perGameSettings_dsiMode == 1) {
					printSmall(false, 180, 120, "DSi mode");
				} else {
					printSmall(false, 180, 120, "DS mode");
				}
			}
			if (REG_SCFG_EXT != 0) {
				printSmall(false, 32, 128, "ARM9 CPU Speed:");
				printSmall(false, 32, 136, "VRAM boost:");
				if (perGameSettings_dsiMode > 0 && isDSiMode()) {
					printSmall(false, 153, 128, "133mhz (TWL)");
					printSmall(false, 180, 136, "On");
				} else {
					if (perGameSettings_boostCpu == -1) {
						printSmall(false, 180, 128, "Default");
					} else if (perGameSettings_boostCpu == 1) {
						printSmall(false, 153, 128, "133mhz (TWL)");
					} else {
						printSmall(false, 156, 128, "67mhz (NTR)");
					}
					if (perGameSettings_boostVram == -1) {
						printSmall(false, 180, 136, "Default");
					} else if (perGameSettings_boostVram == 1) {
						printSmall(false, 180, 136, "On");
					} else {
						printSmall(false, 180, 136, "Off");
					}
				}
				if (useBootstrap) {
					printSmall(false, 32, 144, "Bootstrap:");
					if (perGameSettings_bootstrapFile == -1) {
						printSmall(false, 180, 144, "Default");
					} else if (perGameSettings_bootstrapFile == 1) {
						printSmall(false, 180, 144, "Nightly");
					} else {
						printSmall(false, 180, 144, "Release");
					}
				}
				printSmallCentered(false, 150+(useBootstrap*8), "B: Back");
			} else {
				printSmallCentered(false, 126, "B: Back");
			}
		} else if (isDSiWare || isHomebrew == 2 || (!useBootstrap && REG_SCFG_EXT == 0)) {
			printLargeCentered(false, 84, "Info");
			if (showSDKVersion) printSmall(false, 24, 104, SDKnumbertext);
			printSmall(false, 172, 104, gameTIDText);
			printSmallCentered(false, 118, "A: OK");
		} else {
			printLargeCentered(false, 84, "Game settings");
			if (showSDKVersion) printSmall(false, 24, 98, SDKnumbertext);
			printSmall(false, 172, 98, gameTIDText);
			if (perGameSettings_cursorSide) {
				printSmall(false, 156, 112, ">");
			} else {
				printSmall(false, 24, 112+(perGameSettings_cursorPosition*8), ">");
			}
			if (useBootstrap) {
				printSmall(false, 32, 112, "Language:");
				if (perGameSettings_language == -2) {
					printSmall(false, 104, 112, "Def");
				} else if (perGameSettings_language == -1) {
					printSmall(false, 104, 112, "Sys");
				} else if (perGameSettings_language == 0) {
					printSmall(false, 104, 112, "Jap");
				} else if (perGameSettings_language == 1) {
					printSmall(false, 104, 112, "Eng");
				} else if (perGameSettings_language == 2) {
					printSmall(false, 104, 112, "Fre");
				} else if (perGameSettings_language == 3) {
					printSmall(false, 104, 112, "Ger");
				} else if (perGameSettings_language == 4) {
					printSmall(false, 104, 112, "Ita");
				} else if (perGameSettings_language == 5) {
					printSmall(false, 104, 112, "Spa");
				}
				snprintf (saveNoDisplay, sizeof(saveNoDisplay), "Save no: %i", perGameSettings_saveNo);
				printSmall(false, 164, 112, saveNoDisplay);
			}
			if (isDSiMode()) {
				printSmall(false, 32, 120, "Run in:");
				if (perGameSettings_dsiMode == -1) {
					printSmall(false, 180, 120, "Default");
				} else if (perGameSettings_dsiMode == 2) {
					printSmall(false, 120, 120, "DSi mode (Forced)");
				} else if (perGameSettings_dsiMode == 1) {
					printSmall(false, 180, 120, "DSi mode");
				} else {
					printSmall(false, 180, 120, "DS mode");
				}
			}
			if (REG_SCFG_EXT != 0) {
				printSmall(false, 32, 128, "ARM9 CPU Speed:");
				printSmall(false, 32, 136, "VRAM boost:");
				if (perGameSettings_dsiMode > 0 && isDSiMode()) {
					printSmall(false, 153, 128, "133mhz (TWL)");
					printSmall(false, 180, 136, "On");
				} else {
					if (perGameSettings_boostCpu == -1) {
						printSmall(false, 180, 128, "Default");
					} else if (perGameSettings_boostCpu == 1) {
						printSmall(false, 153, 128, "133mhz (TWL)");
					} else {
						printSmall(false, 156, 128, "67mhz (NTR)");
					}
					if (perGameSettings_boostVram == -1) {
						printSmall(false, 180, 136, "Default");
					} else if (perGameSettings_boostVram == 1) {
						printSmall(false, 180, 136, "On");
					} else {
						printSmall(false, 180, 136, "Off");
					}
				}
			}
			if (useBootstrap) {
				printSmall(false, 32, 144, "Bootstrap:");
				if (perGameSettings_bootstrapFile == -1) {
					printSmall(false, 180, 144, "Default");
				} else if (perGameSettings_bootstrapFile == 1) {
					printSmall(false, 180, 144, "Nightly");
				} else {
					printSmall(false, 180, 144, "Release");
				}
			}
			printSmallCentered(false, 150+(useBootstrap*8), "X: Cheats B: Back");
		}
		do {
			scanKeys();
			pressed = keysDownRepeat();
			swiWaitForVBlank();
		} while (!pressed);

		if (isHomebrew == 1) {
			if (useBootstrap) {
				if (pressed & KEY_UP) {
					if (perGameSettings_cursorPosition == 0) {
						perGameSettings_cursorSide = false;
					}
					perGameSettings_cursorPosition--;
					if (perGameSettings_cursorPosition < 0) perGameSettings_cursorPosition = 4;
					if (!isDSiMode() && REG_SCFG_EXT != 0 && perGameSettings_cursorPosition == 1) perGameSettings_cursorPosition = 0;
					if (!isDSiMode() && REG_SCFG_EXT == 0 && perGameSettings_cursorPosition == 3) perGameSettings_cursorPosition = 0;
				}
				if (pressed & KEY_DOWN) {
					if (perGameSettings_cursorPosition == 0) {
						perGameSettings_cursorSide = false;
					}
					perGameSettings_cursorPosition++;
					if (perGameSettings_cursorPosition > 4) perGameSettings_cursorPosition = 0;
					if (!isDSiMode() && REG_SCFG_EXT != 0 && perGameSettings_cursorPosition == 1) perGameSettings_cursorPosition = 2;
					if (!isDSiMode() && REG_SCFG_EXT == 0 && perGameSettings_cursorPosition == 1) perGameSettings_cursorPosition = 4;
				}
				if ((!secondaryDevice && (pressed & KEY_LEFT))
				|| (!secondaryDevice && (pressed & KEY_RIGHT))) {
					if (perGameSettings_cursorPosition == 0) {
						perGameSettings_cursorSide = !perGameSettings_cursorSide;
					}
				}
			} else {
				if (pressed & KEY_UP) {
					perGameSettings_cursorPosition--;
					if (perGameSettings_cursorPosition < 2) perGameSettings_cursorPosition = 3;
				}
				if (pressed & KEY_DOWN) {
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
				perGameSettingsChanged = !perGameSettingsChanged;
			}

			if (pressed & KEY_B) {
				if (perGameSettingsChanged) {
					savePerGameSettings(filename);
					perGameSettingsChanged = false;
				}
				break;
			}
		} else if (isDSiWare || isHomebrew == 2 || (!useBootstrap && REG_SCFG_EXT == 0)) {
			if ((pressed & KEY_A) || (pressed & KEY_B)) {
				break;
			}
		} else {
			if (useBootstrap) {
				if (pressed & KEY_UP) {
					if (perGameSettings_cursorPosition == 0) {
						perGameSettings_cursorSide = false;
					}
					perGameSettings_cursorPosition--;
					if (perGameSettings_cursorPosition < 0) perGameSettings_cursorPosition = 4;
					if (!isDSiMode() && REG_SCFG_EXT != 0 && perGameSettings_cursorPosition == 1) perGameSettings_cursorPosition = 0;
					if (!isDSiMode() && REG_SCFG_EXT == 0 && perGameSettings_cursorPosition == 3) perGameSettings_cursorPosition = 0;
				}
				if (pressed & KEY_DOWN) {
					if (perGameSettings_cursorPosition == 0) {
						perGameSettings_cursorSide = false;
					}
					perGameSettings_cursorPosition++;
					if (perGameSettings_cursorPosition > 4) perGameSettings_cursorPosition = 0;
					if (!isDSiMode() && REG_SCFG_EXT != 0 && perGameSettings_cursorPosition == 1) perGameSettings_cursorPosition = 2;
					if (!isDSiMode() && REG_SCFG_EXT == 0 && perGameSettings_cursorPosition == 1) perGameSettings_cursorPosition = 4;
				}
				if ((pressed & KEY_LEFT) || (pressed & KEY_RIGHT)) {
					if (perGameSettings_cursorPosition == 0) {
						perGameSettings_cursorSide = !perGameSettings_cursorSide;
					}
				}
			} else {
				if (pressed & KEY_UP) {
					perGameSettings_cursorPosition--;
					if (perGameSettings_cursorPosition < 2) perGameSettings_cursorPosition = 3;
				}
				if (pressed & KEY_DOWN) {
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
				perGameSettingsChanged = true;
			}

			if (pressed & KEY_B) {
				if (perGameSettingsChanged) {
					savePerGameSettings(filename);
					perGameSettingsChanged = false;
				}
				break;
			}
			if ((pressed & KEY_X)) {
				CheatCodelist codelist;
				codelist.selectCheats(filename);
			}
		}
	}
	clearText();
	showdialogbox = false;
	dialogboxHeight = 0;
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
