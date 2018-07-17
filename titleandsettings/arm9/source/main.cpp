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
#include <maxmod9.h>
#include <gl2d.h>

#include "autoboot.h"

#include "graphics/graphics.h"

#include "nds_loader_arm9.h"

#include "graphics/fontHandler.h"

#include "inifile.h"

#include "language.h"

#include "soundbank.h"
#include "soundbank_bin.h"

#include "sr_data_srllastran.h"	// For rebooting into the game (NTR-mode touch screen)
#include "sr_data_srllastran_twltouch.h"	// For rebooting into the game (TWL-mode touch screen)

bool renderScreens = false;
bool fadeType = false;		// false = out, true = in

const char* settingsinipath = "/_nds/dsimenuplusplus/settings.ini";
const char* hiyacfwinipath = "sd:/hiya/settings.ini";
const char* bootstrapinipath = "sd:/_nds/nds-bootstrap.ini";

std::string dsiWareSrlPath;
std::string dsiWarePubPath;
std::string dsiWarePrvPath;
std::string homebrewArg;
std::string bootstrapfilename;

static int consoleModel = 0;
/*	0 = Nintendo DSi (Retail)
	1 = Nintendo DSi (Dev/Panda)
	2 = Nintendo 3DS
	3 = New Nintendo 3DS	*/

static bool showlogo = true;
static bool gotosettings = false;

int guiLanguage = -1;
static int bstrap_loadingScreen = 1;

static int donorSdkVer = 0;

static int launchType = 1;	// 0 = Slot-1, 1 = SD/Flash card, 2 = DSiWare, 3 = NES, 4 = (S)GB(C)
static bool slot1LaunchMethod = true;	// false == Reboot, true == Direct
static bool bootstrapFile = false;
static bool homebrewBootstrap = false;

static bool useGbarunner = false;
static bool autorun = false;
static int theme = 0;
static int subtheme = 0;
static bool showDirectories = true;
static bool showBoxArt = true;
static bool animateDsiIcons = false;

static int bstrap_language = -1;
static bool boostCpu = false;	// false == NTR, true == TWL
static bool bstrap_debug = false;
static int bstrap_romreadled = 0;
static bool bstrap_asyncPrefetch = true;
//static bool bstrap_lockARM9scfgext = false;

static bool soundfreq = false;	// false == 32.73 kHz, true == 47.61 kHz

bool flashcardUsed = false;

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

	guiLanguage = settingsini.GetInt("SRLOADER", "LANGUAGE", -1);
	useGbarunner = settingsini.GetInt("SRLOADER", "USE_GBARUNNER2", 0);
	autorun = settingsini.GetInt("SRLOADER", "AUTORUNGAME", 0);
	showlogo = settingsini.GetInt("SRLOADER", "SHOWLOGO", 1);
	gotosettings = settingsini.GetInt("SRLOADER", "GOTOSETTINGS", 0);
	soundfreq = settingsini.GetInt("SRLOADER", "SOUND_FREQ", 0);
	flashcard = settingsini.GetInt("SRLOADER", "FLASHCARD", 0);
	slot1LaunchMethod = settingsini.GetInt("SRLOADER", "SLOT1_LAUNCHMETHOD", 1);
	bootstrapFile = settingsini.GetInt("SRLOADER", "BOOTSTRAP_FILE", 0);
	launchType = settingsini.GetInt("SRLOADER", "LAUNCH_TYPE", 1);
	if (flashcardUsed && launchType == 0) launchType = 1;
	dsiWareSrlPath = settingsini.GetString("SRLOADER", "DSIWARE_SRL", "");
	dsiWarePubPath = settingsini.GetString("SRLOADER", "DSIWARE_PUB", "");
	dsiWarePrvPath = settingsini.GetString("SRLOADER", "DSIWARE_PRV", "");
	homebrewArg = settingsini.GetString("SRLOADER", "HOMEBREW_ARG", "");
	homebrewBootstrap = settingsini.GetInt("SRLOADER", "HOMEBREW_BOOTSTRAP", 0);
	consoleModel = settingsini.GetInt("SRLOADER", "CONSOLE_MODEL", 0);

	// Customizable UI settings.
	theme = settingsini.GetInt("SRLOADER", "THEME", 0);
	subtheme = settingsini.GetInt("SRLOADER", "SUB_THEME", 0);
	showDirectories = settingsini.GetInt("SRLOADER", "SHOW_DIRECTORIES", 1);
	showBoxArt = settingsini.GetInt("SRLOADER", "SHOW_BOX_ART", 1);
	animateDsiIcons = settingsini.GetInt("SRLOADER", "ANIMATE_DSI_ICONS", 1);

	// Default nds-bootstrap settings
	bstrap_language = settingsini.GetInt("NDS-BOOTSTRAP", "LANGUAGE", -1);
	boostCpu = settingsini.GetInt("NDS-BOOTSTRAP", "BOOST_CPU", 0);
	bstrap_asyncPrefetch = settingsini.GetInt("NDS-BOOTSTRAP", "ASYNC_PREFETCH", 1);

	if(!flashcardUsed) {
		// nds-bootstrap
		CIniFile bootstrapini( bootstrapinipath );

		bstrap_debug = bootstrapini.GetInt("NDS-BOOTSTRAP", "DEBUG", 0);
		bstrap_romreadled = bootstrapini.GetInt("NDS-BOOTSTRAP", "ROMREAD_LED", 0);
		donorSdkVer = bootstrapini.GetInt( "NDS-BOOTSTRAP", "DONOR_SDK_VER", 0);
		bstrap_loadingScreen = bootstrapini.GetInt( "NDS-BOOTSTRAP", "LOADING_SCREEN", 1);
		// bstrap_lockARM9scfgext = bootstrapini.GetInt("NDS-BOOTSTRAP", "LOCK_ARM9_SCFG_EXT", 0);
	}
}

