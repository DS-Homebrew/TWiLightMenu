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

#include "fileBrowse.h"
#include <vector>
#include <algorithm>
#include <unistd.h>
#include <string>
#include <sstream>
#include <stdio.h>
#include <dirent.h>

#include <nds.h>
#include <maxmod9.h>
#include "common/gl2d.h"

#include "date.h"

#include "ndsheaderbanner.h"
#include "flashcard.h"
#include "iconTitle.h"
#include "graphics/fontHandler.h"
#include "graphics/graphics.h"
#include "graphics/FontGraphic.h"
#include "graphics/TextPane.h"
#include "SwitchState.h"
#include "perGameSettings.h"

#include "gbaswitch.h"
#include "nds_loader_arm9.h"

#include "inifile.h"

#include "soundbank.h"
#include "soundbank_bin.h"

#define SCREEN_COLS 32
#define ENTRIES_PER_SCREEN 22
#define ENTRIES_START_ROW 2
#define ENTRY_PAGE_LENGTH 10

extern bool whiteScreen;
extern bool fadeType;
extern bool fadeSpeed;

extern bool startButtonLaunch;
extern bool homebrewBootstrap;
extern bool useGbarunner;
extern int consoleModel;
extern bool isRegularDS;

extern bool showdialogbox;
extern int dialogboxHeight;

extern std::string romfolder[2];

extern std::string arm7DonorPath;
bool donorFound = true;

extern bool applaunch;

extern bool gotosettings;

extern bool useBootstrap;

using namespace std;

extern bool startMenu;

extern int theme;

extern bool showDirectories;
extern int spawnedtitleboxes;
extern int cursorPosition[2];
extern int startMenu_cursorPosition;
extern int pagenum[2];

bool settingsChanged = false;

extern void SaveSettings();

extern std::string ReplaceAll(std::string str, const std::string& from, const std::string& to);

struct DirEntry {
	string name;
	bool isDirectory;
} ;

bool nameEndsWith (const string& name, const vector<string> extensionList) {

	if (name.size() == 0) return false;

	if (extensionList.size() == 0) return true;

	for (int i = 0; i < (int)extensionList.size(); i++) {
		const string ext = extensionList.at(i);
		if ( strcasecmp (name.c_str() + name.size() - ext.size(), ext.c_str()) == 0) return true;
	}
	return false;
}

bool dirEntryPredicate(const DirEntry& lhs, const DirEntry& rhs)
{

	if (!lhs.isDirectory && rhs.isDirectory) {
		return false;
	}
	if (lhs.isDirectory && !rhs.isDirectory) {
		return true;
	}
	return strcasecmp(lhs.name.c_str(), rhs.name.c_str()) < 0;
}

void getDirectoryContents(vector<DirEntry>& dirContents, const vector<string> extensionList)
{
	struct stat st;

	dirContents.clear();

	DIR *pdir = opendir ("."); 
	
	if (pdir == NULL) {
		iprintf ("Unable to open the directory.\n");
	} else {

		while(true) {
			DirEntry dirEntry;

			struct dirent* pent = readdir(pdir);
			if(pent == NULL) break;
				
			stat(pent->d_name, &st);
			dirEntry.name = pent->d_name;
			dirEntry.isDirectory = (st.st_mode & S_IFDIR) ? true : false;

			if (showDirectories) {
				if (dirEntry.name.compare(".") != 0 && dirEntry.name.compare("_nds") != 0 && (dirEntry.isDirectory || nameEndsWith(dirEntry.name, extensionList))) {
					dirContents.push_back (dirEntry);
				}
			} else {
				if (dirEntry.name.compare(".") != 0 && (nameEndsWith(dirEntry.name, extensionList))) {
					dirContents.push_back (dirEntry);
				}
			}

		}

		closedir(pdir);
	}

	sort(dirContents.begin(), dirContents.end(), dirEntryPredicate);
}

void getDirectoryContents(vector<DirEntry>& dirContents)
{
	vector<string> extensionList;
	getDirectoryContents(dirContents, extensionList);
}

void showDirectoryContents (const vector<DirEntry>& dirContents, int startRow) {
	char path[PATH_MAX];
	
	
	getcwd(path, PATH_MAX);
	
	// Clear the screen
	iprintf ("\x1b[2J");
	
	// Print the path
	if (strlen(path) < SCREEN_COLS) {
		iprintf ("%s", path);
	} else {
		iprintf ("%s", path + strlen(path) - SCREEN_COLS);
	}
	
	// Move to 2nd row
	iprintf ("\x1b[1;0H");
	// Print line of dashes
	iprintf ("--------------------------------");
	
	// Print directory listing
	for (int i = 0; i < ((int)dirContents.size() - startRow) && i < ENTRIES_PER_SCREEN; i++) {
		const DirEntry* entry = &dirContents.at(i + startRow);
		char entryName[SCREEN_COLS + 1];
		
		// Set row
		iprintf ("\x1b[%d;0H", i + ENTRIES_START_ROW);
		
		if (entry->isDirectory) {
			strncpy (entryName, entry->name.c_str(), SCREEN_COLS);
			entryName[SCREEN_COLS - 3] = '\0';
			iprintf (" [%s]", entryName);
		} else {
			strncpy (entryName, entry->name.c_str(), SCREEN_COLS);
			entryName[SCREEN_COLS - 1] = '\0';
			iprintf (" %s", entryName);
		}
	}
}

