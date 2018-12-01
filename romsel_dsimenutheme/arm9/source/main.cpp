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
#include <maxmod9.h>

#include <stdio.h>
#include <fat.h>
#include <sys/stat.h>
#include <limits.h>

#include <string.h>
#include <unistd.h>
#include "common/gl2d.h"

#include "date.h"
#include "fileCopy.h"

#include "graphics/graphics.h"

#include "common/nitrofs.h"
#include "flashcard.h"
#include "ndsheaderbanner.h"
#include "nds_loader_arm9.h"
#include "fileBrowse.h"
#include "perGameSettings.h"

#include "graphics/iconHandler.h"
#include "graphics/fontHandler.h"

#include "inifile.h"

#include "language.h"

#include "cheat.h"
#include "crc.h"

#include "soundbank.h"
#include "soundbank_bin.h"

#include "sr_data_srllastran.h"	// For rebooting into the game (NTR-mode touch screen)
#include "sr_data_srllastran_twltouch.h"	// For rebooting into the game (TWL-mode touch screen)

bool whiteScreen = true;
bool fadeType = false;		// false = out, true = in
bool fadeSpeed = true;		// false = slow (for DSi launch effect), true = fast
bool controlTopBright = true;
bool controlBottomBright = true;

extern void ClearBrightness();
extern bool showProgressIcon;

const char* settingsinipath = "sd:/_nds/TWiLightMenu/settings.ini";
const char* bootstrapinipath = "sd:/_nds/nds-bootstrap.ini";

std::string dsiWareSrlPath;
std::string dsiWarePubPath;
std::string dsiWarePrvPath;
std::string homebrewArg;

const char *unlaunchAutoLoadID = "AutoLoadInfo";
static char hiyaNdsPath[14] = {'s','d','m','c',':','/','h','i','y','a','.','d','s','i'};

bool arm7SCFGLocked = false;
int consoleModel = 0;
/*	0 = Nintendo DSi (Retail)
	1 = Nintendo DSi (Dev/Panda)
	2 = Nintendo 3DS
	3 = New Nintendo 3DS	*/
bool isRegularDS = true;

// /**
//  * Remove trailing slashes from a pathname, if present.
//  * @param path Pathname to modify.
//  */
// static void RemoveTrailingSlashes(std::string& path)
// {
// 	while (!path.empty() && path[path.size()-1] == '/') {
// 		path.resize(path.size()-1);
// 	}
// }

/**
 * Remove trailing spaces from a cheat code line, if present.
 * @param path Code line to modify.
 */
static void RemoveTrailingSpaces(std::string& code)
{
	while (!code.empty() && code[code.size()-1] == ' ') {
		code.resize(code.size()-1);
	}
}

bool music = false;
extern mm_sound_effect mus_startup;
extern mm_sound_effect mus_menu;

std::string romfolder[2];

// These are used by flashcard functions and must retain their trailing slash.
static const std::string slashchar = "/";
static const std::string woodfat = "fat0:/";
static const std::string dstwofat = "fat1:/";

int donorSdkVer = 0;

bool gameSoftReset = false;

int mpuregion = 0;
int mpusize = 0;

bool applaunch = false;
bool startMenu = false;
bool gotosettings = false;

bool startButtonLaunch = false;
int launchType = 1;	// 0 = Slot-1, 1 = SD/Flash card, 2 = DSiWare, 3 = NES, 4 = (S)GB(C)
bool slot1LaunchMethod = true;	// false == Reboot, true == Direct
bool bootstrapFile = false;
bool homebrewBootstrap = false;

bool gbaBiosFound[2] = {false};
bool useGbarunner = false;
int appName = 0;
int theme = 0;
int subtheme = 0;
int cursorPosition[2] = {0};
int startMenu_cursorPosition = 0;
int pagenum[2] = {0};
bool showDirectories = true;
bool showBoxArt = true;
bool animateDsiIcons = false;
int launcherApp = -1;
int sysRegion = -1;

int guiLanguage = -1;
int bstrap_language = -1;
bool boostCpu = false;	// false == NTR, true == TWL
bool boostVram = false;
bool soundFix = false;
bool bstrap_dsiMode = false;

void LoadSettings(void) {
	// GUI
	CIniFile settingsini( settingsinipath );

	// UI settings.
	romfolder[0] = settingsini.GetString("SRLOADER", "ROM_FOLDER", "sd:/");
	romfolder[1] = settingsini.GetString("SRLOADER", "SECONDARY_ROM_FOLDER", "fat:/");
	pagenum[0] = settingsini.GetInt("SRLOADER", "PAGE_NUMBER", 0);
	pagenum[1] = settingsini.GetInt("SRLOADER", "SECONDARY_PAGE_NUMBER", 0);
	cursorPosition[0] = settingsini.GetInt("SRLOADER", "CURSOR_POSITION", 0);
	cursorPosition[1] = settingsini.GetInt("SRLOADER", "SECONDARY_CURSOR_POSITION", 0);
	startMenu_cursorPosition = settingsini.GetInt("SRLOADER", "STARTMENU_CURSOR_POSITION", 1);
	consoleModel = settingsini.GetInt("SRLOADER", "CONSOLE_MODEL", 0);
    appName = settingsini.GetInt("SRLOADER", "APP_NAME", appName);

	// Customizable UI settings.
	guiLanguage = settingsini.GetInt("SRLOADER", "LANGUAGE", -1);
	useGbarunner = settingsini.GetInt("SRLOADER", "USE_GBARUNNER2", 0);
	if (!isRegularDS) useGbarunner = true;
	gotosettings = settingsini.GetInt("SRLOADER", "GOTOSETTINGS", 0);
	theme = settingsini.GetInt("SRLOADER", "THEME", 0);
	subtheme = settingsini.GetInt("SRLOADER", "SUB_THEME", 0);
	showDirectories = settingsini.GetInt("SRLOADER", "SHOW_DIRECTORIES", 1);
	showBoxArt = settingsini.GetInt("SRLOADER", "SHOW_BOX_ART", 1);
	animateDsiIcons = settingsini.GetInt("SRLOADER", "ANIMATE_DSI_ICONS", 1);
	if (consoleModel < 2) {
		launcherApp = settingsini.GetInt("SRLOADER", "LAUNCHER_APP", launcherApp);
		sysRegion = settingsini.GetInt("SRLOADER", "SYS_REGION", sysRegion);
	}

	previousUsedDevice = settingsini.GetInt("SRLOADER", "PREVIOUS_USED_DEVICE", previousUsedDevice);
	if (bothSDandFlashcard()) {
		secondaryDevice = settingsini.GetInt("SRLOADER", "SECONDARY_DEVICE", secondaryDevice);
	} else if (flashcardFound()) {
		flashcard = settingsini.GetInt("SRLOADER", "FLASHCARD", 0);
		secondaryDevice = true;
	} else {
		secondaryDevice = false;
	}

	slot1LaunchMethod = settingsini.GetInt("SRLOADER", "SLOT1_LAUNCHMETHOD", 1);
	bootstrapFile = settingsini.GetInt("SRLOADER", "BOOTSTRAP_FILE", 0);
	startButtonLaunch = settingsini.GetInt("SRLOADER", "START_BUTTON_LAUNCH", 0);

	// Default nds-bootstrap settings
	bstrap_language = settingsini.GetInt("NDS-BOOTSTRAP", "LANGUAGE", -1);
	boostCpu = settingsini.GetInt("NDS-BOOTSTRAP", "BOOST_CPU", 0);
	boostVram = settingsini.GetInt("NDS-BOOTSTRAP", "BOOST_VRAM", 0);
	soundFix = settingsini.GetInt("NDS-BOOTSTRAP", "SOUND_FIX", 0);
	bstrap_dsiMode = settingsini.GetInt("NDS-BOOTSTRAP", "DSI_MODE", 0);

    dsiWareSrlPath = settingsini.GetString("SRLOADER", "DSIWARE_SRL", dsiWareSrlPath);
    dsiWarePubPath = settingsini.GetString("SRLOADER", "DSIWARE_PUB", dsiWarePubPath);
    dsiWarePrvPath = settingsini.GetString("SRLOADER", "DSIWARE_PRV", dsiWarePrvPath);
    launchType = settingsini.GetInt("SRLOADER", "LAUNCH_TYPE", launchType);
}

