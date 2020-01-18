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
#include <cstdio>
#include <fat.h>
#include <sys/stat.h>
#include <limits.h>
#include <variant>
#include <string.h>
#include <unistd.h>
#include <maxmod9.h>
#include "common/gl2d.h"

#include "autoboot.h"

#include "graphics/graphics.h"

#include "graphics/fontHandler.h"

#include "common/nds_loader_arm9.h"
#include "common/inifile.h"
#include "common/fileCopy.h"
#include "common/nitrofs.h"
#include "common/bootstrappaths.h"
#include "common/dsimenusettings.h"
#include "common/cardlaunch.h"
#include "common/flashcard.h"
#include "settingspage.h"
#include "settingsgui.h"
#include "language.h"
#include "gbarunner2settings.h"
#include "bootstrapsettings.h"

#include "soundeffect.h"

#include "sr_data_srllastran.h"			 // For rebooting into the game (NTR-mode touch screen)
#include "sr_data_srllastran_twltouch.h" // For rebooting into the game (TWL-mode touch screen)
#include "common/systemdetails.h"

#define DSI_SYSTEM_UI_DIRECTORY "/_nds/TWiLightMenu/dsimenu/themes/"
#define _3DS_SYSTEM_UI_DIRECTORY "/_nds/TWiLightMenu/3dsmenu/themes/"

#define AK_SYSTEM_UI_DIRECTORY "/_nds/TWiLightMenu/akmenu/themes/"
#define R4_SYSTEM_UI_DIRECTORY "/_nds/TWiLightMenu/r4menu/themes/"

static int currentTheme = 0;

std::vector<std::string> akThemeList;
std::vector<std::string> r4ThemeList;
std::vector<std::string> dsiThemeList;
std::vector<std::string> _3dsThemeList;

bool renderScreens = false;
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

const char *unlaunchAutoLoadID = "AutoLoadInfo";
char hiyaNdsPath[14] = {'s','d','m','c',':','/','h','i','y','a','.','d','s','i'};

int subscreenmode = 0;

touchPosition touch;

using namespace std;

//---------------------------------------------------------------------------------
void stop(void)
{
	//---------------------------------------------------------------------------------
	while (1)
	{
		swiWaitForVBlank();
	}
}

char filePath[PATH_MAX];

//---------------------------------------------------------------------------------
void doPause(void)
{
	//---------------------------------------------------------------------------------
	printf("Press start...\n");
	while (1)
	{
		scanKeys();
		if (keysDown() & KEY_START)
			break;
	}
	scanKeys();
}

std::string ReplaceAll(std::string str, const std::string &from, const std::string &to)
{
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos)
	{
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
	}
	return str;
}

void launchSystemSettings()
{
	fifoSendValue32(FIFO_USER_01, 1); // Fade out sound
	for (int i = 0; i < 25; i++)
		swiWaitForVBlank();
	snd().stopBgMusic();
	fifoSendValue32(FIFO_USER_01, 0); // Cancel sound fade out
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
	fifoSendValue32(FIFO_USER_08, 1); // Reboot TWiLight Menu++ for TWL_FIRM changes to take effect
	for (int i = 0; i < 15; i++)
		swiWaitForVBlank();
}*/

void loadMainMenu()
{
	runNdsFile("/_nds/TWiLightMenu/mainmenu.srldr", 0, NULL, true, false, false, true, true);
}

void loadROMselect()
{
	if (ms().theme == 3)
	{
		runNdsFile("/_nds/TWiLightMenu/akmenu.srldr", 0, NULL, true, false, false, true, true);
	}
	else if (ms().theme == 2)
	{
		runNdsFile("/_nds/TWiLightMenu/r4menu.srldr", 0, NULL, true, false, false, true, true);
	}
	else
	{
		runNdsFile("/_nds/TWiLightMenu/dsimenu.srldr", 0, NULL, true, false, false, true, true);
	}
}


