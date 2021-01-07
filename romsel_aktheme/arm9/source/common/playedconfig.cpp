#include "playedconfig.h"
#include <algorithm>

const std::vector<std::string> PlayedConfig::getRecentlyPlayed(
    const std::string dirName)
{
    std::vector<std::string> recentlyPlayed;
    _recentlyPlayed.GetStringVector("RECENT", dirName, recentlyPlayed, ':');
    return recentlyPlayed;
}

const std::vector<std::string> PlayedConfig::getCustomOrder(
    const std::string dirName)
{
    std::vector<std::string> gameOrder;
    _gameOrder.GetStringVector("ORDER", dirName, gameOrder, ':');
    return gameOrder;
}

const int PlayedConfig::getTimesPlayed(const std::string dirName,
                                       const std::string fileName)
{
    return _timesPlayed.GetInt(dirName, fileName, 0);
}

void PlayedConfig::updateRecentlyPlayed(const std::string dirName,
                                        const std::string newRecent)
{
    std::vector<std::string> recentlyPlayed = getRecentlyPlayed(dirName);
    std::vector<std::string>::iterator it = std::find(recentlyPlayed.begin(), recentlyPlayed.end(), newRecent);
    if (it != recentlyPlayed.end())
    {
        recentlyPlayed.erase(it);
    }
    recentlyPlayed.insert(recentlyPlayed.begin(), newRecent);
    _recentlyPlayed.SetStringVector("RECENT", dirName, recentlyPlayed, ':');
    _recentlyPlayed.SaveIniFile(PLAYED_RECENTLY_PLAYED);
}


void PlayedConfig::incrementTimesPlayed(const std::string dirName,
                                        const std::string newRecent)
{
    int timesPlayed = getTimesPlayed(dirName, newRecent);
    _timesPlayed.SetInt(dirName, newRecent, timesPlayed + 1);
    _timesPlayed.SaveIniFile(PLAYED_TIMES_PLAYED);
}