void SaveSettings(void) {
	// GUI
	CIniFile settingsini( settingsinipath );

	settingsini.SetString("SRLOADER", "ROM_FOLDER", romfolder[0]);
	settingsini.SetString("SRLOADER", "SECONDARY_ROM_FOLDER", romfolder[1]);
	settingsini.SetInt("SRLOADER", "PAGE_NUMBER", pagenum[0]);
	settingsini.SetInt("SRLOADER", "SECONDARY_PAGE_NUMBER", pagenum[1]);
	settingsini.SetInt("SRLOADER", "CURSOR_POSITION", cursorPosition[0]);
	settingsini.SetInt("SRLOADER", "SECONDARY_CURSOR_POSITION", cursorPosition[1]);
	settingsini.SetInt("SRLOADER", "STARTMENU_CURSOR_POSITION", startMenu_cursorPosition);

	// UI settings.
	settingsini.SetInt("SRLOADER", "GOTOSETTINGS", gotosettings);
	if (bothSDandFlashcard()) {
		settingsini.SetInt("SRLOADER", "SECONDARY_DEVICE", secondaryDevice);
	}
	if (!gotosettings) {
		settingsini.SetInt("SRLOADER", "PREVIOUS_USED_DEVICE", previousUsedDevice);
		settingsini.SetString("SRLOADER", "DSIWARE_SRL", dsiWareSrlPath);
		settingsini.SetString("SRLOADER", "DSIWARE_PUB", dsiWarePubPath);
		settingsini.SetString("SRLOADER", "DSIWARE_PRV", dsiWarePrvPath);
		settingsini.SetInt("SRLOADER", "LAUNCH_TYPE", launchType);
		settingsini.SetString("SRLOADER", "HOMEBREW_ARG", homebrewArg);
		settingsini.SetInt("SRLOADER", "HOMEBREW_BOOTSTRAP", homebrewBootstrap);
	}
	//settingsini.SetInt("SRLOADER", "THEME", theme);
	//settingsini.SetInt("SRLOADER", "SUB_THEME", subtheme);
	settingsini.SaveIniFile(settingsinipath);
}

int colorRvalue;
int colorGvalue;
int colorBvalue;

/**
 * Load the console's color.
 */
void LoadColor(void) {
	switch (PersonalData->theme) {
		case 0:
		default:
			colorRvalue = 99;
			colorGvalue = 127;
			colorBvalue = 127;
			break;
		case 1:
			colorRvalue = 139;
			colorGvalue = 99;
			colorBvalue = 0;
			break;
		case 2:
			colorRvalue = 255;
			colorGvalue = 0;
			colorBvalue = 0;
			break;
		case 3:
			colorRvalue = 255;
			colorGvalue = 127;
			colorBvalue = 127;
			break;
		case 4:
			colorRvalue = 223;
			colorGvalue = 63;
			colorBvalue = 0;
			break;
		case 5:
			colorRvalue = 215;
			colorGvalue = 215;
			colorBvalue = 0;
			break;
		case 6:
			colorRvalue = 215;
			colorGvalue = 255;
			colorBvalue = 0;
			break;
		case 7:
			colorRvalue = 0;
			colorGvalue = 255;
			colorBvalue = 0;
			break;
		case 8:
			colorRvalue = 63;
			colorGvalue = 255;
			colorBvalue = 63;
			break;
		case 9:
			colorRvalue = 31;
			colorGvalue = 231;
			colorBvalue = 31;
			break;
		case 10:
			colorRvalue = 0;
			colorGvalue = 63;
			colorBvalue = 255;
			break;
		case 11:
			colorRvalue = 63;
			colorGvalue = 63;
			colorBvalue = 255;
			break;
		case 12:
			colorRvalue = 0;
			colorGvalue = 0;
			colorBvalue = 255;
			break;
		case 13:
			colorRvalue = 127;
			colorGvalue = 0;
			colorBvalue = 255;
			break;
		case 14:
			colorRvalue = 255;
			colorGvalue = 0;
			colorBvalue = 255;
			break;
		case 15:
			colorRvalue = 255;
			colorGvalue = 0;
			colorBvalue = 127;
			break;
	}
}

bool useBootstrap = false;

using namespace std;

bool dropDown = false;
bool redoDropDown = false;
bool showbubble = false;
bool showSTARTborder = false;

bool titleboxXmoveleft = false;
bool titleboxXmoveright = false;

bool applaunchprep = false;

int spawnedtitleboxes = 0;

u16 usernameRendered[10];
bool usernameRenderedDone = false;

touchPosition touch;

/**
 * Set donor SDK version for a specific game.
 */
void SetDonorSDK(const char* filename) {
	FILE *f_nds_file = fopen(filename, "rb");

	char game_TID[5];
	char game_TID_letter1[5];
	grabTID(f_nds_file, game_TID);
	grabTID(f_nds_file, game_TID_letter1);
	game_TID[4] = 0;
	game_TID[3] = 0;
	game_TID_letter1[4] = 0;
	game_TID_letter1[3] = 0;
	game_TID_letter1[2] = 0;
	game_TID_letter1[1] = 0;
	fclose(f_nds_file);
	
	donorSdkVer = 0;

	// Check for ROM hacks that need an SDK version.
	static const char sdk2_list[][4] = {
		"AMQ",	// Mario vs. Donkey Kong 2 - March of the Minis
		"AMH",	// Metroid Prime Hunters
		"ASM",	// Super Mario 64 DS
	};
	
	static const char sdk3_list[][4] = {
		"AMC",	// Mario Kart DS
		"EKD",	// Ermii Kart DS (Mario Kart DS hack)
		"A2D",	// New Super Mario Bros.
		"ADA",	// Pokemon Diamond
		"APA",	// Pokemon Pearl
		"ARZ",	// Rockman ZX/MegaMan ZX
		"YZX",	// Rockman ZX Advent/MegaMan ZX Advent
	};
	
	static const char sdk4_list[][4] = {
		"YKW",	// Kirby Super Star Ultra
		"A6C",	// MegaMan Star Force: Dragon
		"A6B",	// MegaMan Star Force: Leo
		"A6A",	// MegaMan Star Force: Pegasus
		"B6Z",	// Rockman Zero Collection/MegaMan Zero Collection
		"YT7",	// SEGA Superstars Tennis
		"AZL",	// Style Savvy
		"BKI",	// The Legend of Zelda: Spirit Tracks
		"B3R",	// Pokemon Ranger: Guardian Signs
	};

	static const char sdk5_list[][4] = {
		"B2D",	// Doctor Who: Evacuation Earth
		"BH2",	// Super Scribblenauts
		"BSD",	// Lufia: Curse of the Sinistrals
		"BXS",	// Sonic Colo(u)rs
		"BOE",	// Inazuma Eleven 3: Sekai heno Chousen! The Ogre
		"BQ8",	// Crafting Mama
		"BK9",	// Kingdom Hearts: Re-Coded
		"BRJ",	// Radiant Historia
		"IRA",	// Pokemon Black Version
		"IRB",	// Pokemon White Version
		"VI2",	// Fire Emblem: Shin Monshou no Nazo Hikari to Kage no Eiyuu
		"BYY",	// Yu-Gi-Oh 5Ds World Championship 2011: Over The Nexus
		"UZP",	// Learn with Pokemon: Typing Adventure
		"B6F",	// LEGO Batman 2: DC Super Heroes
		"IRE",	// Pokemon Black Version 2
		"IRD",	// Pokemon White Version 2
	};

	// TODO: If the list gets large enough, switch to bsearch().
	for (unsigned int i = 0; i < sizeof(sdk2_list)/sizeof(sdk2_list[0]); i++) {
		if (!memcmp(game_TID, sdk2_list[i], 3)) {
			// Found a match.
			donorSdkVer = 2;
			break;
		}
	}
	
	// TODO: If the list gets large enough, switch to bsearch().
	for (unsigned int i = 0; i < sizeof(sdk3_list)/sizeof(sdk3_list[0]); i++) {
		if (!memcmp(game_TID, sdk3_list[i], 3)) {
			// Found a match.
			donorSdkVer = 3;
			break;
		}
	}

	// TODO: If the list gets large enough, switch to bsearch().
	for (unsigned int i = 0; i < sizeof(sdk4_list)/sizeof(sdk4_list[0]); i++) {
		if (!memcmp(game_TID, sdk4_list[i], 3)) {
			// Found a match.
			donorSdkVer = 4;
			break;
		}
	}

	if(strcmp(game_TID_letter1, "V") == 0) {
		donorSdkVer = 5;
	} else {
		// TODO: If the list gets large enough, switch to bsearch().
		for (unsigned int i = 0; i < sizeof(sdk5_list)/sizeof(sdk5_list[0]); i++) {
			if (!memcmp(game_TID, sdk5_list[i], 3)) {
				// Found a match.
				donorSdkVer = 5;
				break;
			}
		}
	}
}

