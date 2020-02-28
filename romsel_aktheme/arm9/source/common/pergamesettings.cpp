#include "pergamesettings.h"
#include "dsimenusettings.h"
#include "common/inifile.h"
#include "tool/stringtool.h"
#include "tool/dbgtool.h"
#include "bootstrappaths.h"
#include <string.h>
#include <cstdio>
#include "systemdetails.h"

extern bool sdFound(void);

PerGameSettings::PerGameSettings(const std::string &romFileName)
{
    _iniPath = formatString(PERGAMESETTINGS_PATH, (ms().secondaryDevice ? "fat:" : "sd:"), romFileName.c_str());
    language = ELangDefault;
	saveNo = 0;
	ramDiskNo = -1;
    boostCpu = EDefault;
    boostVram = EDefault;
    dsiMode = EDefault;
    directBoot = EFalse;
    heapShrink = EDefault;
    bootstrapFile = EDefault;
    wideScreen = EDefault;
    loadSettings();
}

void PerGameSettings::loadSettings()
{
    CIniFile pergameini(_iniPath);
    dbg_printf("CINI LOAD %s", _iniPath.c_str());
    directBoot = (TDefaultBool)pergameini.GetInt("GAMESETTINGS", "DIRECT_BOOT", ms().secondaryDevice);	// Homebrew only
    dsiMode = (TDefaultBool)pergameini.GetInt("GAMESETTINGS", "DSI_MODE", dsiMode);
	language = (TLanguage)pergameini.GetInt("GAMESETTINGS", "LANGUAGE", language);
	saveNo = pergameini.GetInt("GAMESETTINGS", "SAVE_NUMBER", 0);
	ramDiskNo = pergameini.GetInt("GAMESETTINGS", "RAM_DISK", -1);
	boostCpu = (TDefaultBool)pergameini.GetInt("GAMESETTINGS", "BOOST_CPU", boostCpu);
	boostVram = (TDefaultBool)pergameini.GetInt("GAMESETTINGS", "BOOST_VRAM", boostVram);
    heapShrink = (TDefaultBool)pergameini.GetInt("GAMESETTINGS", "HEAP_SHRINK", heapShrink);
    bootstrapFile = (TDefaultBool)pergameini.GetInt("GAMESETTINGS", "BOOTSTRAP_FILE", bootstrapFile);
    wideScreen = (TDefaultBool)pergameini.GetInt("GAMESETTINGS", "WIDESCREEN", wideScreen);
}

void PerGameSettings::saveSettings()
{
    CIniFile pergameini(_iniPath);
    pergameini.SetInt("GAMESETTINGS", "DIRECT_BOOT", directBoot);	// Homebrew only
    if (ms().useBootstrap || !ms().secondaryDevice) {
		pergameini.SetInt("GAMESETTINGS", "LANGUAGE", language);
		pergameini.SetInt("GAMESETTINGS", "SAVE_NUMBER", saveNo);
	}
	if (!ms().secondaryDevice) pergameini.SetInt("GAMESETTINGS", "RAM_DISK", ramDiskNo);
	if ((isDSiMode() && ms().useBootstrap) || !ms().secondaryDevice) {
		pergameini.SetInt("GAMESETTINGS", "DSI_MODE", dsiMode);
	}
	if (REG_SCFG_EXT != 0) {
		pergameini.SetInt("GAMESETTINGS", "BOOST_CPU", boostCpu);
		pergameini.SetInt("GAMESETTINGS", "BOOST_VRAM", boostVram);
	}
    if (ms().useBootstrap || !ms().secondaryDevice) {
		pergameini.SetInt("GAMESETTINGS", "HEAP_SHRINK", heapShrink);
		pergameini.SetInt("GAMESETTINGS", "BOOTSTRAP_FILE", bootstrapFile);
	}
	if (isDSiMode() && ms().consoleModel >= 2 && sdFound()) {
		pergameini.SetInt("GAMESETTINGS", "WIDESCREEN", wideScreen);
	}
    pergameini.SaveIniFile(_iniPath);
}

bool PerGameSettings::checkIfShowAPMsg() {
    CIniFile pergameini(_iniPath);
	if (pergameini.GetInt("GAMESETTINGS", "NO_SHOW_AP_MSG", 0) == 0) {
		return true;	// Show AP message
	}
	return false;	// Don't show AP message
}

void PerGameSettings::dontShowAPMsgAgain() {
    CIniFile pergameini(_iniPath);
	pergameini.SetInt("GAMESETTINGS", "NO_SHOW_AP_MSG", 1);
	pergameini.SaveIniFile(_iniPath);
}

std::string getSavExtension(int number) {
	switch (number) {
		case 0:
		default:
			return ".sav";
			break;
		case 1:
			return ".sav1";
			break;
		case 2:
			return ".sav2";
			break;
		case 3:
			return ".sav3";
			break;
		case 4:
			return ".sav4";
			break;
		case 5:
			return ".sav5";
			break;
		case 6:
			return ".sav6";
			break;
		case 7:
			return ".sav7";
			break;
		case 8:
			return ".sav8";
			break;
		case 9:
			return ".sav9";
			break;
	}
}

std::string getImgExtension(int number) {
	switch (number) {
		case 0:
		default:
			return ".img";
			break;
		case 1:
			return ".img1";
			break;
		case 2:
			return ".img2";
			break;
		case 3:
			return ".img3";
			break;
		case 4:
			return ".img4";
			break;
		case 5:
			return ".img5";
			break;
		case 6:
			return ".img6";
			break;
		case 7:
			return ".img7";
			break;
		case 8:
			return ".img8";
			break;
		case 9:
			return ".img9";
			break;
	}
}
