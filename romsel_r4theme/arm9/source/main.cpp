#include <nds.h>
#include <nds/arm9/dldi.h>
#include "io_m3_common.h"
#include "io_g6_common.h"
#include "io_sc_common.h"
#include "exptools.h"

#include <stdio.h>
#include <fat.h>
#include "fat_ext.h"
#include <sys/stat.h>
#include <limits.h>

#include <algorithm>
#include <string.h>
#include <unistd.h>
#include <gl2d.h>

#include "date.h"
#include "fileCopy.h"

#include "graphics/graphics.h"

#include "myDSiMode.h"
#include "common/tonccpy.h"
#include "common/fatHeader.h"
#include "common/flashcard.h"
#include "ndsheaderbanner.h"
#include "gbaswitch.h"
#include "common/nds_loader_arm9.h"
#include "fileBrowse.h"
#include "perGameSettings.h"
#include "errorScreen.h"

#include "iconTitle.h"
#include "graphics/fontHandler.h"

#include "common/inifile.h"
#include "common/bootstrapsettings.h"
#include "common/stringtool.h"
#include "common/systemdetails.h"
#include "common/twlmenusettings.h"

#include "sound.h"
#include "language.h"

#include "cheat.h"
#include "crc.h"

#include "autoboot.h"	// For rebooting into the game

#include "twlClockExcludeMap.h"
#include "dmaExcludeMap.h"
#include "asyncReadExcludeMap.h"
#include "donorMap.h"
#include "saveMap.h"
#include "ROMList.h"

extern bool useTwlCfg;

bool whiteScreen = false;
bool blackScreen = false;
bool fadeType = true;		// false = out, true = in
bool fadeSpeed = true;		// false = slow (for DSi launch effect), true = fast
bool controlTopBright = true;
bool controlBottomBright = true;
bool widescreenFound = false;

extern void ClearBrightness();
extern bool lcdSwapped;

const char *unlaunchAutoLoadID = "AutoLoadInfo";
static char16_t hiyaNdsPath[] = u"sdmc:/hiya.dsi";
char launcherPath[256];

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

// These are used by flashcard functions and must retain their trailing slash.
static const std::string slashchar = "/";
static const std::string woodfat = "fat0:/";
static const std::string dstwofat = "fat1:/";

int mpuregion = 0;
int mpusize = 0;

bool applaunch = false;
bool dsModeForced = false;

bool startMenu = true;

int startMenu_cursorPosition = 0;

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

struct statvfs st[2];

touchPosition touch;

//---------------------------------------------------------------------------------
void stop (void) {
//---------------------------------------------------------------------------------
	while (1) {
		swiWaitForVBlank();
	}
}

/**
 * Disable TWL clock speed for a specific game.
 */
