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
#include "common/twlmenusettings.h"
#include "common/bootstrapsettings.h"
#include "common/flashcard.h"
#include "common/systemdetails.h"
#include "common/tonccpy.h"
#include "iconTitle.h"
#include "graphics/fontHandler.h"
#include "graphics/graphics.h"
#include "graphics/FontGraphic.h"
#include "SwitchState.h"
#include "perGameSettings.h"
#include "errorScreen.h"
#include "incompatibleGameMap.h"
#include "compatibleDSiWareMap.h"

#include "gbaswitch.h"
#include "myDSiMode.h"

#include "common/inifile.h"
#include "common/logging.h"

#include "sound.h"
#include "fileCopy.h"

#define SCREEN_COLS 32
#define SCREEN_COLS_GBNP 17
#define ENTRIES_PER_SCREEN 15
#define ENTRIES_PER_SCREEN_GBNP 5
#define ENTRIES_START_ROW 2
#define ENTRIES_START_ROW_GBNP 6
#define ENTRY_PAGE_LENGTH ENTRIES_PER_SCREEN/2
#define ENTRY_PAGE_LENGTH_GBNP ENTRIES_PER_SCREEN_GBNP

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

static int fileStartPos = 0; // The position of the first thing that is not a directory.

char path[PATH_MAX] = {0};

static void gbnpBottomInfo(void) {
	if (ms().theme != TWLSettings::EThemeGBC) {
		return;
	}
	getcwd(path, PATH_MAX);

	clearText(false);

	// Print the path
	printSmall(false, 0, 0, path, Alignment::left, FontPalette::white);

	if (!ms().kioskMode) {
		printSmall(false, 0, 96, "SELECT: Settings menu", Alignment::center, FontPalette::white);
	}

	updateText(false);
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
		// logPrint("Checking for %s extension in %s\n", extension.data(), filename.data());
		if ((strlen(filename.data()) > strlen(extension.data())) && (strcasecmp(filename.substr(filename.size() - extension.size()).data(), extension.data()) == 0)) {
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
	resetPreloadedBannerIcons();

	DIR *pdir = opendir(".");

	if (pdir == nullptr) {
		iprintf("Unable to open the directory.\n");
	} else {
		int file_count = 0;
		fileStartPos = 0;

		bool backFound = false;
		int backPos = 0;
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
			if (pent == nullptr || file_count > ((dsiFeatures() || sys().dsDebugRam()) ? 1024 : 512)) {
				logPrint("End of listing, Sorting: ");
				break;
			}

			// Now that we've got the attrs and the name, skip if we should be hiding this
			if (!ms().showHidden && (attrs & ATTR_HIDDEN || (pent->d_name[0] == '.' && strcmp(pent->d_name, "..") != 0)))
				continue;

			bool emplaceBackDirContent = false;
			if (ms().showDirectories) {
				if (!backFound && (pent->d_type == DT_DIR) && (strcmp(pent->d_name, "..") == 0)) {
					backFound = true;
					backPos = file_count;
					file_count++;
					fileStartPos++;
				}
				emplaceBackDirContent =
				((pent->d_type == DT_DIR && strcmp(pent->d_name, ".") != 0 && strcmp(pent->d_name, "..") != 0 && pent->d_name[0] != '_'
					&& strcmp(pent->d_name, "saves") != 0 && strcmp(pent->d_name, "ramdisks") != 0 && strcmp(pent->d_name, "System Volume Information") != 0)
					|| nameEndsWith(pent->d_name, extensionList));
			} else {
				emplaceBackDirContent = (pent->d_type != DT_DIR && nameEndsWith(pent->d_name, extensionList));
			}
			if (emplaceBackDirContent) {
				if ((pent->d_type != DT_DIR) && extension(pent->d_name, {".md"})) {
					FILE* mdFile = fopen(pent->d_name, "rb");
					if (mdFile) {
						u8 segaEntryPointReversed[4] = {0};
						u8 segaEntryPointU8[4] = {0};
						u32 segaEntryPoint = 0;
						fseek(mdFile, 4, SEEK_SET);
						fread(&segaEntryPointReversed, 1, 4, mdFile);
						for (int i = 0; i < 4; i++) {
							segaEntryPointU8[3-i] = segaEntryPointReversed[i];
						}
						tonccpy(&segaEntryPoint, segaEntryPointU8, 4);

						char segaString[5] = {0};
						fseek(mdFile, 0x100, SEEK_SET);
						fread(segaString, 1, 4, mdFile);
						fclose(mdFile);

						if (!((segaEntryPointReversed[0] == 0) && ((strcmp(segaString, "SEGA") == 0) || ((segaEntryPoint >= 8) && (segaEntryPoint < 0x3FFFFF))))) {
							// Invalid string or entry point found
							continue;
						}
					}
				}
				dirContents.emplace_back(pent->d_name, ms().showDirectories ? (pent->d_type == DT_DIR) : false, file_count, false);
				logPrint("%s listed: %s\n", (pent->d_type == DT_DIR) ? "Directory" : "File", pent->d_name);
				file_count++;

				if (pent->d_type == DT_DIR)
					fileStartPos++;
			}
		}

		if (ms().sortMethod == TWLSettings::ESortAlphabetical) { // Alphabetical
			std::sort(dirContents.begin(), dirContents.end(), dirEntryPredicate);
			logPrint("Alphabetical");
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
			logPrint("Recent");
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
			logPrint("Most Played");
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
			logPrint("File type");
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
			logPrint("Custom");
		}
		logPrint("\n\n");
		if (backFound) {
			dirContents.insert(dirContents.begin(), {"..", true, backPos, false});
		}
		closedir(pdir);
	}
}

