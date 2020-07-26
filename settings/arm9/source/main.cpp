#include <nds.h>
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
#include "common/fileCopy.h"
#include "common/nitrofs.h"
#include "common/bootstrappaths.h"
#include "common/dsimenusettings.h"
#include "common/cardlaunch.h"
#include "common/flashcard.h"
#include "common/tonccpy.h"
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

#define UNLAUNCH_BG_DIRECTORY "/_nds/TWiLightMenu/unlaunch/backgrounds/"

bool useTwlCfg = false;

int currentTheme = 0;
static int previousDSiWareExploit = 0;
static int previousSysRegion = 0;

std::vector<std::string> akThemeList;
std::vector<std::string> r4ThemeList;
std::vector<std::string> dsiThemeList;
std::vector<std::string> _3dsThemeList;
std::vector<std::string> unlaunchBgList;

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

bool extention(const std::string& filename, const char* ext) {
	return (strcasecmp(filename.c_str() + filename.size() - strlen(ext), ext) == 0);
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
	/*if (ms().theme == 3)
	{
		runNdsFile("/_nds/TWiLightMenu/akmenu.srldr", 0, NULL, true, false, false, true, true);
	}
	else*/ if (ms().theme == 2)
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

void loadUnlaunchBgList()
{
	DIR *dir;
	struct dirent *ent;
	std::string themeDir;
	if ((dir = opendir(UNLAUNCH_BG_DIRECTORY)) != NULL)
	{
		// print all the files and directories within directory 
		while ((ent = readdir(dir)) != NULL)
		{
			// Reallocation here, but prevents our vector from being filled with

			themeDir = ent->d_name;
			if (themeDir == ".." || themeDir == "..." || themeDir == ".") continue;

			if (extention(themeDir, ".gif")) {
				unlaunchBgList.emplace_back(themeDir);
			}
		}
		closedir(dir);
	}
}

std::optional<Option> opt_subtheme_select(Option::Int &optVal)
{
	switch (optVal.get())
	{
	case 0:
		return Option(STR_SKINSEL_DSI, STR_AB_SETSKIN, Option::Str(&ms().dsi_theme), dsiThemeList);
	case 2:
		return Option(STR_SKINSEL_R4, STR_AB_SETSKIN, Option::Str(&ms().r4_theme), r4ThemeList);
	case 3:
		return Option(STR_SKINSEL_WOOD, STR_AB_SETSKIN, Option::Str(&ms().ak_theme), akThemeList);
	case 1:
		return Option(STR_SKINSEL_3DS, STR_AB_SETSKIN, Option::Str(&ms()._3ds_theme), _3dsThemeList);
	default:
		return nullopt;
	}
}

std::optional<Option> opt_bg_select(Option::Int &optVal)
{
	return Option(STR_BGSEL_UNLAUNCH, STR_AB_SETBG, Option::Str(&ms().unlaunchBg), unlaunchBgList);
}

//bool twlFirmChanged = false;

void defaultExitHandler()
{
	/*if (twlFirmChanged) {
		applyTwlFirmSettings();
		rebootTWLMenuPP();
	}*/

	if (previousDSiWareExploit != ms().dsiWareExploit
	 || previousSysRegion != ms().sysRegion)
	{
		u32 currentSrBackendId[2] = {0};
		u8 sysValue = 0;

		switch (ms().dsiWareExploit) {
			case 1:
			default:
				currentSrBackendId[0] = 0x4B344441;		// SUDOKU
				break;
			case 2:
				currentSrBackendId[0] = 0x4B513941;		// Legend of Zelda: Four Swords
				break;
			case 3:
				currentSrBackendId[0] = 0x4B464441;		// Fieldrunners
				break;
			case 4:
				currentSrBackendId[0] = 0x4B475241;		// Guitar Rock Tour
				break;
			case 5:
				currentSrBackendId[0] = 0x4B475541;		// Flipnote Studio
				break;
			case 6:
				currentSrBackendId[0] = 0x4B554E41;		// UNO
				break;
			case 7:
				currentSrBackendId[0] = 0x484E4941;		// Nintendo DSi Camera
				break;
		}
		switch (ms().sysRegion) {
			case 0:
				sysValue = 0x4A;		// JPN
				break;
			case 1:
				sysValue = 0x45;		// USA
				break;
			case 2:
				sysValue = (ms().dsiWareExploit==7 ? 0x50 : 0x56);		// EUR
				break;
			case 3:
				sysValue = (ms().dsiWareExploit==7 ? 0x55 : 0x56);		// AUS
				break;
			case 4:
				sysValue = 0x43;		// CHN
				break;
			case 5:
				sysValue = 0x4B;		// KOR
				break;
		}
		tonccpy(&currentSrBackendId, &sysValue, 1);
		currentSrBackendId[1] = (ms().dsiWareExploit==7 ? 0x00030005 : 0x00030004);

		if (ms().dsiWareExploit > 0) {
			FILE* file = fopen("sd:/_nds/nds-bootstrap/srBackendId.bin", "wb");
			if (file) {
				fwrite(&currentSrBackendId, sizeof(u32), 2, file);
			}
			fclose(file);
		} else {
			remove("sd:/_nds/nds-bootstrap/srBackendId.bin");
		}
	}

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
	*(u8*)(0x023FFD00) = (next ? 0x13 : 0);		// On/Off
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
//#pragma region init

	sys().initFilesystem();
	sys().flashcardUsed();
	ms();
	defaultExceptionHandler();

	useTwlCfg = (isDSiMode() && (*(u8*)0x02000400 & 0x0F) && (*(u8*)0x02000401 == 0) && (*(u8*)0x02000402 == 0) && (*(u8*)0x02000404 == 0));

	if (!sys().fatInitOk())
	{
		// Read user name
		char usernameRendered[16];
		char *username = (useTwlCfg ? (char*)0x02000448 : (char*)PersonalData->name);

		// text
		for (int i = 0; i < 10; i++)
		{
			if (username[i * 2] == 0x00)
				usernameRendered[i * 2 / 2] = 0;
			else
				usernameRendered[i * 2 / 2] = username[i * 2];
		}

		graphicsInit();
		fontInit();
		fadeType = true;
		printSmall(true, 28, 1, usernameRendered);
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
	loadUnlaunchBgList();
	swiWaitForVBlank();

	snd().init();
	keysSetRepeat(25, 5);
	
	bool widescreenFound = false;
	bool sdAccessible = (access("sd:/", F_OK) == 0);
	if (sdAccessible) {
		widescreenFound = ((access("sd:/_nds/TWiLightMenu/TwlBg/Widescreen.cxi", F_OK) == 0) && (ms().consoleModel >= 2) && (!sys().arm7SCFGLocked()));
	}
	bool fatAccessible = (access("fat:/", F_OK) == 0);

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
	previousDSiWareExploit = ms().dsiWareExploit;
	previousSysRegion = ms().sysRegion;

	int pressed = 0;
//#pragma endregion

	// consoleDemoInit();
	SettingsPage guiPage(STR_GUI_SETTINGS);

	using TLanguage = TWLSettings::TLanguage;
	//using TAKScrollSpeed = TWLSettings::TScrollSpeed;
	guiPage
		.option(STR_FRAMERATE, STR_DESCRIPTION_FRAMERATE, Option::Int(&ms().fps), {"15FPS", "20FPS", "24FPS", "30FPS", "50FPS", "60FPS"}, {15, 20, 24, 30, 50, 60})
		.option(STR_DSCLASSICMENU, STR_DESCRIPTION_DSCLASSICMENU, Option::Bool(&ms().showMainMenu), {STR_YES, STR_NO}, {true, false})
		.option("DSi/Saturn: SELECT", STR_DESCRIPTION_SELECTBUTTONOPTION, Option::Bool(&ms().showSelectMenu), {STR_SELECT_MENU, STR_DS_CLASSIC_MENU}, {true, false})

		// Theme
		.option(STR_THEME,
				STR_DESCRIPTION_THEME_1,
				Option::Int(&ms().theme, opt_subtheme_select, opt_reset_subtheme),
				/*{STR_NINTENDO_DSI, STR_NINTENDO_3DS, STR_SEGA_SATURN, STR_HOMEBREW_LAUNCHER, STR_WOOD_UI, STR_R4_ORIGINAL},
				{0, 1, 4, 5, 3, 2})*/
				{STR_NINTENDO_DSI, STR_NINTENDO_3DS, STR_SEGA_SATURN, STR_HOMEBREW_LAUNCHER, STR_R4_ORIGINAL},
				{0, 1, 4, 5, 2})
		.option(STR_DSIMUSIC,
				STR_DESCRIPTION_DSIMUSIC,
				Option::Int(&ms().dsiMusic),
				{STR_OFF, STR_REGULAR, STR_DSI_SHOP, STR_THEME, STR_CLASSIC},
				{0, 1, 2, 3, 4});

	if (isDSiMode() && sdAccessible) {
		guiPage.option(STR_REFERSD, STR_DESCRIPTION_REFERSD, Option::Bool(&ms().showMicroSd), {STR_MICRO_SD_CARD, STR_SD_CARD}, {true, false});
	}

	guiPage
		.option(STR_UPDATE_RECENTLY_PLAYED_LIST, STR_DESCRIPTION_UPDATE_RECENTLY_PLAYED_LIST, Option::Bool(&ms().updateRecentlyPlayedList), {STR_YES, STR_NO}, {true, false})
		.option(STR_SORT_METHOD, STR_DESCRIPTION_SORT_METHOD, Option::Int(&ms().sortMethod), {STR_ALPHABETICAL, STR_RECENT, STR_MOST_PLAYED, STR_FILE_TYPE, STR_CUSTOM}, {0, 1, 2, 3, 4})
		.option(STR_DIRECTORIES, STR_DESCRIPTION_DIRECTORIES_1, Option::Bool(&ms().showDirectories), {STR_SHOW, STR_HIDE}, {true, false})
		.option(STR_SHOW_HIDDEN, STR_DESCRIPTION_SHOW_HIDDEN_1, Option::Bool(&ms().showHidden), {STR_SHOW, STR_HIDE}, {true, false})
		.option(STR_PREVENT_ROM_DELETION, STR_DESCRIPTION_PREVENT_ROM_DELETION_1, Option::Bool(&ms().preventDeletion), {STR_YES, STR_NO}, {true, false});
	if (isDSiMode()) {
		guiPage.option(STR_BOXART, STR_DESCRIPTION_BOXART_DSI, Option::Int(&ms().showBoxArt), {STR_NON_CACHED, STR_CACHED, STR_HIDE}, {1, 2, 0});
	} else {
		if(ms().showBoxArt == 2) // Reset to 1 if not in DSi mode
			ms().showBoxArt = 1;
		guiPage.option(STR_BOXART, STR_DESCRIPTION_BOXART, Option::Int(&ms().showBoxArt), {STR_SHOW, STR_HIDE}, {1, 0});
	}
	guiPage.option(STR_ANIMATEDSIICONS, STR_DESCRIPTION_ANIMATEDSIICONS_1, Option::Bool(&ms().animateDsiIcons), {STR_YES, STR_NO}, {true, false})
		.option(STR_CLOCK_SYSTEM, STR_DESCRIPTION_CLOCK_SYSTEM, Option::Bool(&ms().show12hrClock), {STR_12_HOUR, STR_24_HOUR}, {true, false})
		/*.option(STR_AK_SCROLLSPEED, STR_DESCRIPTION_AK_SCROLLSPEED, Option::Int(&ms().ak_scrollSpeed), {STR_FAST, STR_MEDIUM, STR_SLOW},
				{TAKScrollSpeed::EScrollFast, TAKScrollSpeed::EScrollMedium, TAKScrollSpeed::EScrollSlow})
		.option(STR_AK_ZOOMING_ICON, STR_DESCRIPTION_ZOOMING_ICON, Option::Bool(&ms().ak_zoomIcons), {STR_ON, STR_OFF}, {true, false})*/;

	SettingsPage emulationPage(STR_EMULATION_HB_SETTINGS);

	emulationPage
		.option(STR_NDS_ROMS, STR_DESCRIPTION_SHOW_NDS, Option::Bool(&ms().showNds), {STR_SHOW, STR_HIDE}, {true, false})
		.option(STR_VIDEOS, STR_DESCRIPTION_SHOW_VIDEO, Option::Bool(&ms().showRvid), {STR_SHOW, STR_HIDE}, {true, false})
		.option(STR_A26_ROMS, STR_DESCRIPTION_SHOW_A26, Option::Bool(&ms().showA26), {"StellaDS", STR_HIDE}, {true, false})
		.option(STR_NES_ROMS, STR_DESCRIPTION_SHOW_NES, Option::Bool(&ms().showNes), {"nesDS", STR_HIDE}, {true, false})
		.option(STR_GB_ROMS, STR_DESCRIPTION_SHOW_GB, Option::Bool(&ms().showGb), {"GameYob", STR_HIDE}, {true, false})
		.option(STR_SMS_ROMS, STR_DESCRIPTION_SHOW_SMS, Option::Bool(&ms().showSmsGg), {"S8DS", STR_HIDE}, {true, false})
		.option(STR_MD_ROMS, STR_DESCRIPTION_SHOW_MD, Option::Int(&ms().showMd), {STR_HIDE, "jEnesisDS", "PicoDriveTWL", STR_HYBRID}, {0, 1, 2, 3})
		.option(STR_SNES_ROMS, STR_DESCRIPTION_SHOW_SNES, Option::Bool(&ms().showSnes), {"SNEmulDS", STR_HIDE}, {true, false})
		.option(STR_PCE_ROMS, STR_DESCRIPTION_SHOW_PCE, Option::Bool(&ms().showPce), {"NitroGrafx", STR_HIDE}, {true, false});

	SettingsPage gbar2Page(STR_GBARUNNER2_SETTINGS);

	gbar2Page.option(sdAccessible ? STR_SLOT_1_DLDI_ACCESS : STR_DLDI_ACCESS, STR_DESCRIPTION_GBAR2_DLDIACCESS, Option::Bool(&ms().gbar2DldiAccess), {"ARM7", "ARM9"}, {true, false})
			.option(STR_USE_BOTTOM_SCREEN, STR_DESCRIPTION_USEBOTTOMSCREEN, Option::Bool(&gs().useBottomScreen), {STR_YES, STR_NO}, {true, false})
			.option(STR_CENTER_AND_MASK, STR_DESCRIPTION_CENTERANDMASK, Option::Bool(&gs().centerMask), {STR_ON, STR_OFF}, {true, false})
			.option(STR_SIMULATE_GBA_COLORS, STR_DESCRIPTION_GBACOLORS, Option::Bool(&gs().gbaColors), {STR_YES, STR_NO}, {true, false})
			.option(STR_DS_MAIN_MEMORY_I_CACHE, STR_DESCRIPTION_GBAR2_MAINMEMICACHE, Option::Bool(&gs().mainMemICache), {STR_ON, STR_OFF}, {true, false})
			.option(STR_WRAM_I_CACHE, STR_DESCRIPTION_GBAR2_WRAMICACHE, Option::Bool(&gs().wramICache), {STR_ON, STR_OFF}, {true, false})
			.option(STR_BIOS_INTRO, STR_DESCRIPTION_BIOSINTRO, Option::Bool(&gs().skipIntro), {STR_OFF, STR_ON}, {true, false});

	SettingsPage gamesPage(STR_GAMESAPPS_SETTINGS);

	if (widescreenFound)
	{
		gamesPage.option(STR_ASPECTRATIO, STR_DESCRIPTION_ASPECTRATIO, Option::Bool(&ms().wideScreen), {STR_WIDESCREEN, STR_FULLSCREEN}, {true, false});
	}

	if (isDSiMode() && sdAccessible)
	{
		gamesPage.option(STR_SMSGGINRAM, STR_DESCRIPTION_SMSGGINRAM, Option::Bool(&ms().smsGgInRam), {STR_YES, STR_NO}, {true, false});
	}

	if (!isDSiMode() && sys().isRegularDS())
	{
		gamesPage.option(STR_USEGBARUNNER2, STR_DESCRIPTION_GBARUNNER2_1, Option::Bool(&ms().useGbarunner), {STR_YES, STR_NO}, {true, false});
	}

	using TRunIn = TWLSettings::TRunIn;
	using TROMReadLED = BootstrapSettings::TROMReadLED;

	if (isDSiMode()) {
		gamesPage.option(STR_RUNIN,
						STR_DESCRIPTION_RUNIN_1,
						Option::Int(&ms().bstrap_dsiMode),
						{STR_DS_MODE, STR_DSI_MODE, STR_DSI_MODE_FORCED},
						{TRunIn::EDSMode, TRunIn::EDSiMode, TRunIn::EDSiModeForced});
	}

	if (REG_SCFG_EXT != 0) {
		gamesPage.option(STR_CPUSPEED,
				STR_DESCRIPTION_CPUSPEED_1,
				Option::Bool(&ms().boostCpu),
				{"133 MHz (TWL)", "67 MHz (NTR)"},
				{true, false})
		.option(STR_VRAMBOOST, STR_DESCRIPTION_VRAMBOOST_1, Option::Bool(&ms().boostVram), {STR_ON, STR_OFF}, {true, false});
		if (isDSiMode()) {
			gamesPage.option(sdAccessible ? STR_SLOT_1_SD+": "+STR_USEBOOTSTRAP : STR_USEBOOTSTRAP, STR_DESCRIPTION_USEBOOTSTRAP, Option::Bool(&ms().useBootstrap), {STR_YES, STR_NO}, {true, false});
			if (sdAccessible) {
				gamesPage.option(STR_FCSAVELOCATION, STR_DESCRIPTION_FCSAVELOCATION, Option::Bool(&ms().fcSaveOnSd), {STR_CONSOLE_SD, STR_SLOT_1_SD}, {true, false});
			}
		} else if (!isDSiMode() && fatAccessible) {
			gamesPage.option(STR_USEBOOTSTRAP+" (B4DS)", STR_DESCRIPTION_USEBOOTSTRAP, Option::Bool(&ms().useBootstrap), {STR_YES, STR_NO}, {true, false});
		}
		if (sdAccessible) {
			gamesPage.option(STR_FORCESLEEPPATCH, STR_DESCRIPTION_FORCESLEEPMODE, Option::Bool(&ms().forceSleepPatch), {STR_ON, STR_OFF}, {true, false})
				.option(STR_SLOT1SDACCESS, STR_DESCRIPTION_SLOT1SDACCESS, Option::Bool(&ms().slot1AccessSD), {STR_ON, STR_OFF}, {true, false})
				.option(STR_SLOT1SCFGUNLOCK, STR_DESCRIPTION_SLOT1SCFGUNLOCK, Option::Bool(&ms().slot1SCFGUnlock), {STR_ON, STR_OFF}, {true, false});
		}
	} else {
		gamesPage.option(STR_USEBOOTSTRAP+" (B4DS)", STR_DESCRIPTION_USEBOOTSTRAP, Option::Bool(&ms().useBootstrap), {STR_YES, STR_NO}, {true, false});
	}

	if (isDSiMode() && !sys().arm7SCFGLocked()) {
		if (ms().consoleModel == 0) {
			gamesPage.option(STR_SLOT1LAUNCHMETHOD, STR_DESCRIPTION_SLOT1LAUNCHMETHOD_1, Option::Int(&ms().slot1LaunchMethod), {STR_REBOOT, STR_DIRECT, "Unlaunch"},
				{0, 1, 2});
		} else {
			gamesPage.option(STR_SLOT1LAUNCHMETHOD, STR_DESCRIPTION_SLOT1LAUNCHMETHOD_1, Option::Int(&ms().slot1LaunchMethod), {STR_REBOOT, STR_DIRECT},
				{0, 1});
		}
	}

	if (!sys().isRegularDS())
	{
		gamesPage.option(STR_SNDFREQ, STR_DESCRIPTION_SNDFREQ_1, Option::Bool(&ms().soundFreq), {"47.61 kHz", "32.73 kHz"}, {true, false});
	}

	if (isDSiMode() && ms().consoleModel < 2)
	{
		if (sdAccessible) {
			gamesPage
				.option(STR_ROMREADLED, STR_DESCRIPTION_ROMREADLED_1, Option::Int(&bs().romreadled), {STR_NONE, "WiFi", STR_POWER, STR_CAMERA},
							 {TROMReadLED::ELEDNone, TROMReadLED::ELEDWifi, TROMReadLED::ELEDPower, TROMReadLED::ELEDCamera})
				.option(STR_DMAROMREADLED, STR_DESCRIPTION_DMAROMREADLED, Option::Int(&bs().dmaromreadled), {STR_SAME_AS_REG, STR_NONE, "WiFi", STR_POWER, STR_CAMERA},
							 {TROMReadLED::ELEDSame, TROMReadLED::ELEDNone, TROMReadLED::ELEDWifi, TROMReadLED::ELEDPower, TROMReadLED::ELEDCamera});
		}
		gamesPage.option(STR_PRECISEVOLUMECTRL, STR_DESCRIPTION_PRECISEVOLUMECTRL, Option::Bool(&bs().preciseVolumeControl), {STR_ON, STR_OFF},
				{true, false});
		if (sdAccessible) {
			gamesPage.option(STR_DSIWAREBOOTER, STR_DESCRIPTION_DSIWAREBOOTER, Option::Bool(&ms().dsiWareBooter), {"nds-bootstrap", "Unlaunch"},
					{true, false});
		}
	}

	if (isDSiMode()) {
		gamesPage.option(STR_EXPANDROMSPACE, (ms().consoleModel==0 ? STR_DESCRIPTION_EXPANDROMSPACE_DSI : STR_DESCRIPTION_EXPANDROMSPACE_3DS), Option::Int(&bs().extendedMemory), {STR_NO, STR_YES, STR_YES+"+512KB"}, {0, 1, 2});
		if (sdAccessible) {
			gamesPage.option(STR_CACHEBLOCKSIZE, STR_DESCRIPTION_CACHEBLOCKSIZE, Option::Bool(&bs().cacheBlockSize), {"16KB", "32KB"}, {false, true});
		}
		gamesPage.option(STR_SAVEFATTABLECACHE, STR_DESCRIPTION_SAVEFATTABLECACHE, Option::Bool(&bs().cacheFatTable), {STR_YES, STR_NO}, {true, false});
	}

	gamesPage
		.option(STR_BOOTSTRAP, STR_DESCRIPTION_BOOTSTRAP_1,
				Option::Bool(&ms().bootstrapFile),
				{STR_NIGHTLY, STR_RELEASE},
				{true, false})
		.option(STR_DEBUG, STR_DESCRIPTION_DEBUG_1, Option::Bool(&bs().debug), {STR_ON, STR_OFF}, {true, false})
		.option(STR_LOGGING, STR_DESCRIPTION_LOGGING_1, Option::Bool(&bs().logging), {STR_ON, STR_OFF}, {true, false});

	SettingsPage miscPage(STR_MISC_SETTINGS);

	using TLanguage = TWLSettings::TLanguage;
	miscPage
		// Language
		.option(STR_LANGUAGE,
				STR_DESCRIPTION_LANGUAGE_1,
				Option::Int(&ms().guiLanguage),
				{STR_SYSTEM,
				 "Dansk",
				 "Deutsch",
				 "English",
				 "Español",
				 "Français",
				 "Italiano",
				 "Polski",
				 "Português",
				 "Svenska",
				 "Türkçe",
				 "Русский",
				 "Українська",
				 "中文 (简体)",
				 "中文 (繁體)",
				 "日本語",
				 "한국어"},
				{TLanguage::ELangDefault,
				 TLanguage::ELangDanish,
				 TLanguage::ELangGerman,
				 TLanguage::ELangEnglish,
				 TLanguage::ELangSpanish,
				 TLanguage::ELangFrench,
				 TLanguage::ELangItalian,
				 TLanguage::ELangPolish,
				 TLanguage::ELangPortuguese,
				 TLanguage::ELangSwedish,
				 TLanguage::ELangTurkish,
				 TLanguage::ELangRussian,
				 TLanguage::ELangUkrainian,
				 TLanguage::ELangChineseS,
				 TLanguage::ELangChineseT,
				 TLanguage::ELangJapanese,
				 TLanguage::ELangKorean})
		.option(STR_GAMELANGUAGE,
				STR_DESCRIPTION_GAMELANGUAGE_1,
				Option::Int(&ms().gameLanguage),
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
				Option::Int(&ms().titleLanguage),
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
				 TLanguage::ELangJapanese,})
		.option(STR_COLORMODE,
				STR_DESCRIPTION_COLORMODE,
				Option::Int(&ms().colorMode),
				{STR_REGULAR,
				 STR_BW_GREYSCALE},
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
		miscPage
			.option(STR_UNLAUNCH_BG,
				STR_DESCRIPTION_UNLAUNCH_BG,
				Option::Int(&ms().subtheme, opt_bg_select, opt_reset_subtheme),
				{STR_PRESS_A},
				{0})
			.option(STR_SDREMOVALDETECTION,
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
		miscPage.option(STR_WIFI,
				STR_DESCRIPTION_WIFI,
				Option::Bool(&ms().wifiLed, opt_wifiLed_toggle),
				{STR_ON, STR_OFF},
				{true, false});
	}

	miscPage
		.option(STR_LASTPLAYEDROM, STR_DESCRIPTION_LASTPLAYEDROM_1, Option::Bool(&ms().autorun), {STR_YES, STR_NO}, {true, false})
		.option(STR_DSISPLASH, STR_DESCRIPTION_DSISPLASH, Option::Int(&ms().dsiSplash), {STR_WITHOUT_HS, STR_WITH_HS, STR_HIDE}, {1, 2, 0})
		.option(STR_DSIMENUPPLOGO, STR_DESCRIPTION_DSIMENUPPLOGO_1, Option::Bool(&ms().showlogo), {STR_SHOW, STR_HIDE}, {true, false});

	if (isDSiMode() && sdAccessible) {
		miscPage
			.option(STR_DSIWARE_EXPLOIT,
				STR_DESCRIPTION_DSIWARE_EXPLOIT,
				Option::Int(&ms().dsiWareExploit),
				{STR_NONE, "sudokuhax", "4swordshax", "fieldrunnerhax", "grtpwn", "ugopwn/Lenny", "UNO*pwn", "Memory Pit"},
				{0, 1, 2, 3, 4, 5, 6, 7})
			.option(STR_SYSREGION,
				STR_DESCRIPTION_SYSREGION_1,
				Option::Int(&ms().sysRegion),
				{STR_AUTO_HIYA_ONLY, "JPN", "USA", "EUR", "AUS", "CHN", "KOR"},
				{-1, 0, 1, 2, 3, 4, 5});
	}
	if (isDSiMode() && sdAccessible && ms().consoleModel < 2) {
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
		.addPage(emulationPage)
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

		gui().processInputs(pressed, touch);
	}

	return 0;
}
