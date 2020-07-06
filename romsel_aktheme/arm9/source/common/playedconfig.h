#include "singleton.h"
#include "inifile.h"

#include <string>

#pragma once
#ifndef __PLAYED_CONFIG__
#define __PLAYED_CONFIG__

#define PLAYED_GAME_ORDER              "/_nds/TWiLightMenu/extras/gameorder.ini"
#define PLAYED_RECENTLY_PLAYED         "/_nds/TWiLightMenu/extras/recentlyplayed.ini"
#define PLAYED_TIMES_PLAYED            "/_nds/TWiLightMenu/extras/timesplayed.ini"

struct TimesPlayed {
	std::string name;
	int amount;

    TimesPlayed(std::string name, int amount) : name(name), amount(amount) {}
};

class PlayedConfig {
    public:
        PlayedConfig() 
            : _recentlyPlayed(PLAYED_RECENTLY_PLAYED), _timesPlayed(PLAYED_TIMES_PLAYED), _gameOrder(PLAYED_GAME_ORDER) {}
        ~PlayedConfig() {}
        const std::vector<std::string> getRecentlyPlayed(const std::string dirName);
        const std::vector<std::string> getCustomOrder(const std::string dirName);
        const int getTimesPlayed(const std::string dirName, const std::string fileName);
    private:
        CIniFile _recentlyPlayed;
        CIniFile _timesPlayed;
        CIniFile _gameOrder;
};

typedef singleton<PlayedConfig> playedConfig_s;
inline PlayedConfig &played() { return playedConfig_s::instance(); }

#endif