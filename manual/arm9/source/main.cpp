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
#include "common/nitrofs.h"
#include "nds_loader_arm9.h"
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

// Do not reorder these, just add to the end
// This should be in dsimenusettings.h, but the manual currently doesn't have that
enum TLanguage {
	ELangDefault = -1,
	ELangJapanese = 0,
	ELangEnglish = 1,
	ELangFrench = 2,
	ELangGerman = 3,
	ELangItalian = 4,
	ELangSpanish = 5,
	ELangChineseS = 6,
	ELangKorean = 7,
	ELangChineseT = 8,
	ELangPolish = 9,
	ELangPortuguese = 10,
	ELangRussian = 11,
	ELangSwedish = 12,
	ELangDanish = 13,
	ELangTurkish = 14,
	ELangUkrainian = 15,
	ELangHungarian = 16,
	ELangNorwegian = 17,
	ELangHebrew = 18,
	ELangDutch = 19,
	ELangIndonesian = 20,
	ELangGreek = 21,
	ELangBulgarian = 22,
	ELangRomanian = 23,
	ELangArabic = 24,
	ELangPortugueseBrazil = 25,
};

int setLanguage = 0;
bool fadeType = false;		// false = out, true = in
bool fadeSpeed = true;		// false = slow (for DSi launch effect), true = fast
bool controlTopBright = true;
bool controlBottomBright = true;
bool macroMode = false;
int colorMode = 0;
int blfLevel = 0;

extern int bgColor1;
extern int bgColor2;

extern void ClearBrightness();

const char* settingsinipath = "sd:/_nds/TWiLightMenu/settings.ini";

int consoleModel = 0;
/*	0 = Nintendo DSi (Retail)
	1 = Nintendo DSi (Dev/Panda)
	2 = Nintendo 3DS
	3 = New Nintendo 3DS	*/

bool showSelectMenu = false;
int theme = 0;
int launcherApp = -1;
int sysRegion = -1;

std::string font = "default";

int guiLanguage = -1;
bool rtl = false;

bool sdRemoveDetect = true;

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

std::string getGuiLanguageString() {
	switch (setLanguage) {
		case ELangJapanese:
			return "ja";
		case ELangEnglish:
		default:
			return "en";
		case ELangFrench:
			return "fr";
		case ELangGerman:
			return "de";
		case ELangItalian:
			return "it";
		case ELangSpanish:
			return "es";
		case ELangChineseS:
			return "zh-CN";
		case ELangKorean:
			return "ko";
		case ELangChineseT:
			return "zh-TW";
		case ELangPolish:
			return "pl";
		case ELangPortuguese:
			return "pt";
		case ELangRussian:
			return "ru";
		case ELangSwedish:
			return "sv";
		case ELangDanish:
			return "da";
		case ELangTurkish:
			return "tr";
		case ELangUkrainian:
			return "uk";
		case ELangHungarian:
			return "hu";
		case ELangNorwegian:
			return "no";
		case ELangHebrew:
			return "he";
		case ELangDutch:
			return "nl";
		case ELangIndonesian:
			return "id";
		case ELangGreek:
			return "el";
		case ELangBulgarian:
			return "bg";
		case ELangRomanian:
			return "ro";
		case ELangArabic:
			return "ar";
		case ELangPortugueseBrazil:
			return "pt-BR";
	}
}

bool sortPagesPredicate(const DirEntry &lhs, const DirEntry &rhs) {
	return strcasecmp(lhs.name.c_str(), rhs.name.c_str()) < 0;
}

