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
#include <nds/arm9/dldi.h>
#include <maxmod9.h>

#include <stdio.h>
#include <fat.h>
#include <sys/stat.h>
#include <limits.h>

#include <string.h>
#include <unistd.h>
#include "common/gl2d.h"

#include "graphics/graphics.h"
#include "graphics/topText.h"

#include "common/nitrofs.h"
#include "nds_loader_arm9.h"
#include "errorScreen.h"

#include "graphics/fontHandler.h"

#include "inifile.h"

#include "soundbank.h"
#include "soundbank_bin.h"

struct DirEntry {
	std::string name;
	bool isDirectory;
};

struct PageLink {
	std::string dest;
	int x;
	int y;
	int w;
	int h;
};

bool fadeType = false;		// false = out, true = in
bool fadeSpeed = true;		// false = slow (for DSi launch effect), true = fast
bool controlTopBright = true;
bool controlBottomBright = true;
int colorMode = 0;
int blfLevel = 0;

extern int bgColor1;
extern int bgColor2;

extern void ClearBrightness();

const char* settingsinipath = "sd:/_nds/TWiLightMenu/settings.ini";

const char *unlaunchAutoLoadID = "AutoLoadInfo";
static char hiyaNdsPath[14] = {'s','d','m','c',':','/','h','i','y','a','.','d','s','i'};
char unlaunchDevicePath[256];

bool arm7SCFGLocked = false;
int consoleModel = 0;
/*	0 = Nintendo DSi (Retail)
	1 = Nintendo DSi (Dev/Panda)
	2 = Nintendo 3DS
	3 = New Nintendo 3DS	*/
bool isRegularDS = true;

int theme = 0;
int launcherApp = -1;
int sysRegion = -1;

int guiLanguage = -1;

std::vector<DirEntry> manPagesList;
std::vector<PageLink> manPageLinks;
char manPageTitle[64] = {0};
int pageYpos = 0;
int pageYsize = 1036;

void loadPageList() {
	struct stat st;

	DIR *pdir = opendir("."); 
	
	if (pdir == NULL) {
		printSmallCentered(false, 64, "Unable to open the directory.\n");
	} else {

		while(true) {
			DirEntry dirEntry;

			struct dirent* pent = readdir(pdir);
			if(pent == NULL) break;

			stat(pent->d_name, &st);
			dirEntry.name = pent->d_name;
			dirEntry.isDirectory = (st.st_mode & S_IFDIR) ? true : false;

			if(dirEntry.name.substr(dirEntry.name.find_last_of(".") + 1) == "bmp") {
				char path[PATH_MAX] = {0};
				getcwd(path, PATH_MAX);
				dirEntry.name = path + dirEntry.name;
				manPagesList.push_back(dirEntry);
			} else if((dirEntry.isDirectory) && (dirEntry.name.compare(".") != 0) && (dirEntry.name.compare("..") != 0)) {
				chdir(dirEntry.name.c_str());
				loadPageList();
				chdir("..");
			}
		}
		closedir(pdir);
	}
}

void loadPageInfo(std::string pagePath) {
	manPageLinks.clear();

	CIniFile pageIni(pagePath);
	
	memset(&manPageTitle[0], 0, sizeof(manPageTitle));
	snprintf(manPageTitle, sizeof(manPageTitle), pageIni.GetString("INFO","TITLE","TWiLight Menu++ Manual").c_str());
	pageYsize = pageIni.GetInt("INFO","HEIGHT",1036);
	bgColor1 = pageIni.GetInt("INFO","BG_COLOR_1",0x6F7B);
	bgColor2 = pageIni.GetInt("INFO","BG_COLOR_2",0x77BD);

	for(int i=1;i<99;i++) {
		char link[7] = {0};
		snprintf(link, sizeof(link),"LINK%i",i);

		if(pageIni.GetString(link,"DEST","NONE") == "NONE")	break;

		PageLink pageLink;
		pageLink.x = pageIni.GetInt(link,"X",0);
		pageLink.y = pageIni.GetInt(link,"Y",0);
		pageLink.w = pageIni.GetInt(link,"W",0);
		pageLink.h = pageIni.GetInt(link,"H",0);
		pageLink.dest = pageIni.GetString(link,"DEST","NONE");

		manPageLinks.push_back(pageLink);
	}
}

