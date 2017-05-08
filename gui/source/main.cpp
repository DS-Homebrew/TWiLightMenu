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
#include "file_browse.h"

#include "dsmenu_top.h"

#include "iconTitle.h"

#include "inifile.h"

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
void doPause() {
//---------------------------------------------------------------------------------
	iprintf("Press start...\n");
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

	if (!fatInitDefault()) {
		bannerTitleInit();
		
		// Subscreen as a console
		videoSetModeSub(MODE_0_2D);
		vramSetBankH(VRAM_H_SUB_BG);
		consoleInit(NULL, 0, BgType_Text4bpp, BgSize_T_256x256, 15, 0, false, true);
		
		iprintf ("fatinitDefault failed!\n");
		
		unsigned int * SCFG_EXT=	(unsigned int*)0x4004008;
		if(*SCFG_EXT>0)
			iprintf ("SCFG_EXT : %x\n",*SCFG_EXT);
			
		stop();
	}

	int pathLen;
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

	bannerTitleInit();

	// Subscreen as a console
	videoSetModeSub(MODE_0_2D);
	vramSetBankH(VRAM_H_SUB_BG);
	consoleInit(NULL, 0, BgType_Text4bpp, BgSize_T_256x256, 15, 0, false, true);
	
	unsigned int * SCFG_ROM=	(unsigned int*)0x4004000;		
	unsigned int * SCFG_CLK=	(unsigned int*)0x4004004;
	unsigned int * SCFG_EXT=	(unsigned int*)0x4004008;
	unsigned int * SCFG_MC=		(unsigned int*)0x4004010;
	unsigned int * SCFG_ROM_ARM7_COPY=	(unsigned int*)0x2370000;
	unsigned int * SCFG_EXT_ARM7_COPY=  (unsigned int*)0x2370001;

	if(*SCFG_EXT>0) {
		iprintf ("DSI SCFG_ROM ARM9 : %x\n\n",*SCFG_ROM);			
		iprintf ("DSI SCFG_CLK ARM9 : %x\n\n",*SCFG_CLK);
		iprintf ("DSI SCFG_EXT ARM9 : %x\n\n",*SCFG_EXT);			
		iprintf ("DSI SCFG_MC ARM9 : %x\n\n",*SCFG_MC);
		
		iprintf ("DSI SCFG_ROM ARM7 : %x\n\n",*SCFG_ROM_ARM7_COPY);
		iprintf ("DSI SCFG_EXT ARM7 : %x\n\n",*SCFG_EXT_ARM7_COPY);
		
		//test RAM
		//unsigned int * TEST32RAM=	(unsigned int*)0xD004000;		
		//*TEST32RAM = 0x55;
		//iprintf ("32 MB RAM ACCESS : %x\n\n",*TEST32RAM);
	}
			
	doPause();

	// Clear the screen
	iprintf ("\x1b[2J");
	
	iconTitleInit();

	keysSetRepeat(25,5);

	vector<string> extensionList;
	extensionList.push_back(".nds");
	extensionList.push_back(".argv");

	while(1) {
	
		if (screenmode == 0) {

			filename = browseForFile(extensionList);

			// Construct a command line
			getcwd (filePath, PATH_MAX);
			pathLen = strlen (filePath);
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
						iprintf ("Creating save file...\n");

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
						iprintf ("Save file created!\n");

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
		} else if (screenmode == 1) {
			
			bool selectmode = true;
			bool menuprinted = false;
			
			while(selectmode) {
				int pressed = 0;

				if (!menuprinted) {
					// Clear the screen so it doesn't over-print
					iprintf ("\x1b[2J");
					
					if(subscreenmode == 0) {
						iprintf ("SRLoader v0.0.2\n");
						iprintf ("\n\n");
						
						if (menucursor == 0)
							iprintf ("*");
						else
							iprintf (" ");
						iprintf ("Return to ROM select\n");
							
						if (menucursor == 1)
							iprintf ("*");
						else
							iprintf (" ");
						iprintf ("Run last played ROM\n");
							
						if (menucursor == 2)
							iprintf ("*");
						else
							iprintf (" ");
						iprintf ("Settings\n");
							
					} else if(subscreenmode == 1) {
						iprintf ("Settings\n");
						iprintf ("\n\n");
						
						if (settingscursor == 0)
							iprintf ("*");
						else
							iprintf (" ");
						iprintf ("Change theme\n");
							
						if (settingscursor == 1)
							iprintf ("*");
						else
							iprintf (" ");
						iprintf ("Run last played ROM on startup ");
						iprintf ("                           ");
						if(autorun) iprintf ("On"); else iprintf ("Off");
						iprintf ("\n");

						if (settingscursor == 2)
							iprintf ("*");
						else
							iprintf (" ");
						iprintf ("Back");
						
						 if (settingscursor == 0) {
							iprintf ("\n\n\n\n\n\n\n\n\n\n\n\n");
							iprintf ("Changes top screen theme.");
						} else if (settingscursor == 1) {
							iprintf ("\n\n\n\n\n\n\n\n\n\n\n\n");
							iprintf ("If turned on, hold B on\n");
							iprintf ("startup to skip to the\n");
							iprintf ("ROM select menu.");
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
								iprintf ("\x1b[2J");
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
								iconTitleInit();
								break;
							case 1:
								autorun = !autorun;
								menuprinted = false;
								break;
							case 2:
								iprintf ("\x1b[2J");
								iprintf ("Saving settings...");
								SaveSettings();
								subscreenmode = 0;
								menuprinted = false;
								break;
						}
					}

					if (pressed & KEY_B) {
						iprintf ("\x1b[2J");
						iprintf ("Saving settings...");
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

