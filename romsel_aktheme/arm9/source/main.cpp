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
#include "esrbSplash.h"

#include "iconTitle.h"
#include "graphics/fontHandler.h"

#include "common/inifile.h"
#include "common/logging.h"
#include "common/bootstrapsettings.h"
#include "common/stringtool.h"
#include "common/systemdetails.h"
#include "common/twlmenusettings.h"

#include "language.h"

#include "cheat.h"
#include "crc.h"

#include "autoboot.h"	// For rebooting into the game

#include "colorLutBlacklist.h"
#include "twlClockExcludeMap.h"
#include "dmaExcludeMap.h"
#include "asyncReadExcludeMap.h"
#include "donorMap.h"
#include "saveMap.h"
#include "ROMList.h"

extern bool useTwlCfg;

bool fadeType = true;		// false = out, true = in
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

std::string iniPath;
std::string customIniPath;

int mpuregion = 0;
int mpusize = 0;

bool applaunch = false;
bool colorLutBlacklisted = false;
bool dsModeForced = false;

bool startMenu = false;

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

int cursorPosOnScreen = 0;

std::string startText = "START";
int startTextX = 0;
int startTextY = 0;
int startX = 4;
int startY = 172;
int startW = 48;
int startH = 18;

int brightnessX = 238;
int brightnessY = 172;
int brightnessW = 16;
int brightnessH = 16;

int folderUpX = 0;
int folderUpY = 2;
int folderUpW = 32;
int folderUpH = 16;

int folderTextX = 20;
int folderTextY = 2;

bool applaunchprep = false;

//char usernameRendered[10];
//bool usernameRenderedDone = false;

struct statvfs st[2];
bool gbaBiosFound[2] = {false};

touchPosition touch;

//---------------------------------------------------------------------------------
void stop (void) {
//---------------------------------------------------------------------------------
	while (1) {
		swiWaitForVBlank();
	}
}

/**
 * Disables DS Phat colors for a specific game.
 */
bool setDSPhatColors() {
	// TODO: If the list gets large enough, switch to bsearch().
	for (unsigned int i = 0; i < sizeof(colorLutBlacklist)/sizeof(colorLutBlacklist[0]); i++) {
		if (memcmp(gameTid[cursorPosOnScreen], colorLutBlacklist[i], 3) == 0) {
			// Found match
			colorLutBlacklisted = true;
			return false;
		}
	}

	return perGameSettings_dsPhatColors == -1 ? DEFAULT_PHAT_COLORS : perGameSettings_dsPhatColors;
}

/**
 * Disable TWL clock speed for a specific game.
 */
bool setClockSpeed(const bool phatColors) {
	if (!ms().ignoreBlacklists) {
		// TODO: If the list gets large enough, switch to bsearch().
		for (unsigned int i = 0; i < sizeof(twlClockExcludeList)/sizeof(twlClockExcludeList[0]); i++) {
			if (memcmp(gameTid[cursorPosOnScreen], twlClockExcludeList[i], 3) == 0) {
				// Found match
				dsModeForced = true;
				return false;
			}
		}
	}

	bool defaultSetting = DEFAULT_BOOST_CPU;
	if (perGameSettings_boostCpu == -1 && !colorLutBlacklisted && ((dsiFeatures() && !bs().b4dsMode) || !ms().secondaryDevice) && sys().dsiWramAccess() && !sys().dsiWramMirrored() && (colorTable || phatColors)) {
		defaultSetting = ms().boostCpuForClut;
	}

	return perGameSettings_boostCpu == -1 ? defaultSetting : perGameSettings_boostCpu;
}

/**
 * Disable card read DMA for a specific game.
 */