bool setClockSpeed(const char* filename) {
	if (!ms().ignoreBlacklists) {
		FILE *f_nds_file = fopen(filename, "rb");

		char game_TID[5];
		grabTID(f_nds_file, game_TID);
		fclose(f_nds_file);
		game_TID[4] = 0;

		// TODO: If the list gets large enough, switch to bsearch().
		for (unsigned int i = 0; i < sizeof(twlClockExcludeList)/sizeof(twlClockExcludeList[0]); i++) {
			if (memcmp(game_TID, twlClockExcludeList[i], 3) == 0) {
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
bool setCardReadDMA(const char* filename) {
	if (!ms().ignoreBlacklists) {
		FILE *f_nds_file = fopen(filename, "rb");

		char game_TID[5];
		grabTID(f_nds_file, game_TID);
		fclose(f_nds_file);
		game_TID[4] = 0;

		// TODO: If the list gets large enough, switch to bsearch().
		for (unsigned int i = 0; i < sizeof(cardReadDMAExcludeList)/sizeof(cardReadDMAExcludeList[0]); i++) {
			if (memcmp(game_TID, cardReadDMAExcludeList[i], 3) == 0) {
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
bool setAsyncCardRead(const char* filename) {
	if (!ms().ignoreBlacklists) {
		FILE *f_nds_file = fopen(filename, "rb");

		char game_TID[5];
		grabTID(f_nds_file, game_TID);
		fclose(f_nds_file);
		game_TID[4] = 0;

		// TODO: If the list gets large enough, switch to bsearch().
		for (unsigned int i = 0; i < sizeof(asyncReadExcludeList)/sizeof(asyncReadExcludeList[0]); i++) {
			if (memcmp(game_TID, asyncReadExcludeList[i], 3) == 0) {
				// Found match
				return false;
			}
		}
	}

	return perGameSettings_asyncCardRead == -1 ? DEFAULT_ASYNC_CARD_READ : perGameSettings_asyncCardRead;
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
void SetMPUSettings() {
	scanKeys();
	int pressed = keysHeld();
	
	if (pressed & KEY_B){
		mpuregion = 1;
	} else if (pressed & KEY_X){
		mpuregion = 2;
	} else if (pressed & KEY_Y){
		mpuregion = 3;
	} else {
		mpuregion = 0;
	}
	if (pressed & KEY_RIGHT){
		mpusize = 3145728;
	} else if (pressed & KEY_LEFT){
		mpusize = 1;
	} else {
		mpusize = 0;
	}
}

void unlaunchRomBoot(std::string_view rom) {
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

	tonccpy((u8*)0x02000800, unlaunchAutoLoadID, 12);
	*(u16*)(0x0200080C) = 0x3F0;		// Unlaunch Length for CRC16 (fixed, must be 3F0h)
	*(u16*)(0x0200080E) = 0;			// Unlaunch CRC16 (empty)
	*(u32*)(0x02000810) |= BIT(0);		// Load the title at 2000838h
	*(u32*)(0x02000810) |= BIT(1);		// Use colors 2000814h
	*(u16*)(0x02000814) = 0x7FFF;		// Unlaunch Upper screen BG color (0..7FFFh)
	*(u16*)(0x02000816) = 0x7FFF;		// Unlaunch Lower screen BG color (0..7FFFh)
	toncset((u8*)0x02000818, 0, 0x20+0x208+0x1C0);		// Unlaunch Reserved (zero)
	for (uint i = 0; i < sizeof(hiyaNdsPath)/sizeof(hiyaNdsPath[0]); i++) {
		((char16_t*)0x02000838)[i] = hiyaNdsPath[i];		// Unlaunch Device:/Path/Filename.ext (16bit Unicode,end by 0000h)
	}
	*(u16*)(0x0200080E) = swiCRC16(0xFFFF, (void*)0x02000810, 0x3F0);		// Unlaunch CRC16
}

/**
 * Reboot into an SD game when in DS mode.
 */
void ntrStartSdGame(void) {
	*(u32*)0x02000000 |= BIT(3);
	*(u32*)0x02000004 = 0;
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
	*(u32*)(0x02000300) = 0x434E4C54;	// Set "CNLT" warmboot flag
	*(u16*)(0x02000304) = 0x1801;
	*(u32*)(0x02000308) = 0x43415254;	// "CART"
	*(u32*)(0x0200030C) = 0x00000000;
	*(u32*)(0x02000310) = 0x43415254;	// "CART"
	*(u32*)(0x02000314) = 0x00000000;
	*(u32*)(0x02000318) = 0x00000013;
	*(u32*)(0x0200031C) = 0x00000000;
	*(u16*)(0x02000306) = swiCRC16(0xFFFF, (void*)0x02000308, 0x18);
	
	unlaunchSetHiyaBoot();

	DC_FlushAll();						// Make reboot not fail
	fifoSendValue32(FIFO_USER_02, 1);	// Reboot into DSiWare title, booted via Launcher
	stop();
}

/**
 * Fix AP for some games.
 */
std::string setApFix(const char *filename) {
	if (flashcardFound()) {
		remove("fat:/_nds/nds-bootstrap/apFix.ips");
		remove("fat:/_nds/nds-bootstrap/apFixCheat.bin");
	}

	FILE *f_nds_file = fopen(filename, "rb");

	char game_TID[5];
	u16 headerCRC16 = 0;
	fseek(f_nds_file, offsetof(sNDSHeaderExt, gameCode), SEEK_SET);
	fread(game_TID, 1, 4, f_nds_file);
	fseek(f_nds_file, offsetof(sNDSHeaderExt, headerCRC16), SEEK_SET);
	fread(&headerCRC16, sizeof(u16), 1, f_nds_file);
	fclose(f_nds_file);
	game_TID[4] = 0;

	bool ipsFound = false;
	bool cheatVer = true;
	char ipsPath[256];
	char ipsPath2[256];
	if (!ipsFound) {
		snprintf(ipsPath, sizeof(ipsPath), "%s:/_nds/TWiLightMenu/extras/apfix/cht/%s.bin", sys().isRunFromSD() ? "sd" : "fat", filename);
		ipsFound = (access(ipsPath, F_OK) == 0);
	}

	if (!ipsFound) {
		snprintf(ipsPath, sizeof(ipsPath), "%s:/_nds/TWiLightMenu/extras/apfix/cht/%s-%X.bin", sys().isRunFromSD() ? "sd" : "fat", game_TID, headerCRC16);
		ipsFound = (access(ipsPath, F_OK) == 0);
	}

	if (!ipsFound) {
		snprintf(ipsPath, sizeof(ipsPath), "%s:/_nds/TWiLightMenu/extras/apfix/%s.ips", sys().isRunFromSD() ? "sd" : "fat", filename);
		ipsFound = (access(ipsPath, F_OK) == 0);
		if (ipsFound) {
			cheatVer = false;
		}
	}

	if (!ipsFound) {
		snprintf(ipsPath, sizeof(ipsPath), "%s:/_nds/TWiLightMenu/extras/apfix/%s-%X.ips", sys().isRunFromSD() ? "sd" : "fat", game_TID, headerCRC16);
		ipsFound = (access(ipsPath, F_OK) == 0);
		if (ipsFound) {
			cheatVer = false;
		}
	}

	if (ipsFound) {
		if (ms().secondaryDevice && sys().isRunFromSD()) {
			mkdir("fat:/_nds", 0777);
			mkdir("fat:/_nds/nds-bootstrap", 0777);
			fcopy(ipsPath, cheatVer ? "fat:/_nds/nds-bootstrap/apFixCheat.bin" : "fat:/_nds/nds-bootstrap/apFix.ips");
			return cheatVer ? "fat:/_nds/nds-bootstrap/apFixCheat.bin" : "fat:/_nds/nds-bootstrap/apFix.ips";
		}
		return ipsPath;
	} else {
		FILE *file = fopen(sys().isRunFromSD() ? "sd:/_nds/TWiLightMenu/extras/apfix.pck" : "fat:/_nds/TWiLightMenu/extras/apfix.pck", "rb");
		if (file) {
			char buf[5] = {0};
			fread(buf, 1, 4, file);
			if (strcmp(buf, ".PCK") != 0) // Invalid file
				return "";

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
				int cmp = strcmp(buf, game_TID);
				if (cmp == 0) { // TID matches, check CRC
					u16 crc;
					fread(&crc, 1, sizeof(crc), file);

					if (crc == headerCRC16) { // CRC matches
						fread(&offset, 1, sizeof(offset), file);
						fread(&size, 1, sizeof(size), file);
						cheatVer = fgetc(file) & 1;
						break;
					} else if (crc < headerCRC16) {
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

			if (offset > 0 && size > 0) {
				fseek(file, offset, SEEK_SET);
				u8 *buffer = new u8[size];
				fread(buffer, 1, size, file);

				if (flashcardFound()) {
					mkdir("fat:/_nds", 0777);
					mkdir("fat:/_nds/nds-bootstrap", 0777);
				}
				snprintf(ipsPath, sizeof(ipsPath), "%s:/_nds/nds-bootstrap/apFix%s", ms().secondaryDevice ? "fat" : "sd", cheatVer ? "Cheat.bin" : ".ips");
				snprintf(ipsPath2, sizeof(ipsPath2), "%s:/_nds/nds-bootstrap/apFix%s", ms().secondaryDevice ? "fat" : "sd", cheatVer ? ".ips" : "Cheat.bin");
				if (access(ipsPath2, F_OK) == 0) {
					remove(ipsPath2); // Delete leftover AP-fix file of opposite format
				}
				FILE *out = fopen(ipsPath, "wb");
				if (out) {
					fwrite(buffer, 1, size, out);
					fclose(out);
				}
				delete[] buffer;
				fclose(file);
				return ipsPath;
			}

			fclose(file);
		}
	}

	return "";
}

sNDSHeader ndsCart;

/**
 * Enable widescreen for some games.
 */
void SetWidescreen(const char *filename) {
	const char* wideCheatDataPath = ms().secondaryDevice && (!isDSiWare || (isDSiWare && !ms().dsiWareToSD)) ? "fat:/_nds/nds-bootstrap/wideCheatData.bin" : "sd:/_nds/nds-bootstrap/wideCheatData.bin";
	remove(wideCheatDataPath);

	bool useWidescreen = (perGameSettings_wideScreen == -1 ? ms().wideScreen : perGameSettings_wideScreen);

	if ((isDSiMode() && sys().arm7SCFGLocked()) || ms().consoleModel < 2
	|| !useWidescreen || !widescreenFound || ms().macroMode) {
		return;
	}

	if (isHomebrew && ms().homebrewHasWide && widescreenFound) {
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
	if (ms().launchType[ms().secondaryDevice] == TWLSettings::ESDFlashcardLaunch) {
		snprintf(wideBinPath, sizeof(wideBinPath), "sd:/_nds/TWiLightMenu/extras/widescreen/%s.bin", filename);
		wideCheatFound = (access(wideBinPath, F_OK) == 0);
	}

	char game_TID[5];
	u16 headerCRC16 = 0;

	if (ms().slot1Launched) {
		// Reset Slot-1 to allow reading card header
		sysSetCardOwner (BUS_OWNER_ARM9);
		disableSlot1();
		for (int i = 0; i < 25; i++) { swiWaitForVBlank(); }
		enableSlot1();
		for (int i = 0; i < 15; i++) { swiWaitForVBlank(); }

		cardReadHeader((uint8*)&ndsCart);

		tonccpy(game_TID, ndsCart.gameCode, 4);
		game_TID[4] = 0;
		headerCRC16 = ndsCart.headerCRC16;

		snprintf(wideBinPath, sizeof(wideBinPath), "sd:/_nds/TWiLightMenu/extras/widescreen/%s-%X.bin", game_TID, ndsCart.headerCRC16);
		wideCheatFound = (access(wideBinPath, F_OK) == 0);
	} else if (!wideCheatFound) {
		FILE *f_nds_file = fopen(filename, "rb");

		fseek(f_nds_file, offsetof(sNDSHeaderExt, gameCode), SEEK_SET);
		fread(game_TID, 1, 4, f_nds_file);
		fseek(f_nds_file, offsetof(sNDSHeaderExt, headerCRC16), SEEK_SET);
		fread(&headerCRC16, sizeof(u16), 1, f_nds_file);
		fclose(f_nds_file);
		game_TID[4] = 0;

		snprintf(wideBinPath, sizeof(wideBinPath), "sd:/_nds/TWiLightMenu/extras/widescreen/%s-%X.bin", game_TID, headerCRC16);
		wideCheatFound = (access(wideBinPath, F_OK) == 0);
	}

	if (isHomebrew) {
		return;
	}

	mkdir(ms().secondaryDevice && (!isDSiWare || (isDSiWare && !ms().dsiWareToSD)) ? "fat:/_nds" : "sd:/_nds", 0777);
	mkdir(ms().secondaryDevice && (!isDSiWare || (isDSiWare && !ms().dsiWareToSD)) ? "fat:/_nds/nds-bootstrap" : "sd:/_nds/nds-bootstrap", 0777);

	if (wideCheatFound) {
		if (fcopy(wideBinPath, wideCheatDataPath) != 0) {
			const char* resultText1 = "Failed to copy widescreen";
			const char* resultText2 = "code for the game.";
			remove(wideCheatDataPath);
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
			return;
		}
	} else {
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
				int cmp = strcmp(buf, game_TID);
				if (cmp == 0) { // TID matches, check CRC
					u16 crc;
					fread(&crc, 1, sizeof(crc), file);

					if (crc == headerCRC16) { // CRC matches
						fread(&offset, 1, sizeof(offset), file);
						fread(&size, 1, sizeof(size), file);
						wideCheatFound = true;
						break;
					} else if (crc < headerCRC16) {
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

				snprintf(wideBinPath, sizeof(wideBinPath), "%s:/_nds/nds-bootstrap/wideCheatData.bin", ms().secondaryDevice && (!isDSiWare || (isDSiWare && !ms().dsiWareToSD)) ? "fat" : "sd");
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

	FILE *f_nds_file = fopen(filename, "rb");
	if (f_nds_file) {
		char game_TID[5];
		fseek(f_nds_file, offsetof(sNDSHeaderExt, gameCode), SEEK_SET);
		fread(game_TID, 1, 4, f_nds_file);
		fclose(f_nds_file);
		game_TID[4] = 0;

		snprintf(manualPath, sizeof(manualPath), "%s:/_nds/TWiLightMenu/extras/manuals/%s.txt", sys().isRunFromSD() ? "sd" : "fat", game_TID);
		if (access(manualPath, F_OK) == 0)
			return manualPath;

		snprintf(manualPath, sizeof(manualPath), "%s:/_nds/TWiLightMenu/extras/manuals/%.3s.txt", sys().isRunFromSD() ? "sd" : "fat", game_TID);
		if (access(manualPath, F_OK) == 0)
			return manualPath;
	}

	return "";
}

char filePath[PATH_MAX];

//---------------------------------------------------------------------------------
void doPause() {
//---------------------------------------------------------------------------------
	while (1) {
		scanKeys();
		if (keysDown() & KEY_START)
			break;
		swiWaitForVBlank();
	}
	scanKeys();
}

void loadGameOnFlashcard (const char* ndsPath, bool dsGame) {
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
		std::string romFolderNoSlash = ms().romfolder[true];
		RemoveTrailingSlashes(romFolderNoSlash);
		mkdir("saves", 0777);
		std::string savepath = romFolderNoSlash + "/saves/" + savename;
		std::string savepathFc = romFolderNoSlash + "/" + savenameFc;
		rename(savepath.c_str(), savepathFc.c_str());
	}

	std::string fcPath;
	int err = 0;
	if ((memcmp(io_dldi_data->friendlyName, "R4(DS) - Revolution for DS", 26) == 0)
	 || (memcmp(io_dldi_data->friendlyName, "R4TF", 4) == 0)
	 || (memcmp(io_dldi_data->friendlyName, "R4iDSN", 6) == 0)
	 || (memcmp(io_dldi_data->friendlyName, "R4iTT", 5) == 0)
	 || (memcmp(io_dldi_data->friendlyName, "Acekard AK2", 0xB) == 0)
     || (memcmp(io_dldi_data->friendlyName, "Ace3DS+", 7) == 0)) {
		if (sys().isDSLite()) {
			CIniFile backlightini("fat:/_wfwd/backlight.ini");
			backlightini.SetInt("brightness", "brightness", *(int*)0x02003000);
			backlightini.SaveIniFile("fat:/_wfwd/backlight.ini");
		}
		CIniFile fcrompathini("fat:/_wfwd/lastsave.ini");
		fcPath = replaceAll(ndsPath, "fat:/", woodfat);
		fcrompathini.SetString("Save Info", "lastLoaded", fcPath);
		fcrompathini.SaveIniFile("fat:/_wfwd/lastsave.ini");
		err = runNdsFile("fat:/Wfwd.dat", 0, NULL, true, true, true, runNds_boostCpu, runNds_boostVram, false, -1);
	} else if (memcmp(io_dldi_data->friendlyName, "DSTWO(Slot-1)", 0xD) == 0) {
		CIniFile fcrompathini("fat:/_dstwo/autoboot.ini");
		fcPath = replaceAll(ndsPath, "fat:/", dstwofat);
		fcrompathini.SetString("Dir Info", "fullName", fcPath);
		fcrompathini.SaveIniFile("fat:/_dstwo/autoboot.ini");
		err = runNdsFile("fat:/_dstwo/autoboot.nds", 0, NULL, true, true, true, runNds_boostCpu, runNds_boostVram, false, -1);
	} else if ((memcmp(io_dldi_data->friendlyName, "TTCARD", 6) == 0)
			 || (memcmp(io_dldi_data->friendlyName, "DSTT", 4) == 0)
			 || (memcmp(io_dldi_data->friendlyName, "DEMON", 5) == 0)
			 || (memcmp(io_dldi_data->friendlyName, "DSONE", 5) == 0)
			 || (memcmp(io_dldi_data->friendlyName, "M3DS DLDI", 9) == 0)
			 || (memcmp(io_dldi_data->friendlyName, "M3-DS", 5) == 0)) {
		CIniFile fcrompathini("fat:/TTMenu/YSMenu.ini");
		fcPath = replaceAll(ndsPath, "fat:/", slashchar);
		fcrompathini.SetString("YSMENU", "AUTO_BOOT", fcPath);
		fcrompathini.SaveIniFile("fat:/TTMenu/YSMenu.ini");
		err = runNdsFile("fat:/YSMenu.nds", 0, NULL, true, true, true, runNds_boostCpu, runNds_boostVram, false, -1);
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
	vector<char *> argarray;
	argarray.push_back((char*)(sys().isRunFromSD() ? "sd:/_nds/TWiLightMenu/r4menu.srldr" : "fat:/_nds/TWiLightMenu/r4menu.srldr"));
	runNdsFile(argarray[0], argarray.size(), (const char**)&argarray[0], true, false, false, true, true, false, -1);
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
		fwrite(&h, sizeof(FATHeader), 1, file); // Write header
		fseek(file, size - 1, SEEK_SET); // Pad rest of the file
		fputc('\0', file);
		fclose(file);
		return true;
	}

	return false;
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

	if (ms().theme == TWLSettings::EThemeGBC) {
		snd().stopStream();
	}
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
	fadeType = true;
	if (ms().theme == TWLSettings::EThemeGBC) {
		snd().beginStream();
	}
	while (!screenFadedIn()) {
		swiWaitForVBlank();
	}
	controlTopBright = topControlBak;
	controlBottomBright = botControlBak;
}

void bgOperations(bool waitFrame) {
	if ((keysHeld() & KEY_LID) && ms().sleepMode) {
		customSleep();
	}
	checkSdEject();
	snd().updateStream();
	if (waitFrame) {
		swiWaitForVBlank();
	}
}

//---------------------------------------------------------------------------------
int r4Theme(void) {
//---------------------------------------------------------------------------------
	graphicsInit();

	if (sdFound()) statvfs("sd:/", &st[0]);
	if (flashcardFound()) statvfs("fat:/", &st[1]);

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

	std::string filename;

	ms().loadSettings();
	bs().loadSettings();

	if (sdFound() && ms().consoleModel >= 2 && !sys().arm7SCFGLocked()) {
		CIniFile lumaConfig("sd:/luma/config.ini");
		widescreenFound = ((access("sd:/_nds/TWiLightMenu/TwlBg/Widescreen.cxi", F_OK) == 0) && (lumaConfig.GetInt("boot", "enable_external_firm_and_modules", 0) == true));
	}

	langInit();

	if (ms().theme == TWLSettings::EThemeGBC) {
		extern int screenBrightness;
		screenBrightness = 31;
		fadeType = false;
		SetBrightness(0, -31);
		SetBrightness(1, -31);
	}

	if (sdFound() && ms().consoleModel < 2 && ms().launcherApp != -1) {
		u8 setRegion = 0;
		if (ms().sysRegion == -1) {
			// Determine SysNAND region by searching region of System Settings on SDNAND
			char tmdpath[256];
			for (u8 i = 0x41; i <= 0x5A; i++) {
				snprintf(tmdpath, sizeof(tmdpath), "sd:/title/00030015/484e42%x/content/title.tmd", i);
				if (access(tmdpath, F_OK) == 0) {
					setRegion = i;
					break;
				}
			}
		} else {
			switch(ms().sysRegion) {
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

		if (ms().sysRegion == 9) {
			sprintf(launcherPath, "nand:/launcher.dsi");
		} else {
			sprintf(launcherPath,
				 "nand:/title/00030017/484E41%X/content/0000000%i.app", setRegion, ms().launcherApp);
		}
	}

	keysSetRepeat(15, 2);

	std::vector<std::string_view> extensionList = {
		".nds", ".dsi", ".ids", ".srl", ".app", ".argv", // NDS
		".agb", ".gba", ".mb", // GBA
		".a26", // Atari 2600
		".a52", // Atari 5200
		".a78", // Atari 7800
		".xex", ".atr", // Atari XEGS
		".col", // ColecoVision
		".int", // Intellivision
		".m5", // Sord M5
		".gb", ".sgb", ".gbc", // Game Boy
		".nes", ".fds", // NES
		".sg", // Sega SG-1000
		".sms", // Sega Master System
		".gg", // Sega Game Gear
		".gen", // Sega Mega Drive/Genesis
		".smc", ".sfc", // SNES
		".ws", ".wsc", // WonderSwan
		".ngp", ".ngc", // Neo Geo Pocket
		".pce", // PC Engine/TurboGrafx-16
		".dsk", // Amstrad CPC
		".avi", // Xvid (AVI)
		".rvid", // Rocket Video
		".fv", // FastVideo
		".gif", // GIF
		".bmp", // BMP
		".png" // Portable Network Graphics
	};

	if (dsiFeatures() && ms().consoleModel < 2) {
		char currentDate[16];
		time_t Raw;
		time(&Raw);
		const struct tm *Time = localtime(&Raw);

		strftime(currentDate, sizeof(currentDate), "%m/%d", Time);

		if (strcmp(currentDate, "04/01") == 0) {
			// 3DS (for April Fools)
			extensionList.emplace_back(".3ds");
			extensionList.emplace_back(".cia");
			extensionList.emplace_back(".cxi");
		}
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

	srand(time(NULL));
	
	bool copyDSiWareSavBack =
	   (ms().previousUsedDevice && bothSDandFlashcard() && ms().launchType[ms().previousUsedDevice] == 3
	&& ((access(ms().dsiWarePubPath.c_str(), F_OK) == 0 && access("sd:/_nds/TWiLightMenu/tempDSiWare.pub", F_OK) == 0)
	 || (access(ms().dsiWarePrvPath.c_str(), F_OK) == 0 && access("sd:/_nds/TWiLightMenu/tempDSiWare.prv", F_OK) == 0)));
	
	if (copyDSiWareSavBack) {
		blackScreen = true;
	}

	graphicsLoad();
	fontInit();

	iconTitleInit();

	bool menuButtonPressed = false;
	
	if (ms().theme == TWLSettings::EThemeGBC) {
		startMenu = false;
		fadeType = true;	// Fade in from white
		snd();
		snd().beginStream();
	}

	char path[256];

	if (copyDSiWareSavBack) {
		printLargeCentered(false, 16, "If this takes a while, close");
		printLargeCentered(false, 24, "and open the console's lid.");
		printLargeCentered(false, 88, "Now copying data...");
		printLargeCentered(false, 96, "Do not turn off the power.");
		if (access(ms().dsiWarePubPath.c_str(), F_OK) == 0) {
			fcopy("sd:/_nds/TWiLightMenu/tempDSiWare.pub", ms().dsiWarePubPath.c_str());
			rename("sd:/_nds/TWiLightMenu/tempDSiWare.pub", "sd:/_nds/TWiLightMenu/tempDSiWare.pub.bak");
		}
		if (access(ms().dsiWarePrvPath.c_str(), F_OK) == 0) {
			fcopy("sd:/_nds/TWiLightMenu/tempDSiWare.prv", ms().dsiWarePrvPath.c_str());
			rename("sd:/_nds/TWiLightMenu/tempDSiWare.prv", "sd:/_nds/TWiLightMenu/tempDSiWare.prv.bak");
		}
		clearText(false);
		blackScreen = false;
	}

	while (1) {

		if (startMenu) {
			fadeType = true;	// Fade in from white

			int pressed = 0;

		  if (ms().theme == TWLSettings::EThemeGBC) {
				vector<char *> argarray;
				argarray.push_back((char*)(sys().isRunFromSD() ? "sd:/_nds/TWiLightMenu/settings.srldr" : "fat:/_nds/TWiLightMenu/settings.srldr"));
				int err = runNdsFile(argarray[0], argarray.size(), (const char**)&argarray[0], true, false, false, true, true, false, -1);
				iprintf ("Start failed. Error %i\n", err);
		  } else {
			lcdMainOnBottom();
			lcdSwapped = true;
			do {
				clearText();
				printLargeCentered(false, -112, 166, DrawDate());
				if (!ms().kioskMode) {
					printLargeCentered(false, 180, "SELECT: Settings menu");
				}
				switch (startMenu_cursorPosition) {
					case 0:
					default:
						printLargeCentered(false, 166, "Game");
						break;
					case 1:
						if (flashcardFound() && (io_dldi_data->ioInterface.features & FEATURE_SLOT_NDS)) {
							printLargeCentered(false, 166, "Not used");
						} else {
							printLargeCentered(false, 166, "Launch Slot-1 card");
						}
						break;
					case 2:
						if (sys().isRegularDS() && ms().gbaBooter != 2) {
							printLargeCentered(false, 166, "Start GBA Mode");
						} else {
							printLargeCentered(false, 166, "Start GBARunner2");
						}
						break;
				}
				printLargeCentered(false, 112, 166, RetTime().c_str());

				scanKeys();
				pressed = keysDownRepeat();
				touchRead(&touch);
				bgOperations(true);
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
				vector<char *> argarray;
				argarray.push_back((char*)(sys().isRunFromSD() ? "sd:/_nds/TWiLightMenu/manual.srldr" : "fat:/_nds/TWiLightMenu/manual.srldr"));
				int err = runNdsFile(argarray[0], argarray.size(), (const char**)&argarray[0], true, false, false, true, true, false, -1);
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

							ms().slot1Launched = true;
							ms().saveSettings();

							bool directMethod = false;
							if (io_dldi_data->ioInterface.features & FEATURE_SLOT_GBA) {
								directMethod = true;
							} else if (ms().slot1LaunchMethod==0 || sys().arm7SCFGLocked()) {
								dsCardLaunch();
							} else if (ms().slot1LaunchMethod==2) {
								unlaunchRomBoot("cart:");
							} else {
								directMethod = true;
							}

							if (directMethod) {
								SetWidescreen(NULL);
								chdir(sys().isRunFromSD() ? "sd:/" : "fat:/");
								int err = runNdsFile ("/_nds/TWiLightMenu/slot1launch.srldr", 0, NULL, true, true, false, true, true, false, -1);
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
						ms().slot1Launched = false;
						if (ms().gbaBooter == TWLSettings::EGbaGbar2) {
							if (ms().secondaryDevice) {
								const char* gbaRunner2Path = ms().gbar2DldiAccess ? "fat:/_nds/GBARunner2_arm7dldi_ds.nds" : "fat:/_nds/GBARunner2_arm9dldi_ds.nds";
								if (isDSiMode()) {
									gbaRunner2Path = ms().consoleModel>0 ? "fat:/_nds/GBARunner2_arm7dldi_3ds.nds" : "fat:/_nds/GBARunner2_arm7dldi_dsi.nds";
								}
								loadPerGameSettings(filename);
								if ((perGameSettings_useBootstrap == -1 ? ms().useBootstrap : perGameSettings_useBootstrap)) {
									int err = runNdsFile (gbaRunner2Path, 0, NULL, true, true, false, true, false, false, -1);
									iprintf ("Start failed. Error %i\n", err);
								} else {
									loadGameOnFlashcard(gbaRunner2Path, false);
								}
							} else {
								std::string bootstrapPath = (ms().bootstrapFile ? "sd:/_nds/nds-bootstrap-hb-nightly.nds" : "sd:/_nds/nds-bootstrap-hb-release.nds");

								std::vector<char*> argarray;
								argarray.push_back(strdup(bootstrapPath.c_str()));
								argarray.at(0) = (char*)bootstrapPath.c_str();

								const char* gbar2Path = ms().consoleModel>0 ? "sd:/_nds/GBARunner2_arm7dldi_3ds.nds" : "sd:/_nds/GBARunner2_arm7dldi_dsi.nds";
								if (sys().arm7SCFGLocked() && !sys().dsiWramAccess()) {
									gbar2Path = ms().consoleModel>0 ? "sd:/_nds/GBARunner2_arm7dldi_nodsp_3ds.nds" : "sd:/_nds/GBARunner2_arm7dldi_nodsp_dsi.nds";
								}

								CIniFile bootstrapini( BOOTSTRAP_INI );
								bootstrapini.SetString("NDS-BOOTSTRAP", "NDS_PATH", gbar2Path);
								bootstrapini.SetString("NDS-BOOTSTRAP", "HOMEBREW_ARG", "");
								bootstrapini.SetString("NDS-BOOTSTRAP", "RAM_DRIVE_PATH", "");
								bootstrapini.SetString("NDS-BOOTSTRAP", "GUI_LANGUAGE", ms().getGuiLanguageString());
								bootstrapini.SetInt("NDS-BOOTSTRAP", "LANGUAGE", ms().getGameLanguage());
								bootstrapini.SetInt("NDS-BOOTSTRAP", "DSI_MODE", 0);
								bootstrapini.SetInt("NDS-BOOTSTRAP", "BOOST_CPU", 1);
								bootstrapini.SetInt("NDS-BOOTSTRAP", "BOOST_VRAM", 0);
								bootstrapini.SaveIniFile( BOOTSTRAP_INI );
								int err = runNdsFile (argarray[0], argarray.size(), (const char **)&argarray[0], false, true, false, true, true, false, -1);
								iprintf ("Start failed. Error %i\n", err);
								if (err == 1) {
									iprintf(ms().bootstrapFile ? "nds-bootstrap for homebrew (Nightly)" : "nds-bootstrap for homebrew (Release)");
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

			if ((pressed & KEY_B) && !sys().isRegularDS()) {
				fadeType = false;	// Fade to white
				for (int i = 0; i < 25; i++) swiWaitForVBlank();
				if (!sdFound() || ms().launcherApp == -1) {
					*(u32*)(0x02000300) = 0x434E4C54;	// Set "CNLT" warmboot flag
					*(u16*)(0x02000304) = 0x1801;
					*(u32*)(0x02000310) = 0x4D454E55;	// "MENU"
					unlaunchSetHiyaBoot();
				} else {
					unlaunchRomBoot(launcherPath);
				}
				fifoSendValue32(FIFO_USER_02, 1);	// ReturntoDSiMenu
			}

			if (((pressed & KEY_START) || (pressed & KEY_SELECT)) && !ms().kioskMode) {
				// Launch settings
				fadeType = false;	// Fade to white
				for (int i = 0; i < 25; i++) {
					swiWaitForVBlank();
				}

				ms().saveSettings();
				vector<char *> argarray;
				argarray.push_back((char*)(sys().isRunFromSD() ? "sd:/_nds/TWiLightMenu/settings.srldr" : "fat:/_nds/TWiLightMenu/settings.srldr"));
				int err = runNdsFile(argarray[0], argarray.size(), (const char**)&argarray[0], true, false, false, true, true, false, -1);
				iprintf ("Start failed. Error %i\n", err);
			}
		} else {
			snprintf (path, sizeof(path), "%s", ms().romfolder[ms().secondaryDevice].c_str());
			// Set directory
			chdir (path);

			if (ms().theme == TWLSettings::EThemeGBC) {
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
			getcwd (filePath, PATH_MAX);
			int pathLen = strlen(filePath);
			if (pathLen < PATH_MAX && filePath[pathLen - 1] != '/') { // Ensure the path ends in a slash
				filePath[pathLen] = '/';
				filePath[pathLen + 1] = '\0';
				pathLen++;
			}
			vector<char*> argarray;

			bool isArgv = false;
			if (extension(filename, {".argv"})) {
				FILE *argfile = fopen(filename.c_str(),"rb");
					char str[PATH_MAX], *pstr;
				const char seps[]= "\n\r\t ";

				while ( fgets(str, PATH_MAX, argfile) ) {
					// Find comment and end string there
					if ( (pstr = strchr(str, '#')) )
						*pstr= '\0';

					// Tokenize arguments
					pstr= strtok(str, seps);

					while ( pstr != NULL ) {
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

			ms().slot1Launched = false;

			// Launch DSiWare .nds via Unlaunch
			if (isDSiWare) {
				remove(sys().isRunFromSD() ? "sd:/_nds/nds-bootstrap/esrb.bin" : "fat:/_nds/nds-bootstrap/esrb.bin");

				std::string typeToReplace = filename.substr(filename.rfind('.'));

				char *name = argarray.at(0);
				strcpy (filePath + pathLen, name);
				free(argarray.at(0));
				argarray.at(0) = filePath;

				std::string romFolderNoSlash = ms().romfolder[ms().secondaryDevice];
				RemoveTrailingSlashes(romFolderNoSlash);
				mkdir ("saves", 0777);

				sNDSHeaderExt NDSHeader;

				FILE *f_nds_file = fopen(filename.c_str(), "rb");

				fread(&NDSHeader, 1, sizeof(NDSHeader), f_nds_file);
				fclose(f_nds_file);

				ms().dsiWareSrlPath = std::string(argarray[0]);
				ms().dsiWarePubPath = romFolderNoSlash + "/saves/" + filename;
				ms().dsiWarePrvPath = ms().dsiWarePubPath;
				bool savFormat = (ms().secondaryDevice && (!sdFound() || !ms().dsiWareToSD || bs().b4dsMode));
				if (savFormat) {
					ms().dsiWarePubPath = replaceAll(ms().dsiWarePubPath, typeToReplace, getSavExtension());
					ms().dsiWarePrvPath = ms().dsiWarePubPath;
				} else {
					ms().dsiWarePubPath = replaceAll(ms().dsiWarePubPath, typeToReplace, getPubExtension());
					ms().dsiWarePrvPath = replaceAll(ms().dsiWarePrvPath, typeToReplace, getPrvExtension());
				}
				if (!isArgv) {
					ms().romPath[ms().secondaryDevice] = argarray[0];
				}
				ms().homebrewBootstrap = isHomebrew;
				ms().launchType[ms().secondaryDevice] = TWLSettings::EDSiWareLaunch;
				ms().previousUsedDevice = ms().secondaryDevice;
				ms().saveSettings();

				if (savFormat) {
					if ((getFileSize(ms().dsiWarePubPath.c_str()) == 0) && ((NDSHeader.pubSavSize > 0) || (NDSHeader.prvSavSize > 0))) {
						clearText();
						dialogboxHeight = 0;
						showdialogbox = true;
						printLargeCentered(false, 74, "Save creation");
						printSmallCentered(false, 98, "Creating save file...");

						FILE *pFile = fopen(ms().dsiWarePubPath.c_str(), "wb");
						if (pFile) {
							u32 savesize = ((NDSHeader.pubSavSize > 0) ? NDSHeader.pubSavSize : NDSHeader.prvSavSize);
							fseek(pFile, savesize - 1, SEEK_SET);
							fputc('\0', pFile);
							fclose(pFile);
						}

						clearText();
						printLargeCentered(false, 74, "Save creation");
						printSmallCentered(false, 98, "Save file created!");
						for (int i = 0; i < 60; i++) swiWaitForVBlank();
					}
				} else {
					if ((getFileSize(ms().dsiWarePubPath.c_str()) == 0) && (NDSHeader.pubSavSize > 0)) {
						clearText();
						dialogboxHeight = 0;
						showdialogbox = true;
						printLargeCentered(false, 74, "Save creation");
						printSmallCentered(false, 98, "Creating public save file...");

						createDSiWareSave(ms().dsiWarePubPath.c_str(), NDSHeader.pubSavSize);

						clearText();
						printLargeCentered(false, 74, "Save creation");
						printSmallCentered(false, 98, "Public save file created!");
						for (int i = 0; i < 60; i++) swiWaitForVBlank();
					}

					if ((getFileSize(ms().dsiWarePrvPath.c_str()) == 0) && (NDSHeader.prvSavSize > 0)) {
						clearText();
						dialogboxHeight = 0;
						showdialogbox = true;
						printLargeCentered(false, 74, "Save creation");
						printSmallCentered(false, 98, "Creating private save file...");

						createDSiWareSave(ms().dsiWarePrvPath.c_str(), NDSHeader.prvSavSize);

						clearText();
						printLargeCentered(false, 74, "Save creation");
						printSmallCentered(false, 98, "Private save file created!");
						for (int i = 0; i < 60; i++) swiWaitForVBlank();
					}
				}

				loadPerGameSettings(filename);

				if (ms().secondaryDevice && !bs().b4dsMode && (ms().dsiWareToSD || (!(perGameSettings_dsiwareBooter == -1 ? ms().dsiWareBooter : perGameSettings_dsiwareBooter) && ms().consoleModel == 0)) && sdFound()) {
					clearText();
					dialogboxHeight = 0;
					showdialogbox = true;
					printLargeCentered(false, 74, "Please wait");
					printSmallCentered(false, 98, "Now copying data...");
					printSmallCentered(false, 110, "Do not turn off the power.");
					fcopy(ms().dsiWareSrlPath.c_str(), "sd:/_nds/TWiLightMenu/tempDSiWare.dsi");
					if ((access(ms().dsiWarePubPath.c_str(), F_OK) == 0) && (NDSHeader.pubSavSize > 0)) {
						fcopy(ms().dsiWarePubPath.c_str(), "sd:/_nds/TWiLightMenu/tempDSiWare.pub");
					}
					if ((access(ms().dsiWarePrvPath.c_str(), F_OK) == 0) && (NDSHeader.prvSavSize > 0)) {
						fcopy(ms().dsiWarePrvPath.c_str(), "sd:/_nds/TWiLightMenu/tempDSiWare.prv");
					}

					clearText();
					if ((access(ms().dsiWarePubPath.c_str(), F_OK) == 0 && (NDSHeader.pubSavSize > 0))
					 || (access(ms().dsiWarePrvPath.c_str(), F_OK) == 0 && (NDSHeader.prvSavSize > 0))) {
						dialogboxHeight = 1;
						printLargeCentered(false, 74, "Important!");
						printSmall(false, 2, 90, "After saving, please re-start");
						printSmall(false, 2, 102, "TWiLight Menu++ to transfer your");
						printSmall(false, 2, 114, "save data back.");
						for (int i = 0; i < 60*3; i++) swiWaitForVBlank();		// Wait 3 seconds
					}
				}

				if ((perGameSettings_dsiwareBooter == -1 ? ms().dsiWareBooter : perGameSettings_dsiwareBooter) || (ms().secondaryDevice && bs().b4dsMode) || sys().arm7SCFGLocked() || ms().consoleModel > 0) {
					CheatCodelist codelist;
					u32 gameCode, crc32;

					bool cheatsEnabled = true;
					const char* cheatDataBin = (ms().secondaryDevice && ms().dsiWareToSD && sdFound()) ? "sd:/_nds/nds-bootstrap/cheatData.bin" : "/_nds/nds-bootstrap/cheatData.bin";
					mkdir((ms().secondaryDevice && ms().dsiWareToSD && sdFound()) ? "sd:/_nds" : "/_nds", 0777);
					mkdir((ms().secondaryDevice && ms().dsiWareToSD && sdFound()) ? "sd:/_nds/nds-bootstrap" : "/_nds/nds-bootstrap", 0777);
					if (codelist.romData(ms().dsiWareSrlPath,gameCode,crc32)) {
						long cheatOffset; size_t cheatSize;
						FILE* dat=fopen(sys().isRunFromSD() ? "sd:/_nds/TWiLightMenu/extras/usrcheat.dat" : "fat:/_nds/TWiLightMenu/extras/usrcheat.dat","rb");
						if (dat) {
							if (codelist.searchCheatData(dat, gameCode, crc32, cheatOffset, cheatSize)) {
								codelist.parse(ms().dsiWareSrlPath);
								codelist.writeCheatsToFile(cheatDataBin);
								FILE* cheatData=fopen(cheatDataBin,"rb");
								if (cheatData) {
									u32 check[2];
									fread(check, 1, 8, cheatData);
									fclose(cheatData);
									if (check[1] == 0xCF000000 || getFileSize(cheatDataBin) > 0x1C00) {
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
					bootstrapini.SetString("NDS-BOOTSTRAP", "AP_FIX_PATH", "");
					bootstrapini.SetString("NDS-BOOTSTRAP", "MANUAL_PATH", getGameManual(filename.c_str()));
					bootstrapini.SetString("NDS-BOOTSTRAP", "GUI_LANGUAGE", ms().getGuiLanguageString());
					bootstrapini.SetInt("NDS-BOOTSTRAP", "LANGUAGE", perGameSettings_language == -2 ? ms().getGameLanguage() : perGameSettings_language);
					bootstrapini.SetInt("NDS-BOOTSTRAP", "REGION", perGameSettings_region < -1 ? ms().gameRegion : perGameSettings_region);
					bootstrapini.SetInt("NDS-BOOTSTRAP", "USE_ROM_REGION", perGameSettings_region < -1 ? ms().useRomRegion : 0);
					bootstrapini.SetInt("NDS-BOOTSTRAP", "DSI_MODE", true);
					bootstrapini.SetInt("NDS-BOOTSTRAP", "BOOST_CPU", true);
					bootstrapini.SetInt("NDS-BOOTSTRAP", "BOOST_VRAM", true);
					if (dsiFeatures() && ms().secondaryDevice && (!ms().dsiWareToSD || sys().arm7SCFGLocked())) {
						bootstrapini.SetInt("NDS-BOOTSTRAP", "CARD_READ_DMA", setCardReadDMA(ms().dsiWareSrlPath.c_str()));
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

					bool useNightly = (perGameSettings_bootstrapFile == -1 ? ms().bootstrapFile : perGameSettings_bootstrapFile);
					bool useWidescreen = (perGameSettings_wideScreen == -1 ? ms().wideScreen : perGameSettings_wideScreen);

					if (!isDSiMode() && (!ms().secondaryDevice || (ms().secondaryDevice && ms().dsiWareToSD && sdFound()))) {
						*(bool*)(0x02000010) = useNightly;
						*(bool*)(0x02000014) = useWidescreen;
					}
					if (isDSiMode() || !ms().secondaryDevice) {
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
					int err = runNdsFile(argarray[0], argarray.size(), (const char **)&argarray[0], true, true, false, true, true, false, -1);
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
					vector<char *> argarray;
					argarray.push_back((char*)(sys().isRunFromSD() ? "sd:/_nds/TWiLightMenu/r4menu.srldr" : "fat:/_nds/TWiLightMenu/r4menu.srldr"));
					runNdsFile(argarray[0], argarray.size(), (const char**)&argarray[0], true, false, false, true, true, false, -1);
					stop();
				}

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

				unlaunchRomBoot(ms().secondaryDevice ? "sdmc:/_nds/TWiLightMenu/tempDSiWare.dsi" : ms().dsiWareSrlPath.c_str());
			} else

			// Launch .nds directly or via nds-bootstrap
			if (extension(filename, {".nds", ".dsi", ".ids", ".srl", ".app"})) {
				remove(sys().isRunFromSD() ? "sd:/_nds/nds-bootstrap/esrb.bin" : "fat:/_nds/nds-bootstrap/esrb.bin");

				std::string typeToReplace = filename.substr(filename.rfind('.'));

				bool dsModeSwitch = false;
				bool dsModeDSiWare = false;

				char game_TID[5];

				FILE *f_nds_file = fopen(argarray[0], "rb");

				fseek(f_nds_file, offsetof(sNDSHeaderExt, gameCode), SEEK_SET);
				fread(game_TID, 1, 4, f_nds_file);
				fclose(f_nds_file);
				game_TID[4] = 0;

				loadPerGameSettings(filename);
				if (memcmp(game_TID, "HND", 3) == 0 || memcmp(game_TID, "HNE", 3) == 0) {
					dsModeSwitch = true;
					dsModeDSiWare = true;
					useBackend = false;	// Bypass nds-bootstrap
					ms().homebrewBootstrap = true;
				} else if (isHomebrew) {
					int pgsDSiMode = (perGameSettings_dsiMode == -1 ? isModernHomebrew : perGameSettings_dsiMode);
					if ((perGameSettings_directBoot && ms().secondaryDevice) || (isModernHomebrew && pgsDSiMode && (ms().secondaryDevice || perGameSettings_ramDiskNo == -1))) {
						useBackend = false;	// Bypass nds-bootstrap
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
				strcpy (filePath + pathLen, name);
				free(argarray.at(0));
				argarray.at(0) = filePath;
				if (useBackend) {
					if (((perGameSettings_useBootstrap == -1 ? ms().useBootstrap : perGameSettings_useBootstrap) || !ms().secondaryDevice) || (dsiFeatures() && romUnitCode > 0 && (perGameSettings_dsiMode == -1 ? DEFAULT_DSI_MODE : perGameSettings_dsiMode))
					|| (game_TID[0] == 'D' && romUnitCode == 3)) {
						std::string path = argarray[0];
						std::string savename = replaceAll(filename, typeToReplace, getSavExtension());
						std::string ramdiskname = replaceAll(filename, typeToReplace, getImgExtension(perGameSettings_ramDiskNo));
						std::string romFolderNoSlash = ms().romfolder[ms().secondaryDevice];
						RemoveTrailingSlashes(romFolderNoSlash);
						mkdir (isHomebrew ? "ramdisks" : "saves", 0777);
						std::string savepath = romFolderNoSlash+"/saves/"+savename;
						if (sdFound() && ms().secondaryDevice && ms().fcSaveOnSd) {
							savepath = replaceAll(savepath, "fat:/", "sd:/");
						}
						std::string ramdiskpath = romFolderNoSlash+"/ramdisks/"+ramdiskname;

						if (!isHomebrew) {
							// Create or expand save if game isn't homebrew
							u32 orgsavesize = getFileSize(savepath.c_str());
							u32 savesize = 524288;	// 512KB (default size)

							u32 gameTidHex = 0;
							tonccpy(&gameTidHex, &game_TID, 4);

							for (int i = 0; i < (int)sizeof(ROMList)/12; i++) {
								ROMListEntry* curentry = &ROMList[i];
								if (gameTidHex == curentry->GameCode) {
									if (curentry->SaveMemType != 0xFFFFFFFF) savesize = sramlen[curentry->SaveMemType];
									break;
								}
							}

							if ((orgsavesize == 0 && savesize > 0) || (orgsavesize < savesize)) {
								clearText();
								dialogboxHeight = 0;
								showdialogbox = true;
								printLargeCentered(false, 74, "Save management");
								printSmallCentered(false, 90, (orgsavesize == 0) ? "Creating save file..." : "Expanding save file...");

								FILE *pFile = fopen(savepath.c_str(), orgsavesize > 0 ? "r+" : "wb");
								if (pFile) {
									fseek(pFile, savesize - 1, SEEK_SET);
									fputc('\0', pFile);
									fclose(pFile);
								}
								clearText();
								printLargeCentered(false, 74, "Save management");
								printSmallCentered(false, 90, (orgsavesize == 0) ? "Save file created!" : "Save file expanded!");
								for (int i = 0; i < 30; i++) swiWaitForVBlank();
							}
						}

						SetMPUSettings();

						bool boostCpu = setClockSpeed(argarray[0]);
						bool useWidescreen = (perGameSettings_wideScreen == -1 ? ms().wideScreen : perGameSettings_wideScreen);

						const char *bootstrapinipath = (sys().isRunFromSD() ? BOOTSTRAP_INI : BOOTSTRAP_INI_FC);
						CIniFile bootstrapini( bootstrapinipath );
						bootstrapini.SetString("NDS-BOOTSTRAP", "NDS_PATH", path);
						bootstrapini.SetString("NDS-BOOTSTRAP", "SAV_PATH", savepath);
						if (!isHomebrew) {
							bootstrapini.SetString("NDS-BOOTSTRAP", "AP_FIX_PATH", setApFix(argarray[0]));
							bootstrapini.SetString("NDS-BOOTSTRAP", "MANUAL_PATH", getGameManual(filename.c_str()));
						}
						bootstrapini.SetString("NDS-BOOTSTRAP", "HOMEBREW_ARG", (useWidescreen && (game_TID[0] == 'W' || romVersion == 0x57)) ? "wide" : "");
						bootstrapini.SetString("NDS-BOOTSTRAP", "RAM_DRIVE_PATH", (perGameSettings_ramDiskNo >= 0 && !ms().secondaryDevice) ? ramdiskpath : "sd:/null.img");
						bootstrapini.SetString("NDS-BOOTSTRAP", "GUI_LANGUAGE", ms().getGuiLanguageString());
						bootstrapini.SetInt("NDS-BOOTSTRAP", "LANGUAGE", perGameSettings_language == -2 ? ms().getGameLanguage() : perGameSettings_language);
						bootstrapini.SetInt("NDS-BOOTSTRAP", "REGION", perGameSettings_region < -1 ? ms().gameRegion : perGameSettings_region);
						bootstrapini.SetInt("NDS-BOOTSTRAP", "USE_ROM_REGION", perGameSettings_region < -1 ? ms().useRomRegion : 0);
						bootstrapini.SetInt("NDS-BOOTSTRAP", "DSI_MODE", (dsModeForced || (romUnitCode == 0 && !isDSiMode())) ? 0 : (perGameSettings_dsiMode == -1 ? DEFAULT_DSI_MODE : perGameSettings_dsiMode));
						if (dsiFeatures() || !ms().secondaryDevice) {
							bootstrapini.SetInt("NDS-BOOTSTRAP", "BOOST_CPU", boostCpu);
							bootstrapini.SetInt("NDS-BOOTSTRAP", "BOOST_VRAM", perGameSettings_boostVram == -1 ? DEFAULT_DSI_MODE : perGameSettings_boostVram);
							bootstrapini.SetInt("NDS-BOOTSTRAP", "CARD_READ_DMA", setCardReadDMA(argarray[0]));
							bootstrapini.SetInt("NDS-BOOTSTRAP", "ASYNC_CARD_READ", setAsyncCardRead(argarray[0]));
						}
						bootstrapini.SetInt("NDS-BOOTSTRAP", "EXTENDED_MEMORY", perGameSettings_expandRomSpace == -1 ? ms().extendedMemory : perGameSettings_expandRomSpace);
						bootstrapini.SetInt("NDS-BOOTSTRAP", "DONOR_SDK_VER", SetDonorSDK(argarray[0]));
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
						bootstrapini.SaveIniFile( bootstrapinipath );

						CheatCodelist codelist;
						u32 gameCode,crc32;

						if (!isHomebrew) {
							bool cheatsEnabled = true;
							const char* cheatDataBin = "/_nds/nds-bootstrap/cheatData.bin";
							mkdir("/_nds", 0777);
							mkdir("/_nds/nds-bootstrap", 0777);
							if (codelist.romData(path,gameCode,crc32)) {
								long cheatOffset; size_t cheatSize;
								FILE* dat = fopen(sys().isRunFromSD() ? "sd:/_nds/TWiLightMenu/extras/usrcheat.dat" : "fat:/_nds/TWiLightMenu/extras/usrcheat.dat","rb");
								if (dat) {
									if (codelist.searchCheatData(dat, gameCode, crc32, cheatOffset, cheatSize)) {
										codelist.parse(path);
										codelist.writeCheatsToFile(cheatDataBin);
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
							ms().romPath[ms().secondaryDevice] = argarray[0];
						}
						ms().homebrewHasWide = (isHomebrew && (game_TID[0] == 'W' || romVersion == 0x57));
						ms().launchType[ms().secondaryDevice] = TWLSettings::ESDFlashcardLaunch;
						ms().previousUsedDevice = ms().secondaryDevice;
						ms().saveSettings();

						if (dsiFeatures() || !ms().secondaryDevice) {
							SetWidescreen(filename.c_str());
						}
						if (!isDSiMode() && !ms().secondaryDevice) {
							ntrStartSdGame();
						}

						bool useNightly = (perGameSettings_bootstrapFile == -1 ? ms().bootstrapFile : perGameSettings_bootstrapFile);

						char ndsToBoot[256];
						sprintf(ndsToBoot, "%s:/_nds/nds-bootstrap-%s%s.nds", sys().isRunFromSD() ? "sd" : "fat", ms().homebrewBootstrap ? "hb-" : "", useNightly ? "nightly" : "release");
						if (access(ndsToBoot, F_OK) != 0) {
							sprintf(ndsToBoot, "%s:/_nds/nds-bootstrap-%s%s.nds", sys().isRunFromSD() ? "fat" : "sd", ms().homebrewBootstrap ? "hb-" : "", useNightly ? "nightly" : "release");
						}

						argarray.at(0) = (char *)ndsToBoot;
						int err = runNdsFile (argarray[0], argarray.size(), (const char **)&argarray[0], (ms().homebrewBootstrap ? false : true), true, false, true, true, false, -1);
						char text[32];
						snprintf (text, sizeof(text), "Start failed. Error %i", err);
						clearText();
						dialogboxHeight = (err==1 ? 2 : 0);
						showdialogbox = true;
						printLargeCentered(false, 74, "Error!");
						printSmallCentered(false, 90, text);
						if (err == 1) {
							if (ms().homebrewBootstrap == true) {
								printSmallCentered(false, 4, 102, useNightly ? "nds-bootstrap for homebrew (Nightly)" : "nds-bootstrap for homebrew (Release)");
								printSmallCentered(false, 4, 114, "not found.");
							} else {
								printSmallCentered(false, 4, 102, useNightly ? "nds-bootstrap (Nightly)" : "nds-bootstrap (Release)");
								printSmallCentered(false, 4, 114, "not found.");
							}
						}
						printSmallCentered(false, (err==1 ? 132 : 108), "\u2428 Back");
						int pressed = 0;
						do {
							scanKeys();
							pressed = keysDownRepeat();
							checkSdEject();
							swiWaitForVBlank();
						} while (!(pressed & KEY_B));
						vector<char *> argarray;
						argarray.push_back((char*)(sys().isRunFromSD() ? "sd:/_nds/TWiLightMenu/r4menu.srldr" : "fat:/_nds/TWiLightMenu/r4menu.srldr"));
						runNdsFile(argarray[0], argarray.size(), (const char**)&argarray[0], true, false, false, true, true, false, -1);
					} else {
						ms().romPath[ms().secondaryDevice] = argarray[0];
						ms().launchType[ms().secondaryDevice] = TWLSettings::ESDFlashcardLaunch;
						ms().previousUsedDevice = ms().secondaryDevice;
						ms().saveSettings();
						loadGameOnFlashcard(argarray[0], true);
					}
				} else {
					if (!isArgv) {
						ms().romPath[ms().secondaryDevice] = argarray[0];
					}
					ms().homebrewHasWide = (isHomebrew && (game_TID[0] == 'W' || romVersion == 0x57));
					ms().launchType[ms().secondaryDevice] = TWLSettings::ESDFlashcardDirectLaunch;
					ms().previousUsedDevice = ms().secondaryDevice;
					if (isDSiMode() || !ms().secondaryDevice) {
						SetWidescreen(filename.c_str());
					}
					ms().saveSettings();

					if (!isDSiMode() && !ms().secondaryDevice && strncmp(filename.c_str(), "GodMode9i", 9) != 0 && strcmp(game_TID, "HGMA") != 0) {
						ntrStartSdGame();
					}

					int runNds_language = perGameSettings_language == -2 ? ms().getGameLanguage() : perGameSettings_language;
					int runNds_gameRegion = perGameSettings_region < -1 ? ms().gameRegion : perGameSettings_region;

					// Set region flag
					if (ms().useRomRegion && perGameSettings_region < -1 && game_TID[3] != 'A' && game_TID[3] != 'O' && game_TID[3] != '#') {
						if (game_TID[3] == 'J') {
							*(u8*)(0x02FFFD70) = 0;
						} else if (game_TID[3] == 'E' || game_TID[3] == 'T') {
							*(u8*)(0x02FFFD70) = 1;
						} else if (game_TID[3] == 'P' || game_TID[3] == 'V') {
							*(u8*)(0x02FFFD70) = 2;
						} else if (game_TID[3] == 'U') {
							*(u8*)(0x02FFFD70) = 3;
						} else if (game_TID[3] == 'C') {
							*(u8*)(0x02FFFD70) = 4;
						} else if (game_TID[3] == 'K') {
							*(u8*)(0x02FFFD70) = 5;
						}
					} else if (runNds_gameRegion == -1) {
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
						*(u8*)(0x02FFFD70) = runNds_gameRegion;
					}

					if (useTwlCfg && runNds_language >= 0 && runNds_language <= 7 && *(u8*)0x02000406 != runNds_language) {
						tonccpy((char*)0x02000600, (char*)0x02000400, 0x200);
						*(u8*)0x02000606 = runNds_language;
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
					int err = runNdsFile (argarray[0], argarray.size(), (const char **)&argarray[0], true, true, dsModeSwitch, runNds_boostCpu, runNds_boostVram, false, runNds_language);
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
					vector<char *> argarray;
					argarray.push_back((char*)(sys().isRunFromSD() ? "sd:/_nds/TWiLightMenu/r4menu.srldr" : "fat:/_nds/TWiLightMenu/r4menu.srldr"));
					runNdsFile(argarray[0], argarray.size(), (const char**)&argarray[0], true, false, false, true, true, false, -1);
				}
			} else {
				bool useNDSB = false;
				bool tgdsMode = false;
				bool dsModeSwitch = false;
				bool boostCpu = true;
				bool boostVram = false;
				bool tscTgds = false;

				std::string romfolderNoSlash = ms().romfolder[ms().secondaryDevice];
				RemoveTrailingSlashes(romfolderNoSlash);
				char ROMpath[256];
				snprintf (ROMpath, sizeof(ROMpath), "%s/%s", romfolderNoSlash.c_str(), filename.c_str());
				ms().romPath[ms().secondaryDevice] = ROMpath;
				ms().previousUsedDevice = ms().secondaryDevice;
				ms().homebrewBootstrap = true;

				const char *ndsToBoot = "sd:/_nds/nds-bootstrap-release.nds";
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
					ms().launchType[ms().secondaryDevice] = TWLSettings::ETunaViDSLaunch;

					ndsToBoot = "sd:/_nds/TWiLightMenu/apps/tuna-vids.nds";
					if (!isDSiMode() || access(ndsToBoot, F_OK) != 0) {
						ndsToBoot = "fat:/_nds/TWiLightMenu/apps/tuna-vids.nds";
						boostVram = true;
					}
				} else if (extension(filename, {".rvid"})) {
					ms().launchType[ms().secondaryDevice] = TWLSettings::ERVideoLaunch;

					ndsToBoot = "sd:/_nds/TWiLightMenu/apps/RocketVideoPlayer.nds";
					if (!isDSiMode() || access(ndsToBoot, F_OK) != 0) {
						ndsToBoot = "fat:/_nds/TWiLightMenu/apps/RocketVideoPlayer.nds";
						boostVram = true;
					}
				} else if (extension(filename, {".fv"})) {
					ms().launchType[ms().secondaryDevice] = TWLSettings::EFastVideoLaunch;

					ndsToBoot = "sd:/_nds/TWiLightMenu/apps/FastVideoDS.nds";
					if (!isDSiMode() || access(ndsToBoot, F_OK) != 0) {
						ndsToBoot = "fat:/_nds/TWiLightMenu/apps/FastVideoDS.nds";
						boostVram = true;
					}
				} else if (extension(filename, {".agb", ".gba", ".mb"})) {
					ms().launchType[ms().secondaryDevice] = (ms().gbaBooter == TWLSettings::EGbaNativeGbar2) ? TWLSettings::EGBANativeLaunch : TWLSettings::ESDFlashcardLaunch;

					if (ms().gbaBooter == TWLSettings::EGbaNativeGbar2) {
						clearText();
						dialogboxHeight = 0;
						showdialogbox = true;
						printLargeCentered(false, 74, "Game loading");
						printSmallCentered(false, 90, "Please wait...");

						u32 ptr = 0x08000000;
						u32 romSize = getFileSize(filename.c_str());
						char titleID[4];
						FILE* gbaFile = fopen(filename.c_str(), "rb");
						fseek(gbaFile, 0xAC, SEEK_SET);
						fread(&titleID, 1, 4, gbaFile);
						if (strncmp(titleID, "AGBJ", 4) == 0 && romSize <= 0x40000) {
							ptr += 0x400;
						}
						fseek(gbaFile, 0, SEEK_SET);

						extern char copyBuf[0x8000];
						if (romSize > 0x2000000) romSize = 0x2000000;

						bool nor = false;
						if (*(u16*)(0x020000C0) == 0x5A45 && strncmp(titleID, "AGBJ", 4) != 0) {
							cExpansion::SetRompage(0);
							expansion().SetRampage(cExpansion::ENorPage);
							cExpansion::OpenNorWrite();
							cExpansion::SetSerialMode();
							for (u32 address=0;address<romSize&&address<0x2000000;address+=0x40000) {
								expansion().Block_Erase(address);
							}
							nor = true;
						} else if (*(u16*)(0x020000C0) == 0x4353 && romSize > 0x1FFFFFE) {
							romSize = 0x1FFFFFE;
						}

						for (u32 len = romSize; len > 0; len -= 0x8000) {
							if (fread(&copyBuf, 1, (len>0x8000 ? 0x8000 : len), gbaFile) > 0) {
								s2RamAccess(true);
								if (nor) {
									expansion().WriteNorFlash(ptr-0x08000000, (u8*)copyBuf, (len>0x8000 ? 0x8000 : len));
								} else {
									tonccpy((u16*)ptr, &copyBuf, (len>0x8000 ? 0x8000 : len));
								}
								s2RamAccess(false);
								ptr += 0x8000;
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

						ndsToBoot = "fat:/_nds/TWiLightMenu/gbapatcher.srldr";
					} else if (ms().secondaryDevice) {
						ms().launchType[ms().secondaryDevice] = TWLSettings::EGBARunner2Launch;

						ndsToBoot = ms().gbar2DldiAccess ? "sd:/_nds/GBARunner2_arm7dldi_ds.nds" : "sd:/_nds/GBARunner2_arm9dldi_ds.nds";
						if (dsiFeatures()) {
							ndsToBoot = ms().consoleModel>0 ? "sd:/_nds/GBARunner2_arm7dldi_3ds.nds" : "sd:/_nds/GBARunner2_arm7dldi_dsi.nds";
						}
						if (!isDSiMode() || access(ndsToBoot, F_OK) != 0) {
							ndsToBoot = ms().gbar2DldiAccess ? "fat:/_nds/GBARunner2_arm7dldi_ds.nds" : "fat:/_nds/GBARunner2_arm9dldi_ds.nds";
							if (dsiFeatures()) {
								ndsToBoot = ms().consoleModel>0 ? "fat:/_nds/GBARunner2_arm7dldi_3ds.nds" : "fat:/_nds/GBARunner2_arm7dldi_dsi.nds";
							}
						}
						boostVram = false;
					} else {
						useNDSB = true;

						const char* gbar2Path = ms().consoleModel>0 ? "sd:/_nds/GBARunner2_arm7dldi_3ds.nds" : "sd:/_nds/GBARunner2_arm7dldi_dsi.nds";
						if (isDSiMode() && sys().arm7SCFGLocked() && !sys().dsiWramAccess()) {
							gbar2Path = ms().consoleModel>0 ? "sd:/_nds/GBARunner2_arm7dldi_nodsp_3ds.nds" : "sd:/_nds/GBARunner2_arm7dldi_nodsp_dsi.nds";
						}

						ndsToBoot = (ms().bootstrapFile ? "sd:/_nds/nds-bootstrap-hb-nightly.nds" : "sd:/_nds/nds-bootstrap-hb-release.nds");
						CIniFile bootstrapini(BOOTSTRAP_INI);

						bootstrapini.SetString("NDS-BOOTSTRAP", "GUI_LANGUAGE", ms().getGuiLanguageString());
						bootstrapini.SetInt("NDS-BOOTSTRAP", "LANGUAGE", ms().getGameLanguage());
						bootstrapini.SetInt("NDS-BOOTSTRAP", "DSI_MODE", 0);
						bootstrapini.SetString("NDS-BOOTSTRAP", "NDS_PATH", gbar2Path);
						bootstrapini.SetString("NDS-BOOTSTRAP", "HOMEBREW_ARG", ROMpath);
						bootstrapini.SetString("NDS-BOOTSTRAP", "RAM_DRIVE_PATH", "");
						bootstrapini.SetInt("NDS-BOOTSTRAP", "BOOST_CPU", 1);
						bootstrapini.SetInt("NDS-BOOTSTRAP", "BOOST_VRAM", 0);

						bootstrapini.SaveIniFile(BOOTSTRAP_INI);
					}
				} else if (extension(filename, {".xex", ".atr"})) {
					ms().launchType[ms().secondaryDevice] = TWLSettings::EXEGSDSLaunch;
					
					ndsToBoot = "sd:/_nds/TWiLightMenu/emulators/XEGS-DS.nds";
					if (!isDSiMode() || access(ndsToBoot, F_OK) != 0) {
						ndsToBoot = "fat:/_nds/TWiLightMenu/emulators/XEGS-DS.nds";
						boostVram = true;
					}
				} else if (extension(filename, {".a26"})) {
					ms().launchType[ms().secondaryDevice] = TWLSettings::EStellaDSLaunch;
					
					ndsToBoot = "sd:/_nds/TWiLightMenu/emulators/StellaDS.nds";
					if (!isDSiMode() || access(ndsToBoot, F_OK) != 0) {
						ndsToBoot = "fat:/_nds/TWiLightMenu/emulators/StellaDS.nds";
						boostVram = true;
					}
				} else if (extension(filename, {".a52"})) {
					ms().launchType[ms().secondaryDevice] = TWLSettings::EA5200DSLaunch;
					
					ndsToBoot = "sd:/_nds/TWiLightMenu/emulators/A5200DS.nds";
					if (!isDSiMode() || access(ndsToBoot, F_OK) != 0) {
						ndsToBoot = "fat:/_nds/TWiLightMenu/emulators/A5200DS.nds";
						boostVram = true;
					}
				} else if (extension(filename, {".a78"})) {
					ms().launchType[ms().secondaryDevice] = TWLSettings::EA7800DSLaunch;
					
					ndsToBoot = "sd:/_nds/TWiLightMenu/emulators/A7800DS.nds";
					if (!isDSiMode() || access(ndsToBoot, F_OK) != 0) {
						ndsToBoot = "fat:/_nds/TWiLightMenu/emulators/A7800DS.nds";
						boostVram = true;
					}
				} else if ((extension(filename, {".sg"}) && ms().sgEmulator == TWLSettings::EColSegaColecoDS) || (extension(filename, {".col"}) && ms().colEmulator == TWLSettings::EColSegaColecoDS) || extension(filename, {".m5"})) {
					ms().launchType[ms().secondaryDevice] = TWLSettings::EColecoDSLaunch;

					ndsToBoot = "sd:/_nds/TWiLightMenu/emulators/ColecoDS.nds";
					if (!isDSiMode() || access(ndsToBoot, F_OK) != 0) {
						ndsToBoot = "fat:/_nds/TWiLightMenu/emulators/ColecoDS.nds";
						boostVram = true;
					}
				} else if (extension(filename, {".int"})) {
					ms().launchType[ms().secondaryDevice] = TWLSettings::ENINTVDSLaunch;
					
					ndsToBoot = "sd:/_nds/TWiLightMenu/emulators/NINTV-DS.nds";
					if (!isDSiMode() || access(ndsToBoot, F_OK) != 0) {
						ndsToBoot = "fat:/_nds/TWiLightMenu/emulators/NINTV-DS.nds";
						boostVram = true;
					}
				} else if (extension(filename, {".gb", ".sgb", ".gbc"})) {
					ms().launchType[ms().secondaryDevice] = TWLSettings::EGameYobLaunch;
					
					ndsToBoot = "sd:/_nds/TWiLightMenu/emulators/gameyob.nds";
					if (!isDSiMode() || access(ndsToBoot, F_OK) != 0) {
						ndsToBoot = "fat:/_nds/TWiLightMenu/emulators/gameyob.nds";
						dsModeSwitch = !isDSiMode();
						boostVram = true;
					}
				} else if (extension(filename, {".nes", ".fds"})) {
					ms().launchType[ms().secondaryDevice] = TWLSettings::ENESDSLaunch;

					ndsToBoot = (ms().secondaryDevice ? "sd:/_nds/TWiLightMenu/emulators/nesds.nds" : "sd:/_nds/TWiLightMenu/emulators/nestwl.nds");
					if (!isDSiMode() || access(ndsToBoot, F_OK) != 0) {
						ndsToBoot = "fat:/_nds/TWiLightMenu/emulators/nesds.nds";
						boostVram = true;
					}
				} else if (extension(filename, {".sms", ".gg"})) {
					mkdir(ms().secondaryDevice ? "fat:/data" : "sd:/data", 0777);
					mkdir(ms().secondaryDevice ? "fat:/data/s8ds" : "sd:/data/s8ds", 0777);

					ms().launchType[ms().secondaryDevice] = TWLSettings::ES8DSLaunch;

					ndsToBoot = "sd:/_nds/TWiLightMenu/emulators/S8DS.nds";
					if (!isDSiMode() || access(ndsToBoot, F_OK) != 0) {
						ndsToBoot = "fat:/_nds/TWiLightMenu/emulators/S8DS.nds";
						boostVram = true;
					}
				} else if (extension(filename, {".gen"})) {
					bool usePicoDrive = ((isDSiMode() && sdFound() && sys().arm7SCFGLocked())
						|| ms().mdEmulator==2 || (ms().mdEmulator==3 && getFileSize(filename.c_str()) > 0x300000));
					ms().launchType[ms().secondaryDevice] = (usePicoDrive ? TWLSettings::EPicoDriveTWLLaunch : TWLSettings::ESDFlashcardLaunch);

					if (usePicoDrive || ms().secondaryDevice) {
						ndsToBoot = usePicoDrive ? "sd:/_nds/TWiLightMenu/emulators/PicoDriveTWL.nds" : (ms().macroMode ? "sd:/_nds/TWiLightMenu/emulators/jEnesisDS_macro.nds" : "sd:/_nds/TWiLightMenu/emulators/jEnesisDS.nds");
						if (!isDSiMode() || access(ndsToBoot, F_OK) != 0) {
							ndsToBoot = usePicoDrive ? "fat:/_nds/TWiLightMenu/emulators/PicoDriveTWL.nds" : (ms().macroMode ? "fat:/_nds/TWiLightMenu/emulators/jEnesisDS_macro.nds" : "fat:/_nds/TWiLightMenu/emulators/jEnesisDS.nds");
							boostVram = true;
						}
						dsModeSwitch = !usePicoDrive;
					} else {
						useNDSB = true;

						ndsToBoot = (ms().bootstrapFile ? "sd:/_nds/nds-bootstrap-hb-nightly.nds" : "sd:/_nds/nds-bootstrap-hb-release.nds");
						CIniFile bootstrapini(BOOTSTRAP_INI);

						bootstrapini.SetString("NDS-BOOTSTRAP", "GUI_LANGUAGE", ms().getGuiLanguageString());
						bootstrapini.SetInt("NDS-BOOTSTRAP", "LANGUAGE", ms().getGameLanguage());
						bootstrapini.SetInt("NDS-BOOTSTRAP", "DSI_MODE", 0);
						bootstrapini.SetString("NDS-BOOTSTRAP", "NDS_PATH", ms().macroMode ? "sd:/_nds/TWiLightMenu/emulators/jEnesisDS_macro.nds" : "sd:/_nds/TWiLightMenu/emulators/jEnesisDS.nds");
						bootstrapini.SetString("NDS-BOOTSTRAP", "HOMEBREW_ARG", "fat:/ROM.BIN");
						bootstrapini.SetInt("NDS-BOOTSTRAP", "BOOST_CPU", 1);
						bootstrapini.SetInt("NDS-BOOTSTRAP", "BOOST_VRAM", 0);

						bootstrapini.SetString("NDS-BOOTSTRAP", "RAM_DRIVE_PATH", ROMpath);
						bootstrapini.SaveIniFile(BOOTSTRAP_INI);
					}
				} else if (extension(filename, {".smc", ".sfc"})) {
					ms().launchType[ms().secondaryDevice] = (ms().newSnesEmuVer ? TWLSettings::ESNEmulDSLaunch : TWLSettings::ESDFlashcardLaunch);
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
						ndsToBoot = "sd:/_nds/TWiLightMenu/emulators/SNEmulDS-legacy.nds";
						if (!isDSiMode() || access(ndsToBoot, F_OK) != 0) {
							ndsToBoot = "fat:/_nds/TWiLightMenu/emulators/SNEmulDS-legacy.nds";
						}
						boostCpu = false;
						dsModeSwitch = true;
					} else {
						useNDSB = true;
						boostCpu = false;

						ndsToBoot = (ms().bootstrapFile ? "sd:/_nds/nds-bootstrap-hb-nightly.nds" : "sd:/_nds/nds-bootstrap-hb-release.nds");
						CIniFile bootstrapini(BOOTSTRAP_INI);

						bootstrapini.SetString("NDS-BOOTSTRAP", "GUI_LANGUAGE", ms().getGuiLanguageString());
						bootstrapini.SetInt("NDS-BOOTSTRAP", "LANGUAGE", ms().gameLanguage);
						bootstrapini.SetInt("NDS-BOOTSTRAP", "DSI_MODE", 0);
						bootstrapini.SetString("NDS-BOOTSTRAP", "NDS_PATH", "sd:/_nds/TWiLightMenu/emulators/SNEmulDS-legacy.nds");
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
						ms().launchType[ms().secondaryDevice] = TWLSettings::ESDFlashcardLaunch;

						useNDSB = true;

						ndsToBoot = (ms().bootstrapFile ? "sd:/_nds/nds-bootstrap-hb-nightly.nds" : "sd:/_nds/nds-bootstrap-hb-release.nds");
						CIniFile bootstrapini(BOOTSTRAP_INI);

						bootstrapini.SetString("NDS-BOOTSTRAP", "GUI_LANGUAGE", ms().getGuiLanguageString());
						bootstrapini.SetInt("NDS-BOOTSTRAP", "LANGUAGE", ms().getGameLanguage());
						bootstrapini.SetInt("NDS-BOOTSTRAP", "DSI_MODE", 0);
						bootstrapini.SetString("NDS-BOOTSTRAP", "NDS_PATH", "sd:/_nds/TWiLightMenu/emulators/NitroGrafx.nds");
						bootstrapini.SetString("NDS-BOOTSTRAP", "HOMEBREW_ARG", ROMpath);
						bootstrapini.SetInt("NDS-BOOTSTRAP", "BOOST_CPU", 1);
						bootstrapini.SetInt("NDS-BOOTSTRAP", "BOOST_VRAM", 0);

						bootstrapini.SetString("NDS-BOOTSTRAP", "RAM_DRIVE_PATH", "");
						bootstrapini.SaveIniFile(BOOTSTRAP_INI);
					} else {
						ms().launchType[ms().secondaryDevice] = TWLSettings::ENitroGrafxLaunch;

						ndsToBoot = "sd:/_nds/TWiLightMenu/emulators/NitroGrafx.nds";
						if (!isDSiMode() || access(ndsToBoot, F_OK) != 0) {
							ndsToBoot = "fat:/_nds/TWiLightMenu/emulators/NitroGrafx.nds";
							boostVram = true;
						}
					}
				} else if (extension(filename, {".ws", ".wsc"})) {
					mkdir(ms().secondaryDevice ? "fat:/data" : "sd:/data", 0777);
					mkdir(ms().secondaryDevice ? "fat:/data/nitroswan" : "sd:/data/nitroswan", 0777);

					ms().launchType[ms().secondaryDevice] = TWLSettings::ENitroSwanLaunch;

					ndsToBoot = "sd:/_nds/TWiLightMenu/emulators/NitroSwan.nds";
					if (!isDSiMode() || access(ndsToBoot, F_OK) != 0) {
						ndsToBoot = "fat:/_nds/TWiLightMenu/emulators/NitroSwan.nds";
						boostVram = true;
					}
				} else if (extension(filename, {".ngp", ".ngc"})) {
					mkdir(ms().secondaryDevice ? "fat:/data" : "sd:/data", 0777);
					mkdir(ms().secondaryDevice ? "fat:/data/ngpds" : "sd:/data/ngpds", 0777);

					ms().launchType[ms().secondaryDevice] = TWLSettings::ENGPDSLaunch;

					ndsToBoot = "sd:/_nds/TWiLightMenu/emulators/NGPDS.nds";
					if (!isDSiMode() || access(ndsToBoot, F_OK) != 0) {
						ndsToBoot = "fat:/_nds/TWiLightMenu/emulators/NGPDS.nds";
						boostVram = true;
					}
				} else if (extension(filename, {".dsk"}) && ms().cpcEmulator == TWLSettings::ECpcAmEDS) {
					ms().launchType[ms().secondaryDevice] = (ms().secondaryDevice ? TWLSettings::EAmEDSLaunch : TWLSettings::ESDFlashcardLaunch);

					if (ms().secondaryDevice) {
						ndsToBoot = "sd:/_nds/TWiLightMenu/emulators/AmEDS.nds";
						if (!isDSiMode() || access(ndsToBoot, F_OK) != 0) {
							ndsToBoot = "fat:/_nds/TWiLightMenu/emulators/AmEDS.nds";
							boostVram = true;
						}
					} else {
						useNDSB = true;

						ndsToBoot = (ms().bootstrapFile ? "sd:/_nds/nds-bootstrap-hb-nightly.nds" : "sd:/_nds/nds-bootstrap-hb-release.nds");
						CIniFile bootstrapini(BOOTSTRAP_INI);

						bootstrapini.SetString("NDS-BOOTSTRAP", "GUI_LANGUAGE", ms().getGuiLanguageString());
						bootstrapini.SetInt("NDS-BOOTSTRAP", "LANGUAGE", ms().gameLanguage);
						bootstrapini.SetInt("NDS-BOOTSTRAP", "DSI_MODE", 0);
						bootstrapini.SetString("NDS-BOOTSTRAP", "NDS_PATH", "sd:/_nds/TWiLightMenu/emulators/AmEDS.nds");
						bootstrapini.SetString("NDS-BOOTSTRAP", "HOMEBREW_ARG", ROMpath);
						bootstrapini.SetInt("NDS-BOOTSTRAP", "BOOST_CPU", 1);
						bootstrapini.SetInt("NDS-BOOTSTRAP", "BOOST_VRAM", 0);

						bootstrapini.SetString("NDS-BOOTSTRAP", "RAM_DRIVE_PATH", "");
						bootstrapini.SaveIniFile(BOOTSTRAP_INI);
					}
				} else if (extension(filename, {".dsk"}) && ms().cpcEmulator == TWLSettings::ECpcCrocoDS) {
					ms().launchType[ms().secondaryDevice] = (ms().secondaryDevice ? TWLSettings::ECrocoDSLaunch : TWLSettings::ESDFlashcardLaunch);

					if (ms().secondaryDevice) {
						ndsToBoot = "sd:/_nds/TWiLightMenu/emulators/CrocoDS.nds";
						if (!isDSiMode() || access(ndsToBoot, F_OK) != 0) {
							ndsToBoot = "fat:/_nds/TWiLightMenu/emulators/CrocoDS.nds";
							boostVram = true;
						}
					} else {
						useNDSB = true;

						ndsToBoot = (ms().bootstrapFile ? "sd:/_nds/nds-bootstrap-hb-nightly.nds" : "sd:/_nds/nds-bootstrap-hb-release.nds");
						CIniFile bootstrapini(BOOTSTRAP_INI);

						bootstrapini.SetString("NDS-BOOTSTRAP", "GUI_LANGUAGE", ms().getGuiLanguageString());
						bootstrapini.SetInt("NDS-BOOTSTRAP", "LANGUAGE", ms().gameLanguage);
						bootstrapini.SetInt("NDS-BOOTSTRAP", "DSI_MODE", 0);
						bootstrapini.SetString("NDS-BOOTSTRAP", "NDS_PATH", "sd:/_nds/TWiLightMenu/emulators/CrocoDS.nds");
						bootstrapini.SetString("NDS-BOOTSTRAP", "HOMEBREW_ARG", ROMpath);
						bootstrapini.SetInt("NDS-BOOTSTRAP", "BOOST_CPU", 1);
						bootstrapini.SetInt("NDS-BOOTSTRAP", "BOOST_VRAM", 0);

						bootstrapini.SetString("NDS-BOOTSTRAP", "RAM_DRIVE_PATH", "");
						bootstrapini.SaveIniFile(BOOTSTRAP_INI);
					}
				} else if (extension(filename, {".3ds", ".cia", ".cxi"})) {
					ms().launchType[ms().secondaryDevice] = TWLSettings::E3DSLaunch;

					ndsToBoot = sys().isRunFromSD() ? "sd:/_nds/TWiLightMenu/3dssplash.srldr" : "fat:/_nds/TWiLightMenu/3dssplash.srldr";
					if (!isDSiMode()) {
						boostVram = true;
					}
				} else if (extension(filename, {".gif", ".bmp", ".png"})) {
					ms().launchType[ms().secondaryDevice] = TWLSettings::EImageLaunch;

					ndsToBoot = sys().isRunFromSD() ? "sd:/_nds/TWiLightMenu/imageview.srldr" : "fat:/_nds/TWiLightMenu/imageview.srldr";
					if (!isDSiMode()) {
						boostVram = true;
					}
				}

				ms().homebrewArg[ms().secondaryDevice] = useNDSB ? "" : ms().romPath[ms().secondaryDevice];
				ms().saveSettings();

				if (!isDSiMode() && !ms().secondaryDevice && !extension(filename, {".plg", ".gif", ".bmp", ".png"})) {
					ntrStartSdGame();
				}

				if (tgdsMode && !ms().secondaryDevice) {
					std::string romfolderFat = replaceAll(romfolderNoSlash, "sd:", "fat:");
					snprintf (ROMpath, sizeof(ROMpath), "%s/%s", romfolderFat.c_str(), filename.c_str());
				}
				argarray.push_back(ROMpath);
				argarray.at(0) = (char *)(tgdsMode ? tgdsNdsPath : ndsToBoot);

				int err = runNdsFile (ndsToBoot, argarray.size(), (const char **)&argarray[0], !useNDSB, true, dsModeSwitch, boostCpu, boostVram, tscTgds, -1);	// Pass ROM to emulator as argument
				char text[32];
				snprintf (text, sizeof(text), "Start failed. Error %i", err);
				clearText();
				dialogboxHeight = (err==1 ? 2 : 0);
				showdialogbox = true;
				printLargeCentered(false, 74, "Error!");
				printSmallCentered(false, 90, text);
				if (err == 1 && useNDSB) {
					printSmallCentered(false, 4, 102, ms().bootstrapFile ? "nds-bootstrap for homebrew (Nightly)" : "nds-bootstrap for homebrew (Release)");
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
				vector<char *> argarray;
				argarray.push_back((char*)(sys().isRunFromSD() ? "sd:/_nds/TWiLightMenu/r4menu.srldr" : "fat:/_nds/TWiLightMenu/r4menu.srldr"));
				runNdsFile(argarray[0], argarray.size(), (const char**)&argarray[0], true, false, false, true, true, false, -1);
			}

			while (argarray.size() !=0 ) {
				free(argarray.at(0));
				argarray.erase(argarray.begin());
			}
		}
	}

	return 0;
}
