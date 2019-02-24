#include "themefilenames.h"
#include "common/dsimenusettings.h"
#include "common/systemdetails.h"
#include "tool/dbgtool.h"
#include "tool/stringtool.h"
#include <nds.h>
#include <stdio.h>

ThemeFilenames::ThemeFilenames() {
	std::string systemDirectory =
	    formatString(TFN_SYSTEM_UI_DIRECTORY "%s/%s", ms().theme == 1 ? "3ds" : "dsi", ms().dsi_theme.c_str());
	if (!sys().useNitroFS()) {
		// nocashMessage("NNONITROFS");
		// nocashMessage(systemDirectory.c_str());
		_uiDirectory = systemDirectory;
		_fallbackDirectory = systemDirectory;
	} else {
		if (access((systemDirectory + "/theme.ini").c_str(), F_OK) == 0) {
			// nocashMessage("NITROFSOK");
			// nocashMessage(systemDirectory.c_str());
			_uiDirectory = systemDirectory;
		} else {
			_uiDirectory = ms().theme == 1 ? TFN_FALLBACK_3DS_UI_DIRECTORY : TFN_FALLBACK_DSI_UI_DIRECTORY;
		}

		_fallbackDirectory = ms().theme == 1 ? TFN_FALLBACK_3DS_UI_DIRECTORY : TFN_FALLBACK_DSI_UI_DIRECTORY;
	}
}