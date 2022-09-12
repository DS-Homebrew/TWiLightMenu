
#include "common/inifile.h"
#include "common/bootstrappaths.h"
#include "common/twlmenusettings.h"
#include "gbarunner2settings.h"
#include <string.h>

GBAR2Settings::GBAR2Settings()
{
	useBottomScreen = false;
	bottomScreenPrefered = false;
	frame = true;
	centerMask = true;
	gbaColors = false;
	mainMemICache = true;
	wramICache = true;
	skipIntro = false;
}

void GBAR2Settings::loadSettings()
{
	CIniFile gbarunner2ini(GBARUNNER2_INI);

	// UI settings.
	useBottomScreen = (gbarunner2ini.GetString("emulation", "useBottomScreen", "false")=="false" ? false : true);
	bottomScreenPrefered = (gbarunner2ini.GetString("emulation", "bottomScreenPrefered", "unset")=="unset" ? useBottomScreen : gbarunner2ini.GetString("emulation", "bottomScreenPrefered", "false")=="false" ? false : true);
	frame = (gbarunner2ini.GetString("emulation", "frame", "true")=="true" ? true : false);
	centerMask = (gbarunner2ini.GetString("emulation", "centerMask", "true")=="true" ? true : false);
	gbaColors = (gbarunner2ini.GetString("emulation", "gbaColors", "false")=="false" ? false : true);
	mainMemICache = (gbarunner2ini.GetString("emulation", "mainMemICache", "true")=="true" ? true : false);
	wramICache = (gbarunner2ini.GetString("emulation", "wramICache", "true")=="true" ? true : false);
	skipIntro = (gbarunner2ini.GetString("emulation", "skipIntro", "false")=="false" ? false : true);
}

void GBAR2Settings::saveSettings()
{
	gbar2Fix = true;
	CIniFile gbarunner2ini(GBARUNNER2_INI);

	gbarunner2ini.SetString("emulation", "useBottomScreen", (bottomScreenPrefered || ms().macroMode) ? "true" : "false");
	gbarunner2ini.SetString("emulation", "bottomScreenPrefered", bottomScreenPrefered ? "true" : "false");
	gbarunner2ini.SetString("emulation", "frame", frame ? "true" : "false");
	gbarunner2ini.SetString("emulation", "centerMask", centerMask ? "true" : "false");
	gbarunner2ini.SetString("emulation", "gbaColors", gbaColors ? "true" : "false");
	gbarunner2ini.SetString("emulation", "mainMemICache", mainMemICache ? "true" : "false");
	gbarunner2ini.SetString("emulation", "wramICache", wramICache ? "true" : "false");
	gbarunner2ini.SetString("emulation", "skipIntro", skipIntro ? "true" : "false");
	gbarunner2ini.SaveIniFile(GBARUNNER2_INI);
	gbar2Fix = false;
}
