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
std::string STR_MISC_SETTINGS = "STR_MISC_SETTINGS";
std::string STR_GUI_SETTINGS = "STR_GUI_SETTINGS";
std::string STR_FILETYPE_SETTINGS = "STR_FILETYPE_SETTINGS";
std::string STR_GBARUNNER2_SETTINGS = "STR_GBARUNNER2_SETTINGS";
std::string STR_GAMESAPPS_SETTINGS = "STR_GAMESAPPS_SETTINGS";

// GUI settings
std::string STR_COLORMODE = "STR_COLORMODE";
std::string STR_WIFILED = "STR_WIFILED";
std::string STR_SDREMOVALDETECTION = "STR_SDREMOVALDETECTION";
std::string STR_S1SDACCESS = "STR_S1SDACCESS";
std::string STR_MAINMENU = "STR_MAINMENU";
std::string STR_THEME = "STR_THEME";
std::string STR_DSIMUSIC = "STR_DSIMUSIC";
std::string STR_LASTPLAYEDROM = "STR_LASTPLAYEDROM";
std::string STR_DSISPLASH = "STR_DSISPLASH";
std::string STR_HSMSG = "STR_HSMSG";
std::string STR_DSIMENUPPLOGO = "STR_DSIMENUPPLOGO";
std::string STR_UPDATE_RECENTLY_PLAYED_LIST = "STR_UPDATE_RECENTLY_PLAYED_LIST";
std::string STR_SORT_METHOD = "STR_SORT_METHOD";
std::string STR_DIRECTORIES = "STR_DIRECTORIES";
std::string STR_SHOW_HIDDEN = "STR_SHOW_HIDDEN";
std::string STR_BOXART = "STR_BOXART";
std::string STR_BOXARTMEM = "STR_BOXARTMEM";
std::string STR_ANIMATEDSIICONS = "STR_ANIMATEDSIICONS";
std::string STR_SYSREGION = "STR_SYSREGION";
std::string STR_LAUNCHERAPP = "STR_LAUNCHERAPP";
std::string STR_SYSTEMSETTINGS = "STR_SYSTEMSETTINGS";

std::string STR_SHOW = "STR_SHOW";
std::string STR_HIDE = "STR_HIDE";

std::string STR_ALPHABETICAL = "STR_ALPHABETICAL";
std::string STR_RECENT = "STR_RECENT";
std::string STR_MOST_PLAYED = "STR_MOST_PLAYED";
std::string STR_FILE_TYPE = "STR_FILE_TYPE";
std::string STR_CUSTOM = "STR_CUSTOM";

std::string STR_DESCRIPTION_COLORMODE = "STR_DESCRIPTION_COLORMODE";
std::string STR_DESCRIPTION_WIFILED = "STR_DESCRIPTION_WIFILED";
std::string STR_DESCRIPTION_SDREMOVALDETECTION = "STR_DESCRIPTION_SDREMOVALDETECTION";
std::string STR_DESCRIPTION_S1SDACCESS_1 = "STR_DESCRIPTION_S1SDACCESS_1";
std::string STR_DESCRIPTION_MAINMENU = "STR_DESCRIPTION_MAINMENU";
std::string STR_DESCRIPTION_THEME_1 = "STR_DESCRIPTION_THEME_1";
std::string STR_DESCRIPTION_DSIMUSIC = "STR_DESCRIPTION_DSIMUSIC";
std::string STR_DESCRIPTION_LASTPLAYEDROM_1 = "STR_DESCRIPTION_LASTPLAYEDROM_1";
std::string STR_DESCRIPTION_DSISPLASH = "STR_DESCRIPTION_DSISPLASH";
std::string STR_DESCRIPTION_HSMSG = "STR_DESCRIPTION_HSMSG";
std::string STR_DESCRIPTION_DSIMENUPPLOGO_1 = "STR_DESCRIPTION_DSIMENUPPLOGO_1";
std::string STR_DESCRIPTION_UPDATE_RECENTLY_PLAYED_LIST = "STR_DESCRIPTION_UPDATE_RECENTLY_PLAYED_LIST";
std::string STR_DESCRIPTION_SORT_METHOD = "STR_DESCRIPTION_SORT_METHOD";
std::string STR_DESCRIPTION_DIRECTORIES_1 = "STR_DESCRIPTION_DIRECTORIES_1";
std::string STR_DESCRIPTION_SHOW_HIDDEN_1 = "STR_DESCRIPTION_SHOW_HIDDEN_1";