void showDirectoryContents (const std::vector<DirEntry>& dirContents, const int startRow, const int fileOffset) {
	getcwd(path, PATH_MAX);

	// Clear the screen
	clearText(true);

	const int xPos = (ms().theme == TWLSettings::EThemeGBC) ? 64 : 1;
	const int yPos = (ms().theme == TWLSettings::EThemeGBC) ? 47 : 12;

	// Print the path
	if (ms().theme != TWLSettings::EThemeGBC) {
		printSmall(true, xPos, 0, path, Alignment::left, FontPalette::black);
	}

	// Print directory listing
	for (int i = 0; i < ((int)dirContents.size() - startRow) && i < (ms().theme==TWLSettings::EThemeGBC ? ENTRIES_PER_SCREEN_GBNP : ENTRIES_PER_SCREEN); i++) {
		const DirEntry* entry = &dirContents.at(i + startRow);
		
		printSmall(true, xPos, yPos+(i*12), entry->isDirectory ? ("["+entry->name+"]") : entry->name, Alignment::left, ((i + startRow) == fileOffset) ? FontPalette::user : FontPalette::white);
	}

	updateText(true);
}

void mdRomTooBig(void) {
	if (ms().macroMode) {
		lcdMainOnBottom();
		lcdSwapped = true;
	}
	dialogboxHeight = 3;
	showdialogbox = true;
	printSmall(false, 0, 74, "Error!", Alignment::center, FontPalette::white);
	printSmall(false, 0, 90, "This SEGA Genesis/Mega Drive", Alignment::center);
	printSmall(false, 0, 102, "ROM cannot be launched,", Alignment::center);
	printSmall(false, 0, 114, "due to its surpassing the", Alignment::center);
	printSmall(false, 0, 126, "size limit of 3MB.", Alignment::center);
	printSmall(false, 0, 144, " OK", Alignment::center);
	updateText(false);
	int pressed = 0;
	do {
		scanKeys();
		pressed = keysDown();
		bgOperations(true);
	} while (!(pressed & KEY_A));
	clearText(false);
	showdialogbox = false;
	dialogboxHeight = 0;
	updateText(false);

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
	printSmall(false, 0, 74, "Error!", Alignment::center, FontPalette::white);
	printSmall(false, 0, 90, "This app requires a", Alignment::center);
	printSmall(false, 0, 102, "RAM disk to work.", Alignment::center);
	printSmall(false, 0, 120, " OK", Alignment::center);
	updateText(false);
	int pressed = 0;
	do {
		scanKeys();
		pressed = keysDown();
		bgOperations(true);
	} while (!(pressed & KEY_A));
	clearText(false);
	showdialogbox = false;
	dialogboxHeight = 0;
	updateText(false);

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
	printSmall(false, 0, 74, "Error!", Alignment::center, FontPalette::white);
	printSmall(false, 0, 90, "The DSi binaries are missing.", Alignment::center);
	printSmall(false, 0, 102, "Please get a clean dump of", Alignment::center);
	printSmall(false, 0, 114, "this ROM, or start in DS mode.", Alignment::center);
	printSmall(false, 0, 132, " Launch in DS mode   Back", Alignment::center);
	updateText(false);
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
	clearText(false);
	showdialogbox = false;
	dialogboxHeight = 0;
	updateText(false);

	if (ms().macroMode) {
		lcdMainOnTop();
		lcdSwapped = false;
	}

	return proceedToLaunch;
}

