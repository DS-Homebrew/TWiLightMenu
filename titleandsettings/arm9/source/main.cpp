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
#include <gl2d.h>

#include "graphics/graphics.h"

#include "ndsLoaderArm9.h"

#include "graphics/fontHandler.h"

#include "inifile.h"

const char* settingsinipath = "/_nds/srloader/settings.ini";
const char* twldrsettingsinipath = "sd:/_nds/twloader/settings.ini";
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

static bool autorun = false;
static int theme = 0;
static int subtheme = 0;

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

	autorun = settingsini.GetInt("SRLOADER", "AUTORUNGAME", 0);
	showlogo = settingsini.GetInt("SRLOADER", "SHOWLOGO", 1);
	gotosettings = settingsini.GetInt("SRLOADER", "GOTOSETTINGS", 0);
	soundfreq = settingsini.GetInt("SRLOADER", "SOUND_FREQ", 0);
	flashcard = settingsini.GetInt("SRLOADER", "FLASHCARD", 0);
	bootstrapFile = settingsini.GetInt("SRLOADER", "BOOTSTRAP_FILE", 0);

	// Customizable UI settings.
	//theme = settingsini.GetInt("SRLOADER", "THEME", 0);
	subtheme = settingsini.GetInt("SRLOADER", "SUB_THEME", 0);
	is3DS = settingsini.GetInt("SRLOADER", "IS_3DS", 0);

	if(!flashcardUsed) {
		// nds-bootstrap
		CIniFile bootstrapini( bootstrapinipath );

		bstrap_useArm7Donor = bootstrapini.GetInt( "NDS-BOOTSTRAP", "USE_ARM7_DONOR", 1);
		bstrap_boostcpu = bootstrapini.GetInt("NDS-BOOTSTRAP", "BOOST_CPU", 0);
		bstrap_debug = bootstrapini.GetInt("NDS-BOOTSTRAP", "DEBUG", 0);
		bstrap_romreadled = bootstrapini.GetInt("NDS-BOOTSTRAP", "ROMREAD_LED", 1);
		ntr_touch = bootstrapini.GetInt("NDS-BOOTSTRAP", "NTR_TOUCH", 1);
		donorSdkVer = bootstrapini.GetInt( "NDS-BOOTSTRAP", "DONOR_SDK_VER", 0);
		bstrap_loadingScreen = bootstrapini.GetInt( "NDS-BOOTSTRAP", "LOADING_SCREEN", 1);
		// bstrap_lockARM9scfgext = bootstrapini.GetInt("NDS-BOOTSTRAP", "LOCK_ARM9_SCFG_EXT", 0);
	}
}

void SaveSettings(void) {
	bool twldrsettingsFound = false;
	if (!access(twldrsettingsinipath, F_OK)) twldrsettingsFound = true;

	// GUI
	CIniFile settingsini( settingsinipath );

	settingsini.SetInt("SRLOADER", "AUTORUNGAME", autorun);
	settingsini.SetInt("SRLOADER", "SHOWLOGO", showlogo);
	settingsini.SetInt("SRLOADER", "GOTOSETTINGS", gotosettings);
	settingsini.SetInt("SRLOADER", "SOUND_FREQ", soundfreq);
	settingsini.SetInt("SRLOADER", "FLASHCARD", flashcard);
	settingsini.SetInt("SRLOADER", "BOOTSTRAP_FILE", bootstrapFile);

	// UI settings.
	settingsini.SetInt("SRLOADER", "THEME", theme);
	settingsini.SetInt("SRLOADER", "SUB_THEME", subtheme);
	settingsini.SetInt("SRLOADER", "IS_3DS", is3DS);
	settingsini.SaveIniFile(settingsinipath);
	
	if(is3DS && twldrsettingsFound && !flashcardUsed) {
		// Save some settings to TWLoader as well.
		CIniFile twldrsettingsini( twldrsettingsinipath );
		
		twldrsettingsini.SetInt("TWL-MODE", "BOOTSTRAP_FILE", bootstrapFile);
		twldrsettingsini.SaveIniFile(twldrsettingsinipath);
	}

	if(!flashcardUsed) {
		// nds-bootstrap
		CIniFile bootstrapini( bootstrapinipath );

		bootstrapini.SetInt("NDS-BOOTSTRAP", "USE_ARM7_DONOR", bstrap_useArm7Donor);
		bootstrapini.SetInt("NDS-BOOTSTRAP", "BOOST_CPU", bstrap_boostcpu);
		bootstrapini.SetInt("NDS-BOOTSTRAP", "DEBUG", bstrap_debug);
		bootstrapini.SetInt("NDS-BOOTSTRAP", "ROMREAD_LED", bstrap_romreadled);
		bootstrapini.SetInt("NDS-BOOTSTRAP", "NTR_TOUCH", ntr_touch);
		bootstrapini.SetInt("NDS-BOOTSTRAP", "LOADING_SCREEN", bstrap_loadingScreen);
		// bootstrapini.SetInt("NDS-BOOTSTRAP", "LOCK_ARM9_SCFG_EXT", bstrap_lockARM9scfgext);
		bootstrapini.SaveIniFile(bootstrapinipath);
	}
}

