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
#include "fileBrowse.h"

#include "dsmenu_top.h"

#include "iconTitle.h"
#include "graphics/fontHandler.h"

#include "inifile.h"

bool dodrawIcon = false;

const char* settingsinipath = "sd:/_nds/srloader/settings.ini";

bool autorun = false;
int theme = 0;

void LoadSettings(void) {
	CIniFile settingsini( settingsinipath );

	// Customizable UI settings.
	autorun = settingsini.GetInt("SRLOADER", "AUTORUNGAME", 0);
	theme = settingsini.GetInt("SRLOADER", "THEME", 0);
}

void SaveSettings(void) {
	CIniFile settingsini( settingsinipath );

	// UI settings.
	settingsini.SetInt("SRLOADER", "AUTORUNGAME", autorun);
	settingsini.SetInt("SRLOADER", "THEME", theme);
	settingsini.SaveIniFile(settingsinipath);
}

int screenmode = 0;
int subscreenmode = 0;

int menucursor = 0;
int settingscursor = 0;

bool useBootstrap = false;

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
		swiWaitForVBlank();
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

//---------------------------------------------------------------------------------
int main(int argc, char **argv) {
//---------------------------------------------------------------------------------

	// overwrite reboot stub identifier
	extern u64 *fake_heap_end;
	*fake_heap_end = 0;

	defaultExceptionHandler();

#ifndef EMULATE_FILES

	if (!fatInitDefault()) {
		graphicsInit();
		fontInit();
		//iprintf ("fatinitDefault failed!\n");
		printSmall(false, 4, 4, "fatinitDefault failed!");
		stop();
	}

#endif

	std::string filename;
	std::string bootstrapfilename;

	LoadSettings();
	
	swiWaitForVBlank();
	scanKeys();

	if (autorun && !(keysHeld() & KEY_B)) {
		CIniFile bootstrapini( "sd:/_nds/nds-bootstrap.ini" );
		bootstrapfilename = bootstrapini.GetString("NDS-BOOTSTRAP", "BOOTSTRAP_PATH","");
		bootstrapfilename = ReplaceAll( bootstrapfilename, "fat:/", "sd:/");
		runNdsFile (bootstrapfilename.c_str(), 0, 0);
	}

	graphicsInit();
	fontInit();

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
		doPause(80, 140);
	}

	loadThemegraphics();

	iconTitleInit();
	/*while (1)
	{
		//swiWaitForVBlank();
		while (REG_DISPCAPCNT & DCAP_ENABLE);
		startRendering(true);
		glBegin2D();
		{
			glBoxFilledGradient(0, 0, 255, 191, RGB15(25, 0, 0), RGB15(23, 20, 0), RGB15(25, 0, 0), RGB15(23, 20, 0));
			drawIcon();
		}
		glEnd2D();
		glFlush(0);
		swiWaitForVBlank();
	}*/
	// fontInit();
		
	dodrawIcon = true;
	
	keysSetRepeat(25,5);

	vector<string> extensionList;
	extensionList.push_back(".nds");
	extensionList.push_back(".argv");
	srand(time(NULL));

	while(1) {
	
		if (screenmode == 0) {
			//Navigates to the file to launch
			filename = browseForFile(extensionList);

			////////////////////////////////////
			// Launch the item
#ifndef EMULATE_FILES
			// Construct a command line
			getcwd (filePath, PATH_MAX);
			int pathLen = strlen(filePath);
			vector<char*> argarray;

			if ( strcasecmp (filename.c_str() + filename.size() - 5, ".argv") == 0) {

				FILE *argfile = fopen(filename.c_str(),"rb");
				char str[PATH_MAX], *pstr;
				const char seps[]= "\n\r\t ";

				while( fgets(str, PATH_MAX, argfile) ) {
					// Find comment and end string there
					if( (pstr = strchr(str, '#')) )
						*pstr= '\0';

					// Tokenize arguments
					pstr= strtok(str, seps);

					while( pstr != NULL ) {
						argarray.push_back(strdup(pstr));
						pstr= strtok(NULL, seps);
					}
				}
				fclose(argfile);
				filename = argarray.at(0);
			} else {
				argarray.push_back(strdup(filename.c_str()));
			}

			if ( strcasecmp (filename.c_str() + filename.size() - 4, ".nds") != 0 || argarray.size() == 0 ) {
				// iprintf("no nds file specified\n");
			} else {
				char *name = argarray.at(0);
				strcpy (filePath + pathLen, name);
				free(argarray.at(0));
				argarray.at(0) = filePath;
				if (useBootstrap) {
					std::string savename = ReplaceAll(argarray[0], ".nds", ".sav");

					if (access(savename.c_str(), F_OK)) {
						// iprintf ("Creating save file...\n");
						printSmall(false, 4, 4, "Creating save file...");

						static const int BUFFER_SIZE = 4096;
						char buffer[BUFFER_SIZE];
						memset(buffer, 0, sizeof(buffer));

						int savesize = 524288;	// 512KB (default size for most games)

						FILE *pFile = fopen(savename.c_str(), "wb");
						if (pFile) {
							for (int i = savesize; i > 0; i -= BUFFER_SIZE) {
								fwrite(buffer, 1, sizeof(buffer), pFile);
							}
							fclose(pFile);
						}
						// iprintf ("Save file created!\n");
						printSmall(false, 4, 12, "Save file created!");

					}

					std::string path = argarray[0];
					std::string savepath = savename;
					CIniFile bootstrapini( "sd:/_nds/nds-bootstrap.ini" );
					path = ReplaceAll( path, "sd:/", "fat:/");
					savepath = ReplaceAll( savepath, "sd:/", "fat:/");
					bootstrapini.SetString("NDS-BOOTSTRAP", "NDS_PATH", path);
					bootstrapini.SetString("NDS-BOOTSTRAP", "SAV_PATH", savepath);
					bootstrapini.SaveIniFile( "sd:/_nds/nds-bootstrap.ini" );
					bootstrapfilename = bootstrapini.GetString("NDS-BOOTSTRAP", "BOOTSTRAP_PATH","");
					bootstrapfilename = ReplaceAll( bootstrapfilename, "fat:/", "sd:/");
					int err = runNdsFile (bootstrapfilename.c_str(), 0, 0);
					iprintf ("Start failed. Error %i\n", err);
				} else {
					iprintf ("Running %s with %d parameters\n", argarray[0], argarray.size());
					int err = runNdsFile (argarray[0], argarray.size(), (const char **)&argarray[0]);
					iprintf ("Start failed. Error %i\n", err);
				}

			}

			while(argarray.size() !=0 ) {
				free(argarray.at(0));
				argarray.erase(argarray.begin());
			}

			// while (1) {
			// 	swiWaitForVBlank();
			// 	scanKeys();
			// 	if (!(keysHeld() & KEY_A)) break;
			// }
#endif
		} else if (screenmode == 1) {
			
			bool selectmode = true;
			bool menuprinted = false;
			
			while(selectmode) {
				int pressed = 0;

				if (!menuprinted) {
					// Clear the screen so it doesn't over-print
					clearText();
					
					if(subscreenmode == 0) {
						printLarge(false, 4, 4, "SRLoader v0.0.2");
						
						int yPos;
						switch (menucursor) {
							case 0:
							default:
								yPos = 20;
								break;
							case 1:
								yPos = 28;
								break;
							case 2:
								yPos = 36;
								break;
						}
							
						printSmall(false, 4, yPos, ">");
						
						printSmall(false, 12, 20, "Return to ROM select");
						printSmall(false, 12, 28, "Run last played ROM");
						printSmall(false, 12, 36, "Settings");
							
					} else if(subscreenmode == 1) {
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
							case 2:
								yPos = 44;
								break;
						}
						
						printSmall(false, 4, yPos, ">");

						printSmall(false, 12, 20, "Theme");
						if(theme == 1)
							printSmall(false, 156, 20, "3DS HOME Menu");
						else
							printSmall(false, 156, 20, "DS Menu");
						printSmall(false, 12, 28, "Run last played ROM on startup ");
						if(autorun)
							printSmall(false, 224, 36, "On");
						else
							printSmall(false, 224, 36, "Off");
						printSmall(false, 12, 44, "Back");
						
						if (settingscursor == 0) {
							printSmall(false, 4, 156, "Changes top screen theme.");
						} else if (settingscursor == 1) {
							printSmall(false, 4, 156, "If turned on, hold B on\n");
							printSmall(false, 4, 164, "startup to skip to the\n");
							printSmall(false, 4, 172, "ROM select menu.");
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
				
				if(subscreenmode == 0) {
					if (pressed & KEY_UP) {
						menucursor -= 1;
						menuprinted = false;
					}
					if (pressed & KEY_DOWN) {
						menucursor += 1;
						menuprinted = false;
					}
					
					if (pressed & KEY_A) {
						switch (menucursor) {
							case 0:
							default:
								screenmode = 0;
								selectmode = false;
								break;
							case 1:
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
							case 2:
								subscreenmode = 1;
								menuprinted = false;
								break;
						}
					}

					if (pressed & KEY_B) {
						menucursor = 0;
						screenmode = 0;
						selectmode = false;
						break;
					}
					
					if (menucursor > 2) menucursor = 0;
					else if (menucursor < 0) menucursor = 2;
				} else if(subscreenmode == 1) {
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
								if (theme > 1) theme = 0;
								loadThemegraphics();
								menuprinted = false;
								break;
							case 1:
								autorun = !autorun;
								menuprinted = false;
								break;
							case 2:
								clearText();
								printSmall(false, 4, 4, "Saving settings...");
								SaveSettings();
								subscreenmode = 0;
								menuprinted = false;
								break;
						}
					}

					if (pressed & KEY_B) {
						clearText();
						printSmall(false, 4, 4, "Saving settings...");
						SaveSettings();
						subscreenmode = 0;
						menuprinted = false;
						break;
					}

					if (settingscursor > 2) settingscursor = 0;
					else if (settingscursor < 0) settingscursor = 2;
					
				}
				
			}

		}
	}

	return 0;
}
