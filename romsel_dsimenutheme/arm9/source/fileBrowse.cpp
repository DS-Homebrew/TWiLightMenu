#include "fileBrowse.h"
#include <algorithm>
#include <dirent.h>
#include <math.h>
#include <sstream>
#include <stdio.h>
#include <string>
#include <unistd.h>
#include <vector>

#include <gl2d.h>
#include <maxmod9.h>
#include <nds.h>
#include <nds/arm9/dldi.h>
#include <fat.h>
#include "fat_ext.h"

#include "date.h"

#include "SwitchState.h"
#include "errorScreen.h"
#include "graphics/ThemeConfig.h"
#include "graphics/ThemeTextures.h"
#include "graphics/fontHandler.h"
#include "graphics/graphics.h"
#include "graphics/iconHandler.h"
#include "iconTitle.h"
#include "ndsheaderbanner.h"
#include "perGameSettings.h"
#include "incompatibleGameMap.h"
#include "compatibleDSiWareMap.h"
#include "gbaswitch.h"

#include "common/twlmenusettings.h"
#include "common/bootstrapsettings.h"
#include "common/flashcard.h"
#include "common/inifile.h"
#include "common/logging.h"
#include "common/nds_loader_arm9.h"
#include "common/stringtool.h"
#include "common/systemdetails.h"
#include "common/tonccpy.h"
#include "defaultSettings.h"
#include "myDSiMode.h"
#include "language.h"

#include "fileCopy.h"
#include "sound.h"

#include "graphics/queueControl.h"

#define SCREEN_COLS 32
#define ENTRIES_PER_SCREEN 15
#define ENTRIES_START_ROW 3
#define ENTRY_PAGE_LENGTH 10

extern bool whiteScreen;
extern bool fadeType;
extern bool fadeSpeed;
extern bool controlTopBright;
extern bool controlBottomBright;

extern const char *unlaunchAutoLoadID;
extern void unlaunchRomBoot(std::string_view rom);

extern bool dropDown;
extern int currentBg;
extern bool showSTARTborder;
extern bool startBorderZoomOut;
extern bool buttonArrowTouched[2];
extern bool scrollWindowTouched;

extern bool applaunchprep;

extern touchPosition touch;

extern bool showdialogbox;
extern bool dboxInFrame;
extern bool dboxStopped;
extern bool dbox_showIcon;
extern bool dbox_selectMenu;

extern bool applaunch;
extern bool dsModeForced;

extern int vblankRefreshCounter;

extern vector<char *> argarray;

int file_count = 0;
int last_used_box = 0;
static int fileStartPos = 0; // The position of the first thing that is not a directory.
static bool backFound = false;

extern int spawnedtitleboxes;

extern int titleboxXpos[2];
extern int titleboxXdest[2];
extern int titlewindowXpos[2];
extern int titlewindowXdest[2];
extern int titleboxXspeed;
extern int titleboxXspacing;
int movingApp = -1;
int movingAppYpos = 0;
bool movingAppIsDir = false;
bool draggingIcons = false;
extern bool showMovingArrow;
extern double movingArrowYpos;
extern bool displayGameIcons;

extern bool showLshoulder;
extern bool showRshoulder;

extern bool showProgressIcon;
extern bool showProgressBar;
extern int progressBarLength;

bool dirInfoIniFound = false;
bool pageLoaded[100] = {false};
bool dirContentBlankFilled[100] = {false};
bool lockOutDirContentBlankFilling = false;
std::string dirContName;

char boxArtPath[256];
const char* boxArtFilename;

bool boxArtFound = false;
bool boxArtLoaded = false;
bool shouldersRendered = false;
bool settingsChanged = false;

static bool musicplaying = false;
bool edgeBumpSoundPlayed = false;
bool needToPlayStopSound = false;
bool stopSoundPlayed = false;
int waitForNeedToPlayStopSound = 0;

bool bannerTextShown = false;

extern void stop();

extern void RemoveTrailingSlashes(std::string &path);
extern void loadGameOnFlashcard(const char *ndsPath, bool dsGame);
extern void dsCardLaunch();
extern void unlaunchSetHiyaBoot();
extern void SetWidescreen(const char *filename);
extern bool createDSiWareSave(const char *path, int size);
extern void createSaveFile(const char* savePath, const bool isHomebrew, const char* gameTid);

extern bool rocketVideo_playVideo;

extern void bgOperations(bool waitFrame);

std::string gameOrderIniPath, recentlyPlayedIniPath, timesPlayedIniPath;

static bool inSelectMenu = false;

struct DirEntry {
	DirEntry(std::string name, bool isDirectory, int position, int customPos) : name(name), isDirectory(isDirectory), position(position), customPos(customPos) {}
	DirEntry() {}

	std::string name;
	bool isDirectory;
	int position;
	bool customPos;
};

char path[PATH_MAX] = {0};

#ifdef EMULATE_FILES
#define chdir(a) chdirFake(a)
void chdirFake(const char *dir) {
	string pathStr(path);
	string dirStr(dir);
	if (dirStr == "..") {
		pathStr.resize(pathStr.find_last_of("/"));
		pathStr.resize(pathStr.find_last_of("/") + 1);
	} else {
		pathStr += dirStr;
		pathStr += "/";
	}
	strcpy(path, pathStr.c_str());
}
#endif

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

