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
#include <algorithm>
#include <dirent.h>
#include <math.h>
#include <sstream>
#include <stdio.h>
#include <string>
#include <unistd.h>
#include <vector>

#include <nds.h>
#include <nds/arm9/dldi.h>
#include <fat.h>
#include <gl2d.h>

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
#include "errorScreen.h"
#include "incompatibleGameMap.h"

#include "gbaswitch.h"
#include "myDSiMode.h"

#include "common/inifile.h"

#include "sound.h"
#include "fileCopy.h"

#define SCREEN_COLS 32
#define SCREEN_COLS_GBNP 17
#define ENTRIES_PER_SCREEN 22
#define ENTRIES_PER_SCREEN_GBNP 7
#define ENTRIES_START_ROW 2
#define ENTRIES_START_ROW_GBNP 6
#define ENTRY_PAGE_LENGTH 10

extern const char *bootstrapinipath;

extern bool whiteScreen;
extern bool fadeType;
extern bool fadeSpeed;
extern bool macroMode;
extern int dsiWareExploit;
extern bool lcdSwapped;

extern bool useBootstrap;
extern bool homebrewBootstrap;
extern bool useGbarunner;
extern int consoleModel;
extern bool isRegularDS;
extern bool dsDebugRam;
extern bool dsiWramAccess;
extern bool arm7SCFGLocked;
extern bool smsGgInRam;
extern bool dsiWareToSD;
extern int b4dsMode;

extern bool showdialogbox;
extern int dialogboxHeight;

extern std::string romfolder[2];

extern std::string arm7DonorPath;
bool donorFound = true;

extern bool applaunch;
extern bool dsModeForced;

extern bool gotosettings;

using namespace std;

extern bool startMenu;

extern int theme;

extern int showMd;
extern bool showDirectories;
extern bool showHidden;
extern bool preventDeletion;
extern int spawnedtitleboxes;
extern int cursorPosition[2];
extern int startMenu_cursorPosition;
extern int pagenum[2];
extern bool showMicroSd;

extern bool dontShowClusterWarning;

extern int updateRecentlyPlayedList;
extern int sortMethod;

std::string gameOrderIniPath, recentlyPlayedIniPath, timesPlayedIniPath;

bool settingsChanged = false;

extern void SaveSettings();

char path[PATH_MAX] = {0};

static void gbnpBottomInfo(void) {
	if (theme == 6) {
		getcwd(path, PATH_MAX);

		clearText(false);

		// Print the path
		printLarge(false, 0, 0, path);

		printLargeCentered(false, 96, "SELECT: Settings menu");
	}
}

extern std::string ReplaceAll(std::string str, const std::string& from, const std::string& to);

struct DirEntry {
	DirEntry(std::string name, bool isDirectory, int position, int customPos) : name(name), isDirectory(isDirectory), position(position), customPos(customPos) {}
	DirEntry() {}

	std::string name;
	bool isDirectory;
	int position;
	bool customPos;
};

bool extension(const std::string_view filename, const std::vector<std::string_view> extensions) {
	for(std::string_view extension : extensions) {
		if(strcasecmp(filename.substr(filename.size() - extension.size()).data(), extension.data()) == 0) {
			return true;
		}
	}

	return false;
}

bool nameEndsWith(const std::string_view name, const std::vector<std::string_view> extensionList) {
	if (name.size() == 0)
		return false;

	if (extensionList.size() == 0)
		return true;

	if (name.substr(0, 2) == "._")
		return false; // Don't show macOS's index files

	for (const std::string_view &ext : extensionList) {
		if(name.length() > ext.length() && strcasecmp(name.substr(name.length() - ext.length()).data(), ext.data()) == 0)
			return true;
	}
	return false;
}

bool dirEntryPredicate(const DirEntry &lhs, const DirEntry &rhs) {
	if (lhs.isDirectory && !lhs.customPos && !rhs.isDirectory) {
		return true;
	}
	if (!lhs.isDirectory && rhs.isDirectory && !rhs.customPos) {
		return false;
	}
	if (lhs.customPos || rhs.customPos) {
		if(!lhs.customPos)	return false;
		else if(!rhs.customPos)	return true;

		if(lhs.position < rhs.position)	return true;
		else return false;
	}
	return strcasecmp(lhs.name.c_str(), rhs.name.c_str()) < 0;
}

