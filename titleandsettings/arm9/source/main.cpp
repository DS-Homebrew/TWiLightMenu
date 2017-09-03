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

const char* settingsinipath = "sd:/_nds/srloader/settings.ini";
const char* twldrsettingsinipath = "sd:/_nds/twloader/settings.ini";
const char* bootstrapinipath = "sd:/_nds/nds-bootstrap.ini";

bool is3DS = false;

bool rebootInRocketLauncher = false;

bool showlogo = true;
bool gotosettings = false;

const char* romreadled_valuetext;
const char* useArm7Donor_valuetext;

int bstrap_useArm7Donor = 1;

bool bootstrapFile = false;

bool ntr_touch = true;

bool autorun = false;
int theme = 0;
int subtheme = 0;

bool bstrap_boostcpu = false;
bool bstrap_debug = false;
int bstrap_romreadled = 0;
// bool bstrap_lockARM9scfgext = false;

bool flashcardUsed = false;

int flashcard;
/* Flashcard value
	0: DSTT/R4i Gold/R4i-SDHC/R4 SDHC Dual-Core/R4 SDHC Upgrade/SC DSONE
	1: R4DS (Original Non-SDHC version)/ M3 Simply
	2: R4iDSN/R4i Gold RTS
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
	is3DS = settingsini.GetInt("SRLOADER", "IS_3DS", 0);
	rebootInRocketLauncher = settingsini.GetInt("SRLOADER", "REBOOT_INTO_ROCKETLAUNCHER", 0);
	flashcard = settingsini.GetInt("SRLOADER", "FLASHCARD", 0);
	bootstrapFile = settingsini.GetInt("SRLOADER", "BOOTSTRAP_FILE", 0);

	// Customizable UI settings.
	theme = settingsini.GetInt("SRLOADER", "THEME", 0);
	subtheme = settingsini.GetInt("SRLOADER", "SUB_THEME", 0);

	// nds-bootstrap
	CIniFile bootstrapini( bootstrapinipath );

	bstrap_useArm7Donor = bootstrapini.GetInt( "NDS-BOOTSTRAP", "USE_ARM7_DONOR", 1);
	bstrap_boostcpu = bootstrapini.GetInt("NDS-BOOTSTRAP", "BOOST_CPU", 0);
	bstrap_debug = bootstrapini.GetInt("NDS-BOOTSTRAP", "DEBUG", 0);
	bstrap_romreadled = bootstrapini.GetInt("NDS-BOOTSTRAP", "ROMREAD_LED", 1);
	ntr_touch = bootstrapini.GetInt("NDS-BOOTSTRAP", "NTR_TOUCH", 1);
	// bstrap_lockARM9scfgext = bootstrapini.GetInt("NDS-BOOTSTRAP", "LOCK_ARM9_SCFG_EXT", 0);
}

void SaveSettings(void) {
	bool twldrsettingsFound = false;
	if (!access(twldrsettingsinipath, F_OK)) twldrsettingsFound = true;

	// GUI
	CIniFile settingsini( settingsinipath );

	settingsini.SetInt("SRLOADER", "AUTORUNGAME", autorun);
	settingsini.SetInt("SRLOADER", "SHOWLOGO", showlogo);
	settingsini.SetInt("SRLOADER", "GOTOSETTINGS", gotosettings);
	settingsini.SetInt("SRLOADER", "IS_3DS", is3DS);
	settingsini.SetInt("SRLOADER", "REBOOT_INTO_ROCKETLAUNCHER", rebootInRocketLauncher);
	settingsini.SetInt("SRLOADER", "FLASHCARD", flashcard);
	settingsini.SetInt("SRLOADER", "BOOTSTRAP_FILE", bootstrapFile);

	// UI settings.
	settingsini.SetInt("SRLOADER", "THEME", theme);
	settingsini.SetInt("SRLOADER", "SUB_THEME", subtheme);
	settingsini.SaveIniFile(settingsinipath);
	
	if(is3DS && twldrsettingsFound) {
		// Save some settings to TWLoader as well.
		CIniFile twldrsettingsini( twldrsettingsinipath );
		
		twldrsettingsini.SetInt("TWL-MODE", "BOOTSTRAP_FILE", bootstrapFile);
		twldrsettingsini.SaveIniFile(twldrsettingsinipath);
	}

	// nds-bootstrap
	CIniFile bootstrapini( bootstrapinipath );

	bootstrapini.SetInt("NDS-BOOTSTRAP", "USE_ARM7_DONOR", bstrap_useArm7Donor);
	bootstrapini.SetInt("NDS-BOOTSTRAP", "BOOST_CPU", bstrap_boostcpu);
	bootstrapini.SetInt("NDS-BOOTSTRAP", "DEBUG", bstrap_debug);
	bootstrapini.SetInt("NDS-BOOTSTRAP", "ROMREAD_LED", bstrap_romreadled);
	bootstrapini.SetInt("NDS-BOOTSTRAP", "NTR_TOUCH", ntr_touch);
	// bootstrapini.SetInt("NDS-BOOTSTRAP", "LOCK_ARM9_SCFG_EXT", bstrap_lockARM9scfgext);
	bootstrapini.SaveIniFile(bootstrapinipath);
}

int screenmode = 0;
int subscreenmode = 0;

int settingscursor = 0;

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

void RocketLauncher() {
	FILE* RocketLauncherPayload = fopen("sd:/_nds/RocketLauncher.bin","rb");
	off_t fsize = 0;
	if (RocketLauncherPayload) {
		fseek(RocketLauncherPayload, 0, SEEK_END);
		fsize = ftell(RocketLauncherPayload);
		fseek(RocketLauncherPayload, 0, SEEK_SET);
		fread((void*)0x02800000,1,fsize,RocketLauncherPayload);
    }
	fclose(RocketLauncherPayload);
	FILE* ResetData = fopen("sd:/_nds/ResetData.bin","rb");
	fread((void*)0x02000300,1,32,ResetData);
	fclose(ResetData);
	fifoSendValue32(FIFO_USER_08, 1);
	for (int i = 0; i < 20; i++) {
		swiWaitForVBlank();
	}
}

void loadROMselect() {
	if (theme == 0) runNdsFile ("/_nds/srloader/dsimenu.srldr", 0, NULL);
	else runNdsFile ("/_nds/srloader/dsmenu.srldr", 0, NULL);
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

#ifndef EMULATE_FILES

	if (!fatInitDefault()) {
		graphicsInit();
		fontInit();
		printSmall(true, 1, 2, username);
		printSmall(false, 4, 4, "fatinitDefault failed!");
		stop();
	}

#endif

	bool arm7SCFGLocked = false;

	std::string filename;
	std::string bootstrapfilename;
	
	if (!access("fat:/", F_OK)) flashcardUsed = true;

	LoadSettings();
	
	swiWaitForVBlank();

	fifoWaitValue32(FIFO_USER_06);
	if (fifoGetValue32(FIFO_USER_03) == 0) arm7SCFGLocked = true;	// If SRLoader is being ran from DSiWarehax, then arm7 SCFG is locked.

	scanKeys();

	if (arm7SCFGLocked && !gotosettings && rebootInRocketLauncher && !(keysHeld() & KEY_L)) {
		RocketLauncher();
	}

	if (!gotosettings && autorun && !(keysHeld() & KEY_B)) {
		if (!arm7SCFGLocked) {
			if (is3DS) {
				if (bootstrapFile) bootstrapfilename = "sd:/_nds/unofficial-bootstrap.nds";
				else bootstrapfilename = "sd:/_nds/release-bootstrap.nds";
			} else {
				bootstrapfilename = "sd:/_nds/rocket-bootstrap.nds";
			}
		} else {
			bootstrapfilename = "sd:/_nds/dsiware-bootstrap.nds";
		}
		runNdsFile (bootstrapfilename.c_str(), 0, 0);
	}
	
	char vertext[12];
	// snprintf(vertext, sizeof(vertext), "Ver %d.%d.%d   ", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH); // Doesn't work :(
	snprintf(vertext, sizeof(vertext), "Ver %d.%d.%d   ", 1, 9, 0);

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
		
		if(*SCFG_EXT>0) {
			printSmall(true, 192, 184, vertext);
			
			char text1[48],
				text2[48],
				text3[48],
				text4[48],
				text5[48],
				text6[48],
				text7[48],
				text8[32];
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
			
			//test RAM
			//unsigned int * TEST32RAM=	(unsigned int*)0xD004000;		
			//*TEST32RAM = 0x55;
			//iprintf ("32 MB RAM ACCESS : %x\n\n",*TEST32RAM);
			// doPause(80, 140);
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
	const char* lrswitchpages = "L/R: Switch pages";

	int pressed = 0;

	while(1) {
	
		if (screenmode == 1) {
			
			if (subscreenmode == 3) {
				pressed = 0;

				if (!menuprinted) {
					// Clear the screen so it doesn't over-print
					clearText();
					
					printSmall(true, 1, 2, username);
					printSmall(true, 192, 184, vertext);
					
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

					printSmall(false, 4, 156, "Left/Right: Select flashcard(s)");
					printSmall(false, 4, 164, "A/B: Set and return");

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
					// Clear the screen so it doesn't over-print
					clearText();
					
					printSmall(true, 1, 2, username);
					printSmall(true, 192, 184, vertext);
					
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
					// Clear the screen so it doesn't over-print
					clearText();

					printSmall(true, 1, 2, username);
					printSmall(true, 1, 184, lrswitchpages);
					printSmall(true, 192, 184, vertext);
					
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

					printSmall(false, 4, yPos, ">");
					
					printSmall(false, 12, 24, "Flashcard(s) select");
						
					printSmall(false, 12, 32, "ARM9 CPU Speed");
					if(bstrap_boostcpu)
						printSmall(false, 156, 32, "133mhz (TWL)");
					else
						printSmall(false, 156, 32, "67mhz (NTR)");

					// if (bstrap_boostcpu) {
					// 	printSmall(false, 12, 96, "VRAM boost");
					// 	if(bstrap_boostvram)
					// 		printSmall(false, 224, 96, "On");
					// 	else
					// 		printSmall(false, 224, 96, "Off");
					// }

					printSmall(false, 12, 40, "Debug");
					if(bstrap_debug)
						printSmall(false, 224, 40, "On");
					else
						printSmall(false, 224, 40, "Off");

					printSmall(false, 12, 48, "ROM read LED");
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
					printSmall(false, 208, 48, romreadled_valuetext);

					printSmall(false, 12, 56, "Use donor ROM");
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
					printSmall(false, 184, 56, useArm7Donor_valuetext);

					if(!arm7SCFGLocked) {
						printSmall(false, 12, 64, "Game console");
						if(is3DS)
							printSmall(false, 200, 64, "3DS/2DS");
						else
							printSmall(false, 200, 64, "DSi");
						
						if(is3DS){
							printSmall(false, 12, 72, "Bootstrap");
							if(bootstrapFile)
								printSmall(false, 184, 72, "Unofficial");
							else
								printSmall(false, 184, 72, "Release");
						} else {
							printSmall(false, 12, 72, "NTR Touch Screen mode");
							if(ntr_touch)
								printSmall(false, 224, 72, "On");
							else
								printSmall(false, 224, 72, "Off");
						}
					}


					if (settingscursor == 0) {
						printSmall(false, 4, 156, "Pick a flashcard to use to");
						printSmall(false, 4, 164, "run ROMs from it.");
					} else if (settingscursor == 1) {
						printSmall(false, 4, 156, "Set to TWL to get rid of lags");
						printSmall(false, 4, 164, "in some games.");
					} /* else if (settingscursor == 4) {
						printSmall(false, 4, 156, "Allows 8 bit VRAM writes");
						printSmall(false, 4, 164, "and expands the bus to 32 bit.");
					} */ else if (settingscursor == 2) {
						printSmall(false, 4, 156, "Displays some text before");
						printSmall(false, 4, 164, "launched game.");
					} else if (settingscursor == 3) {
						// printSmall(false, 4, 156, "Locks the ARM9 SCFG_EXT,");
						// printSmall(false, 4, 164, "avoiding conflict with");
						// printSmall(false, 4, 172, "recent libnds.");
						printSmall(false, 4, 156, "Sets LED as ROM read indicator.");
						printSmall(false, 4, 164, "If on, Camera LED will be");
						printSmall(false, 4, 172, "used as async prefetch indicator.");
					} else if (settingscursor == 4) {
						printSmall(false, 4, 156, "Enable, disable, or force use of");
						printSmall(false, 4, 164, "donor ROM.");
					} else if (settingscursor == 5) {
						printSmall(false, 4, 156, "Are you using a DSi");
						printSmall(false, 4, 164, "or a 3DS/2DS?");
					} else if (settingscursor == 6) {
						if(is3DS){
							printSmall(false, 4, 156, "Pick release or unofficial");
							printSmall(false, 4, 164, "bootstrap.");
						} else {
							printSmall(false, 4, 156, "If launched from DSi Menu via");
							printSmall(false, 4, 164, "HiyaCFW, disable this option.");
						}
					} /* else if (settingscursor == 8) {
						printSmall(false, 4, 148, "If you have Zelda Four Swords");
						printSmall(false, 4, 156, "with 4swordshax installed,");
						printSmall(false, 4, 164, "press POWER while playing a ROM");
						printSmall(false, 4, 172, "to return to the SRLoader menu.");
					} */



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
							subscreenmode = 3;
							menuprinted = false;
							break;
						case 1:
							bstrap_boostcpu = !bstrap_boostcpu;
							menuprinted = false;
							break;
						case 2:
							bstrap_debug = !bstrap_debug;
							menuprinted = false;
							break;
						case 3:
							// bstrap_lockARM9scfgext = !bstrap_lockARM9scfgext;
							if (pressed & KEY_LEFT) {
								bstrap_romreadled -= 1;
								if (bstrap_romreadled < 0) bstrap_romreadled = 2;
							} else if ((pressed & KEY_RIGHT) || (pressed & KEY_A)) {
								bstrap_romreadled += 1;
								if (bstrap_romreadled > 2) bstrap_romreadled = 0;
							}
							menuprinted = false;
							break;
						case 4:
							if (pressed & KEY_LEFT) {
								bstrap_useArm7Donor -= 0;
								if (bstrap_useArm7Donor < 0) bstrap_useArm7Donor = 2;
							} else if ((pressed & KEY_RIGHT) || (pressed & KEY_A)) {
								bstrap_useArm7Donor += 1;
								if (bstrap_useArm7Donor > 2) bstrap_useArm7Donor = 0;
							}
							menuprinted = false;
							break;
						case 5:
							is3DS = !is3DS;
							menuprinted = false;
							break;
						case 6:
							if(is3DS){
								bootstrapFile = !bootstrapFile;
							} else {
								ntr_touch = !ntr_touch;
							}
							menuprinted = false;
							break;
					}
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
				
				if(!arm7SCFGLocked) {
					if (settingscursor > 6) settingscursor = 0;
					else if (settingscursor < 0) settingscursor = 6;
				} else {
					if (settingscursor > 4) settingscursor = 0;
					else if (settingscursor < 0) settingscursor = 4;
				}
			} else {
				pressed = 0;

				if (!menuprinted) {
					// Clear the screen so it doesn't over-print
					clearText();
					
					printSmall(true, 1, 2, username);
					printSmall(true, 1, 184, lrswitchpages);
					printSmall(true, 192, 184, vertext);
					
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
						
					if(arm7SCFGLocked) {
						printSmall(false, 12, 64, "Reboot into RocketLauncher");
						
						if(rebootInRocketLauncher)
							printSmall(false, 224, 64, "On");
						else
							printSmall(false, 224, 64, "Off");
					}


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
					} else if (settingscursor == 3) {
						printSmall(false, 4, 132, "Reboots with unlocked ARM7 SCFG,");
						printSmall(false, 4, 140, "for sound to play correctly in");
						printSmall(false, 4, 148, "nds-bootstrap. Requires a Slot-1");
						printSmall(false, 4, 156, "card in the slot, and it's TID");
						printSmall(false, 4, 164, "with the exploit in the whitelist.");
						printSmall(false, 4, 172, "Press A to reboot in this mode.");
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
								subtheme = 0;
								theme -= 1;
								if (theme < 0) theme = 1;
							} else if (pressed & KEY_RIGHT) {
								subtheme = 0;
								theme += 1;
								if (theme > 1) theme = 0;
							} else
								subscreenmode = 2;
							menuprinted = false;
							break;
						case 1:
							autorun = !autorun;
							menuprinted = false;
							break;
						case 2:
							showlogo = !showlogo;
							menuprinted = false;
							break;
						case 3:
							if (pressed & (KEY_LEFT | KEY_RIGHT)) {
								rebootInRocketLauncher = !rebootInRocketLauncher;
							} else {
								clearText();
								printSmall(false, 4, 4, "Saving settings...");
								SaveSettings();
								RocketLauncher();
							}
							menuprinted = false;
							break;
					}
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
					if (!arm7SCFGLocked) {
						if (is3DS) {
							if (bootstrapFile) bootstrapfilename = "sd:/_nds/unofficial-bootstrap.nds";
							else bootstrapfilename = "sd:/_nds/release-bootstrap.nds";
						} else {
							bootstrapfilename = "sd:/_nds/rocket-bootstrap.nds";
						}
					} else {
						bootstrapfilename = "sd:/_nds/dsiware-bootstrap.nds";
					}
					int err = runNdsFile (bootstrapfilename.c_str(), 0, 0);
					iprintf ("Start failed. Error %i\n", err);
				}
				
				if (pressed & KEY_B) {
					clearText();
					printSmall(false, 4, 4, "Saving settings...");
					SaveSettings();
					loadROMselect();
					break;
				}
				
				if(!arm7SCFGLocked) {
					if (settingscursor > 2) settingscursor = 0;
					else if (settingscursor < 0) settingscursor = 2;
				} else {
					if (settingscursor > 3) settingscursor = 0;
					else if (settingscursor < 0) settingscursor = 3;
				}
			}

		} else {
			loadROMselect();
		}

	}

	return 0;
}