void SaveSettings(void) {
	// GUI
	CIniFile settingsini( settingsinipath );

	settingsini.SetInt("SRLOADER", "LANGUAGE", guiLanguage);
	settingsini.SetInt("SRLOADER", "USE_GBARUNNER2", useGbarunner);
	settingsini.SetInt("SRLOADER", "AUTORUNGAME", autorun);
	settingsini.SetInt("SRLOADER", "SHOWLOGO", showlogo);
	settingsini.SetInt("SRLOADER", "GOTOSETTINGS", gotosettings);
	settingsini.SetInt("SRLOADER", "SOUND_FREQ", soundfreq);
	settingsini.SetInt("SRLOADER", "FLASHCARD", flashcard);
	settingsini.SetInt("SRLOADER", "SLOT1_LAUNCHMETHOD", slot1LaunchMethod);
	settingsini.SetInt("SRLOADER", "BOOTSTRAP_FILE", bootstrapFile);

	// UI settings.
	settingsini.SetInt("SRLOADER", "THEME", theme);
	settingsini.SetInt("SRLOADER", "SUB_THEME", subtheme);
	settingsini.SetInt("SRLOADER", "SHOW_DIRECTORIES", showDirectories);
	settingsini.SetInt("SRLOADER", "SHOW_BOX_ART", showBoxArt);
	settingsini.SetInt("SRLOADER", "ANIMATE_DSI_ICONS", animateDsiIcons);

	// Default nds-bootstrap settings
	settingsini.SetInt("NDS-BOOTSTRAP", "LANGUAGE", bstrap_language);
	settingsini.SetInt("NDS-BOOTSTRAP", "BOOST_CPU", boostCpu);
	settingsini.SetInt("NDS-BOOTSTRAP", "ASYNC_PREFETCH", bstrap_asyncPrefetch);
	settingsini.SaveIniFile(settingsinipath);

	if(!flashcardUsed) {
		// nds-bootstrap
		CIniFile bootstrapini( bootstrapinipath );

		bootstrapini.SetInt("NDS-BOOTSTRAP", "DEBUG", bstrap_debug);
		bootstrapini.SetInt("NDS-BOOTSTRAP", "ROMREAD_LED", bstrap_romreadled);
		bootstrapini.SetInt("NDS-BOOTSTRAP", "LOADING_SCREEN", bstrap_loadingScreen);
		// bootstrapini.SetInt("NDS-BOOTSTRAP", "LOCK_ARM9_SCFG_EXT", bstrap_lockARM9scfgext);
		bootstrapini.SaveIniFile(bootstrapinipath);
	}
}

int screenmode = 0;
int subscreenmode = 0;

static int settingscursor = 0;

static bool arm7SCFGLocked = false;

using namespace std;

mm_sound_effect snd_launch;
mm_sound_effect snd_select;
mm_sound_effect snd_stop;
mm_sound_effect snd_wrong;
mm_sound_effect snd_back;
mm_sound_effect snd_switch;

void InitSound() {
	mmInitDefaultMem((mm_addr)soundbank_bin);
	
	mmLoadEffect( SFX_LAUNCH );
	mmLoadEffect( SFX_SELECT );
	mmLoadEffect( SFX_STOP );
	mmLoadEffect( SFX_WRONG );
	mmLoadEffect( SFX_BACK );
	mmLoadEffect( SFX_SWITCH );

	snd_launch = {
		{ SFX_LAUNCH } ,			// id
		(int)(1.0f * (1<<10)),	// rate
		0,		// handle
		255,	// volume
		128,	// panning
	};
	snd_select = {
		{ SFX_SELECT } ,			// id
		(int)(1.0f * (1<<10)),	// rate
		0,		// handle
		255,	// volume
		128,	// panning
	};
	snd_stop = {
		{ SFX_STOP } ,			// id
		(int)(1.0f * (1<<10)),	// rate
		0,		// handle
		255,	// volume
		128,	// panning
	};
	snd_wrong = {
		{ SFX_WRONG } ,			// id
		(int)(1.0f * (1<<10)),	// rate
		0,		// handle
		255,	// volume
		128,	// panning
	};
	snd_back = {
		{ SFX_BACK } ,			// id
		(int)(1.0f * (1<<10)),	// rate
		0,		// handle
		255,	// volume
		128,	// panning
	};
	snd_switch = {
		{ SFX_SWITCH } ,			// id
		(int)(1.0f * (1<<10)),	// rate
		0,		// handle
		255,	// volume
		128,	// panning
	};
}

//---------------------------------------------------------------------------------
void stop (void) {
//---------------------------------------------------------------------------------
	while (1) {
		swiWaitForVBlank();
	}
}

char filePath[PATH_MAX];

//---------------------------------------------------------------------------------
void doPause(int x, int y) {
//---------------------------------------------------------------------------------
	// iprintf("Press start...\n");
	printSmall(false, x, y, "Press start...");
	while(1) {
		scanKeys();
		if(keysDown() & KEY_START)
			break;
	}
	scanKeys();
}

std::string ReplaceAll(std::string str, const std::string& from, const std::string& to) {
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
    }
    return str;
}

void launchSystemSettings() {
	fadeType = false;
	for (int i = 0; i < 25; i++) swiWaitForVBlank();
	renderScreens = false;

	char tmdpath[256];
	u8 titleID[4];
	for (u8 i = 0x41; i <= 0x5A; i++) {
		snprintf (tmdpath, sizeof(tmdpath), "sd:/title/00030015/484e42%x/content/title.tmd", i);
		if (!access(tmdpath, F_OK)) {
			titleID[0] = i;
			titleID[1] = 0x42;
			titleID[2] = 0x4e;
			titleID[3] = 0x48;
			break;
		}
	}

	*(u32*)(0x02000300) = 0x434E4C54;	// Set "CNLT" warmboot flag
	*(u16*)(0x02000304) = 0x1801;
	*(u8*)(0x02000308) = titleID[0];
	*(u8*)(0x02000309) = titleID[1];
	*(u8*)(0x0200030A) = titleID[2];
	*(u8*)(0x0200030B) = titleID[3];
	*(u32*)(0x0200030C) = 0x00030015;
	*(u8*)(0x02000310) = titleID[0];
	*(u8*)(0x02000311) = titleID[1];
	*(u8*)(0x02000312) = titleID[2];
	*(u8*)(0x02000313) = titleID[3];
	*(u32*)(0x02000314) = 0x00030015;
	*(u32*)(0x02000318) = 0x00000017;
	*(u32*)(0x0200031C) = 0x00000000;
	while (*(u16*)(0x02000306) == 0x0000) {	// Keep running, so that CRC16 isn't 0
		*(u16*)(0x02000306) = swiCRC16(0xFFFF, (void*)0x02000308, 0x18);
	}

	fifoSendValue32(FIFO_USER_08, 1);	// Reboot into System Settings
	for (int i = 0; i < 15; i++) swiWaitForVBlank();
}

void rebootDSiMenuPP() {
	fadeType = false;
	for (int i = 0; i < 25; i++) swiWaitForVBlank();
	memcpy((u32*)0x02000300,autoboot_bin,0x020);
	fifoSendValue32(FIFO_USER_08, 1);	// Reboot DSiMenu++ to avoid potential crashing
	for (int i = 0; i < 15; i++) swiWaitForVBlank();
}

