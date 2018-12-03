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
#include "common/nitrofs.h"
#include "common/bootstrappaths.h"
#include "common/dsimenusettings.h"
#include "common/cardlaunch.h"
#include "settingspage.h"
#include "settingsgui.h"
#include "language.h"
#include "bootstrapsettings.h"

#include "soundeffect.h"

#include "sr_data_srllastran.h"			 // For rebooting into the game (NTR-mode touch screen)
#include "sr_data_srllastran_twltouch.h" // For rebooting into the game (TWL-mode touch screen)
#include "common/systemdetails.h"

#define AK_SYSTEM_UI_DIRECTORY "/_nds/TWiLightMenu/akmenu/themes/"

std::vector<std::string> akThemeList;

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
const char *bootstrapinipath = "sd:/_nds/nds-bootstrap.ini";

std::string homebrewArg;
std::string bootstrapfilename;

const char *unlaunchAutoLoadID = "AutoLoadInfo";
char hiyaNdsPath[14] = {'s','d','m','c',':','/','h','i','y','a','.','d','s','i'};

int screenmode = 0;
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
	//printSmall(false, x, y, "Press start...");
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

void rebootDSiMenuPP()
{
	fifoSendValue32(FIFO_USER_01, 1); // Fade out sound
	for (int i = 0; i < 25; i++)
		swiWaitForVBlank();
	snd().stopBgMusic();
	memcpy((u32 *)0x02000300, autoboot_bin, 0x020);
	fifoSendValue32(FIFO_USER_08, 1); // Reboot DSiMenu++ to avoid potential crashing
	for (int i = 0; i < 15; i++)
		swiWaitForVBlank();
}

void loadROMselect()
{
	fadeType = false;
	fifoSendValue32(FIFO_USER_01, 1); // Fade out sound
	for (int i = 0; i < 25; i++)
		swiWaitForVBlank();
	snd().stopBgMusic();
	// music = false;
	// mmEffectCancelAll();
	fifoSendValue32(FIFO_USER_01, 0); // Cancel sound fade out

	fifoSendValue32(FIFO_USER_07, 0);
	if (ms().soundfreq)
		fifoSendValue32(FIFO_USER_07, 2);
	else
		fifoSendValue32(FIFO_USER_07, 1);
	// if (soundfreqsettingChanged)
	// {
	// 	if (ms().soundfreq)
	// 		fifoSendValue32(FIFO_USER_07, 2);
	// 	else
	// 		fifoSendValue32(FIFO_USER_07, 1);
	// }
	if (ms().theme == 3)
	{
		runNdsFile("/_nds/TWiLightMenu/akmenu.srldr", 0, NULL, false);
	}
	else if (ms().theme == 2)
	{
		runNdsFile("/_nds/TWiLightMenu/r4menu.srldr", 0, NULL, false);
	}
	else
	{
		runNdsFile("/_nds/TWiLightMenu/dsimenu.srldr", 0, NULL, false);
	}
}

