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

#include "soundbank.h"
#include "soundbank_bin.h"

bool renderScreens = true;
bool fadeType = false;		// false = out, true = in

const char* settingsinipath = "/_nds/dsimenuplusplus/settings.ini";
const char* hiyacfwinipath = "sd:/hiya/settings.ini";
const char* bootstrapinipath = "sd:/_nds/nds-bootstrap.ini";

std::string bootstrapfilename;

static int consoleModel = 0;
/*	0 = Nintendo DSi (Retail)
	1 = Nintendo DSi (Dev/Panda)
	2 = Nintendo 3DS
	3 = New Nintendo 3DS	*/

static bool showlogo = true;
static bool gotosettings = false;

const char* romreadled_valuetext;
const char* loadingScreen_valuetext;

static int bstrap_loadingScreen = 1;

static int donorSdkVer = 0;

static bool bootstrapFile = false;
static bool homebrewBootstrap = false;
static bool quickStartRom = false;

static bool useGbarunner = false;
static bool autorun = false;
static int theme = 0;
static int subtheme = 0;
static bool showDirectories = true;
static bool animateDsiIcons = false;

static bool boostCpu = false;	// false == NTR, true == TWL
static bool bstrap_debug = false;
static int bstrap_romreadled = 0;
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

	useGbarunner = settingsini.GetInt("SRLOADER", "USE_GBARUNNER2", 0);
	autorun = settingsini.GetInt("SRLOADER", "AUTORUNGAME", 0);
	showlogo = settingsini.GetInt("SRLOADER", "SHOWLOGO", 1);
	gotosettings = settingsini.GetInt("SRLOADER", "GOTOSETTINGS", 0);
	soundfreq = settingsini.GetInt("SRLOADER", "SOUND_FREQ", 0);
	flashcard = settingsini.GetInt("SRLOADER", "FLASHCARD", 0);
	bootstrapFile = settingsini.GetInt("SRLOADER", "BOOTSTRAP_FILE", 0);
	homebrewBootstrap = settingsini.GetInt("SRLOADER", "HOMEBREW_BOOTSTRAP", 0);
	quickStartRom = settingsini.GetInt("SRLOADER", "QUICK_START_GAME", 0);
	consoleModel = settingsini.GetInt("SRLOADER", "CONSOLE_MODEL", 0);

	// Customizable UI settings.
	theme = settingsini.GetInt("SRLOADER", "THEME", 0);
	subtheme = settingsini.GetInt("SRLOADER", "SUB_THEME", 0);
	showDirectories = settingsini.GetInt("SRLOADER", "SHOW_DIRECTORIES", 1);
	animateDsiIcons = settingsini.GetInt("SRLOADER", "ANIMATE_DSI_ICONS", 0);

	// Default nds-bootstrap settings
	boostCpu = settingsini.GetInt("SRLOADER", "BOOST_CPU", 0);

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

	settingsini.SetInt("SRLOADER", "USE_GBARUNNER2", useGbarunner);
	settingsini.SetInt("SRLOADER", "AUTORUNGAME", autorun);
	settingsini.SetInt("SRLOADER", "SHOWLOGO", showlogo);
	settingsini.SetInt("SRLOADER", "GOTOSETTINGS", gotosettings);
	settingsini.SetInt("SRLOADER", "SOUND_FREQ", soundfreq);
	settingsini.SetInt("SRLOADER", "FLASHCARD", flashcard);
	settingsini.SetInt("SRLOADER", "BOOTSTRAP_FILE", bootstrapFile);
	settingsini.SetInt("SRLOADER", "QUICK_START_GAME", quickStartRom);

	// UI settings.
	settingsini.SetInt("SRLOADER", "THEME", theme);
	settingsini.SetInt("SRLOADER", "SUB_THEME", subtheme);
	settingsini.SetInt("SRLOADER", "SHOW_DIRECTORIES", showDirectories);
	settingsini.SetInt("SRLOADER", "ANIMATE_DSI_ICONS", animateDsiIcons);

	// Default nds-bootstrap settings
	settingsini.SetInt("SRLOADER", "BOOST_CPU", boostCpu);
	settingsini.SaveIniFile(settingsinipath);

	if(!flashcardUsed) {
		// nds-bootstrap
		CIniFile bootstrapini( bootstrapinipath );

		bootstrapini.SetInt("NDS-BOOTSTRAP", "DEBUG", bstrap_debug);
		bootstrapini.SetInt("NDS-BOOTSTRAP", "NTR_TOUCH", quickStartRom);
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
	int err = 0;
	if (!flashcardUsed) {
		if (!arm7SCFGLocked && !quickStartRom) {
			*(u32*)(0x02000300) = 0x434E4C54;	// Set "CNLT" warmboot flag
			*(u16*)(0x02000304) = 0x1801;
			*(u32*)(0x02000308) = 0x534C524E;	// "SLRN"
			*(u32*)(0x0200030C) = 0x00030015;
			*(u32*)(0x02000310) = 0x534C524E;	// "SLRN"
			*(u32*)(0x02000314) = 0x00030015;
			*(u32*)(0x02000318) = 0x00000017;
			*(u32*)(0x0200031C) = 0x00000000;
			while (*(u16*)(0x02000306) == 0x0000) {	// Keep running, so that CRC16 isn't 0
				*(u16*)(0x02000306) = swiCRC16(0xFFFF, (void*)0x02000308, 0x18);
			}

			fifoSendValue32(FIFO_USER_02, 1);	// Reboot into bootstrap with NTR touch/WiFi set
			for (int i = 0; i < 15; i++) swiWaitForVBlank();
		}
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
	
	InitSound();
	
	char vertext[12];
	// snprintf(vertext, sizeof(vertext), "Ver %d.%d.%d   ", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH); // Doesn't work :(
	snprintf(vertext, sizeof(vertext), "Ver %d.%d.%d   ", 4, 3, 0);

	if (autorun || showlogo) {
		graphicsInit();
		fontInit();
		fadeType = true;
	}

	if (gotosettings) {
		if (!showlogo) {
			graphicsInit();
			fontInit();
			fadeType = true;
		}
		screenmode = 1;
		gotosettings = false;
		SaveSettings();
	} else if (autorun || showlogo) {

		for (int i = 0; i < 60*3; i++) {
			swiWaitForVBlank();
		}
		
		scanKeys();

		if (keysHeld() & KEY_START) {
			fadeType = false;
			for (int i = 0; i < 30; i++) {
				swiWaitForVBlank();
			}
			screenmode = 1;
			fadeType = true;
		}
	} else {
		scanKeys();

		if (keysHeld() & KEY_START) {
			graphicsInit();
			fontInit();
			fadeType = true;
			screenmode = 1;
			for (int i = 0; i < 60; i++) {
				swiWaitForVBlank();
			}
		}
	}


	srand(time(NULL));

	bool menuprinted = false;
	//const char* lrswitchpages = "L/R: Switch pages";

	bool hiyaAutobootFound = false;

	int pressed = 0;

	while(1) {
	
		if (screenmode == 1) {

			consoleClear();
			printf("\n ");
			printf(username);
			for (int i = 0; i < 21; i++) {
				printf("\n");
			}

			if (subscreenmode == 3) {
				pressed = 0;

				if (!menuprinted) {
					printSmall(true, 194, 176, vertext);

					// Clear the screen so it doesn't over-print
					clearText();

					printLarge(false, 6, 4, "Flashcard(s) select");

					int yPos = 32;
					switch (flashcard) {
						case 0:
						default:
							printSmall(false, 12, 32+(0*14), "DSTT");
							printSmall(false, 12, 32+(1*14), "R4i Gold");
							printSmall(false, 12, 32+(2*14), "R4i-SDHC (Non-v1.4.x version) (www.r4i-sdhc.com)");
							printSmall(false, 12, 32+(3*14), "R4 SDHC Dual-Core");
							printSmall(false, 12, 32+(4*14), "R4 SDHC Upgrade");
							printSmall(false, 12, 32+(5*14), "SuperCard DSONE");
							break;
						case 1:
							printSmall(false, 12, 32+(0*14), "Original R4");
							printSmall(false, 12, 32+(1*14), "M3 Simply");
							break;
						case 2:
							printSmall(false, 12, 32+(0*14), "R4iDSN");
							printSmall(false, 12, 32+(1*14), "R4i Gold RTS");
							printSmall(false, 12, 32+(2*14), "R4 Ultra");
							break;
						case 3:
							printSmall(false, 12, 32+(0*14), "Acekard 2(i)");
							printSmall(false, 12, 32+(1*14), "Galaxy Eagle");
							printSmall(false, 12, 32+(2*14), "M3DS Real");
							break;
						case 4:
							printSmall(false, 12, 32, "Acekard RPG");
							break;
						case 5:
							printSmall(false, 12, 32+(0*14), "Ace 3DS+");
							printSmall(false, 12, 32+(1*14), "Gateway Blue Card");
							printSmall(false, 12, 32+(2*14), "R4iTT");
							break;
						case 6:
							printSmall(false, 12, 32, "SuperCard DSTWO");
							break;
					}

					printLargeCentered(true, 120, "Left/Right: Select flashcard(s)");
					printLargeCentered(true, 134, "A/B: Set and return");

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
					printSmall(true, 194, 176, vertext);

					// Clear the screen so it doesn't over-print
					clearText();

					switch (theme) {
						case 0:
						default:
							printLarge(false, 6, 4, "Sub-theme select: DSi Menu");
							break;
						case 1:
							printLarge(false, 6, 4, "Sub-theme select: 3DS HOME Menu");
							break;
						case 2:
							printLarge(false, 6, 4, "Sub-theme select: R4");
							break;
					}

					int yPos;
					switch (subtheme) {
						case 0:
						default:
							yPos = 32;
							break;
						case 1:
							yPos = 46;
							break;
					}

					printSmall(false, 4, yPos, ">");

					switch (theme) {
						case 0:
						default:
							printSmall(false, 12, 32, "SD Card Menu");
							printSmall(false, 12, 46, "Normal Menu");
							break;
						case 1:
							//printSmall(false, 12, 32, "DS Menu");
							//printSmall(false, 12, 46, "3DS HOME Menu");
							break;
					}

					printLargeCentered(true, 128, "A/B: Set sub-theme.");

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

				if (subtheme > 1) subtheme = 0;
				else if (subtheme < 0) subtheme = 1;
			} else if (subscreenmode == 1) {
				pressed = 0;

				if (!menuprinted) {
					// Clear the screen so it doesn't over-print
					clearText();

					printSmallCentered(false, 175, "DSiMenu++");

					printSmall(true, 4, 176, "^/~ Switch pages");
					printSmall(true, 194, 176, vertext);

					printLarge(false, 6, 4, "Games and Apps Settings");

					int yPos = 32;
					for (int i = 0; i < settingscursor; i++) {
						yPos += 12;
					}

					int selyPos = 32;

					printSmall(false, 4, yPos, ">");

					if(!flashcardUsed) {
						printSmall(false, 12, selyPos, "ARM9 CPU Speed");
						if(boostCpu)
							printSmall(false, 156, selyPos, "133mhz (TWL)");
						else
							printSmall(false, 156, selyPos, "67mhz (NTR)");
						selyPos += 12;

						printSmall(false, 12, selyPos, "Debug");
						if(bstrap_debug)
							printSmall(false, 224, selyPos, "On");
						else
							printSmall(false, 224, selyPos, "Off");
						selyPos += 12;
						printSmall(false, 12, selyPos, "ROM read LED");
						switch(bstrap_romreadled) {
							case 0:
							default:
								romreadled_valuetext = "None";
								break;
							case 1:
								romreadled_valuetext = "WiFi-Power";
								break;
							case 2:
								romreadled_valuetext = "Power-Wifi";
								break;
							case 3:
								romreadled_valuetext = "Camera";
								break;
						}
						printSmall(false, 184, selyPos, romreadled_valuetext);
						selyPos += 12;

						printSmall(false, 12, selyPos, "Sound/Mic frequency");
						if(soundfreq)
							printSmall(false, 184, selyPos, "47.61 kHz");
						else
							printSmall(false, 184, selyPos, "32.73 kHz");
						selyPos += 12;

						printSmall(false, 12, selyPos, "Loading screen");
						switch(bstrap_loadingScreen) {
							case 0:
							default:
								loadingScreen_valuetext = "None";
								break;
							case 1:
								loadingScreen_valuetext = "Regular";
								break;
							case 2:
								loadingScreen_valuetext = "Pong";
								break;
							case 3:
								loadingScreen_valuetext = "Tic-Tac-Toe";
								break;
						}
						printSmall(false, 176, selyPos, loadingScreen_valuetext);
						selyPos += 12;

						printSmall(false, 12, selyPos, "Bootstrap");
						if(bootstrapFile)
							printSmall(false, 176, selyPos, "Nightly");
						else
							printSmall(false, 176, selyPos, "Release");


						if (settingscursor == 0) {
							printLargeCentered(true, 120, "Set to TWL to get rid of lags");
							printLargeCentered(true, 134, "in some games.");
						} /* else if (settingscursor == 4) {
							printLargeCentered(true, 120, "Allows 8 bit VRAM writes");
							printLargeCentered(true, 134, "and expands the bus to 32 bit.");
						} */ else if (settingscursor == 1) {
							printLargeCentered(true, 120, "Displays some text before");
							printLargeCentered(true, 134, "launched game.");
						} else if (settingscursor == 2) {
							// printLargeCentered(true, 114, "Locks the ARM9 SCFG_EXT,");
							// printLargeCentered(true, 128, "avoiding conflict with");
							// printLargeCentered(true, 142, "recent libnds.");
							printLargeCentered(true, 128, "Sets LED as ROM read indicator.");
						} else if (settingscursor == 3) {
							printLargeCentered(true, 120, "32.73 kHz: Original quality");
							printLargeCentered(true, 134, "47.61 kHz: High quality");
						} else if (settingscursor == 4) {
							printLargeCentered(true, 120, "Shows a loading screen before ROM");
							printLargeCentered(true, 134, "is started in nds-bootstrap.");
						} else if (settingscursor == 5) {
							printLargeCentered(true, 120, "Pick release or nightly");
							printLargeCentered(true, 134, "bootstrap.");
						}
					} else {
						printSmall(false, 12, selyPos, "Flashcard(s) select");
						selyPos += 12;
						if(soundfreqsetting) {
							printSmall(false, 12, selyPos, "Sound/Mic frequency");
							if(soundfreq)
								printSmall(false, 184, selyPos, "47.61 kHz");
							else
								printSmall(false, 184, selyPos, "32.73 kHz");
						} else {
							printSmall(false, 12, selyPos, "Use GBARunner2");
							if(useGbarunner)
								printSmall(false, 224, selyPos, "Yes");
							else
								printSmall(false, 224, selyPos, "No");
						}

						if (settingscursor == 0) {
							printLargeCentered(true, 120, "Pick a flashcard to use to");
							printLargeCentered(true, 134, "run ROMs from it.");
						} else if (settingscursor == 1) {
							if(soundfreqsetting) {
								printLargeCentered(true, 120, "32.73 kHz: Original quality");
								printLargeCentered(true, 134, "47.61 kHz: High quality");
							} else {
								printLargeCentered(true, 120, "Use either GBARunner2 or the");
								printLargeCentered(true, 134, "native GBA mode to play GBA games.");
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
					mmEffectEx(&snd_select);
					menuprinted = false;
				}
				if (pressed & KEY_DOWN) {
					settingscursor++;
					mmEffectEx(&snd_select);
					menuprinted = false;
				}
					
				if ((pressed & KEY_A) || (pressed & KEY_LEFT) || (pressed & KEY_RIGHT)) {
					if(!flashcardUsed) {
						switch (settingscursor) {
							case 0:
							default:
								boostCpu = !boostCpu;
								break;
							case 1:
								bstrap_debug = !bstrap_debug;
								break;
							case 2:
								// bstrap_lockARM9scfgext = !bstrap_lockARM9scfgext;
								if (pressed & KEY_LEFT) {
									bstrap_romreadled--;
									if (bstrap_romreadled < 0) bstrap_romreadled = 2;
								} else if ((pressed & KEY_RIGHT) || (pressed & KEY_A)) {
									bstrap_romreadled++;
									if (bstrap_romreadled > 2) bstrap_romreadled = 0;
								}
								break;
							case 3:
								soundfreq = !soundfreq;
								break;
							case 4:
								if (pressed & KEY_LEFT) {
									bstrap_loadingScreen--;
									if (bstrap_loadingScreen < 0) bstrap_loadingScreen = 3;
								} else if ((pressed & KEY_RIGHT) || (pressed & KEY_A)) {
									bstrap_loadingScreen++;
									if (bstrap_loadingScreen > 3) bstrap_loadingScreen = 0;
								}
								break;
							case 5:
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
				
				if ((pressed & KEY_L) || (pressed & KEY_R)) {
					subscreenmode = 0;
					settingscursor = 0;
					mmEffectEx(&snd_switch);
					menuprinted = false;
				}

				if (pressed & KEY_B) {
					mmEffectEx(&snd_back);
					clearText();
					printSmall(false, 4, 4, "Saving settings...");
					SaveSettings();
					clearText();
					printSmall(false, 4, 4, "Settings saved!");
					for (int i = 0; i < 60; i++) swiWaitForVBlank();
					loadROMselect();
					break;
				}

				if(!flashcardUsed) {
					if (settingscursor > 5) settingscursor = 0;
					else if (settingscursor < 0) settingscursor = 5;
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

					printSmallCentered(false, 175, "DSiMenu++");

					printSmall(true, 4, 176, "^/~ Switch pages");
					printSmall(true, 194, 176, vertext);

					printLarge(false, 6, 4, "GUI Settings");
					
					int yPos = 32;
					for (int i = 0; i < settingscursor; i++) {
						yPos += 12;
					}

					int selyPos = 32;

					printSmall(false, 4, yPos, ">");

					printSmall(false, 12, selyPos, "Theme");
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

					printSmall(false, 12, selyPos, "Last played ROM on startup");
					if(autorun)
						printSmall(false, 224, selyPos, "Yes");
					else
						printSmall(false, 230, selyPos, "No");
					selyPos += 12;

					printSmall(false, 12, selyPos, "DSiMenu++ logo on startup");
					if(showlogo)
						printSmall(false, 216, selyPos, "Show");
					else
						printSmall(false, 222, selyPos, "Hide");
					selyPos += 12;

					printSmall(false, 12, selyPos, "Directories/folders");
					if(showDirectories)
						printSmall(false, 216, selyPos, "Show");
					else
						printSmall(false, 222, selyPos, "Hide");
					selyPos += 12;

					printSmall(false, 12, selyPos, "Animate DSi icons");
					if(animateDsiIcons)
						printSmall(false, 224, selyPos, "Yes");
					else
						printSmall(false, 230, selyPos, "No");
					selyPos += 12;

					if (!flashcardUsed && !arm7SCFGLocked) {
						printSmall(false, 12, selyPos, "Quick-start ROM");
						if(quickStartRom)
							printSmall(false, 224, selyPos, "Yes");
						else
							printSmall(false, 230, selyPos, "No");
						selyPos += 12;

						if (consoleModel < 2) {
							if (hiyaAutobootFound) {
								printSmall(false, 12, selyPos, "Restore DSi Menu");
							} else {
								printSmall(false, 12, selyPos, "Replace DSi Menu");
							}
						}
					}


					if (settingscursor == 0) {
						printLargeCentered(true, 120, "The theme to use in DSiMenu++.");
						printLargeCentered(true, 134, "Press A for sub-themes.");
					} else if (settingscursor == 1) {
						printLargeCentered(true, 106, "If turned on, hold B on");
						printLargeCentered(true, 120, "startup to skip to the");
						printLargeCentered(true, 134, "ROM select menu.");
						printLargeCentered(true, 148, "Press Y to start last played ROM.");
					} else if (settingscursor == 2) {
						printLargeCentered(true, 114, "The DSiMenu++ logo will be");
						printLargeCentered(true, 128, "shown when you start");
						printLargeCentered(true, 142, "DSiMenu++.");
					} else if (settingscursor == 3) {
						printLargeCentered(true, 114, "If you're in a folder where most");
						printLargeCentered(true, 128, "of your games are, it is safe to");
						printLargeCentered(true, 142, "hide directories/folders.");
					} else if (settingscursor == 4) {
						printLargeCentered(true, 114, "Animates DSi-enhanced icons like in");
						printLargeCentered(true, 128, "the DSi/3DS menus. Turning this off");
						printLargeCentered(true, 142, "will fix some icons appearing white.");
					} else if (settingscursor == 5) {
						if (consoleModel < 2) {
							printLargeCentered(true, 114, "Bypasses rebooting, in case if");
							printLargeCentered(true, 128, "you're back in the DSi Menu,");
							printLargeCentered(true, 142, "after launching a game.");
						} else {
							printLargeCentered(true, 106, "Bypasses rebooting, in case if");
							printLargeCentered(true, 120, "you're back in the 3DS HOME Menu,");
							printLargeCentered(true, 134, "after launching a game, or if the");
							printLargeCentered(true, 148, "Luma exception screen appears.");
						}
					} else if (settingscursor == 6) {
						if (hiyaAutobootFound) {
							printLargeCentered(true, 128, "Show DSi Menu on boot again.");
						} else {
							printLargeCentered(true, 120, "Start DSiMenu++ on boot, instead.");
							printLargeCentered(true, 134, "of the regular DSi Menu.");
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
								subtheme = 0;
								theme -= 1;
								if (theme < 0) theme = 2;
								mmEffectEx(&snd_select);
							} else if (pressed & KEY_RIGHT) {
								subtheme = 0;
								theme += 1;
								if (theme > 2) theme = 0;
								mmEffectEx(&snd_select);
							} else if (theme > 0) {
								mmEffectEx(&snd_wrong);
							} else {
								subscreenmode = 2;
								mmEffectEx(&snd_select);
							}
							break;
						case 1:
							autorun = !autorun;
							mmEffectEx(&snd_select);
							break;
						case 2:
							showlogo = !showlogo;
							mmEffectEx(&snd_select);
							break;
						case 3:
							showDirectories = !showDirectories;
							mmEffectEx(&snd_select);
							break;
						case 4:
							animateDsiIcons = !animateDsiIcons;
							mmEffectEx(&snd_select);
							break;
						case 5:
							quickStartRom = !quickStartRom;
							mmEffectEx(&snd_select);
							break;
						case 6:
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

				if ((pressed & KEY_L) || (pressed & KEY_R)) {
					subscreenmode = 1;
					settingscursor = 0;
					mmEffectEx(&snd_switch);
					menuprinted = false;
				}

				if (pressed & KEY_Y && settingscursor == 1) {
					screenmode = 0;
					mmEffectEx(&snd_launch);
					clearText();
					printSmall(false, 4, 4, "Saving settings...");
					SaveSettings();
					clearText();
					printSmall(false, 4, 4, "Settings saved!");
					for (int i = 0; i < 60; i++) swiWaitForVBlank();
					int err = lastRanROM();
					iprintf ("Start failed. Error %i\n", err);
				}

				if (pressed & KEY_B) {
					mmEffectEx(&snd_back);
					clearText();
					printSmall(false, 4, 4, "Saving settings...");
					SaveSettings();
					clearText();
					printSmall(false, 4, 4, "Settings saved!");
					for (int i = 0; i < 60; i++) swiWaitForVBlank();
					loadROMselect();
					break;
				}

				if (!flashcardUsed) {
					if (consoleModel < 2) {
						if (settingscursor > 6) settingscursor = 0;
						else if (settingscursor < 0) settingscursor = 6;
					} else {
						if (settingscursor > 5) settingscursor = 0;
						else if (settingscursor < 0) settingscursor = 5;
					}
				} else {
					if (settingscursor > 4) settingscursor = 0;
					else if (settingscursor < 0) settingscursor = 4;
				}
			}

		} else {
			// Save quick-start ROM setting
			CIniFile bootstrapini( bootstrapinipath );
			bootstrapini.SetInt("NDS-BOOTSTRAP", "NTR_TOUCH", quickStartRom);
			bootstrapini.SaveIniFile(bootstrapinipath);
			loadROMselect();
		}

	}

	return 0;
}
