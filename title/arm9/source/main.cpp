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
#include <cstdio>
#include <fat.h>
#include <sys/stat.h>
#include <limits.h>
#include <variant>
#include <string.h>
#include <unistd.h>
#include <maxmod9.h>
#include "common/gl2d.h"

#include "autoboot.h"

#include "graphics/graphics.h"

#include "common/nds_loader_arm9.h"
#include "common/inifile.h"
#include "common/nitrofs.h"
#include "common/bootstrappaths.h"
#include "common/dsimenusettings.h"
#include "common/perGameSettings.h"
#include "common/cardlaunch.h"
#include "bootstrapsettings.h"
#include "bootsplash.h"
#include "consolemodelselect.h"

#include "sr_data_srllastran.h"			 // For rebooting into the game (NTR-mode touch screen)
#include "sr_data_srllastran_twltouch.h" // For rebooting into the game (TWL-mode touch screen)
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

//---------------------------------------------------------------------------------
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
}

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
	fifoSendValue32(FIFO_USER_07, 0);
	if (ms().soundfreq)
		fifoSendValue32(FIFO_USER_07, 2);
	else
		fifoSendValue32(FIFO_USER_07, 1);

	fadeType = false;
	fifoSendValue32(FIFO_USER_01, 1); // Fade out sound
	for (int i = 0; i < 25; i++)
		swiWaitForVBlank();
	fifoSendValue32(FIFO_USER_01, 0); // Cancel sound fade out

	// if (soundfreqsettingChanged)
	// {
	// 	if (ms().soundfreq)
	// 		fifoSendValue32(FIFO_USER_07, 2);
	// 	else
	// 		fifoSendValue32(FIFO_USER_07, 1);
	// }

	runNdsFile("/_nds/TWiLightMenu/mainmenu.srldr", 0, NULL, false);
	stop();
}

void loadROMselect()
{
	fifoSendValue32(FIFO_USER_07, 0);
	if (ms().soundfreq)
		fifoSendValue32(FIFO_USER_07, 2);
	else
		fifoSendValue32(FIFO_USER_07, 1);

	fadeType = false;
	fifoSendValue32(FIFO_USER_01, 1); // Fade out sound
	for (int i = 0; i < 25; i++)
		swiWaitForVBlank();
	fifoSendValue32(FIFO_USER_01, 0); // Cancel sound fade out

	// if (soundfreqsettingChanged)
	// {
	// 	if (ms().soundfreq)
	// 		fifoSendValue32(FIFO_USER_07, 2);
	// 	else
	// 		fifoSendValue32(FIFO_USER_07, 1);
	// }
	if (ms().theme == 3)
	{
		runNdsFile("/_nds/TWiLightMenu/akmenu.srldr", 0, NULL, false);
	}
	else if (ms().theme == 2)
	{
		runNdsFile("/_nds/TWiLightMenu/r4menu.srldr", 0, NULL, false);
	}
	else
	{
		runNdsFile("/_nds/TWiLightMenu/dsimenu.srldr", 0, NULL, false);
	}
	stop();
}

