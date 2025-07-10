#include "common/twlmenusettings.h"
#include "ndsheaderbanner.h"
#include "perGameSettings.h"
#include <vector>
#include <algorithm>
#include <unistd.h>
#include <string>
#include <sstream>
#include <stdio.h>
#include <dirent.h>

#include <nds.h>

#include "myDSiMode.h"
#include "common/inifile.h"
#include "common/flashcard.h"
#include "common/nds_loader_arm9.h"

int perGameSettings_cursorPosition = 0;
bool perGameSettings_directBoot = false;	// Homebrew only
int perGameSettings_dsiMode = -1;
int perGameSettings_dsPhatColors = -1;
int perGameSettings_language = -2;
int perGameSettings_region = -2;
int perGameSettings_saveNo = 0;
int perGameSettings_ramDiskNo = -1;
int perGameSettings_boostCpu = -1;
int perGameSettings_boostVram = -1;
int perGameSettings_cardReadDMA = -1;
int perGameSettings_asyncCardRead = -1;
int perGameSettings_bootstrapFile = -1;
int perGameSettings_wideScreen = -1;
int perGameSettings_dsiwareBooter = -1;
int perGameSettings_useBootstrap = -1;
int perGameSettings_saveRelocation = -1;

char pergamefilepath[256];

void loadPerGameSettings (std::string filename) {
	snprintf(pergamefilepath, sizeof(pergamefilepath), "%s/_nds/TWiLightMenu/gamesettings/%s.ini", (ms().secondaryDevice ? "fat:" : "sd:"), filename.c_str());
	CIniFile pergameini(pergamefilepath);
	perGameSettings_directBoot = pergameini.GetInt("GAMESETTINGS", "DIRECT_BOOT", (isModernHomebrew[ms().secondaryDevice] || ms().secondaryDevice));	// Homebrew only
	if (isHomebrew[ms().secondaryDevice]) {
		perGameSettings_dsiMode = pergameini.GetInt("GAMESETTINGS", "DSI_MODE", (isModernHomebrew[ms().secondaryDevice] ? true : false));
	} else {
		perGameSettings_dsiMode = pergameini.GetInt("GAMESETTINGS", "DSI_MODE", -1);
	}
	perGameSettings_dsPhatColors = pergameini.GetInt("GAMESETTINGS", "PHAT_COLORS", -1);
	perGameSettings_language = pergameini.GetInt("GAMESETTINGS", "LANGUAGE", -2);
	perGameSettings_region = pergameini.GetInt("GAMESETTINGS", "REGION", -2);
	if (perGameSettings_region < -2 || (!dsiFeatures() && perGameSettings_region == -1)) perGameSettings_region = -2;
	perGameSettings_saveNo = pergameini.GetInt("GAMESETTINGS", "SAVE_NUMBER", 0);
	perGameSettings_ramDiskNo = pergameini.GetInt("GAMESETTINGS", "RAM_DISK", -1);
	perGameSettings_boostCpu = pergameini.GetInt("GAMESETTINGS", "BOOST_CPU", -1);
	perGameSettings_boostVram = pergameini.GetInt("GAMESETTINGS", "BOOST_VRAM", -1);
	perGameSettings_cardReadDMA = pergameini.GetInt("GAMESETTINGS", "CARD_READ_DMA", -1);
	perGameSettings_asyncCardRead = pergameini.GetInt("GAMESETTINGS", "ASYNC_CARD_READ", -1);
	perGameSettings_bootstrapFile = pergameini.GetInt("GAMESETTINGS", "BOOTSTRAP_FILE", -1);
	perGameSettings_wideScreen = pergameini.GetInt("GAMESETTINGS", "WIDESCREEN", -1);
	perGameSettings_dsiwareBooter = pergameini.GetInt("GAMESETTINGS", "DSIWARE_BOOTER", -1);
	perGameSettings_useBootstrap = pergameini.GetInt("GAMESETTINGS", "USE_BOOTSTRAP", -1);
	perGameSettings_saveRelocation = pergameini.GetInt("GAMESETTINGS", "SAVE_RELOCATION", -1);
}

std::string getSavExtension(void) {
	if (ms().saveLocation == TWLSettings::EGamesFolder || perGameSettings_saveNo == 0) {
		return ".sav";
	} else {
		return ".sav" + std::to_string(perGameSettings_saveNo);
	}
}

std::string getPubExtension(void) {
	if (ms().saveLocation == TWLSettings::EGamesFolder || perGameSettings_saveNo == 0) {
		return ".pub";
	} else {
		return ".pu" + std::to_string(perGameSettings_saveNo);
	}
}

std::string getPrvExtension(void) {
	if (ms().saveLocation == TWLSettings::EGamesFolder || perGameSettings_saveNo == 0) {
		return ".prv";
	} else {
		return ".pr" + std::to_string(perGameSettings_saveNo);
	}
}

std::string getImgExtension(void) {
	if (perGameSettings_ramDiskNo == 0) {
		return ".img";
	} else {
		return ".img" + std::to_string(perGameSettings_ramDiskNo);
	}
}
