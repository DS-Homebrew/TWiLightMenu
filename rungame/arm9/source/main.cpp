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

const char* settingsinipath = "/_nds/dsimenuplusplus/settings.ini";
const char* bootstrapinipath = "sd:/_nds/nds-bootstrap.ini";

std::string dsiWareSrlPath;
std::string dsiWarePubPath;
std::string dsiWarePrvPath;
std::string homebrewArg;
std::string bootstrapfilename;
std::string ndsPath;

typedef struct {
	char gameTitle[12];			//!< 12 characters for the game title.
	char gameCode[4];			//!< 4 characters for the game code.
} sNDSHeadertitlecodeonly;

static int consoleModel = 0;
/*	0 = Nintendo DSi (Retail)
	1 = Nintendo DSi (Dev/Panda)
	2 = Nintendo 3DS
	3 = New Nintendo 3DS	*/

static int donorSdkVer = 0;

static bool previousUsedDevice = false;	// true == secondary
static int launchType = 1;	// 0 = Slot-1, 1 = SD/Flash card, 2 = DSiWare, 3 = NES, 4 = (S)GB(C)
static bool bootstrapFile = false;
static bool homebrewBootstrap = false;

static bool soundfreq = false;	// false == 32.73 kHz, true == 47.61 kHz

void LoadSettings(void) {
	// GUI
	CIniFile settingsini( settingsinipath );

	soundfreq = settingsini.GetInt("SRLOADER", "SOUND_FREQ", 0);
	consoleModel = settingsini.GetInt("SRLOADER", "CONSOLE_MODEL", 0);
	previousUsedDevice = settingsini.GetInt("SRLOADER", "PREVIOUS_USED_DEVICE", previousUsedDevice);
	bootstrapFile = settingsini.GetInt("SRLOADER", "BOOTSTRAP_FILE", 0);
	launchType = settingsini.GetInt("SRLOADER", "LAUNCH_TYPE", 1);
	dsiWareSrlPath = settingsini.GetString("SRLOADER", "DSIWARE_SRL", "");
	dsiWarePubPath = settingsini.GetString("SRLOADER", "DSIWARE_PUB", "");
	dsiWarePrvPath = settingsini.GetString("SRLOADER", "DSIWARE_PRV", "");
	homebrewArg = settingsini.GetString("SRLOADER", "HOMEBREW_ARG", "");
	homebrewBootstrap = settingsini.GetInt("SRLOADER", "HOMEBREW_BOOTSTRAP", 0);

	// nds-bootstrap
	CIniFile bootstrapini( bootstrapinipath );

	ndsPath = bootstrapini.GetString( "NDS-BOOTSTRAP", "NDS_PATH", "");
	donorSdkVer = bootstrapini.GetInt( "NDS-BOOTSTRAP", "DONOR_SDK_VER", 0);
}

static bool arm7SCFGLocked = false;

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

int lastRanROM() {
	if(soundfreq) fifoSendValue32(FIFO_USER_07, 2);
	else fifoSendValue32(FIFO_USER_07, 1);

	vector<char*> argarray;
	if (launchType > 2) {
		argarray.push_back(strdup("null"));
		argarray.push_back(strdup(homebrewArg.c_str()));
	}

	if (launchType == 0) {
		return runNdsFile ("/_nds/dsimenuplusplus/slot1launch.srldr", 0, NULL, false);
	} else if (launchType == 1) {
		if (homebrewBootstrap) {
			if (bootstrapFile) bootstrapfilename = "sd:/_nds/nds-bootstrap-hb-nightly.nds";
			else bootstrapfilename = "sd:/_nds/nds-bootstrap-hb-release.nds";
		} else {
			char game_TID[5];

			FILE *f_nds_file = fopen(ndsPath.c_str(), "rb");

			fseek(f_nds_file, offsetof(sNDSHeadertitlecodeonly, gameCode), SEEK_SET);
			fread(game_TID, 1, 4, f_nds_file);
			game_TID[4] = 0;
			game_TID[3] = 0;

			fclose(f_nds_file);

			std::string savename = ReplaceAll(ndsPath, ".nds", ".sav");

			if (access(savename.c_str(), F_OK) && strcmp(game_TID, "###") != 0) {
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

				FILE *pFile = fopen(savename.c_str(), "wb");
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

			if (bootstrapFile) bootstrapfilename = "sd:/_nds/nds-bootstrap-nightly.nds";
			else bootstrapfilename = "sd:/_nds/nds-bootstrap-release.nds";
		}
		return runNdsFile (bootstrapfilename.c_str(), 0, NULL, true);
	} else if (launchType == 2) {
		if (!previousUsedDevice) {
			if (!access(dsiWareSrlPath.c_str(), F_OK) && access("sd:/bootthis.dsi", F_OK))
				rename (dsiWareSrlPath.c_str(), "sd:/bootthis.dsi");	// Rename .nds file to "bootthis.dsi" for Unlaunch to boot it
			if (!access(dsiWarePubPath.c_str(), F_OK) && access("sd:/bootthis.pub", F_OK))
				rename (dsiWarePubPath.c_str(), "sd:/bootthis.pub");
			if (!access(dsiWarePrvPath.c_str(), F_OK) && access("sd:/bootthis.prv", F_OK))
				rename (dsiWarePrvPath.c_str(), "sd:/bootthis.prv");
		}
		fifoSendValue32(FIFO_USER_08, 1);	// Reboot
	} else if (launchType == 3) {
		argarray.at(0) = "sd:/_nds/dsimenuplusplus/emulators/nestwl.nds";
		return runNdsFile ("sd:/_nds/dsimenuplusplus/emulators/nestwl.nds", argarray.size(), (const char **)&argarray[0], true);	// Pass ROM to nesDS as argument
	} else if (launchType == 4) {
		argarray.at(0) = "sd:/_nds/dsimenuplusplus/emulators/gameyob.nds";
		return runNdsFile ("sd:/_nds/dsimenuplusplus/emulators/gameyob.nds", argarray.size(), (const char **)&argarray[0], true);	// Pass ROM to GameYob as argument
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

	bool soundfreqsetting = false;

	LoadSettings();
	
	swiWaitForVBlank();

	fifoWaitValue32(FIFO_USER_06);
	if (fifoGetValue32(FIFO_USER_03) == 0) arm7SCFGLocked = true;	// If DSiMenu++ is being ran from DSiWarehax or flashcard, then arm7 SCFG is locked.

	u16 arm7_SNDEXCNT = fifoGetValue32(FIFO_USER_07);
	if (arm7_SNDEXCNT != 0) soundfreqsetting = true;
	fifoSendValue32(FIFO_USER_07, 0);

	int err = lastRanROM();
	consoleDemoInit();
	iprintf ("Start failed. Error %i", err);
	stop();

	return 0;
}