std::string STR_DESCRIPTION_SHOW_NDS = "STR_DESCRIPTION_SHOW_NDS";
std::string STR_DESCRIPTION_SHOW_RVID = "STR_DESCRIPTION_SHOW_RVID";
std::string STR_DESCRIPTION_SHOW_NES = "STR_DESCRIPTION_SHOW_NES";
std::string STR_DESCRIPTION_SHOW_GB = "STR_DESCRIPTION_SHOW_GB";
std::string STR_DESCRIPTION_SHOW_SMS = "STR_DESCRIPTION_SHOW_SMS";
std::string STR_DESCRIPTION_SHOW_MD = "STR_DESCRIPTION_SHOW_MD";
std::string STR_DESCRIPTION_SHOW_SNES = "STR_DESCRIPTION_SHOW_SNES";

std::string STR_DESCRIPTION_BOXART_1 = "STR_DESCRIPTION_BOXART_1";
std::string STR_DESCRIPTION_BOXARTMEM = "STR_DESCRIPTION_BOXARTMEM";
std::string STR_DESCRIPTION_ANIMATEDSIICONS_1 = "STR_DESCRIPTION_ANIMATEDSIICONS_1";
std::string STR_DESCRIPTION_SYSREGION_1 = "STR_DESCRIPTION_SYSREGION_1";
std::string STR_DESCRIPTION_LAUNCHERAPP_1 = "STR_DESCRIPTION_LAUNCHERAPP_1";
std::string STR_DESCRIPTION_SYSTEMSETTINGS_1 = "STR_DESCRIPTION_SYSTEMSETTINGS_1";

// GBARunner2 settings
std::string STR_DESCRIPTION_DLDIACCESS = "STR_DESCRIPTION_DLDIACCESS";
std::string STR_DESCRIPTION_USEBOTTOMSCREEN = "STR_DESCRIPTION_USEBOTTOMSCREEN";
std::string STR_DESCRIPTION_CENTERANDMASK = "STR_DESCRIPTION_CENTERANDMASK";
std::string STR_DESCRIPTION_MAINMEMICACHE = "STR_DESCRIPTION_MAINMEMICACHE";
std::string STR_DESCRIPTION_WRAMICACHE = "STR_DESCRIPTION_WRAMICACHE";
std::string STR_DESCRIPTION_BIOSINTRO = "STR_DESCRIPTION_BIOSINTRO";

// Games/Apps settings
std::string STR_ASPECTRATIO = "STR_ASPECTRATIO";
std::string STR_LANGUAGE = "STR_LANGUAGE";
std::string STR_CPUSPEED = "STR_CPUSPEED";
std::string STR_VRAMBOOST = "STR_VRAMBOOST";
std::string STR_USEBOOTSTRAP = "STR_USEBOOTSTRAP";
std::string STR_FCSAVELOCATION = "STR_FCSAVELOCATION";
std::string STR_DEBUG = "STR_DEBUG";
std::string STR_LOGGING = "STR_LOGGING";
std::string STR_ROMREADLED = "STR_ROMREADLED";
std::string STR_PRECISEVOLUMECTRL = "STR_PRECISEVOLUMECTRL";
std::string STR_DSIWAREBOOTER = "STR_DSIWAREBOOTER";
std::string STR_RUNIN = "STR_RUNIN";
std::string STR_FORCESLEEPPATCH = "STR_FORCESLEEPPATCH";
std::string STR_SLOT1SCFGUNLOCK = "STR_SLOT1SCFGUNLOCK";
std::string STR_SNDFREQ = "STR_SNDFREQ";
std::string STR_SLOT1LAUNCHMETHOD = "STR_SLOT1LAUNCHMETHOD";
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
std::string STR_RELEASE = "STR_RELEASE";
std::string STR_NIGHTLY = "STR_NIGHTLY";