void LoadSettings(void) {
	// GUI
	CIniFile settingsini( settingsinipath );

	// UI settings.
	consoleModel = settingsini.GetInt("SRLOADER", "CONSOLE_MODEL", 0);
	theme = settingsini.GetInt("SRLOADER", "THEME", 0);

	// Customizable UI settings.
	colorMode = settingsini.GetInt("SRLOADER", "COLOR_MODE", 0);
	blfLevel = settingsini.GetInt("SRLOADER", "BLUE_LIGHT_FILTER_LEVEL", 0);
	guiLanguage = settingsini.GetInt("SRLOADER", "LANGUAGE", -1);
	if (consoleModel < 2) {
		launcherApp = settingsini.GetInt("SRLOADER", "LAUNCHER_APP", launcherApp);
	}
}

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
void doPause() {
//---------------------------------------------------------------------------------
	// iprintf("Press start...\n");
	// printSmall(false, x, y, "Press start...");
	while(1) {
		scanKeys();
		if(keysDown() & KEY_START)
			break;
		swiWaitForVBlank();
	}
	scanKeys();
}

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
	mmLoadEffect( SFX_WRONG );
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

void loadROMselect()
{
	mmEffectEx(&snd_back);
	fadeType = false;	// Fade out to white
	for (int i = 0; i < 25; i++) {
		swiWaitForVBlank();
	}
	chdir((access("sd:/", F_OK) == 0) ? "sd:/" : "fat:/");
	if (theme == 3)
	{
		runNdsFile("/_nds/TWiLightMenu/akmenu.srldr", 0, NULL, false, false, true, true);
	}
	else if (theme == 2)
	{
		runNdsFile("/_nds/TWiLightMenu/r4menu.srldr", 0, NULL, false, false, true, true);
	}
	else
	{
		runNdsFile("/_nds/TWiLightMenu/dsimenu.srldr", 0, NULL, false, false, true, true);
	}
	fadeType = true;	// Fade in from white
}

void unalunchRomBoot(const char* rom) {
	char unlaunchDevicePath[256];
	snprintf(unlaunchDevicePath, sizeof(unlaunchDevicePath), "__%s", rom);
	unlaunchDevicePath[0] = 's';
	unlaunchDevicePath[1] = 'd';
	unlaunchDevicePath[2] = 'm';
	unlaunchDevicePath[3] = 'c';

	memcpy((u8*)0x02000800, unlaunchAutoLoadID, 12);
	*(u16*)(0x0200080C) = 0x3F0;		// Unlaunch Length for CRC16 (fixed, must be 3F0h)
	*(u16*)(0x0200080E) = 0;			// Unlaunch CRC16 (empty)
	*(u32*)(0x02000810) = 0;			// Unlaunch Flags
	*(u32*)(0x02000810) |= BIT(0);		// Load the title at 2000838h
	*(u32*)(0x02000810) |= BIT(1);		// Use colors 2000814h
	*(u16*)(0x02000814) = 0x7FFF;		// Unlaunch Upper screen BG color (0..7FFFh)
	*(u16*)(0x02000816) = 0x7FFF;		// Unlaunch Lower screen BG color (0..7FFFh)
	memset((u8*)0x02000818, 0, 0x20+0x208+0x1C0);		// Unlaunch Reserved (zero)
	int i2 = 0;
	for (int i = 0; i < (int)sizeof(unlaunchDevicePath); i++) {
		*(u8*)(0x02000838+i2) = unlaunchDevicePath[i];		// Unlaunch Device:/Path/Filename.ext (16bit Unicode,end by 0000h)
		i2 += 2;
	}
	while (*(u16*)(0x0200080E) == 0) {	// Keep running, so that CRC16 isn't 0
		*(u16*)(0x0200080E) = swiCRC16(0xFFFF, (void*)0x02000810, 0x3F0);		// Unlaunch CRC16
	}

	fifoSendValue32(FIFO_USER_02, 1);	// Reboot into DSiWare title, booted via Launcher
	for (int i = 0; i < 15; i++) swiWaitForVBlank();
}

