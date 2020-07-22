#include <maxmod9.h>
#include <nds.h>
#include <nds/arm9/dldi.h>

#include <fat.h>
#include <limits.h>
#include <stdio.h>
#include <sys/stat.h>

#include <gl2d.h>
#include <string.h>
#include <unistd.h>

#include "date.h"
#include "fileCopy.h"

#include "graphics/graphics.h"

#include "common/dsimenusettings.h"
#include "common/flashcard.h"
#include "common/nitrofs.h"
#include "common/systemdetails.h"
#include "graphics/ThemeConfig.h"
#include "graphics/ThemeTextures.h"
#include "graphics/themefilenames.h"

#include "errorScreen.h"
#include "fileBrowse.h"
#include "nds_loader_arm9.h"
#include "gbaswitch.h"
#include "ndsheaderbanner.h"
#include "perGameSettings.h"

#include "graphics/fontHandler.h"
#include "graphics/iconHandler.h"

#include "common/inifile.h"
#include "tool/stringtool.h"
#include "common/tonccpy.h"

#include "sound.h"
#include "language.h"

#include "cheat.h"
#include "crc.h"

#include "donorMap.h"
#include "mpuMap.h"
#include "speedBumpExcludeMap.h"
#include "saveMap.h"

#include "sr_data_srllastran.h"		 // For rebooting into the game

bool useTwlCfg = false;

bool whiteScreen = true;
bool fadeType = false; // false = out, true = in
bool fadeSpeed = true; // false = slow (for DSi launch effect), true = fast
bool fadeColor = true; // false = black, true = white
bool controlTopBright = true;
bool controlBottomBright = true;

extern void ClearBrightness();
extern bool displayGameIcons;
extern bool showProgressIcon;

const char *settingsinipath = "sd:/_nds/TWiLightMenu/settings.ini";
const char *bootstrapinipath = "sd:/_nds/nds-bootstrap.ini";

const char *unlaunchAutoLoadID = "AutoLoadInfo";
static char hiyaNdsPath[14] = {'s', 'd', 'm', 'c', ':', '/', 'h', 'i', 'y', 'a', '.', 'd', 's', 'i'};
char unlaunchDevicePath[256];

/**
 * Remove trailing slashes from a pathname, if present.
 * @param path Pathname to modify.
 */
