
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
    BootstrapSettings();
    ~BootstrapSettings();

  public:
    void loadSettings();

  public:
    int b4dsMode;
};

typedef singleton<BootstrapSettings> bootstrapSettings_s;
inline BootstrapSettings &bs() { return bootstrapSettings_s::instance(); }

#endif //_DSIMENUPP_BTSRP_SETTINGS_H_