int lastRunROM()
{
	fifoSendValue32(FIFO_USER_01, 1); // Fade out sound
	for (int i = 0; i < 25; i++)
		swiWaitForVBlank();
	snd().stopBgMusic();
	// music = false;
	// mmEffectCancelAll();
	fifoSendValue32(FIFO_USER_01, 0); // Cancel sound fade out

	fifoSendValue32(FIFO_USER_07, 0);
	if (ms().soundfreq)
		fifoSendValue32(FIFO_USER_07, 2);
	else
		fifoSendValue32(FIFO_USER_07, 1);

	// if (soundfreqsettingChanged)
	// {
	// 	if (ms().soundfreq)
	// 		fifoSendValue32(FIFO_USER_07, 2);
	// 	else
	// 		fifoSendValue32(FIFO_USER_07, 1);
	// }

	vector<char *> argarray;
	if (ms().launchType > 2)
	{
		argarray.push_back(strdup("null"));
		argarray.push_back(strdup(homebrewArg.c_str()));
	}

	int err = 0;
	if (ms().launchType == 0)
	{
		err = runNdsFile("/_nds/TWiLightMenu/slot1launch.srldr", 0, NULL, true);
	}
	else if (ms().launchType == 1)
	{
		if (isDSiMode())
		{
			if (ms().homebrewBootstrap)
			{
				if (ms().bootstrapFile)
					bootstrapfilename = "sd:/_nds/nds-bootstrap-hb-nightly.nds";
				else
					bootstrapfilename = "sd:/_nds/nds-bootstrap-hb-release.nds";
			}
			else
			{
				if (ms().bootstrapFile)
					bootstrapfilename = "sd:/_nds/nds-bootstrap-nightly.nds";
				else
					bootstrapfilename = "sd:/_nds/nds-bootstrap-release.nds";
			}
			err = runNdsFile(bootstrapfilename.c_str(), 0, NULL, true);
		}
		else
		{
			switch (ms().flashcard)
			{
			case 0:
			case 1:
			default:
				err = runNdsFile("fat:/YSMenu.nds", 0, NULL, true);
				break;
			case 2:
			case 4:
			case 5:
				err = runNdsFile("fat:/Wfwd.dat", 0, NULL, true);
				break;
			case 3:
				err = runNdsFile("fat:/Afwd.dat", 0, NULL, true);
				break;
			case 6:
				err = runNdsFile("fat:/_dstwo/autoboot.nds", 0, NULL, true);
				break;
			}
		}
	}
	else if (ms().launchType == 2)
	{
		char unlaunchDevicePath[256];
		if (ms().previousUsedDevice) {
			snprintf(unlaunchDevicePath, sizeof(unlaunchDevicePath), "sdmc:/_nds/TWiLightMenu/tempDSiWare.dsi");
		} else {
			snprintf(unlaunchDevicePath, sizeof(unlaunchDevicePath), "__%s", ms().dsiWareSrlPath.c_str());
			unlaunchDevicePath[0] = 's';
			unlaunchDevicePath[1] = 'd';
			unlaunchDevicePath[2] = 'm';
			unlaunchDevicePath[3] = 'c';
		}

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

		fifoSendValue32(FIFO_USER_08, 1); // Reboot
	}
	else if (ms().launchType == 3)
	{
		if (sys().flashcardUsed())
		{
			argarray.at(0) = "/_nds/TWiLightMenu/emulators/nesds.nds";
			err = runNdsFile("/_nds/TWiLightMenu/emulators/nesds.nds", argarray.size(), (const char **)&argarray[0], true); // Pass ROM to nesDS as argument
		}
		else
		{
			argarray.at(0) = "sd:/_nds/TWiLightMenu/emulators/nestwl.nds";
			err = runNdsFile("sd:/_nds/TWiLightMenu/emulators/nestwl.nds", argarray.size(), (const char **)&argarray[0], true); // Pass ROM to nesDS as argument
		}
	}
	else if (ms().launchType == 4)
	{
		if (sys().flashcardUsed())
		{
			argarray.at(0) = "/_nds/TWiLightMenu/emulators/gameyob.nds";
			err = runNdsFile("/_nds/TWiLightMenu/emulators/gameyob.nds", argarray.size(), (const char **)&argarray[0], true); // Pass ROM to GameYob as argument
		}
		else
		{
			argarray.at(0) = "sd:/_nds/TWiLightMenu/emulators/gameyob.nds";
			err = runNdsFile("sd:/_nds/TWiLightMenu/emulators/gameyob.nds", argarray.size(), (const char **)&argarray[0], true); // Pass ROM to GameYob as argument
		}
	}

	return err;
}

void loadAkThemeList()
{
	DIR *dir;
	struct dirent *ent;
	std::string themeDir;
	if ((dir = opendir(AK_SYSTEM_UI_DIRECTORY)) != NULL)
	{
		/* print all the files and directories within directory */
		while ((ent = readdir(dir)) != NULL)
		{
			// Reallocation here, but prevents our vector from being filled with

			themeDir = ent->d_name;
			if (themeDir == ".." || themeDir == "..." || themeDir == ".") continue;

			akThemeList.emplace_back(themeDir);
		}
		closedir(dir);
	}
	// for (auto &p : std::filesystem::directory_iterator(path))
	// {
	// 	if (p.is_directory())
	// 		akThemeList.emplace_back(p);
	// }
}

