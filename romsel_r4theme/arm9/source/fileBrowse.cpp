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
#include <gl2d.h>

#include "date.h"

#include "ndsheaderbanner.h"
#include "iconTitle.h"
#include "graphics/fontHandler.h"
#include "graphics/graphics.h"
#include "graphics/FontGraphic.h"
#include "graphics/TextPane.h"
#include "SwitchState.h"

#include "gbaswitch.h"
#include "nds_loader_arm9.h"

#include "inifile.h"

#include "soundbank.h"
#include "soundbank_bin.h"

#define SCREEN_COLS 32
#define ENTRIES_PER_SCREEN 22
#define ENTRIES_START_ROW 2
#define ENTRY_PAGE_LENGTH 10

const char* SDKnumbertext;

extern bool whiteScreen;
extern bool fadeType;
extern bool fadeSpeed;

extern bool useGbarunner;
extern bool isRegularDS;

extern bool showdialogbox;

extern std::string romfolder;

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
extern bool dsiWareList;
extern int cursorPosition;
extern int dsiWare_cursorPosition;
extern int startMenu_cursorPosition;
extern int pagenum;
extern int dsiWarePageNum;
extern int titleboxXpos;
extern int titlewindowXpos;
extern int dsiWare_titleboxXpos;
extern int dsiWare_titlewindowXpos;

extern bool flashcardUsed;

bool settingsChanged = false;

extern void SaveSettings();

extern std::string ReplaceAll(std::string str, const std::string& from, const std::string& to);