void loadROMselect() {
	fadeType = false;
	for (int i = 0; i < 30; i++) swiWaitForVBlank();
	renderScreens = false;
	if(soundfreq) fifoSendValue32(FIFO_USER_07, 2);
	else fifoSendValue32(FIFO_USER_07, 1);
	if (theme==2) {
		runNdsFile ("/_nds/dsimenuplusplus/r4menu.srldr", 0, NULL, false);
	} else {
		runNdsFile ("/_nds/dsimenuplusplus/dsimenu.srldr", 0, NULL, false);
	}
}

int lastRanROM() {
	fadeType = false;
	for (int i = 0; i < 30; i++) swiWaitForVBlank();
	renderScreens = false;
	if(soundfreq) fifoSendValue32(FIFO_USER_07, 2);
	else fifoSendValue32(FIFO_USER_07, 1);

	vector<char*> argarray;
	if (launchType > 2) {
		argarray.push_back(strdup("null"));
		argarray.push_back(strdup(homebrewArg.c_str()));
	}

	int err = 0;
	if (launchType == 0) {
		err = runNdsFile ("/_nds/dsimenuplusplus/slot1launch.srldr", 0, NULL, true);
	} else if (launchType == 1) {
		if (!flashcardUsed) {
			if (homebrewBootstrap) {
				bootstrapfilename = "sd:/_nds/hb-bootstrap.nds";
			} else {
				if (donorSdkVer==5) {
					if (bootstrapFile) bootstrapfilename = "sd:/_nds/nightly-bootstrap-sdk5.nds";
					else bootstrapfilename = "sd:/_nds/release-bootstrap-sdk5.nds";
				} else {
					if (bootstrapFile) bootstrapfilename = "sd:/_nds/nightly-bootstrap.nds";
					else bootstrapfilename = "sd:/_nds/release-bootstrap.nds";
				}
			}
			err = runNdsFile (bootstrapfilename.c_str(), 0, NULL, true);
		} else {
			switch (flashcard) {
				case 0:
				case 1:
				default:
					err = runNdsFile ("fat:/YSMenu.nds", 0, NULL, true);
					break;
				case 2:
				case 4:
				case 5:
					err = runNdsFile ("fat:/Wfwd.dat", 0, NULL, true);
					break;
				case 3:
					err = runNdsFile ("fat:/Afwd.dat", 0, NULL, true);
					break;
				case 6:
					err = runNdsFile ("fat:/_dstwo/autoboot.nds", 0, NULL, true);
					break;
			}
		}
	} else if (launchType == 2) {
		if (!access(dsiWareSrlPath.c_str(), F_OK) && access("sd:/bootthis.dsi", F_OK))
			rename (dsiWareSrlPath.c_str(), "sd:/bootthis.dsi");	// Rename .nds file to "bootthis.dsi" for Unlaunch to boot it
		if (!access(dsiWarePubPath.c_str(), F_OK) && access("sd:/bootthis.pub", F_OK))
			rename (dsiWarePubPath.c_str(), "sd:/bootthis.pub");
		if (!access(dsiWarePrvPath.c_str(), F_OK) && access("sd:/bootthis.prv", F_OK))
			rename (dsiWarePrvPath.c_str(), "sd:/bootthis.prv");

		fifoSendValue32(FIFO_USER_08, 1);	// Reboot
	} else if (launchType == 3) {
		if(flashcardUsed) {
			argarray.at(0) = "/_nds/dsimenuplusplus/emulators/nesds.nds";
			err = runNdsFile ("/_nds/dsimenuplusplus/emulators/nesds.nds", argarray.size(), (const char **)&argarray[0], true);	// Pass ROM to nesDS as argument
		} else {
			argarray.at(0) = "sd:/_nds/dsimenuplusplus/emulators/nestwl.nds";
			err = runNdsFile ("sd:/_nds/dsimenuplusplus/emulators/nestwl.nds", argarray.size(), (const char **)&argarray[0], true);	// Pass ROM to nesDS as argument
		}
	} else if (launchType == 4) {
		if(flashcardUsed) {
			argarray.at(0) = "/_nds/dsimenuplusplus/emulators/gameyob.nds";
			err = runNdsFile ("/_nds/dsimenuplusplus/emulators/gameyob.nds", argarray.size(), (const char **)&argarray[0], true);	// Pass ROM to GameYob as argument
		} else {
			argarray.at(0) = "sd:/_nds/dsimenuplusplus/emulators/gameyob.nds";
			err = runNdsFile ("sd:/_nds/dsimenuplusplus/emulators/gameyob.nds", argarray.size(), (const char **)&argarray[0], true);	// Pass ROM to GameYob as argument
		}
	}
	
	return err;
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

	// Read user name
	char *username = (char*)PersonalData->name;

	// text
	for (int i = 0; i < 10; i++) {
		if (username[i*2] == 0x00)
			username[i*2/2] = 0;
		else
			username[i*2/2] = username[i*2];
	}

	if (!fatInitDefault()) {
		graphicsInit();
		fontInit();
		fadeType = true;
		printf("\n ");
		printf(username);
		printSmall(false, 4, 4, "fatinitDefault failed!");
		stop();
	}

	if (!access("fat:/", F_OK)) flashcardUsed = true;

	bool soundfreqsetting = false;

	std::string filename;
	
	LoadSettings();
	
	swiWaitForVBlank();

	fifoWaitValue32(FIFO_USER_06);
	if (fifoGetValue32(FIFO_USER_03) == 0) arm7SCFGLocked = true;	// If DSiMenu++ is being ran from DSiWarehax or flashcard, then arm7 SCFG is locked.

	u16 arm7_SNDEXCNT = fifoGetValue32(FIFO_USER_07);
	if (arm7_SNDEXCNT != 0) soundfreqsetting = true;
	fifoSendValue32(FIFO_USER_07, 0);

	scanKeys();

	if (arm7SCFGLocked && !gotosettings && autorun && !(keysHeld() & KEY_B)) {
		lastRanROM();
	}
	
	if (launchType == 2) {
		if (!access("sd:/bootthis.dsi", F_OK) && access(dsiWareSrlPath.c_str(), F_OK))
			rename ("sd:/bootthis.dsi", dsiWareSrlPath.c_str());	// Rename "bootthis.dsi" back to original .nds filename
		if (!access("sd:/bootthis.pub", F_OK) && access(dsiWarePubPath.c_str(), F_OK))
			rename ("sd:/bootthis.pub", dsiWarePubPath.c_str());
		if (!access("sd:/bootthis.prv", F_OK) && access(dsiWarePrvPath.c_str(), F_OK))
			rename ("sd:/bootthis.prv", dsiWarePrvPath.c_str());
	}

	InitSound();
	
	char vertext[12];
	// snprintf(vertext, sizeof(vertext), "Ver %d.%d.%d   ", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH); // Doesn't work :(
	snprintf(vertext, sizeof(vertext), "Ver %d.%d.%d   ", 5, 2, 0);

	if (gotosettings) {
		graphicsInit();
		fontInit();
		screenmode = 1;
		gotosettings = false;
		SaveSettings();
		langInit();
		fadeType = true;
	} else if (autorun || showlogo) {
		loadTitleGraphics();
		fadeType = true;

		for (int i = 0; i < 60*3; i++) {
			swiWaitForVBlank();
		}
		
		scanKeys();

		if (keysHeld() & KEY_START) {
			fadeType = false;
			for (int i = 0; i < 30; i++) {
				swiWaitForVBlank();
			}
			graphicsInit();
			fontInit();
			screenmode = 1;
			langInit();
			fadeType = true;
		}
	} else {
		scanKeys();

		if (keysHeld() & KEY_START) {
			graphicsInit();
			fontInit();
			fadeType = true;
			screenmode = 1;
			langInit();
			for (int i = 0; i < 60; i++) {
				swiWaitForVBlank();
			}
		}
	}


	srand(time(NULL));

	bool menuprinted = false;

	bool hiyaAutobootFound = false;

	int pressed = 0;

	while(1) {
	
		if (screenmode == 1) {

			if (subscreenmode == 3) {
				pressed = 0;

				if (!menuprinted) {
					// Clear the screen so it doesn't over-print
					clearText();

					printSmall(true, 28, 1, username);
					printSmall(true, 194, 174, vertext);

					printLarge(false, 6, 2, STR_FLASHCARD_SELECT.c_str());

					int yPos = 32;
					switch (flashcard) {
						case 0:
						default:
							printSmall(false, 12, 30+(0*14), "DSTT");
							printSmall(false, 12, 30+(1*14), "R4i Gold");
							printSmall(false, 12, 30+(2*14), "R4i-SDHC (Non-v1.4.x version) (www.r4i-sdhc.com)");
							printSmall(false, 12, 30+(3*14), "R4 SDHC Dual-Core");
							printSmall(false, 12, 30+(4*14), "R4 SDHC Upgrade");
							printSmall(false, 12, 30+(5*14), "SuperCard DSONE");
							break;
						case 1:
							printSmall(false, 12, 30+(0*14), "Original R4");
							printSmall(false, 12, 30+(1*14), "M3 Simply");
							break;
						case 2:
							printSmall(false, 12, 30+(0*14), "R4iDSN");
							printSmall(false, 12, 30+(1*14), "R4i Gold RTS");
							printSmall(false, 12, 30+(2*14), "R4 Ultra");
							break;
						case 3:
							printSmall(false, 12, 30+(0*14), "Acekard 2(i)");
							printSmall(false, 12, 30+(1*14), "Galaxy Eagle");
							printSmall(false, 12, 30+(2*14), "M3DS Real");
							break;
						case 4:
							printSmall(false, 12, 30, "Acekard RPG");
							break;
						case 5:
							printSmall(false, 12, 30+(0*14), "Ace 3DS+");
							printSmall(false, 12, 30+(1*14), "Gateway Blue Card");
							printSmall(false, 12, 30+(2*14), "R4iTT");
							break;
						case 6:
							printSmall(false, 12, 30, "SuperCard DSTWO");
							break;
					}

					printLargeCentered(true, 118, STR_LEFTRIGHT_FLASHCARD.c_str());
					printLargeCentered(true, 132, STR_AB_SETRETURN.c_str());

					menuprinted = true;
				}

				// Power saving loop. Only poll the keys once per frame and sleep the CPU if there is nothing else to do
				do {
					scanKeys();
					pressed = keysDownRepeat();
					swiWaitForVBlank();
				} while (!pressed);
				
				if (pressed & KEY_LEFT) {
					flashcard -= 1;
					mmEffectEx(&snd_select);
					menuprinted = false;
				}
				if (pressed & KEY_RIGHT) {
					flashcard += 1;
					mmEffectEx(&snd_select);
					menuprinted = false;
				}

				if ((pressed & KEY_A) || (pressed & KEY_B)) {
					subscreenmode = 1;
					mmEffectEx(&snd_select);
					menuprinted = false;
				}

				if (flashcard > 6) flashcard = 0;
				else if (flashcard < 0) flashcard = 6;
			} else if (subscreenmode == 2) {
				pressed = 0;

				if (!menuprinted) {
					// Clear the screen so it doesn't over-print
					clearText();

					printSmall(true, 28, 1, username);
					printSmall(true, 194, 174, vertext);

					switch (theme) {
						case 0:
						default:
							printLarge(false, 6, 2, STR_SUBTHEMESEL_DSI.c_str());
							break;
						case 1:
							printLarge(false, 6, 2, STR_SUBTHEMESEL_3DS.c_str());
							break;
						case 2:
							printLarge(false, 6, 2, STR_SUBTHEMESEL_R4.c_str());
							break;
					}

					int yPos = 30;
					if (theme == 2) yPos = 22;
					for (int i = 0; i < subtheme; i++) {
						yPos += 12;
					}
					if (subtheme == 12) yPos = 30;

					if (subtheme == 12) printSmall(false, 126, yPos, ">");
					else printSmall(false, 4, yPos, ">");

					int selyPos = 30;
					if (theme == 2) selyPos = 22;

					switch (theme) {
						case 0:
						default:
							printSmall(false, 12, 30, STR_DSI_DARKMENU.c_str());
							selyPos += 12;
							printSmall(false, 12, 44, STR_DSI_NORMALMENU.c_str());
							break;
						case 1:
							break;
						case 2:
							printSmall(false, 12, selyPos, STR_R4_THEME01.c_str());
							selyPos += 12;
							printSmall(false, 12, selyPos, STR_R4_THEME02.c_str());
							selyPos += 12;
							printSmall(false, 12, selyPos, STR_R4_THEME03.c_str());
							selyPos += 12;
							printSmall(false, 12, selyPos, STR_R4_THEME04.c_str());
							selyPos += 12;
							printSmall(false, 12, selyPos, STR_R4_THEME05.c_str());
							selyPos += 12;
							printSmall(false, 12, selyPos, STR_R4_THEME06.c_str());
							selyPos += 12;
							printSmall(false, 12, selyPos, STR_R4_THEME07.c_str());
							selyPos += 12;
							printSmall(false, 12, selyPos, STR_R4_THEME08.c_str());
							selyPos += 12;
							printSmall(false, 12, selyPos, STR_R4_THEME09.c_str());
							selyPos += 12;
							printSmall(false, 12, selyPos, STR_R4_THEME10.c_str());
							selyPos += 12;
							printSmall(false, 12, selyPos, STR_R4_THEME11.c_str());
							selyPos += 12;
							printSmall(false, 12, selyPos, STR_R4_THEME12.c_str());
							selyPos = 30;
							printSmall(false, 134, selyPos, STR_R4_THEME13.c_str());
							break;
					}

					printLargeCentered(true, 128, STR_AB_SETSUBTHEME.c_str());

					menuprinted = true;
				}

				// Power saving loop. Only poll the keys once per frame and sleep the CPU if there is nothing else to do
				do {
					scanKeys();
					pressed = keysDownRepeat();
					swiWaitForVBlank();
				} while (!pressed);
				
				if (pressed & KEY_UP) {
					subtheme -= 1;
					mmEffectEx(&snd_select);
					menuprinted = false;
				}
				if (pressed & KEY_DOWN) {
					subtheme += 1;
					mmEffectEx(&snd_select);
					menuprinted = false;
				}

				if ((pressed & KEY_A) || (pressed & KEY_B)) {
					subscreenmode = 0;
					mmEffectEx(&snd_select);
					menuprinted = false;
				}

				if (theme == 2) {
					if (subtheme > 12) subtheme = 0;
					else if (subtheme < 0) subtheme = 12;
				} else {
					if (subtheme > 1) subtheme = 0;
					else if (subtheme < 0) subtheme = 1;
				}
			} else if (subscreenmode == 1) {
				pressed = 0;

				if (!menuprinted) {
					// Clear the screen so it doesn't over-print
					clearText();

					printSmallCentered(false, 173, "DSiMenu++");

					printSmall(true, 4, 174, STR_LR_SWITCH.c_str());
					printSmall(true, 28, 1, username);
					printSmall(true, 194, 174, vertext);

					printLarge(false, 6, 2, STR_GAMESAPPS_SETTINGS.c_str());

					int yPos = 30;
					for (int i = 0; i < settingscursor; i++) {
						yPos += 12;
					}

					int selyPos = 30;

					printSmall(false, 4, yPos, ">");

					if(!flashcardUsed) {
						printSmall(false, 12, selyPos, STR_LANGUAGE.c_str());
						switch(bstrap_language) {
							case -1:
							default:
								printSmall(false, 203, selyPos, STR_SYSTEM.c_str());
								break;
							case 0:
								printSmall(false, 194, selyPos, "Japanese");
								break;
							case 1:
								printSmall(false, 206, selyPos, "English");
								break;
							case 2:
								printSmall(false, 207, selyPos, "French");
								break;
							case 3:
								printSmall(false, 200, selyPos, "German");
								break;
							case 4:
								printSmall(false, 210, selyPos, "Italian");
								break;
							case 5:
								printSmall(false, 203, selyPos, "Spanish");
								break;
						}
						selyPos += 12;

						printSmall(false, 12, selyPos, STR_CPUSPEED.c_str());
						if(boostCpu)
							printSmall(false, 158, selyPos, "133mhz (TWL)");
						else
							printSmall(false, 170, selyPos, "67mhz (NTR)");
						selyPos += 12;

						printSmall(false, 12, selyPos, STR_DEBUG.c_str());
						if(bstrap_debug)
							printSmall(false, 224, selyPos, STR_ON.c_str());
						else
							printSmall(false, 224, selyPos, STR_OFF.c_str());
						selyPos += 12;
						if (consoleModel < 2) {
							printSmall(false, 12, selyPos, STR_ROMREADLED.c_str());
							switch(bstrap_romreadled) {
								case 0:
								default:
									printSmall(false, 216, selyPos, STR_NONE.c_str());
									break;
								case 1:
									printSmall(false, 216, selyPos, "WiFi");
									break;
								case 2:
									printSmall(false, 216, selyPos, STR_POWER.c_str());
									break;
								case 3:
									printSmall(false, 216, selyPos, STR_CAMERA.c_str());
									break;
							}
						}
						selyPos += 12;

						printSmall(false, 12, selyPos, STR_ASYNCPREFETCH.c_str());
						if(soundfreq)
							printSmall(false, 224, selyPos, STR_ON.c_str());
						else
							printSmall(false, 224, selyPos, STR_OFF.c_str());
						selyPos += 12;

						printSmall(false, 12, selyPos, STR_SNDFREQ.c_str());
						if(soundfreq)
							printSmall(false, 187, selyPos, "47.61 kHz");
						else
							printSmall(false, 186, selyPos, "32.73 kHz");
						selyPos += 12;

						if (!arm7SCFGLocked) {
							printSmall(false, 12, selyPos, STR_SLOT1LAUNCHMETHOD.c_str());
							if(slot1LaunchMethod)
								printSmall(false, 210, selyPos, STR_DIRECT.c_str());
							else
								printSmall(false, 202, selyPos, STR_REBOOT.c_str());
						}
						selyPos += 12;

						printSmall(false, 12, selyPos, STR_LOADINGSCREEN.c_str());
						switch(bstrap_loadingScreen) {
							case 0:
							default:
								printSmall(false, 216, selyPos, STR_NONE.c_str());
								break;
							case 1:
								printSmall(false, 200, selyPos, STR_REGULAR.c_str());
								break;
							case 2:
								printSmall(false, 216, selyPos, "Pong");
								break;
							case 3:
								printSmall(false, 172, selyPos, "Tic-Tac-Toe");
								break;
						}
						selyPos += 12;

						printSmall(false, 12, selyPos, STR_BOOTSTRAP.c_str());
						if(bootstrapFile)
							printSmall(false, 202, selyPos, STR_NIGHTLY.c_str());
						else
							printSmall(false, 200, selyPos, STR_RELEASE.c_str());


						if (settingscursor == 0) {
							printLargeCentered(true, 112, STR_DESCRIPTION_LANGUAGE_1.c_str());
							printLargeCentered(true, 126, STR_DESCRIPTION_LANGUAGE_2.c_str());
							printLargeCentered(true, 140, STR_DESCRIPTION_LANGUAGE_3.c_str());
						} else if (settingscursor == 1) {
							printLargeCentered(true, 118, STR_DESCRIPTION_CPUSPEED_1.c_str());
							printLargeCentered(true, 132, STR_DESCRIPTION_CPUSPEED_2.c_str());
						} /* else if (settingscursor == 4) {
							printLargeCentered(true, 120, "Allows 8 bit VRAM writes");
							printLargeCentered(true, 134, "and expands the bus to 32 bit.");
						} */ else if (settingscursor == 2) {
							printLargeCentered(true, 118, STR_DESCRIPTION_DEBUG_1.c_str());
							printLargeCentered(true, 132, STR_DESCRIPTION_DEBUG_2.c_str());
						} else if (settingscursor == 3) {
							// printLargeCentered(true, 114, "Locks the ARM9 SCFG_EXT,");
							// printLargeCentered(true, 128, "avoiding conflict with");
							// printLargeCentered(true, 142, "recent libnds.");
							printLargeCentered(true, 126, STR_DESCRIPTION_ROMREADLED_1.c_str());
						} else if (settingscursor == 4) {
							printLargeCentered(true, 112, STR_DESCRIPTION_ASYNCPREFETCH_1.c_str());
							printLargeCentered(true, 126, STR_DESCRIPTION_ASYNCPREFETCH_2.c_str());
							printLargeCentered(true, 140, STR_DESCRIPTION_ASYNCPREFETCH_3.c_str());
						} else if (settingscursor == 5) {
							printLargeCentered(true, 118, STR_DESCRIPTION_SNDFREQ_1.c_str());
							printLargeCentered(true, 132, STR_DESCRIPTION_SNDFREQ_2.c_str());
						} else if (settingscursor == 6) {
							printLargeCentered(true, 104, STR_DESCRIPTION_SLOT1LAUNCHMETHOD_1.c_str());
							printLargeCentered(true, 118, STR_DESCRIPTION_SLOT1LAUNCHMETHOD_2.c_str());
							printLargeCentered(true, 132, STR_DESCRIPTION_SLOT1LAUNCHMETHOD_3.c_str());
							printLargeCentered(true, 146, STR_DESCRIPTION_SLOT1LAUNCHMETHOD_4.c_str());
						} else if (settingscursor == 7) {
							printLargeCentered(true, 118, STR_DESCRIPTION_LOADINGSCREEN_1.c_str());
							printLargeCentered(true, 132, STR_DESCRIPTION_LOADINGSCREEN_2.c_str());
						} else if (settingscursor == 8) {
							printLargeCentered(true, 118, STR_DESCRIPTION_BOOTSTRAP_1.c_str());
							printLargeCentered(true, 132, STR_DESCRIPTION_BOOTSTRAP_2.c_str());
						}
					} else {
						printSmall(false, 12, selyPos, STR_FLASHCARD_SELECT.c_str());
						selyPos += 12;
						if(soundfreqsetting) {
							printSmall(false, 12, selyPos, STR_SNDFREQ.c_str());
							if(soundfreq)
								printSmall(false, 184, selyPos, "47.61 kHz");
							else
								printSmall(false, 184, selyPos, "32.73 kHz");
						} else {
							printSmall(false, 12, selyPos, STR_USEGBARUNNER2.c_str());
							if(useGbarunner)
								printSmall(false, 224, selyPos, STR_YES.c_str());
							else
								printSmall(false, 224, selyPos, STR_NO.c_str());
						}

						if (settingscursor == 0) {
							printLargeCentered(true, 118, STR_DESCRIPTION_FLASHCARD_1.c_str());
							printLargeCentered(true, 132, STR_DESCRIPTION_FLASHCARD_2.c_str());
						} else if (settingscursor == 1) {
							if(soundfreqsetting) {
								printLargeCentered(true, 118, STR_DESCRIPTION_SNDFREQ_1.c_str());
								printLargeCentered(true, 132, STR_DESCRIPTION_SNDFREQ_2.c_str());
							} else {
								printLargeCentered(true, 118, STR_DESCRIPTION_GBARUNNER2_1.c_str());
								printLargeCentered(true, 132, STR_DESCRIPTION_GBARUNNER2_2.c_str());
							}
						}
					}



					menuprinted = true;
				}

				// Power saving loop. Only poll the keys once per frame and sleep the CPU if there is nothing else to do
				do {
					scanKeys();
					pressed = keysDownRepeat();
					swiWaitForVBlank();
				} while (!pressed);
				
				if (pressed & KEY_UP) {
					settingscursor--;
					if (consoleModel > 1 && settingscursor == 3) settingscursor--;
					if (arm7SCFGLocked && settingscursor == 6) settingscursor--;
					mmEffectEx(&snd_select);
					menuprinted = false;
				}
				if (pressed & KEY_DOWN) {
					settingscursor++;
					if (consoleModel > 1 && settingscursor == 3) settingscursor++;
					if (arm7SCFGLocked && settingscursor == 6) settingscursor++;
					mmEffectEx(&snd_select);
					menuprinted = false;
				}
					
				if ((pressed & KEY_A) || (pressed & KEY_LEFT) || (pressed & KEY_RIGHT)) {
					if(!flashcardUsed) {
						switch (settingscursor) {
							case 0:
							default:
								if (pressed & KEY_LEFT) {
									bstrap_language--;
									if (bstrap_language < -1) bstrap_language = 5;
								} else if ((pressed & KEY_RIGHT) || (pressed & KEY_A)) {
									bstrap_language++;
									if (bstrap_language > 5) bstrap_language = -1;
								}
								break;
							case 1:
								boostCpu = !boostCpu;
								break;
							case 2:
								bstrap_debug = !bstrap_debug;
								break;
							case 3:
								// bstrap_lockARM9scfgext = !bstrap_lockARM9scfgext;
								if (pressed & KEY_LEFT) {
									bstrap_romreadled--;
									if (bstrap_romreadled < 0) bstrap_romreadled = 2;
								} else if ((pressed & KEY_RIGHT) || (pressed & KEY_A)) {
									bstrap_romreadled++;
									if (bstrap_romreadled > 2) bstrap_romreadled = 0;
								}
								break;
							case 4:
								bstrap_asyncPrefetch = !bstrap_asyncPrefetch;
								break;
							case 5:
								soundfreq = !soundfreq;
								break;
							case 6:
								slot1LaunchMethod = !slot1LaunchMethod;
								break;
							case 7:
								if (pressed & KEY_LEFT) {
									bstrap_loadingScreen--;
									if (bstrap_loadingScreen < 0) bstrap_loadingScreen = 3;
								} else if ((pressed & KEY_RIGHT) || (pressed & KEY_A)) {
									bstrap_loadingScreen++;
									if (bstrap_loadingScreen > 3) bstrap_loadingScreen = 0;
								}
								break;
							case 8:
								bootstrapFile = !bootstrapFile;
								break;
						}
					} else {
						switch (settingscursor) {
							case 0:
							default:
								subscreenmode = 3;
								break;
							case 1:
								if(soundfreqsetting) soundfreq = !soundfreq;
								else useGbarunner = !useGbarunner;
								break;
						}
					}
					mmEffectEx(&snd_select);
					menuprinted = false;
				}
				
				if ((pressed & KEY_L) || (pressed & KEY_R)
				|| (pressed & KEY_Y) || (pressed & KEY_X)) {
					subscreenmode = 0;
					settingscursor = 0;
					mmEffectEx(&snd_switch);
					menuprinted = false;
				}

				if (pressed & KEY_B) {
					mmEffectEx(&snd_back);
					clearText();
					printSmall(false, 4, 2, STR_SAVING_SETTINGS.c_str());
					SaveSettings();
					clearText();
					printSmall(false, 4, 2, STR_SETTINGS_SAVED.c_str());
					for (int i = 0; i < 60; i++) swiWaitForVBlank();
					if (!arm7SCFGLocked) {
						rebootDSiMenuPP();
					}
					loadROMselect();
					break;
				}

				if(!flashcardUsed) {
					if (settingscursor > 8) settingscursor = 0;
					else if (settingscursor < 0) settingscursor = 8;
				} else {
					if (settingscursor > 1) settingscursor = 0;
					else if (settingscursor < 0) settingscursor = 1;
				}
			} else {
				pressed = 0;

				if (!flashcardUsed && consoleModel < 2) {
					if (!access("sd:/hiya/autoboot.bin", F_OK)) hiyaAutobootFound = true;
					else hiyaAutobootFound = false;
				}

				if (!menuprinted) {
					// Clear the screen so it doesn't over-print
					clearText();

					printSmallCentered(false, 173, "DSiMenu++");

					printSmall(true, 4, 174, STR_LR_SWITCH.c_str());
					printSmall(true, 28, 1, username);
					printSmall(true, 194, 174, vertext);

					printLarge(false, 6, 2, STR_GUI_SETTINGS.c_str());
					
					int yPos = 30;
					for (int i = 0; i < settingscursor; i++) {
						yPos += 12;
					}

					int selyPos = 30;

					printSmall(false, 4, yPos, ">");

					printSmall(false, 12, selyPos, STR_LANGUAGE.c_str());
					switch(guiLanguage) {
						case -1:
						default:
							printSmall(false, 203, selyPos, STR_SYSTEM.c_str());
							break;
						case 0:
							printSmall(false, 194, selyPos, "Japanese");
							break;
						case 1:
							printSmall(false, 206, selyPos, "English");
							break;
						case 2:
							printSmall(false, 207, selyPos, "French");
							break;
						case 3:
							printSmall(false, 200, selyPos, "German");
							break;
						case 4:
							printSmall(false, 210, selyPos, "Italian");
							break;
						case 5:
							printSmall(false, 203, selyPos, "Spanish");
							break;
					}
					selyPos += 12;

					printSmall(false, 12, selyPos, STR_THEME.c_str());
					switch (theme) {
						case 0:
						default:
							printSmall(false, 224, selyPos, "DSi");
							break;
						case 1:
							printSmall(false, 224, selyPos, "3DS");
							break;
						case 2:
							printSmall(false, 224, selyPos, "R4");
							break;
					}
					selyPos += 12;

					printSmall(false, 12, selyPos, STR_LASTPLAYEDROM.c_str());
					if(autorun)
						printSmall(false, 224, selyPos, STR_YES.c_str());
					else
						printSmall(false, 230, selyPos, STR_NO.c_str());
					selyPos += 12;

					printSmall(false, 12, selyPos, STR_DSIMENUPPLOGO.c_str());
					if(showlogo)
						printSmall(false, 216, selyPos, STR_SHOW.c_str());
					else
						printSmall(false, 222, selyPos, STR_HIDE.c_str());
					selyPos += 12;

					printSmall(false, 12, selyPos, STR_DIRECTORIES.c_str());
					if(showDirectories)
						printSmall(false, 216, selyPos, STR_SHOW.c_str());
					else
						printSmall(false, 222, selyPos, STR_HIDE.c_str());
					selyPos += 12;

					printSmall(false, 12, selyPos, STR_BOXART.c_str());
					if(showBoxArt)
						printSmall(false, 216, selyPos, STR_SHOW.c_str());
					else
						printSmall(false, 222, selyPos, STR_HIDE.c_str());
					selyPos += 12;

					printSmall(false, 12, selyPos, STR_ANIMATEDSIICONS.c_str());
					if(animateDsiIcons)
						printSmall(false, 224, selyPos, STR_YES.c_str());
					else
						printSmall(false, 230, selyPos, STR_NO.c_str());
					selyPos += 12;

					if (!flashcardUsed && !arm7SCFGLocked) {
						if (consoleModel < 2) {
							printSmall(false, 12, selyPos, STR_SYSTEMSETTINGS.c_str());
							selyPos += 12;
							if (hiyaAutobootFound) {
								printSmall(false, 12, selyPos, STR_RESTOREDSIMENU.c_str());
							} else {
								printSmall(false, 12, selyPos, STR_REPLACEDSIMENU.c_str());
							}
						}
					}


					if (settingscursor == 0) {
						printLargeCentered(true, 112, STR_DESCRIPTION_LANGUAGE_1.c_str());
						printLargeCentered(true, 126, STR_DESCRIPTION_LANGUAGE_2.c_str());
						printLargeCentered(true, 140, STR_DESCRIPTION_LANGUAGE_3.c_str());
					} else if (settingscursor == 1) {
						printLargeCentered(true, 118, STR_DESCRIPTION_THEME_1.c_str());
						printLargeCentered(true, 132, STR_DESCRIPTION_THEME_2.c_str());
					} else if (settingscursor == 2) {
						printLargeCentered(true, 104, STR_DESCRIPTION_LASTPLAYEDROM_1.c_str());
						printLargeCentered(true, 118, STR_DESCRIPTION_LASTPLAYEDROM_2.c_str());
						printLargeCentered(true, 132, STR_DESCRIPTION_LASTPLAYEDROM_3.c_str());
						printLargeCentered(true, 146, STR_DESCRIPTION_LASTPLAYEDROM_4.c_str());
					} else if (settingscursor == 3) {
						printLargeCentered(true, 112, STR_DESCRIPTION_DSIMENUPPLOGO_1.c_str());
						printLargeCentered(true, 126, STR_DESCRIPTION_DSIMENUPPLOGO_2.c_str());
						printLargeCentered(true, 140, STR_DESCRIPTION_DSIMENUPPLOGO_3.c_str());
					} else if (settingscursor == 4) {
						printLargeCentered(true, 112, STR_DESCRIPTION_DIRECTORIES_1.c_str());
						printLargeCentered(true, 126, STR_DESCRIPTION_DIRECTORIES_2.c_str());
						printLargeCentered(true, 140, STR_DESCRIPTION_DIRECTORIES_3.c_str());
					} else if (settingscursor == 5) {
						printLargeCentered(true, 118, STR_DESCRIPTION_BOXART_1.c_str());
						printLargeCentered(true, 132, STR_DESCRIPTION_BOXART_2.c_str());
					} else if (settingscursor == 6) {
						printLargeCentered(true, 112, STR_DESCRIPTION_ANIMATEDSIICONS_1.c_str());
						printLargeCentered(true, 126, STR_DESCRIPTION_ANIMATEDSIICONS_2.c_str());
						printLargeCentered(true, 140, STR_DESCRIPTION_ANIMATEDSIICONS_3.c_str());
					} else if (settingscursor == 7) {
						printLargeCentered(true, 118, STR_DESCRIPTION_SYSTEMSETTINGS_1.c_str());
						printLargeCentered(true, 132, STR_DESCRIPTION_SYSTEMSETTINGS_2.c_str());
					} else if (settingscursor == 8) {
						if (hiyaAutobootFound) {
							printLargeCentered(true, 126, STR_DESCRIPTION_RESTOREDSIMENU_1.c_str());
						} else {
							printLargeCentered(true, 118, STR_DESCRIPTION_REPLACEDSIMENU_1.c_str());
							printLargeCentered(true, 132, STR_DESCRIPTION_REPLACEDSIMENU_2.c_str());
						}
					}


					menuprinted = true;
				}

				// Power saving loop. Only poll the keys once per frame and sleep the CPU if there is nothing else to do
				do {
					scanKeys();
					pressed = keysDownRepeat();
					swiWaitForVBlank();
				} while (!pressed);

				if (pressed & KEY_UP) {
					settingscursor -= 1;
					mmEffectEx(&snd_select);
					menuprinted = false;
				}
				if (pressed & KEY_DOWN) {
					settingscursor += 1;
					mmEffectEx(&snd_select);
					menuprinted = false;
				}

				if ((pressed & KEY_A) || (pressed & KEY_LEFT) || (pressed & KEY_RIGHT)) {
					switch (settingscursor) {
						case 0:
						default:
							if (pressed & KEY_LEFT) {
								guiLanguage--;
								if (guiLanguage < -1) guiLanguage = 5;
							} else if ((pressed & KEY_RIGHT) || (pressed & KEY_A)) {
								guiLanguage++;
								if (guiLanguage > 5) guiLanguage = -1;
							}
							break;
						case 1:
							if (pressed & KEY_LEFT) {
								subtheme = 0;
								theme -= 1;
								if (theme < 0) theme = 2;
								mmEffectEx(&snd_select);
							} else if (pressed & KEY_RIGHT) {
								subtheme = 0;
								theme += 1;
								if (theme > 2) theme = 0;
								mmEffectEx(&snd_select);
							} else if (theme == 1) {
								mmEffectEx(&snd_wrong);
							} else {
								subscreenmode = 2;
								mmEffectEx(&snd_select);
							}
							break;
						case 2:
							autorun = !autorun;
							mmEffectEx(&snd_select);
							break;
						case 3:
							showlogo = !showlogo;
							mmEffectEx(&snd_select);
							break;
						case 4:
							showDirectories = !showDirectories;
							mmEffectEx(&snd_select);
							break;
						case 5:
							showBoxArt = !showBoxArt;
							mmEffectEx(&snd_select);
							break;
						case 6:
							animateDsiIcons = !animateDsiIcons;
							mmEffectEx(&snd_select);
							break;
						case 7:
							screenmode = 0;
							mmEffectEx(&snd_launch);
							clearText();
							printSmall(false, 4, 2, STR_SAVING_SETTINGS.c_str());
							SaveSettings();
							clearText();
							printSmall(false, 4, 2, STR_SETTINGS_SAVED.c_str());
							for (int i = 0; i < 60; i++) swiWaitForVBlank();
							launchSystemSettings();
							break;
						case 8:
							if (pressed & KEY_A) {
								if (hiyaAutobootFound) {
									if ( remove ("sd:/hiya/autoboot.bin") != 0 ) {
									} else {
										hiyaAutobootFound = false;
									}
								} else {
									FILE* ResetData = fopen("sd:/hiya/autoboot.bin","wb");
									fwrite(autoboot_bin,1,autoboot_bin_len,ResetData);
									fclose(ResetData);
									hiyaAutobootFound = true;

									CIniFile hiyacfwini( hiyacfwinipath );
									hiyacfwini.SetInt("HIYA-CFW", "TITLE_AUTOBOOT", 1);
									hiyacfwini.SaveIniFile(hiyacfwinipath);
								}
							}
							break;
					}
					menuprinted = false;
				}

				if ((pressed & KEY_L) || (pressed & KEY_R)
				|| (pressed & KEY_Y) || (pressed & KEY_X)) {
					subscreenmode = 1;
					settingscursor = 0;
					mmEffectEx(&snd_switch);
					menuprinted = false;
				}

				if (pressed & KEY_Y && settingscursor == 2) {
					screenmode = 0;
					mmEffectEx(&snd_launch);
					clearText();
					printSmall(false, 4, 2, STR_SAVING_SETTINGS.c_str());
					SaveSettings();
					clearText();
					printSmall(false, 4, 2, STR_SETTINGS_SAVED.c_str());
					for (int i = 0; i < 60; i++) swiWaitForVBlank();
					int err = lastRanROM();
					iprintf ("Start failed. Error %i\n", err);
				}

				if (pressed & KEY_B) {
					mmEffectEx(&snd_back);
					clearText();
					printSmall(false, 4, 2, STR_SAVING_SETTINGS.c_str());
					SaveSettings();
					clearText();
					printSmall(false, 4, 2, STR_SETTINGS_SAVED.c_str());
					for (int i = 0; i < 60; i++) swiWaitForVBlank();
					if (!arm7SCFGLocked) {
						rebootDSiMenuPP();
					}
					loadROMselect();
					break;
				}

				if (!flashcardUsed && consoleModel < 2) {
					if (settingscursor > 8) settingscursor = 0;
					else if (settingscursor < 0) settingscursor = 8;
				} else {
					if (settingscursor > 6) settingscursor = 0;
					else if (settingscursor < 0) settingscursor = 6;
				}
			}

		} else {
			loadROMselect();
		}

	}

	return 0;
}
