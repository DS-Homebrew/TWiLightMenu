#include <nds.h>
#include <nds/arm9/dldi.h>
#include "fat_ext.h"
#include "io_m3_common.h"
#include "io_g6_common.h"
#include "io_sc_common.h"
#include "myDSiMode.h"
#include "exptools.h"
#include "nand/nandio.h"

#include "bootsplash.h"
#include "common/bootstrapsettings.h"
#include "common/bootstrappaths.h"
#include "common/cardlaunch.h"
#include "common/twlmenusettings.h"
#include "common/fileCopy.h"
#include "common/fatHeader.h"
#include "common/flashcard.h"
#include "common/inifile.h"
#include "common/logging.h"
#include "common/nds_loader_arm9.h"
#include "ndsheaderbanner.h"
#include "common/pergamesettings.h"
#include "common/stringtool.h"
#include "common/systemdetails.h"
#include "common/tonccpy.h"
#include "defaultSettings.h"
#include "twlFlashcard.h"
#include "graphics/graphics.h"
#include "twlmenuppvideo.h"
#include "language.h"
#include "graphics/fontHandler.h"
#include "sound.h"

#include "autoboot.h"		 // For rebooting into the game

#include "twlClockExcludeMap.h"
#include "dmaExcludeMap.h"
#include "asyncReadExcludeMap.h"
#include "saveMap.h"
#include "ROMList.h"

extern bool useTwlCfg;

bool renderScreens = false;
bool fadeType = false; // false = out, true = in
bool fadeColor = true; // false = black, true = white
extern bool controlTopBright;
extern bool controlBottomBright;

//bool soundfreqsettingChanged = false;
bool hiyaAutobootFound = false;
//static int flashcard;
/* Flashcard value
	0: DSTT/R4i Gold/R4i-SDHC/R4 SDHC Dual-Core/R4 SDHC Upgrade/SC DSONE
	1: R4DS (Original Non-SDHC version)/ M3 Simply
	2: R4iDSN/R4i Gold RTS/R4 Ultra
	3: Acekard 2(i)/Galaxy Eagle/M3DS Real
	4: Acekard RPG
	5: Ace 3DS+/Gateway Blue Card/R4iTT
	6: SuperCard DSTWO
*/

const char *settingsinipath = DSIMENUPP_INI;

static sNDSHeaderExt NDSHeader;

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

static const std::string slashchar = "/";
static const std::string woodfat = "fat0:/";
static const std::string dstwofat = "fat1:/";

typedef TWLSettings::TLaunchType Launch;

int screenmode = 0;
int subscreenmode = 0;

touchPosition touch;

using namespace std;

//---------------------------------------------------------------------------------
void stop(void)
{
	//---------------------------------------------------------------------------------
	while (1) {
		swiWaitForVBlank();
	}
}

char filePath[PATH_MAX];

/*//---------------------------------------------------------------------------------
void doPause(void)
{
	//---------------------------------------------------------------------------------
	printf("Press start...\n");
	//printSmall(false, x, y, "Press start...");
	while (1) {
		scanKeys();
		if (keysDown() & KEY_START)
			break;
	}
	scanKeys();
}*/

void loadMainMenu()
{
	fadeColor = true;
	controlTopBright = true;
	controlBottomBright = true;
	fadeType = false;
	fifoSendValue32(FIFO_USER_01, 1); // Fade out sound
	for (int i = 0; i < 25; i++)
		swiWaitForVBlank();
	fifoSendValue32(FIFO_USER_01, 0); // Cancel sound fade out

	logPrint("Opening DS Classic Menu...\n");

	vector<char *> argarray;
	argarray.push_back((char*)(sys().isRunFromSD() ? "sd:/_nds/TWiLightMenu/mainmenu.srldr" : "fat:/_nds/TWiLightMenu/mainmenu.srldr"));

	runNdsFile(argarray[0], argarray.size(), (const char**)&argarray[0], true, false, false, true, true, false, -1);
}

void loadROMselect(void)
{
	fadeColor = true;
	controlTopBright = true;
	controlBottomBright = true;
	fadeType = false;
	fifoSendValue32(FIFO_USER_01, 1); // Fade out sound
	for (int i = 0; i < 25; i++)
		swiWaitForVBlank();
	fifoSendValue32(FIFO_USER_01, 0); // Cancel sound fade out

	vector<char *> argarray;

	switch (ms().theme) {
		/*case 3:
			logPrint("Opening Wood theme...\n");
			runNdsFile("/_nds/TWiLightMenu/akmenu.srldr", 0, NULL, true, false, false, true, true, false, -1);
			break;*/
		case 2:
		case 6:
			logPrint("Opening R4 Original or GameBoy Color theme...\n");
			argarray.push_back((char*)(sys().isRunFromSD() ? "sd:/_nds/TWiLightMenu/r4menu.srldr" : "fat:/_nds/TWiLightMenu/r4menu.srldr"));
			runNdsFile(argarray[0], argarray.size(), (const char**)&argarray[0], true, false, false, true, true, false, -1);
			break;
		default:
			logPrint("Opening DSi, 3DS, Saturn, or HBL theme...\n");
			argarray.push_back((char*)(sys().isRunFromSD() ? "sd:/_nds/TWiLightMenu/dsimenu.srldr" : "fat:/_nds/TWiLightMenu/dsimenu.srldr"));
			runNdsFile(argarray[0], argarray.size(), (const char**)&argarray[0], true, false, false, true, true, false, -1);
			break;
	}
	stop();
}

/*void loadROMselectAsynch(void)
{
	switch (ms().theme) {
		case 2:
		case 6:
			loadNds9iAsynch(!isDSiMode() ? "fat:/_nds/TWiLightMenu/r4menu.srldr" : "/_nds/TWiLightMenu/r4menu.srldr");
			break;
		default:
			loadNds9iAsynch(!isDSiMode() ? "fat:/_nds/TWiLightMenu/dsimenu.srldr" : "/_nds/TWiLightMenu/dsimenu.srldr");
			break;
	}
}*/

bool extension(const std::string& filename, const char* ext) {
	if (strcasecmp(filename.c_str() + filename.size() - strlen(ext), ext)) {
		return false;
	} else {
		return true;
	}
}

// Unlaunch needs the path in 16 bit unicode so this function is
// from the FontGraphic class, but as that's not used in title
// it's copied here. Remove this if adding FontGraphic to title.
std::u16string utf8to16(std::string_view text) {
	std::u16string out;
	for (uint i=0;i<text.size();) {
		char16_t c = 0;
		if (!(text[i] & 0x80)) {
			c = text[i++];
		} else if ((text[i] & 0xE0) == 0xC0) {
			c  = (text[i++] & 0x1F) << 6;
			c |=  text[i++] & 0x3F;
		} else if ((text[i] & 0xF0) == 0xE0) {
			c  = (text[i++] & 0x0F) << 12;
			c |= (text[i++] & 0x3F) << 6;
			c |=  text[i++] & 0x3F;
		} else {
			i++; // out of range or something (This only does up to 0xFFFF since it goes to a U16 anyways)
		}
		out += c;
	}
	return out;
}

