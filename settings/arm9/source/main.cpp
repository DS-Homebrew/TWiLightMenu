#include <nds.h>
#include <nds/arm9/dldi.h>
#include <cstdio>
#include <fat.h>
#include <sys/stat.h>
#include <limits.h>
#include <variant>
#include <string.h>
#include <unistd.h>
#include <maxmod9.h>

#include "autoboot.h"

#include "graphics/graphics.h"

#include "graphics/fontHandler.h"

#include "common/nds_loader_arm9.h"
#include "common/inifile.h"
#include "common/logging.h"
#include "common/fileCopy.h"
#include "common/bootstrappaths.h"
#include "common/bootstrapsettings.h"
#include "common/twlmenusettings.h"
#include "common/cardlaunch.h"
#include "common/flashcard.h"
#include "common/tonccpy.h"
#include "settingspage.h"
#include "settingsgui.h"
#include "language.h"
#include "gbarunner2settings.h"
#include "twlFlashcard.h"

#include "soundeffect.h"
#include "common/systemdetails.h"
#include "myDSiMode.h"

#define DSI_SYSTEM_UI_DIRECTORY "/_nds/TWiLightMenu/dsimenu/themes/"
#define _3DS_SYSTEM_UI_DIRECTORY "/_nds/TWiLightMenu/3dsmenu/themes/"

#define AK_SYSTEM_UI_DIRECTORY "/_nds/TWiLightMenu/akmenu/themes/"
#define R4_SYSTEM_UI_DIRECTORY "/_nds/TWiLightMenu/r4menu/themes/"

#define GBA_BORDER_DIRECTORY "/_nds/TWiLightMenu/gbaborders/"
#define UNLAUNCH_BG_DIRECTORY "/_nds/TWiLightMenu/unlaunch/backgrounds/"
#define FONT_DIRECTORY "/_nds/TWiLightMenu/extras/fonts/"

//bool widescreenEffects = false;

int currentTheme = 0;
bool currentMacroMode = false;
static TWLSettings::TExploit previousDSiWareExploit = TWLSettings::EExploitNone;
static int previousSysRegion = 0;

std::vector<std::string> akThemeList;
std::vector<std::string> r4ThemeList;
std::vector<std::string> dsiThemeList;
std::vector<std::string> _3dsThemeList;
std::vector<std::string> gbaBorderList;
std::vector<std::string> unlaunchBgList;
std::vector<std::string> fontList;
std::vector<std::string> menuSrldrList;

bool fadeType = false; // false = out, true = in

//bool soundfreqsettingChanged = false;
bool hiyaAutobootFound = false;
//static int flashcard;
/* Flashcard value
	0: DSTT/R4i Gold/R4i-SDHC/R4 SDHC Dual-Core/R4 SDHC Upgrade/SC DSONE
	1: R4DS (Original Non-SDHC version)/ M3 Simply
	2: R4iDSN/R4i Gold RTS/R4 Ultra
	3: Acekard 2(i)/Galaxy Eagle/M3DS Real
	4: Acekard RPG
	5: Ace 3DS+/Gateway Blue Card/R4iTT
	6: SuperCard DSTWO
*/

const char *hiyacfwinipath = "sd:/hiya/settings.ini";

std::string homebrewArg;
std::string bootstrapfilename;

int subscreenmode = 0;
int titleDisplayLength = 0;

int pressed = 0;
touchPosition touch;

using namespace std;

//---------------------------------------------------------------------------------
void stop(void)
{
	//---------------------------------------------------------------------------------
	while (1) {
		swiWaitForVBlank();
	}
}

char filePath[PATH_MAX];

//---------------------------------------------------------------------------------
void doPause(void)
{
	//---------------------------------------------------------------------------------
	printf("Press start...\n");
	while (1) {
		scanKeys();
		if (keysDown() & KEY_START)
			break;
	}
	scanKeys();
}

std::string ReplaceAll(std::string str, const std::string &from, const std::string &to)
{
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
	}
	return str;
}

bool extention(const std::string& filename, const char* ext) {
	return (strcasecmp(filename.c_str() + filename.size() - strlen(ext), ext) == 0);
}

void launchSystemSettings()
{
	*(int*)0x02003004 = 1; // Fade out sound
	for (int i = 0; i < 25; i++)
		swiWaitForVBlank();
	snd().stopBgMusic();
	*(int*)0x02003004 = 0; // Cancel sound fade out
	dsiLaunchSystemSettings();

	for (int i = 0; i < 15; i++)
		swiWaitForVBlank();
}

/*void applyTwlFirmSettings() {
	mkdir("sd:/luma/sysmodules", 0777);
	remove("sd:/luma/sysmodules/twlBg.cxi");
	if (ms().screenScaleSize == 1) {
		fcopy("sd:/_nds/TWiLightMenu/twlBg/screenScale_1.5.cxi", "sd:/luma/sysmodules/twlBg.cxi");
	}
}

void rebootTWLMenuPP()
{
	fifoSendValue32(FIFO_USER_01, 1); // Fade out sound
	for (int i = 0; i < 25; i++)
		swiWaitForVBlank();
	snd().stopBgMusic();
	memcpy((u32 *)0x02000300, autoboot_bin, 0x020);
	for (int i = 0; i < 10; i++)
		swiWaitForVBlank();
	fifoSendValue32(FIFO_USER_02, 1); // Reboot TWiLight Menu++ for TWL_FIRM changes to take effect
	for (int i = 0; i < 15; i++)
		swiWaitForVBlank();
}*/

void loadMainMenu()
{
	logPrint("Opening DS Classic Menu...\n");

	vector<char *> argarray;
	argarray.push_back((char*)(sys().isRunFromSD() ? "sd:/_nds/TWiLightMenu/mainmenu.srldr" : "fat:/_nds/TWiLightMenu/mainmenu.srldr"));

	runNdsFile(argarray[0], argarray.size(), (const char**)&argarray[0], true, false, false, true, true, false, -1);
	fadeType = true;	// Fade in from white
}

void loadROMselect()
{
	vector<char *> argarray;

	/*if (ms().theme == TWLSettings::EThemeWood) {
		argarray.push_back((char*)(sys().isRunFromSD() ? "sd:/_nds/TWiLightMenu/akmenu.srldr" : "fat:/_nds/TWiLightMenu/akmenu.srldr"));
	} else */if (ms().theme == TWLSettings::EThemeR4 || ms().theme == TWLSettings::EThemeGBC) {
		argarray.push_back((char*)(sys().isRunFromSD() ? "sd:/_nds/TWiLightMenu/r4menu.srldr" : "fat:/_nds/TWiLightMenu/r4menu.srldr"));
	} else {
		argarray.push_back((char*)(sys().isRunFromSD() ? "sd:/_nds/TWiLightMenu/dsimenu.srldr" : "fat:/_nds/TWiLightMenu/dsimenu.srldr"));
	}
	runNdsFile(argarray[0], argarray.size(), (const char**)&argarray[0], true, false, false, true, true, false, -1);
	fadeType = true;	// Fade in from white
}


void loadDSiThemeList()
{
	DIR *dir;
	struct dirent *ent;
	std::string themeDir;
	if ((dir = opendir(DSI_SYSTEM_UI_DIRECTORY)) != NULL) {
		// print all the files and directories within directory
		while ((ent = readdir(dir)) != NULL) {
			// Reallocation here, but prevents our vector from being filled with

			themeDir = ent->d_name;
			if (themeDir == ".." || themeDir == "..." || themeDir == "." || themeDir.substr(0, 2) == "._") continue;

			dsiThemeList.emplace_back(themeDir);
		}
		closedir(dir);
	}
}


void load3DSThemeList()
{
	DIR *dir;
	struct dirent *ent;
	std::string themeDir;
	if ((dir = opendir(_3DS_SYSTEM_UI_DIRECTORY)) != NULL) {
		// print all the files and directories within directory
		while ((ent = readdir(dir)) != NULL) {
			// Reallocation here, but prevents our vector from being filled with

			themeDir = ent->d_name;
			if (themeDir == ".." || themeDir == "..." || themeDir == "." || themeDir.substr(0, 2) == "._") continue;

			_3dsThemeList.emplace_back(themeDir);
		}
		closedir(dir);
	}
}


