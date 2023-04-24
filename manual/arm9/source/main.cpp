#include <nds.h>
#include <nds/arm9/dldi.h>
#include <maxmod9.h>

#include <stdio.h>
#include <fat.h>
#include <sys/stat.h>
#include <limits.h>

#include <algorithm>
#include <string.h>
#include <unistd.h>

#include "graphics/graphics.h"

#include "common/systemdetails.h"
#include "common/nds_loader_arm9.h"
#include "common/twlmenusettings.h"
#include "common/flashcard.h"
#include "errorScreen.h"

#include "graphics/fontHandler.h"

#include "myDSiMode.h"
#include "common/inifile.h"
#include "common/tonccpy.h"
#include "language.h"

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

	PageLink(std::string dest, int x, int y, int w, int h) : dest(dest), x(x), y(y), w(w), h(h) {}
};

bool fadeType = false;		// false = out, true = in
bool fadeSpeed = true;		// false = slow (for DSi launch effect), true = fast
bool controlTopBright = true;
bool controlBottomBright = true;

extern int bgColor1;
extern int bgColor2;

extern void ClearBrightness();

std::vector<DirEntry> manPagesList;
std::vector<PageLink> manPageLinks;
std::string manPageTitle;

int manPageTitleX = 4;
Alignment manPageTitleAlign = Alignment::left;

int pageYpos = 0;
int pageYsize = 0;

char filePath[PATH_MAX];

mm_sound_effect snd_launch;
mm_sound_effect snd_select;
mm_sound_effect snd_stop;
mm_sound_effect snd_wrong;
mm_sound_effect snd_back;
mm_sound_effect snd_switch;

bool sortPagesPredicate(const DirEntry &lhs, const DirEntry &rhs) {
	return strcasecmp(lhs.name.c_str(), rhs.name.c_str()) < 0;
}