string browseForFile(const vector<string> extensionList, const char* username)
{
	int pressed = 0;
	int screenOffset = 0;
	int fileOffset = 0;
	vector<DirEntry> dirContents;
	
	getDirectoryContents (dirContents, extensionList);
	showDirectoryContents (dirContents, screenOffset);
	
	whiteScreen = false;
	fadeType = true;	// Fade in from white

	fileOffset = cursorPosition[secondaryDevice];
	if (pagenum[secondaryDevice] > 0) {
		fileOffset += pagenum[secondaryDevice]*40;
	}

	while (true)
	{
		if (fileOffset < 0) 	fileOffset = dirContents.size() - 1;		// Wrap around to bottom of list
		if (fileOffset > ((int)dirContents.size() - 1))		fileOffset = 0;		// Wrap around to top of list

		// Clear old cursors
		for (int i = ENTRIES_START_ROW; i < ENTRIES_PER_SCREEN + ENTRIES_START_ROW; i++) {
			iprintf ("\x1b[%d;0H ", i);
		}
		// Show cursor
		iprintf ("\x1b[%d;0H*", fileOffset - screenOffset + ENTRIES_START_ROW);

		if (dirContents.at(fileOffset).isDirectory) {
			isDirectory = true;
			bnrWirelessIcon = 0;
		} else {
			isDirectory = false;
			std::string std_romsel_filename = dirContents.at(fileOffset).name.c_str();
			if((std_romsel_filename.substr(std_romsel_filename.find_last_of(".") + 1) == "nds")
			|| (std_romsel_filename.substr(std_romsel_filename.find_last_of(".") + 1) == "NDS")
			|| (std_romsel_filename.substr(std_romsel_filename.find_last_of(".") + 1) == "dsi")
			|| (std_romsel_filename.substr(std_romsel_filename.find_last_of(".") + 1) == "DSI")
			|| (std_romsel_filename.substr(std_romsel_filename.find_last_of(".") + 1) == "ids")
			|| (std_romsel_filename.substr(std_romsel_filename.find_last_of(".") + 1) == "IDS")
			|| (std_romsel_filename.substr(std_romsel_filename.find_last_of(".") + 1) == "app")
			|| (std_romsel_filename.substr(std_romsel_filename.find_last_of(".") + 1) == "APP")
			|| (std_romsel_filename.substr(std_romsel_filename.find_last_of(".") + 1) == "argv")
			|| (std_romsel_filename.substr(std_romsel_filename.find_last_of(".") + 1) == "ARGV")
			|| (std_romsel_filename.substr(std_romsel_filename.find_last_of(".") + 1) == "launcharg")
			|| (std_romsel_filename.substr(std_romsel_filename.find_last_of(".") + 1) == "LAUNCHARG"))
			{
				getGameInfo(isDirectory, dirContents.at(fileOffset).name.c_str());
				bnrRomType = 0;
			} else if((std_romsel_filename.substr(std_romsel_filename.find_last_of(".") + 1) == "gb")
					|| (std_romsel_filename.substr(std_romsel_filename.find_last_of(".") + 1) == "GB")
					|| (std_romsel_filename.substr(std_romsel_filename.find_last_of(".") + 1) == "sgb")
					|| (std_romsel_filename.substr(std_romsel_filename.find_last_of(".") + 1) == "SGB"))
			{
				bnrRomType = 1;
				bnrWirelessIcon = 0;
				isDSiWare = false;
				isHomebrew = 0;
			} else if((std_romsel_filename.substr(std_romsel_filename.find_last_of(".") + 1) == "gbc")
					|| (std_romsel_filename.substr(std_romsel_filename.find_last_of(".") + 1) == "GBC"))
			{
				bnrRomType = 2;
				bnrWirelessIcon = 0;
				isDSiWare = false;
				isHomebrew = 0;
			} else if((std_romsel_filename.substr(std_romsel_filename.find_last_of(".") + 1) == "nes")
					|| (std_romsel_filename.substr(std_romsel_filename.find_last_of(".") + 1) == "NES")
					|| (std_romsel_filename.substr(std_romsel_filename.find_last_of(".") + 1) == "fds")
					|| (std_romsel_filename.substr(std_romsel_filename.find_last_of(".") + 1) == "FDS"))
			{
				bnrRomType = 3;
				bnrWirelessIcon = 0;
				isDSiWare = false;
				isHomebrew = 0;
			}
		}

		if (bnrRomType==0) iconUpdate (dirContents.at(fileOffset).isDirectory,dirContents.at(fileOffset).name.c_str());
		titleUpdate (dirContents.at(fileOffset).isDirectory,dirContents.at(fileOffset).name.c_str());

		if (!isRegularDS) {
			printSmall(false, 8, 168, "Location:");
			if (secondaryDevice) {
				printSmall(false, 8, 176, "Slot-1 microSD Card");
			} else if (consoleModel < 3) {
				printSmall(false, 8, 176, "SD Card");
			} else {
				printSmall(false, 8, 176, "microSD Card");
			}
		}

		// Power saving loop. Only poll the keys once per frame and sleep the CPU if there is nothing else to do
		do {
			scanKeys();
			pressed = keysDownRepeat();
			swiWaitForVBlank();
		} while (!pressed);

		if (pressed & KEY_UP) {
			fileOffset -= 1;
			settingsChanged = true;
		}
		if (pressed & KEY_DOWN) {
			fileOffset += 1;
			settingsChanged = true;
		}
		if (pressed & KEY_LEFT) {
			fileOffset -= ENTRY_PAGE_LENGTH;
			settingsChanged = true;
		}
		if (pressed & KEY_RIGHT) {
			fileOffset += ENTRY_PAGE_LENGTH;
			settingsChanged = true;
		}

		if (fileOffset < 0) 	fileOffset = dirContents.size() - 1;		// Wrap around to bottom of list
		if (fileOffset > ((int)dirContents.size() - 1))		fileOffset = 0;		// Wrap around to top of list

		// Scroll screen if needed
		if (fileOffset < screenOffset) 	{
			screenOffset = fileOffset;
			showDirectoryContents (dirContents, screenOffset);
		}
		if (fileOffset > screenOffset + ENTRIES_PER_SCREEN - 1) {
			screenOffset = fileOffset - ENTRIES_PER_SCREEN + 1;
			showDirectoryContents (dirContents, screenOffset);
		}

		int pressedToLaunch = 0;
		if (startButtonLaunch) {
			pressedToLaunch = (pressed & KEY_START);
		} else {
			pressedToLaunch = (pressed & KEY_A);
		}

		if (pressedToLaunch) {
			DirEntry* entry = &dirContents.at(fileOffset);
			if (entry->isDirectory) {
				iprintf("Entering directory\n");
				// Enter selected directory
				chdir (entry->name.c_str());
				char buf[256];
				romfolder[secondaryDevice] = getcwd(buf, 256);
				cursorPosition[secondaryDevice] = 0;
				pagenum[secondaryDevice] = 0;
				SaveSettings();
				settingsChanged = false;
				return "null";
			}
			else if ((isDSiWare && !isDSiMode())
					|| (isDSiWare && !sdFound())
					|| (isDSiWare && consoleModel > 1))
			{
				showdialogbox = true;
				printLargeCentered(false, 84, "Error!");
				printSmallCentered(false, 104, "This game cannot be launched.");
				printSmallCentered(false, 118, "A: OK");
				for (int i = 0; i < 30; i++) swiIntrWait(0, 1);
				pressed = 0;
				do {
					scanKeys();
					pressed = keysDownRepeat();
					swiWaitForVBlank();
				} while (!(pressed & KEY_A));
				showdialogbox = false;
			} else {
				bool hasAP = false;
				if (!secondaryDevice && bnrRomType == 0 && !isDSiWare && isHomebrew == 0)
				{
					FILE *f_nds_file = fopen(dirContents.at(fileOffset).name.c_str(), "rb");
					hasAP = checkRomAP(f_nds_file);
					fclose(f_nds_file);
				}
				if (hasAP) {
				dialogboxHeight = 5;
				showdialogbox = true;
				printLargeCentered(false, 84, "Warning");
				printSmallCentered(false, 104, "This game may not work right,");
				printSmallCentered(false, 112, "if it's not AP-patched.");
				printSmallCentered(false, 128, "If the game freezes, does not");
				printSmallCentered(false, 136, "start, or doesn't seem normal,");
				printSmallCentered(false, 144, "it needs to be AP-patched.");
				printSmallCentered(false, 158, "A: OK");
				for (int i = 0; i < 30; i++) swiIntrWait(0, 1);
				pressed = 0;
				do {
					scanKeys();
					pressed = keysDownRepeat();
					swiWaitForVBlank();
				} while (!(pressed & KEY_A));
				clearText();
				showdialogbox = false;
				dialogboxHeight = 0;
				titleUpdate (dirContents.at(fileOffset).isDirectory,dirContents.at(fileOffset).name.c_str());
				if (!isRegularDS) {
					printSmall(false, 8, 168, "Location:");
					if (secondaryDevice) {
						printSmall(false, 8, 176, "Slot-1 microSD Card");
					} else if (consoleModel < 3) {
						printSmall(false, 8, 176, "SD Card");
					} else {
						printSmall(false, 8, 176, "microSD Card");
					}
				}
				}

				applaunch = true;

				fadeType = false;	// Fade to white
				for (int i = 0; i < 25; i++) {
					swiWaitForVBlank();
				}
				cursorPosition[secondaryDevice] = fileOffset;
				pagenum[secondaryDevice] = 0;
				for (int i = 0; i < 100; i++) {
					if (cursorPosition[secondaryDevice] > 39) {
						cursorPosition[secondaryDevice] -= 40;
						pagenum[secondaryDevice]++;
					} else {
						break;
					}
				}

				// Return the chosen file
				return entry->name;
			}
		}

		if ((pressed & KEY_R) && bothSDandFlashcard())
		{
			consoleClear();
			printf("Please wait...\n");
			cursorPosition[secondaryDevice] = fileOffset;
			pagenum[secondaryDevice] = 0;
			for (int i = 0; i < 100; i++) {
				if (cursorPosition[secondaryDevice] > 39) {
					cursorPosition[secondaryDevice] -= 40;
					pagenum[secondaryDevice]++;
				} else {
					break;
				}
			}
			secondaryDevice = !secondaryDevice;
			SaveSettings();
			settingsChanged = false;
			return "null";		
		}

		if ((pressed & KEY_B) && showDirectories) {
			// Go up a directory
			chdir ("..");
			char buf[256];
			romfolder[secondaryDevice] = getcwd(buf, 256);
			cursorPosition[secondaryDevice] = 0;
			SaveSettings();
			settingsChanged = false;
			return "null";		
		}

		if ((pressed & KEY_X)
		&& strcmp(dirContents.at(fileOffset).name.c_str(), "..") != 0 && !isDirectory)
		{
			showdialogbox = true;
			printLargeCentered(false, 84, "Warning!");
			//if (isDirectory) {
			//	printSmallCentered(false, 104, "Delete this folder?");
			//} else {
				printSmallCentered(false, 104, "Delete this game?");
			//}
			for (int i = 0; i < 90; i++) swiIntrWait(0, 1);
			printSmallCentered(false, 118, "A: Yes  B: No");
			while (1) {
				do {
					scanKeys();
					pressed = keysDownRepeat();
					swiWaitForVBlank();
				} while (!pressed);
				
				if (pressed & KEY_A) {
					clearText();
					showdialogbox = false;
					consoleClear();
					printf("Please wait...\n");
					remove(dirContents.at(fileOffset).name.c_str()); // Remove game/folder
					if (settingsChanged) {
						cursorPosition[secondaryDevice] = fileOffset;
						pagenum[secondaryDevice] = 0;
						for (int i = 0; i < 100; i++) {
							if (cursorPosition[secondaryDevice] > 39) {
								cursorPosition[secondaryDevice] -= 40;
								pagenum[secondaryDevice]++;
							} else {
								break;
							}
						}
					}
					SaveSettings();
					settingsChanged = false;
					return "null";
				}

				if (pressed & KEY_B) {
					break;
				}
			}
			clearText();
			showdialogbox = false;
		}

		int pressedForStartMenu = 0;
		if (startButtonLaunch) {
			pressedForStartMenu = (pressed & KEY_SELECT);
		} else {
			pressedForStartMenu = (pressed & KEY_START);
		}

		if (pressedForStartMenu)
		{
			if (settingsChanged) {
				cursorPosition[secondaryDevice] = fileOffset;
				pagenum[secondaryDevice] = 0;
				for (int i = 0; i < 100; i++) {
					if (cursorPosition[secondaryDevice] > 39) {
						cursorPosition[secondaryDevice] -= 40;
						pagenum[secondaryDevice]++;
					} else {
						break;
					}
				}
			}
			SaveSettings();
			settingsChanged = false;
			consoleClear();
			clearText();
			startMenu = true;
			return "null";		
		}

		int pressedForPerGameSettings = 0;
		if (startButtonLaunch) {
			pressedForPerGameSettings = (pressed & KEY_A);
		} else {
			pressedForPerGameSettings = (pressed & KEY_SELECT);
		}

		if (pressedForPerGameSettings && (isDirectory == false) && (bnrRomType == 0))
		{
			cursorPosition[secondaryDevice] = fileOffset;
			perGameSettings(dirContents.at(fileOffset).name);
		}

	}
}
