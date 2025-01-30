#include <nds.h>
#include <nds/arm9/dldi.h>
#include "io_m3_common.h"
#include "io_g6_common.h"
#include "io_sc_common.h"
#include "exptools.h"

#include <fat.h>
#include "fat_ext.h"
#include <limits.h>
#include <stdio.h>
#include <sys/stat.h>

#include <gl2d.h>
#include <maxmod9.h>
#include <string.h>
#include <unistd.h>

#include "date.h"
#include "fileCopy.h"

#include "graphics/graphics.h"

#include "common/twlmenusettings.h"
#include "common/bootstrapsettings.h"
#include "common/fatHeader.h"
#include "common/flashcard.h"
#include "common/nds_loader_arm9.h"
#include "common/nds_bootstrap_loader.h"
#include "common/systemdetails.h"
#include "common/my_rumble.h"
#include "myDSiMode.h"
#include "graphics/ThemeConfig.h"
#include "graphics/ThemeTextures.h"
#include "graphics/themefilenames.h"

#include "defaultSettings.h"
#include "errorScreen.h"
#include "esrbSplash.h"
#include "fileBrowse.h"
#include "gbaswitch.h"
#include "ndsheaderbanner.h"
#include "perGameSettings.h"

#include "graphics/fontHandler.h"
#include "graphics/iconHandler.h"

#include "common/inifile.h"
#include "common/logging.h"
#include "common/stringtool.h"
#include "common/tonccpy.h"

#include "sound.h"
#include "language.h"

#include "cheat.h"
#include "crc.h"

#include "autoboot.h"		 // For rebooting into the game

#include "twlClockExcludeMap.h"
#include "dmaExcludeMap.h"
#include "asyncReadExcludeMap.h"
#include "donorMap.h"
#include "saveMap.h"
#include "ROMList.h"

extern bool useTwlCfg;

bool whiteScreen = true;
bool fadeType = false; // false = out, true = in
bool fadeSpeed = true; // false = slow (for DSi launch effect), true = fast
bool fadeSleep = false;
bool controlTopBright = true;
bool controlBottomBright = true;
bool widescreenFound = false;
//bool widescreenEffects = false;

extern void ClearBrightness();
extern bool displayGameIcons;
extern bool showProgressIcon;
extern bool showProgressBar;
extern int progressBarLength;

const char *unlaunchAutoLoadID = "AutoLoadInfo";
static char16_t hiyaNdsPath[] = u"sdmc:/hiya.dsi";
char launcherPath[256];

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

bool applaunch = false;
bool dsModeForced = false;

bool useBackend = false;

bool dropDown = false;
int currentBg = 0;
bool showSTARTborder = false;
bool buttonArrowTouched[2] = {false};
bool scrollWindowTouched = false;
bool useRumble = false;

bool applaunchprep = false;

int spawnedtitleboxes = 0;

bool showColon = true;

struct statvfs st[2];
bool gbaBiosFound[2] = {false};

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
 * Disable TWL clock speed for a specific game.
 */
bool setClockSpeed() {
	if (!ms().ignoreBlacklists) {
		// TODO: If the list gets large enough, switch to bsearch().
		for (unsigned int i = 0; i < sizeof(twlClockExcludeList)/sizeof(twlClockExcludeList[0]); i++) {
			if (memcmp(gameTid[CURPOS], twlClockExcludeList[i], 3) == 0) {
				// Found match
				dsModeForced = true;
				return false;
			}
		}
	}

	return perGameSettings_boostCpu == -1 ? DEFAULT_BOOST_CPU : perGameSettings_boostCpu;
}

/**
 * Disable card read DMA for a specific game.
 */
bool setCardReadDMA() {
	if (!ms().ignoreBlacklists) {
		// TODO: If the list gets large enough, switch to bsearch().
		for (unsigned int i = 0; i < sizeof(cardReadDMAExcludeList)/sizeof(cardReadDMAExcludeList[0]); i++) {
			if (memcmp(gameTid[CURPOS], cardReadDMAExcludeList[i], 3) == 0) {
				// Found match
				return false;
			}
		}
	}

	return perGameSettings_cardReadDMA == -1 ? DEFAULT_CARD_READ_DMA : perGameSettings_cardReadDMA;
}

/**
 * Disable asynch card read for a specific game.
 */
bool setAsyncCardRead() {
	if (!ms().ignoreBlacklists) {
		// TODO: If the list gets large enough, switch to bsearch().
		for (unsigned int i = 0; i < sizeof(asyncReadExcludeList)/sizeof(asyncReadExcludeList[0]); i++) {
			if (memcmp(gameTid[CURPOS], asyncReadExcludeList[i], 3) == 0) {
				// Found match
				return false;
			}
		}
	}

	return perGameSettings_asyncCardRead == -1 ? DEFAULT_ASYNC_CARD_READ : perGameSettings_asyncCardRead;
}

/**
 * Set MPU settings for a specific game.
 */
void SetMPUSettings() {
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
}

