#include <nds.h>
#include <stdio.h>
#include <fat.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <string>

#include "common/inifile.h"
#include "common/dsimenusettings.h"

// Strings
std::string STR_SAVING_SETTINGS = "STR_SAVING_SETTINGS";
std::string STR_SETTINGS_SAVED = "STR_SETTINGS_SAVED";

std::string STR_LR_SWITCH = "STR_LR_SWITCH";
std::string STR_MISC_SETTINGS = "STR_MISC_SETTINGS";
std::string STR_GUI_SETTINGS = "STR_GUI_SETTINGS";
std::string STR_FILETYPE_SETTINGS = "STR_FILETYPE_SETTINGS";
std::string STR_GAMESAPPS_SETTINGS = "STR_GAMESAPPS_SETTINGS";

// GUI settings
std::string STR_COLORMODE = "STR_COLORMODE";
std::string STR_BLF = "STR_BLF";
std::string STR_S1SDACCESS = "STR_S1SDACCESS";
std::string STR_MAINMENU = "STR_MAINMENU";
std::string STR_THEME = "STR_THEME";
std::string STR_DSIMUSIC = "STR_DSIMUSIC";
std::string STR_LASTPLAYEDROM = "STR_LASTPLAYEDROM";
std::string STR_DSISPLASH = "STR_DSISPLASH";
std::string STR_HSMSG = "STR_HSMSG";
std::string STR_DSIMENUPPLOGO = "STR_DSIMENUPPLOGO";
std::string STR_SORT_METHOD = "STR_SORT_METHOD";
std::string STR_DIRECTORIES = "STR_DIRECTORIES";
std::string STR_SHOW_HIDDEN = "STR_SHOW_HIDDEN";
std::string STR_BOXART = "STR_BOXART";
std::string STR_ANIMATEDSIICONS = "STR_ANIMATEDSIICONS";
std::string STR_SYSREGION = "STR_SYSREGION";
std::string STR_LAUNCHERAPP = "STR_LAUNCHERAPP";
std::string STR_SYSTEMSETTINGS = "STR_SYSTEMSETTINGS";
std::string STR_REPLACEDSIMENU = "STR_REPLACEDSIMENU";
std::string STR_RESTOREDSIMENU = "STR_RESTOREDSIMENU";

std::string STR_SHOW = "STR_SHOW";
std::string STR_HIDE = "STR_HIDE";

std::string STR_ALPHABETICAL = "STR_ALPHABETICAL";
std::string STR_RECENT = "STR_RECENT";
std::string STR_MOST_PLAYED = "STR_MOST_PLAYED";
std::string STR_CUSTOM = "STR_CUSTOM";

std::string STR_DESCRIPTION_COLORMODE = "STR_DESCRIPTION_COLORMODE";

std::string STR_DESCRIPTION_BLF = "STR_DESCRIPTION_BLF";

std::string STR_DESCRIPTION_S1SDACCESS_1 = "STR_DESCRIPTION_S1SDACCESS_1";

std::string STR_DESCRIPTION_MAINMENU = "STR_DESCRIPTION_MAINMENU";

std::string STR_DESCRIPTION_THEME_1 = "STR_DESCRIPTION_THEME_1";

std::string STR_DESCRIPTION_DSIMUSIC = "STR_DESCRIPTION_DSIMUSIC";

std::string STR_DESCRIPTION_LASTPLAYEDROM_1 = "STR_DESCRIPTION_LASTPLAYEDROM_1";

std::string STR_DESCRIPTION_DSISPLASH = "STR_DESCRIPTION_DSISPLASH";

std::string STR_DESCRIPTION_HSMSG = "STR_DESCRIPTION_HSMSG";

std::string STR_DESCRIPTION_DSIMENUPPLOGO_1 = "STR_DESCRIPTION_DSIMENUPPLOGO_1";

std::string STR_DESCRIPTION_SORT_METHOD = "STR_DESCRIPTION_SORT_METHOD";

std::string STR_DESCRIPTION_DIRECTORIES_1 = "STR_DESCRIPTION_DIRECTORIES_1";

std::string STR_DESCRIPTION_SHOW_HIDDEN_1 = "STR_DESCRIPTION_SHOW_HIDDEN_1";

std::string STR_DESCRIPTION_SHOW_NDS = "STR_DESCRIPTION_SHOW_NDS";

std::string STR_DESCRIPTION_SHOW_NES = "STR_DESCRIPTION_SHOW_NES";

std::string STR_DESCRIPTION_SHOW_GB = "STR_DESCRIPTION_SHOW_GB";

std::string STR_DESCRIPTION_SHOW_SMS = "STR_DESCRIPTION_SHOW_SMS";

std::string STR_DESCRIPTION_SHOW_MD = "STR_DESCRIPTION_SHOW_MD";

std::string STR_DESCRIPTION_SHOW_SNES = "STR_DESCRIPTION_SHOW_SNES";

