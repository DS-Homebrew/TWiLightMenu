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

bool gotosettings = false;

bool autorun = false;
int theme = 0;

void LoadSettings(void) {
	CIniFile settingsini( settingsinipath );

	// Customizable UI settings.
	autorun = settingsini.GetInt("SRLOADER", "AUTORUNGAME", 0);
	gotosettings = settingsini.GetInt("SRLOADER", "GOTOSETTINGS", 0);
	theme = settingsini.GetInt("SRLOADER", "THEME", 0);
}

void SaveSettings(void) {
	CIniFile settingsini( settingsinipath );

	// UI settings.
	settingsini.SetInt("SRLOADER", "AUTORUNGAME", autorun);
	settingsini.SetInt("SRLOADER", "GOTOSETTINGS", gotosettings);
	settingsini.SetInt("SRLOADER", "THEME", theme);
	settingsini.SaveIniFile(settingsinipath);
}

int screenmode = 0;
// int subscreenmode = 0;

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

void loadROMselect() {
	if (theme == 0) runNdsFile ("sd:/_nds/srloader/dsimenu.srldr", 0, 0);
	else runNdsFile ("sd:/_nds/srloader/dsmenu.srldr", 0, 0);
}

//---------------------------------------------------------------------------------
int main(int argc, char **argv) {
//---------------------------------------------------------------------------------

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

	std::string filename;
	std::string bootstrapfilename;

	LoadSettings();
	
	swiWaitForVBlank();
	scanKeys();

	if (!gotosettings && autorun && !(keysHeld() & KEY_B)) {
		CIniFile bootstrapini( "sd:/_nds/nds-bootstrap.ini" );
		bootstrapfilename = bootstrapini.GetString("NDS-BOOTSTRAP", "BOOTSTRAP_PATH","");
		bootstrapfilename = ReplaceAll( bootstrapfilename, "fat:/", "sd:/");
		runNdsFile (bootstrapfilename.c_str(), 0, 0);
	}
	
	char vertext[12];
	// snprintf(vertext, sizeof(vertext), "Ver %d.%d.%d   ", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH); // Doesn't work :(
	snprintf(vertext, sizeof(vertext), "Ver %d.%d.%d   ", 1, 0, 1);

	graphicsInit();
	fontInit();

	if (gotosettings) {
		screenmode = 1;
		gotosettings = false;
		SaveSettings();
	} else {
		unsigned int * SCFG_ROM=	(unsigned int*)0x4004000;		
		unsigned int * SCFG_CLK=	(unsigned int*)0x4004004;
		unsigned int * SCFG_EXT=	(unsigned int*)0x4004008;
		unsigned int * SCFG_MC=		(unsigned int*)0x4004010;
		unsigned int * SCFG_ROM_ARM7_COPY=	(unsigned int*)0x2370000;
		unsigned int * SCFG_EXT_ARM7_COPY=  (unsigned int*)0x2370001;

		if(*SCFG_EXT>0) {
			char text1[48], text2[48], text3[48], text4[48], text5[48], text6[48]; 
			
			snprintf (text1, sizeof(text1), "DSI SCFG_ROM ARM9 : %x",*SCFG_ROM);			
			snprintf (text2, sizeof(text2), "DSI SCFG_CLK ARM9 : %x",*SCFG_CLK);
			snprintf (text3, sizeof(text3), "DSI SCFG_EXT ARM9 : %x",*SCFG_EXT);			
			snprintf (text4, sizeof(text4), "DSI SCFG_MC ARM9 : %x",*SCFG_MC);
			
			snprintf (text5, sizeof(text5), "DSI SCFG_ROM ARM7 : %x",*SCFG_ROM_ARM7_COPY);
			snprintf (text6, sizeof(text6), "DSI SCFG_EXT ARM7 : %x",*SCFG_EXT_ARM7_COPY);
			
			int yPos = 4;
			
			printSmall(false, 4, yPos, text1);
			yPos += 16;
			printSmall(false, 4, yPos, text2);
			yPos += 16;
			printSmall(false, 4, yPos, text3);
			yPos += 16;
			printSmall(false, 4, yPos, text4);
			yPos += 16;
			printSmall(false, 4, yPos, text5);
			yPos += 16;
			printSmall(false, 4, yPos, text6);
			
			//test RAM
			//unsigned int * TEST32RAM=	(unsigned int*)0xD004000;		
			//*TEST32RAM = 0x55;
			//iprintf ("32 MB RAM ACCESS : %x\n\n",*TEST32RAM);
			// doPause(80, 140);
		}
		
		for (int i = 0; i < 60*2; i++) {
			swiWaitForVBlank();
		}
		
		scanKeys();

		if (keysHeld() & KEY_START)
			screenmode = 1;

	}

	srand(time(NULL));

	while(1) {
	
		if (screenmode == 1) {
			
			bool selectmode = true;
			bool menuprinted = false;
			
			while(selectmode) {
				int pressed = 0;

				if (!menuprinted) {
					// Clear the screen so it doesn't over-print
					clearText();
					
					printSmall(true, 1, 2, username);
					printSmall(true, 192, 184, vertext);
					
					printLarge(false, 4, 4, "Settings");
					
					int yPos;
					switch (settingscursor) {
						case 0:
						default:
							yPos = 20;
							break;
						case 1:
							yPos = 28;
							break;
					}
					
					printSmall(false, 4, yPos, ">");
					
					printSmall(false, 12, 20, "Theme");
					if(theme == 2)
						printSmall(false, 156, 20, "3DS HOME Menu");
					else if(theme == 1)
						printSmall(false, 156, 20, "DS Menu");
					else
						printSmall(false, 156, 20, "DSi Menu");
					printSmall(false, 12, 28, "Run last played ROM on startup ");
					if(autorun)
						printSmall(false, 224, 36, "On");
					else
						printSmall(false, 224, 36, "Off");
						
					if (settingscursor == 0) {
						printSmall(false, 4, 156, "Select a theme to use.");
					} else if (settingscursor == 1) {
						printSmall(false, 4, 148, "If turned on, hold B on\n");
						printSmall(false, 4, 156, "startup to skip to the\n");
						printSmall(false, 4, 164, "ROM select menu.");
						printSmall(false, 4, 172, "Press Y to start last played ROM.");
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
					
				if (pressed & KEY_A) {
					switch (settingscursor) {
						case 0:
						default:
							theme += 1;
							if (theme > 2) theme = 0;
							menuprinted = false;
							break;
						case 1:
							autorun = !autorun;
							menuprinted = false;
							break;
					}
				}
				
				if (pressed & KEY_Y) {
					switch (settingscursor) {
						case 1:
						default:
						{
							screenmode = 0;
							selectmode = false;
							clearText();
							CIniFile bootstrapini( "sd:/_nds/nds-bootstrap.ini" );
							bootstrapfilename = bootstrapini.GetString("NDS-BOOTSTRAP", "BOOTSTRAP_PATH","");
							bootstrapfilename = ReplaceAll( bootstrapfilename, "fat:/", "sd:/");
							int err = runNdsFile (bootstrapfilename.c_str(), 0, 0);
							iprintf ("Start failed. Error %i\n", err);
							break;
						}
					}
				}
				
				if (pressed & KEY_B) {
					clearText();
					printSmall(false, 4, 4, "Saving settings...");
					SaveSettings();
					screenmode = 0;
					selectmode = false;
					break;
				}
				
				if (settingscursor > 1) settingscursor = 0;
				else if (settingscursor < 0) settingscursor = 1;
			}

		} else {
			loadROMselect();
		}

	}

	return 0;
}
