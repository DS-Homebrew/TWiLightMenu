#include <nds.h>
#include <stdio.h>
#include <fat.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>

#include "common/inifile.h"
#include "common/dsimenusettings.h"

// Strings
std::string STR_SAVING_SETTINGS = "STR_SAVING_SETTINGS";
std::string STR_SETTINGS_SAVED = "STR_SETTINGS_SAVED";

std::string STR_LR_SWITCH = "STR_LR_SWITCH";
std::string STR_GUI_SETTINGS = "STR_GUI_SETTINGS";
std::string STR_GAMESAPPS_SETTINGS = "STR_GAMESAPPS_SETTINGS";

// GUI settings
std::string STR_S1SDACCESS = "STR_S1SDACCESS";
std::string STR_THEME = "STR_THEME";
std::string STR_LASTPLAYEDROM = "STR_LASTPLAYEDROM";
std::string STR_DSIMENUPPLOGO = "STR_DSIMENUPPLOGO";
std::string STR_DIRECTORIES = "STR_DIRECTORIES";
std::string STR_BOXART = "STR_BOXART";
std::string STR_ANIMATEDSIICONS = "STR_ANIMATEDSIICONS";
std::string STR_SYSREGION = "STR_SYSREGION";
std::string STR_LAUNCHERAPP = "STR_LAUNCHERAPP";
std::string STR_SYSTEMSETTINGS = "STR_SYSTEMSETTINGS";
std::string STR_REPLACEDSIMENU = "STR_REPLACEDSIMENU";
std::string STR_RESTOREDSIMENU = "STR_RESTOREDSIMENU";

std::string STR_SHOW = "STR_SHOW";
std::string STR_HIDE = "STR_HIDE";

std::string STR_DESCRIPTION_S1SDACCESS_1 = "STR_DESCRIPTION_S1SDACCESS_1";

std::string STR_DESCRIPTION_THEME_1 = "STR_DESCRIPTION_THEME_1";

std::string STR_DESCRIPTION_LASTPLAYEDROM_1 = "STR_DESCRIPTION_LASTPLAYEDROM_1";

std::string STR_DESCRIPTION_DSIMENUPPLOGO_1 = "STR_DESCRIPTION_DSIMENUPPLOGO_1";

std::string STR_DESCRIPTION_DIRECTORIES_1 = "STR_DESCRIPTION_DIRECTORIES_1";

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
std::string STR_SOUNDFIX = "STR_SOUNDFIX";
std::string STR_DEBUG = "STR_DEBUG";
std::string STR_LOGGING = "STR_LOGGING";
std::string STR_ROMREADLED = "STR_ROMREADLED";
std::string STR_RUNIN = "STR_RUNIN";
std::string STR_SNDFREQ = "STR_SNDFREQ";
std::string STR_SLOT1LAUNCHMETHOD = "STR_SLOT1LAUNCHMETHOD";
std::string STR_LOADINGSCREEN = "STR_LOADINGSCREEN";
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

std::string STR_DESCRIPTION_LANGUAGE_1 = "STR_DESCRIPTION_LANGUAGE_1";

std::string STR_DESCRIPTION_RUNIN_1 = "STR_DESCRIPTION_RUNIN_1";

std::string STR_DESCRIPTION_CPUSPEED_1 = "STR_DESCRIPTION_CPUSPEED_1";

std::string STR_DESCRIPTION_VRAMBOOST_1 = "STR_DESCRIPTION_VRAMBOOST_1";

std::string STR_DESCRIPTION_SOUNDFIX_1 = "STR_DESCRIPTION_SOUNDFIX_1";

std::string STR_DESCRIPTION_DEBUG_1 = "STR_DESCRIPTION_DEBUG_1";

std::string STR_DESCRIPTION_LOGGING_1 = "STR_DESCRIPTION_LOGGING_1";

std::string STR_DESCRIPTION_ROMREADLED_1 = "STR_DESCRIPTION_ROMREADLED_1";

std::string STR_DESCRIPTION_SNDFREQ_1 = "STR_DESCRIPTION_SNDFREQ_1";

std::string STR_DESCRIPTION_SLOT1LAUNCHMETHOD_1 = "STR_DESCRIPTION_SLOT1LAUNCHMETHOD_1";

