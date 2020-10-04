#include <nds.h>
#include <nds/arm9/dldi.h>

#include <stdio.h>
#include <fat.h>
#include <sys/stat.h>
#include <limits.h>

#include <string.h>
#include <unistd.h>
#include <gl2d.h>

#include "date.h"
#include "fileCopy.h"

#include "graphics/graphics.h"

#include "common/tonccpy.h"
#include "common/nitrofs.h"
#include "flashcard.h"
#include "ndsheaderbanner.h"
#include "gbaswitch.h"
#include "nds_loader_arm9.h"
#include "fileBrowse.h"
#include "perGameSettings.h"
#include "errorScreen.h"

#include "iconTitle.h"
#include "graphics/fontHandler.h"

#include "common/inifile.h"
#include "tool/stringtool.h"

#include "sound.h"
#include "language.h"

#include "cheat.h"
#include "crc.h"

#include "donorMap.h"
#include "mpuMap.h"
#include "speedBumpExcludeMap.h"
#include "saveMap.h"

#include "sr_data_srllastran.h"	// For rebooting into the game

bool useTwlCfg = false;

bool whiteScreen = false;
bool blackScreen = false;
bool fadeType = true;		// false = out, true = in
bool fadeSpeed = true;		// false = slow (for DSi launch effect), true = fast
bool controlTopBright = true;
bool controlBottomBright = true;
int fps = 60;
int colorMode = 0;
int blfLevel = 0;

extern void ClearBrightness();

const char* settingsinipath = "sd:/_nds/TWiLightMenu/settings.ini";
const char* bootstrapinipath = "sd:/_nds/nds-bootstrap.ini";

std::string romPath[2];
std::string dsiWareSrlPath;
std::string dsiWarePubPath;
std::string dsiWarePrvPath;
std::string homebrewArg;

const char *charUnlaunchBg;
std::string unlaunchBg = "default.gif";
bool removeLauncherPatches = true;

const char *unlaunchAutoLoadID = "AutoLoadInfo";
static char hiyaNdsPath[14] = {'s','d','m','c',':','/','h','i','y','a','.','d','s','i'};
char unlaunchDevicePath[256];

bool arm7SCFGLocked = false;
int consoleModel = 0;
/*	0 = Nintendo DSi (Retail)
	1 = Nintendo DSi (Dev/Panda)
	2 = Nintendo 3DS
	3 = New Nintendo 3DS	*/
bool isRegularDS = true;

extern bool showdialogbox;
extern int dialogboxHeight;

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

std::string romfolder[2];

// These are used by flashcard functions and must retain their trailing slash.
static const std::string slashchar = "/";
static const std::string woodfat = "fat0:/";
static const std::string dstwofat = "fat1:/";

std::string r4_theme;

int mpuregion = 0;
int mpusize = 0;
bool ceCached = true;

bool applaunch = false;

bool startMenu = true;
bool gotosettings = false;

bool slot1Launched = false;
int launchType[2] = {0};		// 0 = No launch, 1 = SD/Flash card, 2 = SD/Flash card (Direct boot), 3 = DSiWare, 4 = NES, 5 = (S)GB(C), 6 = SMS/GG
int slot1LaunchMethod = 1;	// 0 == Reboot, 1 == Direct, 2 == Unlaunch
bool useBootstrap = true;
bool bootstrapFile = false;
bool homebrewBootstrap = false;
bool homebrewHasWide = false;
bool show12hrClock = true;
//bool snesEmulator = true;
bool smsGgInRam = false;
bool fcSaveOnSd = false;
bool wideScreen = false;

bool sdRemoveDetect = true;
bool useGbarunner = false;
bool gbar2DldiAccess = false;	// false == ARM9, true == ARM7
int theme = 0;
int subtheme = 0;
int cursorPosition[2] = {0};
int startMenu_cursorPosition = 0;
int pagenum[2] = {0};
bool showMicroSd = false;
bool showNds = true;
bool showRvid = true;
bool showA26 = true;
bool showNes = true;
bool showGb = true;
bool showSmsGg = true;
int showMd = 3;
bool showSnes = true;
bool showPce = true;
bool showDirectories = true;
bool showHidden = false;
bool preventDeletion = false;
bool animateDsiIcons = false;
int launcherApp = -1;
int sysRegion = -1;

int guiLanguage = -1;
int titleLanguage = -1;
int gameLanguage = -1;
bool boostCpu = false;	// false == NTR, true == TWL
bool boostVram = false;
int bstrap_dsiMode = 0;
bool forceSleepPatch = false;
bool dsiWareBooter = false;