std::optional<Option> opt_subtheme_select(Option::Int &optVal)
{
	switch (optVal.get())
	{
	case 0:
		return Option(STR_SUBTHEMESEL_DSI, STR_AB_SETSUBTHEME,
					  Option::Int(&ms().subtheme),
					  {STR_DSI_DARKMENU, STR_DSI_NORMALMENU, STR_DSI_RED, STR_DSI_BLUE, STR_DSI_GREEN, STR_DSI_YELLOW, STR_DSI_PINK, STR_DSI_PURPLE},
					  {0, 1, 2, 3, 4, 5, 6, 7});
	case 2:
		return Option(STR_SUBTHEMESEL_R4, STR_AB_SETSUBTHEME,
					  Option::Int(&ms().subtheme),
					  {
						  STR_R4_THEME01,
						  STR_R4_THEME02,
						  STR_R4_THEME03,
						  STR_R4_THEME04,
						  STR_R4_THEME05,
						  STR_R4_THEME06,
						  STR_R4_THEME07,
						  STR_R4_THEME08,
						  STR_R4_THEME09,
						  STR_R4_THEME10,
						  STR_R4_THEME11,
						  STR_R4_THEME12,
						  STR_R4_THEME13,
						  STR_R4_THEME14,
					  },
					  {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13});
	case 3:
		return Option(STR_SUBTHEMESEL_AK, STR_AB_SETRETURN, Option::Str(&ms().ak_theme), akThemeList);
	case 1:
	default:
		return nullopt;
	}
}

void defaultExitHandler()
{
	/*if (!sys().arm7SCFGLocked())
	{
		rebootDSiMenuPP();
	}*/
	loadROMselect();
}
void opt_reset_subtheme(int prev, int next)
{
	if (prev != next)
	{
		ms().subtheme = 0;
	}
}

// void opt_sound_freq_changed(bool prev, bool next)
// {
// 	if (prev != next && !soundfreqsettingChanged)
// 	{
// 		soundfreqsettingChanged = true;
// 	}
// }