void recalculateBoxesCount() {
	if (ms().hideEmptyBoxes)
		last_used_box = std::clamp(file_count-1-PAGENUM*40, 0, 39);
	else
		last_used_box = 39;
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

void updateDirectoryContents(vector<DirEntry> &dirContents) {
	if (!dirInfoIniFound || pageLoaded[PAGENUM]) return;

	if ((PAGENUM > 0) && !lockOutDirContentBlankFilling) {
		for (int p = 0; p < PAGENUM; p++) {
			for (int i = 0; i < 40; i++) {
				dirContents.insert(dirContents.begin() + i + (p * 40), {"", false, i + (p * 40), false});
			}
			dirContentBlankFilled[p] = true;
		}
	}
	lockOutDirContentBlankFilling = true;

	char str[12] = {0};
	CIniFile twlmDirInfo("dirInfo.twlm.ini");
	int currentPos = PAGENUM*40;
	for (int i = 0; i < 40; i++) {
		sprintf(str, "%d", i+(PAGENUM*40));
		std::string filename = twlmDirInfo.GetString("LIST", str, "");

		if (filename != "") {
			if (dirContentBlankFilled[PAGENUM]) {
				dirContents.erase(dirContents.begin() + i + (PAGENUM * 40));
			}
			dirContents.insert(dirContents.begin() + i + (PAGENUM * 40), {filename, false, currentPos, false});
			currentPos++;
		} else {
			break;
		}
	}
	pageLoaded[PAGENUM] = true;
}

void getDirectoryContents(std::vector<DirEntry> &dirContents, const std::vector<std::string_view> extensionList = {}) {
	dirContents.clear();

	file_count = 0;
	fileStartPos = 0;

	if (access("dirInfo.twlm.ini", F_OK) == 0) {
		dirInfoIniFound = true;

		for (int i = 0; i < (int)sizeof(pageLoaded); i++) {
			pageLoaded[i] = false;
		}

		CIniFile twlmDirInfo("dirInfo.twlm.ini");
		file_count = twlmDirInfo.GetInt("INFO", "GAMES", 0);
		recalculateBoxesCount();
		return;
	} else {
		dirInfoIniFound = false;
	}

	DIR *pdir = opendir(".");

	if (pdir == nullptr) {
		printSmall(false, 4, 4, STR_UNABLE_TO_OPEN_DIRECTORY);
	} else {
		backFound = false;
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
		recalculateBoxesCount();

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

bool nowLoadingDisplaying = false;

void displayNowLoading(void) {
	displayGameIcons = false;
	fadeType = true; // Fade in from white
	std::string msg;
	printLarge(false, 0, 88, STR_NOW_LOADING, Alignment::center);
	if (!sys().isRegularDS()) {
		if (ms().theme == TWLSettings::EThemeSaturn) {
			printSmall(false, 0, 20 + smallFontHeight(), ms().secondaryDevice ? STR_LOCATION_SLOT_1 : (ms().showMicroSd ? STR_LOCATION_MICRO_SD : STR_LOCATION_SD), Alignment::center);
		} else {
			printSmall(false, 0, 168, ms().secondaryDevice ? STR_LOCATION_SLOT_1 : (ms().showMicroSd ? STR_LOCATION_MICRO_SD : STR_LOCATION_SD), Alignment::center);
		}
	}
	updateText(false);
	nowLoadingDisplaying = true;
	while (!screenFadedIn()) 
	{
		bgOperations(true);
	}
	bgOperations(false);
	showProgressIcon = true;
}

void moveCursor(bool right, const std::vector<DirEntry> dirContents, int maxEntry = 0xFFFF) {
	if ((right && CURPOS >= last_used_box) || (!right && CURPOS <= 0)) {
		if (ms().theme != TWLSettings::EThemeSaturn && !edgeBumpSoundPlayed)
			snd().playWrong();
		edgeBumpSoundPlayed = true;
		return;
	}

	waitForNeedToPlayStopSound = 1;
	settingsChanged = true;
	bannerTextShown = false;

	bool firstMove = true;
	touchPosition touch;
	do {
		if (right && CURPOS < last_used_box && CURPOS < maxEntry) {
			CURPOS++;
		} else if (!right && CURPOS > 0) {
			CURPOS--;
		} else  {
			if (ms().theme != TWLSettings::EThemeSaturn && !edgeBumpSoundPlayed)
				snd().playWrong();
			edgeBumpSoundPlayed = true;
			return;
		}

		if (movingApp == -1) {
			clearText(false);
			if (CURPOS + PAGENUM * 40 < (int)dirContents.size()) {
				if (ms().theme != TWLSettings::EThemeSaturn)
					currentBg = 1;
				titleUpdate(dirContents[CURPOS + PAGENUM * 40].isDirectory,
							dirContents[CURPOS + PAGENUM * 40].name,
							CURPOS);
			} else {
				if (ms().theme != TWLSettings::EThemeSaturn)
					currentBg = 0;
			}
			if (ms().theme == TWLSettings::EThemeHBL) {
				printLarge(false, 0, 142, "^", Alignment::center, FontPalette::overlay);
				printSmall(false, 4, 174, (showLshoulder ? STR_L_PREV : STR_L), Alignment::left, FontPalette::overlay);
				printSmall(false, 256-4, 174, (showRshoulder ? STR_NEXT_R : STR_R), Alignment::right, FontPalette::overlay);
			} else if (ms().macroMode && ms().theme != TWLSettings::EThemeSaturn) {
				printSmall(false, 4, 152, (showLshoulder ? STR_L_PREV : STR_L), Alignment::left, FontPalette::overlay);
				printSmall(false, 256-4, 152, (showRshoulder ? STR_NEXT_R : STR_R), Alignment::right, FontPalette::overlay);
			}
			updateText(false);
		}

		int pos = CURPOS + (right ? 2 : -2);
		if (pos >= 0 && pos + PAGENUM * 40 < (int)dirContents.size()) {
			iconUpdate(dirContents[pos + PAGENUM * 40].isDirectory,
						dirContents[pos + PAGENUM * 40].name.c_str(),
						pos);
		}

		bgOperations(false);

		snd().playSelect();

		if (ms().theme != TWLSettings::EThemeSaturn) {
			for (int i = 0; i < 8; i++) {
				if (right) {
					titleboxXdest[ms().secondaryDevice] += titleboxXspacing / 8;
					if (i % 3)
						titlewindowXdest[ms().secondaryDevice]++;
				} else {
					titleboxXdest[ms().secondaryDevice] -= titleboxXspacing / 8;
					if (i % 3)
						titlewindowXdest[ms().secondaryDevice]--;
				}
				swiWaitForVBlank();
			}
			titleboxXdest[ms().secondaryDevice] = CURPOS * titleboxXspacing;
			swiWaitForVBlank();
		} else {
			if (right) {
				titleboxXdest[ms().secondaryDevice] += titleboxXspacing;
				titlewindowXdest[ms().secondaryDevice] += 5;
			} else {
				titleboxXdest[ms().secondaryDevice] -= titleboxXspacing;
				titlewindowXdest[ms().secondaryDevice] -= 5;
			}

			for (int i = 0; i < 4; i++)
				swiWaitForVBlank();
		}

		// Bit of delay the first time to give time to release the button
		// To allow rapid presses, the delay is not applied for 3DS theme
		if (firstMove) {
			firstMove = false;
			if (boxArtLoaded) {
				if (!rocketVideo_playVideo)
					clearBoxArt();
				rocketVideo_playVideo = (ms().theme == TWLSettings::ETheme3DS) ? true : false;
				boxArtLoaded = false;
				if (ms().theme == TWLSettings::EThemeSaturn) {
					for (int i = 0; i < 10; i++)
						swiWaitForVBlank();
				}
			} else if (ms().theme == TWLSettings::ETheme3DS) {
				swiWaitForVBlank();
			} else {
				for (int i = 0; i < (ms().theme == TWLSettings::EThemeSaturn ? 15 : 4); i++)
					swiWaitForVBlank();
			}
		} else {
			if (ms().theme != TWLSettings::ETheme3DS)
				showSTARTborder = false;
		}


		scanKeys();
		touchRead(&touch);
	} while ((keysHeld() & (right ? KEY_RIGHT : KEY_LEFT)) || ((keysHeld() & KEY_TOUCH) && touch.py > 171 && (right ? touch.px > 236 : touch.px < 19) && ms().theme == TWLSettings::EThemeDSi));

	// Wait for movement to finish before showing START boarder and such
	while ((ms().theme != TWLSettings::ETheme3DS) && (titleboxXdest[ms().secondaryDevice] != titleboxXpos[ms().secondaryDevice]) && !(keysHeld() & KEY_TOUCH))
		swiWaitForVBlank();

	if (movingApp == -1 && CURPOS + PAGENUM * 40 < (int)dirContents.size())
		showSTARTborder = true;
	edgeBumpSoundPlayed = false;
	needToPlayStopSound = true;
	startBorderZoomOut = true;
}

void updateBoxArt(void) {
	if (!boxArtFound) return;
	showSTARTborder = true;
	if (ms().theme == TWLSettings::EThemeHBL || ms().macroMode || !ms().showBoxArt || boxArtLoaded) return;

	if (ms().theme == TWLSettings::ETheme3DS && rocketVideo_playVideo) {
		rocketVideo_playVideo = false;
		while (dmaBusy(1)); // Wait for frame to finish rendering
		tex().drawOverRotatingCubes(); // Clear top screen cubes for 3DS theme
	}
	clearBoxArt();

	if (isDirectory[CURPOS]) {
		if (ms().theme == TWLSettings::ETheme3DS && !rocketVideo_playVideo) {
			rocketVideo_playVideo = true;
		}
	} else {
		sprintf(boxArtPath, "%s:/_nds/TWiLightMenu/boxart/%s.png", sys().isRunFromSD() ? "sd" : "fat", boxArtFilename);
		if ((bnrRomType[CURPOS] == 0) && (access(boxArtPath, F_OK) != 0)) {
			sprintf(boxArtPath, "%s:/_nds/TWiLightMenu/boxart/%s.png", sys().isRunFromSD() ? "sd" : "fat", gameTid[CURPOS]);
		}
		tex().drawBoxArt(boxArtPath, (dsiFeatures() && ms().showBoxArt == 2)); // Load box art
	}
	boxArtLoaded = true;
}


void launchDsClassicMenu(void) {
	snd().playLaunch();
	controlTopBright = true;

	fadeType = false;		  // Fade to white
	snd().fadeOutStream();
	for (int i = 0; i < 60; i++) {
		bgOperations(true);
	}
	mmEffectCancelAll();
	snd().stopStream();
	ms().saveSettings();
	// Launch DS Classic Menu
	argarray.push_back((char*)(sys().isRunFromSD() ? "sd:/_nds/TWiLightMenu/mainmenu.srldr" : "fat:/_nds/TWiLightMenu/mainmenu.srldr"));
	int err = runNdsFile(argarray[0], argarray.size(), (const char**)&argarray[0], sys().isRunFromSD(), true, false, false, true, true, false, -1);
	char text[32];
	snprintf(text, sizeof(text), STR_START_FAILED_ERROR.c_str(), err);
	fadeType = true;
	printLarge(false, 4, 4, text);
	stop();
}

void launchSettings(void) {
	snd().playLaunch();
	controlTopBright = true;

	fadeType = false;		  // Fade to white
	snd().fadeOutStream();
	for (int i = 0; i < 60; i++) {
		bgOperations(true);
	}
	mmEffectCancelAll();
	snd().stopStream();
	ms().saveSettings();
	// Launch TWLMenu++ Settings
	argarray.push_back((char*)(sys().isRunFromSD() ? "sd:/_nds/TWiLightMenu/settings.srldr" : "fat:/_nds/TWiLightMenu/settings.srldr"));
	int err = runNdsFile(argarray[0], argarray.size(), (const char**)&argarray[0], sys().isRunFromSD(), true, false, false, true, true, false, -1);
	char text[32];
	snprintf(text, sizeof(text), STR_START_FAILED_ERROR.c_str(), err);
	fadeType = true;
	printLarge(false, 4, 4, text);
	stop();
}

void launchPictochat(const vector<DirEntry>& dirContents) {
	const char* pictochatPath = sys().isRunFromSD() ? "sd:/_nds/pictochat.nds" : "fat:/_nds/pictochat.nds";

	if (access(pictochatPath, F_OK) != 0) {
		if (ms().theme == TWLSettings::EThemeSaturn) {
			snd().playStartup();
			fadeType = false;	   // Fade to black
			for (int i = 0; i < 25; i++) {
				bgOperations(true);
			}
			currentBg = 1;
			displayGameIcons = false;
			fadeType = true;
		} else {
			snd().playWrong();
			showdialogbox = true;
		}
		clearText();
		updateText(false);
		if (ms().theme == TWLSettings::EThemeSaturn) {
			while (!screenFadedIn()) { bgOperations(true); }
			snd().playWrong();
		} else {
			while (!dboxStopped) { bgOperations(true); }
		}
		printSmall(false, 0, 66, STR_NO_PICTOCHAT_MSG, Alignment::center, FontPalette::dialog);
		printSmall(false, 0, 160, STR_A_OK, Alignment::center, FontPalette::dialog);
		updateText(false);
		int pressed = 0;
		do {
			scanKeys();
			pressed = keysDown();
			bgOperations(true);
		} while (!(pressed & KEY_A));
		snd().playBack();
		showdialogbox = false;
		if (ms().theme == TWLSettings::EThemeSaturn) {
			fadeType = false;	   // Fade to black
			for (int i = 0; i < 25; i++) {
				bgOperations(true);
			}
			clearText();
			currentBg = 0;
			displayGameIcons = true;
			fadeType = true;
			snd().playStartup();
		} else {
			clearText();
			updateText(false);
			if (ms().theme == TWLSettings::EThemeDSi) {
				for (int i = 0; i < 25; i++) {
					bgOperations(true);
				}
			}
		}
		if (CURPOS + PAGENUM * 40 < (int)dirContents.size()) {
			titleUpdate(
				dirContents[CURPOS + PAGENUM * 40].isDirectory,
				dirContents[CURPOS + PAGENUM * 40].name,
				CURPOS
			);
		}
		updateText(false);
		return;
	}

	snd().playLaunch();
	controlTopBright = true;

	fadeType = false;		  // Fade to white
	snd().fadeOutStream();
	for (int i = 0; i < 60; i++) {
		bgOperations(true);
	}
	mmEffectCancelAll();
	snd().stopStream();
	ms().saveSettings();
	// Launch Pictochat
	if ((!dsiFeatures() || bs().b4dsMode) && ms().secondaryDevice) {
		int err = runNdsFile(pictochatPath, 0, NULL, sys().isRunFromSD(), true, true, true, false, false, false, ms().gameLanguage);
		char text[32];
		snprintf(text, sizeof(text), STR_START_FAILED_ERROR.c_str(), err);
		fadeType = true;
		printLarge(false, 4, 4, text);
		int pressed = 0;
		do {
			scanKeys();
			pressed = keysDownRepeat();
			checkSdEject();
			swiWaitForVBlank();
		} while (!(pressed & KEY_B));
		fadeType = false;	// Fade to white
		for (int i = 0; i < 25; i++) {
			swiWaitForVBlank();
		}
		std::vector<char*> argarray;
		argarray.push_back((char*)(sys().isRunFromSD() ? "sd:/_nds/TWiLightMenu/dsimenu.srldr" : "fat:/_nds/TWiLightMenu/dsimenu.srldr"));
		runNdsFile(argarray[0], argarray.size(), (const char**)&argarray[0], sys().isRunFromSD(), true, false, false, true, true, false, -1);
	} else {
		char ndsToBoot[256];
		sprintf(ndsToBoot, "%s:/_nds/nds-bootstrap-%s.nds", sys().isRunFromSD() ? "sd" : "fat", ms().bootstrapFile ? "nightly" : "release");
		if (access(ndsToBoot, F_OK) != 0) {
			sprintf(ndsToBoot, "%s:/_nds/nds-bootstrap-%s.nds", sys().isRunFromSD() ? "fat" : "sd", ms().bootstrapFile ? "nightly" : "release");
		}

		std::vector<char*> argarray;
		argarray.push_back(ndsToBoot);

		const char *bootstrapinipath = (sys().isRunFromSD() ? BOOTSTRAP_INI : BOOTSTRAP_INI_FC);
		CIniFile bootstrapini(bootstrapinipath);
		bootstrapini.SetString("NDS-BOOTSTRAP", "NDS_PATH", pictochatPath);
		bootstrapini.SetString("NDS-BOOTSTRAP", "SAV_PATH", "");
		bootstrapini.SetString("NDS-BOOTSTRAP", "HOMEBREW_ARG", "");
		bootstrapini.SetString("NDS-BOOTSTRAP", "RAM_DRIVE_PATH", "");
		bootstrapini.SetString("NDS-BOOTSTRAP", "GUI_LANGUAGE", ms().getGuiLanguageString());
		bootstrapini.SetInt("NDS-BOOTSTRAP", "LANGUAGE", ms().gameLanguage);
		bootstrapini.SetInt("NDS-BOOTSTRAP", "DSI_MODE", 0);
		bootstrapini.SetInt("NDS-BOOTSTRAP", "BOOST_CPU", 0);
		bootstrapini.SetInt("NDS-BOOTSTRAP", "BOOST_VRAM", 0);
		bootstrapini.SaveIniFile(bootstrapinipath);
		int err = runNdsFile(argarray[0], argarray.size(), (const char **)&argarray[0], sys().isRunFromSD(), true, true, false, true, true, false, -1);
		char text[32];
		snprintf(text, sizeof(text), STR_START_FAILED_ERROR.c_str(), err);
		printLarge(false, 4, 4, text);
		if (err == 1) {
			printLarge(false, 4, 20, ms().bootstrapFile ? STR_BOOTSTRAP_NIGHTLY_NOT_FOUND : STR_BOOTSTRAP_RELEASE_NOT_FOUND);
		}
		printSmall(false, 4, 20 + calcLargeFontHeight(ms().bootstrapFile ? STR_BOOTSTRAP_NIGHTLY_NOT_FOUND : STR_BOOTSTRAP_RELEASE_NOT_FOUND), STR_PRESS_B_RETURN);
		updateText(false);
		fadeType = true;
		int pressed = 0;
		do {
			scanKeys();
			pressed = keysDownRepeat();
			checkSdEject();
			swiWaitForVBlank();
		} while (!(pressed & KEY_B));
		fadeType = false;	// Fade to white
		for (int i = 0; i < 25; i++) {
			swiWaitForVBlank();
		}
		// while (argarray.size() != 0) {
			free(argarray.at(0));
			argarray.erase(argarray.begin());
		// }
		argarray.push_back((char*)(sys().isRunFromSD() ? "sd:/_nds/TWiLightMenu/dsimenu.srldr" : "fat:/_nds/TWiLightMenu/dsimenu.srldr"));
		runNdsFile(argarray[0], argarray.size(), (const char**)&argarray[0], sys().isRunFromSD(), true, false, false, true, true, false, -1);
	}
	stop();
}

void launchDownloadPlay(const vector<DirEntry>& dirContents) {
	const char* dlplayPath = sys().isRunFromSD() ? "sd:/_nds/dlplay.nds" : "fat:/_nds/dlplay.nds";

	if ((!isDSiMode() || ms().consoleModel < 2) && access(dlplayPath, F_OK) != 0) {
		if (ms().theme == TWLSettings::EThemeSaturn) {
			snd().playStartup();
			fadeType = false;	   // Fade to black
			for (int i = 0; i < 25; i++) {
				bgOperations(true);
			}
			currentBg = 1;
			displayGameIcons = false;
			fadeType = true;
		} else {
			snd().playWrong();
			showdialogbox = true;
		}
		clearText();
		updateText(false);
		if (ms().theme == TWLSettings::EThemeSaturn) {
			while (!screenFadedIn()) { bgOperations(true); }
			snd().playWrong();
		} else {
			while (!dboxStopped) { bgOperations(true); }
		}
		printSmall(false, 0, 66, STR_NO_DLPLAY_MSG, Alignment::center, FontPalette::dialog);
		printSmall(false, 0, 160, STR_A_OK, Alignment::center, FontPalette::dialog);
		updateText(false);
		int pressed = 0;
		do {
			scanKeys();
			pressed = keysDown();
			bgOperations(true);
		} while (!(pressed & KEY_A));
		snd().playBack();
		showdialogbox = false;
		if (ms().theme == TWLSettings::EThemeSaturn) {
			fadeType = false;	   // Fade to black
			for (int i = 0; i < 25; i++) {
				bgOperations(true);
			}
			clearText();
			currentBg = 0;
			displayGameIcons = true;
			fadeType = true;
			snd().playStartup();
		} else {
			clearText();
			updateText(false);
			if (ms().theme == TWLSettings::EThemeDSi) {
				for (int i = 0; i < 25; i++) {
					bgOperations(true);
				}
			}
		}
		if (CURPOS + PAGENUM * 40 < (int)dirContents.size()) {
			titleUpdate(
				dirContents[CURPOS + PAGENUM * 40].isDirectory,
				dirContents[CURPOS + PAGENUM * 40].name,
				CURPOS
			);
		}
		updateText(false);
		return;
	}

	snd().playLaunch();
	controlTopBright = true;

	fadeType = false;		  // Fade to white
	snd().fadeOutStream();
	for (int i = 0; i < 60; i++) {
		bgOperations(true);
	}
	mmEffectCancelAll();
	snd().stopStream();
	ms().saveSettings();
	// Launch DS Download Play
	if (isDSiMode() && (sys().arm7SCFGLocked() || (ms().consoleModel >= 2 && access(dlplayPath, F_OK) != 0))) {
		*(u32*)(0x02000300) = 0x434E4C54; // Set "CNLT" warmboot flag
		*(u16*)(0x02000304) = 0x1801;

		switch (ms().sysRegion) {
			case 4:
				*(u32*)(0x02000308) = 0x484E4443;
				*(u32*)(0x0200030C) = 0x00030005;
				*(u32*)(0x02000310) = 0x484E4443;
				break;
			case 5:
				*(u32*)(0x02000308) = 0x484E444B;
				*(u32*)(0x0200030C) = 0x00030005;
				*(u32*)(0x02000310) = 0x484E444B;
				break;
			default:
				*(u32*)(0x02000308) = 0x484E4441;	// "HNDA"
				*(u32*)(0x0200030C) = 0x00030005;
				*(u32*)(0x02000310) = 0x484E4441;	// "HNDA"
		}

		*(u32*)(0x02000314) = 0x00030005;
		*(u32*)(0x02000318) = 0x00000017;
		*(u32*)(0x0200031C) = 0x00000000;
		*(u16*)(0x02000306) = swiCRC16(0xFFFF, (void *)0x02000308, 0x18);

		if (ms().consoleModel < 2) {
			unlaunchSetHiyaBoot();
		}

		DC_FlushAll();						// Make reboot not fail
		fifoSendValue32(FIFO_USER_02, 1); // Reboot into DSiWare title, booted via Launcher
		for (int i = 0; i < 15; i++) swiWaitForVBlank();
	} else if ((!dsiFeatures() || bs().b4dsMode) && ms().secondaryDevice) {
		int err = runNdsFile(dlplayPath, 0, NULL, sys().isRunFromSD(), true, true, true, false, false, false, ms().gameLanguage);
		char text[32];
		snprintf(text, sizeof(text), STR_START_FAILED_ERROR.c_str(), err);
		fadeType = true;
		printLarge(false, 4, 4, text);
		int pressed = 0;
		do {
			scanKeys();
			pressed = keysDownRepeat();
			checkSdEject();
			swiWaitForVBlank();
		} while (!(pressed & KEY_B));
		fadeType = false;	// Fade to white
		for (int i = 0; i < 25; i++) {
			swiWaitForVBlank();
		}
		std::vector<char*> argarray;
		argarray.push_back((char*)(sys().isRunFromSD() ? "sd:/_nds/TWiLightMenu/dsimenu.srldr" : "fat:/_nds/TWiLightMenu/dsimenu.srldr"));
		runNdsFile(argarray[0], argarray.size(), (const char**)&argarray[0], sys().isRunFromSD(), true, false, false, true, true, false, -1);
	} else {
		char ndsToBoot[256];
		sprintf(ndsToBoot, "%s:/_nds/nds-bootstrap-%s.nds", sys().isRunFromSD() ? "sd" : "fat", ms().bootstrapFile ? "nightly" : "release");
		if (access(ndsToBoot, F_OK) != 0) {
			sprintf(ndsToBoot, "%s:/_nds/nds-bootstrap-%s.nds", sys().isRunFromSD() ? "fat" : "sd", ms().bootstrapFile ? "nightly" : "release");
		}

		std::vector<char*> argarray;
		argarray.push_back(ndsToBoot);

		const char *bootstrapinipath = (sys().isRunFromSD() ? BOOTSTRAP_INI : BOOTSTRAP_INI_FC);
		CIniFile bootstrapini(bootstrapinipath);
		bootstrapini.SetString("NDS-BOOTSTRAP", "NDS_PATH", dlplayPath);
		bootstrapini.SetString("NDS-BOOTSTRAP", "SAV_PATH", "");
		bootstrapini.SetString("NDS-BOOTSTRAP", "HOMEBREW_ARG", "");
		bootstrapini.SetString("NDS-BOOTSTRAP", "RAM_DRIVE_PATH", "");
		bootstrapini.SetString("NDS-BOOTSTRAP", "GUI_LANGUAGE", ms().getGuiLanguageString());
		bootstrapini.SetInt("NDS-BOOTSTRAP", "LANGUAGE", ms().gameLanguage);
		bootstrapini.SetInt("NDS-BOOTSTRAP", "DSI_MODE", 0);
		bootstrapini.SetInt("NDS-BOOTSTRAP", "BOOST_CPU", 0);
		bootstrapini.SetInt("NDS-BOOTSTRAP", "BOOST_VRAM", 0);
		bootstrapini.SaveIniFile(bootstrapinipath);
		int err = runNdsFile(argarray[0], argarray.size(), (const char **)&argarray[0], sys().isRunFromSD(), true, true, false, true, true, false, -1);
		char text[32];
		snprintf(text, sizeof(text), STR_START_FAILED_ERROR.c_str(), err);
		printLarge(false, 4, 4, text);
		if (err == 1) {
			printLarge(false, 4, 20, ms().bootstrapFile ? STR_BOOTSTRAP_NIGHTLY_NOT_FOUND : STR_BOOTSTRAP_RELEASE_NOT_FOUND);
		}
		printSmall(false, 4, 20 + calcLargeFontHeight(ms().bootstrapFile ? STR_BOOTSTRAP_NIGHTLY_NOT_FOUND : STR_BOOTSTRAP_RELEASE_NOT_FOUND), STR_PRESS_B_RETURN);
		updateText(false);
		fadeType = true;
		int pressed = 0;
		do {
			scanKeys();
			pressed = keysDownRepeat();
			checkSdEject();
			swiWaitForVBlank();
		} while (!(pressed & KEY_B));
		fadeType = false;	// Fade to white
		for (int i = 0; i < 25; i++) {
			swiWaitForVBlank();
		}
		// while (argarray.size() != 0) {
			free(argarray.at(0));
			argarray.erase(argarray.begin());
		// }
		argarray.push_back((char*)(sys().isRunFromSD() ? "sd:/_nds/TWiLightMenu/dsimenu.srldr" : "fat:/_nds/TWiLightMenu/dsimenu.srldr"));
		runNdsFile(argarray[0], argarray.size(), (const char**)&argarray[0], sys().isRunFromSD(), true, false, false, true, true, false, -1);
	}
	stop();
}

void launchInternetBrowser(const vector<DirEntry>& dirContents) {
	if (ms().internetBrowserPath == "" || access(ms().internetBrowserPath.c_str(), F_OK) != 0) {
		if (ms().theme == TWLSettings::EThemeSaturn) {
			snd().playStartup();
			fadeType = false;	   // Fade to black
			for (int i = 0; i < 25; i++) {
				bgOperations(true);
			}
			currentBg = 1;
			displayGameIcons = false;
			fadeType = true;
		} else {
			snd().playWrong();
			showdialogbox = true;
		}
		clearText();
		updateText(false);
		if (ms().theme == TWLSettings::EThemeSaturn) {
			while (!screenFadedIn()) { bgOperations(true); }
			snd().playWrong();
		} else {
			while (!dboxStopped) { bgOperations(true); }
		}
		printSmall(false, 0, 52, dsiFeatures() ? STR_NO_INTERNET_BROWSER_MSG : STR_NO_INTERNET_BROWSER_MSG_NTR, Alignment::center, FontPalette::dialog);
		printSmall(false, 0, 160, STR_A_OK, Alignment::center, FontPalette::dialog);
		updateText(false);
		int pressed = 0;
		do {
			scanKeys();
			pressed = keysDown();
			bgOperations(true);
		} while (!(pressed & KEY_A));
		snd().playBack();
		showdialogbox = false;
		if (ms().theme == TWLSettings::EThemeSaturn) {
			fadeType = false;	   // Fade to black
			for (int i = 0; i < 25; i++) {
				bgOperations(true);
			}
			clearText();
			currentBg = 0;
			displayGameIcons = true;
			fadeType = true;
			snd().playStartup();
		} else {
			clearText();
			updateText(false);
			if (ms().theme == TWLSettings::EThemeDSi) {
				for (int i = 0; i < 25; i++) {
					bgOperations(true);
				}
			}
		}
		if (CURPOS + PAGENUM * 40 < (int)dirContents.size()) {
			titleUpdate(
				dirContents[CURPOS + PAGENUM * 40].isDirectory,
				dirContents[CURPOS + PAGENUM * 40].name,
				CURPOS
			);
		}
		updateText(false);
		return;
	}

	snd().playLaunch();
	controlTopBright = true;

	fadeType = false;		  // Fade to white
	snd().fadeOutStream();
	for (int i = 0; i < 60; i++) {
		bgOperations(true);
	}
	mmEffectCancelAll();
	snd().stopStream();
	ms().internetBrowserLaunched = true;
	ms().saveSettings();
	// Launch Internet Browser
	if ((!dsiFeatures() || bs().b4dsMode) && ms().secondaryDevice && ms().internetBrowserPath[0] == 'f' && ms().internetBrowserPath[1] == 'a' && ms().internetBrowserPath[2] == 't') {
		loadGameOnFlashcard(ms().internetBrowserPath.c_str(), true);
	} else {
		sNDSHeaderExt NDSHeader;

		std::string romfolder = ms().internetBrowserPath;
		while (!romfolder.empty() && romfolder[romfolder.size()-1] != '/') {
			romfolder.resize(romfolder.size()-1);
		}
		std::string romFolderNoSlash = romfolder;
		RemoveTrailingSlashes(romFolderNoSlash);

		chdir(romFolderNoSlash.c_str());

		std::string filename = ms().internetBrowserPath;
		const size_t last_slash_idx = filename.find_last_of("/");
		if (std::string::npos != last_slash_idx) {
			filename.erase(0, last_slash_idx + 1);
		}

		FILE *f_nds_file = fopen(filename.c_str(), "rb");
		fread(&NDSHeader, 1, sizeof(NDSHeader), f_nds_file);
		fclose(f_nds_file);

		if (!dsiFeatures() && NDSHeader.unitCode == 3) {
			stop();
		}

		loadPerGameSettings(filename);

		std::string typeToReplace = filename.substr(filename.rfind('.'));

		char browserTid[5] = {0};
		tonccpy(browserTid, NDSHeader.gameCode, 4);

		std::string savename = replaceAll(filename, typeToReplace, getSavExtension());
		std::string savenamePub = replaceAll(filename, typeToReplace, getPubExtension());
		std::string savenamePrv = replaceAll(filename, typeToReplace, getPrvExtension());
		std::string savepath = romFolderNoSlash + "/saves/" + savename;
		std::string savepathPub = romFolderNoSlash + "/saves/" + savenamePub;
		std::string savepathPrv = romFolderNoSlash + "/saves/" + savenamePrv;
		if (ms().saveLocation == TWLSettings::ETWLMFolder) {
			std::string twlmSavesFolder = sys().isRunFromSD() ? "sd:/_nds/TWiLightMenu/saves" : "fat:/_nds/TWiLightMenu/saves";
			mkdir(twlmSavesFolder.c_str(), 0777);
			savepath = twlmSavesFolder + "/" + savename;
			savepathPub = twlmSavesFolder + "/" + savenamePub;
			savepathPrv = twlmSavesFolder + "/" + savenamePrv;
		} else if (ms().saveLocation == TWLSettings::EGamesFolder) {
			savepath = romFolderNoSlash + "/" + savename;
			savepathPub = romFolderNoSlash + "/" + savenamePub;
			savepathPrv = romFolderNoSlash + "/" + savenamePrv;
		} else {
			mkdir("saves", 0777);
		}

		char sfnSrl[62];
		char sfnPub[62];
		char sfnPrv[62];

		if (NDSHeader.unitCode == 3) {
			if ((getFileSize(savepathPub.c_str()) == 0) && (NDSHeader.pubSavSize > 0)) {
				if (ms().theme == TWLSettings::EThemeHBL) {
					displayGameIcons = false;
				} else if (ms().theme != TWLSettings::EThemeSaturn) {
					while (!screenFadedOut()) {
						swiWaitForVBlank();
					}
					fadeSpeed = true; // Fast fading
					whiteScreen = true;
					tex().clearTopScreen();
				}
				clearText();
				printLarge(false, 0, (ms().theme == TWLSettings::EThemeSaturn ? 80 : 88), STR_CREATING_PUBLIC_SAVE, Alignment::center);
				updateText(false);
				if (ms().theme != TWLSettings::EThemeSaturn && !fadeType) {
					fadeType = true; // Fade in from white
				}
				showProgressIcon = true;
				createDSiWareSave(savepathPub.c_str(), NDSHeader.pubSavSize);
				showProgressIcon = false;
				clearText();
				printLarge(false, 0, (ms().theme == TWLSettings::EThemeSaturn ? 32 : 88), STR_PUBLIC_SAVE_CREATED, Alignment::center);
				updateText(false);
				for (int i = 0; i < 60; i++) {
					swiWaitForVBlank();
				}
				if (ms().theme == TWLSettings::EThemeHBL) displayGameIcons = true;
			}

			if ((getFileSize(savepathPrv.c_str()) == 0) && (NDSHeader.prvSavSize > 0)) {
				if (ms().theme == TWLSettings::EThemeHBL) {
					displayGameIcons = false;
				} else if (ms().theme != TWLSettings::EThemeSaturn) {
					while (!fadeType && !screenFadedOut()) {
						swiWaitForVBlank();
					}
					fadeSpeed = true; // Fast fading
					whiteScreen = true;
					tex().clearTopScreen();
				}
				clearText();
				printLarge(false, 0, (ms().theme == TWLSettings::EThemeSaturn ? 80 : 88), STR_CREATING_PRIVATE_SAVE, Alignment::center);
				updateText(false);
				if (ms().theme != TWLSettings::EThemeSaturn && !fadeType) {
					fadeType = true; // Fade in from white
				}
				showProgressIcon = true;
				createDSiWareSave(savepathPrv.c_str(), NDSHeader.prvSavSize);
				showProgressIcon = false;
				clearText();
				printLarge(false, 0, (ms().theme == TWLSettings::EThemeSaturn ? 32 : 88), STR_PRIVATE_SAVE_CREATED, Alignment::center);
				updateText(false);
				for (int i = 0; i < 60; i++) {
					swiWaitForVBlank();
				}
				if (ms().theme == TWLSettings::EThemeHBL) displayGameIcons = true;
			}

			if (ms().theme != TWLSettings::EThemeSaturn && ms().theme != TWLSettings::EThemeHBL && fadeType) {
				fadeType = false; // Fade to white
			}

			fatGetAliasPath(ms().secondaryDevice ? "fat:/" : "sd:/", ms().internetBrowserPath.c_str(), sfnSrl);
			fatGetAliasPath(ms().secondaryDevice ? "fat:/" : "sd:/", savepathPub.c_str(), sfnPub);
			fatGetAliasPath(ms().secondaryDevice ? "fat:/" : "sd:/", savepathPrv.c_str(), sfnPrv);
		} else {
			createSaveFile(savepath.c_str(), false, browserTid);
		}

		char ndsToBoot[256];
		sprintf(ndsToBoot, "%s:/_nds/nds-bootstrap-%s.nds", sys().isRunFromSD() ? "sd" : "fat", ms().bootstrapFile ? "nightly" : "release");
		if (access(ndsToBoot, F_OK) != 0) {
			sprintf(ndsToBoot, "%s:/_nds/nds-bootstrap-%s.nds", sys().isRunFromSD() ? "fat" : "sd", ms().bootstrapFile ? "nightly" : "release");
		}

		std::vector<char*> argarray;
		argarray.push_back(ndsToBoot);

		const char *bootstrapinipath = (sys().isRunFromSD() ? BOOTSTRAP_INI : BOOTSTRAP_INI_FC);
		CIniFile bootstrapini(bootstrapinipath);
		bootstrapini.SetString("NDS-BOOTSTRAP", "NDS_PATH", ms().internetBrowserPath.c_str());
		if (NDSHeader.unitCode == 3) {
			bootstrapini.SetString("NDS-BOOTSTRAP", "APP_PATH", sfnSrl);
			bootstrapini.SetString("NDS-BOOTSTRAP", "SAV_PATH", sfnPub);
			bootstrapini.SetString("NDS-BOOTSTRAP", "PRV_PATH", sfnPrv);
			bootstrapini.SetInt("NDS-BOOTSTRAP", "DSI_MODE", 1);
			bootstrapini.SetInt("NDS-BOOTSTRAP", "BOOST_CPU", true);
			bootstrapini.SetInt("NDS-BOOTSTRAP", "BOOST_VRAM", true);
		} else {
			extern bool setClockSpeed(const bool phatColors);

			bootstrapini.SetString("NDS-BOOTSTRAP", "SAV_PATH", savepath.c_str());
			bootstrapini.SetString("NDS-BOOTSTRAP", "PRV_PATH", "");
			bootstrapini.SetInt("NDS-BOOTSTRAP", "DSI_MODE", perGameSettings_dsiMode == -1 ? DEFAULT_DSI_MODE : perGameSettings_dsiMode);
			bootstrapini.SetInt("NDS-BOOTSTRAP", "BOOST_CPU", setClockSpeed(false));
			bootstrapini.SetInt("NDS-BOOTSTRAP", "BOOST_VRAM", perGameSettings_boostVram == -1 ? DEFAULT_BOOST_VRAM : perGameSettings_boostVram);
		}
		bootstrapini.SetString("NDS-BOOTSTRAP", "HOMEBREW_ARG", "");
		bootstrapini.SetString("NDS-BOOTSTRAP", "RAM_DRIVE_PATH", "");
		bootstrapini.SetString("NDS-BOOTSTRAP", "GUI_LANGUAGE", ms().getGuiLanguageString());
		bootstrapini.SetInt("NDS-BOOTSTRAP", "LANGUAGE", ms().gameLanguage);
		bootstrapini.SaveIniFile(bootstrapinipath);

		if (ms().theme == TWLSettings::EThemeHBL) {
			fadeType = false;		  // Fade to black
		}

		while (!screenFadedOut()) { swiWaitForVBlank(); }

		int err = runNdsFile(argarray[0], argarray.size(), (const char **)&argarray[0], sys().isRunFromSD(), true, true, false, true, true, false, -1);
		char text[32];
		snprintf(text, sizeof(text), STR_START_FAILED_ERROR.c_str(), err);
		printLarge(false, 4, 4, text);
		if (err == 1) {
			printLarge(false, 4, 20, ms().bootstrapFile ? STR_BOOTSTRAP_NIGHTLY_NOT_FOUND : STR_BOOTSTRAP_RELEASE_NOT_FOUND);
		}
		printSmall(false, 4, 20 + calcLargeFontHeight(ms().bootstrapFile ? STR_BOOTSTRAP_NIGHTLY_NOT_FOUND : STR_BOOTSTRAP_RELEASE_NOT_FOUND), STR_PRESS_B_RETURN);
		updateText(false);
		fadeType = true;
		int pressed = 0;
		do {
			scanKeys();
			pressed = keysDownRepeat();
			checkSdEject();
			swiWaitForVBlank();
		} while (!(pressed & KEY_B));
		fadeType = false;	// Fade to white
		for (int i = 0; i < 25; i++) {
			swiWaitForVBlank();
		}
		// while (argarray.size() != 0) {
			free(argarray.at(0));
			argarray.erase(argarray.begin());
		// }
		argarray.push_back((char*)(sys().isRunFromSD() ? "sd:/_nds/TWiLightMenu/dsimenu.srldr" : "fat:/_nds/TWiLightMenu/dsimenu.srldr"));
		runNdsFile(argarray[0], argarray.size(), (const char**)&argarray[0], sys().isRunFromSD(), true, false, false, true, true, false, -1);
	}
	stop();
}

void launchManual(void) {
	snd().playLaunch();
	controlTopBright = true;

	fadeType = false;		  // Fade to white
	snd().fadeOutStream();
	for (int i = 0; i < 60; i++) {
		bgOperations(true);
	}
	mmEffectCancelAll();
	snd().stopStream();
	ms().saveSettings();
	// Launch manual
	argarray.push_back((char*)(sys().isRunFromSD() ? "sd:/_nds/TWiLightMenu/manual.srldr" : "fat:/_nds/TWiLightMenu/manual.srldr"));
	int err = runNdsFile(argarray[0], argarray.size(), (const char**)&argarray[0], sys().isRunFromSD(), true, false, false, true, true, false, -1);
	char text[32];
	snprintf(text, sizeof(text), STR_START_FAILED_ERROR.c_str(), err);
	fadeType = true;
	printLarge(false, 4, 4, text);
	stop();
}

void exitToSystemMenu(void) {
	snd().playLaunch();
	controlTopBright = true;

	fadeType = false;		  // Fade to white
	snd().fadeOutStream();
	for (int i = 0; i < 60; i++) {
		bgOperations(true);
	}
	
	mmEffectCancelAll();
	snd().stopStream();

	if (settingsChanged) {
		ms().saveSettings();
		settingsChanged = false;
	}
	if (!sdFound() || ms().launcherApp == -1) {
		*(u32 *)(0x02000300) = 0x434E4C54; // Set "CNLT" warmboot flag
		*(u16 *)(0x02000304) = 0x1801;
		*(u32 *)(0x02000310) = 0x4D454E55; // "MENU"
		unlaunchSetHiyaBoot();
	} else {
		extern char launcherPath[256];
		unlaunchRomBoot(launcherPath);
	}
	fifoSendValue32(FIFO_USER_02, 1); // ReturntoDSiMenu
}

void switchDevice(void) {
	if (bothSDandFlashcard()) {
		(ms().theme == TWLSettings::EThemeSaturn) ? snd().playLaunch() : snd().playSwitch();
		if (ms().theme != TWLSettings::EThemeSaturn && ms().theme != TWLSettings::EThemeHBL) {
			fadeType = false; // Fade to white
			for (int i = 0; i < 25; i++) {
				bgOperations(true);
			}
		}
		ms().secondaryDevice = !ms().secondaryDevice;
		if (!rocketVideo_playVideo || ms().showBoxArt)
			clearBoxArt(); // Clear box art
		if (ms().theme != TWLSettings::EThemeSaturn && ms().theme != TWLSettings::EThemeHBL) whiteScreen = true;
		boxArtLoaded = false;
		rocketVideo_playVideo = true;
		shouldersRendered = false;
		currentBg = 0;
		showSTARTborder = false;
		stopSoundPlayed = false;
		clearText();
		updateText(false);
		ms().saveSettings();
		settingsChanged = false;
		if (ms().theme == TWLSettings::EThemeSaturn) {
			snd().playStartup();
		}
	} else {
		snd().playLaunch();
		controlTopBright = true;

		if (ms().theme != TWLSettings::EThemeSaturn) {
			fadeType = false;		  // Fade to white
			snd().fadeOutStream();
			for (int i = 0; i < 60; i++) {
				bgOperations(true);
			}
			mmEffectCancelAll();

			snd().stopStream();
		}

		ms().slot1Launched = true;
		ms().saveSettings();

		bool directMethod = false;
		if (io_dldi_data->ioInterface.features & FEATURE_SLOT_GBA) {
			directMethod = true;
		} else if (ms().slot1LaunchMethod==0 || sys().arm7SCFGLocked()) {
			dsCardLaunch();
		} else if (ms().slot1LaunchMethod==2) {
			unlaunchRomBoot("cart:");
		} else {
			directMethod = true;
		}

		if (directMethod) {
			SetWidescreen(NULL);
			chdir(sys().isRunFromSD() ? "sd:/" : "fat:/");
			int err = runNdsFile("/_nds/TWiLightMenu/slot1launch.srldr", 0, NULL, sys().isRunFromSD(), true, true, false, true, true, false, -1);
			char text[32];
			snprintf(text, sizeof(text), STR_START_FAILED_ERROR.c_str(), err);
			fadeType = true;
			printLarge(false, 4, 4, text);
			stop();
		}
	}
}

void launchGba(void) {
	if (((u8*)GBAROM)[0xB2] != 0x96) {
		snd().playWrong();
		return;
	}

	snd().playLaunch();
	controlTopBright = true;

	fadeType = false;		  // Fade to white
	snd().fadeOutStream();
	for (int i = 0; i < 60; i++) {
		bgOperations(true);
	}
	
	mmEffectCancelAll();
	snd().stopStream();

	ms().slot1Launched = false;
	ms().saveSettings();

	// Switch to GBA mode
	/*if (ms().gbaBooter == TWLSettings::EGbaGbar2) {
		if (ms().secondaryDevice) {
			const char* gbar2Path = ms().gbar2DldiAccess ? "fat:/_nds/GBARunner2_arm7dldi_ds.nds" : "fat:/_nds/GBARunner2_arm9dldi_ds.nds";
			if (isDSiMode()) {
				gbar2Path = ms().consoleModel>0 ? "fat:/_nds/GBARunner2_arm7dldi_3ds.nds" : "fat:/_nds/GBARunner2_arm7dldi_dsi.nds";
			}
			if (perGameSettings_useBootstrap == -1 ? ms().useBootstrap : perGameSettings_useBootstrap) {
				int err = runNdsFile(gbar2Path, 0, NULL, sys().isRunFromSD(), true, true, false, true, false, -1);
				iprintf("Start failed. Error %i\n", err);
			} else {
				loadGameOnFlashcard(gbar2Path, false);
			}
		} else {
			std::string bootstrapPath = (ms().bootstrapFile ? "sd:/_nds/nds-bootstrap-hb-nightly.nds"
								: "sd:/_nds/nds-bootstrap-hb-release.nds");

			std::vector<char*> argarray;
			argarray.push_back(strdup(bootstrapPath.c_str()));
			argarray.at(0) = (char*)bootstrapPath.c_str();

			const char* gbar2Path = ms().consoleModel>0 ? "sd:/_nds/GBARunner2_arm7dldi_3ds.nds" : "sd:/_nds/GBARunner2_arm7dldi_dsi.nds";
			if (isDSiMode() && sys().arm7SCFGLocked()) {
				gbar2Path = ms().consoleModel>0 ? "sd:/_nds/GBARunner2_arm7dldi_nodsp_3ds.nds" : "sd:/_nds/GBARunner2_arm7dldi_nodsp_dsi.nds";
			}

			CIniFile bootstrapini(BOOTSTRAP_INI);
			bootstrapini.SetString("NDS-BOOTSTRAP", "NDS_PATH", gbar2Path);
			bootstrapini.SetString("NDS-BOOTSTRAP", "HOMEBREW_ARG", "");
			bootstrapini.SetString("NDS-BOOTSTRAP", "RAM_DRIVE_PATH", "");
			bootstrapini.SetInt("NDS-BOOTSTRAP", "LANGUAGE", ms().gameLanguage);
			bootstrapini.SetInt("NDS-BOOTSTRAP", "DSI_MODE", 0);
			bootstrapini.SetInt("NDS-BOOTSTRAP", "BOOST_CPU", 1);
			bootstrapini.SetInt("NDS-BOOTSTRAP", "BOOST_VRAM", 0);
			bootstrapini.SaveIniFile(BOOTSTRAP_INI);
			if (!isDSiMode() && !ms().secondaryDevice) {
				extern void ntrStartSdGame();
				ntrStartSdGame();
			}
			int err = runNdsFile(argarray[0], argarray.size(), (const char **)&argarray[0], sys().isRunFromSD(), false, true, false, true, true, -1);
			char text[32];
			snprintf(text, sizeof(text), STR_START_FAILED_ERROR.c_str(), err);
			fadeType = true;
			printLarge(false, 4, 4, text);
			if (err == 1) {
				printLarge(false, 4, 20, ms().bootstrapFile ? STR_BOOTSTRAP_NIGHTLY_NOT_FOUND : STR_BOOTSTRAP_RELEASE_NOT_FOUND);
			}
			updateText(false);
			stop();
		}
	} else {*/
		gbaSwitch();
	//}
}

void mdRomTooBig(void) {
	// int bottomBright = 0;

	if (ms().theme == TWLSettings::EThemeSaturn) {
		snd().playStartup();
		fadeType = false;	   // Fade to black
		for (int i = 0; i < 25; i++) {
			bgOperations(true);
		}
		currentBg = 1;
		displayGameIcons = false;
		fadeType = true;
	} else {
		snd().playWrong();
		showdialogbox = true;
	}
	clearText();
	updateText(false);
	if (ms().theme == TWLSettings::EThemeSaturn) {
		while (!screenFadedIn()) { bgOperations(true); }
		snd().playWrong();
	} else {
		while (!dboxStopped) { bgOperations(true); }
	}
	printSmall(false, 0, 64, STR_MD_ROM_TOO_BIG, Alignment::center, FontPalette::dialog);
	printSmall(false, 0, 160, STR_A_OK, Alignment::center, FontPalette::dialog);
	updateText(false);
	int pressed = 0;
	do {
		scanKeys();
		pressed = keysDown();
		updateBoxArt();
		bgOperations(true);

		// Debug code for changing brightness of BG layer

		/*if (pressed & KEY_UP) {
			bottomBright--;
		} else if (pressed & KEY_DOWN) {
			bottomBright++;
		}
		


		if (bottomBright < 0) bottomBright = 0;
		if (bottomBright > 15) bottomBright = 15;

		switch (bottomBright) {
			case 0:
			default:
				REG_BLDY = 0;
				break;
			case 1:
				REG_BLDY = (0b0001 << 1);
				break;
			case 2:
				REG_BLDY = (0b0010 << 1);
				break;
			case 3:
				REG_BLDY = (0b0011 << 1);
				break;
			case 4:
				REG_BLDY = (0b0100 << 1);
				break;
			case 5:
				REG_BLDY = (0b0101 << 1);
				break;
			case 6:
				REG_BLDY = (0b0110 << 1);
				break;
			case 7:
				REG_BLDY = (0b0111 << 1);
				break;
			case 8:
				REG_BLDY = (0b1000 << 1);
				break;
			case 9:
				REG_BLDY = (0b1001 << 1);
				break;
			case 10:
				REG_BLDY = (0b1010 << 1);
				break;
			case 11:
				REG_BLDY = (0b1011 << 1);
				break;
			case 12:
				REG_BLDY = (0b1100 << 1);
				break;
			case 13:
				REG_BLDY = (0b1101 << 1);
				break;
			case 14:
				REG_BLDY = (0b1110 << 1);
				break;
			case 15:
				REG_BLDY = (0b1111 << 1);
				break;
		}*/
	} while (!(pressed & KEY_A));
	snd().playBack();
	showdialogbox = false;
	if (ms().theme == TWLSettings::EThemeSaturn) {
		fadeType = false;	   // Fade to black
		for (int i = 0; i < 25; i++) {
			bgOperations(true);
		}
		clearText();
		currentBg = 0;
		displayGameIcons = true;
		fadeType = true;
		snd().playStartup();
	} else {
		clearText();
	}
	updateText(false);
}

void ramDiskMsg(const char *filename) {
	clearText();
	updateText(false);
	snd().playWrong();
	if (ms().theme != TWLSettings::EThemeSaturn) {
		dbox_showIcon = true;
		showdialogbox = true;
		while (!dboxStopped) {
			bgOperations(true);
		}
		titleUpdate(false, filename, CURPOS);
	}
	dirContName = filename;
	// About 35 characters fit in the box.
	if (strlen(dirContName.c_str()) > 35) {
		// Truncate to 32, 32 + 3 = 35 (because we append "...").
		dirContName.resize(32, ' ');
		size_t first = dirContName.find_first_not_of(' ');
		size_t last = dirContName.find_last_not_of(' ');
		dirContName = dirContName.substr(first, (last - first + 1));
		dirContName.append("...");
	}
	printSmall(false, 16, 66, dirContName, Alignment::left, FontPalette::dialog);
	printSmall(false, 0, (ms().theme == TWLSettings::EThemeSaturn ? 24 : 112), STR_RAM_DISK_REQUIRED, Alignment::center, FontPalette::dialog);
	printSmall(false, 0, (ms().theme == TWLSettings::EThemeSaturn ? 64 : 160), STR_A_OK, Alignment::center, FontPalette::dialog);
	updateText(false);
	int pressed = 0;
	do {
		scanKeys();
		pressed = keysDown();
		updateBoxArt();
		bgOperations(true);
	} while (!(pressed & KEY_A));
	clearText();
	updateText(false);
	if (ms().theme == TWLSettings::EThemeSaturn) {
		snd().playLaunch();
	} else {
		if (ms().theme == TWLSettings::EThemeHBL) {
			dbox_showIcon = false;
		}
		showdialogbox = false;
	}
}

bool dsiBinariesMissingMsg(const char *filename) {
	bool proceedToLaunch = false;

	clearText();
	updateText(false);
	snd().playWrong();
	if (ms().theme != TWLSettings::EThemeSaturn) {
		dbox_showIcon = true;
		showdialogbox = true;
		while (!dboxStopped) {
			bgOperations(true);
		}
		titleUpdate(false, filename, CURPOS);
		dirContName = filename;
		// About 35 characters fit in the box.
		if (strlen(dirContName.c_str()) > 35) {
			// Truncate to 32, 32 + 3 = 35 (because we append "...").
			dirContName.resize(32, ' ');
			size_t first = dirContName.find_first_not_of(' ');
			size_t last = dirContName.find_last_not_of(' ');
			dirContName = dirContName.substr(first, (last - first + 1));
			dirContName.append("...");
		}
		printSmall(false, 16, 66, dirContName, Alignment::left, FontPalette::dialog);
	}
	printSmall(false, 0, (ms().theme == TWLSettings::EThemeSaturn ? 8 : 96), STR_DSIBINARIES_MISSING, Alignment::center, FontPalette::dialog);
	printSmall(false, 0, (ms().theme == TWLSettings::EThemeSaturn ? 64 : 160), STR_Y_DS_MODE_B_BACK, Alignment::center, FontPalette::dialog);
	updateText(false);
	while (1) {
		scanKeys();
		int pressed = keysDown();
		updateBoxArt();
		bgOperations(true);
		if (pressed & KEY_Y) {
			dsModeForced = true;
			proceedToLaunch = true;
			break;
		} else if (pressed & KEY_B) {
			snd().playBack();
			proceedToLaunch = false;
			break;
		}
	}
	clearText();
	updateText(false);

	if (ms().theme == TWLSettings::EThemeSaturn) {
		snd().playLaunch();
	} else {
		if (ms().theme == TWLSettings::EThemeHBL) {
			dbox_showIcon = false;
		}
		showdialogbox = false;
		for (int i = 0; i < (proceedToLaunch ? 20 : 15); i++) {
			bgOperations(true);
		}
	}

	return proceedToLaunch;
}

bool donorRomMsg(const char *filename) {
	bool proceedToLaunch = false;
	clearText();
	updateText(false);
	snd().playWrong();
	if (ms().theme != TWLSettings::EThemeSaturn) {
		dbox_showIcon = true;
		showdialogbox = true;
		while (!dboxStopped) {
			bgOperations(true);
		}
		dirContName = filename;
		// About 35 characters fit in the box.
		if (strlen(dirContName.c_str()) > 35) {
			// Truncate to 32, 32 + 3 = 35 (because we append "...").
			dirContName.resize(32, ' ');
			size_t first = dirContName.find_first_not_of(' ');
			size_t last = dirContName.find_last_not_of(' ');
			dirContName = dirContName.substr(first, (last - first + 1));
			dirContName.append("...");
		}
	}
	bool ubongo = (memcmp(gameTid[CURPOS], "KUB", 3) == 0);
	int msgPage = 0;
	bool pageLoaded = false;
	bool secondPageViewed = false;
	bool dsModeAllowed = ((requiresDonorRom[CURPOS] == 52 || requiresDonorRom[CURPOS] == 53) && !isDSiWare[CURPOS]);
	bool vramWifi = false;
	if ((!dsiFeatures() || bs().b4dsMode) && ms().secondaryDevice && isDSiWare[CURPOS]) {
		// Find DSiWare title which requires VRAM-WiFi donor ROM
		// TODO: If the list gets large enough, switch to bsearch().
		for (unsigned int i = 0; i < sizeof(compatibleGameListB4DSMEP)/sizeof(compatibleGameListB4DSMEP[0]); i++) {
			if (memcmp(gameTid[CURPOS], compatibleGameListB4DSMEP[i], (compatibleGameListB4DSMEP[i][3] != 0 ? 4 : 3)) == 0) {
				// Found match
				vramWifi = (compatibleGameListB4DSMEPID[i] == 3);
				break;
			}
		}
	}

	int arrowXpos = (ms().theme == TWLSettings::ETheme3DS) ? 20 : 12;
	int arrowYpos = (ms().theme == TWLSettings::EThemeSaturn) ? 64 : 160;
	int yPos = (ms().theme == TWLSettings::EThemeSaturn) ? 8 : 96;

	int pressed = 0;
	while (1) {
		if (!pageLoaded) {
			clearText();
			if (ms().theme != TWLSettings::EThemeSaturn) {
				titleUpdate(false, filename, CURPOS);
				printSmall(false, (ms().theme == TWLSettings::ETheme3DS) ? 24 : 16, 66, dirContName.c_str(), Alignment::left, FontPalette::dialog);
			}
			if (msgPage == 1) {
				switch (requiresDonorRom[CURPOS]) {
					default:
						break;
					case 20:
						printSmall(false, 0, yPos, STR_HOW_TO_SET_DONOR_ROM_SDK20, Alignment::center, FontPalette::dialog);
						break;
					case 51:
						printSmall(false, 0, yPos, ((!dsiFeatures() || bs().b4dsMode) && ms().secondaryDevice && !ubongo) ? (vramWifi ? STR_HOW_TO_SET_DONOR_ROM_VRAM_WIFI_SDK5 : STR_HOW_TO_SET_DONOR_ROM_SDK5) : STR_HOW_TO_SET_DONOR_ROM_SDK5TWL, Alignment::center, FontPalette::dialog);
						break;
					case 52:
						printSmall(false, 0, yPos, STR_HOW_TO_SET_DONOR_ROM_SDK5TWLONLY, Alignment::center, FontPalette::dialog);
						break;
					case 151:
						printSmall(false, 0, yPos, STR_HOW_TO_SET_DONOR_ROM_SDK50TWL, Alignment::center, FontPalette::dialog);
						break;
					case 152:
						printSmall(false, 0, yPos, STR_HOW_TO_SET_DONOR_ROM_SDK50TWLONLY, Alignment::center, FontPalette::dialog);
						break;
				}
				printSmall(false, arrowXpos, arrowYpos, "<", Alignment::left, FontPalette::dialog);
			} else {
				switch (requiresDonorRom[CURPOS]) {
					default:
						break;
					case 20:
						printSmall(false, 0, yPos, STR_DONOR_ROM_MSG_SDK20, Alignment::center, FontPalette::dialog);
						break;
					case 51:
						printSmall(false, 0, yPos, ((!dsiFeatures() || bs().b4dsMode) && ms().secondaryDevice && !ubongo) ? STR_DONOR_ROM_MSG_SDK5 : STR_DONOR_ROM_MSG_SDK5TWL, Alignment::center, FontPalette::dialog);
						break;
					case 52:
						printSmall(false, 0, yPos, !isDSiWare[CURPOS] ? STR_DONOR_ROM_MSG_SDK5TWLONLY_DSI_MODE : STR_DONOR_ROM_MSG_SDK5TWLONLY, Alignment::center, FontPalette::dialog);
						break;
					case 151:
						printSmall(false, 0, yPos, STR_DONOR_ROM_MSG_SDK50TWL, Alignment::center, FontPalette::dialog);
						break;
					case 152:
						printSmall(false, 0, yPos, STR_DONOR_ROM_MSG_SDK50TWLONLY, Alignment::center, FontPalette::dialog);
						break;
				}
				printSmall(false, 256 - arrowXpos, arrowYpos, ">", Alignment::right, FontPalette::dialog);
			}
			if (secondPageViewed) {
				printSmall(false, 0, arrowYpos, dsModeAllowed ? STR_Y_DS_MODE_B_BACK : STR_B_BACK, Alignment::center, FontPalette::dialog);
			}
			updateText(false);
			pageLoaded = true;
		}
		scanKeys();
		pressed = keysDown();
		updateBoxArt();
		bgOperations(true);
		if ((pressed & KEY_LEFT) && msgPage != 0) {
			snd().playSelect();
			msgPage = 0;
			pageLoaded = false;
		} else if (((pressed & KEY_RIGHT) || (((pressed & KEY_B) || (pressed & KEY_A)) && !secondPageViewed)) && msgPage != 1) {
			snd().playSelect();
			msgPage = 1;
			secondPageViewed = true;
			pageLoaded = false;
		} else if (dsModeAllowed && (pressed & KEY_Y)) {
			dsModeForced = true;
			proceedToLaunch = true;
			pressed = 0;
			break;
		} else if ((pressed & KEY_B) && secondPageViewed) {
			snd().playBack();
			proceedToLaunch = false;
			pressed = 0;
			break;
		}
	}
	clearText();
	
	showdialogbox = false;
	if (ms().theme == TWLSettings::EThemeSaturn) {
		snd().playLaunch();
		updateText(false);
	} else {
		if (ms().theme == TWLSettings::EThemeHBL) {
			dbox_showIcon = false;
		}

		clearText();
		updateText(false);
		for (int i = 0; i < (proceedToLaunch ? 20 : 15); i++) {
			bgOperations(true);
		}
	}

	return proceedToLaunch;
}

bool checkForCompatibleGame(const char *filename) {
	bool proceedToLaunch = true;

	/* if (!dsiFeatures() && ms().secondaryDevice) {
		// TODO: If the list gets large enough, switch to bsearch().
		for (unsigned int i = 0; i < sizeof(incompatibleGameListB4DS)/sizeof(incompatibleGameListB4DS[0]); i++) {
			if (memcmp(gameTid[CURPOS], incompatibleGameListB4DS[i], 3) == 0) {
				// Found match
				proceedToLaunch = false;
				break;
			}
		}
	} */

	if (ms().secondaryDevice) {
		// TODO: If the list gets large enough, switch to bsearch().
		for (unsigned int i = 0; i < sizeof(incompatibleGameListFC)/sizeof(incompatibleGameListFC[0]); i++) {
			if (memcmp(gameTid[CURPOS], incompatibleGameListFC[i], 3) == 0) {
				// Found match
				proceedToLaunch = false;
				break;
			}
		}
	}

	/* if (proceedToLaunch) {
		// TODO: If the list gets large enough, switch to bsearch().
		for (unsigned int i = 0; i < sizeof(incompatibleGameList)/sizeof(incompatibleGameList[0]); i++) {
			if (memcmp(gameTid[CURPOS], incompatibleGameList[i], 3) == 0) {
				// Found match
				proceedToLaunch = false;
				break;
			}
		}
	} */

	if (proceedToLaunch) return true;	// Game is compatible

	if (ms().theme == TWLSettings::EThemeSaturn) {
		snd().playStartup();
		fadeType = false;	   // Fade to black
		for (int i = 0; i < 25; i++) {
			bgOperations(true);
		}
		currentBg = 1;
		displayGameIcons = false;
		fadeType = true;
	} else {
		dbox_showIcon = true;
		showdialogbox = true;
	}
	clearText();
	updateText(false);
	if (ms().theme == TWLSettings::EThemeSaturn) {
		while (!screenFadedIn()) { bgOperations(true); }
		dbox_showIcon = true;
		snd().playWrong();
	} else {
		while (!dboxStopped) { bgOperations(true); }
	}
	titleUpdate(false, filename, CURPOS);
	printSmall(false, 0, 72, STR_GAME_INCOMPATIBLE_MSG, Alignment::center, FontPalette::dialog);
	printSmall(false, 0, 160, STR_A_IGNORE_B_DONT_LAUNCH, Alignment::center, FontPalette::dialog);
	updateText(false);
	while (1) {
		scanKeys();
		int pressed = keysDown();
		updateBoxArt();
		bgOperations(true);
		if (pressed & KEY_A) {
			proceedToLaunch = true;
			break;
		} else if (pressed & KEY_B) {
			snd().playBack();
			proceedToLaunch = false;
			break;
		}
	}
	showdialogbox = false;

	if (ms().theme == TWLSettings::EThemeSaturn) {
		fadeType = false;	   // Fade to black
		for (int i = 0; i < 25; i++) {
			bgOperations(true);
		}
		clearText();
		updateText(false);
		currentBg = 0;
		displayGameIcons = true;
		fadeType = true;
		snd().playStartup();
		if (proceedToLaunch) {
			while (!screenFadedIn()) { swiWaitForVBlank(); }
		}
	} else {
		if (ms().theme == TWLSettings::EThemeHBL) {
			dbox_showIcon = false;
		}

		clearText();
		updateText(false);
		for (int i = 0; i < (proceedToLaunch ? 20 : 15); i++) {
			bgOperations(true);
		}
	}
	dbox_showIcon = false;

	return proceedToLaunch;
}

bool gameCompatibleMemoryPit(void) {
	// TODO: If the list gets large enough, switch to bsearch().
	for (unsigned int i = 0; i < sizeof(incompatibleGameListMemoryPit)/sizeof(incompatibleGameListMemoryPit[0]); i++) {
		if (memcmp(gameTid[CURPOS], incompatibleGameListMemoryPit[i], 3) == 0) {
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
		if (memcmp(gameTid[CURPOS], gbaGameListBiosReqiure[i], 3) == 0) {
			// Found match
			return true;
		}
	}

	return false;
}

bool dsiWareInDSModeMsg(std::string filename) {
	if (ms().dontShowDSiWareInDSModeWarning) {
		return true;
	}

	bool proceedToLaunch = true;

	if (ms().theme == TWLSettings::EThemeSaturn) {
		snd().playStartup();
		fadeType = false;	   // Fade to black
		for (int i = 0; i < 25; i++) {
			bgOperations(true);
		}
		currentBg = 1;
		displayGameIcons = false;
		fadeType = true;
	} else {
		dbox_showIcon = true;
		showdialogbox = true;
	}
	clearText();
	updateText(false);
	if (ms().theme == TWLSettings::EThemeSaturn) {
		while (!screenFadedIn()) { bgOperations(true); }
		dbox_showIcon = true;
		snd().playWrong();
	} else {
		while (!dboxStopped) { bgOperations(true); }
	}
	int msgPage = 0;
	bool pageLoaded = false;
	bool secondPageViewed = false;

	int arrowXpos = (ms().theme == TWLSettings::ETheme3DS) ? 20 : 12;
	int yPos = 102;

	while (1) {
		if (!pageLoaded) {
			clearText();
			titleUpdate(false, filename.c_str(), CURPOS);
			switch (msgPage) {
				case 0:
					printSmall(false, 0, yPos - ((calcSmallFontHeight(STR_DSIWARE_DS_MODE_P1) - smallFontHeight()) / 2), STR_DSIWARE_DS_MODE_P1, Alignment::center, FontPalette::dialog);
					printSmall(false, 256 - arrowXpos, 160, ">", Alignment::right, FontPalette::dialog);
					break;
				case 1:
					printSmall(false, 0, yPos - ((calcSmallFontHeight(STR_DSIWARE_DS_MODE_P2) - smallFontHeight()) / 2), STR_DSIWARE_DS_MODE_P2, Alignment::center, FontPalette::dialog);
					printSmall(false, arrowXpos, 160, "<", Alignment::left, FontPalette::dialog);
					break;
			}
			if (secondPageViewed) {
				printSmall(false, 0, 160, STR_B_A_OK_X_DONT_SHOW, Alignment::center, FontPalette::dialog);
			}
			updateText(false);
			pageLoaded = true;
		}
		scanKeys();
		int pressed = keysDown();
		updateBoxArt();
		bgOperations(true);
		if ((pressed & KEY_LEFT) && msgPage != 0) {
			snd().playSelect();
			msgPage = 0;
			pageLoaded = false;
		} else if (((pressed & KEY_RIGHT) || (((pressed & KEY_B) || (pressed & KEY_A)) && !secondPageViewed)) && msgPage != 1) {
			snd().playSelect();
			msgPage = 1;
			secondPageViewed = true;
			pageLoaded = false;
		} else if ((pressed & KEY_A) && secondPageViewed) {
			proceedToLaunch = true;
			break;
		} else if ((pressed & KEY_B) && secondPageViewed) {
			snd().playBack();
			proceedToLaunch = false;
			break;
		} else if (pressed & KEY_X) {
			ms().dontShowDSiWareInDSModeWarning = true;
			proceedToLaunch = true;
			break;
		}
	}
	showdialogbox = false;
	if (ms().theme == TWLSettings::EThemeSaturn) {
		fadeType = false;	   // Fade to black
		for (int i = 0; i < 25; i++) {
			bgOperations(true);
		}
		clearText();
		updateText(false);
		currentBg = 0;
		displayGameIcons = true;
		fadeType = true;
		snd().playStartup();
		if (proceedToLaunch) {
			while (!screenFadedIn()) { bgOperations(true); }
		}
	} else {
		if (ms().theme == TWLSettings::EThemeHBL) {
			dbox_showIcon = false;
		}

		clearText();
		updateText(false);
		for (int i = 0; i < (proceedToLaunch ? 20 : 15); i++) {
			bgOperations(true);
		}
	}
	dbox_showIcon = false;

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
		if (memcmp(gameTid[CURPOS], compatibleGameListB4DSMEP[i], (compatibleGameListB4DSMEP[i][3] != 0 ? 4 : 3)) == 0) {
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
				if (memcmp(gameTid[CURPOS], compatibleGameListB4DSDebugRAMLimited[i], 3) == 0) {
					// Found match
					showMsg = true;
					msgId = compatibleGameListB4DSDebugRAMLimitedID[i];
					break;
				}
			}
		} else {
			// TODO: If the list gets large enough, switch to bsearch().
			for (unsigned int i = 0; i < sizeof(compatibleGameListB4DSRAMLimited)/sizeof(compatibleGameListB4DSRAMLimited[0]); i++) {
				if (memcmp(gameTid[CURPOS], compatibleGameListB4DSRAMLimited[i], 3) == 0) {
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
			if (memcmp(gameTid[CURPOS], compatibleGameListB4DSAllRAMLimited[i], 3) == 0) {
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

	if (ms().theme == TWLSettings::EThemeSaturn) {
		snd().playStartup();
		fadeType = false;	   // Fade to black
		for (int i = 0; i < 25; i++) {
			bgOperations(true);
		}
		currentBg = 1;
		displayGameIcons = false;
		fadeType = true;
	} else {
		dbox_showIcon = true;
		showdialogbox = true;
	}
	clearText();
	updateText(false);
	if (ms().theme == TWLSettings::EThemeSaturn) {
		while (!screenFadedIn()) { bgOperations(true); }
		dbox_showIcon = true;
		snd().playWrong();
	} else {
		if (msgId == 10) {
			snd().playWrong();
		}
		while (!dboxStopped) { bgOperations(true); }
	}
	titleUpdate(false, filename.c_str(), CURPOS);
	int yPos = (ms().theme == TWLSettings::EThemeSaturn ? 30 : 102);
	switch (msgId) {
		case 0:
			printSmall(false, 0, yPos - ((calcSmallFontHeight(STR_RAM_LIMIT_GAME_PART_ONLY) - smallFontHeight()) / 2), STR_RAM_LIMIT_GAME_PART_ONLY, Alignment::center, FontPalette::dialog);
			break;
		case 1:
			printSmall(false, 0, yPos - ((calcSmallFontHeight(STR_RAM_LIMIT_NO_AUDIO) - smallFontHeight()) / 2), STR_RAM_LIMIT_NO_AUDIO, Alignment::center, FontPalette::dialog);
			break;
		case 2:
			printSmall(false, 0, yPos - ((calcSmallFontHeight(STR_RAM_LIMIT_NO_MUSIC) - smallFontHeight()) / 2), STR_RAM_LIMIT_NO_MUSIC, Alignment::center, FontPalette::dialog);
			break;
		case 3:
			printSmall(false, 0, yPos - ((calcSmallFontHeight(STR_RAM_LIMIT_SPECIFIC_AREA) - smallFontHeight()) / 2), STR_RAM_LIMIT_SPECIFIC_AREA, Alignment::center, FontPalette::dialog);
			break;
		case 4:
			printSmall(false, 0, yPos - ((calcSmallFontHeight(STR_RAM_LIMIT_CERTAIN_POINT) - smallFontHeight()) / 2), STR_RAM_LIMIT_CERTAIN_POINT, Alignment::center, FontPalette::dialog);
			break;
		case 5:
			printSmall(false, 0, yPos - ((calcSmallFontHeight(STR_RAM_LIMIT_NO_FMV) - smallFontHeight()) / 2), STR_RAM_LIMIT_NO_FMV, Alignment::center, FontPalette::dialog);
			break;
		case 6:
			printSmall(false, 0, yPos - ((calcSmallFontHeight(STR_RAM_LIMIT_STATE) - smallFontHeight()) / 2), STR_RAM_LIMIT_STATE, Alignment::center, FontPalette::dialog);
			break;
		case 7:
			printSmall(false, 0, yPos - ((calcSmallFontHeight(STR_RAM_LIMIT_NO_SAVE_STATE) - smallFontHeight()) / 2), STR_RAM_LIMIT_NO_SAVE_STATE, Alignment::center, FontPalette::dialog);
			break;
		case 8:
			printSmall(false, 0, yPos - ((calcSmallFontHeight(STR_RAM_LIMIT_NO_SOUND_FX) - smallFontHeight()) / 2), STR_RAM_LIMIT_NO_SOUND_FX, Alignment::center, FontPalette::dialog);
			break;
		case 10:
			if (sys().isRegularDS()) {
				printSmall(false, 0, yPos - ((calcSmallFontHeight(STR_INSERT_MEMORY_EXPANSION_PAK) - smallFontHeight()) / 2), STR_INSERT_MEMORY_EXPANSION_PAK, Alignment::center, FontPalette::dialog);
			} else {
				printSmall(false, 0, yPos - ((calcSmallFontHeight(STR_MEMORY_EXPANSION_PAK_UNAVAILABLE) - smallFontHeight()) / 2), STR_MEMORY_EXPANSION_PAK_UNAVAILABLE, Alignment::center, FontPalette::dialog);
			}
			break;
		case 11:
			if (sys().isRegularDS()) {
				if (mepFound) {
					printSmall(false, 0, yPos - ((calcSmallFontHeight(STR_INSERT_LARGER_THAN_MEP) - smallFontHeight()) / 2), STR_INSERT_LARGER_THAN_MEP, Alignment::center, FontPalette::dialog);
				} else {
					printSmall(false, 0, yPos - ((calcSmallFontHeight(STR_INSERT_SLOT2_RAM_CART) - smallFontHeight()) / 2), STR_INSERT_SLOT2_RAM_CART, Alignment::center, FontPalette::dialog);
				}
			} else {
				printSmall(false, 0, yPos - ((calcSmallFontHeight(STR_SLOT2_RAM_CART_UNAVAILABLE) - smallFontHeight()) / 2), STR_SLOT2_RAM_CART_UNAVAILABLE, Alignment::center, FontPalette::dialog);
			}
			break;
		case 12:
			printSmall(false, 0, yPos - ((calcSmallFontHeight(STR_DONOR_ROM_INCOMPATIBLE_VRAM_WIFI_SDK5) - smallFontHeight()) / 2), STR_DONOR_ROM_INCOMPATIBLE_VRAM_WIFI_SDK5, Alignment::center, FontPalette::dialog);
			break;
	}
	if (msgId >= 10) {
		printSmall(false, 0, (ms().theme == TWLSettings::EThemeSaturn ? 64 : 160), STR_A_OK, Alignment::center, FontPalette::dialog);
	} else {
		printSmall(false, 0, (ms().theme == TWLSettings::EThemeSaturn ? 64 : 160), STR_B_A_OK_X_DONT_SHOW, Alignment::center, FontPalette::dialog);
	}
	updateText(false);
	if (msgId >= 10) {
		while (1) {
			scanKeys();
			int pressed = keysDown();
			updateBoxArt();
			bgOperations(true);
			if ((pressed & KEY_A) || (pressed & KEY_B)) {
				snd().playBack();
				proceedToLaunch = false;
				break;
			}
		}
	} else {
		while (1) {
			scanKeys();
			int pressed = keysDown();
			updateBoxArt();
			bgOperations(true);
			if (pressed & KEY_A) {
				proceedToLaunch = true;
				break;
			} else if (pressed & KEY_B) {
				snd().playBack();
				proceedToLaunch = false;
				break;
			} else if (pressed & KEY_X) {
				dontShowRAMLimitMsgAgain(filename);
				proceedToLaunch = true;
				break;
			}
		}
	}
	showdialogbox = false;
	if (ms().theme == TWLSettings::EThemeSaturn) {
		fadeType = false;	   // Fade to black
		for (int i = 0; i < 25; i++) {
			bgOperations(true);
		}
		clearText();
		updateText(false);
		currentBg = 0;
		displayGameIcons = true;
		fadeType = true;
		snd().playStartup();
		if (proceedToLaunch) {
			while (!screenFadedIn()) { bgOperations(true); }
		}
	} else {
		if (ms().theme == TWLSettings::EThemeHBL) {
			dbox_showIcon = false;
		}
		clearText();
		updateText(false);
		for (int i = 0; i < (proceedToLaunch ? 20 : 15); i++) {
			bgOperations(true);
		}
	}
	dbox_showIcon = false;

	return proceedToLaunch;
}

bool dsiWareCompatibleB4DS(void) {
	if (memcmp(gameTid[CURPOS], "NTRJ", 4) == 0) {
		return true; // No check necessary for NTRJ titles (They are uncommon, and "NTRJ" is not seen in retail titles)
	}

	bool res = false;

	// TODO: If the list gets large enough, switch to bsearch().
	for (unsigned int i = 0; i < sizeof(compatibleGameListB4DS)/sizeof(compatibleGameListB4DS[0]); i++) {
		if (memcmp(gameTid[CURPOS], compatibleGameListB4DS[i], (compatibleGameListB4DS[i][3] != 0 ? 4 : 3)) == 0) {
			// Found match
			res = true;
			break;
		}
	}
	if (!res && (sys().dsDebugRam() || bs().b4dsMode == 2)) {
		for (unsigned int i = 0; i < sizeof(compatibleGameListB4DSDebug)/sizeof(compatibleGameListB4DSDebug[0]); i++) {
			if (memcmp(gameTid[CURPOS], compatibleGameListB4DSDebug[i], (compatibleGameListB4DSDebug[i][3] != 0 ? 4 : 3)) == 0) {
				// Found match
				res = true;
				break;
			}
		}
	}
	return res;
}

bool cannotLaunchMsg(const char *filename) {
	bool res = false;

	clearText();
	updateText(false);
	snd().playWrong();
	if (ms().theme != TWLSettings::EThemeSaturn) {
		dbox_showIcon = true;
		showdialogbox = true;
		while (!dboxStopped) { bgOperations(true); }
		titleUpdate(false, filename, CURPOS);
	}
	const std::string *str = nullptr;
	if (!isValid[CURPOS]) {
		str = (ms().secondaryDevice || ms().showMicroSd) ? &STR_CANNOT_LAUNCH_CORRUPT_TITLE_MICRO_SD : &STR_CANNOT_LAUNCH_CORRUPT_TITLE_SD;
	} else if (isTwlm[CURPOS]) {
		str = &STR_TWLMENU_ALREADY_RUNNING;
	} else if (isUnlaunch[CURPOS]) {
		str = &STR_CANNOT_LAUNCH_WITH_UI;
	} else if (bnrRomType[CURPOS] == 1) {
		str = &STR_GBA_BIOS_ERROR_DESC;
	} else if (bnrRomType[CURPOS] != 0) {
		str = ms().consoleModel >= 2 ? &STR_RELAUNCH_3DS_HOME : &STR_RELAUNCH_UNLAUNCH;
	} else if (isDSiMode() && isDSiWare[CURPOS] && !ms().secondaryDevice && sys().arm7SCFGLocked()) {
		str = ms().consoleModel >= 2 ? &STR_RELAUNCH_DSIWARE_3DS_HOME : &STR_RELAUNCH_DSIWARE_UNLAUNCH;
	} else /*if (isHomebrew[CURPOS] && ms().consoleModel >= 2) {
		str = &STR_CANNOT_LAUNCH_HB_ON_3DS;
	} else*/ if (sys().isRegularDS()) {
		str = &STR_FOR_USE_WITH_DSI_ONLY;
	} else {
		str = /*isDSiMode() ? &STR_CANNOT_LAUNCH_WITHOUT_SD :*/ &STR_CANNOT_LAUNCH_IN_DS_MODE;
	}
	int yPos = (ms().theme == TWLSettings::EThemeSaturn ? 30 : 102);
	printSmall(false, 0, yPos - ((calcSmallFontHeight(*str) - smallFontHeight()) / 2), *str, Alignment::center, FontPalette::dialog);

	printSmall(false, 0, (ms().theme == TWLSettings::EThemeSaturn ? 64 : 160), STR_A_OK, Alignment::center, FontPalette::dialog);
	updateText(false);
	int pressed = 0;
	while (1) {
		scanKeys();
		pressed = keysDown();
		updateBoxArt();
		bgOperations(true);

		if (pressed & KEY_A) {
			break;
		}
		if ((pressed & KEY_Y) && bnrRomType[CURPOS] == 0 && !isDSiWare[CURPOS] && gameTid[CURPOS][0] == 'D') {
			// Hidden button to launch anyways
			res = true;
			break;
		}
	}
	clearText();
	updateText(false);
	if (ms().theme == TWLSettings::EThemeSaturn) {
		snd().playLaunch();
	} else {
		if (ms().theme == TWLSettings::EThemeHBL) {
			dbox_showIcon = false;
		}
		showdialogbox = false;
	}

	return res;
}

bool selectMenu(void) {
	inSelectMenu = true;
	dbox_showIcon = false;
	if (ms().theme == TWLSettings::EThemeSaturn) {
		snd().playStartup();
		fadeType = false;	   // Fade to black
		for (int i = 0; i < 25; i++) {
			bgOperations(true);
		}
		currentBg = 1;
		displayGameIcons = false;
		fadeType = true;
	} else {
		dbox_selectMenu = true;
		showdialogbox = true;
	}
	clearText();
	updateText(false);
	if (!rocketVideo_playVideo || ms().showBoxArt)
		clearBoxArt(); // Clear box art
	boxArtLoaded = false;
	rocketVideo_playVideo = true;
	int maxCursors = 0;
	int selCursorPosition = 0;
	int assignedOp[5] = {-1};
	int selIconYpos = 96;
	if (ms().kioskMode) {
		if (dsiFeatures() && memcmp(io_dldi_data->friendlyName, "CycloDS iEvolution", 18) != 0) {
			for (int i = 0; i < 3; i++) {
				selIconYpos -= 14;
			}
			assignedOp[0] = 0;
			assignedOp[1] = 2;
			assignedOp[2] = 4;
			maxCursors = 2;
		} else {
			for (int i = 0; i < 2; i++) {
				selIconYpos -= 14;
			}
			if (!sys().isRegularDS()) {
				assignedOp[0] = 0;
				assignedOp[1] = 4;
				maxCursors = 1;
			} else if (io_dldi_data->ioInterface.features & FEATURE_SLOT_GBA) {
				assignedOp[0] = 2;
				assignedOp[1] = 4;
				maxCursors = 1;
			} else if (ms().gbaBooter == TWLSettings::EGbaGbar2) {
				assignedOp[0] = 4;
			} else {
				assignedOp[0] = 3;
				assignedOp[1] = 4;
				maxCursors = 1;
			}
		}
	} else {
		if (dsiFeatures() && (bothSDandFlashcard() || !flashcardFound())) {
			for (int i = 0; i < 4; i++) {
				selIconYpos -= 14;
			}
			assignedOp[0] = 0;
			assignedOp[1] = 1;
			assignedOp[2] = 2;
			assignedOp[3] = 4;
			maxCursors = 3;
		} else {
			for (int i = 0; i < 3; i++) {
				selIconYpos -= 14;
			}
			if (!sys().isRegularDS()) {
				assignedOp[0] = 0;
				assignedOp[1] = 1;
				assignedOp[2] = 4;
				maxCursors = 2;
			} else if (io_dldi_data->ioInterface.features & FEATURE_SLOT_GBA) {
				assignedOp[0] = 1;
				assignedOp[1] = 2;
				assignedOp[2] = 4;
				maxCursors = 2;
			} else if (ms().gbaBooter == TWLSettings::EGbaGbar2) {
				assignedOp[0] = 1;
				assignedOp[1] = 4;
				maxCursors = 1;
			} else {
				assignedOp[0] = 1;
				assignedOp[1] = 3;
				assignedOp[2] = 4;
				maxCursors = 2;
			}
		}
	}
	if (ms().theme == TWLSettings::EThemeSaturn) {
		while (!screenFadedIn()) { bgOperations(true); }
		dbox_selectMenu = true;
	} else {
		while (!dboxStopped) { bgOperations(true); }
	}
	int pressed = 0;
	while (1) {
		int textYpos = selIconYpos + 4;
		int textXpos = ms().rtl() ? 256 - 64 : 64;
		Alignment align = ms().rtl() ? Alignment::right : Alignment::left;
		FontPalette pal = FontPalette::dialog;
		clearText();
		printSmall(false, 0, (ms().theme == TWLSettings::EThemeSaturn ? 8 : 16), STR_SELECT_MENU, Alignment::center, pal);
		printSmall(false, ms().rtl() ? 256 - 24: 24, -2 + textYpos + (28 * selCursorPosition), ms().rtl() ? "<" : ">", align, pal);
		for (int i = 0; i <= maxCursors; i++) {
			if (assignedOp[i] == 0) {
				printSmall(false, textXpos, textYpos, (ms().consoleModel < 2) ? STR_DSI_MENU : STR_3DS_HOME_MENU, align, pal);
			} else if (assignedOp[i] == 1) {
				printSmall(false, textXpos, textYpos, STR_TWLMENU_SETTINGS, align, pal);
			} else if (assignedOp[i] == 2) {
				if (bothSDandFlashcard()) {
					if (ms().secondaryDevice) {
						printSmall(false, textXpos, textYpos, ms().showMicroSd ? STR_SWITCH_TO_MICRO_SD : STR_SWITCH_TO_SD, align, pal);
					} else {
						printSmall(false, textXpos, textYpos, STR_SWITCH_TO_SLOT_1, align, pal);
					}
				} else if ((isDSiMode() && !flashcardFound()) || (io_dldi_data->ioInterface.features & FEATURE_SLOT_GBA)) {
					printSmall(false, textXpos, textYpos, (REG_SCFG_MC == 0x11) ? STR_NO_SLOT_1 : STR_LAUNCH_SLOT_1, align, pal);
				}
			} else if (assignedOp[i] == 3) {
				printSmall(false, textXpos, textYpos, "Start GBA Mode", align, pal);
			} else if (assignedOp[i] == 4) {
				printSmall(false, textXpos, textYpos, STR_OPEN_MANUAL, align, pal);
			}
			textYpos += 28;
		}
		printSmall(false, 0, (ms().theme == TWLSettings::EThemeSaturn ? 164 : 160), STR_SELECT_B_BACK_A_SELECT, Alignment::center, pal);
		updateText(false);
		u8 current_SCFG_MC = REG_SCFG_MC;
		do {
			scanKeys();
			pressed = keysDown();
			bgOperations(true);
			if (REG_SCFG_MC != current_SCFG_MC) {
				break;
			}
		} while (!pressed);
		if (pressed & KEY_UP) {
			snd().playSelect();
			selCursorPosition--;
			if (selCursorPosition < 0)
				selCursorPosition = maxCursors;
		}
		if (pressed & KEY_DOWN) {
			snd().playSelect();
			selCursorPosition++;
			if (selCursorPosition > maxCursors)
				selCursorPosition = 0;
		}
		if (pressed & KEY_A) {
			switch (assignedOp[selCursorPosition]) {
			case 0:
			default:
				exitToSystemMenu();
				break;
			case 1:
				launchSettings();
				break;
			case 2:
				if (REG_SCFG_MC != 0x11 || (io_dldi_data->ioInterface.features & FEATURE_SLOT_GBA)) {
					switchDevice();
					inSelectMenu = false;
					return true;
				} else {
					snd().playWrong();
				}
				break;
			case 3:
				launchGba();
				break;
			case 4:
				launchManual();
				break;
			}
		}
		if ((pressed & KEY_B) || (pressed & KEY_SELECT)) {
			snd().playBack();
			break;
		}
	};
	showdialogbox = false;
	if (ms().theme == TWLSettings::EThemeSaturn) {
		fadeType = false;	   // Fade to black
		for (int i = 0; i < 25; i++) {
			bgOperations(true);
		}
		clearText();
		dbox_selectMenu = false;
		inSelectMenu = false;
		currentBg = 0;
		displayGameIcons = true;
		fadeType = true;
		snd().playStartup();
	} else {
		clearText();
		inSelectMenu = false;
	}
	updateText(false);
	return false;
}

void getFileInfo(SwitchState scrn, vector<vector<DirEntry>> dirContents, bool reSpawnBoxes) {
	if (nowLoadingDisplaying) {
		clearText();
		showProgressBar = true;
		progressBarLength = 0;
		displayNowLoading();
	}
	if (reSpawnBoxes)
		spawnedtitleboxes = 0;
	for (int i = 0; i < 40; i++) {
		if (i + PAGENUM * 40 < file_count) {
			isDirectory[i] = dirContents[scrn][i + PAGENUM * 40].isDirectory;
			const std::string &std_romsel_filename = dirContents[scrn][i + PAGENUM * 40].name;
			getGameInfo(isDirectory[i], std_romsel_filename.c_str(), i);

			if (isDirectory[i]) {
				bnrWirelessIcon[i] = 0;
			} else {
				if (extension(std_romsel_filename, {".nds", ".dsi", ".ids", ".srl", ".app", ".argv"})) {
					bnrRomType[i] = 0;
				} else if (extension(std_romsel_filename, {".xex", ".atr", ".a26", ".a52", ".a78"})) {
					bnrRomType[i] = 10;
				} else if (extension(std_romsel_filename, {".msx"})) {
					bnrRomType[i] = 21;
				} else if (extension(std_romsel_filename, {".col"})) {
					bnrRomType[i] = 13;
				} else if (extension(std_romsel_filename, {".m5"})) {
					bnrRomType[i] = 14;
				} else if (extension(std_romsel_filename, {".int"})) {
					bnrRomType[i] = 12;
				} else if (extension(std_romsel_filename, {".plg"})) {
					bnrRomType[i] = 9;
				} else if (extension(std_romsel_filename, {".avi", ".rvid", ".fv"})) {
					bnrRomType[i] = 19;
				} else if (extension(std_romsel_filename, {".gif", ".bmp", ".png"})) {
					bnrRomType[i] = 20;
				} else if (extension(std_romsel_filename, {".agb", ".gba", ".mb"})) {
					bnrRomType[i] = 1;
				} else if (extension(std_romsel_filename, {".gb", ".sgb"})) {
					bnrRomType[i] = 2;
				} else if (extension(std_romsel_filename, {".gbc"})) {
					bnrRomType[i] = 3;
				} else if (extension(std_romsel_filename, {".nes"})) {
					bnrRomType[i] = 4;
				} else if (extension(std_romsel_filename, {".fds"})) {
					bnrRomType[i] = 4;
				} else if (extension(std_romsel_filename, {".sg", ".sc"})) {
					bnrRomType[i] = 15;
				} else if (extension(std_romsel_filename, {".sms"})) {
					bnrRomType[i] = 5;
				} else if (extension(std_romsel_filename, {".gg"})) {
					bnrRomType[i] = 6;
				} else if (extension(std_romsel_filename, {".gen", ".md"})) {
					bnrRomType[i] = 7;
				} else if (extension(std_romsel_filename, {".smc"})) {
					bnrRomType[i] = 8;
				} else if (extension(std_romsel_filename, {".sfc"})) {
					bnrRomType[i] = 8;
				} else if (extension(std_romsel_filename, {".pce"})) {
					bnrRomType[i] = 11;
				} else if (extension(std_romsel_filename, {".ws", ".wsc"})) {
					bnrRomType[i] = 16;
				} else if (extension(std_romsel_filename, {".ngp", ".ngc"})) {
					bnrRomType[i] = 17;
				} else if (extension(std_romsel_filename, {".dsk"})) {
					bnrRomType[i] = 18;
				} else if (extension(std_romsel_filename, {".min"})) {
					bnrRomType[i] = 22;
				} else if (extension(std_romsel_filename, {".ntrb"})) {
					bnrRomType[i] = 23;
				} else {
					bnrRomType[i] = 9;
				}

				if (bnrRomType[i] != 0) {
					bnrWirelessIcon[i] = 0;
					bnrSysSettings[i] = false;
					isValid[i] = true;
					isTwlm[i] = false;
					isDSiWare[i] = false;
					isHomebrew[i] = 0;
				}

				if (dsiFeatures() && !ms().macroMode && ms().showBoxArt == 2 && ms().theme != TWLSettings::EThemeHBL && !isDirectory[i]) {
					snprintf(boxArtPath, sizeof(boxArtPath), "%s:/_nds/TWiLightMenu/boxart/%s.png",
							 sys().isRunFromSD() ? "sd" : "fat",
							 dirContents[scrn][i + PAGENUM * 40].name.c_str());
					if ((bnrRomType[i] == 0) && (access(boxArtPath, F_OK) != 0)) {
						snprintf(boxArtPath, sizeof(boxArtPath), "%s:/_nds/TWiLightMenu/boxart/%s.png",
								 (sys().isRunFromSD() ? "sd" : "fat"),
								 gameTid[i]);
					}
					tex().loadBoxArtToMem(boxArtPath, i);
				}
			}
			if (reSpawnBoxes)
				spawnedtitleboxes++;

			progressBarLength += (200/((file_count - (PAGENUM*40)) > 40 ? 40 : (file_count - (PAGENUM*40))));
			if (progressBarLength > 192) progressBarLength = 192;
			bgOperations(false);
		}
	}
	if (nowLoadingDisplaying) {
		showProgressIcon = false;
		showProgressBar = false;
		progressBarLength = 0;
		if (ms().theme != TWLSettings::EThemeSaturn && ms().theme != TWLSettings::EThemeHBL) fadeType = false; // Fade to white
	}
	// Load correct icons depending on cursor position
	if (CURPOS <= 1) {
		for (int i = 0; i < 5; i++) {
			if (i + PAGENUM * 40 < file_count) {
				bgOperations(true);
				iconUpdate(dirContents[scrn].at(i + PAGENUM * 40).isDirectory,
					   dirContents[scrn].at(i + PAGENUM * 40).name.c_str(), i);
			}
		}
	} else if (CURPOS >= 2 && CURPOS <= 36) {
		for (int i = 0; i < 6; i++) {
			if ((CURPOS - 2 + i) + PAGENUM * 40 < file_count) {
				bgOperations(true);
				iconUpdate(dirContents[scrn].at((CURPOS - 2 + i) + PAGENUM * 40).isDirectory,
					   dirContents[scrn].at((CURPOS - 2 + i) + PAGENUM * 40).name.c_str(),
					   CURPOS - 2 + i);
			}
		}
	} else if (CURPOS >= 37 && CURPOS <= 39) {
		for (int i = 0; i < 5; i++) {
			if ((35 + i) + PAGENUM * 40 < file_count) {
				bgOperations(true);
				iconUpdate(dirContents[scrn].at((35 + i) + PAGENUM * 40).isDirectory,
					   dirContents[scrn].at((35 + i) + PAGENUM * 40).name.c_str(), 35 + i);
			}
		}
	}
}

static bool previousPage(SwitchState scrn, vector<vector<DirEntry>> dirContents) {
	if (CURPOS == 0 && !showLshoulder) {
		snd().playWrong();
		return false;
	}

	snd().playSwitch(55);
	if (ms().theme != TWLSettings::EThemeSaturn && ms().theme != TWLSettings::EThemeHBL) {
		fadeType = false; // Fade to white
		for (int i = 0; i < 6; i++) {
			bgOperations(true);
		}
	}
	if (showLshoulder)
		PAGENUM -= 1;
	CURPOS = 0;
	titleboxXdest[ms().secondaryDevice] = 0;
	titlewindowXdest[ms().secondaryDevice] = 0;
	titleboxXpos[ms().secondaryDevice] = titleboxXdest[ms().secondaryDevice];
	titlewindowXpos[ms().secondaryDevice] = titlewindowXdest[ms().secondaryDevice];
	if (ms().theme != TWLSettings::EThemeSaturn && ms().theme != TWLSettings::EThemeHBL) whiteScreen = true;
	if (ms().showBoxArt)
		clearBoxArt(); // Clear box art
	boxArtLoaded = false;
	bannerTextShown = false;
	rocketVideo_playVideo = true;
	shouldersRendered = false;
	currentBg = 0;
	showSTARTborder = false;
	stopSoundPlayed = false;
	clearText();
	updateText(false);
	ms().saveSettings();
	settingsChanged = false;
	if (showLshoulder) {
		displayNowLoading();
	} else {
		// Load correct icons depending on cursor position
		if (CURPOS <= 1) {
			for (int i = 0; i < 5; i++) {
				if (i + PAGENUM * 40 < file_count) {
					bgOperations(true);
					iconUpdate(dirContents[scrn].at(i + PAGENUM * 40).isDirectory,
						   dirContents[scrn].at(i + PAGENUM * 40).name.c_str(), i);
				}
			}
		} else if (CURPOS >= 2 && CURPOS <= 36) {
			for (int i = 0; i < 6; i++) {
				if ((CURPOS - 2 + i) + PAGENUM * 40 < file_count) {
					bgOperations(true);
					iconUpdate(dirContents[scrn].at((CURPOS - 2 + i) + PAGENUM * 40).isDirectory,
						   dirContents[scrn].at((CURPOS - 2 + i) + PAGENUM * 40).name.c_str(),
						   CURPOS - 2 + i);
				}
			}
		} else if (CURPOS >= 37 && CURPOS <= 39) {
			for (int i = 0; i < 5; i++) {
				if ((35 + i) + PAGENUM * 40 < file_count) {
					bgOperations(true);
					iconUpdate(dirContents[scrn].at((35 + i) + PAGENUM * 40).isDirectory,
						   dirContents[scrn].at((35 + i) + PAGENUM * 40).name.c_str(), 35 + i);
				}
			}
		}
		whiteScreen = false;
		fadeType = true; // Fade in from white
	}
	recalculateBoxesCount();
	return showLshoulder;
}

static bool nextPage(SwitchState scrn, vector<vector<DirEntry>> dirContents) {
	if (CURPOS == (file_count - 1) - PAGENUM * 40 && !showRshoulder) {
		snd().playWrong();
		return false;
	}

	snd().playSwitch(200);
	if (ms().theme != TWLSettings::EThemeSaturn && ms().theme != TWLSettings::EThemeHBL) {
		fadeType = false; // Fade to white
		for (int i = 0; i < 6; i++) {
			bgOperations(true);
		}
	}
	if (showRshoulder) {
		PAGENUM += 1;
		CURPOS = 0;
		titleboxXdest[ms().secondaryDevice] = 0;
		titlewindowXdest[ms().secondaryDevice] = 0;
	} else {
		CURPOS = (file_count - 1) - PAGENUM * 40;
		if (CURPOS < 0) CURPOS = 0;
		titleboxXdest[ms().secondaryDevice] = CURPOS * titleboxXspacing;
		titlewindowXdest[ms().secondaryDevice] = CURPOS * 5;
	}
	titleboxXpos[ms().secondaryDevice] = titleboxXdest[ms().secondaryDevice];
	titlewindowXpos[ms().secondaryDevice] = titlewindowXdest[ms().secondaryDevice];
	if (ms().theme != TWLSettings::EThemeSaturn && ms().theme != TWLSettings::EThemeHBL) whiteScreen = true;
	if (ms().showBoxArt)
		clearBoxArt(); // Clear box art
	boxArtLoaded = false;
	bannerTextShown = false;
	rocketVideo_playVideo = true;
	shouldersRendered = false;
	currentBg = 0;
	showSTARTborder = false;
	stopSoundPlayed = false;
	clearText();
	updateText(false);
	ms().saveSettings();
	settingsChanged = false;
	if (showRshoulder) {
		displayNowLoading();
	} else {
		// Load correct icons depending on cursor position
		if (CURPOS <= 1) {
			for (int i = 0; i < 5; i++) {
				if (i + PAGENUM * 40 < file_count) {
					bgOperations(true);
					iconUpdate(dirContents[scrn].at(i + PAGENUM * 40).isDirectory,
						   dirContents[scrn].at(i + PAGENUM * 40).name.c_str(), i);
				}
			}
		} else if (CURPOS >= 2 && CURPOS <= 36) {
			for (int i = 0; i < 6; i++) {
				if ((CURPOS - 2 + i) + PAGENUM * 40 < file_count) {
					bgOperations(true);
					iconUpdate(dirContents[scrn].at((CURPOS - 2 + i) + PAGENUM * 40).isDirectory,
						   dirContents[scrn].at((CURPOS - 2 + i) + PAGENUM * 40).name.c_str(),
						   CURPOS - 2 + i);
				}
			}
		} else if (CURPOS >= 37 && CURPOS <= 39) {
			for (int i = 0; i < 5; i++) {
				if ((35 + i) + PAGENUM * 40 < file_count) {
					bgOperations(true);
					iconUpdate(dirContents[scrn].at((35 + i) + PAGENUM * 40).isDirectory,
						   dirContents[scrn].at((35 + i) + PAGENUM * 40).name.c_str(), 35 + i);
				}
			}
		}
		whiteScreen = false;
		fadeType = true; // Fade in from white
	}
	recalculateBoxesCount();
	return showRshoulder;
}

// Opens a message box prompt to set/clear the default directory.
bool setDefaultDirectory(std::string_view directory_path)
{
	if (ms().kioskMode || !ms().showDirectories)
		return false;

	if (ms().theme == TWLSettings::EThemeSaturn) {
		snd().playStartup();
		fadeType = false;	   // Fade to black
		for (int i = 0; i < 25; i++) {
			bgOperations(true);
		}
		currentBg = 1;
		displayGameIcons = false;
		fadeType = true;

		clearText();
		updateText(false);

		while (!screenFadedIn()) { bgOperations(true); }
		dbox_showIcon = false;

		printLarge(false, 0, 104, directory_path, Alignment::center);
	} else {
		dbox_showIcon = false;
		showdialogbox = true;

		clearText();
		updateText(false);

		while (!dboxStopped) { bgOperations(true); }

		dbox_showIcon = false;
		showdialogbox = true;

		printLarge(false, 0, 32, directory_path, Alignment::center);
	}

	int yPos = (ms().theme == TWLSettings::EThemeSaturn ? 30 : 98);

	std::string *str;
	bool alreadyDefault, noDefaultDir;
	noDefaultDir = ms().defaultRomfolder[ms().secondaryDevice] == "null";

	if (ms().defaultRomfolder[ms().secondaryDevice] != directory_path)
	{
		str = &STR_DEFAULT_DIR_SET;
		alreadyDefault = false;
	}
	else
	{
		str = &STR_DEFAULT_DIR_ALREADY;
		alreadyDefault = true;
	}

	printSmall(false, 0, yPos - ((calcSmallFontHeight(*str) - smallFontHeight()) / 2), *str, Alignment::center, FontPalette::dialog);
	if (!sys().isRegularDS()) {
		yPos = (ms().theme == TWLSettings::EThemeSaturn ? 96 : 64)-8;
		printSmall(false, 0, (ms().theme == TWLSettings::EThemeSaturn ? 96 : 64)-8, ms().secondaryDevice ? STR_LOCATION_SLOT_1 : (ms().showMicroSd ? STR_LOCATION_MICRO_SD : STR_LOCATION_SD), Alignment::center);
	}

	updateText(false);
	for (int i = 0; i < 90; i++) { // Small delay to avoid activating this accidentally
		bgOperations(true);
	}

	if (!alreadyDefault)
		printSmall(false, 0, 160-16, STR_X_SET_DEFAULT_DIR, Alignment::center, FontPalette::dialog);

	std::string btn = (noDefaultDir ? "" : STR_Y_DISABLE + "    ") + STR_B_BACK;
	printSmall(false, 0, 160, btn, Alignment::center, FontPalette::dialog);

	updateText(false);
	int pressed = 0;
	while (1) {
		scanKeys();
		pressed = keysDown();
		updateBoxArt();
		bgOperations(true);

		// Back
		if (pressed & KEY_B) {
			snd().playBack();
			break;
		}
		// Clear default dir and save settings
		else if (!noDefaultDir && pressed & KEY_Y) {
			snd().playLaunch();

			ms().defaultRomfolder[ms().secondaryDevice] = "null";
			ms().saveSettings();
			break;
		}
		// Set default dir and save settings
		else if (!alreadyDefault && (pressed & KEY_X)) {
			snd().playLaunch();

			ms().defaultRomfolder[ms().secondaryDevice] = directory_path;
			ms().saveSettings();
			break;
		}
	}

	if (ms().theme == TWLSettings::EThemeSaturn) {
		fadeType = false;	   // Fade to black
		for (int i = 0; i < 25; i++) {
			bgOperations(true);
		}
		clearText();
		updateText(false);
		currentBg = 0;
		displayGameIcons = true;
		fadeType = true;
		if (ms().theme == TWLSettings::EThemeSaturn) snd().playStartup();
	} else {
		clearText();
		updateText(false);
		for (int i = 0; i < 15; i++) { bgOperations(true); }

		if (ms().theme == TWLSettings::EThemeHBL) {
			dbox_showIcon = false;
		}
		showdialogbox = false;
	}

	return true;
}

std::string browseForFile(const std::vector<std::string_view> extensionList) {
	displayNowLoading();
	gameOrderIniPath = std::string(sys().isRunFromSD() ? "sd" : "fat") + ":/_nds/TWiLightMenu/extras/gameorder.ini";
	recentlyPlayedIniPath = std::string(sys().isRunFromSD() ? "sd" : "fat") + ":/_nds/TWiLightMenu/extras/recentlyplayed.ini";
	timesPlayedIniPath = std::string(sys().isRunFromSD() ? "sd" : "fat") + ":/_nds/TWiLightMenu/extras/timesplayed.ini";

	bool displayBoxArt = ms().showBoxArt;

	int pressed = 0;
	int held = 0;
	SwitchState scrn(3);
	vector<vector<DirEntry>> dirContents(scrn.SIZE);

	getDirectoryContents(dirContents[scrn], extensionList);
	
	if (CURPOS > last_used_box)
	{
		CURPOS = last_used_box;
		titleboxXpos[ms().secondaryDevice]     = CURPOS * titleboxXspacing;
		titleboxXdest[ms().secondaryDevice]    = CURPOS * titleboxXspacing;
		titlewindowXpos[ms().secondaryDevice]  = CURPOS * 5;
		titlewindowXdest[ms().secondaryDevice] = CURPOS * 5;
	}

	while (1) {
		updateDirectoryContents(dirContents[scrn]);
		getFileInfo(scrn, dirContents, true);
		reloadIconPalettes();
		if (ms().theme != TWLSettings::EThemeSaturn && ms().theme != TWLSettings::EThemeHBL) {
			while (!screenFadedOut()) { swiWaitForVBlank(); }
		}
		nowLoadingDisplaying = false;
		whiteScreen = false;
		if (!musicplaying && ms().theme == TWLSettings::EThemeDSi) {
			fadeSpeed = false; // Slow fade speed
			fadeType = true; // Fade in from white
			for (int i = 0; i < 20; i++) {
				bgOperations(true);
			}
			if (!dropDown) {
				dropDown = true;
			}
		} else {
			fadeType = true; // Fade in from white
			for (int i = 0; i < 5; i++) {
				bgOperations(true);
			}
		}
		displayGameIcons = true;
		clearText(false);
		updateText(false);

		if (!musicplaying && ms().theme != TWLSettings::EThemeSaturn) {
			if (ms().dsiMusic != 0) {
				if (ms().dsiMusic == 4 || (ms().dsiMusic == 3 && tc().playStartupJingle())) {
					//logPrint("snd().playStartup()\n");
					snd().playStartup();
					//logPrint("snd().setStreamDelay(snd().getStartupSoundLength() - tc().startupJingleDelayAdjust())\n");
					snd().setStreamDelay(snd().getStartupSoundLength() - tc().startupJingleDelayAdjust());
				}
				//logPrint("snd().beginStream()\n");
				snd().beginStream();
			}

			controlTopBright = true;
			while (!screenFadedIn()) { swiWaitForVBlank(); }
			musicplaying = true;
		}

		controlTopBright = false;
		fadeSpeed = true; // Fast fade speed
		bool gameTapped = false;
		bool dsiBinariesChecked = false;
		bool hasDsiBinaries = true;
		bool apChecked = false;
		int infoCheckTimer = 0;
		bool hasAP = false;

		while (1) {
			if (!stopSoundPlayed) {
				if (ms().theme == TWLSettings::EThemeDSi &&
					 CURPOS + PAGENUM * 40 <= ((int)dirContents[scrn].size() - 1)) {
					needToPlayStopSound = true;
				}
				stopSoundPlayed = true;
			}

			if (!shouldersRendered) {
				showLshoulder = (PAGENUM != 0);
				showRshoulder = (file_count > 40 + PAGENUM * 40);
				if (ms().theme != TWLSettings::EThemeHBL) {
					tex().drawShoulders(showLshoulder, showRshoulder);
				}
				shouldersRendered = true;
			}

			// u8 current_SCFG_MC = REG_SCFG_MC;

			// Power saving loop. Only poll the keys once per frame and sleep the CPU if there is nothing
			// else to do
			do {
				scanKeys();
				pressed = keysDown();
				held = keysDownRepeat();
				touchRead(&touch);

				boxArtFound = ((CURPOS + PAGENUM * 40) < ((int)dirContents[scrn].size()));
				if (boxArtFound) {
					boxArtFilename = dirContents[scrn].at(CURPOS + PAGENUM * 40).name.c_str();

					logPrint("boxArtFilename: ");
					logPrint(boxArtFilename);
					logPrint("\n");
				}
				updateBoxArt();
				if (ms().theme < 4) {
					while (dboxInFrame) {
						bgOperations(true);
					}
				}
				dbox_showIcon = false;
				dbox_selectMenu = false;

				if (ms().macroMode && ms().theme == TWLSettings::EThemeSaturn) {
					printSmall(false, 4, 4, (showLshoulder ? STR_L_PREV : STR_L), Alignment::left, FontPalette::overlay);
					printSmall(false, 256-4, 4, (showRshoulder ? STR_NEXT_R : STR_R), Alignment::right, FontPalette::overlay);
				}
				if (CURPOS + PAGENUM * 40 < ((int)dirContents[scrn].size())) {
					currentBg = (ms().theme == TWLSettings::EThemeSaturn ? 0 : 1), displayBoxArt = ms().showBoxArt;
					if (!bannerTextShown) {
						clearText();
						titleUpdate(dirContents[scrn].at(CURPOS + PAGENUM * 40).isDirectory,
								dirContents[scrn].at(CURPOS + PAGENUM * 40).name, CURPOS);
						bannerTextShown = true;
					}
					if ((infoCheckTimer < 30) && (bnrRomType[CURPOS] == 0) && (isHomebrew[CURPOS] == 0) && !isDSiWare[CURPOS]) {
						infoCheckTimer++;
						if (infoCheckTimer == 30) {
							if (!dsiBinariesChecked) {
								hasDsiBinaries = checkDsiBinaries(dirContents[scrn].at(CURPOS + PAGENUM * 40).name.c_str(), CURPOS);
							}
							dsiBinariesChecked = true;
							if (!apChecked && checkIfShowAPMsg(dirContents[scrn].at(CURPOS + PAGENUM * 40).name)) {
								hasAP = checkRomAP(dirContents[scrn].at(CURPOS + PAGENUM * 40).name.c_str(), CURPOS);
							}
							apChecked = true;
						}
					}
				} else {
					if (displayBoxArt && !rocketVideo_playVideo) {
						clearBoxArt();
						displayBoxArt = false;
					}
					bannerTextShown = false;
					clearText(false);
					currentBg = 0;
					showSTARTborder = rocketVideo_playVideo = (ms().theme == TWLSettings::ETheme3DS ? true : false);
				}
				if (ms().theme == TWLSettings::EThemeHBL) {
					printLarge(false, 0, 142, "^", Alignment::center, FontPalette::overlay);
					printSmall(false, 4, 174, (showLshoulder ? STR_L_PREV : STR_L), Alignment::left, FontPalette::overlay);
					printSmall(false, 256-4, 174, (showRshoulder ? STR_NEXT_R : STR_R), Alignment::right, FontPalette::overlay);
				} else if (ms().macroMode && ms().theme != TWLSettings::EThemeSaturn) {
					printSmall(false, 4, 152, (showLshoulder ? STR_L_PREV : STR_L), Alignment::left, FontPalette::overlay);
					printSmall(false, 256-4, 152, (showRshoulder ? STR_NEXT_R : STR_R), Alignment::right, FontPalette::overlay);
				}
				updateText(false);
				buttonArrowTouched[0] = ((keysHeld() & KEY_TOUCH) && touch.py > 171 && touch.px < 19);
				buttonArrowTouched[1] = ((keysHeld() & KEY_TOUCH) && touch.py > 171 && touch.px > 236);
				bgOperations(true);
				/*if (REG_SCFG_MC != current_SCFG_MC) {
					break;
				}*/
			} while (!held);

			buttonArrowTouched[0] = ((keysHeld() & KEY_TOUCH) && touch.py > 171 && touch.px < 19);
			buttonArrowTouched[1] = ((keysHeld() & KEY_TOUCH) && touch.py > 171 && touch.px > 236);

			if ((held & KEY_LEFT) || ((held & KEY_TOUCH) && touch.py > 171 && touch.px < 19 && ms().theme == TWLSettings::EThemeDSi)) { // Left or button arrow (DSi theme)
				moveCursor(false, dirContents[scrn]);
				dsiBinariesChecked = false;
				apChecked = false;
				infoCheckTimer = 0;
			} else if ((held & KEY_RIGHT) || ((held & KEY_TOUCH) && touch.py > 171 && touch.px > 236 && ms().theme == TWLSettings::EThemeDSi)) { // Right or button arrow (DSi theme)
				moveCursor(true, dirContents[scrn]);
				dsiBinariesChecked = false;
				apChecked = false;
				infoCheckTimer = 0;
			} else if ((pressed & KEY_UP) && (PAGENUM > 0 || CURPOS > 0 || !backFound) && (ms().theme != TWLSettings::EThemeSaturn && ms().theme != TWLSettings::EThemeHBL) && !dirInfoIniFound && (ms().sortMethod == 4) && (CURPOS + PAGENUM * 40 < ((int)dirContents[scrn].size()))) { // Move apps (DSi & 3DS themes)
				bannerTextShown = false; // Redraw the title when done
				showSTARTborder = false;
				currentBg = 2;
				clearText();
				updateText(false);
				mkdir(sys().isRunFromSD() ? "sd:/_nds/TWiLightMenu/extras" : "fat:/_nds/TWiLightMenu/extras", 0777);
				movingApp = (PAGENUM * 40) + (CURPOS);
				titleboxXspacing = 76;
				titleboxXdest[ms().secondaryDevice] = titleboxXpos[ms().secondaryDevice] = CURPOS * titleboxXspacing;

				if (dirContents[scrn][movingApp].isDirectory)
					movingAppIsDir = true;
				else
					movingAppIsDir = false;

				getGameInfo(dirContents[scrn][movingApp].isDirectory,
							dirContents[scrn][movingApp].name.c_str(), -1);
				iconUpdate(dirContents[scrn][movingApp].isDirectory,
						   dirContents[scrn][movingApp].name.c_str(), -1);

				int movingAppYmax = ms().theme == TWLSettings::ETheme3DS ? 64 : 82;
				while (movingAppYpos < movingAppYmax) {
					movingAppYpos += std::max((movingAppYmax - movingAppYpos) / 3, 1);
					bgOperations(true);
				}

				int orgCursorPosition = CURPOS;
				int orgPage = PAGENUM;
				showMovingArrow = true;

				while (1) {
					scanKeys();
					pressed = keysDown();
					held = keysDownRepeat();
					bgOperations(true);

					// RocketVideo video extraction
					/*if (pressed & KEY_X) {
						FILE* destinationFile =
					fopen("sd:/_nds/TWiLightMenu/extractedvideo.rvid", "wb");
						fwrite((void*)0x02800000, 1, 0x6A0000, destinationFile);
						fclose(destinationFile);
					}*/

					if (held & KEY_LEFT) {
						if (PAGENUM > 0 || CURPOS > 1 || !backFound) {
							moveCursor(false, dirContents[scrn]);
						}
					} else if (held & KEY_RIGHT) {
						if (CURPOS + (PAGENUM * 40) < (int)dirContents[scrn].size() - 1) {
							moveCursor(true, dirContents[scrn], dirContents[scrn].size() - 1 - PAGENUM * 40);
						} else if (ms().theme != TWLSettings::EThemeSaturn && !edgeBumpSoundPlayed) {
							snd().playWrong();
							edgeBumpSoundPlayed = true;
						}
					} else if (pressed & KEY_DOWN) {
						showMovingArrow = false;
						while (movingAppYpos > 0) {
							movingAppYpos -= std::max(movingAppYpos / 3, 1);
							bgOperations(true);
						}
						break;
					} else if (pressed & KEY_L) {
						if (PAGENUM > 0) {
							snd().playSwitch(55);
							fadeType = false; // Fade to white
							for (int i = 0; i < 6; i++) {
								bgOperations(true);
							}
							PAGENUM -= 1;
							CURPOS = 0;
							titleboxXdest[ms().secondaryDevice] = 0;
							titlewindowXdest[ms().secondaryDevice] = 0;
							titleboxXpos[ms().secondaryDevice] = titleboxXdest[ms().secondaryDevice];
							titlewindowXpos[ms().secondaryDevice] = titlewindowXdest[ms().secondaryDevice];
							whiteScreen = true;
							shouldersRendered = false;
							displayNowLoading();
							getDirectoryContents(dirContents[scrn], extensionList);
							getFileInfo(scrn, dirContents, true);

							while (!screenFadedOut()) {
								bgOperations(true);
							}
							nowLoadingDisplaying = false;
							whiteScreen = false;
							displayGameIcons = true;
							fadeType = true; // Fade in from white
							for (int i = 0; i < 5; i++) {
								bgOperations(true);
							}
							reloadIconPalettes();
							clearText();
							updateText(false);
						} else {
							snd().playWrong();
						}
					} else if (pressed & KEY_R) {
						if (file_count > 40 + PAGENUM * 40) {
							snd().playSwitch(200);
							fadeType = false; // Fade to white
							for (int i = 0; i < 6; i++) {
								bgOperations(true);
							}
							PAGENUM += 1;
							CURPOS = 0;
							titleboxXdest[ms().secondaryDevice] = 0;
							titlewindowXdest[ms().secondaryDevice] = 0;
							titleboxXpos[ms().secondaryDevice] = titleboxXdest[ms().secondaryDevice];
							titlewindowXpos[ms().secondaryDevice] = titlewindowXdest[ms().secondaryDevice];
							whiteScreen = true;
							shouldersRendered = false;
							displayNowLoading();
							getDirectoryContents(dirContents[scrn], extensionList);
							getFileInfo(scrn, dirContents, true);

							while (!screenFadedOut()) {
								bgOperations(true);
							}
							nowLoadingDisplaying = false;
							whiteScreen = false;
							displayGameIcons = true;
							fadeType = true; // Fade in from white
							for (int i = 0; i < 5; i++) {
								bgOperations(true);
							}
							reloadIconPalettes();
							clearText();
							updateText(false);
						} else {
							snd().playWrong();
						}
					}
				}

				if ((PAGENUM != orgPage) || (CURPOS != orgCursorPosition)) {
					currentBg = 1;
					writeBannerText(STR_PLEASE_WAIT, STR_PLEASE_WAIT);
					updateText(false);

					int dest = CURPOS + (PAGENUM * 40);

					DirEntry entry = dirContents[scrn][movingApp];
					dirContents[scrn].erase(dirContents[scrn].begin() + movingApp);
					dirContents[scrn].insert(dirContents[scrn].begin() + dest, entry);

					std::vector<std::string> dirNames(dirContents[scrn].size());
					for (uint i=0;i<dirContents[scrn].size();i++) {
						dirNames[i] = dirContents[scrn][i].name;
					}

					CIniFile gameOrderIni(gameOrderIniPath);
					getcwd(path, PATH_MAX);
					gameOrderIni.SetStringVector("ORDER", path, dirNames, ':');
					gameOrderIni.SaveIniFile(gameOrderIniPath);

					if (ms().sortMethod != TWLSettings::ESortCustom) {
						ms().sortMethod = TWLSettings::ESortCustom;
						ms().saveSettings();
					}

					getFileInfo(scrn, dirContents, false);
				}

				movingApp = -1;
				titleboxXspacing = 58;
				titleboxXdest[ms().secondaryDevice] = titleboxXpos[ms().secondaryDevice] = CURPOS * titleboxXspacing;
			} else if ((pressed & KEY_TOUCH) && touch.py > 171 && touch.px >= 19 && touch.px <= 236 && ms().theme == TWLSettings::EThemeDSi) { // Scroll bar (DSi theme)
				touchPosition startTouch = touch;
				showSTARTborder = false;
				titleboxXspeed = 3;
				scrollWindowTouched = true;

				dsiBinariesChecked = false;
				apChecked = false;
				infoCheckTimer = 0;

				int prevPos = CURPOS;
				while (1) {
					scanKeys();
					touchRead(&touch);
					bgOperations(true);

					if (keysHeld() & KEY_TOUCH) {
						titlewindowXdest[ms().secondaryDevice] = std::clamp(touch.px - 30, 0, 192);
						titleboxXdest[ms().secondaryDevice] = std::clamp((touch.px - 30) * titleboxXspacing / 5 - 28, 0, titleboxXspacing * last_used_box);
					} else {
						int dest = (titleboxXdest[ms().secondaryDevice] + 28) / titleboxXspacing;
						titlewindowXdest[ms().secondaryDevice] = dest * 5;
						titleboxXdest[ms().secondaryDevice] = dest * titleboxXspacing;

						if (titleboxXdest[ms().secondaryDevice] == titleboxXpos[ms().secondaryDevice])
							break;
					}

					prevPos = CURPOS;
					CURPOS = std::clamp((titleboxXpos[ms().secondaryDevice] + 28) / titleboxXspacing, 0, last_used_box);

					// Load icons
					if (prevPos != CURPOS) {
						for (int i = 0; i < 6; i++) {
							const int pos = (CURPOS - 2 + i);
							if (pos >= 0 && pos + PAGENUM * 40 < file_count) {
								iconUpdate(dirContents[scrn][pos + PAGENUM * 40].isDirectory,
										dirContents[scrn][pos + PAGENUM * 40].name.c_str(),
										pos);
							}
						}
					}

					if (CURPOS + PAGENUM * 40 < ((int)dirContents[scrn].size())) {
						currentBg = 1;
						clearText();
						titleUpdate(dirContents[scrn][CURPOS + PAGENUM * 40].isDirectory,
									dirContents[scrn][CURPOS + PAGENUM * 40].name,
									CURPOS);
						bannerTextShown = true;
						updateText(false);
					} else {
						clearText();
						updateText(false);
						currentBg = 0;
					}
				}
				scrollWindowTouched = false;
				titleboxXspeed = 8;

				waitForNeedToPlayStopSound = 1;
				snd().playSelect();
				boxArtLoaded = false;
				bannerTextShown = false;
				settingsChanged = true;
				touch = startTouch;
				if (CURPOS + PAGENUM * 40 < ((int)dirContents[scrn].size()))
					showSTARTborder = true;
			} else if ((pressed & KEY_TOUCH) && touch.py > 76 && touch.py < (ms().theme == TWLSettings::EThemeDSi ? 164 : 144)) { // Dragging icons
				touchPosition startTouch = touch;
				touchPosition prevTouch1 = touch;
				touchPosition prevTouch2 = touch;

				dsiBinariesChecked = false;
				apChecked = false;
				infoCheckTimer = 0;

				bool tapped = false;
				bool dsiCursorMove = false;
				while (1) {
					scanKeys();
					touchRead(&touch);
					bgOperations(true);

					prevTouch2 = prevTouch1;
					prevTouch1 = touch;

					if (!(keysHeld() & KEY_TOUCH)) {
						tapped = true;
						break;
					} else if (touch.px < startTouch.px - 2
							|| touch.px > startTouch.px + 2) {
						break;
					}
				}

				if (tapped) {
					int moveBy;
					if (startTouch.px < 39)
						moveBy = -2;
					else if (startTouch.px < 96)
						moveBy = -1;
					else if (startTouch.px < 160)
						moveBy = 0;
					else if (startTouch.px < 217)
						moveBy = 1;
					else
						moveBy = 2;

					if (moveBy == 0) {
						gameTapped = bannerTextShown && showSTARTborder;
					} else if (ms().theme == TWLSettings::EThemeDSi) {
						if (moveBy != -2 && moveBy != 2) {
							moveCursor(moveBy == 1, dirContents[scrn]);
						} else {
							const int CURPOS1 = std::clamp(moveBy > 0 ? (CURPOS + 1) : (CURPOS - 1), 0, last_used_box);
							CURPOS = std::clamp(CURPOS + moveBy, 0, last_used_box);

							snd().playSelect();

							titlewindowXdest[ms().secondaryDevice] = CURPOS * 5;
							titleboxXdest[ms().secondaryDevice] = CURPOS * titleboxXspacing;
							boxArtLoaded = false;
							settingsChanged = true;
							bannerTextShown = false;
							touch = startTouch;
							if (CURPOS + PAGENUM * 40 >= ((int)dirContents[scrn].size())) {
								showSTARTborder = (ms().theme == TWLSettings::ETheme3DS ? true : false);
							} else {
								showSTARTborder = true;
							}

							for (int i = 0; i < 2; i++) {
								swiWaitForVBlank();
							}

							// Load icons
							for (int i = 0; i < 6; i++) {
								const int pos = (CURPOS1 - 2 + i);
								if (pos >= 0 && pos + PAGENUM * 40 < file_count) {
									iconUpdate(dirContents[scrn][pos + PAGENUM * 40].isDirectory,
											dirContents[scrn][pos + PAGENUM * 40].name.c_str(),
											pos);
								}
							}

							for (int i = 0; i < 6; i++) {
								swiWaitForVBlank();
							}

							snd().playSelect();

							for (int i = 0; i < 2; i++) {
								swiWaitForVBlank();
							}

							// Load icons again
							for (int i = 0; i < 6; i++) {
								const int pos = (CURPOS - 2 + i);
								if (pos >= 0 && pos + PAGENUM * 40 < file_count) {
									iconUpdate(dirContents[scrn][pos + PAGENUM * 40].isDirectory,
											dirContents[scrn][pos + PAGENUM * 40].name.c_str(),
											pos);
								}
							}

							while (titleboxXpos[ms().secondaryDevice] != titleboxXdest[ms().secondaryDevice]) {
								swiWaitForVBlank();
							}
							if (tc().playStopSound()) snd().playStop();
							startBorderZoomOut = true;
						}
						dsiCursorMove = true;
					} else if (ms().theme != TWLSettings::EThemeSaturn) {
						CURPOS = std::clamp(CURPOS + moveBy, 0, last_used_box);

						// Load icons
						for (int i = 0; i < 6; i++) {
							const int pos = (CURPOS - 2 + i);
							if (pos >= 0 && pos + PAGENUM * 40 < file_count) {
								iconUpdate(dirContents[scrn][pos + PAGENUM * 40].isDirectory,
										dirContents[scrn][pos + PAGENUM * 40].name.c_str(),
										pos);
							}
						}
					}
				} else if (ms().theme != TWLSettings::EThemeSaturn) {
					draggingIcons = true;
					int prevPos = CURPOS;
					showSTARTborder = false;

					while (1) {
						scanKeys();
						touchRead(&touch);
						bgOperations(true);

						if (!(keysHeld() & KEY_TOUCH)) {
							titleboxXspeed = 6;
							int dx = std::clamp(-(prevTouch1.px - prevTouch2.px) * last_used_box / 192, -last_used_box, last_used_box);

							int dest = std::clamp(CURPOS + dx, 0, last_used_box);
							titlewindowXdest[ms().secondaryDevice] = dest * 5;

							int boxDest = std::clamp(titleboxXdest[ms().secondaryDevice] + dx * titleboxXspacing, -160, titleboxXspacing * last_used_box + 160);
							if (boxDest < 0 || boxDest > titleboxXspacing * 39) {
								titleboxXdest[ms().secondaryDevice] = boxDest;
							} else {
								titleboxXdest[ms().secondaryDevice] = dest * titleboxXspacing;
							}

							int prevPos;
							while (titleboxXdest[ms().secondaryDevice] != titleboxXpos[ms().secondaryDevice] && !(keysHeld() & KEY_TOUCH)) {
								scanKeys();
								if (keysDown() & KEY_TOUCH)
									touchRead(&touch);
								swiWaitForVBlank();

								prevPos = CURPOS;
								CURPOS = std::clamp((titleboxXpos[ms().secondaryDevice] + 28) / titleboxXspacing, 0, last_used_box);

								if (CURPOS != prevPos) {
									// Load icons
									for (int i = 0; i < 6; i++) {
										int pos = (CURPOS - 2 + i);
										if (pos >= 0 && pos + PAGENUM * 40 < file_count) {
											iconUpdate(dirContents[scrn][pos + PAGENUM * 40].isDirectory,
													dirContents[scrn][pos + PAGENUM * 40].name.c_str(),
													pos);
										}
									}

									clearText();
									if (CURPOS + PAGENUM * 40 < ((int)dirContents[scrn].size()) && boxDest > -28 && boxDest < titleboxXspacing * 39 + 28) {
										currentBg = 1;
										titleUpdate(dirContents[scrn][CURPOS + PAGENUM * 40].isDirectory,
													dirContents[scrn][CURPOS + PAGENUM * 40].name,
													CURPOS);
									} else {
										currentBg = 0;
									}
									if (ms().theme == TWLSettings::EThemeHBL) {
										printLarge(false, 0, 142, "^", Alignment::center, FontPalette::overlay);
										printSmall(false, 4, 174, (showLshoulder ? STR_L_PREV : STR_L), Alignment::left, FontPalette::overlay);
										printSmall(false, 256-4, 174, (showRshoulder ? STR_NEXT_R : STR_R), Alignment::right, FontPalette::overlay);
									} else if (ms().macroMode && ms().theme != TWLSettings::EThemeSaturn) {
										printSmall(false, 4, 152, (showLshoulder ? STR_L_PREV : STR_L), Alignment::left, FontPalette::overlay);
										printSmall(false, 256-4, 152, (showRshoulder ? STR_NEXT_R : STR_R), Alignment::right, FontPalette::overlay);
									}
									updateText(false);
								}
							}

							// Wait a little bit to give time to re-grab
							for (int i = 0; i < 10 && !(keysHeld() & KEY_TOUCH); i++) {
								bgOperations(true);
								scanKeys();
							}

							if (keysHeld() & KEY_TOUCH) {
								prevTouch2 = touch;
								touchRead(&prevTouch1);
								titleboxXspeed = 3;
								continue;
							}

							break;
						}

						titleboxXdest[ms().secondaryDevice] = titleboxXdest[ms().secondaryDevice] - (touch.px - prevTouch1.px);
						titlewindowXdest[ms().secondaryDevice] = std::clamp(titleboxXdest[ms().secondaryDevice] * 5 / titleboxXspacing, 0, 192);
						CURPOS = std::clamp((titleboxXpos[ms().secondaryDevice] + 28) / titleboxXspacing, 0, last_used_box);

						if (prevPos != CURPOS) {
							// Load icons
							for (int i = 0; i < 6; i++) {
								int pos = (CURPOS - 2 + i);
								if (pos >= 0 && pos + PAGENUM * 40 < file_count) {
									iconUpdate(dirContents[scrn][pos + PAGENUM * 40].isDirectory,
											dirContents[scrn][pos + PAGENUM * 40].name.c_str(),
											pos);
								}
							}
						}

						clearText();
						if (CURPOS + PAGENUM * 40 < ((int)dirContents[scrn].size()) && titleboxXdest[ms().secondaryDevice] > -28 && titleboxXdest[ms().secondaryDevice] < titleboxXspacing * 39 + 28) {
							currentBg = 1;
							titleUpdate(dirContents[scrn][CURPOS + PAGENUM * 40].isDirectory,
										dirContents[scrn][CURPOS + PAGENUM * 40].name,
										CURPOS);
						} else {
							currentBg = 0;
						}
						if (ms().theme == TWLSettings::EThemeHBL) {
							printLarge(false, 0, 142, "^", Alignment::center, FontPalette::overlay);
							printSmall(false, 4, 174, (showLshoulder ? STR_L_PREV : STR_L), Alignment::left, FontPalette::overlay);
							printSmall(false, 256-4, 174, (showRshoulder ? STR_NEXT_R : STR_R), Alignment::right, FontPalette::overlay);
						} else if (ms().macroMode && ms().theme != TWLSettings::EThemeSaturn) {
							printSmall(false, 4, 152, (showLshoulder ? STR_L_PREV : STR_L), Alignment::left, FontPalette::overlay);
							printSmall(false, 256-4, 152, (showRshoulder ? STR_NEXT_R : STR_R), Alignment::right, FontPalette::overlay);
						}
						updateText(false);

						prevTouch2 = prevTouch1;
						prevTouch1 = touch;
						prevPos = CURPOS;
					}
					draggingIcons = false;
				}

				if (!dsiCursorMove) {
					titlewindowXdest[ms().secondaryDevice] = CURPOS * 5;
					titleboxXdest[ms().secondaryDevice] = CURPOS * titleboxXspacing;
					boxArtLoaded = false;
					settingsChanged = true;
					bannerTextShown = false;
					titleboxXspeed = (ms().theme == TWLSettings::EThemeDSi) ? 8 : 3;
					touch = startTouch;
					needToPlayStopSound = true;
					if (!gameTapped && CURPOS + PAGENUM * 40 < ((int)dirContents[scrn].size())) {
						showSTARTborder = (ms().theme == TWLSettings::ETheme3DS ? true : false);
					}
				}
			}

			if (CURPOS < 0)
				ms().cursorPosition[ms().secondaryDevice] = 0;
			else if (CURPOS > 39)
				ms().cursorPosition[ms().secondaryDevice] = last_used_box;

			// Startup...
			if ((((pressed & KEY_A) || (pressed & KEY_START) || (ms().theme == TWLSettings::ETheme3DS && (pressed & KEY_TOUCH) && touch.py > 171)) && bannerTextShown && showSTARTborder) || gameTapped) {
				bannerTextShown = false; // Redraw title when done
				DirEntry *entry = &dirContents[scrn].at(CURPOS + PAGENUM * 40);
				if (entry->isDirectory) {
					// Enter selected directory
					(ms().theme == TWLSettings::EThemeSaturn) ? snd().playLaunch() : snd().playSelect();
					if (ms().theme != TWLSettings::EThemeSaturn && ms().theme != TWLSettings::EThemeHBL) {
						fadeType = false; // Fade to white
						for (int i = 0; i < 6; i++) {
							bgOperations(true);
						}
					}
					ms().pagenum[ms().secondaryDevice] = 0;
					ms().cursorPosition[ms().secondaryDevice] = 0;
					titleboxXdest[ms().secondaryDevice] = 0;
					titlewindowXdest[ms().secondaryDevice] = 0;
					titleboxXpos[ms().secondaryDevice] = titleboxXdest[ms().secondaryDevice];
					titlewindowXpos[ms().secondaryDevice] = titlewindowXdest[ms().secondaryDevice];
					if (ms().theme != TWLSettings::EThemeSaturn && ms().theme != TWLSettings::EThemeHBL) whiteScreen = true;
					if (ms().showBoxArt)
						clearBoxArt(); // Clear box art
					boxArtLoaded = false;
					shouldersRendered = false;
					currentBg = 0;
					showSTARTborder = false;
					stopSoundPlayed = false;
					clearText();
					updateText(false);
					chdir(entry->name.c_str());
					char buf[256];
					ms().romfolder[ms().secondaryDevice] = std::string(getcwd(buf, 256));
					ms().saveSettings();
					settingsChanged = false;
					return "null";
				} else {
					if (isValid[CURPOS] && !isTwlm[CURPOS]) {
						loadPerGameSettings(dirContents[scrn].at(CURPOS + PAGENUM * 40).name);
					}
					bool proceedToLaunch = true;
					if (!isValid[CURPOS] || isTwlm[CURPOS] || (isUnlaunch[CURPOS] && ms().theme == TWLSettings::ETheme3DS) || (!isDSiWare[CURPOS] && (!dsiFeatures() || bs().b4dsMode) && ms().secondaryDevice && bnrRomType[CURPOS] == 0 && gameTid[CURPOS][0] == 'D' && unitCode[CURPOS] == 3 && requiresDonorRom[CURPOS] != 51)
					|| (isDSiWare[CURPOS] && ((((!dsiFeatures() && (!sdFound() || !ms().dsiWareToSD)) || bs().b4dsMode) && ms().secondaryDevice && !dsiWareCompatibleB4DS())
					|| (isDSiMode() && memcmp(io_dldi_data->friendlyName, "CycloDS iEvolution", 18) != 0 && sys().arm7SCFGLocked() && !sys().dsiWramAccess() && !gameCompatibleMemoryPit())))
					|| (bnrRomType[CURPOS] == 1 && (!ms().secondaryDevice || dsiFeatures() || ms().gbaBooter == TWLSettings::EGbaGbar2) && checkForGbaBiosRequirement())) {
						proceedToLaunch = cannotLaunchMsg(dirContents[scrn].at(CURPOS + PAGENUM * 40).name.c_str());
					}
					bool useBootstrapAnyway = ((perGameSettings_useBootstrap == -1 ? ms().useBootstrap : perGameSettings_useBootstrap) || !ms().secondaryDevice);
					if (proceedToLaunch && useBootstrapAnyway && bnrRomType[CURPOS] == 0 && !isDSiWare[CURPOS]
					 && isHomebrew[CURPOS] == 0
					 && checkIfDSiMode(dirContents[scrn].at(CURPOS + PAGENUM * 40).name)) {
						if (!dsiBinariesChecked && dsiFeatures() && (!ms().secondaryDevice || !bs().b4dsMode)) {
							hasDsiBinaries = checkDsiBinaries(dirContents[scrn].at(CURPOS + PAGENUM * 40).name.c_str(), CURPOS);
							dsiBinariesChecked = true;
						}

						if (!hasDsiBinaries) {
							proceedToLaunch = dsiBinariesMissingMsg(dirContents[scrn].at(CURPOS + PAGENUM * 40).name.c_str());
						}
					}
					if (proceedToLaunch && (useBootstrapAnyway || ((!dsiFeatures() || bs().b4dsMode) && isDSiWare[CURPOS])) && bnrRomType[CURPOS] == 0 && !dsModeForced && isHomebrew[CURPOS] == 0) {
						proceedToLaunch = checkForCompatibleGame(dirContents[scrn].at(CURPOS + PAGENUM * 40).name.c_str());
						if (proceedToLaunch && requiresDonorRom[CURPOS]) {
							const char* pathDefine = "DONORTWL_NDS_PATH"; // SDK5.x (TWL)
							if (requiresDonorRom[CURPOS] == 52) {
								pathDefine = "DONORTWLONLY_NDS_PATH"; // SDK5.x (TWL)
							} else if (requiresDonorRom[CURPOS] > 100) {
								pathDefine = "DONORTWL0_NDS_PATH"; // SDK5.0 (TWL)
								if (requiresDonorRom[CURPOS] == 152) {
									pathDefine = "DONORTWLONLY0_NDS_PATH"; // SDK5.0 (TWL)
								}
							} else if (requiresDonorRom[CURPOS] == 20) {
								pathDefine = "DONOR20_NDS_PATH"; // SDK2.0
							}
							std::string donorRomPath;
							const char *bootstrapinipath = sys().isRunFromSD() ? BOOTSTRAP_INI : BOOTSTRAP_INI_FC;
							int bstrap_dsiMode = (perGameSettings_dsiMode == -1 ? DEFAULT_DSI_MODE : perGameSettings_dsiMode);
							CIniFile bootstrapini(bootstrapinipath);
							donorRomPath = bootstrapini.GetString("NDS-BOOTSTRAP", pathDefine, "");
							bool donorRomFound = (((!dsiFeatures() || bs().b4dsMode) && requiresDonorRom[CURPOS] != 20 && ms().secondaryDevice && access("fat:/_nds/nds-bootstrap/b4dsTwlDonor.bin", F_OK) == 0)
												|| strncmp(donorRomPath.c_str(), "nand:", 5) == 0 || (donorRomPath != "" && access(donorRomPath.c_str(), F_OK) == 0));
							if (!donorRomFound && requiresDonorRom[CURPOS] != 20 && requiresDonorRom[CURPOS] < 100) {
								pathDefine = "DONORTWL0_NDS_PATH"; // SDK5.0 (TWL)
								if (requiresDonorRom[CURPOS] == 52) {
									pathDefine = "DONORTWLONLY0_NDS_PATH"; // SDK5.0 (TWL)
								}
								donorRomPath = bootstrapini.GetString("NDS-BOOTSTRAP", pathDefine, "");
								donorRomFound = (strncmp(donorRomPath.c_str(), "nand:", 5) == 0 || (donorRomPath != "" && access(donorRomPath.c_str(), F_OK) == 0));
							}
							if (!donorRomFound && (!dsiFeatures() || bs().b4dsMode) && ms().secondaryDevice && requiresDonorRom[CURPOS] != 20 && memcmp(gameTid[CURPOS], "KUB", 3) != 0) {
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
							&& (requiresDonorRom[CURPOS] == 20 || requiresDonorRom[CURPOS] == 51 || requiresDonorRom[CURPOS] == 151
							|| (requiresDonorRom[CURPOS] == 52 && (isDSiWare[CURPOS] || bstrap_dsiMode > 0)) || requiresDonorRom[CURPOS] == 152)
							) {
								proceedToLaunch = donorRomMsg(dirContents[scrn].at(CURPOS + PAGENUM * 40).name.c_str());
							}
						}
						if (proceedToLaunch && !apChecked && !isDSiWare[CURPOS] && checkIfShowAPMsg(dirContents[scrn].at(CURPOS + PAGENUM * 40).name)) {
							hasAP = checkRomAP(dirContents[scrn].at(CURPOS + PAGENUM * 40).name.c_str(), CURPOS);
							apChecked = true;
						}
						if (proceedToLaunch && isDSiWare[CURPOS] && (!dsiFeatures() || bs().b4dsMode) && ms().secondaryDevice) {
							if (!dsiFeatures() && !sys().isRegularDS()) {
								proceedToLaunch = dsiWareInDSModeMsg(dirContents[scrn].at(CURPOS + PAGENUM * 40).name);
							}
							if (proceedToLaunch) {
								proceedToLaunch = dsiWareRAMLimitMsg(dirContents[scrn].at(CURPOS + PAGENUM * 40).name);
							}
						}
					} else if (isHomebrew[CURPOS] == 1) {
						loadPerGameSettings(dirContents[scrn].at(CURPOS + PAGENUM * 40).name);
						if (requiresRamDisk[CURPOS] && perGameSettings_ramDiskNo == -1) {
							proceedToLaunch = false;
							ramDiskMsg(dirContents[scrn].at(CURPOS + PAGENUM * 40).name.c_str());
						}
					} else if (bnrRomType[CURPOS] == 7) {
						if (ms().mdEmulator == TWLSettings::EMegaDriveJenesis && getFileSize(
							dirContents[scrn].at(CURPOS + PAGENUM * 40).name.c_str()) >
							0x300000) {
							proceedToLaunch = false;
							mdRomTooBig();
						}
					} else if ((bnrRomType[CURPOS] == 8 || (bnrRomType[CURPOS] == 11 && ms().smsGgInRam))
							&& isDSiMode() && memcmp(io_dldi_data->friendlyName, "CycloDS iEvolution", 18) != 0 && sys().arm7SCFGLocked()) {
						proceedToLaunch = cannotLaunchMsg(dirContents[scrn].at(CURPOS + PAGENUM * 40).name.c_str());
					}
					if (hasAP) {
						if (ms().theme == TWLSettings::EThemeSaturn) {
							snd().playStartup();
							fadeType = false;	   // Fade to black
							for (int i = 0; i < 25; i++) {
								bgOperations(true);
							}
							currentBg = 1;
							displayGameIcons = false;
							fadeType = true;
						} else {
							dbox_showIcon = true;
							showdialogbox = true;
						}
						clearText();
						updateText(false);
						if (ms().theme == TWLSettings::EThemeSaturn) {
							while (!screenFadedIn()) { swiWaitForVBlank(); }
							dbox_showIcon = true;
							snd().playWrong();
						} else {
							for (int i = 0; i < 30; i++) { bgOperations(true); }
						}
						titleUpdate(dirContents[scrn].at(CURPOS + PAGENUM * 40).isDirectory,
								dirContents[scrn].at(CURPOS + PAGENUM * 40).name,
								CURPOS);
						printSmall(false, 0, 72, STR_AP_USE_LATEST, Alignment::center, FontPalette::dialog);
						printSmall(false, 0, 160, STR_B_A_OK_X_DONT_SHOW, Alignment::center, FontPalette::dialog);
						updateText(false);
						pressed = 0;
						while (1) {
							scanKeys();
							pressed = keysDown();
							updateBoxArt();
							bgOperations(true);
							if (pressed & KEY_A) {
								pressed = 0;
								break;
							} else if (pressed & KEY_B) {
								snd().playBack();
								proceedToLaunch = false;
								pressed = 0;
								break;
							} else if (pressed & KEY_X) {
								dontShowAPMsgAgain(
									dirContents[scrn].at(CURPOS + PAGENUM * 40).name);
								pressed = 0;
								break;
							}
						}
						showdialogbox = false;
						if (ms().theme == TWLSettings::EThemeSaturn) {
							fadeType = false;	   // Fade to black
							for (int i = 0; i < 25; i++) {
								bgOperations(true);
							}
							clearText();
							updateText(false);
							currentBg = 0;
							displayGameIcons = true;
							fadeType = true;
							snd().playStartup();
							if (proceedToLaunch) {
								while (!screenFadedIn()) { bgOperations(true); }
							}
						} else {
							clearText();
							updateText(false);
							for (int i = 0; i < (proceedToLaunch ? 20 : 15); i++) {
								bgOperations(true);
							}
						}
						dbox_showIcon = false;
					}

					// If SD card's cluster size is less than 32KB, then show warning for DS games with nds-bootstrap
					extern struct statvfs st[2];
					if ((useBootstrapAnyway || isDSiWare[CURPOS]) && bnrRomType[CURPOS] == 0 && (!isDSiWare[CURPOS] || (ms().secondaryDevice && (!sdFound() || !ms().dsiWareToSD || bs().b4dsMode))) && isHomebrew[CURPOS] == 0
					 && proceedToLaunch && st[ms().secondaryDevice].f_bsize < (32 << 10) && !ms().dontShowClusterWarning) {
						if (ms().theme == TWLSettings::EThemeSaturn) {
							snd().playStartup();
							fadeType = false; // Fade to black
							for (int i = 0; i < 25; i++) {
								bgOperations(true);
							}
							currentBg = 1;
							displayGameIcons = false;
							fadeType = true;
						} else {
							showdialogbox = true;
						}
						dbox_showIcon = false;
						clearText();
						updateText(false);
						if (ms().theme == TWLSettings::EThemeSaturn) {
							while (!screenFadedIn()) { bgOperations(true); }
							snd().playWrong();
						} else {
							while (!dboxStopped) { bgOperations(true); }
						}

						printSmall(false, 0, 40, STR_BAD_CLUSTER_SIZE, Alignment::center, FontPalette::dialog);
						printSmall(false, 0, 160, STR_B_A_OK_X_DONT_SHOW, Alignment::center, FontPalette::dialog);
						updateText(false);
						pressed = 0;
						while (1) {
							scanKeys();
							pressed = keysDown();
							updateBoxArt();
							bgOperations(true);
							if (pressed & KEY_A) {
								pressed = 0;
								break;
							}
							if (pressed & KEY_B) {
								snd().playBack();
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
						showdialogbox = false;
						if (ms().theme == TWLSettings::EThemeSaturn) {
							fadeType = false;	   // Fade to black
							for (int i = 0; i < 25; i++) {
								swiWaitForVBlank();
							}
							clearText();
							updateText(false);
							currentBg = 0;
							displayGameIcons = true;
							fadeType = true;
							snd().playStartup();
							if (proceedToLaunch) {
								while (!screenFadedIn()) { bgOperations(true); }
							}
						} else {
							clearText();
							updateText(false);
							for (int i = 0; i < (proceedToLaunch ? 20 : 15); i++) {
								bgOperations(true);
							}
						}
					}

					if (proceedToLaunch) {
						snd().playLaunch();
						controlTopBright = true;
						applaunch = true;

						if (ms().theme == TWLSettings::EThemeDSi) {
							applaunchprep = true;
							currentBg = 0;
							showSTARTborder = false;
							clearText(false); // Clear title
							updateText(false);

							fadeSpeed = false; // Slow fade speed
						}
						if (ms().theme == TWLSettings::EThemeHBL) {
							currentBg = 0;
							snd().fadeOutStream();
						} else if (ms().theme != TWLSettings::EThemeSaturn) {
							fadeType = false;		  // Fade to white
							snd().fadeOutStream();

							// Clear screen with white
							rocketVideo_playVideo = false;
							tex().unloadRotatingCubes();
						}

						if (ms().updateRecentlyPlayedList) {
							while (ms().theme != TWLSettings::EThemeSaturn && ms().theme != TWLSettings::EThemeHBL && !screenFadedOut()) {
								swiWaitForVBlank();
							}
							clearText();
							printLarge(false, 0, (ms().theme == TWLSettings::EThemeSaturn ? 72 : 88), STR_NOW_SAVING, Alignment::center);
							updateText(false);
							if (ms().theme == TWLSettings::EThemeHBL) {
								displayGameIcons = false;
								showProgressIcon = true;
							} else if (ms().theme != TWLSettings::EThemeSaturn) {
								fadeSpeed = true; // Fast fading
								whiteScreen = true;
								tex().clearTopScreen();
								fadeType = true; // Fade in from white
								for (int i = 0; i < 25; i++) {
									swiWaitForVBlank();
								}
								showProgressIcon = true;
							}

							printSmall(false, 0, 20, STR_IF_CRASH_DISABLE_RECENT, Alignment::center);
							updateText(false);

							if (!dsiFeatures()) {
								// Free some RAM space to avoid possible memory leaks
								snd().unloadStream();
								mmEffectCancelAll(); // Stop sound effects from playing to avoid sound glitches
								snd().unloadSfxData();
								tex().unloadPhotoBuffer();
							}

							mkdir(sys().isRunFromSD() ? "sd:/_nds/TWiLightMenu/extras" : "fat:/_nds/TWiLightMenu/extras",
						  0777);

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

							if (ms().theme == TWLSettings::EThemeHBL) {
								displayGameIcons = true;
							} else if (ms().theme != TWLSettings::EThemeSaturn) {
								showProgressIcon = false;
								fadeType = false;	   // Fade to white
							}
						}

						// Return the chosen file
						return entry->name;
					}
				}
			}
			gameTapped = false;

			if (ms().theme == TWLSettings::ETheme3DS) {
				int topIconXpos = 116;
				for (int i = 0; i < 3; i++) {
					topIconXpos -= 16;
				}
				int savedTopIconXpos[5] = {0};
				int topIconOp[5] = {0};
				int topIconCount = 5;
				if ((isDSiMode() && sdFound()) || bothSDandFlashcard() || (io_dldi_data->ioInterface.features & FEATURE_SLOT_GBA)) {
					//for (int i = 0; i < 2; i++) {
						topIconXpos -= 16;
					//}
					for (int i = 0; i < 5; i++) {
						savedTopIconXpos[i] = topIconXpos;
						topIconXpos += 32;
					}

					topIconOp[0] = 0;
					topIconOp[1] = 2;
					topIconOp[2] = 3;
					topIconOp[3] = 4;
					topIconOp[4] = 5;
				} else if (ms().gbaBooter == TWLSettings::EGbaNativeGbar2) {
					// for (int i = 0; i < 2; i++) {
						topIconXpos -= 16;
					//}
					for (int i = 0; i < 5; i++) {
						savedTopIconXpos[i] = topIconXpos;
						topIconXpos += 32;
					}

					topIconOp[0] = 1;
					topIconOp[1] = 2;
					topIconOp[2] = 3;
					topIconOp[3] = 4;
					topIconOp[4] = 5;
				} else {
					topIconCount = 4;
					for (int i = 0; i < 4; i++) {
						savedTopIconXpos[i] = topIconXpos;
						topIconXpos += 32;
					}

					topIconOp[0] = 2;
					topIconOp[1] = 3;
					topIconOp[2] = 4;
					topIconOp[3] = 5;
				}

				// Launch TWLMenu++ Settings by touching corner button
				if ((pressed & KEY_TOUCH) && touch.py >= 0 && touch.py <= 26 && touch.px >= 0 && touch.px <= 44) {
					if (ms().kioskMode) {
						snd().playWrong();
					} else {
						launchSettings();
					}
				} else

				// Exit to system menu by touching corner button
				if ((pressed & KEY_TOUCH) && touch.py >= 0 && touch.py <= 26 && touch.px >= 212 && touch.px <= 255 && !sys().isRegularDS()) {
					exitToSystemMenu();
				} else

				for (int i = 0; i < topIconCount; i++) {
					if ((pressed & KEY_TOUCH) && touch.py >= 0 && touch.py <= 26 && touch.px >= savedTopIconXpos[i] && touch.px < savedTopIconXpos[i] + 24) {
						switch (topIconOp[i]) {
							case 0: { // Switch devices or launch Slot-1 by touching button
								if (ms().secondaryDevice || REG_SCFG_MC != 0x11) {
									switchDevice();
									return "null";
								} else {
									snd().playWrong();
								}
							} break;
							case 1: // Launch GBA
								launchGba();
								break;
							case 2: // Launch Pictochat
								launchPictochat(dirContents[scrn]);
								break;
							case 3: // Launch DS Download Play
								launchDownloadPlay(dirContents[scrn]);
								break;
							case 4: // Launch Internet Browser
								launchInternetBrowser(dirContents[scrn]);
								break;
							case 5: // Open the manual
								launchManual();
								break;
							default:
								snd().playWrong();
								break;
						}
						break;
					}
				}
			}

			// page switch
			if (pressed & KEY_L) {
				if (previousPage(scrn, dirContents)) {
					break;
				}
			} else if (pressed & KEY_R) {
				if (nextPage(scrn, dirContents)) {
					break;
				}
			}

			if ((pressed & KEY_B) && ms().showDirectories) {
				// Go up a directory
				snd().playBack();
				if (ms().theme != TWLSettings::EThemeSaturn && ms().theme != TWLSettings::EThemeHBL) {
					fadeType = false; // Fade to white
					for (int i = 0; i < 6; i++) {
						bgOperations(true);
					}
				}
				PAGENUM = 0;
				CURPOS = 0;
				titleboxXdest[ms().secondaryDevice] = 0;
				titlewindowXdest[ms().secondaryDevice] = 0;
				titleboxXpos[ms().secondaryDevice] = titleboxXdest[ms().secondaryDevice];
				titlewindowXpos[ms().secondaryDevice] = titlewindowXdest[ms().secondaryDevice];
				if (ms().theme != TWLSettings::EThemeSaturn && ms().theme != TWLSettings::EThemeHBL) whiteScreen = true;
				if (ms().showBoxArt)
					clearBoxArt(); // Clear box art
				boxArtLoaded = false;
				bannerTextShown = false;
				rocketVideo_playVideo = true;
				shouldersRendered = false;
				currentBg = 0;
				showSTARTborder = false;
				stopSoundPlayed = false;
				clearText();
				updateText(false);
				chdir("..");
				char buf[256];

				ms().romfolder[ms().secondaryDevice] = std::string(getcwd(buf, 256));
				ms().saveSettings();
				settingsChanged = false;
				return "null";
			}

			// (un)Hide file/folder
			if ((pressed & KEY_X) && !ms().kioskMode && !ms().preventDeletion && bannerTextShown && showSTARTborder
			&& dirContents[scrn].at(CURPOS + PAGENUM * 40).name != "..") {
				DirEntry *entry = &dirContents[scrn].at((PAGENUM * 40) + (CURPOS));
				bool unHide = (FAT_getAttr(entry->name.c_str()) & ATTR_HIDDEN || (strncmp(entry->name.c_str(), ".", 1) == 0 && entry->name != ".."));
				if (ms().theme == TWLSettings::EThemeSaturn) {
					snd().playStartup();
					fadeType = false;	   // Fade to black
					for (int i = 0; i < 25; i++) {
						bgOperations(true);
					}
					currentBg = 1;
					displayGameIcons = false;
					fadeType = true;
				} else {
					dbox_showIcon = true;
					showdialogbox = true;
				}
				clearText();
				updateText(false);
				if (ms().theme == TWLSettings::EThemeSaturn) {
					while (!screenFadedIn()) { bgOperations(true); }
					dbox_showIcon = true;
				} else {
					while (!dboxStopped) { bgOperations(true); }
				}
				snprintf(fileCounter, sizeof(fileCounter), "%i/%i", (CURPOS + 1) + PAGENUM * 40,
					 file_count);
				titleUpdate(dirContents[scrn].at(CURPOS + PAGENUM * 40).isDirectory,
						dirContents[scrn].at(CURPOS + PAGENUM * 40).name, CURPOS);
				dirContName = dirContents[scrn].at(CURPOS + PAGENUM * 40).name;
				// About 38 characters fit in the box.
				if (strlen(dirContName.c_str()) > 38) {
					// Truncate to 35, 35 + 3 = 38 (because we append "...").
					dirContName.resize(35, ' ');
					size_t first = dirContName.find_first_not_of(' ');
					size_t last = dirContName.find_last_not_of(' ');
					dirContName = dirContName.substr(first, (last - first + 1));
					dirContName.append("...");
				}
				printSmall(false, 16, 66, dirContName, Alignment::left, FontPalette::dialog);
				printSmall(false, 16, 160, fileCounter, Alignment::left, FontPalette::dialog);
				if (isTwlm[CURPOS]) {
					if (unHide)
						printSmall(false, 0, 112, STR_ARE_YOU_SURE_UNHIDE_TITLE, Alignment::center, FontPalette::dialog);
					else
						printSmall(false, 0, 112, STR_ARE_YOU_SURE_HIDE_TITLE, Alignment::center, FontPalette::dialog);
				} else if (isDirectory[CURPOS]) {
					if (unHide)
						printSmall(false, 0, 112, STR_ARE_YOU_SURE_UNHIDE, Alignment::center, FontPalette::dialog);
					else
						printSmall(false, 0, 112, STR_ARE_YOU_SURE_HIDE, Alignment::center, FontPalette::dialog);
				} else {
					if (unHide)
						printSmall(false, 0, 112, STR_ARE_YOU_SURE_DELETE_UNHIDE, Alignment::center, FontPalette::dialog);
					else
						printSmall(false, 0, 112, STR_ARE_YOU_SURE_DELETE_HIDE, Alignment::center, FontPalette::dialog);
				}
				updateText(false);
				for (int i = 0; i < 90; i++) {
					bgOperations(true);
				}
				if (isTwlm[CURPOS] || isDirectory[CURPOS]) {
					printSmall(false, 240, 160, (unHide ? STR_Y_UNHIDE : STR_Y_HIDE) + "  " + STR_B_NO, Alignment::right, FontPalette::dialog);
				} else {
					printSmall(false, 240, 160, (unHide ? STR_Y_UNHIDE : STR_Y_HIDE) + "  " + STR_A_DEL + "  " + STR_B_NO, Alignment::right, FontPalette::dialog);
				}
				updateText(false);
				while (1) {
					do {
						scanKeys();
						pressed = keysDown();
						bgOperations(true);
					} while (!pressed);

					if ((pressed & KEY_A) && !isTwlm[CURPOS] && !isDirectory[CURPOS]) {
						snd().playLaunch();
						if (ms().theme != TWLSettings::EThemeSaturn && ms().theme != TWLSettings::EThemeHBL) {
							fadeType = false; // Fade to white
							for (int i = 0; i < 30; i++) {
								bgOperations(true);
							}
							whiteScreen = true;
						}
						remove(dirContents[scrn]
							   .at(CURPOS + PAGENUM * 40)
							   .name.c_str()); // Remove game/folder
						if (ms().showBoxArt)
							clearBoxArt(); // Clear box art
						boxArtLoaded = false;
						bannerTextShown = false;
						rocketVideo_playVideo = true;
						shouldersRendered = false;
						currentBg = 0;
						showSTARTborder = false;
						stopSoundPlayed = false;
						clearText();
						updateText(false);
						showdialogbox = false;
						dbox_showIcon = false;
						ms().saveSettings();
						settingsChanged = false;
						return "null";
					}

					if (pressed & KEY_B) {
						snd().playBack();
						break;
					}

					if (pressed & KEY_Y) {
						snd().playLaunch();
						if (ms().theme != TWLSettings::EThemeSaturn && ms().theme != TWLSettings::EThemeHBL) {
							fadeType = false; // Fade to white
							for (int i = 0; i < 30; i++) {
								bgOperations(true);
							}
							whiteScreen = true;
						}

						// Remove leading . if it exists
						if ((strncmp(entry->name.c_str(), ".", 1) == 0 && entry->name != "..")) {
							rename(entry->name.c_str(), entry->name.substr(1).c_str());
						} else { // Otherwise toggle the hidden attribute bit
							FAT_setAttr(entry->name.c_str(), FAT_getAttr(entry->name.c_str()) ^ ATTR_HIDDEN);
						}

						if (ms().showBoxArt)
							clearBoxArt(); // Clear box art
						boxArtLoaded = false;
						bannerTextShown = false;
						shouldersRendered = false;
						currentBg = 0;
						showSTARTborder = false;
						stopSoundPlayed = false;
						clearText();
						updateText(false);
						showdialogbox = false;
						dbox_showIcon = false;
						ms().saveSettings();
						settingsChanged = false;
						return "null";
					}
				}
				showdialogbox = false;
				if (ms().theme == TWLSettings::EThemeHBL) {
					dbox_showIcon = false;
				}
				if (ms().theme == TWLSettings::EThemeSaturn) {
					fadeType = false;	   // Fade to black
					for (int i = 0; i < 25; i++) {
						bgOperations(true);
					}
					clearText();
					updateText(false);
					currentBg = 0;
					displayGameIcons = true;
					fadeType = true;
					if (ms().theme == TWLSettings::EThemeSaturn) snd().playStartup();
				} else {
					clearText();
					updateText(false);
					for (int i = 0; i < 15; i++) { bgOperations(true); }
				}
				dbox_showIcon = false;
				bannerTextShown = false;
			}

			if ((pressed & KEY_Y) && !ms().kioskMode && isValid[CURPOS] && !isTwlm[CURPOS] && !isDirectory[CURPOS] && bannerTextShown && showSTARTborder) {
				perGameSettings(dirContents[scrn].at(CURPOS + PAGENUM * 40).name, &hasDsiBinaries, &dsiBinariesChecked);
				bannerTextShown = false;
			}

			if (held & KEY_SELECT) {
				bannerTextShown = false;
				bool runSelectMenu = pressed & KEY_SELECT;
				bool break2 = false;
				if (pressed & KEY_SELECT && dsiFeatures() && ms().consoleModel < 2) {
					fifoSendValue32(FIFO_USER_04, 1); // enable backlight level change reports
				}
				while (held & KEY_SELECT) {
					scanKeys();
					pressed = keysDown();
					held = keysHeld();
					bgOperations(true);

					// page switch
					if (pressed & KEY_LEFT) {
						runSelectMenu = false;
						if (previousPage(scrn, dirContents)) {
							break2 = true;
							break;
						}
					} else if (pressed & KEY_RIGHT) {
						runSelectMenu = false;
						if (nextPage(scrn, dirContents)) {
							break2 = true;
							break;
						}
					}

					// set/clear default directory
					if ((pressed & KEY_X)) {
						runSelectMenu = false;
						if (setDefaultDirectory(ms().romfolder[ms().secondaryDevice]))
							break;
					}

					if (ms().theme == TWLSettings::EThemeDSi || ms().theme == TWLSettings::EThemeSaturn || ms().theme == TWLSettings::EThemeHBL) {
						if (bothSDandFlashcard() && ((pressed & KEY_UP) || (pressed & KEY_DOWN))) {
							switchDevice();
							
							return "null";
						}
					}

					// detect backlight level change on DSi consoles
					if (runSelectMenu && dsiFeatures() && ms().consoleModel < 2 && fifoGetValue32(FIFO_USER_04)) {
						runSelectMenu = false;
						fifoSendValue32(FIFO_USER_04, 2); // disable backlight level change report
					}
				}
				if (dsiFeatures() && ms().consoleModel < 2) fifoSendValue32(FIFO_USER_04, 2); // disable backlight level change report
				if (break2) break;
				if (runSelectMenu && (ms().theme == TWLSettings::EThemeDSi || ms().theme == TWLSettings::EThemeSaturn || ms().theme == TWLSettings::EThemeHBL)) {
					if (ms().showSelectMenu) {
						if (selectMenu()) {
							clearText();
							updateText(false);
							showdialogbox = false;
							dbox_selectMenu = false;
							if (ms().theme == TWLSettings::EThemeSaturn) currentBg = 0;
							return "null";
						}
					} else {
						launchDsClassicMenu();
					}
				}
			}
		}
	}
}
