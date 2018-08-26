#ifndef PERGAMESETTINGS_H
#define PERGAMESETTINGS_H

#include <string>

extern bool perGameSettingsButtons;

extern int perGameSettings_language;
extern int perGameSettings_boostCpu;
extern int perGameSettings_asyncPrefetch;

void loadPerGameSettings(std::string filename);
void savePerGameSettings(std::string filename);
void perGameSettings(std::string filename);



#endif //PERGAMESETTINGS_H
