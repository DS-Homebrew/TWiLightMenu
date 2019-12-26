#include "easysave/ini.hpp"
#include "common/bootstrappaths.h"
#include "gbarunner2settings.h"
#include <string.h>

GBAR2Settings::GBAR2Settings()
{
	useBottomScreen = false;
	centerMask = true;
	mainMemICache = true;
	wramICache = true;
	skipIntro = false;
}

void GBAR2Settings::loadSettings()
{
	easysave::ini gbarunner2ini(GBARUNNER2_INI);

	// UI settings.
	useBottomScreen = (gbarunner2ini.GetString("emulation", "useBottomScreen", "false") == "false" ? false : true);
	centerMask = (gbarunner2ini.GetString("emulation", "centerMask", "true") == "true" ? true : false);
	mainMemICache = (gbarunner2ini.GetString("emulation", "mainMemICache", "true") == "true" ? true : false);
	wramICache = (gbarunner2ini.GetString("emulation", "wramICache", "true") == "true" ? true : false);
	skipIntro = (gbarunner2ini.GetString("emulation", "skipIntro", "false") == "false" ? false : true);
}

void GBAR2Settings::saveSettings()
{
	easysave::ini gbarunner2ini(GBARUNNER2_INI);

	gbarunner2ini.SetString("emulation", "useBottomScreen", useBottomScreen ? "true" : "false");
	gbarunner2ini.SetString("emulation", "centerMask", centerMask ? "true" : "false");
	gbarunner2ini.SetString("emulation", "mainMemICache", mainMemICache ? "true" : "false");
	gbarunner2ini.SetString("emulation", "wramICache", wramICache ? "true" : "false");
	gbarunner2ini.SetString("emulation", "skipIntro", skipIntro ? "true" : "false");
	gbarunner2ini.flush();
}