void loadAkThemeList()
{
	DIR *dir;
	struct dirent *ent;
	std::string themeDir;
	if ((dir = opendir(AK_SYSTEM_UI_DIRECTORY)) != NULL) {
		// print all the files and directories within directory
		while ((ent = readdir(dir)) != NULL) {
			// Reallocation here, but prevents our vector from being filled with

			themeDir = ent->d_name;
			if (themeDir == ".." || themeDir == "..." || themeDir == "." || themeDir.substr(0, 2) == "._") continue;

			akThemeList.emplace_back(themeDir);
		}
		closedir(dir);
	}
}

void loadR4ThemeList()
{
	DIR *dir;
	struct dirent *ent;
	std::string themeDir;
	if ((dir = opendir(R4_SYSTEM_UI_DIRECTORY)) != NULL) {
		// print all the files and directories within directory 
		while ((ent = readdir(dir)) != NULL) {
			// Reallocation here, but prevents our vector from being filled with

			themeDir = ent->d_name;
			if (themeDir == ".." || themeDir == "..." || themeDir == "." || themeDir.substr(0, 2) == "._") continue;

			r4ThemeList.emplace_back(themeDir);
		}
		closedir(dir);
	}
}

void loadUnlaunchBgList()
{
	DIR *dir;
	struct dirent *ent;
	std::string themeDir;
	if ((dir = opendir(UNLAUNCH_BG_DIRECTORY)) != NULL) {
		// print all the files and directories within directory 
		while ((ent = readdir(dir)) != NULL) {
			// Reallocation here, but prevents our vector from being filled with

			themeDir = ent->d_name;
			if (themeDir == ".." || themeDir == "..." || themeDir == "." || themeDir.substr(0, 2) == "._") continue;

			if (extention(themeDir, ".gif")) {
				unlaunchBgList.emplace_back(themeDir);
			}
		}
		closedir(dir);
	}
}

void loadFontList()
{
	DIR *dir;
	struct dirent *ent;
	std::string fontDir;
	if ((dir = opendir(FONT_DIRECTORY)) != NULL) {
		// print all the files and directories within directory
		while ((ent = readdir(dir)) != NULL) {
			// Reallocation here, but prevents our vector from being filled with

			fontDir = ent->d_name;
			if (fontDir == ".." || fontDir == "..." || fontDir == "." || fontDir.substr(0, 2) == "._") continue;

			fontList.emplace_back(fontDir);
		}
		closedir(dir);
	}
}

void loadGbaBorderList()
{
	DIR *dir;
	struct dirent *ent;
	std::string themeDir;
	if ((dir = opendir(GBA_BORDER_DIRECTORY)) != NULL) {
		// print all the files and directories within directory 
		while ((ent = readdir(dir)) != NULL) {
			// Reallocation here, but prevents our vector from being filled with

			themeDir = ent->d_name;
			if (themeDir == ".." || themeDir == "..." || themeDir == "." || themeDir.substr(0, 2) == "._") continue;

			if (extention(themeDir, ".png")) {
				gbaBorderList.emplace_back(themeDir);
			}
		}
		closedir(dir);
	}
}

void loadMenuSrldrList (const char* dirPath) {
	DIR *dir;
	struct dirent *ent;
	std::string srldrDir;
	if ((dir = opendir(dirPath)) != NULL) {
		// print all the files and directories within directory 
		while ((ent = readdir(dir)) != NULL) {
			// Reallocation here, but prevents our vector from being filled with

			srldrDir = ent->d_name;
			if (srldrDir == ".." || srldrDir == "..." || srldrDir == "." || srldrDir.substr(0, 2) == "._") continue;

			if (extention(srldrDir, "menu.srldr")) {
				menuSrldrList.emplace_back(srldrDir);
			}
		}
		closedir(dir);
	}
}

std::optional<Option> opt_subtheme_select(Option::Int &optVal)
{
	switch ((TWLSettings::TTheme)optVal.get()) {
	case TWLSettings::EThemeDSi:
		return Option(STR_SKINSEL_DSI, STR_AB_SETSKIN, Option::Str(&ms().dsi_theme), dsiThemeList);
	case TWLSettings::EThemeR4:
		return Option(STR_SKINSEL_R4, STR_AB_SETSKIN, Option::Str(&ms().r4_theme), r4ThemeList);
	// case TWLSettings::EThemeWood:
	// 	return Option(STR_SKINSEL_WOOD, STR_AB_SETSKIN, Option::Str(&ms().ak_theme), akThemeList);
	case TWLSettings::ETheme3DS:
		return Option(STR_SKINSEL_3DS, STR_AB_SETSKIN, Option::Str(&ms()._3ds_theme), _3dsThemeList);
	default:
		return nullopt;
	}
}

std::optional<Option> opt_gba_border_select(void)
{
	return Option(STR_BORDERSEL_GBA, STR_AB_SETBORDER, Option::Str(&ms().gbaBorder), gbaBorderList);
}

std::optional<Option> opt_bg_select(void)
{
	return Option(STR_BGSEL_UNLAUNCH, STR_AB_SETBG, Option::Str(&ms().unlaunchBg), unlaunchBgList);
}

std::optional<Option> opt_font_select(void)
{
	return Option(STR_FONTSEL, STR_AB_SETFONT, Option::Str(&ms().font), fontList);
}

std::string keyString(u16 pressed) {
	std::string keys = "";

	if (pressed & KEY_L)
		keys += " ";
	if (pressed & KEY_R)
		keys += " ";
	if (pressed & KEY_UP)
		keys += " ";
	if (pressed & KEY_DOWN)
		keys += " ";
	if (pressed & KEY_LEFT)
		keys += " ";
	if (pressed & KEY_RIGHT)
		keys += " ";
	if (pressed & KEY_START)
		keys += "START ";
	if (pressed & KEY_SELECT)
		keys += "SELECT ";
	if (pressed & KEY_A)
		keys += " ";
	if (pressed & KEY_B)
		keys += " ";
	if (pressed & KEY_X)
		keys += " ";
	if (pressed & KEY_Y)
		keys += " ";
	if (pressed & KEY_TOUCH)
		keys += STR_TOUCH + " ";

	return keys.substr(0, keys.size() - 1); // Remove trailing space
}

void opt_set_hotkey(void) {
	clearScroller();
	bool updateText = true;
	u16 held = 0, set = 0, timer = 0;
	int buttonsHeld = 0;
	std::string keys = "";
	while (timer < 60 || !(buttonsHeld >= 2 || set == KEY_B)) {
		if (updateText) {
			clearText(false);
			if (ms().rtl())
				printLarge(false, 256 - 4, 0, STR_HOLD_1S_SET, Alignment::right);
			else
				printLarge(false, 4, 0, STR_HOLD_1S_SET);

			keys = keyString(set);
			printSmall(false, 0, 48, keys, Alignment::center);
			printSmall(false, 0, 170 - calcSmallFontHeight(STR_HOLD_1S_DETAILS), STR_HOLD_1S_DETAILS, Alignment::center);
			updateText = false;
		}

		if (!gui().isExited())
			snd().playBgMusic(ms().settingsMusic);

		do {
			scanKeys();
			held = keysHeld();
			swiWaitForVBlank();
			if (set)
				timer++;
		} while (held == set && timer < 60);

		if (held != set) {
			timer = 0;
			set = held;
			updateText = true;
			buttonsHeld = 0;
			for(int i = 0; i < 16; i++) {
				if(set & BIT(i))
					buttonsHeld++;
			}
		}
	}

	// Holding *only* B cancels
	if(set == KEY_B) {
		keys = keyString(bs().bootstrapHotkey);
	} else {
		bs().bootstrapHotkey = set;
	}

	mmEffectEx(currentTheme == 4 ? &snd().snd_saturn_select : &snd().snd_select);

	clearText(false);
	if (ms().rtl())
		printLarge(false, 256 - 4, 0, set == KEY_B ? STR_HOTKEY_SETTING_CANCELLED : STR_HOTKEY_SET, Alignment::right);
	else
		printLarge(false, 4, 0, set == KEY_B ? STR_HOTKEY_SETTING_CANCELLED : STR_HOTKEY_SET);
	printSmall(false, 0, 48, keys, Alignment::center);
	do {
		scanKeys();
		held = keysHeld();
		swiWaitForVBlank();
	} while (held & set);

	clearText();
}