std::string STR_DESCRIPTION_LOADINGSCREEN_1 = "STR_DESCRIPTION_LOADINGSCREEN_1";

std::string STR_DESCRIPTION_BOOTSTRAP_1 = "STR_DESCRIPTION_BOOTSTRAP_1";

std::string STR_DESCRIPTION_FLASHCARD_1 = "STR_DESCRIPTION_FLASHCARD_1";

std::string STR_DESCRIPTION_GBARUNNER2_1 = "STR_DESCRIPTION_GBARUNNER2_1";

// Flashcard settings
std::string STR_FLASHCARD_SELECT = "STR_FLASHCARD_SELECT";
std::string STR_LEFTRIGHT_FLASHCARD = "STR_LEFTRIGHT_FLASHCARD";
std::string STR_AB_SETRETURN = "STR_AB_SETRETURN";

// Sub-theme select
std::string STR_SUBTHEMESEL_DSI = "STR_SUBTHEMESEL_DSI";

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

std::string STR_AK_ZOOMING_ICON = "STR_AK_ZOOMING_ICON";
std::string STR_DESCRIPTION_AK_ZOOMING_ICON = "STR_DESCRIPTION_AK_ZOOMING_ICON";

std::string STR_AK_SCROLLSPEED = "STR_AK_SCROLLSPEED";
std::string STR_DESCRIPTION_AK_SCROLLSPEED = "STR_DESCRIPTION_AK_SCROLLSPEED";

std::string STR_AK_VIEWMODE = "STR_AK_VIEWMODE";
std::string STR_DESCRIPTION_AK_VIEWMODE = "STR_DESCRIPTION_AK_VIEWMODE";


const char *languageIniPath;

