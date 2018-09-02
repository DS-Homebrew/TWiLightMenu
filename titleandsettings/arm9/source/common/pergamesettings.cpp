#include "pergamesettings.h"
#include "common/inifile.h"
#include "tool/stringtool.h"
#include "bootstrappaths.h"
#include <string.h>
#include <cstdio>
#include "systemdetails.h"

PerGameSettings::PerGameSettings(const std::string &romFileName)
{
    _iniPath = formatString(PERGAMESETTINGS_PATH, romFileName.c_str());
    language = ELangDefault;
    boostCpu = EDefault;
    boostVram = EDefault;
    soundFix = EDefault;
    asyncPrefetch = EDefault;
    directBoot = EDefault;
    loadSettings();
}

void PerGameSettings::loadSettings()
{
    CIniFile pergameini(_iniPath);
    directBoot = (TDefaultBool)pergameini.GetInt("GAMESETTINGS", "DIRECT_BOOT", directBoot);	// Homebrew only
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
    pergameini.SetInt("GAMESETTINGS", "LANGUAGE", language);
    pergameini.SetInt("GAMESETTINGS", "BOOST_CPU", boostCpu);
    pergameini.SetInt("GAMESETTINGS", "BOOST_VRAM", boostVram);
    pergameini.SetInt("GAMESETTINGS", "SOUND_FIX", soundFix);
    pergameini.SetInt("GAMESETTINGS", "ASYNC_PREFETCH", asyncPrefetch);
    pergameini.SaveIniFile(_iniPath);
}