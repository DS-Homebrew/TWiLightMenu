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
#include "bootstrapsettings.h"
#include "bootsplash.h"
#include "twlmenuppvideo.h"
#include "consolemodelselect.h"

#include "sr_data_srllastran.h"			 // For rebooting into the game
#include "common/systemdetails.h"

bool renderScreens = false;
bool fadeType = false; // false = out, true = in

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

std::string homebrewArg;

const char *unlaunchAutoLoadID = "AutoLoadInfo";
char hiyaNdsPath[14] = {'s','d','m','c',':','/','h','i','y','a','.','d','s','i'};

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

typedef DSiMenuPlusPlusSettings::TLaunchType Launch;

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
	fadeType = false;
	fifoSendValue32(FIFO_USER_01, 1); // Fade out sound
	for (int i = 0; i < 25; i++)
		swiWaitForVBlank();
	fifoSendValue32(FIFO_USER_01, 0); // Cancel sound fade out

	runNdsFile("/_nds/TWiLightMenu/mainmenu.srldr", 0, NULL, true, false, false, true, true);
}

void loadROMselect(int number)
{
	fadeType = false;
	fifoSendValue32(FIFO_USER_01, 1); // Fade out sound
	for (int i = 0; i < 25; i++)
		swiWaitForVBlank();
	fifoSendValue32(FIFO_USER_01, 0); // Cancel sound fade out

	switch (number) {
		case 3:
			runNdsFile("/_nds/TWiLightMenu/akmenu.srldr", 0, NULL, true, false, false, true, true);
			break;
		case 2:
			runNdsFile("/_nds/TWiLightMenu/r4menu.srldr", 0, NULL, true, false, false, true, true);
			break;
		default:
			runNdsFile("/_nds/TWiLightMenu/dsimenu.srldr", 0, NULL, true, false, false, true, true);
			break;
	}
	stop();
}

