
#include "common/inifile.h"
#include "common/bootstrappaths.h"
#include "gbarunner2settings.h"
#include <string.h>

GBAR2Settings::GBAR2Settings()
{
    useBottomScreen = false;
    centerMask = true;
    skipIntro = false;
}

void GBAR2Settings::loadSettings()
{
    CIniFile gbarunner2ini(GBARUNNER2_INI);

    // UI settings.
   	useBottomScreen = (gbarunner2ini.GetString("emulation", "useBottomScreen", "false")=="false" ? false : true);
   	centerMask = (gbarunner2ini.GetString("emulation", "centerMask", "true")=="true" ? true : false);
   	skipIntro = (gbarunner2ini.GetString("emulation", "skipIntro", "false")=="false" ? false : true);

}

void GBAR2Settings::saveSettings()
{
    CIniFile gbarunner2ini(GBARUNNER2_INI);

   	gbarunner2ini.SetString("emulation", "useBottomScreen", useBottomScreen ? "true" : "false");
   	gbarunner2ini.SetString("emulation", "centerMask", centerMask ? "true" : "false");
   	gbarunner2ini.SetString("emulation", "skipIntro", skipIntro ? "true" : "false");
    gbarunner2ini.SaveIniFile(GBARUNNER2_INI);
}
