#include "playedconfig.h"

const std::vector<std::string>
PlayedConfig::getRecentlyPlayed(const std::string dirName) {
  std::vector<std::string> recentlyPlayed;
  _recentlyPlayed.GetStringVector("RECENT", dirName, recentlyPlayed, ':');
  return recentlyPlayed;
}

const std::vector<std::string>
PlayedConfig::getCustomOrder(const std::string dirName) {
  std::vector<std::string> gameOrder;
  _gameOrder.GetStringVector("ORDER", dirName, gameOrder, ':');
  return gameOrder;
}

const int PlayedConfig::getTimesPlayed(const std::string dirName,
                                       const std::string fileName) {
  return _timesPlayed.GetInt(dirName, fileName, 0);
}