void getDirectoryContents(std::vector<DirEntry> &dirContents, const std::vector<std::string_view> extensionList = {}) {
	dirContents.clear();

	DIR *pdir = opendir(".");

	if (pdir == nullptr) {
		iprintf("Unable to open the directory.\n");
	} else {
		int currentPos = 0;
		while (true) {
			snd().updateStream();

			dirent *pent = readdir(pdir);
			if (pent == nullptr)
				break;

			if (showDirectories) {
				if (strcmp(pent->d_name, ".") != 0 && strcmp(pent->d_name, "_nds") != 0
					&& strcmp(pent->d_name, "saves") != 0
					&& (pent->d_type == DT_DIR || nameEndsWith(pent->d_name, extensionList))) {
					if (showHidden || !(FAT_getAttr(pent->d_name) & ATTR_HIDDEN || (pent->d_name[0] == '.' && strcmp(pent->d_name, "..") != 0))) {
						dirContents.emplace_back(pent->d_name, pent->d_type == DT_DIR, currentPos, false);
					}
				}
			} else {
				if (pent->d_type != DT_DIR && nameEndsWith(pent->d_name, extensionList)) {
					if (showHidden || !(FAT_getAttr(pent->d_name) & ATTR_HIDDEN || (pent->d_name[0] == '.' && strcmp(pent->d_name, "..") != 0))) {
						dirContents.emplace_back(pent->d_name, false, currentPos, false);
					}
				}
			}
		}

		if (sortMethod == 0) { // Alphabetical
			std::sort(dirContents.begin(), dirContents.end(), dirEntryPredicate);
		} else if (sortMethod == 1) { // Recent
			CIniFile recentlyPlayedIni(recentlyPlayedIniPath);
			std::vector<std::string> recentlyPlayed;
			getcwd(path, PATH_MAX);
			recentlyPlayedIni.GetStringVector("RECENT", path, recentlyPlayed, ':');

			int i = 0;
			for (const std::string &recentlyPlayedName : recentlyPlayed) {
				for (DirEntry &dirEntry : dirContents) {
					if (recentlyPlayedName == dirEntry.name) {
						dirEntry.position = i++;
						dirEntry.customPos = true;
						break;
					}
				}
			}
			sort(dirContents.begin(), dirContents.end(), dirEntryPredicate);
		} else if (sortMethod == 2) { // Most Played
			CIniFile timesPlayedIni(timesPlayedIniPath);

			getcwd(path, PATH_MAX);
			for (DirEntry &dirEntry : dirContents) {
				dirEntry.position = timesPlayedIni.GetInt(path, dirEntry.name, 0);
			}

			std::sort(dirContents.begin(), dirContents.end(), [](const DirEntry &lhs, const DirEntry &rhs) {
					if (!lhs.isDirectory && rhs.isDirectory)
						return false;
					else if (lhs.isDirectory && !rhs.isDirectory)
						return true;

					if (lhs.position > rhs.position)
						return true;
					else if (lhs.position < rhs.position)
						return false;
					else
						return strcasecmp(lhs.name.c_str(), rhs.name.c_str()) < 0;
				});
		} else if (sortMethod == 3) { // File type
			sort(dirContents.begin(), dirContents.end(), [](const DirEntry &lhs, const DirEntry &rhs) {
					if (!lhs.isDirectory && rhs.isDirectory)
						return false;
					else if (lhs.isDirectory && !rhs.isDirectory)
						return true;

					int extCmp = strcasecmp(lhs.name.substr(lhs.name.find_last_of('.') + 1).c_str(), rhs.name.substr(rhs.name.find_last_of('.') + 1).c_str());
					if(extCmp == 0)
						return strcasecmp(lhs.name.c_str(), rhs.name.c_str()) < 0;
					else
						return extCmp < 0;
				});
		} else if (sortMethod == 4) { // Custom
			CIniFile gameOrderIni(gameOrderIniPath);
			std::vector<std::string> gameOrder;
			getcwd(path, PATH_MAX);
			gameOrderIni.GetStringVector("ORDER", path, gameOrder, ':');

			for (uint i = 0; i < gameOrder.size(); i++) {
				for (DirEntry &dirEntry : dirContents) {
					if (gameOrder[i] == dirEntry.name) {
						dirEntry.position = i;
						dirEntry.customPos = true;
						break;
					}
				}
			}
			sort(dirContents.begin(), dirContents.end(), dirEntryPredicate);
		}
		closedir(pdir);
	}
}

void showDirectoryContents (const vector<DirEntry>& dirContents, int startRow) {
	getcwd(path, PATH_MAX);

	// Clear the screen
	iprintf ("\x1b[2J");

	// Print the path
	if (theme != 6) {
	if (strlen(path) < SCREEN_COLS) {
		iprintf ("%s", path);
	} else {
		iprintf ("%s", path + strlen(path) - SCREEN_COLS);
	}
	}

	if (theme != 6) {
		// Move to 2nd row
		iprintf ("\x1b[1;0H");
		// Print line of dashes
		iprintf ("--------------------------------");
	}

	int screenCols = (theme==6 ? SCREEN_COLS_GBNP : SCREEN_COLS);

	// Print directory listing
	for (int i = 0; i < ((int)dirContents.size() - startRow) && i < (theme==6 ? ENTRIES_PER_SCREEN_GBNP : ENTRIES_PER_SCREEN); i++) {
		const DirEntry* entry = &dirContents.at(i + startRow);
		char entryName[screenCols + 1];
		
		// Set row
		iprintf ("\x1b[%d;%dH", i + (theme==6 ? ENTRIES_START_ROW_GBNP : ENTRIES_START_ROW), (theme==6 ? 7 : 0));
		
		if (entry->isDirectory) {
			strncpy (entryName, entry->name.c_str(), screenCols);
			entryName[screenCols - 3] = '\0';
			iprintf (" [%s]", entryName);
		} else {
			strncpy (entryName, entry->name.c_str(), screenCols);
			entryName[screenCols - 1] = '\0';
			iprintf (" %s", entryName);
		}
	}
}

