#ifndef FAVORITES_H
#define FAVORITES_H

#include <string>
#include <vector>

std::vector<std::string> &getFavorites();
bool isFavorite(const std::string &fullPath);
void toggleFavorite(const std::string &fullPath);

#endif // FAVORITES_H