static int screenmode = 0;
static int subscreenmode = 0;

static int settingscursor = 0;

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
	if(soundfreq) fifoSendValue32(FIFO_MAXMOD, 2);
	else fifoSendValue32(FIFO_MAXMOD, 1);
	runNdsFile ("/_nds/srloader/dsimenu.srldr", 0, NULL);
}

int lastRanROM() {
	int err = 0;
	if(!flashcardUsed) {
		if (!arm7SCFGLocked) {
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
		} else {
			if(donorSdkVer==5) {
				if (bootstrapFile) bootstrapfilename = "sd:/_nds/unofficial-dsi-bootstrap-sdk5.nds";
				else bootstrapfilename = "sd:/_nds/release-dsi-bootstrap-sdk5.nds";
			} else {
				if (bootstrapFile) bootstrapfilename = "sd:/_nds/unofficial-dsi-bootstrap.nds";
				else bootstrapfilename = "sd:/_nds/release-dsi-bootstrap.nds";
			}
		}
		err = runNdsFile (bootstrapfilename.c_str(), 0, NULL);
	} else {
		switch (flashcard) {
			case 0:
			case 1:
			default:
				err = runNdsFile ("fat:/YSMenu.nds", 0, NULL);
				break;
			case 2:
			case 4:
			case 5:
				err = runNdsFile ("fat:/Wfwd.dat", 0, NULL);
				break;
			case 3:
				err = runNdsFile ("fat:/Afwd.dat", 0, NULL);
				break;
			case 6:
				err = runNdsFile ("fat:/_dstwo/autoboot.nds", 0, NULL);
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

	if (fatInitDefault()) {
		if (!access("fat:/", F_OK)) flashcardUsed = true;
	}

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
		printf("\n ");
		printf(username);
		printSmall(false, 4, 4, "fatinitDefault failed!");
		stop();
	}

	bool soundfreqsetting = false;

	std::string filename;
	
	LoadSettings();
	
	swiWaitForVBlank();

	fifoWaitValue32(FIFO_USER_06);
	if (fifoGetValue32(FIFO_USER_03) == 0) arm7SCFGLocked = true;	// If SRLoader is being ran from DSiWarehax or flashcard, then arm7 SCFG is locked.

	u16 arm7_SNDEXCNT = fifoGetValue32(FIFO_MAXMOD);
	if (arm7_SNDEXCNT != 0) soundfreqsetting = true;
	fifoSendValue32(FIFO_MAXMOD, 0);

	scanKeys();

	if(!flashcardUsed) {
		if(keysHeld() & KEY_UP) {
			is3DS = true;
		}
		if(keysHeld() & KEY_DOWN) {
			is3DS = false;
		}

		if(is3DS) {
			consoleText = "Console: Nintendo 3DS/2DS";
		} else {
			consoleText = "Console: Nintendo DSi";
		}
	}

	if (!gotosettings && autorun && !(keysHeld() & KEY_B)) {
		lastRanROM();
	}
	
	char vertext[12];
	// snprintf(vertext, sizeof(vertext), "Ver %d.%d.%d   ", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH); // Doesn't work :(
	snprintf(vertext, sizeof(vertext), "Ver %d.%d.%d   ", 2, 2, 0);

	if (showlogo) {
		graphicsInit();
		fontInit();
	}

	if (gotosettings) {
		if (!showlogo) {
			graphicsInit();
			fontInit();
		}
		screenmode = 1;
		gotosettings = false;
		SaveSettings();
	} else if (showlogo) {
		unsigned int * SCFG_ROM=(unsigned int*)0x4004000;
		unsigned int * SCFG_CLK=(unsigned int*)0x4004004; 
		unsigned int * SCFG_EXT=(unsigned int*)0x4004008;
		unsigned int * SCFG_MC=(unsigned int*)0x4004010;

		for (int i = 0; i < 20; i++) {
			printf("\n");
		}
		iprintf(" %s\n\n", consoleText);
		printf("                      %s", vertext);
		if(*SCFG_EXT>0) {
			char text1[48],
				text2[48],
				text3[48],
				text4[48],
				text5[48],
				text6[48],
				text7[48],
				text8[32],
				text9[32];
			//arm9 SCFG
			snprintf (text1, sizeof(text1), "SCFG_ROM: %x",*SCFG_ROM);
			snprintf (text2, sizeof(text2), "SCFG_CLK: %x",*SCFG_CLK);
			snprintf (text3, sizeof(text3), "SCFG_EXT: %x",*SCFG_EXT);			
			snprintf (text4, sizeof(text4), "SCFG_MC: %x",*SCFG_MC);
			if (!arm7SCFGLocked) {
				//arm7 SCFG
				snprintf (text5, sizeof(text5), "SCFG_ROM: %x",fifoGetValue32(FIFO_USER_01));
				snprintf (text6, sizeof(text6), "SCFG_CLK: %x",fifoGetValue32(FIFO_USER_02));
				snprintf (text7, sizeof(text7), "SCFG_EXT: %x",fifoGetValue32(FIFO_USER_03));
				//ConsoleID
				snprintf (text8, sizeof(text8), "ConsoleID: %x%x",fifoGetValue32(FIFO_USER_04),fifoGetValue32(FIFO_USER_05));
			}
			//snprintf (text9, sizeof(text9), "Console: %x", *(u8*)0x0DFFFFFA);

			int yPos = 4;

			printSmall(false, 4, yPos, "ARM9 SCFG:");
			yPos += 16;
			printSmall(false, 4, yPos, text1);
			yPos += 8;
			printSmall(false, 4, yPos, text2);
			yPos += 8;
			printSmall(false, 4, yPos, text3);
			yPos += 24;
			printSmall(false, 4, yPos, "Slot-1 Status:");
			yPos += 16;
			printSmall(false, 4, yPos, text4);
			if (!arm7SCFGLocked) {
				yPos += 24;
				printSmall(false, 4, yPos, "ARM7 SCFG:");
				yPos += 16;
				printSmall(false, 4, yPos, text5);
				yPos += 8;
				printSmall(false, 4, yPos, text6);
				yPos += 8;
				printSmall(false, 4, yPos, text7);
				yPos += 24;
				printSmall(false, 4, yPos, text8);
			}
			//yPos += 24;
			//printSmall(false, 4, yPos, text9);
		}

		for (int i = 0; i < 60*3; i++) {
			swiWaitForVBlank();
		}
		
		scanKeys();

		if (keysHeld() & KEY_START)
			screenmode = 1;

	} else {
		scanKeys();

		if (keysHeld() & KEY_START) {
			graphicsInit();
			fontInit();
			screenmode = 1;
			for (int i = 0; i < 60; i++) {
				swiWaitForVBlank();
			}
		}
	}


	srand(time(NULL));

	bool menuprinted = false;
	//const char* lrswitchpages = "L/R: Switch pages";

	int pressed = 0;

	while(1) {
	
		if (screenmode == 1) {

			consoleClear();
			printf("\n ");
			printf(username);
			for (int i = 0; i < 19; i++) {
				printf("\n");
			}
			iprintf(" %s\n\n", consoleText);

			if (subscreenmode == 3) {
				pressed = 0;

				if (!menuprinted) {
					printf("                      %s", vertext);

					// Clear the screen so it doesn't over-print
					clearText();

					printLarge(false, 4, 4, "Flashcard(s) select");

					int yPos;
					switch (flashcard) {
						case 0:
						default:
							printSmall(false, 12, 24, "DSTT");
							printSmall(false, 12, 32, "R4i Gold");
							printSmall(false, 12, 40, "R4i-SDHC (Non-v1.4.x version) (www.r4i-sdhc.com)");
							printSmall(false, 12, 48, "R4 SDHC Dual-Core");
							printSmall(false, 12, 56, "R4 SDHC Upgrade");
							printSmall(false, 12, 64, "SuperCard DSONE");
							break;
						case 1:
							printSmall(false, 12, 24, "Original R4");
							printSmall(false, 12, 32, "M3 Simply");
							break;
						case 2:
							printSmall(false, 12, 24, "R4iDSN");
							printSmall(false, 12, 32, "R4i Gold RTS");
							printSmall(false, 12, 40, "R4 Ultra");
							break;
						case 3:
							printSmall(false, 12, 24, "Acekard 2(i)");
							printSmall(false, 12, 32, "Galaxy Eagle");
							printSmall(false, 12, 40, "M3DS Real");
							break;
						case 4:
							printSmall(false, 12, 24, "Acekard RPG");
							break;
						case 5:
							printSmall(false, 12, 24, "Ace 3DS+");
							printSmall(false, 12, 32, "Gateway Blue Card");
							printSmall(false, 12, 40, "R4iTT");
							break;
						case 6:
							printSmall(false, 12, 24, "SuperCard DSTWO");
							break;
					}

					printSmall(false, 4, 164, "Left/Right: Select flashcard(s)");
					printSmall(false, 4, 172, "A/B: Set and return");

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
					menuprinted = false;
				}
				if (pressed & KEY_RIGHT) {
					flashcard += 1;
					menuprinted = false;
				}

				if ((pressed & KEY_A) || (pressed & KEY_B)) {
					subscreenmode = 1;
					menuprinted = false;
				}

				if (flashcard > 6) flashcard = 0;
				else if (flashcard < 0) flashcard = 6;
			} else if (subscreenmode == 2) {
				pressed = 0;

				if (!menuprinted) {
					printf("                      %s", vertext);

					// Clear the screen so it doesn't over-print
					clearText();

					switch (theme) {
						case 0:
						default:
							printLarge(false, 4, 4, "Sub-theme select: DSi Menu");
							break;
						case 1:
							printLarge(false, 4, 4, "Sub-theme select: Aura Launcher");
							break;
					}

					int yPos;
					switch (subtheme) {
						case 0:
						default:
							yPos = 24;
							break;
						case 1:
							yPos = 32;
							break;
					}

					printSmall(false, 4, yPos, ">");

					switch (theme) {
						case 0:
						default:
							printSmall(false, 12, 24, "SD Card Menu");
							printSmall(false, 12, 32, "Normal Menu");
							break;
						case 1:
							printSmall(false, 12, 24, "DS Menu");
							printSmall(false, 12, 32, "3DS HOME Menu");
							break;
					}

					printSmall(false, 4, 156, "A/B: Set sub-theme.");

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
					menuprinted = false;
				}
				if (pressed & KEY_DOWN) {
					subtheme += 1;
					menuprinted = false;
				}

				if ((pressed & KEY_A) || (pressed & KEY_B)) {
					subscreenmode = 0;
					menuprinted = false;
				}

				if (subtheme > 1) subtheme = 0;
				else if (subtheme < 0) subtheme = 1;
			} else if (subscreenmode == 1) {
				pressed = 0;

				if (!menuprinted) {
					printf(" L/R: Switch pages    %s", vertext);

					// Clear the screen so it doesn't over-print
					clearText();

					printLarge(false, 4, 4, "Settings: Games/Apps");

					int yPos;
					switch (settingscursor) {
						case 0:
						default:
							yPos = 24;
							break;
						case 1:
							yPos = 32;
							break;
						case 2:
							yPos = 40;
							break;
						case 3:
							yPos = 48;
							break;
						case 4:
							yPos = 56;
							break;
						case 5:
							yPos = 64;
							break;
						case 6:
							yPos = 72;
							break;
					}

					int selyPos = 24;

					printSmall(false, 4, yPos, ">");

					if(!flashcardUsed) {
						printSmall(false, 12, selyPos, "ARM9 CPU Speed");
						if(bstrap_boostcpu)
							printSmall(false, 156, selyPos, "133mhz (TWL)");
						else
							printSmall(false, 156, selyPos, "67mhz (NTR)");
						selyPos += 8;

						// if (bstrap_boostcpu) {
						// 	printSmall(false, 12, 96, "VRAM boost");
						// 	if(bstrap_boostvram)
						// 		printSmall(false, 224, 96, "On");
						// 	else
						// 		printSmall(false, 224, 96, "Off");
						// }

						printSmall(false, 12, selyPos, "Debug");
						if(bstrap_debug)
							printSmall(false, 224, selyPos, "On");
						else
							printSmall(false, 224, selyPos, "Off");
						selyPos += 8;
						printSmall(false, 12, selyPos, "ROM read LED");
						switch(bstrap_romreadled) {
							case 0:
							default:
								romreadled_valuetext = "None";
								break;
							case 1:
								romreadled_valuetext = "WiFi";
								break;
							case 2:
								romreadled_valuetext = "Power";
								break;
							case 3:
								romreadled_valuetext = "Camera";
								break;
						}
						printSmall(false, 208, selyPos, romreadled_valuetext);
						selyPos += 8;
						printSmall(false, 12, selyPos, "Use donor ROM");
						switch(bstrap_useArm7Donor) {
							case 0:
							default:
								useArm7Donor_valuetext = "Off";
								break;
							case 1:
								useArm7Donor_valuetext = "On";
								break;
							case 2:
								useArm7Donor_valuetext = "Force-use";
								break;
						}
						printSmall(false, 184, selyPos, useArm7Donor_valuetext);
						selyPos += 8;

						printSmall(false, 12, selyPos, "Sound/Mic frequency");
						if(soundfreq)
							printSmall(false, 184, selyPos, "47.61 kHz");
						else
							printSmall(false, 184, selyPos, "32.73 kHz");
						selyPos += 8;

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
						selyPos += 8;

						printSmall(false, 12, selyPos, "Bootstrap");
						if(bootstrapFile)
							printSmall(false, 184, selyPos, "Unofficial");
						else
							printSmall(false, 184, selyPos, "Release");

						if(!arm7SCFGLocked && !is3DS){
							selyPos += 8;
							printSmall(false, 12, selyPos, "NTR Touch Screen mode");
							if(ntr_touch)
								printSmall(false, 224, selyPos, "On");
							else
								printSmall(false, 224, selyPos, "Off");
						}


						if (settingscursor == 0) {
							printSmall(false, 4, 164, "Set to TWL to get rid of lags");
							printSmall(false, 4, 172, "in some games.");
						} /* else if (settingscursor == 4) {
							printSmall(false, 4, 164, "Allows 8 bit VRAM writes");
							printSmall(false, 4, 172, "and expands the bus to 32 bit.");
						} */ else if (settingscursor == 1) {
							printSmall(false, 4, 164, "Displays some text before");
							printSmall(false, 4, 172, "launched game.");
						} else if (settingscursor == 2) {
							// printSmall(false, 4, 156, "Locks the ARM9 SCFG_EXT,");
							// printSmall(false, 4, 164, "avoiding conflict with");
							// printSmall(false, 4, 172, "recent libnds.");
							printSmall(false, 4, 164, "Sets LED as ROM read indicator.");
						} else if (settingscursor == 3) {
							printSmall(false, 4, 164, "Enable, disable, or force use of");
							printSmall(false, 4, 172, "donor ROM.");
						} else if (settingscursor == 4) {
							printSmall(false, 4, 164, "32.73 kHz: Original quality");
							printSmall(false, 4, 172, "47.61 kHz: High quality");
						} else if (settingscursor == 5) {
							printSmall(false, 4, 164, "Shows a loading screen before ROM");
							printSmall(false, 4, 172, "is started in nds-bootstrap.");
						} else if (settingscursor == 6) {
							printSmall(false, 4, 164, "Pick release or unofficial");
							printSmall(false, 4, 172, "bootstrap.");
						} else if (settingscursor == 7 && !arm7SCFGLocked && !is3DS) {
							printSmall(false, 4, 164, "If launched from DSi Menu via");
							printSmall(false, 4, 172, "HiyaCFW, disable this option.");
						}
					} else {
						printSmall(false, 12, selyPos, "Flashcard(s) select");
						selyPos += 8;
						if(soundfreqsetting) {
							printSmall(false, 12, selyPos, "Sound/Mic frequency");
							if(soundfreq)
								printSmall(false, 184, selyPos, "47.61 kHz");
							else
								printSmall(false, 184, selyPos, "32.73 kHz");
						}

						if (settingscursor == 0) {
							printSmall(false, 4, 164, "Pick a flashcard to use to");
							printSmall(false, 4, 172, "run ROMs from it.");
						} else if (settingscursor == 1) {
							printSmall(false, 4, 164, "32.73 kHz: Original quality");
							printSmall(false, 4, 172, "47.61 kHz: High quality");
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
					menuprinted = false;
				}
				if (pressed & KEY_DOWN) {
					settingscursor += 1;
					menuprinted = false;
				}
					
				if ((pressed & KEY_A) || (pressed & KEY_LEFT) || (pressed & KEY_RIGHT)) {
					if(!flashcardUsed) {
						switch (settingscursor) {
							case 0:
							default:
								bstrap_boostcpu = !bstrap_boostcpu;
								break;
							case 1:
								bstrap_debug = !bstrap_debug;
								break;
							case 2:
								// bstrap_lockARM9scfgext = !bstrap_lockARM9scfgext;
								if (pressed & KEY_LEFT) {
									bstrap_romreadled -= 1;
									if (bstrap_romreadled < 0) bstrap_romreadled = 2;
								} else if ((pressed & KEY_RIGHT) || (pressed & KEY_A)) {
									bstrap_romreadled += 1;
									if (bstrap_romreadled > 2) bstrap_romreadled = 0;
								}
								break;
							case 3:
								if (pressed & KEY_LEFT) {
									bstrap_useArm7Donor -= 0;
									if (bstrap_useArm7Donor < 0) bstrap_useArm7Donor = 2;
								} else if ((pressed & KEY_RIGHT) || (pressed & KEY_A)) {
									bstrap_useArm7Donor += 1;
									if (bstrap_useArm7Donor > 2) bstrap_useArm7Donor = 0;
								}
								break;
							case 4:
								soundfreq = !soundfreq;
								break;
							case 5:
								if (pressed & KEY_LEFT) {
									bstrap_loadingScreen -= 0;
									if (bstrap_loadingScreen < 0) bstrap_loadingScreen = 3;
								} else if ((pressed & KEY_RIGHT) || (pressed & KEY_A)) {
									bstrap_loadingScreen += 1;
									if (bstrap_loadingScreen > 3) bstrap_loadingScreen = 0;
								}
								break;
							case 6:
								bootstrapFile = !bootstrapFile;
								break;
							case 7:
								if (!arm7SCFGLocked && !is3DS) ntr_touch = !ntr_touch;
								break;
						}
					} else {
						switch (settingscursor) {
							case 0:
							default:
								subscreenmode = 3;
								break;
							case 1:
								soundfreq = !soundfreq;
								break;
						}
					}
					menuprinted = false;
				}
				
				if ((pressed & KEY_L) || (pressed & KEY_R)) {
					subscreenmode = 0;
					settingscursor = 0;
					menuprinted = false;
				}

				if (pressed & KEY_B) {
					clearText();
					printSmall(false, 4, 4, "Saving settings...");
					SaveSettings();
					loadROMselect();
					break;
				}

				if(!flashcardUsed) {
					if(!arm7SCFGLocked && !is3DS) {
						if (settingscursor > 7) settingscursor = 0;
						else if (settingscursor < 0) settingscursor = 7;
					} else {
						if (settingscursor > 6) settingscursor = 0;
						else if (settingscursor < 0) settingscursor = 6;
					}
				} else {
					if(soundfreqsetting) {
						if (settingscursor > 1) settingscursor = 0;
						else if (settingscursor < 0) settingscursor = 1;
					} else {
						if (settingscursor != 0) settingscursor = 0;
					}
				}
			} else {
				pressed = 0;

				if (!menuprinted) {
					printf(" L/R: Switch pages    %s", vertext);

					// Clear the screen so it doesn't over-print
					clearText();

					printLarge(false, 4, 4, "Settings: GUI");
					
					int yPos;
					switch (settingscursor) {
						case 0:
						default:
							yPos = 24;
							break;
						case 1:
							yPos = 32;
							break;
						case 2:
							yPos = 48;
							break;
						case 3:
							yPos = 64;
							break;
					}

					printSmall(false, 4, yPos, ">");

					printSmall(false, 12, 24, "Theme");
					if(theme == 1)
						printSmall(false, 156, 24, "Aura Launcher");
					else
						printSmall(false, 156, 24, "DSi Menu");

					printSmall(false, 12, 32, "Run last played ROM on startup");
					if(autorun)
						printSmall(false, 224, 40, "On");
					else
						printSmall(false, 224, 40, "Off");

					printSmall(false, 12, 48, "Show SRLoader logo on startup");
					if(showlogo)
						printSmall(false, 224, 56, "On");
					else
						printSmall(false, 224, 56, "Off");


					if (settingscursor == 0) {
						printSmall(false, 4, 164, "The theme to use in SRLoader.");
						printSmall(false, 4, 172, "Press A for sub-themes.");
					} else if (settingscursor == 1) {
						printSmall(false, 4, 148, "If turned on, hold B on");
						printSmall(false, 4, 156, "startup to skip to the");
						printSmall(false, 4, 164, "ROM select menu.");
						printSmall(false, 4, 172, "Press Y to start last played ROM.");
					} else if (settingscursor == 2) {
						printSmall(false, 4, 156, "The SRLoader logo will be");
						printSmall(false, 4, 164, "shown when you start");
						printSmall(false, 4, 172, "SRLoader.");
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
					menuprinted = false;
				}
				if (pressed & KEY_DOWN) {
					settingscursor += 1;
					menuprinted = false;
				}
					
				if ((pressed & KEY_A) || (pressed & KEY_LEFT) || (pressed & KEY_RIGHT)) {
					switch (settingscursor) {
						case 0:
						default:
							if (pressed & KEY_LEFT) {
								//subtheme = 0;
								//theme -= 1;
								//if (theme < 0) theme = 1;
							} else if (pressed & KEY_RIGHT) {
								//subtheme = 0;
								//theme += 1;
								//if (theme > 1) theme = 0;
							} else
								subscreenmode = 2;
							break;
						case 1:
							autorun = !autorun;
							break;
						case 2:
							showlogo = !showlogo;
							break;
					}
					menuprinted = false;
				}
				
				if ((pressed & KEY_L) || (pressed & KEY_R)) {
					subscreenmode = 1;
					settingscursor = 0;
					menuprinted = false;
				}

				if (pressed & KEY_Y && settingscursor == 1) {
					screenmode = 0;
					clearText();
					printSmall(false, 4, 4, "Saving settings...");
					SaveSettings();
					int err = lastRanROM();
					iprintf ("Start failed. Error %i\n", err);
				}
				
				if (pressed & KEY_B) {
					clearText();
					printSmall(false, 4, 4, "Saving settings...");
					SaveSettings();
					loadROMselect();
					break;
				}
				
				if (settingscursor > 2) settingscursor = 0;
				else if (settingscursor < 0) settingscursor = 2;
			}

		} else {
			loadROMselect();
		}

	}

	return 0;
}