void begin_update(int opt)
{
	ms().saveSettings();
	gs().saveSettings();
	bs().saveSettings();

	clearText();
	printLarge(false, ms().rtl() ? 256 - 4 : 4, 0, STR_NOW_UPDATING, ms().rtl() ? Alignment::right : Alignment::left);

	logPrint("\n");
	if (opt == 1) {
		// Slot-1 microSD > Console SD
		logPrint("Copying main.srldr from fat to sd\n");
		fcopy("fat:/_nds/TWiLightMenu/main.srldr", "sd:/_nds/TWiLightMenu/main.srldr");
		logPrint("Copying manual.srldr from fat to sd\n");
		fcopy("fat:/_nds/TWiLightMenu/manual.srldr", "sd:/_nds/TWiLightMenu/manual.srldr");
		logPrint("Copying slot1launch.srldr from fat to sd\n");
		fcopy("fat:/_nds/TWiLightMenu/slot1launch.srldr", "sd:/_nds/TWiLightMenu/slot1launch.srldr");
		logPrint("Copying resetgame.srldr from fat to sd\n");
		fcopy("fat:/_nds/TWiLightMenu/resetgame.srldr", "sd:/_nds/TWiLightMenu/resetgame.srldr");
		logPrint("Copying settings.srldr from fat to sd\n");
		fcopy("fat:/_nds/TWiLightMenu/settings.srldr", "sd:/_nds/TWiLightMenu/settings.srldr");
	} else {
		// Console SD > Slot-1 microSD
		logPrint("Copying main.srldr from sd to fat\n");
		fcopy("sd:/_nds/TWiLightMenu/main.srldr", "fat:/_nds/TWiLightMenu/main.srldr");
		logPrint("Copying manual.srldr from sd to fat\n");
		fcopy("sd:/_nds/TWiLightMenu/manual.srldr", "fat:/_nds/TWiLightMenu/manual.srldr");
		logPrint("Copying slot1launch.srldr from sd to fat\n");
		fcopy("sd:/_nds/TWiLightMenu/slot1launch.srldr", "fat:/_nds/TWiLightMenu/slot1launch.srldr");
		logPrint("Copying resetgame.srldr from sd to fat\n");
		fcopy("sd:/_nds/TWiLightMenu/resetgame.srldr", "fat:/_nds/TWiLightMenu/resetgame.srldr");
		logPrint("Copying settings.srldr from sd to fat\n");
		fcopy("sd:/_nds/TWiLightMenu/settings.srldr", "fat:/_nds/TWiLightMenu/settings.srldr");
	}

	loadMenuSrldrList(opt==1 ? "fat:/_nds/TWiLightMenu/" : "sd:/_nds/TWiLightMenu/");

	// Copy theme srldr files
	logPrint(opt==1 ? "Copying *menu.srldr from fat to sd\n" : "Copying *menu.srldr from sd to fat\n");
	char srldrPath[2][256];
	for (int i = 0; i < (int)menuSrldrList.size(); i++) {
		sprintf(srldrPath[0], opt==1 ? "fat:/_nds/TWiLightMenu/%s" : "sd:/_nds/TWiLightMenu/%s", menuSrldrList[i].c_str());
		sprintf(srldrPath[1], opt==1 ? "sd:/_nds/TWiLightMenu/%s" : "fat:/_nds/TWiLightMenu/%s", menuSrldrList[i].c_str());
		fcopy(srldrPath[0], srldrPath[1]);
	}

	if (opt == 1) {
		// Slot-1 microSD > Console SD
		logPrint("Copying nds-bootstrap release from fat to sd\n");
		fcopy("fat:/_nds/nds-bootstrap-release.nds", "sd:/_nds/nds-bootstrap-release.nds");
		fcopy("fat:/_nds/release-bootstrap.ver", "sd:/_nds/release-bootstrap.ver");
		logPrint("Copying nds-bootstrap nightly from fat to sd\n");
		fcopy("fat:/_nds/nds-bootstrap-nightly.nds", "sd:/_nds/nds-bootstrap-nightly.nds");
		fcopy("fat:/_nds/nightly-bootstrap.ver", "sd:/_nds/nightly-bootstrap.ver");
	} else {
		// Console SD > Slot-1 microSD
		logPrint("Copying nds-bootstrap release from sd to fat\n");
		fcopy("sd:/_nds/nds-bootstrap-release.nds", "fat:/_nds/nds-bootstrap-release.nds");
		fcopy("sd:/_nds/release-bootstrap.ver", "fat:/_nds/release-bootstrap.ver");
		logPrint("Copying nds-bootstrap nightly from sd to fat\n");
		fcopy("sd:/_nds/nds-bootstrap-nightly.nds", "fat:/_nds/nds-bootstrap-nightly.nds");
		fcopy("sd:/_nds/nightly-bootstrap.ver", "fat:/_nds/nightly-bootstrap.ver");
	}

	fadeType = false;
	*(int*)0x02003004 = 1; // Fade out sound
	for (int i = 0; i < 25; i++)
		swiWaitForVBlank();
	snd().stopBgMusic();
	*(int*)0x02003004 = 0; // Cancel sound fade out
	
	runNdsFile("/_nds/TWiLightMenu/settings.srldr", 0, NULL, true, false, false, true, true, false, -1);
	stop();
}

void opt_update()
{
	int cursorPosition = 0;
	bool updateText = true;
	while (1) {
		if (updateText) {
			clearText();
			if (ms().rtl()) {
				printLarge(false, 256 - 4, 0, STR_HOW_WANT_UPDATE, Alignment::right);
				printSmall(false, 256 - 12, 29, ms().showMicroSd ? STR_CONSOLE_MICRO_SLOT1_MICRO : STR_CONSOLE_SD_SLOT1_MICRO, Alignment::right);
				printSmall(false, 256 - 12, 43, ms().showMicroSd ? STR_SLOT1_MICRO_CONSOLE_MICRO : STR_SLOT1_MICRO_CONSOLE_SD, Alignment::right);
				printSmall(false, 256 - 4, 29+(14*cursorPosition), "<", Alignment::right);
			} else {
				printLarge(false, 4, 0, STR_HOW_WANT_UPDATE);
				printSmall(false, 12, 29, ms().showMicroSd ? STR_CONSOLE_MICRO_SLOT1_MICRO : STR_CONSOLE_SD_SLOT1_MICRO);
				printSmall(false, 12, 43, ms().showMicroSd ? STR_SLOT1_MICRO_CONSOLE_MICRO : STR_SLOT1_MICRO_CONSOLE_SD);
				printSmall(false, 4, 29+(14*cursorPosition), ">");
			}
			updateText = false;
		}

		if (!gui().isExited()) {
			snd().playBgMusic(ms().settingsMusic);
		}

		do
		{
			scanKeys();
			pressed = keysDownRepeat();
			touchRead(&touch);
			swiWaitForVBlank();
		} while (!pressed);

		if (pressed & KEY_UP) {
			mmEffectEx(currentTheme==4 ? &snd().snd_saturn_select : &snd().snd_select);
			cursorPosition--;
			if (cursorPosition < 0) cursorPosition = 1;
			updateText = true;
		}

		if (pressed & KEY_DOWN) {
			mmEffectEx(currentTheme==4 ? &snd().snd_saturn_select : &snd().snd_select);
			cursorPosition++;
			if (cursorPosition > 1) cursorPosition = 0;
			updateText = true;
		}

		if (pressed & KEY_A) {
			mmEffectEx(currentTheme==4 ? &snd().snd_saturn_launch : &snd().snd_launch);
			begin_update(cursorPosition);
			break;
		}

		if (pressed & KEY_B) {
			mmEffectEx(currentTheme==4 ? &snd().snd_saturn_back : &snd().snd_back);
			break;
		}
	}
	clearText();
}

//bool twlFirmChanged = false;

