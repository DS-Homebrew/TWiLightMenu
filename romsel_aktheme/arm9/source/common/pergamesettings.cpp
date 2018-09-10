#include "pergamesettings.h"
#include "dsimenusettings.h"
#include "common/inifile.h"
#include "tool/stringtool.h"
#include "tool/dbgtool.h"
#include "bootstrappaths.h"
#include <string.h>
#include <cstdio>
#include "systemdetails.h"

PerGameSettings::PerGameSettings(const std::string &romFileName)
{
    _iniPath = formatString(PERGAMESETTINGS_PATH, (ms().secondaryDevice ? "fat:" : "sd:"), romFileName.c_str());
    language = ELangDefault;
    boostCpu = EDefault;
    boostVram = EDefault;
    soundFix = EDefault;
    asyncPrefetch = EDefault;
    directBoot = EFalse;
    dsiMode = EFalse;
    loadSettings();
}

void PerGameSettings::loadSettings()
{
    CIniFile pergameini(_iniPath);
    dbg_printf("CINI LOAD %s", _iniPath.c_str());
    directBoot = (TDefaultBool)pergameini.GetInt("GAMESETTINGS", "DIRECT_BOOT", ms().secondaryDevice);	// Homebrew only
    dsiMode = (TDefaultBool)pergameini.GetInt("GAMESETTINGS", "DSI_MODE", dsiMode);
	language = (TLanguage)pergameini.GetInt("GAMESETTINGS", "LANGUAGE", language);
	boostCpu = (TDefaultBool)pergameini.GetInt("GAMESETTINGS", "BOOST_CPU", boostCpu);
	boostVram = (TDefaultBool)pergameini.GetInt("GAMESETTINGS", "BOOST_VRAM", boostVram);
	soundFix = (TDefaultBool)pergameini.GetInt("GAMESETTINGS", "SOUND_FIX", soundFix);
	asyncPrefetch = (TDefaultBool)pergameini.GetInt("GAMESETTINGS", "ASYNC_PREFETCH", asyncPrefetch);
}

void PerGameSettings::saveSettings()
{
    CIniFile pergameini(_iniPath);
    pergameini.SetInt("GAMESETTINGS", "DIRECT_BOOT", directBoot);	// Homebrew only
    if (isDSiMode() && !ms().secondaryDevice) pergameini.SetInt("GAMESETTINGS", "LANGUAGE", language);
    pergameini.SetInt("GAMESETTINGS", "BOOST_CPU", boostCpu);
    pergameini.SetInt("GAMESETTINGS", "BOOST_VRAM", boostVram);
	if (isDSiMode() && !ms().secondaryDevice) {
		pergameini.SetInt("GAMESETTINGS", "SOUND_FIX", soundFix);
		pergameini.SetInt("GAMESETTINGS", "ASYNC_PREFETCH", asyncPrefetch);
	}
    pergameini.SaveIniFile(_iniPath);
}