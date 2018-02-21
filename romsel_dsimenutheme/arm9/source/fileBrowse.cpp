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

#include "ndsLoaderArm9.h"

#include "inifile.h"

#include "soundbank.h"
#include "soundbank_bin.h"

#define SCREEN_COLS 32
#define ENTRIES_PER_SCREEN 15
#define ENTRIES_START_ROW 3
#define ENTRY_PAGE_LENGTH 10

extern bool whiteScreen;

extern bool isRegularDS;

extern bool showbubble;
extern bool showSTARTborder;

extern bool titleboxXmoveleft;
extern bool titleboxXmoveright;

extern bool applaunchprep;

extern std::string romfolder;

extern std::string arm7DonorPath;
bool donorFound = true;

extern int romtype;

extern bool applaunch;

extern bool gotosettings;

extern bool useBootstrap;

using namespace std;

extern int theme;

extern int spawnedtitleboxes;
static int file_count = 0;
extern int cursorPosition;
extern int pagenum;
extern int titleboxXpos;
extern int titlewindowXpos;

extern bool flashcardUsed;

extern void SaveSettings();

extern void gbaMode();

mm_sound_effect snd_launch;
mm_sound_effect snd_select;
mm_sound_effect snd_stop;
mm_sound_effect snd_wrong;
mm_sound_effect snd_back;
mm_sound_effect snd_switch;