bool setCardReadDMA() {
	if (!ms().ignoreBlacklists) {
		// TODO: If the list gets large enough, switch to bsearch().
		for (unsigned int i = 0; i < sizeof(cardReadDMAExcludeList)/sizeof(cardReadDMAExcludeList[0]); i++) {
			if (memcmp(gameTid[cursorPosOnScreen], cardReadDMAExcludeList[i], 3) == 0) {
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
			if (memcmp(gameTid[cursorPosOnScreen], asyncReadExcludeList[i], 3) == 0) {
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
int SetDonorSDK() {
	char gameTid3[5];
	for (int i = 0; i < 3; i++) {
		gameTid3[i] = gameTid[cursorPosOnScreen][i];
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

sNDSHeader ndsCart;

/**
 * Enable widescreen for some games.
 */
void SetWidescreen(const char *filename) {
	const char* wideCheatDataPath = ms().secondaryDevice && (!isDSiWare[cursorPosOnScreen] || (isDSiWare[cursorPosOnScreen] && !ms().dsiWareToSD)) ? "fat:/_nds/nds-bootstrap/wideCheatData.bin" : "sd:/_nds/nds-bootstrap/wideCheatData.bin";
	remove(wideCheatDataPath);

	bool useWidescreen = (perGameSettings_wideScreen == -1 ? ms().wideScreen : perGameSettings_wideScreen);

	if ((isDSiMode() && sys().arm7SCFGLocked()) || ms().consoleModel < 2
	|| !useWidescreen || !widescreenFound || ms().macroMode) {
		return;
	}

	if (isHomebrew[cursorPosOnScreen] && ms().homebrewHasWide && widescreenFound) {
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

	char s1GameTid[5];
	u16 headerCRC16 = 0;

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
		headerCRC16 = ndsCart.headerCRC16;

		snprintf(wideBinPath, sizeof(wideBinPath), "sd:/_nds/TWiLightMenu/extras/widescreen/%s-%X.bin", s1GameTid, ndsCart.headerCRC16);
		wideCheatFound = (access(wideBinPath, F_OK) == 0);
	} else if (!wideCheatFound) {
		FILE *f_nds_file = fopen(filename, "rb");
		fseek(f_nds_file, offsetof(sNDSHeaderExt, headerCRC16), SEEK_SET);
		fread(&headerCRC16, sizeof(u16), 1, f_nds_file);
		fclose(f_nds_file);

		snprintf(wideBinPath, sizeof(wideBinPath), "sd:/_nds/TWiLightMenu/extras/widescreen/%s-%X.bin", gameTid[cursorPosOnScreen], headerCRC16);
		wideCheatFound = (access(wideBinPath, F_OK) == 0);
	}

	if (isHomebrew[cursorPosOnScreen]) {
		return;
	}

	mkdir(ms().secondaryDevice && (!isDSiWare[cursorPosOnScreen] || (isDSiWare[cursorPosOnScreen] && !ms().dsiWareToSD)) ? "fat:/_nds" : "sd:/_nds", 0777);
	mkdir(ms().secondaryDevice && (!isDSiWare[cursorPosOnScreen] || (isDSiWare[cursorPosOnScreen] && !ms().dsiWareToSD)) ? "fat:/_nds/nds-bootstrap" : "sd:/_nds/nds-bootstrap", 0777);

	if (wideCheatFound) {
		if (fcopy(wideBinPath, wideCheatDataPath) != 0) {
			const char* resultText1 = "Failed to copy widescreen";
			const char* resultText2 = "code for the game.";
			remove(wideCheatDataPath);
			int textYpos[2] = {0};
			textYpos[0] = 72;
			textYpos[1] = 84;
			clearText(false);
			dialogboxHeight = 2;
			showdialogbox = true;
			printSmall(false, 0, 74, "Error!", Alignment::center, FontPalette::formTitleText);
			printSmall(false, 0, textYpos[0], resultText1, Alignment::center, FontPalette::formText);
			printSmall(false, 0, textYpos[1], resultText2, Alignment::center, FontPalette::formText);
			updateText(false);
			for (int i = 0; i < 60 * 3; i++) {
				swiWaitForVBlank(); // Wait 3 seconds
			}
			showdialogbox = false;
			clearText(false);
			updateText(false);
			return;
		}
	} else {
		char *tid = ms().slot1Launched ? s1GameTid : gameTid[cursorPosOnScreen];
		u16 crc16 = ms().slot1Launched ? ndsCart.headerCRC16 : headerCRC16;

		displayDiskIcon(!sys().isRunFromSD());
		FILE *file = fopen(sys().isRunFromSD() ? "sd:/_nds/TWiLightMenu/extras/widescreen.pck" : "fat:/_nds/TWiLightMenu/extras/widescreen.pck", "rb");
		if (file) {
			char buf[5] = {0};
			fread(buf, 1, 4, file);
			if (strcmp(buf, ".PCK") != 0) // Invalid file
				return;

			u32 fileCount;
			fread(&fileCount, 1, sizeof(fileCount), file);

			u32 offset = 0, size = 0;
			u32 offsetAlt = 0, sizeAlt = 0;

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
					} else if (crc == 0xFFFF) {
						fread(&offsetAlt, 1, sizeof(offsetAlt), file);
						fread(&sizeAlt, 1, sizeof(sizeAlt), file);
					}

					if (crc < headerCRC16) {
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

			if (offsetAlt > 0 && offset == 0)
			{
				offset = offsetAlt;
				size = sizeAlt;
				wideCheatFound = true;
			}

			if (offset > 0) {
				fseek(file, offset, SEEK_SET);
				u8 *buffer = new u8[size];
				fread(buffer, 1, size, file);

				snprintf(wideBinPath, sizeof(wideBinPath), "%s:/_nds/nds-bootstrap/wideCheatData.bin", ms().secondaryDevice && (!isDSiWare[cursorPosOnScreen] || (isDSiWare[cursorPosOnScreen] && !ms().dsiWareToSD)) ? "fat" : "sd");
				FILE *out = fopen(wideBinPath, "wb");
				if (out) {
					fwrite(buffer, 1, size, out);
					fclose(out);
				}
				delete[] buffer;
			}

			fclose(file);
		}
		displayDiskIcon(false);
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

	snprintf(manualPath, sizeof(manualPath), "%s:/_nds/TWiLightMenu/extras/manuals/%s.txt", sys().isRunFromSD() ? "sd" : "fat", gameTid[cursorPosOnScreen]);
	if (access(manualPath, F_OK) == 0)
		return manualPath;

	snprintf(manualPath, sizeof(manualPath), "%s:/_nds/TWiLightMenu/extras/manuals/%.3s.txt", sys().isRunFromSD() ? "sd" : "fat", gameTid[cursorPosOnScreen]);
	if (access(manualPath, F_OK) == 0)
		return manualPath;

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

void loadGameOnFlashcard(const char* ndsPath, bool dsGame) {
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
	if ((memcmp(io_dldi_data->friendlyName, "R4iDSN", 6) == 0)
	 || (memcmp(io_dldi_data->friendlyName, "R4iTT", 5) == 0)
	 || (memcmp(io_dldi_data->friendlyName, "Acekard AK2", 11) == 0)
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
	} else if (memcmp(io_dldi_data->friendlyName, "DSTWO(Slot-1)", 13) == 0) {
		CIniFile fcrompathini("fat:/_dstwo/autoboot.ini");
		fcPath = replaceAll(ndsPath, "fat:/", dstwofat);
		fcrompathini.SetString("Dir Info", "fullName", fcPath);
		fcrompathini.SaveIniFile("fat:/_dstwo/autoboot.ini");
		err = runNdsFile("fat:/_dstwo/autoboot.nds", 0, NULL, sys().isRunFromSD(), true, true, true, runNds_boostCpu, runNds_boostVram, false, -1);
	} else if ((memcmp(io_dldi_data->friendlyName, "TTCARD", 6) == 0)
			|| (memcmp(io_dldi_data->friendlyName, "DSTT", 4) == 0)
			|| (memcmp(io_dldi_data->friendlyName, "DEMON", 5) == 0)
			|| (memcmp(io_dldi_data->friendlyName, "R4(DS) - Revolution for DS", 26) == 0)
			|| (memcmp(io_dldi_data->friendlyName, "R4TF", 4) == 0)
			|| (memcmp(io_dldi_data->friendlyName, "DSONE", 5) == 0)
			|| (memcmp(io_dldi_data->friendlyName, "M3DS", 4) == 0)
			|| (memcmp(io_dldi_data->friendlyName, "M3-DS", 5) == 0)) {
		CIniFile fcrompathini("fat:/TTMenu/YSMenu.ini");
		fcPath = replaceAll(ndsPath, "fat:/", slashchar);
		fcrompathini.SetString("YSMENU", "AUTO_BOOT", fcPath);
		fcrompathini.SaveIniFile("fat:/TTMenu/YSMenu.ini");
		err = runNdsFile("fat:/YSMenu.nds", 0, NULL, sys().isRunFromSD(), true, true, true, runNds_boostCpu, runNds_boostVram, false, -1);
	}

	char text[32];
	snprintf (text, sizeof(text), "Start failed. Error %i", err);
	clearText(false);
	dialogboxHeight = (err==0 ? 2 : 0);
	showdialogbox = true;
	printSmall(false, 0, 74, "Error!", Alignment::center, FontPalette::formTitleText);
	if (err == 0) {
		printSmall(false, 0, 90, "Flashcard may be unsupported.", Alignment::center, FontPalette::formText);
		printSmall(false, 0, 102, "Flashcard name:", Alignment::center, FontPalette::formText);
		printSmall(false, 0, 114, io_dldi_data->friendlyName, Alignment::center, FontPalette::formText);
	} else {
		printSmall(false, 0, 90, text, Alignment::center, FontPalette::formText);
	}
	printSmall(false, 0, (err==0 ? 132 : 108), " Back", Alignment::center, FontPalette::formText);
	updateText(false);
	int pressed = 0;
	do {
		scanKeys();
		pressed = keysDown();
		checkSdEject();
		swiWaitForVBlank();
	} while (!(pressed & KEY_B));
	vector<char *> argarray;
	argarray.push_back((char*)(sys().isRunFromSD() ? "sd:/_nds/TWiLightMenu/akmenu.srldr" : "fat:/_nds/TWiLightMenu/akmenu.srldr"));
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
		displayDiskIcon(ms().secondaryDevice);
		fwrite(&h, sizeof(FATHeader), 1, file); // Write header
		fseek(file, size - 1, SEEK_SET); // Pad rest of the file
		fputc('\0', file);
		fclose(file);
		displayDiskIcon(false);
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
	drawTime();
	drawYear();
	drawMonth();
	drawDayX();
	drawDay();
	drawWeekday();
	if (waitFrame) {
		swiWaitForVBlank();
	}
}

//---------------------------------------------------------------------------------
int akTheme(void) {
//---------------------------------------------------------------------------------
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
	logInit();

	graphicsInit();
	langInit();

	if (sdFound() && ms().consoleModel >= 2 && !sys().arm7SCFGLocked()) {
		CIniFile lumaConfig("sd:/luma/config.ini");
		widescreenFound = ((access("sd:/_nds/TWiLightMenu/TwlBg/Widescreen.cxi", F_OK) == 0) && (lumaConfig.GetInt("boot", "enable_external_firm_and_modules", 0) == true));
		logPrint(widescreenFound ? "Widescreen found\n" : "Widescreen not found\n");
	}

	logPrint("\n");

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

	const bool emulatorsInstalled = (access(sys().isRunFromSD() ? "sd:/_nds/TWiLightMenu/addons/Virtual Console" : "fat:/_nds/TWiLightMenu/addons/Virtual Console", F_OK) == 0);
	const bool multimediaInstalled = (access(sys().isRunFromSD() ? "sd:/_nds/TWiLightMenu/addons/Multimedia" : "fat:/_nds/TWiLightMenu/addons/Multimedia", F_OK) == 0);

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

	srand(time(NULL));

	graphicsLoad();
	fontInit();

	{
		CIniFile ini( iniPath.c_str() );
		startText = ini.GetString("start button", "text", startText);
		if (startText == "ini") {
			startText = "START";
		}
		startX = ini.GetInt("start button", "x", startX);
		startY = ini.GetInt("start button", "y", startY);
		startW = ini.GetInt("start button", "w", startW);
		startH = ini.GetInt("start button", "h", startH);

		displayStartButton(startX, startY);

		startTextX = startX + startW/5;
		startTextY = startY + startH/5;

		if (sys().isRegularDS() || (dsiFeatures() && !sys().i2cBricked() && ms().consoleModel < 2)) {
			brightnessX = ini.GetInt("brightness btn", "x", brightnessX);
			brightnessY = ini.GetInt("brightness btn", "y", brightnessY);
			brightnessW = ini.GetInt("brightness btn", "w", brightnessW);
			brightnessH = ini.GetInt("brightness btn", "h", brightnessH);

			displayBrightnessBtn(brightnessX, brightnessY);
		}

		if (ms().showDirectories) {
			folderUpX = ini.GetInt("folderup btn", "x", folderUpX);
			folderUpY = ini.GetInt("folderup btn", "y", folderUpY);
			folderUpW = ini.GetInt("folderup btn", "w", folderUpW);
			folderUpH = ini.GetInt("folderup btn", "h", folderUpH);

			displayFolderUp(folderUpX, folderUpY);
		}

		folderTextX = ini.GetInt("folder text", "x", folderTextX);
		folderTextY = ini.GetInt("folder text", "y", folderTextY);
	}

	if (!ms().macroMode) {
		CIniFile ini( customIniPath.c_str() );
		if (ini.GetInt("user name", "show", 0)) {
			const int x = ini.GetInt("user name", "x", 0);
			const int y = ini.GetInt("user name", "y", 0);

			char16_t username[11] = {0};
			tonccpy(username, useTwlCfg ? (s16 *)0x02000448 : PersonalData->name, 10 * sizeof(char16_t));

			printSmall(true, x, y, username, Alignment::left, FontPalette::usernameText);
			updateText(true);
		}

		if (ini.GetInt("custom text", "show", 0)) {
			const int x = ini.GetInt("custom text", "x", 0);
			const int y = ini.GetInt("custom text", "y", 0);
			const int w = ini.GetInt("custom text", "w", 0);

			const std::string userText = ini.GetString("custom text", "text", "");

			std::vector<std::string> lines;
			lines.push_back(userText);

			for (uint i = 0; i < lines.size(); i++) {
				int width = calcSmallFontWidth(lines[i]);
				if (width > w) {
					int mid = lines[i].length() / 2;
					bool foundSpace = false;
					for (uint j = 0; j < lines[i].length() / 2; j++) {
						if (lines[i][mid + j] == ' ') {
							lines.insert(lines.begin() + i, lines[i].substr(0, mid + j));
							lines[i + 1] = lines[i + 1].substr(mid + j + 1);
							i--;
							foundSpace = true;
							break;
						} else if (lines[i][mid - j] == ' ') {
							lines.insert(lines.begin() + i, lines[i].substr(0, mid - j));
							lines[i + 1] = lines[i + 1].substr(mid - j + 1);
							i--;
							foundSpace = true;
							break;
						}
					}
					if (!foundSpace) {
						lines.insert(lines.begin() + i, lines[i].substr(0, mid));
						lines[i + 1] = lines[i + 1].substr(mid);
						i--;
					}
				}
			}

			std::string out;
			for (auto line : lines) {
				out += line + '\n';
			}
			out.pop_back();

			printSmall(true, x, y, out, Alignment::left, FontPalette::customText);
			updateText(true);
		}
	}

	iconTitleInit();

	char path[256];

	const bool copyDSiWareSavBack =
	   (ms().previousUsedDevice && bothSDandFlashcard() && ms().launchType[ms().previousUsedDevice] == 3
	&& ((access(ms().dsiWarePubPath.c_str(), F_OK) == 0 && access("sd:/_nds/TWiLightMenu/tempDSiWare.pub", F_OK) == 0)
	 || (access(ms().dsiWarePrvPath.c_str(), F_OK) == 0 && access("sd:/_nds/TWiLightMenu/tempDSiWare.prv", F_OK) == 0)));

	if (copyDSiWareSavBack) {
		dialogboxHeight = 2;
		showdialogbox = true;
		printSmall(false, 0, 74, "Please wait", Alignment::center, FontPalette::formTitleText);
		printSmall(false, 0, 90, "Now copying data...", Alignment::center, FontPalette::formText);
		printSmall(false, 0, 102, "Do not turn off the power.", Alignment::center, FontPalette::formText);
		updateText(false);
		if (access(ms().dsiWarePubPath.c_str(), F_OK) == 0) {
			fcopy("sd:/_nds/TWiLightMenu/tempDSiWare.pub", ms().dsiWarePubPath.c_str());
			rename("sd:/_nds/TWiLightMenu/tempDSiWare.pub", "sd:/_nds/TWiLightMenu/tempDSiWare.pub.bak");
		}
		if (access(ms().dsiWarePrvPath.c_str(), F_OK) == 0) {
			fcopy("sd:/_nds/TWiLightMenu/tempDSiWare.prv", ms().dsiWarePrvPath.c_str());
			rename("sd:/_nds/TWiLightMenu/tempDSiWare.prv", "sd:/_nds/TWiLightMenu/tempDSiWare.prv.bak");
		}
		logPrint("Copied DSiWare save back to flashcard\n");
		clearText(false);
		showdialogbox = true;
		dialogboxHeight = 0;
		updateText(false);
	}

	while (1) {
		if (startMenu) {
			// Launch DS Classic Menu
			fadeType = false;	// Fade to white
			for (int i = 0; i < 25; i++) {
				swiWaitForVBlank();
			}

			vector<char *> argarray;
			argarray.push_back((char*)(sys().isRunFromSD() ? "sd:/_nds/TWiLightMenu/mainmenu.srldr" : "fat:/_nds/TWiLightMenu/mainmenu.srldr"));
			runNdsFile(argarray[0], argarray.size(), (const char**)&argarray[0], sys().isRunFromSD(), true, false, false, true, true, false, -1);
			stop();
		} else {
			std::vector<std::string_view> extensionList = {
				".nds", ".dsi", ".ids", ".srl", ".app", ".argv", // NDS
				".agb", ".gba", ".mb" // GBA
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

			if (memcmp(io_dldi_data->friendlyName, "DSTWO(Slot-1)", 0xD) == 0) {
				extensionList.emplace_back(".plg"); // DSTWO Plugin
			}

			if (emulatorsInstalled) {
				std::vector<std::string_view> extensionListEmus = {
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
					".min" // Pokémon mini
				};

				for (int i = 0; i < 28; i++) {
					extensionList.emplace_back(extensionListEmus[i]);
				}

				if (!ms().secondaryDevice || ms().mdEmulator == 2) {
					extensionList.emplace_back(".md"); // Sega Mega Drive
				}
			}

			if (multimediaInstalled) {
				std::vector<std::string_view> extensionListMedia = {
					".avi", // Xvid (AVI)
					".rvid", // Rocket Video
					".fv", // FastVideo
					".gif", // GIF
					".bmp", // BMP
					".png" // Portable Network Graphics
				};

				for (int i = 0; i < 5; i++) {
					extensionList.emplace_back(extensionListMedia[i]);
				}
			}

			if(ms().blockedExtensions.size() > 0) {
				auto toErase = std::remove_if(extensionList.begin(), extensionList.end(), [](std::string_view str) {
					return std::find(ms().blockedExtensions.begin(), ms().blockedExtensions.end(), str) != ms().blockedExtensions.end();
				});
				extensionList.erase(toErase, extensionList.end());
			}

			snprintf (path, sizeof(path), "%s", ms().romfolder[ms().secondaryDevice].c_str());
			// Set directory
			chdir (path);

			//Navigates to the file to launch
			filename = browseForFile(extensionList);

			extensionList.clear();
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
			if (isDSiWare[cursorPosOnScreen]) {
				remove(sys().isRunFromSD() ? "sd:/_nds/nds-bootstrap/esrb.bin" : "fat:/_nds/nds-bootstrap/esrb.bin");

				std::string typeToReplace = filename.substr(filename.rfind('.'));

				char *name = argarray.at(0);
				strcpy (filePath + pathLen, name);
				free(argarray.at(0));
				argarray.at(0) = filePath;

				std::string romFolderNoSlash = ms().romfolder[ms().secondaryDevice];
				RemoveTrailingSlashes(romFolderNoSlash);

				sNDSHeaderExt NDSHeader;

				displayDiskIcon(ms().secondaryDevice);
				FILE *f_nds_file = fopen(filename.c_str(), "rb");

				fread(&NDSHeader, 1, sizeof(NDSHeader), f_nds_file);
				fclose(f_nds_file);
				displayDiskIcon(false);

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
					ms().romPath[ms().secondaryDevice] = argarray[0];
				}
				ms().homebrewBootstrap = isHomebrew[cursorPosOnScreen];
				ms().launchType[ms().secondaryDevice] = TWLSettings::EDSiWareLaunch;
				ms().previousUsedDevice = ms().secondaryDevice;
				displayDiskIcon(!sys().isRunFromSD());
				ms().saveSettings();
				displayDiskIcon(false);

				if (savFormat) {
					if ((getFileSize(ms().dsiWarePubPath.c_str()) == 0) && ((NDSHeader.pubSavSize > 0) || (NDSHeader.prvSavSize > 0))) {
						clearText(false);
						dialogboxHeight = 0;
						showdialogbox = true;
						printSmall(false, 0, 74, "Save creation", Alignment::center, FontPalette::formTitleText);
						printSmall(false, 0, 98, "Creating save file...", Alignment::center, FontPalette::formText);
						updateText(false);

						FILE *pFile = fopen(ms().dsiWarePubPath.c_str(), "wb");
						if (pFile) {
							u32 savesize = ((NDSHeader.pubSavSize > 0) ? NDSHeader.pubSavSize : NDSHeader.prvSavSize);
							fseek(pFile, savesize - 1, SEEK_SET);
							fputc('\0', pFile);
							fclose(pFile);
						}

						clearText(false);
						printSmall(false, 0, 74, "Save creation", Alignment::center, FontPalette::formTitleText);
						printSmall(false, 0, 98, "Save file created!", Alignment::center, FontPalette::formText);
						updateText(false);
						for (int i = 0; i < 60; i++) swiWaitForVBlank();
					}
				} else {
					if ((getFileSize(ms().dsiWarePubPath.c_str()) == 0) && (NDSHeader.pubSavSize > 0)) {
						clearText(false);
						dialogboxHeight = 0;
						showdialogbox = true;
						printSmall(false, 0, 74, "Save creation", Alignment::center, FontPalette::formTitleText);
						printSmall(false, 0, 98, "Creating public save file...", Alignment::center, FontPalette::formText);
						updateText(false);

						createDSiWareSave(ms().dsiWarePubPath.c_str(), NDSHeader.pubSavSize);

						clearText(false);
						printSmall(false, 0, 74, "Save creation", Alignment::center, FontPalette::formTitleText);
						printSmall(false, 0, 98, "Public save file created!", Alignment::center, FontPalette::formText);
						updateText(false);
						for (int i = 0; i < 60; i++) swiWaitForVBlank();
					}

					if ((getFileSize(ms().dsiWarePrvPath.c_str()) == 0) && (NDSHeader.prvSavSize > 0)) {
						clearText(false);
						dialogboxHeight = 0;
						showdialogbox = true;
						printSmall(false, 0, 74, "Save creation", Alignment::center, FontPalette::formTitleText);
						printSmall(false, 0, 98, "Creating private save file...", Alignment::center, FontPalette::formText);
						updateText(false);

						createDSiWareSave(ms().dsiWarePrvPath.c_str(), NDSHeader.prvSavSize);

						clearText(false);
						printSmall(false, 0, 74, "Save creation", Alignment::center, FontPalette::formTitleText);
						printSmall(false, 0, 98, "Private save file created!", Alignment::center, FontPalette::formText);
						updateText(false);
						for (int i = 0; i < 60; i++) swiWaitForVBlank();
					}
				}

				loadPerGameSettings(filename);

				if (ms().secondaryDevice && !bs().b4dsMode && (ms().dsiWareToSD || (!(perGameSettings_dsiwareBooter == -1 ? ms().dsiWareBooter : perGameSettings_dsiwareBooter) && ms().consoleModel == 0)) && sdFound()) {
					clearText(false);
					dialogboxHeight = 0;
					showdialogbox = true;
					printSmall(false, 0, 74, "Please wait", Alignment::center, FontPalette::formTitleText);
					printSmall(false, 0, 98, "Now copying data...", Alignment::center, FontPalette::formText);
					printSmall(false, 0, 110, "Do not turn off the power.", Alignment::center, FontPalette::formText);
					updateText(false);
					fcopy(ms().dsiWareSrlPath.c_str(), "sd:/_nds/TWiLightMenu/tempDSiWare.dsi");
					if ((access(ms().dsiWarePubPath.c_str(), F_OK) == 0) && (NDSHeader.pubSavSize > 0)) {
						fcopy(ms().dsiWarePubPath.c_str(), "sd:/_nds/TWiLightMenu/tempDSiWare.pub");
					}
					if ((access(ms().dsiWarePrvPath.c_str(), F_OK) == 0) && (NDSHeader.prvSavSize > 0)) {
						fcopy(ms().dsiWarePrvPath.c_str(), "sd:/_nds/TWiLightMenu/tempDSiWare.prv");
					}

					if ((access(ms().dsiWarePubPath.c_str(), F_OK) == 0 && (NDSHeader.pubSavSize > 0))
					 || (access(ms().dsiWarePrvPath.c_str(), F_OK) == 0 && (NDSHeader.prvSavSize > 0))) {
						dialogboxHeight = 1;
						clearText(false);
						printSmall(false, 0, 74, "Important!", Alignment::center, FontPalette::formTitleText);
						printSmall(false, 0, 90, "After saving, please re-start", Alignment::center, FontPalette::formText);
						printSmall(false, 0, 102, "TWiLight Menu++ to transfer your", Alignment::center, FontPalette::formText);
						printSmall(false, 0, 114, "save data back.", Alignment::center, FontPalette::formText);
						updateText(false);
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
								loadPerGameSettings(ms().dsiWareSrlPath.substr(ms().dsiWareSrlPath.find_last_of('/') + 1));
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
					bootstrapini.SetString("NDS-BOOTSTRAP", "MANUAL_PATH", getGameManual(filename.c_str()));
					bootstrapini.SetString("NDS-BOOTSTRAP", "GUI_LANGUAGE", ms().getGuiLanguageString());
					bootstrapini.SetInt("NDS-BOOTSTRAP", "PHAT_COLORS", setDSPhatColors());
					bootstrapini.SetInt("NDS-BOOTSTRAP", "LANGUAGE", perGameSettings_language == -2 ? ms().getGameLanguage() : perGameSettings_language);
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
					bootstrapini.SetInt("NDS-BOOTSTRAP", "FORCE_SLEEP_PATCH", ms().forceSleepPatch);
					displayDiskIcon(!sdFound());
					bootstrapini.SaveIniFile(bootstrapinipath);
					displayDiskIcon(false);

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
					int err = runNdsFile(argarray[0], argarray.size(), (const char **)&argarray[0], sys().isRunFromSD(), true, true, false, true, true, false, -1);
					char text[32];
					snprintf (text, sizeof(text), "Start failed. Error %i", err);
					clearText(false);
					dialogboxHeight = (err==1 ? 2 : 0);
					showdialogbox = true;
					printSmall(false, 0, 74, "Error!", Alignment::center, FontPalette::formTitleText);
					printSmall(false, 0, 90, text, Alignment::center, FontPalette::formText);
					if (err == 1) {
						printSmall(false, 4, 102, useNightly ? "nds-bootstrap (Nightly)" : "nds-bootstrap (Release)", Alignment::center, FontPalette::formText);
						printSmall(false, 4, 114, "not found.", Alignment::center, FontPalette::formText);
					}
					printSmall(false, 0, (err==1 ? 132 : 108), " Back", Alignment::center, FontPalette::formText);
					updateText(false);
					int pressed = 0;
					do {
						scanKeys();
						pressed = keysDownRepeat();
						checkSdEject();
						swiWaitForVBlank();
					} while (!(pressed & KEY_B));
					vector<char *> argarray;
					argarray.push_back((char*)(sys().isRunFromSD() ? "sd:/_nds/TWiLightMenu/akmenu.srldr" : "fat:/_nds/TWiLightMenu/akmenu.srldr"));
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

				unlaunchRomBoot(ms().secondaryDevice ? "sdmc:/_nds/TWiLightMenu/tempDSiWare.dsi" : ms().dsiWareSrlPath.c_str());
			} else

			// Launch .nds directly or via nds-bootstrap
			if (extension(filename, {".nds", ".dsi", ".ids", ".srl", ".app"})) {
				std::string typeToReplace = filename.substr(filename.rfind('.'));

				bool dsModeSwitch = false;
				bool dsModeDSiWare = false;

				loadPerGameSettings(filename);
				if (!dsiFeatures() && (memcmp(gameTid[cursorPosOnScreen], "HND", 3) == 0 || memcmp(gameTid[cursorPosOnScreen], "HNE", 3) == 0)) {
					dsModeSwitch = true;
					dsModeDSiWare = true;
					useBackend = false;	// Bypass nds-bootstrap
					ms().homebrewBootstrap = true;
				} else if (isHomebrew[cursorPosOnScreen]) {
					int pgsDSiMode = (perGameSettings_dsiMode == -1 ? isModernHomebrew[cursorPosOnScreen] : perGameSettings_dsiMode);
					if ((perGameSettings_directBoot && ms().secondaryDevice) || (isModernHomebrew[cursorPosOnScreen] && pgsDSiMode && (ms().secondaryDevice || perGameSettings_ramDiskNo == -1))) {
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
				if (!ms().secondaryDevice && !sys().arm7SCFGLocked() && ms().consoleModel == TWLSettings::EDSiRetail && isHomebrew[cursorPosOnScreen] && !(perGameSettings_useBootstrap == -1 ? true : perGameSettings_useBootstrap)) {
					ms().romPath[ms().secondaryDevice] = std::string(argarray[0]);
					ms().launchType[ms().secondaryDevice] = TWLSettings::ESDFlashcardLaunch;
					ms().previousUsedDevice = ms().secondaryDevice;
					displayDiskIcon(!sys().isRunFromSD());
					ms().saveSettings();
					displayDiskIcon(false);

					unlaunchRomBoot(argarray[0]);
				} else if (useBackend) {
					if ((((perGameSettings_useBootstrap == -1 ? ms().useBootstrap : perGameSettings_useBootstrap) && !ms().homebrewBootstrap) || !ms().secondaryDevice) || (dsiFeatures() && romUnitCode[cursorPosOnScreen] > 0 && (perGameSettings_dsiMode == -1 ? DEFAULT_DSI_MODE : perGameSettings_dsiMode))
					|| (ms().secondaryDevice && !ms().kernelUseable)
					|| (romUnitCode[cursorPosOnScreen] == 3 && !ms().homebrewBootstrap)) {
						std::string path = argarray[0];
						std::string savename = replaceAll(filename, typeToReplace, getSavExtension());
						std::string ramdiskname = replaceAll(filename, typeToReplace, getImgExtension(perGameSettings_ramDiskNo));
						std::string romFolderNoSlash = ms().romfolder[ms().secondaryDevice];
						RemoveTrailingSlashes(romFolderNoSlash);
						std::string savepath = romFolderNoSlash + "/saves/" + savename;
						std::string ramdiskpath = romFolderNoSlash + "/ramdisks/" + ramdiskname;
						if (isHomebrew[cursorPosOnScreen]) {
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

						if (!isHomebrew[cursorPosOnScreen]) {
							// Create or expand save if game isn't homebrew
							u32 orgsavesize = getFileSize(savepath.c_str());
							u32 savesize = 524288;	// 512KB (default size)

							u32 gameTidHex = 0;
							tonccpy(&gameTidHex, gameTid[cursorPosOnScreen], 4);

							for (int i = 0; i < (int)sizeof(ROMList)/8; i++) {
								ROMListEntry* curentry = &ROMList[i];
								if (gameTidHex == curentry->GameCode) {
									if (curentry->SaveMemType != 0xFFFFFFFF) savesize = sramlen[curentry->SaveMemType];
									break;
								}
							}

							if ((orgsavesize == 0 && savesize > 0) || (orgsavesize < savesize)) {
								clearText(false);
								dialogboxHeight = 0;
								showdialogbox = true;
								printSmall(false, 0, 74, "Save management", Alignment::center, FontPalette::formTitleText);
								printSmall(false, 0, 90, (orgsavesize == 0) ? "Creating save file..." : "Expanding save file...", Alignment::center, FontPalette::formText);
								updateText(false);

								FILE *pFile = fopen(savepath.c_str(), orgsavesize > 0 ? "r+" : "wb");
								if (pFile) {
									displayDiskIcon(ms().secondaryDevice);
									fseek(pFile, savesize - 1, SEEK_SET);
									fputc('\0', pFile);
									displayDiskIcon(false);
									fclose(pFile);
								}
								clearText(false);
								printSmall(false, 0, 74, "Save management", Alignment::center, FontPalette::formTitleText);
								printSmall(false, 0, 90, (orgsavesize == 0) ? "Save file created!" : "Save file expanded!", Alignment::center, FontPalette::formText);
								updateText(false);
								for (int i = 0; i < 30; i++) swiWaitForVBlank();
							}
						}

						SetMPUSettings();

						const bool phatColors = setDSPhatColors();
						const bool boostCpu = setClockSpeed(phatColors);
						const bool useWidescreen = (perGameSettings_wideScreen == -1 ? ms().wideScreen : perGameSettings_wideScreen);

						const char *bootstrapinipath = (sys().isRunFromSD() ? BOOTSTRAP_INI : BOOTSTRAP_INI_FC);
						CIniFile bootstrapini( bootstrapinipath );
						bootstrapini.SetString("NDS-BOOTSTRAP", "NDS_PATH", path);
						bootstrapini.SetString("NDS-BOOTSTRAP", "SAV_PATH", savepath);
						if (!isHomebrew[cursorPosOnScreen]) {
							bootstrapini.SetString("NDS-BOOTSTRAP", "MANUAL_PATH", getGameManual(filename.c_str()));
						}
						bootstrapini.SetString("NDS-BOOTSTRAP", "HOMEBREW_ARG", (useWidescreen && (gameTid[cursorPosOnScreen][0] == 'W' || romVersion[cursorPosOnScreen] == 0x57)) ? "wide" : "");
						bootstrapini.SetString("NDS-BOOTSTRAP", "RAM_DRIVE_PATH", (perGameSettings_ramDiskNo >= 0 && !ms().secondaryDevice) ? ramdiskpath : "sd:/null.img");
						bootstrapini.SetString("NDS-BOOTSTRAP", "GUI_LANGUAGE", ms().getGuiLanguageString());
						bootstrapini.SetInt("NDS-BOOTSTRAP", "PHAT_COLORS", phatColors);
						bootstrapini.SetInt("NDS-BOOTSTRAP", "LANGUAGE", perGameSettings_language == -2 ? ms().getGameLanguage() : perGameSettings_language);
						bootstrapini.SetInt("NDS-BOOTSTRAP", "REGION", perGameSettings_region < -1 ? ms().gameRegion : perGameSettings_region);
						bootstrapini.SetInt("NDS-BOOTSTRAP", "USE_ROM_REGION", perGameSettings_region < -1 ? ms().useRomRegion : 0);
						bootstrapini.SetInt("NDS-BOOTSTRAP", "DSI_MODE", (dsModeForced || (romUnitCode[cursorPosOnScreen] == 0 && !isDSiMode())) ? 0 : (perGameSettings_dsiMode == -1 ? DEFAULT_DSI_MODE : perGameSettings_dsiMode));
						bootstrapini.SetInt("NDS-BOOTSTRAP", "CARD_READ_DMA", setCardReadDMA());
						if (dsiFeatures() || !ms().secondaryDevice) {
							bootstrapini.SetInt("NDS-BOOTSTRAP", "BOOST_CPU", boostCpu);
							bootstrapini.SetInt("NDS-BOOTSTRAP", "BOOST_VRAM", perGameSettings_boostVram == -1 ? DEFAULT_DSI_MODE : perGameSettings_boostVram);
							bootstrapini.SetInt("NDS-BOOTSTRAP", "ASYNC_CARD_READ", setAsyncCardRead());
						}
						bootstrapini.SetInt("NDS-BOOTSTRAP", "DONOR_SDK_VER", SetDonorSDK());
						bootstrapini.SetInt("NDS-BOOTSTRAP", "PATCH_MPU_REGION", mpuregion);
						bootstrapini.SetInt("NDS-BOOTSTRAP", "PATCH_MPU_SIZE", mpusize);
						bootstrapini.SetInt("NDS-BOOTSTRAP", "FORCE_SLEEP_PATCH", ms().forceSleepPatch);
						if (!isDSiMode() && ms().secondaryDevice && sdFound()) {
							CIniFile bootstrapiniSD(BOOTSTRAP_INI);
							bootstrapini.SetInt("NDS-BOOTSTRAP", "DEBUG", bootstrapiniSD.GetInt("NDS-BOOTSTRAP", "DEBUG", 0));
							bootstrapini.SetInt("NDS-BOOTSTRAP", "LOGGING", bootstrapiniSD.GetInt("NDS-BOOTSTRAP", "LOGGING", 0));
						}
						bootstrapini.SetInt("NDS-BOOTSTRAP", "SAVE_RELOCATION", perGameSettings_saveRelocation == -1 ? ms().saveRelocation : perGameSettings_saveRelocation);
						displayDiskIcon(!sys().isRunFromSD());
						bootstrapini.SaveIniFile( bootstrapinipath );
						displayDiskIcon(false);

						CheatCodelist codelist;
						u32 gameCode,crc32;

						if (!isHomebrew[cursorPosOnScreen]) {
							bool cheatsEnabled = true;
							const char* cheatDataBin = "/_nds/nds-bootstrap/cheatData.bin";
							mkdir("/_nds", 0777);
							mkdir("/_nds/nds-bootstrap", 0777);
							if (codelist.romData(path,gameCode,crc32)) {
								long cheatOffset; size_t cheatSize;
								FILE* dat = fopen(sys().isRunFromSD() ? "sd:/_nds/TWiLightMenu/extras/usrcheat.dat" : "fat:/_nds/TWiLightMenu/extras/usrcheat.dat","rb");
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
						ms().homebrewHasWide = (isHomebrew[cursorPosOnScreen] && (gameTid[cursorPosOnScreen][0] == 'W' || romVersion[cursorPosOnScreen] == 0x57));
						ms().launchType[ms().secondaryDevice] = TWLSettings::ESDFlashcardLaunch;
						ms().previousUsedDevice = ms().secondaryDevice;
						displayDiskIcon(!sys().isRunFromSD());
						ms().saveSettings();
						displayDiskIcon(false);

						createEsrbSplash();

						if (sdFound() && ms().homebrewBootstrap && (access("sd:/moonshl2/logbuf.txt", F_OK) == 0)) {
							remove("sd:/moonshl2/logbuf.txt"); // Delete file for Moonshell 2 to boot properly
						}

						if (dsiFeatures() || !ms().secondaryDevice) {
							SetWidescreen(filename.c_str());
						}
						if (!isDSiMode() && !ms().secondaryDevice) {
							ntrStartSdGame();
						}

						const bool useNightly = (perGameSettings_bootstrapFile == -1 ? ms().bootstrapFile : perGameSettings_bootstrapFile);

						char ndsToBoot[256];
						sprintf(ndsToBoot, "%s:/_nds/nds-bootstrap-%s%s.nds", sys().isRunFromSD() ? "sd" : "fat", ms().homebrewBootstrap ? "hb-" : "", useNightly ? "nightly" : "release");
						if (access(ndsToBoot, F_OK) != 0) {
							sprintf(ndsToBoot, "%s:/_nds/nds-bootstrap-%s%s.nds", sys().isRunFromSD() ? "fat" : "sd", ms().homebrewBootstrap ? "hb-" : "", useNightly ? "nightly" : "release");
						}

						argarray.at(0) = (char *)ndsToBoot;
						int err = runNdsFile (argarray[0], argarray.size(), (const char **)&argarray[0], sys().isRunFromSD(), (ms().homebrewBootstrap ? false : true), true, false, true, true, false, -1);
						char text[32];
						snprintf (text, sizeof(text), "Start failed. Error %i", err);
						clearText(false);
						dialogboxHeight = (err==1 ? 2 : 0);
						showdialogbox = true;
						printSmall(false, 0, 74, "Error!", Alignment::center, FontPalette::formTitleText);
						printSmall(false, 0, 90, text, Alignment::center, FontPalette::formText);
						if (err == 1) {
							if (ms().homebrewBootstrap) {
								printSmall(false, 0, 102, useNightly ? "nds-bootstrap for homebrew (Nightly)" : "nds-bootstrap for homebrew (Release)", Alignment::center, FontPalette::formText);
							} else {
								printSmall(false, 0, 102, useNightly ? "nds-bootstrap (Nightly)" : "nds-bootstrap (Release)", Alignment::center, FontPalette::formText);
							}
							printSmall(false, 0, 114, "not found.", Alignment::center, FontPalette::formText);
						}
						printSmall(false, 0, (err==1 ? 132 : 108), " Back", Alignment::center, FontPalette::formText);
						updateText(false);
						int pressed = 0;
						do {
							scanKeys();
							pressed = keysDownRepeat();
							checkSdEject();
							swiWaitForVBlank();
						} while (!(pressed & KEY_B));
						vector<char *> argarray;
						argarray.push_back((char*)(sys().isRunFromSD() ? "sd:/_nds/TWiLightMenu/akmenu.srldr" : "fat:/_nds/TWiLightMenu/akmenu.srldr"));
						runNdsFile(argarray[0], argarray.size(), (const char**)&argarray[0], sys().isRunFromSD(), true, false, false, true, true, false, -1);
					} else {
						ms().romPath[ms().secondaryDevice] = argarray[0];
						ms().launchType[ms().secondaryDevice] = TWLSettings::ESDFlashcardLaunch;
						ms().previousUsedDevice = ms().secondaryDevice;
						displayDiskIcon(!sys().isRunFromSD());
						ms().saveSettings();
						displayDiskIcon(false);
						loadGameOnFlashcard(argarray[0], true);
					}
				} else {
					if (!isArgv) {
						ms().romPath[ms().secondaryDevice] = argarray[0];
					}
					ms().homebrewHasWide = (isHomebrew[cursorPosOnScreen] && (gameTid[cursorPosOnScreen][0] == 'W' || romVersion[cursorPosOnScreen] == 0x57));
					ms().launchType[ms().secondaryDevice] = TWLSettings::ESDFlashcardDirectLaunch;
					ms().previousUsedDevice = ms().secondaryDevice;
					if (isDSiMode() || !ms().secondaryDevice) {
						SetWidescreen(filename.c_str());
					}
					displayDiskIcon(!sys().isRunFromSD());
					ms().saveSettings();
					displayDiskIcon(false);

					if (!isDSiMode() && !ms().secondaryDevice && strncmp(filename.c_str(), "GodMode9i", 9) != 0 && strcmp(gameTid[cursorPosOnScreen], "HGMA") != 0) {
						ntrStartSdGame();
					}

					int runNds_language = perGameSettings_language == -2 ? ms().getGameLanguage() : perGameSettings_language;
					int runNds_gameRegion = perGameSettings_region < -1 ? ms().gameRegion : perGameSettings_region;

					// Set region flag
					if (ms().useRomRegion && perGameSettings_region < -1 && gameTid[cursorPosOnScreen][3] != 'A' && gameTid[cursorPosOnScreen][3] != 'O' && gameTid[cursorPosOnScreen][3] != '#') {
						if (gameTid[cursorPosOnScreen][3] == 'J') {
							*(u8*)(0x02FFFD70) = 0;
						} else if (gameTid[cursorPosOnScreen][3] == 'E' || gameTid[cursorPosOnScreen][3] == 'T') {
							*(u8*)(0x02FFFD70) = 1;
						} else if (gameTid[cursorPosOnScreen][3] == 'P' || gameTid[cursorPosOnScreen][3] == 'V') {
							*(u8*)(0x02FFFD70) = 2;
						} else if (gameTid[cursorPosOnScreen][3] == 'U') {
							*(u8*)(0x02FFFD70) = 3;
						} else if (gameTid[cursorPosOnScreen][3] == 'C') {
							*(u8*)(0x02FFFD70) = 4;
						} else if (gameTid[cursorPosOnScreen][3] == 'K') {
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
					int err = runNdsFile (argarray[0], argarray.size(), (const char **)&argarray[0], sys().isRunFromSD(), true, true, dsModeSwitch, runNds_boostCpu, runNds_boostVram, false, runNds_language);
					char text[32];
					snprintf (text, sizeof(text), "Start failed. Error %i", err);
					clearText(false);
					dialogboxHeight = 0;
					showdialogbox = true;
					printSmall(false, 0, 74, "Error!", Alignment::center, FontPalette::formTitleText);
					printSmall(false, 0, 90, text, Alignment::center, FontPalette::formText);
					printSmall(false, 0, 108, " Back", Alignment::center, FontPalette::formText);
					updateText(false);
					int pressed = 0;
					do {
						scanKeys();
						pressed = keysDownRepeat();
						checkSdEject();
						swiWaitForVBlank();
					} while (!(pressed & KEY_B));
					vector<char *> argarray;
					argarray.push_back((char*)(sys().isRunFromSD() ? "sd:/_nds/TWiLightMenu/akmenu.srldr" : "fat:/_nds/TWiLightMenu/akmenu.srldr"));
					runNdsFile(argarray[0], argarray.size(), (const char**)&argarray[0], sys().isRunFromSD(), true, false, false, true, true, false, -1);
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
				} else if (extension(filename, {".fv", ".ntrb"})) {
					ms().launchType[ms().secondaryDevice] = TWLSettings::EFastVideoLaunch;

					ndsToBoot = (flashcardFound() && io_dldi_data->driverSize >= 0xF) ? "sd:/_nds/TWiLightMenu/apps/FastVideoDS32.nds" : "sd:/_nds/TWiLightMenu/apps/FastVideoDS.nds";
					if (!isDSiMode() || access(ndsToBoot, F_OK) != 0) {
						ndsToBoot = (io_dldi_data->driverSize >= 0xF) ? "fat:/_nds/TWiLightMenu/apps/FastVideoDS32.nds" : "fat:/_nds/TWiLightMenu/apps/FastVideoDS.nds";
						boostVram = true;
					}
				} else if (extension(filename, {".agb", ".gba", ".mb"})) {
					ms().launchType[ms().secondaryDevice] = (ms().gbaBooter == TWLSettings::EGbaNativeGbar2 && *(u16*)(0x020000C0) != 0) ? TWLSettings::EGBANativeLaunch : TWLSettings::ESDFlashcardLaunch;

					if (ms().launchType[ms().secondaryDevice] == TWLSettings::EGBANativeLaunch) {
						clearText(false);
						dialogboxHeight = 0;
						showdialogbox = true;
						printSmall(false, 0, 74, "Game loading", Alignment::center, FontPalette::formTitleText);
						printSmall(false, 0, 90, "Please wait...", Alignment::center, FontPalette::formText);
						updateText(false);

						displayDiskIcon(true);

						u32 ptr = 0x08000000;
						u32 romSize = getFileSize(filename.c_str());
						FILE* gbaFile = fopen(filename.c_str(), "rb");
						if (strncmp(gameTid[cursorPosOnScreen], "AGBJ", 4) == 0 && romSize <= 0x40000) {
							ptr += 0x400;
						}

						extern char copyBuf[0x8000];
						if (romSize > 0x2000000) romSize = 0x2000000;

						bool nor = false;
						if (*(u16*)(0x020000C0) == 0x5A45 && strncmp(gameTid[cursorPosOnScreen], "AGBJ", 4) != 0) {
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

						displayDiskIcon(false);

						ndsToBoot = "fat:/_nds/TWiLightMenu/gbapatcher.srldr";
					} else if (ms().gbaR3Test) {
						ms().launchType[ms().secondaryDevice] = TWLSettings::EGBARunner2Launch;

						ndsToBoot = sys().isRunFromSD() ? "sd:/_nds/TWiLightMenu/emulators/GBARunner3.nds" : "fat:/_nds/TWiLightMenu/emulators/GBARunner3.nds";
						if (!isDSiMode()) {
							boostVram = true;
						}
					} else if (ms().secondaryDevice) {
						ms().launchType[ms().secondaryDevice] = TWLSettings::EGBARunner2Launch;

						if (dsiFeatures()) {
							ndsToBoot = ms().consoleModel > 0 ? "sd:/_nds/GBARunner2_arm7dldi_3ds.nds" : "sd:/_nds/GBARunner2_arm7dldi_dsi.nds";
						} else if (memcmp(gameTid[cursorPosOnScreen], "BPE", 3) == 0) { // If game is Pokemon Emerald...
							ndsToBoot = ms().gbar2DldiAccess ? "sd:/_nds/GBARunner2_arm7dldi_rom3m_ds.nds" : "sd:/_nds/GBARunner2_arm9dldi_rom3m_ds.nds";
						} else {
							ndsToBoot = ms().gbar2DldiAccess ? "sd:/_nds/GBARunner2_arm7dldi_ds.nds" : "sd:/_nds/GBARunner2_arm9dldi_ds.nds";
						}
						if (!isDSiMode() || access(ndsToBoot, F_OK) != 0) {
							if (dsiFeatures()) {
								ndsToBoot = ms().consoleModel > 0 ? "fat:/_nds/GBARunner2_arm7dldi_3ds.nds" : "fat:/_nds/GBARunner2_arm7dldi_dsi.nds";
							} else if (memcmp(gameTid[cursorPosOnScreen], "BPE", 3) == 0) { // If game is Pokemon Emerald...
								ndsToBoot = ms().gbar2DldiAccess ? "fat:/_nds/GBARunner2_arm7dldi_rom3m_ds.nds" : "fat:/_nds/GBARunner2_arm9dldi_rom3m_ds.nds";
							} else {
								ndsToBoot = ms().gbar2DldiAccess ? "fat:/_nds/GBARunner2_arm7dldi_ds.nds" : "fat:/_nds/GBARunner2_arm9dldi_ds.nds";
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

					ndsToBoot = "sd:/_nds/TWiLightMenu/emulators/A8DS.nds";
					if (!isDSiMode() || access(ndsToBoot, F_OK) != 0) {
						ndsToBoot = "fat:/_nds/TWiLightMenu/emulators/A8DS.nds";
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
				} else if ((extension(filename, {".sg", ".sc"}) && ms().sgEmulator == TWLSettings::EColSegaColecoDS) || (extension(filename, {".col"}) && ms().colEmulator == TWLSettings::EColSegaColecoDS) || extension(filename, {".m5", ".msx"})) {
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

					ndsToBoot = "sd:/_nds/TWiLightMenu/emulators/nesds.nds";
					if (!isDSiMode() || access(ndsToBoot, F_OK) != 0) {
						ndsToBoot = "fat:/_nds/TWiLightMenu/emulators/nesds.nds";
						boostVram = true;
					}
				} else if ((extension(filename, {".sg", ".sc"}) && ms().sgEmulator == TWLSettings::EColSegaS8DS) || (extension(filename, {".sms", ".gg"})) || (extension(filename, {".col"}) && ms().colEmulator == TWLSettings::EColSegaS8DS)) {
					mkdir(ms().secondaryDevice ? "fat:/data" : "sd:/data", 0777);
					mkdir(ms().secondaryDevice ? "fat:/data/s8ds" : "sd:/data/s8ds", 0777);

					ms().launchType[ms().secondaryDevice] = TWLSettings::ES8DSLaunch;

					ndsToBoot = "sd:/_nds/TWiLightMenu/emulators/S8DS.nds";
					if (!isDSiMode() || access(ndsToBoot, F_OK) != 0) {
						ndsToBoot = "fat:/_nds/TWiLightMenu/emulators/S8DS.nds";
						boostVram = true;
					}
				} else if (extension(filename, {".gen", ".md"})) {
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
					ms().launchType[ms().secondaryDevice] = ((ms().newSnesEmuVer || ms().secondaryDevice) ? TWLSettings::ESNEmulDSLaunch : TWLSettings::ESDFlashcardLaunch);
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
				} else if (extension(filename, {".dsk"})) {
					ms().launchType[ms().secondaryDevice] = TWLSettings::ESugarDSLaunch;

					ndsToBoot = "sd:/_nds/TWiLightMenu/emulators/SugarDS.nds";
					if (!isDSiMode() || access(ndsToBoot, F_OK) != 0) {
						ndsToBoot = "fat:/_nds/TWiLightMenu/emulators/SugarDS.nds";
						boostVram = true;
					}
				} else if (extension(filename, {".min"})) {
					ms().launchType[ms().secondaryDevice] = TWLSettings::EPokeMiniLaunch;

					ndsToBoot = "sd:/_nds/TWiLightMenu/emulators/PokeMini.nds";
					if (!isDSiMode() || access(ndsToBoot, F_OK) != 0) {
						ndsToBoot = "fat:/_nds/TWiLightMenu/emulators/PokeMini.nds";
						boostVram = true;
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
				displayDiskIcon(!sys().isRunFromSD());
				ms().saveSettings();
				displayDiskIcon(false);

				if (!isDSiMode() && !ms().secondaryDevice && !extension(filename, {".plg", ".gif", ".bmp", ".png"})) {
					ntrStartSdGame();
				}

				if (tgdsMode && !ms().secondaryDevice) {
					std::string romfolderFat = replaceAll(romfolderNoSlash, "sd:", "fat:");
					snprintf (ROMpath, sizeof(ROMpath), "%s/%s", romfolderFat.c_str(), filename.c_str());
				}
				argarray.push_back(ROMpath);
				argarray.at(0) = (char *)(tgdsMode ? tgdsNdsPath : ndsToBoot);

				int err = runNdsFile (ndsToBoot, argarray.size(), (const char **)&argarray[0], sys().isRunFromSD(), !useNDSB, true, dsModeSwitch, boostCpu, boostVram, tscTgds, -1);	// Pass ROM to emulator as argument
				char text[32];
				snprintf (text, sizeof(text), "Start failed. Error %i", err);
				clearText(false);
				dialogboxHeight = ((err == 1 && useNDSB) ? 2 : 0);
				showdialogbox = true;
				printSmall(false, 0, 74, "Error!", Alignment::center, FontPalette::formTitleText);
				printSmall(false, 0, 90, text, Alignment::center, FontPalette::formText);
				if (err == 1 && useNDSB) {
					printSmall(false, 0, 102, ms().bootstrapFile ? "nds-bootstrap for homebrew (Nightly)" : "nds-bootstrap for homebrew (Release)", Alignment::center, FontPalette::formText);
					printSmall(false, 0, 114, "not found.", Alignment::center, FontPalette::formText);
				}
				printSmall(false, 0, ((err == 1 && useNDSB) ? 132 : 108), " Back", Alignment::center, FontPalette::formText);
				updateText(false);
				int pressed = 0;
				do {
					scanKeys();
					pressed = keysDownRepeat();
					checkSdEject();
					swiWaitForVBlank();
				} while (!(pressed & KEY_B));
				vector<char *> argarray;
				argarray.push_back((char*)(sys().isRunFromSD() ? "sd:/_nds/TWiLightMenu/akmenu.srldr" : "fat:/_nds/TWiLightMenu/akmenu.srldr"));
				runNdsFile(argarray[0], argarray.size(), (const char**)&argarray[0], sys().isRunFromSD(), true, false, false, true, true, false, -1);
			}

			while (argarray.size() !=0 ) {
				free(argarray.at(0));
				argarray.erase(argarray.begin());
			}
		}
	}

	return 0;
}
