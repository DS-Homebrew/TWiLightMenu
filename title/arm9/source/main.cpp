#include <nds.h>
#include <nds/arm9/dldi.h>
#include <cstdio>
#include <fat.h>
#include <sys/stat.h>
#include <limits.h>
#include <variant>
#include <string.h>
#include <unistd.h>
#include "common/gl2d.h"

#include "autoboot.h"

#include "graphics/graphics.h"

#include "common/nds_loader_arm9.h"
#include "common/inifile.h"
#include "common/nitrofs.h"
#include "common/bootstrappaths.h"
#include "common/dsimenusettings.h"
#include "common/pergamesettings.h"
#include "common/cardlaunch.h"
#include "common/flashcard.h"
#include "common/fileCopy.h"
#include "common/tonccpy.h"
#include "nandio.h"
#include "bootstrapsettings.h"
#include "bootsplash.h"
#include "twlmenuppvideo.h"
#include "consolemodelselect.h"

#include "sr_data_srllastran.h"			 // For rebooting into the game
#include "common/systemdetails.h"

#include "saveMap.h"

bool useTwlCfg = false;

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

const char *hiyacfwinipath = "sd:/hiya/settings.ini";

const char *unlaunchAutoLoadID = "AutoLoadInfo";
char hiyaNdsPath[14] = {'s','d','m','c',':','/','h','i','y','a','.','d','s','i'};
char unlaunchDevicePath[256];

typedef struct {
	char gameTitle[12];			//!< 12 characters for the game title.
	char gameCode[4];			//!< 4 characters for the game code.
} sNDSHeadertitlecodeonly;

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
	while (1)
	{
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
	while (1)
	{
		scanKeys();
		if (keysDown() & KEY_START)
			break;
	}
	scanKeys();
}*/

std::string ReplaceAll(std::string str, const std::string &from, const std::string &to)
{
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos)
	{
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
	}
	return str;
}

void rebootDSiMenuPP()
{
	fifoSendValue32(FIFO_USER_01, 1); // Fade out sound
	for (int i = 0; i < 25; i++)
		swiWaitForVBlank();
	memcpy((u32 *)0x02000300, autoboot_bin, 0x020);
	fifoSendValue32(FIFO_USER_08, 1); // Reboot DSiMenu++ to avoid potential crashing
	for (int i = 0; i < 15; i++)
		swiWaitForVBlank();
}

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

	if (!isDSiMode()) {
		chdir("fat:/");
	}
	runNdsFile("/_nds/TWiLightMenu/mainmenu.srldr", 0, NULL, true, false, false, true, true);
}

void loadROMselect(int number)
{
	fadeColor = true;
	controlTopBright = true;
	controlBottomBright = true;
	fadeType = false;
	fifoSendValue32(FIFO_USER_01, 1); // Fade out sound
	for (int i = 0; i < 25; i++)
		swiWaitForVBlank();
	fifoSendValue32(FIFO_USER_01, 0); // Cancel sound fade out

	if (!isDSiMode()) {
		chdir("fat:/");
	}
	switch (number) {
		/*case 3:
			runNdsFile("/_nds/TWiLightMenu/akmenu.srldr", 0, NULL, true, false, false, true, true);
			break;*/
		case 2:
			runNdsFile("/_nds/TWiLightMenu/r4menu.srldr", 0, NULL, true, false, false, true, true);
			break;
		default:
			runNdsFile("/_nds/TWiLightMenu/dsimenu.srldr", 0, NULL, true, false, false, true, true);
			break;
	}
	stop();
}

