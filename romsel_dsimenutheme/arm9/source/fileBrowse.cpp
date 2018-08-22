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
#include "graphics/iconHandler.h"
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
#define ENTRIES_PER_SCREEN 15
#define ENTRIES_START_ROW 3
#define ENTRY_PAGE_LENGTH 10

extern bool whiteScreen;
extern bool fadeType;
extern bool fadeSpeed;
extern bool controlTopBright;
extern bool controlBottomBright;

extern bool startButtonLaunch;
extern int launchType;
extern bool slot1LaunchMethod;
extern bool bootstrapFile;
extern bool homebrewBootstrap;
extern bool useGbarunner;
extern bool arm7SCFGLocked;
extern int consoleModel;
extern bool isRegularDS;

extern bool dropDown;
extern bool redoDropDown;
extern bool showbubble;
extern bool showSTARTborder;

extern bool titleboxXmoveleft;
extern bool titleboxXmoveright;

extern bool applaunchprep;

extern touchPosition touch;

extern bool showdialogbox;

extern std::string romfolder;

extern bool applaunch;

extern bool gotosettings;

extern bool useBootstrap;

using namespace std;

extern bool startMenu;

extern int theme;

int file_count = 0;

extern bool showDirectories;
extern bool showBoxArt;
extern int spawnedtitleboxes;
extern int cursorPosition;
extern int startMenu_cursorPosition;
extern int pagenum;
extern int dsiWarePageNum;
extern int titleboxXpos;
extern int titlewindowXpos;

extern bool showLshoulder;
extern bool showRshoulder;

extern bool showProgressIcon;

extern bool flashcardUsed;

char boxArtPath[40][256];

bool boxArtLoaded = false;
bool shouldersRendered = false;
bool settingsChanged = false;

extern void SaveSettings();

extern std::string ReplaceAll(std::string str, const std::string& from, const std::string& to);

extern void loadGameOnFlashcard(const char* filename);
extern void dsCardLaunch();

mm_sound_effect snd_launch;
mm_sound_effect snd_select;
mm_sound_effect snd_stop;
mm_sound_effect snd_wrong;
mm_sound_effect snd_back;
mm_sound_effect snd_switch;
mm_sound_effect mus_menu;

void InitSound() {
	mmInitDefaultMem((mm_addr)soundbank_bin);
	
	mmLoadEffect( SFX_LAUNCH );
	mmLoadEffect( SFX_SELECT );
	mmLoadEffect( SFX_STOP );
	mmLoadEffect( SFX_WRONG );
	mmLoadEffect( SFX_BACK );
	mmLoadEffect( SFX_SWITCH );
	mmLoadEffect( SFX_MENU );

	snd_launch = {
		{ SFX_LAUNCH } ,			// id
		(int)(1.0f * (1<<10)),	// rate
		0,		// handle
		255,	// volume
		128,	// panning
	};
	snd_select = {
		{ SFX_SELECT } ,			// id
		(int)(1.0f * (1<<10)),	// rate
		0,		// handle
		255,	// volume
		128,	// panning
	};
	snd_stop = {
		{ SFX_STOP } ,			// id
		(int)(1.0f * (1<<10)),	// rate
		0,		// handle
		255,	// volume
		128,	// panning
	};
	snd_wrong = {
		{ SFX_WRONG } ,			// id
		(int)(1.0f * (1<<10)),	// rate
		0,		// handle
		255,	// volume
		128,	// panning
	};
	snd_back = {
		{ SFX_BACK } ,			// id
		(int)(1.0f * (1<<10)),	// rate
		0,		// handle
		255,	// volume
		128,	// panning
	};
	snd_switch = {
		{ SFX_SWITCH } ,			// id
		(int)(1.0f * (1<<10)),	// rate
		0,		// handle
		255,	// volume
		128,	// panning
	};
	mus_menu = {
		{ SFX_MENU } ,			// id
		(int)(1.0f * (1<<10)),	// rate
		0,		// handle
		255,	// volume
		128,	// panning
	};
}

extern bool music;

extern char usernameRendered[11];
extern bool usernameRenderedDone;

struct DirEntry
{
	string name;
	string visibleName;
	bool isDirectory;
};

TextEntry *pathText = nullptr;
char *path = new char[PATH_MAX];

#ifdef EMULATE_FILES
#define chdir(a) chdirFake(a)
void chdirFake(const char *dir)
{
	string pathStr(path);
	string dirStr(dir);
	if (dirStr == "..")
	{
		pathStr.resize(pathStr.find_last_of("/"));
		pathStr.resize(pathStr.find_last_of("/") + 1);
	}
	else
	{
		pathStr += dirStr;
		pathStr += "/";
	}
	strcpy(path, pathStr.c_str());
}
#endif

