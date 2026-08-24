#ifndef FAVORITES_H
#define FAVORITES_H

#include <string>
#include <vector>

std::vector<std::string> &getFavorites();
bool isFavorite(const std::string &fullPath);
void toggleFavorite(const std::string &fullPath);
void flagReturnToFavorites();
bool consumeReturnToFavorites();

#endif // FAVORITES_H