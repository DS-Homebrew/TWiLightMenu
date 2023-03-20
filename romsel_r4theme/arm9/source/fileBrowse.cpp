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
#include "common/bootstrapsettings.h"
#include "common/flashcard.h"
#include "common/systemdetails.h"
#include "common/twlmenusettings.h"
#include "iconTitle.h"
#include "graphics/fontHandler.h"
#include "graphics/graphics.h"
#include "graphics/FontGraphic.h"
#include "graphics/TextPane.h"
#include "SwitchState.h"
#include "perGameSettings.h"
#include "errorScreen.h"
#include "incompatibleGameMap.h"
#include "compatibleDSiWareMap.h"

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

extern bool whiteScreen;
extern bool fadeType;
extern bool fadeSpeed;
extern bool lcdSwapped;

extern bool showdialogbox;
extern int dialogboxHeight;

extern bool applaunch;
extern bool dsModeForced;

extern bool startMenu;

extern void bgOperations(bool waitFrame);

std::string gameOrderIniPath, recentlyPlayedIniPath, timesPlayedIniPath;

char path[PATH_MAX] = {0};

static void gbnpBottomInfo(void) {
	if (ms().theme != TWLSettings::EThemeGBC) {
		return;
	}
	getcwd(path, PATH_MAX);

	clearText(false);

	// Print the path
	printLarge(false, 0, 0, path);

	if (!ms().kioskMode) {
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
	for (std::string_view extension : extensions) {
		if (strcasecmp(filename.substr(filename.size() - extension.size()).data(), extension.data()) == 0) {
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
		if (name.length() > ext.length() && strcasecmp(name.substr(name.length() - ext.length()).data(), ext.data()) == 0)
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
		if (!lhs.customPos)	return false;
		else if (!rhs.customPos)	return true;

		if (lhs.position < rhs.position)	return true;
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
		int file_count = 0;
		while (1) {
			bgOperations(false);

			// This has to be done *before* readdir, since readdir increments
			// the internal state's DIR_ENTRY for the next time
			int attrs = 0;
			if (!ms().showHidden) {
				// Get FAT attrs, this is equivalent to FAT_getAttr(pent->d_name)
				// but much quicker since we don't have to search the filesystem
				// for the name.
				// It's also *very* heavily dependant on internal libfat structs
				// being exactly as they are now.
				static_assert(_LIBFAT_MAJOR_ == 1 && _LIBFAT_MINOR_ == 1 && _LIBFAT_PATCH_ == 5, "libfat updated! Check that this is still correct");

				// state->currentEntry.entryData[DIR_ENTRY_attributes]
				u8 *state = (u8 *)pdir->dirData->dirStruct;
				attrs = state[4 + 0xB];
			}

			dirent *pent = readdir(pdir);
			if (pent == nullptr || file_count > ((dsiFeatures() || sys().dsDebugRam()) ? 1024 : 512))
				break;

			// Now that we've got the attrs and the name, skip if we should be hiding this
			if (!ms().showHidden && (attrs & ATTR_HIDDEN || (pent->d_name[0] == '.' && strcmp(pent->d_name, "..") != 0)))
				continue;

			if (ms().showDirectories) {
				if ((pent->d_type == DT_DIR && strcmp(pent->d_name, ".") != 0 && strcmp(pent->d_name, "_nds") != 0
					&& strcmp(pent->d_name, "saves") != 0 && strcmp(pent->d_name, "ramdisks") != 0)
					|| nameEndsWith(pent->d_name, extensionList)) {
					dirContents.emplace_back(pent->d_name, pent->d_type == DT_DIR, file_count, false);
					file_count++;
				}
			} else {
				if (pent->d_type != DT_DIR && nameEndsWith(pent->d_name, extensionList)) {
					dirContents.emplace_back(pent->d_name, false, file_count, false);
					file_count++;
				}
			}
		}

		if (ms().sortMethod == TWLSettings::ESortAlphabetical) { // Alphabetical
			std::sort(dirContents.begin(), dirContents.end(), dirEntryPredicate);
		} else if (ms().sortMethod == TWLSettings::ESortRecent) { // Recent
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
		} else if (ms().sortMethod == TWLSettings::ESortMostPlayed) { // Most Played
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
		} else if (ms().sortMethod == TWLSettings::ESortFileType) { // File type
			sort(dirContents.begin(), dirContents.end(), [](const DirEntry &lhs, const DirEntry &rhs) {
					if (!lhs.isDirectory && rhs.isDirectory)
						return false;
					else if (lhs.isDirectory && !rhs.isDirectory)
						return true;

					int extCmp = strcasecmp(lhs.name.substr(lhs.name.find_last_of('.') + 1).c_str(), rhs.name.substr(rhs.name.find_last_of('.') + 1).c_str());
					if (extCmp == 0)
						return strcasecmp(lhs.name.c_str(), rhs.name.c_str()) < 0;
					else
						return extCmp < 0;
				});
		} else if (ms().sortMethod == TWLSettings::ESortCustom) { // Custom
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

void showDirectoryContents (const std::vector<DirEntry>& dirContents, int startRow) {
	getcwd(path, PATH_MAX);

	// Clear the screen
	iprintf ("\x1b[2J");

	// Print the path
	if (ms().theme != TWLSettings::EThemeGBC) {
	if (strlen(path) < SCREEN_COLS) {
		iprintf ("%s", path);
	} else {
		iprintf ("%s", path + strlen(path) - SCREEN_COLS);
	}
	}

	if (ms().theme != TWLSettings::EThemeGBC) {
		// Move to 2nd row
		iprintf ("\x1b[1;0H");
		// Print line of dashes
		iprintf ("--------------------------------");
	}

	int screenCols = (ms().theme==6 ? SCREEN_COLS_GBNP : SCREEN_COLS);

	// Print directory listing
	for (int i = 0; i < ((int)dirContents.size() - startRow) && i < (ms().theme==6 ? ENTRIES_PER_SCREEN_GBNP : ENTRIES_PER_SCREEN); i++) {
		const DirEntry* entry = &dirContents.at(i + startRow);
		char entryName[screenCols + 1];
		
		// Set row
		iprintf ("\x1b[%d;%dH", i + (ms().theme==6 ? ENTRIES_START_ROW_GBNP : ENTRIES_START_ROW), (ms().theme==6 ? 7 : 0));
		
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
	if (ms().macroMode) {
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
		bgOperations(true);
	} while (!(pressed & KEY_A));
	clearText();
	showdialogbox = false;
	dialogboxHeight = 0;

	if (ms().macroMode) {
		lcdMainOnTop();
		lcdSwapped = false;
	}
}

void ramDiskMsg(void) {
	if (ms().macroMode) {
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
		scanKeys();
		pressed = keysDown();
		bgOperations(true);
	} while (!(pressed & KEY_A));
	clearText();
	showdialogbox = false;
	dialogboxHeight = 0;

	if (ms().macroMode) {
		lcdMainOnTop();
		lcdSwapped = false;
	}
}

bool dsiBinariesMissingMsg(void) {
	bool proceedToLaunch = false;

	if (ms().macroMode) {
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
		bgOperations(true);
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

	if (ms().macroMode) {
		lcdMainOnTop();
		lcdSwapped = false;
	}

	return proceedToLaunch;
}

bool donorRomMsg(void) {
	bool proceedToLaunch = true;
	bool dsModeAllowed = ((requiresDonorRom == 52 || requiresDonorRom == 53) && !isDSiWare);

	if (ms().macroMode) {
		lcdMainOnBottom();
		lcdSwapped = true;
	}
	int msgPage = 0;
	bool pageLoaded = false;
	bool secondPageViewed = false;
	dialogboxHeight = 2;
	showdialogbox = true;
	int pressed = 0;
	while (1) {
		if (!pageLoaded) {
			clearText();
			printLargeCentered(false, 74, "Error!");
			if (msgPage == 1) {
				switch (requiresDonorRom) {
					default:
						break;
					case 20:
						printSmallCentered(false, 90, "Find the SDK2.0 title,");
						break;
					case 51:
						printSmallCentered(false, 90, "Find the DSi-Enhanced title,");
						break;
					case 52:
						printSmallCentered(false, 90, "Find the DSi(Ware) title,");
						break;
					case 151:
						printSmallCentered(false, 90, "Find the SDK5.0 DSi-Enhanced title,");
						break;
					case 152:
						printSmallCentered(false, 90, "Find the SDK5.0 DSi(Ware) title,");
						break;
				}
				printSmallCentered(false, 102, "press (Y), and select");
				printSmallCentered(false, 114, "\"Set as Donor ROM\".");
				printSmall(false, 18, 132, "<");
			} else {
				switch (requiresDonorRom) {
					default:
						break;
					case 20:
						printSmallCentered(false, 90, "Please set a different SDK2.0");
						printSmallCentered(false, 102, "title as a donor ROM, in order");
						printSmallCentered(false, 114, "to launch this title.");
						break;
					case 51:
						printSmallCentered(false, 90, "Please set a DSi-Enhanced title");
						printSmallCentered(false, 102, "as a donor ROM, in order");
						printSmallCentered(false, 114, "to launch this title.");
						break;
					case 52:
						printSmallCentered(false, 90, dsModeAllowed ? "Please set a DSi(Ware) title" : "Please set a different DSi(Ware)");
						printSmallCentered(false, 102, dsModeAllowed ? "as a donor ROM, in order" : "title as a donor ROM, in order");
						printSmallCentered(false, 114, dsModeAllowed ? "to launch this title in DSi mode." : "to launch this title.");
						break;
					case 151:
						printSmallCentered(false, 90, "Please set an SDK5.0 DSi-Enhanced");
						printSmallCentered(false, 102, "title as a donor ROM, in order");
						printSmallCentered(false, 114, "to launch this title.");
						break;
					case 152:
						printSmallCentered(false, 90, "Please set a different SDK5.0");
						printSmallCentered(false, 102, "DSi(Ware) title as a donor ROM,");
						printSmallCentered(false, 114, "in order to launch this title.");
						break;
				}
				printSmallRightAlign(false, 256 - 16, 132, ">");
			}
			if (secondPageViewed) {
				printSmallCentered(false, 132, dsModeAllowed ? "(Y) Launch in DS mode  \u2428 Back" : "\u2428 Back");
			}
			pageLoaded = true;
		}
		scanKeys();
		pressed = keysDown();
		bgOperations(true);
		if ((pressed & KEY_LEFT) && msgPage != 0) {
			msgPage = 0;
			pageLoaded = false;
		} else if (((pressed & KEY_RIGHT) || (((pressed & KEY_B) || (pressed & KEY_A)) && !secondPageViewed)) && msgPage != 1) {
			msgPage = 1;
			secondPageViewed = true;
			pageLoaded = false;
		} else if (dsModeAllowed && (pressed & KEY_Y)) {
			dsModeForced = true;
			proceedToLaunch = true;
			pressed = 0;
			break;
		} else if ((pressed & KEY_B) && secondPageViewed) {
			proceedToLaunch = false;
			pressed = 0;
			break;
		}
	}
	clearText();
	showdialogbox = false;
	dialogboxHeight = 0;

	if (ms().macroMode) {
		lcdMainOnTop();
		lcdSwapped = false;
	}

	return proceedToLaunch;
}

void showLocation(void) {
	if (sys().isRegularDS()) return;

	printSmall(false, 8, 162, "Location:");
	if (ms().secondaryDevice) {
		printSmall(false, 8, 174, "Slot-1 microSD Card");
	} else {
		printSmall(false, 8, 174, ms().showMicroSd ? "microSD Card" : "SD Card");
	}
}

bool checkForCompatibleGame(char gameTid[5], const char *filename) {
	bool proceedToLaunch = true;

	if (!dsiFeatures() && ms().secondaryDevice) {
		// TODO: If the list gets large enough, switch to bsearch().
		for (unsigned int i = 0; i < sizeof(incompatibleGameListB4DS)/sizeof(incompatibleGameListB4DS[0]); i++) {
			if (memcmp(gameTid, incompatibleGameListB4DS[i], 3) == 0) {
				// Found match
				proceedToLaunch = false;
				break;
			}
		}
	}

	if (proceedToLaunch && ms().secondaryDevice) {
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

	if (ms().macroMode) {
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
		bgOperations(true);
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
	} else if (ms().macroMode) {
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

void cannotLaunchMsg(void) {
	if (ms().macroMode) {
		lcdMainOnBottom();
		lcdSwapped = true;
	}
	showdialogbox = true;
	printLargeCentered(false, 74, isTwlm ? "Information" : "Error!");
	printSmallCentered(false, 90, isTwlm ? "TWiLight Menu++ is already running." : "This game cannot be launched.");
	printSmallCentered(false, 108, "\u2427 OK");
	int pressed = 0;
	do {
		scanKeys();
		pressed = keysDown();
		bgOperations(true);
	} while (!(pressed & KEY_A));
	showdialogbox = false;
	if (ms().macroMode) {
		lcdMainOnTop();
		lcdSwapped = false;
	}
	for (int i = 0; i < 25; i++) swiWaitForVBlank();
}

bool dsiWareInDSModeMsg(void) {
	if (ms().dontShowDSiWareInDSModeWarning) {
		return true;
	}

	bool proceedToLaunch = true;

	if (ms().macroMode) {
		lcdMainOnBottom();
		lcdSwapped = true;
	}
	dialogboxHeight = 4;
	showdialogbox = true;
	printLargeCentered(false, 74, "Compatibility Warning");
	printSmallCentered(false, 90, "You are attempting to launch a DSiWare");
	printSmallCentered(false, 102, "title in DS mode on a DSi or 3DS system.");
	printSmallCentered(false, 114, "For increased compatibility, and saving");
	printSmallCentered(false, 126, "data in more titles, please relaunch");
	printSmallCentered(false, 138, "TWLMenu++ from the console's SD Card slot.");
	printSmallCentered(false, 154, "\u2428 Return   \u2427 Launch");

	int pressed = 0;
	while (1) {
		scanKeys();
		pressed = keysDown();
		bgOperations(true);
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
		if (pressed & KEY_X) {
			ms().dontShowDSiWareInDSModeWarning = true;
			proceedToLaunch = true;
			pressed = 0;
			break;
		}
	}
	clearText();
	showdialogbox = false;
	dialogboxHeight = 0;

	if (ms().macroMode) {
		lcdMainOnTop();
		lcdSwapped = false;
	}

	return proceedToLaunch;
}

bool dsiWareRAMLimitMsg(char gameTid[5], std::string filename) {
	bool showMsg = false;
	bool mepFound = false;
	int msgId = 0;

	bool b4dsDebugConsole = ((sys().isRegularDS() && sys().dsDebugRam()) || (dsiFeatures() && bs().b4dsMode == 2));

	// Find DSiWare title which requires Slot-2 RAM expansion such as the Memory Expansion Pak
	// TODO: If the list gets large enough, switch to bsearch().
	for (unsigned int i = 0; i < sizeof(compatibleGameListB4DSMEP)/sizeof(compatibleGameListB4DSMEP[0]); i++) {
		if (memcmp(gameTid, compatibleGameListB4DSMEP[i], 3) == 0) {
			// Found match
			if (compatibleGameListB4DSMEPID[i] == 0 && b4dsDebugConsole) {
				// Do nothing
			} else if (sys().isRegularDS()) {
				/*if (*(u16*)0x020000C0 == 0x5A45) {
					showMsg = true;
				} else*/
				if (io_dldi_data->ioInterface.features & FEATURE_SLOT_NDS) {
					u16 hwordBak = *(vu16*)(0x08240000);
					*(vu16*)(0x08240000) = 1; // Detect Memory Expansion Pak
					mepFound = (*(vu16*)(0x08240000) == 1);
					*(vu16*)(0x08240000) = hwordBak;
					showMsg = (!mepFound || (compatibleGameListB4DSMEPID[i] == 2 && *(u16*)0x020000C0 == 0)); // Show message if not found
				}
			} else {
				showMsg = true;
			}
			msgId = (compatibleGameListB4DSMEPID[i] == 2) ? 11 : 10;
			break;
		}
	}
	if (!showMsg) {
		if (b4dsDebugConsole) {
			// TODO: If the list gets large enough, switch to bsearch().
			for (unsigned int i = 0; i < sizeof(compatibleGameListB4DSDebugRAMLimited)/sizeof(compatibleGameListB4DSDebugRAMLimited[0]); i++) {
				if (memcmp(gameTid, compatibleGameListB4DSDebugRAMLimited[i], 3) == 0) {
					// Found match
					showMsg = true;
					msgId = compatibleGameListB4DSDebugRAMLimitedID[i];
					break;
				}
			}
		} else {
			// TODO: If the list gets large enough, switch to bsearch().
			for (unsigned int i = 0; i < sizeof(compatibleGameListB4DSRAMLimited)/sizeof(compatibleGameListB4DSRAMLimited[0]); i++) {
				if (memcmp(gameTid, compatibleGameListB4DSRAMLimited[i], 3) == 0) {
					// Found match
					showMsg = true;
					msgId = compatibleGameListB4DSRAMLimitedID[i];
					break;
				}
			}
		}
	}
	if (!showMsg) {
		// TODO: If the list gets large enough, switch to bsearch().
		for (unsigned int i = 0; i < sizeof(compatibleGameListB4DSAllRAMLimited)/sizeof(compatibleGameListB4DSAllRAMLimited[0]); i++) {
			if (memcmp(gameTid, compatibleGameListB4DSAllRAMLimited[i], 3) == 0) {
				// Found match
				showMsg = true;
				msgId = compatibleGameListB4DSAllRAMLimitedID[i];
				break;
			}
		}
	}

	if (!showMsg || (!checkIfShowRAMLimitMsg(filename) && msgId < 10)) {
		return true;
	}

	bool proceedToLaunch = true;

	if (msgId >= 10 && !sys().isRegularDS()) {
		cannotLaunchMsg();
		return false;
	}

	if (ms().macroMode) {
		lcdMainOnBottom();
		lcdSwapped = true;
	}
	dialogboxHeight = 3;
	showdialogbox = true;
	printLargeCentered(false, 74, "Compatibility Warning");
	switch (msgId) {
		case 0:
			printSmallCentered(false, 90, "Due to memory limitations, only part");
			printSmallCentered(false, 102, "of this game can be played. To play");
			printSmallCentered(false, 114, "the full game, launch this on");
			printSmallCentered(false, 126, "Nintendo DSi or 3DS systems.");
			break;
		case 1:
		case 2:
			printSmallCentered(false, 90, msgId == 2 ? "Due to memory limitations, music" : "Due to memory limitations, audio");
			printSmallCentered(false, 102, "will not be played. To play this");
			printSmallCentered(false, 114, msgId == 2 ? "game with music, launch this on" : "game with audio, launch this on");
			printSmallCentered(false, 126, "Nintendo DSi or 3DS systems.");
			break;
		case 3:
		case 4:
			printSmallCentered(false, 90, "Due to memory limitations, the game");
			printSmallCentered(false, 102, msgId == 4 ? "will crash at certain point(s). To work" : "will crash at a specific area. To work");
			printSmallCentered(false, 114, "around the crash, launch this on");
			printSmallCentered(false, 126, "Nintendo DSi or 3DS systems.");
			break;
		case 5:
			printSmallCentered(false, 90, "Due to memory limitations, FMVs");
			printSmallCentered(false, 102, "will not be played. For playback");
			printSmallCentered(false, 114, "of FMVs, launch this on");
			printSmallCentered(false, 126, "Nintendo DSi or 3DS systems.");
			break;
		case 6:
		case 7:
			printSmallCentered(false, 90, msgId == 7 ? "Due to no save support, the game" : "Due to memory limitations, the game");
			printSmallCentered(false, 102, "will run in a limited state. To play");
			printSmallCentered(false, 114, "the full version, launch this on");
			printSmallCentered(false, 126, "Nintendo DSi or 3DS systems.");
			break;
		case 10:
			printSmallCentered(false, 102, "To launch this title, please");
			printSmallCentered(false, 114, "insert the Memory Expansion Pak.");
			break;
		case 11:
			if (mepFound) {
				printSmallCentered(false, 90, "This title requires a larger amount");
				printSmallCentered(false, 102, "amount of memory than the Expansion Pak.");
				printSmallCentered(false, 114, "Please turn off the POWER, and insert");
				printSmallCentered(false, 126, "a Slot-2 cart with more memory.");
			} else {
				printSmallCentered(false, 90, "To launch this title, please turn off the");
				printSmallCentered(false, 102, "POWER, and insert a Slot-2 memory expansion");
				printSmallCentered(false, 114, "cart which isn't the Memory Expansion Pak.");
			}
			break;
	}
	if (msgId >= 10) {
		printSmallCentered(false, 142, "\u2427 OK");
	} else {
		printSmallCentered(false, 142, "\u2428 Return   \u2427 Launch");
	}

	int pressed = 0;
	if (msgId >= 10) {
		while (1) {
			scanKeys();
			pressed = keysDown();
			bgOperations(true);
			if ((pressed & KEY_A) || (pressed & KEY_B)) {
				proceedToLaunch = false;
				pressed = 0;
				break;
			}
		}
	} else {
		while (1) {
			scanKeys();
			pressed = keysDown();
			bgOperations(true);
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
			if (pressed & KEY_X) {
				dontShowRAMLimitMsgAgain(filename);
				proceedToLaunch = true;
				pressed = 0;
				break;
			}
		}
	}
	clearText();
	showdialogbox = false;
	dialogboxHeight = 0;

	if (proceedToLaunch) {
		titleUpdate(false, filename.c_str());
		showLocation();
	} else if (ms().macroMode) {
		lcdMainOnTop();
		lcdSwapped = false;
	}

	return proceedToLaunch;
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
	if (!res && (sys().dsDebugRam() || bs().b4dsMode == 2)) {
		for (unsigned int i = 0; i < sizeof(compatibleGameListB4DSDebug)/sizeof(compatibleGameListB4DSDebug[0]); i++) {
			if (memcmp(game_TID, compatibleGameListB4DSDebug[i], (compatibleGameListB4DSDebug[i][3] != 0 ? 4 : 3)) == 0) {
				// Found match
				res = true;
				break;
			}
		}
	}
	return res;
}

std::string browseForFile(const std::vector<std::string_view> extensionList) {
	if (ms().macroMode) {
		lcdMainOnTop();
		lcdSwapped = false;
	}

	gameOrderIniPath = std::string(sys().isRunFromSD() ? "sd" : "fat") + ":/_nds/TWiLightMenu/extras/gameorder.ini";
	recentlyPlayedIniPath = std::string(sys().isRunFromSD() ? "sd" : "fat") + ":/_nds/TWiLightMenu/extras/recentlyplayed.ini";
	timesPlayedIniPath = std::string(sys().isRunFromSD() ? "sd" : "fat") + ":/_nds/TWiLightMenu/extras/timesplayed.ini";

	int pressed = 0;
	int screenOffset = 0;
	int fileOffset = 0;
	std::vector<DirEntry> dirContents;
	
	getDirectoryContents (dirContents, extensionList);
	showDirectoryContents (dirContents, screenOffset);
	
	whiteScreen = false;
	fadeType = true;	// Fade in from white

	fileOffset = ms().cursorPosition[ms().secondaryDevice];
	if (ms().pagenum[ms().secondaryDevice] > 0) {
		fileOffset += ms().pagenum[ms().secondaryDevice]*40;
	}

	while (true) {
		if (fileOffset < 0) 	fileOffset = dirContents.size() - 1;		// Wrap around to bottom of list
		if (fileOffset > ((int)dirContents.size() - 1))		fileOffset = 0;		// Wrap around to top of list

		int entriesStartRow = (ms().theme==6 ? ENTRIES_START_ROW_GBNP : ENTRIES_START_ROW);
		int entriesPerScreen = (ms().theme==6 ? ENTRIES_PER_SCREEN_GBNP : ENTRIES_PER_SCREEN);

		// Clear old cursors
		for (int i = entriesStartRow; i < entriesPerScreen + entriesStartRow; i++) {
			iprintf ("\x1b[%d;%dH ", i, (ms().theme==6 ? 7 : 0));
			if (ms().theme==6) {
				iprintf ("\x1b[%d;24H ", i);
			}
		}
		// Show cursor
		if (ms().theme==6) {
			iprintf ("\x1B[43m");		// Print foreground yellow color
		}
		iprintf ("\x1b[%d;%dH", fileOffset - screenOffset + entriesStartRow, (ms().theme==6 ? 7 : 0));
		iprintf ("%s", (ms().theme==6 ? "<" : "*"));
		if (ms().theme==6) {
			iprintf ("\x1b[%d;24H>", fileOffset - screenOffset + entriesStartRow);
			iprintf ("\x1B[47m");		// Print foreground white color
		}

		if (dirContents.at(fileOffset).isDirectory) {
			isDirectory = true;
			bnrWirelessIcon = 0;
		} else {
			isDirectory = false;
			std::string std_romsel_filename = dirContents.at(fileOffset).name.c_str();
			getGameInfo(isDirectory, dirContents.at(fileOffset).name.c_str());

			if (extension(std_romsel_filename, {".nds", ".dsi", ".ids", ".srl", ".app", ".argv"})) {
				bnrRomType = 0;
			} else if (extension(std_romsel_filename, {".xex", ".atr", ".a26", ".a52", ".a78"})) {
				bnrRomType = 10;
			} else if (extension(std_romsel_filename, {".col"})) {
				bnrRomType = 13;
			} else if (extension(std_romsel_filename, {".m5"})) {
				bnrRomType = 14;
			} else if (extension(std_romsel_filename, {".int"})) {
				bnrRomType = 12;
			} else if (extension(std_romsel_filename, {".plg"})) {
				bnrRomType = 9;
			} else if (extension(std_romsel_filename, {".avi", ".rvid", ".fv"})) {
				bnrRomType = 19;
			} else if (extension(std_romsel_filename, {".gif", ".bmp", ".png"})) {
				bnrRomType = 20;
			} else if (extension(std_romsel_filename, {".agb", ".gba", ".mb"})) {
				bnrRomType = 1;
			} else if (extension(std_romsel_filename, {".gb", ".sgb"})) {
				bnrRomType = 2;
			} else if (extension(std_romsel_filename,{ ".gbc"})) {
				bnrRomType = 3;
			} else if (extension(std_romsel_filename, {".nes", ".fds"})) {
				bnrRomType = 4;
			} else if (extension(std_romsel_filename, {".sg"})) {
				bnrRomType = 15;
			} else if (extension(std_romsel_filename, {".sms"})) {
				bnrRomType = 5;
			} else if (extension(std_romsel_filename, {".gg"})) {
				bnrRomType = 6;
			} else if (extension(std_romsel_filename, {".gen"})) {
				bnrRomType = 7;
			} else if (extension(std_romsel_filename, {".smc", ".sfc"})) {
				bnrRomType = 8;
			} else if (extension(std_romsel_filename, {".pce"})) {
				bnrRomType = 11;
			} else if (extension(std_romsel_filename, {".ws", ".wsc"})) {
				bnrRomType = 16;
			} else if (extension(std_romsel_filename, {".ngp", ".ngc"})) {
				bnrRomType = 17;
			} else if (extension(std_romsel_filename, {".dsk"})) {
				bnrRomType = 18;
			} else {
				bnrRomType = 9;
			}
		}

		if (bnrRomType != 0) {
			bnrWirelessIcon = 0;
			isTwlm = false;
			isDSiWare = false;
			isHomebrew = 0;
		}

		iconUpdate (dirContents.at(fileOffset).isDirectory,dirContents.at(fileOffset).name.c_str());
		titleUpdate (dirContents.at(fileOffset).isDirectory,dirContents.at(fileOffset).name.c_str());

		showLocation();

		// Power saving loop. Only poll the keys once per frame and sleep the CPU if there is nothing else to do
		do {
			scanKeys();
			pressed = keysDownRepeat();
			bgOperations(true);
		} while (!pressed);

		if (pressed & KEY_UP) {
			fileOffset -= 1;
			if (ms().theme == TWLSettings::EThemeGBC) {
				snd().playSelect();
			}
		}
		if (pressed & KEY_DOWN) {
			fileOffset += 1;
			if (ms().theme == TWLSettings::EThemeGBC) {
				snd().playSelect();
			}
		}
		if (pressed & KEY_LEFT) {
			fileOffset -= ENTRY_PAGE_LENGTH;
			if (ms().theme == TWLSettings::EThemeGBC) {
				snd().playSelect();
			}
		}
		if (pressed & KEY_RIGHT) {
			fileOffset += ENTRY_PAGE_LENGTH;
			if (ms().theme == TWLSettings::EThemeGBC) {
				snd().playSelect();
			}
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
				if (ms().theme == TWLSettings::EThemeGBC) {
					snd().playSelect();

					// Clear the screen
					iprintf ("\x1b[2J");

					iprintf ("\x1b[6;8H");
				}
				iprintf("Entering directory\n");
				// Enter selected directory
				chdir (entry->name.c_str());
				char buf[256];
				ms().romfolder[ms().secondaryDevice] = getcwd(buf, 256);
				ms().cursorPosition[ms().secondaryDevice] = 0;
				ms().pagenum[ms().secondaryDevice] = 0;
				ms().saveSettings();

				return "null";
			} else if (isTwlm || (isDSiWare && ((((!dsiFeatures() && (!sdFound() || !ms().dsiWareToSD)) || bs().b4dsMode) && ms().secondaryDevice && !dsiWareCompatibleB4DS(dirContents.at(fileOffset).name.c_str()))
			|| (isDSiMode() && memcmp(io_dldi_data->friendlyName, "CycloDS iEvolution", 18) != 0 && sys().arm7SCFGLocked() && !sys().dsiWramAccess() && !gameCompatibleMemoryPit(dirContents.at(fileOffset).name.c_str()))))) {
				cannotLaunchMsg();
			} else {
				loadPerGameSettings(dirContents.at(fileOffset).name);
				int hasAP = 0;
				bool proceedToLaunch = true;
				bool useBootstrapAnyway = ((perGameSettings_useBootstrap == -1 ? ms().useBootstrap : perGameSettings_useBootstrap) || !ms().secondaryDevice);
				if (useBootstrapAnyway && bnrRomType == 0 && !isDSiWare
				 && isHomebrew == 0
				 && checkIfDSiMode(dirContents.at(fileOffset).name)) {
					bool hasDsiBinaries = true;
					if (dsiFeatures() && (!ms().secondaryDevice || !bs().b4dsMode)) {
						FILE *f_nds_file = fopen(dirContents.at(fileOffset).name.c_str(), "rb");
						hasDsiBinaries = checkDsiBinaries(f_nds_file);
						fclose(f_nds_file);
					}

					if (!hasDsiBinaries) {
						proceedToLaunch = dsiBinariesMissingMsg();
					}
				}
				if (proceedToLaunch && (useBootstrapAnyway || ((!dsiFeatures() || bs().b4dsMode) && isDSiWare)) && bnrRomType == 0 && !dsModeForced && isHomebrew == 0) {
					FILE *f_nds_file = fopen(dirContents.at(fileOffset).name.c_str(), "rb");
					char game_TID[5];
					grabTID(f_nds_file, game_TID);
					game_TID[4] = 0;
					fclose(f_nds_file);

					proceedToLaunch = checkForCompatibleGame(game_TID, dirContents.at(fileOffset).name.c_str());
					if (proceedToLaunch && requiresDonorRom) {
						const char* pathDefine = "DONORTWL_NDS_PATH"; // SDK5.x
						if (requiresDonorRom == 52) {
							pathDefine = "DONORTWLONLY_NDS_PATH"; // SDK5.x
						} else if (requiresDonorRom > 100) {
							pathDefine = "DONORTWL0_NDS_PATH"; // SDK5.0
							if (requiresDonorRom == 152) {
								pathDefine = "DONORTWLONLY0_NDS_PATH"; // SDK5.0
							}
						} else if (requiresDonorRom == 20) {
							pathDefine = "DONOR20_NDS_PATH"; // SDK2.0
						}
						std::string donorRomPath;
						const char *bootstrapinipath = sys().isRunFromSD() ? BOOTSTRAP_INI : BOOTSTRAP_INI_FC;
						int dsiModeSetting = (perGameSettings_dsiMode == -1 ? DEFAULT_DSI_MODE : perGameSettings_dsiMode);
						CIniFile bootstrapini(bootstrapinipath);
						donorRomPath = bootstrapini.GetString("NDS-BOOTSTRAP", pathDefine, "");
						bool donorRomFound = (((!dsiFeatures() || bs().b4dsMode) && requiresDonorRom != 20 && ms().secondaryDevice && access("fat:/_nds/nds-bootstrap/b4dsTwlDonor.bin", F_OK) == 0)
											|| strncmp(donorRomPath.c_str(), "nand:", 5) == 0 || (donorRomPath != "" && access(donorRomPath.c_str(), F_OK) == 0));
						if (!donorRomFound && requiresDonorRom != 20 && requiresDonorRom < 100) {
							pathDefine = "DONORTWL0_NDS_PATH"; // SDK5.0
							if (requiresDonorRom == 52) {
								pathDefine = "DONORTWLONLY0_NDS_PATH"; // SDK5.0
							}
							donorRomPath = bootstrapini.GetString("NDS-BOOTSTRAP", pathDefine, "");
							donorRomFound = (strncmp(donorRomPath.c_str(), "nand:", 5) == 0 || (donorRomPath != "" && access(donorRomPath.c_str(), F_OK) == 0));
						}
						if (!donorRomFound
						&& (requiresDonorRom == 20 || requiresDonorRom == 51 || requiresDonorRom == 151
						|| (requiresDonorRom == 52 && (isDSiWare || dsiModeSetting > 0)) || requiresDonorRom == 152)
						) {
							proceedToLaunch = donorRomMsg();
						}
					}
					if (proceedToLaunch && !isDSiWare && checkIfShowAPMsg(dirContents.at(fileOffset).name)) {
						FILE *f_nds_file = fopen(dirContents.at(fileOffset).name.c_str(), "rb");
						hasAP = checkRomAP(f_nds_file);
						fclose(f_nds_file);
					}
					if (proceedToLaunch && isDSiWare && (!dsiFeatures() || bs().b4dsMode) && ms().secondaryDevice) {
						if (!dsiFeatures() && !sys().isRegularDS()) {
							proceedToLaunch = dsiWareInDSModeMsg();
						}
						if (proceedToLaunch) {
							proceedToLaunch = dsiWareRAMLimitMsg(game_TID, dirContents.at(fileOffset).name);
						}
					}
				} else if (isHomebrew == 1) {
					loadPerGameSettings(dirContents.at(fileOffset).name);
					if (requiresRamDisk && perGameSettings_ramDiskNo == -1) {
						proceedToLaunch = false;
						ramDiskMsg();
					}
				} else if (bnrRomType == 7) {
					if (ms().mdEmulator==1 && getFileSize(dirContents.at(fileOffset).name.c_str()) > 0x300000) {
						proceedToLaunch = false;
						mdRomTooBig();
					}
				} else if ((bnrRomType == 8 || (bnrRomType == 11 && ms().smsGgInRam))
							&& isDSiMode() && memcmp(io_dldi_data->friendlyName, "CycloDS iEvolution", 18) != 0 && sys().arm7SCFGLocked()) {
					proceedToLaunch = false;
					cannotLaunchMsg();
				}

				if (hasAP > 0) {
					if (ms().macroMode) {
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
						scanKeys();
						pressed = keysDown();
						bgOperations(true);
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
					} else if (ms().macroMode) {
						lcdMainOnTop();
						lcdSwapped = false;
					}
				}

				// If SD card's cluster size is less than 32KB, then show warning for DS games with nds-bootstrap
				extern struct statvfs st[2];
				if ((useBootstrapAnyway || isDSiWare) && bnrRomType == 0 && (!isDSiWare || (ms().secondaryDevice && (!sdFound() || !ms().dsiWareToSD || bs().b4dsMode))) && isHomebrew == 0
				 && proceedToLaunch && st[ms().secondaryDevice].f_bsize < (32 << 10) && !ms().dontShowClusterWarning) {
					if (ms().macroMode) {
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
						scanKeys();
						pressed = keysDown();
						bgOperations(true);
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
							ms().dontShowClusterWarning = true;
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
					} else if (ms().macroMode) {
						lcdMainOnTop();
						lcdSwapped = false;
					}
				}

				if (proceedToLaunch) {
					if (ms().theme == TWLSettings::EThemeGBC) {
						snd().playLaunch();
						snd().stopStream();
						for (int i = 0; i < 25; i++) swiWaitForVBlank();
					}
					applaunch = true;

					ms().cursorPosition[ms().secondaryDevice] = fileOffset;
					ms().pagenum[ms().secondaryDevice] = 0;
					for (int i = 0; i < 100; i++) {
						if (ms().cursorPosition[ms().secondaryDevice] > 39) {
							ms().cursorPosition[ms().secondaryDevice] -= 40;
							ms().pagenum[ms().secondaryDevice]++;
						} else {
							break;
						}
					}

					if (ms().updateRecentlyPlayedList) {
						dialogboxHeight = 2;
						showdialogbox = true;

						printLargeCentered(false, 74, "Now saving...");

						printSmallCentered(false, 0, 98, "If this crashes with an error, please");
						printSmallCentered(false, 0, 110, "disable \"Update recently played list\".");

						mkdir(sys().isRunFromSD() ? "sd:/_nds/TWiLightMenu/extras" : "fat:/_nds/TWiLightMenu/extras", 0777);

						CIniFile recentlyPlayedIni(recentlyPlayedIniPath);
						std::vector<std::string> recentlyPlayed;

						getcwd(path, PATH_MAX);
						recentlyPlayedIni.GetStringVector("RECENT", path, recentlyPlayed, ':'); // : isn't allowed in FAT-32 names, so its a good deliminator

						std::vector<std::string>::iterator it = std::find(recentlyPlayed.begin(), recentlyPlayed.end(), entry->name);
						if (it != recentlyPlayed.end()) {
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
					if (ms().theme == TWLSettings::EThemeGBC) {
						gbnpBottomInfo();
					}
					for (int i = 0; i < 25; i++) swiWaitForVBlank();
				}
			}
		}

		if ((pressed & KEY_R) && bothSDandFlashcard()) {
			consoleClear();
			if (ms().theme == TWLSettings::EThemeGBC) {
				iprintf ("\x1b[6;8H");
			}
			printf("Please wait...\n");
			ms().cursorPosition[ms().secondaryDevice] = fileOffset;
			ms().pagenum[ms().secondaryDevice] = 0;
			for (int i = 0; i < 100; i++) {
				if (ms().cursorPosition[ms().secondaryDevice] > 39) {
					ms().cursorPosition[ms().secondaryDevice] -= 40;
					ms().pagenum[ms().secondaryDevice]++;
				} else {
					break;
				}
			}
			ms().secondaryDevice = !ms().secondaryDevice;
			ms().saveSettings();
			return "null";
		}

		if ((pressed & KEY_B) && ms().showDirectories) {
			// Go up a directory
			chdir ("..");
			char buf[256];
			ms().romfolder[ms().secondaryDevice] = getcwd(buf, 256);
			ms().cursorPosition[ms().secondaryDevice] = 0;
			ms().saveSettings();
			return "null";
		}

		if ((pressed & KEY_X) && !ms().kioskMode && !ms().preventDeletion && dirContents.at(fileOffset).name != "..") {
			if (ms().macroMode) {
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
				printLargeCentered(false, 74, "Title Management options");
				printSmallCentered(false, 98, "What would you like");
				printSmallCentered(false, 110, "to do with this title?");
			}

			for (int i = 0; i < 90; i++) swiWaitForVBlank();

			if (isTwlm || isDirectory) {
				if (unHide)	printSmallCentered(false, 128, "Y: Unhide  \u2428 Nothing");
				else		printSmallCentered(false, 128, "Y: Hide    \u2428 Nothing");
			} else {
				if (unHide)	printSmallCentered(false, 128, "Y: Unhide  \u2427 Delete  \u2428 Nothing");
				else		printSmallCentered(false, 128, "Y: Hide   \u2427 Delete   \u2428 Nothing");
			}

			while (1) {
				do {
					scanKeys();
					pressed = keysDown();
					bgOperations(true);
				} while (!pressed);
				
				if (((pressed & KEY_A) && !isTwlm && !isDirectory) || (pressed & KEY_Y)) {
					clearText();
					showdialogbox = false;
					consoleClear();
					if (ms().theme == TWLSettings::EThemeGBC) {
						iprintf ("\x1b[6;8H");
					}
					printf("Please wait...\n");

					if (pressed & KEY_A && !isDirectory) {
						remove(dirContents.at(fileOffset).name.c_str());
					} else if (pressed & KEY_Y) {
						// Remove leading . if it exists
						if ((strncmp(entry->name.c_str(), ".", 1) == 0 && entry->name != "..")) {
							rename(entry->name.c_str(), entry->name.substr(1).c_str());
						} else { // Otherwise toggle the hidden attribute bit
							FAT_setAttr(entry->name.c_str(), FAT_getAttr(entry->name.c_str()) ^ ATTR_HIDDEN);
						}
					}
					
					ms().cursorPosition[ms().secondaryDevice] = fileOffset;
					ms().pagenum[ms().secondaryDevice] = 0;
					for (int i = 0; i < 100; i++) {
						if (ms().cursorPosition[ms().secondaryDevice] > 39) {
							ms().cursorPosition[ms().secondaryDevice] -= 40;
							ms().pagenum[ms().secondaryDevice]++;
						} else {
							break;
						}
					}
					ms().saveSettings();
					return "null";
				}

				if (pressed & KEY_B) {
					if (ms().theme == TWLSettings::EThemeGBC) {
						gbnpBottomInfo();
					}
					break;
				}
			}
			clearText();
			showdialogbox = false;

			if (ms().macroMode) {
				lcdMainOnTop();
				lcdSwapped = false;
			}
		}

		if ((ms().theme != TWLSettings::EThemeGBC && (pressed & KEY_START)) || (ms().theme == TWLSettings::EThemeGBC && (pressed & KEY_SELECT) && !ms().kioskMode)) {
			if (ms().theme == TWLSettings::EThemeGBC) {
				snd().playLaunch();
				snd().stopStream();
				for (int i = 0; i < 25; i++) swiWaitForVBlank();
			}
			ms().cursorPosition[ms().secondaryDevice] = fileOffset;
			ms().pagenum[ms().secondaryDevice] = 0;
			for (int i = 0; i < 100; i++) {
				if (ms().cursorPosition[ms().secondaryDevice] > 39) {
					ms().cursorPosition[ms().secondaryDevice] -= 40;
					ms().pagenum[ms().secondaryDevice]++;
				} else {
					break;
				}
			}
			ms().saveSettings();
			consoleClear();
			clearText();
			startMenu = true;
			return "null";		
		}

		if ((pressed & KEY_Y) && !ms().kioskMode && !isTwlm && !isDirectory && (bnrRomType == 0)) {
			ms().cursorPosition[ms().secondaryDevice] = fileOffset;
			perGameSettings(dirContents.at(fileOffset).name);
			if (ms().theme == TWLSettings::EThemeGBC) {
				gbnpBottomInfo();
			}
			for (int i = 0; i < 25; i++) bgOperations(true);
		}

	}
}
