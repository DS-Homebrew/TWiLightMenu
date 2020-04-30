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
#include "language.h"
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


extern bool useTwlCfg;

extern const char *bootstrapinipath;

extern int currentBg;
extern bool displayGameIcons;

const char* SDKnumbertext;

extern bool fadeType;
extern bool showdialogbox;
extern bool dbox_showIcon;

bool perGameSettingsChanged = false;

int perGameSettings_cursorPosition = 0;
bool perGameSettings_directBoot = false;	// Homebrew only
int perGameSettings_dsiMode = -1;
int perGameSettings_language = -2;
int perGameSettings_saveNo = 0;
int perGameSettings_ramDiskNo = -1;
int perGameSettings_boostCpu = -1;
int perGameSettings_boostVram = -1;
int perGameSettings_heapShrink = -1;
int perGameSettings_bootstrapFile = -1;
int perGameSettings_wideScreen = -1;

static char LANGUAGE[32];
static char RAM_DISK[32];
static char SAVE_NO[32];
static char RUN_IN[32];
static char ARM9_CPU_SPEED[32];
static char VRAM_BOOST[32];
static char HEAP_SHRINK[32];
static char DIRECT_BOOT[32];
static char SCREEN_ASPECT_RATIO[32];
static char SET_AS_DONOR_ROM[32];

extern int file_count;

char pergamefilepath[256];

extern void RemoveTrailingSlashes(std::string &path);

extern std::string dirContName;

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
char saveNoDisplay[8];

int firstPerGameOpShown = 0;
int perGameOps = -1;
int perGameOp[10] = {-1};

void loadPerGameSettings (std::string filename) {
	snprintf(pergamefilepath, sizeof(pergamefilepath), "%s/_nds/TWiLightMenu/gamesettings/%s.ini", (ms().secondaryDevice ? "fat:" : "sd:"), filename.c_str());
	CIniFile pergameini( pergamefilepath );
	perGameSettings_directBoot = pergameini.GetInt("GAMESETTINGS", "DIRECT_BOOT", (isModernHomebrew[CURPOS] || ms().secondaryDevice));	// Homebrew only
	if ((isDSiMode() && ms().useBootstrap) || !ms().secondaryDevice) {
		perGameSettings_dsiMode = pergameini.GetInt("GAMESETTINGS", "DSI_MODE", (isModernHomebrew[CURPOS] ? true : -1));
	} else {
		perGameSettings_dsiMode = -1;
	}
	perGameSettings_language = pergameini.GetInt("GAMESETTINGS", "LANGUAGE", -2);
	perGameSettings_saveNo = pergameini.GetInt("GAMESETTINGS", "SAVE_NUMBER", 0);
	perGameSettings_ramDiskNo = pergameini.GetInt("GAMESETTINGS", "RAM_DISK", -1);
	perGameSettings_boostCpu = pergameini.GetInt("GAMESETTINGS", "BOOST_CPU", -1);
	perGameSettings_boostVram = pergameini.GetInt("GAMESETTINGS", "BOOST_VRAM", -1);
	perGameSettings_heapShrink = pergameini.GetInt("GAMESETTINGS", "HEAP_SHRINK", -1);
    perGameSettings_bootstrapFile = pergameini.GetInt("GAMESETTINGS", "BOOTSTRAP_FILE", -1);
    perGameSettings_wideScreen = pergameini.GetInt("GAMESETTINGS", "WIDESCREEN", -1);
}

