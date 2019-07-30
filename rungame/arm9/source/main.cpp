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
#include <stdio.h>
#include <fat.h>
#include <sys/stat.h>
#include <limits.h>

#include <string.h>
#include <unistd.h>

#include "nds_loader_arm9.h"

#include "inifile.h"

#include "perGameSettings.h"
#include "fileCopy.h"
#include "flashcard.h"

const char* settingsinipath = "sd:/_nds/TWiLightMenu/settings.ini";
const char* bootstrapinipath = "sd:/_nds/nds-bootstrap.ini";

std::string dsiWareSrlPath;
std::string dsiWarePubPath;
std::string dsiWarePrvPath;
std::string homebrewArg;
std::string ndsPath;
std::string romfolder;
std::string filename;

static const char *unlaunchAutoLoadID = "AutoLoadInfo";

typedef struct {
	char gameTitle[12];			//!< 12 characters for the game title.
	char gameCode[4];			//!< 4 characters for the game code.
} sNDSHeadertitlecodeonly;

static int consoleModel = 0;
/*	0 = Nintendo DSi (Retail)
	1 = Nintendo DSi (Dev/Panda)
	2 = Nintendo 3DS
	3 = New Nintendo 3DS	*/

static std::string romPath;

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

static int launchType = 1;	// 0 = Slot-1, 1 = SD/Flash card, 2 = SD/Flash card (Direct boot), 3 = DSiWare, 4 = NES, 5 = (S)GB(C), 6 = SMS/GG
static bool useBootstrap = true;
static bool bootstrapFile = false;
static bool homebrewBootstrap = false;
static bool fcSaveOnSd = false;
static bool wideScreen = false;

static bool soundfreq = false;	// false == 32.73 kHz, true == 47.61 kHz

static int bstrap_language = -1;
static bool boostCpu = false;	// false == NTR, true == TWL
static bool boostVram = false;
static bool bstrap_dsiMode = false;

TWL_CODE void LoadSettings(void) {
	// GUI
	CIniFile settingsini( settingsinipath );

	soundfreq = settingsini.GetInt("SRLOADER", "SOUND_FREQ", 0);
	consoleModel = settingsini.GetInt("SRLOADER", "CONSOLE_MODEL", 0);
	previousUsedDevice = settingsini.GetInt("SRLOADER", "PREVIOUS_USED_DEVICE", previousUsedDevice);
	fcSaveOnSd = settingsini.GetInt("SRLOADER", "FC_SAVE_ON_SD", fcSaveOnSd);
	useBootstrap = settingsini.GetInt("SRLOADER", "USE_BOOTSTRAP", useBootstrap);
	bootstrapFile = settingsini.GetInt("SRLOADER", "BOOTSTRAP_FILE", 0);

	// Default nds-bootstrap settings
	bstrap_language = settingsini.GetInt("NDS-BOOTSTRAP", "LANGUAGE", -1);
	boostCpu = settingsini.GetInt("NDS-BOOTSTRAP", "BOOST_CPU", 0);
	boostVram = settingsini.GetInt("NDS-BOOTSTRAP", "BOOST_VRAM", 0);
	bstrap_dsiMode = settingsini.GetInt("NDS-BOOTSTRAP", "DSI_MODE", 0);

	dsiWareSrlPath = settingsini.GetString("SRLOADER", "DSIWARE_SRL", "");
	dsiWarePubPath = settingsini.GetString("SRLOADER", "DSIWARE_PUB", "");
	dsiWarePrvPath = settingsini.GetString("SRLOADER", "DSIWARE_PRV", "");
	launchType = settingsini.GetInt("SRLOADER", "LAUNCH_TYPE", 1);
	romPath = settingsini.GetString("SRLOADER", "ROM_PATH", romPath);
	homebrewArg = settingsini.GetString("SRLOADER", "HOMEBREW_ARG", "");
	homebrewBootstrap = settingsini.GetInt("SRLOADER", "HOMEBREW_BOOTSTRAP", 0);

	wideScreen = settingsini.GetInt("SRLOADER", "WIDESCREEN", wideScreen);

	// nds-bootstrap
	CIniFile bootstrapini( bootstrapinipath );

	ndsPath = bootstrapini.GetString( "NDS-BOOTSTRAP", "NDS_PATH", "");
}

