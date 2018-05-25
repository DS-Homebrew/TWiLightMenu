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

const char* settingsinipath = "/_nds/srloader/settings.ini";
const char* twldrsettingsinipath = "sd:/_nds/twloader/settings.ini";
const char* hiyacfwinipath = "sd:/hiya/settings.ini";
const char* bootstrapinipath = "sd:/_nds/nds-bootstrap.ini";

std::string bootstrapfilename;

static bool is3DS = false;

const char* consoleText = "";

static bool showlogo = true;
static bool gotosettings = false;

const char* romreadled_valuetext;
const char* useArm7Donor_valuetext;
const char* loadingScreen_valuetext;

static int bstrap_useArm7Donor = 1;
static int bstrap_loadingScreen = 1;

static int donorSdkVer = 0;

static bool bootstrapFile = false;

static bool ntr_touch = true;

static bool useGbarunner = false;
static bool autorun = false;
static int theme = 0;
static int subtheme = 0;
static bool showDirectories = true;

static bool bstrap_boostcpu = false;	// false == NTR, true == TWL
static bool bstrap_debug = false;
static int bstrap_romreadled = 0;
//static bool bstrap_lockARM9scfgext = false;

static bool soundfreq = false;	// false == 32.73 kHz, true == 47.61 kHz

static bool flashcardUsed = false;

static int flashcard;
/* Flashcard value
	0: DSTT/R4i Gold/R4i-SDHC/R4 SDHC Dual-Core/R4 SDHC Upgrade/SC DSONE
	1: R4DS (Original Non-SDHC version)/ M3 Simply
	2: R4iDSN/R4i Gold RTS/R4 Ultra
	3: Acekard 2(i)/Galaxy Eagle/M3DS Real
	4: Acekard RPG
	5: Ace 3DS+/Gateway Blue Card/R4iTT
	6: SuperCard DSTWO
*/

void LoadSettings(void) {
	// GUI
	CIniFile settingsini( settingsinipath );

	soundfreq = settingsini.GetInt("SRLOADER", "SOUND_FREQ", 0);
	bootstrapFile = settingsini.GetInt("SRLOADER", "BOOTSTRAP_FILE", 0);

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
	if (is3DS) {
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

	// Turn on screen backlights if they're disabled
	powerOn(PM_BACKLIGHT_TOP);
	powerOn(PM_BACKLIGHT_BOTTOM);

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
	if (fifoGetValue32(FIFO_USER_03) == 0) arm7SCFGLocked = true;	// If SRLoader is being ran from DSiWarehax or flashcard, then arm7 SCFG is locked.

	u16 arm7_SNDEXCNT = fifoGetValue32(FIFO_USER_07);
	if (arm7_SNDEXCNT != 0) soundfreqsetting = true;
	fifoSendValue32(FIFO_USER_07, 0);

	scanKeys();

	if(!flashcardUsed) {
		if(keysHeld() & KEY_UP) {
			is3DS = true;
		}
		if(keysHeld() & KEY_DOWN) {
			is3DS = false;
		}
	}

	int err = lastRanROM();
	consoleDemoInit();
	iprintf ("Start failed. Error %i", err);
	stop();

	return 0;
}
