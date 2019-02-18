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
int colorMode = 0;
int blfLevel = 0;

extern void ClearBrightness();
extern bool showProgressIcon;

const char* settingsinipath = "sd:/_nds/TWiLightMenu/settings.ini";
const char* bootstrapinipath = "sd:/_nds/nds-bootstrap.ini";

std::string romPath;
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

/**
 * Remove trailing slashes from a pathname, if present.
 * @param path Pathname to modify.
 */
void RemoveTrailingSlashes(std::string& path)
{
	while (!path.empty() && path[path.size()-1] == '/') {
		path.resize(path.size()-1);
	}
}

/**
 * Remove trailing spaces from a cheat code line, if present.
 * @param path Code line to modify.
 */
/*static void RemoveTrailingSpaces(std::string& code)
{
	while (!code.empty() && code[code.size()-1] == ' ') {
		code.resize(code.size()-1);
	}
}*/

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
bool ceCached = false;

bool applaunch = false;
bool startMenu = false;
bool gotosettings = false;

int launchType = 1;	// 0 = Slot-1, 1 = SD/Flash card, 2 = DSiWare, 3 = NES, 4 = (S)GB(C), 5 = SMS/GG
bool slot1LaunchMethod = true;	// false == Reboot, true == Direct
bool useBootstrap = true;
bool bootstrapFile = false;
bool homebrewBootstrap = false;
bool snesEmulator = true;

bool gbaBiosFound[2] = {false};
bool useGbarunner = false;
int theme = 0;
int subtheme = 0;
int cursorPosition[2] = {0};
int startMenu_cursorPosition = 0;
int pagenum[2] = {0};
bool showNds = true;
bool showNes = true;
bool showGb = true;
bool showSmsGg = true;
bool showMd = true;
bool showSnes = true;
bool showDirectories = true;
bool showHidden = false;
bool showBoxArt = true;
bool animateDsiIcons = false;
int launcherApp = -1;
int sysRegion = -1;