void unlaunchRomBoot(std::string_view rom) {
	std::u16string path(utf8to16(rom));
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
	while (*(u16*)(0x0200080E) == 0) {	// Keep running, so that CRC16 isn't 0
		*(u16*)(0x0200080E) = swiCRC16(0xFFFF, (void*)0x02000810, 0x3F0);		// Unlaunch CRC16
	}

	DC_FlushAll();						// Make reboot not fail
	fifoSendValue32(FIFO_USER_02, 1);	// Reboot into DSiWare title, booted via Unlaunch
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

/**
 * Used to reboot into an SD game when in DS mode.
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

bool autoRunBit = false;
bool runTempDSiWare = false;
bool twlBgCxiFound = false;

void wideCheck(bool useWidescreen, bool checkCheatData) {
	if ((isDSiMode() && sys().arm7SCFGLocked()) || ms().consoleModel < 2 || !useWidescreen || ms().macroMode) {
		remove("/_nds/nds-bootstrap/wideCheatData.bin");
		return;
	}

	CIniFile lumaConfig("sd:/luma/config.ini");

	bool wideCheatFound = !checkCheatData ? true : (access("/_nds/nds-bootstrap/wideCheatData.bin", F_OK) == 0);
	if (useWidescreen && wideCheatFound && (lumaConfig.GetInt("boot", "enable_external_firm_and_modules", 0) == true)) {
		if (access("sd:/_nds/TWiLightMenu/TwlBg/Widescreen.cxi", F_OK) == 0 && !autoRunBit) {
			// If title previously launched in widescreen, move Widescreen.cxi again, and reboot again
			if (access("sd:/luma/sysmodules/TwlBg.cxi", F_OK) == 0) {
				rename("sd:/luma/sysmodules/TwlBg.cxi", "sd:/_nds/TWiLightMenu/TwlBg/TwlBg.cxi.bak");
			}
			if (rename("sd:/_nds/TWiLightMenu/TwlBg/Widescreen.cxi", "sd:/luma/sysmodules/TwlBg.cxi") == 0) {
				ntrStartSdGame();
			}
		} else if (twlBgCxiFound) {
			// Revert back to 4:3 for when returning to TWLMenu++
			mkdir("sd:/_nds/TWiLightMenu/TwlBg", 0777);
			if (rename("sd:/luma/sysmodules/TwlBg.cxi", "sd:/_nds/TWiLightMenu/TwlBg/Widescreen.cxi") != 0) {
				consoleDemoInit();
				iprintf("Failed to rename TwlBg.cxi\n");
				iprintf("back to Widescreen.cxi\n");
				for (int i = 0; i < 60*3; i++) swiWaitForVBlank();
			}
			if (access("sd:/_nds/TWiLightMenu/TwlBg/TwlBg.cxi.bak", F_OK) == 0) {
				rename("sd:/_nds/TWiLightMenu/TwlBg/TwlBg.cxi.bak", "sd:/luma/sysmodules/TwlBg.cxi");
			}
		}
	}
}

void lastRunROM()
{
	/*fifoSendValue32(FIFO_USER_01, 1); // Fade out sound
	for (int i = 0; i < 25; i++)
		swiWaitForVBlank();
	fifoSendValue32(FIFO_USER_01, 0); // Cancel sound fade out*/

	std::string romfolder = ms().romPath[ms().previousUsedDevice];
	while (!romfolder.empty() && romfolder[romfolder.size()-1] != '/') {
		romfolder.resize(romfolder.size()-1);
	}
	chdir(romfolder.c_str());

	std::string filename = ms().romPath[ms().previousUsedDevice];
	const size_t last_slash_idx = filename.find_last_of("/");
	if (std::string::npos != last_slash_idx) {
		filename.erase(0, last_slash_idx + 1);
	}

	vector<char *> argarray;
	if (ms().launchType[ms().previousUsedDevice] == Launch::ESNEmulDSLaunch && !ms().previousUsedDevice) {
		std::string homebrewArgFat = replaceAll(ms().homebrewArg[ms().previousUsedDevice], "sd:", "fat:");
		argarray.push_back(strdup("null"));
		argarray.push_back((char*)homebrewArgFat.c_str());
	} else
	if (ms().launchType[ms().previousUsedDevice] > Launch::EDSiWareLaunch) {
		argarray.push_back(strdup("null"));
		if (ms().launchType[ms().previousUsedDevice] == Launch::EGBANativeLaunch) {
			argarray.push_back(strdup(ms().romPath[ms().previousUsedDevice].c_str()));
		} else {
			argarray.push_back(strdup(ms().homebrewArg[ms().previousUsedDevice].c_str()));
		}
	}

	if (ms().consoleModel >= 2) {
		twlBgCxiFound = (access("sd:/luma/sysmodules/TwlBg.cxi", F_OK) == 0);
	}

	int err = 0;
	if (!runTempDSiWare && ms().slot1Launched && (!flashcardFound() || (io_dldi_data->ioInterface.features & FEATURE_SLOT_GBA))) {
		if (io_dldi_data->ioInterface.features & FEATURE_SLOT_GBA) {
			err = runNdsFile("/_nds/TWiLightMenu/slot1launch.srldr", 0, NULL, true, true, false, true, true, false, -1);
		} else if (ms().slot1LaunchMethod==0 || sys().arm7SCFGLocked()) {
			dsCardLaunch();
		} else if (ms().slot1LaunchMethod==2) {
			unlaunchRomBoot("cart:");
		} else {
			wideCheck(ms().wideScreen, true);
			err = runNdsFile("/_nds/TWiLightMenu/slot1launch.srldr", 0, NULL, true, true, false, true, true, false, -1);
		}
	}
	if (!runTempDSiWare && ms().launchType[ms().previousUsedDevice] == Launch::ESDFlashcardLaunch) {
		if (access(ms().romPath[ms().previousUsedDevice].c_str(), F_OK) != 0) return;	// Skip to running TWiLight Menu++

		loadPerGameSettings(filename);

		bool dsiBinariesFound = true;
		char game_TID[5];
		u8 unitCode = 0;

		FILE *f_nds_file = fopen(filename.c_str(), "rb");
		dsiBinariesFound = checkDsiBinaries(f_nds_file);
		fseek(f_nds_file, offsetof(sNDSHeaderExt, gameCode), SEEK_SET);
		fread(game_TID, 1, 4, f_nds_file);
		fseek(f_nds_file, 0x12, SEEK_SET);
		fread(&unitCode, 1, 1, f_nds_file);
		game_TID[4] = 0;

		fclose(f_nds_file);

		if ((perGameSettings_useBootstrap == -1 ? ms().useBootstrap : perGameSettings_useBootstrap) || !ms().previousUsedDevice || (dsiFeatures() && unitCode > 0 && (perGameSettings_dsiMode == -1 ? DEFAULT_DSI_MODE : perGameSettings_dsiMode))
		|| (game_TID[0] == 'D' && unitCode == 3)) {
			std::string savepath;

			bool useWidescreen = (perGameSettings_wideScreen == -1 ? ms().wideScreen : perGameSettings_wideScreen);
			bool useNightly = (perGameSettings_bootstrapFile == -1 ? ms().bootstrapFile : perGameSettings_bootstrapFile);
			bool boostCpu = (perGameSettings_boostCpu == -1 ? DEFAULT_BOOST_CPU : perGameSettings_boostCpu);
			bool cardReadDMA = (perGameSettings_cardReadDMA == -1 ? DEFAULT_CARD_READ_DMA : perGameSettings_cardReadDMA);
			bool asyncCardRead = (perGameSettings_asyncCardRead == -1 ? DEFAULT_ASYNC_CARD_READ : perGameSettings_asyncCardRead);
			bool dsModeForced = false;

			wideCheck(useWidescreen, !ms().homebrewBootstrap);

			if (ms().homebrewBootstrap) {
				argarray.push_back((char*)(useNightly ? "sd:/_nds/nds-bootstrap-hb-nightly.nds" : "sd:/_nds/nds-bootstrap-hb-release.nds"));
			} else {
				const char *typeToReplace = ".nds";
				if (extension(filename, ".dsi")) {
					typeToReplace = ".dsi";
				} else if (extension(filename, ".ids")) {
					typeToReplace = ".ids";
				} else if (extension(filename, ".srl")) {
					typeToReplace = ".srl";
				} else if (extension(filename, ".app")) {
					typeToReplace = ".app";
				}

				std::string savename = replaceAll(filename, typeToReplace, getSavExtension());
				std::string romFolderNoSlash = romfolder;
				RemoveTrailingSlashes(romFolderNoSlash);
				mkdir ("saves", 0777);
				savepath = romFolderNoSlash+"/saves/"+savename;
				if (ms().previousUsedDevice && ms().fcSaveOnSd) {
					savepath = replaceAll(savepath, "fat:/", "sd:/");
				}

				u32 orgsavesize = getFileSize(savepath.c_str());
				u32 savesize = 524288;	// 512KB (default size for most games)

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
					consoleDemoInit();
					iprintf((orgsavesize == 0) ? "Creating save file...\n" : "Expanding save file...\n");
					iprintf ("\n");
					fadeType = true;

					FILE *pFile = fopen(savepath.c_str(), orgsavesize > 0 ? "r+" : "wb");
					if (pFile) {
						fseek(pFile, savesize - 1, SEEK_SET);
						fputc('\0', pFile);
						fclose(pFile);
					}
					iprintf((orgsavesize == 0) ? "Save file created!\n" : "Save file expanded!\n");

					for (int i = 0; i < 30; i++) {
						swiWaitForVBlank();
					}
					fadeType = false;
					for (int i = 0; i < 25; i++) {
						swiWaitForVBlank();
					}
				}

				if (!ms().ignoreBlacklists) {
					// TODO: If the list gets large enough, switch to bsearch().
					for (unsigned int i = 0; i < sizeof(twlClockExcludeList)/sizeof(twlClockExcludeList[0]); i++) {
						if (memcmp(game_TID, twlClockExcludeList[i], 3) == 0) {
							// Found match
							boostCpu = false;
							dsModeForced = true;
							break;
						}
					}

					for (unsigned int i = 0; i < sizeof(cardReadDMAExcludeList)/sizeof(cardReadDMAExcludeList[0]); i++) {
						if (memcmp(game_TID, cardReadDMAExcludeList[i], 3) == 0) {
							// Found match
							cardReadDMA = false;
							break;
						}
					}

					for (unsigned int i = 0; i < sizeof(asyncReadExcludeList)/sizeof(asyncReadExcludeList[0]); i++) {
						if (memcmp(game_TID, asyncReadExcludeList[i], 3) == 0) {
							// Found match
							asyncCardRead = false;
							break;
						}
					}
				}

				if (sys().isRunFromSD()) {
					argarray.push_back((char*)(useNightly ? "sd:/_nds/nds-bootstrap-nightly.nds" : "sd:/_nds/nds-bootstrap-release.nds"));
				} else {
					argarray.push_back((char*)(useNightly ? "fat:/_nds/nds-bootstrap-nightly.nds" : "fat:/_nds/nds-bootstrap-release.nds"));
				}
			}
			if (ms().previousUsedDevice || !ms().homebrewBootstrap) {
				CIniFile bootstrapini( sys().isRunFromSD() ? BOOTSTRAP_INI : BOOTSTRAP_INI_FC );
				bootstrapini.SetString("NDS-BOOTSTRAP", "NDS_PATH", ms().romPath[ms().previousUsedDevice]);
				bootstrapini.SetString("NDS-BOOTSTRAP", "SAV_PATH", savepath);
				bootstrapini.SetString("NDS-BOOTSTRAP", "GUI_LANGUAGE", ms().getGuiLanguageString());
				bootstrapini.SetInt("NDS-BOOTSTRAP", "LANGUAGE", (perGameSettings_language == -2 ? ms().gameLanguage : perGameSettings_language));
				bootstrapini.SetInt("NDS-BOOTSTRAP", "REGION", (perGameSettings_region < -1 ? ms().gameRegion : perGameSettings_region));
				bootstrapini.SetInt("NDS-BOOTSTRAP", "USE_ROM_REGION", (perGameSettings_region < -1 ? ms().useRomRegion : 0));
				bootstrapini.SetInt("NDS-BOOTSTRAP", "DSI_MODE", (dsModeForced || !dsiBinariesFound) ? 0 : (perGameSettings_dsiMode == -1 ? DEFAULT_DSI_MODE : perGameSettings_dsiMode));
				bootstrapini.SetInt("NDS-BOOTSTRAP", "BOOST_CPU", boostCpu);
				bootstrapini.SetInt("NDS-BOOTSTRAP", "BOOST_VRAM", (perGameSettings_boostVram == -1 ? DEFAULT_BOOST_VRAM : perGameSettings_boostVram));
				bootstrapini.SetInt("NDS-BOOTSTRAP", "CARD_READ_DMA", cardReadDMA);
				bootstrapini.SetInt("NDS-BOOTSTRAP", "ASYNC_CARD_READ", asyncCardRead);
				bootstrapini.SetInt("NDS-BOOTSTRAP", "EXTENDED_MEMORY", perGameSettings_expandRomSpace == -1 ? ms().extendedMemory : perGameSettings_expandRomSpace);
				bootstrapini.SetInt("NDS-BOOTSTRAP", "FORCE_SLEEP_PATCH", 
					(ms().forceSleepPatch
				|| (memcmp(io_dldi_data->friendlyName, "TTCARD", 6) == 0 && !sys().isRegularDS())
				|| (memcmp(io_dldi_data->friendlyName, "DSTT", 4) == 0 && !sys().isRegularDS())
				|| (memcmp(io_dldi_data->friendlyName, "DEMON", 5) == 0 && !sys().isRegularDS())
				|| (memcmp(io_dldi_data->friendlyName, "R4iDSN", 6) == 0 && !sys().isRegularDS()))
				);
				bootstrapini.SaveIniFile( sys().isRunFromSD() ? BOOTSTRAP_INI : BOOTSTRAP_INI_FC );
			}
			err = runNdsFile(argarray[0], argarray.size(), (const char **)&argarray[0], (ms().homebrewBootstrap ? false : true), true, false, true, true, false, -1);
		} else {
			bool runNds_boostCpu = false;
			bool runNds_boostVram = false;
			loadPerGameSettings(filename);
			if (REG_SCFG_EXT != 0) {
				runNds_boostCpu = perGameSettings_boostCpu == -1 ? DEFAULT_BOOST_CPU : perGameSettings_boostCpu;
				runNds_boostVram = perGameSettings_boostVram == -1 ? DEFAULT_BOOST_VRAM : perGameSettings_boostVram;
			}

			// Move .sav outside of "saves" folder for flashcard kernel usage
			const char *typeToReplace = ".nds";
			if (extension(filename, ".dsi")) {
				typeToReplace = ".dsi";
			} else if (extension(filename, ".ids")) {
				typeToReplace = ".ids";
			} else if (extension(filename, ".srl")) {
				typeToReplace = ".srl";
			} else if (extension(filename, ".app")) {
				typeToReplace = ".app";
			}

			std::string savename = replaceAll(filename, typeToReplace, getSavExtension());
			std::string savenameFc = replaceAll(filename, typeToReplace, ".sav");
			std::string romFolderNoSlash = romfolder;
			RemoveTrailingSlashes(romFolderNoSlash);
			mkdir("saves", 0777);
			std::string savepath = romFolderNoSlash + "/saves/" + savename;
			std::string savepathFc = romFolderNoSlash + "/" + savenameFc;
			rename(savepath.c_str(), savepathFc.c_str());

			std::string fcPath;
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
				fcPath = replaceAll(ms().romPath[ms().previousUsedDevice], "fat:/", woodfat);
				fcrompathini.SetString("Save Info", "lastLoaded", fcPath);
				fcrompathini.SaveIniFile("fat:/_wfwd/lastsave.ini");
				err = runNdsFile("fat:/Wfwd.dat", 0, NULL, true, true, true, runNds_boostCpu, runNds_boostVram, false, -1);
			} else if (memcmp(io_dldi_data->friendlyName, "DSTWO(Slot-1)", 0xD) == 0) {
				CIniFile fcrompathini("fat:/_dstwo/autoboot.ini");
				fcPath = replaceAll(ms().romPath[ms().previousUsedDevice], "fat:/", dstwofat);
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
				fcPath = replaceAll(ms().romPath[ms().previousUsedDevice], "fat:/", slashchar);
				fcrompathini.SetString("YSMENU", "AUTO_BOOT", fcPath);
				fcrompathini.SaveIniFile("fat:/TTMenu/YSMenu.ini");
				err = runNdsFile("fat:/YSMenu.nds", 0, NULL, true, true, true, runNds_boostCpu, runNds_boostVram, false, -1);
			}
		}
	} else if (!runTempDSiWare && ms().launchType[ms().previousUsedDevice] == Launch::ESDFlashcardDirectLaunch) {
		if (access(ms().romPath[ms().previousUsedDevice].c_str(), F_OK) != 0) return;	// Skip to running TWiLight Menu++

		argarray.push_back((char*)ms().romPath[ms().previousUsedDevice].c_str());

		char game_TID[5];

		FILE *f_nds_file = fopen(filename.c_str(), "rb");
		fseek(f_nds_file, offsetof(sNDSHeaderExt, gameCode), SEEK_SET);
		fread(game_TID, 1, 4, f_nds_file);
		game_TID[4] = 0;

		fclose(f_nds_file);

		loadPerGameSettings(filename);

		int language = perGameSettings_language == -2 ? ms().gameLanguage : perGameSettings_language;
		int gameRegion = perGameSettings_region < -1 ? ms().gameRegion : perGameSettings_region;

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

		bool runNds_boostCpu = false;
		bool runNds_boostVram = false;

		bool useWidescreen = (perGameSettings_wideScreen == -1 ? ms().wideScreen : perGameSettings_wideScreen);

		if (ms().consoleModel >= 2 && useWidescreen && ms().homebrewHasWide) {
			//argarray.push_back((char*)"wide");
			wideCheck(useWidescreen, false);
		}

		runNds_boostCpu = perGameSettings_boostCpu == -1 ? DEFAULT_BOOST_CPU : perGameSettings_boostCpu;
		runNds_boostVram = perGameSettings_boostVram == -1 ? DEFAULT_BOOST_VRAM : perGameSettings_boostVram;

		err = runNdsFile (argarray[0], argarray.size(), (const char **)&argarray[0], true, true, (!perGameSettings_dsiMode ? true : false), runNds_boostCpu, runNds_boostVram, false, language);
	} else if (runTempDSiWare || ms().launchType[ms().previousUsedDevice] == Launch::EDSiWareLaunch) {
		if (!runTempDSiWare && access(ms().romPath[ms().previousUsedDevice].c_str(), F_OK) != 0) return;	// Skip to running TWiLight Menu++

		loadPerGameSettings(filename);
		if ((perGameSettings_dsiwareBooter == -1 ? ms().dsiWareBooter : perGameSettings_dsiwareBooter) || (ms().previousUsedDevice && bs().b4dsMode) || sys().arm7SCFGLocked() || ms().consoleModel > 0) {
			if (ms().homebrewBootstrap) {
				unlaunchRomBoot(ms().previousUsedDevice ? "sdmc:/_nds/TWiLightMenu/tempDSiWare.dsi" : ms().dsiWareSrlPath);
			} else {
				bool useWidescreen = (perGameSettings_wideScreen == -1 ? ms().wideScreen : perGameSettings_wideScreen);
				bool useNightly = (perGameSettings_bootstrapFile == -1 ? ms().bootstrapFile : perGameSettings_bootstrapFile);
				bool cardReadDMA = (perGameSettings_cardReadDMA == -1 ? DEFAULT_CARD_READ_DMA : perGameSettings_cardReadDMA);
				if (runTempDSiWare) {
					useWidescreen = *(bool*)(0x02000014);
					useNightly = *(bool*)(0x02000010);
				}
				wideCheck(useWidescreen, true);

				const char *typeToReplace = ".nds";
				if (extension(filename, ".dsi")) {
					typeToReplace = ".dsi";
				} else if (extension(filename, ".ids")) {
					typeToReplace = ".ids";
				} else if (extension(filename, ".srl")) {
					typeToReplace = ".srl";
				} else if (extension(filename, ".app")) {
					typeToReplace = ".app";
				}

				std::string romFolderNoSlash = romfolder;
				RemoveTrailingSlashes(romFolderNoSlash);
				mkdir ("saves", 0777);

				FILE *f_nds_file = fopen(filename.c_str(), "rb");

				fread(&NDSHeader, 1, sizeof(NDSHeader), f_nds_file);
				fclose(f_nds_file);

				bool savFormat = (ms().previousUsedDevice && (!sdFound() || !ms().dsiWareToSD || bs().b4dsMode));

				if (!runTempDSiWare) {
					ms().dsiWareSrlPath = ms().romPath[ms().previousUsedDevice];
					ms().dsiWarePubPath = romFolderNoSlash + "/saves/" + filename;
					ms().dsiWarePrvPath = ms().dsiWarePubPath;
					if (savFormat) {
						ms().dsiWarePubPath = replaceAll(ms().dsiWarePubPath, typeToReplace, getSavExtension());
						ms().dsiWarePrvPath = ms().dsiWarePubPath;
					} else {
						ms().dsiWarePubPath = replaceAll(ms().dsiWarePubPath, typeToReplace, getPubExtension());
						ms().dsiWarePrvPath = replaceAll(ms().dsiWarePrvPath, typeToReplace, getPrvExtension());
					}
					ms().saveSettings();
				}

				if (savFormat) {
					if ((getFileSize(ms().dsiWarePubPath.c_str()) == 0) && ((NDSHeader.pubSavSize > 0) || (NDSHeader.prvSavSize > 0))) {
						consoleDemoInit();
						iprintf("Creating save file...\n");
						iprintf ("\n");
						fadeType = true;

						FILE *pFile = fopen(ms().dsiWarePubPath.c_str(), "wb");
						if (pFile) {
							u32 savesize = ((NDSHeader.pubSavSize > 0) ? NDSHeader.pubSavSize : NDSHeader.prvSavSize);
							fseek(pFile, savesize - 1, SEEK_SET);
							fputc('\0', pFile);
							fclose(pFile);
						}

						iprintf("Save file created!\n");

						for (int i = 0; i < 30; i++) {
							swiWaitForVBlank();
						}
						fadeType = false;
						for (int i = 0; i < 25; i++) {
							swiWaitForVBlank();
						}
					}
				} else {
					if ((getFileSize(ms().dsiWarePubPath.c_str()) == 0) && (NDSHeader.pubSavSize > 0)) {
						consoleDemoInit();
						iprintf("Creating public save file...\n");
						iprintf ("\n");
						fadeType = true;

						createDSiWareSave(ms().dsiWarePubPath.c_str(), NDSHeader.pubSavSize);

						iprintf("Public save file created!\n");

						for (int i = 0; i < 30; i++) {
							swiWaitForVBlank();
						}
						fadeType = false;
						for (int i = 0; i < 25; i++) {
							swiWaitForVBlank();
						}
					}

					if ((getFileSize(ms().dsiWarePrvPath.c_str()) == 0) && (NDSHeader.prvSavSize > 0)) {
						consoleDemoInit();
						iprintf("Creating private save file...\n");
						iprintf ("\n");
						fadeType = true;

						createDSiWareSave(ms().dsiWarePrvPath.c_str(), NDSHeader.prvSavSize);

						iprintf("Private save file created!\n");

						for (int i = 0; i < 30; i++) {
							swiWaitForVBlank();
						}
						fadeType = false;
						for (int i = 0; i < 25; i++) {
							swiWaitForVBlank();
						}
					}
				}

				if (!ms().ignoreBlacklists) {
					// TODO: If the list gets large enough, switch to bsearch().
					for (unsigned int i = 0; i < sizeof(cardReadDMAExcludeList)/sizeof(cardReadDMAExcludeList[0]); i++) {
						if (memcmp(NDSHeader.gameCode, cardReadDMAExcludeList[i], 3) == 0) {
							// Found match
							cardReadDMA = false;
							break;
						}
					}
				}

				bool useTempDSiWare = (ms().previousUsedDevice && !bs().b4dsMode && ms().dsiWareToSD && sdFound());

				if (useTempDSiWare || sys().isRunFromSD()) {
					argarray.push_back((char*)(useNightly ? "sd:/_nds/nds-bootstrap-nightly.nds" : "sd:/_nds/nds-bootstrap-release.nds"));
				} else {
					argarray.push_back((char*)(useNightly ? "fat:/_nds/nds-bootstrap-nightly.nds" : "fat:/_nds/nds-bootstrap-release.nds"));
				}

				char sfnSrl[62];
				char sfnPub[62];
				char sfnPrv[62];
				if (ms().previousUsedDevice && !bs().b4dsMode && ms().dsiWareToSD && sdFound()) {
					if (access("sd:/_nds/TWiLightMenu/tempDSiWare.pub.bak", F_OK) == 0) {
						if (access("sd:/_nds/TWiLightMenu/tempDSiWare.pub", F_OK) == 0) {
							remove("sd:/_nds/TWiLightMenu/tempDSiWare.pub");
						}
						rename("sd:/_nds/TWiLightMenu/tempDSiWare.pub.bak", "sd:/_nds/TWiLightMenu/tempDSiWare.pub");
					}
					if (access("sd:/_nds/TWiLightMenu/tempDSiWare.prv.bak", F_OK) == 0) {
						if (access("sd:/_nds/TWiLightMenu/tempDSiWare.prv", F_OK) == 0) {
							remove("sd:/_nds/TWiLightMenu/tempDSiWare.prv");
						}
						rename("sd:/_nds/TWiLightMenu/tempDSiWare.prv.bak", "sd:/_nds/TWiLightMenu/tempDSiWare.prv");
					}
					fatGetAliasPath("sd:/", "sd:/_nds/TWiLightMenu/tempDSiWare.dsi", sfnSrl);
					fatGetAliasPath("sd:/", "sd:/_nds/TWiLightMenu/tempDSiWare.pub", sfnPub);
					fatGetAliasPath("sd:/", "sd:/_nds/TWiLightMenu/tempDSiWare.prv", sfnPrv);
				} else {
					fatGetAliasPath(ms().previousUsedDevice ? "fat:/" : "sd:/", ms().dsiWareSrlPath.c_str(), sfnSrl);
					fatGetAliasPath(ms().previousUsedDevice ? "fat:/" : "sd:/", ms().dsiWarePubPath.c_str(), sfnPub);
					fatGetAliasPath(ms().previousUsedDevice ? "fat:/" : "sd:/", ms().dsiWarePrvPath.c_str(), sfnPrv);
				}

				CIniFile bootstrapini((useTempDSiWare || sys().isRunFromSD()) ? BOOTSTRAP_INI : BOOTSTRAP_INI_FC);
				bootstrapini.SetString("NDS-BOOTSTRAP", "NDS_PATH", ms().previousUsedDevice && !bs().b4dsMode && ms().dsiWareToSD && sdFound() ? "sd:/_nds/TWiLightMenu/tempDSiWare.dsi" : ms().dsiWareSrlPath);
				bootstrapini.SetString("NDS-BOOTSTRAP", "APP_PATH", sfnSrl);
				bootstrapini.SetString("NDS-BOOTSTRAP", "SAV_PATH", sfnPub);
				bootstrapini.SetString("NDS-BOOTSTRAP", "PRV_PATH", sfnPrv);
				bootstrapini.SetString("NDS-BOOTSTRAP", "AP_FIX_PATH", "");
				bootstrapini.SetString("NDS-BOOTSTRAP", "GUI_LANGUAGE", ms().getGuiLanguageString());
				bootstrapini.SetInt("NDS-BOOTSTRAP", "LANGUAGE",
					(perGameSettings_language == -2 ? ms().gameLanguage : perGameSettings_language));
				bootstrapini.SetInt("NDS-BOOTSTRAP", "REGION", (perGameSettings_region < -1 ? ms().gameRegion : perGameSettings_region));
				bootstrapini.SetInt("NDS-BOOTSTRAP", "USE_ROM_REGION", (perGameSettings_region < -1 ? ms().useRomRegion : 0));
				bootstrapini.SetInt("NDS-BOOTSTRAP", "DSI_MODE", true);
				bootstrapini.SetInt("NDS-BOOTSTRAP", "BOOST_CPU", true);
				bootstrapini.SetInt("NDS-BOOTSTRAP", "BOOST_VRAM", true);
				bootstrapini.SetInt("NDS-BOOTSTRAP", "CARD_READ_DMA", cardReadDMA);
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
				bootstrapini.SaveIniFile((useTempDSiWare || sys().isRunFromSD()) ? BOOTSTRAP_INI : BOOTSTRAP_INI_FC);

				if (!isDSiMode() && (!ms().previousUsedDevice || (ms().previousUsedDevice && ms().dsiWareToSD && sdFound()))) {
					*(bool*)(0x02000010) = useNightly;
					*(bool*)(0x02000014) = useWidescreen;
					*(u32*)0x02000000 |= BIT(4);
					ntrStartSdGame();
				}

				err = runNdsFile(argarray[0], argarray.size(), (const char **)&argarray[0], true, true, false, true, true, false, -1);
			}
		} else {
			// Move .pub and/or .prv out of "saves" folder
			std::string filename = ms().romPath[ms().previousUsedDevice];
			const size_t last_slash_idx = filename.find_last_of("/");
			if (std::string::npos != last_slash_idx) {
				filename.erase(0, last_slash_idx + 1);
			}

			loadPerGameSettings(filename);

			const char *typeToReplace = ".nds";
			if (extension(filename, ".dsi")) {
				typeToReplace = ".dsi";
			} else if (extension(filename, ".ids")) {
				typeToReplace = ".ids";
			} else if (extension(filename, ".srl")) {
				typeToReplace = ".srl";
			} else if (extension(filename, ".app")) {
				typeToReplace = ".app";
			}

			std::string pubname = replaceAll(filename, typeToReplace, getPubExtension());
			std::string prvname = replaceAll(filename, typeToReplace, getPrvExtension());
			std::string pubnameUl = replaceAll(filename, typeToReplace, ".pub");
			std::string prvnameUl = replaceAll(filename, typeToReplace, ".prv");
			std::string romfolder = ms().romPath[ms().previousUsedDevice];
			while (!romfolder.empty() && romfolder[romfolder.size()-1] != '/') {
				romfolder.resize(romfolder.size()-1);
			}
			std::string romFolderNoSlash = romfolder;
			RemoveTrailingSlashes(romFolderNoSlash);
			std::string pubpath = romFolderNoSlash + "/saves/" + pubname;
			std::string prvpath = romFolderNoSlash + "/saves/" + prvname;
			std::string pubpathUl = romFolderNoSlash + "/" + pubnameUl;
			std::string prvpathUl = romFolderNoSlash + "/" + prvnameUl;
			if (access(pubpath.c_str(), F_OK) == 0) {
				rename(pubpath.c_str(), pubpathUl.c_str());
			}
			if (access(prvpath.c_str(), F_OK) == 0) {
				rename(prvpath.c_str(), prvpathUl.c_str());
			}

			unlaunchRomBoot(ms().previousUsedDevice ? "sdmc:/_nds/TWiLightMenu/tempDSiWare.dsi" : ms().dsiWareSrlPath);
		}
	} else if (ms().launchType[ms().previousUsedDevice] == Launch::ENESDSLaunch) {
		if (access(ms().romPath[ms().previousUsedDevice].c_str(), F_OK) != 0) return;	// Skip to running TWiLight Menu++

		argarray.at(0) = (char*)"sd:/_nds/TWiLightMenu/emulators/nestwl.nds";
		if (!isDSiMode() || access(argarray[0], F_OK) != 0) {
			argarray.at(0) = (char*)"fat:/_nds/TWiLightMenu/emulators/nesds.nds";
		}
		err = runNdsFile(argarray[0], argarray.size(), (const char **)&argarray[0], true, true, false, true, true, false, -1); // Pass ROM to nesDS as argument
	} else if (ms().launchType[ms().previousUsedDevice] == Launch::EGameYobLaunch) {
		if (access(ms().romPath[ms().previousUsedDevice].c_str(), F_OK) != 0) return;	// Skip to running TWiLight Menu++

		argarray.at(0) = (char*)"sd:/_nds/TWiLightMenu/emulators/gameyob.nds";
		if (!isDSiMode() || access(argarray[0], F_OK) != 0) {
			argarray.at(0) = (char*)"fat:/_nds/TWiLightMenu/emulators/gameyob.nds";
		}
		err = runNdsFile(argarray[0], argarray.size(), (const char **)&argarray[0], true, true, false, true, true, false, -1); // Pass ROM to GameYob as argument
	} else if (ms().launchType[ms().previousUsedDevice] == Launch::ES8DSLaunch) {
		if ((extension(ms().romPath[ms().previousUsedDevice], ".col") && ms().colEmulator != 1)
		 || (extension(ms().romPath[ms().previousUsedDevice], ".sg") && ms().sgEmulator != 1)
		 || access(ms().romPath[ms().previousUsedDevice].c_str(), F_OK) != 0) return;	// Skip to running TWiLight Menu++

		mkdir(ms().previousUsedDevice ? "fat:/data" : "sd:/data", 0777);
		mkdir(ms().previousUsedDevice ? "fat:/data/s8ds" : "sd:/data/s8ds", 0777);

		argarray.at(0) = (char*)"sd:/_nds/TWiLightMenu/emulators/S8DS.nds";
		if (!isDSiMode() || access(argarray[0], F_OK) != 0) {
			argarray.at(0) = (char*)"fat:/_nds/TWiLightMenu/emulators/S8DS.nds";
		}
		err = runNdsFile(argarray[0], argarray.size(), (const char **)&argarray[0], true, true, false, true, true, false, -1); // Pass ROM to S8DS as argument
	} else if (ms().launchType[ms().previousUsedDevice] == Launch::ERVideoLaunch) {
		if (access(ms().romPath[ms().previousUsedDevice].c_str(), F_OK) != 0) return;	// Skip to running TWiLight Menu++

		argarray.at(0) = (char*)"sd:/_nds/TWiLightMenu/apps/RocketVideoPlayer.nds";
		if (!isDSiMode() || access(argarray[0], F_OK) != 0) {
			argarray.at(0) = (char*)"fat:/_nds/TWiLightMenu/apps/RocketVideoPlayer.nds";
		}
		err = runNdsFile(argarray[0], argarray.size(), (const char **)&argarray[0], true, true, false, true, true, false, -1); // Pass video to Rocket Video Player as argument
	} else if (ms().launchType[ms().previousUsedDevice] == Launch::EFastVideoLaunch) {
		if (access(ms().romPath[ms().previousUsedDevice].c_str(), F_OK) != 0) return;	// Skip to running TWiLight Menu++

		argarray.at(0) = (char*)"sd:/_nds/TWiLightMenu/apps/FastVideoDS.nds";
		if (!isDSiMode() || access(argarray[0], F_OK) != 0) {
			argarray.at(0) = (char*)"fat:/_nds/TWiLightMenu/apps/FastVideoDS.nds";
		}
		err = runNdsFile(argarray[0], argarray.size(), (const char **)&argarray[0], true, true, false, true, true, false, -1); // Pass video to FastVideoDS as argument
	} else if (ms().launchType[ms().previousUsedDevice] == Launch::EStellaDSLaunch) {
		if (access(ms().romPath[ms().previousUsedDevice].c_str(), F_OK) != 0) return;	// Skip to running TWiLight Menu++

		argarray.at(0) = (char*)"sd:/_nds/TWiLightMenu/emulators/StellaDS.nds";
		if (!isDSiMode() || access(argarray[0], F_OK) != 0) {
			argarray.at(0) = (char*)"fat:/_nds/TWiLightMenu/emulators/StellaDS.nds";
		}
		err = runNdsFile(argarray[0], argarray.size(), (const char **)&argarray[0], true, true, false, true, true, false, -1); // Pass ROM to StellaDS as argument
	} else if (ms().launchType[ms().previousUsedDevice] == Launch::EPicoDriveTWLLaunch) {
		if (ms().mdEmulator >= 2 || access(ms().romPath[ms().previousUsedDevice].c_str(), F_OK) != 0) return;	// Skip to running TWiLight Menu++

		argarray.at(0) = (char*)"sd:/_nds/TWiLightMenu/emulators/PicoDriveTWL.nds";
		if (!isDSiMode() || access(argarray[0], F_OK) != 0) {
			argarray.at(0) = (char*)"fat:/_nds/TWiLightMenu/emulators/PicoDriveTWL.nds";
		}
		err = runNdsFile(argarray[0], argarray.size(), (const char **)&argarray[0], true, true, false, true, true, false, -1); // Pass ROM to PicoDrive TWL as argument
	} else if (ms().launchType[ms().previousUsedDevice] == Launch::EGBANativeLaunch) {
		if (!sys().isRegularDS() || *(u16*)(0x020000C0) == 0 || (ms().gbaBooter != 1) || access(ms().romPath[true].c_str(), F_OK) != 0) return;	// Skip to running TWiLight Menu++

		std::string savepath = replaceAll(ms().romPath[true], ".gba", ".sav");
		u32 romSize = getFileSize(ms().romPath[true].c_str());
		if (romSize > 0x2000000) romSize = 0x2000000;
		u32 savesize = getFileSize(savepath.c_str());
		if (savesize > 0x20000) savesize = 0x20000;

		consoleDemoInit();
		iprintf("Now Loading...\n");
		iprintf("\n");
		fadeType = true;

		u32 ptr = 0x08000000;
		char titleID[4];
		FILE* gbaFile = fopen(ms().romPath[true].c_str(), "rb");
		fseek(gbaFile, 0xAC, SEEK_SET);
		fread(&titleID, 1, 4, gbaFile);
		if (strncmp(titleID, "AGBJ", 4) == 0 && romSize <= 0x40000) {
			ptr += 0x400;
		}
		fseek(gbaFile, 0, SEEK_SET);

		extern char copyBuf[0x8000];
		bool nor = false;
		if (*(u16*)(0x020000C0) == 0x5A45 && strncmp(titleID, "AGBJ", 4) != 0) {
			cExpansion::SetRompage(0);
			expansion().SetRampage(cExpansion::ENorPage);
			cExpansion::OpenNorWrite();
			cExpansion::SetSerialMode();
			nor = true;
		} else if (*(u16*)(0x020000C0) == 0x4353 && romSize > 0x1FFFFFE) {
			romSize = 0x1FFFFFE;
		}

		if (!nor) {
			for (u32 len = romSize; len > 0; len -= 0x8000) {
				if (fread(&copyBuf, 1, (len>0x8000 ? 0x8000 : len), gbaFile) > 0) {
					s2RamAccess(true);
					tonccpy((u16*)ptr, &copyBuf, (len>0x8000 ? 0x8000 : len));
					s2RamAccess(false);
					ptr += 0x8000;
				} else {
					break;
				}
			}
			fclose(gbaFile);
		}

		ptr = 0x0A000000;

		if (savesize > 0) {
			FILE* savFile = fopen(savepath.c_str(), "rb");
			for (u32 len = (savesize > 0x10000 ? 0x10000 : savesize); len > 0; len -= 0x8000) {
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

		fadeType = false;
		for (int i = 0; i < 25; i++) {
			swiWaitForVBlank();
		}

		argarray.at(0) = (char*)"fat:/_nds/TWiLightMenu/gbapatcher.srldr";
		err = runNdsFile(argarray[0], argarray.size(), (const char **)&argarray[0], true, true, false, true, true, false, -1);
	} else if (ms().launchType[ms().previousUsedDevice] == Launch::EA7800DSLaunch) {
		if (access(ms().romPath[ms().previousUsedDevice].c_str(), F_OK) != 0) return;	// Skip to running TWiLight Menu++

		argarray.at(0) = (char*)"sd:/_nds/TWiLightMenu/emulators/A7800DS.nds";
		if (!isDSiMode() || access(argarray[0], F_OK) != 0) {
			argarray.at(0) = (char*)"fat:/_nds/TWiLightMenu/emulators/A7800DS.nds";
		}
		err = runNdsFile(argarray[0], argarray.size(), (const char **)&argarray[0], true, true, false, true, true, false, -1); // Pass ROM to A7800DS as argument
	} else if (ms().launchType[ms().previousUsedDevice] == Launch::EA5200DSLaunch) {
		if (access(ms().romPath[ms().previousUsedDevice].c_str(), F_OK) != 0) return;	// Skip to running TWiLight Menu++

		argarray.at(0) = (char*)"sd:/_nds/TWiLightMenu/emulators/A5200DS.nds";
		if (!isDSiMode() || access(argarray[0], F_OK) != 0) {
			argarray.at(0) = (char*)"fat:/_nds/TWiLightMenu/emulators/A5200DS.nds";
		}
		err = runNdsFile(argarray[0], argarray.size(), (const char **)&argarray[0], true, true, false, true, true, false, -1); // Pass ROM to A5200DS as argument
	} else if (ms().launchType[ms().previousUsedDevice] == Launch::ENitroGrafxLaunch) {
		if (access(ms().romPath[ms().previousUsedDevice].c_str(), F_OK) != 0) return;	// Skip to running TWiLight Menu++

		mkdir(ms().previousUsedDevice ? "fat:/data" : "sd:/data", 0777);
		mkdir(ms().previousUsedDevice ? "fat:/data/NitroGrafx" : "sd:/data/NitroGrafx", 0777);

		argarray.at(0) = (char*)"sd:/_nds/TWiLightMenu/emulators/NitroGrafx.nds";
		if (!isDSiMode() || access(argarray[0], F_OK) != 0) {
			argarray.at(0) = (char*)"fat:/_nds/TWiLightMenu/emulators/NitroGrafx.nds";
		}
		err = runNdsFile(argarray[0], argarray.size(), (const char **)&argarray[0], true, true, false, true, true, false, -1); // Pass ROM to NitroGrafx as argument
	} else if (ms().launchType[ms().previousUsedDevice] == Launch::EXEGSDSLaunch) {
		if (access(ms().romPath[ms().previousUsedDevice].c_str(), F_OK) != 0) return;	// Skip to running TWiLight Menu++

		argarray.at(0) = (char*)"sd:/_nds/TWiLightMenu/emulators/XEGS-DS.nds";
		if (!isDSiMode() || access(argarray[0], F_OK) != 0) {
			argarray.at(0) = (char*)"fat:/_nds/TWiLightMenu/emulators/XEGS-DS.nds";
		}
		err = runNdsFile(argarray[0], argarray.size(), (const char **)&argarray[0], true, true, false, true, true, false, -1); // Pass ROM to XEGS-DS as argument
	} else if (ms().launchType[ms().previousUsedDevice] == Launch::ENINTVDSLaunch) {
		if (access(ms().romPath[ms().previousUsedDevice].c_str(), F_OK) != 0) return;	// Skip to running TWiLight Menu++

		argarray.at(0) = (char*)"sd:/_nds/TWiLightMenu/emulators/NINTV-DS.nds";
		if (!isDSiMode() || access(argarray[0], F_OK) != 0) {
			argarray.at(0) = (char*)"fat:/_nds/TWiLightMenu/emulators/NINTV-DS.nds";
		}
		err = runNdsFile(argarray[0], argarray.size(), (const char **)&argarray[0], true, true, false, true, true, false, -1); // Pass ROM to NINTV-DS as argument
	} else if (ms().launchType[ms().previousUsedDevice] == Launch::EGBARunner2Launch) {
		if (access(ms().romPath[ms().previousUsedDevice].c_str(), F_OK) != 0) return;	// Skip to running TWiLight Menu++

		argarray.at(0) = (char*)(ms().gbar2DldiAccess ? "sd:/_nds/GBARunner2_arm7dldi_ds.nds" : "sd:/_nds/GBARunner2_arm9dldi_ds.nds");
		if (isDSiMode() || REG_SCFG_EXT != 0) {
			argarray.at(0) = (char*)(ms().consoleModel > 0 ? "sd:/_nds/GBARunner2_arm7dldi_3ds.nds" : "sd:/_nds/GBARunner2_arm7dldi_dsi.nds");
		}
		if (!isDSiMode() || access(argarray[0], F_OK) != 0) {
			argarray.at(0) = (char*)(ms().gbar2DldiAccess ? "fat:/_nds/GBARunner2_arm7dldi_ds.nds" : "fat:/_nds/GBARunner2_arm9dldi_ds.nds");
			if (isDSiMode() || REG_SCFG_EXT != 0) {
				argarray.at(0) = (char*)(ms().consoleModel > 0 ? "fat:/_nds/GBARunner2_arm7dldi_3ds.nds" : "fat:/_nds/GBARunner2_arm7dldi_dsi.nds");
			}
		}
		err = runNdsFile(argarray[0], argarray.size(), (const char **)&argarray[0], true, true, false, true, true, false, -1); // Pass ROM to GBARunner2 as argument
	} else if (ms().launchType[ms().previousUsedDevice] == Launch::EColecoDSLaunch) {
		if ((extension(ms().romPath[ms().previousUsedDevice], ".col") && ms().colEmulator != 2)
		 || (extension(ms().romPath[ms().previousUsedDevice], ".sg") && ms().sgEmulator != 2)
		 || access(ms().romPath[ms().previousUsedDevice].c_str(), F_OK) != 0) return;	// Skip to running TWiLight Menu++

		argarray.at(0) = (char*)"sd:/_nds/TWiLightMenu/emulators/ColecoDS.nds";
		if (!isDSiMode() || access(argarray[0], F_OK) != 0) {
			argarray.at(0) = (char*)"fat:/_nds/TWiLightMenu/emulators/ColecoDS.nds";
		}
		err = runNdsFile(argarray[0], argarray.size(), (const char **)&argarray[0], true, true, false, true, true, false, -1); // Pass ROM to NINTV-DS as argument
	} else if (ms().launchType[ms().previousUsedDevice] == Launch::ENitroSwanLaunch) {
		if (access(ms().romPath[ms().previousUsedDevice].c_str(), F_OK) != 0) return;	// Skip to running TWiLight Menu++

		mkdir(ms().previousUsedDevice ? "fat:/data" : "sd:/data", 0777);
		mkdir(ms().previousUsedDevice ? "fat:/data/nitroswan" : "sd:/data/nitroswan", 0777);

		argarray.at(0) = (char*)"sd:/_nds/TWiLightMenu/emulators/NitroSwan.nds";
		if (!isDSiMode() || access(argarray[0], F_OK) != 0) {
			argarray.at(0) = (char*)"fat:/_nds/TWiLightMenu/emulators/NitroSwan.nds";
		}
		err = runNdsFile(argarray[0], argarray.size(), (const char **)&argarray[0], true, true, false, true, true, false, -1); // Pass ROM to NINTV-DS as argument
	} else if (ms().launchType[ms().previousUsedDevice] == Launch::ENGPDSLaunch) {
		if (access(ms().romPath[ms().previousUsedDevice].c_str(), F_OK) != 0) return;	// Skip to running TWiLight Menu++

		mkdir(ms().previousUsedDevice ? "fat:/data" : "sd:/data", 0777);
		mkdir(ms().previousUsedDevice ? "fat:/data/ngpds" : "sd:/data/ngpds", 0777);

		argarray.at(0) = (char*)"sd:/_nds/TWiLightMenu/emulators/NGPDS.nds";
		if (!isDSiMode() || access(argarray[0], F_OK) != 0) {
			argarray.at(0) = (char*)"fat:/_nds/TWiLightMenu/emulators/NGPDS.nds";
		}
		err = runNdsFile(argarray[0], argarray.size(), (const char **)&argarray[0], true, true, false, true, true, false, -1); // Pass ROM to NINTV-DS as argument
	} else if (ms().launchType[ms().previousUsedDevice] == Launch::ESNEmulDSLaunch) {
		if (access(ms().romPath[ms().previousUsedDevice].c_str(), F_OK) != 0) return;	// Skip to running TWiLight Menu++

		const char* ndsToBoot = (char*)"sd:/_nds/TWiLightMenu/emulators/SNEmulDS.srl";
		if (!isDSiMode() || access(ndsToBoot, F_OK) != 0) {
			ndsToBoot = (char*)"fat:/_nds/TWiLightMenu/emulators/SNEmulDS.nds";
		}
		argarray.at(0) = (char*)"fat:/SNEmulDS.srl";
		err = runNdsFile(ndsToBoot, argarray.size(), (const char **)&argarray[0], true, true, false, true, true, true, -1); // Pass ROM to SNEmulDS as argument
	} else if (ms().launchType[ms().previousUsedDevice] == Launch::EAmEDSLaunch) {
		if (access(ms().romPath[ms().previousUsedDevice].c_str(), F_OK) != 0) return;	// Skip to running TWiLight Menu++

		argarray.at(0) = (char*)"sd:/_nds/TWiLightMenu/emulators/AmEDS.nds";
		if (!isDSiMode() || access(argarray[0], F_OK) != 0) {
			argarray.at(0) = (char*)"fat:/_nds/TWiLightMenu/emulators/AmEDS.nds";
		}
		err = runNdsFile(argarray[0], argarray.size(), (const char **)&argarray[0], true, true, false, true, true, false, -1); // Pass ROM to AmEDS as argument
	} else if (ms().launchType[ms().previousUsedDevice] == Launch::ECrocoDSLaunch) {
		if (access(ms().romPath[ms().previousUsedDevice].c_str(), F_OK) != 0) return;	// Skip to running TWiLight Menu++

		argarray.at(0) = (char*)"sd:/_nds/TWiLightMenu/emulators/CrocoDS.nds";
		if (!isDSiMode() || access(argarray[0], F_OK) != 0) {
			argarray.at(0) = (char*)"fat:/_nds/TWiLightMenu/emulators/CrocoDS.nds";
		}
		err = runNdsFile(argarray[0], argarray.size(), (const char **)&argarray[0], true, true, false, true, true, false, -1); // Pass ROM to CrocoDS as argument
	} else if (ms().launchType[ms().previousUsedDevice] == Launch::ETunaViDSLaunch) {
		if (access(ms().romPath[ms().previousUsedDevice].c_str(), F_OK) != 0) return;	// Skip to running TWiLight Menu++

		argarray.at(0) = (char*)"sd:/_nds/TWiLightMenu/apps/tuna-vids.nds";
		if (!isDSiMode() || access(argarray[0], F_OK) != 0) {
			argarray.at(0) = (char*)"fat:/_nds/TWiLightMenu/apps/tuna-vids.nds";
		}
		err = runNdsFile(argarray[0], argarray.size(), (const char **)&argarray[0], true, true, false, true, true, false, -1); // Pass video to tuna-viDS as argument
	} else if (ms().launchType[ms().previousUsedDevice] == Launch::EImageLaunch) {
		if (access(ms().romPath[ms().previousUsedDevice].c_str(), F_OK) != 0) return;	// Skip to running TWiLight Menu++

		argarray.at(0) = (char*)"sd:/_nds/TWiLightMenu/imageview.srldr";
		if (!isDSiMode() || access(argarray[0], F_OK) != 0) {
			argarray.at(0) = (char*)"fat:/_nds/TWiLightMenu/imageview.srldr";
		}
		err = runNdsFile(argarray[0], argarray.size(), (const char **)&argarray[0], true, true, false, true, true, false, -1); // Pass image to image viewer as argument
	} else if (ms().launchType[ms().previousUsedDevice] == Launch::E3DSLaunch) {
		if (!dsiFeatures() || access(ms().romPath[ms().previousUsedDevice].c_str(), F_OK) != 0) return;	// Skip to running TWiLight Menu++

		argarray.at(0) = (char*)"sd:/_nds/TWiLightMenu/3dssplash.srldr";
		if (!isDSiMode() || access(argarray[0], F_OK) != 0) {
			argarray.at(0) = (char*)"fat:/_nds/TWiLightMenu/3dssplash.srldr";
		}
		err = runNdsFile(argarray[0], argarray.size(), (const char **)&argarray[0], true, true, false, true, true, false, -1); // Pass ROM to 3DS Splash as argument (Does nothing)
	}
	if (err > 0) {
		consoleDemoInit();
		iprintf("Start failed. Error %i\n", err);
		fadeType = true;
		for (int i = 0; i < 60*3; i++) {
			swiWaitForVBlank();
		}
		fadeType = false;
		for (int i = 0; i < 25; i++) {
			swiWaitForVBlank();
		}
	}
}

bool graphicsInited = false;
void graphicsInit(void) {
	if (graphicsInited)
		return;

	graphicsInited = true;

	videoSetMode(MODE_5_2D);
	videoSetModeSub(MODE_5_2D);

	vramSetBankA(VRAM_A_MAIN_BG);
	vramSetBankC(VRAM_C_SUB_BG);

	bgInit(3, BgType_Bmp8, BgSize_B8_256x256, 0, 0);
	bgSetPriority(3, 3);
	bgInit(2, BgType_Bmp8, BgSize_B8_256x256, 3, 0);
	bgSetPriority(2, 2);
	bgInitSub(3, BgType_Bmp8, BgSize_B8_256x256, 0, 0);
	bgSetPriority(7, 3);
	bgInitSub(2, BgType_Bmp8, BgSize_B8_256x256, 3, 0);
	bgSetPriority(6, 2);

	BG_PALETTE[0x10] = 0xFFFF;
	BG_PALETTE_SUB[0x10] = 0xFFFF;
	toncset16(bgGetGfxPtr(3), 0x1010, 256 * 192);
	toncset16(bgGetGfxPtr(7), 0x1010, 256 * 192);

	fontInit();
}

void resetSettingsPrompt(void) {
	TWLSettings settings;
	settings.loadSettings();

	langInit(settings.getGuiLanguageString().c_str());

	graphicsInit();

	fadeType = true;

	Alignment align = ms().rtl() ? Alignment::right : Alignment::left;
	int x = ms().rtl() ? 256 - 2 : 2;

	clearText();
	printLarge(false, x, 0, STR_RESET_TWILIGHT_SETTINGS, align);
	int y = calcLargeFontHeight(STR_RESET_TWILIGHT_SETTINGS);
	printSmall(false, x, y, STR_PGS_WILL_BE_KEPT, align);

	printSmall(false, x, y + 20, STR_A_YES, align);
	printSmall(false, x, y + 20 + 14, STR_B_NO, align);

	updateText(false);

	u16 pressed = 0;
	do {
		swiWaitForVBlank();
		scanKeys();
		pressed = keysDown();
	} while (!(pressed & (KEY_A | KEY_B)));

	if (pressed & KEY_A) {
		remove(settingsinipath);	// Delete "settings.ini"
	}


	fadeType = false;
	for (int i = 0; i < 30; i++)
		swiWaitForVBlank();

	clearText();
	updateText(false);
}

static bool languageNowSet = false;
static bool regionNowSet = false;

const char *languages[] = {
	"日本語",
	"English",
	"Français",
	"Deutsch",
	"Italiano",
	"Español",
	"中文 (简体)",
	"한국어",
	"中文 (繁體)",
	"Polski",
	"Português (Portugal)",
	"Русский",
	"Svenska",
	"Dansk",
	"Türkçe",
	"Українська",
	"Magyar",
	"Norsk",
	"עברית",
	"Nederlands",
	"Bahasa Indonesia",
	"Ελληνικά",
	"Български",
	"Română",
	"العربية",
	"Português (Brasil)",
	"Tiếng Việt",
	"Valencià",
	"Català",
	"琉球諸語",
};

const TWLSettings::TLanguage guiLanguages[] = {
	TWLSettings::TLanguage::ELangIndonesian,
	TWLSettings::TLanguage::ELangCatalan,
	TWLSettings::TLanguage::ELangDanish,
	TWLSettings::TLanguage::ELangGerman,
	TWLSettings::TLanguage::ELangEnglish,
	TWLSettings::TLanguage::ELangSpanish,
	TWLSettings::TLanguage::ELangFrench,
	TWLSettings::TLanguage::ELangItalian,
	TWLSettings::TLanguage::ELangHungarian,
	TWLSettings::TLanguage::ELangDutch,
	TWLSettings::TLanguage::ELangNorwegian,
	TWLSettings::TLanguage::ELangPolish,
	TWLSettings::TLanguage::ELangPortugueseBrazil,
	TWLSettings::TLanguage::ELangPortuguese,
	TWLSettings::TLanguage::ELangRomanian,
	TWLSettings::TLanguage::ELangSwedish,
	TWLSettings::TLanguage::ELangVietnamese,
	TWLSettings::TLanguage::ELangTurkish,
	TWLSettings::TLanguage::ELangValencian,
	TWLSettings::TLanguage::ELangGreek,
	TWLSettings::TLanguage::ELangBulgarian,
	TWLSettings::TLanguage::ELangRussian,
	TWLSettings::TLanguage::ELangUkrainian,
	TWLSettings::TLanguage::ELangHebrew,
	TWLSettings::TLanguage::ELangArabic,
	TWLSettings::TLanguage::ELangChineseS,
	TWLSettings::TLanguage::ELangChineseT,
	TWLSettings::TLanguage::ELangJapanese,
	TWLSettings::TLanguage::ELangRyukyuan,
	TWLSettings::TLanguage::ELangKorean,
};

const TWLSettings::TLanguage gameLanguages[] = {
	TWLSettings::TLanguage::ELangGerman,
	TWLSettings::TLanguage::ELangEnglish,
	TWLSettings::TLanguage::ELangSpanish,
	TWLSettings::TLanguage::ELangFrench,
	TWLSettings::TLanguage::ELangItalian,
	TWLSettings::TLanguage::ELangChineseS,
	TWLSettings::TLanguage::ELangJapanese,
	TWLSettings::TLanguage::ELangKorean
};
const TWLSettings::TLanguage titleLanguages[] = {
	TWLSettings::TLanguage::ELangGerman,
	TWLSettings::TLanguage::ELangEnglish,
	TWLSettings::TLanguage::ELangSpanish,
	TWLSettings::TLanguage::ELangFrench,
	TWLSettings::TLanguage::ELangItalian,
	TWLSettings::TLanguage::ELangJapanese
};

static const char* displayLanguage(int l, int type) {
	if (l == -1) {
		return STR_SYSTEM.c_str();
	} else {
		switch(type) {
			case 0:
			default:
				return languages[guiLanguages[l]];
			case 1:
				return languages[gameLanguages[l]];
			case 2:
				return languages[titleLanguages[l]];
		}
	}
}

void languageSelect(void) {
	graphicsInit();
	langInit();

	fadeType = true;

	int guiLanguage = -1, gameLanguage = -1, titleLanguage = -1;
	for (uint i = 0; i < sizeof(guiLanguages) / sizeof(guiLanguages[0]); i++) {
		if (guiLanguages[i] == ms().guiLanguage)
			guiLanguage = i;
	}
	for (uint i = 0; i < sizeof(gameLanguages) / sizeof(gameLanguages[0]); i++) {
		if (gameLanguages[i] == ms().gameLanguage)
			gameLanguage = i;
	}
	for (uint i = 0; i < sizeof(titleLanguages) / sizeof(titleLanguages[0]); i++) {
		if (titleLanguages[i] == ms().titleLanguage)
			titleLanguage = i;
	}

	printSmall(true, 2, 4, "\uE07Eを使用して言語を選択してください。");
	printSmall(true, 2, 28, "Select your language with \uE07E.");
	printSmall(true, 2, 52, "Sélectionnez votre langage avec \uE07E.");
	printSmall(true, 2, 76, "Wähle deine Sprache mit \uE07E.");
	printSmall(true, 2, 100, "Seleziona la tua lingua con \uE07E.");
	printSmall(true, 2, 124, "Selecciona tu idioma con \uE07E.");
	printSmall(true, 2, 148, "使用 \uE07E 选择你的语言。");
	printSmall(true, 2, 172, "\uE07E를 사용하여 언어를 선택해주세요.");
	updateText(true);

	if (ms().macroMode) {
		u16 pressed = 0;
		for (int i = 0; i < 60*10; i++) {
			swiWaitForVBlank();
			scanKeys();
			pressed = keysDown();
			if ((pressed & KEY_A) || (pressed & KEY_START) || (pressed & KEY_TOUCH)) break;
		}
		fadeType = false;
		for (int i = 0; i < 25; i++)
			swiWaitForVBlank();

		fadeType = true;
	}

	int cursorPosition = 0;
	char buffer[64] = {0};
	u16 held = 0, pressed = 0;
	while (1) {
		clearText(ms().macroMode);

		Alignment align = ms().rtl() ? Alignment::right : Alignment::left;
		int x1 = ms().rtl() ? 256 - 2 : 2, x2 = ms().rtl() ? 256 - 15 : 15;

		printLarge(ms().macroMode, x1, 0, STR_SELECT_YOUR_LANGUAGE, align);

		snprintf(buffer, sizeof(buffer), STR_GUI.c_str(), displayLanguage(guiLanguage, 0));
		printSmall(ms().macroMode, x2, 20, buffer, align);
		snprintf(buffer, sizeof(buffer), STR_GAME.c_str(), displayLanguage(gameLanguage, 1));
		printSmall(ms().macroMode, x2, 34, buffer, align);
		snprintf(buffer, sizeof(buffer), STR_DS_BANNER_TITLE.c_str(), displayLanguage(titleLanguage, 2));
		printSmall(ms().macroMode, x2, 48, buffer, align);

		printSmall(ms().macroMode, x1, 20 + cursorPosition * 14, ms().rtl() ? "<" : ">", align);

		printSmall(ms().macroMode, x1, 68, STR_UP_DOWN_CHOOSE, align);
		printSmall(ms().macroMode, x1, 82, STR_LEFT_RIGHT_CHANGE_LANGUAGE, align);
		printSmall(ms().macroMode, x1, 96, STR_A_PROCEED, align);

		updateText(ms().macroMode);

		do {
			swiWaitForVBlank();
			scanKeys();
			pressed = keysDown();
			held = keysDownRepeat();
		} while (!held);

		if (held & KEY_UP) {
			if (cursorPosition > 0)
				cursorPosition--;
		} else if (held & KEY_DOWN) {
			if (cursorPosition < 2)
				cursorPosition++;
		} else if (held & KEY_LEFT) {
			switch (cursorPosition) {
				case 0:
					if (guiLanguage > -1) {
						guiLanguage--;
						ms().guiLanguage = guiLanguage == -1 ? TWLSettings::ELangDefault : guiLanguages[guiLanguage];
						ms().currentLanguage = ms().guiLanguage;
						langInit();
					}
					break;
				case 1:
					if (gameLanguage > -1)
						gameLanguage--;
					break;
				case 2:
					if (titleLanguage > -1)
						titleLanguage--;
					break;
			}
		} else if (held & KEY_RIGHT) {
			switch (cursorPosition) {
				case 0:
					if (guiLanguage < (int)(sizeof(languages) / sizeof(languages[0])) - 1) {
						guiLanguage++;
						ms().guiLanguage = guiLanguage == -1 ? TWLSettings::ELangDefault : guiLanguages[guiLanguage];
						ms().currentLanguage = ms().guiLanguage;
						langInit();
					}
					break;
				case 1:
					if (gameLanguage < 7)
						gameLanguage++;
					break;
				case 2:
					if (titleLanguage < 5)
						titleLanguage++;
					break;
			}
		}

		if (pressed & KEY_A) {
			ms().gameLanguage = gameLanguage == -1 ? TWLSettings::ELangDefault : gameLanguages[gameLanguage];
			ms().titleLanguage = titleLanguage == -1 ? TWLSettings::ELangDefault : titleLanguages[titleLanguage];
			ms().languageSet = true;
			languageNowSet = true;
			break;
		}
	}

	fadeType = false;
	for (int i = 0; i < 30; i++)
		swiWaitForVBlank();

	clearText();
	updateText(true);
	updateText(false);
}

const std::string *regions[] {
	&STR_SYSTEM,
	&STR_JAPAN,
	&STR_USA,
	&STR_EUROPE,
	&STR_AUSTRALIA,
	&STR_CHINA,
	&STR_KOREA
};

void regionSelect(void) {
	graphicsInit();
	langInit();

	fadeType = true;

	Alignment align = ms().rtl() ? Alignment::right : Alignment::left;
	int x1 = ms().rtl() ? 256 - 2 : 2, x2 = ms().rtl() ? 256 - 15 : 15;

	u16 held = 0, pressed = 0;
	while (1) {
		clearText();
		printLarge(ms().macroMode, x1, 0, STR_SELECT_YOUR_REGION, align);

		for (uint i = dsiFeatures() ? 0 : 1, p = 0; i < sizeof(regions) / sizeof(regions[0]); i++, p++) {
			printSmall(ms().macroMode, x2, 20 + p * 14, *regions[i], align);
		}

		printSmall(ms().macroMode, x1, 20 + (ms().gameRegion + (dsiFeatures() ? 1 : 0)) * 14, ms().rtl() ? "<" : ">", align);

		int y = 20 + (sizeof(regions) / sizeof(regions[0])) * 14 + 6 - (dsiFeatures() ? 0 : 14);
		printSmall(ms().macroMode, x1, y, STR_UP_DOWN_CHOOSE, align);
		printSmall(ms().macroMode, x1, y + 14, STR_A_PROCEED, align);

		updateText(ms().macroMode);

		do {
			swiWaitForVBlank();
			scanKeys();
			pressed = keysDown();
			held = keysDownRepeat();
		} while (!held);

		if (held & KEY_UP) {
			if (ms().gameRegion > (dsiFeatures() ? -1 : 0))
				ms().gameRegion = (TWLSettings::TRegion)((int)ms().gameRegion - 1);
		} else if (held & KEY_DOWN) {
			if (ms().gameRegion < (int)(sizeof(regions) / sizeof(regions[0])) - 2)
				ms().gameRegion = (TWLSettings::TRegion)((int)ms().gameRegion + 1);
		}

		if (pressed & KEY_A) {
			ms().regionSet = true;
			regionNowSet = true;
			break;
		}
	}


	fadeType = false;
	for (int i = 0; i < 30; i++)
		swiWaitForVBlank();

	clearText();
	updateText(ms().macroMode);
}

//---------------------------------------------------------------------------------
int titleMode(void)
{
//---------------------------------------------------------------------------------
	keysSetRepeat(25, 5);

	*(u32*)0x02FFFDFC = 0; // Reset TWLCFG location

	u32 softResetParamsBak = 0;
	if (*(u32*)(0x02000004) != 0) {
		*(u32*)(0x02000000) = 0; // Clear soft-reset params
	} else {
		softResetParamsBak = *(u32*)(0x02000000);
	}
	// Soft-reset parameters
	/*
		0: Skip DS(i) splash
		1: Skip TWLMenu++ splash
		2: Skip last-run game
		3: Auto-start ROM
		4: Run temporary DSiWare
	*/

	if (isDSiMode() && sdFound()) {
		if ((access("sd:/_nds/bios9i.bin", F_OK) != 0) && (access("sd:/_nds/bios9i_part1.bin", F_OK) != 0)) {
			extern char copyBuf[0x8000];
			tonccpy(copyBuf, (u8*)0xFFFF0000, 0x8000);

			FILE* bios = fopen("sd:/_nds/bios9i_part1.bin", "wb");
			fwrite(copyBuf, 1, 0x8000, bios);
			fclose(bios);
		}
		if ((access("sd:/_nds/bios7i.bin", F_OK) != 0) && (access("sd:/_nds/bios7i_part1.bin", F_OK) != 0)) {
			FILE* bios = fopen("sd:/_nds/bios7i_part1.bin", "wb");
			fwrite((char*)0x0CF80000, 1, 0x8000, bios);
			fclose(bios);
		}
	} else if (sys().isRegularDS()) {
		sysSetCartOwner(BUS_OWNER_ARM9); // Allow arm9 to access GBA ROM
		if (*(u16*)(0x020000C0) != 0x334D && *(u16*)(0x020000C0) != 0x3647 && *(u16*)(0x020000C0) != 0x4353 && *(u16*)(0x020000C0) != 0x5A45) {
			*(u16*)(0x020000C0) = 0;	// Clear Slot-2 flashcard flag
		}

		if (*(u16*)(0x020000C0) == 0) {
		  if (io_dldi_data->ioInterface.features & FEATURE_SLOT_NDS) {
			*(vu16*)(0x08000000) = 0x4D54;	// Write test
			if (*(vu16*)(0x08000000) != 0x4D54) {	// If not writeable
				_M3_changeMode(M3_MODE_RAM);	// Try again with M3
				*(u16*)(0x020000C0) = 0x334D;
				*(vu16*)(0x08000000) = 0x4D54;
			}
			if (*(vu16*)(0x08000000) != 0x4D54) {
				_G6_SelectOperation(G6_MODE_RAM);	// Try again with G6
				*(u16*)(0x020000C0) = 0x3647;
				*(vu16*)(0x08000000) = 0x4D54;
			}
			if (*(vu16*)(0x08000000) != 0x4D54) {
				_SC_changeMode(SC_MODE_RAM);	// Try again with SuperCard
				*(u16*)(0x020000C0) = 0x4353;
				*(vu16*)(0x08000000) = 0x4D54;
			}
			if (*(vu16*)(0x08000000) != 0x4D54) {
				TWLSettings settings;
				settings.loadSettings();

				if (settings.ezFlashRam) {
					cExpansion::SetRompage(381);	// Try again with EZ Flash
					cExpansion::OpenNorWrite();
					cExpansion::SetSerialMode();
					*(u16*)(0x020000C0) = 0x5A45;
					*(vu16*)(0x08000000) = 0x4D54;
				}
			}
			if (*(vu16*)(0x08000000) != 0x4D54) {
				*(u16*)(0x020000C0) = 0;
			}
		  } else if (io_dldi_data->ioInterface.features & FEATURE_SLOT_GBA) {
			if (memcmp(io_dldi_data->friendlyName, "M3 Adapter", 10) == 0) {
				*(u16*)(0x020000C0) = 0x334D;
			} else if (memcmp(io_dldi_data->friendlyName, "G6", 2) == 0) {
				*(u16*)(0x020000C0) = 0x3647;
			} else if (memcmp(io_dldi_data->friendlyName, "SuperCard", 9) == 0) {
				*(u16*)(0x020000C0) = 0x4353;
			}
		  }
		}
	}

	bool is3DS = fifoGetValue32(FIFO_USER_05) != 0xD2;

	useTwlCfg = (REG_SCFG_EXT!=0 && (*(u8*)0x02000400 != 0) && (*(u8*)0x02000401 == 0) && (*(u8*)0x02000402 == 0) && (*(u8*)0x02000404 == 0) && (*(u8*)0x02000448 != 0));
	bool nandMounted = false;
	if (REG_SCFG_EXT != 0) {
		const char* cachePath = sys().isRunFromSD() ? "sd:/_nds/TWiLightMenu/16KBcache.bin" : "fat:/_nds/TWiLightMenu/16KBcache.bin";
		if (!useTwlCfg && isDSiMode() && sdFound() && !is3DS) {
			bool hiyaFound = (access("sd:/hiya.dsi", F_OK) == 0 && access("sd:/shared1/TWLCFG0.dat", F_OK) == 0 && access("sd:/sys/HWINFO_N.dat", F_OK) == 0 && !sys().arm7SCFGLocked());
			if (!hiyaFound) {
				// Mount NAND, if not using hiyaCFW
				nandMounted = fatMountSimple("nand", &io_dsi_nand);
			}

			if (nandMounted || hiyaFound) {
				//toncset((void*)0x02000004, 0, 0x3FFC); // Already done by exploit

				FILE* twlCfgFile = fopen(nandMounted ? "nand:/shared1/TWLCFG0.dat" : "sd:/shared1/TWLCFG0.dat", "rb");
				fseek(twlCfgFile, 0x88, SEEK_SET);
				fread((void*)0x02000400, 1, 0x128, twlCfgFile);
				fclose(twlCfgFile);

				u32 srBackendId[2] = {*(u32*)0x02000428, *(u32*)0x0200042C};
				if ((srBackendId[0] != 0x53524C41 || srBackendId[1] != 0x00030004) && isDSiMode() && !nandMounted) {
					nandMounted = fatMountSimple("nand", &io_dsi_nand);
					if (nandMounted) {
						twlCfgFile = fopen("nand:/shared1/TWLCFG0.dat", "rb");
						fseek(twlCfgFile, 0x88, SEEK_SET);
						fread((void*)0x02000400, 1, 0x128, twlCfgFile);
						fclose(twlCfgFile);
					}
				}

				// WiFi RAM data
				u8* twlCfg = (u8*)0x02000400;
				readFirmware(0x1FD, twlCfg+0x1E0, 1); // WlFirm Type (1=DWM-W015, 2=W024, 3=W028)
				if (twlCfg[0x1E0] == 2 || twlCfg[0x1E0] == 3) {
					toncset32(twlCfg+0x1E4, 0x520000, 1); // WlFirm RAM vars
					toncset32(twlCfg+0x1E8, 0x520000, 1); // WlFirm RAM base
					toncset32(twlCfg+0x1EC, 0x020000, 1); // WlFirm RAM size
				} else {
					toncset32(twlCfg+0x1E4, 0x500400, 1); // WlFirm RAM vars
					toncset32(twlCfg+0x1E8, 0x500000, 1); // WlFirm RAM base
					toncset32(twlCfg+0x1EC, 0x02E000, 1); // WlFirm RAM size
				}
				*(u16*)(twlCfg+0x1E2) = swiCRC16(0xFFFF, twlCfg+0x1E4, 0xC); // WlFirm CRC16

				twlCfgFile = fopen(nandMounted ? "nand:/sys/HWINFO_N.dat" : "sd:/sys/HWINFO_N.dat", "rb");
				fseek(twlCfgFile, 0x88, SEEK_SET);
				fread((void*)0x02000600, 1, 0x14, twlCfgFile);
				fclose(twlCfgFile);

				useTwlCfg = true;
				u32 params = BIT(0);
				if (softResetParamsBak & BIT(1)) {
					params |= BIT(1);
				}
				if (softResetParamsBak & BIT(2)) {
					params |= BIT(2);
				}
				twlCfgFile = fopen(cachePath, "wb");
				fwrite(&params, 1, 4, twlCfgFile);
				fwrite((void*)0x02000004, 1, 0x3FFC, twlCfgFile);
				fclose(twlCfgFile);
			}
		} else {
			if (useTwlCfg) {
				u32 params = BIT(0);
				if (softResetParamsBak & BIT(1)) {
					params |= BIT(1);
				}
				if (softResetParamsBak & BIT(2)) {
					params |= BIT(2);
				}
				FILE* twlCfgFile = fopen(cachePath, "wb");
				fwrite(&params, 1, 4, twlCfgFile);
				fwrite((void*)0x02000004, 1, 0x3FFC, twlCfgFile);
				fclose(twlCfgFile);
			} else {
				FILE* twlCfgFile = fopen(cachePath, "rb");
				if (twlCfgFile) {
					fread((void*)0x02000000, 1, 0x4000, twlCfgFile);
					fclose(twlCfgFile);

					useTwlCfg = ((*(u8*)0x02000400 != 0) && (*(u8*)0x02000401 == 0) && (*(u8*)0x02000402 == 0) && (*(u8*)0x02000404 == 0) && (*(u8*)0x02000448 != 0));
					if (*(u32*)0x02000000 & BIT(0)) {
						softResetParamsBak |= BIT(0);
					}
					if (*(u32*)0x02000000 & BIT(1)) {
						softResetParamsBak |= BIT(1);
					}
					if (*(u32*)0x02000000 & BIT(2)) {
						softResetParamsBak |= BIT(2);
					}
				}
			}
		}
		if (useTwlCfg && isDSiMode() && sdFound() && sys().arm7SCFGLocked() && *(u32*)0x020007F0 != 0x4D44544C) {
			u32 srBackendId[2] = {*(u32*)0x02000428, *(u32*)0x0200042C};
			u16 tidPart = 0;
			tonccpy(&tidPart, (void*)((u32)srBackendId+6), sizeof(u16));
			if (is3DS) {
				if (tidPart != 0x0003) {
					u32 currentSrBackendId[2] = {0};
					u8 sysValue = 0;

					TWLSettings settings;
					settings.loadSettings();

					switch (settings.dsiWareExploit) {
						case TWLSettings::EExploitSudokuhax:
						default:
							currentSrBackendId[0] = 0x4B344441;		// SUDOKU
							break;
						case TWLSettings::EExploit4Swordshax:
							currentSrBackendId[0] = 0x4B513941;		// Legend of Zelda: Four Swords
							break;
						case TWLSettings::EExploitFieldrunnerhax:
							currentSrBackendId[0] = 0x4B464441;		// Fieldrunners
							break;
						case TWLSettings::EExploitGrtpwn:
							currentSrBackendId[0] = 0x4B475241;		// Guitar Rock Tour
							break;
						case TWLSettings::EExploitUgopwn:
							currentSrBackendId[0] = 0x4B475541;		// Flipnote Studio
							break;
						case TWLSettings::EExploitUnopwn:
							currentSrBackendId[0] = 0x4B554E41;		// UNO
							break;
						case TWLSettings::EExploitMemoryPit:
							currentSrBackendId[0] = 0x484E4941;		// Nintendo DSi Camera
							break;
						case TWLSettings::EExploitNone:
							break;
					}
					switch (settings.sysRegion) {
						case TWLSettings::ERegionJapan:
							sysValue = 0x4A;		// JPN
							break;
						case TWLSettings::ERegionUSA:
							sysValue = 0x45;		// USA
							break;
						case TWLSettings::ERegionEurope:
							sysValue = (settings.dsiWareExploit==7 ? 0x50 : 0x56);		// EUR
							break;
						case TWLSettings::ERegionAustralia:
							sysValue = (settings.dsiWareExploit==7 ? 0x55 : 0x56);		// AUS
							break;
						case TWLSettings::ERegionChina:
							sysValue = 0x43;		// CHN
							break;
						case TWLSettings::ERegionKorea:
							sysValue = 0x4B;		// KOR
							break;
						case TWLSettings::ERegionDefault:
							break;
					}
					tonccpy(&currentSrBackendId, &sysValue, 1);
					currentSrBackendId[1] = (settings.dsiWareExploit==7 ? 0x00030005 : 0x00030004);

					if (settings.dsiWareExploit > 0) {
						mkdir("sd:/_nds/nds-bootstrap", 0777);
						FILE* file = fopen("sd:/_nds/nds-bootstrap/srBackendId.bin", "wb");
						if (file) {
							fwrite(currentSrBackendId, sizeof(u32), 2, file);
						}
						fclose(file);
					} else if (access("sd:/_nds/nds-bootstrap/srBackendId.bin", F_OK) == 0) {
						remove("sd:/_nds/nds-bootstrap/srBackendId.bin");
					}
				} else {
					mkdir("sd:/_nds/nds-bootstrap", 0777);
					FILE* srBackendIdFile = fopen("sd:/_nds/nds-bootstrap/srBackendId.bin", "wb");
					fwrite(srBackendId, 1, 8, srBackendIdFile);
					fclose(srBackendIdFile);
				}
			} else {
				if (tidPart != 0x0003) {
					if (!nandMounted) {
						nandMounted = fatMountSimple("nand", &io_dsi_nand);
					}
					// Read correct title ID of launched System Menu title
					FILE* twlCfgFile = fopen("nand:/shared1/TWLCFG0.dat", "rb");
					fseek(twlCfgFile, 0xB0, SEEK_SET);
					fread(srBackendId, sizeof(u32), 2, twlCfgFile);
					fclose(twlCfgFile);

					tonccpy(&tidPart, (void*)((u32)srBackendId+6), sizeof(u16));
				}
				if (tidPart == 0x0003) {
					mkdir("sd:/_nds/nds-bootstrap", 0777);
					FILE* srBackendIdFile = fopen("sd:/_nds/nds-bootstrap/srBackendId.bin", "wb");
					fwrite(srBackendId, 1, 8, srBackendIdFile);
					fclose(srBackendIdFile);
				} else if (access("sd:/_nds/nds-bootstrap/srBackendId.bin", F_OK) == 0) {
					remove("sd:/_nds/nds-bootstrap/srBackendId.bin");
				}
			}
		} else if (access("sd:/_nds/nds-bootstrap/srBackendId.bin", F_OK) == 0) {
			remove("sd:/_nds/nds-bootstrap/srBackendId.bin");
		}
	}

	*(u32*)(0x02000000) = softResetParamsBak;

	scanKeys();
	if ((keysHeld() & KEY_A)
	 && (keysHeld() & KEY_B)
	 && (keysHeld() & KEY_X)
	 && (keysHeld() & KEY_Y)) {
		runGraphicIrq();
		resetSettingsPrompt();
	}

	if (isDSiMode()) {
		scanKeys();
		if (!(keysHeld() & KEY_SELECT)) {
			flashcardInit();
		}
	}

	ms().loadSettings();
	bs().loadSettings();
	logInit();

	// Get SysNAND region and launcher app
	if (isDSiMode() && sdFound() && !is3DS && (ms().sysRegion == TWLSettings::ERegionDefault || ms().launcherApp == -1)) {
		if (!nandMounted) {
			nandMounted = fatMountSimple("nand", &io_dsi_nand);
		}

		FILE *hwinfo_s = fopen("nand:/sys/HWINFO_S.dat", "rb");
		if (hwinfo_s) {
			if (ms().sysRegion == TWLSettings::ERegionDefault) {
				fseek(hwinfo_s, 0x90, SEEK_SET);
				ms().sysRegion = (TWLSettings::TRegion)fgetc(hwinfo_s);
			}

			if (ms().launcherApp == -1) {
				if (access("nand:/launcher.dsi", F_OK) == 0) {
					// DSi Language Patcher
					ms().launcherApp = 9;
				} else {
					fseek(hwinfo_s, 0xA0, SEEK_SET);
					u32 launcherTid;
					fread(&launcherTid, sizeof(u32), 1, hwinfo_s);

					char tmdPath[64];
					snprintf(tmdPath, sizeof(tmdPath), "nand:/title/00030017/%08lx/content/title.tmd", launcherTid);
					FILE *launcherTmd = fopen(tmdPath, "rb");
					if (launcherTmd) {
						fseek(launcherTmd, 0x1E7, SEEK_SET);
						ms().launcherApp = fgetc(launcherTmd);
						fclose(launcherTmd);
					}
				}
			}

			fclose(hwinfo_s);
			ms().saveSettings();
		}

	}

	// Set DSi donor ROM
	if (useTwlCfg && isDSiMode() && sdFound() && !is3DS && sys().arm7SCFGLocked()) {
		if (!nandMounted) {
			nandMounted = fatMountSimple("nand", &io_dsi_nand);
		}

		const char* pathDefine0 = *(u32*)0x02FFE1A0 == 0x080037C0 ? "DONORTWLONLY0_NDS_PATH" : "DONORTWL0_NDS_PATH"; // SDK5.0
		const char* pathDefine = *(u32*)0x02FFE1A0 == 0x080037C0 ? "DONORTWLONLY_NDS_PATH" : "DONORTWL_NDS_PATH"; // SDK5.x

		const char *bootstrapinipath = sys().isRunFromSD() ? BOOTSTRAP_INI : BOOTSTRAP_INI_FC;
		CIniFile bootstrapini(bootstrapinipath);
		std::string donorRomPath0 = bootstrapini.GetString("NDS-BOOTSTRAP", pathDefine0, "");
		std::string donorRomPath = bootstrapini.GetString("NDS-BOOTSTRAP", pathDefine, "");

		u32 srBackendId[2] = {*(u32*)0x02000428, *(u32*)0x0200042C};
		u32 donorArm7Len = 0;
		bool currentAppRead = false;
		char tmdPath[64];
		char appPath[64];
		int appVer = 0;
		TWLSettings::TRegion region = TWLSettings::ERegionJapan;
		FILE *hwinfo_s = fopen("nand:/sys/HWINFO_S.dat", "rb");
		fseek(hwinfo_s, 0x90, SEEK_SET);
		region = (TWLSettings::TRegion)fgetc(hwinfo_s);
		fclose(hwinfo_s);

		if (donorRomPath0 != "" && access(donorRomPath0.c_str(), F_OK) != 0) {
			donorRomPath0 = "";
		}

		if (donorRomPath0 == "") {
			snprintf(tmdPath, sizeof(tmdPath), "nand:/title/%08lx/%08lx/content/title.tmd", srBackendId[1], srBackendId[0]);
			FILE* donorTmd = fopen(tmdPath, "rb");
			if (donorTmd) {
				fseek(donorTmd, 0x1E7, SEEK_SET);
				appVer = fgetc(donorTmd);
				fclose(donorTmd);
			}

			snprintf(appPath, sizeof(appPath), "nand:/title/%08lx/%08lx/content/0000000%i.app", srBackendId[1], srBackendId[0], appVer);
			FILE* donorRom = fopen(appPath, "rb");
			if (donorRom) {
				fseek(donorRom, 0x3C, SEEK_SET);
				fread(&donorArm7Len, sizeof(u32), 1, donorRom);
				fclose(donorRom);
			}
			currentAppRead = true;

			bool validDonor = false;
			if (*(u32*)0x02FFE1A0 == 0x080037C0) {
				if (donorArm7Len == 0x26CC8
				 || donorArm7Len == 0x28E54) {
					bootstrapini.SetString("NDS-BOOTSTRAP", pathDefine0, appPath);
					validDonor = true;
				}
			} else {
				if (donorArm7Len==0x29EE8) {
					bootstrapini.SetString("NDS-BOOTSTRAP", pathDefine0, appPath);
					validDonor = true;
				}
			}

			if (!validDonor) {
				bool validAppFound = false;
				char validTmdPath[64];
				char validAppPath[64];
				int validAppVer = 0;
				u32 tid1 = 0;
				u32 tid2 = 0;

				if (*(u32*)0x02FFE1A0 == 0x080037C0) {
					tid1 = 0x484E4B45; // Nintendo DSi Sound
					tid2 = 0x00030005;

					switch ((int)region) {
						case TWLSettings::ERegionJapan:
							tid1 = 0x484E4B4A;
							break;
						case TWLSettings::ERegionEurope:
							tid1 = 0x484E4B50;
							break;
						case TWLSettings::ERegionAustralia:
							tid1 = 0x484E4B55;
							break;
						case TWLSettings::ERegionChina:
							tid1 = 0x484E4B43;
							break;
						case TWLSettings::ERegionKorea:
							tid1 = 0x484E4B4B;
							break;
					}

					snprintf(validTmdPath, sizeof(validTmdPath), "nand:/title/%08lx/%08lx/content/title.tmd", tid2, tid1);
					FILE* donorTmd = fopen(validTmdPath, "rb");
					if (donorTmd) {
						fseek(donorTmd, 0x1E7, SEEK_SET);
						validAppVer = fgetc(donorTmd);
						fclose(donorTmd);
						validAppFound = true;
					}
				} else for (int i = 0; i < 5; i++) {
					tid2 = 0x00030004;
					switch (i) {
						case 0: { // Dr. Mario Express
							tid1 = 0x4B443945;

							switch ((int)region) {
								case TWLSettings::ERegionJapan:
									tid1 = 0x4B44394A;
									break;
								case TWLSettings::ERegionEurope:
								case TWLSettings::ERegionAustralia:
									tid1 = 0x4B443956;
									break;
								case TWLSettings::ERegionChina:
									tid1 = 0x4B443943;
									break;
								case TWLSettings::ERegionKorea:
									continue; // Unavailable
							}
						}	break;
						case 1: { // Art Style Series: BASE 10
							tid1 = 0x4B414445;

							switch ((int)region) {
								case TWLSettings::ERegionJapan:
									tid1 = 0x4B41444A;
									break;
								case TWLSettings::ERegionEurope:
								case TWLSettings::ERegionAustralia:
									tid1 = 0x4B414456;
									break;
								case TWLSettings::ERegionChina:
									continue; // Unavailable
								case TWLSettings::ERegionKorea:
									continue; // Unavailable
							}
						}	break;
						case 2: { // Clubhouse Games Express: Card Classics
							tid1 = 0x4B545254;

							switch ((int)region) {
								case TWLSettings::ERegionJapan:
									tid1 = 0x4B54524A;
									break;
								case TWLSettings::ERegionEurope:
									continue; // Not SDK5.0
								case TWLSettings::ERegionChina:
									continue; // Not SDK5.0
								case TWLSettings::ERegionKorea:
									continue; // Not SDK5.0
							}
						}	break;
						case 3: { // Chotto Asobi Taizen: Otegaru Toranpu (Japan only)
							if (region != TWLSettings::ERegionJapan) {
								continue;
							}
							tid1 = 0x4B545054;
						}	break;
						case 4: { // Chotto Asobi Taizen: Onajimi Teburu (Japan only)
							if (region != TWLSettings::ERegionJapan) {
								continue;
							}
							tid1 = 0x4B544254;
						}	break;
					}

					snprintf(validTmdPath, sizeof(validTmdPath), "nand:/title/%08lx/%08lx/content/title.tmd", tid2, tid1);
					FILE* donorTmd = fopen(validTmdPath, "rb");
					if (donorTmd) {
						fseek(donorTmd, 0x1E7, SEEK_SET);
						validAppVer = fgetc(donorTmd);
						fclose(donorTmd);
						validAppFound = true;
						break;
					}
				}

				if (validAppFound) {
					snprintf(validAppPath, sizeof(validAppPath), "nand:/title/%08lx/%08lx/content/0000000%i.app", tid2, tid1, validAppVer);
					bootstrapini.SetString("NDS-BOOTSTRAP", pathDefine0, validAppPath);
					validDonor = true;
				}
			}
			if (validDonor) {
				logPrint("Donor ROM has been automatically set!\n");
				bootstrapini.SaveIniFile(bootstrapinipath);
			}
		}

		if (donorRomPath != "" && access(donorRomPath.c_str(), F_OK) != 0) {
			donorRomPath = "";
		}

		if (donorRomPath == "") {
			if (!currentAppRead) {
				snprintf(tmdPath, sizeof(tmdPath), "nand:/title/%08lx/%08lx/content/title.tmd", srBackendId[1], srBackendId[0]);
				FILE* donorTmd = fopen(tmdPath, "rb");
				if (donorTmd) {
					fseek(donorTmd, 0x1E7, SEEK_SET);
					appVer = fgetc(donorTmd);
					fclose(donorTmd);
				}

				snprintf(appPath, sizeof(appPath), "nand:/title/%08lx/%08lx/content/0000000%i.app", srBackendId[1], srBackendId[0], appVer);
				FILE* donorRom = fopen(appPath, "rb");
				if (donorRom) {
					fseek(donorRom, 0x3C, SEEK_SET);
					fread(&donorArm7Len, sizeof(u32), 1, donorRom);
					fclose(donorRom);
				}
				currentAppRead = true;
			}

			bool validDonor = false;
			if (*(u32*)0x02FFE1A0 == 0x080037C0) {
				if (donorArm7Len==0x1D43C
				 || donorArm7Len==0x1D5A8
				 || donorArm7Len==0x1E1E8
				 || donorArm7Len==0x1E22C
				 || donorArm7Len==0x25664
				 || donorArm7Len==0x257DC
				 || donorArm7Len==0x25860
				 || donorArm7Len==0x268DC
				 || donorArm7Len==0x26BA8
				 || donorArm7Len==0x26C5C
				 || donorArm7Len==0x26D10
				 || donorArm7Len==0x26D48
				 || donorArm7Len==0x26D50
				 || donorArm7Len==0x26DF4
				 || donorArm7Len==0x27FB4) {
					bootstrapini.SetString("NDS-BOOTSTRAP", pathDefine, appPath);
					validDonor = true;
				}
			} else {
				if (donorArm7Len==0x22B40
				 || donorArm7Len==0x22BCC
				 || donorArm7Len==0x28F84
				 || donorArm7Len==0x2909C
				 || donorArm7Len==0x2914C
				 || donorArm7Len==0x29164
				 || donorArm7Len==0x2A2EC
				 || donorArm7Len==0x2A318
				 || donorArm7Len==0x2AF18
				 || donorArm7Len==0x2B184
				 || donorArm7Len==0x2B24C
				 || donorArm7Len==0x2C5B4) {
					bootstrapini.SetString("NDS-BOOTSTRAP", pathDefine, appPath);
					validDonor = true;
				}
			}

			if (validDonor) {
				logPrint("Donor ROM has been automatically set!\n");
				bootstrapini.SaveIniFile(bootstrapinipath);
			}
		}
	}

	if (!ms().languageSet) {
		runGraphicIrq();
		languageSelect();
	}

	if (!ms().regionSet || (!dsiFeatures() && ms().gameRegion < 0) || (dsiFeatures() && ms().gameRegion < -1)) {
		if (!dsiFeatures() && ms().gameRegion < TWLSettings::ERegionJapan) ms().gameRegion = TWLSettings::ERegionJapan;
		else if (dsiFeatures() && ms().gameRegion < TWLSettings::ERegionDefault) ms().gameRegion = TWLSettings::ERegionDefault;
		runGraphicIrq();
		regionSelect();
	}

	if (graphicsInited) {
		unloadFont();
		graphicsInited = false;
	}

	bool fcFound = flashcardFound();

	if (ms().launchType[true] == Launch::EDSiWareLaunch) {
		// Move .pub and/or .prv back to "saves" folder
		std::string filename = ms().romPath[ms().previousUsedDevice];
		const size_t last_slash_idx = filename.find_last_of("/");
		if (std::string::npos != last_slash_idx) {
			filename.erase(0, last_slash_idx + 1);
		}

		loadPerGameSettings(filename);

		const char *typeToReplace = ".nds";
		if (extension(filename, ".dsi")) {
			typeToReplace = ".dsi";
		} else if (extension(filename, ".ids")) {
			typeToReplace = ".ids";
		} else if (extension(filename, ".srl")) {
			typeToReplace = ".srl";
		} else if (extension(filename, ".app")) {
			typeToReplace = ".app";
		}

		std::string pubname = replaceAll(filename, typeToReplace, getPubExtension());
		std::string prvname = replaceAll(filename, typeToReplace, getPrvExtension());
		std::string pubnameUl = replaceAll(filename, typeToReplace, ".pub");
		std::string prvnameUl = replaceAll(filename, typeToReplace, ".prv");
		std::string romfolder = ms().romPath[ms().previousUsedDevice];
		while (!romfolder.empty() && romfolder[romfolder.size()-1] != '/') {
			romfolder.resize(romfolder.size()-1);
		}
		std::string romFolderNoSlash = romfolder;
		RemoveTrailingSlashes(romFolderNoSlash);
		std::string pubpath = romFolderNoSlash + "/saves/" + pubname;
		std::string prvpath = romFolderNoSlash + "/saves/" + prvname;
		std::string pubpathUl = romFolderNoSlash + "/" + pubnameUl;
		std::string prvpathUl = romFolderNoSlash + "/" + prvnameUl;
		if (access(pubpathUl.c_str(), F_OK) == 0) {
			rename(pubpathUl.c_str(), pubpath.c_str());
			logPrint("Moved back to saves folder:\n%s\n%s\n\n", pubpathUl.c_str(), pubpath.c_str());
		}
		if (access(prvpathUl.c_str(), F_OK) == 0) {
			rename(prvpathUl.c_str(), prvpath.c_str());
			logPrint("Moved back to saves folder:\n%s\n%s\n\n", prvpathUl.c_str(), prvpath.c_str());
		}
	}

	if (fcFound) {
	  if (ms().launchType[true] == Launch::ESDFlashcardLaunch) {
		// Move .sav back to "saves" folder
		std::string filename = ms().romPath[true];
		const size_t last_slash_idx = filename.find_last_of("/");
		if (std::string::npos != last_slash_idx) {
			filename.erase(0, last_slash_idx + 1);
		}

		loadPerGameSettings(filename);

		const char *typeToReplace = ".nds";
		if (extension(filename, ".dsi")) {
			typeToReplace = ".dsi";
		} else if (extension(filename, ".ids")) {
			typeToReplace = ".ids";
		} else if (extension(filename, ".srl")) {
			typeToReplace = ".srl";
		} else if (extension(filename, ".app")) {
			typeToReplace = ".app";
		}

		std::string savename = replaceAll(filename, typeToReplace, getSavExtension());
		std::string savenameFc = replaceAll(filename, typeToReplace, ".sav");
		std::string romfolder = ms().romPath[true];
		while (!romfolder.empty() && romfolder[romfolder.size()-1] != '/') {
			romfolder.resize(romfolder.size()-1);
		}
		std::string romFolderNoSlash = romfolder;
		RemoveTrailingSlashes(romFolderNoSlash);
		std::string savepath = romFolderNoSlash + "/saves/" + savename;
		std::string savepathFc = romFolderNoSlash + "/" + savenameFc;
		if (access(savepathFc.c_str(), F_OK) == 0
		&& (extension(filename, ".nds")
		 || extension(filename, ".dsi")
		 || extension(filename, ".ids")
		 || extension(filename, ".srl")
		 || extension(filename, ".app"))) {
			rename(savepathFc.c_str(), savepath.c_str());
			logPrint("Moved back to saves folder:\n%s\n%s\n\n", savepathFc.c_str(), savepath.c_str());
		}
	  } else if (sys().isRegularDS() && (*(u16*)(0x020000C0) != 0) && (ms().launchType[true] == Launch::EGBANativeLaunch)) {
			u8 byteBak = *(vu8*)(0x0A000000);
			*(vu8*)(0x0A000000) = 'T';	// SRAM write test
		  if (*(vu8*)(0x0A000000) == 'T') {	// Check if SRAM is writeable
			*(vu8*)(0x0A000000) = byteBak;
			std::string savepath = replaceAll(ms().romPath[true], ".gba", ".sav");
			u32 savesize = getFileSize(savepath.c_str());
			if (savesize > 0x20000) savesize = 0x20000;
			if (savesize > 0) {
				// Try to restore save from SRAM
				bool restoreSave = false;
				extern char copyBuf[0x8000];
				u32 ptr = 0x0A000000;
				u32 len = savesize;
				gbaSramAccess(true);	// Switch to GBA SRAM
				for (u32 i = 0; i < savesize; i += 0x8000) {
					if (ptr >= 0x0A010000 || len <= 0) {
						break;
					}
					cExpansion::ReadSram(ptr,(u8*)copyBuf,(len>0x8000 ? 0x8000 : len));
					for (u32 i2 = 0; i2 < (len>0x8000 ? 0x8000 : len); i2++) {
						if (copyBuf[i2] != 0) {
							restoreSave = true;
							break;
						}
					}
					ptr += 0x8000;
					len -= 0x8000;
					if (restoreSave) break;
				}
				gbaSramAccess(false);	// Switch out of GBA SRAM
				if (restoreSave) {
					ptr = 0x0A000000;
					len = savesize;
					FILE* savFile = fopen(savepath.c_str(), "wb");
					for (u32 i = 0; i < savesize; i += 0x8000) {
						if (ptr >= 0x0A020000 || len <= 0) {
							break;	// In case if this writes a save bigger than 128KB
						} else if (ptr >= 0x0A010000) {
							toncset(&copyBuf, 0, 0x8000);
						} else {
							gbaSramAccess(true);	// Switch to GBA SRAM
							cExpansion::ReadSram(ptr,(u8*)copyBuf,0x8000);
							gbaSramAccess(false);	// Switch out of GBA SRAM
						}
						fwrite(&copyBuf, 1, (len>0x8000 ? 0x8000 : len), savFile);
						ptr += 0x8000;
						len -= 0x8000;
					}
					fclose(savFile);
					logPrint("Restored GBA save from Slot-2 flashcard\n");

					gbaSramAccess(true);	// Switch to GBA SRAM
					// Wipe out SRAM after restoring save
					toncset(&copyBuf, 0, 0x8000);
					cExpansion::WriteSram(0x0A000000, (u8*)copyBuf, 0x8000);
					cExpansion::WriteSram(0x0A008000, (u8*)copyBuf, 0x8000);
					gbaSramAccess(false);	// Switch out of GBA SRAM
				}
			}
		  }
	  }
	}

	if (isDSiMode()) {
		/*if (ms().wifiLed == -1) {
			if (ms().consoleModel >= 2) {
				ms().wifiLed = true;
				*(u8*)(0x02FFFD00) = 0x13;		// WiFi On
			} else if (*(u8*)(0x02FFFD01) == 0x13) {
				ms().wifiLed = true;
			} else if (*(u8*)(0x02FFFD01) == 0 || *(u8*)(0x02FFFD01) == 0x12) {
				ms().wifiLed = false;
			}
		} else {*/
			*(u8*)(0x02FFFD00) = (ms().wifiLed ? 0x13 : 0x12);		// WiFi On/Off
		//}
		if (ms().consoleModel < 2) {
			*(u8*)(0x02FFFD02) = (ms().powerLedColor ? 0xFF : 0x00);
		}
	}

	if (sdFound()) {
		mkdir("sd:/_gba", 0777);
		mkdir("sd:/_nds", 0777);
		mkdir("sd:/_nds/nds-bootstrap", 0777);
		mkdir("sd:/_nds/nds-bootstrap/patchOffsetCache", 0777);
		mkdir("sd:/_nds/TWiLightMenu", 0777);
		mkdir("sd:/_nds/TWiLightMenu/gamesettings", 0777);
	}
	if (flashcardFound()) {
		mkdir("fat:/_gba", 0777);
		mkdir("fat:/_nds", 0777);
		mkdir("fat:/_nds/nds-bootstrap", 0777);
		mkdir("fat:/_nds/nds-bootstrap/patchOffsetCache", 0777);
		mkdir("fat:/_nds/TWiLightMenu", 0777);
		mkdir("fat:/_nds/TWiLightMenu/gamesettings", 0777);
	}

	if (REG_SCFG_EXT != 0) {
		*(vu32*)(0x0DFFFE0C) = 0x53524C41;		// Check for 32MB of RAM
		bool isDevConsole = (*(vu32*)(0x0DFFFE0C) == 0x53524C41);
		if (isDevConsole) {
			// Check for Nintendo 3DS console
			TWLSettings::TConsoleModel resultModel = is3DS ? TWLSettings::E3DSOriginal : TWLSettings::EDSiDebug;
			if (ms().consoleModel != resultModel || bs().consoleModel != resultModel) {
				ms().consoleModel = resultModel;
				bs().consoleModel = resultModel;
				ms().saveSettings();
				bs().saveSettings();
			}
		} else if (ms().consoleModel != 0 || bs().consoleModel != 0) {
			ms().consoleModel = TWLSettings::EDSiRetail;
			bs().consoleModel = TWLSettings::EDSiRetail;
			ms().saveSettings();
			bs().saveSettings();
		}
	}

	bool softResetParamsFound = false;
	const char* softResetParamsPath = (!ms().previousUsedDevice ? "sd:/_nds/nds-bootstrap/softResetParams.bin" : "fat:/_nds/nds-bootstrap/softResetParams.bin");
	u32 softResetParams = 0;
	FILE* file = fopen(softResetParamsPath, "rb");
	if (file) {
		fread(&softResetParams, sizeof(u32), 1, file);
		softResetParamsFound = (softResetParams != 0xFFFFFFFF);
	}
	fclose(file);

	if (softResetParamsFound) {
		logPrint("Soft-reset parameters found\n");
		scanKeys();
		if (keysHeld() & KEY_X) {
			softResetParams = 0xFFFFFFFF;
			file = fopen(softResetParamsPath, "wb");
			fwrite(&softResetParams, sizeof(u32), 1, file);
			fseek(file, 0x10 - 1, SEEK_SET);
			fputc('\0', file);
			fclose(file);
			logPrint("Cleared soft-reset parameters\n");
			softResetParamsFound = false;
		}
	}

	if ((access(settingsinipath, F_OK) != 0)
	|| languageNowSet || regionNowSet
	|| (ms().theme < TWLSettings::EThemeDSi) || (ms().theme == TWLSettings::EThemeWood) || (ms().theme > TWLSettings::EThemeGBC)) {
		// Create or modify "settings.ini"
		if (ms().theme == TWLSettings::EThemeWood) {
			ms().theme = TWLSettings::EThemeR4;
		} else if (ms().theme < TWLSettings::EThemeDSi || ms().theme > TWLSettings::EThemeGBC) {
			ms().theme = TWLSettings::EThemeDSi;
		}
		ms().saveSettings();
		if (regionNowSet) {
			CIniFile bootstrapini(sys().isRunFromSD() ? BOOTSTRAP_INI : BOOTSTRAP_INI_FC);
			bootstrapini.SetInt("NDS-BOOTSTRAP", "USE_ROM_REGION", ms().useRomRegion);
			bootstrapini.SaveIniFileModified(sys().isRunFromSD() ? BOOTSTRAP_INI : BOOTSTRAP_INI_FC);
		}
	}

	if (!softResetParamsFound && !(*(u32*)0x02000000 & BIT(3)) && ms().dsiSplash && (REG_SCFG_EXT!=0&&ms().consoleModel<2 ? fifoGetValue32(FIFO_USER_04) != 0x01 : !(*(u32*)0x02000000 & BIT(0)))) {
		runGraphicIrq();
		bootSplashInit();
		if (REG_SCFG_EXT != 0 && ms().consoleModel < 2) fifoSendValue32(FIFO_USER_04, 10);
	}
	*(u32*)0x02000000 |= BIT(0);

	if (access(sys().isRunFromSD() ? BOOTSTRAP_INI : BOOTSTRAP_INI_FC, F_OK) != 0) {
		// Create "nds-bootstrap.ini"
		bs().saveSettings();
	}
	
	scanKeys();

	autoRunBit = (*(u32*)0x02000000 & BIT(3));
	runTempDSiWare = (*(u32*)0x02000000 & BIT(4));
	if (autoRunBit || (!(*(u32*)0x02000000 & BIT(2))
	&& ((softResetParamsFound && (ms().launchType[ms().previousUsedDevice] == Launch::ESDFlashcardLaunch || ms().launchType[ms().previousUsedDevice] == Launch::EDSiWareLaunch))
	|| (ms().autorun ? !(keysHeld() & KEY_B) : (keysHeld() & KEY_B))))) {
		*(u32*)0x02000000 &= ~BIT(3);
		*(u32*)0x02000000 &= ~BIT(4);
		//unloadNds9iAsynch();
		if (isDSiMode() && sdFound() && !fcFound && !sys().arm7SCFGLocked() && ms().limitedMode > 0) {
			*(u32*)0x02FFFD0C = ms().limitedMode == 2 ? 0x4E44544C : ms().limitedMode == 3 ? 0x6D44544C : 0x4D44544C;
			*(u32*)0x020007F0 = 0x4D44544C;
		}
		lastRunROM();
	}

	// If in DSi mode with no flashcard & with SCFG access, attempt to cut slot1 power to save battery
	if (isDSiMode() && !fcFound && !sys().arm7SCFGLocked() && !ms().autostartSlot1) {
		disableSlot1();
	}

	if (!softResetParamsFound && ms().autostartSlot1 && isDSiMode() && REG_SCFG_MC != 0x11 && !fcFound && !(keysHeld() & KEY_SELECT)) {
		//unloadNds9iAsynch();
		if (ms().slot1LaunchMethod==0 || sys().arm7SCFGLocked()) {
			dsCardLaunch();
		} else if (ms().slot1LaunchMethod==2) {
			unlaunchRomBoot("cart:");
		} else {
			runNdsFile("/_nds/TWiLightMenu/slot1launch.srldr", 0, NULL, true, true, false, true, true, false, -1);
		}
	}

	// snprintf(vertext, sizeof(vertext), "Ver %d.%d.%d   ", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH); // Doesn't work :(

	if (ms().showlogo && !(*(u32*)0x02000000 & BIT(1))) {
		runGraphicIrq();
		loadTitleGraphics();
		twlMenuVideo();
		snd().fadeOutStream();
	}
	*(u32*)0x02000000 &= ~BIT(1);
	*(u32*)0x02000000 &= ~BIT(2);

	scanKeys();

	if (keysHeld() & KEY_SELECT) {
		screenmode = 1;
	}

	srand(time(NULL));

	while (1) {
		runGraphicIrq();
		if (screenmode == 1) {
			//unloadNds9iAsynch();
			fadeColor = true;
			controlTopBright = true;
			controlBottomBright = true;
			fadeType = false;
			for (int i = 0; i < 25; i++) {
				swiWaitForVBlank();
			}
			logPrint("Opening TWLMenu++ Settings...\n");

			vector<char *> argarray;
			argarray.push_back((char*)(sys().isRunFromSD() ? "sd:/_nds/TWiLightMenu/settings.srldr" : "fat:/_nds/TWiLightMenu/settings.srldr"));

			runNdsFile(argarray[0], argarray.size(), (const char**)&argarray[0], true, false, false, true, true, false, -1);
		} else {
			if (isDSiMode() && sdFound() && !fcFound && !sys().arm7SCFGLocked() && ms().limitedMode > 0) {
				*(u32*)0x02FFFD0C = ms().limitedMode == 2 ? 0x4E44544C : ms().limitedMode == 3 ? 0x6D44544C : 0x4D44544C;
				*(u32*)0x020007F0 = 0x4D44544C;
			}
			if (ms().showMainMenu) {
				loadMainMenu();
			}
			loadROMselect();
		}
	}

	return 0;
}