int lastRunROM()
{
	fifoSendValue32(FIFO_USER_07, 0);
	if (ms().soundfreq)
		fifoSendValue32(FIFO_USER_07, 2);
	else
		fifoSendValue32(FIFO_USER_07, 1);

	fifoSendValue32(FIFO_USER_01, 1); // Fade out sound
	for (int i = 0; i < 25; i++)
		swiWaitForVBlank();
	fifoSendValue32(FIFO_USER_01, 0); // Cancel sound fade out

	// if (soundfreqsettingChanged)
	// {
	// 	if (ms().soundfreq)
	// 		fifoSendValue32(FIFO_USER_07, 2);
	// 	else
	// 		fifoSendValue32(FIFO_USER_07, 1);
	// }

	vector<char *> argarray;
	if (ms().launchType > 2)
	{
		argarray.push_back(strdup("null"));
		argarray.push_back(strdup(homebrewArg.c_str()));
	}

	int err = 0;
	if (ms().launchType == 0)
	{
		err = runNdsFile("/_nds/TWiLightMenu/slot1launch.srldr", 0, NULL, true);
	}
	else if (ms().launchType == 1)
	{
		if (ms().useBootstrap || isDSiMode())
		{
			CIniFile bootstrapini( BOOTSTRAP_INI );
			std::string filename = bootstrapini.GetString( "NDS-BOOTSTRAP", "NDS_PATH", "");

			const size_t last_slash_idx = filename.find_last_of("/");
			if (std::string::npos != last_slash_idx)
			{
				filename.erase(0, last_slash_idx + 1);
			}

			argarray.push_back(strdup(filename.c_str()));

			loadPerGameSettings(filename);
			if (access("fat:/", F_OK) == 0) {
				if (perGameSettings_bootstrapFile == -1) {
					if (ms().homebrewBootstrap) {
						argarray.at(0) = (char*)(ms().bootstrapFile ? "fat:/_nds/nds-bootstrap-hb-nightly.nds" : "fat:/_nds/nds-bootstrap-hb-release.nds");
					} else {
						argarray.at(0) = (char*)(ms().bootstrapFile ? "fat:/_nds/nds-bootstrap-nightly.nds" : "fat:/_nds/nds-bootstrap-release.nds");
					}
				} else {
					if (ms().homebrewBootstrap) {
						argarray.at(0) = (char*)(perGameSettings_bootstrapFile ? "fat:/_nds/nds-bootstrap-hb-nightly.nds" : "fat:/_nds/nds-bootstrap-hb-release.nds");
					} else {
						argarray.at(0) = (char*)(perGameSettings_bootstrapFile ? "fat:/_nds/nds-bootstrap-nightly.nds" : "fat:/_nds/nds-bootstrap-release.nds");
					}
				}
			} else {
				if (perGameSettings_bootstrapFile == -1) {
					if (ms().homebrewBootstrap) {
						argarray.at(0) = (char*)(ms().bootstrapFile ? "sd:/_nds/nds-bootstrap-hb-nightly.nds" : "sd:/_nds/nds-bootstrap-hb-release.nds");
					} else {
						argarray.at(0) = (char*)(ms().bootstrapFile ? "sd:/_nds/nds-bootstrap-nightly.nds" : "sd:/_nds/nds-bootstrap-release.nds");
					}
				} else {
					if (ms().homebrewBootstrap) {
						argarray.at(0) = (char*)(perGameSettings_bootstrapFile ? "sd:/_nds/nds-bootstrap-hb-nightly.nds" : "sd:/_nds/nds-bootstrap-hb-release.nds");
					} else {
						argarray.at(0) = (char*)(perGameSettings_bootstrapFile ? "sd:/_nds/nds-bootstrap-nightly.nds" : "sd:/_nds/nds-bootstrap-release.nds");
					}
				}
			}
			err = runNdsFile(argarray[0], argarray.size(), (const char **)&argarray[0], true);
		}
		else
		{
			switch (ms().flashcard)
			{
			case 0:
			case 1:
			default:
				err = runNdsFile("fat:/YSMenu.nds", 0, NULL, true);
				break;
			case 2:
			case 4:
			case 5:
				err = runNdsFile("fat:/Wfwd.dat", 0, NULL, true);
				break;
			case 3:
				err = runNdsFile("fat:/Afwd.dat", 0, NULL, true);
				break;
			case 6:
				err = runNdsFile("fat:/_dstwo/autoboot.nds", 0, NULL, true);
				break;
			}
		}
	}
	else if (ms().launchType == 2)
	{
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
	}
	else if (ms().launchType == 3)
	{
		if (sys().flashcardUsed())
		{
			argarray.at(0) = (char*)"/_nds/TWiLightMenu/emulators/nesds.nds";
		}
		else
		{
			argarray.at(0) = (char*)"sd:/_nds/TWiLightMenu/emulators/nestwl.nds";
		}
		err = runNdsFile(argarray[0], argarray.size(), (const char **)&argarray[0], true); // Pass ROM to nesDS as argument
	}
	else if (ms().launchType == 4)
	{
		if (sys().flashcardUsed())
		{
			argarray.at(0) = (char*)"/_nds/TWiLightMenu/emulators/gameyob.nds";
		}
		else
		{
			argarray.at(0) = (char*)"sd:/_nds/TWiLightMenu/emulators/gameyob.nds";
		}
		err = runNdsFile(argarray[0], argarray.size(), (const char **)&argarray[0], true); // Pass ROM to GameYob as argument
	}
	else if (ms().launchType == 5)
	{
		if (sys().flashcardUsed())
		{
			argarray.at(0) = (char*)"/_nds/TWiLightMenu/emulators/S8DS.nds";
		}
		else
		{
			argarray.at(0) = (char*)(!sys().arm7SCFGLocked() ? "sd:/_nds/TWiLightMenu/emulators/S8DS_notouch.nds" : "sd:/_nds/TWiLightMenu/emulators/S8DS.nds");
		}
		err = runNdsFile(argarray[0], argarray.size(), (const char **)&argarray[0], true); // Pass ROM to S8DS as argument
	}

	return err;
}