/**
 * Disable soft-reset, in favor of non OS_Reset one, for a specific game.
 */
void SetGameSoftReset(const char* filename) {
	FILE *f_nds_file = fopen(filename, "rb");

	char game_TID[5];
	fseek(f_nds_file, offsetof(sNDSHeadertitlecodeonly, gameCode), SEEK_SET);
	fread(game_TID, 1, 4, f_nds_file);
	game_TID[4] = 0;
	game_TID[3] = 0;
	fclose(f_nds_file);

	scanKeys();
	int pressed = keysDownRepeat();
	
	gameSoftReset = false;

	// Check for games that have it's own reset function (OS_Reset not used).
	static const char list[][4] = {
		"NTR",	// Download Play ROMs
		"ASM",	// Super Mario 64 DS
		"SMS",	// Super Mario Star World, and Mario's Holiday
		"AMC",	// Mario Kart DS
		"EKD",	// Ermii Kart DS
		"A2D",	// New Super Mario Bros.
		"ARZ",	// Rockman ZX/MegaMan ZX
		"AKW",	// Kirby Squeak Squad/Mouse Attack
		"YZX",	// Rockman ZX Advent/MegaMan ZX Advent
		"B6Z",	// Rockman Zero Collection/MegaMan Zero Collection
	};

	// TODO: If the list gets large enough, switch to bsearch().
	for (unsigned int i = 0; i < sizeof(list)/sizeof(list[0]); i++) {
		if (!memcmp(game_TID, list[i], 3)) {
			// Found a match.
			gameSoftReset = true;
			break;
		}
	}

	if(pressed & KEY_R){
		gameSoftReset = true;
	}
}

/**
 * Set MPU settings for a specific game.
 */
void SetMPUSettings(const char* filename) {
	FILE *f_nds_file = fopen(filename, "rb");

	char game_TID[5];
	fseek(f_nds_file, offsetof(sNDSHeadertitlecodeonly, gameCode), SEEK_SET);
	fread(game_TID, 1, 4, f_nds_file);
	game_TID[4] = 0;
	game_TID[3] = 0;
	fclose(f_nds_file);

	scanKeys();
	int pressed = keysDownRepeat();
	
	if(pressed & KEY_B){
		mpuregion = 1;
	} else if(pressed & KEY_X){
		mpuregion = 2;
	} else if(pressed & KEY_Y){
		mpuregion = 3;
	} else {
		mpuregion = 0;
	}
	if(pressed & KEY_RIGHT){
		mpusize = 3145728;
	} else if(pressed & KEY_LEFT){
		mpusize = 1;
	} else {
		mpusize = 0;
	}

	// Check for games that need an MPU size of 3 MB.
	static const char mpu_3MB_list[][4] = {
		"A7A",	// DS Download Station - Vol 1
		"A7B",	// DS Download Station - Vol 2
		"A7C",	// DS Download Station - Vol 3
		"A7D",	// DS Download Station - Vol 4
		"A7E",	// DS Download Station - Vol 5
		"A7F",	// DS Download Station - Vol 6 (EUR)
		"A7G",	// DS Download Station - Vol 6 (USA)
		"A7H",	// DS Download Station - Vol 7
		"A7I",	// DS Download Station - Vol 8
		"A7J",	// DS Download Station - Vol 9
		"A7K",	// DS Download Station - Vol 10
		"A7L",	// DS Download Station - Vol 11
		"A7M",	// DS Download Station - Vol 12
		"A7N",	// DS Download Station - Vol 13
		"A7O",	// DS Download Station - Vol 14
		"A7P",	// DS Download Station - Vol 15
		"A7Q",	// DS Download Station - Vol 16
		"A7R",	// DS Download Station - Vol 17
		"A7S",	// DS Download Station - Vol 18
		"A7T",	// DS Download Station - Vol 19
		"ARZ",	// Rockman ZX/MegaMan ZX
		"YZX",	// Rockman ZX Advent/MegaMan ZX Advent
		"B6Z",	// Rockman Zero Collection/MegaMan Zero Collection
		"A2D",	// New Super Mario Bros.
	};

	// TODO: If the list gets large enough, switch to bsearch().
	for (unsigned int i = 0; i < sizeof(mpu_3MB_list)/sizeof(mpu_3MB_list[0]); i++) {
		if (!memcmp(game_TID, mpu_3MB_list[i], 3)) {
			// Found a match.
			mpuregion = 1;
			mpusize = 3145728;
			break;
		}
	}
}

//---------------------------------------------------------------------------------
void stop (void) {
//---------------------------------------------------------------------------------
	while (1) {
		swiIntrWait(0, 1);
	}
}

char filePath[PATH_MAX];