void InitSound() {
	mmInitDefaultMem((mm_addr)soundbank_bin);
	
	mmLoadEffect( SFX_LAUNCH );
	mmLoadEffect( SFX_SELECT );
	mmLoadEffect( SFX_STOP );
	mmLoadEffect( SFX_BACK );
	mmLoadEffect( SFX_SWITCH );

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
}

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

			// if (dirEntry.name.compare(".") != 0 && (dirEntry.isDirectory || nameEndsWith(dirEntry.name, extensionList)))
			if (dirEntry.name.compare(".") != 0 && (nameEndsWith(dirEntry.name, extensionList))) {
				dirContents.push_back(dirEntry);
				file_count++;
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
		if (i+pagenum*40 < file_count) {
			if (romtype == 0) {
				getGameInfo(dirContents[scrn].at(i+pagenum*40).isDirectory, dirContents[scrn].at(i+pagenum*40).name.c_str(), i);
			} else if (romtype == 1) {
				std::string std_romsel_filename = dirContents[scrn].at(i+pagenum*40).name.c_str();
				if(std_romsel_filename.substr(std_romsel_filename.find_last_of(".") + 1) == "gbc") {
					isGBC[i] = true;
				} else {
					isGBC[i] = false;
				}
				launchable[i] = true;
				isHomebrew[i] = false;
			} else {
				launchable[i] = true;
				isHomebrew[i] = false;
			}
			spawnedtitleboxes++;
		}
	}
	if (romtype == 0) {
		// Load correct icons depending on cursor position
		if (cursorPosition <= 1) {
			for(int i = 0; i < 5; i++) {
				if (i+pagenum*40 < file_count) {
					iconUpdate(dirContents[scrn].at(i+pagenum*40).isDirectory, dirContents[scrn].at(i+pagenum*40).name.c_str(), i);
				}
			}
		} else if (cursorPosition >= 2 && cursorPosition <= 36) {
			for(int i = 0; i < 6; i++) {
				if ((cursorPosition-2+i)+pagenum*40 < file_count) {
					iconUpdate(dirContents[scrn].at((cursorPosition-2+i)+pagenum*40).isDirectory, dirContents[scrn].at((cursorPosition-2+i)+pagenum*40).name.c_str(), cursorPosition-2+i);
				}
			}
		} else if (cursorPosition >= 37 && cursorPosition <= 39) {
			for(int i = 0; i < 5; i++) {
				if ((35+i)+pagenum*40 < file_count) {
					iconUpdate(dirContents[scrn].at((35+i)+pagenum*40).isDirectory, dirContents[scrn].at((35+i)+pagenum*40).name.c_str(), 35+i);
				}
			}
		}
	}
	
	whiteScreen = false;
	
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

		if (cursorPosition+pagenum*40 > ((int) dirContents[scrn].size() - 1)) {
			showbubble = false;
			showSTARTborder = false;
			clearText(false);	// Clear title
		} else if (cursorPosition == -2) {
			showbubble = true;
			showSTARTborder = true;
			titleUpdate(false, "settings");
		} else if (cursorPosition == -1) {
			showbubble = true;
			showSTARTborder = true;
			titleUpdate(false, "gba");
		} else {
			showbubble = true;
			showSTARTborder = true;
			titleUpdate(dirContents[scrn].at(cursorPosition+pagenum*40).isDirectory, dirContents[scrn].at(cursorPosition+pagenum*40).name.c_str());
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
			cursorPosition -= 1;
			if (pagenum == 0) {
				if (!isRegularDS && cursorPosition == -1) cursorPosition -= 1;	// Skip "Start GBA Mode"
				if (cursorPosition >= -2) {
					titleboxXmoveleft = true;
					mmEffectEx(&snd_select);
				} else {
					mmEffectEx(&snd_wrong);
				}
			} else {
				if (cursorPosition >= 0) {
					titleboxXmoveleft = true;
					mmEffectEx(&snd_select);
				} else {
					mmEffectEx(&snd_wrong);
				}
			}
			if(romtype == 0 && cursorPosition >= 2 && cursorPosition <= 36) {
				if ((cursorPosition-2)+pagenum*40 < file_count) {
					iconUpdate(dirContents[scrn].at((cursorPosition-2)+pagenum*40).isDirectory, dirContents[scrn].at((cursorPosition-2)+pagenum*40).name.c_str(), cursorPosition-2);
				}
			}
		} else if ((pressed & KEY_RIGHT) && !titleboxXmoveleft && !titleboxXmoveright) {
			cursorPosition += 1;
			if (!isRegularDS && cursorPosition == -1) cursorPosition += 1;	// Skip "Start GBA Mode"
			if (cursorPosition <= 39) {
				titleboxXmoveright = true;
				mmEffectEx(&snd_select);
			} else {
				mmEffectEx(&snd_wrong);
			}
			if(romtype == 0 && cursorPosition >= 3 && cursorPosition <= 37) {
				if ((cursorPosition+2)+pagenum*40 < file_count) {
					iconUpdate(dirContents[scrn].at((cursorPosition+2)+pagenum*40).isDirectory, dirContents[scrn].at((cursorPosition+2)+pagenum*40).name.c_str(), cursorPosition+2);
				}
			}
		}
		// if (pressed & KEY_UP) cursorPosition -= ENTRY_PAGE_LENGTH;
		// if (pressed & KEY_DOWN) cursorPosition += ENTRY_PAGE_LENGTH;

		if ((pressed & KEY_DOWN) && !titleboxXmoveleft && !titleboxXmoveright)
		{
			mmEffectEx(&snd_switch);
			pagenum = 0;
			cursorPosition = 0;
			titleboxXpos = 0;
			titlewindowXpos = 0;
			whiteScreen = true;
			clearText(true);
			clearText(false);
			romtype +=1;
			if (romtype > 2) romtype = 0;
			return "null";
		} 
		if ((pressed & KEY_UP) && !titleboxXmoveleft && !titleboxXmoveright)
		{
			mmEffectEx(&snd_switch);
			pagenum = 0;
			cursorPosition = 0;
			titleboxXpos = 0;
			titlewindowXpos = 0;
			whiteScreen = true;
			clearText(true);
			clearText(false);
			romtype -=1;
			if (romtype < 0) romtype = 2;
			return "null";
		} 

		if (pagenum == 0) {
			if (cursorPosition < -2)
			{
				cursorPosition = -2;
			}
			else if (cursorPosition > 39)
			{
				cursorPosition = 39;
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
			if (cursorPosition == -1 || cursorPosition == -2) {
				mmEffectEx(&snd_launch);
				applaunch = true;
				applaunchprep = true;
				if (cursorPosition == -2) gotosettings = true;
				useBootstrap = false;

				showbubble = false;
				showSTARTborder = false;
				clearText(false);	// Clear title

				for (int i = 0; i < 60; i++) {
					swiWaitForVBlank();
				}

				clearText(true);
				for (int i = 0; i < 4; i++) swiWaitForVBlank();
				SaveSettings();

				if (cursorPosition == -2) {
					// Launch settings
					int err = runNdsFile ("/_nds/srloader/main.srldr", 0, NULL);
					iprintf ("Start failed. Error %i\n", err);
				} else if (cursorPosition == -1) {
					gbaMode();
				}
			}

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
			else if (launchable[cursorPosition])
			{
				donorFound = true;
				if(!flashcardUsed && romtype == 0 && arm7DonorPath.compare("") == 0) {
					FILE *f_nds_file = fopen(dirContents[scrn].at(cursorPosition+pagenum*40).name.c_str(), "rb");

					u32 SDKVersion = 0;
					char game_TID[5];
					grabTID(f_nds_file, game_TID);
					game_TID[4] = 0;
					game_TID[3] = 0;
					if(strcmp(game_TID, "###") != 0) SDKVersion = getSDKVersion(f_nds_file);
					fclose(f_nds_file);

					if(SDKVersion > 0x3000000 && SDKVersion < 0x5000000 && (strcmp(game_TID, "AMC") != 0)) {
						donorFound = false;
					}
				}
				if(donorFound) {
					mmEffectEx(&snd_launch);
					applaunch = true;
					applaunchprep = true;
					if (!isHomebrew[cursorPosition]) {
						useBootstrap = true;
					} else {
						useBootstrap = false;
					}
					
					showbubble = false;
					showSTARTborder = false;
					clearText(false);	// Clear title
					
					for (int i = 0; i < 60; i++) {
						swiWaitForVBlank();
					}

					clearText(true);
					for (int i = 0; i < 4; i++) swiWaitForVBlank();
					SaveSettings();

					// Return the chosen file
					return entry->name;
				} else {
					mmEffectEx(&snd_wrong);
					int yPos = 160;
					if (theme == 1) yPos -= 4;
					printSmallCentered(false, yPos, "Please set Mario Kart DS as donor ROM.");
					for (int i = 0; i < 60*2; i++) swiWaitForVBlank();
				}
			} else {
				mmEffectEx(&snd_wrong);
				int yPos = 160;
				if (theme == 1) yPos -= 4;
				printSmallCentered(false, yPos, "This game cannot be launched.");
				for (int i = 0; i < 90; i++) swiWaitForVBlank();
			}
		}

		if ((pressed & KEY_Y) && romtype == 0 && !titleboxXmoveleft && !titleboxXmoveright && showSTARTborder && cursorPosition >= 0)
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

				showbubble = false;
				showSTARTborder = false;
				clearText(false);	// Clear title

				for (int i = 0; i < 60; i++) {
					swiWaitForVBlank();
				}
				
				clearText(true);
				for (int i = 0; i < 4; i++) swiWaitForVBlank();
				SaveSettings();

				// Return the chosen file
				return entry->name;
			}
		}

		if ((pressed & KEY_L) && !titleboxXmoveleft && !titleboxXmoveright && pagenum != 0)
		{
			mmEffectEx(&snd_switch);
			pagenum -= 1;
			cursorPosition = 0;
			titleboxXpos = 0;
			titlewindowXpos = 0;
			whiteScreen = true;
			clearText(true);
			clearText(false);
			return "null";		
		} else 	if ((pressed & KEY_R) && !titleboxXmoveleft && !titleboxXmoveright && file_count > 40+pagenum*40)
		{
			mmEffectEx(&snd_switch);
			pagenum += 1;
			cursorPosition = 0;
			titleboxXpos = 0;
			titlewindowXpos = 0;
			whiteScreen = true;
			clearText(true);
			clearText(false);
			return "null";		
		}


		/* if (pressed & KEY_B && !isTopLevel(path))
		{
			// Go up a directory
			chdir("..");
			updatePath();
			pane->slideTransition(false, true);
			pane = &createTextPane(20, 3 + ENTRIES_START_ROW*FONT_SY, ENTRIES_PER_SCREEN);
			getDirectoryContents(dirContents[++scrn], extensionList);
			for (auto &i : dirContents[scrn])
				pane->addLine(i.visibleName.c_str());
			pane->createDefaultEntries();
			pane->slideTransition(true, true, 20);
			screenOffset = 0;
			cursorPosition = 0;
		} */
		
		if ((pressed & KEY_B) && !flashcardUsed) {
			clearText(false);
			clearText(true);
			whiteScreen = true;
			mmEffectEx(&snd_back);
			for (int i = 0; i < 4; i++) swiWaitForVBlank();
			SaveSettings();
			fifoSendValue32(FIFO_USER_02, 1);	// ReturntoDSiMenu
		}
		
		if (pressed & KEY_START)
		{
			applaunch = true;
			useBootstrap = false;
			gotosettings = true;
			// pane->slideTransition(false, true, 0, cursorPosition - screenOffset);
			// Return the chosen file
			// waitForPanesToClear();
			clearText(false);
			clearText(true);
			whiteScreen = true;
			for (int i = 0; i < 4; i++) swiWaitForVBlank();
			SaveSettings();
			int err = runNdsFile ("/_nds/srloader/main.srldr", 0, NULL);
			iprintf ("Start failed. Error %i\n", err);
		}

		if ((pressed & KEY_SELECT) && (romtype == 0) && !titleboxXmoveleft && !titleboxXmoveright && showSTARTborder && !flashcardUsed)
		{
			arm7DonorPath = "sd:/"+romfolder+"/"+dirContents[scrn].at(cursorPosition+pagenum*40).name.c_str();
			int yPos = 160;
			if (theme == 1) yPos -= 4;
			printSmallCentered(false, yPos, "Donor ROM is set.");
			for (int i = 0; i < 90; i++) swiWaitForVBlank();
		}

	}
}