void defaultExitHandler()
{
	/*if (!sys().arm7SCFGLocked())
	{
		rebootDSiMenuPP();
	}*/
	loadROMselect();
}

//---------------------------------------------------------------------------------
int main(int argc, char **argv)
{
	//---------------------------------------------------------------------------------

	// Turn on screen backlights if they're disabled
	powerOn(PM_BACKLIGHT_TOP);
	powerOn(PM_BACKLIGHT_BOTTOM);

	//consoleDemoInit();
	//gotosettings = true;
	//bool fatInited = fatInitDefault();

	// overwrite reboot stub identifier
	extern u64 *fake_heap_end;
	*fake_heap_end = 0;

	sys().initFilesystem("/_nds/TWiLightMenu/main.srldr");
	sys().flashcardUsed();
	ms();
	// consoleDemoInit();
	// printf("%i", sys().flashcardUsed());
	// stop();
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

	if (isDSiMode()) {
		bool sdAccessible = false;
		if (access("sd:/", F_OK) == 0) {
			sdAccessible = true;
		}

		bool dsiSplashEnabled = false;
		if (sdAccessible) {
			CIniFile hiyacfwini(hiyacfwinipath);
			dsiSplashEnabled = hiyacfwini.GetInt("HIYA-CFW", "DSI_SPLASH", 1);
		}

		if (ms().consoleModel < 2 && dsiSplashEnabled && !sys().arm7SCFGLocked() && fifoGetValue32(FIFO_USER_01) != 0x01) {
			BootSplashInit();
			fifoSendValue32(FIFO_USER_01, 10);
		}
	}

	if (access(DSIMENUPP_INI, F_OK) != 0) {
		// Create "settings.ini"
		ms().saveSettings();
	}

	if (access(BOOTSTRAP_INI, F_OK) != 0) {
		// Create "nds-bootstrap.ini"
		bs().saveSettings();
	}

	scanKeys();

	if (ms().autorun && !(keysHeld() & KEY_B))
	{
		lastRunROM();
	}

	keysSetRepeat(25, 5);
	// snprintf(vertext, sizeof(vertext), "Ver %d.%d.%d   ", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH); // Doesn't work :(

	if (ms().autorun || ms().showlogo)
	{
		loadTitleGraphics();
		fadeType = true;

		for (int i = 0; i < 60 * 3; i++)
		{
			swiWaitForVBlank();
		}
	}

	scanKeys();

	if ((keysHeld() & KEY_START) || (keysHeld() & KEY_SELECT))
	{
		screenmode = 1;
	}

	srand(time(NULL));

	int pressed = 0;
	while (1)
	{
		if (screenmode == 1)
		{
			fadeType = false;
			for (int i = 0; i < 25; i++) {
				swiWaitForVBlank();
			}
			runNdsFile("/_nds/TWiLightMenu/settings.srldr", 0, NULL, false);
		}
		else
		{
			if (ms().showMainMenu)
			{
				loadMainMenu();
			}
			else
			{
				loadROMselect();
			}
		}
	}

	return 0;
}
