#include "themefilenames.h"
#include "common/dsimenusettings.h"
#include "common/systemdetails.h"
#include "tool/dbgtool.h"
#include "tool/stringtool.h"
#include <nds.h>
#include <stdio.h>

ThemeFilenames::ThemeFilenames() {
	std::string systemDirectory;
	switch (ms().theme) {
		case 0:
		default:
			systemDirectory =
				formatString(TFN_SYSTEM_UI_DIRECTORY "%s", 
					"dsi", ms().dsi_theme.c_str());
			break;
		case 1:
			systemDirectory =
				formatString(TFN_SYSTEM_UI_DIRECTORY "%s", 
					"3ds", ms()._3ds_theme.c_str());
			break;
		case 4:
			systemDirectory =
				formatString(TFN_SYSTEM_UI_DIRECTORY "%s", 
					"saturn",  "default");
			break;
	}
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
			switch (ms().theme) {
				case 0:
				default:
					_uiDirectory = TFN_FALLBACK_DSI_UI_DIRECTORY;
					break;
				case 1:
					_uiDirectory = TFN_FALLBACK_3DS_UI_DIRECTORY;
					break;
				case 4:
					_uiDirectory = TFN_FALLBACK_SATURN_UI_DIRECTORY;
					break;
			}
		}

		switch (ms().theme) {
			case 0:
			default:
				_fallbackDirectory = TFN_FALLBACK_DSI_UI_DIRECTORY;
				break;
			case 1:
				_fallbackDirectory = TFN_FALLBACK_3DS_UI_DIRECTORY;
				break;
			case 4:
				_fallbackDirectory = TFN_FALLBACK_SATURN_UI_DIRECTORY;
				break;
		}
	}
}