void loadPageList() {
	struct stat st;

	DIR *pdir = opendir(".");

	if (pdir == NULL) {
		printSmall(false, 0, 64, "Unable to open the directory.\n", Alignment::center);
	} else {

		while (true) {
			DirEntry dirEntry;

			struct dirent* pent = readdir(pdir);
			if (pent == NULL) break;

			stat(pent->d_name, &st);
			dirEntry.name = pent->d_name;
			dirEntry.isDirectory = (st.st_mode & S_IFDIR) ? true : false;

			if (dirEntry.name.substr(dirEntry.name.find_last_of(".") + 1) == "gif" && dirEntry.name.substr(0, 2) != "._") {
				char path[PATH_MAX] = {0};
				getcwd(path, PATH_MAX);
				manPagesList.push_back(dirEntry);
			} else if ((dirEntry.isDirectory) && (dirEntry.name.compare(".") != 0) && (dirEntry.name.compare("..") != 0)) {
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

	manPageTitle = pageIni.GetString("INFO","TITLE","TWiLight Menu++ Manual");
	toncset16(BG_PALETTE_SUB + 0xF6, pageIni.GetInt("INFO","BG_COLOR_1",0x6F7B), 1);
	toncset16(BG_PALETTE_SUB + 0xF7, pageIni.GetInt("INFO","BG_COLOR_2",0x77BD), 1);

	for (int i=1;true;i++) {
		std::string link = "LINK" + std::to_string(i);
		if (pageIni.GetString(link,"DEST","NONE") == "NONE")
			break;

		manPageLinks.emplace_back(pageIni.GetString(link,"DEST","NONE"),
								  pageIni.GetInt(link,"X",0),
								  pageIni.GetInt(link,"Y",0),
								  pageIni.GetInt(link,"W",0),
								  pageIni.GetInt(link,"H",0));
	}
}

//---------------------------------------------------------------------------------
void stop (void) {
//---------------------------------------------------------------------------------
	while (1) {
		swiWaitForVBlank();
	}
}

void InitSound() {
	mmInitDefaultMem((mm_addr)soundbank_bin);

	mmLoadEffect(SFX_LAUNCH);
	mmLoadEffect(SFX_SELECT);
	mmLoadEffect(SFX_STOP);
	mmLoadEffect(SFX_WRONG);
	mmLoadEffect(SFX_BACK);
	mmLoadEffect(SFX_SWITCH);

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

void loadROMselect() {
	mmEffectEx(&snd_back);
	fadeType = false;	// Fade out to white
	for (int i = 0; i < 25; i++) {
		swiWaitForVBlank();
	}

	std::vector<char *> argarray;

	switch (ms().theme) {
		case TWLSettings::EThemeDSi:
		case TWLSettings::EThemeHBL:
		case TWLSettings::EThemeSaturn:
			if (!ms().showSelectMenu) {
				argarray.push_back((char*)(sys().isRunFromSD() ? "sd:/_nds/TWiLightMenu/mainmenu.srldr" : "fat:/_nds/TWiLightMenu/mainmenu.srldr"));
				break;
			}
			// fall through
		case TWLSettings::ETheme3DS:
			argarray.push_back((char*)(sys().isRunFromSD() ? "sd:/_nds/TWiLightMenu/dsimenu.srldr" : "fat:/_nds/TWiLightMenu/dsimenu.srldr"));
			break;
		case TWLSettings::EThemeR4:
		case TWLSettings::EThemeGBC:
			argarray.push_back((char*)(sys().isRunFromSD() ? "sd:/_nds/TWiLightMenu/r4menu.srldr" : "fat:/_nds/TWiLightMenu/r4menu.srldr"));
			break;
		case TWLSettings::EThemeWood:
			argarray.push_back((char*)(sys().isRunFromSD() ? "sd:/_nds/TWiLightMenu/akmenu.srldr" : "fat:/_nds/TWiLightMenu/akmenu.srldr"));
			break;
	}

	runNdsFile(argarray[0], argarray.size(), (const char**)&argarray[0], true, false, false, true, true, false, -1);
	fadeType = true;	// Fade in from white
}

void customSleep() {
	fadeType = false;
	for (int i = 0; i < 25; i++) {
		swiWaitForVBlank();
	}
	if (!ms().macroMode) {
		powerOff(PM_BACKLIGHT_TOP);
	}
	powerOff(PM_BACKLIGHT_BOTTOM);
	irqDisable(IRQ_VBLANK & IRQ_VCOUNT);
	while (keysHeld() & KEY_LID) {
		scanKeys();
		swiWaitForVBlank();
	}
	irqEnable(IRQ_VBLANK & IRQ_VCOUNT);
	if (!ms().macroMode) {
		powerOn(PM_BACKLIGHT_TOP);
	}
	powerOn(PM_BACKLIGHT_BOTTOM);
	fadeType = true;
}

//---------------------------------------------------------------------------------
int manualScreen(void) {
//---------------------------------------------------------------------------------
	keysSetRepeat(25, 25);

	ms().loadSettings();

	graphicsInit();
	fontInit();

	InitSound();

	if (ms().rtl()) {
		manPageTitleX = 256 - manPageTitleX;
		manPageTitleAlign = Alignment::right;
	}
	int ySizeSub = ms().macroMode ? 176 : 368;

	langInit();

	chdir(("nitro:/pages/" + ms().getGuiLanguageString()).c_str());

	loadPageList();
	// Move index.gif to the start
	std::sort(manPagesList.begin(), manPagesList.end(), [](DirEntry a, DirEntry b) { return a.name == "index.gif"; });

	loadPageInfo(manPagesList[0].name.substr(0,manPagesList[0].name.length()-3) + "ini");
	pageLoad(manPagesList[0].name);
	topBarLoad();
	printSmall(true, manPageTitleX, 0, manPageTitle, manPageTitleAlign);

	int pressed = 0;
	int held = 0;
	int repeat = 0;
	int currentPage = 0, returnPage = -1;
	touchPosition touch;

	fadeType = true;	// Fade in from white

	while (1) {
		do {
			scanKeys();
			touchRead(&touch);
			pressed = keysDown();
			held = keysHeld();
			repeat = keysDownRepeat();
			checkSdEject();
			swiWaitForVBlank();
		} while (!held);

		if ((pressed & KEY_LID) && ms().sleepMode) {
			customSleep();
		}

		if (pressed & KEY_B) {
			if (returnPage != -1) {
				currentPage = returnPage;
				returnPage = -1;
				pageYpos = 0;
				loadPageInfo(manPagesList[currentPage].name.substr(0,manPagesList[currentPage].name.length()-3) + "ini");
				pageLoad(manPagesList[currentPage].name);
				clearText(true);
				printSmall(true, manPageTitleX, 0, manPageTitle, manPageTitleAlign);
			}
		} else if (held & KEY_UP) {
			pageYpos -= 4;
			if (pageYpos < 0) pageYpos = 0;
			pageScroll();
		} else if (held & KEY_DOWN) {
			pageYpos += 4;
			if (pageYpos > pageYsize-ySizeSub) pageYpos = pageYsize-ySizeSub;
			pageScroll();
		} else if (repeat & KEY_LEFT) {
			if (currentPage > 0) {
				pageYpos = 0;
				currentPage--;
				loadPageInfo(manPagesList[currentPage].name.substr(0,manPagesList[currentPage].name.length()-3) + "ini");
				pageLoad(manPagesList[currentPage].name);
				clearText(true);
				printSmall(true, manPageTitleX, 0, manPageTitle, manPageTitleAlign);
			}
		} else if (repeat & KEY_RIGHT) {
			if (currentPage < (int)manPagesList.size()-1) {
				pageYpos = 0;
				currentPage++;
				loadPageInfo(manPagesList[currentPage].name.substr(0,manPagesList[currentPage].name.length()-3) + "ini");
				pageLoad(manPagesList[currentPage].name);
				clearText(true);
				printSmall(true, manPageTitleX, 0, manPageTitle, manPageTitleAlign);
			}
		} else if (pressed & KEY_TOUCH) {
			touchPosition touchStart = touch;
			while ((touch.px < touchStart.px+10) && (touch.px > touchStart.px-10) && (touch.py < touchStart.py+10) && (touch.py > touchStart.py-10)) {
				touchRead(&touch);
			}
			scanKeys();
			if (keysHeld() & KEY_TOUCH) {
				touchStart = touch;
				touchPosition prevTouch2 = touch;
				while (1) {
					touchRead(&touch);
					scanKeys();
					if (!(keysHeld() & KEY_TOUCH)) {
						bool tapped = false;
						int dY = (-(touchStart.py - prevTouch2.py));
						while (!(dY < 0.25 && dY > -0.25)) {
							pageYpos += dY;
							if (pageYpos < 0) {
								pageYpos = 0;
								pageScroll();
								break;
							} else if (pageYpos > (pageYsize-ySizeSub)) {
								pageYpos = pageYsize-ySizeSub;
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
						if (tapped) {
							touchStart = touch;
							prevTouch2 = touch;
							continue;
						} else {
							break;
						}
					}

					if (((pageYpos + touchStart.py - touch.py) > 0) && ((pageYpos + touchStart.py - touch.py) < (pageYsize - ySizeSub)))
						pageYpos += touchStart.py - touch.py;
					pageScroll();

					prevTouch2 = touchStart;
					touchStart = touch;
					swiWaitForVBlank();
				}
			} else {
				for (uint i=0;i<manPageLinks.size();i++) {
					if (((touchStart.px >= manPageLinks[i].x) && (touchStart.px <= (manPageLinks[i].x + manPageLinks[i].w))) &&
						(((touchStart.py + pageYpos) >= manPageLinks[i].y - (ms().macroMode ? 0 : 176)) && ((touchStart.py + pageYpos) <= (manPageLinks[i].y - (ms().macroMode ? 0 : 176) + manPageLinks[i].h)))) {
						pageYpos = 0;
						returnPage = currentPage;
						for (uint j=0;j<manPagesList.size();j++) {
							if (manPagesList[j].name == (manPageLinks[i].dest + ".gif")) {
								currentPage = j;
								break;
							}
						}
						loadPageInfo(manPageLinks[i].dest + ".ini");
						pageLoad(manPagesList[currentPage].name);
						clearText(true);
						printSmall(true, manPageTitleX, 0, manPageTitle, manPageTitleAlign);
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