int guiLanguage = -1;
int bstrap_language = -1;
bool boostCpu = false;	// false == NTR, true == TWL
bool boostVram = false;
int bstrap_dsiMode = 0;

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

	showNds = settingsini.GetInt("SRLOADER", "SHOW_NDS", true);
	showNes = settingsini.GetInt("SRLOADER", "SHOW_NES", true);
	showGb = settingsini.GetInt("SRLOADER", "SHOW_GB", true);
	showSmsGg = settingsini.GetInt("SRLOADER", "SHOW_SMSGG", true);
	showMd = settingsini.GetInt("SRLOADER", "SHOW_MDGEN", true);
	showSnes = settingsini.GetInt("SRLOADER", "SHOW_SNES", true);

	// Customizable UI settings.
	colorMode = settingsini.GetInt("SRLOADER", "COLOR_MODE", 0);
	blfLevel = settingsini.GetInt("SRLOADER", "BLUE_LIGHT_FILTER_LEVEL", 0);
	guiLanguage = settingsini.GetInt("SRLOADER", "LANGUAGE", -1);
	useGbarunner = settingsini.GetInt("SRLOADER", "USE_GBARUNNER2", 0);
	if (!isRegularDS) useGbarunner = true;
	theme = settingsini.GetInt("SRLOADER", "THEME", 0);
	subtheme = settingsini.GetInt("SRLOADER", "SUB_THEME", 0);
	showDirectories = settingsini.GetInt("SRLOADER", "SHOW_DIRECTORIES", 1);
	showHidden = settingsini.GetInt("SRLOADER", "SHOW_HIDDEN", 0);
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
	if (!isDSiMode()) useBootstrap = settingsini.GetInt("SRLOADER", "USE_BOOTSTRAP", useBootstrap);
	bootstrapFile = settingsini.GetInt("SRLOADER", "BOOTSTRAP_FILE", 0);
	snesEmulator = settingsini.GetInt("SRLOADER", "SNES_EMULATOR", snesEmulator);

	// Default nds-bootstrap settings
	bstrap_language = settingsini.GetInt("NDS-BOOTSTRAP", "LANGUAGE", -1);
	boostCpu = settingsini.GetInt("NDS-BOOTSTRAP", "BOOST_CPU", 0);
	boostVram = settingsini.GetInt("NDS-BOOTSTRAP", "BOOST_VRAM", 0);
	bstrap_dsiMode = settingsini.GetInt("NDS-BOOTSTRAP", "DSI_MODE", 0);

	romPath = settingsini.GetString("SRLOADER", "ROM_PATH", romPath);
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
	if (bothSDandFlashcard()) {
		settingsini.SetInt("SRLOADER", "SECONDARY_DEVICE", secondaryDevice);
	}
	if (!gotosettings) {
		settingsini.SetInt("SRLOADER", "PREVIOUS_USED_DEVICE", previousUsedDevice);
		settingsini.SetString("SRLOADER", "ROM_PATH", romPath);
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

bool useBackend = false;

using namespace std;

bool dropDown = false;
bool redoDropDown = false;
bool showbubble = false;
bool showSTARTborder = false;
bool buttonArrowTouched[2] = {false};
bool scrollWindowTouched = false;

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
	int pressed = keysHeld();
	
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
	int pressed = keysHeld();
	
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

/**
 * Gives some games slight speed bump by moving nds-bootstrap's cardEngine_arm9 to cached memory region.
 */
void SetSpeedBump(const char* filename) {
	FILE *f_nds_file = fopen(filename, "rb");

	char game_TID[5];
	fseek(f_nds_file, offsetof(sNDSHeadertitlecodeonly, gameCode), SEEK_SET);
	fread(game_TID, 1, 4, f_nds_file);
	game_TID[4] = 0;
	game_TID[3] = 0;
	fclose(f_nds_file);

	scanKeys();
	int pressed = keysHeld();
	
	ceCached = false;

	static const char list[][4] = {
		"AYI",	// Yoshi Touch & Go
		"AZW",	// WarioWare: Touched
		"ARR",	// Ridge Racer DS
		"ASM",	// Super Mario 64 DS
		"SMS",	// Super Mario Star World, and Mario's Holiday
		"AS2",	// Spider-Man 2
		"ABM",	// Bomberman
		"ACV",	// Castlevania: Dawn of Sorrow
		"AGY",	// Phoenix Wright: Ace Attorney
		"A5T",	// MegaMan Battle Network 5: Double Team DS
		"ASC",	// Sonic Rush
		"ADM",	// Animal Crossing: Wild World
		"ABH",	// Resident Evil: Deadly Silence
		"ARJ",	// Kanji Sonomama Rakubiki Jiten
		//"ARZ",	// Rockman ZX/MegaMan ZX
		"AKW",	// Kirby Squeak Squad/Mouse Attack
		"ADA",	// Pokemon Diamond
		"APA",	// Pokemon Pearl
		"A8N",	// Planet Puzzle League
		"A6A",	// MegaMan Star Force: Pegasus
		"A6B",	// MegaMan Star Force: Leo
		"A6C",	// MegaMan Star Force: Dragon
		"A3Y",	// Sonic Rush Adventure
		//"YZX",	// Rockman ZX Advent/MegaMan ZX Advent
		"A8T",	// Mario Party DS
		"A2L",	// Anno 1701: Dawn of Discovery
		"A5F",	// Professor Layton and the Curious Village
		"AWL",	// The World Ends With You
		"YRV",	// MegaMan Star Force 2: Zerker x Ninja
		"YRW",	// MegaMan Star Force 2: Zerker x Saurian
		"YKW",	// Kirby Super Star Ultra
		"YGX",	// Grand Theft Auto: Chinatown Wars
		"CPU",	// Pokemon Platinum
		"CRB",	// MegaMan Star Force 3: Black Ace
		"CRR",	// MegaMan Star Force 3: Red Joker
		"B62",	// Chibi-Robo: Clean Sweep!
		"B6X",	// Rockman EXE: Operate Shooting Star
		"IPG",	// Pokemon SoulSilver
		"IPK",	// Pokemon HeartGold
		"B6Z",	// Rockman Zero Collection/MegaMan Zero Collection
	};

	// TODO: If the list gets large enough, switch to bsearch().
	for (unsigned int i = 0; i < sizeof(list)/sizeof(list[0]); i++) {
		if (memcmp(game_TID, list[i], 3) == 0) {
			// Found a match.
			ceCached = true;
			break;
		}
	}

	if(pressed & KEY_L){
		ceCached = true;
	}
}

//---------------------------------------------------------------------------------
void stop (void) {
//---------------------------------------------------------------------------------
	while (1) {
		swiWaitForVBlank();
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
		swiWaitForVBlank();
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
	for (int i = 0; i < 15; i++) swiWaitForVBlank();
}

//---------------------------------------------------------------------------------
int main(int argc, char **argv) {
//---------------------------------------------------------------------------------

	//overwrite reboot stub identifier
	
	extern u64 *fake_heap_end;
	*fake_heap_end = 0;

	defaultExceptionHandler();

	bool fatInited = fatInitDefault();

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

	if (!fatInited) {
		graphicsInit();
		fontInit();
		InitSound();
		if (!music) {
			mmEffectEx(&mus_menu);
			music = true;
		}
		whiteScreen = false;
		fadeType = true;
		for (int i = 0; i < 5; i++) swiWaitForVBlank();
		if (!dropDown && theme == 0) {
			dropDown = true;
			for (int i = 0; i < 72; i++) swiWaitForVBlank();
		} else {
			for (int i = 0; i < 25; i++) swiWaitForVBlank();
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
				swiWaitForVBlank();
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

	fifoWaitValue32(FIFO_USER_06);
	if (fifoGetValue32(FIFO_USER_03) == 0) arm7SCFGLocked = true;	// If DSiMenu++ is being run from DSiWarehax or flashcard, then arm7 SCFG is locked.
	u16 arm7_SNDEXCNT = fifoGetValue32(FIFO_USER_07);
	if (arm7_SNDEXCNT != 0) isRegularDS = false;	// If sound frequency setting is found, then the console is not a DS Phat/Lite
	fifoSendValue32(FIFO_USER_07, 0);

	LoadSettings();
	
	if (sdFound() && access("sd:/bios.bin", F_OK) == 0) {
		gbaBiosFound[0] = true;
	} else if (secondaryDevice && access("fat:/bios.bin", F_OK) == 0) {
		gbaBiosFound[1] = true;
	}

	graphicsInit();
	fontInit();
	iconManagerInit();

	keysSetRepeat(25, 5);

	vector<string> extensionList;
	if (showNds) {
		extensionList.push_back(".nds");
		extensionList.push_back(".dsi");
		extensionList.push_back(".ids");
		extensionList.push_back(".argv");
	}
	if (showGb) {
		extensionList.push_back(".gb");
		extensionList.push_back(".sgb");
		extensionList.push_back(".gbc");
	}
	if (showNes) {
		extensionList.push_back(".nes");
		extensionList.push_back(".fds");
	}
	if (showSmsGg) {
		extensionList.push_back(".sms");
		extensionList.push_back(".gg");
	}
	if (showMd) {
		extensionList.push_back(".gen");
	}
	if (showSnes) {
		extensionList.push_back(".smc");
		extensionList.push_back(".sfc");
	}
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

	if ((consoleModel < 2 && previousUsedDevice && bothSDandFlashcard() && launchType == 2 && access(dsiWarePubPath.c_str(), F_OK) == 0)
	|| (consoleModel < 2 && previousUsedDevice && bothSDandFlashcard() && launchType == 2 && access(dsiWarePrvPath.c_str(), F_OK) == 0))
	{
		fadeType = true;	// Fade in from white
		printLargeCentered(false, 88, "Now copying data...");
		printSmallCentered(false, 104, "Do not turn off the power.");
		for (int i = 0; i < 15; i++) swiWaitForVBlank();
		reloadFontPalettes();
		for (int i = 0; i < 20; i++) swiWaitForVBlank();
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
		for (int i = 0; i < 30; i++) swiWaitForVBlank();
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
			extern bool rocketVideo_playVideo;
			rocketVideo_playVideo = false;
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

			bool isArgv = false;
			if ((strcasecmp (filename.c_str() + filename.size() - 5, ".argv") == 0)
			|| (strcasecmp (filename.c_str() + filename.size() - 5, ".ARGV") == 0))
			{
				romPath = filePath+filename;

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
				isArgv = true;
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
				if (!isArgv) {
					romPath = argarray[0];
				}
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
						for (int i = 0; i < 35; i++) swiWaitForVBlank();
					}

					static const int BUFFER_SIZE = 4096;
					char buffer[BUFFER_SIZE];
					memset(buffer, 0, sizeof(buffer));
					bool bufferCleared = false;
					char savHdrPath[64];
					snprintf(savHdrPath, sizeof(savHdrPath), "nitro:/DSiWareSaveHeaders/%x.savhdr", (unsigned int)NDSHeader.pubSavSize);
					FILE *hdrFile = fopen(savHdrPath, "rb");
					if (hdrFile) fread(buffer, 1, 0x200, hdrFile);
					fclose(hdrFile);

					FILE *pFile = fopen(dsiWarePubPath.c_str(), "wb");
					if (pFile) {
						for (int i = NDSHeader.pubSavSize; i > 0; i -= BUFFER_SIZE) {
							fwrite(buffer, 1, sizeof(buffer), pFile);
							if (!bufferCleared) {
								memset(buffer, 0, sizeof(buffer));
								bufferCleared = true;
							}
						}
						fclose(pFile);
					}
					printLarge(false, 4, 20, savecreated);
					for (int i = 0; i < 60; i++) swiWaitForVBlank();
				}

				if ((access(dsiWarePrvPath.c_str(), F_OK) != 0) && (NDSHeader.prvSavSize > 0)) {
					const char* savecreate = "Creating private save file...";
					const char* savecreated = "Private save file created!";
					clearText();
					printLarge(false, 4, 4, savecreate);
					if (!fadeType) {
						fadeType = true;	// Fade in from white
						for (int i = 0; i < 35; i++) swiWaitForVBlank();
					}

					static const int BUFFER_SIZE = 4096;
					char buffer[BUFFER_SIZE];
					memset(buffer, 0, sizeof(buffer));
					bool bufferCleared = false;
					char savHdrPath[64];
					snprintf(savHdrPath, sizeof(savHdrPath), "nitro:/DSiWareSaveHeaders/%x.savhdr", (unsigned int)NDSHeader.prvSavSize);
					FILE *hdrFile = fopen(savHdrPath, "rb");
					if (hdrFile) fread(buffer, 1, 0x200, hdrFile);
					fclose(hdrFile);

					FILE *pFile = fopen(dsiWarePrvPath.c_str(), "wb");
					if (pFile) {
						for (int i = NDSHeader.prvSavSize; i > 0; i -= BUFFER_SIZE) {
							fwrite(buffer, 1, sizeof(buffer), pFile);
							if (!bufferCleared) {
								memset(buffer, 0, sizeof(buffer));
								bufferCleared = true;
							}
						}
						fclose(pFile);
					}
					printLarge(false, 4, 20, savecreated);
					for (int i = 0; i < 60; i++) swiWaitForVBlank();
				}

				if (fadeType) {
					fadeType = false;	// Fade to white
					for (int i = 0; i < 25; i++) swiWaitForVBlank();
				}

				if (secondaryDevice) {
					clearText();
					printLargeCentered(false, 88, "Now copying data...");
					printSmallCentered(false, 104, "Do not turn off the power.");
					fadeType = true;	// Fade in from white
					for (int i = 0; i < 35; i++) swiWaitForVBlank();
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
					for (int i = 0; i < 25; i++) swiWaitForVBlank();

					if (access(dsiWarePubPath.c_str(), F_OK) == 0 || access(dsiWarePrvPath.c_str(), F_OK) == 0) {
						clearText();
						printLarge(false, 4, 64, "After saving, please re-start");
						printLarge(false, 4, 80, "TWiLight Menu++ to transfer your");
						printLarge(false, 4, 96, "save data back.");
						fadeType = true;	// Fade in from white
						for (int i = 0; i < 60*3; i++) swiWaitForVBlank();		// Wait 3 seconds
						fadeType = false;	// Fade to white
						for (int i = 0; i < 25; i++) swiWaitForVBlank();
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
				for (int i = 0; i < 15; i++) swiWaitForVBlank();
			}

			// Launch .nds directly or via nds-bootstrap
			if ((strcasecmp (substr(filename.c_str() + filename.size() - 4), ".nds") == 0)
			 || (strcasecmp (substr(filename.c_str() + filename.size() - 4), ".dsi") == 0)
			 || (strcasecmp (substr(filename.c_str() + filename.size() - 4), ".ids") == 0))
			{
				bool dsModeSwitch = false;
				bool dsModeDSiWare = false;

				char game_TID[5];

				FILE *f_nds_file = fopen(argarray[0], "rb");

				fseek(f_nds_file, offsetof(sNDSHeadertitlecodeonly, gameCode), SEEK_SET);
				fread(game_TID, 1, 4, f_nds_file);
				game_TID[4] = 0;
				game_TID[3] = 0;
				fclose(f_nds_file);

				if (strcmp(game_TID, "HND") == 0 || strcmp(game_TID, "HNE") == 0) {
					dsModeSwitch = true;
					dsModeDSiWare = true;
					useBackend = false;	// Bypass nds-bootstrap
					homebrewBootstrap = true;
				} else if (isHomebrew[cursorPosition[secondaryDevice]] == 2) {
					useBackend = false;	// Bypass nds-bootstrap
					homebrewBootstrap = true;
				} else if (isHomebrew[cursorPosition[secondaryDevice]] == 1) {
					loadPerGameSettings(filename);
					if (perGameSettings_directBoot || (useBootstrap && secondaryDevice)) {
						useBackend = false;	// Bypass nds-bootstrap
					} else {
						useBackend = true;
					}
					if (isDSiMode() && !perGameSettings_dsiMode) {
						dsModeSwitch = true;
					}
					homebrewBootstrap = true;
				} else {
					loadPerGameSettings(filename);
					useBackend = true;
					homebrewBootstrap = false;
				}

				char *name = argarray.at(0);
				strcpy (filePath + pathLen, name);
				free(argarray.at(0));
				argarray.at(0) = filePath;
				if(useBackend) {
					if(useBootstrap || isDSiMode()) {
						if (secondaryDevice && (access("fat:/BTSTRP.TMP", F_OK) != 0)) {
							// Create temporary file for nds-bootstrap
							ClearBrightness();
							printLarge(false, 4, 4, "Creating \"BTSTRP.TMP\"...");

							static const int BUFFER_SIZE = 4096;
							char buffer[BUFFER_SIZE];
							memset(buffer, 0, sizeof(buffer));

							u32 fileSize = 0x40000;	// 256KB
							FILE *pFile = fopen("fat:/BTSTRP.TMP", "wb");
							if (pFile) {
								for (u32 i = fileSize; i > 0; i -= BUFFER_SIZE) {
									fwrite(buffer, 1, sizeof(buffer), pFile);
								}
								fclose(pFile);
							}
							printLarge(false, 4, 20, "Done!");
							for (int i = 0; i < 30; i++) swiWaitForVBlank();
							clearText();
						}

						std::string path = argarray[0];
						std::string savename = ReplaceAll(filename, ".nds", getSavExtension());
						std::string ramdiskname = ReplaceAll(filename, ".nds", getImgExtension(perGameSettings_ramDiskNo));
						std::string romFolderNoSlash = romfolder[secondaryDevice];
						RemoveTrailingSlashes(romFolderNoSlash);
						mkdir ((isHomebrew[cursorPosition[secondaryDevice]] == 1) ? "ramdisks" : "saves", 0777);
						std::string savepath = romFolderNoSlash+"/saves/"+savename;
						std::string ramdiskpath = romFolderNoSlash+"/ramdisks/"+ramdiskname;

						if (access(savepath.c_str(), F_OK) != 0 && isHomebrew[cursorPosition[secondaryDevice]] == 0) {		// Create save if game isn't homebrew
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

							FILE *pFile = fopen(savepath.c_str(), "wb");
							if (pFile) {
								for (int i = savesize; i > 0; i -= BUFFER_SIZE) {
									fwrite(buffer, 1, sizeof(buffer), pFile);
								}
								fclose(pFile);
							}
							printLarge(false, 4, 20, savecreated);
							for (int i = 0; i < 30; i++) swiWaitForVBlank();
						}

						SetDonorSDK(argarray[0]);
						SetGameSoftReset(argarray[0]);
						SetMPUSettings(argarray[0]);
						SetSpeedBump(argarray[0]);

						if (sdFound() && secondaryDevice) {
							fcopy("sd:/_nds/nds-bootstrap.ini", "fat:/_nds/nds-bootstrap.ini");		// Sync nds-bootstrap SD settings to flashcard
						}
						bootstrapinipath = (secondaryDevice ? "fat:/_nds/nds-bootstrap.ini" : "sd:/_nds/nds-bootstrap.ini");
						CIniFile bootstrapini( bootstrapinipath );
						bootstrapini.SetString("NDS-BOOTSTRAP", "NDS_PATH", path);
						bootstrapini.SetString("NDS-BOOTSTRAP", "SAV_PATH", savepath);
						bootstrapini.SetString("NDS-BOOTSTRAP", "RAM_DRIVE_PATH", (perGameSettings_ramDiskNo >= 0 && !secondaryDevice) ? ramdiskpath : "sd:/null.img");
						if (perGameSettings_language == -2) {
							bootstrapini.SetInt( "NDS-BOOTSTRAP", "LANGUAGE", bstrap_language);
						} else {
							bootstrapini.SetInt( "NDS-BOOTSTRAP", "LANGUAGE", perGameSettings_language);
						}
						if (isDSiMode()) {
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
						}
						bootstrapini.SetInt( "NDS-BOOTSTRAP", "DONOR_SDK_VER", donorSdkVer);
						bootstrapini.SetInt( "NDS-BOOTSTRAP", "GAME_SOFT_RESET", gameSoftReset);
						bootstrapini.SetInt( "NDS-BOOTSTRAP", "PATCH_MPU_REGION", mpuregion);
						bootstrapini.SetInt( "NDS-BOOTSTRAP", "PATCH_MPU_SIZE", mpusize);
						bootstrapini.SetInt( "NDS-BOOTSTRAP", "CARDENGINE_CACHED", ceCached);
						if (memcmp(io_dldi_data->friendlyName, "R4iDSN", 6) == 0 && !isRegularDS) {
							bootstrapini.SetInt( "NDS-BOOTSTRAP", "FORCE_SLEEP_PATCH", 1);
						} else {
							bootstrapini.SetInt( "NDS-BOOTSTRAP", "FORCE_SLEEP_PATCH", 0);
						}

                        CheatCodelist codelist;
						u32 gameCode,crc32;

						if (isHomebrew[cursorPosition[secondaryDevice]] == 0)
						{
							if(codelist.romData(path,gameCode,crc32))
							{
								long cheatOffset; size_t cheatSize;
								FILE* dat = fopen(sdFound() ? "sd:/_nds/TWiLightMenu/extras/usrcheat.dat" : "fat:/_nds/TWiLightMenu/extras/usrcheat.dat", "rb");
								if(dat)
								{
									if(codelist.searchCheatData(dat,gameCode,crc32,cheatOffset,cheatSize))
									{
										codelist.parse(path);
										bootstrapini.SetString("NDS-BOOTSTRAP", "CHEAT_DATA", codelist.getCheats());
									}
									fclose(dat);
								}
							}
						}

						bootstrapini.SaveIniFile( bootstrapinipath );
						if (!isArgv) {
							romPath = argarray[0];
						}
						launchType = 1;
						previousUsedDevice = secondaryDevice;
						SaveSettings();
						if (secondaryDevice) {
							if (perGameSettings_bootstrapFile == -1) {
								if (homebrewBootstrap) {
									argarray.at(0) = (char*)(bootstrapFile ? "fat:/_nds/nds-bootstrap-hb-nightly.nds" : "fat:/_nds/nds-bootstrap-hb-release.nds");
								} else {
									argarray.at(0) = (char*)(bootstrapFile ? "fat:/_nds/nds-bootstrap-nightly.nds" : "fat:/_nds/nds-bootstrap-release.nds");
								}
							} else {
								if (homebrewBootstrap) {
									argarray.at(0) = (char*)(perGameSettings_bootstrapFile ? "fat:/_nds/nds-bootstrap-hb-nightly.nds" : "fat:/_nds/nds-bootstrap-hb-release.nds");
								} else {
									argarray.at(0) = (char*)(perGameSettings_bootstrapFile ? "fat:/_nds/nds-bootstrap-nightly.nds" : "fat:/_nds/nds-bootstrap-release.nds");
								}
							}
						} else {
							if (perGameSettings_bootstrapFile == -1) {
								if (homebrewBootstrap) {
									argarray.at(0) = (char*)(bootstrapFile ? "sd:/_nds/nds-bootstrap-hb-nightly.nds" : "sd:/_nds/nds-bootstrap-hb-release.nds");
								} else {
									argarray.at(0) = (char*)(bootstrapFile ? "sd:/_nds/nds-bootstrap-nightly.nds" : "sd:/_nds/nds-bootstrap-release.nds");
								}
							} else {
								if (homebrewBootstrap) {
									argarray.at(0) = (char*)(perGameSettings_bootstrapFile ? "sd:/_nds/nds-bootstrap-hb-nightly.nds" : "sd:/_nds/nds-bootstrap-hb-release.nds");
								} else {
									argarray.at(0) = (char*)(perGameSettings_bootstrapFile ? "sd:/_nds/nds-bootstrap-nightly.nds" : "sd:/_nds/nds-bootstrap-release.nds");
								}
							}
						}
						int err = runNdsFile (argarray[0], argarray.size(), (const char **)&argarray[0], true, false, true, true);
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
						romPath = argarray[0];
						launchType = 1;
						previousUsedDevice = secondaryDevice;
						SaveSettings();
						loadGameOnFlashcard(argarray[0], filename, true);
					}
				} else {
					if (!isArgv) {
						romPath = argarray[0];
					}
					launchType = 1;
					previousUsedDevice = secondaryDevice;
					SaveSettings();
					bool runNds_boostCpu = false;
					bool runNds_boostVram = false;
					if (isDSiMode() && !dsModeDSiWare) {
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
					int err = runNdsFile (argarray[0], argarray.size(), (const char **)&argarray[0], true, dsModeSwitch, runNds_boostCpu, runNds_boostVram);
					char text[32];
					snprintf (text, sizeof(text), "Start failed. Error %i", err);
					ClearBrightness();
					printLarge(false, 4, 4, text);
					stop();
				}
			} else if ((strcasecmp (substr(filename.c_str() + filename.size() - 3), ".gb") == 0)
					|| (strcasecmp (substr(filename.c_str() + filename.size() - 4), ".sgb") == 0)
					|| (strcasecmp (substr(filename.c_str() + filename.size() - 4), ".gbc") == 0))
			{
				std::string romfolderNoSlash = romfolder[secondaryDevice];
				RemoveTrailingSlashes(romfolderNoSlash);
				char gbROMpath[256];
				snprintf (gbROMpath, sizeof(gbROMpath), "%s/%s", romfolderNoSlash.c_str(), filename.c_str());
				romPath = gbROMpath;
				homebrewArg = gbROMpath;
				launchType = 4;
				previousUsedDevice = secondaryDevice;
				SaveSettings();
				argarray.push_back(gbROMpath);
				int err = 0;

				argarray.at(0) = (char*)(secondaryDevice ? "/_nds/TWiLightMenu/emulators/gameyob.nds" : "sd:/_nds/TWiLightMenu/emulators/gameyob.nds");
				err = runNdsFile (argarray[0], argarray.size(), (const char **)&argarray[0], true, false, true, true);	// Pass ROM to GameYob as argument

				char text[32];
				snprintf (text, sizeof(text), "Start failed. Error %i", err);
				ClearBrightness();
				printLarge(false, 4, 4, text);
				stop();
			} else if ((strcasecmp (substr(filename.c_str() + filename.size() - 4), ".nes") == 0)
					|| (strcasecmp (substr(filename.c_str() + filename.size() - 4), ".fds") == 0))
			{
				std::string romfolderNoSlash = romfolder[secondaryDevice];
				RemoveTrailingSlashes(romfolderNoSlash);
				char nesROMpath[256];
				snprintf (nesROMpath, sizeof(nesROMpath), "%s/%s", romfolderNoSlash.c_str(), filename.c_str());
				romPath = nesROMpath;
				homebrewArg = nesROMpath;
				launchType = 3;
				previousUsedDevice = secondaryDevice;
				SaveSettings();
				argarray.push_back(nesROMpath);
				int err = 0;

				argarray.at(0) = (char*)(secondaryDevice ? "/_nds/TWiLightMenu/emulators/nesds.nds" : "sd:/_nds/TWiLightMenu/emulators/nestwl.nds");
				err = runNdsFile (argarray[0], argarray.size(), (const char **)&argarray[0], true, false, true, true);	// Pass ROM to nesDS as argument

				char text[32];
				snprintf (text, sizeof(text), "Start failed. Error %i", err);
				ClearBrightness();
				printLarge(false, 4, 4, text);
				stop();
			} else if ((strcasecmp (substr(filename.c_str() + filename.size() - 4), ".sms") == 0)
					|| (strcasecmp (substr(filename.c_str() + filename.size() - 3), ".gg" ) == 0))
			{
				mkdir(secondaryDevice ? "fat:/data" : "sd:/data", 0777);
				mkdir(secondaryDevice ? "fat:/data/s8ds" : "sd:/data/s8ds", 0777);
				std::string romfolderNoSlash = romfolder[secondaryDevice];
				RemoveTrailingSlashes(romfolderNoSlash);
				char smsROMpath[256];
				snprintf (smsROMpath, sizeof(smsROMpath), "%s/%s", romfolderNoSlash.c_str(), filename.c_str());
				romPath = smsROMpath;
				homebrewArg = smsROMpath;
				launchType = 5;
				previousUsedDevice = secondaryDevice;
				SaveSettings();
				argarray.push_back(smsROMpath);
				int err = 0;

				argarray.at(0) = (char*)(secondaryDevice ? "/_nds/TWiLightMenu/emulators/S8DS.nds" : (!arm7SCFGLocked ? "sd:/_nds/TWiLightMenu/emulators/S8DS_notouch.nds" : "sd:/_nds/TWiLightMenu/emulators/S8DS.nds"));
				err = runNdsFile (argarray[0], argarray.size(), (const char **)&argarray[0], true, false, true, true);	// Pass ROM to S8DS as argument

				char text[32];
				snprintf (text, sizeof(text), "Start failed. Error %i", err);
				ClearBrightness();
				printLarge(false, 4, 4, text);
				stop();
			} else if ((strcasecmp (substr(filename.c_str() + filename.size() - 4, ".gen")) == 0) {
				std::string romfolderNoSlash = romfolder[secondaryDevice];
				RemoveTrailingSlashes(romfolderNoSlash);
				char genROMpath[256];
				snprintf (genROMpath, sizeof(genROMpath), "%s/%s", romfolderNoSlash.c_str(), filename.c_str());
				homebrewBootstrap = true;
				romPath = genROMpath;
				launchType = 1;
				previousUsedDevice = secondaryDevice;
				SaveSettings();
				if (secondaryDevice) {
					argarray.at(0) = (char*)("fat:/_nds/TWiLightMenu/emulators/jEnesisDS.nds");
				} else {
					argarray.at(0) = (char*)(bootstrapFile ? "sd:/_nds/nds-bootstrap-hb-nightly.nds" : "sd:/_nds/nds-bootstrap-hb-release.nds");
					CIniFile bootstrapini( "sd:/_nds/nds-bootstrap.ini" );
					bootstrapini.SetString("NDS-BOOTSTRAP", "NDS_PATH", "sd:/_nds/TWiLightMenu/emulators/jEnesisDS.nds");
					bootstrapini.SetString("NDS-BOOTSTRAP", "HOMEBREW_ARG", "fat:/ROM.BIN");
					bootstrapini.SetString("NDS-BOOTSTRAP", "RAM_DRIVE_PATH", genROMpath);
					bootstrapini.SetInt("NDS-BOOTSTRAP", "BOOST_CPU", 1);
					bootstrapini.SaveIniFile( "sd:/_nds/nds-bootstrap.ini" );
				}
				int err = runNdsFile (argarray[0], argarray.size(), (const char **)&argarray[0], true, false, true, true);
				char text[32];
				snprintf (text, sizeof(text), "Start failed. Error %i", err);
				ClearBrightness();
				printLarge(false, 4, 4, text);
				stop();
			} else if ((strcasecmp (substr(filename.c_str() + filename.size() - 4), ".smc") == 0)
					|| (strcasecmp (substr(filename.c_str() + filename.size() - 4), ".sfc") == 0))
			{
				std::string romfolderNoSlash = romfolder[secondaryDevice];
				RemoveTrailingSlashes(romfolderNoSlash);
				char snesROMpath[256];
				snprintf (snesROMpath, sizeof(snesROMpath), "%s/%s", romfolderNoSlash.c_str(), filename.c_str());
				homebrewBootstrap = true;
				romPath = snesROMpath;
				launchType = 1;
				previousUsedDevice = secondaryDevice;
				SaveSettings();
				if (secondaryDevice) {
					argarray.at(0) = (char*)("fat:/_nds/TWiLightMenu/emulators/SNEmulDS.nds");
				} else {
					argarray.at(0) = (char*)(bootstrapFile ? "sd:/_nds/nds-bootstrap-hb-nightly.nds" : "sd:/_nds/nds-bootstrap-hb-release.nds");
					CIniFile bootstrapini( "sd:/_nds/nds-bootstrap.ini" );
					bootstrapini.SetString("NDS-BOOTSTRAP", "NDS_PATH", "sd:/_nds/TWiLightMenu/emulators/SNEmulDS.nds");
					bootstrapini.SetString("NDS-BOOTSTRAP", "HOMEBREW_ARG", "fat:/snes/ROM.SMC");
					bootstrapini.SetString("NDS-BOOTSTRAP", "RAM_DRIVE_PATH", snesROMpath);
					bootstrapini.SetInt("NDS-BOOTSTRAP", "BOOST_CPU", 0);
					bootstrapini.SaveIniFile( "sd:/_nds/nds-bootstrap.ini" );
				}
				int err = runNdsFile (argarray[0], argarray.size(), (const char **)&argarray[0], true, false, true, true);
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
		}
	}

	return 0;
}