int setLanguage = 0;

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
	}

	CIniFile languageini(languageIniPath);

	STR_SAVING_SETTINGS = languageini.GetString("LANGUAGE", "SAVING_SETTINGS", "Saving settings...");
	STR_SETTINGS_SAVED = languageini.GetString("LANGUAGE", "SETTINGS_SAVED", "Settings saved.");

	STR_LR_SWITCH = languageini.GetString("LANGUAGE", "LR_SWITCH", "L/R/Y/X Switch Pages");
	STR_GUI_SETTINGS = languageini.GetString("LANGUAGE", "GUI_SETTINGS", "GUI Settings");
	STR_GAMESAPPS_SETTINGS = languageini.GetString("LANGUAGE", "GAMESAPPS_SETTINGS", "Games and Apps Settings");

	// GUI settings
	STR_S1SDACCESS = languageini.GetString("LANGUAGE", "S1SDACCESS", "Slot-1 microSD access");
	STR_THEME = languageini.GetString("LANGUAGE", "THEME", "Theme");
	STR_LASTPLAYEDROM = languageini.GetString("LANGUAGE", "LASTPLAYEDROM", "Last played ROM on startup.");
	switch (ms().appName)
	{
	case 0:
	default:
		STR_DSIMENUPPLOGO = languageini.GetString("LANGUAGE", "DSIMENUPPLOGO", "TWLMenu++ logo on startup");
		break;
	case 1:
		STR_DSIMENUPPLOGO = languageini.GetString("LANGUAGE", "SRLOADERLOGO", "SRLoader on startup");
		break;
	case 2:
		STR_DSIMENUPPLOGO = languageini.GetString("LANGUAGE", "DSISIONXLOGO", "DSiMenu++ on startup");
		break;
	}
	STR_DIRECTORIES = languageini.GetString("LANGUAGE", "DIRECTORIES", "Directories/Folders");
	STR_BOXART = languageini.GetString("LANGUAGE", "BOXART", "Box art/Game covers");
	STR_ANIMATEDSIICONS = languageini.GetString("LANGUAGE", "ANIMATEDSIICONS", "Animate DSi icons");
	STR_SYSREGION = languageini.GetString("LANGUAGE", "SYSREGION", "SysNAND Region");
	STR_LAUNCHERAPP = languageini.GetString("LANGUAGE", "LAUNCHERAPP", "SysNAND Launcher");
	STR_SYSTEMSETTINGS = languageini.GetString("LANGUAGE", "SYSTEMSETTINGS", "System Settings");
	STR_REPLACEDSIMENU = languageini.GetString("LANGUAGE", "REPLACEDSIMENU", "Replace DSi Menu");
	STR_RESTOREDSIMENU = languageini.GetString("LANGUAGE", "RESTOREDSIMENU", "Restore DSi Menu");

	STR_SHOW = languageini.GetString("LANGUAGE", "SHOW", "Show");
	STR_HIDE = languageini.GetString("LANGUAGE", "HIDE", "Hide");

	STR_DESCRIPTION_S1SDACCESS_1 = languageini.GetString("LANGUAGE", "DESCRIPTION_S1SDACCESS_1", "Allows your flashcard to be used as a secondary device. Turn this off, if IR functionality doesn't work, or if the app crashes.");

	switch (ms().appName)
	{
	case 0:
	default:
		STR_DESCRIPTION_THEME_1 = languageini.GetString("LANGUAGE", "DESCRIPTION_THEME_1", "The theme to use in TWiLight Menu++. Press Left/Right to select, A for sub-themes.");

		break;
	case 1:
		STR_DESCRIPTION_THEME_1 = languageini.GetString("LANGUAGE", "DESCRIPTION_THEME_1_SRLOADER", "The theme to use in SRLoader Press Left/Right to select, A for sub-themes.");

		break;
	case 2:
		STR_DESCRIPTION_THEME_1 = languageini.GetString("LANGUAGE", "DESCRIPTION_THEME_1_DSISIONX", "The theme to use in DSiMenu++ Press Left/Right to select, A for sub-themes.");

		break;
	}

	STR_DESCRIPTION_LASTPLAYEDROM_1 = languageini.GetString("LANGUAGE", "DESCRIPTION_LASTPLAYEDROM_1", "If turned on, hold B on startup to skip to the  ROM select menu. Press Y to start last played ROM.");

	switch (ms().appName)
	{
	case 0:
	default:
		STR_DESCRIPTION_DSIMENUPPLOGO_1 = languageini.GetString("LANGUAGE", "DESCRIPTION_DSIMENUPPLOGO_1", "The logo will be shown when you start TWiLight Menu++.");

		break;
	case 1:
		STR_DESCRIPTION_DSIMENUPPLOGO_1 = languageini.GetString("LANGUAGE", "DESCRIPTION_SRLOADERLOGO_1", "The logo will be shown when you start SRLoader.");

		break;
	case 2:
		STR_DESCRIPTION_DSIMENUPPLOGO_1 = languageini.GetString("LANGUAGE", "DESCRIPTION_DSISIONXLOGO_1", "The logo will be shown when you start DSiMenu++.");

		break;
	}

	STR_DESCRIPTION_DIRECTORIES_1 = languageini.GetString("LANGUAGE", "DESCRIPTION_DIRECTORIES_1", "If you're in a folder where most of your games are, it is safe to hide directories/folders.");

	STR_DESCRIPTION_BOXART_1 = languageini.GetString("LANGUAGE", "DESCRIPTION_BOXART_1", "Displayed in the top screen of the DSi/3DS theme.");

	STR_DESCRIPTION_ANIMATEDSIICONS_1 = languageini.GetString("LANGUAGE", "DESCRIPTION_ANIMATEDSIICONS_1", "Animate DSi-enhanced icons like in the DSi/3DS menus.");

	STR_DESCRIPTION_SYSREGION_1 = languageini.GetString("LANGUAGE", "DESCRIPTION_SYSREGION_1", "The region of SysNAND. \"Auto\" option will only work if SDNAND is set up.");

	STR_DESCRIPTION_LAUNCHERAPP_1 = languageini.GetString("LANGUAGE", "DESCRIPTION_LAUNCHERAPP_1", "To get the .app name, press POWER, hold A, then highlight LAUNCHER.");

	STR_DESCRIPTION_SYSTEMSETTINGS_1 = languageini.GetString("LANGUAGE", "DESCRIPTION_SYSTEMSETTINGS_1", "Press A to change settings related to the DSi system.");

	switch (ms().appName)
	{
	case 0:
	default:
		STR_DESCRIPTION_REPLACEDSIMENU_1 = languageini.GetString("LANGUAGE", "DESCRIPTION_REPLACEDSIMENU_1", "Start TWiLight Menu++ on boot, instead of the DSi Menu.");

		break;
	case 1:
		STR_DESCRIPTION_REPLACEDSIMENU_1 = languageini.GetString("LANGUAGE", "DESCRIPTION_REPLACEDSIMENU_1_SRLOADER", "");

		break;
	case 2:
		STR_DESCRIPTION_REPLACEDSIMENU_1 = languageini.GetString("LANGUAGE", "DESCRIPTION_REPLACEDSIMENU_1_DSISIONX", "");

		break;
	}

	STR_DESCRIPTION_RESTOREDSIMENU_1 = languageini.GetString("LANGUAGE", "DESCRIPTION_RESTOREDSIMENU_1", "Show DSi Menu on boot again.");

	// Games/Apps settings
	STR_LANGUAGE = languageini.GetString("LANGUAGE", "LANGUAGE", "Language");
	STR_CPUSPEED = languageini.GetString("LANGUAGE", "CPUSPEED", "ARM9 CPU Speed");
	STR_VRAMBOOST = languageini.GetString("LANGUAGE", "VRAMBOOST", "VRAM boost");
	STR_SOUNDFIX = languageini.GetString("LANGUAGE", "SOUNDFIX", "Sound fix");
	STR_DEBUG = languageini.GetString("LANGUAGE", "DEBUG", "Debug");
	STR_LOGGING = languageini.GetString("LANGUAGE", "LOGGING", "Logging");
	STR_ROMREADLED = languageini.GetString("LANGUAGE", "ROMREADLED", "ROM read LED");
	STR_RUNIN = languageini.GetString("LANGUAGE", "RUNIN", "Run in");
	STR_SNDFREQ = languageini.GetString("LANGUAGE", "SNDFREQ", "Sound/Mic frequency");
	STR_SLOT1LAUNCHMETHOD = languageini.GetString("LANGUAGE", "SLOT1LAUNCHMETHOD", "Slot-1 launch method");
	STR_LOADINGSCREEN = languageini.GetString("LANGUAGE", "LOADINGSCREEN", "Loading screen");
	STR_BOOTSTRAP = languageini.GetString("LANGUAGE", "BOOTSTRAP", "Bootstrap");
	STR_USEGBARUNNER2 = languageini.GetString("LANGUAGE", "USEGBARUNNER2", "Use GBARunner2");

	STR_SYSTEM = languageini.GetString("LANGUAGE", "SYSTEM", "System");
	STR_ON = languageini.GetString("LANGUAGE", "ON", "On");
	STR_OFF = languageini.GetString("LANGUAGE", "OFF", "Off");
	STR_YES = languageini.GetString("LANGUAGE", "YES", "Yes");
	STR_NO = languageini.GetString("LANGUAGE", "NO", "No");
	STR_NONE = languageini.GetString("LANGUAGE", "NONE", "None");
	STR_POWER = languageini.GetString("LANGUAGE", "POWER", "Power");
	STR_CAMERA = languageini.GetString("LANGUAGE", "CAMERA", "Camera");
	STR_REBOOT = languageini.GetString("LANGUAGE", "REBOOT", "Reboot");
	STR_DIRECT = languageini.GetString("LANGUAGE", "DIRECT", "Direct");
	STR_REGULAR = languageini.GetString("LANGUAGE", "REGULAR", "Regular");
	STR_RELEASE = languageini.GetString("LANGUAGE", "RELEASE", "Release");
	STR_NIGHTLY = languageini.GetString("LANGUAGE", "NIGHTLY", "Nightly");

	STR_DESCRIPTION_LANGUAGE_1 = languageini.GetString("LANGUAGE", "DESCRIPTION_LANGUAGE_1", "Avoid the limited selections of your console language by setting this option.");

	STR_DESCRIPTION_RUNIN_1 = languageini.GetString("LANGUAGE", "DESCRIPTION_RUNIN_1", "Run in either DS or DSi mode.");

	STR_DESCRIPTION_CPUSPEED_1 = languageini.GetString("LANGUAGE", "DESCRIPTION_CPUSPEED_1", "Set to TWL to get rid of lags in some games.");

	STR_DESCRIPTION_VRAMBOOST_1 = languageini.GetString("LANGUAGE", "DESCRIPTION_VRAMBOOST_1", "Allow 8 bit VRAM writes and expands the bus to 32 bit.");

	STR_DESCRIPTION_SOUNDFIX_1 = languageini.GetString("LANGUAGE", "DESCRIPTION_SOUNDFIX_1", "Fixes most sound crackles doubles, and split-second pauses.");

	STR_DESCRIPTION_DEBUG_1 = languageini.GetString("LANGUAGE", "DESCRIPTION_DEBUG_1", "Displays some text before launched game.");

	STR_DESCRIPTION_LOGGING_1 = languageini.GetString("LANGUAGE", "DESCRIPTION_LOGGING_1", "Logs the process of patching to sd:/NDSBTSRP.LOG");

	STR_DESCRIPTION_ROMREADLED_1 = languageini.GetString("LANGUAGE", "DESCRIPTION_ROMREADLED_1", "Sets LED as ROM read indicator.");

	STR_DESCRIPTION_SNDFREQ_1 = languageini.GetString("LANGUAGE", "DESCRIPTION_SNDFREQ_1", "32.73kHz is original quality, 47.61kHz is high quality.");

	STR_DESCRIPTION_SLOT1LAUNCHMETHOD_1 = languageini.GetString("LANGUAGE", "DESCRIPTION_SLOT1LAUNCHMETHOD_1",
																"Change this if some Slot-1 cards are not booting. Please note the reboot method will not use your set language or CPU speed.");

	STR_DESCRIPTION_LOADINGSCREEN_1 = languageini.GetString("LANGUAGE", "DESCRIPTION_LOADINGSCREEN_1", "Shows a loading screen before ROM is started in nds-bootstrap");

	STR_DESCRIPTION_BOOTSTRAP_1 = languageini.GetString("LANGUAGE", "DESCRIPTION_BOOTSTRAP_1", "Pick release or nightly bootstrap");

	STR_DESCRIPTION_FLASHCARD_1 = languageini.GetString("LANGUAGE", "DESCRIPTION_FLASHCARD_1", "");

	STR_DESCRIPTION_GBARUNNER2_1 = languageini.GetString("LANGUAGE", "DESCRIPTION_GBARUNNER2_1", "");

	// Flashcard settings
	STR_FLASHCARD_SELECT = languageini.GetString("LANGUAGE", "FLASHCARD_SELECT", "Select Flashcard");
	STR_LEFTRIGHT_FLASHCARD = languageini.GetString("LANGUAGE", "LEFTRIGHT_FLASHCARD", "");
	STR_AB_SETRETURN = languageini.GetString("LANGUAGE", "AB_SETRETURN", "A/B: Set and Return");

	// Sub-theme select
	STR_SUBTHEMESEL_DSI = languageini.GetString("LANGUAGE", "SUBTHEMESEL_DSI", "Sub-theme select: DSi Menu");

	STR_SUBTHEMESEL_R4 = languageini.GetString("LANGUAGE", "SUBTHEMESEL_R4", "Sub-theme select: R4");
	STR_SUBTHEMESEL_AK = languageini.GetString("LANGUAGE", "SUBTHEMESEL_AK", "Sub-theme select: Acekard Menu");

	STR_AB_SETSUBTHEME = languageini.GetString("LANGUAGE", "AB_SETSUBTHEME", "A/B: Set sub-theme");
	STR_DSI_DARKMENU = languageini.GetString("LANGUAGE", "DSI_DARKMENU", "SD/Black");
	STR_DSI_NORMALMENU = languageini.GetString("LANGUAGE", "DSI_NORMALMENU", "Normal/White");
	STR_DSI_RED = languageini.GetString("LANGUAGE", "DSI_RED", "Red");
	STR_DSI_BLUE = languageini.GetString("LANGUAGE", "DSI_BLUE", "Blue");
	STR_DSI_GREEN = languageini.GetString("LANGUAGE", "DSI_GREEN", "Green");
	STR_DSI_YELLOW = languageini.GetString("LANGUAGE", "DSI_YELLOW", "Yellow");
	STR_DSI_PINK = languageini.GetString("LANGUAGE", "DSI_PINK", "Pink");
	STR_DSI_PURPLE = languageini.GetString("LANGUAGE", "DSI_PURPLE", "Purple");
	STR_R4_THEME01 = languageini.GetString("LANGUAGE", "R4_THEME01", "Snow hill");
	STR_R4_THEME02 = languageini.GetString("LANGUAGE", "R4_THEME02", "Snow land");
	STR_R4_THEME03 = languageini.GetString("LANGUAGE", "R4_THEME03", "Green leaf");
	STR_R4_THEME04 = languageini.GetString("LANGUAGE", "R4_THEME04", "Pink flower");
	STR_R4_THEME05 = languageini.GetString("LANGUAGE", "R4_THEME05", "Park");
	STR_R4_THEME06 = languageini.GetString("LANGUAGE", "R4_THEME06", "Cherry blossoms");
	STR_R4_THEME07 = languageini.GetString("LANGUAGE", "R4_THEME07", "Beach");
	STR_R4_THEME08 = languageini.GetString("LANGUAGE", "R4_THEME08", "Summer sky");
	STR_R4_THEME09 = languageini.GetString("LANGUAGE", "R4_THEME09", "River");
	STR_R4_THEME10 = languageini.GetString("LANGUAGE", "R4_THEME10", "Fall trees");
	STR_R4_THEME11 = languageini.GetString("LANGUAGE", "R4_THEME11", "Christmas tree");
	STR_R4_THEME12 = languageini.GetString("LANGUAGE", "R4_THEME12", "Drawn symbol");
	STR_R4_THEME13 = languageini.GetString("LANGUAGE", "R4_THEME13", "Blue moon");
	STR_R4_THEME14 = languageini.GetString("LANGUAGE", "R4_THEME14", "Mac-like");

	STR_DEFAULT_LAUNCHER = languageini.GetString("LANGUAGE", "DEFAULT_LAUNCHER", "Default launcher");

    STR_12_HOUR_CLOCK =  languageini.GetString("LANGUAGE", "12_HOUR_CLOCK", "Use a 12 hour clock");
    STR_DESCRIPTION_12_HOUR_CLOCK = languageini.GetString("LANGUAGE", "DESCRIPTION_12_HOUR_CLOCK", "Use a 12-hour clock instead of a 24 hour clock in the Acekard theme.");

    STR_AK_ZOOMING_ICON = languageini.GetString("LANGUAGE", "AK_ZOOMING_ICON", "Zooming icons");
    STR_DESCRIPTION_AK_ZOOMING_ICON = languageini.GetString("LANGUAGE", "DESCRIPTION_ZOOMING_ICON", "Display a zoom effect for the selected icon in the Acekard theme.");

    STR_AK_SCROLLSPEED = languageini.GetString("LANGUAGE", "AK_SCROLLSPEED", "Scroll speed");
    STR_DESCRIPTION_AK_SCROLLSPEED =  languageini.GetString("LANGUAGE", "DESCRIPTION_AK_SCROLLSPEED", "Sets the scroll speed in the Acekard theme.");

	// The localestrign is here but the setting isn't displayed
	// we can just keep the default viewmode
	// which is icons with rom internal names.
	// akmenu should save the view mode anyways when its changed with SELECT.
	
    STR_AK_VIEWMODE = languageini.GetString("LANGUAGE", "AK_VIEWMODE", "Default viewmode");
    STR_DESCRIPTION_AK_VIEWMODE = languageini.GetString("LANGUAGE", "DESCRIPTION_AK_VIEWMODE", "Sets the default view mode in the Acekard theme.");

	switch (ms().appName)
	{
	case 0:
	default:
		STR_DESCRIPTION_DEFAULT_LAUNCHER_1 = languageini.GetString("LANGUAGE", "DESCRIPTION_DEFAULT_LAUNCHER_1_DSIMENUPP", "Launch Nintendo DSi Menu or TWiLight Menu++ on boot.");
		break;
	case 1:
		STR_DESCRIPTION_DEFAULT_LAUNCHER_1 = languageini.GetString("LANGUAGE", "DESCRIPTION_DEFAULT_LAUNCHER_1_SRLOADER", "Launch Nintendo DSi Menu or SRLoader on boot.");
		break;
	case 2:
		STR_DESCRIPTION_DEFAULT_LAUNCHER_1 = languageini.GetString("LANGUAGE", "DESCRIPTION_DEFAULT_LAUNCHER_1_DSISIONX", "Launch Nintendo DSi Menu or DSiMenu++ on boot.");
		break;
	}
}