std::string STR_DESCRIPTION_BOXART_1 = "STR_DESCRIPTION_BOXART_1";

std::string STR_DESCRIPTION_ANIMATEDSIICONS_1 = "STR_DESCRIPTION_ANIMATEDSIICONS_1";

std::string STR_DESCRIPTION_SYSREGION_1 = "STR_DESCRIPTION_SYSREGION_1";

std::string STR_DESCRIPTION_LAUNCHERAPP_1 = "STR_DESCRIPTION_LAUNCHERAPP_1";

std::string STR_DESCRIPTION_SYSTEMSETTINGS_1 = "STR_DESCRIPTION_SYSTEMSETTINGS_1";

std::string STR_DESCRIPTION_REPLACEDSIMENU_1 = "STR_DESCRIPTION_REPLACEDSIMENU_1";

std::string STR_DESCRIPTION_RESTOREDSIMENU_1 = "STR_DESCRIPTION_RESTOREDSIMENU_1";

// Games/Apps settings
std::string STR_LANGUAGE = "STR_LANGUAGE";
std::string STR_CPUSPEED = "STR_CPUSPEED";
std::string STR_VRAMBOOST = "STR_VRAMBOOST";
std::string STR_USEBOOTSTRAP = "STR_BOOTSTRAP";
std::string STR_DEBUG = "STR_DEBUG";
std::string STR_LOGGING = "STR_LOGGING";
std::string STR_ROMREADLED = "STR_ROMREADLED";
std::string STR_RUNIN = "STR_RUNIN";
std::string STR_FORCESLEEPPATCH = "STR_FORCESLEEPPATCH";
std::string STR_SLOT1SCFGUNLOCK = "STR_SLOT1SCFGUNLOCK";
std::string STR_SNDFREQ = "STR_SNDFREQ";
std::string STR_SLOT1LAUNCHMETHOD = "STR_SLOT1LAUNCHMETHOD";
std::string STR_LOADINGSCREEN = "STR_LOADINGSCREEN";
std::string STR_LOADINGSCREENTHEME = "STR_LOADINGSCREENTHEME";
std::string STR_LOADINGSCREENLOCATION = "STR_LOADINGSCREENLOCATION";
std::string STR_BOOTSTRAP = "STR_BOOTSTRAP";
std::string STR_USEGBARUNNER2 = "STR_USEGBARUNNER2";

std::string STR_SYSTEM = "STR_SYSTEM";
std::string STR_ON = "STR_ON";
std::string STR_OFF = "STR_OFF";
std::string STR_YES = "STR_YES";
std::string STR_NO = "STR_NO";
std::string STR_NONE = "STR_NONE";
std::string STR_POWER = "STR_POWER";
std::string STR_CAMERA = "STR_CAMERA";
std::string STR_REBOOT = "STR_REBOOT";
std::string STR_DIRECT = "STR_DIRECT";
std::string STR_REGULAR = "STR_REGULAR";
std::string STR_DARK = "STR_DARK";
std::string STR_LIGHT = "STR_LIGHT";
std::string STR_BOTTOM = "STR_BOTTOM";
std::string STR_TOP = "STR_TOP";
std::string STR_RELEASE = "STR_RELEASE";
std::string STR_NIGHTLY = "STR_NIGHTLY";

std::string STR_DESCRIPTION_LANGUAGE_1 = "STR_DESCRIPTION_LANGUAGE_1";

std::string STR_DESCRIPTION_RUNIN_1 = "STR_DESCRIPTION_RUNIN_1";

std::string STR_DESCRIPTION_CPUSPEED_1 = "STR_DESCRIPTION_CPUSPEED_1";

std::string STR_DESCRIPTION_VRAMBOOST_1 = "STR_DESCRIPTION_VRAMBOOST_1";

std::string STR_DESCRIPTION_FORCESLEEPPATCH = "STR_DESCRIPTION_FORCESLEEPPATCH";

std::string STR_DESCRIPTION_SLOT1SCFGUNLOCK = "STR_DESCRIPTION_SLOT1SCFGUNLOCK";

std::string STR_DESCRIPTION_USEBOOTSTRAP = "STR_DESCRIPTION_USEBOOTSTRAP";

std::string STR_DESCRIPTION_DEBUG_1 = "STR_DESCRIPTION_DEBUG_1";

std::string STR_DESCRIPTION_LOGGING_1 = "STR_DESCRIPTION_LOGGING_1";

std::string STR_DESCRIPTION_ROMREADLED_1 = "STR_DESCRIPTION_ROMREADLED_1";

std::string STR_DESCRIPTION_SNDFREQ_1 = "STR_DESCRIPTION_SNDFREQ_1";

std::string STR_DESCRIPTION_SLOT1LAUNCHMETHOD_1 = "STR_DESCRIPTION_SLOT1LAUNCHMETHOD_1";

