
#include "common/inifile.h"
#include "common/bootstrappaths.h"
#include "common/dsimenusettings.h"
#include "bootstrapsettings.h"
#include <string.h>

BootstrapSettings::BootstrapSettings()
{
    bstrap_debug = false;
	bstrap_logging = false;
	bstrap_romreadled = BootstrapSettings::ELEDNone;
	bstrap_loadingScreen = BootstrapSettings::ELoadingRegular;
	bstrap_loadingScreenTheme = false;
	bstrap_loadingScreenLocation = false;
	loadingFrames = 0;
	loadingFps = 0;
}

void BootstrapSettings::loadSettings()
{
    CIniFile bootstrapini(BOOTSTRAP_INI);

    // UI settings.
   	bstrap_debug = bootstrapini.GetInt("NDS-BOOTSTRAP", "DEBUG", bstrap_debug);
	bstrap_logging = bootstrapini.GetInt("NDS-BOOTSTRAP", "LOGGING", bstrap_logging);
	if (isDSiMode()) bstrap_romreadled = bootstrapini.GetInt("NDS-BOOTSTRAP", "ROMREAD_LED", bstrap_romreadled);
	bstrap_loadingScreen = bootstrapini.GetInt( "NDS-BOOTSTRAP", "LOADING_SCREEN", bstrap_loadingScreen);
	bstrap_loadingScreenTheme = bootstrapini.GetInt( "NDS-BOOTSTRAP", "LOADING_DARK_THEME", bstrap_loadingScreenTheme);
	bstrap_loadingScreenLocation = bootstrapini.GetInt( "NDS-BOOTSTRAP", "LOADING_SWAP_LCDS", bstrap_loadingScreenLocation);

}

void BootstrapSettings::saveSettings()
{
	std::string loadingFolder;
	std::string themeIniPath;
	switch (ms().theme) {
		case 0:
		default:
			loadingFolder = "/_nds/TWiLightMenu/dsimenu/loading/";
			themeIniPath = loadingFolder+"config.ini";
			break;
		case 1:
			loadingFolder = "/_nds/TWiLightMenu/3dsmenu/loading/";
			themeIniPath = loadingFolder+"config.ini";
			break;
		case 2:
			loadingFolder = "/_nds/TWiLightMenu/r4menu/themes/"+(ms().r4_theme)+"/loading/";
			themeIniPath = loadingFolder+"theme.ini";
			break;
		case 3:
			loadingFolder = "/_nds/TWiLightMenu/akmenu/themes/"+(ms().ak_theme)+"/loading/";
			themeIniPath = loadingFolder+"uisettings.ini";
			break;
	}

	CIniFile themeini(themeIniPath);
	switch (ms().theme) {
		case 0:
		default:
			loadingFrames = themeini.GetInt( "loading screen", "frames", 0);
			loadingFps = themeini.GetInt( "loading screen", "fps", 0);
			break;
		case 1:
			loadingFrames = themeini.GetInt( "loading screen", "frames", 0);
			loadingFps = themeini.GetInt( "loading screen", "fps", 0);
			break;
		case 2:
			loadingFrames = themeini.GetInt( "Loading", "Frames", 0);
			loadingFps = themeini.GetInt( "Loading", "Fps", 0);
			break;
		case 3:
			loadingFrames = themeini.GetInt( "loading screen", "frames", 0);
			loadingFps = themeini.GetInt( "loading screen", "fps", 0);
			break;
	}

    CIniFile bootstrapini(BOOTSTRAP_INI);

    // UI settings.
    bootstrapini.SetInt("NDS-BOOTSTRAP", "DEBUG", bstrap_debug);
	bootstrapini.SetInt("NDS-BOOTSTRAP", "LOGGING", bstrap_logging);
	if (isDSiMode()) bootstrapini.SetInt("NDS-BOOTSTRAP", "ROMREAD_LED", bstrap_romreadled);
	bootstrapini.SetInt( "NDS-BOOTSTRAP", "LOADING_SCREEN", bstrap_loadingScreen);
	bootstrapini.SetInt( "NDS-BOOTSTRAP", "LOADING_DARK_THEME", bstrap_loadingScreenTheme);
	bootstrapini.SetInt( "NDS-BOOTSTRAP", "LOADING_SWAP_LCDS", bstrap_loadingScreenLocation);
	bootstrapini.SetString( "NDS-BOOTSTRAP", "LOADING_SCREEN_FOLDER", loadingFolder);
	bootstrapini.SetInt( "NDS-BOOTSTRAP", "LOADING_FRAMES", loadingFrames);
	bootstrapini.SetInt( "NDS-BOOTSTRAP", "LOADING_FPS", loadingFps);
    bootstrapini.SaveIniFile(BOOTSTRAP_INI);
}