void mdRomTooBig(void) {
	if (macroMode) {
		lcdMainOnBottom();
		lcdSwapped = true;
	}
	dialogboxHeight = 3;
	showdialogbox = true;
	printLargeCentered(false, 74, "Error!");
	printSmallCentered(false, 90, "This SEGA Genesis/Mega Drive");
	printSmallCentered(false, 102, "ROM cannot be launched,");
	printSmallCentered(false, 114, "due to its surpassing the");
	printSmallCentered(false, 126, "size limit of 3MB.");
	printSmallCentered(false, 144, "\u2427 OK");
	int pressed = 0;
	do {
		scanKeys();
		pressed = keysDown();
		checkSdEject();
		snd().updateStream();
		swiWaitForVBlank();
	} while (!(pressed & KEY_A));
	clearText();
	showdialogbox = false;
	dialogboxHeight = 0;

	if (macroMode) {
		lcdMainOnTop();
		lcdSwapped = false;
	}
}

void ramDiskMsg(void) {
	if (macroMode) {
		lcdMainOnBottom();
		lcdSwapped = true;
	}
	dialogboxHeight = 1;
	showdialogbox = true;
	printLargeCentered(false, 74, "Error!");
	printSmallCentered(false, 90, "This app requires a");
	printSmallCentered(false, 102, "RAM disk to work.");
	printSmallCentered(false, 120, "\u2427 OK");
	int pressed = 0;
	do {
		snd().updateStream();
		scanKeys();
		pressed = keysDown();
		checkSdEject();
		swiWaitForVBlank();
	} while (!(pressed & KEY_A));
	clearText();
	showdialogbox = false;
	dialogboxHeight = 0;

	if (macroMode) {
		lcdMainOnTop();
		lcdSwapped = false;
	}
}

bool dsiBinariesMissingMsg(void) {
	bool proceedToLaunch = false;

	if (macroMode) {
		lcdMainOnBottom();
		lcdSwapped = true;
	}
	dialogboxHeight = 2;
	showdialogbox = true;
	printLargeCentered(false, 74, "Error!");
	printSmallCentered(false, 90, "The DSi binaries are missing.");
	printSmallCentered(false, 102, "Please get a clean dump of");
	printSmallCentered(false, 114, "this ROM, or start in DS mode.");
	printSmallCentered(false, 132, "\u2430 Launch in DS mode  \u2428 Back");
	int pressed = 0;
	while (1) {
		scanKeys();
		pressed = keysDown();
		checkSdEject();
		snd().updateStream();
		swiWaitForVBlank();
		if (pressed & KEY_Y) {
			dsModeForced = true;
			proceedToLaunch = true;
			pressed = 0;
			break;
		}
		if (pressed & KEY_B) {
			proceedToLaunch = false;
			pressed = 0;
			break;
		}
	}
	clearText();
	showdialogbox = false;
	dialogboxHeight = 0;

	if (macroMode) {
		lcdMainOnTop();
		lcdSwapped = false;
	}

	return proceedToLaunch;
}

bool donorRomMsg(void) {
	bool proceedToLaunch = true;
	bool dsModeAllowed = ((requiresDonorRom == 52 || requiresDonorRom == 53) && !isDSiWare);

	if (macroMode) {
		lcdMainOnBottom();
		lcdSwapped = true;
	}
	int msgPage = 0;
	bool pageLoaded = false;
	dialogboxHeight = 2;
	showdialogbox = true;
	int pressed = 0;
	while (1) {
		if (!pageLoaded) {
			clearText();
			printLargeCentered(false, 74, "Error!");
			if (msgPage == 1) {
				printSmallCentered(false, 90, "To set a donor ROM, find");
				printSmallCentered(false, 102, "mentioned ROM, press (Y), and");
				printSmallCentered(false, 114, "select \"Set as Donor ROM\".");
				printSmall(false, 18, 132, "<");
			} else {
				switch (requiresDonorRom) {
					default:
						break;
					case 51:
						printSmallCentered(false, 90, "This title requires a donor ROM");
						printSmallCentered(false, 102, romUnitCode == 3 ? "to run. Please set a" : "to run. Please set another");
						printSmallCentered(false, 114, "DSi-Enhanced title as a donor ROM.");
						break;
					case 52:
						printSmallCentered(false, 90, dsModeAllowed ? "DSi mode requires a donor ROM" : "This title requires a donor ROM");
						printSmallCentered(false, 102, dsModeAllowed ? "to run. Please set a" : "to run. Please set another");
						printSmallCentered(false, 114, "DSi(Ware) title as a donor ROM.");
						break;
				}
				printSmallRightAlign(false, 256 - 16, 132, ">");
			}
			printSmallCentered(false, 132, dsModeAllowed ? "(Y) Launch in DS mode  \u2428 Back" : "\u2428 Back");
			pageLoaded = true;
		}
		scanKeys();
		pressed = keysDown();
		checkSdEject();
		snd().updateStream();
		swiWaitForVBlank();
		if ((pressed & KEY_LEFT) && msgPage != 0) {
			msgPage = 0;
			pageLoaded = false;
		} else if ((pressed & KEY_RIGHT) && msgPage != 1) {
			msgPage = 1;
			pageLoaded = false;
		}
		if (dsModeAllowed && (pressed & KEY_Y)) {
			dsModeForced = true;
			proceedToLaunch = true;
			pressed = 0;
			break;
		}
		if (pressed & KEY_B) {
			proceedToLaunch = false;
			pressed = 0;
			break;
		}
	}
	clearText();
	showdialogbox = false;
	dialogboxHeight = 0;

	if (macroMode) {
		lcdMainOnTop();
		lcdSwapped = false;
	}

	return proceedToLaunch;
}