void loadDSiThemeList()
{
	DIR *dir;
	struct dirent *ent;
	std::string themeDir;
	if ((dir = opendir(DSI_SYSTEM_UI_DIRECTORY)) != NULL)
	{
		// print all the files and directories within directory
		while ((ent = readdir(dir)) != NULL)
		{
			// Reallocation here, but prevents our vector from being filled with

			themeDir = ent->d_name;
			if (themeDir == ".." || themeDir == "..." || themeDir == ".") continue;

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
	if ((dir = opendir(_3DS_SYSTEM_UI_DIRECTORY)) != NULL)
	{
		// print all the files and directories within directory
		while ((ent = readdir(dir)) != NULL)
		{
			// Reallocation here, but prevents our vector from being filled with

			themeDir = ent->d_name;
			if (themeDir == ".." || themeDir == "..." || themeDir == ".") continue;

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
	if ((dir = opendir(AK_SYSTEM_UI_DIRECTORY)) != NULL)
	{
		// print all the files and directories within directory
		while ((ent = readdir(dir)) != NULL)
		{
			// Reallocation here, but prevents our vector from being filled with

			themeDir = ent->d_name;
			if (themeDir == ".." || themeDir == "..." || themeDir == ".") continue;

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
	if ((dir = opendir(R4_SYSTEM_UI_DIRECTORY)) != NULL)
	{
		// print all the files and directories within directory 
		while ((ent = readdir(dir)) != NULL)
		{
			// Reallocation here, but prevents our vector from being filled with

			themeDir = ent->d_name;
			if (themeDir == ".." || themeDir == "..." || themeDir == ".") continue;

			r4ThemeList.emplace_back(themeDir);
		}
		closedir(dir);
	}
}

std::optional<Option> opt_subtheme_select(Option::Int &optVal)
{
	switch (optVal.get())
	{
	case 0:
		return Option(STR_SUBTHEMESEL_DSI, STR_AB_SETSUBTHEME, Option::Str(&ms().dsi_theme), dsiThemeList);
	case 2:
		return Option(STR_SUBTHEMESEL_R4, STR_AB_SETRETURN, Option::Str(&ms().r4_theme), r4ThemeList);
	case 3:
		return Option(STR_SUBTHEMESEL_AK, STR_AB_SETRETURN, Option::Str(&ms().ak_theme), akThemeList);
	case 1:
		return Option(STR_SUBTHEMESEL_3DS, STR_AB_SETSUBTHEME, Option::Str(&ms()._3ds_theme), _3dsThemeList);
	default:
		return nullopt;
	}
}

//bool twlFirmChanged = false;

void defaultExitHandler()
{
	/*if (twlFirmChanged) {
		applyTwlFirmSettings();
		rebootTWLMenuPP();
	}*/
	flashcardInit();
	if (ms().showMainMenu)
	{
		loadMainMenu();
	}
	loadROMselect();
}
void opt_reset_subtheme(int prev, int next)
{
	if (prev != next)
	{
		ms().subtheme = 0;
	}
}

void opt_reboot_system_menu()
{
	gui().onExit(launchSystemSettings).saveAndExit(currentTheme);
}

void opt_hiya_autoboot_toggle(bool prev, bool next)
{
	if (!next)
	{
		if (remove("sd:/hiya/autoboot.bin") != 0)
		{
		}
		else
		{
			hiyaAutobootFound = false;
		}
	}
	else
	{
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
	*(u8*)(0x023FFD00) = (next ? 0x13 : 0x12);		// On/Off
}

/*void opt_twlFirm_changed(int prev, int next)
{
	twlFirmChanged = true;
}*/

inline bool between_incl(int x, int a, int b)
{
	return (a <= x && x >= b);
}

//---------------------------------------------------------------------------------
int main(int argc, char **argv)
{
	//---------------------------------------------------------------------------------

	// Turn on screen backlights if they're disabled
	powerOn(PM_BACKLIGHT_TOP);
	powerOn(PM_BACKLIGHT_BOTTOM);
#pragma region init

	sys().initFilesystem();
	sys().flashcardUsed();
	ms();
	defaultExceptionHandler();

	// Read user name
	char *username = (char *)PersonalData->name;

	// text
	for (int i = 0; i < 10; i++)
	{
		if (username[i * 2] == 0x00)
			username[i * 2 / 2] = 0;
		else
			username[i * 2 / 2] = username[i * 2];
	}

	if (!sys().fatInitOk())
	{
		graphicsInit();
		fontInit();
		fadeType = true;
		printSmall(true, 28, 1, username);
		printSmall(false, 4, 4, "fatinitDefault failed!");
		stop();
	}

	std::string filename;

	ms().loadSettings();
	gs().loadSettings();
	bs().loadSettings();
	loadAkThemeList();
	loadR4ThemeList();
	load3DSThemeList();
	loadDSiThemeList();
	swiWaitForVBlank();

	snd().init();
	keysSetRepeat(25, 5);
	
	bool sdAccessible = false;
	if (access("sd:/", F_OK) == 0) {
		sdAccessible = true;
	}

	graphicsInit();
	fontInit();
	langInit();
	fadeType = true;

	srand(time(NULL));

	if (!sys().flashcardUsed() && ms().consoleModel < 2)
	{
		if (access("sd:/hiya/autoboot.bin", F_OK) == 0)
			hiyaAutobootFound = true;
		else
			hiyaAutobootFound = false;
	}

	currentTheme = ms().theme;

	int pressed = 0;
#pragma endregion

	// consoleDemoInit();
	SettingsPage guiPage(STR_GUI_SETTINGS);

	using TLanguage = DSiMenuPlusPlusSettings::TLanguage;
	using TAKScrollSpeed = DSiMenuPlusPlusSettings::TScrollSpeed;
	guiPage
		.option(STR_MAINMENU, STR_DESCRIPTION_MAINMENU, Option::Bool(&ms().showMainMenu), {STR_SHOW, STR_HIDE}, {true, false})

		// Theme
		.option(STR_THEME,
				STR_DESCRIPTION_THEME_1,
				Option::Int(&ms().theme, opt_subtheme_select, opt_reset_subtheme),
				{"DSi", "3DS", "R4", "Acekard", "SEGA Saturn"},
				{0, 1, 2, 3, 4})
		.option(STR_DSIMUSIC,
				STR_DESCRIPTION_DSIMUSIC,
				Option::Int(&ms().dsiMusic),
				{STR_OFF, "Regular", "DSi Shop", "Theme"},
				{0, 1, 2, 3});

	if (isDSiMode() && sdAccessible) {
		guiPage.option(STR_REFERSD, STR_DESCRIPTION_REFERSD, Option::Bool(&ms().showMicroSd), {"microSD Card", "SD Card"}, {true, false});
	}

	guiPage
		.option(STR_UPDATE_RECENTLY_PLAYED_LIST, STR_DESCRIPTION_UPDATE_RECENTLY_PLAYED_LIST, Option::Bool(&ms().updateRecentlyPlayedList), {STR_YES, STR_NO}, {true, false})
		.option(STR_SORT_METHOD, STR_DESCRIPTION_SORT_METHOD, Option::Int(&ms().sortMethod), {STR_ALPHABETICAL, STR_RECENT, STR_MOST_PLAYED, STR_FILE_TYPE, STR_CUSTOM}, {0, 1, 2, 3, 4})
		.option(STR_DIRECTORIES, STR_DESCRIPTION_DIRECTORIES_1, Option::Bool(&ms().showDirectories), {STR_SHOW, STR_HIDE}, {true, false})
		.option(STR_SHOW_HIDDEN, STR_DESCRIPTION_SHOW_HIDDEN_1, Option::Bool(&ms().showHidden), {STR_SHOW, STR_HIDE}, {true, false})
		.option(STR_BOXART, STR_DESCRIPTION_BOXART_1, Option::Bool(&ms().showBoxArt), {STR_SHOW, STR_HIDE}, {true, false});
	if (isDSiMode()) {
		guiPage.option(STR_BOXARTMEM, STR_DESCRIPTION_BOXARTMEM, Option::Bool(&ms().cacheBoxArt), {STR_YES, STR_NO}, {true, false});
	}
	guiPage.option(STR_ANIMATEDSIICONS, STR_DESCRIPTION_ANIMATEDSIICONS_1, Option::Bool(&ms().animateDsiIcons), {STR_YES, STR_NO}, {true, false})
		.option(STR_12_HOUR_CLOCK, STR_DESCRIPTION_12_HOUR_CLOCK, Option::Bool(&ms().show12hrClock), {STR_YES, STR_NO}, {true, false})
		.option(STR_AK_SCROLLSPEED, STR_DESCRIPTION_AK_SCROLLSPEED, Option::Int(&ms().ak_scrollSpeed), {"Fast", "Medium", "Slow"},
				{TAKScrollSpeed::EScrollFast, TAKScrollSpeed::EScrollMedium, TAKScrollSpeed::EScrollSlow})
		.option(STR_AK_ZOOMING_ICON, STR_DESCRIPTION_AK_ZOOMING_ICON, Option::Bool(&ms().ak_zoomIcons), {STR_ON, STR_OFF}, {true, false});

	SettingsPage filetypePage(STR_FILETYPE_SETTINGS);

	filetypePage
		.option("NDS ROMs", STR_DESCRIPTION_SHOW_NDS, Option::Bool(&ms().showNds), {STR_SHOW, STR_HIDE}, {true, false})
		.option("Videos (.RVID and .MP4)", STR_DESCRIPTION_SHOW_VIDEO, Option::Bool(&ms().showRvid), {STR_SHOW, STR_HIDE}, {true, false})
		.option("NES/FDS ROMs", STR_DESCRIPTION_SHOW_NES, Option::Bool(&ms().showNes), {STR_SHOW, STR_HIDE}, {true, false})
		.option("GameBoy (Color) ROMs", STR_DESCRIPTION_SHOW_GB, Option::Bool(&ms().showGb), {STR_SHOW, STR_HIDE}, {true, false})
		.option("Sega MS/GG ROMs", STR_DESCRIPTION_SHOW_SMS, Option::Bool(&ms().showSmsGg), {STR_SHOW, STR_HIDE}, {true, false})
		.option("Sega MD/Gen ROMs", STR_DESCRIPTION_SHOW_MD, Option::Bool(&ms().showMd), {STR_SHOW, STR_HIDE}, {true, false})
		.option("SNES/SFC ROMs", STR_DESCRIPTION_SHOW_SNES, Option::Bool(&ms().showSnes), {STR_SHOW, STR_HIDE}, {true, false});

	SettingsPage gbar2Page(STR_GBARUNNER2_SETTINGS);

	gbar2Page.option(sdAccessible ? "Slot-1 SD: DLDI access" : "DLDI access", STR_DESCRIPTION_DLDIACCESS, Option::Bool(&ms().gbar2DldiAccess), {"ARM7", "ARM9"}, {true, false})
			.option("Use bottom screen", STR_DESCRIPTION_USEBOTTOMSCREEN, Option::Bool(&gs().useBottomScreen), {STR_YES, STR_NO}, {true, false})
			.option("Center and mask", STR_DESCRIPTION_CENTERANDMASK, Option::Bool(&gs().centerMask), {STR_ON, STR_OFF}, {true, false})
			.option("DS main memory i-cache", STR_DESCRIPTION_MAINMEMICACHE, Option::Bool(&gs().mainMemICache), {STR_ON, STR_OFF}, {true, false})
			.option("WRAM i-cache", STR_DESCRIPTION_WRAMICACHE, Option::Bool(&gs().wramICache), {STR_ON, STR_OFF}, {true, false})
			.option("BIOS intro", STR_DESCRIPTION_BIOSINTRO, Option::Bool(&gs().skipIntro), {STR_OFF, STR_ON}, {true, false});

	SettingsPage gamesPage(STR_GAMESAPPS_SETTINGS);

	if (isDSiMode() && ms().consoleModel >= 2 && !sys().arm7SCFGLocked())
	{
		gamesPage.option(STR_ASPECTRATIO, STR_DESCRIPTION_ASPECTRATIO, Option::Bool(&ms().wideScreen), {"16:10 (Widescreen)", "4:3 (Full Screen)"}, {true, false});
	}

	if (!isDSiMode() && sys().isRegularDS())
	{
		gamesPage.option(STR_USEGBARUNNER2, STR_DESCRIPTION_GBARUNNER2_1, Option::Bool(&ms().useGbarunner), {STR_YES, STR_NO}, {true, false});
	}

	using TRunIn = DSiMenuPlusPlusSettings::TRunIn;
	using TROMReadLED = BootstrapSettings::TROMReadLED;

	if (isDSiMode()) {
		gamesPage.option(STR_RUNIN,
						STR_DESCRIPTION_RUNIN_1,
						Option::Int(&ms().bstrap_dsiMode),
						{"DS mode", "DSi mode", "DSi mode (Forced)"},
						{TRunIn::EDSMode, TRunIn::EDSiMode, TRunIn::EDSiModeForced});
	}

	if (REG_SCFG_EXT != 0) {
		gamesPage.option(STR_CPUSPEED,
				STR_DESCRIPTION_CPUSPEED_1,
				Option::Bool(&ms().boostCpu),
				{"133 MHz (TWL)", "67 MHz (NTR)"},
				{true, false})
		.option(STR_VRAMBOOST, STR_DESCRIPTION_VRAMBOOST_1, Option::Bool(&ms().boostVram), {STR_ON, STR_OFF}, {true, false});
		if (!sys().arm7SCFGLocked()) {
			if (isDSiMode()) {
				gamesPage.option(sdAccessible ? "Slot-1 SD: "+STR_USEBOOTSTRAP : STR_USEBOOTSTRAP, STR_DESCRIPTION_USEBOOTSTRAP, Option::Bool(&ms().useBootstrap), {STR_YES, STR_NO}, {true, false});
			} else {
				gamesPage.option(STR_USEBOOTSTRAP+" (B4DS)", STR_DESCRIPTION_USEBOOTSTRAP, Option::Bool(&ms().useBootstrap), {STR_YES, STR_NO}, {true, false});
			}
			gamesPage.option(STR_FCSAVELOCATION, STR_DESCRIPTION_FCSAVELOCATION, Option::Bool(&ms().fcSaveOnSd), {"Console's SD", "Slot-1 SD"}, {true, false});
		}
		if (sdAccessible) {
			gamesPage.option(STR_FORCESLEEPPATCH, STR_DESCRIPTION_FORCESLEEPPATCH, Option::Bool(&ms().forceSleepPatch), {STR_ON, STR_OFF}, {true, false})
				.option(STR_SLOT1SCFGUNLOCK, STR_DESCRIPTION_SLOT1SCFGUNLOCK, Option::Bool(&ms().slot1SCFGUnlock), {STR_ON, STR_OFF}, {true, false});
		}
	} else {
		gamesPage.option(STR_USEBOOTSTRAP+" (B4DS)", STR_DESCRIPTION_USEBOOTSTRAP, Option::Bool(&ms().useBootstrap), {STR_YES, STR_NO}, {true, false});
	}

	if (isDSiMode() && !sys().arm7SCFGLocked()) {
		gamesPage.option(STR_SLOT1LAUNCHMETHOD, STR_DESCRIPTION_SLOT1LAUNCHMETHOD_1, Option::Bool(&ms().slot1LaunchMethod), {STR_DIRECT, STR_REBOOT},
				{true, false});
	}

	if (!sys().isRegularDS())
	{
		gamesPage.option(STR_SNDFREQ, STR_DESCRIPTION_SNDFREQ_1, Option::Bool(&ms().soundFreq), {"47.61 kHz", "32.73 kHz"}, {true, false});
	}

	if (isDSiMode() && ms().consoleModel < 2)
	{
		if (sdAccessible) {
			gamesPage.option(STR_ROMREADLED, STR_DESCRIPTION_ROMREADLED_1, Option::Int(&bs().romreadled), {STR_NONE, "WiFi", STR_POWER, STR_CAMERA},
							 {TROMReadLED::ELEDNone, TROMReadLED::ELEDWifi, TROMReadLED::ELEDPower, TROMReadLED::ELEDCamera});
		}
		gamesPage.option(STR_PRECISEVOLUMECTRL, STR_DESCRIPTION_PRECISEVOLUMECTRL, Option::Bool(&bs().preciseVolumeControl), {STR_ON, STR_OFF},
				{true, false});
		if (sdAccessible) {
			gamesPage.option(STR_DSIWAREBOOTER, STR_DESCRIPTION_DSIWAREBOOTER, Option::Bool(&ms().dsiWareBooter), {"nds-bootstrap", "Unlaunch"},
					{true, false});
		}
	}

	gamesPage
		.option(STR_BOOTSTRAP, STR_DESCRIPTION_BOOTSTRAP_1,
				Option::Bool(&ms().bootstrapFile),
				{STR_NIGHTLY, STR_RELEASE},
				{true, false})

		.option(STR_DEBUG, STR_DESCRIPTION_DEBUG_1, Option::Bool(&bs().debug), {STR_ON, STR_OFF}, {true, false})
		.option(STR_LOGGING, STR_DESCRIPTION_LOGGING_1, Option::Bool(&bs().logging), {STR_ON, STR_OFF}, {true, false});

	SettingsPage miscPage(STR_MISC_SETTINGS);

	using TLanguage = DSiMenuPlusPlusSettings::TLanguage;
	using TAKScrollSpeed = DSiMenuPlusPlusSettings::TScrollSpeed;
	miscPage
		// Language
		.option(STR_LANGUAGE,
				STR_DESCRIPTION_LANGUAGE_1,
				Option::Int(&ms().guiLanguage),
				{STR_SYSTEM,
				 "Japanese",
				 "English",
				 "French",
				 "German",
				 "Italian",
				 "Spanish",
				 "Chinese"},
				{TLanguage::ELangDefault,
				 TLanguage::ELangJapanese,
				 TLanguage::ELangEnglish,
				 TLanguage::ELangFrench,
				 TLanguage::ELangGerman,
				 TLanguage::ELangItalian,
				 TLanguage::ELangSpanish,
				 TLanguage::ELangChinese})
		.option(STR_COLORMODE,
				STR_DESCRIPTION_COLORMODE,
				Option::Int(&ms().colorMode),
				{STR_REGULAR,
				 "B&W/Greyscale"},
				{0,
				 1});
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

	if (isDSiMode() && sdAccessible) {
		miscPage.option(STR_SDREMOVALDETECTION,
				STR_DESCRIPTION_SDREMOVALDETECTION,
				Option::Bool(&ms().sdRemoveDetect),
				{STR_ON, STR_OFF},
				{true, false});
	}

	if (isDSiMode() && sdAccessible && !sys().arm7SCFGLocked()) {
		miscPage.option(STR_S1SDACCESS,
				STR_DESCRIPTION_S1SDACCESS_1,
				Option::Bool(&ms().secondaryAccess),
				{STR_ON, STR_OFF},
				{true, false});
	}

	if (isDSiMode() && ms().consoleModel < 2) {
		miscPage.option(STR_WIFILED,
				STR_DESCRIPTION_WIFILED,
				Option::Bool(&ms().wifiLed, opt_wifiLed_toggle),
				{STR_ON, STR_OFF},
				{true, false});
	}

	miscPage
		.option(STR_LASTPLAYEDROM, STR_DESCRIPTION_LASTPLAYEDROM_1, Option::Bool(&ms().autorun), {STR_YES, STR_NO}, {true, false})
		.option(STR_DSISPLASH, STR_DESCRIPTION_DSISPLASH, Option::Bool(&ms().dsiSplash), {STR_SHOW, STR_HIDE}, {true, false})
		.option(STR_HSMSG, STR_DESCRIPTION_HSMSG, Option::Bool(&ms().hsMsg), {STR_SHOW, STR_HIDE}, {true, false})
		.option(STR_DSIMENUPPLOGO, STR_DESCRIPTION_DSIMENUPPLOGO_1, Option::Bool(&ms().showlogo), {STR_SHOW, STR_HIDE}, {true, false});

	if (isDSiMode() && sdAccessible && ms().consoleModel < 2) {
		miscPage
			.option(STR_SYSREGION,
				STR_DESCRIPTION_SYSREGION_1,
				Option::Int(&ms().sysRegion),
				{"Auto", "JAP", "USA", "EUR", "AUS", "CHN", "KOR"},
				{-1, 0, 1, 2, 3, 4, 5})
			.option(STR_LAUNCHERAPP,
				STR_DESCRIPTION_LAUNCHERAPP_1,
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
				"00000008.app"},
				{-1, 0, 1, 2, 3, 4, 5, 6, 7, 8});
	}

	if (isDSiMode() && ms().consoleModel < 2)
	{
		// Actions do not have to bound to an object.
		// See for exam here we have bound an option to
		// hiyaAutobootFound.

		// We are also using the changed callback to write
		// or delete the hiya autoboot file.
		miscPage
			.option(STR_DEFAULT_LAUNCHER, STR_DESCRIPTION_DEFAULT_LAUNCHER_1, Option::Bool(&hiyaAutobootFound, opt_hiya_autoboot_toggle), {"TWiLight Menu++", "System Menu"}, {true, false})
			.option(STR_SYSTEMSETTINGS, STR_DESCRIPTION_SYSTEMSETTINGS_1, Option::Nul(opt_reboot_system_menu), {}, {});
	}
	
	/*SettingsPage twlfirmPage(STR_TWLFIRM_SETTINGS);
	if (isDSiMode() && ms().consoleModel >= 2) {
		twlfirmPage
			.option(STR_SCREENSCALESIZE, STR_DESCRIPTION_SCREENSCALESIZE, Option::Int(&ms().screenScaleSize, opt_twlFirm_changed), {"1x/1.25x", "1.5x"}, {0, 1});
	}*/
	
	gui()
		.addPage(guiPage)
		.addPage(filetypePage)
		.addPage(gbar2Page)
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
	while (1)
	{
		if (!gui().isExited() && currentTheme != 4)
		{
			snd().playBgMusic();
		}

		gui().draw();
		do
		{
			scanKeys();
			pressed = keysDownRepeat();
			touchRead(&touch);
			swiWaitForVBlank();
		} while (!pressed);

		gui().processInputs(pressed, touch, currentTheme);
	}

	return 0;
}