void defaultExitHandler()
{
	/*if (twlFirmChanged) {
		applyTwlFirmSettings();
		rebootTWLMenuPP();
	}*/

	if (isDSiMode() && sdFound() && !flashcardFound() && !sys().arm7SCFGLocked() && ms().limitedMode > 0) {
		*(u32*)0x02FFFD0C = ms().limitedMode == 2 ? 0x4E44544C : ms().limitedMode == 3 ? 0x6D44544C : 0x4D44544C;
		*(u32*)0x020007F0 = 0x4D44544C;
	}

	if (isDSiMode() && sdFound() && ms().consoleModel >= 2 && sys().arm7SCFGLocked() &&
	   (previousDSiWareExploit != ms().dsiWareExploit || previousSysRegion != ms().sysRegion)) {
		u32 currentSrBackendId[2] = {0};
		u8 sysValue = 0;

		switch (ms().dsiWareExploit) {
			case TWLSettings::EExploitSudokuhax:
				currentSrBackendId[0] = 0x4B344441;		// SUDOKU
				break;
			case TWLSettings::EExploit4Swordshax:
				currentSrBackendId[0] = 0x4B513941;		// Legend of Zelda: Four Swords
				break;
			case TWLSettings::EExploitFieldrunnerhax:
				currentSrBackendId[0] = 0x4B464441;		// Fieldrunners
				break;
			case TWLSettings::EExploitGrtpwn:
				currentSrBackendId[0] = 0x4B475241;		// Guitar Rock Tour
				break;
			case TWLSettings::EExploitUgopwn:
				currentSrBackendId[0] = 0x4B475541;		// Flipnote Studio
				break;
			case TWLSettings::EExploitUnopwn:
				currentSrBackendId[0] = 0x4B554E41;		// UNO
				break;
			case TWLSettings::EExploitMemoryPit:
				currentSrBackendId[0] = 0x484E4941;		// Nintendo DSi Camera
				break;
			case TWLSettings::EExploitNone:
				break;
		}
		switch (ms().sysRegion) {
			case TWLSettings::ERegionJapan:
				sysValue = 0x4A;		// JPN
				break;
			case TWLSettings::ERegionUSA:
				sysValue = 0x45;		// USA
				break;
			case TWLSettings::ERegionEurope:
				sysValue = (ms().dsiWareExploit==TWLSettings::EExploitMemoryPit ? 0x50 : 0x56);		// EUR
				break;
			case TWLSettings::ERegionAustralia:
				sysValue = (ms().dsiWareExploit==TWLSettings::EExploitMemoryPit ? 0x55 : 0x56);		// AUS
				break;
			case TWLSettings::ERegionChina:
				sysValue = 0x43;		// CHN
				break;
			case TWLSettings::ERegionKorea:
				sysValue = 0x4B;		// KOR
				break;
			case TWLSettings::ERegionDefault:
				break;
		}
		tonccpy(&currentSrBackendId, &sysValue, 1);
		currentSrBackendId[1] = (ms().dsiWareExploit==TWLSettings::EExploitMemoryPit ? 0x00030005 : 0x00030004);

		if (ms().dsiWareExploit != TWLSettings::EExploitNone) {
			FILE* file = fopen("sd:/_nds/nds-bootstrap/srBackendId.bin", "wb");
			if (file) {
				fwrite(currentSrBackendId, sizeof(u32), 2, file);
			}
			fclose(file);
		} else {
			remove("sd:/_nds/nds-bootstrap/srBackendId.bin");
		}
	}

	flashcardInit();
	if (!isDSiMode()) {
		chdir("fat:/");
	}
	if (ms().macroMode) {
		powerOff(PM_BACKLIGHT_TOP);
	} else {
		powerOn(PM_BACKLIGHT_TOP);
	}
	if (ms().showMainMenu) {
		loadMainMenu();
	}
	loadROMselect();
}

void opt_reboot_system_menu()
{
	gui().onExit(launchSystemSettings).saveAndExit(currentTheme);
}

void opt_hiya_autoboot_toggle(bool prev, bool next)
{
	if (!next) {
		if (remove("sd:/hiya/autoboot.bin") == 0)
			hiyaAutobootFound = false;
	} else {
		FILE *ResetData = fopen("sd:/hiya/autoboot.bin", "wb");
		fwrite(autoboot_bin, 1, autoboot_bin_len, ResetData);
		fclose(ResetData);
		CIniFile hiyacfwini(hiyacfwinipath);
		hiyacfwini.SetInt("HIYA-CFW", "TITLE_AUTOBOOT", 1);
		hiyacfwini.SaveIniFile(hiyacfwinipath);
	}
}

void opt_wifiLed_toggle(bool prev, bool next)
{
	*(u8*)(0x02FFFD00) = (next ? 0x13 : 0x12);		// On/Off
}

void opt_powerLed_toggle(bool prev, bool next)
{
	*(u8*)(0x02FFFD02) = (next ? 0xFF : 0x00);
}

/*void opt_twlFirm_changed(int prev, int next)
{
	twlFirmChanged = true;
}*/

inline bool between_incl(int x, int a, int b)
{
	return (a <= x && x >= b);
}

void customSleep() {
	*(int*)0x02003004 = 1; // Fade out sound
	fadeType = false;
	while (!screenFadedOut()) {
		swiWaitForVBlank();
	}
	mmPause();
	if (!currentMacroMode) {
		powerOff(PM_BACKLIGHT_TOP);
	}
	powerOff(PM_BACKLIGHT_BOTTOM);
	irqDisable(IRQ_VBLANK & IRQ_VCOUNT);
	while (keysHeld() & KEY_LID) {
		scanKeys();
		swiWaitForVBlank();
	}
	irqEnable(IRQ_VBLANK & IRQ_VCOUNT);
	if (!currentMacroMode) {
		powerOn(PM_BACKLIGHT_TOP);
	}
	powerOn(PM_BACKLIGHT_BOTTOM);
	mmResume();
	fadeType = true;
	*(int*)0x02003004 = 2; // Fade in sound
}