void LoadSettings(void) {
	useBootstrap = isDSiMode();

	// GUI
	CIniFile settingsini( settingsinipath );

	// UI settings.
	romfolder[0] = settingsini.GetString("SRLOADER", "ROM_FOLDER", "sd:/");
	romfolder[1] = settingsini.GetString("SRLOADER", "SECONDARY_ROM_FOLDER", "fat:/");
	pagenum[0] = settingsini.GetInt("SRLOADER", "PAGE_NUMBER", 0);
	pagenum[1] = settingsini.GetInt("SRLOADER", "SECONDARY_PAGE_NUMBER", 0);
	cursorPosition[0] = settingsini.GetInt("SRLOADER", "CURSOR_POSITION", 0);
	cursorPosition[1] = settingsini.GetInt("SRLOADER", "SECONDARY_CURSOR_POSITION", 0);
	//startMenu_cursorPosition = settingsini.GetInt("SRLOADER", "STARTMENU_CURSOR_POSITION", 1);
	consoleModel = settingsini.GetInt("SRLOADER", "CONSOLE_MODEL", 0);

	showNds = settingsini.GetInt("SRLOADER", "SHOW_NDS", true);
	showRvid = settingsini.GetInt("SRLOADER", "SHOW_RVID", true);
	showA26 = settingsini.GetInt("SRLOADER", "SHOW_A26", true);
	showNes = settingsini.GetInt("SRLOADER", "SHOW_NES", true);
	showGb = settingsini.GetInt("SRLOADER", "SHOW_GB", true);
	showSmsGg = settingsini.GetInt("SRLOADER", "SHOW_SMSGG", true);
	showMd = settingsini.GetInt("SRLOADER", "SHOW_MDGEN", showMd);
	showSnes = settingsini.GetInt("SRLOADER", "SHOW_SNES", true);
	showPce = settingsini.GetInt("SRLOADER", "SHOW_PCE", true);

	// Customizable UI settings.
	fps = settingsini.GetInt("SRLOADER", "FRAME_RATE", fps);
	colorMode = settingsini.GetInt("SRLOADER", "COLOR_MODE", 0);
	blfLevel = settingsini.GetInt("SRLOADER", "BLUE_LIGHT_FILTER_LEVEL", 0);
	guiLanguage = settingsini.GetInt("SRLOADER", "LANGUAGE", -1);
	titleLanguage = settingsini.GetInt("SRLOADER", "TITLELANGUAGE", titleLanguage);
	sdRemoveDetect = settingsini.GetInt("SRLOADER", "SD_REMOVE_DETECT", 1);
	useGbarunner = settingsini.GetInt("SRLOADER", "USE_GBARUNNER2", 0);
	if (!isRegularDS) useGbarunner = true;
	gbar2DldiAccess = settingsini.GetInt("SRLOADER", "GBAR2_DLDI_ACCESS", gbar2DldiAccess);
	showMicroSd = settingsini.GetInt("SRLOADER", "SHOW_MICROSD", showMicroSd);
	theme = settingsini.GetInt("SRLOADER", "THEME", 0);
	subtheme = settingsini.GetInt("SRLOADER", "SUB_THEME", 0);
	showDirectories = settingsini.GetInt("SRLOADER", "SHOW_DIRECTORIES", 1);
	showHidden = settingsini.GetInt("SRLOADER", "SHOW_HIDDEN", 0);
    preventDeletion = settingsini.GetInt("SRLOADER", "PREVENT_ROM_DELETION", preventDeletion);
	animateDsiIcons = settingsini.GetInt("SRLOADER", "ANIMATE_DSI_ICONS", 0);
	if (consoleModel < 2) {
		launcherApp = settingsini.GetInt("SRLOADER", "LAUNCHER_APP", launcherApp);
	}

	r4_theme = "sd:/";
	if (strcmp(settingsinipath, "fat:/_nds/TWiLightMenu/settings.ini") == 0) {		// Fallback to .ini path on flashcard, if not found on SD card, or if SD access is disabled
		r4_theme = "fat:/";
	}

	r4_theme += "_nds/TWiLightMenu/r4menu/themes/";
	r4_theme += settingsini.GetString("SRLOADER", "R4_THEME", "") + "/";

	previousUsedDevice = settingsini.GetInt("SRLOADER", "PREVIOUS_USED_DEVICE", previousUsedDevice);
	secondaryDevice = bothSDandFlashcard() ? settingsini.GetInt("SRLOADER", "SECONDARY_DEVICE", secondaryDevice) : flashcardFound();
	fcSaveOnSd = settingsini.GetInt("SRLOADER", "FC_SAVE_ON_SD", fcSaveOnSd);

	slot1LaunchMethod = settingsini.GetInt("SRLOADER", "SLOT1_LAUNCHMETHOD", slot1LaunchMethod);
	useBootstrap = settingsini.GetInt("SRLOADER", "USE_BOOTSTRAP", useBootstrap);
	bootstrapFile = settingsini.GetInt("SRLOADER", "BOOTSTRAP_FILE", 0);
    //snesEmulator = settingsini.GetInt("SRLOADER", "SNES_EMULATOR", snesEmulator);
    smsGgInRam = settingsini.GetInt("SRLOADER", "SMS_GG_IN_RAM", smsGgInRam);

    show12hrClock = settingsini.GetInt("SRLOADER", "SHOW_12H_CLOCK", show12hrClock);

    unlaunchBg = settingsini.GetString("SRLOADER", "UNLAUNCH_BG", unlaunchBg);
	charUnlaunchBg = unlaunchBg.c_str();
	removeLauncherPatches = settingsini.GetInt("SRLOADER", "UNLAUNCH_PATCH_REMOVE", removeLauncherPatches);

	// Default nds-bootstrap settings
	gameLanguage = settingsini.GetInt("NDS-BOOTSTRAP", "LANGUAGE", -1);
	boostCpu = settingsini.GetInt("NDS-BOOTSTRAP", "BOOST_CPU", 0);
	boostVram = settingsini.GetInt("NDS-BOOTSTRAP", "BOOST_VRAM", 0);
	bstrap_dsiMode = settingsini.GetInt("NDS-BOOTSTRAP", "DSI_MODE", 0);
	forceSleepPatch = settingsini.GetInt("NDS-BOOTSTRAP", "FORCE_SLEEP_PATCH", 0);
	dsiWareBooter = settingsini.GetInt("SRLOADER", "DSIWARE_BOOTER", dsiWareBooter);

	romPath[0] = settingsini.GetString("SRLOADER", "ROM_PATH", romPath[0]);
	romPath[1] = settingsini.GetString("SRLOADER", "SECONDARY_ROM_PATH", romPath[1]);
	dsiWareSrlPath = settingsini.GetString("SRLOADER", "DSIWARE_SRL", dsiWareSrlPath);
	dsiWarePubPath = settingsini.GetString("SRLOADER", "DSIWARE_PUB", dsiWarePubPath);
	dsiWarePrvPath = settingsini.GetString("SRLOADER", "DSIWARE_PRV", dsiWarePrvPath);
	launchType[0] = settingsini.GetInt("SRLOADER", "LAUNCH_TYPE", launchType[0]);
	launchType[1] = settingsini.GetInt("SRLOADER", "SECONDARY_LAUNCH_TYPE", launchType[1]);

	wideScreen = settingsini.GetInt("SRLOADER", "WIDESCREEN", wideScreen);
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
	//settingsini.SetInt("SRLOADER", "STARTMENU_CURSOR_POSITION", startMenu_cursorPosition);

	// UI settings.
	if (bothSDandFlashcard()) {
		settingsini.SetInt("SRLOADER", "SECONDARY_DEVICE", secondaryDevice);
	}
	if (!gotosettings) {
		settingsini.SetInt("SRLOADER", "PREVIOUS_USED_DEVICE", previousUsedDevice);
		settingsini.SetString("SRLOADER", "ROM_PATH", romPath[0]);
		settingsini.SetString("SRLOADER", "SECONDARY_ROM_PATH", romPath[1]);
		settingsini.SetString("SRLOADER", "DSIWARE_SRL", dsiWareSrlPath);
		settingsini.SetString("SRLOADER", "DSIWARE_PUB", dsiWarePubPath);
		settingsini.SetString("SRLOADER", "DSIWARE_PRV", dsiWarePrvPath);
		settingsini.SetInt("SRLOADER", "SLOT1_LAUNCHED", slot1Launched);
		settingsini.SetInt("SRLOADER", "LAUNCH_TYPE", launchType[0]);
		settingsini.SetInt("SRLOADER", "SECONDARY_LAUNCH_TYPE", launchType[1]);
		settingsini.SetString("SRLOADER", secondaryDevice ? "SECONDARY_HOMEBREW_ARG" : "HOMEBREW_ARG", homebrewArg);
		settingsini.SetInt("SRLOADER", "HOMEBREW_BOOTSTRAP", homebrewBootstrap);
		settingsini.SetInt("SRLOADER", "HOMEBREW_HAS_WIDE", homebrewHasWide);
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
	switch (useTwlCfg ? *(u8*)0x02000444 : PersonalData->theme) {
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

bool useBackend = false;

using namespace std;

bool showbubble = false;
bool showSTARTborder = false;

bool titleboxXmoveleft = false;
bool titleboxXmoveright = false;

bool applaunchprep = false;

int spawnedtitleboxes = 0;

//char usernameRendered[10];
//bool usernameRenderedDone = false;

touchPosition touch;

//---------------------------------------------------------------------------------
void stop (void) {
//---------------------------------------------------------------------------------
	while (1) {
		swiWaitForVBlank();
	}
}

/**
 * Set donor SDK version for a specific game.
 */
int SetDonorSDK(const char* filename) {
	FILE *f_nds_file = fopen(filename, "rb");

	char game_TID[5];
	grabTID(f_nds_file, game_TID);
	fclose(f_nds_file);
	game_TID[4] = 0;
	game_TID[3] = 0;
	
	for (auto i : donorMap) {
		if (i.first == 5 && game_TID[0] == 'V')
			return 5;

		if (i.second.find(game_TID) != i.second.cend())
			return i.first;
	}

	return 0;
}

/**
 * Set MPU settings for a specific game.
 */
void SetMPUSettings(const char* filename) {
	FILE *f_nds_file = fopen(filename, "rb");

	char game_TID[5];
	fseek(f_nds_file, offsetof(sNDSHeaderExt, gameCode), SEEK_SET);
	fread(game_TID, 1, 4, f_nds_file);
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
 * Move nds-bootstrap's cardEngine_arm9 to cached memory region for some games.
 */
void SetSpeedBumpExclude(const char* filename) {
	if (!isDSiMode() || (perGameSettings_heapShrink >= 0 && perGameSettings_heapShrink < 2)) {
		ceCached = perGameSettings_heapShrink;
		return;
	}

	FILE *f_nds_file = fopen(filename, "rb");

	char game_TID[5];
	fseek(f_nds_file, offsetof(sNDSHeaderExt, gameCode), SEEK_SET);
	fread(game_TID, 1, 4, f_nds_file);
	fclose(f_nds_file);

	// TODO: If the list gets large enough, switch to bsearch().
	for (unsigned int i = 0; i < sizeof(sbeList2)/sizeof(sbeList2[0]); i++) {
		if (memcmp(game_TID, sbeList2[i], 3) == 0) {
			// Found match
			ceCached = false;
		}
	}
}

/**
 * Fix AP for some games.
 */
std::string setApFix(const char *filename) {
	remove("fat:/_nds/nds-bootstrap/apFix.ips");

	bool ipsFound = false;
	char ipsPath[256];
	snprintf(ipsPath, sizeof(ipsPath), "%s:/_nds/TWiLightMenu/apfix/%s.ips", sdFound() ? "sd" : "fat", filename);
	ipsFound = (access(ipsPath, F_OK) == 0);

	if (!ipsFound) {
		FILE *f_nds_file = fopen(filename, "rb");

		char game_TID[5];
		u16 headerCRC16 = 0;
		fseek(f_nds_file, offsetof(sNDSHeaderExt, gameCode), SEEK_SET);
		fread(game_TID, 1, 4, f_nds_file);
		fseek(f_nds_file, offsetof(sNDSHeaderExt, headerCRC16), SEEK_SET);
		fread(&headerCRC16, sizeof(u16), 1, f_nds_file);
		fclose(f_nds_file);
		game_TID[4] = 0;

		snprintf(ipsPath, sizeof(ipsPath), "%s:/_nds/TWiLightMenu/apfix/%s-%X.ips", sdFound() ? "sd" : "fat", game_TID, headerCRC16);
		ipsFound = (access(ipsPath, F_OK) == 0);
	}

	if (ipsFound) {
		if (secondaryDevice && sdFound()) {
			mkdir("fat:/_nds", 0777);
			mkdir("fat:/_nds/nds-bootstrap", 0777);
			fcopy(ipsPath, "fat:/_nds/nds-bootstrap/apFix.ips");
			return "fat:/_nds/nds-bootstrap/apFix.ips";
		}
		return ipsPath;
	}

	return "";
}

sNDSHeader ndsCart;

/**
 * Enable widescreen for some games.
 */
void SetWidescreen(const char *filename) {
	remove("/_nds/nds-bootstrap/wideCheatData.bin");

	bool useWidescreen = (perGameSettings_wideScreen == -1 ? wideScreen : perGameSettings_wideScreen);

	if ((isDSiMode() && arm7SCFGLocked) || consoleModel < 2 || !useWidescreen
	|| (access("sd:/luma/sysmodules/TwlBg.cxi", F_OK) != 0)) {
		homebrewHasWide = false;
		return;
	}

	bool wideCheatFound = false;
	char wideBinPath[256];
	if (launchType[secondaryDevice] == 1) {
		snprintf(wideBinPath, sizeof(wideBinPath), "sd:/_nds/TWiLightMenu/widescreen/%s.bin", filename);
		wideCheatFound = (access(wideBinPath, F_OK) == 0);
	}

	if (slot1Launched) {
		// Reset Slot-1 to allow reading card header
		sysSetCardOwner (BUS_OWNER_ARM9);
		disableSlot1();
		for(int i = 0; i < 25; i++) { swiWaitForVBlank(); }
		enableSlot1();
		for(int i = 0; i < 15; i++) { swiWaitForVBlank(); }

		cardReadHeader((uint8*)&ndsCart);

		char game_TID[5];
		tonccpy(game_TID, ndsCart.gameCode, 4);
		game_TID[4] = 0;

		snprintf(wideBinPath, sizeof(wideBinPath), "sd:/_nds/TWiLightMenu/widescreen/%s-%X.bin", game_TID, ndsCart.headerCRC16);
		wideCheatFound = (access(wideBinPath, F_OK) == 0);
	} else if (!wideCheatFound) {
		FILE *f_nds_file = fopen(filename, "rb");

		char game_TID[5];
		u16 headerCRC16 = 0;
		fseek(f_nds_file, offsetof(sNDSHeaderExt, gameCode), SEEK_SET);
		fread(game_TID, 1, 4, f_nds_file);
		fseek(f_nds_file, offsetof(sNDSHeaderExt, headerCRC16), SEEK_SET);
		fread(&headerCRC16, sizeof(u16), 1, f_nds_file);
		fclose(f_nds_file);
		game_TID[4] = 0;

		snprintf(wideBinPath, sizeof(wideBinPath), "sd:/_nds/TWiLightMenu/widescreen/%s-%X.bin", game_TID, headerCRC16);
		wideCheatFound = (access(wideBinPath, F_OK) == 0);
	}

	if (isHomebrew) {
		return;
	}

	if (wideCheatFound) {
		const char* resultText1;
		const char* resultText2;
		mkdir("/_nds", 0777);
		mkdir("/_nds/nds-bootstrap", 0777);
		if (fcopy(wideBinPath, "/_nds/nds-bootstrap/wideCheatData.bin") == 0) {
			return;
		} else {
			resultText1 = "Failed to copy widescreen";
			resultText2 = "code for the game.";
		}
		remove("/_nds/nds-bootstrap/wideCheatData.bin");
		int textXpos[2] = {0};
		textXpos[0] = 72;
		textXpos[1] = 84;
		clearText();
		printSmallCentered(false, textXpos[0], resultText1);
		printSmallCentered(false, textXpos[1], resultText2);
		fadeType = true; // Fade in from white
		for (int i = 0; i < 60 * 3; i++) {
			swiWaitForVBlank(); // Wait 3 seconds
		}
		fadeType = false;	   // Fade to white
		for (int i = 0; i < 25; i++) {
			swiWaitForVBlank();
		}
	}
}

char filePath[PATH_MAX];

//---------------------------------------------------------------------------------
void doPause() {
//---------------------------------------------------------------------------------
	while(1) {
		scanKeys();
		if(keysDown() & KEY_START)
			break;
		swiWaitForVBlank();
	}
	scanKeys();
}

void loadGameOnFlashcard (const char* ndsPath, bool usePerGameSettings) {
	bool runNds_boostCpu = false;
	bool runNds_boostVram = false;
	if ((REG_SCFG_EXT != 0) && usePerGameSettings) {
		std::string filename = ndsPath;

		const size_t last_slash_idx = filename.find_last_of("/");
		if (std::string::npos != last_slash_idx) {
			filename.erase(0, last_slash_idx + 1);
		}

		loadPerGameSettings(filename);

		runNds_boostCpu = perGameSettings_boostCpu == -1 ? boostCpu : perGameSettings_boostCpu;
		runNds_boostVram = perGameSettings_boostVram == -1 ? boostVram : perGameSettings_boostVram;
	}
	std::string path;
	int err = 0;
	if ((memcmp(io_dldi_data->friendlyName, "R4(DS) - Revolution for DS", 26) == 0)
	 || (memcmp(io_dldi_data->friendlyName, "R4TF", 4) == 0)
	 || (memcmp(io_dldi_data->friendlyName, "R4iDSN", 6) == 0)) {
		CIniFile fcrompathini("fat:/_wfwd/lastsave.ini");
		path = replaceAll(ndsPath, "fat:/", woodfat);
		fcrompathini.SetString("Save Info", "lastLoaded", path);
		fcrompathini.SaveIniFile("fat:/_wfwd/lastsave.ini");
		err = runNdsFile("fat:/Wfwd.dat", 0, NULL, true, true, true, runNds_boostCpu, runNds_boostVram);
	} else if (memcmp(io_dldi_data->friendlyName, "Acekard AK2", 0xB) == 0) {
		CIniFile fcrompathini("fat:/_afwd/lastsave.ini");
		path = replaceAll(ndsPath, "fat:/", woodfat);
		fcrompathini.SetString("Save Info", "lastLoaded", path);
		fcrompathini.SaveIniFile("fat:/_afwd/lastsave.ini");
		err = runNdsFile("fat:/Afwd.dat", 0, NULL, true, true, true, runNds_boostCpu, runNds_boostVram);
	} else if (memcmp(io_dldi_data->friendlyName, "DSTWO(Slot-1)", 0xD) == 0) {
		CIniFile fcrompathini("fat:/_dstwo/autoboot.ini");
		path = replaceAll(ndsPath, "fat:/", dstwofat);
		fcrompathini.SetString("Dir Info", "fullName", path);
		fcrompathini.SaveIniFile("fat:/_dstwo/autoboot.ini");
		err = runNdsFile("fat:/_dstwo/autoboot.nds", 0, NULL, true, true, true, runNds_boostCpu, runNds_boostVram);
	} else if ((memcmp(io_dldi_data->friendlyName, "TTCARD", 6) == 0)
			 || (memcmp(io_dldi_data->friendlyName, "DSTT", 4) == 0)
			 || (memcmp(io_dldi_data->friendlyName, "DEMON", 5) == 0)) {
		CIniFile fcrompathini("fat:/TTMenu/YSMenu.ini");
		path = replaceAll(ndsPath, "fat:/", slashchar);
		fcrompathini.SetString("YSMENU", "AUTO_BOOT", path);
		fcrompathini.SaveIniFile("fat:/TTMenu/YSMenu.ini");
		err = runNdsFile("fat:/YSMenu.nds", 0, NULL, true, true, true, runNds_boostCpu, runNds_boostVram);
	}

	char text[32];
	snprintf (text, sizeof(text), "Start failed. Error %i", err);
	clearText();
	dialogboxHeight = (err==0 ? 2 : 0);
	showdialogbox = true;
	printLargeCentered(false, 74, "Error!");
	if (err == 0) {
		printSmallCentered(false, 90, "Flashcard may be unsupported.");
		printSmallCentered(false, 102, "Flashcard name:");
		printSmallCentered(false, 114, io_dldi_data->friendlyName);
	} else {
		printSmallCentered(false, 90, text);
	}
	printSmallCentered(false, (err==0 ? 132 : 108), "\u2428 Back");
	int pressed = 0;
	do {
		scanKeys();
		pressed = keysDown();
		checkSdEject();
		swiWaitForVBlank();
	} while (!(pressed & KEY_B));
	if (!isDSiMode()) {
		chdir("fat:/");
	}
	runNdsFile("/_nds/TWiLightMenu/r4menu.srldr", 0, NULL, true, true, false, true, true);
	stop();
}

void unlaunchRomBoot(const char* rom) {
	if (strncmp(rom, "cart:", 5) == 0) {
		sprintf(unlaunchDevicePath, "cart:");
	} else {
		sprintf(unlaunchDevicePath, "__%s", rom);
		unlaunchDevicePath[0] = 's';
		unlaunchDevicePath[1] = 'd';
		unlaunchDevicePath[2] = 'm';
		unlaunchDevicePath[3] = 'c';
	}

	tonccpy((u8*)0x02000800, unlaunchAutoLoadID, 12);
	*(u16*)(0x0200080C) = 0x3F0;		// Unlaunch Length for CRC16 (fixed, must be 3F0h)
	*(u16*)(0x0200080E) = 0;			// Unlaunch CRC16 (empty)
	*(u32*)(0x02000810) = 0;			// Unlaunch Flags
	*(u32*)(0x02000810) |= BIT(0);		// Load the title at 2000838h
	*(u32*)(0x02000810) |= BIT(1);		// Use colors 2000814h
	*(u16*)(0x02000814) = 0x7FFF;		// Unlaunch Upper screen BG color (0..7FFFh)
	*(u16*)(0x02000816) = 0x7FFF;		// Unlaunch Lower screen BG color (0..7FFFh)
	toncset((u8*)0x02000818, 0, 0x20+0x208+0x1C0);		// Unlaunch Reserved (zero)
	int i2 = 0;
	for (int i = 0; i < (int)sizeof(unlaunchDevicePath); i++) {
		*(u8*)(0x02000838+i2) = unlaunchDevicePath[i];		// Unlaunch Device:/Path/Filename.ext (16bit Unicode,end by 0000h)
		i2 += 2;
	}
	while (*(u16*)(0x0200080E) == 0) {	// Keep running, so that CRC16 isn't 0
		*(u16*)(0x0200080E) = swiCRC16(0xFFFF, (void*)0x02000810, 0x3F0);		// Unlaunch CRC16
	}

	DC_FlushAll();						// Make reboot not fail
	fifoSendValue32(FIFO_USER_02, 1);	// Reboot into DSiWare title, booted via Unlaunch
	stop();
}

void unlaunchSetHiyaBoot(void) {
	tonccpy((u8*)0x02000800, unlaunchAutoLoadID, 12);
	*(u16*)(0x0200080C) = 0x3F0;		// Unlaunch Length for CRC16 (fixed, must be 3F0h)
	*(u16*)(0x0200080E) = 0;			// Unlaunch CRC16 (empty)
	*(u32*)(0x02000810) |= BIT(0);		// Load the title at 2000838h
	*(u32*)(0x02000810) |= BIT(1);		// Use colors 2000814h
	*(u16*)(0x02000814) = 0x7FFF;		// Unlaunch Upper screen BG color (0..7FFFh)
	*(u16*)(0x02000816) = 0x7FFF;		// Unlaunch Lower screen BG color (0..7FFFh)
	toncset((u8*)0x02000818, 0, 0x20+0x208+0x1C0);		// Unlaunch Reserved (zero)
	int i2 = 0;
	for (int i = 0; i < 14; i++) {
		*(u8*)(0x02000838+i2) = hiyaNdsPath[i];		// Unlaunch Device:/Path/Filename.ext (16bit Unicode,end by 0000h)
		i2 += 2;
	}
	while (*(u16*)(0x0200080E) == 0) {	// Keep running, so that CRC16 isn't 0
		*(u16*)(0x0200080E) = swiCRC16(0xFFFF, (void*)0x02000810, 0x3F0);		// Unlaunch CRC16
	}
}

/**
 * Reboot into an SD game when in DS mode.
 */
void ntrStartSdGame(void) {
	if (consoleModel == 0) {
		unlaunchRomBoot("sd:/_nds/TWiLightMenu/resetgame.srldr");
	} else {
		tonccpy((u32 *)0x02000300, sr_data_srllastran, 0x020);
		DC_FlushAll();						// Make reboot not fail
		fifoSendValue32(FIFO_USER_02, 1);
		stop();
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
	while (*(u16*)(0x02000306) == 0) {	// Keep running, so that CRC16 isn't 0
		*(u16*)(0x02000306) = swiCRC16(0xFFFF, (void*)0x02000308, 0x18);
	}
	
	unlaunchSetHiyaBoot();

	DC_FlushAll();						// Make reboot not fail
	fifoSendValue32(FIFO_USER_02, 1);	// Reboot into DSiWare title, booted via Launcher
	stop();
}

bool extention(const std::string& filename, const char* ext) {
	if(strcasecmp(filename.c_str() + filename.size() - strlen(ext), ext)) {
		return false;
	} else {
		return true;
	}
}

//---------------------------------------------------------------------------------
int main(int argc, char **argv) {
//---------------------------------------------------------------------------------

	defaultExceptionHandler();

	useTwlCfg = (REG_SCFG_EXT!=0 && (*(u8*)0x02000400 & 0x0F) && (*(u8*)0x02000401 == 0) && (*(u8*)0x02000402 == 0) && (*(u8*)0x02000404 == 0) && (*(u8*)0x02000448 != 0));

	graphicsInit();

	extern const DISC_INTERFACE __my_io_dsisd;

	*(u32*)(0x2FFFD0C) = 0x54494D52;	// Run reboot timer
	fatMountSimple("sd", &__my_io_dsisd);
	fatMountSimple("fat", dldiGetInternal());
    bool fatInited = (sdFound() || flashcardFound());
	*(u32*)(0x2FFFD0C) = 0;
	chdir(sdFound()&&isDSiMode() ? "sd:/" : "fat:/");

	if (!fatInited) {
		fontInit();
		printSmall(false, 64, 32, "fatinitDefault failed!");
		fadeType = true;
		stop();
	}

	// Read user name
	/*char *username = (char*)PersonalData->name;

	// text
	for (int i = 0; i < 10; i++) {
		if (username[i*2] == 0x00)
			username[i*2/2] = 0;
		else
			username[i*2/2] = username[i*2];
	}*/
	
	LoadColor();

	nitroFSInit("/_nds/TWiLightMenu/r4menu.srldr");

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
	if (theme == 6) {
		extern int screenBrightness;
		screenBrightness = 31;
		fadeType = false;
		SetBrightness(0, -31);
		SetBrightness(1, -31);
	}

	if (isDSiMode() && sdFound() && consoleModel < 2 && launcherApp != -1) {
		u8 setRegion = 0;
		if (sysRegion == -1) {
			// Determine SysNAND region by searching region of System Settings on SDNAND
			char tmdpath[256];
			for (u8 i = 0x41; i <= 0x5A; i++)
			{
				snprintf(tmdpath, sizeof(tmdpath), "sd:/title/00030015/484e42%x/content/title.tmd", i);
				if (access(tmdpath, F_OK) == 0)
				{
					setRegion = i;
					break;
				}
			}
		} else {
			switch(sysRegion) {
				case 0:
				default:
					setRegion = 0x4A;	// JAP
					break;
				case 1:
					setRegion = 0x45;	// USA
					break;
				case 2:
					setRegion = 0x50;	// EUR
					break;
				case 3:
					setRegion = 0x55;	// AUS
					break;
				case 4:
					setRegion = 0x43;	// CHN
					break;
				case 5:
					setRegion = 0x4B;	// KOR
					break;
			}
		}

		snprintf(unlaunchDevicePath, sizeof(unlaunchDevicePath), "nand:/title/00030017/484E41%x/content/0000000%i.app", setRegion, launcherApp);
	}

	keysSetRepeat(10, 2);

	vector<string> extensionList;
	if (showNds) {
		extensionList.push_back(".nds");
		extensionList.push_back(".dsi");
		extensionList.push_back(".ids");
		extensionList.push_back(".srl");
		extensionList.push_back(".app");
		extensionList.push_back(".argv");
	}
	if (memcmp(io_dldi_data->friendlyName, "DSTWO(Slot-1)", 0xD) == 0) {
		extensionList.push_back(".plg");
	}
	if (showRvid) {
		extensionList.push_back(".rvid");
		extensionList.push_back(".mp4");
	}
	if (useGbarunner) {
		extensionList.emplace_back(".gba");
	}
	if (showA26) {
		extensionList.emplace_back(".a26");
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
	if (!isDSiMode() || (isDSiMode() && (flashcardFound() || !arm7SCFGLocked))) {
		if (showSnes) {
			extensionList.push_back(".smc");
			extensionList.push_back(".sfc");
		}
		if (showPce) {
			extensionList.push_back(".pce");
		}
	}
	srand(time(NULL));
	
	bool copyDSiWareSavBack = ((consoleModel < 2 && previousUsedDevice && bothSDandFlashcard() && launchType[previousUsedDevice] == 3 && !dsiWareBooter && access(dsiWarePubPath.c_str(), F_OK) == 0 && extention(dsiWarePubPath.c_str(), ".pub"))
							|| (consoleModel < 2 && previousUsedDevice && bothSDandFlashcard() && launchType[previousUsedDevice] == 3 && !dsiWareBooter && access(dsiWarePrvPath.c_str(), F_OK) == 0 && extention(dsiWarePrvPath.c_str(), ".prv")));
	
	if (copyDSiWareSavBack) {
		blackScreen = true;
	}

	graphicsLoad();
	fontInit();

	iconTitleInit();

	bool menuButtonPressed = false;
	
	if (theme == 6) {
		startMenu = false;
		fadeType = true;	// Fade in from white
		snd();
		snd().beginStream();
	}

	char path[256];

	if (copyDSiWareSavBack)
	{
		printLargeCentered(false, 16, "If this takes a while, close");
		printLargeCentered(false, 24, "and open the console's lid.");
		printLargeCentered(false, 88, "Now copying data...");
		printLargeCentered(false, 96, "Do not turn off the power.");
		if (access(dsiWarePubPath.c_str(), F_OK) == 0) {
			fcopy("sd:/_nds/TWiLightMenu/tempDSiWare.pub", dsiWarePubPath.c_str());
		}
		if (access(dsiWarePrvPath.c_str(), F_OK) == 0) {
			fcopy("sd:/_nds/TWiLightMenu/tempDSiWare.prv", dsiWarePrvPath.c_str());
		}
		clearText(false);
		blackScreen = false;
	}

	while(1) {

		if (startMenu) {
			fadeType = true;	// Fade in from white

			int pressed = 0;

		  if (theme == 6) {
				if (!isDSiMode()) {
					chdir("fat:/");
				} else if (sdFound()) {
					chdir("sd:/");
				}
				int err = runNdsFile ("/_nds/TWiLightMenu/settings.srldr", 0, NULL, true, false, false, true, true);
				iprintf ("Start failed. Error %i\n", err);
		  } else {
			do {
				clearText();
				printLargeCentered(false, -112, 166, DrawDate());
				printLargeCentered(false, 180, "SELECT: Settings menu");
				switch (startMenu_cursorPosition) {
					case 0:
					default:
						printLargeCentered(false, 166, "Game");
						break;
					case 1:
						if (flashcardFound()) {
							printLargeCentered(false, 166, "Not used");
						} else {
							printLargeCentered(false, 166, "Launch Slot-1 card");
						}
						break;
					case 2:
						if (useGbarunner) {
							printLargeCentered(false, 166, "Start GBARunner2");
						} else {
							printLargeCentered(false, 166, "Start GBA Mode");
						}
						break;
				}
				printLargeCentered(false, 112, 166, RetTime().c_str());

				scanKeys();
				pressed = keysDownRepeat();
				touchRead(&touch);
				checkSdEject();
				swiWaitForVBlank();
			} while (!pressed);

			if (pressed & KEY_LEFT) startMenu_cursorPosition--;
			if (pressed & KEY_RIGHT) startMenu_cursorPosition++;

			if ((pressed & KEY_TOUCH) && touch.py >= 62 && touch.py <= 133) {
				if (touch.px >= 10 && touch.px <= 82) {
					if (startMenu_cursorPosition != 0) {
						startMenu_cursorPosition = 0;
					} else {
						menuButtonPressed = true;
					}
				} else if (touch.px >= 92 && touch.px <= 164) {
					if (startMenu_cursorPosition != 1) {
						startMenu_cursorPosition = 1;
					} else {
						menuButtonPressed = true;
					}
				} else if (touch.px >= 174 && touch.px <= 246) {
					if (startMenu_cursorPosition != 2) {
						startMenu_cursorPosition = 2;
					} else {
						menuButtonPressed = true;
					}
				}
			}

			if ((pressed & KEY_TOUCH) && touch.px >= 236 && touch.px <= 251 && touch.py >= 3 && touch.py <= 23) {
				// Open the manual
				fadeType = false;	// Fade to white
				for (int i = 0; i < 25; i++) {
					swiWaitForVBlank();
				}
				if (!isDSiMode()) {
					chdir("fat:/");
				} else if (sdFound()) {
					chdir("sd:/");
				}
				int err = runNdsFile ("/_nds/TWiLightMenu/manual.srldr", 0, NULL, true, true, false, true, true);
				iprintf ("Start failed. Error %i\n", err);
			}

			if (pressed & KEY_A) {
				menuButtonPressed = true;
			}

			if (startMenu_cursorPosition < 0) startMenu_cursorPosition = 0;
			if (startMenu_cursorPosition > 2) startMenu_cursorPosition = 2;
		  }

			if (menuButtonPressed) {
				switch (startMenu_cursorPosition) {
					case 0:
					default:
						clearText();
						startMenu = false;
						break;
					case 1:
						if (!flashcardFound() && REG_SCFG_MC != 0x11) {
							fadeType = false;	// Fade to white
							for (int i = 0; i < 25; i++) {
								swiWaitForVBlank();
							}

							slot1Launched = 0;
							SaveSettings();
							if (slot1LaunchMethod==0 || arm7SCFGLocked) {
								dsCardLaunch();
							} else if (slot1LaunchMethod==2) {
								unlaunchRomBoot("cart:");
							} else {
								SetWidescreen(NULL);
								if (!isDSiMode()) {
									chdir("fat:/");
								} else if (sdFound()) {
									chdir("sd:/");
								}
								int err = runNdsFile ("/_nds/TWiLightMenu/slot1launch.srldr", 0, NULL, true, true, false, true, true);
								iprintf ("Start failed. Error %i\n", err);
							}
						}
						break;
					case 2:
						// Switch to GBA mode
						fadeType = false;	// Fade to white
						for (int i = 0; i < 25; i++) {
							swiWaitForVBlank();
						}
						if (useGbarunner) {
							if (secondaryDevice) {
								const char* gbaRunner2Path = gbar2DldiAccess ? "fat:/_nds/GBARunner2_arm7dldi_ds.nds" : "fat:/_nds/GBARunner2_arm9dldi_ds.nds";
								if (isDSiMode()) {
									gbaRunner2Path = consoleModel>0 ? "fat:/_nds/GBARunner2_arm7dldi_3ds.nds" : "fat:/_nds/GBARunner2_arm7dldi_dsi.nds";
								}
								if (useBootstrap) {
									int err = runNdsFile (gbaRunner2Path, 0, NULL, true, true, false, true, false);
									iprintf ("Start failed. Error %i\n", err);
								} else {
									loadGameOnFlashcard(gbaRunner2Path, false);
								}
							} else {
								std::string bootstrapPath = (bootstrapFile ? "sd:/_nds/nds-bootstrap-hb-nightly.nds" : "sd:/_nds/nds-bootstrap-hb-release.nds");

								std::vector<char*> argarray;
								argarray.push_back(strdup(bootstrapPath.c_str()));
								argarray.at(0) = (char*)bootstrapPath.c_str();

								const char* gbar2Path = consoleModel>0 ? "sd:/_nds/GBARunner2_arm7dldi_3ds.nds" : "sd:/_nds/GBARunner2_arm7dldi_dsi.nds";
								if (arm7SCFGLocked) {
									gbar2Path = consoleModel>0 ? "sd:/_nds/GBARunner2_arm7dldi_nodsp_3ds.nds" : "sd:/_nds/GBARunner2_arm7dldi_nodsp_dsi.nds";
								}

								CIniFile bootstrapini( "sd:/_nds/nds-bootstrap.ini" );
								bootstrapini.SetString("NDS-BOOTSTRAP", "NDS_PATH", gbar2Path);
								bootstrapini.SetString("NDS-BOOTSTRAP", "HOMEBREW_ARG", "");
								bootstrapini.SetString("NDS-BOOTSTRAP", "RAM_DRIVE_PATH", "");
								bootstrapini.SetInt("NDS-BOOTSTRAP", "LANGUAGE", gameLanguage);
								bootstrapini.SetInt("NDS-BOOTSTRAP", "DSI_MODE", 0);
								bootstrapini.SetInt("NDS-BOOTSTRAP", "BOOST_CPU", 1);
								bootstrapini.SetInt("NDS-BOOTSTRAP", "BOOST_VRAM", 0);
								bootstrapini.SaveIniFile( "sd:/_nds/nds-bootstrap.ini" );
								int err = runNdsFile (argarray[0], argarray.size(), (const char **)&argarray[0], false, true, false, true, true);
								iprintf ("Start failed. Error %i\n", err);
								if (err == 1) {
									iprintf(bootstrapFile ? "nds-bootstrap (Nightly)" : "nds-bootstrap (Release)");
									iprintf("\nnot found.");
								}
							}
						} else {
							gbaSwitch();
						}
						break;
				}

				menuButtonPressed = false;
			}

			if ((pressed & KEY_B) && !isRegularDS) {
				fadeType = false;	// Fade to white
				for (int i = 0; i < 25; i++) swiWaitForVBlank();
				if (!isDSiMode() || launcherApp == -1) {
					*(u32*)(0x02000300) = 0x434E4C54;	// Set "CNLT" warmboot flag
					*(u16*)(0x02000304) = 0x1801;
					*(u32*)(0x02000310) = 0x4D454E55;	// "MENU"
					unlaunchSetHiyaBoot();
				} else {
					tonccpy((u8*)0x02000800, unlaunchAutoLoadID, 12);
					*(u16*)(0x0200080C) = 0x3F0;		// Unlaunch Length for CRC16 (fixed, must be 3F0h)
					*(u16*)(0x0200080E) = 0;			// Unlaunch CRC16 (empty)
					*(u32*)(0x02000810) |= BIT(0);		// Load the title at 2000838h
					*(u32*)(0x02000810) |= BIT(1);		// Use colors 2000814h
					*(u16*)(0x02000814) = 0x7FFF;		// Unlaunch Upper screen BG color (0..7FFFh)
					*(u16*)(0x02000816) = 0x7FFF;		// Unlaunch Lower screen BG color (0..7FFFh)
					toncset((u8*)0x02000818, 0, 0x20+0x208+0x1C0);		// Unlaunch Reserved (zero)
					int i2 = 0;
					for (int i = 0; i < (int)sizeof(unlaunchDevicePath); i++) {
						*(u8*)(0x02000838+i2) = unlaunchDevicePath[i];		// Unlaunch Device:/Path/Filename.ext (16bit Unicode,end by 0000h)
						i2 += 2;
					}
					while (*(u16*)(0x0200080E) == 0) {	// Keep running, so that CRC16 isn't 0
						*(u16*)(0x0200080E) = swiCRC16(0xFFFF, (void*)0x02000810, 0x3F0);		// Unlaunch CRC16
					}
				}
				fifoSendValue32(FIFO_USER_02, 1);	// ReturntoDSiMenu
			}

			if ((pressed & KEY_START) || (pressed & KEY_SELECT)) {
				// Launch settings
				fadeType = false;	// Fade to white
				for (int i = 0; i < 25; i++) {
					swiWaitForVBlank();
				}

				gotosettings = true;
				SaveSettings();
				if (!isDSiMode()) {
					chdir("fat:/");
				} else if (sdFound()) {
					chdir("sd:/");
				}
				int err = runNdsFile ("/_nds/TWiLightMenu/settings.srldr", 0, NULL, true, false, false, true, true);
				iprintf ("Start failed. Error %i\n", err);
			}
		} else {
			snprintf (path, sizeof(path), "%s", romfolder[secondaryDevice].c_str());
			// Set directory
			chdir (path);

			if (theme == 6) {
				clearText(false);

				// Print the path
				printLarge(false, 0, 0, path);

				printLargeCentered(false, 96, "SELECT: Settings menu");
			}

			//Navigates to the file to launch
			filename = browseForFile(extensionList);
		}

		////////////////////////////////////
		// Launch the item

		if (applaunch) {
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
			if (extention(filename, ".argv"))
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
				isArgv = true;
			} else {
				argarray.push_back(strdup(filename.c_str()));
			}

			// Launch DSiWare .nds via Unlaunch
			if ((isDSiMode() || sdFound()) && isDSiWare) {
				const char *typeToReplace = ".nds";
				if (extention(filename, ".dsi")) {
					typeToReplace = ".dsi";
				} else if (extention(filename, ".ids")) {
					typeToReplace = ".ids";
				} else if (extention(filename, ".srl")) {
					typeToReplace = ".srl";
				} else if (extention(filename, ".app")) {
					typeToReplace = ".app";
				}

				char *name = argarray.at(0);
				strcpy (filePath + pathLen, name);
				free(argarray.at(0));
				argarray.at(0) = filePath;

				dsiWareSrlPath = argarray[0];
				dsiWarePubPath = replaceAll(argarray[0], typeToReplace, ".pub");
				dsiWarePrvPath = replaceAll(argarray[0], typeToReplace, ".prv");
				if (!isArgv) {
					romPath[secondaryDevice] = argarray[0];
				}
				launchType[secondaryDevice] = (consoleModel>0 ? 1 : 3);
				previousUsedDevice = secondaryDevice;
				SaveSettings();

				sNDSHeaderExt NDSHeader;

				FILE *f_nds_file = fopen(filename.c_str(), "rb");

				fread(&NDSHeader, 1, sizeof(NDSHeader), f_nds_file);
				fclose(f_nds_file);

				if ((getFileSize(dsiWarePubPath.c_str()) == 0) && (NDSHeader.pubSavSize > 0)) {
					clearText();
					dialogboxHeight = 0;
					showdialogbox = true;
					printLargeCentered(false, 74, "Save creation");
					printSmallCentered(false, 98, "Creating public save file...");

					static const int BUFFER_SIZE = 4096;
					char buffer[BUFFER_SIZE];
					toncset(buffer, 0, sizeof(buffer));
					char savHdrPath[64];
					snprintf(savHdrPath, sizeof(savHdrPath), "nitro:/DSiWareSaveHeaders/%x.savhdr", (unsigned int)NDSHeader.pubSavSize);
					FILE *hdrFile = fopen(savHdrPath, "rb");
					if (hdrFile) fread(buffer, 1, 0x200, hdrFile);
					fclose(hdrFile);

					FILE *pFile = fopen(dsiWarePubPath.c_str(), "wb");
					if (pFile) {
						fwrite(buffer, 1, sizeof(buffer), pFile);
						fseek(pFile, NDSHeader.pubSavSize - 1, SEEK_SET);
						fputc('\0', pFile);
						fclose(pFile);
					}
					clearText();
					printLargeCentered(false, 74, "Save creation");
					printSmallCentered(false, 98, "Public save file created!");
					for (int i = 0; i < 60; i++) swiWaitForVBlank();
				}

				if ((getFileSize(dsiWarePrvPath.c_str()) == 0) && (NDSHeader.prvSavSize > 0)) {
					clearText();
					dialogboxHeight = 0;
					showdialogbox = true;
					printLargeCentered(false, 74, "Save creation");
					printSmallCentered(false, 98, "Creating private save file...");

					static const int BUFFER_SIZE = 4096;
					char buffer[BUFFER_SIZE];
					toncset(buffer, 0, sizeof(buffer));
					char savHdrPath[64];
					snprintf(savHdrPath, sizeof(savHdrPath), "nitro:/DSiWareSaveHeaders/%x.savhdr", (unsigned int)NDSHeader.prvSavSize);
					FILE *hdrFile = fopen(savHdrPath, "rb");
					if (hdrFile) fread(buffer, 1, 0x200, hdrFile);
					fclose(hdrFile);

					FILE *pFile = fopen(dsiWarePrvPath.c_str(), "wb");
					if (pFile) {
						fwrite(buffer, 1, sizeof(buffer), pFile);
						fseek(pFile, NDSHeader.prvSavSize - 1, SEEK_SET);
						fputc('\0', pFile);
						fclose(pFile);
					}
					clearText();
					printLargeCentered(false, 74, "Save creation");
					printSmallCentered(false, 98, "Private save file created!");
					for (int i = 0; i < 60; i++) swiWaitForVBlank();
				}

				if (dsiWareBooter || consoleModel > 0) {
					// Use nds-bootstrap
					loadPerGameSettings(filename);

					bootstrapinipath = "sd:/_nds/nds-bootstrap.ini";
					CIniFile bootstrapini(bootstrapinipath);
					bootstrapini.SetString("NDS-BOOTSTRAP", "NDS_PATH", dsiWareSrlPath);
					bootstrapini.SetString("NDS-BOOTSTRAP", "SAV_PATH", dsiWarePubPath);
					bootstrapini.SetString("NDS-BOOTSTRAP", "AP_FIX_PATH", "");
					bootstrapini.SetInt("NDS-BOOTSTRAP", "LANGUAGE", -1);
					bootstrapini.SetInt("NDS-BOOTSTRAP", "DSI_MODE", true);
					bootstrapini.SetInt("NDS-BOOTSTRAP", "BOOST_CPU", true);
					bootstrapini.SetInt("NDS-BOOTSTRAP", "BOOST_VRAM", true);
					bootstrapini.SetInt("NDS-BOOTSTRAP", "DONOR_SDK_VER", 5);
					bootstrapini.SetInt("NDS-BOOTSTRAP", "GAME_SOFT_RESET", 1);
					bootstrapini.SetInt("NDS-BOOTSTRAP", "PATCH_MPU_REGION", 0);
					bootstrapini.SetInt("NDS-BOOTSTRAP", "PATCH_MPU_SIZE", 0);
					bootstrapini.SetInt("NDS-BOOTSTRAP", "CARDENGINE_CACHED", 1);
					bootstrapini.SetInt("NDS-BOOTSTRAP", "FORCE_SLEEP_PATCH", 
						(forceSleepPatch
					|| (memcmp(io_dldi_data->friendlyName, "TTCARD", 6) == 0 && !isRegularDS)
					|| (memcmp(io_dldi_data->friendlyName, "DSTT", 4) == 0 && !isRegularDS)
					|| (memcmp(io_dldi_data->friendlyName, "DEMON", 5) == 0 && !isRegularDS)
					|| (memcmp(io_dldi_data->friendlyName, "R4iDSN", 6) == 0 && !isRegularDS))
					);
					bootstrapini.SaveIniFile(bootstrapinipath);

					if (isDSiMode() || !secondaryDevice) {
						SetWidescreen(filename.c_str());
					}
					if (!isDSiMode() && !secondaryDevice) {
						ntrStartSdGame();
					}

					bool useNightly = (perGameSettings_bootstrapFile == -1 ? bootstrapFile : perGameSettings_bootstrapFile);

					char ndsToBoot[256];
					sprintf(ndsToBoot, "sd:/_nds/nds-bootstrap-%s.nds", useNightly ? "nightly" : "release");
					if(access(ndsToBoot, F_OK) != 0) {
						sprintf(ndsToBoot, "fat:/_nds/nds-bootstrap-%s.nds", useNightly ? "nightly" : "release");
					}

					argarray.at(0) = (char *)ndsToBoot;
					int err = runNdsFile(argarray[0], argarray.size(), (const char **)&argarray[0], true, true, false, true, true);
					char text[32];
					snprintf (text, sizeof(text), "Start failed. Error %i", err);
					clearText();
					dialogboxHeight = (err==1 ? 2 : 0);
					showdialogbox = true;
					printLargeCentered(false, 74, "Error!");
					printSmallCentered(false, 90, text);
					if (err == 1) {
						printSmallCentered(false, 4, 102, useNightly ? "nds-bootstrap (Nightly)" : "nds-bootstrap (Release)");
						printSmallCentered(false, 4, 114, "not found.");
					}
					printSmallCentered(false, (err==1 ? 132 : 108), "\u2428 Back");
					int pressed = 0;
					do {
						scanKeys();
						pressed = keysDownRepeat();
						checkSdEject();
						swiWaitForVBlank();
					} while (!(pressed & KEY_B));
					if (!isDSiMode()) {
						chdir("fat:/");
					} else if (sdFound()) {
						chdir("sd:/");
					}
					runNdsFile("/_nds/TWiLightMenu/r4menu.srldr", 0, NULL, true, true, false, true, true);
					stop();
				}

				if (secondaryDevice) {
					clearText();
					dialogboxHeight = 0;
					showdialogbox = true;
					printLargeCentered(false, 74, "Please wait");
					printSmallCentered(false, 98, "Now copying data...");
					printSmallCentered(false, 110, "Do not turn off the power.");
					fcopy(dsiWareSrlPath.c_str(), "sd:/_nds/TWiLightMenu/tempDSiWare.dsi");
					if ((access(dsiWarePubPath.c_str(), F_OK) == 0) && (NDSHeader.pubSavSize > 0)) {
						fcopy(dsiWarePubPath.c_str(), "sd:/_nds/TWiLightMenu/tempDSiWare.pub");
					}
					if ((access(dsiWarePrvPath.c_str(), F_OK) == 0) && (NDSHeader.prvSavSize > 0)) {
						fcopy(dsiWarePrvPath.c_str(), "sd:/_nds/TWiLightMenu/tempDSiWare.prv");
					}

					clearText();
					if ((access(dsiWarePubPath.c_str(), F_OK) == 0 && (NDSHeader.pubSavSize > 0))
					 || (access(dsiWarePrvPath.c_str(), F_OK) == 0 && (NDSHeader.prvSavSize > 0))) {
						dialogboxHeight = 1;
						printLargeCentered(false, 74, "Important!");
						printSmall(false, 2, 90, "After saving, please re-start");
						printSmall(false, 2, 102, "TWiLight Menu++ to transfer your");
						printSmall(false, 2, 114, "save data back.");
						for (int i = 0; i < 60*3; i++) swiWaitForVBlank();		// Wait 3 seconds
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

				tonccpy((u8*)0x02000800, unlaunchAutoLoadID, 12);
				*(u16*)(0x0200080C) = 0x3F0;		// Unlaunch Length for CRC16 (fixed, must be 3F0h)
				*(u16*)(0x0200080E) = 0;			// Unlaunch CRC16 (empty)
				*(u32*)(0x02000810) = 0;			// Unlaunch Flags
				*(u32*)(0x02000810) |= BIT(0);		// Load the title at 2000838h
				*(u32*)(0x02000810) |= BIT(1);		// Use colors 2000814h
				*(u16*)(0x02000814) = 0x7FFF;		// Unlaunch Upper screen BG color (0..7FFFh)
				*(u16*)(0x02000816) = 0x7FFF;		// Unlaunch Lower screen BG color (0..7FFFh)
				toncset((u8*)0x02000818, 0, 0x20+0x208+0x1C0);		// Unlaunch Reserved (zero)
				int i2 = 0;
				for (int i = 0; i < (int)sizeof(unlaunchDevicePath); i++) {
					*(u8*)(0x02000838+i2) = unlaunchDevicePath[i];		// Unlaunch Device:/Path/Filename.ext (16bit Unicode,end by 0000h)
					i2 += 2;
				}
				while (*(u16*)(0x0200080E) == 0) {	// Keep running, so that CRC16 isn't 0
					*(u16*)(0x0200080E) = swiCRC16(0xFFFF, (void*)0x02000810, 0x3F0);		// Unlaunch CRC16
				}
				// Stabilization code to make DSiWare always boot successfully(?)
				clearText();
				for (int i = 0; i < 15; i++) swiWaitForVBlank();

				fifoSendValue32(FIFO_USER_02, 1);	// Reboot into DSiWare title, booted via Launcher
				for (int i = 0; i < 15; i++) swiWaitForVBlank();
			} else

			// Launch .nds directly or via nds-bootstrap
			if (extention(filename, ".nds") || extention(filename, ".dsi")
			 || extention(filename, ".ids") || extention(filename, ".srl")
			 || extention(filename, ".app")) {
				const char *typeToReplace = ".nds";
				if (extention(filename, ".dsi")) {
					typeToReplace = ".dsi";
				} else if (extention(filename, ".ids")) {
					typeToReplace = ".ids";
				} else if (extention(filename, ".srl")) {
					typeToReplace = ".srl";
				} else if (extention(filename, ".app")) {
					typeToReplace = ".app";
				}

				bool dsModeSwitch = false;
				bool dsModeDSiWare = false;

				char game_TID[5];

				FILE *f_nds_file = fopen(argarray[0], "rb");

				fseek(f_nds_file, offsetof(sNDSHeaderExt, gameCode), SEEK_SET);
				fread(game_TID, 1, 4, f_nds_file);
				fclose(f_nds_file);
				game_TID[4] = 0;
				game_TID[3] = 0;

				if (memcmp(game_TID, "HND", 3) == 0 || memcmp(game_TID, "HNE", 3) == 0) {
					dsModeSwitch = true;
					dsModeDSiWare = true;
					useBackend = false;	// Bypass nds-bootstrap
					homebrewBootstrap = true;
				} else if (isHomebrew) {
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
					if(useBootstrap || !secondaryDevice) {
						std::string path = argarray[0];
						std::string savename = replaceAll(filename, typeToReplace, getSavExtension());
						std::string ramdiskname = replaceAll(filename, typeToReplace, getImgExtension(perGameSettings_ramDiskNo));
						std::string romFolderNoSlash = romfolder[secondaryDevice];
						RemoveTrailingSlashes(romFolderNoSlash);
						mkdir (isHomebrew ? "ramdisks" : "saves", 0777);
						std::string savepath = romFolderNoSlash+"/saves/"+savename;
						if (sdFound() && secondaryDevice && fcSaveOnSd) {
							savepath = replaceAll(savepath, "fat:/", "sd:/");
						}
						std::string ramdiskpath = romFolderNoSlash+"/ramdisks/"+ramdiskname;

						if (!isHomebrew && (strcmp(game_TID, "NTR") != 0)) {
							// Create or expand save if game isn't homebrew
							int orgsavesize = getFileSize(savepath.c_str());
							int savesize = 524288;	// 512KB (default size)

							for (auto i : saveMap) {
								if (i.second.find(game_TID) != i.second.cend()) {
									savesize = i.first;
									break;
								}
							}

							bool saveSizeFixNeeded = false;

							// TODO: If the list gets large enough, switch to bsearch().
							for (unsigned int i = 0; i < sizeof(saveSizeFixList) / sizeof(saveSizeFixList[0]); i++) {
								if (memcmp(game_TID, saveSizeFixList[i], 3) == 0) {
									// Found a match.
									saveSizeFixNeeded = true;
									break;
								}
							}

							if ((orgsavesize == 0 && savesize > 0) || (orgsavesize < savesize && saveSizeFixNeeded)) {
								clearText();
								dialogboxHeight = 0;
								showdialogbox = true;
								printLargeCentered(false, 74, "Save management");
								printSmallCentered(false, 90, (orgsavesize == 0) ? "Creating save file..." : "Expanding save file...");

								if (orgsavesize > 0) {
									fsizeincrease(savepath.c_str(), sdFound() ? "sd:/_nds/TWiLightMenu/temp.sav" : "fat:/_nds/TWiLightMenu/temp.sav", savesize);
								} else {
									FILE *pFile = fopen(savepath.c_str(), "wb");
									if (pFile) {
										fseek(pFile, savesize - 1, SEEK_SET);
										fputc('\0', pFile);
										fclose(pFile);
									}
								}
								clearText();
								printLargeCentered(false, 74, "Save management");
								printSmallCentered(false, 90, (orgsavesize == 0) ? "Save file created!" : "Save file expanded!");
								for (int i = 0; i < 30; i++) swiWaitForVBlank();
							}
						}

						int donorSdkVer = SetDonorSDK(argarray[0]);
						SetMPUSettings(argarray[0]);
						SetSpeedBumpExclude(argarray[0]);

						bool useWidescreen = (perGameSettings_wideScreen == -1 ? wideScreen : perGameSettings_wideScreen);

						bootstrapinipath = ((!secondaryDevice || (isDSiMode() && sdFound())) ? "sd:/_nds/nds-bootstrap.ini" : "fat:/_nds/nds-bootstrap.ini");
						CIniFile bootstrapini( bootstrapinipath );
						bootstrapini.SetString("NDS-BOOTSTRAP", "NDS_PATH", path);
						bootstrapini.SetString("NDS-BOOTSTRAP", "SAV_PATH", savepath);
						if (!isHomebrew) {
							bootstrapini.SetString("NDS-BOOTSTRAP", "AP_FIX_PATH", setApFix(argarray[0]));
						}
						bootstrapini.SetString("NDS-BOOTSTRAP", "HOMEBREW_ARG", (useWidescreen && (game_TID[0] == 'W' || romVersion == 0x57)) ? "wide" : "");
						bootstrapini.SetString("NDS-BOOTSTRAP", "RAM_DRIVE_PATH", (perGameSettings_ramDiskNo >= 0 && !secondaryDevice) ? ramdiskpath : "sd:/null.img");
						bootstrapini.SetInt("NDS-BOOTSTRAP", "LANGUAGE", perGameSettings_language == -2 ? gameLanguage : perGameSettings_language);
						if (isDSiMode() || !secondaryDevice) {
							bootstrapini.SetInt("NDS-BOOTSTRAP", "DSI_MODE", perGameSettings_dsiMode == -1 ? bstrap_dsiMode : perGameSettings_dsiMode);
						}
						if ((REG_SCFG_EXT != 0) || !secondaryDevice) {
							bootstrapini.SetInt("NDS-BOOTSTRAP", "BOOST_CPU", perGameSettings_boostCpu == -1 ? boostCpu : perGameSettings_boostCpu);
							bootstrapini.SetInt("NDS-BOOTSTRAP", "BOOST_VRAM", perGameSettings_boostVram == -1 ? boostVram : perGameSettings_boostVram);
						}
						bootstrapini.SetInt("NDS-BOOTSTRAP", "DONOR_SDK_VER", donorSdkVer);
						bootstrapini.SetInt("NDS-BOOTSTRAP", "PATCH_MPU_REGION", mpuregion);
						bootstrapini.SetInt("NDS-BOOTSTRAP", "PATCH_MPU_SIZE", mpusize);
						bootstrapini.SetInt("NDS-BOOTSTRAP", "CARDENGINE_CACHED", ceCached);
						bootstrapini.SetInt("NDS-BOOTSTRAP", "FORCE_SLEEP_PATCH", 
							(forceSleepPatch
						|| (memcmp(io_dldi_data->friendlyName, "TTCARD", 6) == 0 && !isRegularDS)
						|| (memcmp(io_dldi_data->friendlyName, "DSTT", 4) == 0 && !isRegularDS)
						|| (memcmp(io_dldi_data->friendlyName, "DEMON", 5) == 0 && !isRegularDS)
						|| (memcmp(io_dldi_data->friendlyName, "R4iDSN", 6) == 0 && !isRegularDS))
						);
						if (!isDSiMode() && secondaryDevice && sdFound()) {
							CIniFile bootstrapiniSD("sd:/_nds/nds-bootstrap.ini");
							bootstrapini.SetInt("NDS-BOOTSTRAP", "DEBUG", bootstrapiniSD.GetInt("NDS-BOOTSTRAP", "DEBUG", 0));
							bootstrapini.SetInt("NDS-BOOTSTRAP", "LOGGING", bootstrapiniSD.GetInt("NDS-BOOTSTRAP", "LOGGING", 0)); 
						}
						bootstrapini.SaveIniFile( bootstrapinipath );

						CheatCodelist codelist;
						u32 gameCode,crc32;

						if ((isDSiMode() || !secondaryDevice) && !isHomebrew) {
							bool cheatsEnabled = true;
							const char* cheatDataBin = "/_nds/nds-bootstrap/cheatData.bin";
							mkdir("/_nds", 0777);
							mkdir("/_nds/nds-bootstrap", 0777);
							if(codelist.romData(path,gameCode,crc32)) {
								long cheatOffset; size_t cheatSize;
								FILE* dat=fopen(sdFound() ? "sd:/_nds/TWiLightMenu/extras/usrcheat.dat" : "fat:/_nds/TWiLightMenu/extras/usrcheat.dat","rb");
								if (dat) {
									if (codelist.searchCheatData(dat, gameCode,
												     crc32, cheatOffset,
												     cheatSize)) {
										codelist.parse(path);
										writeCheatsToFile(codelist.getCheats(), cheatDataBin);
										FILE* cheatData=fopen(cheatDataBin,"rb");
										if (cheatData) {
											u32 check[2];
											fread(check, 1, 8, cheatData);
											fclose(cheatData);
											if (check[1] == 0xCF000000
											|| getFileSize(cheatDataBin) > 0x8000) {
												cheatsEnabled = false;
											}
										}
									} else {
										cheatsEnabled = false;
									}
									fclose(dat);
								} else {
									cheatsEnabled = false;
								}
							} else {
								cheatsEnabled = false;
							}
							if (!cheatsEnabled) {
								remove(cheatDataBin);
							}
						}

						if (!isArgv) {
							romPath[secondaryDevice] = argarray[0];
						}
						homebrewHasWide = (isHomebrew && (game_TID[0] == 'W' || romVersion == 0x57));
						launchType[secondaryDevice] = 1;
						previousUsedDevice = secondaryDevice;
						SaveSettings();

						if (isDSiMode() || !secondaryDevice) {
							SetWidescreen(filename.c_str());
						}
						if (!isDSiMode() && !secondaryDevice) {
							ntrStartSdGame();
						}

						bool useNightly = (perGameSettings_bootstrapFile == -1 ? bootstrapFile : perGameSettings_bootstrapFile);

						char ndsToBoot[256];
						sprintf(ndsToBoot, "sd:/_nds/nds-bootstrap-%s%s.nds", homebrewBootstrap ? "hb-" : "", useNightly ? "nightly" : "release");
						if(!isDSiMode() || access(ndsToBoot, F_OK) != 0) {
							sprintf(ndsToBoot, "fat:/_nds/%s-%s%s.nds", isDSiMode() ? "nds-bootstrap" : "b4ds", homebrewBootstrap ? "hb-" : "", useNightly ? "nightly" : "release");
						}

						argarray.at(0) = (char *)ndsToBoot;
						int err = runNdsFile (argarray[0], argarray.size(), (const char **)&argarray[0], (homebrewBootstrap ? false : true), true, false, true, true);
						char text[32];
						snprintf (text, sizeof(text), "Start failed. Error %i", err);
						clearText();
						dialogboxHeight = (err==1 ? 2 : 0);
						showdialogbox = true;
						printLargeCentered(false, 74, "Error!");
						printSmallCentered(false, 90, text);
						if (err == 1) {
							printSmallCentered(false, 4, 102, useNightly ? "nds-bootstrap (Nightly)" : "nds-bootstrap (Release)");
							printSmallCentered(false, 4, 114, "not found.");
						}
						printSmallCentered(false, (err==1 ? 132 : 108), "\u2428 Back");
						int pressed = 0;
						do {
							scanKeys();
							pressed = keysDownRepeat();
							checkSdEject();
							swiWaitForVBlank();
						} while (!(pressed & KEY_B));
						if (!isDSiMode()) {
							chdir("fat:/");
						} else if (sdFound()) {
							chdir("sd:/");
						}
						runNdsFile("/_nds/TWiLightMenu/r4menu.srldr", 0, NULL, true, true, false, true, true);
					} else {
						romPath[secondaryDevice] = argarray[0];
						launchType[secondaryDevice] = 1;
						previousUsedDevice = secondaryDevice;
						SaveSettings();
						loadGameOnFlashcard(argarray[0], true);
					}
				} else {
					if (!isArgv) {
						romPath[secondaryDevice] = argarray[0];
					}
					homebrewHasWide = (isHomebrew && (game_TID[0] == 'W' || romVersion == 0x57));
					launchType[secondaryDevice] = 2;
					previousUsedDevice = secondaryDevice;
					if (isDSiMode() || !secondaryDevice) {
						SetWidescreen(filename.c_str());
					}
					SaveSettings();

					if (!isDSiMode() && !secondaryDevice) {
						ntrStartSdGame();
					}

					bool useWidescreen = (perGameSettings_wideScreen == -1 ? wideScreen : perGameSettings_wideScreen);

					if (consoleModel >= 2 && useWidescreen && homebrewHasWide) {
						argarray.push_back((char*)"wide");
					}

					bool runNds_boostCpu = false;
					bool runNds_boostVram = false;
					if (REG_SCFG_EXT != 0 && !dsModeDSiWare) {
						loadPerGameSettings(filename);

						runNds_boostCpu = perGameSettings_boostCpu == -1 ? boostCpu : perGameSettings_boostCpu;
						runNds_boostVram = perGameSettings_boostVram == -1 ? boostVram : perGameSettings_boostVram;
					}
					int err = runNdsFile (argarray[0], argarray.size(), (const char **)&argarray[0], true, true, dsModeSwitch, runNds_boostCpu, runNds_boostVram);
					char text[32];
					snprintf (text, sizeof(text), "Start failed. Error %i", err);
					clearText();
					dialogboxHeight = 0;
					showdialogbox = true;
					printLargeCentered(false, 74, "Error!");
					printSmallCentered(false, 90, text);
					printSmallCentered(false, 108, "\u2428 Back");
					int pressed = 0;
					do {
						scanKeys();
						pressed = keysDownRepeat();
						checkSdEject();
						swiWaitForVBlank();
					} while (!(pressed & KEY_B));
					if (!isDSiMode()) {
						chdir("fat:/");
					} else if (sdFound()) {
						chdir("sd:/");
					}
					runNdsFile("/_nds/TWiLightMenu/r4menu.srldr", 0, NULL, true, true, false, true, true);
				}
			} else {
				bool useNDSB = false;
				bool dsModeSwitch = false;
				bool boostCpu = true;
				bool boostVram = false;

				std::string romfolderNoSlash = romfolder[secondaryDevice];
				RemoveTrailingSlashes(romfolderNoSlash);
				char ROMpath[256];
				snprintf (ROMpath, sizeof(ROMpath), "%s/%s", romfolderNoSlash.c_str(), filename.c_str());
				romPath[secondaryDevice] = ROMpath;
				previousUsedDevice = secondaryDevice;
				homebrewBootstrap = true;

				const char *ndsToBoot = "sd:/_nds/nds-bootstrap-release.nds";
				if (extention(filename, ".plg")) {
					ndsToBoot = "/_nds/TWiLightMenu/bootplg.srldr";

					// Print .plg path without "fat:" at the beginning
					char ROMpathDS2[256];
					if (secondaryDevice) {
						for (int i = 0; i < 252; i++) {
							ROMpathDS2[i] = ROMpath[4+i];
							if (ROMpath[4+i] == '\x00') break;
						}
					} else {
						sprintf(ROMpathDS2, "/_nds/TWiLightMenu/tempPlugin.plg");
						fcopy(ROMpath, "fat:/_nds/TWiLightMenu/tempPlugin.plg");
					}

					CIniFile dstwobootini( "fat:/_dstwo/twlm.ini" );
					dstwobootini.SetString("boot_settings", "file", ROMpathDS2);
					dstwobootini.SaveIniFile( "fat:/_dstwo/twlm.ini" );
				} else if (extention(filename, ".rvid")) {
					launchType[secondaryDevice] = 7;

					ndsToBoot = "sd:/_nds/TWiLightMenu/apps/RocketVideoPlayer.nds";
					if(!isDSiMode() || access(ndsToBoot, F_OK) != 0) {
						ndsToBoot = "fat:/_nds/TWiLightMenu/apps/RocketVideoPlayer.nds";
						boostVram = true;
					}
				} else if (extention(filename, ".mp4")) {
					launchType[secondaryDevice] = 8;

					ndsToBoot = "sd:/_nds/TWiLightMenu/apps/MPEG4Player.nds";
					if(!isDSiMode() || access(ndsToBoot, F_OK) != 0) {
						ndsToBoot = "fat:/_nds/TWiLightMenu/apps/MPEG4Player.nds";
						boostVram = true;
					}
				} else if (extention(filename, ".gba")) {
					launchType[secondaryDevice] = 1;

					if (secondaryDevice) {
						ndsToBoot = gbar2DldiAccess ? "sd:/_nds/GBARunner2_arm7dldi_ds.nds" : "sd:/_nds/GBARunner2_arm9dldi_ds.nds";
						if (REG_SCFG_EXT != 0) {
							ndsToBoot = consoleModel>0 ? "sd:/_nds/GBARunner2_arm7dldi_3ds.nds" : "sd:/_nds/GBARunner2_arm7dldi_dsi.nds";
						}
						if(!isDSiMode() || access(ndsToBoot, F_OK) != 0) {
							ndsToBoot = gbar2DldiAccess ? "fat:/_nds/GBARunner2_arm7dldi_ds.nds" : "fat:/_nds/GBARunner2_arm9dldi_ds.nds";
							if (REG_SCFG_EXT != 0) {
								ndsToBoot = consoleModel>0 ? "fat:/_nds/GBARunner2_arm7dldi_3ds.nds" : "fat:/_nds/GBARunner2_arm7dldi_dsi.nds";
							}
						}
						boostVram = false;
					} else {
						useNDSB = true;

						const char* gbar2Path = consoleModel>0 ? "sd:/_nds/GBARunner2_arm7dldi_3ds.nds" : "sd:/_nds/GBARunner2_arm7dldi_dsi.nds";
						if (isDSiMode() && arm7SCFGLocked) {
							gbar2Path = consoleModel>0 ? "sd:/_nds/GBARunner2_arm7dldi_nodsp_3ds.nds" : "sd:/_nds/GBARunner2_arm7dldi_nodsp_dsi.nds";
						}

						ndsToBoot = (bootstrapFile ? "sd:/_nds/nds-bootstrap-hb-nightly.nds" : "sd:/_nds/nds-bootstrap-hb-release.nds");
						CIniFile bootstrapini("sd:/_nds/nds-bootstrap.ini");

						bootstrapini.SetInt("NDS-BOOTSTRAP", "LANGUAGE", gameLanguage);
						bootstrapini.SetInt("NDS-BOOTSTRAP", "DSI_MODE", 0);
						bootstrapini.SetString("NDS-BOOTSTRAP", "NDS_PATH", gbar2Path);
						bootstrapini.SetString("NDS-BOOTSTRAP", "HOMEBREW_ARG", ROMpath);
						bootstrapini.SetString("NDS-BOOTSTRAP", "RAM_DRIVE_PATH", "");
						bootstrapini.SetInt("NDS-BOOTSTRAP", "BOOST_CPU", 1);
						bootstrapini.SetInt("NDS-BOOTSTRAP", "BOOST_VRAM", 0);

						bootstrapini.SaveIniFile("sd:/_nds/nds-bootstrap.ini");
					}
				} else if (extention(filename, ".a26")) {
					launchType[secondaryDevice] = 9;
					
					ndsToBoot = "sd:/_nds/TWiLightMenu/emulators/StellaDS.nds";
					if(!isDSiMode() || access(ndsToBoot, F_OK) != 0) {
						ndsToBoot = "fat:/_nds/TWiLightMenu/emulators/StellaDS.nds";
						boostVram = true;
					}
				} else if (extention(filename, ".gb") || extention(filename, ".sgb") || extention(filename, ".gbc")) {
					launchType[secondaryDevice] = 5;
					
					ndsToBoot = "sd:/_nds/TWiLightMenu/emulators/gameyob.nds";
					if(!isDSiMode() || access(ndsToBoot, F_OK) != 0) {
						ndsToBoot = "fat:/_nds/TWiLightMenu/emulators/gameyob.nds";
						dsModeSwitch = !isDSiMode();
						boostVram = true;
					}
				} else if (extention(filename, ".nes") || extention(filename, ".fds")) {
					launchType[secondaryDevice] = 4;

					ndsToBoot = (secondaryDevice ? "sd:/_nds/TWiLightMenu/emulators/nesds.nds" : "sd:/_nds/TWiLightMenu/emulators/nestwl.nds");
					if(!isDSiMode() || access(ndsToBoot, F_OK) != 0) {
						ndsToBoot = "fat:/_nds/TWiLightMenu/emulators/nesds.nds";
						boostVram = true;
					}
				} else if (extention(filename, ".sms") || extention(filename, ".gg")) {
					mkdir(secondaryDevice ? "fat:/data" : "sd:/data", 0777);
					mkdir(secondaryDevice ? "fat:/data/s8ds" : "sd:/data/s8ds", 0777);

					if (!secondaryDevice && !arm7SCFGLocked && smsGgInRam) {
						launchType[secondaryDevice] = 1;

						useNDSB = true;

						ndsToBoot = (bootstrapFile ? "sd:/_nds/nds-bootstrap-hb-nightly.nds" : "sd:/_nds/nds-bootstrap-hb-release.nds");
						CIniFile bootstrapini("sd:/_nds/nds-bootstrap.ini");

						bootstrapini.SetInt("NDS-BOOTSTRAP", "LANGUAGE", gameLanguage);
						bootstrapini.SetInt("NDS-BOOTSTRAP", "DSI_MODE", 0);
						bootstrapini.SetString("NDS-BOOTSTRAP", "NDS_PATH", "sd:/_nds/TWiLightMenu/emulators/S8DS07.nds");
						bootstrapini.SetString("NDS-BOOTSTRAP", "HOMEBREW_ARG", "");
						bootstrapini.SetInt("NDS-BOOTSTRAP", "BOOST_CPU", 1);
						bootstrapini.SetInt("NDS-BOOTSTRAP", "BOOST_VRAM", 0);

						bootstrapini.SetString("NDS-BOOTSTRAP", "RAM_DRIVE_PATH", ROMpath);
						bootstrapini.SaveIniFile("sd:/_nds/nds-bootstrap.ini");
					} else {
						launchType[secondaryDevice] = 6;

						ndsToBoot = "sd:/_nds/TWiLightMenu/emulators/S8DS.nds";
						if(!isDSiMode() || access(ndsToBoot, F_OK) != 0) {
							ndsToBoot = "fat:/_nds/TWiLightMenu/emulators/S8DS.nds";
							boostVram = true;
						}
					}
				} else if (extention(filename, ".gen")) {
					bool usePicoDrive = ((isDSiMode() && sdFound() && arm7SCFGLocked)
						|| showMd==2 || (showMd==3 && getFileSize(filename.c_str()) > 0x300000));
					launchType[secondaryDevice] = (usePicoDrive ? 10 : 1);

					if (usePicoDrive || secondaryDevice) {
						ndsToBoot = usePicoDrive ? "sd:/_nds/TWiLightMenu/emulators/PicoDriveTWL.nds" : "sd:/_nds/TWiLightMenu/emulators/jEnesisDS.nds";
						if(!isDSiMode() || access(ndsToBoot, F_OK) != 0) {
							ndsToBoot = usePicoDrive ? "fat:/_nds/TWiLightMenu/emulators/PicoDriveTWL.nds" : "fat:/_nds/TWiLightMenu/emulators/jEnesisDS.nds";
							boostVram = true;
						}
						dsModeSwitch = !usePicoDrive;
					} else {
						useNDSB = true;

						ndsToBoot = (bootstrapFile ? "sd:/_nds/nds-bootstrap-hb-nightly.nds" : "sd:/_nds/nds-bootstrap-hb-release.nds");
						CIniFile bootstrapini("sd:/_nds/nds-bootstrap.ini");

						bootstrapini.SetInt("NDS-BOOTSTRAP", "LANGUAGE", gameLanguage);
						bootstrapini.SetInt("NDS-BOOTSTRAP", "DSI_MODE", 0);
						bootstrapini.SetString("NDS-BOOTSTRAP", "NDS_PATH", "sd:/_nds/TWiLightMenu/emulators/jEnesisDS.nds");
						bootstrapini.SetString("NDS-BOOTSTRAP", "HOMEBREW_ARG", "fat:/ROM.BIN");
						bootstrapini.SetInt("NDS-BOOTSTRAP", "BOOST_CPU", 1);
						bootstrapini.SetInt("NDS-BOOTSTRAP", "BOOST_VRAM", 0);

						bootstrapini.SetString("NDS-BOOTSTRAP", "RAM_DRIVE_PATH", ROMpath);
						bootstrapini.SaveIniFile("sd:/_nds/nds-bootstrap.ini");
					}
				} else if (extention(filename, ".smc") || extention(filename, ".sfc")) {
					launchType[secondaryDevice] = 1;

					if (secondaryDevice) {
						ndsToBoot = "sd:/_nds/TWiLightMenu/emulators/SNEmulDS.nds";
						if(!isDSiMode() || access(ndsToBoot, F_OK) != 0) {
							ndsToBoot = "fat:/_nds/TWiLightMenu/emulators/SNEmulDS.nds";
							boostCpu = false;
							boostVram = true;
						}
						dsModeSwitch = true;
					} else {
						useNDSB = true;

						ndsToBoot = (bootstrapFile ? "sd:/_nds/nds-bootstrap-hb-nightly.nds" : "sd:/_nds/nds-bootstrap-hb-release.nds");
						CIniFile bootstrapini("sd:/_nds/nds-bootstrap.ini");

						bootstrapini.SetInt("NDS-BOOTSTRAP", "LANGUAGE", gameLanguage);
						bootstrapini.SetInt("NDS-BOOTSTRAP", "DSI_MODE", 0);
						bootstrapini.SetString("NDS-BOOTSTRAP", "NDS_PATH", "sd:/_nds/TWiLightMenu/emulators/SNEmulDS.nds");
						bootstrapini.SetString("NDS-BOOTSTRAP", "HOMEBREW_ARG", "fat:/snes/ROM.SMC");
						bootstrapini.SetInt("NDS-BOOTSTRAP", "BOOST_CPU", 0);
						bootstrapini.SetInt("NDS-BOOTSTRAP", "BOOST_VRAM", 0);

						bootstrapini.SetString("NDS-BOOTSTRAP", "RAM_DRIVE_PATH", ROMpath);
						bootstrapini.SaveIniFile("sd:/_nds/nds-bootstrap.ini");
					}
				} else if (extention(filename, ".pce")) {
					launchType[secondaryDevice] = 1;

					if (secondaryDevice) {
						ndsToBoot = "sd:/_nds/TWiLightMenu/emulators/NitroGrafx.nds";
						if(access(ndsToBoot, F_OK) != 0) {
							ndsToBoot = "/_nds/TWiLightMenu/emulators/NitroGrafx.nds";
							boostVram = true;
						}
						dsModeSwitch = true;
					} else {
						useNDSB = true;

						ndsToBoot = (bootstrapFile ? "sd:/_nds/nds-bootstrap-hb-nightly.nds" : "sd:/_nds/nds-bootstrap-hb-release.nds");
						CIniFile bootstrapini("sd:/_nds/nds-bootstrap.ini");

						bootstrapini.SetInt("NDS-BOOTSTRAP", "LANGUAGE", gameLanguage);
						bootstrapini.SetInt("NDS-BOOTSTRAP", "DSI_MODE", 0);
						bootstrapini.SetString("NDS-BOOTSTRAP", "NDS_PATH", "sd:/_nds/TWiLightMenu/emulators/NitroGrafx.nds");
						bootstrapini.SetString("NDS-BOOTSTRAP", "HOMEBREW_ARG", ROMpath);
						bootstrapini.SetInt("NDS-BOOTSTRAP", "BOOST_CPU", 0);
						bootstrapini.SetInt("NDS-BOOTSTRAP", "BOOST_VRAM", 0);

						bootstrapini.SetString("NDS-BOOTSTRAP", "RAM_DRIVE_PATH", "");
						bootstrapini.SaveIniFile("sd:/_nds/nds-bootstrap.ini");
					}
				}

				SaveSettings();
				if (!isDSiMode() && !secondaryDevice && !extention(filename, ".plg")) {
					ntrStartSdGame();
				}
				argarray.push_back(ROMpath);
				argarray.at(0) = (char *)ndsToBoot;
				int err = runNdsFile (argarray[0], argarray.size(), (const char **)&argarray[0], !useNDSB, true, dsModeSwitch, boostCpu, boostVram);	// Pass ROM to emulator as argument
				char text[32];
				snprintf (text, sizeof(text), "Start failed. Error %i", err);
				clearText();
				dialogboxHeight = (err==1 ? 2 : 0);
				showdialogbox = true;
				printLargeCentered(false, 74, "Error!");
				printSmallCentered(false, 90, text);
				if (err == 1 && useNDSB) {
					printSmallCentered(false, 4, 102, bootstrapFile ? "nds-bootstrap (Nightly)" : "nds-bootstrap (Release)");
					printSmallCentered(false, 4, 114, "not found.");
				}
				printSmallCentered(false, (err==1 ? 132 : 108), "\u2428 Back");
				int pressed = 0;
				do {
					scanKeys();
					pressed = keysDownRepeat();
					checkSdEject();
					swiWaitForVBlank();
				} while (!(pressed & KEY_B));
				if (!isDSiMode()) {
					chdir("fat:/");
				} else if (sdFound()) {
					chdir("sd:/");
				}
				runNdsFile("/_nds/TWiLightMenu/r4menu.srldr", 0, NULL, true, true, false, true, true);
			}

			while(argarray.size() !=0 ) {
				free(argarray.at(0));
				argarray.erase(argarray.begin());
			}
		}
	}

	return 0;
}