std::string STR_DESCRIPTION_LOADINGSCREEN_1 = "STR_DESCRIPTION_LOADINGSCREEN_1";

std::string STR_DESCRIPTION_LOADINGSCREENTHEME = "STR_DESCRIPTION_LOADINGSCREENTHEME";

std::string STR_DESCRIPTION_LOADINGSCREENLOCATION = "STR_DESCRIPTION_LOADINGSCREENLOCATION";

std::string STR_DESCRIPTION_BOOTSTRAP_1 = "STR_DESCRIPTION_BOOTSTRAP_1";

std::string STR_DESCRIPTION_FLASHCARD_1 = "STR_DESCRIPTION_FLASHCARD_1";

std::string STR_DESCRIPTION_GBARUNNER2_1 = "STR_DESCRIPTION_GBARUNNER2_1";

// Flashcard settings
std::string STR_FLASHCARD_SELECT = "STR_FLASHCARD_SELECT";
std::string STR_LEFTRIGHT_FLASHCARD = "STR_LEFTRIGHT_FLASHCARD";
std::string STR_AB_SETRETURN = "STR_AB_SETRETURN";

// Sub-theme select
std::string STR_SUBTHEMESEL_DSI = "STR_SUBTHEMESEL_DSI";
std::string STR_SUBTHEMESEL_3DS = "STR_SUBTHEMESEL_3DS";

std::string STR_SUBTHEMESEL_R4 = "STR_SUBTHEMESEL_R4";
std::string STR_SUBTHEMESEL_AK = "STR_SUBTHEMESEL_AK";

std::string STR_AB_SETSUBTHEME = "STR_AB_SETSUBTHEME";
std::string STR_DSI_DARKMENU = "STR_DSI_DARKMENU";
std::string STR_DSI_NORMALMENU = "STR_DSI_NORMALMENU";
std::string STR_DSI_RED = "STR_DSI_RED";
std::string STR_DSI_BLUE = "STR_DSI_BLUE";
std::string STR_DSI_GREEN = "STR_DSI_GREEN";
std::string STR_DSI_YELLOW = "STR_DSI_YELLOW";
std::string STR_DSI_PINK = "STR_DSI_PINK";
std::string STR_DSI_PURPLE = "STR_DSI_PURPLE";
std::string STR_R4_THEME01 = "STR_R4_THEME01";
std::string STR_R4_THEME02 = "STR_R4_THEME02";
std::string STR_R4_THEME03 = "STR_R4_THEME03";
std::string STR_R4_THEME04 = "STR_R4_THEME04";
std::string STR_R4_THEME05 = "STR_R4_THEME05";
std::string STR_R4_THEME06 = "STR_R4_THEME06";
std::string STR_R4_THEME07 = "STR_R4_THEME07";
std::string STR_R4_THEME08 = "STR_R4_THEME08";
std::string STR_R4_THEME09 = "STR_R4_THEME09";
std::string STR_R4_THEME10 = "STR_R4_THEME10";
std::string STR_R4_THEME11 = "STR_R4_THEME11";
std::string STR_R4_THEME12 = "STR_R4_THEME12";
std::string STR_R4_THEME13 = "STR_R4_THEME13";
std::string STR_R4_THEME14 = "STR_R4_THEME14";

std::string STR_DEFAULT_LAUNCHER = "STR_DEFAULT_LAUNCHER";
std::string STR_DESCRIPTION_DEFAULT_LAUNCHER_1 = "STR_DESCRIPTION_DEFAULT_LAUNCHER_1";

std::string STR_12_HOUR_CLOCK ="STR_12_HOUR_CLOCK";
std::string STR_DESCRIPTION_12_HOUR_CLOCK = "STR_DESCRIPTION_12_HOUR_CLOCK";

std::string STR_SNES_EMULATOR ="STR_SNES_EMULATOR";
std::string STR_DESCRIPTION_SNES_EMULATOR = "STR_DESCRIPTION_SNES_EMULATOR";

std::string STR_AK_ZOOMING_ICON = "STR_AK_ZOOMING_ICON";
std::string STR_DESCRIPTION_AK_ZOOMING_ICON = "STR_DESCRIPTION_AK_ZOOMING_ICON";

std::string STR_AK_SCROLLSPEED = "STR_AK_SCROLLSPEED";
std::string STR_DESCRIPTION_AK_SCROLLSPEED = "STR_DESCRIPTION_AK_SCROLLSPEED";

std::string STR_AK_VIEWMODE = "STR_AK_VIEWMODE";
std::string STR_DESCRIPTION_AK_VIEWMODE = "STR_DESCRIPTION_AK_VIEWMODE";


const char *languageIniPath;

int setLanguage = 0;