std::string STR_DESCRIPTION_ASPECTRATIO = "STR_DESCRIPTION_ASPECTRATIO";
std::string STR_DESCRIPTION_LANGUAGE_1 = "STR_DESCRIPTION_LANGUAGE_1";
std::string STR_DESCRIPTION_RUNIN_1 = "STR_DESCRIPTION_RUNIN_1";
std::string STR_DESCRIPTION_CPUSPEED_1 = "STR_DESCRIPTION_CPUSPEED_1";
std::string STR_DESCRIPTION_VRAMBOOST_1 = "STR_DESCRIPTION_VRAMBOOST_1";
std::string STR_DESCRIPTION_FORCESLEEPPATCH = "STR_DESCRIPTION_FORCESLEEPPATCH";
std::string STR_DESCRIPTION_SLOT1SCFGUNLOCK = "STR_DESCRIPTION_SLOT1SCFGUNLOCK";
std::string STR_DESCRIPTION_USEBOOTSTRAP = "STR_DESCRIPTION_USEBOOTSTRAP";
std::string STR_DESCRIPTION_FCSAVELOCATION = "STR_DESCRIPTION_FCSAVELOCATION";
std::string STR_DESCRIPTION_DEBUG_1 = "STR_DESCRIPTION_DEBUG_1";
std::string STR_DESCRIPTION_LOGGING_1 = "STR_DESCRIPTION_LOGGING_1";
std::string STR_DESCRIPTION_ROMREADLED_1 = "STR_DESCRIPTION_ROMREADLED_1";
std::string STR_DESCRIPTION_PRECISEVOLUMECTRL = "STR_DESCRIPTION_PRECISEVOLUMECTRL";

std::string STR_DESCRIPTION_DSIWAREBOOTER = "STR_DESCRIPTION_DSIWAREBOOTER";

std::string STR_DESCRIPTION_SNDFREQ_1 = "STR_DESCRIPTION_SNDFREQ_1";
std::string STR_DESCRIPTION_SLOT1LAUNCHMETHOD_1 = "STR_DESCRIPTION_SLOT1LAUNCHMETHOD_1";
std::string STR_DESCRIPTION_BOOTSTRAP_1 = "STR_DESCRIPTION_BOOTSTRAP_1";
std::string STR_DESCRIPTION_GBARUNNER2_1 = "STR_DESCRIPTION_GBARUNNER2_1";

// Flashcard settings
std::string STR_AB_SETRETURN = "STR_AB_SETRETURN";

// Sub-theme select
std::string STR_SUBTHEMESEL_DSI = "STR_SUBTHEMESEL_DSI";
std::string STR_SUBTHEMESEL_3DS = "STR_SUBTHEMESEL_3DS";

std::string STR_SUBTHEMESEL_R4 = "STR_SUBTHEMESEL_R4";
std::string STR_SUBTHEMESEL_AK = "STR_SUBTHEMESEL_AK";

std::string STR_AB_SETSUBTHEME = "STR_AB_SETSUBTHEME";

std::string STR_DEFAULT_LAUNCHER = "STR_DEFAULT_LAUNCHER";
std::string STR_DESCRIPTION_DEFAULT_LAUNCHER_1 = "STR_DESCRIPTION_DEFAULT_LAUNCHER_1";

std::string STR_12_HOUR_CLOCK ="STR_12_HOUR_CLOCK";
std::string STR_DESCRIPTION_12_HOUR_CLOCK = "STR_DESCRIPTION_12_HOUR_CLOCK";

