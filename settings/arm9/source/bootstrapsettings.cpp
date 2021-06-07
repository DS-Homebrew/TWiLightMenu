
#include "myDSiMode.h"
#include "common/inifile.h"
#include "common/bootstrappaths.h"
#include "common/dsimenusettings.h"
#include "common/flashcard.h"
#include "bootstrapsettings.h"
#include <string.h>

BootstrapSettings::BootstrapSettings()
{
    cacheFatTable = false;
    debug = false;
	logging = false;
	romreadled = BootstrapSettings::ELEDNone;
	dmaromreadled = BootstrapSettings::ELEDSame;
	preciseVolumeControl = false;
    soundFreq = false;
    sdNand = false;
}

void BootstrapSettings::loadSettings()
{
    CIniFile bootstrapini(BOOTSTRAP_INI);

   	debug = bootstrapini.GetInt("NDS-BOOTSTRAP", "DEBUG", debug);
	logging = bootstrapini.GetInt("NDS-BOOTSTRAP", "LOGGING", logging);
	if (dsiFeatures()) {
		cacheFatTable = bootstrapini.GetInt("NDS-BOOTSTRAP", "CACHE_FAT_TABLE", cacheFatTable);
		romreadled = bootstrapini.GetInt("NDS-BOOTSTRAP", "ROMREAD_LED", romreadled);
		dmaromreadled = bootstrapini.GetInt("NDS-BOOTSTRAP", "DMA_ROMREAD_LED", dmaromreadled);
		preciseVolumeControl = bootstrapini.GetInt("NDS-BOOTSTRAP", "PRECISE_VOLUME_CONTROL", preciseVolumeControl);
	}
	soundFreq = bootstrapini.GetInt( "NDS-BOOTSTRAP", "SOUND_FREQ", soundFreq);
	sdNand = bootstrapini.GetInt( "NDS-BOOTSTRAP", "SDNAND", sdNand);

	bootstrapHotkey = strtol(bootstrapini.GetString("NDS-BOOTSTRAP", "HOTKEY", "284").c_str(), NULL, 16);

}

void BootstrapSettings::saveSettings()
{
    CIniFile bootstrapini(BOOTSTRAP_INI);

    bootstrapini.SetInt("NDS-BOOTSTRAP", "DEBUG", debug);
	bootstrapini.SetInt("NDS-BOOTSTRAP", "LOGGING", logging);
	if (dsiFeatures()) {
		bootstrapini.SetInt("NDS-BOOTSTRAP", "CACHE_FAT_TABLE", cacheFatTable);
		bootstrapini.SetInt("NDS-BOOTSTRAP", "ROMREAD_LED", romreadled);
		bootstrapini.SetInt("NDS-BOOTSTRAP", "DMA_ROMREAD_LED", dmaromreadled);
		bootstrapini.SetInt("NDS-BOOTSTRAP", "PRECISE_VOLUME_CONTROL", preciseVolumeControl);
	}
	if (sdFound()) {
		bootstrapini.SetInt( "NDS-BOOTSTRAP", "SDNAND", sdNand);
	}
	bootstrapini.SetInt( "NDS-BOOTSTRAP", "MACRO_MODE", ms().macroMode);
	//bootstrapini.SetInt( "NDS-BOOTSTRAP", "COLOR_MODE", ms().colorMode);
	bootstrapini.SetInt( "NDS-BOOTSTRAP", "SOUND_FREQ", ms().soundFreq);

	char hotkey[8] = {0};
	itoa(bootstrapHotkey, hotkey, 16);
	bootstrapini.SetString("NDS-BOOTSTRAP", "HOTKEY", hotkey);

    bootstrapini.SaveIniFile(BOOTSTRAP_INI);
}
