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
#include <stdio.h>
#include <fat.h>
#include <sys/stat.h>
#include <limits.h>

#include <string.h>
#include <unistd.h>

#include "nds_loader_arm9.h"

#include "inifile.h"

#include "perGameSettings.h"

const char* settingsinipath = "/_nds/TWiLightMenu/settings.ini";
const char* bootstrapinipath = "sd:/_nds/nds-bootstrap.ini";

std::string dsiWareSrlPath;
std::string dsiWarePubPath;
std::string dsiWarePrvPath;
std::string homebrewArg;
std::string bootstrapfilename;
std::string ndsPath;
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

static std::string romfolder;

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

static bool previousUsedDevice = false;	// true == secondary
static int launchType = 1;	// 0 = Slot-1, 1 = SD/Flash card, 2 = DSiWare, 3 = NES, 4 = (S)GB(C)
static bool bootstrapFile = false;
static bool homebrewBootstrap = false;

static bool soundfreq = false;	// false == 32.73 kHz, true == 47.61 kHz

static int bstrap_language = -1;
static bool boostCpu = false;	// false == NTR, true == TWL
static bool boostVram = false;
static bool bstrap_dsiMode = false;

TWL_CODE void LoadSettings(void) {
	// GUI
	CIniFile settingsini( settingsinipath );

	romfolder = settingsini.GetString("SRLOADER", "ROM_FOLDER", "sd:/");
	soundfreq = settingsini.GetInt("SRLOADER", "SOUND_FREQ", 0);
	consoleModel = settingsini.GetInt("SRLOADER", "CONSOLE_MODEL", 0);
	previousUsedDevice = settingsini.GetInt("SRLOADER", "PREVIOUS_USED_DEVICE", previousUsedDevice);
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
	homebrewArg = settingsini.GetString("SRLOADER", "HOMEBREW_ARG", "");
	homebrewBootstrap = settingsini.GetInt("SRLOADER", "HOMEBREW_BOOTSTRAP", 0);

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

	vector<char*> argarray;
	if (launchType > 2) {
		argarray.push_back(strdup("null"));
		argarray.push_back(strdup(homebrewArg.c_str()));
	}

	if (launchType == 0) {
		return runNdsFile ("/_nds/TWiLightMenu/slot1launch.srldr", 0, NULL, false);
	} else if (launchType == 1) {
		filename = ndsPath;
		const size_t last_slash_idx = filename.find_last_of("/");
		if (std::string::npos != last_slash_idx)
		{
			filename.erase(0, last_slash_idx + 1);
		}

		loadPerGameSettings(filename);
		if (homebrewBootstrap) {
			if (perGameSettings_bootstrapFile == -1) {
				bootstrapfilename = (bootstrapFile ? "sd:/_nds/nds-bootstrap-hb-nightly.nds" : "sd:/_nds/nds-bootstrap-hb-release.nds");
			} else {
				bootstrapfilename = (perGameSettings_bootstrapFile ? "sd:/_nds/nds-bootstrap-hb-nightly.nds" : "sd:/_nds/nds-bootstrap-hb-release.nds");
			}
		} else {
			char game_TID[5];

			FILE *f_nds_file = fopen(ndsPath.c_str(), "rb");

			fseek(f_nds_file, offsetof(sNDSHeadertitlecodeonly, gameCode), SEEK_SET);
			fread(game_TID, 1, 4, f_nds_file);
			game_TID[4] = 0;
			game_TID[3] = 0;

			fclose(f_nds_file);

			std::string savename = ReplaceAll(filename, ".nds", ".sav");
			std::string romFolderNoSlash = romfolder;
			RemoveTrailingSlashes(romFolderNoSlash);
			std::string savepath = romFolderNoSlash+"/saves/"+savename;

			if ((access(savepath.c_str(), F_OK) != 0) && (strcmp(game_TID, "###") != 0)) {
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
				printf("Save file created!\n");
				
				for (int i = 0; i < 30; i++) {
					swiWaitForVBlank();
				}

			}

			CIniFile bootstrapini( bootstrapinipath );
			if (perGameSettings_language == -2) {
				bootstrapini.SetInt( "NDS-BOOTSTRAP", "LANGUAGE", bstrap_language);
			} else {
				bootstrapini.SetInt( "NDS-BOOTSTRAP", "LANGUAGE", perGameSettings_language);
			}
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
			bootstrapini.SaveIniFile( bootstrapinipath );
			if (perGameSettings_bootstrapFile == -1) {
				bootstrapfilename = (bootstrapFile ? "sd:/_nds/nds-bootstrap-nightly.nds" : "sd:/_nds/nds-bootstrap-release.nds");
			} else {
				bootstrapfilename = (perGameSettings_bootstrapFile ? "sd:/_nds/nds-bootstrap-nightly.nds" : "sd:/_nds/nds-bootstrap-release.nds");
			}
		}
		return runNdsFile (bootstrapfilename.c_str(), 0, NULL, true);
	} else if (launchType == 2) {
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
		*(u16*)(0x0200080C) = 0x3F0;		// Unlaunch Length for CRC16 (fixed, must be 3F0h)
		*(u16*)(0x0200080E) = 0;			// Unlaunch CRC16 (empty)
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

		fifoSendValue32(FIFO_USER_08, 1);	// Reboot
		for (int i = 0; i < 15; i++) swiIntrWait(0, 1);
	} else if (launchType == 3) {
		argarray.at(0) = "sd:/_nds/TWiLightMenu/emulators/nestwl.nds";
		return runNdsFile ("sd:/_nds/TWiLightMenu/emulators/nestwl.nds", argarray.size(), (const char **)&argarray[0], true);	// Pass ROM to nesDS as argument
	} else if (launchType == 4) {
		argarray.at(0) = "sd:/_nds/TWiLightMenu/emulators/gameyob.nds";
		return runNdsFile ("sd:/_nds/TWiLightMenu/emulators/gameyob.nds", argarray.size(), (const char **)&argarray[0], true);	// Pass ROM to GameYob as argument
	}
}

//---------------------------------------------------------------------------------
int main(int argc, char **argv) {
//---------------------------------------------------------------------------------

	// overwrite reboot stub identifier
	extern u64 *fake_heap_end;
	*fake_heap_end = 0;

	defaultExceptionHandler();

	if (!fatInitDefault()) {
		consoleDemoInit();
		printf("fatInitDefault failed!");
		stop();
	}

	fifoWaitValue32(FIFO_USER_06);

	int err = lastRunROM();
	consoleDemoInit();
	iprintf ("Start failed. Error %i", err);
	stop();

	return 0;
}