void lastRunROM()
{
	/*fifoSendValue32(FIFO_USER_01, 1); // Fade out sound
	for (int i = 0; i < 25; i++)
		swiWaitForVBlank();
	fifoSendValue32(FIFO_USER_01, 0); // Cancel sound fade out*/

	std::string romfolder = ms().romPath;
	while (!romfolder.empty() && romfolder[romfolder.size()-1] != '/') {
		romfolder.resize(romfolder.size()-1);
	}
	chdir(romfolder.c_str());

	std::string filename = ms().romPath;
	const size_t last_slash_idx = filename.find_last_of("/");
	if (std::string::npos != last_slash_idx)
	{
		filename.erase(0, last_slash_idx + 1);
	}

	// Set Widescreen
	if (!sys().arm7SCFGLocked() && ms().consoleModel >= 2 && ms().wideScreen
	&& (access("sd:/_nds/TWiLightMenu/TwlBg/Widescreen.cxi", F_OK) == 0)
	&& (access("/_nds/nds-bootstrap/wideCheatData.bin", F_OK) == 0)) {
		// Prepare for reboot into 16:10 TWL_FIRM
		rename("sd:/luma/sysmodules/TwlBg.cxi", "sd:/luma/sysmodules/TwlBg_bak.cxi");
		rename("sd:/_nds/TWiLightMenu/TwlBg/Widescreen.cxi", "sd:/luma/sysmodules/TwlBg.cxi");

		irqDisable(IRQ_VBLANK);				// Fix the throwback to 3DS HOME Menu bug
		memcpy((u32 *)0x02000300, sr_data_srllastran, 0x020);
		fifoSendValue32(FIFO_USER_02, 1); // Reboot in 16:10 widescreen
		stop();
	}

	vector<char *> argarray;
	if (ms().launchType > Launch::EDSiWareLaunch)
	{
		argarray.push_back(strdup("null"));
		argarray.push_back(strdup(homebrewArg.c_str()));
	}

	int err = 0;
	if (ms().launchType == Launch::ESlot1)
	{
		err = runNdsFile("/_nds/TWiLightMenu/slot1launch.srldr", 0, NULL, true, true, false, true, true);
	}
	else if (ms().launchType == Launch::ESDFlashcardLaunch)
	{
		if (access(ms().romPath.c_str(), F_OK) != 0) return;	// Skip to running TWiLight Menu++
		if ((ms().useBootstrap && !ms().homebrewBootstrap) || !ms().previousUsedDevice)
		{
			std::string savepath;

			argarray.push_back(strdup(filename.c_str()));

			loadPerGameSettings(filename);
			if (ms().homebrewBootstrap) {
				if (perGameSettings_bootstrapFile == -1) {
					argarray.at(0) = (char*)(ms().bootstrapFile ? "sd:/_nds/nds-bootstrap-hb-nightly.nds" : "sd:/_nds/nds-bootstrap-hb-release.nds");
				} else {
					argarray.at(0) = (char*)(perGameSettings_bootstrapFile ? "sd:/_nds/nds-bootstrap-hb-nightly.nds" : "sd:/_nds/nds-bootstrap-hb-release.nds");
				}
			} else {
				char game_TID[5];

				FILE *f_nds_file = fopen(filename.c_str(), "rb");

				fseek(f_nds_file, offsetof(sNDSHeadertitlecodeonly, gameCode), SEEK_SET);
				fread(game_TID, 1, 4, f_nds_file);
				game_TID[4] = 0;
				game_TID[3] = 0;

				fclose(f_nds_file);

				std::string savename = ReplaceAll(filename, ".nds", getSavExtension());
				std::string romFolderNoSlash = romfolder;
				RemoveTrailingSlashes(romFolderNoSlash);
				mkdir ("saves", 0777);
				savepath = romFolderNoSlash+"/saves/"+savename;
				if (ms().previousUsedDevice && ms().fcSaveOnSd) {
					savepath = ReplaceAll(savepath, "fat:/", "sd:/");
				}

				if ((getFileSize(savepath.c_str()) == 0) && (strcmp(game_TID, "###") != 0)) {
					consoleDemoInit();
					printf("Creating save file...\n");

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
					if ( strcmp(game_TID, "UOR") == 0	// WarioWare - D.I.Y. (Do It Yourself)
						|| strcmp(game_TID, "UXB") == 0 )	// Jam with the Band
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
					printf("Save file created!\n");
					
					for (int i = 0; i < 30; i++) {
						swiWaitForVBlank();
					}

				}
				if (perGameSettings_bootstrapFile == -1) {
					argarray.at(0) = (char*)(ms().bootstrapFile ? "sd:/_nds/nds-bootstrap-nightly.nds" : "sd:/_nds/nds-bootstrap-release.nds");
				} else {
					argarray.at(0) = (char*)(perGameSettings_bootstrapFile ? "sd:/_nds/nds-bootstrap-nightly.nds" : "sd:/_nds/nds-bootstrap-release.nds");
				}
			}
			CIniFile bootstrapini( sdFound() ? BOOTSTRAP_INI_SD : BOOTSTRAP_INI_FC );
			bootstrapini.SetString( "NDS-BOOTSTRAP", "NDS_PATH", ms().romPath);
			bootstrapini.SetString( "NDS-BOOTSTRAP", "SAV_PATH", savepath);
			if (perGameSettings_language == -2) {
				bootstrapini.SetInt( "NDS-BOOTSTRAP", "LANGUAGE", ms().bstrap_language);
			} else {
				bootstrapini.SetInt( "NDS-BOOTSTRAP", "LANGUAGE", perGameSettings_language);
			}
			if (perGameSettings_dsiMode == -1) {
				bootstrapini.SetInt( "NDS-BOOTSTRAP", "DSI_MODE", ms().bstrap_dsiMode);
			} else {
				bootstrapini.SetInt( "NDS-BOOTSTRAP", "DSI_MODE", perGameSettings_dsiMode);
			}
			if (perGameSettings_boostCpu == -1) {
				bootstrapini.SetInt( "NDS-BOOTSTRAP", "BOOST_CPU", ms().boostCpu);
			} else {
				bootstrapini.SetInt( "NDS-BOOTSTRAP", "BOOST_CPU", perGameSettings_boostCpu);
			}
			if (perGameSettings_boostVram == -1) {
				bootstrapini.SetInt( "NDS-BOOTSTRAP", "BOOST_VRAM", ms().boostVram);
			} else {
				bootstrapini.SetInt( "NDS-BOOTSTRAP", "BOOST_VRAM", perGameSettings_boostVram);
			}
			bootstrapini.SaveIniFile( sdFound() ? BOOTSTRAP_INI_SD : BOOTSTRAP_INI_FC );

			err = runNdsFile(argarray[0], argarray.size(), (const char **)&argarray[0], (ms().homebrewBootstrap ? false : true), true, false, true, true);
		}
		else
		{
			bool runNds_boostCpu = false;
			bool runNds_boostVram = false;
			if (isDSiMode()) {
				std::string filename = ms().romPath;
				const size_t last_slash_idx = filename.find_last_of("/");
				if (std::string::npos != last_slash_idx)
				{
					filename.erase(0, last_slash_idx + 1);
				}

				loadPerGameSettings(filename);
				if (perGameSettings_boostCpu == -1) {
					runNds_boostCpu = ms().boostCpu;
				} else {
					runNds_boostCpu = perGameSettings_boostCpu;
				}
				if (perGameSettings_boostVram == -1) {
					runNds_boostVram = ms().boostVram;
				} else {
					runNds_boostVram = perGameSettings_boostVram;
				}
			}
			std::string path;
			if (memcmp(io_dldi_data->friendlyName, "R4iDSN", 6) == 0) {
				CIniFile fcrompathini("fat:/_wfwd/lastsave.ini");
				path = ReplaceAll(ms().romPath, "fat:/", woodfat);
				fcrompathini.SetString("Save Info", "lastLoaded", path);
				fcrompathini.SaveIniFile("fat:/_wfwd/lastsave.ini");
				err = runNdsFile("fat:/Wfwd.dat", 0, NULL, true, true, true, runNds_boostCpu, runNds_boostVram);
			} else if (memcmp(io_dldi_data->friendlyName, "Acekard AK2", 0xB) == 0) {
				CIniFile fcrompathini("fat:/_afwd/lastsave.ini");
				path = ReplaceAll(ms().romPath, "fat:/", woodfat);
				fcrompathini.SetString("Save Info", "lastLoaded", path);
				fcrompathini.SaveIniFile("fat:/_afwd/lastsave.ini");
				err = runNdsFile("fat:/Afwd.dat", 0, NULL, true, true, true, runNds_boostCpu, runNds_boostVram);
			} else if (memcmp(io_dldi_data->friendlyName, "DSTWO(Slot-1)", 0xD) == 0) {
				CIniFile fcrompathini("fat:/_dstwo/autoboot.ini");
				path = ReplaceAll(ms().romPath, "fat:/", dstwofat);
				fcrompathini.SetString("Dir Info", "fullName", path);
				fcrompathini.SaveIniFile("fat:/_dstwo/autoboot.ini");
				err = runNdsFile("fat:/_dstwo/autoboot.nds", 0, NULL, true, true, true, runNds_boostCpu, runNds_boostVram);
			} else if (memcmp(io_dldi_data->friendlyName, "R4(DS) - Revolution for DS (v2)", 0xB) == 0) {
				CIniFile fcrompathini("fat:/__rpg/lastsave.ini");
				path = ReplaceAll(ms().romPath, "fat:/", woodfat);
				fcrompathini.SetString("Save Info", "lastLoaded", path);
				fcrompathini.SaveIniFile("fat:/__rpg/lastsave.ini");
				// Does not support autoboot; so only nds-bootstrap launching works.
				err = runNdsFile(path.c_str(), 0, NULL, true, true, true, runNds_boostCpu, runNds_boostVram);
			}
		}
	}
	else if (ms().launchType == Launch::ESDFlashcardDirectLaunch)
	{
		if (access(ms().romPath.c_str(), F_OK) != 0) return;	// Skip to running TWiLight Menu++

		argarray.push_back((char*)ms().romPath.c_str());

		bool runNds_boostCpu = false;
		bool runNds_boostVram = false;

		loadPerGameSettings(filename);

		runNds_boostCpu = perGameSettings_boostCpu == -1 ? ms().boostCpu : perGameSettings_boostCpu;
		runNds_boostVram = perGameSettings_boostVram == -1 ? ms().boostVram : perGameSettings_boostVram;

		err = runNdsFile (argarray[0], argarray.size(), (const char **)&argarray[0], true, true, (!perGameSettings_dsiMode ? true : false), runNds_boostCpu, runNds_boostVram);
	}
	else if (ms().launchType == Launch::EDSiWareLaunch)
	{
		if (access(ms().romPath.c_str(), F_OK) != 0) return;	// Skip to running TWiLight Menu++

		char unlaunchDevicePath[256];
		if (ms().previousUsedDevice) {
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
	else if (ms().launchType == Launch::ENESDSLaunch)
	{
		if (access(ms().romPath.c_str(), F_OK) != 0) return;	// Skip to running TWiLight Menu++

		if (sys().flashcardUsed())
		{
			argarray.at(0) = (char*)"/_nds/TWiLightMenu/emulators/nesds.nds";
		}
		else
		{
			argarray.at(0) = (char*)"sd:/_nds/TWiLightMenu/emulators/nestwl.nds";
		}
		err = runNdsFile(argarray[0], argarray.size(), (const char **)&argarray[0], true, true, false, true, true); // Pass ROM to nesDS as argument
	}
	else if (ms().launchType == Launch::EGameYobLaunch)
	{
		if (access(ms().romPath.c_str(), F_OK) != 0) return;	// Skip to running TWiLight Menu++

		if (sys().flashcardUsed())
		{
			argarray.at(0) = (char*)"/_nds/TWiLightMenu/emulators/gameyob.nds";
		}
		else
		{
			argarray.at(0) = (char*)"sd:/_nds/TWiLightMenu/emulators/gameyob.nds";
		}
		err = runNdsFile(argarray[0], argarray.size(), (const char **)&argarray[0], true, true, false, true, true); // Pass ROM to GameYob as argument
	}
	else if (ms().launchType == Launch::ES8DSLaunch)
	{
		if (access(ms().romPath.c_str(), F_OK) != 0) return;	// Skip to running TWiLight Menu++

		if (sys().flashcardUsed())
		{
			mkdir("fat:/data", 0777);
			mkdir("fat:/data/s8ds", 0777);
			argarray.at(0) = (char*)"/_nds/TWiLightMenu/emulators/S8DS.nds";
		}
		else
		{
			mkdir("sd:/data", 0777);
			mkdir("sd:/data/s8ds", 0777);
			argarray.at(0) = (char*)"sd:/_nds/TWiLightMenu/emulators/S8DS.nds";
		}
		err = runNdsFile(argarray[0], argarray.size(), (const char **)&argarray[0], true, true, false, true, true); // Pass ROM to S8DS as argument
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

	std::string filename;

	ms().loadSettings();
	bs().loadSettings();

	runGraphicIrq();

	swiWaitForVBlank();

	if (REG_SCFG_EXT != 0) {
		if (!isDSiMode()) {
			REG_SCFG_EXT = 0x8300C000;
		}
		*(vu32*)(0x0DFFFE0C) = 0x53524C41;		// Check for 32MB of RAM
		bool isDevConsole = (*(vu32*)(0x0DFFFE0C) == 0x53524C41);
		if (isDevConsole)
		{
			if (ms().consoleModel < 1 || ms().consoleModel > 3
			|| bs().consoleModel < 1 || bs().consoleModel > 3)
			{
				consoleModelSelect();
			}
		}
		else
		if (ms().consoleModel < 0 || ms().consoleModel > 0
		|| bs().consoleModel < 0 || bs().consoleModel > 0)
		{
			ms().consoleModel = 0;
			bs().consoleModel = 0;
			ms().saveSettings();
			bs().saveSettings();
		}
		if (!isDSiMode()) {
			REG_SCFG_EXT = 0x83000000;
		}
	}

	if (ms().dsiSplash || ms().showlogo) {
		// Load sound bank into memory
		FILE* soundBank = fopen("nitro:/soundbank.bin", "rb");
		fread((void*)0x023A0000, 1, 0x58000, soundBank);
		fclose(soundBank);
	}

	if (ms().dsiSplash && fifoGetValue32(FIFO_USER_01) != 0x01) {
		BootSplashInit();
		fifoSendValue32(FIFO_USER_01, 10);
	}

	if (access(DSIMENUPP_INI, F_OK) != 0) {
		// Create "settings.ini"
		ms().saveSettings();
	}

	mkdir("/_gba", 0777);

	if (access(BOOTSTRAP_INI, F_OK) != 0) {
		// Create "nds-bootstrap.ini"
		bs().saveSettings();
	}

	scanKeys();

	if ((ms().autorun && !(keysHeld() & KEY_B)) || (!ms().autorun && (keysHeld() & KEY_B)))
	{
		flashcardInit();
		lastRunROM();
	}

	keysSetRepeat(25, 5);
	// snprintf(vertext, sizeof(vertext), "Ver %d.%d.%d   ", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH); // Doesn't work :(

	if (ms().autorun || ms().showlogo)
	{
		loadTitleGraphics();
		fadeType = true;
		for (int i = 0; i < 25; i++)
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
			fadeType = false;
			for (int i = 0; i < 25; i++) {
				swiWaitForVBlank();
			}
			runNdsFile("/_nds/TWiLightMenu/settings.srldr", 0, NULL, true, false, false, true, true);
		} else {
			flashcardInit();
			if (ms().showMainMenu) {
				loadMainMenu();
			}
			loadROMselect(ms().theme);
		}
	}

	return 0;
}
