#include "common/favorites.h"

#include <algorithm>
#include <sys/stat.h>

#include "common/inifile.h"
#include "common/systemdetails.h"

static const std::string &favoritesIniPath() {
	static std::string path = std::string(sys().isRunFromSD() ? "sd" : "fat") + ":/_nds/TWiLightMenu/extras/favorites.ini";
	return path;
}

std::vector<std::string> &getFavorites() {
	static std::vector<std::string> favorites;
	static bool loaded = false;
	if (!loaded) {
		loaded = true;
		CIniFile favoritesIni(favoritesIniPath());
		favoritesIni.GetStringVector("FAVORITES", "PATHS", favorites, '|'); // '|' isn't allowed in FAT names, and unlike ':' never appears in drive prefixes
	}
	return favorites;
}

bool isFavorite(const std::string &fullPath) {
	std::vector<std::string> &favorites = getFavorites();
	return std::find(favorites.begin(), favorites.end(), fullPath) != favorites.end();
}

void toggleFavorite(const std::string &fullPath) {
	std::vector<std::string> &favorites = getFavorites();
	std::vector<std::string>::iterator it = std::find(favorites.begin(), favorites.end(), fullPath);
	if (it != favorites.end()) {
		favorites.erase(it);
	} else {
		favorites.push_back(fullPath);
	}

	mkdir(sys().isRunFromSD() ? "sd:/_nds/TWiLightMenu/extras" : "fat:/_nds/TWiLightMenu/extras", 0777);
	CIniFile favoritesIni(favoritesIniPath());
	favoritesIni.SetStringVector("FAVORITES", "PATHS", favorites, '|');
	favoritesIni.SaveIniFile(favoritesIniPath());
}

void flagReturnToFavorites() {
	CIniFile favoritesIni(favoritesIniPath());
	favoritesIni.SetInt("FAVORITES", "LASTVIEW", 1);
	favoritesIni.SaveIniFile(favoritesIniPath());
}

bool consumeReturnToFavorites() {
	CIniFile favoritesIni(favoritesIniPath());
	if (favoritesIni.GetInt("FAVORITES", "LASTVIEW", 0) == 0) {
		return false;
	}
	favoritesIni.SetInt("FAVORITES", "LASTVIEW", 0);
	favoritesIni.SaveIniFile(favoritesIniPath());
	return true;
}
