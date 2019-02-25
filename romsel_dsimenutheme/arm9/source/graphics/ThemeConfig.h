#include <nds.h>
#include <string>
#include "common/singleton.h"

#pragma once
#ifndef _THEMECONFIG_H_
#define _THEMECONFIG_H_

class ThemeConfig {
    
};

typedef singleton<ThemeConfig> themeConfig_s;
inline ThemeConfig &ms() { return themeConfig_s::instance(); }

#endif