bool nameEndsWith(const string& name, const vector<string> extensionList)
{

	if (name.size() == 0) return false;

	if (extensionList.size() == 0) return true;

	for (int i = 0; i < (int) extensionList.size(); i++)
	{
		const string ext = extensionList.at(i);
		if (strcasecmp(name.c_str() + name.size() - ext.size(), ext.c_str()) == 0) return true;
	}
	return false;
}

bool dirEntryPredicate(const DirEntry& lhs, const DirEntry& rhs)
{

	if (!lhs.isDirectory && rhs.isDirectory)
	{
		return false;
	}
	if (lhs.isDirectory && !rhs.isDirectory)
	{
		return true;
	}
	return strcasecmp(lhs.name.c_str(), rhs.name.c_str()) < 0;
}

void getDirectoryContents(vector<DirEntry>& dirContents, const vector<string> extensionList)
{

	dirContents.clear();

#ifdef EMULATE_FILES

	string vowels = "aeiou";
	string consonants = "bcdfghjklmnpqrstvwxyz";

	DirEntry first;
	first.name = "AAA First";
	first.visibleName = "[AAA First]";
	first.isDirectory = true;
	dirContents.push_back(first);

	for (int i = 0; i < rand() % 10 + 5; ++i)
	{
		ostringstream fileName;
		DirEntry dirEntry;
		for (int j = 0; j < rand() % 15 + 4; ++j)
			fileName << (j % 2 == 0 ? consonants[rand() % consonants.size()]
				: vowels[rand() % vowels.size()]);
		dirEntry.name = fileName.str();
		if ((dirEntry.isDirectory = rand() % 2))
			dirEntry.visibleName = "[" + dirEntry.name + "]";
		else
			dirEntry.visibleName = dirEntry.name;
		dirContents.push_back(dirEntry);
	}
	DirEntry last;
	last.name = "ZZZ Last";
	last.visibleName = last.name;
	last.isDirectory = false;
	dirContents.push_back(last);

#else
	file_count = 0;
	
	struct stat st;
	DIR *pdir = opendir(".");

	if (pdir == NULL)
	{
		// iprintf("Unable to open the directory.\n");
		printSmall (false, 4, 4, "Unable to open the directory.");
	}
	else
	{

		while (true)
		{
			DirEntry dirEntry;

			struct dirent* pent = readdir(pdir);
			if (pent == NULL) break;

			stat(pent->d_name, &st);
			dirEntry.name = pent->d_name;
			dirEntry.isDirectory = (st.st_mode & S_IFDIR) ? true : false;

			if (dirEntry.isDirectory)
				dirEntry.visibleName = "[" + dirEntry.name + "]";
			else
				dirEntry.visibleName = dirEntry.name;

			if (showDirectories) {
				if (dirEntry.name.compare(".") != 0 && dirEntry.name.compare("_nds") != 0 && (dirEntry.isDirectory || nameEndsWith(dirEntry.name, extensionList))) {
					dirContents.push_back(dirEntry);
					file_count++;
				}
			} else {
				if (dirEntry.name.compare(".") != 0 && (nameEndsWith(dirEntry.name, extensionList))) {
					dirContents.push_back(dirEntry);
					file_count++;
				}
			}

		}

		closedir(pdir);
	}

#endif

	sort(dirContents.begin(), dirContents.end(), dirEntryPredicate);
}

void getDirectoryContents(vector<DirEntry>& dirContents)
{
	vector<string> extensionList;
	getDirectoryContents(dirContents, extensionList);
}

void updatePath()
{
#ifndef EMULATE_FILES
	getcwd(path, PATH_MAX);
#else
	if (strlen(path) < 1)
	{
		path[0] = '/';
		path[1] = '\0';
	}
#endif
	if (pathText == nullptr)
	{
		printLarge(false, 2 * FONT_SX, 1 * FONT_SY, path);
		pathText = getPreviousTextEntry(false);
		pathText->anim = TextEntry::AnimType::IN;
		pathText->fade = TextEntry::FadeType::NONE;
		pathText->y = 100;
		pathText->immune = true;
	}

	int pathWidth = calcLargeFontWidth(path);
	pathText->delay = TextEntry::ACTIVE;
	pathText->finalX = min(2 * FONT_SX, -(pathWidth + 2 * FONT_SX - 256));
}

bool isTopLevel(const char *path)
{
#ifdef EMULATE_FILES
	return strlen(path) <= strlen("/");
#else
	return strlen(path) <= strlen("fat: /");
#endif
}

void waitForFadeOut (void) {
	if (!dropDown && theme == 0) {
		dropDown = true;
		for (int i = 0; i < 72; i++) swiWaitForVBlank();
	} else {
		for (int i = 0; i < 25; i++) swiWaitForVBlank();
	}
}

bool nowLoadingDisplaying = false;

