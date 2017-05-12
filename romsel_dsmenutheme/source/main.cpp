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
	
	graphicsInit();
	fontInit();

	iconTitleInit();
	
	keysSetRepeat(25,5);

	vector<string> extensionList;
	extensionList.push_back(".nds");
	extensionList.push_back(".argv");
	srand(time(NULL));

	while(1) {
	
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
	}

	return 0;
}
