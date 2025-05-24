#include "common/bootstrapsettings.h"

#include "common/bootstrappaths.h"
#include "common/flashcard.h"
#include "common/inifile.h"
#include "myDSiMode.h"

#include <string.h>

BootstrapSettings::BootstrapSettings()
{
	b4dsMode = 0;
	debug = false;
	logging = false;
	romreadled = BootstrapSettings::ELEDNone;
	dmaromreadled = BootstrapSettings::ELEDSame;
	preciseVolumeControl = false;
	soundFreq = TWLSettings::EFreq32KHz;
	sdNand = false;
	consoleModel = TWLSettings::EDSiRetail;
	bootstrapHotkey = KEY_L | KEY_DOWN | KEY_SELECT;
	saveRelocation = TWLSettings::ERelocOnSDCard;
}

void BootstrapSettings::loadSettings()
{
	if (access(bootstrapinipath, F_OK) != 0 && flashcardFound()) {
		bootstrapinipath = BOOTSTRAP_INI_FC; // Fallback to .ini path on flashcard, if not found on SD card, or if SD access is disabled
	}

	CIniFile bootstrapini(bootstrapinipath);

	debug = bootstrapini.GetInt("NDS-BOOTSTRAP", "DEBUG", debug);
	logging = bootstrapini.GetInt("NDS-BOOTSTRAP", "LOGGING", logging);
	if (dsiFeatures()) {
		b4dsMode = bootstrapini.GetInt("NDS-BOOTSTRAP", "B4DS_MODE", b4dsMode);
		romreadled = bootstrapini.GetInt("NDS-BOOTSTRAP", "ROMREAD_LED", romreadled);
		dmaromreadled = bootstrapini.GetInt("NDS-BOOTSTRAP", "DMA_ROMREAD_LED", dmaromreadled);
		preciseVolumeControl = bootstrapini.GetInt("NDS-BOOTSTRAP", "PRECISE_VOLUME_CONTROL", preciseVolumeControl);
	}
	soundFreq = (TWLSettings::TSoundFreq)bootstrapini.GetInt( "NDS-BOOTSTRAP", "SOUND_FREQ", soundFreq);
	sdNand = bootstrapini.GetInt( "NDS-BOOTSTRAP", "SDNAND", sdNand);
	consoleModel = (TWLSettings::TConsoleModel)bootstrapini.GetInt("NDS-BOOTSTRAP", "CONSOLE_MODEL", consoleModel);
	bootstrapHotkey = strtol(bootstrapini.GetString("NDS-BOOTSTRAP", "HOTKEY", "284").c_str(), NULL, 16);
	saveRelocation = (TWLSettings::TSaveRelocation)bootstrapini.GetInt( "NDS-BOOTSTRAP", "SAVE_RELOCATION", saveRelocation);
}

void BootstrapSettings::saveSettings()
{
	CIniFile bootstrapini(bootstrapinipath);

	bootstrapini.SetInt("NDS-BOOTSTRAP", "DEBUG", debug);
	bootstrapini.SetInt("NDS-BOOTSTRAP", "LOGGING", logging);
	if (dsiFeatures()) {
		bootstrapini.SetInt("NDS-BOOTSTRAP", "B4DS_MODE", b4dsMode);
		bootstrapini.SetInt("NDS-BOOTSTRAP", "ROMREAD_LED", romreadled);
		bootstrapini.SetInt("NDS-BOOTSTRAP", "DMA_ROMREAD_LED", dmaromreadled);
		bootstrapini.SetInt("NDS-BOOTSTRAP", "PRECISE_VOLUME_CONTROL", preciseVolumeControl);
	}
	if (sdFound()) {
		bootstrapini.SetInt( "NDS-BOOTSTRAP", "SDNAND", sdNand);
	}
	bootstrapini.SetInt( "NDS-BOOTSTRAP", "MACRO_MODE", ms().macroMode);
	bootstrapini.SetInt( "NDS-BOOTSTRAP", "SLEEP_MODE", ms().sleepMode);
	bootstrapini.SetInt( "NDS-BOOTSTRAP", "SOUND_FREQ", ms().soundFreq);
	bootstrapini.SetInt( "NDS-BOOTSTRAP", "CONSOLE_MODEL", consoleModel);

	char hotkey[8] = {0};
	itoa(bootstrapHotkey, hotkey, 16);
	bootstrapini.SetString("NDS-BOOTSTRAP", "HOTKEY", hotkey);
	bootstrapini.SetInt("NDS-BOOTSTRAP", "SAVE_RELOCATION", saveRelocation);

	bootstrapini.SaveIniFile(bootstrapinipath);
}
