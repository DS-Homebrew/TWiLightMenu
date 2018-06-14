#ifndef PERGAMESETTINGS_H
#define PERGAMESETTINGS_H

#include <string>

extern bool perGameSettingsButtons;

extern int perGameSettings_boostCpu;
extern int perGameSettings_noSoundStutter;

void perGameSettings(std::string filename, const char* username);



#endif //PERGAMESETTINGS_H