std::string ConvertFromUTF8(const std::string& input) {
	std::string res;
	for(size_t i = 0; i < input.size();) {
		auto c = input[i];
		if((c & 0x80) == 0) {
			res += c;
			i++;
		} else if((c & 0xe0) == 0xc0) {
			res += ((((unsigned)c & 0x1f) << 6) | ((unsigned)input[i + 1] & 0x3f));
			i += 2;
		}
		//characters not rendered by the actual font as they are bigger than a char
		else if((c & 0xf0) == 0xe0) {
			i += 3;
		} else if((c & 0xf8) == 0xf0) {
			i += 4;
		}
	}
	return res;
}

/**
 * Initialize translations.
 * Uses the language ID specified in settings.ui.language.
 */
void langInit(void)
{
	switch (ms().getGuiLanguage())
	{
	case 0:
	default:
		languageIniPath = "nitro:/languages/japanese.ini";
		break;
	case 1:
		languageIniPath = "nitro:/languages/english.ini";
		break;
	case 2:
		languageIniPath = "nitro:/languages/french.ini";
		break;
	case 3:
		languageIniPath = "nitro:/languages/german.ini";
		break;
	case 4:
		languageIniPath = "nitro:/languages/italian.ini";
		break;
	case 5:
		languageIniPath = "nitro:/languages/spanish.ini";
		break;
	case 6:
		languageIniPath = "nitro:/languages/english.ini";
		break;
	}

	CIniFile languageini(languageIniPath);

	STR_SAVING_SETTINGS = ConvertFromUTF8(languageini.GetString("LANGUAGE", "SAVING_SETTINGS", "Saving settings..."));
	STR_SETTINGS_SAVED = ConvertFromUTF8(languageini.GetString("LANGUAGE", "SETTINGS_SAVED", "Settings saved."));

	STR_LR_SWITCH = ConvertFromUTF8(languageini.GetString("LANGUAGE", "LR_SWITCH", "L/R/Y/X Switch Pages"));
	STR_MISC_SETTINGS = ConvertFromUTF8(languageini.GetString("LANGUAGE", "MISC_SETTINGS", "Misc. Settings"));
	STR_GUI_SETTINGS = ConvertFromUTF8(languageini.GetString("LANGUAGE", "GUI_SETTINGS", "GUI Settings"));
	STR_FILETYPE_SETTINGS = ConvertFromUTF8(languageini.GetString("LANGUAGE", "FILETYPE_SETTINGS", "Filetype Settings"));
	STR_GAMESAPPS_SETTINGS = ConvertFromUTF8(languageini.GetString("LANGUAGE", "GAMESAPPS_SETTINGS", "Games and Apps Settings"));

	// GUI settings
	STR_COLORMODE = ConvertFromUTF8(languageini.GetString("LANGUAGE", "COLORMODE", "Color mode"));
	STR_BLF = ConvertFromUTF8(languageini.GetString("LANGUAGE", "BLUELIGHTFILTER", "Blue light filter"));
	STR_S1SDACCESS = ConvertFromUTF8(languageini.GetString("LANGUAGE", "S1SDACCESS", "Slot-1 microSD access"));
	STR_MAINMENU = ConvertFromUTF8(languageini.GetString("LANGUAGE", "QUICKMENU", "Quick Menu"));
	STR_THEME = ConvertFromUTF8(languageini.GetString("LANGUAGE", "THEME", "Theme"));
	STR_DSIMUSIC = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DSIMUSIC", "DSi/3DS Theme Music"));
	STR_LASTPLAYEDROM = ConvertFromUTF8(languageini.GetString("LANGUAGE", "LASTPLAYEDROM", "Last played ROM on startup."));
	STR_DSISPLASH = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DSISPLASH", "Nintendo DSi Splash Screen"));
	STR_HSMSG = ConvertFromUTF8(languageini.GetString("LANGUAGE", "HSMSG", "Health & Safety message"));
	STR_DSIMENUPPLOGO = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DSIMENUPPLOGO", "TWLMenu++ Splash Screen"));
	STR_SORT_METHOD = ConvertFromUTF8(languageini.GetString("LANGUAGE", "SORT_METHOD", "Sort Method"));
	STR_DIRECTORIES = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DIRECTORIES", "Directories/Folders"));
	STR_SHOW_HIDDEN = ConvertFromUTF8(languageini.GetString("LANGUAGE", "SHOW_HIDDEN", "Show hidden files"));
	STR_BOXART = ConvertFromUTF8(languageini.GetString("LANGUAGE", "BOXART", "Box art/Game covers"));
	STR_ANIMATEDSIICONS = ConvertFromUTF8(languageini.GetString("LANGUAGE", "ANIMATEDSIICONS", "Animate DSi icons"));
	STR_SYSREGION = ConvertFromUTF8(languageini.GetString("LANGUAGE", "SYSREGION", "SysNAND Region"));
	STR_LAUNCHERAPP = ConvertFromUTF8(languageini.GetString("LANGUAGE", "LAUNCHERAPP", "SysNAND Launcher"));
	STR_SYSTEMSETTINGS = ConvertFromUTF8(languageini.GetString("LANGUAGE", "SYSTEMSETTINGS", "System Settings"));
	STR_REPLACEDSIMENU = ConvertFromUTF8(languageini.GetString("LANGUAGE", "REPLACEDSIMENU", "Replace DSi Menu"));
	STR_RESTOREDSIMENU = ConvertFromUTF8(languageini.GetString("LANGUAGE", "RESTOREDSIMENU", "Restore DSi Menu"));

	STR_SHOW = ConvertFromUTF8(languageini.GetString("LANGUAGE", "SHOW", "Show"));
	STR_HIDE = ConvertFromUTF8(languageini.GetString("LANGUAGE", "HIDE", "Hide"));

	STR_ALPHABETICAL = ConvertFromUTF8(languageini.GetString("LANGUAGE", "ALPHABETICAL", "Alphabetical"));
	STR_RECENT = ConvertFromUTF8(languageini.GetString("LANGUAGE", "RECENT", "Recent"));
	STR_MOST_PLAYED = ConvertFromUTF8(languageini.GetString("LANGUAGE", "MOST_PLAYED", "Most Played"));
	STR_CUSTOM = ConvertFromUTF8(languageini.GetString("LANGUAGE", "CUSTOM", "Custom"));

	STR_DESCRIPTION_COLORMODE = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_COLORMODE", "Changes the color of your screens. May not work in all areas. Exit settings for the change to take effect."));

	STR_DESCRIPTION_BLF = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_BLF", "Decreases/Blocks blue light from coming through your screens, and reduces eye strain. This feature is experimental, so expect issues."));

	STR_DESCRIPTION_S1SDACCESS_1 = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_S1SDACCESS_1", "Allows your flashcard to be used as a secondary device. Turn this off, if IR functionality doesn't work, or if the app crashes."));

	STR_DESCRIPTION_MAINMENU = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_QUICKMENU", "The menu that is shown before the ROM select menu. Has the look of the original DS UI."));

	STR_DESCRIPTION_THEME_1 = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_THEME_1", "The theme to use in TWiLight Menu++. Press Left/Right to select, A for sub-themes."));

	STR_DESCRIPTION_DSIMUSIC = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_DSIMUSIC", "Music to use in the DSi and 3DS themes."));

	STR_DESCRIPTION_LASTPLAYEDROM_1 = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_LASTPLAYEDROM_1", "If turned on, hold B on startup to skip to the ROM select menu."));

	STR_DESCRIPTION_DSISPLASH = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_DSISPLASH", "Show the Nintendo DSi splash screen on boot."));

	STR_DESCRIPTION_HSMSG = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_HSMSG", "Show the H&S message on the bottom screen of the DSi splash."));

	STR_DESCRIPTION_DSIMENUPPLOGO_1 = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_DSIMENUPPLOGO_1", "The logo will be shown when you start TWiLight Menu++."));

	STR_DESCRIPTION_SORT_METHOD = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_SORT_METHOD", "Changes whether to sort alphabetically, by recently played, by most played, or custom."));

	STR_DESCRIPTION_DIRECTORIES_1 = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_DIRECTORIES_1", "If you're in a folder where most of your games are, it is safe to hide directories/folders."));

	STR_DESCRIPTION_SHOW_HIDDEN_1 = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_SHOW_HIDDEN_1", "If turned on, whether an app is set to hidden or not will be ignored and it will be displayed anyways."));

	STR_DESCRIPTION_SHOW_NDS = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_SHOW_NDS", "Display Nintendo DS/DSi ROMs in the ROM list."));

	STR_DESCRIPTION_SHOW_NES = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_SHOW_NES", "Display NES and Famicom Disk System ROMs in the ROM list."));

	STR_DESCRIPTION_SHOW_GB = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_SHOW_GB", "Display GameBoy, Super GB, and GBC ROMs in the ROM list."));

	STR_DESCRIPTION_SHOW_SMS = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_SHOW_SMS", "Display Sega Master System and Game Gear ROMs in the ROM list."));

	STR_DESCRIPTION_SHOW_MD = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_SHOW_MD", "Display Sega Genesis/Mega Drive ROMs in the ROM list."));

	STR_DESCRIPTION_SHOW_SNES = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_SHOW_SNES", "Display Super Nintendo/Super Famicom ROMs in the ROM list."));

	STR_DESCRIPTION_BOXART_1 = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_BOXART_1", "Displayed in the top screen of the DSi/3DS theme."));

	STR_DESCRIPTION_ANIMATEDSIICONS_1 = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_ANIMATEDSIICONS_1", "Animate DSi-enhanced icons like in the DSi/3DS menus."));

	STR_DESCRIPTION_SYSREGION_1 = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_SYSREGION", "The region of SysNAND. \"Auto\" option will only work if SDNAND is set up."));

	STR_DESCRIPTION_LAUNCHERAPP_1 = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_LAUNCHERAPP", "To get the .app name, press POWER, hold A, then highlight LAUNCHER."));

	STR_DESCRIPTION_SYSTEMSETTINGS_1 = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_SYSTEMSETTINGS_1", "Press A to change settings related to the DSi system."));

	STR_DESCRIPTION_REPLACEDSIMENU_1 = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_REPLACEDSIMENU_1", "Start TWiLight Menu++ on boot, instead of the DSi Menu."));

	STR_DESCRIPTION_RESTOREDSIMENU_1 = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_RESTOREDSIMENU_1", "Show DSi Menu on boot again."));

	// Games/Apps settings
	STR_LANGUAGE = ConvertFromUTF8(languageini.GetString("LANGUAGE", "LANGUAGE", "Language"));
	STR_CPUSPEED = ConvertFromUTF8(languageini.GetString("LANGUAGE", "CPUSPEED", "ARM9 CPU Speed"));
	STR_VRAMBOOST = ConvertFromUTF8(languageini.GetString("LANGUAGE", "VRAMBOOST", "VRAM boost"));
	STR_USEBOOTSTRAP = ConvertFromUTF8(languageini.GetString("LANGUAGE", "USEBOOTSTRAP", "Use nds-bootstrap"));
	STR_DEBUG = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DEBUG", "Debug"));
	STR_LOGGING = ConvertFromUTF8(languageini.GetString("LANGUAGE", "LOGGING", "Logging"));
	STR_ROMREADLED = ConvertFromUTF8(languageini.GetString("LANGUAGE", "ROMREADLED", "ROM read LED"));
	STR_RUNIN = ConvertFromUTF8(languageini.GetString("LANGUAGE", "RUNIN", "Run in"));
	STR_FORCESLEEPPATCH = ConvertFromUTF8(languageini.GetString("LANGUAGE", "FORCESLEEPPATCH", "Force sleep mode patch"));
	STR_SLOT1SCFGUNLOCK = ConvertFromUTF8(languageini.GetString("LANGUAGE", "SLOT1SCFGUNLOCK", "SCFG access in Slot-1"));
	STR_SNDFREQ = ConvertFromUTF8(languageini.GetString("LANGUAGE", "SNDFREQ", "Sound/Mic frequency"));
	STR_SLOT1LAUNCHMETHOD = ConvertFromUTF8(languageini.GetString("LANGUAGE", "SLOT1LAUNCHMETHOD", "Slot-1 launch method"));
	STR_LOADINGSCREEN = ConvertFromUTF8(languageini.GetString("LANGUAGE", "LOADINGSCREEN", "Loading screen"));
	STR_LOADINGSCREENTHEME = ConvertFromUTF8(languageini.GetString("LANGUAGE", "LOADINGSCREENTHEME", "Loading screen theme"));
	STR_LOADINGSCREENLOCATION = ConvertFromUTF8(languageini.GetString("LANGUAGE", "LOADINGSCREENLOCATION", "Loading screen location"));
	STR_BOOTSTRAP = ConvertFromUTF8(languageini.GetString("LANGUAGE", "BOOTSTRAP", "Bootstrap"));
	STR_USEGBARUNNER2 = ConvertFromUTF8(languageini.GetString("LANGUAGE", "USEGBARUNNER2", "Use GBARunner2"));

	STR_SYSTEM = ConvertFromUTF8(languageini.GetString("LANGUAGE", "SYSTEM", "System"));
	STR_ON = ConvertFromUTF8(languageini.GetString("LANGUAGE", "ON", "On"));
	STR_OFF = ConvertFromUTF8(languageini.GetString("LANGUAGE", "OFF", "Off"));
	STR_YES = ConvertFromUTF8(languageini.GetString("LANGUAGE", "YES", "Yes"));
	STR_NO = ConvertFromUTF8(languageini.GetString("LANGUAGE", "NO", "No"));
	STR_NONE = ConvertFromUTF8(languageini.GetString("LANGUAGE", "NONE", "None"));
	STR_POWER = ConvertFromUTF8(languageini.GetString("LANGUAGE", "POWER", "Power"));
	STR_CAMERA = ConvertFromUTF8(languageini.GetString("LANGUAGE", "CAMERA", "Camera"));
	STR_REBOOT = ConvertFromUTF8(languageini.GetString("LANGUAGE", "REBOOT", "Reboot"));
	STR_DIRECT = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DIRECT", "Direct"));
	STR_REGULAR = ConvertFromUTF8(languageini.GetString("LANGUAGE", "REGULAR", "Regular"));
	STR_DARK = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DARK", "Dark"));
	STR_LIGHT = ConvertFromUTF8(languageini.GetString("LANGUAGE", "LIGHT", "Light"));
	STR_BOTTOM = ConvertFromUTF8(languageini.GetString("LANGUAGE", "BOTTOM", "Bottom"));
	STR_TOP = ConvertFromUTF8(languageini.GetString("LANGUAGE", "TOP", "Top"));
	STR_RELEASE = ConvertFromUTF8(languageini.GetString("LANGUAGE", "RELEASE", "Release"));
	STR_NIGHTLY = ConvertFromUTF8(languageini.GetString("LANGUAGE", "NIGHTLY", "Nightly"));

	STR_DESCRIPTION_LANGUAGE_1 = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_LANGUAGE_1", "Avoid the limited selections of your console language by setting this option."));

	STR_DESCRIPTION_RUNIN_1 = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_RUNIN_1", "Run in either DS or DSi mode."));

	STR_DESCRIPTION_CPUSPEED_1 = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_CPUSPEED_1", "Set to TWL to get rid of lags in some games."));

	STR_DESCRIPTION_VRAMBOOST_1 = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_VRAMBOOST_1", "Allow 8 bit VRAM writes and expands the bus to 32 bit."));

	STR_DESCRIPTION_FORCESLEEPPATCH = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_FORCESLEEPMODE", "If a certain Slot-1 card is preventing sleep mode from working, please turn this on. Remember to turn this off if you have a retail game card inserted."));

	STR_DESCRIPTION_SLOT1SCFGUNLOCK = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_SLOT1SCFGUNLOCK", "Have access to SCFG while running a Slot-1 flashcard. Allows setting extended memory mode and/or clock speed in flashcard homebrew."));

	STR_DESCRIPTION_USEBOOTSTRAP = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_USEBOOTSTRAP", "nds-bootstrap is used instead of the flashcard kernel or firmware."));

	STR_DESCRIPTION_DEBUG_1 = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_DEBUG_1", "Displays some text before launched game."));

	STR_DESCRIPTION_LOGGING_1 = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_LOGGING_1", "Logs the process of patching to sd:/NDSBTSRP.LOG"));

	STR_DESCRIPTION_ROMREADLED_1 = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_ROMREADLED_1", "Sets LED as ROM read indicator."));

	STR_DESCRIPTION_SNDFREQ_1 = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_SNDFREQ_1", "32.73kHz is original quality, 47.61kHz is high quality. Will not persist when soft-resetting, or launching last-run ROM."));

	STR_DESCRIPTION_SLOT1LAUNCHMETHOD_1 = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_SLOT1LAUNCHMETHOD_1",
																"Change this if some Slot-1 cards are not booting. Please note the reboot method will not use your set language or CPU speed."));

	STR_DESCRIPTION_LOADINGSCREEN_1 = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_LOADINGSCREEN_1", "Shows a loading screen before ROM is started in nds-bootstrap"));

	STR_DESCRIPTION_LOADINGSCREENTHEME = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_LOADINGSCREENTHEME", "Choose the loading screen to be light or dark."));

	STR_DESCRIPTION_LOADINGSCREENLOCATION = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_LOADINGSCREENLOCATION", "Show the loading screen on either the top or botton screen."));

	STR_DESCRIPTION_BOOTSTRAP_1 = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_BOOTSTRAP_1", "Pick release or nightly bootstrap"));

	STR_DESCRIPTION_FLASHCARD_1 = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_FLASHCARD_1", ""));

	STR_DESCRIPTION_GBARUNNER2_1 = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_GBARUNNER2_1", ""));

	// Flashcard settings
	STR_FLASHCARD_SELECT = ConvertFromUTF8(languageini.GetString("LANGUAGE", "FLASHCARD_SELECT", "Select Flashcard"));
	STR_LEFTRIGHT_FLASHCARD = ConvertFromUTF8(languageini.GetString("LANGUAGE", "LEFTRIGHT_FLASHCARD", ""));
	STR_AB_SETRETURN = ConvertFromUTF8(languageini.GetString("LANGUAGE", "AB_SETRETURN", "A/B: Set and Return"));

	// Sub-theme select
	STR_SUBTHEMESEL_DSI = ConvertFromUTF8(languageini.GetString("LANGUAGE", "SUBTHEMESEL_DSI", "Sub-theme select: DSi Menu"));
	STR_SUBTHEMESEL_3DS = ConvertFromUTF8(languageini.GetString("LANGUAGE", "SUBTHEMESEL_3DS", "Sub-theme select: DSi Menu"));

	STR_SUBTHEMESEL_R4 = ConvertFromUTF8(languageini.GetString("LANGUAGE", "SUBTHEMESEL_R4", "Sub-theme select: R4"));
	STR_SUBTHEMESEL_AK = ConvertFromUTF8(languageini.GetString("LANGUAGE", "SUBTHEMESEL_AK", "Sub-theme select: Acekard Menu"));

	STR_AB_SETSUBTHEME = ConvertFromUTF8(languageini.GetString("LANGUAGE", "AB_SETSUBTHEME", "A/B: Set sub-theme"));
	STR_DSI_DARKMENU = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DSI_DARKMENU", "SD/Black"));
	STR_DSI_NORMALMENU = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DSI_NORMALMENU", "Normal/White"));
	STR_DSI_RED = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DSI_RED", "Red"));
	STR_DSI_BLUE = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DSI_BLUE", "Blue"));
	STR_DSI_GREEN = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DSI_GREEN", "Green"));
	STR_DSI_YELLOW = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DSI_YELLOW", "Yellow"));
	STR_DSI_PINK = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DSI_PINK", "Pink"));
	STR_DSI_PURPLE = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DSI_PURPLE", "Purple"));
	STR_R4_THEME01 = ConvertFromUTF8(languageini.GetString("LANGUAGE", "R4_THEME01", "Snow hill"));
	STR_R4_THEME02 = ConvertFromUTF8(languageini.GetString("LANGUAGE", "R4_THEME02", "Snow land"));
	STR_R4_THEME03 = ConvertFromUTF8(languageini.GetString("LANGUAGE", "R4_THEME03", "Green leaf"));
	STR_R4_THEME04 = ConvertFromUTF8(languageini.GetString("LANGUAGE", "R4_THEME04", "Pink flower"));
	STR_R4_THEME05 = ConvertFromUTF8(languageini.GetString("LANGUAGE", "R4_THEME05", "Park"));
	STR_R4_THEME06 = ConvertFromUTF8(languageini.GetString("LANGUAGE", "R4_THEME06", "Cherry blossoms"));
	STR_R4_THEME07 = ConvertFromUTF8(languageini.GetString("LANGUAGE", "R4_THEME07", "Beach"));
	STR_R4_THEME08 = ConvertFromUTF8(languageini.GetString("LANGUAGE", "R4_THEME08", "Summer sky"));
	STR_R4_THEME09 = ConvertFromUTF8(languageini.GetString("LANGUAGE", "R4_THEME09", "River"));
	STR_R4_THEME10 = ConvertFromUTF8(languageini.GetString("LANGUAGE", "R4_THEME10", "Fall trees"));
	STR_R4_THEME11 = ConvertFromUTF8(languageini.GetString("LANGUAGE", "R4_THEME11", "Christmas tree"));
	STR_R4_THEME12 = ConvertFromUTF8(languageini.GetString("LANGUAGE", "R4_THEME12", "Drawn symbol"));
	STR_R4_THEME13 = ConvertFromUTF8(languageini.GetString("LANGUAGE", "R4_THEME13", "Blue moon"));
	STR_R4_THEME14 = ConvertFromUTF8(languageini.GetString("LANGUAGE", "R4_THEME14", "Mac-like"));

	STR_DEFAULT_LAUNCHER = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DEFAULT_LAUNCHER", "Default launcher"));

    STR_12_HOUR_CLOCK =  ConvertFromUTF8(languageini.GetString("LANGUAGE", "12_HOUR_CLOCK", "Use a 12 hour clock"));
    STR_DESCRIPTION_12_HOUR_CLOCK = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_12_HOUR_CLOCK", "Use a 12-hour clock instead of a 24 hour clock in the Acekard theme."));

		STR_SNES_EMULATOR =  ConvertFromUTF8(languageini.GetString("LANGUAGE", "SNES_EMULATOR", "Choose a SNES emulator"));
    STR_DESCRIPTION_SNES_EMULATOR = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_SNES_EMULATOR", "Choose whether you would rather use SNEmulDS or lolSNES."));

    STR_AK_ZOOMING_ICON = ConvertFromUTF8(languageini.GetString("LANGUAGE", "AK_ZOOMING_ICON", "Zooming icons"));
    STR_DESCRIPTION_AK_ZOOMING_ICON = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_ZOOMING_ICON", "Display a zoom effect for the selected icon in the Acekard theme."));

    STR_AK_SCROLLSPEED = ConvertFromUTF8(languageini.GetString("LANGUAGE", "AK_SCROLLSPEED", "Scroll speed"));
    STR_DESCRIPTION_AK_SCROLLSPEED =  ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_AK_SCROLLSPEED", "Sets the scroll speed in the Acekard theme."));

	// The localestrign is here but the setting isn't displayed
	// we can just keep the default viewmode
	// which is icons with rom internal names.
	// akmenu should save the view mode anyways when its changed with SELECT.
	
    STR_AK_VIEWMODE = ConvertFromUTF8(languageini.GetString("LANGUAGE", "AK_VIEWMODE", "Default viewmode"));
    STR_DESCRIPTION_AK_VIEWMODE = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_AK_VIEWMODE", "Sets the default view mode in the Acekard theme."));

	STR_DESCRIPTION_DEFAULT_LAUNCHER_1 = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_DEFAULT_LAUNCHER_1_DSIMENUPP", "Launch Nintendo DSi Menu or TWiLight Menu++ on boot."));
}

