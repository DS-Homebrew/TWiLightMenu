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
#define ENTRIES_PER_SCREEN 15
#define ENTRIES_START_ROW 3
#define ENTRY_PAGE_LENGTH 10

const char* SDKnumbertext;

extern bool whiteScreen;
extern bool fadeType;
extern bool fadeSpeed;

extern bool homebrewBootstrap;
extern bool useGbarunner;
extern int consoleModel;
extern bool isRegularDS;

extern bool showbubble;
extern bool showSTARTborder;

extern bool titleboxXmoveleft;
extern bool titleboxXmoveright;

extern bool applaunchprep;

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
static int file_count = 0;
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
extern void attemptReboot();
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

bool music = false;

extern char usernameRendered[10];
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

			if (showDirectories && !dsiWareList) {
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

string browseForFile(const vector<string> extensionList, const char* username)
{
	int pressed = 0;
	SwitchState scrn(3);
	vector<DirEntry> dirContents[scrn.SIZE];

	getDirectoryContents(dirContents[scrn], extensionList);

	spawnedtitleboxes = 0;
	for(int i = 0; i < 40; i++) {
		if (dsiWareList) {
			if (i+dsiWarePageNum*40 < file_count) {
				isDirectory[i] = false;
				std::string std_romsel_filename = dirContents[scrn].at(i+dsiWarePageNum*40).name.c_str();
				if((std_romsel_filename.substr(std_romsel_filename.find_last_of(".") + 1) == "app")
				|| (std_romsel_filename.substr(std_romsel_filename.find_last_of(".") + 1) == "argv"))
				{
					getGameInfo(dirContents[scrn].at(i+dsiWarePageNum*40).isDirectory, dirContents[scrn].at(i+dsiWarePageNum*40).name.c_str(), i);
					bnrRomType[i] = 0;
				}
				spawnedtitleboxes++;
			}
		} else {
			if (i+pagenum*40 < file_count) {
				if (dirContents[scrn].at(i+pagenum*40).isDirectory) {
					isDirectory[i] = true;
					bnrWirelessIcon[i] = 0;
				} else {
					isDirectory[i] = false;
					std::string std_romsel_filename = dirContents[scrn].at(i+pagenum*40).name.c_str();
					if((std_romsel_filename.substr(std_romsel_filename.find_last_of(".") + 1) == "nds")
					|| (std_romsel_filename.substr(std_romsel_filename.find_last_of(".") + 1) == "app")
					|| (std_romsel_filename.substr(std_romsel_filename.find_last_of(".") + 1) == "argv"))
					{
						getGameInfo(dirContents[scrn].at(i+pagenum*40).isDirectory, dirContents[scrn].at(i+pagenum*40).name.c_str(), i);
						bnrRomType[i] = 0;
					} else if((std_romsel_filename.substr(std_romsel_filename.find_last_of(".") + 1) == "gb")
							|| std_romsel_filename.substr(std_romsel_filename.find_last_of(".") + 1) == "sgb")
					{
						bnrRomType[i] = 1;
						bnrWirelessIcon[i] = 0;
						launchable[i] = true;
						isHomebrew[i] = false;
					} else if(std_romsel_filename.substr(std_romsel_filename.find_last_of(".") + 1) == "gbc") {
						bnrRomType[i] = 2;
						bnrWirelessIcon[i] = 0;
						launchable[i] = true;
						isHomebrew[i] = false;
					} else if((std_romsel_filename.substr(std_romsel_filename.find_last_of(".") + 1) == "nes")
							|| std_romsel_filename.substr(std_romsel_filename.find_last_of(".") + 1) == "fds")
					{
						bnrRomType[i] = 3;
						bnrWirelessIcon[i] = 0;
						launchable[i] = true;
						isHomebrew[i] = false;
					}
				}
				spawnedtitleboxes++;
			}
		}
	}
	// Load correct icons depending on cursor position
	if (dsiWareList) {
		if (dsiWare_cursorPosition <= 1) {
			for(int i = 0; i < 5; i++) {
				if (bnrRomType[i] == 0 && i+dsiWarePageNum*40 < file_count) {
					iconUpdate(dirContents[scrn].at(i+dsiWarePageNum*40).isDirectory, dirContents[scrn].at(i+dsiWarePageNum*40).name.c_str(), i);
				}
			}
		} else if (dsiWare_cursorPosition >= 2 && dsiWare_cursorPosition <= 36) {
			for(int i = 0; i < 6; i++) {
				if (bnrRomType[i] == 0 && (dsiWare_cursorPosition-2+i)+dsiWarePageNum*40 < file_count) {
					iconUpdate(dirContents[scrn].at((dsiWare_cursorPosition-2+i)+dsiWarePageNum*40).isDirectory, dirContents[scrn].at((dsiWare_cursorPosition-2+i)+dsiWarePageNum*40).name.c_str(), dsiWare_cursorPosition-2+i);
				}
			}
		} else if (dsiWare_cursorPosition >= 37 && dsiWare_cursorPosition <= 39) {
			for(int i = 0; i < 5; i++) {
				if (bnrRomType[i] == 0 && (35+i)+dsiWarePageNum*40 < file_count) {
					iconUpdate(dirContents[scrn].at((35+i)+dsiWarePageNum*40).isDirectory, dirContents[scrn].at((35+i)+dsiWarePageNum*40).name.c_str(), 35+i);
				}
			}
		}
	} else {
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
	}

	if (!music) {
		mmEffectEx(&mus_menu);
		music = true;
	}
	whiteScreen = false;
	fadeType = true;	// Fade in from white
	for (int i = 0; i < 30; i++) swiWaitForVBlank();
	
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

	while (true)
	{
		// cursor->finalY = 4 + 10 * (cursorPosition - screenOffset + ENTRIES_START_ROW);
		// cursor->delay = TextEntry::ACTIVE;

		if (startMenu) {
			if (startMenu_cursorPosition < (3-flashcardUsed)) {
				showbubble = true;
				showSTARTborder = true;
				titleUpdate(false, "startMenu");
			} else {
				showbubble = false;
				showSTARTborder = false;
				clearText(false);	// Clear title
			}
		} else if (dsiWareList) {
			if (dsiWare_cursorPosition+dsiWarePageNum*40 > ((int) dirContents[scrn].size() - 1)) {
				showbubble = false;
				showSTARTborder = false;
				clearText(false);	// Clear title
			} else {
				showbubble = true;
				showSTARTborder = true;
				titleUpdate(dirContents[scrn].at(dsiWare_cursorPosition+dsiWarePageNum*40).isDirectory, dirContents[scrn].at(dsiWare_cursorPosition+dsiWarePageNum*40).name.c_str());
			}
		} else {
			if (cursorPosition+pagenum*40 > ((int) dirContents[scrn].size() - 1)) {
				showbubble = false;
				showSTARTborder = false;
				clearText(false);	// Clear title
			} else {
				showbubble = true;
				showSTARTborder = true;
				titleUpdate(dirContents[scrn].at(cursorPosition+pagenum*40).isDirectory, dirContents[scrn].at(cursorPosition+pagenum*40).name.c_str());
			}
		}

		// Power saving loop. Only poll the keys once per frame and sleep the CPU if there is nothing else to do
		do
		{
			consoleClear();
			if(!usernameRenderedDone) {
				for (int i = 0; i < 10; i++) {
					if (username[i] == 0)
						usernameRendered[i] = 0x20;
					else
						usernameRendered[i] = username[i];
				}
				usernameRenderedDone = true;
			}
			iprintf("\n   %s           %s", usernameRendered, RetTime().c_str());
			if (startMenu) {
			} else if (dsiWareList) {
				for(int i = 0; i < 21; i++) {
					printf("\n");
				}
				printf(" ");
				if (dsiWarePageNum != 0) {
					printf("L: Previous");
				} else {
					printf("           ");
				}
				printf("            ");
				if (file_count > 40+dsiWarePageNum*40) {
					printf("R: Next");
				} else {
					printf("       ");
				}
			} else {
				for(int i = 0; i < 21; i++) {
					printf("\n");
				}
				printf(" ");
				if (pagenum != 0) {
					printf("L: Previous");
				} else {
					printf("           ");
				}
				printf("            ");
				if (file_count > 40+pagenum*40) {
					printf("R: Next");
				} else {
					printf("       ");
				}

				//if (pagenum != 0) printSmall(true, 16, 177, "Prev. Page");
				//if (file_count > 40+pagenum*40) printSmall(true, 182, 177, "Next Page");
			}

			scanKeys();
			pressed = keysDownRepeat();
			swiWaitForVBlank();
		}
		while (!pressed);
		// if (cursor->fade == TextEntry::FadeType::IN)
		// {
		// 	cursor->fade = TextEntry::FadeType::NONE;
		// 	cursor->invAccel = 4;
		// }

		if ((pressed & KEY_LEFT) && !titleboxXmoveleft && !titleboxXmoveright) {
			if (startMenu) {
				startMenu_cursorPosition -= 1;
				if (startMenu_cursorPosition >= 0) {
					titleboxXmoveleft = true;
					mmEffectEx(&snd_select);
					settingsChanged = true;
				} else {
					mmEffectEx(&snd_wrong);
				}
			} else if (dsiWareList) {
				dsiWare_cursorPosition -= 1;
				if (dsiWare_cursorPosition >= 0) {
					titleboxXmoveleft = true;
					mmEffectEx(&snd_select);
					settingsChanged = true;
				} else {
					mmEffectEx(&snd_wrong);
				}
				if(dsiWare_cursorPosition >= 2 && dsiWare_cursorPosition <= 36) {
					if (bnrRomType[dsiWare_cursorPosition-2] == 0 && (dsiWare_cursorPosition-2)+dsiWarePageNum*40 < file_count) {
						iconUpdate(dirContents[scrn].at((dsiWare_cursorPosition-2)+dsiWarePageNum*40).isDirectory, dirContents[scrn].at((dsiWare_cursorPosition-2)+dsiWarePageNum*40).name.c_str(), dsiWare_cursorPosition-2);
					}
				}
			} else {
				cursorPosition -= 1;
				if (cursorPosition >= 0) {
					titleboxXmoveleft = true;
					mmEffectEx(&snd_select);
					settingsChanged = true;
				} else {
					mmEffectEx(&snd_wrong);
				}
				if(cursorPosition >= 2 && cursorPosition <= 36) {
					if (bnrRomType[cursorPosition-2] == 0 && (cursorPosition-2)+pagenum*40 < file_count) {
						iconUpdate(dirContents[scrn].at((cursorPosition-2)+pagenum*40).isDirectory, dirContents[scrn].at((cursorPosition-2)+pagenum*40).name.c_str(), cursorPosition-2);
					}
				}
			}
		} else if ((pressed & KEY_RIGHT) && !titleboxXmoveleft && !titleboxXmoveright) {
			if (startMenu) {
				startMenu_cursorPosition += 1;
				if (startMenu_cursorPosition <= 39) {
					titleboxXmoveright = true;
					mmEffectEx(&snd_select);
					settingsChanged = true;
				} else {
					mmEffectEx(&snd_wrong);
				}
			} else if (dsiWareList) {
				dsiWare_cursorPosition += 1;
				if (dsiWare_cursorPosition <= 39) {
					titleboxXmoveright = true;
					mmEffectEx(&snd_select);
					settingsChanged = true;
				} else {
					mmEffectEx(&snd_wrong);
				}
				if(dsiWare_cursorPosition >= 3 && dsiWare_cursorPosition <= 37) {
					if (bnrRomType[dsiWare_cursorPosition+2] == 0 && (dsiWare_cursorPosition+2)+dsiWarePageNum*40 < file_count) {
						iconUpdate(dirContents[scrn].at((dsiWare_cursorPosition+2)+dsiWarePageNum*40).isDirectory, dirContents[scrn].at((dsiWare_cursorPosition+2)+dsiWarePageNum*40).name.c_str(), dsiWare_cursorPosition+2);
					}
				}
			} else {
				cursorPosition += 1;
				if (cursorPosition <= 39) {
					titleboxXmoveright = true;
					mmEffectEx(&snd_select);
					settingsChanged = true;
				} else {
					mmEffectEx(&snd_wrong);
				}
				if(cursorPosition >= 3 && cursorPosition <= 37) {
					if (bnrRomType[cursorPosition+2] == 0 && (cursorPosition+2)+pagenum*40 < file_count) {
						iconUpdate(dirContents[scrn].at((cursorPosition+2)+pagenum*40).isDirectory, dirContents[scrn].at((cursorPosition+2)+pagenum*40).name.c_str(), cursorPosition+2);
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
		} else if (dsiWareList) {
			if (dsiWare_cursorPosition < 0)
			{
				dsiWare_cursorPosition = 0;
			}
			else if (dsiWare_cursorPosition > 39)
			{
				dsiWare_cursorPosition = 39;
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
		// else if (cursorPosition > ((int) dirContents[scrn].size() - 1))
		// {
		// 	cursorPosition = dirContents[scrn].size() - 1;
		// }
		
		if ((pressed & KEY_A) && !titleboxXmoveleft && !titleboxXmoveright && showSTARTborder)
		{
			if ((startMenu_cursorPosition == 0 && startMenu)
			|| (startMenu_cursorPosition == 1 && startMenu)
			|| (startMenu_cursorPosition == 2 && startMenu))
			{
				mmEffectEx(&snd_launch);
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
				if (startMenu_cursorPosition == 2) homebrewBootstrap = true;
				SaveSettings();

				if (startMenu_cursorPosition == 0) {
					// Launch settings
					int err = runNdsFile ("/_nds/dsimenuplusplus/main.srldr", 0, NULL, false);
					iprintf ("Start failed. Error %i\n", err);
				} else if (startMenu_cursorPosition == 1) {
					if (!flashcardUsed) {
						dsCardLaunch();
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
					attemptReboot();
					int err = runNdsFile ("sd:/_nds/hb-bootstrap.nds", 0, NULL, true);
					iprintf ("Start failed. Error %i\n", err);
				}
			}

			DirEntry* entry;
			if (dsiWareList) {
				entry = &dirContents[scrn].at(dsiWare_cursorPosition+dsiWarePageNum*40);
			} else {
				entry = &dirContents[scrn].at(cursorPosition+pagenum*40);
			}
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
			else if (dsiWareList || launchable[cursorPosition])
			{
				mmEffectEx(&snd_launch);
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
				}
				fadeType = false;	// Fade to white
				for (int i = 0; i < 60; i++) {
					swiWaitForVBlank();
				}
				music = false;
				mmEffectCancelAll();

				clearText(true);
				if (!dsiWareList) {
					FILE *f_nds_file = fopen(dirContents[scrn].at(cursorPosition+pagenum*40).name.c_str(), "rb");

					char game_TID[5];
					grabTID(f_nds_file, game_TID);
					game_TID[4] = 0;
					game_TID[3] = 0;
					if(strcmp(game_TID, "###") == 0) homebrewBootstrap = true;
					fclose(f_nds_file);
				}
				SaveSettings();

				// Return the chosen file
				return entry->name;
			} else {
				mmEffectEx(&snd_wrong);
				clearText();
				showdialogbox = true;
				for (int i = 0; i < 30; i++) swiWaitForVBlank();
				printSmallCentered(false, 88, "This game cannot be launched.");
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
			}
		}

		if ((pressed & KEY_Y) && !startMenu && !dsiWareList && bnrRomType[cursorPosition] == 0 && !titleboxXmoveleft && !titleboxXmoveright && showSTARTborder && cursorPosition >= 0)
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
				applaunch = true;
				applaunchprep = true;
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
				SaveSettings();

				// Return the chosen file
				return entry->name;
			}
		}

		if ((pressed & KEY_L) && !startMenu && !titleboxXmoveleft && !titleboxXmoveright)
		{
			mmEffectEx(&snd_switch);
			fadeType = false;	// Fade to white
			for (int i = 0; i < 30; i++) swiWaitForVBlank();
			if (dsiWareList) {
				dsiWarePageNum -= 1;
				if (dsiWarePageNum < 0) dsiWarePageNum = 0;
				dsiWare_cursorPosition = 0;
				dsiWare_titleboxXpos = 0;
				dsiWare_titlewindowXpos = 0;
			} else {
				pagenum -= 1;
				if (pagenum < 0) pagenum = 0;
				cursorPosition = 0;
				titleboxXpos = 0;
				titlewindowXpos = 0;
			}
			whiteScreen = true;
			showbubble = false;
			showSTARTborder = false;
			clearText();
			SaveSettings();
			settingsChanged = false;
			return "null";		
		} else 	if ((pressed & KEY_R) && !startMenu && !titleboxXmoveleft && !titleboxXmoveright && file_count > 40+pagenum*40)
		{
			mmEffectEx(&snd_switch);
			fadeType = false;	// Fade to white
			for (int i = 0; i < 30; i++) swiWaitForVBlank();
			if (dsiWareList) {
				dsiWarePageNum += 1;
				dsiWare_cursorPosition = 0;
				dsiWare_titleboxXpos = 0;
				dsiWare_titlewindowXpos = 0;
			} else {
				pagenum += 1;
				cursorPosition = 0;
				titleboxXpos = 0;
				titlewindowXpos = 0;
			}
			whiteScreen = true;
			showbubble = false;
			showSTARTborder = false;
			clearText();
			SaveSettings();
			settingsChanged = false;
			return "null";		
		}

		if (((pressed & KEY_UP) || (pressed & KEY_DOWN))
		&& !startMenu && !titleboxXmoveleft && !titleboxXmoveright && !flashcardUsed && consoleModel < 2)
		{
			mmEffectEx(&snd_switch);
			fadeType = false;	// Fade to white
			for (int i = 0; i < 30; i++) swiWaitForVBlank();
			dsiWareList = !dsiWareList;
			whiteScreen = true;
			showbubble = false;
			showSTARTborder = false;
			clearText();
			SaveSettings();
			settingsChanged = false;
			return "null";		
		}

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
				clearText();
				whiteScreen = false;
				fadeType = true;	// Fade in from white
				for (int i = 0; i < 30; i++) swiWaitForVBlank();
			} else if (showDirectories && !dsiWareList) {
				// Go up a directory
				mmEffectEx(&snd_select);
				fadeType = false;	// Fade to white
				for (int i = 0; i < 30; i++) swiWaitForVBlank();
				pagenum = 0;
				cursorPosition = 0;
				titleboxXpos = 0;
				titlewindowXpos = 0;
				whiteScreen = true;
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

		if (pressed & KEY_START)
		{
			mmEffectEx(&snd_switch);
			fadeType = false;	// Fade to white
			for (int i = 0; i < 30; i++) swiWaitForVBlank();
			startMenu = !startMenu;
			if (settingsChanged) {
				SaveSettings();
				settingsChanged = false;
			}
			whiteScreen = true;
			showbubble = false;
			showSTARTborder = false;
			clearText();
			whiteScreen = false;
			fadeType = true;	// Fade in from white
			for (int i = 0; i < 30; i++) swiWaitForVBlank();
		}

		if ((pressed & KEY_SELECT) && !startMenu
		&& (isDirectory[cursorPosition] == false) && (bnrRomType[cursorPosition] == 0) && (isHomebrew[cursorPosition] == false)
		&& !titleboxXmoveleft && !titleboxXmoveright && showSTARTborder && !dsiWareList)
		{
			clearText();
			showdialogbox = true;

			FILE *f_nds_file = fopen(dirContents[scrn].at(cursorPosition+pagenum*40).name.c_str(), "rb");

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
			for (int i = 0; i < 30; i++) swiWaitForVBlank();
			printSmallCentered(false, 88, SDKnumbertext);
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
			
		}

	}
}