void RemoveTrailingSlashes(std::string &path) {
	if (path.size() == 0) return;
	while (!path.empty() && path[path.size() - 1] == '/') {
		path.resize(path.size() - 1);
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

// These are used by flashcard functions and must retain their trailing slash.
static const std::string slashchar = "/";
static const std::string woodfat = "fat0:/";
static const std::string dstwofat = "fat1:/";

typedef TWLSettings::TLaunchType Launch;

int mpuregion = 0;
int mpusize = 0;
bool ceCached = true;

bool applaunch = false;

bool gbaBiosFound[2] = {false};

bool useBackend = false;

bool dropDown = false;
int currentBg = 0;
bool showSTARTborder = false;
bool buttonArrowTouched[2] = {false};
bool scrollWindowTouched = false;

bool titleboxXmoveleft = false;
bool titleboxXmoveright = false;

bool applaunchprep = false;

int spawnedtitleboxes = 0;

s16 usernameRendered[11] = {0};
bool usernameRenderedDone = false;

bool showColon = true;

touchPosition touch;

//---------------------------------------------------------------------------------
void stop(void) {
	//---------------------------------------------------------------------------------
	while (1) {
		swiWaitForVBlank();
	}
}

/**
 * Set donor SDK version for a specific game.
 */
int SetDonorSDK() {
	char gameTid3[5];
	for (int i = 0; i < 3; i++) {
		gameTid3[i] = gameTid[CURPOS][i];
	}

	for (auto i : donorMap) {
		if (i.first == 5 && gameTid3[0] == 'V')
			return 5;

		if (i.second.find(gameTid3) != i.second.cend())
			return i.first;
	}

	return 0;
}

/**
 * Set MPU settings for a specific game.
 */
void SetMPUSettings(const char *filename) {
	scanKeys();
	int pressed = keysHeld();

	if (pressed & KEY_B) {
		mpuregion = 1;
	} else if (pressed & KEY_X) {
		mpuregion = 2;
	} else if (pressed & KEY_Y) {
		mpuregion = 3;
	} else {
		mpuregion = 0;
	}
	if (pressed & KEY_RIGHT) {
		mpusize = 3145728;
	} else if (pressed & KEY_LEFT) {
		mpusize = 1;
	} else {
		mpusize = 0;
	}

	// Check for games that need an MPU size of 3 MB.
	// TODO: If the list gets large enough, switch to bsearch().
	for (unsigned int i = 0; i < sizeof(mpu_3MB_list) / sizeof(mpu_3MB_list[0]); i++) {
		if (memcmp(gameTid[CURPOS], mpu_3MB_list[i], 3) == 0) {
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
void SetSpeedBumpExclude(void) {
	if (!isDSiMode() || (perGameSettings_heapShrink >= 0 && perGameSettings_heapShrink < 2)) {
		ceCached = perGameSettings_heapShrink;
		return;
	}

	// TODO: If the list gets large enough, switch to bsearch().
	for (unsigned int i = 0; i < sizeof(sbeList2)/sizeof(sbeList2[0]); i++) {
		if (memcmp(gameTid[CURPOS], sbeList2[i], 3) == 0) {
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
		snprintf(ipsPath, sizeof(ipsPath), "%s:/_nds/TWiLightMenu/apfix/%s-%X.ips", sdFound() ? "sd" : "fat", gameTid[CURPOS], headerCRC[CURPOS]);
		ipsFound = (access(ipsPath, F_OK) == 0);
	}

	if (ipsFound) {
		if (ms().secondaryDevice && sdFound()) {
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
TWL_CODE void SetWidescreen(const char *filename) {
	remove("/_nds/nds-bootstrap/wideCheatData.bin");

	bool useWidescreen = (perGameSettings_wideScreen == -1 ? ms().wideScreen : perGameSettings_wideScreen);

	if (sys().arm7SCFGLocked() || ms().consoleModel < 2 || !useWidescreen
	|| (access("sd:/_nds/TWiLightMenu/TwlBg/Widescreen.cxi", F_OK) != 0)) {
		return;
	}
	
	bool wideCheatFound = false;
	char wideBinPath[256];
	if (ms().launchType[ms().secondaryDevice] == Launch::ESDFlashcardLaunch) {
		snprintf(wideBinPath, sizeof(wideBinPath), "sd:/_nds/TWiLightMenu/widescreen/%s.bin", filename);
		wideCheatFound = (access(wideBinPath, F_OK) == 0);
	}

	if (ms().slot1Launched) {
		// Reset Slot-1 to allow reading card header
		sysSetCardOwner (BUS_OWNER_ARM9);
		disableSlot1();
		for(int i = 0; i < 25; i++) { swiWaitForVBlank(); }
		enableSlot1();
		for(int i = 0; i < 15; i++) { swiWaitForVBlank(); }

		cardReadHeader((uint8*)&ndsCart);

		char s1GameTid[5];
		tonccpy(s1GameTid, ndsCart.gameCode, 4);
		s1GameTid[4] = 0;

		snprintf(wideBinPath, sizeof(wideBinPath), "sd:/_nds/TWiLightMenu/widescreen/%s-%X.bin", s1GameTid, ndsCart.headerCRC16);
		wideCheatFound = (access(wideBinPath, F_OK) == 0);
	} else if (!wideCheatFound) {
		snprintf(wideBinPath, sizeof(wideBinPath), "sd:/_nds/TWiLightMenu/widescreen/%s-%X.bin", gameTid[CURPOS], headerCRC[CURPOS]);
		wideCheatFound = (access(wideBinPath, F_OK) == 0);
	}

	if (isHomebrew[CURPOS]) {
		if (!ms().homebrewHasWide) return;

		std::string resultText;
		// Prepare for reboot into 16:10 TWL_FIRM
		mkdir("sd:/luma", 0777);
		mkdir("sd:/luma/sysmodules", 0777);
		if ((access("sd:/luma/sysmodules/TwlBg.cxi", F_OK) == 0)
		&& (rename("sd:/luma/sysmodules/TwlBg.cxi", "sd:/luma/sysmodules/TwlBg_bak.cxi") != 0)) {
			resultText = STR_FAILED_TO_BACKUP_TWLBG;
		} else {
			if (fcopy("sd:/_nds/TWiLightMenu/TwlBg/Widescreen.cxi", "sd:/luma/sysmodules/TwlBg.cxi") == 0) {
				irqDisable(IRQ_VBLANK);				// Fix the throwback to 3DS HOME Menu bug
				tonccpy((u32 *)0x02000300, sr_data_srllastran, 0x020);
				fifoSendValue32(FIFO_USER_02, 1); // Reboot in 16:10 widescreen
				stop();
			} else {
				resultText = STR_FAILED_TO_REBOOT_TWLBG;
			}
		}
		rename("sd:/luma/sysmodules/TwlBg_bak.cxi", "sd:/luma/sysmodules/TwlBg.cxi");
		clearText();
		printLarge(false, 0, ms().theme == 4 ? 24 : 72, resultText, Alignment::center);
		if (ms().theme != 4) {
			fadeType = true; // Fade in from white
		}
		for (int i = 0; i < 60 * 3; i++) {
			swiWaitForVBlank(); // Wait 3 seconds
		}
		if (ms().theme != 4) {
			fadeType = false;	   // Fade to white
			for (int i = 0; i < 25; i++) {
				swiWaitForVBlank();
			}
		}
		return;
	}

	if (wideCheatFound) {
		std::string resultText;
		mkdir("/_nds", 0777);
		mkdir("/_nds/nds-bootstrap", 0777);
		if (fcopy(wideBinPath, "/_nds/nds-bootstrap/wideCheatData.bin") == 0) {
			// Prepare for reboot into 16:10 TWL_FIRM
			mkdir("sd:/luma", 0777);
			mkdir("sd:/luma/sysmodules", 0777);
			if ((access("sd:/luma/sysmodules/TwlBg.cxi", F_OK) == 0)
			&& (rename("sd:/luma/sysmodules/TwlBg.cxi", "sd:/luma/sysmodules/TwlBg_bak.cxi") != 0)) {
				resultText = STR_FAILED_TO_BACKUP_TWLBG;
			} else {
				if (fcopy("sd:/_nds/TWiLightMenu/TwlBg/Widescreen.cxi", "sd:/luma/sysmodules/TwlBg.cxi") == 0) {
					irqDisable(IRQ_VBLANK);				// Fix the throwback to 3DS HOME Menu bug
					tonccpy((u32 *)0x02000300, sr_data_srllastran, 0x020);
					fifoSendValue32(FIFO_USER_02, 1); // Reboot in 16:10 widescreen
					stop();
				} else {
					resultText = STR_FAILED_TO_REBOOT_TWLBG;
				}
			}
			rename("sd:/luma/sysmodules/TwlBg_bak.cxi", "sd:/luma/sysmodules/TwlBg.cxi");
		} else {
			resultText = STR_FAILED_TO_COPY_WIDESCREEN;
		}
		remove("/_nds/nds-bootstrap/wideCheatData.bin");
		clearText();
		printLarge(false, 0, ms().theme == 4 ? 24 : 72, resultText, Alignment::center);
		if (ms().theme != 4) {
			fadeType = true; // Fade in from white
		}
		for (int i = 0; i < 60 * 3; i++) {
			swiWaitForVBlank(); // Wait 3 seconds
		}
		if (ms().theme != 4) {
			fadeType = false;	   // Fade to white
			for (int i = 0; i < 25; i++) {
				swiWaitForVBlank();
			}
		}
	}
}

char filePath[PATH_MAX];

void doPause() {
	while (1) {
		scanKeys();
		if (keysDown() & KEY_START)
			break;
		snd().updateStream();
		swiWaitForVBlank();
	}
	scanKeys();
}

void loadGameOnFlashcard (const char *ndsPath, bool usePerGameSettings) {
	bool runNds_boostCpu = false;
	bool runNds_boostVram = false;
	if (isDSiMode() && usePerGameSettings) {
		std::string filename = ndsPath;

		const size_t last_slash_idx = filename.find_last_of("/");
		if (std::string::npos != last_slash_idx) {
			filename.erase(0, last_slash_idx + 1);
		}

		loadPerGameSettings(filename);

		runNds_boostCpu = perGameSettings_boostCpu == -1 ? ms().boostCpu : perGameSettings_boostCpu;
		runNds_boostVram = perGameSettings_boostVram == -1 ? ms().boostVram : perGameSettings_boostVram;
	}
	std::string path;
	int err = 0;
	snd().stopStream();

	if ((memcmp(io_dldi_data->friendlyName, "R4(DS) - Revolution for DS", 26) == 0)
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
	} /*else if ((memcmp(io_dldi_data->friendlyName, "TTCARD", 6) == 0)
			 || (memcmp(io_dldi_data->friendlyName, "DSTT", 4) == 0)
			 || (memcmp(io_dldi_data->friendlyName, "DEMON", 5) == 0) {
		CIniFile fcrompathini("fat:/TTMenu/YSMenu.ini");
		path = replaceAll(ndsPath, "fat:/", slashchar);
		fcrompathini.SetString("YSMENU", "AUTO_BOOT", path);
		fcrompathini.SaveIniFile("fat:/TTMenu/YSMenu.ini");
		err = runNdsFile("fat:/YSMenu.nds", 0, NULL, true, true, true, runNds_boostCpu, runNds_boostVram);
	}*/

	char text[64];
	snprintf(text, sizeof(text), STR_START_FAILED_ERROR.c_str(), err);
	fadeType = true;	// Fade from white
	if (err == 0) {
		printLarge(false, 4, 4, STR_ERROR_FLASHCARD_UNSUPPORTED);
		printLarge(false, 4, 68, io_dldi_data->friendlyName);
	} else {
		printLarge(false, 4, 4, text);
	}
	printSmall(false, 4, 90, STR_PRESS_B_RETURN);
	int pressed = 0;
	do {
		scanKeys();
		pressed = keysDownRepeat();
		checkSdEject();
		swiWaitForVBlank();
	} while (!(pressed & KEY_B));
	fadeType = false;	// Fade to white
	for (int i = 0; i < 25; i++) {
		swiWaitForVBlank();
	}
	if (sdFound()) {
		chdir("sd:/");
	}
	runNdsFile("/_nds/TWiLightMenu/dsimenu.srldr", 0, NULL, true, false, false, true, true);
	stop();
}

void unlaunchRomBoot(const char* rom) {
	snd().stopStream();
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
	snd().stopStream();
	tonccpy((u8 *)0x02000800, unlaunchAutoLoadID, 12);
	*(u16 *)(0x0200080C) = 0x3F0;			   // Unlaunch Length for CRC16 (fixed, must be 3F0h)
	*(u16 *)(0x0200080E) = 0;			   // Unlaunch CRC16 (empty)
	*(u32 *)(0x02000810) = (BIT(0) | BIT(1));	  // Load the title at 2000838h
							   // Use colors 2000814h
	*(u16 *)(0x02000814) = 0x7FFF;			   // Unlaunch Upper screen BG color (0..7FFFh)
	*(u16 *)(0x02000816) = 0x7FFF;			   // Unlaunch Lower screen BG color (0..7FFFh)
	toncset((u8 *)0x02000818, 0, 0x20 + 0x208 + 0x1C0); // Unlaunch Reserved (zero)
	int i2 = 0;
	for (int i = 0; i < 14; i++) {
		*(u8 *)(0x02000838 + i2) =
		    hiyaNdsPath[i]; // Unlaunch Device:/Path/Filename.ext (16bit Unicode,end by 0000h)
		i2 += 2;
	}
	while (*(vu16 *)(0x0200080E) == 0) { // Keep running, so that CRC16 isn't 0
		*(u16 *)(0x0200080E) = swiCRC16(0xFFFF, (void *)0x02000810, 0x3F0); // Unlaunch CRC16
	}
}

void dsCardLaunch() {
	snd().stopStream();
	*(u32 *)(0x02000300) = 0x434E4C54; // Set "CNLT" warmboot flag
	*(u16 *)(0x02000304) = 0x1801;
	*(u32 *)(0x02000308) = 0x43415254; // "CART"
	*(u32 *)(0x0200030C) = 0x00000000;
	*(u32 *)(0x02000310) = 0x43415254; // "CART"
	*(u32 *)(0x02000314) = 0x00000000;
	*(u32 *)(0x02000318) = 0x00000013;
	*(u32 *)(0x0200031C) = 0x00000000;
	while (*(u16 *)(0x02000306) == 0) { // Keep running, so that CRC16 isn't 0
		*(u16 *)(0x02000306) = swiCRC16(0xFFFF, (void *)0x02000308, 0x18);
	}

	unlaunchSetHiyaBoot();

	DC_FlushAll();						// Make reboot not fail
	fifoSendValue32(FIFO_USER_02, 1); // Reboot into DSiWare title, booted via Launcher
	stop();
}

bool extention(const std::string& filename, const char* ext) {
	if(strcasecmp(filename.c_str() + filename.size() - strlen(ext), ext)) {
		return false;
	} else {
		return true;
	}
}

int main(int argc, char **argv) {
	/*SetBrightness(0, 0);
	SetBrightness(1, 0);
	consoleDemoInit();*/

	defaultExceptionHandler();
	sys().initFilesystem();
	sys().initArm7RegStatuses();

	if (access(settingsinipath, F_OK) != 0 && flashcardFound()) {
		settingsinipath =
		    "fat:/_nds/TWiLightMenu/settings.ini"; // Fallback to .ini path on flashcard, if not found on
							   // SD card, or if SD access is disabled
	}

	useTwlCfg = (isDSiMode() && (*(u8*)0x02000400 & 0x0F) && (*(u8*)0x02000401 == 0) && (*(u8*)0x02000402 == 0) && (*(u8*)0x02000404 == 0));

	ms().loadSettings();
	tfn(); //
	tc().loadConfig();
	tex().videoSetup(); // allocate texture pointers

	fontInit();

	if (ms().theme == 5) {
		tex().loadHBTheme();
	} else if (ms().theme == 4) {
		tex().loadSaturnTheme();
	} else if (ms().theme == 1) {
		tex().load3DSTheme();
	} else {
		tex().loadDSiTheme();
	}

	//printf("Username copied\n");
	tonccpy(usernameRendered, (useTwlCfg ? (s16*)0x02000448 : PersonalData->name), sizeof(s16) * 10);

	if (!sys().fatInitOk()) {
		graphicsInit();
		fontInit();
		whiteScreen = false;
		fadeType = true;
		for (int i = 0; i < 5; i++)
			swiWaitForVBlank();
		if (!dropDown && ms().theme == 0) {
			dropDown = true;
			for (int i = 0; i < 72; i++) 
				swiWaitForVBlank();
		} else {
			for (int i = 0; i < 25; i++)
				swiWaitForVBlank();
		}
		currentBg = 1;
		printLarge(false, 0, 32, "fatInitDefault failed!", Alignment::center);

		// Control the DSi Menu, but can't launch anything.
		int pressed = 0;

		while (1) {
			// Power saving loop. Only poll the keys once per frame and sleep the CPU if there is nothing
			// else to do
			do {
				scanKeys();
				pressed = keysDownRepeat();
				snd().updateStream();
				swiWaitForVBlank();
			} while (!pressed);

			if ((pressed & KEY_LEFT) && !titleboxXmoveleft && !titleboxXmoveright) {
				CURPOS -= 1;
				if (CURPOS >= 0)
					titleboxXmoveleft = true;
			} else if ((pressed & KEY_RIGHT) && !titleboxXmoveleft && !titleboxXmoveright) {
				CURPOS += 1;
				if (CURPOS <= 39)
					titleboxXmoveright = true;
			}
			if (CURPOS < 0) {
				CURPOS = 0;
			} else if (CURPOS > 39) {
				CURPOS = 39;
			}
		}
	}

	if (ms().theme == 4 || ms().theme == 5) {
		whiteScreen = false;
		fadeColor = false;
	}

	langInit();

	std::string filename;

	gbaBiosFound[0] = ((access("sd:/bios.bin", F_OK) == 0) || (access("sd:/gba/bios.bin", F_OK) == 0) || (access("sd:/_gba/bios.bin", F_OK) == 0));
	gbaBiosFound[1] = ((access("fat:/bios.bin", F_OK) == 0) || (access("fat:/gba/bios.bin", F_OK) == 0) || (access("fat:/_gba/bios.bin", F_OK) == 0));

	if (isDSiMode() && sdFound() && ms().consoleModel < 2 && ms().launcherApp != -1) {
		u8 setRegion = 0;

		if (ms().sysRegion == -1) {
			// Determine SysNAND region by searching region of System Settings on SDNAND
			char tmdpath[256] = {0};
			for (u8 i = 0x41; i <= 0x5A; i++) {
				snprintf(tmdpath, sizeof(tmdpath), "sd:/title/00030015/484e42%x/content/title.tmd", i);
				if (access(tmdpath, F_OK) == 0) {
					setRegion = i;
					break;
				}
			}
		} else {
			switch (ms().sysRegion) {
			case 0:
			default:
				setRegion = 0x4A; // JAP
				break;
			case 1:
				setRegion = 0x45; // USA
				break;
			case 2:
				setRegion = 0x50; // EUR
				break;
			case 3:
				setRegion = 0x55; // AUS
				break;
			case 4:
				setRegion = 0x43; // CHN
				break;
			case 5:
				setRegion = 0x4B; // KOR
				break;
			}
		}

		snprintf(unlaunchDevicePath, sizeof(unlaunchDevicePath),
			 "nand:/title/00030017/484E41%x/content/0000000%i.app", setRegion, ms().launcherApp);
	}

	graphicsInit();
	iconManagerInit();

	keysSetRepeat(10, 2);

	std::vector<std::string> extensionList;
	if (ms().showNds) {
		extensionList.emplace_back(".nds");
		extensionList.emplace_back(".dsi");
		extensionList.emplace_back(".ids");
		extensionList.emplace_back(".srl");
		extensionList.emplace_back(".app");
		extensionList.emplace_back(".argv");
	}
	if (memcmp(io_dldi_data->friendlyName, "DSTWO(Slot-1)", 0xD) == 0) {
		extensionList.emplace_back(".plg");
	}
	if (ms().showRvid) {
		extensionList.emplace_back(".rvid");
		extensionList.emplace_back(".mp4");
	}
	if (ms().useGbarunner) {
		extensionList.emplace_back(".gba");
	}
	if (ms().showA26) {
		extensionList.emplace_back(".a26");
	}
	if (ms().showGb) {
		extensionList.emplace_back(".gb");
		extensionList.emplace_back(".sgb");
		extensionList.emplace_back(".gbc");
	}
	if (ms().showNes) {
		extensionList.emplace_back(".nes");
		extensionList.emplace_back(".fds");
	}
	if (ms().showSmsGg) {
		extensionList.emplace_back(".sms");
		extensionList.emplace_back(".gg");
	}
	if (ms().showMd) {
		extensionList.emplace_back(".gen");
	}
	if (ms().showSnes) {
		extensionList.emplace_back(".smc");
		extensionList.emplace_back(".sfc");
	}
	if (ms().showPce) {
		extensionList.emplace_back(".pce");
	}
	srand(time(NULL));

	char path[256] = {0};

	snd();

	if (ms().theme == 4) {
		snd().playStartup();
	} else if (ms().dsiMusic != 0) {
		if ((ms().theme == 1 && ms().dsiMusic == 1) || ms().dsiMusic == 2 || (ms().dsiMusic == 3 && tc().playStartupJingle())) {
			snd().playStartup();
			snd().setStreamDelay(snd().getStartupSoundLength() - tc().startupJingleDelayAdjust());
		}
		snd().beginStream();
	}

	if (ms().consoleModel < 2 && ms().previousUsedDevice && bothSDandFlashcard()
	&& ms().launchType[ms().previousUsedDevice] == Launch::EDSiWareLaunch && !ms().dsiWareBooter) {
	  if ((access(ms().dsiWarePubPath.c_str(), F_OK) == 0 && extention(ms().dsiWarePubPath.c_str(), ".pub")) ||
	    (access(ms().dsiWarePrvPath.c_str(), F_OK) == 0 && extention(ms().dsiWarePrvPath.c_str(), ".prv"))) {
		fadeType = true; // Fade in from white
		printSmall(false, 0, 20, STR_TAKEWHILE_CLOSELID, Alignment::center);
		printLarge(false, 0, (ms().theme == 4 ? 80 : 88), STR_NOW_COPYING_DATA, Alignment::center);
		printSmall(false, 0, (ms().theme == 4 ? 96 : 104), STR_DONOT_TURNOFF_POWER, Alignment::center);
		for (int i = 0; i < 15; i++) {
			snd().updateStream();
			swiWaitForVBlank();
		}
		for (int i = 0; i < 20; i++) {
			snd().updateStream();
			swiWaitForVBlank();
		}
		showProgressIcon = true;
		controlTopBright = false;
		if (access(ms().dsiWarePubPath.c_str(), F_OK) == 0) {
			fcopy("sd:/_nds/TWiLightMenu/tempDSiWare.pub", ms().dsiWarePubPath.c_str());
		}
		if (access(ms().dsiWarePrvPath.c_str(), F_OK) == 0) {
			fcopy("sd:/_nds/TWiLightMenu/tempDSiWare.prv", ms().dsiWarePrvPath.c_str());
		}
		showProgressIcon = false;
		if (ms().theme != 4) {
			fadeType = false; // Fade to white
			for (int i = 0; i < 25; i++) {
				snd().updateStream();
				swiWaitForVBlank();
			}
		}
		clearText();
	  }
	}

	while (1) {

		snprintf(path, sizeof(path), "%s", ms().romfolder[ms().secondaryDevice].c_str());
		// Set directory
		chdir(path);

		// Navigates to the file to launch
		filename = browseForFile(extensionList);

		////////////////////////////////////
		// Launch the item

		if (applaunch) {
			// Delete previously used DSiWare of flashcard from SD
			if (!ms().gotosettings && ms().consoleModel < 2 && ms().previousUsedDevice &&
			    bothSDandFlashcard()) {
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
			getcwd(filePath, PATH_MAX);
			int pathLen = strlen(filePath);
			vector<char *> argarray;

			bool isArgv = false;
			if (strcasecmp(filename.c_str() + filename.size() - 5, ".argv") == 0) {
				ms().romPath[ms().secondaryDevice] = std::string(filePath) + std::string(filename);

				FILE *argfile = fopen(filename.c_str(), "rb");
				char str[PATH_MAX], *pstr;
				const char seps[] = "\n\r\t ";

				while (fgets(str, PATH_MAX, argfile)) {
					// Find comment and end string there
					if ((pstr = strchr(str, '#')))
						*pstr = '\0';

					// Tokenize arguments
					pstr = strtok(str, seps);

					while (pstr != NULL) {
						argarray.push_back(strdup(pstr));
						pstr = strtok(NULL, seps);
					}
				}
				fclose(argfile);
				filename = argarray.at(0);
				isArgv = true;
			} else {
				argarray.push_back(strdup(filename.c_str()));
			}

			bool dstwoPlg = false;
			bool rvid = false;
			bool mpeg4 = false;
			bool GBA = false;
			bool SNES = false;
			bool GENESIS = false, usePicoDrive = false;
			bool gameboy = false;
			bool nes = false;
			bool gamegear = false;
			bool atari2600 = false;
			bool pcEngine = false;

			// Launch DSiWare .nds via Unlaunch
			if (isDSiMode() && isDSiWare[CURPOS]) {
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
				strcpy(filePath + pathLen, name);
				free(argarray.at(0));
				argarray.at(0) = filePath;

				ms().dsiWareSrlPath = std::string(argarray[0]);
				ms().dsiWarePubPath = replaceAll(argarray[0], typeToReplace, ".pub");
				ms().dsiWarePrvPath = replaceAll(argarray[0], typeToReplace, ".prv");
				if (!isArgv) {
					ms().romPath[ms().secondaryDevice] = std::string(argarray[0]);
				}
				ms().launchType[ms().secondaryDevice] = (ms().consoleModel>0 ? Launch::ESDFlashcardLaunch : Launch::EDSiWareLaunch);
				ms().previousUsedDevice = ms().secondaryDevice;
				ms().saveSettings();

				sNDSHeaderExt NDSHeader;

				FILE *f_nds_file = fopen(filename.c_str(), "rb");

				fread(&NDSHeader, 1, sizeof(NDSHeader), f_nds_file);
				fclose(f_nds_file);

				fadeSpeed = true; // Fast fading

				if ((getFileSize(ms().dsiWarePubPath.c_str()) == 0) && (NDSHeader.pubSavSize > 0)) {
					if (ms().theme == 5) displayGameIcons = false;
					clearText();
					if (memcmp(io_dldi_data->friendlyName, "CycloDS iEvolution", 18) == 0) {
						// Display nothing
					} else if (ms().consoleModel >= 2) {
						printSmall(false, 0, 20, STR_TAKEWHILE_PRESSHOME, Alignment::center);
					} else {
						printSmall(false, 0, 20, STR_TAKEWHILE_CLOSELID, Alignment::center);
					}
					printLarge(false, 0, (ms().theme == 4 ? 80 : 88), STR_CREATING_PUBLIC_SAVE, Alignment::center);
					if (ms().theme != 4 && !fadeType) {
						fadeType = true; // Fade in from white
						for (int i = 0; i < 35; i++) {
							swiWaitForVBlank();
						}
					}
					showProgressIcon = true;

					static const int BUFFER_SIZE = 4096;
					char buffer[BUFFER_SIZE];
					toncset(buffer, 0, sizeof(buffer));
					char savHdrPath[64];
					snprintf(savHdrPath, sizeof(savHdrPath), "nitro:/DSiWareSaveHeaders/%x.savhdr",
						 (unsigned int)NDSHeader.pubSavSize);
					FILE *hdrFile = fopen(savHdrPath, "rb");
					if (hdrFile)
						fread(buffer, 1, 0x200, hdrFile);
					fclose(hdrFile);

					FILE *pFile = fopen(ms().dsiWarePubPath.c_str(), "wb");
					if (pFile) {
						fwrite(buffer, 1, sizeof(buffer), pFile);
						fseek(pFile, NDSHeader.pubSavSize - 1, SEEK_SET);
						fputc('\0', pFile);
						fclose(pFile);
					}
					showProgressIcon = false;
					clearText();
					printLarge(false, 0, (ms().theme == 4 ? 32 : 88), STR_PUBLIC_SAVE_CREATED, Alignment::center);
					for (int i = 0; i < 60; i++) {
						swiWaitForVBlank();
					}
					if (ms().theme == 5) displayGameIcons = true;
				}

				if ((getFileSize(ms().dsiWarePrvPath.c_str()) == 0) && (NDSHeader.prvSavSize > 0)) {
					if (ms().theme == 5) displayGameIcons = false;
					clearText();
					if (memcmp(io_dldi_data->friendlyName, "CycloDS iEvolution", 18) == 0) {
						// Display nothing
					} else if (ms().consoleModel >= 2) {
						printSmall(false, 0, 20, STR_TAKEWHILE_PRESSHOME, Alignment::center);
					} else {
						printSmall(false, 0, 20, STR_TAKEWHILE_CLOSELID, Alignment::center);
					}
					printLarge(false, 0, (ms().theme == 4 ? 80 : 88), STR_CREATING_PRIVATE_SAVE, Alignment::center);
					if (ms().theme != 4 && !fadeType) {
						fadeType = true; // Fade in from white
						for (int i = 0; i < 35; i++) {
							swiWaitForVBlank();
						}
					}
					showProgressIcon = true;

					static const int BUFFER_SIZE = 4096;
					char buffer[BUFFER_SIZE];
					toncset(buffer, 0, sizeof(buffer));
					char savHdrPath[64];
					snprintf(savHdrPath, sizeof(savHdrPath), "nitro:/DSiWareSaveHeaders/%x.savhdr",
						 (unsigned int)NDSHeader.prvSavSize);
					FILE *hdrFile = fopen(savHdrPath, "rb");
					if (hdrFile)
						fread(buffer, 1, 0x200, hdrFile);
					fclose(hdrFile);

					FILE *pFile = fopen(ms().dsiWarePrvPath.c_str(), "wb");
					if (pFile) {
						fwrite(buffer, 1, sizeof(buffer), pFile);
						fseek(pFile, NDSHeader.prvSavSize - 1, SEEK_SET);
						fputc('\0', pFile);
						fclose(pFile);
					}
					showProgressIcon = false;
					clearText();
					printLarge(false, 0, (ms().theme == 4 ? 32 : 88), STR_PRIVATE_SAVE_CREATED, Alignment::center);
					for (int i = 0; i < 60; i++) {
						swiWaitForVBlank();
					}
					if (ms().theme == 5) displayGameIcons = true;
				}

				if (ms().theme != 4 && ms().theme != 5 && fadeType) {
					fadeType = false; // Fade to white
					for (int i = 0; i < 25; i++) {
						swiWaitForVBlank();
					}
				}

				if (ms().dsiWareBooter || ms().consoleModel > 0) {
					// Use nds-bootstrap
					loadPerGameSettings(filename);

					bootstrapinipath = "sd:/_nds/nds-bootstrap.ini";
					CIniFile bootstrapini(bootstrapinipath);
					bootstrapini.SetString("NDS-BOOTSTRAP", "NDS_PATH", ms().dsiWareSrlPath);
					bootstrapini.SetString("NDS-BOOTSTRAP", "SAV_PATH", ms().dsiWarePubPath);
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
						(ms().forceSleepPatch
					|| (memcmp(io_dldi_data->friendlyName, "TTCARD", 6) == 0 && !sys().isRegularDS())
					|| (memcmp(io_dldi_data->friendlyName, "DSTT", 4) == 0 && !sys().isRegularDS())
					|| (memcmp(io_dldi_data->friendlyName, "DEMON", 5) == 0 && !sys().isRegularDS())
					|| (memcmp(io_dldi_data->friendlyName, "R4iDSN", 6) == 0 && !sys().isRegularDS()))
					);

					bootstrapini.SaveIniFile(bootstrapinipath);

					if (ms().theme == 5) {
						fadeType = false;		  // Fade to black
						for (int i = 0; i < 60; i++) {
							swiWaitForVBlank();
						}
					}

					if (isDSiMode()) {
						SetWidescreen(filename.c_str());
					}

					bool useNightly = (perGameSettings_bootstrapFile == -1 ? ms().bootstrapFile : perGameSettings_bootstrapFile);

					char ndsToBoot[256];
					sprintf(ndsToBoot, "sd:/_nds/nds-bootstrap-%s.nds", useNightly ? "nightly" : "release");
					if(access(ndsToBoot, F_OK) != 0) {
						sprintf(ndsToBoot, "fat:/_nds/nds-bootstrap-%s.nds", useNightly ? "nightly" : "release");
					}

					argarray.at(0) = (char *)ndsToBoot;
					snd().stopStream();
					int err = runNdsFile(argarray[0], argarray.size(), (const char **)&argarray[0], true, true, false, true, true);
					char text[64];
					snprintf(text, sizeof(text), STR_START_FAILED_ERROR.c_str(), err);
					clearText();
					fadeType = true;
					printLarge(false, 4, 4, text);
					if (err == 1) {
						printLarge(false, 4, 20, useNightly ? STR_BOOTSTRAP_NIGHTLY_NOT_FOUND : STR_BOOTSTRAP_RELEASE_NOT_FOUND);
					}
					printSmall(false, 4, 20 + calcLargeFontHeight(useNightly ? STR_BOOTSTRAP_NIGHTLY_NOT_FOUND : STR_BOOTSTRAP_RELEASE_NOT_FOUND), STR_PRESS_B_RETURN);
					int pressed = 0;
					do {
						scanKeys();
						pressed = keysDownRepeat();
						checkSdEject();
						swiWaitForVBlank();
					} while (!(pressed & KEY_B));
					fadeType = false;	// Fade to white
					for (int i = 0; i < 25; i++) {
						swiWaitForVBlank();
					}
					if (sdFound()) {
						chdir("sd:/");
					}
					runNdsFile("/_nds/TWiLightMenu/dsimenu.srldr", 0, NULL, true, false, false, true, true);
					stop();
				}

				if (ms().secondaryDevice) {
					clearText();
					printSmall(false, 0, 20, STR_TAKEWHILE_CLOSELID, Alignment::center);
					printLarge(false, 0, (ms().theme == 4 ? 80 : 88), STR_NOW_COPYING_DATA, Alignment::center);
					printSmall(false, 0, (ms().theme == 4 ? 96 : 104), STR_DONOT_TURNOFF_POWER, Alignment::center);
					if (ms().theme != 4) {
						fadeType = true; // Fade in from white
						for (int i = 0; i < 35; i++) {
							swiWaitForVBlank();
						}
					}
					showProgressIcon = true;
					fcopy(ms().dsiWareSrlPath.c_str(), "sd:/_nds/TWiLightMenu/tempDSiWare.dsi");
					if ((access(ms().dsiWarePubPath.c_str(), F_OK) == 0) && (NDSHeader.pubSavSize > 0)) {
						fcopy(ms().dsiWarePubPath.c_str(),
						      "sd:/_nds/TWiLightMenu/tempDSiWare.pub");
					}
					if ((access(ms().dsiWarePrvPath.c_str(), F_OK) == 0) && (NDSHeader.prvSavSize > 0)) {
						fcopy(ms().dsiWarePrvPath.c_str(),
						      "sd:/_nds/TWiLightMenu/tempDSiWare.prv");
					}
					showProgressIcon = false;
					if (ms().theme != 4 && ms().theme != 5) {
						fadeType = false; // Fade to white
						for (int i = 0; i < 25; i++) {
							swiWaitForVBlank();
						}
					}

					if ((access(ms().dsiWarePubPath.c_str(), F_OK) == 0 && (NDSHeader.pubSavSize > 0))
					 || (access(ms().dsiWarePrvPath.c_str(), F_OK) == 0 && (NDSHeader.prvSavSize > 0))) {
						clearText();
						printLarge(false, 0, ms().theme == 4 ? 16 : 72, STR_RESTART_AFTER_SAVING, Alignment::center);
						if (ms().theme != 4) {
							fadeType = true; // Fade in from white
						}
						for (int i = 0; i < 60 * 3; i++) {
							swiWaitForVBlank(); // Wait 3 seconds
						}
						if (ms().theme != 4 && ms().theme != 5) {
							fadeType = false;	   // Fade to white
							for (int i = 0; i < 25; i++) {
								swiWaitForVBlank();
							}
						}
					}
				}

				if (ms().theme == 5) {
					fadeType = false;		  // Fade to black
					for (int i = 0; i < 60; i++) {
						swiWaitForVBlank();
					}
				}

				char unlaunchDevicePath[256];
				if (ms().secondaryDevice) {
					snprintf(unlaunchDevicePath, sizeof(unlaunchDevicePath),
						 "sdmc:/_nds/TWiLightMenu/tempDSiWare.dsi");
				} else {
					snprintf(unlaunchDevicePath, sizeof(unlaunchDevicePath), "__%s",
						 ms().dsiWareSrlPath.c_str());
					unlaunchDevicePath[0] = 's';
					unlaunchDevicePath[1] = 'd';
					unlaunchDevicePath[2] = 'm';
					unlaunchDevicePath[3] = 'c';
				}

				tonccpy((u8 *)0x02000800, unlaunchAutoLoadID, 12);
				*(u16 *)(0x0200080C) = 0x3F0;   // Unlaunch Length for CRC16 (fixed, must be 3F0h)
				*(u16 *)(0x0200080E) = 0;       // Unlaunch CRC16 (empty)
				*(u32 *)(0x02000810) = 0;       // Unlaunch Flags
				*(u32 *)(0x02000810) |= BIT(0); // Load the title at 2000838h
				*(u32 *)(0x02000810) |= BIT(1); // Use colors 2000814h
				*(u16 *)(0x02000814) = 0x7FFF;  // Unlaunch Upper screen BG color (0..7FFFh)
				*(u16 *)(0x02000816) = 0x7FFF;  // Unlaunch Lower screen BG color (0..7FFFh)
				toncset((u8 *)0x02000818, 0, 0x20 + 0x208 + 0x1C0); // Unlaunch Reserved (zero)
				int i2 = 0;
				for (int i = 0; i < (int)sizeof(unlaunchDevicePath); i++) {
					*(u8 *)(0x02000838 + i2) =
					    unlaunchDevicePath[i]; // Unlaunch Device:/Path/Filename.ext (16bit
								   // Unicode,end by 0000h)
					i2 += 2;
				}
				while (*(u16 *)(0x0200080E) == 0) { // Keep running, so that CRC16 isn't 0
					*(u16 *)(0x0200080E) =
					    swiCRC16(0xFFFF, (void *)0x02000810, 0x3F0); // Unlaunch CRC16
				}
				DC_FlushAll();
				fifoSendValue32(FIFO_USER_02, 1); // Reboot into DSiWare title, booted via Unlaunch
				for (int i = 0; i < 15; i++) {
					swiWaitForVBlank();
				}
			}

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

				char gameTid3[5];
				for (int i = 0; i < 3; i++) {
					gameTid3[i] = gameTid[CURPOS][i];
				}

				if (memcmp(gameTid[CURPOS], "HND", 3) == 0 || memcmp(gameTid[CURPOS], "HNE", 3) == 0) {
					dsModeSwitch = true;
					dsModeDSiWare = true;
					useBackend = false; // Bypass nds-bootstrap
					ms().homebrewBootstrap = true;
				} else if (isHomebrew[CURPOS]) {
					loadPerGameSettings(filename);
					if (perGameSettings_directBoot || (ms().useBootstrap && ms().secondaryDevice)) {
						useBackend = false; // Bypass nds-bootstrap
					} else {
						useBackend = true;
					}
					if (isDSiMode() && !perGameSettings_dsiMode) {
						dsModeSwitch = true;
					}
					ms().homebrewBootstrap = true;
				} else {
					loadPerGameSettings(filename);
					useBackend = true;
					ms().homebrewBootstrap = false;
				}

				char *name = argarray.at(0);
				strcpy(filePath + pathLen, name);
				free(argarray.at(0));
				argarray.at(0) = filePath;
				if (useBackend) {
					if (ms().useBootstrap || !ms().secondaryDevice) {
						std::string path = argarray[0];
						std::string savename = replaceAll(filename, typeToReplace, getSavExtension());
						std::string ramdiskname = replaceAll(filename, typeToReplace, getImgExtension(perGameSettings_ramDiskNo));
						std::string romFolderNoSlash = ms().romfolder[ms().secondaryDevice];
						RemoveTrailingSlashes(romFolderNoSlash);
						mkdir(isHomebrew[CURPOS] ? "ramdisks" : "saves", 0777);
						std::string savepath = romFolderNoSlash + "/saves/" + savename;
						if (sdFound() && ms().secondaryDevice && ms().fcSaveOnSd) {
							savepath = replaceAll(savepath, "fat:/", "sd:/");
						}
						std::string ramdiskpath = romFolderNoSlash + "/ramdisks/" + ramdiskname;

						if (!isHomebrew[CURPOS] && (strcmp(gameTid[CURPOS], "NTR") != 0))
						{ // Create or expand save if game isn't homebrew
							int orgsavesize = getFileSize(savepath.c_str());
							int savesize = 524288; // 512KB (default size for most games)

							for (auto i : saveMap) {
								if (i.second.find(gameTid3) != i.second.cend()) {
									savesize = i.first;
									break;
								}
							}

							bool saveSizeFixNeeded = false;

							// TODO: If the list gets large enough, switch to bsearch().
							for (unsigned int i = 0; i < sizeof(saveSizeFixList) / sizeof(saveSizeFixList[0]); i++) {
								if (memcmp(gameTid[CURPOS], saveSizeFixList[i], 3) == 0) {
									// Found a match.
									saveSizeFixNeeded = true;
									break;
								}
							}

							if ((orgsavesize == 0 && savesize > 0) || (orgsavesize < savesize && saveSizeFixNeeded)) {
								if (ms().theme == 5) displayGameIcons = false;
								if (isDSiMode() && memcmp(io_dldi_data->friendlyName, "CycloDS iEvolution", 18) == 0) {
									// Display nothing
								} else if (REG_SCFG_EXT != 0 && ms().consoleModel >= 2) {
									printSmall(false, 0, 20, STR_TAKEWHILE_PRESSHOME, Alignment::center);
								} else {
									printSmall(false, 0, 20, STR_TAKEWHILE_CLOSELID, Alignment::center);
								}
								printLarge(false, 0, (ms().theme == 4 ? 80 : 88), (orgsavesize == 0) ? STR_CREATING_SAVE : STR_EXPANDING_SAVE, Alignment::center);

								if (ms().theme != 4 && ms().theme != 5) {
									fadeSpeed = true; // Fast fading
									fadeType = true; // Fade in from white
								}
								showProgressIcon = true;

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
								showProgressIcon = false;
								clearText();
								printLarge(false, 0, (ms().theme == 4 ? 32 : 88), (orgsavesize == 0) ? STR_SAVE_CREATED : STR_SAVE_EXPANDED, Alignment::center);
								for (int i = 0; i < 30; i++) {
									swiWaitForVBlank();
								}
								if (ms().theme != 4 && ms().theme != 5) {
									fadeType = false;	   // Fade to white
									for (int i = 0; i < 25; i++) {
										swiWaitForVBlank();
									}
								}
								clearText();
								if (ms().theme == 5) displayGameIcons = true;
							}
						}

						int donorSdkVer = SetDonorSDK();
						SetMPUSettings(argarray[0]);
						SetSpeedBumpExclude();

						bool useWidescreen = (perGameSettings_wideScreen == -1 ? ms().wideScreen : perGameSettings_wideScreen);

						bootstrapinipath = (sdFound() ? "sd:/_nds/nds-bootstrap.ini" : "fat:/_nds/nds-bootstrap.ini");
						CIniFile bootstrapini(bootstrapinipath);
						bootstrapini.SetString("NDS-BOOTSTRAP", "NDS_PATH", path);
						bootstrapini.SetString("NDS-BOOTSTRAP", "SAV_PATH", savepath);
						bootstrapini.SetString("NDS-BOOTSTRAP", "HOMEBREW_ARG", (useWidescreen && (gameTid[CURPOS][0] == 'W' || romVersion[CURPOS] == 0x57)) ? "wide" : "");
						if (!isHomebrew[CURPOS]) {
							bootstrapini.SetString("NDS-BOOTSTRAP", "AP_FIX_PATH", setApFix(argarray[0]));
						}
						bootstrapini.SetString("NDS-BOOTSTRAP", "RAM_DRIVE_PATH", (perGameSettings_ramDiskNo >= 0 && !ms().secondaryDevice) ? ramdiskpath : "sd:/null.img");
						bootstrapini.SetInt("NDS-BOOTSTRAP", "LANGUAGE", perGameSettings_language == -2 ? ms().gameLanguage : perGameSettings_language);
						if (isDSiMode()) {
							bootstrapini.SetInt("NDS-BOOTSTRAP", "DSI_MODE", perGameSettings_dsiMode == -1 ? ms().bstrap_dsiMode : perGameSettings_dsiMode);
						}
						if (REG_SCFG_EXT != 0) {
							bootstrapini.SetInt("NDS-BOOTSTRAP", "BOOST_CPU", perGameSettings_boostCpu == -1 ? ms().boostCpu : perGameSettings_boostCpu);
							bootstrapini.SetInt("NDS-BOOTSTRAP", "BOOST_VRAM", perGameSettings_boostVram == -1 ? ms().boostVram : perGameSettings_boostVram);
						}
						bootstrapini.SetInt("NDS-BOOTSTRAP", "DONOR_SDK_VER", donorSdkVer);
						bootstrapini.SetInt("NDS-BOOTSTRAP", "PATCH_MPU_REGION", mpuregion);
						bootstrapini.SetInt("NDS-BOOTSTRAP", "PATCH_MPU_SIZE", mpusize);
						bootstrapini.SetInt("NDS-BOOTSTRAP", "CARDENGINE_CACHED", ceCached);
						bootstrapini.SetInt("NDS-BOOTSTRAP", "FORCE_SLEEP_PATCH", 
							(ms().forceSleepPatch
						|| (memcmp(io_dldi_data->friendlyName, "TTCARD", 6) == 0 && !sys().isRegularDS())
						|| (memcmp(io_dldi_data->friendlyName, "DSTT", 4) == 0 && !sys().isRegularDS())
						|| (memcmp(io_dldi_data->friendlyName, "DEMON", 5) == 0 && !sys().isRegularDS())
						|| (memcmp(io_dldi_data->friendlyName, "R4iDSN", 6) == 0 && !sys().isRegularDS()))
						);
						bootstrapini.SaveIniFile(bootstrapinipath);

						CheatCodelist codelist;
						u32 gameCode, crc32;

						if (isDSiMode() && !isHomebrew[CURPOS]) {
							bool cheatsEnabled = true;
							const char* cheatDataBin = "/_nds/nds-bootstrap/cheatData.bin";
							mkdir("/_nds", 0777);
							mkdir("/_nds/nds-bootstrap", 0777);
							if(codelist.romData(path,gameCode,crc32)) {
								long cheatOffset; size_t cheatSize;
								FILE* dat=fopen(sdFound() ? "sd:/_nds/TWiLightMenu/extras/usrcheat.dat" : "fat:/_nds/TWiLightMenu/extras/usrcheat.dat","rb");
								if (dat) {
									if (codelist.searchCheatData(dat, gameCode, crc32, cheatOffset, cheatSize)) {
										codelist.parse(path);
										writeCheatsToFile(codelist.getCheats(), cheatDataBin);
										FILE* cheatData=fopen(cheatDataBin,"rb");
										if (cheatData) {
											u32 check[2];
											fread(check, 1, 8, cheatData);
											fclose(cheatData);
											if (check[1] == 0xCF000000 || getFileSize(cheatDataBin) > 0x8000) {
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
							ms().romPath[ms().secondaryDevice] = std::string(argarray[0]);
						}
						ms().homebrewHasWide = (isHomebrew[CURPOS] && gameTid[CURPOS][0] == 'W');
						ms().launchType[ms().secondaryDevice] = Launch::ESDFlashcardLaunch; // 1
						ms().previousUsedDevice = ms().secondaryDevice;
						ms().saveSettings();

						if (ms().theme == 5) {
							fadeType = false;		  // Fade to black
							for (int i = 0; i < 60; i++) {
								swiWaitForVBlank();
							}
						}

						if (isDSiMode()) {
							SetWidescreen(filename.c_str());
						}

						bool useNightly = (perGameSettings_bootstrapFile == -1 ? ms().bootstrapFile : perGameSettings_bootstrapFile);

						char ndsToBoot[256];
						sprintf(ndsToBoot, "sd:/_nds/nds-bootstrap-%s%s.nds", ms().homebrewBootstrap ? "hb-" : "", useNightly ? "nightly" : "release");
						if(access(ndsToBoot, F_OK) != 0) {
							sprintf(ndsToBoot, "fat:/_nds/%s-%s%s.nds", isDSiMode() ? "nds-bootstrap" : "b4ds", ms().homebrewBootstrap ? "hb-" : "", useNightly ? "nightly" : "release");
						}

						argarray.at(0) = (char *)ndsToBoot;
						snd().stopStream();
						int err = runNdsFile(argarray[0], argarray.size(), (const char **)&argarray[0], (ms().homebrewBootstrap ? false : true), true, false, true, true);
						char text[64];
						snprintf(text, sizeof(text), STR_START_FAILED_ERROR.c_str(), err);
						clearText();
						fadeType = true;
						printLarge(false, 4, 4, text);
						if (err == 1) {
							printLarge(false, 4, 20, useNightly ? STR_BOOTSTRAP_NIGHTLY_NOT_FOUND : STR_BOOTSTRAP_RELEASE_NOT_FOUND);
						}
						printSmall(false, 4, 20 + calcLargeFontHeight(useNightly ? STR_BOOTSTRAP_NIGHTLY_NOT_FOUND : STR_BOOTSTRAP_RELEASE_NOT_FOUND), STR_PRESS_B_RETURN);
						int pressed = 0;
						do {
							scanKeys();
							pressed = keysDownRepeat();
							checkSdEject();
							swiWaitForVBlank();
						} while (!(pressed & KEY_B));
						fadeType = false;	// Fade to white
						for (int i = 0; i < 25; i++) {
							swiWaitForVBlank();
						}
						if (sdFound()) {
							chdir("sd:/");
						}
						runNdsFile("/_nds/TWiLightMenu/dsimenu.srldr", 0, NULL, true, false, false, true, true);
						stop();
					} else {
						ms().romPath[ms().secondaryDevice] = std::string(argarray[0]);
						ms().launchType[ms().secondaryDevice] = Launch::ESDFlashcardLaunch;
						ms().previousUsedDevice = ms().secondaryDevice;
						ms().saveSettings();

						if (ms().theme == 5) {
							fadeType = false;		  // Fade to black
							for (int i = 0; i < 60; i++) {
								swiWaitForVBlank();
							}
						}

						loadGameOnFlashcard(argarray[0], true);
					}
				} else {
					if (!isArgv) {
						ms().romPath[ms().secondaryDevice] = std::string(argarray[0]);
					}
					ms().homebrewHasWide = (isHomebrew[CURPOS] && (gameTid[CURPOS][0] == 'W' || romVersion[CURPOS] == 0x57));
					ms().launchType[ms().secondaryDevice] = Launch::ESDFlashcardDirectLaunch;
					ms().previousUsedDevice = ms().secondaryDevice;
					ms().saveSettings();

					if (ms().theme == 5) {
						fadeType = false;		  // Fade to black
						for (int i = 0; i < 60; i++) {
							swiWaitForVBlank();
						}
					}

					if (isDSiMode()) {
						SetWidescreen(filename.c_str());
					}

					bool runNds_boostCpu = false;
					bool runNds_boostVram = false;
					if (isDSiMode() && !dsModeDSiWare) {
						loadPerGameSettings(filename);

						runNds_boostCpu = perGameSettings_boostCpu == -1 ? ms().boostCpu : perGameSettings_boostCpu;
						runNds_boostVram = perGameSettings_boostVram == -1 ? ms().boostVram : perGameSettings_boostVram;
					}
					snd().stopStream();
					int err = runNdsFile(argarray[0], argarray.size(), (const char **)&argarray[0], true, true, dsModeSwitch, runNds_boostCpu, runNds_boostVram);
					char text[64];
					snprintf(text, sizeof(text), STR_START_FAILED_ERROR.c_str(), err);
					fadeType = true;
					printLarge(false, 4, 4, text);
					printSmall(false, 4, 20, STR_PRESS_B_RETURN);
					int pressed = 0;
					do {
						scanKeys();
						pressed = keysDownRepeat();
						checkSdEject();
						swiWaitForVBlank();
					} while (!(pressed & KEY_B));
					fadeType = false;	// Fade to white
					for (int i = 0; i < 25; i++) {
						swiWaitForVBlank();
					}
					if (sdFound()) {
						chdir("sd:/");
					}
					runNdsFile("/_nds/TWiLightMenu/dsimenu.srldr", 0, NULL, true, false, false, true, true);
					stop();
				}
			} else if (extention(filename, ".plg")) {
				dstwoPlg = true;
			} else if (extention(filename, ".rvid")) {
				rvid = true;
			} else if (extention(filename, ".mp4")) {
				mpeg4 = true;
			} else if (extention(filename, ".gba")) {
				//ms().launchType[ms().secondaryDevice] = Launch::ESDFlashcardLaunch;
				//ms().previousUsedDevice = ms().secondaryDevice;
				/*ms().saveSettings();

				sysSetCartOwner(BUS_OWNER_ARM9);	// Allow arm9 to access GBA ROM in memory map

				// Load GBA ROM into EZ Flash 3-in-1
				FILE *gbaFile = fopen(filename.c_str(), "rb");
				fread((void*)0x08000000, 1, 0x1000000, gbaFile);
				fclose(gbaFile);
				gbaSwitch();*/
				GBA = true;
			} else if (extention(filename, ".gb") || extention(filename, ".sgb") ||
				   extention(filename, ".gbc")) {
				gameboy = true;
			} else if (extention(filename, ".nes") || extention(filename, ".fds")) {
				nes = true;
			} else if (extention(filename, ".sms") || extention(filename, ".gg")) {
				mkdir(ms().secondaryDevice ? "fat:/data" : "sd:/data", 0777);
				mkdir(ms().secondaryDevice ? "fat:/data/s8ds" : "sd:/data/s8ds", 0777);

				gamegear = true;
			} else if (extention(filename, ".gen")) {
				GENESIS = true;
				usePicoDrive = (ms().showMd==2 || (ms().showMd==3 && getFileSize(filename.c_str()) > 0x300000));
			} else if (extention(filename, ".smc") || extention(filename, ".sfc")) {
				SNES = true;
			} else if (extention(filename, ".a26")) {
				atari2600 = true;
			} else if (extention(filename, ".pce")) {
				pcEngine = true;
			}

			if (dstwoPlg || rvid || mpeg4 || gameboy || nes
			|| (gamegear && !ms().smsGgInRam)
			|| (gamegear && ms().secondaryDevice)
			|| (GENESIS && usePicoDrive)
			|| atari2600) {
				const char *ndsToBoot = "";
				std::string romfolderNoSlash = ms().romfolder[ms().secondaryDevice];
				RemoveTrailingSlashes(romfolderNoSlash);
				char ROMpath[256];
				snprintf(ROMpath, sizeof(ROMpath), "%s/%s", romfolderNoSlash.c_str(), filename.c_str());
				ms().romPath[ms().secondaryDevice] = std::string(ROMpath);
				ms().homebrewArg = std::string(ROMpath);

				if (gameboy) {
					ms().launchType[ms().secondaryDevice] = Launch::EGameYobLaunch;
				} else if (nes) {
					ms().launchType[ms().secondaryDevice] = Launch::ENESDSLaunch;
				} else if (gamegear) {
					ms().launchType[ms().secondaryDevice] = Launch::ES8DSLaunch;
				} else if (rvid) {
					ms().launchType[ms().secondaryDevice] = Launch::ERVideoLaunch;
				} else if (mpeg4) {
					ms().launchType[ms().secondaryDevice] = Launch::EMPEG4Launch;
				} else if (atari2600) {
					ms().launchType[ms().secondaryDevice] = Launch::EStellaDSLaunch;
				} else if (GENESIS) {
					ms().launchType[ms().secondaryDevice] = Launch::EPicoDriveTWLLaunch;
				}

				ms().previousUsedDevice = ms().secondaryDevice;
				ms().saveSettings();

				if (ms().theme == 5) {
					fadeType = false;		  // Fade to black
					for (int i = 0; i < 60; i++) {
						swiWaitForVBlank();
					}
				}

				argarray.push_back(ROMpath);
				int err = 0;

				if (dstwoPlg) {
					ndsToBoot = "/_nds/TWiLightMenu/bootplg.srldr";

					// Print .plg path without "fat:" at the beginning
					char ROMpathDS2[256];
					for (int i = 0; i < 252; i++) {
						ROMpathDS2[i] = ROMpath[4+i];
						if (ROMpath[4+i] == '\x00') break;
					}

					CIniFile dstwobootini( "fat:/_dstwo/twlm.ini" );
					dstwobootini.SetString("boot_settings", "file", ROMpathDS2);
					dstwobootini.SaveIniFile( "fat:/_dstwo/twlm.ini" );
				} else if (rvid) {
					ndsToBoot = "sd:/_nds/TWiLightMenu/apps/RocketVideoPlayer.nds";
					if(access(ndsToBoot, F_OK) != 0) {
						ndsToBoot = "/_nds/TWiLightMenu/apps/RocketVideoPlayer.nds";
					}
				} else if (mpeg4) {
					ndsToBoot = "sd:/_nds/TWiLightMenu/apps/MPEG4Player.nds";
					if(access(ndsToBoot, F_OK) != 0) {
						ndsToBoot = "/_nds/TWiLightMenu/apps/MPEG4Player.nds";
					}
				} else if (gameboy) {
					ndsToBoot = "sd:/_nds/TWiLightMenu/emulators/gameyob.nds";
					if(access(ndsToBoot, F_OK) != 0) {
						ndsToBoot = "/_nds/TWiLightMenu/emulators/gameyob.nds";
					}
				} else if (nes) {
					ndsToBoot = (ms().secondaryDevice ? "sd:/_nds/TWiLightMenu/emulators/nesds.nds" : "sd:/_nds/TWiLightMenu/emulators/nestwl.nds");
					if(access(ndsToBoot, F_OK) != 0) {
						ndsToBoot = "/_nds/TWiLightMenu/emulators/nesds.nds";
					}
				} else if (gamegear) {
					ndsToBoot = "sd:/_nds/TWiLightMenu/emulators/S8DS.nds";
					if(access(ndsToBoot, F_OK) != 0) {
						ndsToBoot = "/_nds/TWiLightMenu/emulators/S8DS.nds";
					}
				} else if (GENESIS) {
					ndsToBoot = "sd:/_nds/TWiLightMenu/emulators/PicoDriveTWL.nds";
					if(access(ndsToBoot, F_OK) != 0) {
						ndsToBoot = "/_nds/TWiLightMenu/emulators/PicoDriveTWL.nds";
					}
				} else if (atari2600) {
					ndsToBoot = "sd:/_nds/TWiLightMenu/emulators/StellaDS.nds";
					if(access(ndsToBoot, F_OK) != 0) {
						ndsToBoot = "/_nds/TWiLightMenu/emulators/StellaDS.nds";
					}
				}
				argarray.at(0) = (char *)ndsToBoot;
				snd().stopStream();
				err = runNdsFile(argarray[0], argarray.size(), (const char **)&argarray[0], true, true, false, true, true); // Pass ROM to emulator as argument

				char text[64];
				snprintf(text, sizeof(text), STR_START_FAILED_ERROR.c_str(), err);
				fadeType = true;
				printLarge(false, 4, 4, text);
				printLarge(false, 4, 20, STR_PRESS_B_RETURN);
				int pressed = 0;
				do {
					scanKeys();
					pressed = keysDownRepeat();
					checkSdEject();
					swiWaitForVBlank();
				} while (!(pressed & KEY_B));
				fadeType = false;	// Fade to white
				for (int i = 0; i < 25; i++) {
					swiWaitForVBlank();
				}
				if (sdFound()) {
					chdir("sd:/");
				}
				runNdsFile("/_nds/TWiLightMenu/dsimenu.srldr", 0, NULL, true, false, false, true, true);
				stop();
			} else if (GBA || gamegear || SNES || GENESIS || pcEngine) {
				const char *ndsToBoot;
				std::string romfolderNoSlash = ms().romfolder[ms().secondaryDevice];
				RemoveTrailingSlashes(romfolderNoSlash);
				char ROMpath[256];
				snprintf(ROMpath, sizeof(ROMpath), "%s/%s", romfolderNoSlash.c_str(), filename.c_str());
				ms().homebrewBootstrap = true;
				ms().romPath[ms().secondaryDevice] = std::string(ROMpath);
				ms().launchType[ms().secondaryDevice] = Launch::ESDFlashcardLaunch; // 1
				ms().previousUsedDevice = ms().secondaryDevice;
				ms().saveSettings();

				if (ms().theme == 5) {
					fadeType = false;		  // Fade to black
					for (int i = 0; i < 60; i++) {
						swiWaitForVBlank();
					}
				}

				if (ms().secondaryDevice) {
					if (GBA) {
						ndsToBoot = ms().gbar2DldiAccess ? "sd:/_nds/GBARunner2_arm7dldi_ds.nds" : "sd:/_nds/GBARunner2_arm9dldi_ds.nds";
						if (isDSiMode()) {
							ndsToBoot = ms().consoleModel>0 ? "sd:/_nds/GBARunner2_arm7dldi_3ds.nds" : "sd:/_nds/GBARunner2_arm7dldi_dsi.nds";
						}
						if(access(ndsToBoot, F_OK) != 0) {
							ndsToBoot = ms().gbar2DldiAccess ? "/_nds/GBARunner2_arm7dldi_ds.nds" : "/_nds/GBARunner2_arm9dldi_ds.nds";
							if (isDSiMode()) {
								ndsToBoot = ms().consoleModel>0 ? "/_nds/GBARunner2_arm7dldi_3ds.nds" : "/_nds/GBARunner2_arm7dldi_dsi.nds";
							}
						}
						argarray.push_back(ROMpath);
					} else if (gamegear) {
						ndsToBoot = "sd:/_nds/TWiLightMenu/emulators/S8DS07.nds";
						if(access(ndsToBoot, F_OK) != 0) {
							ndsToBoot = "/_nds/TWiLightMenu/emulators/S8DS07.nds";
						}
					} else if (SNES) {
						ndsToBoot = "sd:/_nds/TWiLightMenu/emulators/SNEmulDS.nds";
						if(access(ndsToBoot, F_OK) != 0) {
							ndsToBoot = "/_nds/TWiLightMenu/emulators/SNEmulDS.nds";
						}
					} else if (pcEngine) {
						ndsToBoot = "sd:/_nds/TWiLightMenu/emulators/NitroGrafx.nds";
						if(access(ndsToBoot, F_OK) != 0) {
							ndsToBoot = "/_nds/TWiLightMenu/emulators/NitroGrafx.nds";
						}
					} else {
						ndsToBoot = "sd:/_nds/TWiLightMenu/emulators/jEnesisDS.nds";
						if(access(ndsToBoot, F_OK) != 0) {
							ndsToBoot = "/_nds/TWiLightMenu/emulators/jEnesisDS.nds";
						}
					}
				} else {
					ndsToBoot =
					    (ms().bootstrapFile ? "sd:/_nds/nds-bootstrap-hb-nightly.nds" : "sd:/_nds/nds-bootstrap-hb-release.nds");
					CIniFile bootstrapini("sd:/_nds/nds-bootstrap.ini");

					bootstrapini.SetInt("NDS-BOOTSTRAP", "LANGUAGE", ms().gameLanguage);
					bootstrapini.SetInt("NDS-BOOTSTRAP", "DSI_MODE", 0);
					if (GBA) {
						const char* gbar2Path = ms().consoleModel>0 ? "sd:/_nds/GBARunner2_arm7dldi_3ds.nds" : "sd:/_nds/GBARunner2_arm7dldi_dsi.nds";
						if (sys().arm7SCFGLocked()) {
							gbar2Path = ms().consoleModel>0 ? "sd:/_nds/GBARunner2_arm7dldi_nodsp_3ds.nds" : "sd:/_nds/GBARunner2_arm7dldi_nodsp_dsi.nds";
						}

						bootstrapini.SetString("NDS-BOOTSTRAP", "NDS_PATH", gbar2Path);
						bootstrapini.SetString("NDS-BOOTSTRAP", "HOMEBREW_ARG", ROMpath);
						bootstrapini.SetString("NDS-BOOTSTRAP", "RAM_DRIVE_PATH", "");
						bootstrapini.SetInt("NDS-BOOTSTRAP", "BOOST_CPU", 1);
					} else if (gamegear) {
						bootstrapini.SetString("NDS-BOOTSTRAP", "NDS_PATH", "sd:/_nds/TWiLightMenu/emulators/S8DS07.nds");
						bootstrapini.SetString("NDS-BOOTSTRAP", "HOMEBREW_ARG", "");
						bootstrapini.SetString("NDS-BOOTSTRAP", "RAM_DRIVE_PATH", ROMpath);
						bootstrapini.SetInt("NDS-BOOTSTRAP", "BOOST_CPU", 1);
					} else if (SNES) {
						bootstrapini.SetString("NDS-BOOTSTRAP", "NDS_PATH", "sd:/_nds/TWiLightMenu/emulators/SNEmulDS.nds");
						bootstrapini.SetString("NDS-BOOTSTRAP", "HOMEBREW_ARG", "fat:/snes/ROM.SMC");
						bootstrapini.SetString("NDS-BOOTSTRAP", "RAM_DRIVE_PATH", ROMpath);
						bootstrapini.SetInt("NDS-BOOTSTRAP", "BOOST_CPU", 0);
					} else if (pcEngine) {
						bootstrapini.SetString("NDS-BOOTSTRAP", "NDS_PATH", "sd:/_nds/TWiLightMenu/emulators/NitroGrafx.nds");
						bootstrapini.SetString("NDS-BOOTSTRAP", "HOMEBREW_ARG", ROMpath);
						bootstrapini.SetString("NDS-BOOTSTRAP", "RAM_DRIVE_PATH", "");
						bootstrapini.SetInt("NDS-BOOTSTRAP", "BOOST_CPU", 1);
					} else {
						bootstrapini.SetString("NDS-BOOTSTRAP", "NDS_PATH", "sd:/_nds/TWiLightMenu/emulators/jEnesisDS.nds");
						bootstrapini.SetString("NDS-BOOTSTRAP", "HOMEBREW_ARG", "fat:/ROM.BIN");
						bootstrapini.SetString("NDS-BOOTSTRAP", "RAM_DRIVE_PATH", ROMpath);
						bootstrapini.SetInt("NDS-BOOTSTRAP", "BOOST_CPU", 1);
					}
					bootstrapini.SetInt("NDS-BOOTSTRAP", "BOOST_VRAM", 0);

					bootstrapini.SaveIniFile("sd:/_nds/nds-bootstrap.ini");
				}
				argarray.at(0) = (char *)ndsToBoot;
				snd().stopStream();
				int err = runNdsFile(argarray[0], argarray.size(), (const char **)&argarray[0], ms().secondaryDevice, true, (ms().secondaryDevice && !GBA), true, !GBA);
				char text[64];
				snprintf(text, sizeof(text), STR_START_FAILED_ERROR.c_str(), err);
				fadeType = true;
				printLarge(false, 4, 4, text);
				if (err == 1) {
					printLarge(false, 4, 20, ms().bootstrapFile ? STR_BOOTSTRAP_NIGHTLY_NOT_FOUND : STR_BOOTSTRAP_RELEASE_NOT_FOUND);
				}
				printSmall(false, 4, 20 + calcLargeFontHeight(ms().bootstrapFile ? STR_BOOTSTRAP_NIGHTLY_NOT_FOUND : STR_BOOTSTRAP_RELEASE_NOT_FOUND), STR_PRESS_B_RETURN);
				int pressed = 0;
				do {
					scanKeys();
					pressed = keysDownRepeat();
					checkSdEject();
					swiWaitForVBlank();
				} while (!(pressed & KEY_B));
				fadeType = false;	// Fade to white
				for (int i = 0; i < 25; i++) {
					swiWaitForVBlank();
				}
				if (sdFound()) {
					chdir("sd:/");
				}
				runNdsFile("/_nds/TWiLightMenu/dsimenu.srldr", 0, NULL, true, false, false, true, true);
				stop();
			}

			while (argarray.size() != 0) {
				free(argarray.at(0));
				argarray.erase(argarray.begin());
			}
		}
	}

	return 0;
}