void savePerGameSettings (std::string filename) {
	snprintf(pergamefilepath, sizeof(pergamefilepath), "%s/_nds/TWiLightMenu/gamesettings/%s.ini", (ms().secondaryDevice ? "fat:" : "sd:"), filename.c_str());
	CIniFile pergameini( pergamefilepath );
	if (isHomebrew[CURPOS]) {
		if (!ms().secondaryDevice) pergameini.SetInt("GAMESETTINGS", "LANGUAGE", perGameSettings_language);
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
		if (ms().useBootstrap || !ms().secondaryDevice) {
			pergameini.SetInt("GAMESETTINGS", "HEAP_SHRINK", perGameSettings_heapShrink);
			pergameini.SetInt("GAMESETTINGS", "BOOTSTRAP_FILE", perGameSettings_bootstrapFile);
		}
		if (isDSiMode() && ms().consoleModel >= 2 && sdFound()) {
			pergameini.SetInt("GAMESETTINGS", "WIDESCREEN", perGameSettings_wideScreen);
		}
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

bool checkIfDSiMode (std::string filename) {
	if (!isDSiMode() || (!ms().useBootstrap && ms().secondaryDevice)) {
		return false;
	}

	snprintf(pergamefilepath, sizeof(pergamefilepath), "%s/_nds/TWiLightMenu/gamesettings/%s.ini", (ms().secondaryDevice ? "fat:" : "sd:"), filename.c_str());
	CIniFile pergameini( pergamefilepath );
	perGameSettings_dsiMode = pergameini.GetInt("GAMESETTINGS", "DSI_MODE", (isModernHomebrew[CURPOS] ? true : -1));
	if (perGameSettings_dsiMode == -1) {
		return ms().bstrap_dsiMode;
	} else {
		return perGameSettings_dsiMode;
	}
}

bool showSetDonorRom(u32 arm7size, u32 SDKVersion) {
	if (requiresDonorRom[CURPOS]) return false;

	if ((!isDSiMode() && SDKVersion >= 0x2000000 && SDKVersion < 0x2008000
	 && (arm7size==0x25F70
	  || arm7size==0x25FA4
	  || arm7size==0x2619C
	  || arm7size==0x26A60
	  || arm7size==0x27218
	  || arm7size==0x27224
	  || arm7size==0x2724C))
	 || (SDKVersion >= 0x2008000 && SDKVersion < 0x3000000 && (arm7size==0x26F24 || arm7size==0x26F28))
	 || (memcmp(gameTid[CURPOS], "AMC", 3) == 0)	// Mario Kart DS
	 || (SDKVersion > 0x5000000 && (arm7size==0x26370 || arm7size==0x2642C || arm7size==0x26488))) {
		return true;
	}

	return false;
}

bool donorRomTextShown = false;
void revertDonorRomText(void) {
	if (!donorRomTextShown || strncmp(SET_AS_DONOR_ROM, "Done!", 5) != 0) return;

	sprintf(SET_AS_DONOR_ROM, "%s", STR_SET_AS_DONOR_ROM.c_str());
}

void perGameSettings (std::string filename) {
	int pressed = 0;

	if (ms().theme == 5) {
		displayGameIcons = false;
	} else if (ms().theme == 4) {
		snd().playStartup();
		fadeType = false;	   // Fade to black
		for (int i = 0; i < 25; i++) {
			swiWaitForVBlank();
		}
		currentBg = 1;
		displayGameIcons = false;
		fadeType = true;
	} else {
		dbox_showIcon = true;
		showdialogbox = true;
	}
	clearText();
	
	snprintf (fileCounter, sizeof(fileCounter), "%i/%i", (CURPOS+1)+PAGENUM*40, file_count);
	
	perGameSettings_cursorPosition = 0;
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
	if (memcmp(gameTid[CURPOS], "HND", 3) == 0 || memcmp(gameTid[CURPOS], "HNE", 3) == 0 || !isHomebrew[CURPOS]) {
		SDKVersion = getSDKVersion(f_nds_file);
		showSDKVersion = true;
	}
	u32 arm9dst = 0;
	u32 arm7size = 0;
	fseek(f_nds_file, 0x28, SEEK_SET);
	fread(&arm9dst, sizeof(u32), 1, f_nds_file);
	fseek(f_nds_file, 0x3C, SEEK_SET);
	fread(&arm7size, sizeof(u32), 1, f_nds_file);
	fclose(f_nds_file);
	
	bool showPerGameSettings =
		(!isDSiWare[CURPOS]
		&& memcmp(gameTid[CURPOS], "HND", 3) != 0
		&& memcmp(gameTid[CURPOS], "HNE", 3) != 0);
	if (!ms().useBootstrap && !isHomebrew[CURPOS] && REG_SCFG_EXT == 0) {
		showPerGameSettings = false;
	}

	firstPerGameOpShown = 0;
	perGameOps = -1;
	for (int i = 0; i < 10; i++) {
		perGameOp[i] = -1;
	}
	if (isHomebrew[CURPOS]) {		// Per-game settings for homebrew
		if (!ms().secondaryDevice) {
			perGameOps++;
			perGameOp[perGameOps] = 0;	// Language
			perGameOps++;
			perGameOp[perGameOps] = 1;	// RAM disk number
		}
		perGameOps++;
		perGameOp[perGameOps] = 6;	// Direct boot
		if (isDSiMode() || !ms().secondaryDevice) {
			perGameOps++;
			perGameOp[perGameOps] = 2;	// Run in
		}
		if (REG_SCFG_EXT != 0) {
			perGameOps++;
			perGameOp[perGameOps] = 3;	// ARM9 CPU Speed
			perGameOps++;
			perGameOp[perGameOps] = 4;	// VRAM Boost
		}
		if (!ms().secondaryDevice) {
			perGameOps++;
			perGameOp[perGameOps] = 7;	// Bootstrap
		}
		if (isDSiMode() && ms().consoleModel >= 2 && sdFound() && (gameTid[CURPOS][0] == 'W' || romVersion[CURPOS] == 0x57)) {
			perGameOps++;
			perGameOp[perGameOps] = 8;	// Screen Aspect Ratio
		}
	} else if (showPerGameSettings) {	// Per-game settings for retail/commercial games with nds-bootstrap/B4DS
		if (ms().useBootstrap || !ms().secondaryDevice) {
			perGameOps++;
			perGameOp[perGameOps] = 0;	// Language
			perGameOps++;
			perGameOp[perGameOps] = 1;	// Save number
			if (isDSiMode()) {
				perGameOps++;
				perGameOp[perGameOps] = 2;	// Run in
			}
		}
		if (REG_SCFG_EXT != 0) {
			perGameOps++;
			perGameOp[perGameOps] = 3;	// ARM9 CPU Speed
			perGameOps++;
			perGameOp[perGameOps] = 4;	// VRAM Boost
		}
		if (ms().useBootstrap || !ms().secondaryDevice) {
			if ((arm9dst != 0x02004000 && SDKVersion >= 0x2008000 && SDKVersion < 0x5000000) || !isDSiMode()) {
				perGameOps++;
				perGameOp[perGameOps] = 5;	// Heap shrink
			}
			perGameOps++;
			perGameOp[perGameOps] = 7;	// Bootstrap
			if (isDSiMode() && ms().consoleModel >= 2 && sdFound()) {
				perGameOps++;
				perGameOp[perGameOps] = 8;	// Screen Aspect Ratio
			}
			if (showSetDonorRom(arm7size, SDKVersion)) {
				perGameOps++;
				perGameOp[perGameOps] = 9;	// Set as Donor ROM
				donorRomTextShown = true;
			} else {
				donorRomTextShown = false;
			}
		}
	}

	snprintf (gameTIDText, sizeof(gameTIDText), gameTid[CURPOS][0]==0 ? "" : "TID: %s", gameTid[CURPOS]);

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
	if (ms().theme == 5) {
		dbox_showIcon = true;
	} else if (ms().theme == 4) {
		while (!screenFadedIn()) { swiWaitForVBlank(); }
		dbox_showIcon = true;
	} else {
		for (int i = 0; i < 30; i++) { snd().updateStream(); swiWaitForVBlank(); }
	}

	sprintf(LANGUAGE, "%s:", STR_LANGUAGE.c_str());
	sprintf(RAM_DISK, "%s:", STR_RAM_DISK.c_str());
	sprintf(SAVE_NO, "%s:", STR_SAVE_NO.c_str());
	sprintf(RUN_IN, "%s:", STR_RUN_IN.c_str());
	sprintf(ARM9_CPU_SPEED, "%s:", STR_ARM9_CPU_SPEED.c_str());
	sprintf(VRAM_BOOST, "%s:", STR_VRAM_BOOST.c_str());
	sprintf(HEAP_SHRINK, "%s:", STR_HEAP_SHRINK.c_str());
	sprintf(DIRECT_BOOT, "%s:", STR_DIRECT_BOOT.c_str());
	sprintf(SCREEN_ASPECT_RATIO, "%s:", STR_SCREEN_ASPECT_RATIO.c_str());
	sprintf(SET_AS_DONOR_ROM, "%s", STR_SET_AS_DONOR_ROM.c_str());

	// About 38 characters fit in the box.
	dirContName = filename;
	if (strlen(dirContName.c_str()) > 35) {
		// Truncate to 35, 35 + 3 = 38 (because we append "...").
		dirContName.resize(32, ' ');
		size_t first = dirContName.find_first_not_of(' ');
		size_t last = dirContName.find_last_not_of(' ');
		dirContName = dirContName.substr(first, (last - first + 1));
		dirContName.append("...");
	}

	// Test 3DS TWLCFG extraction
	/*FILE* cfgFile = fopen("sd:/TWLCFG.bin", "wb");
	fwrite((void*)0x02000000, 1, 0x4000, cfgFile);
	fclose(cfgFile);*/

	int row1Y = (ms().theme==5 ? 78 : 66);
	if (!showSDKVersion && gameTid[CURPOS][0] == 0) {
		row1Y += 4;
	}
	int row2Y = (ms().theme==5 ? 92 : 80);
	int botRowY = (ms().theme==5 ? 172 : 160);

	while (1) {
		clearText();
		titleUpdate(isDirectory[CURPOS], filename.c_str(), CURPOS);

		printSmall(false, 16, row1Y, dirContName.c_str());
		//if (showSDKVersion) printSmall(false, 16, row2Y, (useTwlCfg ? "TwlCfg found!" : SDKnumbertext));
		if (showSDKVersion) printSmall(false, 16, row2Y, SDKnumbertext);
		printSmall(false, 176, row2Y, gameTIDText);
		printSmall(false, 16, botRowY, fileCounter);

		int perGameOpYpos = (ms().theme==5 ? 110 : 98);

		if (showPerGameSettings) {
			printSmall(false, 16, perGameOpYpos+(perGameSettings_cursorPosition*14)-(firstPerGameOpShown*14), ">");
		}
		for (int i = firstPerGameOpShown; i < firstPerGameOpShown+4; i++) {
		if (!showPerGameSettings || perGameOp[i] == -1) break;
		switch (perGameOp[i]) {
			case 0:
				printSmall(false, 24, perGameOpYpos, LANGUAGE);
				if (perGameSettings_language == -2) {
					printSmallRightAlign(false, 256-24, perGameOpYpos, STR_DEFAULT.c_str());
				} else if (perGameSettings_language == -1) {
					printSmallRightAlign(false, 256-24, perGameOpYpos, STR_SYSTEM.c_str());
				} else if (perGameSettings_language == 0) {
					printSmallRightAlign(false, 256-24, perGameOpYpos, "Japanese");
				} else if (perGameSettings_language == 1) {
					printSmallRightAlign(false, 256-24, perGameOpYpos, "English");
				} else if (perGameSettings_language == 2) {
					printSmallRightAlign(false, 256-24, perGameOpYpos, "French");
				} else if (perGameSettings_language == 3) {
					printSmallRightAlign(false, 256-24, perGameOpYpos, "German");
				} else if (perGameSettings_language == 4) {
					printSmallRightAlign(false, 256-24, perGameOpYpos, "Italian");
				} else if (perGameSettings_language == 5) {
					printSmallRightAlign(false, 256-24, perGameOpYpos, "Spanish");
				} else if (perGameSettings_language == 6) {
					printSmallRightAlign(false, 256-24, perGameOpYpos, "Chinese");
				} else if (perGameSettings_language == 7) {
					printSmallRightAlign(false, 256-24, perGameOpYpos, "Korean");
				}
				break;
			case 1:
				if (isHomebrew[CURPOS]) {
					printSmall(false, 24, perGameOpYpos, RAM_DISK);
					snprintf (saveNoDisplay, sizeof(saveNoDisplay), "%i", perGameSettings_ramDiskNo);
				} else {
					printSmall(false, 24, perGameOpYpos, SAVE_NO);
					snprintf (saveNoDisplay, sizeof(saveNoDisplay), "%i", perGameSettings_saveNo);
				}
				if (isHomebrew[CURPOS] && perGameSettings_ramDiskNo == -1) {
					printSmallRightAlign(false, 256-24, perGameOpYpos, STR_NONE.c_str());
				} else {
					printSmallRightAlign(false, 256-24, perGameOpYpos, saveNoDisplay);
				}
				break;
			case 2:
				printSmall(false, 24, perGameOpYpos, RUN_IN);
				if (perGameSettings_dsiMode == -1) {
					printSmallRightAlign(false, 256-24, perGameOpYpos, STR_DEFAULT.c_str());
				} else if (perGameSettings_dsiMode == 2) {
					printSmallRightAlign(false, 256-24, perGameOpYpos, "DSi mode (Forced)");
				} else if (perGameSettings_dsiMode == 1) {
					printSmallRightAlign(false, 256-24, perGameOpYpos, "DSi mode");
				} else {
					printSmallRightAlign(false, 256-24, perGameOpYpos, "DS mode");
				}
				break;
			case 3:
				printSmall(false, 24, perGameOpYpos, ARM9_CPU_SPEED);
				if (perGameSettings_dsiMode > 0 && isDSiMode()) {
					printSmallRightAlign(false, 256-24, perGameOpYpos, "133mhz (TWL)");
				} else {
					if (perGameSettings_boostCpu == -1) {
						printSmallRightAlign(false, 256-24, perGameOpYpos, STR_DEFAULT.c_str());
					} else if (perGameSettings_boostCpu == 1) {
						printSmallRightAlign(false, 256-24, perGameOpYpos, "133mhz (TWL)");
					} else {
						printSmallRightAlign(false, 256-24, perGameOpYpos, "67mhz (NTR)");
					}
				}
				break;
			case 4:
				printSmall(false, 24, perGameOpYpos, VRAM_BOOST);
				if (perGameSettings_dsiMode > 0 && isDSiMode()) {
					printSmallRightAlign(false, 256-24, perGameOpYpos, STR_ON.c_str());
				} else {
					if (perGameSettings_boostVram == -1) {
						printSmallRightAlign(false, 256-24, perGameOpYpos, STR_DEFAULT.c_str());
					} else if (perGameSettings_boostVram == 1) {
						printSmallRightAlign(false, 256-24, perGameOpYpos, STR_ON.c_str());
					} else {
						printSmallRightAlign(false, 256-24, perGameOpYpos, STR_OFF.c_str());
					}
				}
				break;
			case 5:
				printSmall(false, 24, perGameOpYpos, HEAP_SHRINK);
				if (perGameSettings_heapShrink == -1) {
					printSmallRightAlign(false, 256-24, perGameOpYpos, STR_AUTO.c_str());
				} else if (perGameSettings_heapShrink == 1) {
					printSmallRightAlign(false, 256-24, perGameOpYpos, STR_ON.c_str());
				} else {
					printSmallRightAlign(false, 256-24, perGameOpYpos, STR_OFF.c_str());
				}
				break;
			case 6:
				printSmall(false, 24, perGameOpYpos, DIRECT_BOOT);
				if (perGameSettings_directBoot) {
					printSmallRightAlign(false, 256-24, perGameOpYpos, STR_YES.c_str());
				} else {
					printSmallRightAlign(false, 256-24, perGameOpYpos, STR_NO.c_str());
				}
				break;
			case 7:
				printSmall(false, 24, perGameOpYpos, "Bootstrap:");
				if (perGameSettings_bootstrapFile == -1) {
					printSmallRightAlign(false, 256-24, perGameOpYpos, STR_DEFAULT.c_str());
				} else if (perGameSettings_bootstrapFile == 1) {
					printSmallRightAlign(false, 256-24, perGameOpYpos, STR_NIGHTLY.c_str());
				} else {
					printSmallRightAlign(false, 256-24, perGameOpYpos, STR_RELEASE.c_str());
				}
				break;
			case 8:
				printSmall(false, 24, perGameOpYpos, SCREEN_ASPECT_RATIO);
				if (perGameSettings_wideScreen == -1) {
					printSmallRightAlign(false, 256-24, perGameOpYpos, STR_DEFAULT.c_str());
				} else if (perGameSettings_wideScreen == 1) {
					printSmallRightAlign(false, 256-24, perGameOpYpos, "16:10");
				} else {
					printSmallRightAlign(false, 256-24, perGameOpYpos, "4:3");
				}
				break;
			case 9:
				printSmallCentered(false, perGameOpYpos, SET_AS_DONOR_ROM);
				break;
		}
		perGameOpYpos += 14;
		}
		if (isHomebrew[CURPOS]) {		// Per-game settings for homebrew
			printSmall(false, 194, botRowY, BUTTON_B" Back");
		} else if (!showPerGameSettings) {
			printSmall(false, 208, botRowY, BUTTON_A" OK");
		} else {	// Per-game settings for retail/commercial games
			if ((isDSiMode() && ms().useBootstrap) || !ms().secondaryDevice) {
				printSmall(false, 128, botRowY, BUTTON_X " Cheats  " BUTTON_B" Back");
			} else {
				printSmall(false, 194, botRowY, BUTTON_B" Back");
			}
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

		if (!showPerGameSettings) {
			if ((pressed & KEY_A) || (pressed & KEY_B)) {
				snd().playBack();
				break;
			}
		} else {
			if (pressed & KEY_UP) {
				snd().playSelect();
				revertDonorRomText();
				perGameSettings_cursorPosition--;
				if (perGameSettings_cursorPosition < 0) {
					perGameSettings_cursorPosition = perGameOps;
					firstPerGameOpShown = (perGameOps>2 ? perGameOps-3 : 0);
				} else if (perGameSettings_cursorPosition < firstPerGameOpShown) {
					firstPerGameOpShown--;
				}
			}
			if (pressed & KEY_DOWN) {
				snd().playSelect();
				revertDonorRomText();
				perGameSettings_cursorPosition++;
				if (perGameSettings_cursorPosition > perGameOps) {
					perGameSettings_cursorPosition = 0;
					firstPerGameOpShown = 0;
				} else if (perGameSettings_cursorPosition > firstPerGameOpShown+3) {
					firstPerGameOpShown++;
				}
			}

			if (pressed & KEY_LEFT) {
				switch (perGameOp[perGameSettings_cursorPosition]) {
					case 0:
						perGameSettings_language--;
						if (perGameSettings_language < -2) perGameSettings_language = 7;
						break;
					case 1:
						if (isHomebrew[CURPOS]) {
							if (!perGameSettings_directBoot) {
								perGameSettings_ramDiskNo--;
								if (perGameSettings_ramDiskNo < -1) perGameSettings_ramDiskNo = 9;
							}
						} else {
							perGameSettings_saveNo--;
							if (perGameSettings_saveNo < 0) perGameSettings_saveNo = 9;
						}
						break;
					case 2:
						perGameSettings_dsiMode--;
						if (perGameSettings_dsiMode < -1) perGameSettings_dsiMode = 2;
						break;
					case 3:
						if (perGameSettings_dsiMode < 1) {
							perGameSettings_boostCpu--;
							if (perGameSettings_boostCpu < -1) perGameSettings_boostCpu = 1;
						}
						break;
					case 4:
						if (perGameSettings_dsiMode < 1) {
							perGameSettings_boostVram--;
							if (perGameSettings_boostVram < -1) perGameSettings_boostVram = 1;
						}
						break;
					case 5:
						perGameSettings_heapShrink--;
						if (perGameSettings_heapShrink < -1) perGameSettings_heapShrink = 1;
						break;
					case 6:
						perGameSettings_directBoot = !perGameSettings_directBoot;
						break;
					case 7:
						perGameSettings_bootstrapFile--;
						if (perGameSettings_bootstrapFile < -1) perGameSettings_bootstrapFile = 1;
						break;
					case 8:
						perGameSettings_wideScreen--;
						if (perGameSettings_wideScreen < -1) perGameSettings_wideScreen = 1;
						break;
				}
				(ms().theme == 4) ? snd().playLaunch() : snd().playSelect();
				perGameSettingsChanged = true;
			} else if ((pressed & KEY_A) || (pressed & KEY_RIGHT)) {
				switch (perGameOp[perGameSettings_cursorPosition]) {
					case 0:
						perGameSettings_language++;
						if (perGameSettings_language > 7) perGameSettings_language = -2;
						break;
					case 1:
						if (isHomebrew[CURPOS]) {
							if (!perGameSettings_directBoot) {
								perGameSettings_ramDiskNo++;
								if (perGameSettings_ramDiskNo > 9) perGameSettings_ramDiskNo = -1;
							}
						} else {
							perGameSettings_saveNo++;
							if (perGameSettings_saveNo > 9) perGameSettings_saveNo = 0;
						}
						break;
					case 2:
						perGameSettings_dsiMode++;
						if (perGameSettings_dsiMode > 2-isHomebrew[CURPOS]) perGameSettings_dsiMode = -1;
						break;
					case 3:
						if (perGameSettings_dsiMode < 1) {
							perGameSettings_boostCpu++;
							if (perGameSettings_boostCpu > 1) perGameSettings_boostCpu = -1;
						}
						break;
					case 4:
						if (perGameSettings_dsiMode < 1) {
							perGameSettings_boostVram++;
							if (perGameSettings_boostVram > 1) perGameSettings_boostVram = -1;
						}
						break;
					case 5:
						perGameSettings_heapShrink++;
						if (perGameSettings_heapShrink > 1) perGameSettings_heapShrink = -1;
						break;
					case 6:
						perGameSettings_directBoot = !perGameSettings_directBoot;
						break;
					case 7:
						perGameSettings_bootstrapFile++;
						if (perGameSettings_bootstrapFile > 1) perGameSettings_bootstrapFile = -1;
						break;
					case 8:
						perGameSettings_wideScreen++;
						if (perGameSettings_wideScreen > 1) perGameSettings_wideScreen = -1;
						break;
					case 9:
					  if (pressed & KEY_A) {
						const char* pathDefine = "DONOR_NDS_PATH";
						if (SDKVersion >= 0x2000000 && SDKVersion < 0x2008000) {
							pathDefine = "DONORE2_NDS_PATH";
						} else if (SDKVersion >= 0x2008000 && SDKVersion < 0x3000000) {
							pathDefine = "DONOR2_NDS_PATH";
						} else if (SDKVersion >= 0x3000000 && SDKVersion < 0x5000000) {
							pathDefine = "DONOR3_NDS_PATH";
						}
						std::string romFolderNoSlash = ms().romfolder[ms().secondaryDevice];
						RemoveTrailingSlashes(romFolderNoSlash);
						bootstrapinipath = (sdFound() ? "sd:/_nds/nds-bootstrap.ini" : "fat:/_nds/nds-bootstrap.ini");
						CIniFile bootstrapini(bootstrapinipath);
						bootstrapini.SetString("NDS-BOOTSTRAP", pathDefine, romFolderNoSlash+"/"+filename);
						bootstrapini.SaveIniFile(bootstrapinipath);
						sprintf(SET_AS_DONOR_ROM, "Done!");
					  }
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
			if ((pressed & KEY_X) && !isHomebrew[CURPOS]) {
			  if ((isDSiMode() && ms().useBootstrap) || !ms().secondaryDevice) {
				(ms().theme == 4) ? snd().playLaunch() : snd().playSelect();
				CheatCodelist codelist;
				codelist.selectCheats(filename);
			  }
			}
		}
	}
	showdialogbox = false;
	if (ms().theme == 5) {
		displayGameIcons = true;
		dbox_showIcon = false;
	} else if (ms().theme == 4) {
		fadeType = false;	   // Fade to black
		for (int i = 0; i < 25; i++) {
			swiWaitForVBlank();
		}
		clearText();
		currentBg = 0;
		displayGameIcons = true;
		fadeType = true;
		snd().playStartup();
	} else {
		clearText();
	}
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