void unlaunchSetHiyaBoot(void) {
	memcpy((u8*)0x02000800, unlaunchAutoLoadID, 12);
	*(u16*)(0x0200080C) = 0x3F0;		// Unlaunch Length for CRC16 (fixed, must be 3F0h)
	*(u16*)(0x0200080E) = 0;			// Unlaunch CRC16 (empty)
	*(u32*)(0x02000810) |= BIT(0);		// Load the title at 2000838h
	*(u32*)(0x02000810) |= BIT(1);		// Use colors 2000814h
	*(u16*)(0x02000814) = 0x7FFF;		// Unlaunch Upper screen BG color (0..7FFFh)
	*(u16*)(0x02000816) = 0x7FFF;		// Unlaunch Lower screen BG color (0..7FFFh)
	memset((u8*)0x02000818, 0, 0x20+0x208+0x1C0);		// Unlaunch Reserved (zero)
	int i2 = 0;
	for (int i = 0; i < 14; i++) {
		*(u8*)(0x02000838+i2) = hiyaNdsPath[i];		// Unlaunch Device:/Path/Filename.ext (16bit Unicode,end by 0000h)
		i2 += 2;
	}
	while (*(u16*)(0x0200080E) == 0) {	// Keep running, so that CRC16 isn't 0
		*(u16*)(0x0200080E) = swiCRC16(0xFFFF, (void*)0x02000810, 0x3F0);		// Unlaunch CRC16
	}
}

void pageScroll(void) {
	extern u16 pageImage[256*1036];
	dmaCopyWordsAsynch(0, (u16*)pageImage+(pageYpos*256), (u16*)BG_GFX_SUB+(18*256), 0x15C00);
	dmaCopyWordsAsynch(1, (u16*)pageImage+((174+pageYpos)*256), (u16*)BG_GFX, 0x18000);
	while (dmaBusy(0) || dmaBusy(1));
}

