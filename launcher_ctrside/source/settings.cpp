#include "settings.h"
#include "inifile.h"

#include <unistd.h>
#include <string>
using std::string;
using std::wstring;

#include <3ds.h>

static CIniFile settingsini( "sdmc:/_nds/dsimenuplusplus/settings.ini" );

// Settings
Settings_t settings;

/**
 * Save settings.
 */
void SaveSettings(void) {
	// UI settings.
	bool isNew = 0;
	APT_CheckNew3DS(&isNew);
	if (isNew) {
		settings.twl.consoleModel = 3;
	} else {
		settings.twl.consoleModel = 2;
	}
	settingsini.SetInt("SRLOADER", "CONSOLE_MODEL", settings.twl.consoleModel);
	settingsini.SaveIniFile("sdmc:/_nds/dsimenuplusplus/settings.ini");
}