void loadPageList() {
	struct stat st;

	DIR *pdir = opendir(".");

	if (pdir == NULL) {
		printSmall(false, 0, 64, "Unable to open the directory.\n", Alignment::center);
	} else {

		while(true) {
			DirEntry dirEntry;

			struct dirent* pent = readdir(pdir);
			if(pent == NULL) break;

			stat(pent->d_name, &st);
			dirEntry.name = pent->d_name;
			dirEntry.isDirectory = (st.st_mode & S_IFDIR) ? true : false;

			if(dirEntry.name.substr(dirEntry.name.find_last_of(".") + 1) == "gif" && dirEntry.name.substr(0, 2) != "._") {
				char path[PATH_MAX] = {0};
				getcwd(path, PATH_MAX);
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

	manPageTitle = pageIni.GetString("INFO","TITLE","TWiLight Menu++ Manual");
	toncset16(BG_PALETTE_SUB + 0xF6, pageIni.GetInt("INFO","BG_COLOR_1",0x6F7B), 1);
	toncset16(BG_PALETTE_SUB + 0xF7, pageIni.GetInt("INFO","BG_COLOR_2",0x77BD), 1);

	for(int i=1;true;i++) {
		std::string link = "LINK" + std::to_string(i);
		if(pageIni.GetString(link,"DEST","NONE") == "NONE")
			break;

		manPageLinks.emplace_back(pageIni.GetString(link,"DEST","NONE"),
								  pageIni.GetInt(link,"X",0),
								  pageIni.GetInt(link,"Y",0),
								  pageIni.GetInt(link,"W",0),
								  pageIni.GetInt(link,"H",0));
	}
}

void LoadSettings(void) {
	// GUI
	CIniFile settingsini( settingsinipath );

	consoleModel = settingsini.GetInt("SRLOADER", "CONSOLE_MODEL", 0);
	showSelectMenu = settingsini.GetInt("SRLOADER", "SHOW_SELECT_MENU", 0);
	theme = settingsini.GetInt("SRLOADER", "THEME", 0);

	font = settingsini.GetString("SRLOADER", "FONT", font);

	macroMode = settingsini.GetInt("SRLOADER", "MACRO_MODE", macroMode);
	colorMode = settingsini.GetInt("SRLOADER", "COLOR_MODE", 0);
	blfLevel = settingsini.GetInt("SRLOADER", "BLUE_LIGHT_FILTER_LEVEL", 0);
	guiLanguage = settingsini.GetInt("SRLOADER", "LANGUAGE", -1);
	sdRemoveDetect = settingsini.GetInt("SRLOADER", "SD_REMOVE_DETECT", 1);
	if (consoleModel < 2) {
		launcherApp = settingsini.GetInt("SRLOADER", "LAUNCHER_APP", launcherApp);
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

void loadROMselect() {
	mmEffectEx(&snd_back);
	fadeType = false;	// Fade out to white
	for (int i = 0; i < 25; i++) {
		swiWaitForVBlank();
	}
	if (!isDSiMode()) {
		chdir("fat:/");
	} else {
		chdir((access("sd:/", F_OK) == 0) ? "sd:/" : "fat:/");
	}
	if (theme == 3)
	{
		runNdsFile("/_nds/TWiLightMenu/akmenu.srldr", 0, NULL, true, false, false, true, true);
	}
	else if (theme == 2 || theme == 6)
	{
		runNdsFile("/_nds/TWiLightMenu/r4menu.srldr", 0, NULL, true, false, false, true, true);
	}
	else if ((showSelectMenu && theme==0)
			|| (theme==1)
		|| (showSelectMenu && theme==4))
	{
		runNdsFile("/_nds/TWiLightMenu/dsimenu.srldr", 0, NULL, true, false, false, true, true);
	}
	else
	{
		runNdsFile("/_nds/TWiLightMenu/mainmenu.srldr", 0, NULL, true, false, false, true, true);
	}
	fadeType = true;	// Fade in from white
}

//---------------------------------------------------------------------------------
int main(int argc, char **argv) {
//---------------------------------------------------------------------------------
	defaultExceptionHandler();
	sys().initFilesystem();
	sys().initArm7RegStatuses();

	if (!sys().fatInitOk()) {
		SetBrightness(0, 0);
		SetBrightness(1, 0);
		consoleDemoInit();
		iprintf("FAT init failed!");
		stop();
	}

	if ((access(settingsinipath, F_OK) != 0) && (access("fat:/", F_OK) == 0)) {
		settingsinipath = "fat:/_nds/TWiLightMenu/settings.ini";		// Fallback to .ini path on flashcard, if not found on SD card, or if SD access is disabled
	}

	keysSetRepeat(25, 25);

	bool useTwlCfg = (dsiFeatures() && (*(u8*)0x02000400 == 0x07 || *(u8*)0x02000400 == 0x0F) && (*(u8*)0x02000401 == 0) && (*(u8*)0x02000402 == 0) && (*(u8*)0x02000404 == 0) && (*(u8*)0x02000448 != 0));

	sysSetCartOwner(BUS_OWNER_ARM9); // Allow arm9 to access GBA ROM

	LoadSettings();

	graphicsInit();
	fontInit();

	InitSound();

	int userLanguage = (useTwlCfg ? *(u8*)0x02000406 : PersonalData->language);
	setLanguage = (guiLanguage == -1) ? userLanguage : guiLanguage;
	bool rtl = (guiLanguage == ELangHebrew || guiLanguage == ELangArabic);
	if(rtl) {
		manPageTitleX = 256 - manPageTitleX;
		manPageTitleAlign = Alignment::right;
	}
	int ySizeSub = macroMode ? 176 : 368;

	langInit();

	chdir(("nitro:/pages/" + getGuiLanguageString()).c_str());

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

	while(1) {
		do {
			scanKeys();
			touchRead(&touch);
			pressed = keysDown();
			held = keysHeld();
			repeat = keysDownRepeat();
			checkSdEject();
			swiWaitForVBlank();
		} while(!held);

		if (pressed & KEY_B) {
			if(returnPage != -1) {
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
			if(currentPage > 0) {
				pageYpos = 0;
				currentPage--;
				loadPageInfo(manPagesList[currentPage].name.substr(0,manPagesList[currentPage].name.length()-3) + "ini");
				pageLoad(manPagesList[currentPage].name);
				clearText(true);
				printSmall(true, manPageTitleX, 0, manPageTitle, manPageTitleAlign);
			}
		} else if (repeat & KEY_RIGHT) {
			if(currentPage < (int)manPagesList.size()-1) {
				pageYpos = 0;
				currentPage++;
				loadPageInfo(manPagesList[currentPage].name.substr(0,manPagesList[currentPage].name.length()-3) + "ini");
				pageLoad(manPagesList[currentPage].name);
				clearText(true);
				printSmall(true, manPageTitleX, 0, manPageTitle, manPageTitleAlign);
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
						if(tapped) {
							touchStart = touch;
							prevTouch2 = touch;
							continue;
						} else {
							break;
						}
					}

					if(((pageYpos + touchStart.py - touch.py) > 0) && ((pageYpos + touchStart.py - touch.py) < (pageYsize - ySizeSub)))
						pageYpos += touchStart.py - touch.py;
					pageScroll();

					prevTouch2 = touchStart;
					touchStart = touch;
					swiWaitForVBlank();
				}
			} else {
				for(uint i=0;i<manPageLinks.size();i++) {
					if(((touchStart.px >= manPageLinks[i].x) && (touchStart.px <= (manPageLinks[i].x + manPageLinks[i].w))) &&
						(((touchStart.py + pageYpos) >= manPageLinks[i].y - (macroMode ? 0 : 176)) && ((touchStart.py + pageYpos) <= (manPageLinks[i].y - (macroMode ? 0 : 176) + manPageLinks[i].h)))) {
						pageYpos = 0;
						returnPage = currentPage;
						for(uint j=0;j<manPagesList.size();j++) {
							if(manPagesList[j].name == (manPageLinks[i].dest + ".gif")) {
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