//---------------------------------------------------------------------------------
int main(int argc, char **argv) {
//---------------------------------------------------------------------------------

	// overwrite reboot stub identifier
	extern u64 *fake_heap_end;
	*fake_heap_end = 0;

	defaultExceptionHandler();

	bool fatInited = fatInitDefault();

	if (!fatInited) {
		graphicsInit();
		fontInit();
		printSmall(false, 64, 32, "fatinitDefault failed!");
		fadeType = true;
		stop();
	}

	nitroFSInit("/_nds/TWiLightMenu/manual.srldr");

	if ((access(settingsinipath, F_OK) != 0) && (access("fat:/", F_OK) == 0)) {
		settingsinipath = "fat:/_nds/TWiLightMenu/settings.ini";		// Fallback to .ini path on flashcard, if not found on SD card, or if SD access is disabled
	}

	fifoWaitValue32(FIFO_USER_06);
	if (fifoGetValue32(FIFO_USER_03) == 0) arm7SCFGLocked = true;	// If TWiLight Menu++ is being run from DSiWarehax or flashcard, then arm7 SCFG is locked.
	u16 arm7_SNDEXCNT = fifoGetValue32(FIFO_USER_07);
	if (arm7_SNDEXCNT != 0) isRegularDS = false;	// If sound frequency setting is found, then the console is not a DS Phat/Lite
	fifoSendValue32(FIFO_USER_07, 0);

	LoadSettings();

	if (isDSiMode() && (access("sd:/", F_OK) == 0) && consoleModel < 2 && launcherApp != -1) {
		u8 setRegion = 0;
		if (sysRegion == -1) {
			// Determine SysNAND region by searching region of System Settings on SDNAND
			char tmdpath[256];
			for (u8 i = 0x41; i <= 0x5A; i++)
			{
				snprintf(tmdpath, sizeof(tmdpath), "sd:/title/00030015/484e42%x/content/title.tmd", i);
				if (access(tmdpath, F_OK) == 0)
				{
					setRegion = i;
					break;
				}
			}
		} else {
			switch(sysRegion) {
				case 0:
				default:
					setRegion = 0x4A;	// JAP
					break;
				case 1:
					setRegion = 0x45;	// USA
					break;
				case 2:
					setRegion = 0x50;	// EUR
					break;
				case 3:
					setRegion = 0x55;	// AUS
					break;
				case 4:
					setRegion = 0x43;	// CHN
					break;
				case 5:
					setRegion = 0x4B;	// KOR
					break;
			}
		}

		snprintf(unlaunchDevicePath, sizeof(unlaunchDevicePath), "nand:/title/00030017/484E41%x/content/0000000%i.app", setRegion, launcherApp);
	}

	graphicsInit();
	fontInit();

	InitSound();

	chdir("nitro:/pages/");
	loadPageList();
	loadPageInfo(manPagesList[0].name.substr(0,manPagesList[0].name.length()-3) + "ini");
	pageLoad(manPagesList[0].name.c_str());
	topBarLoad();
	printTopText(manPageTitle);
	//bottomBgLoad();
	
	int pressed = 0;
	int held = 0;
	uint currentPage = 0;
	touchPosition touch;

	fadeType = true;	// Fade in from white

	// printSmallCentered(true, 0, "TWiLight Menu++ Manual");

	while(1) {
		scanKeys();
		touchRead(&touch);
		pressed = keysDown();
		held = keysHeld();
		checkSdEject();
		swiWaitForVBlank();

		if (held & KEY_UP) {
			pageYpos -= 4;
			if (pageYpos < 0) pageYpos = 0;
			pageScroll();
		} else if (held & KEY_DOWN) {
			pageYpos += 4;
			if (pageYpos > pageYsize-174) pageYpos = pageYsize-174;
			pageScroll();
		} else if (pressed & KEY_LEFT) {
			if(currentPage > 0) {
				pageYpos = 0;
				currentPage--;
				loadPageInfo(manPagesList[currentPage].name.substr(0,manPagesList[currentPage].name.length()-3) + "ini");
				pageLoad(manPagesList[currentPage].name.c_str());
				topBarLoad();
				printTopText(manPageTitle);
			}
		} else if (pressed & KEY_RIGHT) {
			if(currentPage < manPagesList.size()-1) {
				pageYpos = 0;
				currentPage++;
				loadPageInfo(manPagesList[currentPage].name.substr(0,manPagesList[currentPage].name.length()-3) + "ini");
				pageLoad(manPagesList[currentPage].name.c_str());
				topBarLoad();
				printTopText(manPageTitle);
			}
		} else if (pressed & KEY_TOUCH) {
			touchPosition touchStart = touch;
			while((touch.px < touchStart.px+10) && (touch.px > touchStart.px-10) && (touch.py < touchStart.py+10) && (touch.py > touchStart.py-10)) {
				touchRead(&touch);
			}
			scanKeys();
			if(keysHeld() & KEY_TOUCH) {
				touchStart = touch;
				touchPosition prevTouch2 = touch;
				while(1) {
					touchRead(&touch);
					scanKeys();
					if(!(keysHeld() & KEY_TOUCH)) {
						bool tapped = false;
						int dY = (-(touchStart.py - prevTouch2.py));
						while(!(dY < 0.25 && dY > -0.25)) {
							pageYpos += dY;
							if (pageYpos < 0) {
								pageYpos = 0;
								pageScroll();
								break;
							} else if (pageYpos > (pageYsize-174)) {
								pageYpos = pageYsize-174;
								pageScroll();
								break;
							}
							scanKeys();
							if (keysHeld() & KEY_TOUCH) {
								touchRead(&touch);
								tapped = true;
								break;
							}

							pageScroll();
							dY = dY / 1.125;
							swiWaitForVBlank();
						}
						if(tapped) {
							touchStart = touch;
							prevTouch2 = touch;
							continue;
						} else {
							break;
						}	
					}

					if(((pageYpos + touchStart.py - touch.py) > 0) && ((pageYpos + touchStart.py - touch.py) < (pageYsize - 174)))
						pageYpos += touchStart.py - touch.py;
					pageScroll();
					
					prevTouch2 = touchStart;
					touchStart = touch;
					swiWaitForVBlank();
				}
			} else {
				for(uint i=0;i<manPageLinks.size();i++) {
					if(((touchStart.px >= manPageLinks[i].x) && (touchStart.px <= (manPageLinks[i].x + manPageLinks[i].w))) &&
						(((touchStart.py + pageYpos) >= manPageLinks[i].y - 174) && ((touchStart.py + pageYpos) <= (manPageLinks[i].y - 174 + manPageLinks[i].h)))) {
						pageYpos = 0;
						for(uint j=0;j<manPagesList.size();j++) {
							if(manPagesList[j].name == (manPageLinks[i].dest + ".bmp")) {
								currentPage = j;
								break;
							}
						}
						loadPageInfo(manPageLinks[i].dest + ".ini");
						pageLoad((manPagesList[currentPage].name).c_str());
						topBarLoad();
						printTopText(manPageTitle);
					}
				}
			}
		}
		if (pressed & KEY_START) {
			loadROMselect();
		}
	}

	return 0;
}