//---------------------------------------------------------------------------------
int settingsMode(void)
{
//---------------------------------------------------------------------------------
	ms().loadSettings();
	gs().loadSettings();
	bs().loadSettings();
	logInit();
	//loadAkThemeList();
	loadR4ThemeList();
	load3DSThemeList();
	loadDSiThemeList();
	if (sys().isRegularDS()) {
		loadGbaBorderList();
	}
	if (dsiFeatures() && ms().consoleModel == 0) {
		loadUnlaunchBgList();
	}
	loadFontList();
	swiWaitForVBlank();

	snd().init();
	keysSetRepeat(25, 5);

	bool widescreenFound = false;
	if (sdFound() && ms().consoleModel >= 2 && (!isDSiMode() || !sys().arm7SCFGLocked())) {
		CIniFile lumaConfig("sd:/luma/config.ini");
		widescreenFound = ((access("sd:/_nds/TWiLightMenu/TwlBg/Widescreen.cxi", F_OK) == 0) && (lumaConfig.GetInt("boot", "enable_external_firm_and_modules", 0) == true));
		logPrint(widescreenFound ? "Widescreen found\n" : "Widescreen not found\n");
	}

	const bool sharedFound = (access("sd:/shared1", F_OK) == 0);

	//widescreenEffects = (ms().wideScreen && widescreenFound);

	sysSetCartOwner(BUS_OWNER_ARM9); // Allow arm9 to access GBA ROM

	graphicsInit();
	fontInit();
	langInit();
	fadeType = true;

	srand(time(NULL));

	if (sdFound() && ms().consoleModel < 2) {
		hiyaAutobootFound = (access("sd:/hiya/autoboot.bin", F_OK) == 0);
		logPrint(hiyaAutobootFound ? "hiya autoboot file found\n" : "hiya autoboot file not found\n");
	}

	currentTheme = ms().theme;
	currentMacroMode = ms().macroMode;
	previousDSiWareExploit = ms().dsiWareExploit;
	previousSysRegion = ms().sysRegion;

//#pragma endregion

	// consoleDemoInit();
	SettingsPage guiPage(STR_GUI_SETTINGS);

	using TLanguage = TWLSettings::TLanguage;
	using TRegion = TWLSettings::TRegion;
	using TTheme = TWLSettings::TTheme;
	using TDSiMusic = TWLSettings::TDSiMusic;
	using TSettingsMusic = TWLSettings::TSettingsMusic;
	using TSortMethod = TWLSettings::TSortMethod;
	//using TAKScrollSpeed = TWLSettings::TScrollSpeed;

	guiPage
		// Language
		.option(STR_LANGUAGE,
				STR_DESCRIPTION_LANGUAGE_1,
				Option::Int((int *)&ms().guiLanguage),
				{STR_SYSTEM,
				 "Bahasa Indonesia",
				 "Català",
				 "Dansk",
				 "Deutsch",
				 "English",
				 "Español",
				 "Français",
				 "Italiano",
				 "Magyar",
				 "Nederlands",
				 "Norsk",
				 "Polski",
				 "Português (Portugal)",
				 "Português (Brasil)",
				 "Română",
				 "Svenska",
				 "Tiếng Việt",
				 "Türkçe",
				 "Valencià",
				 "Ελληνικά",
				 "Български",
				 "Русский",
				 "Українська",
				 "עברית",
				 "العربية",
				 "中文 (简体)",
				 "中文 (繁體)",
				 "日本語",
				 "琉球諸語",
				 "한국어"},
				{TLanguage::ELangDefault,
				 TLanguage::ELangIndonesian,
				 TLanguage::ELangCatalan,
				 TLanguage::ELangDanish,
				 TLanguage::ELangGerman,
				 TLanguage::ELangEnglish,
				 TLanguage::ELangSpanish,
				 TLanguage::ELangFrench,
				 TLanguage::ELangItalian,
				 TLanguage::ELangHungarian,
				 TLanguage::ELangDutch,
				 TLanguage::ELangNorwegian,
				 TLanguage::ELangPolish,
				 TLanguage::ELangPortuguese,
				 TLanguage::ELangPortugueseBrazil,
				 TLanguage::ELangRomanian,
				 TLanguage::ELangSwedish,
				 TLanguage::ELangVietnamese,
				 TLanguage::ELangTurkish,
				 TLanguage::ELangValencian,
				 TLanguage::ELangGreek,
				 TLanguage::ELangBulgarian,
				 TLanguage::ELangRussian,
				 TLanguage::ELangUkrainian,
				 TLanguage::ELangHebrew,
				 TLanguage::ELangArabic,
				 TLanguage::ELangChineseS,
				 TLanguage::ELangChineseT,
				 TLanguage::ELangJapanese,
				 TLanguage::ELangRyukyuan,
				 TLanguage::ELangKorean})
		// Theme
		.option(STR_THEME,
				STR_DESCRIPTION_THEME_1,
				Option::Int((int *)&ms().theme, opt_subtheme_select),
				{STR_NINTENDO_DSI, STR_NINTENDO_3DS, STR_SEGA_SATURN, STR_HOMEBREW_LAUNCHER, STR_R4_ORIGINAL, STR_GAMEBOY_COLOR},
				{TTheme::EThemeDSi, TTheme::ETheme3DS, TTheme::EThemeSaturn, TTheme::EThemeHBL, TTheme::EThemeR4, TTheme::EThemeGBC})
		.option(STR_DSCLASSICMENU, STR_DESCRIPTION_DSCLASSICMENU, Option::Bool(&ms().showMainMenu), {STR_YES, STR_NO}, {true, false})
		.option("DSi/Saturn: SELECT", STR_DESCRIPTION_SELECTBUTTONOPTION, Option::Bool(&ms().showSelectMenu), {STR_SELECT_MENU, STR_DS_CLASSIC_MENU}, {true, false})
		.option(STR_DSIMUSIC,
				STR_DESCRIPTION_DSIMUSIC,
				Option::Int((int *)&ms().dsiMusic),
				{STR_OFF, STR_REGULAR, STR_DSI_SHOP, "HBL", STR_THEME},
				{TDSiMusic::EMusicOff, TDSiMusic::EMusicRegular, TDSiMusic::EMusicShop, TDSiMusic::EMusicHBL, TDSiMusic::EMusicTheme})
		.option(STR_SETTINGSMUSIC,
				STR_DESCRIPTION_SETTINGSMUSIC,
				Option::Int((int *)&ms().settingsMusic),
				{STR_THEME, STR_OFF, STR_NINTENDO_DSI, STR_NINTENDO_3DS},
				{TSettingsMusic::ESMusicTheme, TSettingsMusic::ESMusicOff, TSettingsMusic::ESMusicDSi, TSettingsMusic::ESMusic3DS})
		.option(STR_FONT,
				STR_DESCRIPTION_FONT,
				Option::Nul(opt_font_select),
				{STR_PRESS_A},
				{0})
		.option(STR_USE_THEME_FONT,
				STR_DESCRIPTION_USE_THEME_FONT,
				Option::Bool(&ms().useThemeFont),
				{STR_YES, STR_NO},
				{true, false})
		.option(STR_DS_CLASSIC_CUSTOM_FONT,
				STR_DESCRIPTION_DS_CLASSIC_CUSTOM_FONT,
				Option::Bool(&ms().dsClassicCustomFont),
				{STR_DEFAULT, STR_CUSTOM_SPLASH},
				{false, true})
		.option(STR_SORT_METHOD, STR_DESCRIPTION_SORT_METHOD, Option::Int((int *)&ms().sortMethod), {STR_ALPHABETICAL, STR_RECENT, STR_MOST_PLAYED, STR_FILE_TYPE, STR_CUSTOM}, {TSortMethod::ESortAlphabetical, TSortMethod::ESortRecent, TSortMethod::ESortMostPlayed, TSortMethod::ESortFileType, TSortMethod::ESortCustom})
		.option(STR_DSIMENUPPLOGO, STR_DESCRIPTION_DSIMENUPPLOGO_1, Option::Bool(&ms().showlogo), {STR_SHOW, STR_HIDE}, {true, false})
		.option(STR_SPLASH_JINGLE_LENGTH, STR_DESCRIPTION_SPLASH_JINGLE_LENGTH, Option::Bool(&ms().longSplashJingle), {STR_LONG, STR_SHORT}, {true, false});
	if (ms().macroMode) {
		guiPage
			.option(STR_GBSPLASH, STR_DESCRIPTION_GBSPLASH, Option::Int(&ms().dsiSplash), {STR_SHOW, STR_CUSTOM_SPLASH, STR_HIDE}, {(ms().dsiSplash==2 ? 2 : 1), 3, 0});
	} else {
		guiPage
			.option(sys().isRegularDS() ? STR_DSSPLASH : STR_DSISPLASH, sys().isRegularDS() ? STR_DESCRIPTION_DSSPLASH : STR_DESCRIPTION_DSISPLASH, Option::Int(&ms().dsiSplash), {STR_WITHOUT_HS, STR_WITH_HS, STR_CUSTOM_SPLASH, STR_HIDE}, {1, 2, 3, 0})
			.option(sys().isRegularDS() ? STR_DSSPLASHAUTOSKIP : STR_DSISPLASHAUTOSKIP, sys().isRegularDS() ? STR_DESCRIPTION_DSSPLASHAUTOSKIP : STR_DESCRIPTION_DSISPLASHAUTOSKIP, Option::Bool(&ms().dsiSplashAutoSkip), {STR_OFF, STR_ON}, {false, true});
	}
	guiPage
		.option(STR_NINTENDOLOGOCOLOR, STR_DESCRIPTION_NINTENDOLOGOCOLOR, Option::Int(&ms().nintendoLogoColor), {STR_RED, STR_BLUE, STR_MAGENTA, STR_GRAY}, {1, 2, 3, 0})
		.option(STR_DIRECTORIES, STR_DESCRIPTION_DIRECTORIES_1, Option::Bool(&ms().showDirectories), {STR_SHOW, STR_HIDE}, {true, false})
		.option(STR_SHOW_HIDDEN, STR_DESCRIPTION_SHOW_HIDDEN_1, Option::Bool(&ms().showHidden), {STR_SHOW, STR_HIDE}, {true, false});

	if (dsiFeatures()) {
		guiPage
			.option(STR_BOXART, STR_DESCRIPTION_BOXART_DSI, Option::Int(&ms().showBoxArt), {STR_NON_CACHED, STR_CACHED, STR_HIDE}, {1, 2, 0})
			.option(STR_PHOTO_BOXART_COLOR_DEBAND, STR_DESCRIPTION_PHOTO_BOXART_COLOR_DEBAND, Option::Bool(&ms().boxArtColorDeband), {STR_ON, STR_OFF}, {true, false});
	} else {
		guiPage.option(STR_BOXART, STR_DESCRIPTION_BOXART, Option::Int(&ms().showBoxArt), {STR_SHOW, STR_HIDE}, {1, 0});
	}

	if (sdFound()) {
		guiPage.option(STR_REFERSD, STR_DESCRIPTION_REFERSD, Option::Bool(&ms().showMicroSd), {STR_MICRO_SD_CARD, STR_SD_CARD}, {true, false});
	}

	guiPage
		.option(STR_CLOCK_SYSTEM, STR_DESCRIPTION_CLOCK_SYSTEM, Option::Bool(&ms().show12hrClock), {STR_12_HOUR, STR_24_HOUR}, {true, false})
		.option(STR_COLORMODE, STR_DESCRIPTION_COLORMODE, Option::Int(&ms().colorMode), {STR_REGULAR, STR_BW_GREYSCALE}, {0, 1})
		.option(STR_ANIMATEDSIICONS, STR_DESCRIPTION_ANIMATEDSIICONS_1, Option::Bool(&ms().animateDsiIcons), {STR_YES, STR_NO}, {true, false})
		.option(STR_CUSTOMICONS, STR_DESCRIPTION_CUSTOMICONS, Option::Bool(&ms().showCustomIcons), {STR_ON, STR_OFF}, {true, false})
		.option(STR_FRAMERATE, STR_DESCRIPTION_FRAMERATE, Option::Int(&ms().fps), {STR_15FPS, STR_20FPS, STR_24FPS, STR_30FPS, STR_50FPS, STR_60FPS}, {15, 20, 24, 30, 50, 60})
		.option(STR_LOGGING, STR_DESCRIPTION_LOGGING_TWLMENU, Option::Bool(&ms().logging), {STR_ON, STR_OFF}, {true, false})
		/*.option(STR_AK_SCROLLSPEED, STR_DESCRIPTION_AK_SCROLLSPEED, Option::Int(&ms().ak_scrollSpeed), {STR_FAST, STR_MEDIUM, STR_SLOW},
				{TAKScrollSpeed::EScrollFast, TAKScrollSpeed::EScrollMedium, TAKScrollSpeed::EScrollSlow})
		.option(STR_AK_ZOOMING_ICON, STR_DESCRIPTION_ZOOMING_ICON, Option::Bool(&ms().ak_zoomIcons), {STR_ON, STR_OFF}, {true, false})*/;

	SettingsPage bootstrapPage(STR_BOOTSTRAP_SETTINGS);

	bootstrapPage
		.option(STR_GAMELANGUAGE,
				STR_DESCRIPTION_GAMELANGUAGE_1,
				Option::Int((int *)&ms().gameLanguage),
				{STR_SYSTEM,
				 "Deutsch",
				 "English",
				 "Español",
				 "Français",
				 "Italiano",
				 "中文 (简体)",
				 "日本語",
				 "한국어"},
				{TLanguage::ELangDefault, 
				 TLanguage::ELangGerman,
				 TLanguage::ELangEnglish,
				 TLanguage::ELangSpanish,
				 TLanguage::ELangFrench,
				 TLanguage::ELangItalian,
				 TLanguage::ELangChineseS,
				 TLanguage::ELangJapanese,
				 TLanguage::ELangKorean})
		.option(STR_TITLELANGUAGE,
				STR_DESCRIPTION_TITLELANGUAGE_1,
				Option::Int((int *)&ms().titleLanguage),
				{STR_SYSTEM,
				 "Deutsch",
				 "English",
				 "Español",
				 "Français",
				 "Italiano",
				 "日本語",},
				{TLanguage::ELangDefault, 
				 TLanguage::ELangGerman,
				 TLanguage::ELangEnglish,
				 TLanguage::ELangSpanish,
				 TLanguage::ELangFrench,
				 TLanguage::ELangItalian,
				 TLanguage::ELangJapanese,});
	if (dsiFeatures()) {
		bootstrapPage
		.option(STR_GAMEREGION,
				STR_DESCRIPTION_GAMEREGION,
				Option::Int((int *)&ms().gameRegion),
				{STR_SYSTEM,
				 STR_JAPAN,
				 STR_USA,
				 STR_EUROPE,
				 STR_AUSTRALIA,
				 STR_CHINA,
				 STR_KOREA},
				{TRegion::ERegionDefault,
				 TRegion::ERegionJapan,
				 TRegion::ERegionUSA,
				 TRegion::ERegionEurope,
				 TRegion::ERegionAustralia,
				 TRegion::ERegionChina,
				 TRegion::ERegionKorea});
	} else {
		bootstrapPage
		.option(STR_GAMEREGION,
				STR_DESCRIPTION_GAMEREGION,
				Option::Int((int *)&ms().gameRegion),
				{STR_JAPAN,
				 STR_USA,
				 STR_EUROPE,
				 STR_AUSTRALIA,
				 STR_CHINA,
				 STR_KOREA},
				{TRegion::ERegionJapan,
				 TRegion::ERegionUSA,
				 TRegion::ERegionEurope,
				 TRegion::ERegionAustralia,
				 TRegion::ERegionChina,
				 TRegion::ERegionKorea});
	}
	bootstrapPage
		.option(STR_USEROMREGION,
				STR_DESCRIPTION_USEROMREGION,
				Option::Bool(&ms().useRomRegion),
				{STR_YES, STR_NO},
				{true, false});

	if (flashcardFound() && (dsiFeatures() || sdFound())) {
		if (sdFound() && (!isDSiMode() || (dsiFeatures() && !sys().arm7SCFGLocked()))) {
			bootstrapPage.option("S1SD: "+STR_GAMELOADER, STR_DESCRIPTION_GAMELOADER, Option::Bool(&ms().useBootstrap), {"nds-bootstrap", STR_KERNEL}, {true, false});
			if (dsiFeatures()) {
				bootstrapPage
					.option(STR_FCSAVELOCATION, STR_DESCRIPTION_FCSAVELOCATION, Option::Bool(&ms().fcSaveOnSd), {STR_CONSOLE_SD, STR_SLOT_1_SD}, {true, false})
					.option(STR_S1SD_B4DSMODE, STR_DESCRIPTION_B4DSMODE, Option::Int(&bs().b4dsMode), {STR_OFF, STR_4MB_RAM, STR_8MB_RAM}, {0, 1, 2});
			}
		} else if (isDSiMode()) {
			bootstrapPage.option(STR_B4DSMODE, STR_DESCRIPTION_B4DSMODE, Option::Int(&bs().b4dsMode), {STR_OFF, STR_4MB_RAM, STR_8MB_RAM}, {0, 1, 2});
		} else {
			bootstrapPage.option(STR_GAMELOADER, STR_DESCRIPTION_GAMELOADER, Option::Bool(&ms().useBootstrap), {"nds-bootstrap", STR_KERNEL}, {true, false});
		}
	} else if (io_dldi_data->ioInterface.features & FEATURE_SLOT_NDS && !(dsiFeatures() || sdFound())) {
		bootstrapPage.option(STR_GAMELOADER, STR_DESCRIPTION_GAMELOADER, Option::Bool(&ms().useBootstrap), {"nds-bootstrap", STR_KERNEL}, {true, false});
	}

	if (widescreenFound) {
		bootstrapPage.option((dsiFeatures() ? STR_ASPECTRATIO : STR_SD_ASPECTRATIO),
			STR_DESCRIPTION_ASPECTRATIO,
			Option::Bool(&ms().wideScreen),
			{STR_WIDESCREEN, STR_FULLSCREEN},
			{true, false});
	}

	bootstrapPage
		.option(STR_ESRBRATINGSCREEN, STR_DESCRIPTION_ESRBRATINGSCREEN, Option::Bool(&ms().esrbRatingScreen), {STR_ON, STR_OFF}, {true, false})
		.option(STR_HOTKEY, STR_DESCRIPTION_HOTKEY, Option::Nul(opt_set_hotkey), {STR_PRESS_A}, {0});

	if (!sys().isRegularDS()) {
		bootstrapPage.option(STR_SNDFREQ, STR_DESCRIPTION_SNDFREQ_1, Option::Bool((bool *)&ms().soundFreq), {"47.61 kHz", "32.73 kHz"}, {true, false});
	}

	using TROMReadLED = BootstrapSettings::TROMReadLED;

	if ((isDSiMode() || sdFound()) && ms().consoleModel == 0) {
		if (sdFound()) {
			bootstrapPage
				.option((flashcardFound() ? STR_SYSSD_ROMREADLED : STR_ROMREADLED),
					STR_DESCRIPTION_ROMREADLED_1,
					Option::Int(&bs().romreadled),
					{STR_NONE, STR_WIFI, STR_POWER, STR_CAMERA},
					{TROMReadLED::ELEDNone, TROMReadLED::ELEDWifi, TROMReadLED::ELEDPower, TROMReadLED::ELEDCamera})
				.option((flashcardFound() ? STR_SD_DMAROMREADLED : STR_DMAROMREADLED),
					STR_DESCRIPTION_DMAROMREADLED,
					Option::Int(&bs().dmaromreadled),
					{STR_SAME_AS_REG, STR_NONE, STR_WIFI, STR_POWER, STR_CAMERA},
					{TROMReadLED::ELEDSame, TROMReadLED::ELEDNone, TROMReadLED::ELEDWifi, TROMReadLED::ELEDPower, TROMReadLED::ELEDCamera});
		}
		bootstrapPage.option((dsiFeatures() ? STR_PRECISEVOLUMECTRL : STR_SYSSD_PRECISEVOLUMECTRL),
			STR_DESCRIPTION_PRECISEVOLUMECTRL,
			Option::Bool(&bs().preciseVolumeControl),
			{STR_ON, STR_OFF},
			{true, false});
	}

	if (sdFound()) {
		if (flashcardFound()) {
			bootstrapPage
				.option(STR_DSIWARETOSD,
					STR_DESCRIPTION_DSIWARETOSD,
					Option::Bool(&ms().dsiWareToSD),
					{STR_YES, STR_NO},
					{true, false});
		}
		bootstrapPage
			.option(sharedFound ? STR_TWLNANDLOCATION : STR_PHOTOLOCATION,
				sharedFound ? STR_DESCRIPTION_TWLNANDLOCATION : STR_DESCRIPTION_PHOTOLOCATION,
				Option::Bool(&bs().sdNand),
				{ms().showMicroSd ? STR_MICRO_SD_CARD : STR_SD_CARD, sharedFound ? STR_SYSTEM : STR_NAND},
				{true, false});

		if (dsiFeatures() && (!isDSiMode() || (isDSiMode() && !sys().arm7SCFGLocked()))) {
			bootstrapPage.option((isDSiMode() ? STR_FORCESLEEPPATCH : STR_SYSSD_FORCESLEEPPATCH),
				STR_DESCRIPTION_FORCESLEEPMODE,
				Option::Bool(&ms().forceSleepPatch),
				{STR_ON, STR_OFF},
				{true, false});
		}

		bootstrapPage
			.option(STR_LOAD_BOOTLOADER, STR_DESCRIPTION_LOAD_BOOTLOADER, Option::Bool(&ms().btsrpBootloaderDirect), {STR_DIRECT, STR_THRU_NDS_BS}, {true, false});
	}

	bootstrapPage
		.option(STR_BOOTSTRAP, STR_DESCRIPTION_BOOTSTRAP_1, Option::Bool((bool *)&ms().bootstrapFile), {STR_NIGHTLY, STR_RELEASE}, {true, false})
		.option(STR_DEBUG, STR_DESCRIPTION_DEBUG_1, Option::Bool(&bs().debug), {STR_ON, STR_OFF}, {true, false})
		.option(STR_LOGGING, STR_DESCRIPTION_LOGGING_1, Option::Bool(&bs().logging), {STR_ON, STR_OFF}, {true, false});

	SettingsPage gbar2Page(STR_GBARUNNER2_SETTINGS);

	if (flashcardFound()) {
		gbar2Page.option(sdFound() ? STR_SLOT_1_DLDI_ACCESS : STR_DLDI_ACCESS, STR_DESCRIPTION_GBAR2_DLDIACCESS, Option::Bool(&ms().gbar2DldiAccess), {"ARM7", "ARM9"}, {true, false});
	}
	gbar2Page
		.option(STR_DS_MAIN_MEMORY_I_CACHE, STR_DESCRIPTION_GBAR2_MAINMEMICACHE, Option::Bool(&gs().mainMemICache), {STR_ON, STR_OFF}, {true, false})
		.option(STR_WRAM_I_CACHE, STR_DESCRIPTION_GBAR2_WRAMICACHE, Option::Bool(&gs().wramICache), {STR_ON, STR_OFF}, {true, false})
		.option(STR_BIOS_INTRO, STR_DESCRIPTION_BIOSINTRO, Option::Bool(&gs().skipIntro), {STR_OFF, STR_ON}, {true, false});
	if (!ms().macroMode) {
		gbar2Page.option(STR_DISPLAY_SCREEN, STR_DESCRIPTION_DISPLAY_SCREEN, Option::Bool(&gs().bottomScreenPrefered), {STR_BOTTOM, STR_TOP}, {true, false});
	}
	gbar2Page
		.option(STR_BORDER_FRAME, STR_DESCRIPTION_BORDER_FRAME, Option::Bool(&gs().frame), {STR_ON, STR_OFF}, {true, false})
		.option(STR_CENTER_AND_MASK, STR_DESCRIPTION_CENTERANDMASK, Option::Bool(&gs().centerMask), {STR_ON, STR_OFF}, {true, false})
		.option(STR_SIMULATE_GBA_COLORS, STR_DESCRIPTION_GBACOLORS, Option::Bool(&gs().gbaColors), {STR_YES, STR_NO}, {true, false});

	SettingsPage unlaunchPage(STR_UNLAUNCH_SETTINGS);
	if (sdFound() && ms().consoleModel == 0) {
		unlaunchPage
			.option(STR_BACKGROUND,
				STR_DESCRIPTION_UNLAUNCH_BG,
				Option::Nul(opt_bg_select),
				{STR_PRESS_A},
				{0})
			.option(STR_LAUNCHER_PATCHES,
				STR_DESCRIPTION_LAUNCHER_PATCHES,
				Option::Int(&ms().removeLauncherPatches),
				{STR_OFF, STR_FULL, STR_DEFAULT},
				{0, 1, 2});
	}

	SettingsPage gamesPage(STR_GAMESAPPS_SETTINGS);

	using TGbaBooter = TWLSettings::TGbaBooter;
	using TColSegaEmulator = TWLSettings::TColSegaEmulator;
	using TCpcEmulator = TWLSettings::TCpcEmulator;
	using TMegaDriveEmulator = TWLSettings::TMegaDriveEmulator;
	using TSlot1LaunchMethod = TWLSettings::TSlot1LaunchMethod;

	gamesPage.option(STR_COL_EMULATOR, STR_DESCRIPTION_COL_EMULATOR, Option::Int((int *)&ms().colEmulator), {"S8DS", "ColecoDS"}, {TColSegaEmulator::EColSegaS8DS, TColSegaEmulator::EColSegaColecoDS});
	if (ms().consoleModel == 0 && sdFound() && !sys().arm7SCFGLocked())
		gamesPage.option(STR_DSIWAREBOOTER, STR_DESCRIPTION_DSIWAREBOOTER, Option::Bool((bool *)&ms().dsiWareBooter), {"nds-bootstrap", "Unlaunch"}, {true, false});
	if (sys().isRegularDS()) {
		gamesPage
			.option(STR_GBA_BOOTER, STR_DESCRIPTION_GBA_BOOTER, Option::Int((int *)&ms().gbaBooter), {STR_NATIVE_GBARUNNER2, STR_GBARUNNER2_ONLY}, {TGbaBooter::EGbaNativeGbar2, TGbaBooter::EGbaGbar2})
			.option(STR_GBABORDER, STR_DESCRIPTION_GBABORDER, Option::Nul(opt_gba_border_select), {STR_PRESS_A}, {0});
	}
	if (!(isDSiMode() && sdFound() && sys().arm7SCFGLocked()))
		gamesPage.option(STR_MD_EMULATOR, STR_DESCRIPTION_MD_EMULATOR, Option::Int((int *)&ms().mdEmulator), {"jEnesisDS", "PicoDriveTWL", STR_HYBRID}, {TMegaDriveEmulator::EMegaDriveJenesis, TMegaDriveEmulator::EMegaDrivePico, TMegaDriveEmulator::EMegaDriveHybrid});
	gamesPage.option(STR_SG_EMULATOR, STR_DESCRIPTION_SG_EMULATOR, Option::Int((int *)&ms().sgEmulator), {"S8DS", "ColecoDS"}, {TColSegaEmulator::EColSegaS8DS, TColSegaEmulator::EColSegaColecoDS});
	gamesPage.option(STR_CPC_EMULATOR, STR_DESCRIPTION_CPC_EMULATOR, Option::Int((int *)&ms().cpcEmulator), {"AmEDS", "CrocoDS"}, {TCpcEmulator::ECpcAmEDS, TCpcEmulator::ECpcCrocoDS});

	if (isDSiMode() && sdFound() && !sys().arm7SCFGLocked()) {
		gamesPage
			.option((flashcardFound() ? STR_SYSSD_RUNFLUBBAEMUSIN : STR_RUNFLUBBAEMUSIN),
					STR_DESCRIPTION_RUNFLUBBAEMUSIN,
					Option::Bool(&ms().smsGgInRam),
					{STR_DS_MODE, STR_DSI_MODE},
					{true, false});

		if (ms().consoleModel == 0) {
			gamesPage.option(STR_SLOT1LAUNCHMETHOD, STR_DESCRIPTION_SLOT1LAUNCHMETHOD_1, Option::Int((int *)&ms().slot1LaunchMethod), {STR_REBOOT, STR_DIRECT, "Unlaunch"}, {TSlot1LaunchMethod::EReboot, TSlot1LaunchMethod::EDirect, TSlot1LaunchMethod::EUnlaunch});
		} else {
			gamesPage.option(STR_SLOT1LAUNCHMETHOD, STR_DESCRIPTION_SLOT1LAUNCHMETHOD_1, Option::Int((int *)&ms().slot1LaunchMethod), {STR_REBOOT, STR_DIRECT}, {TSlot1LaunchMethod::EReboot, TSlot1LaunchMethod::EDirect});
		}
	}

	SettingsPage miscPage(STR_MISC_SETTINGS);

	if (sdFound() && flashcardFound()) {
		miscPage.option(STR_UPDATETWLMENU,
						STR_DESCRIPTION_UPDATETWLMENU,
						Option::Nul(opt_update),
						{STR_PRESS_A},
						{0});
	}

	if (sdFound() && isDSiMode()) {
		if (!sys().arm7SCFGLocked()) {
			miscPage
				.option(STR_SLOT1SCFGUNLOCK, STR_DESCRIPTION_SLOT1SCFGUNLOCK, Option::Bool(&ms().slot1SCFGUnlock), {STR_ON, STR_OFF}, {true, false})
				.option(STR_SLOT1SDACCESS, STR_DESCRIPTION_SLOT1SDACCESS, Option::Bool(&ms().slot1AccessSD), {STR_ON, STR_OFF}, {true, false})
				.option(STR_SLOT1TOUCHMODE, STR_DESCRIPTION_SLOT1TOUCHMODE, Option::Bool(&ms().slot1TouchMode), {STR_DSI_MODE, STR_DS_MODE}, {true, false})
				.option(STR_S1SDACCESS, STR_DESCRIPTION_S1SDACCESS_1, Option::Bool(&ms().secondaryAccess), {STR_ON, STR_OFF}, {true, false});
		}

		miscPage
			.option(STR_AUTOSTARTSLOT1, STR_DESCRIPTION_AUTOSTARTSLOT1, Option::Bool(&ms().autostartSlot1), {STR_YES, STR_NO}, {true, false});
	}

	miscPage
		.option(STR_PREVENT_ROM_DELETION, STR_DESCRIPTION_PREVENT_ROM_DELETION_1, Option::Bool(&ms().preventDeletion), {STR_YES, STR_NO}, {true, false})
		.option(STR_UPDATE_RECENTLY_PLAYED_LIST, STR_DESCRIPTION_UPDATE_RECENTLY_PLAYED_LIST, Option::Bool(&ms().updateRecentlyPlayedList), {STR_YES, STR_NO}, {true, false});

	if (isDSiMode()) {
		miscPage
			.option(STR_WIFI,
					STR_DESCRIPTION_WIFI,
					Option::Bool(&ms().wifiLed, opt_wifiLed_toggle),
					{STR_ON, STR_OFF},
					{true, false});
	}

	if (isDSiMode() && ms().consoleModel < 2) {
		miscPage
			.option(STR_POWERLEDCOLOR,
					STR_DESCRIPTION_POWERLEDCOLOR,
					Option::Bool(&ms().powerLedColor, opt_powerLed_toggle),
					{STR_PURPLE, STR_BLUE+"/"+STR_RED},
					{true, false});

		if (ms().consoleModel == 0 && sdFound()) {
			miscPage
				.option(STR_SDREMOVALDETECTION,
					STR_DESCRIPTION_SDREMOVALDETECTION,
					Option::Bool(&ms().sdRemoveDetect),
					{STR_ON, STR_OFF},
					{true, false});
		}
	}

	miscPage.option(STR_MACROMODE,
					STR_DESCRIPTION_MACROMODE,
					Option::Bool(&ms().macroMode),
					{STR_ON, STR_OFF},
					{true, false});
		/*.option(STR_BLF,
				STR_DESCRIPTION_BLF,
				Option::Int(&ms().blfLevel),
				{STR_OFF,
				 "Lv. 1",
				 "Lv. 2",
				 "Lv. 3",
				 "Lv. 4",
				 "Lv. 5"},
				{0,
				 1,
				 2,
				 3,
				 4,
				 5,});*/

	if (isDSiMode() && sdFound() && !flashcardFound() && (!sys().arm7SCFGLocked() || *(u32*)0x020007F0 == 0x4D44544C)) {
		miscPage.option(STR_LIMITEDMODE,
				STR_DESCRIPTION_LIMITEDMODE,
				Option::Int(&ms().limitedMode),
				{STR_OFF, STR_GENERAL, "Memory Pit"},
				{0, 1, 2});
	}

	using TExploit = TWLSettings::TExploit;

	if (isDSiMode() && sdFound() && ms().consoleModel >= 2 && sys().arm7SCFGLocked()) {
		miscPage
			.option(STR_DSIWARE_EXPLOIT,
				STR_DESCRIPTION_DSIWARE_EXPLOIT,
				Option::Int((int *)&ms().dsiWareExploit),
				{STR_NONE, "sudokuhax", "4swordshax", "fieldrunnerhax", "grtpwn", "ugopwn/Lenny", "UNO*pwn", "Memory Pit"},
				{TExploit::EExploitNone, TExploit::EExploitSudokuhax, TExploit::EExploit4Swordshax, TExploit::EExploitFieldrunnerhax, TExploit::EExploitGrtpwn, TExploit::EExploitUgopwn, TExploit::EExploitUnopwn, TExploit::EExploitMemoryPit});
	}
	if (sdFound() && (ms().consoleModel < 2 || sys().arm7SCFGLocked())) {
		miscPage
			.option(STR_SYSREGION,
				STR_DESCRIPTION_SYSREGION_1,
				Option::Int((int *)&ms().sysRegion),
				{STR_AUTO_HIYA_ONLY, "JPN", "USA", "EUR", "AUS", "CHN", "KOR"},
				{TRegion::ERegionDefault, TRegion::ERegionJapan, TRegion::ERegionUSA, TRegion::ERegionEurope, TRegion::ERegionAustralia, TRegion::ERegionChina, TRegion::ERegionKorea});
	}
	if (sdFound() && ms().consoleModel < 2) {
		miscPage
			.option(STR_LAUNCHERAPP,
				STR_DESCRIPTION_LAUNCHERAPP,
				Option::Int(&ms().launcherApp),
				{STR_NONE,
				"00000000.app",
				"00000001.app",
				"00000002.app",
				"00000003.app",
				"00000004.app",
				"00000005.app",
				"00000006.app",
				"00000007.app",
				"00000008.app",
				"launcher.dsi"},
				{-1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9});
	}

	if (isDSiMode() && ms().consoleModel < 2) {
		// Actions do not have to bound to an object.
		// See for exam here we have bound an option to
		// hiyaAutobootFound.

		// We are also using the changed callback to write
		// or delete the hiya autoboot file.
		miscPage
			.option(STR_DEFAULT_LAUNCHER, STR_DESCRIPTION_DEFAULT_LAUNCHER_1, Option::Bool(&hiyaAutobootFound, opt_hiya_autoboot_toggle), {"TWiLight Menu++", STR_SYSTEM_MENU}, {true, false})
			.option(STR_SYSTEMSETTINGS, STR_DESCRIPTION_SYSTEMSETTINGS_1, Option::Nul(opt_reboot_system_menu), {}, {});
	}

	/*SettingsPage twlfirmPage(STR_TWLFIRM_SETTINGS);
	if (isDSiMode() && ms().consoleModel >= 2) {
		twlfirmPage
			.option(STR_SCREENSCALESIZE, STR_DESCRIPTION_SCREENSCALESIZE, Option::Int(&ms().screenScaleSize, opt_twlFirm_changed), {"1x/1.25x", "1.5x"}, {0, 1});
	}*/
	
	gui()
		.addPage(guiPage)
		.addPage(bootstrapPage)
		.addPage(gbar2Page);
	if (sdFound() && ms().consoleModel == 0)
		gui().addPage(unlaunchPage);
	gui()
		.addPage(gamesPage)
		.addPage(miscPage);

	/*if (isDSiMode() && ms().consoleModel >= 2) {
		gui().addPage(twlfirmPage);
	}*/
	gui()
		.onExit(defaultExitHandler)
		// Prep and show the first page.
		.show();
	//	stop();
	while (1) {
		if (!gui().isExited()) {
			snd().playBgMusic(ms().settingsMusic);
		}

		gui().draw();
		do
		{
			if (currentMacroMode) {
				titleDisplayLength++;
				if (titleDisplayLength >= 60*8) titleDisplayLength = 0;
				if (titleDisplayLength == 60*4 || titleDisplayLength == 0) gui().draw();
			}
			scanKeys();
			pressed = keysDownRepeat();
			if ((pressed & KEY_LID) && ms().sleepMode) {
				customSleep();
			}
			touchRead(&touch);
			swiWaitForVBlank();
		} while (!pressed);

		gui().processInputs(pressed, touch);
	}

	return 0;
}
