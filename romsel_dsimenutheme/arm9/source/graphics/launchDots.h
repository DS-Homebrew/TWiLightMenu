#pragma once
#ifndef __TWILIGHTMENU_LAUNCHDOTS__
#define __TWILIGHTMENU_LAUNCHDOTS__

#include <nds.h>

#include "common/singleton.h"

class LaunchDots
{
    public: 
        LaunchDots();
        void draw();
};

typedef singleton<LaunchDots> launchDots_s;

inline LaunchDots &dots() { return launchDots_s::instance(); }
#endif