void displayNowLoading(void) {
	fadeType = true;	// Fade in from white
	printLargeCentered(false, 88, "Now Loading...");
	nowLoadingDisplaying = true;
	for (int i = 0; i < 35; i++) swiWaitForVBlank();
	showProgressIcon = true;
	controlTopBright = false;
}

string browseForFile(const vector<string> extensionList, const char* username)
{
	displayNowLoading();

	int pressed = 0;
	SwitchState scrn(3);
	vector<DirEntry> dirContents[scrn.SIZE];

	getDirectoryContents(dirContents[scrn], extensionList);

	while (1) {
		spawnedtitleboxes = 0;
		for(int i = 0; i < 40; i++) {
			if (i+pagenum*40 < file_count) {
				if (dirContents[scrn].at(i+pagenum*40).isDirectory) {
					isDirectory[i] = true;
					bnrWirelessIcon[i] = 0;
				} else {
					isDirectory[i] = false;
					std::string std_romsel_filename = dirContents[scrn].at(i+pagenum*40).name.c_str();
					if((std_romsel_filename.substr(std_romsel_filename.find_last_of(".") + 1) == "nds")
					|| (std_romsel_filename.substr(std_romsel_filename.find_last_of(".") + 1) == "app")
					|| (std_romsel_filename.substr(std_romsel_filename.find_last_of(".") + 1) == "argv")
					|| (std_romsel_filename.substr(std_romsel_filename.find_last_of(".") + 1) == "launcharg"))
					{
						getGameInfo(isDirectory[i], dirContents[scrn].at(i+pagenum*40).name.c_str(), i);
						bnrRomType[i] = 0;
					} else if((std_romsel_filename.substr(std_romsel_filename.find_last_of(".") + 1) == "gb")
							|| std_romsel_filename.substr(std_romsel_filename.find_last_of(".") + 1) == "sgb")
					{
						bnrRomType[i] = 1;
						bnrWirelessIcon[i] = 0;
						isDSiWare[i] = false;
						isHomebrew[i] = false;
					} else if(std_romsel_filename.substr(std_romsel_filename.find_last_of(".") + 1) == "gbc") {
						bnrRomType[i] = 2;
						bnrWirelessIcon[i] = 0;
						isDSiWare[i] = false;
						isHomebrew[i] = false;
					} else if((std_romsel_filename.substr(std_romsel_filename.find_last_of(".") + 1) == "nes")
							|| std_romsel_filename.substr(std_romsel_filename.find_last_of(".") + 1) == "fds")
					{
						bnrRomType[i] = 3;
						bnrWirelessIcon[i] = 0;
						isDSiWare[i] = false;
						isHomebrew[i] = false;
					}

					if (showBoxArt) {
						// Store box art path
						snprintf (boxArtPath[i], sizeof(boxArtPath[i]), "/_nds/dsimenuplusplus/boxart/%s.bmp", dirContents[scrn].at(i+pagenum*40).name.c_str());
						if (!access(boxArtPath[i], F_OK)) {
						} else if (bnrRomType[i] == 0) {
							if((std_romsel_filename.substr(std_romsel_filename.find_last_of(".") + 1) == "argv")
							|| (std_romsel_filename.substr(std_romsel_filename.find_last_of(".") + 1) == "launcharg"))
							{
								vector<char*> argarray;

								FILE *argfile = fopen(std_romsel_filename.c_str(),"rb");
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
								std_romsel_filename = argarray.at(0);
							}
							// Get game's TID
							FILE *f_nds_file = fopen(std_romsel_filename.c_str(), "rb");
							char game_TID[5];
							grabTID(f_nds_file, game_TID);
							game_TID[4] = 0;
							fclose(f_nds_file);

							snprintf (boxArtPath[i], sizeof(boxArtPath[i]), "/_nds/dsimenuplusplus/boxart/%s.bmp", game_TID);
						}
					}
				}
				spawnedtitleboxes++;
			}
		}
		// Load correct icons depending on cursor position
		if (cursorPosition <= 1) {
			for(int i = 0; i < 5; i++) {
				if (bnrRomType[i] == 0 && i+pagenum*40 < file_count) {
					iconUpdate(dirContents[scrn].at(i+pagenum*40).isDirectory, dirContents[scrn].at(i+pagenum*40).name.c_str(), i);
				}
			}
		} else if (cursorPosition >= 2 && cursorPosition <= 36) {
			for(int i = 0; i < 6; i++) {
				if (bnrRomType[i] == 0 && (cursorPosition-2+i)+pagenum*40 < file_count) {
					iconUpdate(dirContents[scrn].at((cursorPosition-2+i)+pagenum*40).isDirectory, dirContents[scrn].at((cursorPosition-2+i)+pagenum*40).name.c_str(), cursorPosition-2+i);
				}
			}
		} else if (cursorPosition >= 37 && cursorPosition <= 39) {
			for(int i = 0; i < 5; i++) {
				if (bnrRomType[i] == 0 && (35+i)+pagenum*40 < file_count) {
					iconUpdate(dirContents[scrn].at((35+i)+pagenum*40).isDirectory, dirContents[scrn].at((35+i)+pagenum*40).name.c_str(), 35+i);
				}
			}
		}

		if (nowLoadingDisplaying) {
			showProgressIcon = false;
			fadeType = false;	// Fade to white
			for (int i = 0; i < 30; i++) swiWaitForVBlank();
			nowLoadingDisplaying = false;
			clearText(false);
		}
		whiteScreen = false;
		fadeType = true;	// Fade in from white
		for (int i = 0; i < 5; i++) swiWaitForVBlank();
		waitForFadeOut();

		/* clearText(false);
		updatePath();
		TextPane *pane = &createTextPane(20, 3 + ENTRIES_START_ROW*FONT_SY, ENTRIES_PER_SCREEN);
		for (auto &i : dirContents[scrn])
			pane->addLine(i.visibleName.c_str());
		pane->createDefaultEntries();
		pane->slideTransition(true);

		printSmall(false, 12 - 16, 4 + 10 * (cursorPosition - screenOffset + ENTRIES_START_ROW), ">");
		TextEntry *cursor = getPreviousTextEntry(false);
		cursor->fade = TextEntry::FadeType::IN;
		cursor->finalX += 16; */

		while (1)
		{
			// cursor->finalY = 4 + 10 * (cursorPosition - screenOffset + ENTRIES_START_ROW);
			// cursor->delay = TextEntry::ACTIVE;

			if (startMenu) {
				if (startMenu_cursorPosition < (3-flashcardUsed)) {
					showbubble = true;
					showSTARTborder = true;
					titleUpdate(false, "startMenu", startMenu_cursorPosition);
				} else {
					showbubble = false;
					showSTARTborder = false;
					clearText(false);	// Clear title
				}
			} else {
				if (cursorPosition+pagenum*40 > ((int) dirContents[scrn].size() - 1)) {
					showbubble = false;
					showSTARTborder = false;
					clearText(false);	// Clear title
					if (!boxArtLoaded && showBoxArt) {
						clearBoxArt();	// Clear box art
						boxArtLoaded = true;
					}
				} else {
					showbubble = true;
					showSTARTborder = true;
					titleUpdate(dirContents[scrn].at(cursorPosition+pagenum*40).isDirectory, dirContents[scrn].at(cursorPosition+pagenum*40).name.c_str(), cursorPosition);
					if (!boxArtLoaded && showBoxArt) {
						if (isDirectory[cursorPosition]) {
							clearBoxArt();	// Clear box art, if it's a directory
						} else {
							loadBoxArt(boxArtPath[cursorPosition]);	// Load box art
						}
						boxArtLoaded = true;
					}
				}
			}

			if (!shouldersRendered) {
				showLshoulder = false;
				showRshoulder = false;
				if (startMenu) {
				} else {
					if (pagenum != 0) {
						showLshoulder = true;
					}
					if (file_count > 40+pagenum*40) {
						showRshoulder = true;
					}
				}
				loadShoulders();
				shouldersRendered = true;
			}

			// Power saving loop. Only poll the keys once per frame and sleep the CPU if there is nothing else to do
			do
			{
				scanKeys();
				pressed = keysDownRepeat();
				touchRead(&touch);
				swiWaitForVBlank();
			}
			while (!pressed);

			if (((pressed & KEY_LEFT) && !titleboxXmoveleft && !titleboxXmoveright)
			|| ((pressed & KEY_TOUCH) && touch.py > 88 && touch.py < 144 && touch.px < 96 && !titleboxXmoveleft && !titleboxXmoveright)		// Title box
			|| ((pressed & KEY_TOUCH) && touch.py > 171 && touch.px < 19 && theme == 0 && !titleboxXmoveleft && !titleboxXmoveright))		// Button arrow (DSi theme)
			{
				if (startMenu) {
					startMenu_cursorPosition -= 1;
					if (startMenu_cursorPosition >= 0) {
						titleboxXmoveleft = true;
						mmEffectEx(&snd_select);
						settingsChanged = true;
					} else {
						mmEffectEx(&snd_wrong);
					}
				} else {
					cursorPosition -= 1;
					if (cursorPosition >= 0) {
						titleboxXmoveleft = true;
						mmEffectEx(&snd_select);
						boxArtLoaded = false;
						settingsChanged = true;
					} else {
						mmEffectEx(&snd_wrong);
					}
					if(cursorPosition >= 2 && cursorPosition <= 36) {
						if (bnrRomType[cursorPosition-2] == 0 && (cursorPosition-2)+pagenum*40 < file_count) {
							iconUpdate(dirContents[scrn].at((cursorPosition-2)+pagenum*40).isDirectory, dirContents[scrn].at((cursorPosition-2)+pagenum*40).name.c_str(), cursorPosition-2);
							reloadFontPalettes();
							reloadIconPalettes();
						}
					}
				}
			} else if (((pressed & KEY_RIGHT) && !titleboxXmoveleft && !titleboxXmoveright)
					|| ((pressed & KEY_TOUCH) && touch.py > 88 && touch.py < 144 && touch.px > 160 && !titleboxXmoveleft && !titleboxXmoveright)		// Title box
					|| ((pressed & KEY_TOUCH) && touch.py > 171 && touch.px > 236 && theme == 0 && !titleboxXmoveleft && !titleboxXmoveright))		// Button arrow (DSi theme)
			{
				if (startMenu) {
					startMenu_cursorPosition += 1;
					if (startMenu_cursorPosition <= 39) {
						titleboxXmoveright = true;
						mmEffectEx(&snd_select);
						settingsChanged = true;
					} else {
						mmEffectEx(&snd_wrong);
					}
				} else {
					cursorPosition += 1;
					if (cursorPosition <= 39) {
						titleboxXmoveright = true;
						mmEffectEx(&snd_select);
						boxArtLoaded = false;
						settingsChanged = true;
					} else {
						mmEffectEx(&snd_wrong);
					}
					if(cursorPosition >= 3 && cursorPosition <= 37) {
						if (bnrRomType[cursorPosition+2] == 0 && (cursorPosition+2)+pagenum*40 < file_count) {
							iconUpdate(dirContents[scrn].at((cursorPosition+2)+pagenum*40).isDirectory, dirContents[scrn].at((cursorPosition+2)+pagenum*40).name.c_str(), cursorPosition+2);
							reloadFontPalettes(); 	// Reload font to avoid font corruption
							reloadIconPalettes();
						}
					}
				}
			}

			if (startMenu) {
				if (startMenu_cursorPosition < 0)
				{
					startMenu_cursorPosition = 0;
				}
				else if (startMenu_cursorPosition > 39)
				{
					startMenu_cursorPosition = 39;
				}
			} else {
				if (cursorPosition < 0)
				{
					cursorPosition = 0;
				}
				else if (cursorPosition > 39)
				{
					cursorPosition = 39;
				}
			}

			int pressedToLaunch = 0;
			if (startButtonLaunch) {
				pressedToLaunch = (pressed & KEY_START);
			} else {
				pressedToLaunch = (pressed & KEY_A);
			}

			if ((pressedToLaunch && !titleboxXmoveleft && !titleboxXmoveright && showSTARTborder)
			|| ((pressed & KEY_TOUCH) && touch.py > 88 && touch.py < 144 && touch.px > 96 && touch.px < 160 && !titleboxXmoveleft && !titleboxXmoveright && showSTARTborder)
			|| ((pressed & KEY_TOUCH) && touch.py > 170 && theme == 1 && !titleboxXmoveleft && !titleboxXmoveright && showSTARTborder))											// START button/text (3DS theme)
			{
				if ((startMenu_cursorPosition == 0 && startMenu)
				|| (startMenu_cursorPosition == 1 && startMenu)
				|| (startMenu_cursorPosition == 2 && startMenu))
				{
					mmEffectEx(&snd_launch);
					controlTopBright = true;
					applaunch = true;
					applaunchprep = true;
					if (startMenu_cursorPosition == 0) gotosettings = true;
					useBootstrap = false;

					if (theme == 0) {
						showbubble = false;
						showSTARTborder = false;
						clearText(false);	// Clear title

						fadeSpeed = false;	// Slow fade speed
					}
					fadeType = false;	// Fade to white
					for (int i = 0; i < 60; i++) {
						swiWaitForVBlank();
					}
					music = false;
					mmEffectCancelAll();

					clearText(true);
					if (startMenu_cursorPosition == 1) launchType = 0;
					if (startMenu_cursorPosition == 2) homebrewBootstrap = true;
					SaveSettings();

					if (startMenu_cursorPosition == 0) {
						// Launch settings
						int err = runNdsFile ("/_nds/dsimenuplusplus/main.srldr", 0, NULL, false);
						iprintf ("Start failed. Error %i\n", err);
					} else if (startMenu_cursorPosition == 1) {
						if (!flashcardUsed) {
							if (!slot1LaunchMethod || arm7SCFGLocked) {
								dsCardLaunch();
							} else {
								int err = runNdsFile ("/_nds/dsimenuplusplus/slot1launch.srldr", 0, NULL, true);
								iprintf ("Start failed. Error %i\n", err);
							}
						} else {
							// Switch to GBA mode
							useBootstrap = true;
							if (useGbarunner) {
								loadGameOnFlashcard("fat:/_nds/GBARunner2_fc.nds");
							} else {
								gbaSwitch();
							}
						}
					} else if (startMenu_cursorPosition == 2) {
						// Switch to GBA mode
						useBootstrap = true;
						CIniFile bootstrapini( "sd:/_nds/nds-bootstrap.ini" );
						bootstrapini.SetString("NDS-BOOTSTRAP", "NDS_PATH", "sd:/_nds/GBARunner2.nds");
						bootstrapini.SaveIniFile( "sd:/_nds/nds-bootstrap.ini" );
						int err = 0;
						if (bootstrapFile) err = runNdsFile ("sd:/_nds/nds-bootstrap-hb-nightly.nds", 0, NULL, true);
						else err = runNdsFile ("sd:/_nds/nds-bootstrap-hb-release.nds", 0, NULL, true);
						iprintf ("Start failed. Error %i\n", err);
					}
				}

				DirEntry* entry = &dirContents[scrn].at(cursorPosition+pagenum*40);
				if (entry->isDirectory)
				{
					// Enter selected directory
					mmEffectEx(&snd_select);
					fadeType = false;	// Fade to white
					for (int i = 0; i < 30; i++) swiWaitForVBlank();
					pagenum = 0;
					cursorPosition = 0;
					titleboxXpos = 0;
					titlewindowXpos = 0;
					whiteScreen = true;
					if (showBoxArt) clearBoxArt();	// Clear box art
					boxArtLoaded = false;
					redoDropDown = true;
					shouldersRendered = false;
					showbubble = false;
					showSTARTborder = false;
					clearText();
					chdir(entry->name.c_str());
					char buf[256];
					romfolder = getcwd(buf, 256);
					SaveSettings();
					settingsChanged = false;
					return "null";
				}
				else if (isDSiWare[cursorPosition])
				{
					if (flashcardUsed || consoleModel > 1) {
						mmEffectEx(&snd_wrong);
						clearText();
						showdialogbox = true;
						for (int i = 0; i < 30; i++) swiWaitForVBlank();
						titleUpdate(dirContents[scrn].at(cursorPosition+pagenum*40).isDirectory, dirContents[scrn].at(cursorPosition+pagenum*40).name.c_str(), cursorPosition);
						printSmallCentered(false, 112, "This game cannot be launched");
						if (flashcardUsed) {
							printSmallCentered(false, 128, "in DS mode.");
						} else {
							printSmallCentered(false, 128, "as a .nds file on 3DS/2DS.");
						}
						printSmall(false, 208, 166, "A: OK");
						pressed = 0;
						do {
							scanKeys();
							pressed = keysDownRepeat();
							swiWaitForVBlank();
						} while (!(pressed & KEY_A));
						clearText();
						showdialogbox = false;
						for (int i = 0; i < 15; i++) swiWaitForVBlank();
					} else {
						mmEffectEx(&snd_launch);
						controlTopBright = true;
						applaunch = true;
						applaunchprep = true;
						useBootstrap = true;

						if (theme == 0) {
							showbubble = false;
							showSTARTborder = false;
							clearText(false);	// Clear title

							fadeSpeed = false;	// Slow fade speed
							for (int i = 0; i < 5; i++) {
								swiWaitForVBlank();
							}
						}
						fadeType = false;	// Fade to white
						for (int i = 0; i < 60; i++) {
							swiWaitForVBlank();
						}
						music = false;
						mmEffectCancelAll();

						clearText(true);

						// Return the chosen file
						return entry->name;
					}
				}
				else
				{
					mmEffectEx(&snd_launch);
					controlTopBright = true;
					applaunch = true;
					applaunchprep = true;
					if (!isHomebrew[cursorPosition]) {
						useBootstrap = true;
					} else {
						useBootstrap = false;
					}

					if (theme == 0) {
						showbubble = false;
						showSTARTborder = false;
						clearText(false);	// Clear title

						fadeSpeed = false;	// Slow fade speed
						for (int i = 0; i < 5; i++) {
							swiWaitForVBlank();
						}
					}
					fadeType = false;	// Fade to white
					for (int i = 0; i < 60; i++) {
						swiWaitForVBlank();
					}
					music = false;
					mmEffectCancelAll();

					clearText(true);
					if (bnrRomType[cursorPosition] == 0) {
						FILE *f_nds_file = fopen(dirContents[scrn].at(cursorPosition+pagenum*40).name.c_str(), "rb");

						char game_TID[5];
						grabTID(f_nds_file, game_TID);
						game_TID[4] = 0;
						game_TID[3] = 0;
						if(strcmp(game_TID, "###") == 0) homebrewBootstrap = true;
						fclose(f_nds_file);
					}

					// Return the chosen file
					return entry->name;
				}
			}

			if ((pressed & KEY_Y) && !startMenu && bnrRomType[cursorPosition] == 0 && !titleboxXmoveleft && !titleboxXmoveright && showSTARTborder && cursorPosition >= 0)
			{
				DirEntry* entry = &dirContents[scrn].at(cursorPosition+pagenum*40);
				if (entry->isDirectory)
				{
					// Enter selected directory
					/* chdir(entry->name.c_str());
					updatePath();
					pane->slideTransition(false, false, 0, cursorPosition - screenOffset);
					pane = &createTextPane(20, 3 + ENTRIES_START_ROW*FONT_SY, ENTRIES_PER_SCREEN);
					getDirectoryContents(dirContents[++scrn], extensionList);
					for (auto &i : dirContents[scrn])
						pane->addLine(i.visibleName.c_str());
					pane->createDefaultEntries();
					pane->slideTransition(true, false, 20);
					screenOffset = 0;
					cursorPosition = 0; */
				}
				else
				{
					mmEffectEx(&snd_launch);
					controlTopBright = true;
					applaunch = true;
					applaunchprep = true;
					useBootstrap = false;

					if (theme == 0) {
						showbubble = false;
						showSTARTborder = false;
						clearText(false);	// Clear title

						fadeSpeed = false;	// Slow fade speed
						for (int i = 0; i < 5; i++) {
							swiWaitForVBlank();
						}
					}
					fadeType = false;	// Fade to white
					for (int i = 0; i < 60; i++) {
						swiWaitForVBlank();
					}
					music = false;
					mmEffectCancelAll();

					clearText(true);
					SaveSettings();

					// Return the chosen file
					return entry->name;
				}
			}

			if (pressed & KEY_L)
			{
				if (!startMenu && !titleboxXmoveleft && !titleboxXmoveright && pagenum != 0) {
					mmEffectEx(&snd_switch);
					fadeType = false;	// Fade to white
					for (int i = 0; i < 30; i++) swiWaitForVBlank();
					pagenum -= 1;
					cursorPosition = 0;
					titleboxXpos = 0;
					titlewindowXpos = 0;
					whiteScreen = true;
					if (showBoxArt) clearBoxArt();	// Clear box art
					boxArtLoaded = false;
					redoDropDown = true;
					shouldersRendered = false;
					showbubble = false;
					showSTARTborder = false;
					clearText();
					SaveSettings();
					settingsChanged = false;
					displayNowLoading();
					break;
				} else {
					mmEffectEx(&snd_wrong);
				}
			} else 	if (pressed & KEY_R)
			{
				if (!startMenu && !titleboxXmoveleft && !titleboxXmoveright && file_count > 40+pagenum*40) {
					mmEffectEx(&snd_switch);
					fadeType = false;	// Fade to white
					for (int i = 0; i < 30; i++) swiWaitForVBlank();
					pagenum += 1;
					cursorPosition = 0;
					titleboxXpos = 0;
					titlewindowXpos = 0;
					whiteScreen = true;
					if (showBoxArt) clearBoxArt();	// Clear box art
					boxArtLoaded = false;
					redoDropDown = true;
					shouldersRendered = false;
					showbubble = false;
					showSTARTborder = false;
					clearText();
					SaveSettings();
					settingsChanged = false;
					displayNowLoading();
					break;
				} else {
					mmEffectEx(&snd_wrong);
				}
			}

			/*if (((pressed & KEY_UP) || (pressed & KEY_DOWN))
			&& !startMenu && !titleboxXmoveleft && !titleboxXmoveright && !flashcardUsed && consoleModel < 2)
			{
				mmEffectEx(&snd_switch);
				fadeType = false;	// Fade to white
				for (int i = 0; i < 30; i++) swiWaitForVBlank();
				dsiWareList = !dsiWareList;
				whiteScreen = true;
				if (showBoxArt) clearBoxArt();	// Clear box art
				boxArtLoaded = false;
				showbubble = false;
				showSTARTborder = false;
				clearText();
				SaveSettings();
				settingsChanged = false;
				return "null";		
			}*/

			if ((pressed & KEY_B)) {
				if (startMenu) {
					mmEffectEx(&snd_back);
					fadeType = false;	// Fade to white
					for (int i = 0; i < 25; i++) swiWaitForVBlank();
					startMenu = false;
					if (settingsChanged) {
						SaveSettings();
						settingsChanged = false;
					}
					whiteScreen = true;
					redoDropDown = true;
					shouldersRendered = false;
					showbubble = false;
					showSTARTborder = false;
					clearText();
					whiteScreen = false;
					fadeType = true;	// Fade in from white
					for (int i = 0; i < 5; i++) swiWaitForVBlank();
					waitForFadeOut();
				} else if (showDirectories) {
					// Go up a directory
					mmEffectEx(&snd_select);
					fadeType = false;	// Fade to white
					for (int i = 0; i < 30; i++) swiWaitForVBlank();
					pagenum = 0;
					cursorPosition = 0;
					titleboxXpos = 0;
					titlewindowXpos = 0;
					whiteScreen = true;
					if (showBoxArt) clearBoxArt();	// Clear box art
					boxArtLoaded = false;
					redoDropDown = true;
					shouldersRendered = false;
					showbubble = false;
					showSTARTborder = false;
					clearText();
					chdir("..");
					char buf[256];
					romfolder = getcwd(buf, 256);
					SaveSettings();
					settingsChanged = false;
					return "null";
				}
			}

			if ((pressed & KEY_X) && !startMenu && showSTARTborder
			&& strcmp(dirContents[scrn].at(cursorPosition+pagenum*40).name.c_str(), "..") != 0)
			{
				clearText();
				showdialogbox = true;
				for (int i = 0; i < 30; i++) swiWaitForVBlank();
				snprintf (fileCounter, sizeof(fileCounter), "%i/%i", (cursorPosition+1)+pagenum*40, file_count);
				titleUpdate(dirContents[scrn].at(cursorPosition+pagenum*40).isDirectory, dirContents[scrn].at(cursorPosition+pagenum*40).name.c_str(), cursorPosition);
				printSmall(false, 16, 64, dirContents[scrn].at(cursorPosition+pagenum*40).name.c_str());
				printSmall(false, 16, 166, fileCounter);
				printSmallCentered(false, 112, "Are you sure you want to");
				if (isDirectory[cursorPosition]) {
					printSmallCentered(false, 128, "delete this folder?");
				} else {
					printSmallCentered(false, 128, "delete this game?");
				}
				for (int i = 0; i < 90; i++) swiWaitForVBlank();
				printSmall(false, 160, 166, "A: Yes");
				printSmall(false, 208, 166, "B: No");
				while (1) {
					do {
						scanKeys();
						pressed = keysDownRepeat();
						swiWaitForVBlank();
					} while (!pressed);
					
					if (pressed & KEY_A) {
						fadeType = false;	// Fade to white
						for (int i = 0; i < 30; i++) swiWaitForVBlank();
						whiteScreen = true;
						remove(dirContents[scrn].at(cursorPosition+pagenum*40).name.c_str()); // Remove game/folder
						if (showBoxArt) clearBoxArt();	// Clear box art
						boxArtLoaded = false;
						shouldersRendered = false;
						showbubble = false;
						showSTARTborder = false;
						clearText();
						showdialogbox = false;
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
				for (int i = 0; i < 15; i++) swiWaitForVBlank();
			}

			if ((pressed & KEY_X) && startMenu && !flashcardUsed) {
				mmEffectEx(&snd_back);
				fadeType = false;	// Fade to white
				for (int i = 0; i < 25; i++) swiWaitForVBlank();
				music = false;
				mmEffectCancelAll();
				if (settingsChanged) {
					SaveSettings();
					settingsChanged = false;
				}
				*(u32*)(0x02000300) = 0x434E4C54;	// Set "CNLT" warmboot flag
				*(u16*)(0x02000304) = 0x1801;
				*(u32*)(0x02000310) = 0x4D454E55;	// "MENU"
				fifoSendValue32(FIFO_USER_02, 1);	// ReturntoDSiMenu
			}

			int pressedForStartMenu = 0;
			if (startButtonLaunch) {
				pressedForStartMenu = (pressed & KEY_SELECT);
			} else {
				pressedForStartMenu = (pressed & KEY_START);
			}

			if (pressedForStartMenu)
			{
				mmEffectEx(&snd_switch);
				fadeType = false;	// Fade to white
				for (int i = 0; i < 30; i++) swiWaitForVBlank();
				if (showBoxArt && !startMenu) clearBoxArt();	// Clear box art
				startMenu = !startMenu;
				if (settingsChanged) {
					SaveSettings();
					settingsChanged = false;
				}
				whiteScreen = true;
				boxArtLoaded = false;
				redoDropDown = true;
				shouldersRendered = false;
				showbubble = false;
				showSTARTborder = false;
				clearText();
				whiteScreen = false;
				fadeType = true;	// Fade in from white
				for (int i = 0; i < 5; i++) swiWaitForVBlank();
				waitForFadeOut();
			}

			int pressedForPerGameSettings = 0;
			if (startButtonLaunch) {
				pressedForPerGameSettings = (pressed & KEY_A);
			} else {
				pressedForPerGameSettings = (pressed & KEY_SELECT);
			}

			if (pressedForPerGameSettings && !startMenu
			&& (isDirectory[cursorPosition] == false) && (bnrRomType[cursorPosition] == 0) && (isHomebrew[cursorPosition] == false)
			&& !titleboxXmoveleft && !titleboxXmoveright && showSTARTborder)
			{
				perGameSettings(dirContents[scrn].at(cursorPosition+pagenum*40).name);
			}

		}
	}
}
