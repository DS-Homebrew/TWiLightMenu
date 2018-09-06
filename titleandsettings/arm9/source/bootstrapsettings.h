
#include <nds.h>
#include <string>
#include "common/singleton.h"

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
    enum TROMReadLED
    {
        ELEDNone = 0,
        ELEDWifi = 1,
        ELEDPower = 2,
        ELEDCamera = 3
    };
    enum TLoadingScreen
    {
        ELoadingNone = 0,
        ELoadingRegular = 1,
        ELoadingPong = 2,
        ELoadingTicTacToe = 3
    };

  public:
    BootstrapSettings();
    ~BootstrapSettings();

  public:
    void loadSettings();
    void saveSettings();

  public:
    bool bstrap_debug;
	bool bstrap_logging;
	int bstrap_romreadled;
	int bstrap_loadingScreen;
};

typedef singleton<BootstrapSettings> bootstrapSettings_s;
inline BootstrapSettings &bs() { return bootstrapSettings_s::instance(); }

#endif //_DSIMENUPP_BTSRP_SETTINGS_H_
