
#include <nds.h>
#include <string>
#include "common/singleton.h"

#pragma once
#ifndef _DSIMENUPP_GBAR2_SETTINGS_H_
#define _DSIMENUPP_GBAR2_SETTINGS_H_

/**
 * Multi use class for DSiMenuPlusPlus INI file.
 */
class GBAR2Settings
{
  public:
    GBAR2Settings();
    ~GBAR2Settings();

  public:
    void loadSettings();
    void saveSettings();

  public:
    bool useBottomScreen;
    bool bottomScreenPrefered;
    bool frame;
    bool centerMask;
    bool gbaColors;
    bool mainMemICache;
    bool wramICache;
    bool skipIntro;
};

typedef singleton<GBAR2Settings> gbarunner2Settings_s;
inline GBAR2Settings &gs() { return gbarunner2Settings_s::instance(); }

#endif //_DSIMENUPP_GBAR2_SETTINGS_H_