bool extention(const std::string& filename, const char* ext) {
	if(strcasecmp(filename.c_str() + filename.size() - strlen(ext), ext)) {
		return false;
	} else {
		return true;
	}
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

void dsCardLaunch() {
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

void lastRunROM()
{
	/*fifoSendValue32(FIFO_USER_01, 1); // Fade out sound
	for (int i = 0; i < 25; i++)
		swiWaitForVBlank();
	fifoSendValue32(FIFO_USER_01, 0); // Cancel sound fade out*/

	if (bothSDandFlashcard()) {
		// Do nothing
	} else if (flashcardFound()) {
		ms().secondaryDevice = true;
	} else {
		ms().secondaryDevice = false;
	}

	std::string romfolder = ms().romPath[ms().secondaryDevice];
	while (!romfolder.empty() && romfolder[romfolder.size()-1] != '/') {
		romfolder.resize(romfolder.size()-1);
	}
	chdir(romfolder.c_str());

	std::string filename = ms().romPath[ms().secondaryDevice];
	const size_t last_slash_idx = filename.find_last_of("/");
	if (std::string::npos != last_slash_idx)
	{
		filename.erase(0, last_slash_idx + 1);
	}

	vector<char *> argarray;
	if (ms().launchType[ms().secondaryDevice] > Launch::EDSiWareLaunch)
	{
		argarray.push_back(strdup("null"));
		argarray.push_back(strdup(ms().homebrewArg[ms().secondaryDevice].c_str()));
	}

	int err = 0;
	if (ms().slot1Launched && !flashcardFound())
	{
		if (ms().slot1LaunchMethod==0 || sys().arm7SCFGLocked()) {
			dsCardLaunch();
		} else if (ms().slot1LaunchMethod==2) {
			unlaunchRomBoot("cart:");
		} else {
			// Set Widescreen
			if (!sys().arm7SCFGLocked() && ms().consoleModel >= 2 && ms().wideScreen
			&& (access("sd:/_nds/TWiLightMenu/TwlBg/Widescreen.cxi", F_OK) == 0)
			&& (access("/_nds/nds-bootstrap/wideCheatData.bin", F_OK) == 0)) {
				// Prepare for reboot into 16:10 TWL_FIRM
				rename("sd:/luma/sysmodules/TwlBg.cxi", "sd:/luma/sysmodules/TwlBg_bak.cxi");
				fcopy("sd:/_nds/TWiLightMenu/TwlBg/Widescreen.cxi", "sd:/luma/sysmodules/TwlBg.cxi");

				irqDisable(IRQ_VBLANK);				// Fix the throwback to 3DS HOME Menu bug
				memcpy((u32 *)0x02000300, sr_data_srllastran, 0x020);
				fifoSendValue32(FIFO_USER_02, 1); // Reboot in 16:10 widescreen
				stop();
			}

			err = runNdsFile("/_nds/TWiLightMenu/slot1launch.srldr", 0, NULL, true, true, false, true, true);
		}
	}
	if (ms().launchType[ms().secondaryDevice] == Launch::ESDFlashcardLaunch)
	{
		if (access(ms().romPath[ms().secondaryDevice].c_str(), F_OK) != 0) return;	// Skip to running TWiLight Menu++
		if (ms().useBootstrap || !ms().secondaryDevice)
		{
			std::string savepath;

			loadPerGameSettings(filename);

			bool useWidescreen = (perGameSettings_wideScreen == -1 ? ms().wideScreen : perGameSettings_wideScreen);

			if (ms().homebrewBootstrap) {
				// Set Widescreen
				if (!sys().arm7SCFGLocked() && ms().consoleModel >= 2 && useWidescreen
				&& (access("sd:/_nds/TWiLightMenu/TwlBg/Widescreen.cxi", F_OK) == 0)
				&& ms().homebrewHasWide) {
					// Prepare for reboot into 16:10 TWL_FIRM
					rename("sd:/luma/sysmodules/TwlBg.cxi", "sd:/luma/sysmodules/TwlBg_bak.cxi");
					fcopy("sd:/_nds/TWiLightMenu/TwlBg/Widescreen.cxi", "sd:/luma/sysmodules/TwlBg.cxi");

					irqDisable(IRQ_VBLANK);				// Fix the throwback to 3DS HOME Menu bug
					memcpy((u32 *)0x02000300, sr_data_srllastran, 0x020);
					fifoSendValue32(FIFO_USER_02, 1); // Reboot in 16:10 widescreen
					stop();
				}

				bool useNightly = (perGameSettings_bootstrapFile == -1 ? ms().bootstrapFile : perGameSettings_bootstrapFile);
				argarray.push_back((char*)(useNightly ? "sd:/_nds/nds-bootstrap-hb-nightly.nds" : "sd:/_nds/nds-bootstrap-hb-release.nds"));
			} else {
				// Set Widescreen
				if (!sys().arm7SCFGLocked() && ms().consoleModel >= 2 && useWidescreen
				&& (access("sd:/_nds/TWiLightMenu/TwlBg/Widescreen.cxi", F_OK) == 0)
				&& (access("/_nds/nds-bootstrap/wideCheatData.bin", F_OK) == 0)) {
					// Prepare for reboot into 16:10 TWL_FIRM
					rename("sd:/luma/sysmodules/TwlBg.cxi", "sd:/luma/sysmodules/TwlBg_bak.cxi");
					fcopy("sd:/_nds/TWiLightMenu/TwlBg/Widescreen.cxi", "sd:/luma/sysmodules/TwlBg.cxi");

					irqDisable(IRQ_VBLANK);				// Fix the throwback to 3DS HOME Menu bug
					memcpy((u32 *)0x02000300, sr_data_srllastran, 0x020);
					fifoSendValue32(FIFO_USER_02, 1); // Reboot in 16:10 widescreen
					stop();
				}

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

				char game_TID[5];

				FILE *f_nds_file = fopen(filename.c_str(), "rb");

				fseek(f_nds_file, offsetof(sNDSHeadertitlecodeonly, gameCode), SEEK_SET);
				fread(game_TID, 1, 4, f_nds_file);
				game_TID[4] = 0;
				game_TID[3] = 0;

				fclose(f_nds_file);

				std::string savename = ReplaceAll(filename, typeToReplace, getSavExtension());
				std::string romFolderNoSlash = romfolder;
				RemoveTrailingSlashes(romFolderNoSlash);
				mkdir ("saves", 0777);
				savepath = romFolderNoSlash+"/saves/"+savename;
				if (ms().previousUsedDevice && ms().fcSaveOnSd) {
					savepath = ReplaceAll(savepath, "fat:/", "sd:/");
				}

				if ((strcmp(game_TID, "###") != 0) && (strcmp(game_TID, "NTR") != 0)) {
					int orgsavesize = getFileSize(savepath.c_str());
					int savesize = 524288;	// 512KB (default size for most games)

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
						consoleDemoInit();
						printf((orgsavesize == 0) ? "Creating save file...\n" : "Expanding save file...\n");
						fadeType = true;

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
						printf((orgsavesize == 0) ? "Save file created!\n" : "Save file expanded!\n");

						for (int i = 0; i < 30; i++) {
							swiWaitForVBlank();
						}
						fadeType = false;
						for (int i = 0; i < 25; i++) {
							swiWaitForVBlank();
						}
					}
				}

				bool useNightly = (perGameSettings_bootstrapFile == -1 ? ms().bootstrapFile : perGameSettings_bootstrapFile);

				if (sdFound() && !ms().secondaryDevice) {
					argarray.push_back((char*)(useNightly ? "sd:/_nds/nds-bootstrap-nightly.nds" : "sd:/_nds/nds-bootstrap-release.nds"));
				} else {
					if (isDSiMode()) {
						argarray.push_back((char*)(useNightly ? "/_nds/nds-bootstrap-nightly.nds" : "/_nds/nds-bootstrap-release.nds"));
					} else {
						argarray.push_back((char*)(useNightly ? "/_nds/b4ds-nightly.nds" : "/_nds/b4ds-release.nds"));
					}
				}
			}
			if (ms().secondaryDevice || !ms().homebrewBootstrap) {
				CIniFile bootstrapini( sdFound() ? BOOTSTRAP_INI_SD : BOOTSTRAP_INI_FC );
				bootstrapini.SetString("NDS-BOOTSTRAP", "NDS_PATH", ms().romPath[ms().secondaryDevice]);
				bootstrapini.SetString("NDS-BOOTSTRAP", "SAV_PATH", savepath);
				bootstrapini.SetInt("NDS-BOOTSTRAP", "LANGUAGE",
					(perGameSettings_language == -2 ? ms().gameLanguage : perGameSettings_language));
				bootstrapini.SetInt("NDS-BOOTSTRAP", "DSI_MODE",
					(perGameSettings_dsiMode == -1 ? ms().bstrap_dsiMode : perGameSettings_dsiMode));
				bootstrapini.SetInt("NDS-BOOTSTRAP", "BOOST_CPU",
					(perGameSettings_boostCpu == -1 ? ms().boostCpu : perGameSettings_boostCpu));
				bootstrapini.SetInt("NDS-BOOTSTRAP", "BOOST_VRAM",
					(perGameSettings_boostVram == -1 ? ms().boostVram : perGameSettings_boostVram));
				bootstrapini.SaveIniFile( sdFound() ? BOOTSTRAP_INI_SD : BOOTSTRAP_INI_FC );
			}
			err = runNdsFile(argarray[0], argarray.size(), (const char **)&argarray[0], (ms().homebrewBootstrap ? false : true), true, false, true, true);
		}
		else
		{
			bool runNds_boostCpu = false;
			bool runNds_boostVram = false;
			if (isDSiMode()) {
				std::string filename = ms().romPath[1];
				const size_t last_slash_idx = filename.find_last_of("/");
				if (std::string::npos != last_slash_idx)
				{
					filename.erase(0, last_slash_idx + 1);
				}

				loadPerGameSettings(filename);
				runNds_boostCpu = perGameSettings_boostCpu == -1 ? ms().boostCpu : perGameSettings_boostCpu;
				runNds_boostVram = perGameSettings_boostVram == -1 ? ms().boostVram : perGameSettings_boostVram;
			}
			std::string path;
			if ((memcmp(io_dldi_data->friendlyName, "R4(DS) - Revolution for DS", 26) == 0)
			 || (memcmp(io_dldi_data->friendlyName, "R4iDSN", 6) == 0)) {
				CIniFile fcrompathini("fat:/_wfwd/lastsave.ini");
				path = ReplaceAll(ms().romPath[ms().secondaryDevice], "fat:/", woodfat);
				fcrompathini.SetString("Save Info", "lastLoaded", path);
				fcrompathini.SaveIniFile("fat:/_wfwd/lastsave.ini");
				err = runNdsFile("fat:/Wfwd.dat", 0, NULL, true, true, true, runNds_boostCpu, runNds_boostVram);
			} else if (memcmp(io_dldi_data->friendlyName, "Acekard AK2", 0xB) == 0) {
				CIniFile fcrompathini("fat:/_afwd/lastsave.ini");
				path = ReplaceAll(ms().romPath[ms().secondaryDevice], "fat:/", woodfat);
				fcrompathini.SetString("Save Info", "lastLoaded", path);
				fcrompathini.SaveIniFile("fat:/_afwd/lastsave.ini");
				err = runNdsFile("fat:/Afwd.dat", 0, NULL, true, true, true, runNds_boostCpu, runNds_boostVram);
			} else if (memcmp(io_dldi_data->friendlyName, "DSTWO(Slot-1)", 0xD) == 0) {
				CIniFile fcrompathini("fat:/_dstwo/autoboot.ini");
				path = ReplaceAll(ms().romPath[ms().secondaryDevice], "fat:/", dstwofat);
				fcrompathini.SetString("Dir Info", "fullName", path);
				fcrompathini.SaveIniFile("fat:/_dstwo/autoboot.ini");
				err = runNdsFile("fat:/_dstwo/autoboot.nds", 0, NULL, true, true, true, runNds_boostCpu, runNds_boostVram);
			} /*else if ((memcmp(io_dldi_data->friendlyName, "TTCARD", 6) == 0)
					 || (memcmp(io_dldi_data->friendlyName, "DSTT", 4) == 0)
					 || (memcmp(io_dldi_data->friendlyName, "DEMON", 5) == 0) {
				CIniFile fcrompathini("fat:/TTMenu/YSMenu.ini");
				path = replaceAll(ms().romPath[ms().secondaryDevice], "fat:/", slashchar);
				fcrompathini.SetString("YSMENU", "AUTO_BOOT", path);
				fcrompathini.SaveIniFile("fat:/TTMenu/YSMenu.ini");
				err = runNdsFile("fat:/YSMenu.nds", 0, NULL, true, true, true, runNds_boostCpu, runNds_boostVram);
			}*/
		}
	}
	else if (ms().launchType[ms().secondaryDevice] == Launch::ESDFlashcardDirectLaunch)
	{
		if (access(ms().romPath[ms().secondaryDevice].c_str(), F_OK) != 0) return;	// Skip to running TWiLight Menu++

		argarray.push_back((char*)ms().romPath[ms().secondaryDevice].c_str());

		bool runNds_boostCpu = false;
		bool runNds_boostVram = false;

		loadPerGameSettings(filename);

		bool useWidescreen = (perGameSettings_wideScreen == -1 ? ms().wideScreen : perGameSettings_wideScreen);

		// Set Widescreen
		if (!sys().arm7SCFGLocked() && ms().consoleModel >= 2 && useWidescreen
			&& (access("sd:/_nds/TWiLightMenu/TwlBg/Widescreen.cxi", F_OK) == 0)
			&& ms().homebrewHasWide) {
			// Prepare for reboot into 16:10 TWL_FIRM
			rename("sd:/luma/sysmodules/TwlBg.cxi", "sd:/luma/sysmodules/TwlBg_bak.cxi");
			rename("sd:/_nds/TWiLightMenu/TwlBg/Widescreen.cxi", "sd:/luma/sysmodules/TwlBg.cxi");

			irqDisable(IRQ_VBLANK);				// Fix the throwback to 3DS HOME Menu bug
			memcpy((u32 *)0x02000300, sr_data_srllastran, 0x020);
			fifoSendValue32(FIFO_USER_02, 1); // Reboot in 16:10 widescreen
			stop();
		}

		runNds_boostCpu = perGameSettings_boostCpu == -1 ? ms().boostCpu : perGameSettings_boostCpu;
		runNds_boostVram = perGameSettings_boostVram == -1 ? ms().boostVram : perGameSettings_boostVram;

		err = runNdsFile (argarray[0], argarray.size(), (const char **)&argarray[0], true, true, (!perGameSettings_dsiMode ? true : false), runNds_boostCpu, runNds_boostVram);
	}
	else if (ms().launchType[ms().secondaryDevice] == Launch::EDSiWareLaunch)
	{
		if (access(ms().romPath[ms().secondaryDevice].c_str(), F_OK) != 0) return;	// Skip to running TWiLight Menu++

		if (ms().secondaryDevice) {
			snprintf(unlaunchDevicePath, sizeof(unlaunchDevicePath), "sdmc:/_nds/TWiLightMenu/tempDSiWare.dsi");
		} else {
			snprintf(unlaunchDevicePath, sizeof(unlaunchDevicePath), "__%s", ms().dsiWareSrlPath.c_str());
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

		fifoSendValue32(FIFO_USER_08, 1); // Reboot
		for (int i = 0; i < 15; i++) swiWaitForVBlank();
	}
	else if (ms().launchType[ms().secondaryDevice] == Launch::ENESDSLaunch)
	{
		if (access(ms().romPath[ms().secondaryDevice].c_str(), F_OK) != 0) return;	// Skip to running TWiLight Menu++

		if (sys().flashcardUsed())
		{
			argarray.at(0) = (char*)"fat:/_nds/TWiLightMenu/emulators/nesds.nds";
		}
		else
		{
			argarray.at(0) = (char*)"sd:/_nds/TWiLightMenu/emulators/nestwl.nds";
		}
		err = runNdsFile(argarray[0], argarray.size(), (const char **)&argarray[0], true, true, false, true, true); // Pass ROM to nesDS as argument
	}
	else if (ms().launchType[ms().secondaryDevice] == Launch::EGameYobLaunch)
	{
		if (access(ms().romPath[ms().secondaryDevice].c_str(), F_OK) != 0) return;	// Skip to running TWiLight Menu++

		if (sys().flashcardUsed())
		{
			argarray.at(0) = (char*)"fat:/_nds/TWiLightMenu/emulators/gameyob.nds";
		}
		else
		{
			argarray.at(0) = (char*)"sd:/_nds/TWiLightMenu/emulators/gameyob.nds";
		}
		err = runNdsFile(argarray[0], argarray.size(), (const char **)&argarray[0], true, true, false, true, true); // Pass ROM to GameYob as argument
	}
	else if (ms().launchType[ms().secondaryDevice] == Launch::ES8DSLaunch)
	{
		if (access(ms().romPath[ms().secondaryDevice].c_str(), F_OK) != 0) return;	// Skip to running TWiLight Menu++

		if (sys().flashcardUsed())
		{
			mkdir("fat:/data", 0777);
			mkdir("fat:/data/s8ds", 0777);
			argarray.at(0) = (char*)"fat:/_nds/TWiLightMenu/emulators/S8DS.nds";
		}
		else
		{
			mkdir("sd:/data", 0777);
			mkdir("sd:/data/s8ds", 0777);
			argarray.at(0) = (char*)"sd:/_nds/TWiLightMenu/emulators/S8DS.nds";
		}
		err = runNdsFile(argarray[0], argarray.size(), (const char **)&argarray[0], true, true, false, true, true); // Pass ROM to S8DS as argument
	}
	else if (ms().launchType[ms().secondaryDevice] == Launch::ERVideoLaunch)
	{
		if (access(ms().romPath[ms().secondaryDevice].c_str(), F_OK) != 0) return;	// Skip to running TWiLight Menu++

		if (sys().flashcardUsed())
		{
			argarray.at(0) = (char*)"fat:/_nds/TWiLightMenu/apps/RocketVideoPlayer.nds";
		}
		else
		{
			argarray.at(0) = (char*)"sd:/_nds/TWiLightMenu/apps/RocketVideoPlayer.nds";
		}
		err = runNdsFile(argarray[0], argarray.size(), (const char **)&argarray[0], true, true, false, true, true); // Pass video to Rocket Video Player as argument
	}
	else if (ms().launchType[ms().secondaryDevice] == Launch::EMPEG4Launch)
	{
		if (access(ms().romPath[ms().secondaryDevice].c_str(), F_OK) != 0) return;	// Skip to running TWiLight Menu++

		if (sys().flashcardUsed())
		{
			argarray.at(0) = (char*)"fat:/_nds/TWiLightMenu/apps/MPEG4Player.nds";
		}
		else
		{
			argarray.at(0) = (char*)"sd:/_nds/TWiLightMenu/apps/MPEG4Player.nds";
		}
		err = runNdsFile(argarray[0], argarray.size(), (const char **)&argarray[0], true, true, false, true, true); // Pass video to MPEG4Player as argument
	}
	else if (ms().launchType[ms().secondaryDevice] == Launch::EStellaDSLaunch)
	{
		if (access(ms().romPath[ms().secondaryDevice].c_str(), F_OK) != 0) return;	// Skip to running TWiLight Menu++

		if (sys().flashcardUsed())
		{
			argarray.at(0) = (char*)"fat:/_nds/TWiLightMenu/emulators/StellaDS.nds";
		}
		else
		{
			argarray.at(0) = (char*)"sd:/_nds/TWiLightMenu/emulators/StellaDS.nds";
		}
		err = runNdsFile(argarray[0], argarray.size(), (const char **)&argarray[0], true, true, false, true, true); // Pass ROM to StellaDS as argument
	}
	else if (ms().launchType[ms().secondaryDevice] == Launch::EPicoDriveTWLLaunch && ms().showMd >= 2)
	{
		if (access(ms().romPath[ms().secondaryDevice].c_str(), F_OK) != 0) return;	// Skip to running TWiLight Menu++

		if (sys().flashcardUsed())
		{
			argarray.at(0) = (char*)"fat:/_nds/TWiLightMenu/emulators/PicoDriveTWL.nds";
		}
		else
		{
			argarray.at(0) = (char*)"sd:/_nds/TWiLightMenu/emulators/PicoDriveTWL.nds";
		}
		err = runNdsFile(argarray[0], argarray.size(), (const char **)&argarray[0], true, true, false, true, true); // Pass ROM to PicoDrive TWL as argument
	}
	if (err > 0) {
		consoleDemoInit();
		printf("Start failed. Error %i\n", err);
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

void defaultExitHandler()
{
	/*if (!sys().arm7SCFGLocked())
	{
		rebootDSiMenuPP();
	}*/
	loadROMselect(ms().theme);
}

//---------------------------------------------------------------------------------
int main(int argc, char **argv)
{
//---------------------------------------------------------------------------------

	// overwrite reboot stub identifier
	/*extern char *fake_heap_end;
	*fake_heap_end = 0;*/

	sys().initFilesystem("/_nds/TWiLightMenu/main.srldr");
	sys().flashcardUsed();
	ms();
	defaultExceptionHandler();

	if (!sys().fatInitOk())
	{
		consoleDemoInit();
		printf("fatInitDefault failed!");
		stop();
	}

	useTwlCfg = (isDSiMode() && (*(u8*)0x02000400 & 0x0F) && (*(u8*)0x02000401 == 0) && (*(u8*)0x02000402 == 0) && (*(u8*)0x02000404 == 0));
	if (isDSiMode()) {
		u16 cfgCrc = swiCRC16(0xFFFF, (void*)0x02000400, 0x128);
		u16 cfgCrcFromFile = 0;

		char twlCfgPath[256];
		sprintf(twlCfgPath, "%s:/_nds/TWiLightMenu/16KBcache.bin", sdFound() ? "sd" : "fat");

		FILE* twlCfg = fopen(twlCfgPath, "rb");
		if (twlCfg) {
			fseek(twlCfg, 0x4000, SEEK_SET);
			fread((u16*)&cfgCrcFromFile, sizeof(u16), 1, twlCfg);
		}
		if (useTwlCfg) {
			if (cfgCrcFromFile != cfgCrc) {
				fclose(twlCfg);
				// Cache first 16KB containing TWLCFG, in case some homebrew overwrites it
				twlCfg = fopen(twlCfgPath, "wb");
				fwrite((void*)0x02000000, 1, 0x4000, twlCfg);
				fwrite((u16*)&cfgCrc, sizeof(u16), 1, twlCfg);
			}
		} else if (twlCfg) {
			if (cfgCrc != cfgCrcFromFile) {
				// Reload first 16KB from cache
				fseek(twlCfg, 0, SEEK_SET);
				fread((void*)0x02000000, 1, 0x4000, twlCfg);
				useTwlCfg = ((*(u8*)0x02000400 & 0x0F) && (*(u8*)0x02000401 == 0) && (*(u8*)0x02000402 == 0) && (*(u8*)0x02000404 == 0));
			}
		}
		fclose(twlCfg);
	}

	ms().loadSettings();
	bs().loadSettings();

	if (isDSiMode() && ms().consoleModel < 2) {
		if (ms().wifiLed == -1) {
			if (*(u8*)(0x023FFD01) == 0x13) {
				ms().wifiLed = true;
			} else if (*(u8*)(0x023FFD01) == 0 || *(u8*)(0x023FFD01) == 0x12) {
				ms().wifiLed = false;
			}
		} else {
			*(u8*)(0x023FFD00) = (ms().wifiLed ? 0x13 : 0);		// WiFi On/Off
		}
	}

	if (ms().dsiWareExploit == 0) {
		if (access("sd:/_nds/nds-bootstrap/srBackendId.bin", F_OK) == 0)
			remove("sd:/_nds/nds-bootstrap/srBackendId.bin");
	}

	if (sdFound()) {
		mkdir("sd:/_gba", 0777);
		mkdir("sd:/_nds/TWiLightMenu/gamesettings", 0777);
	}
	if (flashcardFound()) {
		mkdir("fat:/_gba", 0777);
		mkdir("fat:/_nds/TWiLightMenu/gamesettings", 0777);
	}

	runGraphicIrq();

	if (REG_SCFG_EXT != 0) {
		*(vu32*)(0x0DFFFE0C) = 0x53524C41;		// Check for 32MB of RAM
		bool isDevConsole = (*(vu32*)(0x0DFFFE0C) == 0x53524C41);
		if (isDevConsole)
		{
			// Check for Nintendo 3DS console
			if (isDSiMode() && sdFound())
			{
				bool is3DS = !nandio_startup();
				int resultModel = 1+is3DS;
				if (ms().consoleModel != resultModel || bs().consoleModel != resultModel)
				{
					ms().consoleModel = resultModel;
					bs().consoleModel = resultModel;
					ms().saveSettings();
					bs().saveSettings();
				}
			}
			else if (ms().consoleModel < 1 || ms().consoleModel > 2
				  || bs().consoleModel < 1 || bs().consoleModel > 2)
			{
				consoleModelSelect();			// There's no NAND or SD card
			}
		}
		else
		if (ms().consoleModel != 0 || bs().consoleModel != 0)
		{
			ms().consoleModel = 0;
			bs().consoleModel = 0;
			ms().saveSettings();
			bs().saveSettings();
		}
	}

	bool soundBankLoaded = false;

	bool softResetParamsFound = false;
	const char* softResetParamsPath = (isDSiMode() ? (sdFound() ? "sd:/_nds/nds-bootstrap/softResetParams.bin" : "fat:/_nds/nds-bootstrap/softResetParams.bin") : "fat:/_nds/nds-bootstrap/B4DS-softResetParams.bin");
	u32 softResetParams = 0;
	FILE* file = fopen(softResetParamsPath, "rb");
	if (file) {
		fread(&softResetParams, sizeof(u32), 1, file);
		softResetParamsFound = (softResetParams != 0xFFFFFFFF);
	}
	fclose(file);

	if (softResetParamsFound) {
		scanKeys();
		if (keysHeld() & KEY_B) {
			softResetParams = 0xFFFFFFFF;
			file = fopen(softResetParamsPath, "wb");
			fwrite(&softResetParams, sizeof(u32), 1, file);
			fclose(file);
			softResetParamsFound = false;
		}
	}

	if (!softResetParamsFound && (ms().dsiSplash || ms().showlogo)) {
		// Get date
		int birthMonth = (useTwlCfg ? *(u8*)0x02000446 : PersonalData->birthMonth);
		int birthDay = (useTwlCfg ? *(u8*)0x02000447 : PersonalData->birthDay);
		char soundBankPath[32], currentDate[16], birthDate[16], dateOutput[2][5];
		time_t Raw;
		time(&Raw);
		const struct tm *Time = localtime(&Raw);

		strftime(currentDate, sizeof(currentDate), "%m/%d", Time);
		if (birthMonth >= 1 && birthMonth < 10) {
			sprintf(dateOutput[0], "0%i", birthMonth);
		} else {
			sprintf(dateOutput[0], "%i", birthMonth);
		}
		if (birthDay >= 1 && birthDay < 10) {
			sprintf(dateOutput[1], "0%i", birthDay);
		} else {
			sprintf(dateOutput[1], "%i", birthDay);
		}
		sprintf(birthDate, "%s/%s", dateOutput[0], dateOutput[1]);

		sprintf(soundBankPath, "nitro:/soundbank%s.bin", (strcmp(currentDate, birthDate) == 0) ? "_bday" : "");

		// Load sound bank into memory
		FILE* soundBank = fopen(soundBankPath, "rb");
		fread((void*)0x02FA0000, 1, 0x58000, soundBank);
		fclose(soundBank);
		soundBankLoaded = true;
	}

	if (!softResetParamsFound && ms().dsiSplash && ((REG_SCFG_EXT != 0) ? fifoGetValue32(FIFO_USER_01) != 0x01 : *(u32*)0x02000000 != 1)) {
		BootSplashInit();
		(REG_SCFG_EXT != 0) ? fifoSendValue32(FIFO_USER_01, 10) : *(u32*)0x02000000 = 1;
	}

	if ((access(DSIMENUPP_INI, F_OK) != 0)
	|| (ms().theme < 0) || (ms().theme == 3) || (ms().theme > 5)) {
		// Create or modify "settings.ini"
		(ms().theme == 3) ? ms().theme = 2 : ms().theme = 0;
		ms().saveSettings();
	}

	if (access(BOOTSTRAP_INI, F_OK) != 0) {
		u64 driveSize = 0;
		int gbNumber = 0;
		struct statvfs st;
		if (statvfs(sdFound() ? "sd:/" : "fat:/", &st) == 0) {
			driveSize = st.f_bsize * st.f_blocks;
		}
		for (u64 i = 0; i <= driveSize; i += 0x40000000) {
			gbNumber++;	// Get GB number
		}
		bs().cacheFatTable = (gbNumber < 32);	// Enable saving FAT table cache, if SD/Flashcard is 32GB or less

		// Create "nds-bootstrap.ini"
		bs().saveSettings();
	}
	
	scanKeys();

	if (softResetParamsFound
	 || (ms().autorun && !(keysHeld() & KEY_B))
	 || (!ms().autorun && (keysHeld() & KEY_B)))
	{
		flashcardInit();
		lastRunROM();
	}

	if (!softResetParamsFound && ms().autostartSlot1 && isDSiMode() && REG_SCFG_MC != 0x11 && !flashcardFound() && !(keysHeld() & KEY_SELECT)) {
		if (ms().slot1LaunchMethod==0 || sys().arm7SCFGLocked()) {
			dsCardLaunch();
		} else if (ms().slot1LaunchMethod==2) {
			unlaunchRomBoot("cart:");
		} else {
			runNdsFile("/_nds/TWiLightMenu/slot1launch.srldr", 0, NULL, true, true, false, true, true);
		}
	}

	keysSetRepeat(25, 5);
	// snprintf(vertext, sizeof(vertext), "Ver %d.%d.%d   ", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH); // Doesn't work :(

	if (ms().showlogo)
	{
		if (!soundBankLoaded) {
			// Load sound bank into memory
			FILE* soundBank = fopen("nitro:/soundbank.bin", "rb");
			fread((void*)0x02FA0000, 1, 0x58000, soundBank);
			fclose(soundBank);
			soundBankLoaded = true;
		}

		loadTitleGraphics();
		fadeType = true;
		for (int i = 0; i < 15; i++)
		{
			swiWaitForVBlank();
		}
		twlMenuVideo();
	}

	scanKeys();

	if (keysHeld() & KEY_SELECT)
	{
		screenmode = 1;
	}

	srand(time(NULL));

	while (1) {
		if (screenmode == 1) {
			fadeColor = true;
			controlTopBright = true;
			controlBottomBright = true;
			fadeType = false;
			for (int i = 0; i < 25; i++) {
				swiWaitForVBlank();
			}
			if (!isDSiMode()) {
				chdir("fat:/");
			}
			runNdsFile("/_nds/TWiLightMenu/settings.srldr", 0, NULL, true, false, false, true, true);
		} else {
			flashcardInit();
			if (flashcardFound()) {
				mkdir("fat:/_gba", 0777);
				mkdir("fat:/_nds/TWiLightMenu/gamesettings", 0777);
			}
			if (ms().showMainMenu) {
				loadMainMenu();
			}
			loadROMselect(ms().theme);
		}
	}

	return 0;
}