void opt_reboot_system_menu()
{
	gui().onExit(launchSystemSettings).saveAndExit();
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
	//consoleDemoInit();
	//gotosettings = true;
	//bool fatInited = fatInitDefault();

	// overwrite reboot stub identifier
	extern u64 *fake_heap_end;
	*fake_heap_end = 0;

	sys().initFilesystem("/_nds/TWiLightMenu/settings.srldr");
	sys().flashcardUsed();
	ms();
	// consoleDemoInit();
	// printf("%i", sys().flashcardUsed());
	// stop();
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
	bs().loadSettings();
	loadAkThemeList();

	swiWaitForVBlank();

	// u16 arm7_SNDEXCNT = fifoGetValue32(FIFO_USER_07);
	// if (arm7_SNDEXCNT != 0) stop();

	fifoSendValue32(FIFO_USER_07, 0);
	if (ms().soundfreq)
		fifoSendValue32(FIFO_USER_07, 2);
	else
		fifoSendValue32(FIFO_USER_07, 1);

	//	InitSound();
	snd().init();
	keysSetRepeat(25, 5);
	// snprintf(vertext, sizeof(vertext), "Ver %d.%d.%d   ", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH); // Doesn't work :(

	bool sdAccessible = false;
	if (access("sd:/", F_OK) == 0) {
		sdAccessible = true;
	}

	if (access(DSIMENUPP_INI, F_OK) != 0) {
		// Create "settings.ini"
		ms().saveSettings();
	}

	if (access(BOOTSTRAP_INI, F_OK) != 0) {
		// Create "nds-bootstrap.ini"
		bs().saveSettings();
	}

	graphicsInit();
	fontInit();
	screenmode = 1;
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

	int pressed = 0;
#pragma endregion

	// consoleDemoInit();
	SettingsPage guiPage(STR_GUI_SETTINGS);

	using TLanguage = DSiMenuPlusPlusSettings::TLanguage;
	using TAKScrollSpeed = DSiMenuPlusPlusSettings::TScrollSpeed;
	guiPage
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
				 "Spanish"},
				{TLanguage::ELangDefault,
				 TLanguage::ELangJapanese,
				 TLanguage::ELangEnglish,
				 TLanguage::ELangFrench,
				 TLanguage::ELangGerman,
				 TLanguage::ELangItalian,
				 TLanguage::ELangSpanish});

	if (isDSiMode() && sdAccessible && !sys().arm7SCFGLocked()) {
		guiPage.option(STR_S1SDACCESS,
				STR_DESCRIPTION_S1SDACCESS_1,
				Option::Bool(&ms().secondaryAccess),
				{STR_ON, STR_OFF},
				{true, false});
	}

	guiPage
		// Theme
		.option(STR_THEME,
				STR_DESCRIPTION_THEME_1,
				Option::Int(&ms().theme, opt_subtheme_select, opt_reset_subtheme),
				{"DSi", "3DS", "R4", "Acekard"},
				{0, 1, 2, 3})

		.option(STR_LASTPLAYEDROM, STR_DESCRIPTION_LASTPLAYEDROM_1, Option::Bool(&ms().autorun), {STR_YES, STR_NO}, {true, false})
		.option(STR_DSIMENUPPLOGO, STR_DESCRIPTION_DSIMENUPPLOGO_1, Option::Bool(&ms().showlogo), {STR_SHOW, STR_HIDE}, {true, false})
		.option(STR_DIRECTORIES, STR_DESCRIPTION_DIRECTORIES_1, Option::Bool(&ms().showDirectories), {STR_SHOW, STR_HIDE}, {true, false})
		.option(STR_BOXART, STR_DESCRIPTION_BOXART_1, Option::Bool(&ms().showBoxArt), {STR_SHOW, STR_HIDE}, {true, false})
		.option(STR_ANIMATEDSIICONS, STR_DESCRIPTION_ANIMATEDSIICONS_1, Option::Bool(&ms().animateDsiIcons), {STR_YES, STR_NO}, {true, false})
		.option(STR_12_HOUR_CLOCK, STR_DESCRIPTION_12_HOUR_CLOCK, Option::Bool(&ms().show12hrClock), {STR_YES, STR_NO}, {true, false})
		.option(STR_AK_SCROLLSPEED, STR_DESCRIPTION_AK_SCROLLSPEED, Option::Int(&ms().ak_scrollSpeed), {"Fast", "Medium", "Slow"},
				{TAKScrollSpeed::EScrollFast, TAKScrollSpeed::EScrollMedium, TAKScrollSpeed::EScrollSlow})
		.option(STR_AK_ZOOMING_ICON, STR_DESCRIPTION_AK_ZOOMING_ICON, Option::Bool(&ms().ak_zoomIcons), {STR_ON, STR_OFF}, {true, false});

	if (isDSiMode() && sdAccessible && ms().consoleModel < 2) {
		guiPage
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

	SettingsPage gamesPage(STR_GAMESAPPS_SETTINGS);

	if (sys().flashcardUsed() && sys().isRegularDS())
	{
		gamesPage.option(STR_USEGBARUNNER2, STR_DESCRIPTION_GBARUNNER2_1, Option::Bool(&ms().useGbarunner), {STR_YES, STR_NO}, {true, false});
	}

	using TROMReadLED = BootstrapSettings::TROMReadLED;
	using TLoadingScreen = BootstrapSettings::TLoadingScreen;

	if (isDSiMode())
	{
		gamesPage
			.option(STR_LANGUAGE,
					STR_DESCRIPTION_LANGUAGE_1,
					Option::Int(&ms().bstrap_language),
					{STR_SYSTEM,
					 "Japanese",
					 "English",
					 "French",
					 "German",
					 "Italian",
					 "Spanish"},
					{TLanguage::ELangDefault,
					 TLanguage::ELangJapanese,
					 TLanguage::ELangEnglish,
					 TLanguage::ELangFrench,
					 TLanguage::ELangGerman,
					 TLanguage::ELangItalian,
					 TLanguage::ELangSpanish})

			.option(STR_RUNIN, STR_DESCRIPTION_RUNIN_1, Option::Bool(&ms().bstrap_dsiMode), {"DSi mode", "DS mode"}, {true, false})

			.option(STR_CPUSPEED,
					STR_DESCRIPTION_CPUSPEED_1,
					Option::Bool(&ms().boostCpu),
					{"133 MHz (TWL)", "67 MHz (NTR)"},
					{true, false})
			.option(STR_VRAMBOOST, STR_DESCRIPTION_VRAMBOOST_1, Option::Bool(&ms().boostVram), {STR_ON, STR_OFF}, {true, false})
			.option(STR_SOUNDFIX, STR_DESCRIPTION_SOUNDFIX_1, Option::Bool(&ms().soundFix), {STR_ON, STR_OFF}, {true, false});

		if (!sys().arm7SCFGLocked()) {
			gamesPage.option(STR_SLOT1LAUNCHMETHOD, STR_DESCRIPTION_SLOT1LAUNCHMETHOD_1, Option::Bool(&ms().slot1LaunchMethod), {STR_DIRECT, STR_REBOOT},
					{true, false});
		}
	}

	if (!sys().isRegularDS())
	{
		gamesPage.option(STR_SNDFREQ, STR_DESCRIPTION_SNDFREQ_1, Option::Bool(&ms().soundfreq), {"47.61 kHz", "32.73 kHz"}, {true, false});
	}

	if (isDSiMode())
	{
		if (ms().consoleModel < 2)
		{
			gamesPage.option(STR_ROMREADLED, STR_DESCRIPTION_ROMREADLED_1, Option::Int(&bs().bstrap_romreadled), {STR_NONE, "WiFi", STR_POWER, STR_CAMERA},
							 {TROMReadLED::ELEDNone, TROMReadLED::ELEDWifi, TROMReadLED::ELEDPower, TROMReadLED::ELEDCamera});
		}

		gamesPage
			.option(STR_LOADINGSCREEN, STR_DESCRIPTION_LOADINGSCREEN_1,
					Option::Int(&bs().bstrap_loadingScreen),
					{STR_NONE, STR_REGULAR, "Pong", "Tic-Tac-Toe"},
					{TLoadingScreen::ELoadingNone,
					 TLoadingScreen::ELoadingRegular,
					 TLoadingScreen::ELoadingPong,
					 TLoadingScreen::ELoadingTicTacToe})

			.option(STR_BOOTSTRAP, STR_DESCRIPTION_BOOTSTRAP_1,
					Option::Bool(&ms().bootstrapFile),
					{STR_NIGHTLY, STR_RELEASE},
					{true, false})

			.option(STR_DEBUG, STR_DESCRIPTION_DEBUG_1, Option::Bool(&bs().bstrap_debug), {STR_ON, STR_OFF}, {true, false})
			.option(STR_LOGGING, STR_DESCRIPTION_LOGGING_1, Option::Bool(&bs().bstrap_logging), {STR_ON, STR_OFF}, {true, false});

		if (ms().consoleModel < 2)
		{
			// Actions do not have to bound to an object.
			// See for exam here we have bound an option to
			// hiyaAutobootFound.

			// We are also using the changed callback to write
			// or delete the hiya autoboot file.
			guiPage
				.option(STR_DEFAULT_LAUNCHER, STR_DESCRIPTION_DEFAULT_LAUNCHER_1, Option::Bool(&hiyaAutobootFound, opt_hiya_autoboot_toggle), {ms().getAppName(), "System Menu"}, {true, false})
				.option(STR_SYSTEMSETTINGS, STR_DESCRIPTION_SYSTEMSETTINGS_1, Option::Nul(opt_reboot_system_menu), {}, {});
		}
	}
	gui()
		.addPage(guiPage)
		.addPage(gamesPage)
		.onExit(defaultExitHandler)
		// Prep and show the first page.
		.show();
	//	stop();
	while (1)
	{
		if (screenmode == 1)
		{
			if (!gui().isExited())
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
		else
		{
			loadROMselect();
		}
	}

	return 0;
}