void unlaunchRomBoot(std::string_view rom) {
	snd().stopStream();
	std::u16string path(FontGraphic::utf8to16(rom));
	if (path.substr(0, 3) == u"sd:") {
		path = u"sdmc:" + path.substr(3);
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
	for (uint i = 0; i < std::min(path.length(), 0x103u); i++) {
		((char16_t*)0x02000838)[i] = path[i];		// Unlaunch Device:/Path/Filename.ext (16bit Unicode,end by 0000h)
	}
	*(u16*)(0x0200080E) = swiCRC16(0xFFFF, (void*)0x02000810, 0x3F0);		// Unlaunch CRC16

	DC_FlushAll();						// Make reboot not fail
	fifoSendValue32(FIFO_USER_02, 1);	// Reboot into DSiWare title, booted via Unlaunch
	stop();
}

void unlaunchSetHiyaBoot(void) {
	if (access("sd:/hiya.dsi", F_OK) != 0) return;

	tonccpy((u8 *)0x02000800, unlaunchAutoLoadID, 12);
	*(u16 *)(0x0200080C) = 0x3F0;			   // Unlaunch Length for CRC16 (fixed, must be 3F0h)
	*(u16 *)(0x0200080E) = 0;			   // Unlaunch CRC16 (empty)
	*(u32 *)(0x02000810) = (BIT(0) | BIT(1));	  // Load the title at 2000838h
							   // Use colors 2000814h
	*(u16 *)(0x02000814) = 0x7FFF;			   // Unlaunch Upper screen BG color (0..7FFFh)
	*(u16 *)(0x02000816) = 0x7FFF;			   // Unlaunch Lower screen BG color (0..7FFFh)
	toncset((u8 *)0x02000818, 0, 0x20 + 0x208 + 0x1C0); // Unlaunch Reserved (zero)
	for (uint i = 0; i < sizeof(hiyaNdsPath)/sizeof(hiyaNdsPath[0]); i++) {
		((char16_t*)0x02000838)[i] = hiyaNdsPath[i];		// Unlaunch Device:/Path/Filename.ext (16bit Unicode,end by 0000h)
	}
	*(u16 *)(0x0200080E) = swiCRC16(0xFFFF, (void *)0x02000810, 0x3F0); // Unlaunch CRC16
}

/**
 * Reboot into an SD game when in DS mode.
 */
void ntrStartSdGame(void) {
	*(u32*)0x02000000 |= BIT(3);
	*(u32*)0x02000004 = 0x54455352; // 'RSET'
	if (ms().consoleModel == 0) {
		unlaunchRomBoot("sd:/_nds/TWiLightMenu/main.srldr");
	} else {
		tonccpy((u32 *)0x02000300, autoboot_bin, 0x20);
		DC_FlushAll();						// Make reboot not fail
		fifoSendValue32(FIFO_USER_02, 1);
		stop();
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
	*(u16 *)(0x02000306) = swiCRC16(0xFFFF, (void *)0x02000308, 0x18);

	unlaunchSetHiyaBoot();

	DC_FlushAll();						// Make reboot not fail
	fifoSendValue32(FIFO_USER_02, 1); // Reboot into DSiWare title, booted via Launcher
	stop();
}

sNDSHeader ndsCart;

/**
 * Enable widescreen for some games.
 */
void SetWidescreen(const char *filename) {
	const char* wideCheatDataPath = ms().secondaryDevice && (!isDSiWare[CURPOS] || (isDSiWare[CURPOS] && !ms().dsiWareToSD)) ? "fat:/_nds/nds-bootstrap/wideCheatData.bin" : "sd:/_nds/nds-bootstrap/wideCheatData.bin";
	remove(wideCheatDataPath);

	bool useWidescreen = (perGameSettings_wideScreen == -1 ? ms().wideScreen : perGameSettings_wideScreen);

	if ((isDSiMode() && sys().arm7SCFGLocked()) || ms().consoleModel < 2
	|| !useWidescreen || !widescreenFound || ms().macroMode) {
		return;
	}
	
	if (isHomebrew[CURPOS] && ms().homebrewHasWide && widescreenFound) {
		if (access("sd:/luma/sysmodules/TwlBg.cxi", F_OK) == 0) {
			rename("sd:/luma/sysmodules/TwlBg.cxi", "sd:/_nds/TWiLightMenu/TwlBg/TwlBg.cxi.bak");
		}
		if (rename("sd:/_nds/TWiLightMenu/TwlBg/Widescreen.cxi", "sd:/luma/sysmodules/TwlBg.cxi") == 0) {
			ntrStartSdGame();
		}
		return;
	}

	bool wideCheatFound = false;
	char wideBinPath[256];
	if (ms().launchType[ms().secondaryDevice] == Launch::ESDFlashcardLaunch) {
		snprintf(wideBinPath, sizeof(wideBinPath), "sd:/_nds/TWiLightMenu/extras/widescreen/%s.bin", filename);
		wideCheatFound = (access(wideBinPath, F_OK) == 0);
	}

	char s1GameTid[5];

	if (ms().slot1Launched) {
		// Reset Slot-1 to allow reading card header
		sysSetCardOwner (BUS_OWNER_ARM9);
		disableSlot1();
		for (int i = 0; i < 25; i++) { swiWaitForVBlank(); }
		enableSlot1();
		for (int i = 0; i < 15; i++) { swiWaitForVBlank(); }

		cardReadHeader((uint8*)&ndsCart);

		tonccpy(s1GameTid, ndsCart.gameCode, 4);
		s1GameTid[4] = 0;

		snprintf(wideBinPath, sizeof(wideBinPath), "sd:/_nds/TWiLightMenu/extras/widescreen/%s-%X.bin", s1GameTid, ndsCart.headerCRC16);
		wideCheatFound = (access(wideBinPath, F_OK) == 0);
	} else if (!wideCheatFound) {
		snprintf(wideBinPath, sizeof(wideBinPath), "sd:/_nds/TWiLightMenu/extras/widescreen/%s-%X.bin", gameTid[CURPOS], headerCRC[CURPOS]);
		wideCheatFound = (access(wideBinPath, F_OK) == 0);
	}

	if (isHomebrew[CURPOS]) {
		return;
	}

	mkdir(ms().secondaryDevice && (!isDSiWare[CURPOS] || (isDSiWare[CURPOS] && !ms().dsiWareToSD)) ? "fat:/_nds" : "sd:/_nds", 0777);
	mkdir(ms().secondaryDevice && (!isDSiWare[CURPOS] || (isDSiWare[CURPOS] && !ms().dsiWareToSD)) ? "fat:/_nds/nds-bootstrap" : "sd:/_nds/nds-bootstrap", 0777);

	if (wideCheatFound) {
		if (fcopy(wideBinPath, wideCheatDataPath) != 0) {
			std::string resultText = STR_FAILED_TO_COPY_WIDESCREEN;
			remove(wideCheatDataPath);
			clearText();
			printLarge(false, 0, ms().theme == TWLSettings::EThemeSaturn ? 24 : 72, resultText, Alignment::center);
			if (ms().theme != TWLSettings::EThemeSaturn) {
				fadeType = true; // Fade in from white
			}
			for (int i = 0; i < 60 * 3; i++) {
				swiWaitForVBlank(); // Wait 3 seconds
			}
			if (ms().theme != TWLSettings::EThemeSaturn) {
				fadeType = false;	   // Fade to white
				for (int i = 0; i < 25; i++) {
					swiWaitForVBlank();
				}
			}
			return;
		}
	} else {
		char *tid = ms().slot1Launched ? s1GameTid : gameTid[CURPOS];
		u16 crc16 = ms().slot1Launched ? ndsCart.headerCRC16 : headerCRC[CURPOS];

		FILE *file = fopen(sys().isRunFromSD() ? "sd:/_nds/TWiLightMenu/extras/widescreen.pck" : "fat:/_nds/TWiLightMenu/extras/widescreen.pck", "rb");
		if (file) {
			char buf[5] = {0};
			fread(buf, 1, 4, file);
			if (strcmp(buf, ".PCK") != 0) // Invalid file
				return;

			u32 fileCount;
			fread(&fileCount, 1, sizeof(fileCount), file);

			u32 offset = 0, size = 0;

			// Try binary search for the game
			int left = 0;
			int right = fileCount;

			while (left <= right) {
				int mid = left + ((right - left) / 2);
				fseek(file, 16 + mid * 16, SEEK_SET);
				fread(buf, 1, 4, file);
				int cmp = strcmp(buf, tid);
				if (cmp == 0) { // TID matches, check CRC
					u16 crc;
					fread(&crc, 1, sizeof(crc), file);

					if (crc == crc16) { // CRC matches
						fread(&offset, 1, sizeof(offset), file);
						fread(&size, 1, sizeof(size), file);
						wideCheatFound = true;
						break;
					} else if (crc < crc16) {
						left = mid + 1;
					} else {
						right = mid - 1;
					}
				} else if (cmp < 0) {
					left = mid + 1;
				} else {
					right = mid - 1;
				}
			}

			if (offset > 0) {
				fseek(file, offset, SEEK_SET);
				u8 *buffer = new u8[size];
				fread(buffer, 1, size, file);

				snprintf(wideBinPath, sizeof(wideBinPath), "%s:/_nds/nds-bootstrap/wideCheatData.bin", ms().secondaryDevice && (!isDSiWare[CURPOS] || (isDSiWare[CURPOS] && !ms().dsiWareToSD)) ? "fat" : "sd");
				FILE *out = fopen(wideBinPath, "wb");
				if (out) {
					fwrite(buffer, 1, size, out);
					fclose(out);
				}
				delete[] buffer;
			}

			fclose(file);
		}
	}
	if (wideCheatFound && widescreenFound) {
		if (access("sd:/luma/sysmodules/TwlBg.cxi", F_OK) == 0) {
			rename("sd:/luma/sysmodules/TwlBg.cxi", "sd:/_nds/TWiLightMenu/TwlBg/TwlBg.cxi.bak");
		}
		if (rename("sd:/_nds/TWiLightMenu/TwlBg/Widescreen.cxi", "sd:/luma/sysmodules/TwlBg.cxi") == 0) {
			ntrStartSdGame();
		}
	}
}

/**
 * Gets the in-game manual for a game
 */
std::string getGameManual(const char *filename) {
	char manualPath[256];
	snprintf(manualPath, sizeof(manualPath), "%s:/_nds/TWiLightMenu/extras/manuals/%s.txt", sys().isRunFromSD() ? "sd" : "fat", filename);
	if (access(manualPath, F_OK) == 0)
		return manualPath;

	snprintf(manualPath, sizeof(manualPath), "%s:/_nds/TWiLightMenu/extras/manuals/%s.txt", sys().isRunFromSD() ? "sd" : "fat", gameTid[CURPOS]);
	if (access(manualPath, F_OK) == 0)
		return manualPath;

	snprintf(manualPath, sizeof(manualPath), "%s:/_nds/TWiLightMenu/extras/manuals/%.3s.txt", sys().isRunFromSD() ? "sd" : "fat", gameTid[CURPOS]);
	if (access(manualPath, F_OK) == 0)
		return manualPath;

	return "";
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

void loadGameOnFlashcard(const char *ndsPath, bool dsGame) {
	bool runNds_boostCpu = false;
	bool runNds_boostVram = false;
	std::string filename = ndsPath;
	const size_t last_slash_idx = filename.find_last_of("/");
	if (std::string::npos != last_slash_idx) {
		filename.erase(0, last_slash_idx + 1);
	}

	loadPerGameSettings(filename);

	if (dsiFeatures() && dsGame) {
		runNds_boostCpu = perGameSettings_boostCpu == -1 ? DEFAULT_BOOST_CPU : perGameSettings_boostCpu;
		runNds_boostVram = perGameSettings_boostVram == -1 ? DEFAULT_BOOST_VRAM : perGameSettings_boostVram;
	}
	if (dsGame) {
		// Move .sav outside of "saves" folder for flashcard kernel usage
		std::string typeToReplace = filename.substr(filename.rfind('.'));

		std::string savename = replaceAll(filename, typeToReplace, getSavExtension());
		std::string savenameFc = replaceAll(filename, typeToReplace, ".sav");
		std::string romfolder = ndsPath;
		while (!romfolder.empty() && romfolder[romfolder.size()-1] != '/') {
			romfolder.resize(romfolder.size()-1);
		}
		std::string romFolderNoSlash = romfolder;
		RemoveTrailingSlashes(romFolderNoSlash);
		chdir(romFolderNoSlash.c_str());
		std::string savepath = romFolderNoSlash + "/saves/" + savename;
		if (ms().saveLocation == TWLSettings::ETWLMFolder) {
			std::string twlmSavesFolder = sys().isRunFromSD() ? "sd:/_nds/TWiLightMenu/saves" : "fat:/_nds/TWiLightMenu/saves";
			mkdir(twlmSavesFolder.c_str(), 0777);
			savepath = twlmSavesFolder + "/" + savename;
		} else if (ms().saveLocation == TWLSettings::EGamesFolder) {
			savepath = romFolderNoSlash + "/" + savename;
		} else {
			mkdir("saves", 0777);
		}
		std::string savepathFc = romFolderNoSlash + "/" + savenameFc;
		if (savepath[0] == savepathFc[0] && savepath[1] == savepathFc[1] && savepath[2] == savepathFc[2]) {
			rename(savepath.c_str(), savepathFc.c_str());
		} else {
			fcopy(savepath.c_str(), savepathFc.c_str());
		}
	}

	std::string fcPath;
	int err = 0;
	snd().stopStream();

	if ((memcmp(io_dldi_data->friendlyName, "R4(DS) - Revolution for DS", 26) == 0)
	 || (memcmp(io_dldi_data->friendlyName, "R4TF", 4) == 0)
	 || (memcmp(io_dldi_data->friendlyName, "R4iDSN", 6) == 0)
	 || (memcmp(io_dldi_data->friendlyName, "R4iTT", 5) == 0)
	 || (memcmp(io_dldi_data->friendlyName, "Acekard AK2", 0xB) == 0)
     || (memcmp(io_dldi_data->friendlyName, "Ace3DS+", 7) == 0)) {
		if (sys().hasRegulableBacklight()) {
			CIniFile backlightini("fat:/_wfwd/backlight.ini");
			backlightini.SetInt("brightness", "brightness", *(int*)0x02003000);
			backlightini.SaveIniFile("fat:/_wfwd/backlight.ini");
		}
		CIniFile fcrompathini("fat:/_wfwd/lastsave.ini");
		fcPath = replaceAll(ndsPath, "fat:/", woodfat);
		fcrompathini.SetString("Save Info", "lastLoaded", fcPath);
		fcrompathini.SaveIniFile("fat:/_wfwd/lastsave.ini");
		err = runNdsFile("fat:/Wfwd.dat", 0, NULL, sys().isRunFromSD(), true, true, true, runNds_boostCpu, runNds_boostVram, false, -1);
	} else if (memcmp(io_dldi_data->friendlyName, "DSTWO(Slot-1)", 0xD) == 0) {
		CIniFile fcrompathini("fat:/_dstwo/autoboot.ini");
		fcPath = replaceAll(ndsPath, "fat:/", dstwofat);
		fcrompathini.SetString("Dir Info", "fullName", fcPath);
		fcrompathini.SaveIniFile("fat:/_dstwo/autoboot.ini");
		err = runNdsFile("fat:/_dstwo/autoboot.nds", 0, NULL, sys().isRunFromSD(), true, true, true, runNds_boostCpu, runNds_boostVram, false, -1);
	} else if ((memcmp(io_dldi_data->friendlyName, "TTCARD", 6) == 0)
			 || (memcmp(io_dldi_data->friendlyName, "DSTT", 4) == 0)
			 || (memcmp(io_dldi_data->friendlyName, "DEMON", 5) == 0)
			 || (memcmp(io_dldi_data->friendlyName, "DSONE", 5) == 0)
			 || (memcmp(io_dldi_data->friendlyName, "M3DS", 4) == 0)
			 || (memcmp(io_dldi_data->friendlyName, "M3-DS", 5) == 0)) {
		CIniFile fcrompathini("fat:/TTMenu/YSMenu.ini");
		fcPath = replaceAll(ndsPath, "fat:/", slashchar);
		fcrompathini.SetString("YSMENU", "AUTO_BOOT", fcPath);
		fcrompathini.SaveIniFile("fat:/TTMenu/YSMenu.ini");
		err = runNdsFile("fat:/YSMenu.nds", 0, NULL, sys().isRunFromSD(), true, true, true, runNds_boostCpu, runNds_boostVram, false, -1);
	}

	char text[64];
	snprintf(text, sizeof(text), STR_START_FAILED_ERROR.c_str(), err);
	clearText(false);
	if (err == 0) {
		printLarge(false, 4, 4, STR_ERROR_FLASHCARD_UNSUPPORTED);
		printLarge(false, 4, 68, io_dldi_data->friendlyName);
	} else {
		printLarge(false, 4, 4, text);
	}
	printSmall(false, 4, 90, STR_PRESS_B_RETURN);
	updateText(false);
	fadeSpeed = true; // Fast fading
	if (ms().theme != TWLSettings::EThemeSaturn && ms().theme != TWLSettings::EThemeHBL) {
		whiteScreen = true;
		tex().clearTopScreen();
	}
	fadeType = true;	// Fade from white
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
	vector<char *> argarray;
	argarray.push_back((char*)(sys().isRunFromSD() ? "sd:/_nds/TWiLightMenu/dsimenu.srldr" : "fat:/_nds/TWiLightMenu/dsimenu.srldr"));
	runNdsFile(argarray[0], argarray.size(), (const char**)&argarray[0], sys().isRunFromSD(), true, false, false, true, true, false, -1);
	stop();
}

// From NTM
// https://github.com/Epicpkmn11/NTM/blob/db69aca1b49733da51f64ee857ac9b861b1c468c/arm9/src/sav.c#L7-L93
bool createDSiWareSave(const char *path, int size) {
	const u16 sectorSize = 0x200;

	if (!path || size < sectorSize)
		return false;

	//fit maximum sectors for the size
	const u16 maxSectors = size / sectorSize;
	u16 sectorCount = 1;
	u16 secPerTrk = 1;
	u16 numHeads = 1;
	u16 sectorCountNext = 0;
	while (sectorCountNext <= maxSectors) {
		sectorCountNext = secPerTrk * (numHeads + 1) * (numHeads + 1);
		if (sectorCountNext <= maxSectors) {
			numHeads++;
			sectorCount = sectorCountNext;

			secPerTrk++;
			sectorCountNext = secPerTrk * numHeads * numHeads;
			if (sectorCountNext <= maxSectors) {
				sectorCount = sectorCountNext;
			}
		}
	}
	sectorCountNext = (secPerTrk + 1) * numHeads * numHeads;
	if (sectorCountNext <= maxSectors) {
		secPerTrk++;
		sectorCount = sectorCountNext;
	}

	u8 secPerCluster = (sectorCount > (8 << 10)) ? 8 : (sectorCount > (1 << 10) ? 4 : 1);

	u16 rootEntryCount = size < 0x8C000 ? 0x20 : 0x200;

	#define ALIGN_TO_MULTIPLE(v, a) (((v) % (a)) ? ((v) + (a) - ((v) % (a))) : (v))
	u16 totalClusters = ALIGN_TO_MULTIPLE(sectorCount, secPerCluster) / secPerCluster;
	u32 fatBytes = (ALIGN_TO_MULTIPLE(totalClusters, 2) / 2) * 3; // 2 sectors -> 3 byte
	u16 fatSize = ALIGN_TO_MULTIPLE(fatBytes, sectorSize) / sectorSize;


	FATHeader h;
	toncset(&h, 0, sizeof(FATHeader));

	h.BS_JmpBoot[0] = 0xE9;
	h.BS_JmpBoot[1] = 0;
	h.BS_JmpBoot[2] = 0;

	tonccpy(h.BS_OEMName, "MSWIN4.1", 8);

	h.BPB_BytesPerSec = sectorSize;
	h.BPB_SecPerClus = secPerCluster;
	h.BPB_RsvdSecCnt = 0x0001;
	h.BPB_NumFATs = 0x02;
	h.BPB_RootEntCnt = rootEntryCount;
	h.BPB_TotSec16 = sectorCount;
	h.BPB_Media = 0xF8; // "hard drive"
	h.BPB_FATSz16 = fatSize;
	h.BPB_SecPerTrk = secPerTrk;
	h.BPB_NumHeads = numHeads;
	h.BS_DrvNum = 0x05;
	h.BS_BootSig = 0x29;
	h.BS_VolID = 0x12345678;
	tonccpy(h.BS_VolLab, "VOLUMELABEL", 11);
	tonccpy(h.BS_FilSysType,"FAT12   ", 8);
	h.BS_BootSign = 0xAA55;

	FILE *file = fopen(path, "wb");
	if (file) {
		showProgressBar = true;
		fwrite(&h, sizeof(FATHeader), 1, file); // Write header
		int i = 0;
		while (1) {
			i += 0x8000;
			if (i > size) i = size;
			progressBarLength = i/(size/192);
			fseek(file, i - 1, SEEK_SET); // Pad rest of the file
			fputc('\0', file);
			if (i == size) break;
		}
		fclose(file);
		showProgressBar = false;
		return true;
	}

	return false;
}

void createSaveFile(const char* savePath, const bool isHomebrew, const char* gameTid) {
	if (isHomebrew) { // Create or expand save if game isn't homebrew
		return;
	}

	u32 orgsavesize = getFileSize(savePath);
	u32 savesize = 524288; // 512KB (default size for most games)

	u32 gameTidHex = 0;
	tonccpy(&gameTidHex, gameTid, 4);

	for (int i = 0; i < (int)sizeof(ROMList)/12; i++) {
		ROMListEntry* curentry = &ROMList[i];
		if (gameTidHex == curentry->GameCode) {
			if (curentry->SaveMemType != 0xFFFFFFFF) savesize = sramlen[curentry->SaveMemType];
			break;
		}
	}

	if ((orgsavesize == 0 && savesize > 0) || (orgsavesize < savesize)) {
		if (ms().theme == TWLSettings::EThemeHBL) {
			displayGameIcons = false;
		} else if (ms().theme != TWLSettings::EThemeSaturn) {
			while (!screenFadedOut()) {
				swiWaitForVBlank();
			}
			whiteScreen = true;
			tex().clearTopScreen();
		}
		clearText();
		printLarge(false, 0, (ms().theme == TWLSettings::EThemeSaturn ? 80 : 88), (orgsavesize == 0) ? STR_CREATING_SAVE : STR_EXPANDING_SAVE, Alignment::center);
		updateText(false);

		if (ms().theme != TWLSettings::EThemeSaturn && ms().theme != TWLSettings::EThemeHBL) {
			fadeSpeed = true; // Fast fading
			fadeType = true; // Fade in from white
		}
		showProgressIcon = true;

		FILE *pFile = fopen(savePath, orgsavesize > 0 ? "r+" : "wb");
		if (pFile) {
			showProgressBar = true;
			u32 i = (orgsavesize>0 ? orgsavesize : 0);
			while (1) {
				i += 0x8000;
				if (i > savesize) i = savesize;
				progressBarLength = i/(savesize/192);
				fseek(pFile, i - 1, SEEK_SET);
				fputc('\0', pFile);
				if (i == savesize) break;
			}
			fclose(pFile);
			showProgressBar = false;
		}
		showProgressIcon = false;
		clearText();
		printLarge(false, 0, (ms().theme == TWLSettings::EThemeSaturn ? 32 : 88), (orgsavesize == 0) ? STR_SAVE_CREATED : STR_SAVE_EXPANDED, Alignment::center);
		updateText(false);
		for (int i = 0; i < 30; i++) {
			swiWaitForVBlank();
		}
		if (ms().theme == TWLSettings::EThemeSaturn || ms().theme == TWLSettings::EThemeHBL) {
			clearText();
			updateText(false);
		} else {
			fadeType = false;	   // Fade to white
		}
		if (ms().theme == TWLSettings::EThemeHBL) displayGameIcons = true;
	}
}

void prepareCheats(std::string path, const bool dsiWare) {
	if (isHomebrew[CURPOS]) {
		return;
	}

	CheatCodelist codelist;
	u32 gameCode, crc32;

	bool cheatsEnabled = true;
	const char* cheatDataBin = dsiWare ? "sd:/_nds/nds-bootstrap/cheatData.bin" : "/_nds/nds-bootstrap/cheatData.bin";
	mkdir(dsiWare ? "sd:/_nds" : "/_nds", 0777);
	mkdir(dsiWare ? "sd:/_nds/nds-bootstrap" : "/_nds/nds-bootstrap", 0777);
	if (codelist.romData(path,gameCode,crc32)) {
		long cheatOffset; size_t cheatSize;
		FILE* dat=fopen(sys().isRunFromSD() ? "sd:/_nds/TWiLightMenu/extras/usrcheat.dat" : "fat:/_nds/TWiLightMenu/extras/usrcheat.dat","rb");
		if (dat) {
			if (codelist.searchCheatData(dat, gameCode, crc32, cheatOffset, cheatSize)) {
				loadPerGameSettings(path.substr(path.find_last_of('/') + 1));
				codelist.parse(path);
				codelist.writeCheatsToFile(cheatDataBin);
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

void s2RamAccess(bool open) {
	if (io_dldi_data->ioInterface.features & FEATURE_SLOT_NDS) return;

	if (open) {
		if (*(u16*)(0x020000C0) == 0x334D) {
			_M3_changeMode(M3_MODE_RAM);
		} else if (*(u16*)(0x020000C0) == 0x3647) {
			_G6_SelectOperation(G6_MODE_RAM);
		} else if (*(u16*)(0x020000C0) == 0x4353) {
			_SC_changeMode(SC_MODE_RAM);
		}
	} else {
		if (*(u16*)(0x020000C0) == 0x334D) {
			_M3_changeMode(M3_MODE_MEDIA);
		} else if (*(u16*)(0x020000C0) == 0x3647) {
			_G6_SelectOperation(G6_MODE_MEDIA);
		} else if (*(u16*)(0x020000C0) == 0x4353) {
			_SC_changeMode(SC_MODE_MEDIA);
		}
	}
}

void gbaSramAccess(bool open) {
	if (open) {
		if (*(u16*)(0x020000C0) == 0x334D) {
			_M3_changeMode(M3_MODE_RAM);
		} else if (*(u16*)(0x020000C0) == 0x3647) {
			_G6_SelectOperation(G6_MODE_RAM);
		} else if (*(u16*)(0x020000C0) == 0x4353) {
			_SC_changeMode(SC_MODE_RAM_RO);
		}
	} else {
		if (*(u16*)(0x020000C0) == 0x334D) {
			_M3_changeMode((io_dldi_data->ioInterface.features & FEATURE_SLOT_GBA) ? M3_MODE_MEDIA : M3_MODE_RAM);
		} else if (*(u16*)(0x020000C0) == 0x3647) {
			_G6_SelectOperation((io_dldi_data->ioInterface.features & FEATURE_SLOT_GBA) ? G6_MODE_MEDIA : G6_MODE_RAM);
		} else if (*(u16*)(0x020000C0) == 0x4353) {
			_SC_changeMode((io_dldi_data->ioInterface.features & FEATURE_SLOT_GBA) ? SC_MODE_MEDIA : SC_MODE_RAM);
		}
	}
}

void customSleep() {
	bool topControlBak = controlTopBright;
	bool botControlBak = controlBottomBright;
	controlTopBright = true;
	controlBottomBright = true;

	snd().stopStream();
	fadeSleep = true;
	fadeType = false;
	while (!screenFadedOut()) {
		swiWaitForVBlank();
	}
	if (!ms().macroMode) {
		powerOff(PM_BACKLIGHT_TOP);
	}
	powerOff(PM_BACKLIGHT_BOTTOM);
	irqDisable(IRQ_VBLANK & IRQ_VCOUNT);
	while (keysHeld() & KEY_LID) {
		scanKeys();
		swiWaitForVBlank();
	}
	irqEnable(IRQ_VBLANK & IRQ_VCOUNT);
	if (!ms().macroMode) {
		powerOn(PM_BACKLIGHT_TOP);
	}
	powerOn(PM_BACKLIGHT_BOTTOM);
	if (!ms().macroMode && ms().showPhoto && tc().renderPhoto()) {
		srand(time(NULL));
		loadPhotoList();

		extern bool boxArtLoaded;
		extern bool showLshoulder;
		extern bool showRshoulder;
		extern int file_count;

		boxArtLoaded = false;
		showLshoulder = (PAGENUM != 0);
		showRshoulder = (file_count > 40 + PAGENUM * 40);
		if (ms().theme != TWLSettings::EThemeHBL) {
			tex().drawShoulders(showLshoulder, showRshoulder);
		}
	}
	fadeType = true;
	snd().beginStream();
	while (!screenFadedIn()) {
		swiWaitForVBlank();
	}
	fadeSleep = false;
	controlTopBright = topControlBak;
	controlBottomBright = botControlBak;
}

void bgOperations(bool waitFrame) {
	if ((keysHeld() & KEY_LID) && ms().sleepMode) {
		customSleep();
	}
	checkSdEject();
	tex().drawVolumeImageCached();
	tex().drawBatteryImageCached();
	drawCurrentTime();
	drawCurrentDate();
	snd().updateStream();
	if (waitFrame) {
		swiWaitForVBlank();
	}
}

int dsiMenuTheme(void) {
	ms().loadSettings();
	bs().loadSettings();
	logInit();
	if (sdFound() && ms().consoleModel >= 2 && (!isDSiMode() || !sys().arm7SCFGLocked())) {
		CIniFile lumaConfig("sd:/luma/config.ini");
		widescreenFound = ((access("sd:/_nds/TWiLightMenu/TwlBg/Widescreen.cxi", F_OK) == 0) && (lumaConfig.GetInt("boot", "enable_external_firm_and_modules", 0) == true));
		logPrint(widescreenFound ? "Widescreen found\n" : "Widescreen not found\n");
	}
	if (ms().theme == TWLSettings::EThemeDSi && sys().isRegularDS()) {
		useRumble = my_isRumbleInserted();
		logPrint(useRumble ? "Rumble found\n" : "Rumble not found\n");
	}
	tfn(); //
	tc().loadConfig();
	tex().videoSetup(); // allocate texture pointers

	fontInit();
	logPrint("\n");

	if (ms().theme == TWLSettings::EThemeHBL) {
		tex().loadHBTheme();
	} else if (ms().theme == TWLSettings::EThemeSaturn) {
		tex().loadSaturnTheme();
	} else if (ms().theme == TWLSettings::ETheme3DS) {
		tex().load3DSTheme();
	} else {
		tex().loadDSiTheme();
	}

	ms().gbaR3Test = (access(sys().isRunFromSD() ? "sd:/_nds/TWiLightMenu/emulators/GBARunner3.nds" : "fat:/_nds/TWiLightMenu/emulators/GBARunner3.nds", F_OK) == 0);

	if (sdFound()) {
		statvfs("sd:/", &st[0]);

		gbaBiosFound[0] = (access("sd:/_gba/bios.bin", F_OK) == 0);
		if (!ms().gbaR3Test) {
			if (!gbaBiosFound[0]) gbaBiosFound[0] = (access("sd:/gba/bios.bin", F_OK) == 0);
			if (!gbaBiosFound[0]) gbaBiosFound[0] = (access("sd:/bios.bin", F_OK) == 0);
		}
		logPrint(gbaBiosFound[0] ? "GBA BIOS found on sd\n" : "GBA BIOS not found on sd\n");
	}
	if (flashcardFound()) {
		statvfs("fat:/", &st[1]);

		gbaBiosFound[1] = (access("fat:/_gba/bios.bin", F_OK) == 0);
		if (!ms().gbaR3Test) {
			if (!gbaBiosFound[1]) gbaBiosFound[1] = (access("fat:/gba/bios.bin", F_OK) == 0);
			if (!gbaBiosFound[1]) gbaBiosFound[1] = (access("fat:/bios.bin", F_OK) == 0);
		}
		logPrint(gbaBiosFound[1] ? "GBA BIOS found on fat\n" : "GBA BIOS not found on fat\n");
	}

	if (ms().theme == TWLSettings::EThemeSaturn || ms().theme == TWLSettings::EThemeHBL) {
		whiteScreen = false;
	}

	langInit();

	std::string filename;

	if (sdFound() && ms().consoleModel < 2 && ms().launcherApp != -1) {
		u8 setRegion = 0;

		if (ms().sysRegion == -1) {
			// Determine SysNAND region by searching region of System Settings on SDNAND
			FILE* f_hwinfoS = fopen("sd:/sys/HWINFO_S.dat", "rb");
			if (f_hwinfoS) {
				fseek(f_hwinfoS, 0xA0, SEEK_SET);
				fread(&setRegion, 1, 1, f_hwinfoS);
				fclose(f_hwinfoS);
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

		if (ms().sysRegion == 9) {
			sprintf(launcherPath, "nand:/launcher.dsi");
		} else {
			sprintf(launcherPath,
				 "nand:/title/00030017/484E41%X/content/0000000%i.app", setRegion, ms().launcherApp);
		}
	}

	srand(time(NULL));
	
	graphicsInit();
	iconManagerInit();

	keysSetRepeat(10, 2);

	logPrint("snd()\n");
	snd();

	if (ms().theme == TWLSettings::EThemeSaturn) {
		//logPrint("snd().playStartup()\n");
		snd().playStartup();
	} else if (ms().theme != TWLSettings::EThemeHBL) {
		controlTopBright = false;
	}

	if (ms().previousUsedDevice && bothSDandFlashcard() && ms().launchType[ms().previousUsedDevice] == Launch::EDSiWareLaunch
	&& ((access(ms().dsiWarePubPath.c_str(), F_OK) == 0 && access("sd:/_nds/TWiLightMenu/tempDSiWare.pub", F_OK) == 0)
	 || (access(ms().dsiWarePrvPath.c_str(), F_OK) == 0 && access("sd:/_nds/TWiLightMenu/tempDSiWare.prv", F_OK) == 0))) {
		fadeType = true; // Fade in from white
		printLarge(false, 0, (ms().theme == TWLSettings::EThemeSaturn ? 80 : 88), STR_NOW_COPYING_DATA, Alignment::center);
		printSmall(false, 0, (ms().theme == TWLSettings::EThemeSaturn ? 96 : 104), STR_DONOT_TURNOFF_POWER, Alignment::center);
		updateText(false);
		for (int i = 0; i < 30; i++) {
			swiWaitForVBlank();
		}
		showProgressIcon = true;
		if (access(ms().dsiWarePubPath.c_str(), F_OK) == 0) {
			fcopy("sd:/_nds/TWiLightMenu/tempDSiWare.pub", ms().dsiWarePubPath.c_str());
			rename("sd:/_nds/TWiLightMenu/tempDSiWare.pub", "sd:/_nds/TWiLightMenu/tempDSiWare.pub.bak");
		}
		if (access(ms().dsiWarePrvPath.c_str(), F_OK) == 0) {
			fcopy("sd:/_nds/TWiLightMenu/tempDSiWare.prv", ms().dsiWarePrvPath.c_str());
			rename("sd:/_nds/TWiLightMenu/tempDSiWare.prv", "sd:/_nds/TWiLightMenu/tempDSiWare.prv.bak");
		}
		showProgressIcon = false;
		logPrint("Copied DSiWare save back to flashcard\n");
		if (ms().theme != TWLSettings::EThemeSaturn) {
			fadeType = false; // Fade to white
			for (int i = 0; i < 25; i++) {
				swiWaitForVBlank();
			}
		}
		clearText();
		updateText(false);
	}

	while (1) {
		std::vector<std::string_view> extensionList = {
			".nds", ".dsi", ".ids", ".srl", ".app", ".argv", // NDS
			".agb", ".gba", ".mb", // GBA
			".a26", // Atari 2600
			".a52", // Atari 5200
			".a78", // Atari 7800
			".xex", ".atr", // Atari XEGS
			".msx", // MSX
			".col", // ColecoVision
			".int", // Intellivision
			".m5", // Sord M5
			".gb", ".sgb", ".gbc", // Game Boy
			".nes", ".fds", // NES/Famicom
			".sg", // Sega SG-1000
			".sc", // Sega SC-3000
			".sms", // Sega Master System
			".gg", // Sega Game Gear
			".gen", // Genesis
			".smc", ".sfc", // SNES
			".ws", ".wsc", // WonderSwan
			".ngp", ".ngc", // Neo Geo Pocket
			".pce", // PC Engine/TurboGrafx-16
			".dsk", // Amstrad CPC
			".min", // Pokémon mini
			".avi", // Xvid (AVI)
			".fv", // FastVideo
			".gif", // GIF
			".bmp", // BMP
			".png" // Portable Network Graphics
		};

		{
			char currentDate[16];
			time_t Raw;
			time(&Raw);
			const struct tm *Time = localtime(&Raw);

			strftime(currentDate, sizeof(currentDate), "%m/%d", Time);

			if (strcmp(currentDate, "04/01") == 0) {
				// April Fools extensions
				if (dsiFeatures() && ms().consoleModel < 2) {
					// 3DS
					extensionList.emplace_back(".3ds");
					extensionList.emplace_back(".cia");
					extensionList.emplace_back(".cxi");
				}
				extensionList.emplace_back(".ntrb"); // ShaberuSoft
			}
		}

		if (!ms().secondaryDevice || ms().mdEmulator == 2) {
			extensionList.emplace_back(".md"); // Sega Mega Drive
		}

		if (memcmp(io_dldi_data->friendlyName, "DSTWO(Slot-1)", 0xD) == 0) {
			extensionList.emplace_back(".plg"); // DSTWO Plugin
		}

		if(ms().blockedExtensions.size() > 0) {
			auto toErase = std::remove_if(extensionList.begin(), extensionList.end(), [](std::string_view str) {
				return std::find(ms().blockedExtensions.begin(), ms().blockedExtensions.end(), str) != ms().blockedExtensions.end();
			});
			extensionList.erase(toErase, extensionList.end());
		}

		char path[256] = {0};
		snprintf(path, sizeof(path), "%s", ms().romfolder[ms().secondaryDevice].c_str());
		// Set directory
		chdir(path);

		// Navigates to the file to launch
		filename = browseForFile(extensionList);

		extensionList.clear();

		////////////////////////////////////
		// Launch the item

		if (applaunch) {
			// Delete previously launched DSiWare copied from flashcard to SD
			if (access("sd:/_nds/TWiLightMenu/tempDSiWare.dsi", F_OK) == 0) {
				remove("sd:/_nds/TWiLightMenu/tempDSiWare.dsi");
			}
			if (access("sd:/_nds/TWiLightMenu/tempDSiWare.pub.bak", F_OK) == 0) {
				remove("sd:/_nds/TWiLightMenu/tempDSiWare.pub.bak");
			}
			if (access("sd:/_nds/TWiLightMenu/tempDSiWare.prv.bak", F_OK) == 0) {
				remove("sd:/_nds/TWiLightMenu/tempDSiWare.prv.bak");
			}
			if (access("sd:/_nds/nds-bootstrap/patchOffsetCache/tempDSiWare.bin", F_OK) == 0) {
				remove("sd:/_nds/nds-bootstrap/patchOffsetCache/tempDSiWare.bin");
			}

			// Construct a command line
			getcwd(filePath, PATH_MAX);
			int pathLen = strlen(filePath);
			if (pathLen < PATH_MAX && filePath[pathLen - 1] != '/') { // Ensure the path ends in a slash
				filePath[pathLen] = '/';
				filePath[pathLen + 1] = '\0';
				pathLen++;
			}
			vector<char *> argarray;

			bool isArgv = false;
			if (extension(filename, {".argv"})) {
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

			ms().slot1Launched = false;

			// Launch DSiWare .nds via Unlaunch
			if (isDSiWare[CURPOS]) {
				remove(sys().isRunFromSD() ? "sd:/_nds/nds-bootstrap/esrb.bin" : "fat:/_nds/nds-bootstrap/esrb.bin");

				std::string typeToReplace = filename.substr(filename.rfind('.'));

				char *name = argarray.at(0);
				strcpy(filePath + pathLen, name);
				free(argarray.at(0));
				argarray.at(0) = filePath;

				std::string romFolderNoSlash = ms().romfolder[ms().secondaryDevice];
				RemoveTrailingSlashes(romFolderNoSlash);

				ms().dsiWareSrlPath = std::string(argarray[0]);
				ms().dsiWarePubPath = romFolderNoSlash + "/saves/" + filename;
				if (ms().saveLocation == TWLSettings::ETWLMFolder) {
					std::string twlmSavesFolder = sys().isRunFromSD() ? "sd:/_nds/TWiLightMenu/saves" : "fat:/_nds/TWiLightMenu/saves";
					mkdir(twlmSavesFolder.c_str(), 0777);
					ms().dsiWarePubPath = twlmSavesFolder + "/" + filename;
				} else if (ms().saveLocation == TWLSettings::EGamesFolder) {
					ms().dsiWarePubPath = romFolderNoSlash + "/" + filename;
				} else {
					mkdir("saves", 0777);
				}
				ms().dsiWarePrvPath = ms().dsiWarePubPath;
				const bool savFormat = (ms().secondaryDevice && (!isDSiMode() || !sys().scfgSdmmcEnabled() || bs().b4dsMode));
				if (savFormat) {
					ms().dsiWarePubPath = replaceAll(ms().dsiWarePubPath, typeToReplace, getSavExtension());
					ms().dsiWarePrvPath = ms().dsiWarePubPath;
				} else {
					ms().dsiWarePubPath = replaceAll(ms().dsiWarePubPath, typeToReplace, getPubExtension());
					ms().dsiWarePrvPath = replaceAll(ms().dsiWarePrvPath, typeToReplace, getPrvExtension());
				}
				if (!isArgv) {
					ms().romPath[ms().secondaryDevice] = std::string(argarray[0]);
				}
				ms().homebrewBootstrap = isHomebrew[CURPOS];
				ms().launchType[ms().secondaryDevice] = Launch::EDSiWareLaunch;
				ms().previousUsedDevice = ms().secondaryDevice;
				ms().saveSettings();

				sNDSHeaderExt NDSHeader;

				FILE *f_nds_file = fopen(filename.c_str(), "rb");
				fread(&NDSHeader, 1, sizeof(NDSHeader), f_nds_file);
				fclose(f_nds_file);

				if (savFormat) {
					if ((getFileSize(ms().dsiWarePubPath.c_str()) == 0) && ((NDSHeader.pubSavSize > 0) || (NDSHeader.prvSavSize > 0))) {
						if (ms().theme == TWLSettings::EThemeHBL) {
							displayGameIcons = false;
						} else if (ms().theme != TWLSettings::EThemeSaturn) {
							while (!screenFadedOut()) {
								swiWaitForVBlank();
							}
							fadeSpeed = true; // Fast fading
							whiteScreen = true;
							tex().clearTopScreen();
						}
						clearText();
						printLarge(false, 0, (ms().theme == TWLSettings::EThemeSaturn ? 80 : 88), STR_CREATING_SAVE, Alignment::center);
						updateText(false);
						if (ms().theme != TWLSettings::EThemeSaturn && !fadeType) {
							fadeType = true; // Fade in from white
						}
						showProgressIcon = true;
						FILE *pFile = fopen(ms().dsiWarePubPath.c_str(), "wb");
						if (pFile) {
							showProgressBar = true;
							u32 savesize = ((NDSHeader.pubSavSize > 0) ? NDSHeader.pubSavSize : NDSHeader.prvSavSize);
							u32 i = 0;
							while (1) {
								i += 0x8000;
								if (i > savesize) i = savesize;
								progressBarLength = i/(savesize/192);
								fseek(pFile, i - 1, SEEK_SET);
								fputc('\0', pFile);
								if (i == savesize) break;
							}
							fclose(pFile);
							showProgressBar = false;
						}
						showProgressIcon = false;
						clearText();
						printLarge(false, 0, (ms().theme == TWLSettings::EThemeSaturn ? 32 : 88), STR_SAVE_CREATED, Alignment::center);
						updateText(false);
						for (int i = 0; i < 60; i++) {
							swiWaitForVBlank();
						}
						if (ms().theme == TWLSettings::EThemeHBL) displayGameIcons = true;
					}
				} else {
					if ((getFileSize(ms().dsiWarePubPath.c_str()) == 0) && (NDSHeader.pubSavSize > 0)) {
						if (ms().theme == TWLSettings::EThemeHBL) {
							displayGameIcons = false;
						} else if (ms().theme != TWLSettings::EThemeSaturn) {
							while (!screenFadedOut()) {
								swiWaitForVBlank();
							}
							fadeSpeed = true; // Fast fading
							whiteScreen = true;
							tex().clearTopScreen();
						}
						clearText();
						printLarge(false, 0, (ms().theme == TWLSettings::EThemeSaturn ? 80 : 88), STR_CREATING_PUBLIC_SAVE, Alignment::center);
						updateText(false);
						if (ms().theme != TWLSettings::EThemeSaturn && !fadeType) {
							fadeType = true; // Fade in from white
						}
						showProgressIcon = true;
						createDSiWareSave(ms().dsiWarePubPath.c_str(), NDSHeader.pubSavSize);
						showProgressIcon = false;
						clearText();
						printLarge(false, 0, (ms().theme == TWLSettings::EThemeSaturn ? 32 : 88), STR_PUBLIC_SAVE_CREATED, Alignment::center);
						updateText(false);
						for (int i = 0; i < 60; i++) {
							swiWaitForVBlank();
						}
						if (ms().theme == TWLSettings::EThemeHBL) displayGameIcons = true;
					}

					if ((getFileSize(ms().dsiWarePrvPath.c_str()) == 0) && (NDSHeader.prvSavSize > 0)) {
						if (ms().theme == TWLSettings::EThemeHBL) {
							displayGameIcons = false;
						} else if (ms().theme != TWLSettings::EThemeSaturn) {
							while (!fadeType && !screenFadedOut()) {
								swiWaitForVBlank();
							}
							fadeSpeed = true; // Fast fading
							whiteScreen = true;
							tex().clearTopScreen();
						}
						clearText();
						printLarge(false, 0, (ms().theme == TWLSettings::EThemeSaturn ? 80 : 88), STR_CREATING_PRIVATE_SAVE, Alignment::center);
						updateText(false);
						if (ms().theme != TWLSettings::EThemeSaturn && !fadeType) {
							fadeType = true; // Fade in from white
						}
						showProgressIcon = true;
						createDSiWareSave(ms().dsiWarePrvPath.c_str(), NDSHeader.prvSavSize);
						showProgressIcon = false;
						clearText();
						printLarge(false, 0, (ms().theme == TWLSettings::EThemeSaturn ? 32 : 88), STR_PRIVATE_SAVE_CREATED, Alignment::center);
						updateText(false);
						for (int i = 0; i < 60; i++) {
							swiWaitForVBlank();
						}
						if (ms().theme == TWLSettings::EThemeHBL) displayGameIcons = true;
					}
				}

				if (ms().theme != TWLSettings::EThemeSaturn && ms().theme != TWLSettings::EThemeHBL && fadeType) {
					fadeType = false; // Fade to white
				}

				loadPerGameSettings(filename);

				if (ms().secondaryDevice && !bs().b4dsMode && (ms().dsiWareToSD || (!(perGameSettings_dsiwareBooter == -1 ? ms().dsiWareBooter : perGameSettings_dsiwareBooter) && ms().consoleModel == 0)) && sdFound()) {
					if (ms().theme != TWLSettings::EThemeHBL && ms().theme != TWLSettings::EThemeSaturn) {
						while (!fadeType && !screenFadedOut()) {
							swiWaitForVBlank();
						}
						fadeSpeed = true; // Fast fading
						whiteScreen = true;
						tex().clearTopScreen();
					}
					clearText();
					printLarge(false, 0, (ms().theme == TWLSettings::EThemeSaturn ? 80 : 88), STR_NOW_COPYING_DATA, Alignment::center);
					printSmall(false, 0, (ms().theme == TWLSettings::EThemeSaturn ? 96 : 104), STR_DONOT_TURNOFF_POWER, Alignment::center);
					updateText(false);
					if (ms().theme != TWLSettings::EThemeSaturn) {
						fadeType = true; // Fade in from white
					}
					showProgressIcon = true;
					fcopy(ms().dsiWareSrlPath.c_str(), "sd:/_nds/TWiLightMenu/tempDSiWare.dsi");
					if ((access(ms().dsiWarePubPath.c_str(), F_OK) == 0) && (NDSHeader.pubSavSize > 0)) {
						fcopy(ms().dsiWarePubPath.c_str(), "sd:/_nds/TWiLightMenu/tempDSiWare.pub");
					}
					if ((access(ms().dsiWarePrvPath.c_str(), F_OK) == 0) && (NDSHeader.prvSavSize > 0)) {
						fcopy(ms().dsiWarePrvPath.c_str(), "sd:/_nds/TWiLightMenu/tempDSiWare.prv");
					}
					showProgressIcon = false;
					if (ms().theme != TWLSettings::EThemeSaturn && ms().theme != TWLSettings::EThemeHBL) {
						fadeType = false; // Fade to white
					}

					if ((access(ms().dsiWarePubPath.c_str(), F_OK) == 0 && (NDSHeader.pubSavSize > 0))
					 || (access(ms().dsiWarePrvPath.c_str(), F_OK) == 0 && (NDSHeader.prvSavSize > 0))) {
						for (int i = 0; i < 25; i++) {
							swiWaitForVBlank();
						}
						clearText();
						printLarge(false, 0, ms().theme == TWLSettings::EThemeSaturn ? 16 : 72, STR_RESTART_AFTER_SAVING, Alignment::center);
						updateText(false);
						if (ms().theme != TWLSettings::EThemeSaturn) {
							fadeType = true; // Fade in from white
						}
						for (int i = 0; i < 60 * 3; i++) {
							swiWaitForVBlank(); // Wait 3 seconds
						}
						if (ms().theme != TWLSettings::EThemeSaturn && ms().theme != TWLSettings::EThemeHBL) {
							fadeType = false;	   // Fade to white
						}
					}
				}

				if (dsiFeatures() && ((perGameSettings_dsiwareBooter == -1 ? ms().dsiWareBooter : perGameSettings_dsiwareBooter) || (ms().secondaryDevice && bs().b4dsMode) || sys().arm7SCFGLocked() || ms().consoleModel > 0)) {
					prepareCheats(ms().dsiWareSrlPath, (ms().secondaryDevice && ms().dsiWareToSD && sdFound()));
				}

				if (((perGameSettings_dsiwareBooter == -1 ? ms().dsiWareBooter : perGameSettings_dsiwareBooter) || (ms().secondaryDevice && bs().b4dsMode) || sys().arm7SCFGLocked() || ms().consoleModel > 0) && !ms().homebrewBootstrap) {
					// Use nds-bootstrap

					char sfnSrl[62];
					char sfnPub[62];
					char sfnPrv[62];
					if (ms().secondaryDevice && !bs().b4dsMode && ms().dsiWareToSD && sdFound()) {
						fatGetAliasPath("sd:/", "sd:/_nds/TWiLightMenu/tempDSiWare.dsi", sfnSrl);
						fatGetAliasPath("sd:/", "sd:/_nds/TWiLightMenu/tempDSiWare.pub", sfnPub);
						fatGetAliasPath("sd:/", "sd:/_nds/TWiLightMenu/tempDSiWare.prv", sfnPrv);
					} else {
						fatGetAliasPath(ms().secondaryDevice ? "fat:/" : "sd:/", ms().dsiWareSrlPath.c_str(), sfnSrl);
						fatGetAliasPath(ms().secondaryDevice ? "fat:/" : "sd:/", ms().dsiWarePubPath.c_str(), sfnPub);
						fatGetAliasPath(ms().secondaryDevice ? "fat:/" : "sd:/", ms().dsiWarePrvPath.c_str(), sfnPrv);
					}

					const char *bootstrapinipath = sdFound() ? BOOTSTRAP_INI : BOOTSTRAP_INI_FC;
					CIniFile bootstrapini(bootstrapinipath);
					bootstrapini.SetString("NDS-BOOTSTRAP", "NDS_PATH", ms().secondaryDevice && !bs().b4dsMode && ms().dsiWareToSD && sdFound() ? "sd:/_nds/TWiLightMenu/tempDSiWare.dsi" : ms().dsiWareSrlPath);
					bootstrapini.SetString("NDS-BOOTSTRAP", "APP_PATH", sfnSrl);
					bootstrapini.SetString("NDS-BOOTSTRAP", "SAV_PATH", sfnPub);
					bootstrapini.SetString("NDS-BOOTSTRAP", "PRV_PATH", sfnPrv);
					bootstrapini.SetString("NDS-BOOTSTRAP", "MANUAL_PATH", getGameManual(filename.c_str()));
					bootstrapini.SetString("NDS-BOOTSTRAP", "GUI_LANGUAGE", ms().getGuiLanguageString());
					bootstrapini.SetInt("NDS-BOOTSTRAP", "LANGUAGE", perGameSettings_language == -2 ? ms().gameLanguage : perGameSettings_language);
					bootstrapini.SetInt("NDS-BOOTSTRAP", "REGION", perGameSettings_region < -1 ? ms().gameRegion : perGameSettings_region);
					bootstrapini.SetInt("NDS-BOOTSTRAP", "USE_ROM_REGION", perGameSettings_region < -1 ? ms().useRomRegion : 0);
					bootstrapini.SetInt("NDS-BOOTSTRAP", "DSI_MODE", true);
					bootstrapini.SetInt("NDS-BOOTSTRAP", "BOOST_CPU", true);
					bootstrapini.SetInt("NDS-BOOTSTRAP", "BOOST_VRAM", true);
					if (ms().secondaryDevice && (!dsiFeatures() || bs().b4dsMode || !ms().dsiWareToSD || sys().arm7SCFGLocked())) {
						bootstrapini.SetInt("NDS-BOOTSTRAP", "CARD_READ_DMA", setCardReadDMA());
					}
					bootstrapini.SetInt("NDS-BOOTSTRAP", "DONOR_SDK_VER", 5);
					bootstrapini.SetInt("NDS-BOOTSTRAP", "GAME_SOFT_RESET", 1);
					bootstrapini.SetInt("NDS-BOOTSTRAP", "PATCH_MPU_REGION", 0);
					bootstrapini.SetInt("NDS-BOOTSTRAP", "PATCH_MPU_SIZE", 0);
					bootstrapini.SetInt("NDS-BOOTSTRAP", "FORCE_SLEEP_PATCH", 
						(ms().forceSleepPatch
					|| (memcmp(io_dldi_data->friendlyName, "TTCARD", 6) == 0 && !sys().isRegularDS())
					|| (memcmp(io_dldi_data->friendlyName, "DSTT", 4) == 0 && !sys().isRegularDS())
					|| (memcmp(io_dldi_data->friendlyName, "DEMON", 5) == 0 && !sys().isRegularDS())
					|| (memcmp(io_dldi_data->friendlyName, "R4iDSN", 6) == 0 && !sys().isRegularDS()))
					);

					bootstrapini.SaveIniFile(bootstrapinipath);

					if (ms().theme == TWLSettings::EThemeHBL) {
						fadeType = false;		  // Fade to black
					}

					while (!screenFadedOut()) {
						swiWaitForVBlank();
					}

					snd().stopStream();

					if (!dsiFeatures()) {
						snd().unloadSfxData();
						prepareCheats(ms().dsiWareSrlPath, (ms().secondaryDevice && ms().dsiWareToSD && sdFound()));
					}

					bool useNightly = (perGameSettings_bootstrapFile == -1 ? ms().bootstrapFile : perGameSettings_bootstrapFile);
					bool useWidescreen = (perGameSettings_wideScreen == -1 ? ms().wideScreen : perGameSettings_wideScreen);

					if (!isDSiMode() && (!ms().secondaryDevice || (ms().secondaryDevice && ms().dsiWareToSD && sdFound()))) {
						*(bool*)(0x02000010) = useNightly;
						*(bool*)(0x02000014) = useWidescreen;
					}
					if (dsiFeatures() || !ms().secondaryDevice) {
						if (!isDSiMode() && (!ms().secondaryDevice || (ms().secondaryDevice && ms().dsiWareToSD && sdFound()))) {
							*(u32*)0x02000000 |= BIT(4);
						}
						SetWidescreen(filename.c_str());
						*(u32*)0x02000000 &= ~BIT(4);
					}
					if (!isDSiMode() && (!ms().secondaryDevice || (ms().secondaryDevice && ms().dsiWareToSD && sdFound()))) {
						*(u32*)0x02000000 |= BIT(4);
						ntrStartSdGame();
					}

					char ndsToBoot[256];
					sprintf(ndsToBoot, "sd:/_nds/nds-bootstrap-%s.nds", useNightly ? "nightly" : "release");
					if (access(ndsToBoot, F_OK) != 0) {
						sprintf(ndsToBoot, "fat:/_nds/nds-bootstrap-%s.nds", useNightly ? "nightly" : "release");
					}

					argarray.at(0) = (char *)ndsToBoot;
					int err = runNdsFile(argarray[0], argarray.size(), (const char **)&argarray[0], sys().isRunFromSD(), true, true, false, true, true, false, -1);
					char text[64];
					snprintf(text, sizeof(text), STR_START_FAILED_ERROR.c_str(), err);
					clearText();
					fadeType = true;
					printLarge(false, 4, 4, text);
					if (err == 1) {
						printLarge(false, 4, 20, useNightly ? STR_BOOTSTRAP_NIGHTLY_NOT_FOUND : STR_BOOTSTRAP_RELEASE_NOT_FOUND);
					}
					printSmall(false, 4, 20 + calcLargeFontHeight(useNightly ? STR_BOOTSTRAP_NIGHTLY_NOT_FOUND : STR_BOOTSTRAP_RELEASE_NOT_FOUND), STR_PRESS_B_RETURN);
					updateText(false);
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
					vector<char *> argarray;
					argarray.push_back((char*)(sys().isRunFromSD() ? "sd:/_nds/TWiLightMenu/dsimenu.srldr" : "fat:/_nds/TWiLightMenu/dsimenu.srldr"));
					runNdsFile(argarray[0], argarray.size(), (const char**)&argarray[0], sys().isRunFromSD(), true, false, false, true, true, false, -1);
					stop();
				}

				if (ms().saveLocation != TWLSettings::EGamesFolder) {
					// Move .pub and/or .prv out of "saves" folder
					std::string pubnameUl = replaceAll(filename, typeToReplace, ".pub");
					std::string prvnameUl = replaceAll(filename, typeToReplace, ".prv");
					std::string pubpathUl = romFolderNoSlash + "/" + pubnameUl;
					std::string prvpathUl = romFolderNoSlash + "/" + prvnameUl;
					if (access(ms().dsiWarePubPath.c_str(), F_OK) == 0) {
						rename(ms().dsiWarePubPath.c_str(), pubpathUl.c_str());
					}
					if (access(ms().dsiWarePrvPath.c_str(), F_OK) == 0) {
						rename(ms().dsiWarePrvPath.c_str(), prvpathUl.c_str());
					}
				}

				while (!screenFadedOut()) {
					swiWaitForVBlank();
				}

				unlaunchRomBoot(ms().secondaryDevice ? "sdmc:/_nds/TWiLightMenu/tempDSiWare.dsi" : ms().dsiWareSrlPath);
			}

			// Launch .nds directly or via nds-bootstrap
			if (extension(filename, {".nds", ".dsi", ".ids", ".srl", ".app"})) {
				std::string typeToReplace = filename.substr(filename.rfind('.'));

				bool dsModeSwitch = false;
				bool dsModeDSiWare = false;

				loadPerGameSettings(filename);
				/* if (memcmp(gameTid[CURPOS], "HND", 3) == 0 || memcmp(gameTid[CURPOS], "HNE", 3) == 0) {
					dsModeSwitch = true;
					dsModeDSiWare = true;
					useBackend = false; // Bypass nds-bootstrap
					ms().homebrewBootstrap = true;
				} else */ if (isHomebrew[CURPOS]) {
					int pgsDSiMode = (perGameSettings_dsiMode == -1 ? isModernHomebrew[CURPOS] : perGameSettings_dsiMode);
					if ((perGameSettings_directBoot && ms().secondaryDevice) || (isModernHomebrew[CURPOS] && pgsDSiMode && (ms().secondaryDevice || perGameSettings_ramDiskNo == -1))) {
						useBackend = false; // Bypass nds-bootstrap
					} else {
						useBackend = true;
					}
					if (isDSiMode() && !pgsDSiMode) {
						dsModeSwitch = true;
					}
					ms().homebrewBootstrap = true;
				} else {
					useBackend = true;
					ms().homebrewBootstrap = false;
				}

				char *name = argarray.at(0);
				strcpy(filePath + pathLen, name);
				free(argarray.at(0));
				argarray.at(0) = filePath;
				if (!ms().secondaryDevice && !sys().arm7SCFGLocked() && ms().consoleModel == TWLSettings::EDSiRetail && isHomebrew[CURPOS] && !(perGameSettings_useBootstrap == -1 ? true : perGameSettings_useBootstrap)) {
					ms().romPath[ms().secondaryDevice] = std::string(argarray[0]);
					ms().launchType[ms().secondaryDevice] = Launch::ESDFlashcardLaunch;
					ms().previousUsedDevice = ms().secondaryDevice;
					ms().saveSettings();

					if (ms().theme == TWLSettings::EThemeHBL) {
						fadeType = false;		  // Fade to black
					}

					while (ms().theme != TWLSettings::EThemeSaturn && !screenFadedOut()) {
						swiWaitForVBlank();
					}

					unlaunchRomBoot(argarray[0]);
				} else if (useBackend) {
					if ((((perGameSettings_useBootstrap == -1 ? ms().useBootstrap : perGameSettings_useBootstrap) && !ms().homebrewBootstrap) || !ms().secondaryDevice) || (dsiFeatures() && unitCode[CURPOS] > 0 && (perGameSettings_dsiMode == -1 ? DEFAULT_DSI_MODE : perGameSettings_dsiMode))
					|| (ms().secondaryDevice && (io_dldi_data->ioInterface.features & FEATURE_SLOT_GBA))
					|| (unitCode[CURPOS] == 3 && !ms().homebrewBootstrap)) {
						std::string path = argarray[0];
						std::string savename = replaceAll(filename, typeToReplace, getSavExtension());
						std::string ramdiskname = replaceAll(filename, typeToReplace, getImgExtension(perGameSettings_ramDiskNo));
						std::string romFolderNoSlash = ms().romfolder[ms().secondaryDevice];
						RemoveTrailingSlashes(romFolderNoSlash);
						std::string savepath = romFolderNoSlash + "/saves/" + savename;
						std::string ramdiskpath = romFolderNoSlash + "/ramdisks/" + ramdiskname;
						if (isHomebrew[CURPOS]) {
							mkdir("ramdisks", 0777);
						} else if (ms().saveLocation == TWLSettings::ETWLMFolder) {
							std::string twlmSavesFolder = sys().isRunFromSD() ? "sd:/_nds/TWiLightMenu/saves" : "fat:/_nds/TWiLightMenu/saves";
							mkdir(twlmSavesFolder.c_str(), 0777);
							savepath = twlmSavesFolder + "/" + savename;
						} else if (ms().saveLocation == TWLSettings::EGamesFolder) {
							savepath = romFolderNoSlash + "/" + savename;
						} else {
							mkdir("saves", 0777);
						}

						createSaveFile(savepath.c_str(), isHomebrew[CURPOS], gameTid[CURPOS]);

						SetMPUSettings();

						bool boostCpu = setClockSpeed();
						bool useWidescreen = (perGameSettings_wideScreen == -1 ? ms().wideScreen : perGameSettings_wideScreen);

						const char *bootstrapinipath = (sys().isRunFromSD() ? BOOTSTRAP_INI : BOOTSTRAP_INI_FC);
						CIniFile bootstrapini(bootstrapinipath);
						bootstrapini.SetString("NDS-BOOTSTRAP", "NDS_PATH", path);
						bootstrapini.SetString("NDS-BOOTSTRAP", "SAV_PATH", savepath);
						bootstrapini.SetString("NDS-BOOTSTRAP", "HOMEBREW_ARG", (useWidescreen && (gameTid[CURPOS][0] == 'W' || romVersion[CURPOS] == 0x57)) ? "wide" : "");
						if (!isHomebrew[CURPOS]) {
							bootstrapini.SetString("NDS-BOOTSTRAP", "MANUAL_PATH", getGameManual(filename.c_str()));
						}
						bootstrapini.SetString("NDS-BOOTSTRAP", "RAM_DRIVE_PATH", (perGameSettings_ramDiskNo >= 0 && !ms().secondaryDevice) ? ramdiskpath : "sd:/null.img");
						bootstrapini.SetString("NDS-BOOTSTRAP", "GUI_LANGUAGE", ms().getGuiLanguageString());
						bootstrapini.SetInt("NDS-BOOTSTRAP", "LANGUAGE", perGameSettings_language == -2 ? ms().gameLanguage : perGameSettings_language);
						bootstrapini.SetInt("NDS-BOOTSTRAP", "REGION", perGameSettings_region < -1 ? ms().gameRegion : perGameSettings_region);
						bootstrapini.SetInt("NDS-BOOTSTRAP", "USE_ROM_REGION", perGameSettings_region < -1 ? ms().useRomRegion : 0);
						bootstrapini.SetInt("NDS-BOOTSTRAP", "DSI_MODE", (dsModeForced || (unitCode[CURPOS] == 0 && !isDSiMode())) ? 0 : (perGameSettings_dsiMode == -1 ? DEFAULT_DSI_MODE : perGameSettings_dsiMode));
						bootstrapini.SetInt("NDS-BOOTSTRAP", "CARD_READ_DMA", setCardReadDMA());
						if (dsiFeatures() || !ms().secondaryDevice) {
							bootstrapini.SetInt("NDS-BOOTSTRAP", "BOOST_CPU", boostCpu);
							bootstrapini.SetInt("NDS-BOOTSTRAP", "BOOST_VRAM", perGameSettings_boostVram == -1 ? DEFAULT_BOOST_VRAM : perGameSettings_boostVram);
							bootstrapini.SetInt("NDS-BOOTSTRAP", "ASYNC_CARD_READ", setAsyncCardRead());
						}
						bootstrapini.SetInt("NDS-BOOTSTRAP", "DONOR_SDK_VER", SetDonorSDK());
						bootstrapini.SetInt("NDS-BOOTSTRAP", "PATCH_MPU_REGION", mpuregion);
						bootstrapini.SetInt("NDS-BOOTSTRAP", "PATCH_MPU_SIZE", mpusize);
						bootstrapini.SetInt("NDS-BOOTSTRAP", "FORCE_SLEEP_PATCH", 
							(ms().forceSleepPatch
						|| (memcmp(io_dldi_data->friendlyName, "TTCARD", 6) == 0 && !sys().isRegularDS())
						|| (memcmp(io_dldi_data->friendlyName, "DSTT", 4) == 0 && !sys().isRegularDS())
						|| (memcmp(io_dldi_data->friendlyName, "DEMON", 5) == 0 && !sys().isRegularDS())
						|| (memcmp(io_dldi_data->friendlyName, "R4iDSN", 6) == 0 && !sys().isRegularDS()))
						);
						if (!isDSiMode() && ms().secondaryDevice && sdFound()) {
							CIniFile bootstrapiniSD(BOOTSTRAP_INI);
							bootstrapini.SetInt("NDS-BOOTSTRAP", "DEBUG", bootstrapiniSD.GetInt("NDS-BOOTSTRAP", "DEBUG", 0));
							bootstrapini.SetInt("NDS-BOOTSTRAP", "LOGGING", bootstrapiniSD.GetInt("NDS-BOOTSTRAP", "LOGGING", 0)); 
						}
						bootstrapini.SaveIniFile(bootstrapinipath);

						if (!isArgv) {
							ms().romPath[ms().secondaryDevice] = std::string(argarray[0]);
						}
						ms().homebrewHasWide = (isHomebrew[CURPOS] && gameTid[CURPOS][0] == 'W');
						ms().launchType[ms().secondaryDevice] = Launch::ESDFlashcardLaunch; // 1
						ms().previousUsedDevice = ms().secondaryDevice;
						ms().saveSettings();

						if (dsiFeatures()) {
							prepareCheats(path, false);
						}

						createEsrbSplash();

						if (ms().theme == TWLSettings::EThemeHBL) {
							fadeType = false;		  // Fade to black
						}

						if (sdFound() && ms().homebrewBootstrap && (access("sd:/moonshl2/logbuf.txt", F_OK) == 0)) {
							remove("sd:/moonshl2/logbuf.txt"); // Delete file for Moonshell 2 to boot properly
						}

						const bool useNightly = (perGameSettings_bootstrapFile == -1 ? ms().bootstrapFile : perGameSettings_bootstrapFile);

						char ndsToBoot[256];
						sprintf(ndsToBoot, "%s:/_nds/nds-bootstrap-%s%s.nds", sys().isRunFromSD() ? "sd" : "fat", ms().homebrewBootstrap ? "hb-" : "", useNightly ? "nightly" : "release");
						if (access(ndsToBoot, F_OK) != 0) {
							sprintf(ndsToBoot, "%s:/_nds/nds-bootstrap-%s%s.nds", sys().isRunFromSD() ? "fat" : "sd", ms().homebrewBootstrap ? "hb-" : "", useNightly ? "nightly" : "release");
						}

						if (ms().btsrpBootloaderDirect && isHomebrew[CURPOS]) {
							bootFSInit(ndsToBoot);
							bootstrapHbRunPrep(-1);
						}

						while (ms().theme != TWLSettings::EThemeSaturn && !screenFadedOut()) {
							swiWaitForVBlank();
						}

						snd().stopStream();

						if (!dsiFeatures()) {
							mmEffectCancelAll(); // Stop sound effects from playing to avoid sound glitches
							snd().unloadSfxData();
							prepareCheats(path, false);
						}

						if (dsiFeatures() || !ms().secondaryDevice) {
							SetWidescreen(filename.c_str());
						}
						if (!isDSiMode() && !ms().secondaryDevice) {
							ntrStartSdGame();
						}

						int err = 0;
						if (ms().btsrpBootloaderDirect && isHomebrew[CURPOS]) {
							if (access(ndsToBoot, F_OK) == 0) {
								if (gameTid[CURPOS][0] == 0) {
									toncset(gameTid[CURPOS], '#', 4); // Fix blank TID
								}
								char patchOffsetCacheFilePath[64];
								sprintf(patchOffsetCacheFilePath, "sd:/_nds/nds-bootstrap/patchOffsetCache/%s-%04X.bin", gameTid[CURPOS], headerCRC[CURPOS]);
								std::string fatPath = replaceAll(path, "sd:/", "fat:/");

								err = bootstrapHbRunNdsFile (path.c_str(), fatPath.c_str(),
								perGameSettings_ramDiskNo >= 0 ? ramdiskpath.c_str() : "sd:/null.img",
								"sd:/snemulds.cfg",
								perGameSettings_ramDiskNo >= 0 ? getFileSize(ramdiskpath.c_str()) : 0,
								"sd:/_nds/nds-bootstrap/softResetParams.bin",
								patchOffsetCacheFilePath,
								getFileSize("sd:/snemulds.cfg"),
								-1,
								false,
								argarray.size(),
								(const char **)&argarray[0],
								perGameSettings_language == -2 ? ms().gameLanguage : perGameSettings_language,
								perGameSettings_dsiMode == -1 ? isModernHomebrew[CURPOS] : perGameSettings_dsiMode,
								perGameSettings_boostCpu == -1 ? DEFAULT_BOOST_CPU : perGameSettings_boostCpu,
								perGameSettings_boostVram == -1 ? DEFAULT_BOOST_VRAM : perGameSettings_boostVram,
								ms().consoleModel, ms().soundFreq, false);
							} else {
								err = 1;
							}
						} else {
							argarray.at(0) = (char *)ndsToBoot;
							err = runNdsFile(argarray[0], argarray.size(), (const char **)&argarray[0], sys().isRunFromSD(), (ms().homebrewBootstrap ? false : true), true, false, true, true, false, -1);
						}
						char text[64];
						snprintf(text, sizeof(text), STR_START_FAILED_ERROR.c_str(), err);
						clearText();
						printLarge(false, 4, 4, text);
						if (err == 1) {
							if (ms().homebrewBootstrap) {
								printLarge(false, 4, 20, useNightly ? STR_BOOTSTRAP_HB_NIGHTLY_NOT_FOUND : STR_BOOTSTRAP_HB_RELEASE_NOT_FOUND);
							} else {
								printLarge(false, 4, 20, useNightly ? STR_BOOTSTRAP_NIGHTLY_NOT_FOUND : STR_BOOTSTRAP_RELEASE_NOT_FOUND);
							}
						}
						if (ms().homebrewBootstrap) {
							printSmall(false, 4, 20 + calcLargeFontHeight(useNightly ? STR_BOOTSTRAP_HB_NIGHTLY_NOT_FOUND : STR_BOOTSTRAP_HB_RELEASE_NOT_FOUND), STR_PRESS_B_RETURN);
						} else {
							printSmall(false, 4, 20 + calcLargeFontHeight(useNightly ? STR_BOOTSTRAP_NIGHTLY_NOT_FOUND : STR_BOOTSTRAP_RELEASE_NOT_FOUND), STR_PRESS_B_RETURN);
						}
						updateText(false);
						fadeSpeed = true; // Fast fading
						if (ms().theme != TWLSettings::EThemeSaturn && ms().theme != TWLSettings::EThemeHBL) {
							whiteScreen = true;
							tex().clearTopScreen();
						}
						fadeType = true;
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
						vector<char *> argarray;
						argarray.push_back((char*)(sys().isRunFromSD() ? "sd:/_nds/TWiLightMenu/dsimenu.srldr" : "fat:/_nds/TWiLightMenu/dsimenu.srldr"));
						runNdsFile(argarray[0], argarray.size(), (const char**)&argarray[0], sys().isRunFromSD(), true, false, false, true, true, false, -1);
						stop();
					} else {
						ms().romPath[ms().secondaryDevice] = std::string(argarray[0]);
						ms().launchType[ms().secondaryDevice] = Launch::ESDFlashcardLaunch;
						ms().previousUsedDevice = ms().secondaryDevice;
						ms().saveSettings();

						if (ms().theme == TWLSettings::EThemeHBL) {
							fadeType = false;		  // Fade to black
						}

						while (ms().theme != TWLSettings::EThemeSaturn && !screenFadedOut()) {
							swiWaitForVBlank();
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
					if (isDSiMode() || !ms().secondaryDevice) {
						SetWidescreen(filename.c_str());
					}
					ms().saveSettings();

					if (ms().theme == TWLSettings::EThemeHBL) {
						fadeType = false;		  // Fade to black
					}

					while (ms().theme != TWLSettings::EThemeSaturn && !screenFadedOut()) {
						swiWaitForVBlank();
					}

					if (!isDSiMode() && !ms().secondaryDevice && strncmp(filename.c_str(), "GodMode9i", 9) != 0 && strcmp(gameTid[CURPOS], "HGMA") != 0) {
						ntrStartSdGame();
					}

					int language = perGameSettings_language == -2 ? ms().gameLanguage : perGameSettings_language;
					int gameRegion = perGameSettings_region < -1 ? ms().gameRegion : perGameSettings_region;

					// Set region flag
					if (ms().useRomRegion && perGameSettings_region < -1 && gameTid[CURPOS][3] != 'A' && gameTid[CURPOS][3] != 'O' && gameTid[CURPOS][3] != '#') {
						if (gameTid[CURPOS][3] == 'J') {
							*(u8*)(0x02FFFD70) = 0;
						} else if (gameTid[CURPOS][3] == 'E' || gameTid[CURPOS][3] == 'T') {
							*(u8*)(0x02FFFD70) = 1;
						} else if (gameTid[CURPOS][3] == 'P' || gameTid[CURPOS][3] == 'V') {
							*(u8*)(0x02FFFD70) = 2;
						} else if (gameTid[CURPOS][3] == 'U') {
							*(u8*)(0x02FFFD70) = 3;
						} else if (gameTid[CURPOS][3] == 'C') {
							*(u8*)(0x02FFFD70) = 4;
						} else if (gameTid[CURPOS][3] == 'K') {
							*(u8*)(0x02FFFD70) = 5;
						}
					} else if (gameRegion == -1) {
					  if (useTwlCfg) {
						u8 country = *(u8*)0x02000405;
						if (country == 0x01) {
							*(u8*)(0x02FFFD70) = 0;	// Japan
						} else if (country == 0xA0) {
							*(u8*)(0x02FFFD70) = 4;	// China
						} else if (country == 0x88) {
							*(u8*)(0x02FFFD70) = 5;	// Korea
						} else if (country == 0x41 || country == 0x5F) {
							*(u8*)(0x02FFFD70) = 3;	// Australia
						} else if ((country >= 0x08 && country <= 0x34) || country == 0x99 || country == 0xA8) {
							*(u8*)(0x02FFFD70) = 1;	// USA
						} else if (country >= 0x40 && country <= 0x70) {
							*(u8*)(0x02FFFD70) = 2;	// Europe
						}
					  } else {
						u8 consoleType = 0;
						readFirmware(0x1D, &consoleType, 1);
						if (consoleType == 0x43 || consoleType == 0x63) {
							*(u8*)(0x02FFFD70) = 4;	// China
						} else if (PersonalData->language == 0) {
							*(u8*)(0x02FFFD70) = 0;	// Japan
						} else if (PersonalData->language == 1 /*|| PersonalData->language == 2 || PersonalData->language == 5*/) {
							*(u8*)(0x02FFFD70) = 1;	// USA
						} else /*if (PersonalData->language == 3 || PersonalData->language == 4)*/ {
							*(u8*)(0x02FFFD70) = 2;	// Europe
						} /*else {
							*(u8*)(0x02FFFD70) = 5;	// Korea
						}*/
					  }
					} else {
						*(u8*)(0x02FFFD70) = gameRegion;
					}

					if (useTwlCfg && language >= 0 && language <= 7 && *(u8*)0x02000406 != language) {
						tonccpy((char*)0x02000600, (char*)0x02000400, 0x200);
						*(u8*)0x02000606 = language;
						*(u32*)0x02FFFDFC = 0x02000600;
					}

					bool useWidescreen = (perGameSettings_wideScreen == -1 ? ms().wideScreen : perGameSettings_wideScreen);

					if (ms().consoleModel >= 2 && useWidescreen && ms().homebrewHasWide) {
						//argarray.push_back((char*)"wide");
						SetWidescreen(NULL);
					}

					bool runNds_boostCpu = false;
					bool runNds_boostVram = false;
					if (dsiFeatures() && !dsModeDSiWare) {
						runNds_boostCpu = perGameSettings_boostCpu == -1 ? DEFAULT_BOOST_CPU : perGameSettings_boostCpu;
						runNds_boostVram = perGameSettings_boostVram == -1 ? DEFAULT_BOOST_VRAM : perGameSettings_boostVram;
					}
					int err = runNdsFile(argarray[0], argarray.size(), (const char **)&argarray[0], sys().isRunFromSD(), true, true, dsModeSwitch, runNds_boostCpu, runNds_boostVram, false, language);
					char text[64];
					snprintf(text, sizeof(text), STR_START_FAILED_ERROR.c_str(), err);
					printLarge(false, 4, 4, text);
					printSmall(false, 4, 20, STR_PRESS_B_RETURN);
					updateText(false);
					fadeSpeed = true;
					if (ms().theme != TWLSettings::EThemeSaturn && ms().theme != TWLSettings::EThemeHBL) {
						whiteScreen = true;
						tex().clearTopScreen();
					}
					fadeType = true;
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
					vector<char *> argarray;
					argarray.push_back((char*)(sys().isRunFromSD() ? "sd:/_nds/TWiLightMenu/dsimenu.srldr" : "fat:/_nds/TWiLightMenu/dsimenu.srldr"));
					runNdsFile(argarray[0], argarray.size(), (const char**)&argarray[0], sys().isRunFromSD(), true, false, false, true, true, false, -1);
					stop();
				}
			} else {
				bool useNDSB = false;
				bool tgdsMode = false;
				bool dsModeSwitch = false;
				bool boostCpu = true;
				bool boostVram = false;
				bool tscTgds = false;
				int romToRamDisk = -1;

				std::string romfolderNoSlash = ms().romfolder[ms().secondaryDevice];
				RemoveTrailingSlashes(romfolderNoSlash);
				char ROMpath[256];
				snprintf (ROMpath, sizeof(ROMpath), "%s/%s", romfolderNoSlash.c_str(), filename.c_str());
				std::string ROMpathFAT;
				if (!ms().secondaryDevice) {
					ROMpathFAT = replaceAll(ROMpath, "sd:/", "fat:/");
				}
				ms().romPath[ms().secondaryDevice] = std::string(ROMpath);
				ms().previousUsedDevice = ms().secondaryDevice;
				ms().homebrewBootstrap = true;

				const char *ndsToBoot = "";
				std::string ndsToBootFat;
				const char *tgdsNdsPath = "sd:/_nds/TWiLightMenu/apps/ToolchainGenericDS-multiboot.srl";
				if (extension(filename, {".plg"})) {
					ndsToBoot = "fat:/_nds/TWiLightMenu/bootplg.srldr";
					dsModeSwitch = true;

					// Print .plg path without "fat:" at the beginning
					char ROMpathDS2[256];
					if (ms().secondaryDevice) {
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
				} else if (extension(filename, {".avi"})) {
					ms().launchType[ms().secondaryDevice] = Launch::ETunaViDSLaunch;

					ndsToBoot = "sd:/_nds/TWiLightMenu/apps/tuna-vids.nds";
					if (!isDSiMode() || access(ndsToBoot, F_OK) != 0) {
						ndsToBoot = "fat:/_nds/TWiLightMenu/apps/tuna-vids.nds";
						boostVram = true;
					}
				} else if (extension(filename, {".rvid"})) {
					ms().launchType[ms().secondaryDevice] = Launch::ERVideoLaunch;

					ndsToBoot = "sd:/_nds/TWiLightMenu/apps/RocketVideoPlayer.nds";
					if (!isDSiMode() || access(ndsToBoot, F_OK) != 0) {
						ndsToBoot = "fat:/_nds/TWiLightMenu/apps/RocketVideoPlayer.nds";
						boostVram = true;
					}
				} else if (extension(filename, {".fv", ".ntrb"})) {
					ms().launchType[ms().secondaryDevice] = Launch::EFastVideoLaunch;

					ndsToBoot = (flashcardFound() && io_dldi_data->driverSize >= 0xF) ? "sd:/_nds/TWiLightMenu/apps/FastVideoDS32.nds" : "sd:/_nds/TWiLightMenu/apps/FastVideoDS.nds";
					if (!isDSiMode() || access(ndsToBoot, F_OK) != 0) {
						ndsToBoot = (io_dldi_data->driverSize >= 0xF) ? "fat:/_nds/TWiLightMenu/apps/FastVideoDS32.nds" : "fat:/_nds/TWiLightMenu/apps/FastVideoDS.nds";
						boostVram = true;
					}
				} else if (extension(filename, {".agb", ".gba", ".mb"})) {
					ms().launchType[ms().secondaryDevice] = (ms().gbaBooter == TWLSettings::EGbaNativeGbar2 && *(u16*)(0x020000C0) != 0) ? Launch::EGBANativeLaunch : Launch::ESDFlashcardLaunch;

					if (ms().launchType[ms().secondaryDevice] == Launch::EGBANativeLaunch) {
						if (ms().theme == TWLSettings::EThemeHBL) {
							displayGameIcons = false;
						} else {
							while (!screenFadedOut()) {
								swiWaitForVBlank();
							}
							whiteScreen = true;
							tex().clearTopScreen();
						}
						if (*(u16*)(0x020000C0) == 0x5A45) {
							printLarge(false, 0, (ms().theme == TWLSettings::EThemeSaturn ? 80 : 88), STR_PLEASE_WAIT, Alignment::center);
							updateText(false);
						}

						if (ms().theme != TWLSettings::EThemeSaturn && ms().theme != TWLSettings::EThemeHBL) {
							fadeSpeed = true; // Fast fading
							fadeType = true; // Fade in from white
						}
						showProgressIcon = true;
						showProgressBar = true;
						progressBarLength = 0;

						u32 ptr = 0x08000000;
						u32 romSize = getFileSize(filename.c_str());
						FILE* gbaFile = fopen(filename.c_str(), "rb");
						if (strncmp(gameTid[CURPOS], "AGBJ", 4) == 0 && romSize <= 0x40000) {
							ptr += 0x400;
						}
						u32 curPtr = ptr;

						extern char copyBuf[0x8000];
						if (romSize > 0x2000000) romSize = 0x2000000;

						bool nor = false;
						if (*(u16*)(0x020000C0) == 0x5A45 && strncmp(gameTid[CURPOS], "AGBJ", 4) != 0) {
							cExpansion::SetRompage(0);
							expansion().SetRampage(cExpansion::ENorPage);
							cExpansion::OpenNorWrite();
							cExpansion::SetSerialMode();
							for (u32 address=0;address<romSize&&address<0x2000000;address+=0x40000) {
								expansion().Block_Erase(address);
								progressBarLength = (address+0x40000)/(romSize/192);
							}
							nor = true;
						} else if (*(u16*)(0x020000C0) == 0x4353 && romSize > 0x1FFFFFE) {
							romSize = 0x1FFFFFE;
						}

						clearText();
						printLarge(false, 0, (ms().theme == TWLSettings::EThemeSaturn ? 80 : 88), STR_NOW_LOADING, Alignment::center);
						updateText(false);

						for (u32 len = romSize; len > 0; len -= 0x8000) {
							if (fread(&copyBuf, 1, (len>0x8000 ? 0x8000 : len), gbaFile) > 0) {
								s2RamAccess(true);
								if (nor) {
									expansion().WriteNorFlash(curPtr-ptr, (u8*)copyBuf, (len>0x8000 ? 0x8000 : len));
								} else {
									tonccpy((u16*)curPtr, &copyBuf, (len>0x8000 ? 0x8000 : len));
								}
								s2RamAccess(false);
								curPtr += 0x8000;
								progressBarLength = ((curPtr-ptr)+0x8000)/(romSize/192);
							} else {
								break;
							}
						}
						fclose(gbaFile);

						ptr = 0x0A000000;

						std::string savename = replaceAll(filename, ".gba", ".sav");
						u32 savesize = getFileSize(savename.c_str());
						if (savesize > 0x10000) savesize = 0x10000;

						if (savesize > 0) {
							FILE* savFile = fopen(savename.c_str(), "rb");
							for (u32 len = savesize; len > 0; len -= 0x8000) {
								if (fread(&copyBuf, 1, (len>0x8000 ? 0x8000 : len), savFile) > 0) {
									gbaSramAccess(true);	// Switch to GBA SRAM
									cExpansion::WriteSram(ptr,(u8*)copyBuf,0x8000);
									gbaSramAccess(false);	// Switch out of GBA SRAM
									ptr += 0x8000;
								} else {
									break;
								}
							}
							fclose(savFile);
						}

						showProgressIcon = false;
						if (ms().theme != TWLSettings::EThemeSaturn && ms().theme != TWLSettings::EThemeHBL) {
							fadeType = false;	   // Fade to white
						}
						if (ms().theme == TWLSettings::EThemeHBL) displayGameIcons = true;

						ndsToBoot = "fat:/_nds/TWiLightMenu/gbapatcher.srldr";
					} else if (ms().gbaR3Test) {
						ms().launchType[ms().secondaryDevice] = Launch::EGBARunner2Launch;

						ndsToBoot = sys().isRunFromSD() ? "sd:/_nds/TWiLightMenu/emulators/GBARunner3.nds" : "fat:/_nds/TWiLightMenu/emulators/GBARunner3.nds";
						if (!isDSiMode()) {
							boostVram = true;
						}
					} else if (ms().secondaryDevice) {
						ms().launchType[ms().secondaryDevice] = Launch::EGBARunner2Launch;

						if (dsiFeatures()) {
							ndsToBoot = ms().consoleModel > 0 ? "sd:/_nds/GBARunner2_arm7dldi_3ds.nds" : "sd:/_nds/GBARunner2_arm7dldi_dsi.nds";
						} else if (memcmp(gameTid[CURPOS], "BPE", 3) == 0) { // If game is Pokemon Emerald...
							ndsToBoot = ms().gbar2DldiAccess ? "sd:/_nds/GBARunner2_arm7dldi_rom3m_ds.nds" : "sd:/_nds/GBARunner2_arm9dldi_rom3m_ds.nds";
						} else {
							ndsToBoot = ms().gbar2DldiAccess ? "sd:/_nds/GBARunner2_arm7dldi_ds.nds" : "sd:/_nds/GBARunner2_arm9dldi_ds.nds";
						}
						if (!isDSiMode() || access(ndsToBoot, F_OK) != 0) {
							if (dsiFeatures()) {
								ndsToBoot = ms().consoleModel > 0 ? "fat:/_nds/GBARunner2_arm7dldi_3ds.nds" : "fat:/_nds/GBARunner2_arm7dldi_dsi.nds";
							} else if (memcmp(gameTid[CURPOS], "BPE", 3) == 0) { // If game is Pokemon Emerald...
								ndsToBoot = ms().gbar2DldiAccess ? "fat:/_nds/GBARunner2_arm7dldi_rom3m_ds.nds" : "fat:/_nds/GBARunner2_arm9dldi_rom3m_ds.nds";
							} else {
								ndsToBoot = ms().gbar2DldiAccess ? "fat:/_nds/GBARunner2_arm7dldi_ds.nds" : "fat:/_nds/GBARunner2_arm9dldi_ds.nds";
							}
						}
						boostVram = false;
					} else {
						useNDSB = true;

						ndsToBoot = ms().consoleModel>0 ? "sd:/_nds/GBARunner2_arm7dldi_3ds.nds" : "sd:/_nds/GBARunner2_arm7dldi_dsi.nds";
						if (isDSiMode() && sys().arm7SCFGLocked() && !sys().dsiWramAccess()) {
							ndsToBoot = ms().consoleModel > 0 ? "sd:/_nds/GBARunner2_arm7dldi_nodsp_3ds.nds" : "sd:/_nds/GBARunner2_arm7dldi_nodsp_dsi.nds";
						}

						ndsToBootFat = replaceAll(ndsToBoot, "sd:/", "fat:/");
						CIniFile bootstrapini(BOOTSTRAP_INI);

						bootstrapini.SetString("NDS-BOOTSTRAP", "GUI_LANGUAGE", ms().getGuiLanguageString());
						bootstrapini.SetInt("NDS-BOOTSTRAP", "LANGUAGE", ms().gameLanguage);
						bootstrapini.SetInt("NDS-BOOTSTRAP", "DSI_MODE", 0);
						bootstrapini.SetString("NDS-BOOTSTRAP", "NDS_PATH", ndsToBoot);
						bootstrapini.SetString("NDS-BOOTSTRAP", "HOMEBREW_ARG", ROMpath);
						bootstrapini.SetString("NDS-BOOTSTRAP", "RAM_DRIVE_PATH", "");
						bootstrapini.SetInt("NDS-BOOTSTRAP", "BOOST_CPU", 1);
						bootstrapini.SetInt("NDS-BOOTSTRAP", "BOOST_VRAM", 0);

						bootstrapini.SaveIniFile(BOOTSTRAP_INI);
					}
				} else if (extension(filename, {".xex", ".atr"})) {
					ms().launchType[ms().secondaryDevice] = Launch::EXEGSDSLaunch;

					ndsToBoot = "sd:/_nds/TWiLightMenu/emulators/A8DS.nds";
					if (!isDSiMode() || access(ndsToBoot, F_OK) != 0) {
						ndsToBoot = "fat:/_nds/TWiLightMenu/emulators/A8DS.nds";
						boostVram = true;
					}
				} else if (extension(filename, {".a26"})) {
					ms().launchType[ms().secondaryDevice] = Launch::EStellaDSLaunch;

					ndsToBoot = "sd:/_nds/TWiLightMenu/emulators/StellaDS.nds";
					if (!isDSiMode() || access(ndsToBoot, F_OK) != 0) {
						ndsToBoot = "fat:/_nds/TWiLightMenu/emulators/StellaDS.nds";
						boostVram = true;
					}
				} else if (extension(filename, {".a52"})) {
					ms().launchType[ms().secondaryDevice] = Launch::EA5200DSLaunch;

					ndsToBoot = "sd:/_nds/TWiLightMenu/emulators/A5200DS.nds";
					if (!isDSiMode() || access(ndsToBoot, F_OK) != 0) {
						ndsToBoot = "fat:/_nds/TWiLightMenu/emulators/A5200DS.nds";
						boostVram = true;
					}
				} else if (extension(filename, {".a78"})) {
					ms().launchType[ms().secondaryDevice] = Launch::EA7800DSLaunch;

					ndsToBoot = "sd:/_nds/TWiLightMenu/emulators/A7800DS.nds";
					if (!isDSiMode() || access(ndsToBoot, F_OK) != 0) {
						ndsToBoot = "fat:/_nds/TWiLightMenu/emulators/A7800DS.nds";
						boostVram = true;
					}
				} else if ((extension(filename, {".sg", ".sc"}) && ms().sgEmulator == TWLSettings::EColSegaColecoDS) || (extension(filename, {".col"}) && ms().colEmulator == TWLSettings::EColSegaColecoDS) || extension(filename, {".m5", ".msx"})) {
					ms().launchType[ms().secondaryDevice] = Launch::EColecoDSLaunch;

					ndsToBoot = "sd:/_nds/TWiLightMenu/emulators/ColecoDS.nds";
					if (!isDSiMode() || access(ndsToBoot, F_OK) != 0) {
						ndsToBoot = "fat:/_nds/TWiLightMenu/emulators/ColecoDS.nds";
						boostVram = true;
					}
				} else if (extension(filename, {".int"})) {
					ms().launchType[ms().secondaryDevice] = Launch::ENINTVDSLaunch;

					ndsToBoot = "sd:/_nds/TWiLightMenu/emulators/NINTV-DS.nds";
					if (!isDSiMode() || access(ndsToBoot, F_OK) != 0) {
						ndsToBoot = "fat:/_nds/TWiLightMenu/emulators/NINTV-DS.nds";
						boostVram = true;
					}
				} else if (extension(filename, {".gb", ".sgb", ".gbc"})) {
					ms().launchType[ms().secondaryDevice] = Launch::EGameYobLaunch;

					ndsToBoot = "sd:/_nds/TWiLightMenu/emulators/gameyob.nds";
					if (!isDSiMode() || access(ndsToBoot, F_OK) != 0) {
						ndsToBoot = "fat:/_nds/TWiLightMenu/emulators/gameyob.nds";
						dsModeSwitch = !isDSiMode();
						boostVram = true;
					}
				} else if (extension(filename, {".nes", ".fds"})) {
					ms().launchType[ms().secondaryDevice] = Launch::ENESDSLaunch;

					ndsToBoot = "sd:/_nds/TWiLightMenu/emulators/nesds.nds";
					if (!isDSiMode() || access(ndsToBoot, F_OK) != 0) {
						ndsToBoot = "fat:/_nds/TWiLightMenu/emulators/nesds.nds";
						boostVram = true;
					}
				} else if ((extension(filename, {".sg", ".sc"}) && ms().sgEmulator == TWLSettings::EColSegaS8DS) || (extension(filename, {".sms", ".gg"})) || (extension(filename, {".col"}) && ms().colEmulator == TWLSettings::EColSegaS8DS)) {
					mkdir(ms().secondaryDevice ? "fat:/data" : "sd:/data", 0777);
					mkdir(ms().secondaryDevice ? "fat:/data/s8ds" : "sd:/data/s8ds", 0777);

					ms().launchType[ms().secondaryDevice] = Launch::ES8DSLaunch;

					ndsToBoot = "sd:/_nds/TWiLightMenu/emulators/S8DS.nds";
					if (!isDSiMode() || access(ndsToBoot, F_OK) != 0) {
						ndsToBoot = "fat:/_nds/TWiLightMenu/emulators/S8DS.nds";
						boostVram = true;
					}
				} else if (extension(filename, {".gen", ".md"})) {
					bool usePicoDrive = ((isDSiMode() && sdFound() && sys().arm7SCFGLocked())
						|| ms().mdEmulator==2 || (ms().mdEmulator==3 && getFileSize(filename.c_str()) > 0x300000));
					ms().launchType[ms().secondaryDevice] = (usePicoDrive ? Launch::EPicoDriveTWLLaunch : Launch::ESDFlashcardLaunch);

					if (usePicoDrive || ms().secondaryDevice) {
						ndsToBoot = usePicoDrive ? "sd:/_nds/TWiLightMenu/emulators/PicoDriveTWL.nds" : (ms().macroMode ? "sd:/_nds/TWiLightMenu/emulators/jEnesisDS_macro.nds" : "sd:/_nds/TWiLightMenu/emulators/jEnesisDS.nds");
						if (!isDSiMode() || access(ndsToBoot, F_OK) != 0) {
							ndsToBoot = usePicoDrive ? "fat:/_nds/TWiLightMenu/emulators/PicoDriveTWL.nds" : (ms().macroMode ? "fat:/_nds/TWiLightMenu/emulators/jEnesisDS_macro.nds" : "fat:/_nds/TWiLightMenu/emulators/jEnesisDS.nds");
							boostVram = true;
						}
						dsModeSwitch = !usePicoDrive;
					} else {
						useNDSB = true;
						romToRamDisk = 0;

						ndsToBoot = ms().macroMode ? "sd:/_nds/TWiLightMenu/emulators/jEnesisDS_macro.nds" : "sd:/_nds/TWiLightMenu/emulators/jEnesisDS.nds";
						ndsToBootFat = replaceAll(ndsToBoot, "sd:/", "fat:/");
						CIniFile bootstrapini(BOOTSTRAP_INI);

						bootstrapini.SetString("NDS-BOOTSTRAP", "GUI_LANGUAGE", ms().getGuiLanguageString());
						bootstrapini.SetInt("NDS-BOOTSTRAP", "LANGUAGE", ms().gameLanguage);
						bootstrapini.SetInt("NDS-BOOTSTRAP", "DSI_MODE", 0);
						bootstrapini.SetString("NDS-BOOTSTRAP", "NDS_PATH", ndsToBoot);
						bootstrapini.SetString("NDS-BOOTSTRAP", "HOMEBREW_ARG", "fat:/ROM.BIN");
						bootstrapini.SetInt("NDS-BOOTSTRAP", "BOOST_CPU", 1);
						bootstrapini.SetInt("NDS-BOOTSTRAP", "BOOST_VRAM", 0);

						bootstrapini.SetString("NDS-BOOTSTRAP", "RAM_DRIVE_PATH", ROMpath);
						bootstrapini.SaveIniFile(BOOTSTRAP_INI);
					}
				} else if (extension(filename, {".smc", ".sfc"})) {
					ms().launchType[ms().secondaryDevice] = ((ms().newSnesEmuVer || ms().secondaryDevice) ? Launch::ESNEmulDSLaunch : Launch::ESDFlashcardLaunch);
					if (ms().newSnesEmuVer) {
						tgdsMode = true;
						tscTgds = true;

						ndsToBoot = "sd:/_nds/TWiLightMenu/emulators/SNEmulDS.srl";
						tgdsNdsPath = "fat:/SNEmulDS.srl";
						if (!isDSiMode() || access(ndsToBoot, F_OK) != 0) {
							ndsToBoot = "fat:/_nds/TWiLightMenu/emulators/SNEmulDS.nds";
							if (ms().secondaryDevice) {
								boostVram = true;
								dsModeSwitch = true;
							}
						}
					} else if (ms().secondaryDevice) {
						const bool twlmPath = (romfolderNoSlash == "fat:/roms/snes");
						ndsToBoot = twlmPath ? "sd:/_nds/TWiLightMenu/emulators/SNEmulDS-legacy-twlm-path.nds" : "sd:/_nds/TWiLightMenu/emulators/SNEmulDS-legacy.nds";
						if (!isDSiMode() || access(ndsToBoot, F_OK) != 0) {
							ndsToBoot = twlmPath ? "fat:/_nds/TWiLightMenu/emulators/SNEmulDS-legacy-twlm-path.nds" : "fat:/_nds/TWiLightMenu/emulators/SNEmulDS-legacy.nds";
						}
						boostCpu = false;
						dsModeSwitch = true;
					} else {
						useNDSB = true;
						boostCpu = false;
						romToRamDisk = 1;

						ndsToBoot = "sd:/_nds/TWiLightMenu/emulators/SNEmulDS-legacy.nds";
						ndsToBootFat = replaceAll(ndsToBoot, "sd:/", "fat:/");
						CIniFile bootstrapini(BOOTSTRAP_INI);

						bootstrapini.SetString("NDS-BOOTSTRAP", "GUI_LANGUAGE", ms().getGuiLanguageString());
						bootstrapini.SetInt("NDS-BOOTSTRAP", "LANGUAGE", ms().gameLanguage);
						bootstrapini.SetInt("NDS-BOOTSTRAP", "DSI_MODE", 0);
						bootstrapini.SetString("NDS-BOOTSTRAP", "NDS_PATH", ndsToBoot);
						bootstrapini.SetString("NDS-BOOTSTRAP", "HOMEBREW_ARG", "fat:/ROM.SMC");
						bootstrapini.SetInt("NDS-BOOTSTRAP", "BOOST_CPU", 0);
						bootstrapini.SetInt("NDS-BOOTSTRAP", "BOOST_VRAM", 0);

						bootstrapini.SetString("NDS-BOOTSTRAP", "RAM_DRIVE_PATH", ROMpath);
						bootstrapini.SaveIniFile(BOOTSTRAP_INI);
					}
				} else if (extension(filename, {".pce"})) {
					mkdir(ms().secondaryDevice ? "fat:/data" : "sd:/data", 0777);
					mkdir(ms().secondaryDevice ? "fat:/data/NitroGrafx" : "sd:/data/NitroGrafx", 0777);

					if (!ms().secondaryDevice && !sys().arm7SCFGLocked() && ms().smsGgInRam) {
						ms().launchType[ms().secondaryDevice] = Launch::ESDFlashcardLaunch;

						useNDSB = true;
						romToRamDisk = 4;

						ndsToBoot = "sd:/_nds/TWiLightMenu/emulators/NitroGrafx.nds";
						ndsToBootFat = replaceAll(ndsToBoot, "sd:/", "fat:/");
						CIniFile bootstrapini(BOOTSTRAP_INI);

						bootstrapini.SetString("NDS-BOOTSTRAP", "GUI_LANGUAGE", ms().getGuiLanguageString());
						bootstrapini.SetInt("NDS-BOOTSTRAP", "LANGUAGE", ms().gameLanguage);
						bootstrapini.SetInt("NDS-BOOTSTRAP", "DSI_MODE", 0);
						bootstrapini.SetString("NDS-BOOTSTRAP", "NDS_PATH", ndsToBoot);
						bootstrapini.SetString("NDS-BOOTSTRAP", "HOMEBREW_ARG", ROMpath);
						bootstrapini.SetInt("NDS-BOOTSTRAP", "BOOST_CPU", 1);
						bootstrapini.SetInt("NDS-BOOTSTRAP", "BOOST_VRAM", 0);

						bootstrapini.SetString("NDS-BOOTSTRAP", "RAM_DRIVE_PATH", "");
						bootstrapini.SaveIniFile(BOOTSTRAP_INI);
					} else {
						ms().launchType[ms().secondaryDevice] = Launch::ENitroGrafxLaunch;

						ndsToBoot = "sd:/_nds/TWiLightMenu/emulators/NitroGrafx.nds";
						if (!isDSiMode() || access(ndsToBoot, F_OK) != 0) {
							ndsToBoot = "fat:/_nds/TWiLightMenu/emulators/NitroGrafx.nds";
							boostVram = true;
						}
					}
				} else if (extension(filename, {".ws", ".wsc"})) {
					mkdir(ms().secondaryDevice ? "fat:/data" : "sd:/data", 0777);
					mkdir(ms().secondaryDevice ? "fat:/data/nitroswan" : "sd:/data/nitroswan", 0777);

					ms().launchType[ms().secondaryDevice] = Launch::ENitroSwanLaunch;

					ndsToBoot = "sd:/_nds/TWiLightMenu/emulators/NitroSwan.nds";
					if (!isDSiMode() || access(ndsToBoot, F_OK) != 0) {
						ndsToBoot = "fat:/_nds/TWiLightMenu/emulators/NitroSwan.nds";
						boostVram = true;
					}
				} else if (extension(filename, {".ngp", ".ngc"})) {
					mkdir(ms().secondaryDevice ? "fat:/data" : "sd:/data", 0777);
					mkdir(ms().secondaryDevice ? "fat:/data/ngpds" : "sd:/data/ngpds", 0777);

					ms().launchType[ms().secondaryDevice] = Launch::ENGPDSLaunch;

					ndsToBoot = "sd:/_nds/TWiLightMenu/emulators/NGPDS.nds";
					if (!isDSiMode() || access(ndsToBoot, F_OK) != 0) {
						ndsToBoot = "fat:/_nds/TWiLightMenu/emulators/NGPDS.nds";
						boostVram = true;
					}
				} else if (extension(filename, {".dsk"}) && ms().cpcEmulator == TWLSettings::ECpcAmEDS) {
					ms().launchType[ms().secondaryDevice] = (ms().secondaryDevice ? Launch::EAmEDSLaunch : Launch::ESDFlashcardLaunch);

					if (ms().secondaryDevice) {
						ndsToBoot = "sd:/_nds/TWiLightMenu/emulators/AmEDS.nds";
						if (!isDSiMode() || access(ndsToBoot, F_OK) != 0) {
							ndsToBoot = "fat:/_nds/TWiLightMenu/emulators/AmEDS.nds";
							boostVram = true;
						}
					} else {
						useNDSB = true;

						ndsToBoot = "sd:/_nds/TWiLightMenu/emulators/AmEDS.nds";
						ndsToBootFat = replaceAll(ndsToBoot, "sd:/", "fat:/");
						CIniFile bootstrapini(BOOTSTRAP_INI);

						bootstrapini.SetString("NDS-BOOTSTRAP", "GUI_LANGUAGE", ms().getGuiLanguageString());
						bootstrapini.SetInt("NDS-BOOTSTRAP", "LANGUAGE", ms().gameLanguage);
						bootstrapini.SetInt("NDS-BOOTSTRAP", "DSI_MODE", 0);
						bootstrapini.SetString("NDS-BOOTSTRAP", "NDS_PATH", ndsToBoot);
						bootstrapini.SetString("NDS-BOOTSTRAP", "HOMEBREW_ARG", ROMpath);
						bootstrapini.SetInt("NDS-BOOTSTRAP", "BOOST_CPU", 1);
						bootstrapini.SetInt("NDS-BOOTSTRAP", "BOOST_VRAM", 0);

						bootstrapini.SetString("NDS-BOOTSTRAP", "RAM_DRIVE_PATH", "");
						bootstrapini.SaveIniFile(BOOTSTRAP_INI);
					}
				} else if (extension(filename, {".dsk"}) && ms().cpcEmulator == TWLSettings::ECpcCrocoDS) {
					ms().launchType[ms().secondaryDevice] = (ms().secondaryDevice ? Launch::ECrocoDSLaunch : Launch::ESDFlashcardLaunch);

					if (ms().secondaryDevice) {
						ndsToBoot = "sd:/_nds/TWiLightMenu/emulators/CrocoDS.nds";
						if (!isDSiMode() || access(ndsToBoot, F_OK) != 0) {
							ndsToBoot = "fat:/_nds/TWiLightMenu/emulators/CrocoDS.nds";
							boostVram = true;
						}
					} else {
						useNDSB = true;

						ndsToBoot = "sd:/_nds/TWiLightMenu/emulators/CrocoDS.nds";
						ndsToBootFat = replaceAll(ndsToBoot, "sd:/", "fat:/");
						CIniFile bootstrapini(BOOTSTRAP_INI);

						bootstrapini.SetString("NDS-BOOTSTRAP", "GUI_LANGUAGE", ms().getGuiLanguageString());
						bootstrapini.SetInt("NDS-BOOTSTRAP", "LANGUAGE", ms().gameLanguage);
						bootstrapini.SetInt("NDS-BOOTSTRAP", "DSI_MODE", 0);
						bootstrapini.SetString("NDS-BOOTSTRAP", "NDS_PATH", ndsToBoot);
						bootstrapini.SetString("NDS-BOOTSTRAP", "HOMEBREW_ARG", ROMpath);
						bootstrapini.SetInt("NDS-BOOTSTRAP", "BOOST_CPU", 1);
						bootstrapini.SetInt("NDS-BOOTSTRAP", "BOOST_VRAM", 0);

						bootstrapini.SetString("NDS-BOOTSTRAP", "RAM_DRIVE_PATH", "");
						bootstrapini.SaveIniFile(BOOTSTRAP_INI);
					}
				} else if (extension(filename, {".min"})) {
					ms().launchType[ms().secondaryDevice] = Launch::EPokeMiniLaunch;

					ndsToBoot = "sd:/_nds/TWiLightMenu/emulators/PokeMini.nds";
					if (!isDSiMode() || access(ndsToBoot, F_OK) != 0) {
						ndsToBoot = "fat:/_nds/TWiLightMenu/emulators/PokeMini.nds";
						boostVram = true;
					}
				} else if (extension(filename, {".3ds", ".cia", ".cxi"})) {
					ms().launchType[ms().secondaryDevice] = Launch::E3DSLaunch;

					ndsToBoot = sys().isRunFromSD() ? "sd:/_nds/TWiLightMenu/3dssplash.srldr" : "fat:/_nds/TWiLightMenu/3dssplash.srldr";
					if (!isDSiMode()) {
						boostVram = true;
					}
				} else if (extension(filename, {".gif", ".bmp", ".png"})) {
					ms().launchType[ms().secondaryDevice] = Launch::EImageLaunch;

					ndsToBoot = sys().isRunFromSD() ? "sd:/_nds/TWiLightMenu/imageview.srldr" : "fat:/_nds/TWiLightMenu/imageview.srldr";
					if (!isDSiMode()) {
						boostVram = true;
					}
				}

				ms().homebrewArg[ms().secondaryDevice] = useNDSB ? "" : ms().romPath[ms().secondaryDevice];
				ms().saveSettings();

				if (ms().theme == TWLSettings::EThemeHBL) {
					fadeType = false;		  // Fade to black
				}

				if (ms().btsrpBootloaderDirect && useNDSB) {
					bootFSInit(ms().bootstrapFile ? "sd:/_nds/nds-bootstrap-hb-nightly.nds" : "sd:/_nds/nds-bootstrap-hb-release.nds");
					bootstrapHbRunPrep(romToRamDisk);
				}

				while (ms().theme != TWLSettings::EThemeSaturn && !screenFadedOut()) {
					swiWaitForVBlank();
				}

				if (!isDSiMode() && !ms().secondaryDevice && !extension(filename, {".plg", ".gif", ".bmp", ".png"})) {
					ntrStartSdGame();
				}

				if (tgdsMode && !ms().secondaryDevice) {
					std::string romfolderFat = replaceAll(romfolderNoSlash, "sd:", "fat:");
					snprintf (ROMpath, sizeof(ROMpath), "%s/%s", romfolderFat.c_str(), filename.c_str());
				}
				argarray.push_back(useNDSB ? (char*)ROMpathFAT.c_str() : ROMpath);
				if (!ms().btsrpBootloaderDirect && useNDSB) {
					ndsToBoot = (ms().bootstrapFile ? "sd:/_nds/nds-bootstrap-hb-nightly.nds" : "sd:/_nds/nds-bootstrap-hb-release.nds");
				}
				argarray.at(0) = (char *)(tgdsMode ? tgdsNdsPath : ndsToBoot);
				snd().stopStream();

				int err = 0;
				if (ms().btsrpBootloaderDirect && useNDSB) {
					if (access(ms().bootstrapFile ? "sd:/_nds/nds-bootstrap-hb-nightly.nds" : "sd:/_nds/nds-bootstrap-hb-release.nds", F_OK) == 0) {
						bool romIsCompressed = false;
						if (romToRamDisk == 0) {
							romIsCompressed = (extension(ROMpath, {".lz77.gen", ".lz77.md"}));
						} else if (romToRamDisk == 1) {
							romIsCompressed = (extension(ROMpath, {".lz77.smc", ".lz77.sfc"}));
						} else if (romToRamDisk == 4) {
							romIsCompressed = (extension(ROMpath, {".lz77.pce"}));
						}

						FILE* ndsFile = fopen(ndsToBoot, "rb");
						fseek(ndsFile, 0xC, SEEK_SET);
						fread(&gameTid[0], 1, 4, ndsFile);
						fseek(ndsFile, 0x15E, SEEK_SET);
						fread(&headerCRC[0], sizeof(u16), 1, ndsFile);
						fclose(ndsFile);

						if (gameTid[0][0] == 0) {
							toncset(gameTid[0], '#', 4); // Fix blank TID
						}
						char patchOffsetCacheFilePath[64];
						sprintf(patchOffsetCacheFilePath, "sd:/_nds/nds-bootstrap/patchOffsetCache/%s-%04X.bin", gameTid[0], headerCRC[0]);

						err = bootstrapHbRunNdsFile (ndsToBoot, ndsToBootFat.c_str(),
						romToRamDisk != -1 ? ROMpath : "",
						"sd:/snemulds.cfg",
						romToRamDisk != -1 ? getFileSize(ROMpath) : 0,
						"sd:/_nds/nds-bootstrap/softResetParams.bin",
						patchOffsetCacheFilePath,
						getFileSize("sd:/snemulds.cfg"),
						romToRamDisk,
						romIsCompressed,
						argarray.size(),
						(const char **)&argarray[0],
						ms().gameLanguage,
						0,
						boostCpu,
						boostVram,
						ms().consoleModel, ms().soundFreq, false);
					} else {
						err = 1;
					}
				} else {
					err = runNdsFile (ndsToBoot, argarray.size(), (const char **)&argarray[0], sys().isRunFromSD(), !useNDSB, true, dsModeSwitch, boostCpu, boostVram, tscTgds, -1);	// Pass ROM to emulator as argument
				}

				char text[64];
				snprintf(text, sizeof(text), STR_START_FAILED_ERROR.c_str(), err);
				clearText();
				printLarge(false, 4, 4, text);
				if (err == 1 && useNDSB) {
					printLarge(false, 4, 20, ms().bootstrapFile ? STR_BOOTSTRAP_HB_NIGHTLY_NOT_FOUND : STR_BOOTSTRAP_HB_RELEASE_NOT_FOUND);
				}
				printSmall(false, 4, 20 + calcLargeFontHeight(ms().bootstrapFile ? STR_BOOTSTRAP_HB_NIGHTLY_NOT_FOUND : STR_BOOTSTRAP_HB_RELEASE_NOT_FOUND), STR_PRESS_B_RETURN);
				updateText(false);
				fadeSpeed = true;
				if (ms().theme != TWLSettings::EThemeSaturn && ms().theme != TWLSettings::EThemeHBL) {
					whiteScreen = true;
					tex().clearTopScreen();
				}
				fadeType = true;
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
				vector<char *> argarray;
				argarray.push_back((char*)(sys().isRunFromSD() ? "sd:/_nds/TWiLightMenu/dsimenu.srldr" : "fat:/_nds/TWiLightMenu/dsimenu.srldr"));
				runNdsFile(argarray[0], argarray.size(), (const char**)&argarray[0], sys().isRunFromSD(), true, false, false, true, true, false, -1);
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