bool donorRomMsg(void) {
	bool proceedToLaunch = true;
	bool dsModeAllowed = ((requiresDonorRom == 52 || requiresDonorRom == 53) && !isDSiWare);
	bool vramWifi = false;
	if ((!dsiFeatures() || bs().b4dsMode) && ms().secondaryDevice && isDSiWare) {
		// Find DSiWare title which requires VRAM-WiFi donor ROM
		// TODO: If the list gets large enough, switch to bsearch().
		for (unsigned int i = 0; i < sizeof(compatibleGameListB4DSMEP)/sizeof(compatibleGameListB4DSMEP[0]); i++) {
			if (memcmp(gameTid, compatibleGameListB4DSMEP[i], (compatibleGameListB4DSMEP[i][3] != 0 ? 4 : 3)) == 0) {
				// Found match
				vramWifi = (compatibleGameListB4DSMEPID[i] == 3);
				break;
			}
		}
	}

	if (ms().macroMode) {
		lcdMainOnBottom();
		lcdSwapped = true;
	}
	bool ubongo = (memcmp(gameTid, "KUB", 3) == 0);
	int msgPage = 0;
	bool pageLoaded = false;
	bool secondPageViewed = false;
	dialogboxHeight = 2;
	showdialogbox = true;
	int pressed = 0;
	while (1) {
		if (!pageLoaded) {
			clearText(false);
			printSmall(false, 0, 74, "Error!", Alignment::center, FontPalette::white);
			if (msgPage == 1) {
				switch (requiresDonorRom) {
					default:
						break;
					case 20:
						printSmall(false, 0, 90, "Find the SDK2.0 title,", Alignment::center);
						break;
					case 51:
						printSmall(false, 0, 90, ((!dsiFeatures() || bs().b4dsMode) && ms().secondaryDevice && !ubongo) ? (vramWifi ? "Find the VRAM-WiFi SDK5 DS title," : "Find the SDK5 DS title,") : "Find the DSi-Enhanced title,", Alignment::center);
						break;
					case 52:
						printSmall(false, 0, 90, "Find the DSi(Ware) title,", Alignment::center);
						break;
					case 151:
						printSmall(false, 0, 90, "Find the SDK5.0 DSi-Enhanced title,", Alignment::center);
						break;
					case 152:
						printSmall(false, 0, 90, "Find the SDK5.0 DSi(Ware) title,", Alignment::center);
						break;
				}
				printSmall(false, 0, 102, "press (Y), and select", Alignment::center);
				printSmall(false, 0, 114, "\"Set as Donor ROM\".", Alignment::center);
				printSmall(false, 18, 132, "<");
			} else {
				switch (requiresDonorRom) {
					default:
						break;
					case 20:
						printSmall(false, 0, 90, "Please set a different SDK2.0", Alignment::center);
						printSmall(false, 0, 102, "title as a donor ROM, in order", Alignment::center);
						printSmall(false, 0, 114, "to launch this title.", Alignment::center);
						break;
					case 51:
						printSmall(false, 0, 90, ((!dsiFeatures() || bs().b4dsMode) && ms().secondaryDevice && !ubongo) ? "Please set an SDK5 Nintendo DS title" : "Please set a DSi-Enhanced title", Alignment::center);
						printSmall(false, 0, 102, "as a donor ROM, in order", Alignment::center);
						printSmall(false, 0, 114, "to launch this title.", Alignment::center);
						break;
					case 52:
						printSmall(false, 0, 90, dsModeAllowed ? "Please set a DSi(Ware) title" : "Please set a different DSi(Ware)", Alignment::center);
						printSmall(false, 0, 102, dsModeAllowed ? "as a donor ROM, in order" : "title as a donor ROM, in order", Alignment::center);
						printSmall(false, 0, 114, dsModeAllowed ? "to launch this title in DSi mode." : "to launch this title.", Alignment::center);
						break;
					case 151:
						printSmall(false, 0, 90, "Please set an SDK5.0 DSi-Enhanced", Alignment::center);
						printSmall(false, 0, 102, "title as a donor ROM, in order", Alignment::center);
						printSmall(false, 0, 114, "to launch this title.", Alignment::center);
						break;
					case 152:
						printSmall(false, 0, 90, "Please set a different SDK5.0", Alignment::center);
						printSmall(false, 0, 102, "DSi(Ware) title as a donor ROM,", Alignment::center);
						printSmall(false, 0, 114, "in order to launch this title.", Alignment::center);
						break;
				}
				printSmall(false, 256 - 16, 132, ">", Alignment::right);
			}
			if (secondPageViewed) {
				printSmall(false, 0, 132, dsModeAllowed ? "(Y) Launch in DS mode   Back" : " Back", Alignment::center);
			}
			pageLoaded = true;
			updateText(false);
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
	if (sys().isRegularDS() || (ms().theme == TWLSettings::EThemeGBC)) return;

	if (ms().secondaryDevice) {
		printSmall(false, 0, 174, "Location: Slot-1 microSD Card", Alignment::center);
	} else {
		printSmall(false, 0, 174, ms().showMicroSd ? "Location: microSD Card" : "Location: SD Card", Alignment::center);
	}
}

bool checkForCompatibleGame(const char *filename) {
	bool proceedToLaunch = true;

	/* if (!dsiFeatures() && ms().secondaryDevice) {
		// TODO: If the list gets large enough, switch to bsearch().
		for (unsigned int i = 0; i < sizeof(incompatibleGameListB4DS)/sizeof(incompatibleGameListB4DS[0]); i++) {
			if (memcmp(gameTid, incompatibleGameListB4DS[i], 3) == 0) {
				// Found match
				proceedToLaunch = false;
				break;
			}
		}
	} */

	if (ms().secondaryDevice) {
		// TODO: If the list gets large enough, switch to bsearch().
		for (unsigned int i = 0; i < sizeof(incompatibleGameListFC)/sizeof(incompatibleGameListFC[0]); i++) {
			if (memcmp(gameTid, incompatibleGameListFC[i], 3) == 0) {
				// Found match
				proceedToLaunch = false;
				break;
			}
		}
	}

	/* if (proceedToLaunch) {
		// TODO: If the list gets large enough, switch to bsearch().
		for (unsigned int i = 0; i < sizeof(incompatibleGameList)/sizeof(incompatibleGameList[0]); i++) {
			if (memcmp(gameTid, incompatibleGameList[i], 3) == 0) {
				// Found match
				proceedToLaunch = false;
				break;
			}
		}
	} */

	if (proceedToLaunch) return true;	// Game is compatible

	if (ms().macroMode) {
		lcdMainOnBottom();
		lcdSwapped = true;
	}
	dialogboxHeight = 3;
	showdialogbox = true;
	printSmall(false, 0, 74, "Compatibility Warning", Alignment::center, FontPalette::white);
	printSmall(false, 0, 90, "This game is known to not run.", Alignment::center);
	printSmall(false, 0, 102, "If there's an nds-bootstrap", Alignment::center);
	printSmall(false, 0, 114, "version that fixes this,", Alignment::center);
	printSmall(false, 0, 126, "please ignore this message.", Alignment::center);
	printSmall(false, 0, 144, " Ignore    Don't launch", Alignment::center);
	updateText(false);

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
	clearText(false);
	showdialogbox = false;
	dialogboxHeight = 0;
	updateText(false);

	if (proceedToLaunch) {
		titleUpdate(false, filename);
		showLocation();
	} else if (ms().macroMode) {
		lcdMainOnTop();
		lcdSwapped = false;
	}

	return proceedToLaunch;
}

bool gameCompatibleMemoryPit(void) {
	// TODO: If the list gets large enough, switch to bsearch().
	for (unsigned int i = 0; i < sizeof(incompatibleGameListMemoryPit)/sizeof(incompatibleGameListMemoryPit[0]); i++) {
		if (memcmp(gameTid, incompatibleGameListMemoryPit[i], 3) == 0) {
			// Found match
			return false;
		}
	}
	return true;
}

bool checkForGbaBiosRequirement(void) {
	extern bool gbaBiosFound[2];

	if (gbaBiosFound[ms().secondaryDevice]) {
		return false;
	}

	if (ms().gbaR3Test) {
		return true;
	}

	// TODO: If the list gets large enough, switch to bsearch().
	for (unsigned int i = 0; i < sizeof(gbaGameListBiosReqiure)/sizeof(gbaGameListBiosReqiure[0]); i++) {
		if (memcmp(gameTid, gbaGameListBiosReqiure[i], 3) == 0) {
			// Found match
			return true;
		}
	}

	return false;
}

bool cannotLaunchMsg(char tid1) {
	bool res = false;

	if (ms().macroMode) {
		lcdMainOnBottom();
		lcdSwapped = true;
	}
	showdialogbox = true;
	printSmall(false, 0, 74, isTwlm ? "Information" : "Error!", Alignment::center, FontPalette::white);
	if (!isTwlm && bnrRomType == 0 && sys().isRegularDS()) {
		printSmall(false, 0, 90, "For use with Nintendo DSi systems only.", Alignment::center);
	} else if (bnrRomType == 1) {
		printSmall(false, 0, 90, "GBA BIOS is missing!", Alignment::center);
	} else {
		printSmall(false, 0, 90, isTwlm ? "TWiLight Menu++ is already running." : "This game cannot be launched.", Alignment::center);
	}
	printSmall(false, 0, 108, " OK", Alignment::center);
	updateText(false);
	int pressed = 0;
	while (1) {
		scanKeys();
		pressed = keysDown();
		bgOperations(true);

		if (pressed & KEY_A) {
			break;
		}
		if ((pressed & KEY_Y) && bnrRomType == 0 && !isDSiWare && tid1 == 'D') {
			// Hidden button to launch anyways
			res = true;
			break;
		}
	}
	showdialogbox = false;
	if (ms().macroMode) {
		lcdMainOnTop();
		lcdSwapped = false;
	}
	for (int i = 0; i < 25; i++) swiWaitForVBlank();

	return res;
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
	printSmall(false, 0, 74, "Compatibility Warning", Alignment::center, FontPalette::white);
	printSmall(false, 0, 90, "You are attempting to launch a DSiWare", Alignment::center);
	printSmall(false, 0, 102, "title in DS mode on a DSi or 3DS system.", Alignment::center);
	printSmall(false, 0, 114, "For increased compatibility, and saving", Alignment::center);
	printSmall(false, 0, 126, "data in more titles, please relaunch", Alignment::center);
	printSmall(false, 0, 138, "TWLMenu++ from the console's SD Card slot.", Alignment::center);
	printSmall(false, 0, 154, " Return    Launch", Alignment::center);
	updateText(false);

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

bool dsiWareRAMLimitMsg(std::string filename) {
	bool showMsg = false;
	bool mepFound = false;
	int msgId = 0;

	bool b4dsDebugConsole = ((sys().isRegularDS() && sys().dsDebugRam()) || (dsiFeatures() && bs().b4dsMode == 2));

	// Find DSiWare title which requires Slot-2 RAM expansion such as the Memory Expansion Pak
	// TODO: If the list gets large enough, switch to bsearch().
	for (unsigned int i = 0; i < sizeof(compatibleGameListB4DSMEP)/sizeof(compatibleGameListB4DSMEP[0]); i++) {
		if (memcmp(gameTid, compatibleGameListB4DSMEP[i], (compatibleGameListB4DSMEP[i][3] != 0 ? 4 : 3)) == 0) {
			// Found match
			msgId = (compatibleGameListB4DSMEPID[i] == 2) ? 11 : 10;
			if ((compatibleGameListB4DSMEPID[i] == 0 || compatibleGameListB4DSMEPID[i] == 3) && b4dsDebugConsole) {
				// Do nothing
			} else if (compatibleGameListB4DSMEPID[i] == 3) {
				msgId = 12;

				const char *bootstrapinipath = sys().isRunFromSD() ? BOOTSTRAP_INI : BOOTSTRAP_INI_FC;
				CIniFile bootstrapini(bootstrapinipath);
				std::string donorRomPath = bootstrapini.GetString("NDS-BOOTSTRAP", "DONOR5_NDS_PATH_ALT", "");
				const bool donorRomFound = (donorRomPath != "" && access(donorRomPath.c_str(), F_OK) == 0);

				showMsg = !donorRomFound;
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

	if (ms().macroMode) {
		lcdMainOnBottom();
		lcdSwapped = true;
	}
	dialogboxHeight = 3;
	showdialogbox = true;
	printSmall(false, 0, 74, ((msgId == 10 || msgId == 11) && !sys().isRegularDS()) ? "Error!" : "Compatibility Warning", Alignment::center, FontPalette::white);
	switch (msgId) {
		case 0:
			printSmall(false, 0, 90, "Due to memory limitations, only part", Alignment::center);
			printSmall(false, 0, 102, "of this game can be played. To play", Alignment::center);
			printSmall(false, 0, 114, "the full game, launch this on", Alignment::center);
			printSmall(false, 0, 126, "Nintendo DSi or 3DS systems.", Alignment::center);
			break;
		case 1:
		case 2:
			printSmall(false, 0, 90, msgId == 2 ? "Due to memory limitations, music" : "Due to memory limitations, audio", Alignment::center);
			printSmall(false, 0, 102, "will not be played. To play this", Alignment::center);
			printSmall(false, 0, 114, msgId == 2 ? "game with music, launch this on" : "game with audio, launch this on", Alignment::center);
			printSmall(false, 0, 126, "Nintendo DSi or 3DS systems.", Alignment::center);
			break;
		case 3:
		case 4:
			printSmall(false, 0, 90, "Due to memory limitations, the game", Alignment::center);
			printSmall(false, 0, 102, msgId == 4 ? "will crash at certain point(s). To work" : "will crash at a specific area. To work", Alignment::center);
			printSmall(false, 0, 114, "around the crash, launch this on", Alignment::center);
			printSmall(false, 0, 126, "Nintendo DSi or 3DS systems.", Alignment::center);
			break;
		case 5:
			printSmall(false, 0, 90, "Due to memory limitations, FMVs", Alignment::center);
			printSmall(false, 0, 102, "will not be played. For playback", Alignment::center);
			printSmall(false, 0, 114, "of FMVs, launch this on", Alignment::center);
			printSmall(false, 0, 126, "Nintendo DSi or 3DS systems.", Alignment::center);
			break;
		case 6:
		case 7:
			printSmall(false, 0, 90, msgId == 7 ? "Due to no save support, the game" : "Due to memory limitations, the game", Alignment::center);
			printSmall(false, 0, 102, "will run in a limited state. To play", Alignment::center);
			printSmall(false, 0, 114, "the full version, launch this on", Alignment::center);
			printSmall(false, 0, 126, "Nintendo DSi or 3DS systems.", Alignment::center);
			break;
		case 8:
			printSmall(false, 0, 90, "Due to memory limitations, sound effects", Alignment::center);
			printSmall(false, 0, 102, "will not be played. To play this", Alignment::center);
			printSmall(false, 0, 114, "game with sound effects, launch this on", Alignment::center);
			printSmall(false, 0, 126, "Nintendo DSi or 3DS systems.", Alignment::center);
			break;
		case 10:
			if (sys().isRegularDS()) {
				printSmall(false, 0, 102, "To launch this title, please", Alignment::center);
				printSmall(false, 0, 114, "insert the Memory Expansion Pak.", Alignment::center);
			} else {
				printSmall(false, 0, 90, "This title requires the Memory Expansion Pak,", Alignment::center);
				printSmall(false, 0, 102, "but the slot to insert it does not exist.", Alignment::center);
				printSmall(false, 0, 114, "As a result, this title cannot be launched.", Alignment::center);
			}
			break;
		case 11:
			if (sys().isRegularDS()) {
				if (mepFound) {
					printSmall(false, 0, 90, "This title requires a larger amount", Alignment::center);
					printSmall(false, 0, 102, "amount of memory than the Expansion Pak.", Alignment::center);
					printSmall(false, 0, 114, "Please turn off the POWER, and insert", Alignment::center);
					printSmall(false, 0, 126, "a Slot-2 cart with more memory.", Alignment::center);
				} else {
					printSmall(false, 0, 90, "To launch this title, please turn off the", Alignment::center);
					printSmall(false, 0, 102, "POWER, and insert a Slot-2 memory expansion", Alignment::center);
					printSmall(false, 0, 114, "cart which isn't the Memory Expansion Pak.", Alignment::center);
				}
			} else {
				printSmall(false, 0, 90, "This title requires a Slot-2 expansion cart,", Alignment::center);
				printSmall(false, 0, 102, "but the slot to insert it does not exist.", Alignment::center);
				printSmall(false, 0, 114, "As a result, this title cannot be launched.", Alignment::center);
			}
			break;
		case 12:
			printSmall(false, 0, 90, "The currently set donor ROM is incompatible", Alignment::center);
			printSmall(false, 0, 102, "with this title. Please find a VRAM-WiFi", Alignment::center);
			printSmall(false, 0, 114, "SDK5 DS title to set as a donor ROM.", Alignment::center);
			break;
	}
	if (msgId >= 10) {
		printSmall(false, 0, 142, " OK", Alignment::center);
	} else {
		printSmall(false, 0, 142, " Return    Launch", Alignment::center);
	}
	updateText(false);

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
	clearText(false);
	showdialogbox = false;
	dialogboxHeight = 0;
	updateText(false);

	if (proceedToLaunch) {
		titleUpdate(false, filename.c_str());
		showLocation();
	} else if (ms().macroMode) {
		lcdMainOnTop();
		lcdSwapped = false;
	}

	return proceedToLaunch;
}

bool dsiWareCompatibleB4DS(void) {
	if (memcmp(gameTid, "NTRJ", 4) == 0) {
		return true; // No check necessary for NTRJ titles (They are uncommon, and "NTRJ" is not seen in retail titles)
	}

	bool res = false;

	// TODO: If the list gets large enough, switch to bsearch().
	for (unsigned int i = 0; i < sizeof(compatibleGameListB4DS)/sizeof(compatibleGameListB4DS[0]); i++) {
		if (memcmp(gameTid, compatibleGameListB4DS[i], (compatibleGameListB4DS[i][3] != 0 ? 4 : 3)) == 0) {
			// Found match
			res = true;
			break;
		}
	}
	if (!res && (sys().dsDebugRam() || bs().b4dsMode == 2)) {
		for (unsigned int i = 0; i < sizeof(compatibleGameListB4DSDebug)/sizeof(compatibleGameListB4DSDebug[0]); i++) {
			if (memcmp(gameTid, compatibleGameListB4DSDebug[i], (compatibleGameListB4DSDebug[i][3] != 0 ? 4 : 3)) == 0) {
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

	const int entriesPerScreen = (ms().theme==6 ? ENTRIES_PER_SCREEN_GBNP : ENTRIES_PER_SCREEN);

	fileOffset = ms().cursorPosition[ms().secondaryDevice];
	if (ms().pagenum[ms().secondaryDevice] > 0) {
		fileOffset += ms().pagenum[ms().secondaryDevice]*40;
	}

	// Scroll screen if needed
	if (fileOffset > screenOffset + entriesPerScreen - 1) {
		screenOffset = fileOffset - entriesPerScreen + 1;
	}

	showDirectoryContents (dirContents, screenOffset, fileOffset);

	whiteScreen = false;
	fadeType = true;	// Fade in from white

	while (true) {
		if (fileOffset < 0) 	fileOffset = dirContents.size() - 1;		// Wrap around to bottom of list
		if (fileOffset > ((int)dirContents.size() - 1))		fileOffset = 0;		// Wrap around to top of list

		getGameInfo(fileOffset, dirContents.at(fileOffset).isDirectory, dirContents.at(fileOffset).name.c_str(), false);

		if (dirContents.at(fileOffset).isDirectory) {
			isDirectory = true;
		} else {
			isDirectory = false;
			std::string std_romsel_filename = dirContents.at(fileOffset).name.c_str();

			if (extension(std_romsel_filename, {".nds", ".dsi", ".ids", ".srl", ".app", ".argv"})) {
				bnrRomType = 0;
			} else if (extension(std_romsel_filename, {".xex", ".atr", ".a26", ".a52", ".a78"})) {
				bnrRomType = 10;
			} else if (extension(std_romsel_filename, {".msx"})) {
				bnrRomType = 21;
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
			} else if (extension(std_romsel_filename, {".sg", ".sc"})) {
				bnrRomType = 15;
			} else if (extension(std_romsel_filename, {".sms"})) {
				bnrRomType = 5;
			} else if (extension(std_romsel_filename, {".gg"})) {
				bnrRomType = 6;
			} else if (extension(std_romsel_filename, {".gen", ".md"})) {
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
			} else if (extension(std_romsel_filename, {".min"})) {
				bnrRomType = 22;
			} else if (extension(std_romsel_filename, {".ntrb"})) {
				bnrRomType = 23;
			} else {
				bnrRomType = 9;
			}
		}

		if (bnrRomType != 0) {
			bnrWirelessIcon = 0;
			isValid = true;
			isTwlm = false;
			isDSiWare = false;
			isHomebrew = 0;
		}

		iconUpdate (dirContents.at(fileOffset).isDirectory,dirContents.at(fileOffset).name.c_str());
		titleUpdate (dirContents.at(fileOffset).isDirectory,dirContents.at(fileOffset).name.c_str()); // clearText(false) is run

		showLocation();

		updateText(false);
		if (ms().theme == TWLSettings::EThemeGBC) {
			updateText(true); // Update banner text
		}

		// Power saving loop. Only poll the keys once per frame and sleep the CPU if there is nothing else to do
		do {
			scanKeys();
			pressed = keysDownRepeat();
			bgOperations(true);
		} while (!pressed);

		bool refreshDirectoryContents = false;

		if (pressed & KEY_UP) {
			fileOffset -= 1;
			if (ms().theme == TWLSettings::EThemeGBC) {
				snd().playSelect();
			}
			refreshDirectoryContents = true;
		}
		if (pressed & KEY_DOWN) {
			fileOffset += 1;
			if (ms().theme == TWLSettings::EThemeGBC) {
				snd().playSelect();
			}
			refreshDirectoryContents = true;
		}
		if (pressed & KEY_LEFT) {
			fileOffset -= (ms().theme == TWLSettings::EThemeGBC) ? ENTRY_PAGE_LENGTH_GBNP : ENTRY_PAGE_LENGTH;
			if (ms().theme == TWLSettings::EThemeGBC) {
				snd().playSelect();
			}
			refreshDirectoryContents = true;
		}
		if (pressed & KEY_RIGHT) {
			fileOffset += (ms().theme == TWLSettings::EThemeGBC) ? ENTRY_PAGE_LENGTH_GBNP : ENTRY_PAGE_LENGTH;
			if (ms().theme == TWLSettings::EThemeGBC) {
				snd().playSelect();
			}
			refreshDirectoryContents = true;
		}

		if (fileOffset < 0) 	fileOffset = dirContents.size() - 1;		// Wrap around to bottom of list
		if (fileOffset > ((int)dirContents.size() - 1))		fileOffset = 0;		// Wrap around to top of list

		// Scroll screen if needed
		if (fileOffset < screenOffset) {
			screenOffset = fileOffset;
		}
		if (fileOffset > screenOffset + entriesPerScreen - 1) {
			screenOffset = fileOffset - entriesPerScreen + 1;
		}

		if (pressed & KEY_A) {
			DirEntry* entry = &dirContents.at(fileOffset);
			if (entry->isDirectory) {
				if (ms().theme == TWLSettings::EThemeGBC) {
					snd().playSelect();
				}
				// Enter selected directory
				chdir (entry->name.c_str());
				char buf[256];
				ms().romfolder[ms().secondaryDevice] = getcwd(buf, 256);
				ms().cursorPosition[ms().secondaryDevice] = 0;
				ms().pagenum[ms().secondaryDevice] = 0;
				ms().saveSettings();

				return "null";
			} else {
				if (isValid && !isTwlm) {
					loadPerGameSettings(dirContents.at(fileOffset).name);
				}
				int hasAP = 0;
				bool proceedToLaunch = true;

				if (!isValid || isTwlm || (!isDSiWare && (!dsiFeatures() || bs().b4dsMode) && ms().secondaryDevice && bnrRomType == 0 && gameTid[0] == 'D' && romUnitCode == 3 && requiresDonorRom != 51)
				|| (isDSiWare && ((((!dsiFeatures() && (!sdFound() || !ms().dsiWareToSD)) || bs().b4dsMode) && ms().secondaryDevice && !dsiWareCompatibleB4DS())
				|| (isDSiMode() && memcmp(io_dldi_data->friendlyName, "CycloDS iEvolution", 18) != 0 && sys().arm7SCFGLocked() && !sys().dsiWramAccess() && !gameCompatibleMemoryPit())))
				|| (bnrRomType == 1 && (!ms().secondaryDevice || dsiFeatures() || ms().gbaBooter == TWLSettings::EGbaGbar2) && checkForGbaBiosRequirement())) {
					proceedToLaunch = cannotLaunchMsg(gameTid[0]);
				}
				bool useBootstrapAnyway = ((perGameSettings_useBootstrap == -1 ? ms().useBootstrap : perGameSettings_useBootstrap) || !ms().secondaryDevice);
				if (proceedToLaunch && useBootstrapAnyway && bnrRomType == 0 && !isDSiWare
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
					proceedToLaunch = checkForCompatibleGame(dirContents.at(fileOffset).name.c_str());
					if (proceedToLaunch && requiresDonorRom) {
						const char* pathDefine = "DONORTWL_NDS_PATH"; // SDK5.x (TWL)
						if (requiresDonorRom == 52) {
							pathDefine = "DONORTWLONLY_NDS_PATH"; // SDK5.x (TWL)
						} else if (requiresDonorRom > 100) {
							pathDefine = "DONORTWL0_NDS_PATH"; // SDK5.0 (TWL)
							if (requiresDonorRom == 152) {
								pathDefine = "DONORTWLONLY0_NDS_PATH"; // SDK5.0 (TWL)
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
							pathDefine = "DONORTWL0_NDS_PATH"; // SDK5.0 (TWL)
							if (requiresDonorRom == 52) {
								pathDefine = "DONORTWLONLY0_NDS_PATH"; // SDK5.0 (TWL)
							}
							donorRomPath = bootstrapini.GetString("NDS-BOOTSTRAP", pathDefine, "");
							donorRomFound = (strncmp(donorRomPath.c_str(), "nand:", 5) == 0 || (donorRomPath != "" && access(donorRomPath.c_str(), F_OK) == 0));
						}
						if (!donorRomFound && (!dsiFeatures() || bs().b4dsMode) && ms().secondaryDevice && memcmp(gameTid, "KUB", 3) != 0 && requiresDonorRom != 20) {
							pathDefine = "DONOR5_NDS_PATH"; // SDK5.x (NTR)
							donorRomPath = bootstrapini.GetString("NDS-BOOTSTRAP", pathDefine, "");
							donorRomFound = (donorRomPath != "" && access(donorRomPath.c_str(), F_OK) == 0);
							if (!donorRomFound) {
								pathDefine = "DONOR5_NDS_PATH_ALT"; // SDK5.x (NTR)
								donorRomPath = bootstrapini.GetString("NDS-BOOTSTRAP", pathDefine, "");
								donorRomFound = (donorRomPath != "" && access(donorRomPath.c_str(), F_OK) == 0);
							}
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
						hasAP = checkRomAP(f_nds_file, dirContents.at(fileOffset).name.c_str());
						fclose(f_nds_file);
					}
					if (proceedToLaunch && isDSiWare && (!dsiFeatures() || bs().b4dsMode) && ms().secondaryDevice) {
						if (!dsiFeatures() && !sys().isRegularDS()) {
							proceedToLaunch = dsiWareInDSModeMsg();
						}
						if (proceedToLaunch) {
							proceedToLaunch = dsiWareRAMLimitMsg(dirContents.at(fileOffset).name);
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
					proceedToLaunch = cannotLaunchMsg(0);
				}

				if (hasAP > 0) {
					if (ms().macroMode) {
						lcdMainOnBottom();
						lcdSwapped = true;
					}

					dialogboxHeight = 3;
					showdialogbox = true;
					printSmall(false, 0, 74, "Anti-Piracy Warning", Alignment::center, FontPalette::white);
					printSmall(false, 0, 98, "This game has AP. Please make", Alignment::center);
					printSmall(false, 0, 110, "sure you're using the latest", Alignment::center);
					printSmall(false, 0, 122, "version of nds-bootstrap.", Alignment::center);
					printSmall(false, 0, 142, " Return    Launch", Alignment::center);
					updateText(false);

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
					clearText(false);
					showdialogbox = false;
					dialogboxHeight = 0;

					if (proceedToLaunch) {
						titleUpdate (dirContents.at(fileOffset).isDirectory,dirContents.at(fileOffset).name.c_str());
						showLocation();
						updateText(false);
					} else if (ms().macroMode) {
						updateText(false);
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
					clearText(false);
					titleUpdate(dirContents.at(fileOffset).isDirectory,dirContents.at(fileOffset).name.c_str());

					printSmall(false, 0, 74, "Cluster Size Warning", Alignment::center, FontPalette::white);
					printSmall(false, 0, 98, "Your SD card is not formatted", Alignment::center);
					printSmall(false, 0, 110, "using 32KB clusters, this causes", Alignment::center);
					printSmall(false, 0, 122, "some games to load very slowly.", Alignment::center);
					printSmall(false, 0, 134, "It's recommended to reformat your", Alignment::center);
					printSmall(false, 0, 146, "SD card using 32KB clusters.", Alignment::center);
					printSmall(false, 0, 166, " Return    Launch", Alignment::center);
					updateText(false);

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
					clearText(false);
					showdialogbox = false;
					dialogboxHeight = 0;

					if (proceedToLaunch) {
						titleUpdate(dirContents.at(fileOffset).isDirectory,dirContents.at(fileOffset).name.c_str());
						showLocation();
						updateText(false);
					} else if (ms().macroMode) {
						updateText(false);
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

						printSmall(false, 0, 74, "Now saving...", Alignment::center, FontPalette::white);

						printSmall(false, 0, 98, "If this crashes with an error, please", Alignment::center);
						printSmall(false, 0, 110, "disable \"Update recently played list\".", Alignment::center);
						updateText(false);

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

						if (ms().sortMethod == TWLSettings::ESortRecent) {
							// Set cursor pos to the first slot that isn't a directory so it won't be misplaced with recent sort
							ms().saveCursorPosition[ms().secondaryDevice] = fileStartPos;
						}
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
			ms().pagenum[ms().secondaryDevice] = 0;
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
				printSmall(false, 0, 74, "Folder Management options", Alignment::center, FontPalette::white);
				printSmall(false, 0, 110, "to do with this folder?", Alignment::center);
			} else {
				printSmall(false, 0, 74, "Title Management options", Alignment::center, FontPalette::white);
				printSmall(false, 0, 110, "to do with this title?", Alignment::center);
			}
			printSmall(false, 0, 98, "What would you like", Alignment::center);
			updateText(false);

			for (int i = 0; i < 90; i++) swiWaitForVBlank();

			if (isTwlm || isDirectory) {
				if (unHide)	printSmall(false, 0, 128, " Unhide   Nothing", Alignment::center);
				else		printSmall(false, 0, 128, " Hide     Nothing", Alignment::center);
			} else {
				if (unHide)	printSmall(false, 0, 128, " Unhide   Delete   Nothing", Alignment::center);
				else		printSmall(false, 0, 128, " Hide    Delete    Nothing", Alignment::center);
			}
			updateText(false);

			while (1) {
				do {
					scanKeys();
					pressed = keysDown();
					bgOperations(true);
				} while (!pressed);
				
				if (((pressed & KEY_A) && !isTwlm && !isDirectory) || (pressed & KEY_Y)) {
					clearText();
					showdialogbox = false;
					updateText(true);
					updateText(false);

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
					break;
				}
			}
			clearText(false);
			showdialogbox = false;
			if (ms().theme == TWLSettings::EThemeGBC) {
				gbnpBottomInfo();
			}
			updateText(false);

			if (ms().macroMode) {
				lcdMainOnTop();
				lcdSwapped = false;
			}
			for (int i = 0; i < 25; i++) swiWaitForVBlank();
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
			clearText();
			updateText(true);
			updateText(false);
			startMenu = true;
			return "null";		
		}

		if ((pressed & KEY_Y) && !ms().kioskMode && isValid && !isTwlm && !isDirectory && (bnrRomType == 0 || bnrRomType == 1 || bnrRomType == 3)) {
			ms().cursorPosition[ms().secondaryDevice] = fileOffset;
			perGameSettings(dirContents.at(fileOffset).name);
			if (ms().theme == TWLSettings::EThemeGBC) {
				gbnpBottomInfo();
				refreshDirectoryContents = true;
			}
			for (int i = 0; i < 25; i++) bgOperations(true);
		}

		if (refreshDirectoryContents) {
			showDirectoryContents (dirContents, screenOffset, fileOffset);
		}
	}
}
