
#include <nds.h>
#include <string>
#include "common/twlmenusettings.h"

#pragma once
#ifndef _DSIMENUPP_BTSRP_SETTINGS_H_
#define _DSIMENUPP_BTSRP_SETTINGS_H_

/**
 * Multi use class for DSiMenuPlusPlus INI file.
 *
 * Try not to change settings that are not related to the current theme.
 */
class BootstrapSettings
{
public:
	const char *bootstrapinipath = BOOTSTRAP_INI;

	enum TROMReadLED
	{
		ELEDSame = -1,
		ELEDNone = 0,
		ELEDWifi = 1,
		ELEDPower = 2,
		ELEDCamera = 3
	};

public:
	BootstrapSettings();
	~BootstrapSettings();

public:
	void loadSettings();
	void saveSettings();

public:
	int b4dsMode;
	bool debug;
	bool logging;
	int romreadled;
	int dmaromreadled;
	bool preciseVolumeControl;
	TWLSettings::TSoundFreq soundFreq;
	bool sdNand;
	TWLSettings::TConsoleModel consoleModel;
	int bootstrapHotkey;
	TWLSettings::TSaveRelocation saveRelocation;

};

typedef singleton<BootstrapSettings> bootstrapSettings_s;
inline BootstrapSettings &bs() { return bootstrapSettings_s::instance(); }

#endif //_DSIMENUPP_BTSRP_SETTINGS_H_