std::string STR_AK_ZOOMING_ICON = "STR_AK_ZOOMING_ICON";
std::string STR_DESCRIPTION_AK_ZOOMING_ICON = "STR_DESCRIPTION_AK_ZOOMING_ICON";

std::string STR_AK_SCROLLSPEED = "STR_AK_SCROLLSPEED";
std::string STR_DESCRIPTION_AK_SCROLLSPEED = "STR_DESCRIPTION_AK_SCROLLSPEED";

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
	switch (ms().getGuiLanguage()) {
		case 0:
			languageIniPath = "nitro:/languages/japanese.ini";
			break;
		case 1:
		case 6:
		default:
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
	}

	CIniFile languageini(languageIniPath);

	STR_MISC_SETTINGS = ConvertFromUTF8(languageini.GetString("LANGUAGE", "MISC_SETTINGS", "Misc. settings"));
	STR_GUI_SETTINGS = ConvertFromUTF8(languageini.GetString("LANGUAGE", "GUI_SETTINGS", "GUI Settings"));
	STR_FILETYPE_SETTINGS = ConvertFromUTF8(languageini.GetString("LANGUAGE", "FILETYPE_SETTINGS", "Filetype settings"));
	STR_GBARUNNER2_SETTINGS = ConvertFromUTF8(languageini.GetString("LANGUAGE", "GBARUNNER2_SETTINGS", "GBARunner2 Emulation settings"));
	STR_GAMESAPPS_SETTINGS = ConvertFromUTF8(languageini.GetString("LANGUAGE", "GAMESAPPS_SETTINGS", "Games and Apps settings"));

	// GUI settings
	STR_COLORMODE = ConvertFromUTF8(languageini.GetString("LANGUAGE", "COLORMODE", "Color mode"));
	STR_DESCRIPTION_COLORMODE = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_COLORMODE", "Changes the color of your screens. May not work in all areas. Exit settings for the change to take effect."));

	STR_WIFILED = ConvertFromUTF8(languageini.GetString("LANGUAGE", "WIFILED", "WiFi LED"));
	STR_DESCRIPTION_WIFILED = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_WIFILED", "Enables/Disables the DSi console's WiFi LED."));

	STR_SDREMOVALDETECTION = ConvertFromUTF8(languageini.GetString("LANGUAGE", "SDREMOVALDETECTION", "SD removal detection"));
	STR_DESCRIPTION_SDREMOVALDETECTION = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_SDREMOVALDETECTION", "Enables/Disables SD removal detection. Disable it, if an error message is shown, instead of the ROM menu."));

	STR_S1SDACCESS = ConvertFromUTF8(languageini.GetString("LANGUAGE", "S1SDACCESS", "Slot-1 microSD access"));
	STR_DESCRIPTION_S1SDACCESS_1 = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_S1SDACCESS_1", "Allows your flashcard to be used as a secondary device. Turn this off if IR functionality does not work, or if the app crashes."));

	STR_MAINMENU = ConvertFromUTF8(languageini.GetString("LANGUAGE", "QUICKMENU", "Quick Menu"));
	STR_DESCRIPTION_MAINMENU = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_QUICKMENU", "Toggling this option allows you to have a menu similar to the original DS UI before the ROM select menu."));

	STR_THEME = ConvertFromUTF8(languageini.GetString("LANGUAGE", "THEME", "Theme"));
	STR_DESCRIPTION_THEME_1 = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_THEME_1", "The theme to use in TWiLight Menu++. Press Left/Right to select, A for sub-themes."));

	STR_DSIMUSIC = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DSIMUSIC", "DSi/3DS Theme Music"));
	STR_DESCRIPTION_DSIMUSIC = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_DSIMUSIC", "Music to use in the DSi and 3DS themes."));

	STR_LASTPLAYEDROM = ConvertFromUTF8(languageini.GetString("LANGUAGE", "LASTPLAYEDROM", "Last played ROM on startup."));
	STR_DESCRIPTION_LASTPLAYEDROM_1 = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_LASTPLAYEDROM_1", "If turned on, hold B on startup to skip to the ROM select menu."));

	STR_DSISPLASH = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DSISPLASH", "Nintendo DSi Splash Screen"));
	STR_DESCRIPTION_DSISPLASH = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_DSISPLASH", "Show the Nintendo DSi splash screen on boot."));

	STR_HSMSG = ConvertFromUTF8(languageini.GetString("LANGUAGE", "HSMSG", "Health & Safety message"));
	STR_DESCRIPTION_HSMSG = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_HSMSG", "Show the H&S message on the bottom screen of the DSi splash."));

	STR_DSIMENUPPLOGO = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DSIMENUPPLOGO", "TWLMenu++ Splash Screen"));
	STR_DESCRIPTION_DSIMENUPPLOGO_1 = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_DSIMENUPPLOGO_1", "The logo will be shown when you start TWiLight Menu++."));

	STR_UPDATE_RECENTLY_PLAYED_LIST = ConvertFromUTF8(languageini.GetString("LANGUAGE", "UPDATE_RECENTLY_PLAYED_LIST", "Update Recently Played list"));
	STR_DESCRIPTION_UPDATE_RECENTLY_PLAYED_LIST = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_UPDATE_RECENTLY_PLAYED_LIST", "Updates the list based on what game you recently played. Disable this, if launching a game or app causes a Guru Meditation Error."));

	STR_SORT_METHOD = ConvertFromUTF8(languageini.GetString("LANGUAGE", "SORT_METHOD", "Sort Method"));
	STR_DESCRIPTION_SORT_METHOD = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_SORT_METHOD", "Changes whether to sort alphabetically, by recently played, by most played, by file type, or custom."));

	STR_DIRECTORIES = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DIRECTORIES", "Directories/Folders"));
	STR_DESCRIPTION_DIRECTORIES_1 = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_DIRECTORIES_1", "If you're in a folder where most of your games are, it is safe to hide directories/folders."));

	STR_SHOW_HIDDEN = ConvertFromUTF8(languageini.GetString("LANGUAGE", "SHOW_HIDDEN", "Show hidden files"));
	STR_DESCRIPTION_SHOW_HIDDEN_1 = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_SHOW_HIDDEN_1", "If turned on, whether an app is set to hidden or not will be ignored and it will be displayed anyways."));

	STR_BOXART = ConvertFromUTF8(languageini.GetString("LANGUAGE", "BOXART", "Box art/Game covers"));
	STR_DESCRIPTION_BOXART_1 = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_BOXART_1", "Displayed in the top screen of the DSi/3DS theme."));

	STR_BOXARTMEM = ConvertFromUTF8(languageini.GetString("LANGUAGE", "BOXARTMEM", "Cache box art to memory"));
	STR_DESCRIPTION_BOXARTMEM = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_BOXARTMEM", "Loads the box art images into RAM, for lagless scrolling, at the cost of slow ROM list loading."));

	STR_ANIMATEDSIICONS = ConvertFromUTF8(languageini.GetString("LANGUAGE", "ANIMATEDSIICONS", "Animate DSi icons"));
	STR_DESCRIPTION_ANIMATEDSIICONS_1 = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_ANIMATEDSIICONS_1", "Animate DSi-enhanced icons like in the DSi/3DS menus."));

	STR_SYSREGION = ConvertFromUTF8(languageini.GetString("LANGUAGE", "SYSREGION", "SysNAND Region"));
	STR_DESCRIPTION_SYSREGION_1 = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_SYSREGION", "The region of SysNAND. \"Auto\" option will only work if SDNAND is set up."));

	STR_LAUNCHERAPP = ConvertFromUTF8(languageini.GetString("LANGUAGE", "LAUNCHERAPP", "SysNAND Launcher"));
	STR_DESCRIPTION_LAUNCHERAPP_1 = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_LAUNCHERAPP", "To get the .app name, press POWER, hold A+B, then highlight LAUNCHER."));

	STR_SYSTEMSETTINGS = ConvertFromUTF8(languageini.GetString("LANGUAGE", "SYSTEMSETTINGS", "System Settings"));
	STR_DESCRIPTION_SYSTEMSETTINGS_1 = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_SYSTEMSETTINGS_1", "Press A to change settings related to the DSi system."));

	STR_DEFAULT_LAUNCHER = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DEFAULT_LAUNCHER", "Default launcher"));
	STR_DESCRIPTION_DEFAULT_LAUNCHER_1 = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_DEFAULT_LAUNCHER_1_DSIMENUPP", "Launch Nintendo DSi Menu or TWiLight Menu++ on boot."));

	STR_SHOW = ConvertFromUTF8(languageini.GetString("LANGUAGE", "SHOW", "Show"));
	STR_HIDE = ConvertFromUTF8(languageini.GetString("LANGUAGE", "HIDE", "Hide"));

	STR_ALPHABETICAL = ConvertFromUTF8(languageini.GetString("LANGUAGE", "ALPHABETICAL", "Alphabetical"));
	STR_RECENT = ConvertFromUTF8(languageini.GetString("LANGUAGE", "RECENT", "Recent"));
	STR_MOST_PLAYED = ConvertFromUTF8(languageini.GetString("LANGUAGE", "MOST_PLAYED", "Most Played"));
	STR_FILE_TYPE = ConvertFromUTF8(languageini.GetString("LANGUAGE", "FILE_TYPE", "File Type"));
	STR_CUSTOM = ConvertFromUTF8(languageini.GetString("LANGUAGE", "CUSTOM", "Custom"));

	STR_DESCRIPTION_SHOW_NDS = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_SHOW_NDS", "Display Nintendo DS/DSi ROMs in the ROM list."));
	STR_DESCRIPTION_SHOW_RVID = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_SHOW_RVID", "Display Rocket Video files in the ROM list."));
	STR_DESCRIPTION_SHOW_NES = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_SHOW_NES", "Display NES and Famicom Disk System ROMs in the ROM list."));
	STR_DESCRIPTION_SHOW_GB = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_SHOW_GB", "Display GameBoy, Super GB, and GBC ROMs in the ROM list."));
	STR_DESCRIPTION_SHOW_SMS = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_SHOW_SMS", "Display Sega Master System and Game Gear ROMs in the ROM list."));
	STR_DESCRIPTION_SHOW_MD = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_SHOW_MD", "Display Sega Genesis/Mega Drive ROMs in the ROM list."));
	STR_DESCRIPTION_SHOW_SNES = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_SHOW_SNES", "Display Super Nintendo/Super Famicom ROMs in the ROM list."));

	// GBARunner2 settings
	STR_DESCRIPTION_DLDIACCESS = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_GBAR2_DLDIACCESS", "Select which CPU to access the flashcard's DLDI driver. Select ARM7, if ARM9 causes a white screen crash."));
	STR_DESCRIPTION_USEBOTTOMSCREEN = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_USEBOTTOMSCREEN", "Play GBA games on the bottom screen."));
	STR_DESCRIPTION_CENTERANDMASK = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_CENTERANDMASK", "Centers the game with a border. Adds 1 frame delay."));
	STR_DESCRIPTION_MAINMEMICACHE = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_GBAR2_MAINMEMICACHE", "Boosts speed, but causes timing bugs in a few games."));
	STR_DESCRIPTION_WRAMICACHE = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_GBAR2_WRAMICACHE", "Boosts speed, but some games may crash."));
	STR_DESCRIPTION_BIOSINTRO = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_BIOSINTRO", "Plays the GBA logo before starting GBA game."));

	// Games/Apps settings
	STR_ASPECTRATIO = ConvertFromUTF8(languageini.GetString("LANGUAGE", "ASPECTRATIO", "Screen Aspect Ratio"));
	STR_DESCRIPTION_ASPECTRATIO = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_ASPECTRATIO", "Adjusts the top screen to be the original size, or wider, for DS games. Some games may not use this. Requires Sono's patcher to be used, as well as external sysmodules enabled in Luma."));

	STR_LANGUAGE = ConvertFromUTF8(languageini.GetString("LANGUAGE", "LANGUAGE", "Language"));
	STR_DESCRIPTION_LANGUAGE_1 = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_LANGUAGE_1", "Avoid the limited selections of your console language by setting this option."));

	STR_CPUSPEED = ConvertFromUTF8(languageini.GetString("LANGUAGE", "CPUSPEED", "ARM9 CPU Speed"));
	STR_DESCRIPTION_CPUSPEED_1 = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_CPUSPEED_1", "Set to TWL to get rid of lags in some games."));

	STR_VRAMBOOST = ConvertFromUTF8(languageini.GetString("LANGUAGE", "VRAMBOOST", "VRAM boost"));
	STR_DESCRIPTION_VRAMBOOST_1 = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_VRAMBOOST_1", "Allow 8 bit VRAM writes and expands the bus to 32 bit."));

	STR_USEBOOTSTRAP = ConvertFromUTF8(languageini.GetString("LANGUAGE", "USEBOOTSTRAP", "Use nds-bootstrap"));
	STR_DESCRIPTION_USEBOOTSTRAP = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_USEBOOTSTRAP", "nds-bootstrap is used instead of the flashcard kernel or firmware."));

	STR_FCSAVELOCATION = ConvertFromUTF8(languageini.GetString("LANGUAGE", "FCSAVELOCATION", "Slot-1 SD save location"));
	STR_DESCRIPTION_FCSAVELOCATION = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_FCSAVELOCATION", "Pick the location of Slot-1 SD (or flashcard) saves."));

	STR_DEBUG = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DEBUG", "Debug"));
	STR_DESCRIPTION_DEBUG_1 = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_DEBUG_1", "Displays some text before launched game."));

	STR_LOGGING = ConvertFromUTF8(languageini.GetString("LANGUAGE", "LOGGING", "Logging"));
	STR_DESCRIPTION_LOGGING_1 = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_LOGGING_1", "Logs the process of patching to sd:/NDSBTSRP.LOG"));

	STR_ROMREADLED = ConvertFromUTF8(languageini.GetString("LANGUAGE", "ROMREADLED", "ROM read LED"));
	STR_DESCRIPTION_ROMREADLED_1 = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_ROMREADLED_1", "Sets LED as ROM read indicator."));

	STR_PRECISEVOLUMECTRL = ConvertFromUTF8(languageini.GetString("LANGUAGE", "PRECISEVOLUMECTRL", "Precise volume control"));
	STR_DSIWAREBOOTER = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DSIWAREBOOTER", "DSiWare booter"));
	STR_RUNIN = ConvertFromUTF8(languageini.GetString("LANGUAGE", "RUNIN", "Run in"));
	STR_DESCRIPTION_RUNIN_1 = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_RUNIN_1", "Run in either DS or DSi mode."));

	STR_FORCESLEEPPATCH = ConvertFromUTF8(languageini.GetString("LANGUAGE", "FORCESLEEPPATCH", "Force sleep mode patch"));
	STR_DESCRIPTION_FORCESLEEPPATCH = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_FORCESLEEPMODE", "If a certain Slot-1 card is preventing sleep mode from working, please turn this on. Remember to turn this off if you have a retail game card inserted."));

	STR_SLOT1SCFGUNLOCK = ConvertFromUTF8(languageini.GetString("LANGUAGE", "SLOT1SCFGUNLOCK", "SCFG access in Slot-1"));
	STR_DESCRIPTION_SLOT1SCFGUNLOCK = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_SLOT1SCFGUNLOCK", "Have access to SCFG while running a Slot-1 flashcard. Allows setting extended memory mode and/or clock speed in flashcard homebrew."));

	STR_SNDFREQ = ConvertFromUTF8(languageini.GetString("LANGUAGE", "SNDFREQ", "Sound/Mic frequency"));
	STR_DESCRIPTION_SNDFREQ_1 = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_SNDFREQ_1", "32.73kHz is original quality, 47.61kHz is high quality. Does not affect retail Slot-1 cards."));

	STR_SLOT1LAUNCHMETHOD = ConvertFromUTF8(languageini.GetString("LANGUAGE", "SLOT1LAUNCHMETHOD", "Slot-1 launch method"));
	STR_DESCRIPTION_SLOT1LAUNCHMETHOD_1 = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_SLOT1LAUNCHMETHOD_1", "Change this if some Slot-1 cards are not booting. Please note the reboot method will not use your set language or CPU speed."));

	STR_BOOTSTRAP = ConvertFromUTF8(languageini.GetString("LANGUAGE", "BOOTSTRAP", "Bootstrap"));
	STR_DESCRIPTION_BOOTSTRAP_1 = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_BOOTSTRAP_1", "Pick release or nightly bootstrap"));

	STR_USEGBARUNNER2 = ConvertFromUTF8(languageini.GetString("LANGUAGE", "USEGBARUNNER2", "Use GBARunner2"));
	STR_DESCRIPTION_GBARUNNER2_1 = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_GBARUNNER2_1", ""));

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
	STR_RELEASE = ConvertFromUTF8(languageini.GetString("LANGUAGE", "RELEASE", "Release"));
	STR_NIGHTLY = ConvertFromUTF8(languageini.GetString("LANGUAGE", "NIGHTLY", "Nightly"));

	// Flashcard settings
	STR_AB_SETRETURN = ConvertFromUTF8(languageini.GetString("LANGUAGE", "AB_SETRETURN", "A/B: Set and Return"));

	// Sub-theme select
	STR_SUBTHEMESEL_DSI = ConvertFromUTF8(languageini.GetString("LANGUAGE", "SUBTHEMESEL_DSI", "Sub-theme select: DSi Menu"));
	STR_SUBTHEMESEL_3DS = ConvertFromUTF8(languageini.GetString("LANGUAGE", "SUBTHEMESEL_3DS", "Sub-theme select: DSi Menu"));

	STR_SUBTHEMESEL_R4 = ConvertFromUTF8(languageini.GetString("LANGUAGE", "SUBTHEMESEL_R4", "Sub-theme select: R4"));
	STR_SUBTHEMESEL_AK = ConvertFromUTF8(languageini.GetString("LANGUAGE", "SUBTHEMESEL_AK", "Sub-theme select: Acekard Menu"));

	STR_AB_SETSUBTHEME = ConvertFromUTF8(languageini.GetString("LANGUAGE", "AB_SETSUBTHEME", "A/B: Set sub-theme"));

	STR_12_HOUR_CLOCK =  ConvertFromUTF8(languageini.GetString("LANGUAGE", "12_HOUR_CLOCK", "Use a 12 hour clock"));
	STR_DESCRIPTION_12_HOUR_CLOCK = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_12_HOUR_CLOCK", "Use a 12-hour clock instead of a 24 hour clock in the Acekard theme."));

	STR_AK_ZOOMING_ICON = ConvertFromUTF8(languageini.GetString("LANGUAGE", "AK_ZOOMING_ICON", "Zooming icons"));
	STR_DESCRIPTION_AK_ZOOMING_ICON = ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_ZOOMING_ICON", "Display a zoom effect for the selected icon in the Acekard theme."));

	STR_AK_SCROLLSPEED = ConvertFromUTF8(languageini.GetString("LANGUAGE", "AK_SCROLLSPEED", "Scroll speed"));
	STR_DESCRIPTION_AK_SCROLLSPEED =  ConvertFromUTF8(languageini.GetString("LANGUAGE", "DESCRIPTION_AK_SCROLLSPEED", "Sets the scroll speed in the Acekard theme."));
}