void showLocation(void) {
	if (isRegularDS) return;

	printSmall(false, 8, 162, "Location:");
	if (secondaryDevice) {
		printSmall(false, 8, 174, "Slot-1 microSD Card");
	} else {
		printSmall(false, 8, 174, showMicroSd ? "microSD Card" : "SD Card");
	}
}

bool checkForCompatibleGame(char gameTid[5], const char *filename) {
	bool proceedToLaunch = true;

	if (!dsiFeatures() && secondaryDevice) {
		// TODO: If the list gets large enough, switch to bsearch().
		for (unsigned int i = 0; i < sizeof(incompatibleGameListB4DS)/sizeof(incompatibleGameListB4DS[0]); i++) {
			if (memcmp(gameTid, incompatibleGameListB4DS[i], 3) == 0) {
				// Found match
				proceedToLaunch = false;
				break;
			}
		}
	}

	if (proceedToLaunch && secondaryDevice) {
		// TODO: If the list gets large enough, switch to bsearch().
		for (unsigned int i = 0; i < sizeof(incompatibleGameListFC)/sizeof(incompatibleGameListFC[0]); i++) {
			if (memcmp(gameTid, incompatibleGameListFC[i], 3) == 0) {
				// Found match
				proceedToLaunch = false;
				break;
			}
		}
	}

	if (proceedToLaunch) {
		// TODO: If the list gets large enough, switch to bsearch().
		for (unsigned int i = 0; i < sizeof(incompatibleGameList)/sizeof(incompatibleGameList[0]); i++) {
			if (memcmp(gameTid, incompatibleGameList[i], 3) == 0) {
				// Found match
				proceedToLaunch = false;
				break;
			}
		}
	}

	if (proceedToLaunch) return true;	// Game is compatible

	if (macroMode) {
		lcdMainOnBottom();
		lcdSwapped = true;
	}
	dialogboxHeight = 3;
	showdialogbox = true;
	printLargeCentered(false, 74, "Compatibility Warning");
	printSmallCentered(false, 90, "This game is known to not run.");
	printSmallCentered(false, 102, "If there's an nds-bootstrap");
	printSmallCentered(false, 114, "version that fixes this,");
	printSmallCentered(false, 126, "please ignore this message.");
	printSmallCentered(false, 144, "\u2427 Ignore   \u2428 Don't launch");

	int pressed = 0;
	while (1) {
		scanKeys();
		pressed = keysDown();
		checkSdEject();
		snd().updateStream();
		swiWaitForVBlank();
		if (pressed & KEY_A) {
			proceedToLaunch = true;
			pressed = 0;
			break;
		}
		if (pressed & KEY_B) {
			proceedToLaunch = false;
			pressed = 0;
			break;
		}
	}
	clearText();
	showdialogbox = false;
	dialogboxHeight = 0;

	if (proceedToLaunch) {
		titleUpdate(false, filename);
		showLocation();
	} else if (macroMode) {
		lcdMainOnTop();
		lcdSwapped = false;
	}

	return proceedToLaunch;
}

bool gameCompatibleMemoryPit(const char* filename) {
	FILE *f_nds_file = fopen(filename, "rb");
	char game_TID[5];
	grabTID(f_nds_file, game_TID);
	fclose(f_nds_file);

	// TODO: If the list gets large enough, switch to bsearch().
	for (unsigned int i = 0; i < sizeof(incompatibleGameListMemoryPit)/sizeof(incompatibleGameListMemoryPit[0]); i++) {
		if (memcmp(game_TID, incompatibleGameListMemoryPit[i], 3) == 0) {
			// Found match
			return false;
		}
	}
	return true;
}

bool dsiWareCompatibleB4DS(const char* filename) {
	bool res = false;
	FILE *f_nds_file = fopen(filename, "rb");
	char game_TID[5];
	grabTID(f_nds_file, game_TID);
	fclose(f_nds_file);

	// TODO: If the list gets large enough, switch to bsearch().
	for (unsigned int i = 0; i < sizeof(compatibleGameListB4DS)/sizeof(compatibleGameListB4DS[0]); i++) {
		if (memcmp(game_TID, compatibleGameListB4DS[i], (compatibleGameListB4DS[i][3] != 0 ? 4 : 3)) == 0) {
			// Found match
			res = true;
			break;
		}
	}
	if (!res && (dsDebugRam || b4dsMode == 2)) {
		for (unsigned int i = 0; i < sizeof(compatibleGameListB4DSDebug)/sizeof(compatibleGameListB4DSDebug[0]); i++) {
			if (memcmp(game_TID, compatibleGameListB4DSDebug[i], 3) == 0) {
				// Found match
				res = true;
				break;
			}
		}
	}
	return res;
}