extern void loadGameOnFlashcard(const char* filename);

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

			if (showDirectories && !dsiWareList) {
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

	if (dsiWareList) {
		fileOffset = dsiWare_cursorPosition;
		if (dsiWarePageNum > 0) {
			fileOffset += dsiWarePageNum*40;
		}
	} else {
		fileOffset = cursorPosition;
		if (pagenum > 0) {
			fileOffset += pagenum*40;
		}
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

		if (dsiWareList) {
			isDirectory = false;
			std::string std_romsel_filename = dirContents.at(fileOffset).name.c_str();
			if((std_romsel_filename.substr(std_romsel_filename.find_last_of(".") + 1) == "app")
			|| (std_romsel_filename.substr(std_romsel_filename.find_last_of(".") + 1) == "argv"))
			{
				getGameInfo(dirContents.at(fileOffset).isDirectory, dirContents.at(fileOffset).name.c_str());
				bnrRomType = 0;
			}
		} else {
			if (dirContents.at(fileOffset).isDirectory) {
				isDirectory = true;
				bnrWirelessIcon = 0;
			} else {
				isDirectory = false;
				std::string std_romsel_filename = dirContents.at(fileOffset).name.c_str();
				if((std_romsel_filename.substr(std_romsel_filename.find_last_of(".") + 1) == "nds")
				|| (std_romsel_filename.substr(std_romsel_filename.find_last_of(".") + 1) == "app")
				|| (std_romsel_filename.substr(std_romsel_filename.find_last_of(".") + 1) == "argv"))
				{
					getGameInfo(dirContents.at(fileOffset).isDirectory, dirContents.at(fileOffset).name.c_str());
					bnrRomType = 0;
				} else if((std_romsel_filename.substr(std_romsel_filename.find_last_of(".") + 1) == "gb")
						|| std_romsel_filename.substr(std_romsel_filename.find_last_of(".") + 1) == "sgb")
				{
					bnrRomType = 1;
					bnrWirelessIcon = 0;
					launchable = true;
					isHomebrew = false;
				} else if(std_romsel_filename.substr(std_romsel_filename.find_last_of(".") + 1) == "gbc") {
					bnrRomType = 2;
					bnrWirelessIcon = 0;
					launchable = true;
					isHomebrew = false;
				} else if((std_romsel_filename.substr(std_romsel_filename.find_last_of(".") + 1) == "nes")
						|| std_romsel_filename.substr(std_romsel_filename.find_last_of(".") + 1) == "fds")
				{
					bnrRomType = 3;
					bnrWirelessIcon = 0;
					launchable = true;
					isHomebrew = false;
				}
			}
		}

		if (bnrRomType==0) iconUpdate (dirContents.at(fileOffset).isDirectory,dirContents.at(fileOffset).name.c_str());
		titleUpdate (dirContents.at(fileOffset).isDirectory,dirContents.at(fileOffset).name.c_str());

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
		
		if (pressed & KEY_A) {
			DirEntry* entry = &dirContents.at(fileOffset);
			if (entry->isDirectory) {
				iprintf("Entering directory\n");
				// Enter selected directory
				chdir (entry->name.c_str());
				char buf[256];
				romfolder = getcwd(buf, 256);
				cursorPosition = 0;
				pagenum = 0;
				SaveSettings();
				settingsChanged = false;
				return "null";
			} else if (dsiWareList || launchable) {
				applaunch = true;
				if (!isHomebrew) {
					useBootstrap = true;
				} else {
					useBootstrap = false;
				}

				fadeType = false;	// Fade to white
				for (int i = 0; i < 25; i++) {
					swiWaitForVBlank();
				}
				if (dsiWareList) {
					dsiWare_cursorPosition = fileOffset;
					dsiWarePageNum = 0;
					for (int i = 0; i < 100; i++) {
						if (dsiWare_cursorPosition > 39) {
							dsiWare_cursorPosition -= 40;
							dsiWarePageNum++;
						} else {
							break;
						}
					}
				} else {
					cursorPosition = fileOffset;
					pagenum = 0;
					for (int i = 0; i < 100; i++) {
						if (cursorPosition > 39) {
							cursorPosition -= 40;
							pagenum++;
						} else {
							break;
						}
					}
				}
				SaveSettings();

				// Return the chosen file
				return entry->name;
			} else {
				showdialogbox = true;
				printLargeCentered(false, 84, "Error!");
				printSmallCentered(false, 104, "This game cannot be launched.");
				printSmallCentered(false, 118, "A: OK");
				for (int i = 0; i < 30; i++) swiWaitForVBlank();
				pressed = 0;
				do {
					scanKeys();
					pressed = keysDownRepeat();
					swiWaitForVBlank();
				} while (!(pressed & KEY_A));
				showdialogbox = false;
			}
		}

		if ((pressed & KEY_Y) && !dsiWareList && bnrRomType == 0 && fileOffset >= 0)
		{
			DirEntry* entry = &dirContents.at(fileOffset);
			if (entry->isDirectory)
			{
			}
			else
			{
				applaunch = true;
				useBootstrap = false;

				fadeType = false;	// Fade to white
				for (int i = 0; i < 25; i++) {
					swiWaitForVBlank();
				}
				cursorPosition = fileOffset;
				pagenum = 0;
				for (int i = 0; i < 100; i++) {
					if (cursorPosition > 39) {
						cursorPosition -= 40;
						pagenum++;
					} else {
						break;
					}
				}
				SaveSettings();

				// Return the chosen file
				return entry->name;
			}
		}

		if ((pressed & KEY_R) && !flashcardUsed)
		{
			consoleClear();
			printf("Please wait...\n");
			if (dsiWareList) {
				dsiWare_cursorPosition = fileOffset;
				dsiWarePageNum = 0;
				for (int i = 0; i < 100; i++) {
					if (dsiWare_cursorPosition > 39) {
						dsiWare_cursorPosition -= 40;
						dsiWarePageNum++;
					} else {
						break;
					}
				}
			} else {
				cursorPosition = fileOffset;
				pagenum = 0;
				for (int i = 0; i < 100; i++) {
					if (cursorPosition > 39) {
						cursorPosition -= 40;
						pagenum++;
					} else {
						break;
					}
				}
			}
			dsiWareList = !dsiWareList;
			SaveSettings();
			settingsChanged = false;
			return "null";		
		}

		if ((pressed & KEY_B) && showDirectories && !dsiWareList) {
			// Go up a directory
			chdir ("..");
			char buf[256];
			romfolder = getcwd(buf, 256);
			cursorPosition = 0;
			SaveSettings();
			settingsChanged = false;
			return "null";		
		}

		if (pressed & KEY_START)
		{
			if (settingsChanged) {
				if (dsiWareList) {
					dsiWare_cursorPosition = fileOffset;
					dsiWarePageNum = 0;
					for (int i = 0; i < 100; i++) {
						if (dsiWare_cursorPosition > 39) {
							dsiWare_cursorPosition -= 40;
							dsiWarePageNum++;
						} else {
							break;
						}
					}
				} else {
					cursorPosition = fileOffset;
					pagenum = 0;
					for (int i = 0; i < 100; i++) {
						if (cursorPosition > 39) {
							cursorPosition -= 40;
							pagenum++;
						} else {
							break;
						}
					}
				}
				SaveSettings();
				settingsChanged = false;
			}
			consoleClear();
			clearText();
			startMenu = true;
			return "null";		
		}

		if ((pressed & KEY_SELECT) && (isDirectory == false) && (bnrRomType == 0) && (isHomebrew == false)
		&& !dsiWareList)
		{
			FILE *f_nds_file = fopen(dirContents.at(fileOffset).name.c_str(), "rb");

			u32 SDKVersion = 0;
			char game_TID[5];
			grabTID(f_nds_file, game_TID);
			game_TID[4] = 0;
			game_TID[3] = 0;
			if(strcmp(game_TID, "###") != 0) SDKVersion = getSDKVersion(f_nds_file);
			fclose(f_nds_file);

			if((SDKVersion > 0x1000000) && (SDKVersion < 0x2000000)) {
				SDKnumbertext = "SDK ver: 1";
			} else if((SDKVersion > 0x2000000) && (SDKVersion < 0x3000000)) {
				SDKnumbertext = "SDK ver: 2";
			} else if((SDKVersion > 0x3000000) && (SDKVersion < 0x4000000)) {
				SDKnumbertext = "SDK ver: 3";
			} else if((SDKVersion > 0x4000000) && (SDKVersion < 0x5000000)) {
				SDKnumbertext = "SDK ver: 4";
			} else if((SDKVersion > 0x5000000) && (SDKVersion < 0x6000000)) {
				SDKnumbertext = "SDK ver: 5 (TWLSDK)";
			} else {
				SDKnumbertext = "SDK ver: ?";
			}
			showdialogbox = true;
			printLargeCentered(false, 84, "Info");
			printSmallCentered(false, 104, SDKnumbertext);
			printSmallCentered(false, 118, "A: OK");
			for (int i = 0; i < 30; i++) swiWaitForVBlank();
			pressed = 0;
			do {
				scanKeys();
				pressed = keysDownRepeat();
				swiWaitForVBlank();
			} while (!(pressed & KEY_A));
			showdialogbox = false;
		}

	}
}