using namespace std;

//---------------------------------------------------------------------------------
void stop (void) {
//---------------------------------------------------------------------------------
	while (1) {
		swiWaitForVBlank();
	}
}

char filePath[PATH_MAX];

std::string ReplaceAll(std::string str, const std::string& from, const std::string& to) {
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
    }
    return str;
}

TWL_CODE int lastRunROM() {
	LoadSettings();

	if (consoleModel >= 2 && wideScreen && access("sd:/_nds/TWiLightMenu/TwlBg/Widescreen.cxi", F_OK) != 0) {
		// Revert back to 4:3 for when returning to TWLMenu++
		rename("sd:/luma/sysmodules/TwlBg.cxi", "sd:/_nds/TWiLightMenu/TwlBg/Widescreen.cxi");
		rename("sd:/luma/sysmodules/TwlBg_bak.cxi", "sd:/luma/sysmodules/TwlBg.cxi");
	}

	vector<char*> argarray;
	if (launchType > 3) {
		argarray.push_back(strdup("null"));
		argarray.push_back(strdup(homebrewArg.c_str()));
	}

	if (access(romPath.c_str(), F_OK) != 0 && launchType != 0) {
		return runNdsFile ("/_nds/TWiLightMenu/main.srldr", 0, NULL, true, false, false, true, true);	// Skip to running TWiLight Menu++
	}

	switch (launchType) {
		case 0:
			return runNdsFile ("/_nds/TWiLightMenu/slot1launch.srldr", 0, NULL, true, false, false, true, true);
		case 1:
			if ((useBootstrap && !homebrewBootstrap) || !previousUsedDevice)
			{
				std::string savepath;

				romfolder = romPath;
				while (!romfolder.empty() && romfolder[romfolder.size()-1] != '/') {
					romfolder.resize(romfolder.size()-1);
				}
				chdir(romfolder.c_str());

				filename = romPath;
				const size_t last_slash_idx = filename.find_last_of("/");
				if (std::string::npos != last_slash_idx)
				{
					filename.erase(0, last_slash_idx + 1);
				}

				argarray.push_back(strdup(filename.c_str()));

				loadPerGameSettings(filename);
				bool useNightly = (perGameSettings_bootstrapFile == -1 ? bootstrapFile : perGameSettings_bootstrapFile);

				if (!homebrewBootstrap) {
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
					if (previousUsedDevice && fcSaveOnSd) {
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
						if (strcmp(game_TID, "AZL") == 0	// Wagamama Fashion: Girls Mode/Style Savvy/Nintendo presents: Style Boutique/Namanui Collection: Girls Style
						 || strcmp(game_TID, "BKI") == 0)	// The Legend of Zelda: Spirit Tracks
						{
							savesize = 1048576;
						}

						// Set save size to 32MB for the following games
						if (strcmp(game_TID, "UOR") == 0	// WarioWare - D.I.Y. (Do It Yourself)
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
				}

				char ndsToBoot[256];
				sprintf(ndsToBoot, "sd:/_nds/nds-bootstrap-%s%s.nds", homebrewBootstrap ? "hb-" : "", useNightly ? "nightly" : "release");
				if(access(ndsToBoot, F_OK) != 0) {
					sprintf(ndsToBoot, "fat:/_nds/nds-bootstrap-%s%s.nds", homebrewBootstrap ? "hb-" : "", useNightly ? "nightly" : "release");
				}

				argarray.at(0) = (char *)ndsToBoot;
				CIniFile bootstrapini(bootstrapinipath);
				bootstrapini.SetString("NDS-BOOTSTRAP", "NDS_PATH", romPath);
				bootstrapini.SetString("NDS-BOOTSTRAP", "SAV_PATH", savepath);
				bootstrapini.SetInt("NDS-BOOTSTRAP", "LANGUAGE", perGameSettings_language == -2 ? bstrap_language : perGameSettings_language);
				bootstrapini.SetInt("NDS-BOOTSTRAP", "DSI_MODE", perGameSettings_dsiMode == -1 ? bstrap_dsiMode : perGameSettings_dsiMode);
				bootstrapini.SetInt("NDS-BOOTSTRAP", "BOOST_CPU", perGameSettings_boostCpu == -1 ? boostCpu : perGameSettings_boostCpu);
				bootstrapini.SetInt( "NDS-BOOTSTRAP", "BOOST_VRAM", perGameSettings_boostVram == -1 ? boostVram : perGameSettings_boostVram);
				bootstrapini.SaveIniFile(bootstrapinipath);

				return runNdsFile (argarray[0], argarray.size(), (const char **)&argarray[0], (homebrewBootstrap ? false : true), true, false, true, true);
			} else {
				std::string filename = romPath;
				const size_t last_slash_idx = filename.find_last_of("/");
				if (std::string::npos != last_slash_idx)
				{
					filename.erase(0, last_slash_idx + 1);
				}

				loadPerGameSettings(filename);
				bool runNds_boostCpu = perGameSettings_boostCpu == -1 ? boostCpu : perGameSettings_boostCpu;
				bool runNds_boostVram = perGameSettings_boostVram == -1 ? boostVram : perGameSettings_boostVram;

				std::string path;
				if (memcmp(io_dldi_data->friendlyName, "R4iDSN", 6) == 0) {
					CIniFile fcrompathini("fat:/_wfwd/lastsave.ini");
					path = ReplaceAll(romPath, "fat:/", woodfat);
					fcrompathini.SetString("Save Info", "lastLoaded", path);
					fcrompathini.SaveIniFile("fat:/_wfwd/lastsave.ini");
					return runNdsFile("fat:/Wfwd.dat", 0, NULL, true, true, true, runNds_boostCpu, runNds_boostVram);
				} else if (memcmp(io_dldi_data->friendlyName, "Acekard AK2", 0xB) == 0) {
					CIniFile fcrompathini("fat:/_afwd/lastsave.ini");
					path = ReplaceAll(romPath, "fat:/", woodfat);
					fcrompathini.SetString("Save Info", "lastLoaded", path);
					fcrompathini.SaveIniFile("fat:/_afwd/lastsave.ini");
					return runNdsFile("fat:/Afwd.dat", 0, NULL, true, true, true, runNds_boostCpu, runNds_boostVram);
				} else if (memcmp(io_dldi_data->friendlyName, "DSTWO(Slot-1)", 0xD) == 0) {
					CIniFile fcrompathini("fat:/_dstwo/autoboot.ini");
					path = ReplaceAll(romPath, "fat:/", dstwofat);
					fcrompathini.SetString("Dir Info", "fullName", path);
					fcrompathini.SaveIniFile("fat:/_dstwo/autoboot.ini");
					return runNdsFile("fat:/_dstwo/autoboot.nds", 0, NULL, true, true, true, runNds_boostCpu, runNds_boostVram);
				} else if (memcmp(io_dldi_data->friendlyName, "R4(DS) - Revolution for DS (v2)", 0xB) == 0) {
					CIniFile fcrompathini("fat:/__rpg/lastsave.ini");
					path = ReplaceAll(romPath, "fat:/", woodfat);
					fcrompathini.SetString("Save Info", "lastLoaded", path);
					fcrompathini.SaveIniFile("fat:/__rpg/lastsave.ini");
					// Does not support autoboot; so only nds-bootstrap launching works.
					return runNdsFile(path.c_str(), 0, NULL, true, true, true, runNds_boostCpu, runNds_boostVram);
				}
			}
		case 2: {
			romfolder = romPath;
			while (!romfolder.empty() && romfolder[romfolder.size()-1] != '/') {
				romfolder.resize(romfolder.size()-1);
			}
			chdir(romfolder.c_str());

			filename = romPath;
			const size_t last_slash_idx = filename.find_last_of("/");
			if (std::string::npos != last_slash_idx)
			{
				filename.erase(0, last_slash_idx + 1);
			}

			argarray.push_back((char*)romPath.c_str());

			loadPerGameSettings(filename);

			bool runNds_boostCpu = perGameSettings_boostCpu == -1 ? boostCpu : perGameSettings_boostCpu;
			bool runNds_boostVram = perGameSettings_boostVram == -1 ? boostVram : perGameSettings_boostVram;

			return runNdsFile (argarray[0], argarray.size(), (const char **)&argarray[0], true, true, (!perGameSettings_dsiMode ? true : false), runNds_boostCpu, runNds_boostVram);
		} case 3: {
			char unlaunchDevicePath[256];
			if (previousUsedDevice) {
				snprintf(unlaunchDevicePath, (int)sizeof(unlaunchDevicePath), "sdmc:/_nds/TWiLightMenu/tempDSiWare.dsi");
			} else {
				snprintf(unlaunchDevicePath, (int)sizeof(unlaunchDevicePath), "__%s", dsiWareSrlPath.c_str());
				unlaunchDevicePath[0] = 's';
				unlaunchDevicePath[1] = 'd';
				unlaunchDevicePath[2] = 'm';
				unlaunchDevicePath[3] = 'c';
			}

			memcpy((u8*)0x02000800, unlaunchAutoLoadID, 12);
			*(u16*)(0x0200080C) = 0x3F0;			// Unlaunch Length for CRC16 (fixed, must be 3F0h)
			*(u16*)(0x0200080E) = 0;			// Unlaunch CRC16 (empty)
			*(u32*)(0x02000810) |= BIT(0);			// Load the title at 2000838h
			*(u32*)(0x02000810) |= BIT(1);			// Use colors 2000814h
			*(u16*)(0x02000814) = 0x7FFF;			// Unlaunch Upper screen BG color (0..7FFFh)
			*(u16*)(0x02000816) = 0x7FFF;			// Unlaunch Lower screen BG color (0..7FFFh)
			memset((u8*)0x02000818, 0, 0x20+0x208+0x1C0);	// Unlaunch Reserved (zero)
			int i2 = 0;
			for (int i = 0; i < (int)sizeof(unlaunchDevicePath); i++) {
				*(u8*)(0x02000838+i2) = unlaunchDevicePath[i];		// Unlaunch Device:/Path/Filename.ext (16bit Unicode,end by 0000h)
				i2 += 2;
			}
			while (*(u16*)(0x0200080E) == 0) {	// Keep running, so that CRC16 isn't 0
				*(u16*)(0x0200080E) = swiCRC16(0xFFFF, (void*)0x02000810, 0x3F0);		// Unlaunch CRC16
			}

			fifoSendValue32(FIFO_USER_08, 1);	// Reboot
			for (int i = 0; i < 15; i++) swiWaitForVBlank();
			break;
		} case 4:
			argarray.at(0) = (char*)"sd:/_nds/TWiLightMenu/emulators/nestwl.nds";
			return runNdsFile (argarray[0], argarray.size(), (const char **)&argarray[0], true, true, false, true, true);	// Pass ROM to nesDS as argument
		case 5:
			argarray.at(0) = (char*)"sd:/_nds/TWiLightMenu/emulators/gameyob.nds";
			return runNdsFile (argarray[0], argarray.size(), (const char **)&argarray[0], true, true, false, true, true);	// Pass ROM to GameYob as argument
		case 6:
			mkdir("sd:/data", 0777);
			mkdir("sd:/data/s8ds", 0777);
			argarray.at(0) = (char*)"sd:/_nds/TWiLightMenu/emulators/S8DS.nds";
			return runNdsFile(argarray[0], argarray.size(), (const char **)&argarray[0], true, true, false, true, true); // Pass ROM to S8DS as argument
	}
	
	return -1;
}

//---------------------------------------------------------------------------------
int main(int argc, char **argv) {
//---------------------------------------------------------------------------------

	// overwrite reboot stub identifier
	extern char *fake_heap_end;
	*fake_heap_end = 0;

	defaultExceptionHandler();

	if (!fatInitDefault()) {
		consoleDemoInit();
		printf("fatInitDefault failed!");
		stop();
	}

	flashcardInit();

	fifoWaitValue32(FIFO_USER_06);

	int err = lastRunROM();
	consoleDemoInit();
	iprintf ("Start failed. Error %i", err);
	stop();

	return 0;
}
