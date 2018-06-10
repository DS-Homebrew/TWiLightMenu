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

std::string bootstrapfilename;

static int consoleModel = 0;
/*	0 = Nintendo DSi (Retail)
	1 = Nintendo DSi (Dev/Panda)
	2 = Nintendo 3DS
	3 = New Nintendo 3DS	*/

static int donorSdkVer = 0;

static bool bootstrapFile = false;
static bool homebrewBootstrap = false;

static bool soundfreq = false;	// false == 32.73 kHz, true == 47.61 kHz

void LoadSettings(void) {
	// GUI
	CIniFile settingsini( settingsinipath );

	soundfreq = settingsini.GetInt("SRLOADER", "SOUND_FREQ", 0);
	consoleModel = settingsini.GetInt("SRLOADER", "CONSOLE_MODEL", 0);
	bootstrapFile = settingsini.GetInt("SRLOADER", "BOOTSTRAP_FILE", 0);
	homebrewBootstrap = settingsini.GetInt("SRLOADER", "HOMEBREW_BOOTSTRAP", 0);

	// nds-bootstrap
	CIniFile bootstrapini( bootstrapinipath );

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
	if (homebrewBootstrap) {
		bootstrapfilename = "sd:/_nds/hb-bootstrap.nds";
	} else if (consoleModel > 0) {
		if(donorSdkVer==5) {
			if (bootstrapFile) bootstrapfilename = "sd:/_nds/unofficial-bootstrap-sdk5.nds";
			else bootstrapfilename = "sd:/_nds/release-bootstrap-sdk5.nds";
		} else {
			if (bootstrapFile) bootstrapfilename = "sd:/_nds/unofficial-bootstrap.nds";
			else bootstrapfilename = "sd:/_nds/release-bootstrap.nds";
		}
	} else {
		if(donorSdkVer==5) {
			if (bootstrapFile) bootstrapfilename = "sd:/_nds/unofficial-dsi-bootstrap-sdk5.nds";
			else bootstrapfilename = "sd:/_nds/release-dsi-bootstrap-sdk5.nds";
		} else {
			if (bootstrapFile) bootstrapfilename = "sd:/_nds/unofficial-dsi-bootstrap.nds";
			else bootstrapfilename = "sd:/_nds/release-dsi-bootstrap.nds";
		}
	}
	
	return runNdsFile (bootstrapfilename.c_str(), 0, NULL, true);
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

	std::string filename;
	
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