void cannotLaunchMsg(void) {
	if (macroMode) {
		lcdMainOnBottom();
		lcdSwapped = true;
	}
	showdialogbox = true;
	printLargeCentered(false, 74, "Error!");
	printSmallCentered(false, 90, "This game cannot be launched.");
	printSmallCentered(false, 108, "\u2427 OK");
	int pressed = 0;
	do {
		snd().updateStream();
		scanKeys();
		pressed = keysDown();
		checkSdEject();
		swiWaitForVBlank();
	} while (!(pressed & KEY_A));
	showdialogbox = false;
	if (macroMode) {
		lcdMainOnTop();
		lcdSwapped = false;
	}
	for (int i = 0; i < 25; i++) swiWaitForVBlank();
}

string browseForFile(const vector<string_view> extensionList) {
	if (macroMode) {
		lcdMainOnTop();
		lcdSwapped = false;
	}

	gameOrderIniPath = std::string(sdFound() ? "sd" : "fat") + ":/_nds/TWiLightMenu/extras/gameorder.ini";
	recentlyPlayedIniPath = std::string(sdFound() ? "sd" : "fat") + ":/_nds/TWiLightMenu/extras/recentlyplayed.ini";
	timesPlayedIniPath = std::string(sdFound() ? "sd" : "fat") + ":/_nds/TWiLightMenu/extras/timesplayed.ini";

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

		int entriesStartRow = (theme==6 ? ENTRIES_START_ROW_GBNP : ENTRIES_START_ROW);
		int entriesPerScreen = (theme==6 ? ENTRIES_PER_SCREEN_GBNP : ENTRIES_PER_SCREEN);

		// Clear old cursors
		for (int i = entriesStartRow; i < entriesPerScreen + entriesStartRow; i++) {
			iprintf ("\x1b[%d;%dH ", i, (theme==6 ? 7 : 0));
			if (theme==6) {
				iprintf ("\x1b[%d;24H ", i);
			}
		}
		// Show cursor
		if (theme==6) {
			iprintf ("\x1B[43m");		// Print foreground yellow color
		}
		iprintf ("\x1b[%d;%dH", fileOffset - screenOffset + entriesStartRow, (theme==6 ? 7 : 0));
		iprintf ("%s", (theme==6 ? "<" : "*"));
		if (theme==6) {
			iprintf ("\x1b[%d;24H>", fileOffset - screenOffset + entriesStartRow);
			iprintf ("\x1B[47m");		// Print foreground white color
		}

		if (dirContents.at(fileOffset).isDirectory) {
			isDirectory = true;
			bnrWirelessIcon = 0;
		} else {
			isDirectory = false;
			std::string std_romsel_filename = dirContents.at(fileOffset).name.c_str();

			if (extension(std_romsel_filename, {".nds", ".dsi", ".ids", ".srl", ".app", ".argv"}))
			{
				getGameInfo(isDirectory, dirContents.at(fileOffset).name.c_str());
				bnrRomType = 0;
			} else if (extension(std_romsel_filename, {".pce"})) {
				bnrRomType = 11;
			} else if (extension(std_romsel_filename, {".xex", ".atr", ".a26", ".a52", ".a78"})) {
				bnrRomType = 10;
			} else if (extension(std_romsel_filename, {".int"})) {
				bnrRomType = 12;
			} else if (extension(std_romsel_filename, {".plg", ".rvid", ".fv"})) {
				bnrRomType = 9;
			} else if (extension(std_romsel_filename, {".agb", ".gba", ".mb"})) {
				bnrRomType = 1;
			} else if (extension(std_romsel_filename, {".gb", ".sgb"})) {
				bnrRomType = 2;
			} else if (extension(std_romsel_filename,{ ".gbc"})) {
				bnrRomType = 3;
			} else if (extension(std_romsel_filename, {".nes", ".fds"})) {
				bnrRomType = 4;
			} else if(extension(std_romsel_filename, {".sms"})) {
				bnrRomType = 5;
			} else if(extension(std_romsel_filename, {".gg"})) {
				bnrRomType = 6;
			} else if(extension(std_romsel_filename, {".gen"})) {
				bnrRomType = 7;
			} else if(extension(std_romsel_filename, {".smc", ".sfc"})) {
				bnrRomType = 8;
			}
		}

		if (bnrRomType > 0 && bnrRomType < 10) {
			bnrWirelessIcon = 0;
			isDSiWare = false;
			isHomebrew = 0;
		}

		if (bnrRomType == 0) iconUpdate (dirContents.at(fileOffset).isDirectory,dirContents.at(fileOffset).name.c_str());
		titleUpdate (dirContents.at(fileOffset).isDirectory,dirContents.at(fileOffset).name.c_str());

		showLocation();

		// Power saving loop. Only poll the keys once per frame and sleep the CPU if there is nothing else to do
		do {
			snd().updateStream();
			scanKeys();
			pressed = keysDownRepeat();
			checkSdEject();
			swiWaitForVBlank();
		} while (!pressed);

		if (pressed & KEY_UP) {
			fileOffset -= 1;
			if (theme == 6) {
				snd().playSelect();
			}
			settingsChanged = true;
		}
		if (pressed & KEY_DOWN) {
			fileOffset += 1;
			if (theme == 6) {
				snd().playSelect();
			}
			settingsChanged = true;
		}
		if (pressed & KEY_LEFT) {
			fileOffset -= ENTRY_PAGE_LENGTH;
			if (theme == 6) {
				snd().playSelect();
			}
			settingsChanged = true;
		}
		if (pressed & KEY_RIGHT) {
			fileOffset += ENTRY_PAGE_LENGTH;
			if (theme == 6) {
				snd().playSelect();
			}
			settingsChanged = true;
		}

		if (fileOffset < 0) 	fileOffset = dirContents.size() - 1;		// Wrap around to bottom of list
		if (fileOffset > ((int)dirContents.size() - 1))		fileOffset = 0;		// Wrap around to top of list

		// Scroll screen if needed
		if (fileOffset < screenOffset) 	{
			screenOffset = fileOffset;
			showDirectoryContents (dirContents, screenOffset);
		}
		if (fileOffset > screenOffset + entriesPerScreen - 1) {
			screenOffset = fileOffset - entriesPerScreen + 1;
			showDirectoryContents (dirContents, screenOffset);
		}

		if (pressed & KEY_A) {
			DirEntry* entry = &dirContents.at(fileOffset);
			if (entry->isDirectory) {
				if (theme == 6) {
					snd().playSelect();

					// Clear the screen
					iprintf ("\x1b[2J");

					iprintf ("\x1b[6;8H");
				}
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
			else if (isDSiWare && ((((!dsiFeatures() && (!sdFound() || !dsiWareToSD)) || b4dsMode) && secondaryDevice && !dsiWareCompatibleB4DS(dirContents.at(fileOffset).name.c_str()))
			|| (isDSiMode() && memcmp(io_dldi_data->friendlyName, "CycloDS iEvolution", 18) != 0 && arm7SCFGLocked && !dsiWramAccess && !gameCompatibleMemoryPit(dirContents.at(fileOffset).name.c_str())))) {
				cannotLaunchMsg();
			} else {
				int hasAP = 0;
				bool proceedToLaunch = true;
				bool useBootstrapAnyway = (useBootstrap || !secondaryDevice);
				if (useBootstrapAnyway && bnrRomType == 0 && isHomebrew == 0)
				{
					FILE *f_nds_file = fopen(dirContents.at(fileOffset).name.c_str(), "rb");
					char game_TID[5];
					grabTID(f_nds_file, game_TID);
					game_TID[4] = 0;
					fclose(f_nds_file);

					proceedToLaunch = checkForCompatibleGame(game_TID, dirContents.at(fileOffset).name.c_str());
					if (proceedToLaunch && requiresDonorRom)
					{
						const char* pathDefine = "DONORTWL_NDS_PATH";
						if (requiresDonorRom == 52) {
							pathDefine = "DONORTWLONLY_NDS_PATH";
						}
						std::string donorRomPath;
						bootstrapinipath = sdFound() ? "sd:/_nds/nds-bootstrap.ini" : "fat:/_nds/nds-bootstrap.ini";
						loadPerGameSettings(dirContents.at(fileOffset).name);
						int dsiModeSetting = (perGameSettings_dsiMode == -1 ? DEFAULT_DSI_MODE : perGameSettings_dsiMode);
						CIniFile bootstrapini(bootstrapinipath);
						donorRomPath = bootstrapini.GetString("NDS-BOOTSTRAP", pathDefine, "");
						if ((donorRomPath == "" || access(donorRomPath.c_str(), F_OK) != 0)
						&& (requiresDonorRom == 51 || (requiresDonorRom == 52 && (isDSiWare || dsiModeSetting > 0)))) {
							proceedToLaunch = donorRomMsg();
						}
					}
					if (proceedToLaunch && checkIfShowAPMsg(dirContents.at(fileOffset).name))
					{
						FILE *f_nds_file = fopen(dirContents.at(fileOffset).name.c_str(), "rb");
						hasAP = checkRomAP(f_nds_file);
						fclose(f_nds_file);
					}
				}
				else if (isHomebrew == 1)
				{
					loadPerGameSettings(dirContents.at(fileOffset).name);
					if (requiresRamDisk && perGameSettings_ramDiskNo == -1) {
						proceedToLaunch = false;
						ramDiskMsg();
					}
				}
				else if (bnrRomType == 7)
				{
					if (showMd==1 && getFileSize(dirContents.at(fileOffset).name.c_str()) > 0x300000) {
						proceedToLaunch = false;
						mdRomTooBig();
					}
				}
				else if ((bnrRomType == 8 || (bnrRomType == 11 && smsGgInRam))
							&& isDSiMode() && memcmp(io_dldi_data->friendlyName, "CycloDS iEvolution", 18) != 0 && arm7SCFGLocked)
				{
					proceedToLaunch = false;
					cannotLaunchMsg();
				}

				if (hasAP > 0) {
					if (macroMode) {
						lcdMainOnBottom();
						lcdSwapped = true;
					}

					dialogboxHeight = 3;
					showdialogbox = true;
					printLargeCentered(false, 74, "Anti-Piracy Warning");
					if (hasAP == 2) {
						printSmallCentered(false, 98, "This game has AP, and MUST");
						printSmallCentered(false, 110, "be patched using the RGF");
						printSmallCentered(false, 122, "TWiLight Menu AP patcher.");
					} else {
						printSmallCentered(false, 98, "This game has AP. Please");
						printSmallCentered(false, 110, "make sure you're using the");
						printSmallCentered(false, 122, "latest TWiLight Menu++.");
					}
					printSmallCentered(false, 142, "\u2428 Return   \u2427 Launch");

					pressed = 0;
					while (1) {
						snd().updateStream();
						scanKeys();
						pressed = keysDown();
						checkSdEject();
						swiWaitForVBlank();
						if (pressed & KEY_A) {
							pressed = 0;
							break;
						}
						if (pressed & KEY_B) {
							proceedToLaunch = false;
							pressed = 0;
							break;
						}
						if (pressed & KEY_X) {
							dontShowAPMsgAgain(dirContents.at(fileOffset).name);
							pressed = 0;
							break;
						}
					}
					clearText();
					showdialogbox = false;
					dialogboxHeight = 0;

					if (proceedToLaunch) {
						titleUpdate (dirContents.at(fileOffset).isDirectory,dirContents.at(fileOffset).name.c_str());
						showLocation();
					} else if (macroMode) {
						lcdMainOnTop();
						lcdSwapped = false;
					}
				}

				if (proceedToLaunch && useBootstrapAnyway && bnrRomType == 0 && !isDSiWare
				 && !dsModeForced && isHomebrew == 0
				 && checkIfDSiMode(dirContents.at(fileOffset).name)) {
					bool hasDsiBinaries = true;
					if (!arm7SCFGLocked || memcmp(io_dldi_data->friendlyName, "CycloDS iEvolution", 18) == 0) {
						FILE *f_nds_file = fopen(dirContents.at(fileOffset).name.c_str(), "rb");
						hasDsiBinaries = checkDsiBinaries(f_nds_file);
						fclose(f_nds_file);
					}

					if (!hasDsiBinaries) {
						proceedToLaunch = dsiBinariesMissingMsg();
					}
				}

				// If SD card's cluster size is less than 32KB, then show warning for DS games with nds-bootstrap
				extern struct statvfs st[2];
				if (useBootstrapAnyway && bnrRomType == 0 && !isDSiWare && isHomebrew == 0
				 && proceedToLaunch && st[secondaryDevice].f_bsize < (32 << 10) && !dontShowClusterWarning) {
					if (macroMode) {
						lcdMainOnBottom();
						lcdSwapped = true;
					}

					dialogboxHeight = 5;
					showdialogbox = true;
					// Clear location text
					clearText();
					titleUpdate(dirContents.at(fileOffset).isDirectory,dirContents.at(fileOffset).name.c_str());

					printLargeCentered(false, 74, "Cluster Size Warning");
					printSmallCentered(false, 98, "Your SD card is not formatted");
					printSmallCentered(false, 110, "using 32KB clusters, this causes");
					printSmallCentered(false, 122, "some games to load very slowly.");
					printSmallCentered(false, 134, "It's recommended to reformat your");
					printSmallCentered(false, 146, "SD card using 32KB clusters.");
					printSmallCentered(false, 166, "\u2428 Return   \u2427 Launch");

					pressed = 0;
					while (1) {
						snd().updateStream();
						scanKeys();
						pressed = keysDown();
						checkSdEject();
						swiWaitForVBlank();
						if (pressed & KEY_A) {
							pressed = 0;
							break;
						}
						if (pressed & KEY_B) {
							proceedToLaunch = false;
							pressed = 0;
							break;
						}
						if (pressed & KEY_X) {
							dontShowClusterWarning = true;
							pressed = 0;
							break;
						}
					}
					clearText();
					showdialogbox = false;
					dialogboxHeight = 0;

					if (proceedToLaunch) {
						titleUpdate(dirContents.at(fileOffset).isDirectory,dirContents.at(fileOffset).name.c_str());
						showLocation();
					} else if (macroMode) {
						lcdMainOnTop();
						lcdSwapped = false;
					}
				}

				if (proceedToLaunch) {
					if (theme == 6) {
						snd().playLaunch();
						snd().stopStream();
						for (int i = 0; i < 25; i++) swiWaitForVBlank();
					}
					applaunch = true;

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

					if(updateRecentlyPlayedList) {
						dialogboxHeight = 2;
						showdialogbox = true;

						printLargeCentered(false, 74, "Now saving...");

						printSmallCentered(false, 0, 98, "If this crashes with an error, please");
						printSmallCentered(false, 0, 110, "disable \"Update recently played list\".");

						mkdir(sdFound() ? "sd:/_nds/TWiLightMenu/extras" : "fat:/_nds/TWiLightMenu/extras", 0777);

						CIniFile recentlyPlayedIni(recentlyPlayedIniPath);
						std::vector<std::string> recentlyPlayed;

						getcwd(path, PATH_MAX);
						recentlyPlayedIni.GetStringVector("RECENT", path, recentlyPlayed, ':'); // : isn't allowed in FAT-32 names, so its a good deliminator

						std::vector<std::string>::iterator it = std::find(recentlyPlayed.begin(), recentlyPlayed.end(), entry->name);
						if(it != recentlyPlayed.end()) {
							recentlyPlayed.erase(it);
						}

						recentlyPlayed.insert(recentlyPlayed.begin(), entry->name);

						recentlyPlayedIni.SetStringVector("RECENT", path, recentlyPlayed, ':');
						recentlyPlayedIni.SaveIniFile(recentlyPlayedIniPath);

						CIniFile timesPlayedIni(timesPlayedIniPath);
						timesPlayedIni.SetInt(path, entry->name, (timesPlayedIni.GetInt(path, entry->name, 0) + 1));
						timesPlayedIni.SaveIniFile(timesPlayedIniPath);
					}

					// Return the chosen file
					return entry->name;
				} else {
					if (theme == 6) {
						gbnpBottomInfo();
					}
					for (int i = 0; i < 25; i++) swiWaitForVBlank();
				}
			}
		}

		if ((pressed & KEY_R) && bothSDandFlashcard()) {
			consoleClear();
			if (theme == 6) {
				iprintf ("\x1b[6;8H");
			}
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

		if ((pressed & KEY_X) && !preventDeletion && dirContents.at(fileOffset).name != "..")
		{
			if (macroMode) {
				lcdMainOnBottom();
				lcdSwapped = true;
			}

			DirEntry *entry = &dirContents.at(fileOffset);
			bool unHide = (FAT_getAttr(entry->name.c_str()) & ATTR_HIDDEN || (strncmp(entry->name.c_str(), ".", 1) == 0 && entry->name != ".."));

			showdialogbox = true;
			dialogboxHeight = 3;

			if (isDirectory) {
				printLargeCentered(false, 74, "Folder Management options");
				printSmallCentered(false, 98, "What would you like");
				printSmallCentered(false, 110, "to do with this folder?");
			} else {
				printLargeCentered(false, 74, "ROM Management options");
				printSmallCentered(false, 98, "What would you like");
				printSmallCentered(false, 110, "to do with this ROM?");
			}

			for (int i = 0; i < 90; i++) swiWaitForVBlank();

			if (isDirectory) {
				if(unHide)	printSmallCentered(false, 128, "Y: Unhide  \u2428 Nothing");
				else		printSmallCentered(false, 128, "Y: Hide    \u2428 Nothing");
			} else {
				if(unHide)	printSmallCentered(false, 128, "Y: Unhide  \u2427 Delete  \u2428 Nothing");
				else		printSmallCentered(false, 128, "Y: Hide   \u2427 Delete   \u2428 Nothing");
			}

			while (1) {
				do {
					snd().updateStream();
					scanKeys();
					pressed = keysDown();
					checkSdEject();
					swiWaitForVBlank();
				} while (!pressed);
				
				if ((pressed & KEY_A && !isDirectory) || (pressed & KEY_Y)) {
					clearText();
					showdialogbox = false;
					consoleClear();
					if (theme == 6) {
						iprintf ("\x1b[6;8H");
					}
					printf("Please wait...\n");

					if (pressed & KEY_A && !isDirectory) {
						remove(dirContents.at(fileOffset).name.c_str());
					} else if (pressed & KEY_Y) {
						// Remove leading . if it exists
						if((strncmp(entry->name.c_str(), ".", 1) == 0 && entry->name != "..")) {
							rename(entry->name.c_str(), entry->name.substr(1).c_str());
						} else { // Otherwise toggle the hidden attribute bit
							FAT_setAttr(entry->name.c_str(), FAT_getAttr(entry->name.c_str()) ^ ATTR_HIDDEN);
						}
					}
					
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
					if (theme == 6) {
						gbnpBottomInfo();
					}
					break;
				}
			}
			clearText();
			showdialogbox = false;

			if (macroMode) {
				lcdMainOnTop();
				lcdSwapped = false;
			}
		}

		if ((theme!=6 && (pressed & KEY_START)) || (theme==6 && (pressed & KEY_SELECT)))
		{
			if (theme == 6) {
				snd().playLaunch();
				snd().stopStream();
				for (int i = 0; i < 25; i++) swiWaitForVBlank();
			}
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

		if ((pressed & KEY_Y) && (isDirectory == false) && (bnrRomType == 0))
		{
			cursorPosition[secondaryDevice] = fileOffset;
			perGameSettings(dirContents.at(fileOffset).name);
			if (theme == 6) {
				gbnpBottomInfo();
			}
			for (int i = 0; i < 25; i++) swiWaitForVBlank();
		}

	}
}