//---------------------------------------------------------------------------------
void doPause() {
//---------------------------------------------------------------------------------
	// iprintf("Press start...\n");
	// printSmall(false, x, y, "Press start...");
	while(1) {
		scanKeys();
		if(keysDown() & KEY_START)
			break;
		swiIntrWait(0, 1);
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

void loadGameOnFlashcard (const char* ndsPath, std::string filename, bool usePerGameSettings) {
	bool runNds_boostCpu = false;
	bool runNds_boostVram = false;
	if (isDSiMode() && usePerGameSettings) {
		loadPerGameSettings(filename);
		if (perGameSettings_boostCpu == -1) {
			runNds_boostCpu = boostCpu;
		} else {
			runNds_boostCpu = perGameSettings_boostCpu;
		}
		if (perGameSettings_boostVram == -1) {
			runNds_boostVram = boostVram;
		} else {
			runNds_boostVram = perGameSettings_boostVram;
		}
	}
	std::string path;
	int err = 0;
	if (memcmp(io_dldi_data->friendlyName, "R4iDSN", 6) == 0)
	{
		CIniFile fcrompathini("fat:/_wfwd/lastsave.ini");
		path = ReplaceAll(ndsPath, "fat:/", woodfat);
		fcrompathini.SetString("Save Info", "lastLoaded", path);
		fcrompathini.SaveIniFile("fat:/_wfwd/lastsave.ini");
		err = runNdsFile ("fat:/Wfwd.dat", 0, NULL, true, true, runNds_boostCpu, runNds_boostVram);
	}
	else if (memcmp(io_dldi_data->friendlyName, "Acekard AK2", 0xB) == 0)
	{
		CIniFile fcrompathini("fat:/_afwd/lastsave.ini");
		path = ReplaceAll(ndsPath, "fat:/", woodfat);
		fcrompathini.SetString("Save Info", "lastLoaded", path);
		fcrompathini.SaveIniFile("fat:/_afwd/lastsave.ini");
		err = runNdsFile ("fat:/Afwd.dat", 0, NULL, true, true, runNds_boostCpu, runNds_boostVram);
	}
	else if (memcmp(io_dldi_data->friendlyName, "DSTWO(Slot-1)", 0xD) == 0)
	{
		CIniFile fcrompathini("fat:/_dstwo/autoboot.ini");
		path = ReplaceAll(ndsPath, "fat:/", dstwofat);
		fcrompathini.SetString("Dir Info", "fullName", path);
		fcrompathini.SaveIniFile("fat:/_dstwo/autoboot.ini");
		err = runNdsFile ("fat:/_dstwo/autoboot.nds", 0, NULL, true, true, runNds_boostCpu, runNds_boostVram);
	}
	/*switch (flashcard) {
		case 0:
		case 1:
		default: {
			CIniFile fcrompathini("fat:/TTMenu/YSMenu.ini");
			path = ReplaceAll(ndsPath, "fat:/", slashchar);
			fcrompathini.SetString("YSMENU", "AUTO_BOOT", path);
			fcrompathini.SetString("YSMENU", "DEFAULT_DMA", "true");
			fcrompathini.SetString("YSMENU", "DEFAULT_RESET", "false");
			fcrompathini.SaveIniFile("fat:/TTMenu/YSMenu.ini");
			err = runNdsFile ("fat:/YSMenu.nds", 0, NULL, true, true, runNds_boostCpu, runNds_boostVram);
			break;
		}

		case 2:
		case 4:
		case 5: {
			CIniFile fcrompathini("fat:/_wfwd/lastsave.ini");
			path = ReplaceAll(ndsPath, "fat:/", woodfat);
			fcrompathini.SetString("Save Info", "lastLoaded", path);
			fcrompathini.SaveIniFile("fat:/_wfwd/lastsave.ini");
			err = runNdsFile ("fat:/Wfwd.dat", 0, NULL, true, true, runNds_boostCpu, runNds_boostVram);
			break;
		}

		case 3: {
			CIniFile fcrompathini("fat:/_afwd/lastsave.ini");
			path = ReplaceAll(ndsPath, "fat:/", woodfat);
			fcrompathini.SetString("Save Info", "lastLoaded", path);
			fcrompathini.SaveIniFile("fat:/_afwd/lastsave.ini");
			err = runNdsFile ("fat:/Afwd.dat", 0, NULL, true, true, runNds_boostCpu, runNds_boostVram);
			break;
		}

		case 6: {
			CIniFile fcrompathini("fat:/_dstwo/autoboot.ini");
			path = ReplaceAll(ndsPath, "fat:/", dstwofat);
			fcrompathini.SetString("Dir Info", "fullName", path);
			fcrompathini.SaveIniFile("fat:/_dstwo/autoboot.ini");
			err = runNdsFile ("fat:/_dstwo/autoboot.nds", 0, NULL, true, true, runNds_boostCpu, runNds_boostVram);
			break;
		}
	}*/
	char text[32];
	snprintf (text, sizeof(text), "Start failed. Error %i", err);
	ClearBrightness();
	printLarge(false, 4, 4, text);
	if (err == 0) {
		printLarge(false, 4, 20, "Flashcard may be unsupported.");
	}
	stop();
}

void unlaunchSetHiyaBoot(void) {
	memcpy((u8*)0x02000800, unlaunchAutoLoadID, 12);
	*(u16*)(0x0200080C) = 0x3F0;		// Unlaunch Length for CRC16 (fixed, must be 3F0h)
	*(u16*)(0x0200080E) = 0;			// Unlaunch CRC16 (empty)
	*(u32*)(0x02000810) |= BIT(0);		// Load the title at 2000838h
	*(u32*)(0x02000810) |= BIT(1);		// Use colors 2000814h
	*(u16*)(0x02000814) = 0x7FFF;		// Unlaunch Upper screen BG color (0..7FFFh)
	*(u16*)(0x02000816) = 0x7FFF;		// Unlaunch Lower screen BG color (0..7FFFh)
	memset((u8*)0x02000818, 0, 0x20+0x208+0x1C0);		// Unlaunch Reserved (zero)
	int i2 = 0;
	for (int i = 0; i < 14; i++) {
		*(u8*)(0x02000838+i2) = hiyaNdsPath[i];		// Unlaunch Device:/Path/Filename.ext (16bit Unicode,end by 0000h)
		i2 += 2;
	}
	while (*(u16*)(0x0200080E) == 0) {	// Keep running, so that CRC16 isn't 0
		*(u16*)(0x0200080E) = swiCRC16(0xFFFF, (void*)0x02000810, 0x3F0);		// Unlaunch CRC16
	}
}

void dsCardLaunch() {
	*(u32*)(0x02000300) = 0x434E4C54;	// Set "CNLT" warmboot flag
	*(u16*)(0x02000304) = 0x1801;
	*(u32*)(0x02000308) = 0x43415254;	// "CART"
	*(u32*)(0x0200030C) = 0x00000000;
	*(u32*)(0x02000310) = 0x43415254;	// "CART"
	*(u32*)(0x02000314) = 0x00000000;
	*(u32*)(0x02000318) = 0x00000013;
	*(u32*)(0x0200031C) = 0x00000000;
	while (*(u16*)(0x02000306) == 0x0000) {	// Keep running, so that CRC16 isn't 0
		*(u16*)(0x02000306) = swiCRC16(0xFFFF, (void*)0x02000308, 0x18);
	}

	unlaunchSetHiyaBoot();

	fifoSendValue32(FIFO_USER_02, 1);	// Reboot into DSiWare title, booted via Launcher
	for (int i = 0; i < 15; i++) swiIntrWait(0, 1);
}

//---------------------------------------------------------------------------------
int main(int argc, char **argv) {
//---------------------------------------------------------------------------------

	//overwrite reboot stub identifier
	
	extern u64 *fake_heap_end;
	*fake_heap_end = 0;

	defaultExceptionHandler();
	
	bool fatInited = fatInitDefault();

	// consoleDemoInit();
	// iprintf("Hello World");
	
	// while(1) {
	// 	swiIntrWait(0, 1);
	// }
	// return 0;

	// TODO: turn this into swiCopy
	memcpy(usernameRendered, PersonalData->name, sizeof(usernameRendered));
	//swiCopy(PersonalData->name, usernameRendered, )
	
	// Read user name
  	char *username = (char*)PersonalData->name;
		
	// text
	for (int i = 0; i < 10; i++) {
		if (username[i*2] == 0x00)
			username[i*2/2] = 0;
		else
			username[i*2/2] = username[i*2];
	}
	
	//snprintf (usernameRendered, sizeof(usernameRendered), "%s", username);
	
	LoadColor();

	if (!fatInited) {
		graphicsInit();
		fontInit();
		whiteScreen = false;
		fadeType = true;
		for (int i = 0; i < 5; i++) swiIntrWait(0, 1);
		if (!dropDown && theme == 0) {
			dropDown = true;
			for (int i = 0; i < 72; i++) swiIntrWait(0, 1);
		} else {
			for (int i = 0; i < 25; i++) swiIntrWait(0, 1);
		}
		showbubble = true;
		printLargeCentered(false, 32, "fatInitDefault failed!");

		// Control the DSi Menu, but can't launch anything.
		int pressed = 0;

		while (1) {
			// Power saving loop. Only poll the keys once per frame and sleep the CPU if there is nothing else to do
			do
			{
				scanKeys();
				pressed = keysDownRepeat();
				swiIntrWait(0, 1);
			}
			while (!pressed);

			if ((pressed & KEY_LEFT) && !titleboxXmoveleft && !titleboxXmoveright) {
				cursorPosition[secondaryDevice] -= 1;
				if (cursorPosition[secondaryDevice] >= 0) titleboxXmoveleft = true;
			} else if ((pressed & KEY_RIGHT) && !titleboxXmoveleft && !titleboxXmoveright) {
				cursorPosition[secondaryDevice] += 1;
				if (cursorPosition[secondaryDevice] <= 39) titleboxXmoveright = true;
			}
			if (cursorPosition[secondaryDevice] < 0)
			{
				cursorPosition[secondaryDevice] = 0;
			}
			else if (cursorPosition[secondaryDevice] > 39)
			{
				cursorPosition[secondaryDevice] = 39;
			}
		}
	}

	nitroFSInit("/_nds/TWiLightMenu/dsimenu.srldr");

	flashcardInit();

	if (access(settingsinipath, F_OK) != 0 && flashcardFound()) {
		settingsinipath = "fat:/_nds/TWiLightMenu/settings.ini";		// Fallback to .ini path on flashcard, if not found on SD card, or if SD access is disabled
	}
	
	langInit();

	std::string filename;
	std::string bootstrapfilename;

	fifoWaitValue32(FIFO_USER_06);
	if (fifoGetValue32(FIFO_USER_03) == 0) arm7SCFGLocked = true;	// If DSiMenu++ is being run from DSiWarehax or flashcard, then arm7 SCFG is locked.
	u16 arm7_SNDEXCNT = fifoGetValue32(FIFO_USER_07);
	if (arm7_SNDEXCNT != 0) isRegularDS = false;	// If sound frequency setting is found, then the console is not a DS Phat/Lite
	fifoSendValue32(FIFO_USER_07, 0);

	LoadSettings();
	
	if (access("sd:/bios.bin", F_OK) == 0) {
		gbaBiosFound[0] = true;
	}
	if (access("fat:/bios.bin", F_OK) == 0) {
		gbaBiosFound[1] = true;
	}

	graphicsInit();
	fontInit();
	iconManagerInit();

	keysSetRepeat(25, 5);

	vector<string> extensionList;
	extensionList.push_back(".nds");
	extensionList.push_back(".dsi");
	extensionList.push_back(".ids");
	if (consoleModel < 2) extensionList.push_back(".launcharg");	// Display .launcharg files, if on a DSi
	extensionList.push_back(".argv");
	extensionList.push_back(".gb");
	extensionList.push_back(".sgb");
	extensionList.push_back(".gbc");
	extensionList.push_back(".nes");
	extensionList.push_back(".fds");
	srand(time(NULL));
	
	char path[256];

	InitSound();

	if (!music) {
		if (theme == 1) {
			mmEffectEx(&mus_startup);
		} else {
			mmEffectEx(&mus_menu);
		}
		music = true;
	}

	//fcopy("sd:/test.txt", "fat:/delivery from SD.txt");

	if ((consoleModel < 2 && previousUsedDevice && bothSDandFlashcard() && launchType == 2 && access(dsiWarePubPath.c_str(), F_OK) == 0)
	|| (consoleModel < 2 && previousUsedDevice && bothSDandFlashcard() && launchType == 2 && access(dsiWarePrvPath.c_str(), F_OK) == 0))
	{
		fadeType = true;	// Fade in from white
		printLargeCentered(false, 88, "Now copying data...");
		printSmallCentered(false, 104, "Do not turn off the power.");
		for (int i = 0; i < 15; i++) swiIntrWait(0, 1);
		reloadFontPalettes();
		for (int i = 0; i < 20; i++) swiIntrWait(0, 1);
		showProgressIcon = true;
		controlTopBright = false;
		if (access(dsiWarePubPath.c_str(), F_OK) == 0) {
			fcopy("sd:/_nds/TWiLightMenu/tempDSiWare.pub", dsiWarePubPath.c_str());
		}
		if (access(dsiWarePrvPath.c_str(), F_OK) == 0) {
			fcopy("sd:/_nds/TWiLightMenu/tempDSiWare.prv", dsiWarePrvPath.c_str());
		}
		showProgressIcon = false;
		fadeType = false;	// Fade to white
		for (int i = 0; i < 30; i++) swiIntrWait(0, 1);
		clearText(false);
	}

	while(1) {

		snprintf (path, sizeof(path), "%s", romfolder[secondaryDevice].c_str());
		// Set directory
		chdir (path);

		//Navigates to the file to launch
		filename = browseForFile(extensionList, username);

		////////////////////////////////////
		// Launch the item

		if (applaunch) {
			// Clear screen with white
			whiteScreen = true;
			clearText();
			clearBmpScreen();

			// Delete previously used DSiWare of flashcard from SD
			if (!gotosettings && consoleModel < 2 && previousUsedDevice && bothSDandFlashcard()) {
				if (access("sd:/_nds/TWiLightMenu/tempDSiWare.dsi", F_OK) == 0) {
					remove("sd:/_nds/TWiLightMenu/tempDSiWare.dsi");
				}
				if (access("sd:/_nds/TWiLightMenu/tempDSiWare.pub", F_OK) == 0) {
					remove("sd:/_nds/TWiLightMenu/tempDSiWare.pub");
				}
				if (access("sd:/_nds/TWiLightMenu/tempDSiWare.prv", F_OK) == 0) {
					remove("sd:/_nds/TWiLightMenu/tempDSiWare.prv");
				}
			}

			// Construct a command line
			getcwd (filePath, PATH_MAX);
			int pathLen = strlen(filePath);
			vector<char*> argarray;

			// Launch DSiWare via launcharg
			if ((strcasecmp (filename.c_str() + filename.size() - 10, ".launcharg") == 0)
			|| (strcasecmp (filename.c_str() + filename.size() - 10, ".LAUNCHARG") == 0))
			{
				FILE *argfile = fopen(filename.c_str(),"rb");
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
				filename = argarray.at(0);

				launchType = 0;	// No launch type for launcharg
				previousUsedDevice = secondaryDevice;
				SaveSettings();

				char appPath[256];
				for (u8 appVer = 0; appVer <= 0xFF; appVer++)
				{
					if (appVer > 0xF) {
						snprintf(appPath, sizeof(appPath), "%scontent/000000%x.app", filename.c_str(), appVer);
					} else {
						snprintf(appPath, sizeof(appPath), "%scontent/0000000%x.app", filename.c_str(), appVer);
					}
					if (access(appPath, F_OK) == 0)
					{
						break;
					}
				}

				sNDSHeaderExt NDSHeader;

				FILE *f_nds_file = fopen(appPath, "rb");

				fread(&NDSHeader, 1, sizeof(NDSHeader), f_nds_file);
				fclose(f_nds_file);

				*(u32*)(0x02000300) = 0x434E4C54;	// Set "CNLT" warmboot flag
				*(u16*)(0x02000304) = 0x1801;
				*(u32*)(0x02000308) = NDSHeader.dsi_tid;
				*(u32*)(0x0200030C) = NDSHeader.dsi_tid2;
				*(u32*)(0x02000310) = NDSHeader.dsi_tid;
				*(u32*)(0x02000314) = NDSHeader.dsi_tid2;
				*(u32*)(0x02000318) = 0x00000017;
				*(u32*)(0x0200031C) = 0x00000000;
				while (*(u16*)(0x02000306) == 0x0000) {	// Keep running, so that CRC16 isn't 0
					*(u16*)(0x02000306) = swiCRC16(0xFFFF, (void*)0x02000308, 0x18);
				}

				unlaunchSetHiyaBoot();

				fifoSendValue32(FIFO_USER_02, 1);	// Reboot into DSiWare title, booted via Launcher
				for (int i = 0; i < 15; i++) swiIntrWait(0, 1);
			}

			if ((strcasecmp (filename.c_str() + filename.size() - 5, ".argv") == 0)
			|| (strcasecmp (filename.c_str() + filename.size() - 5, ".ARGV") == 0))
			{
				FILE *argfile = fopen(filename.c_str(),"rb");
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
				filename = argarray.at(0);
			} else {
				argarray.push_back(strdup(filename.c_str()));
			}

			// Launch DSiWare .nds via Unlaunch
			if (isDSiMode() && isDSiWare[cursorPosition[secondaryDevice]]) {
				const char *typeToReplace = ".nds";
				if (strcasecmp (filename.c_str() + filename.size() - 4, ".dsi") == 0) {
					typeToReplace = ".dsi";
				} else if (strcasecmp (filename.c_str() + filename.size() - 4, ".ids") == 0) {
					typeToReplace = ".ids";
				}

				char *name = argarray.at(0);
				strcpy (filePath + pathLen, name);
				free(argarray.at(0));
				argarray.at(0) = filePath;

				dsiWareSrlPath = argarray[0];
				dsiWarePubPath = ReplaceAll(argarray[0], typeToReplace, ".pub");
				dsiWarePrvPath = ReplaceAll(argarray[0], typeToReplace, ".prv");
				launchType = 2;
				previousUsedDevice = secondaryDevice;
				SaveSettings();

				sNDSHeaderExt NDSHeader;

				FILE *f_nds_file = fopen(filename.c_str(), "rb");

				fread(&NDSHeader, 1, sizeof(NDSHeader), f_nds_file);
				fclose(f_nds_file);

				fadeSpeed = true;	// Fast fading

				if ((access(dsiWarePubPath.c_str(), F_OK) != 0) && (NDSHeader.pubSavSize > 0)) {
					const char* savecreate = "Creating public save file...";
					const char* savecreated = "Public save file created!";
					clearText();
					printLarge(false, 4, 4, savecreate);
					if (!fadeType) {
						fadeType = true;	// Fade in from white
						for (int i = 0; i < 35; i++) swiIntrWait(0, 1);
					}

					static const int BUFFER_SIZE = 4096;
					char buffer[BUFFER_SIZE];
					memset(buffer, 0, sizeof(buffer));

					FILE *pFile = fopen(dsiWarePubPath.c_str(), "wb");
					if (pFile) {
						for (int i = NDSHeader.pubSavSize; i > 0; i -= BUFFER_SIZE) {
							fwrite(buffer, 1, sizeof(buffer), pFile);
						}
						fclose(pFile);
					}
					printLarge(false, 4, 20, savecreated);
					for (int i = 0; i < 60; i++) swiIntrWait(0, 1);
				}

				if ((access(dsiWarePrvPath.c_str(), F_OK) != 0) && (NDSHeader.prvSavSize > 0)) {
					const char* savecreate = "Creating private save file...";
					const char* savecreated = "Private save file created!";
					clearText();
					printLarge(false, 4, 4, savecreate);
					if (!fadeType) {
						fadeType = true;	// Fade in from white
						for (int i = 0; i < 35; i++) swiIntrWait(0, 1);
					}

					static const int BUFFER_SIZE = 4096;
					char buffer[BUFFER_SIZE];
					memset(buffer, 0, sizeof(buffer));

					FILE *pFile = fopen(dsiWarePrvPath.c_str(), "wb");
					if (pFile) {
						for (int i = NDSHeader.prvSavSize; i > 0; i -= BUFFER_SIZE) {
							fwrite(buffer, 1, sizeof(buffer), pFile);
						}
						fclose(pFile);
					}
					printLarge(false, 4, 20, savecreated);
					for (int i = 0; i < 60; i++) swiIntrWait(0, 1);
				}

				if (fadeType) {
					fadeType = false;	// Fade to white
					for (int i = 0; i < 25; i++) swiIntrWait(0, 1);
				}

				if (secondaryDevice) {
					clearText();
					printLargeCentered(false, 88, "Now copying data...");
					printSmallCentered(false, 104, "Do not turn off the power.");
					fadeType = true;	// Fade in from white
					for (int i = 0; i < 35; i++) swiIntrWait(0, 1);
					showProgressIcon = true;
					fcopy(dsiWareSrlPath.c_str(), "sd:/_nds/TWiLightMenu/tempDSiWare.dsi");
					if (access(dsiWarePubPath.c_str(), F_OK) == 0) {
						fcopy(dsiWarePubPath.c_str(), "sd:/_nds/TWiLightMenu/tempDSiWare.pub");
					}
					if (access(dsiWarePrvPath.c_str(), F_OK) == 0) {
						fcopy(dsiWarePrvPath.c_str(), "sd:/_nds/TWiLightMenu/tempDSiWare.prv");
					}
					showProgressIcon = false;
					fadeType = false;	// Fade to white
					for (int i = 0; i < 25; i++) swiIntrWait(0, 1);

					if (access(dsiWarePubPath.c_str(), F_OK) == 0 || access(dsiWarePrvPath.c_str(), F_OK) == 0) {
						clearText();
						printLarge(false, 4, 64, "After saving, please re-start");
						if (appName == 0) {
							printLarge(false, 4, 80, "TWiLight Menu++ to transfer your");
						} else if (appName == 1) {
							printLarge(false, 4, 80, "SRLoader to transfer your");
						} else if (appName == 2) {
							printLarge(false, 4, 80, "DSiMenu++ to transfer your");
						}
						printLarge(false, 4, 96, "save data back.");
						fadeType = true;	// Fade in from white
						for (int i = 0; i < 60*3; i++) swiIntrWait(0, 1);		// Wait 3 seconds
						fadeType = false;	// Fade to white
						for (int i = 0; i < 25; i++) swiIntrWait(0, 1);
					}
				}

				char unlaunchDevicePath[256];
				if (secondaryDevice) {
					snprintf(unlaunchDevicePath, sizeof(unlaunchDevicePath), "sdmc:/_nds/TWiLightMenu/tempDSiWare.dsi");
				} else {
					snprintf(unlaunchDevicePath, sizeof(unlaunchDevicePath), "__%s", dsiWareSrlPath.c_str());
					unlaunchDevicePath[0] = 's';
					unlaunchDevicePath[1] = 'd';
					unlaunchDevicePath[2] = 'm';
					unlaunchDevicePath[3] = 'c';
				}

				memcpy((u8*)0x02000800, unlaunchAutoLoadID, 12);
				*(u16*)(0x0200080C) = 0x3F0;		// Unlaunch Length for CRC16 (fixed, must be 3F0h)
				*(u16*)(0x0200080E) = 0;			// Unlaunch CRC16 (empty)
				*(u32*)(0x02000810) = 0;			// Unlaunch Flags
				*(u32*)(0x02000810) |= BIT(0);		// Load the title at 2000838h
				*(u32*)(0x02000810) |= BIT(1);		// Use colors 2000814h
				*(u16*)(0x02000814) = 0x7FFF;		// Unlaunch Upper screen BG color (0..7FFFh)
				*(u16*)(0x02000816) = 0x7FFF;		// Unlaunch Lower screen BG color (0..7FFFh)
				memset((u8*)0x02000818, 0, 0x20+0x208+0x1C0);		// Unlaunch Reserved (zero)
				int i2 = 0;
				for (int i = 0; i < (int)sizeof(unlaunchDevicePath); i++) {
					*(u8*)(0x02000838+i2) = unlaunchDevicePath[i];		// Unlaunch Device:/Path/Filename.ext (16bit Unicode,end by 0000h)
					i2 += 2;
				}
				while (*(u16*)(0x0200080E) == 0) {	// Keep running, so that CRC16 isn't 0
					*(u16*)(0x0200080E) = swiCRC16(0xFFFF, (void*)0x02000810, 0x3F0);		// Unlaunch CRC16
				}

				fifoSendValue32(FIFO_USER_02, 1);	// Reboot into DSiWare title, booted via Unlaunch
				for (int i = 0; i < 15; i++) swiIntrWait(0, 1);
			}

			// Launch .nds directly or via nds-bootstrap
			if ((strcasecmp (filename.c_str() + filename.size() - 4, ".nds") == 0)
			|| (strcasecmp (filename.c_str() + filename.size() - 4, ".NDS") == 0)
			|| (strcasecmp (filename.c_str() + filename.size() - 4, ".dsi") == 0)
			|| (strcasecmp (filename.c_str() + filename.size() - 4, ".DSI") == 0)
			|| (strcasecmp (filename.c_str() + filename.size() - 4, ".ids") == 0)
			|| (strcasecmp (filename.c_str() + filename.size() - 4, ".IDS") == 0)) {
				bool dsModeSwitch = false;
				if (isHomebrew[cursorPosition[secondaryDevice]] == 2) {
					useBootstrap = false;	// Bypass nds-bootstrap
					homebrewBootstrap = true;
				} else if (isHomebrew[cursorPosition[secondaryDevice]] == 1) {
					loadPerGameSettings(filename);
					if (perGameSettings_directBoot) {
						useBootstrap = false;	// Bypass nds-bootstrap
					} else {
						useBootstrap = true;
					}
					if (isDSiMode() && !perGameSettings_dsiMode) {
						dsModeSwitch = true;
					}
					homebrewBootstrap = true;
				} else {
					useBootstrap = true;
					homebrewBootstrap = false;
				}

				char *name = argarray.at(0);
				strcpy (filePath + pathLen, name);
				free(argarray.at(0));
				argarray.at(0) = filePath;
				if(useBootstrap) {
					if(!secondaryDevice) {
						char game_TID[5];
                        char  gameid[4]; // for nitrohax cheat parsing
                        u32 ndsHeader[0x80];
                        uint32_t headerCRC;

						FILE *f_nds_file = fopen(argarray[0], "rb");

						fseek(f_nds_file, offsetof(sNDSHeadertitlecodeonly, gameCode), SEEK_SET);
						fread(game_TID, 1, 4, f_nds_file);
						game_TID[4] = 0;
						game_TID[3] = 0;
                        
                        fseek(f_nds_file, offsetof(sNDSHeadertitlecodeonly, gameCode), SEEK_SET);
                        int sizeread = fread(gameid, 1, 4, f_nds_file);
                        
                        #ifdef DEBUG
                        char * sizereads;
                        sprintf (sizereads, "%08X", sizeread);
                        nocashMessage("gameid readed");
                        nocashMessage(sizereads);
                        #endif
                        
                        #ifdef DEBUG
                        char* gameIdS;
                        sprintf (gameIdS, "%c%c%c%c", gameid[0], gameid[1], gameid[2], gameid[3]);
                        nocashMessage("gameId");
                        nocashMessage(gameIdS);
                        #endif
                        
                        fseek(f_nds_file, 0, SEEK_SET);
                        sizeread = fread(ndsHeader, 1, 0x80*4, f_nds_file);
                        
                        #ifdef DEBUG
                        sprintf (sizereads, "%08X", sizeread);                        
                        nocashMessage("ndsHeader readed");
                        nocashMessage(sizereads);
                        #endif
                        
                        headerCRC = crc32((const char*)ndsHeader, sizeof(ndsHeader));

                        #ifdef DEBUG    
                        char * headerCrcS;
                        sprintf (headerCrcS, "%08X", headerCRC);                  
                        nocashMessage("headerCRC computed");
                        nocashMessage(headerCrcS);
                        #endif
                         
						fclose(f_nds_file);

						std::string savename = ReplaceAll(argarray[0], ".nds", ".sav");

						if (access(savename.c_str(), F_OK) && isHomebrew[cursorPosition[secondaryDevice]] == 0) {		// Create save if game isn't homebrew
							ClearBrightness();
							const char* savecreate = "Creating save file...";
							const char* savecreated = "Save file created!";
							printLarge(false, 4, 4, savecreate);

							static const int BUFFER_SIZE = 4096;
							char buffer[BUFFER_SIZE];
							memset(buffer, 0, sizeof(buffer));

							int savesize = 524288;	// 512KB (default size for most games)

							// Set save size to 8KB for the following games
							if (strcmp(game_TID, "ASC") == 0 )	// Sonic Rush
							{
								savesize = 8192;
							}

							// Set save size to 256KB for the following games
							if (strcmp(game_TID, "AMH") == 0 )	// Metroid Prime Hunters
							{
								savesize = 262144;
							}

							// Set save size to 1MB for the following games
							if ( strcmp(game_TID, "AZL") == 0		// Wagamama Fashion: Girls Mode/Style Savvy/Nintendo presents: Style Boutique/Namanui Collection: Girls Style
								|| strcmp(game_TID, "BKI") == 0 )	// The Legend of Zelda: Spirit Tracks
							{
								savesize = 1048576;
							}

							// Set save size to 32MB for the following games
							if (strcmp(game_TID, "UOR") == 0 )	// WarioWare - D.I.Y. (Do It Yourself)
							{
								savesize = 1048576*32;
							}

							FILE *pFile = fopen(savename.c_str(), "wb");
							if (pFile) {
								for (int i = savesize; i > 0; i -= BUFFER_SIZE) {
									fwrite(buffer, 1, sizeof(buffer), pFile);
								}
								fclose(pFile);
							}
							printLarge(false, 4, 20, savecreated);
						}

						SetDonorSDK(argarray[0]);
						SetGameSoftReset(argarray[0]);
						SetMPUSettings(argarray[0]);

						std::string path = argarray[0];
						std::string savepath = savename;
						CIniFile bootstrapini( "sd:/_nds/nds-bootstrap.ini" );
						bootstrapini.SetString("NDS-BOOTSTRAP", "NDS_PATH", path);
						bootstrapini.SetString("NDS-BOOTSTRAP", "SAV_PATH", savepath);
						loadPerGameSettings(filename);
						if (perGameSettings_language == -2) {
							bootstrapini.SetInt( "NDS-BOOTSTRAP", "LANGUAGE", bstrap_language);
						} else {
							bootstrapini.SetInt( "NDS-BOOTSTRAP", "LANGUAGE", perGameSettings_language);
						}
						if (perGameSettings_dsiMode == -1) {
							bootstrapini.SetInt( "NDS-BOOTSTRAP", "DSI_MODE", bstrap_dsiMode);
						} else {
							bootstrapini.SetInt( "NDS-BOOTSTRAP", "DSI_MODE", perGameSettings_dsiMode);
						}
						if (perGameSettings_boostCpu == -1) {
							bootstrapini.SetInt( "NDS-BOOTSTRAP", "BOOST_CPU", boostCpu);
						} else {
							bootstrapini.SetInt( "NDS-BOOTSTRAP", "BOOST_CPU", perGameSettings_boostCpu);
						}
						if (perGameSettings_boostVram == -1) {
							bootstrapini.SetInt( "NDS-BOOTSTRAP", "BOOST_VRAM", boostVram);
						} else {
							bootstrapini.SetInt( "NDS-BOOTSTRAP", "BOOST_VRAM", perGameSettings_boostVram);
						}
						if (perGameSettings_soundFix == -1) {
							bootstrapini.SetInt( "NDS-BOOTSTRAP", "SOUND_FIX", soundFix);
						} else {
							bootstrapini.SetInt( "NDS-BOOTSTRAP", "SOUND_FIX", perGameSettings_soundFix);
						}
						bootstrapini.SetInt( "NDS-BOOTSTRAP", "DONOR_SDK_VER", donorSdkVer);
						bootstrapini.SetInt( "NDS-BOOTSTRAP", "GAME_SOFT_RESET", gameSoftReset);
						bootstrapini.SetInt( "NDS-BOOTSTRAP", "PATCH_MPU_REGION", mpuregion);
						bootstrapini.SetInt( "NDS-BOOTSTRAP", "PATCH_MPU_SIZE", mpusize);
						// Read cheats
						std::string cheatpath = "sd:/_nds/cheats.xml";
                        int c;
	                    FILE* cheatFile;
                        bool doFilter=true;
                        bool cheatsFound = false;
                        char cheatData[2305]; // 9*256 + 1

						if ((!access(cheatpath.c_str(), F_OK)) && isHomebrew[cursorPosition[secondaryDevice]] == 0) {
                            cheatData[0] = 0;
                            cheatData[2304] = 0;

                            nocashMessage("cheat file present");
                            
                            cheatFile = fopen (cheatpath.c_str(), "rb");
		                    if (NULL != cheatFile)  {
                                c = fgetc(cheatFile);
                                
                                if(c != 0xFF && c != 0xFE) {                            
                                    fseek (cheatFile, 0, SEEK_SET);
                                    
                                    CheatCodelist* codelist = new CheatCodelist();
                                    
                                    nocashMessage("parsing cheat file");
                                    
                                    if(codelist->load(cheatFile, gameid, headerCRC, doFilter)) {
                                        nocashMessage("cheat file parsed");
                                    	CheatFolder *gameCodes = codelist->getGame (gameid, headerCRC);
                                        
                                        if(!codelist->getContents().empty()) {
                                           nocashMessage("cheats found");
                                           gameCodes->enableAll(true);
                                           nocashMessage("all cheats enabled");
                                           std::list<CheatWord> cheatsword = gameCodes->getEnabledCodeData();
                                           nocashMessage("cheatword list recovered");
                                           int count =0;
                                           for (std::list<CheatWord>::iterator it=cheatsword.begin(); it != cheatsword.end(); ++it) {
                                                #ifdef DEBUG
                                                char * cheatwords;
                                                sprintf (cheatwords,"%08X",*it);
                                                nocashMessage("cheatword");
                                                nocashMessage(cheatwords);
                                                #endif
                                                if(!cheatsFound) snprintf (cheatData, sizeof(cheatData), "%08X ", *it);                                               
                                                else snprintf (cheatData, sizeof(cheatData), "%s%08X ", cheatData, *it);
                                                cheatsFound = true;
                                                #ifdef DEBUG
                                                nocashMessage("cheatData");
                                                nocashMessage(cheatData);
                                                #endif
                                                count++;
                                           }
                                           
                                           if(!cheatsFound) {
                                                //ClearBrightness();
                								const char* error = "no cheat found\n";
                                                nocashMessage(error);
                								//printLarge(false, 4, 4, error);
                                            }
                                            if(count>256) {
                                                const char* error = "maximum cheat data size exceeded\n";
                                                nocashMessage(error);
                                                cheatsFound = false;    
                                            }
                                        } else {
                                            //ClearBrightness();
            								const char* error = "Cheat list empty\n";
                                            nocashMessage(error);
            								//printLarge(false, 4, 4, error);
                                        }
                                    } else {
                                        //ClearBrightness();
        								const char* error = "Can't read cheat list\n";
                                        nocashMessage(error);
        								//printLarge(false, 4, 4, error);
                                    }
                                } else {
                                    //ClearBrightness();
    								const char* error = "cheats.xml File is in an unsupported unicode encoding";
                                    nocashMessage(error);
    								//printLarge(false, 4, 4, error);
                                }
                                fclose(cheatFile);
                            } 
						} else {                            
                            nocashMessage("cheat file not present");
                        }
                        if (cheatsFound) bootstrapini.SetString("NDS-BOOTSTRAP", "CHEAT_DATA", cheatData);
                        else bootstrapini.SetString("NDS-BOOTSTRAP", "CHEAT_DATA", "");
						bootstrapini.SaveIniFile( "sd:/_nds/nds-bootstrap.ini" );
						if (homebrewBootstrap) {
							bootstrapfilename = (bootstrapFile ? "sd:/_nds/nds-bootstrap-hb-nightly.nds" : "sd:/_nds/nds-bootstrap-hb-release.nds");
						} else {
							bootstrapfilename = (bootstrapFile ? "sd:/_nds/nds-bootstrap-nightly.nds" : "sd:/_nds/nds-bootstrap-release.nds");
						}
						launchType = 1;
						previousUsedDevice = secondaryDevice;
						SaveSettings();
						int err = runNdsFile (bootstrapfilename.c_str(), 0, NULL, true, false, true, true);
						char text[32];
						snprintf (text, sizeof(text), "Start failed. Error %i", err);
						clearText();
						ClearBrightness();
						printLarge(false, 4, 4, text);
						if (err == 1) {
							printLarge(false, 4, 20, "nds-bootstrap not found.");
						}
						stop();
					} else {
						launchType = 1;
						previousUsedDevice = secondaryDevice;
						SaveSettings();
						loadGameOnFlashcard(argarray[0], filename, true);
					}
				} else {
					launchType = 1;
					previousUsedDevice = secondaryDevice;
					SaveSettings();
					bool runNds_boostCpu = false;
					bool runNds_boostVram = false;
					if (isDSiMode()) {
						loadPerGameSettings(filename);
						if (perGameSettings_boostCpu == -1) {
							runNds_boostCpu = boostCpu;
						} else {
							runNds_boostCpu = perGameSettings_boostCpu;
						}
						if (perGameSettings_boostVram == -1) {
							runNds_boostVram = boostVram;
						} else {
							runNds_boostVram = perGameSettings_boostVram;
						}
					}
					//iprintf ("Running %s with %d parameters\n", argarray[0], argarray.size());
					int err = runNdsFile (argarray[0], argarray.size(), (const char **)&argarray[0], true, dsModeSwitch, runNds_boostCpu, runNds_boostVram);
					char text[32];
					snprintf (text, sizeof(text), "Start failed. Error %i", err);
					ClearBrightness();
					printLarge(false, 4, 4, text);
					stop();
				}
			} else if ((strcasecmp (filename.c_str() + filename.size() - 3, ".gb") == 0)
					|| (strcasecmp (filename.c_str() + filename.size() - 4, ".GB") == 0)
					|| (strcasecmp (filename.c_str() + filename.size() - 4, ".sgb") == 0)
					|| (strcasecmp (filename.c_str() + filename.size() - 4, ".SGB") == 0)
					|| (strcasecmp (filename.c_str() + filename.size() - 4, ".gbc") == 0)
					|| (strcasecmp (filename.c_str() + filename.size() - 4, ".GBC") == 0))
			{
				char gbROMpath[256];
				snprintf (gbROMpath, sizeof(gbROMpath), "%s/%s", romfolder[secondaryDevice].c_str(), filename.c_str());
				homebrewArg = gbROMpath;
				launchType = 4;
				previousUsedDevice = secondaryDevice;
				SaveSettings();
				argarray.push_back(gbROMpath);
				int err = 0;
				if(secondaryDevice) {
					argarray.at(0) = "/_nds/TWiLightMenu/emulators/gameyob.nds";
					err = runNdsFile ("/_nds/TWiLightMenu/emulators/gameyob.nds", argarray.size(), (const char **)&argarray[0], true, false, true, true);	// Pass ROM to GameYob as argument
				} else {
					argarray.at(0) = "sd:/_nds/TWiLightMenu/emulators/gameyob.nds";
					err = runNdsFile ("sd:/_nds/TWiLightMenu/emulators/gameyob.nds", argarray.size(), (const char **)&argarray[0], true, false, true, true);	// Pass ROM to GameYob as argument
				}
				char text[32];
				snprintf (text, sizeof(text), "Start failed. Error %i", err);
				ClearBrightness();
				printLarge(false, 4, 4, text);
				stop();
			} else if ((strcasecmp (filename.c_str() + filename.size() - 4, ".nes") == 0)
					|| (strcasecmp (filename.c_str() + filename.size() - 4, ".NDS") == 0)
					|| (strcasecmp (filename.c_str() + filename.size() - 4, ".fds") == 0)
					|| (strcasecmp (filename.c_str() + filename.size() - 4, ".FDS") == 0))
			{
				char nesROMpath[256];
				snprintf (nesROMpath, sizeof(nesROMpath), "%s/%s", romfolder[secondaryDevice].c_str(), filename.c_str());
				homebrewArg = nesROMpath;
				launchType = 3;
				previousUsedDevice = secondaryDevice;
				SaveSettings();
				argarray.push_back(nesROMpath);
				int err = 0;
				if(secondaryDevice) {
					argarray.at(0) = "/_nds/TWiLightMenu/emulators/nesds.nds";
					err = runNdsFile ("/_nds/TWiLightMenu/emulators/nesds.nds", argarray.size(), (const char **)&argarray[0], true, false, true, true);	// Pass ROM to nesDS as argument
				} else {
					argarray.at(0) = "sd:/_nds/TWiLightMenu/emulators/nestwl.nds";
					err = runNdsFile ("sd:/_nds/TWiLightMenu/emulators/nestwl.nds", argarray.size(), (const char **)&argarray[0], true, false, true, true);	// Pass ROM to nesDS as argument
				}
				char text[32];
				snprintf (text, sizeof(text), "Start failed. Error %i", err);
				ClearBrightness();
				printLarge(false, 4, 4, text);
				stop();
			}

			while(argarray.size() !=0 ) {
				free(argarray.at(0));
				argarray.erase(argarray.begin());
			}

			// while (1) {
			// 	swiIntrWait(0, 1);
			// 	scanKeys();
			// 	if (!(keysHeld() & KEY_A)) break;
			// }
		}
	}

	return 0;
}
