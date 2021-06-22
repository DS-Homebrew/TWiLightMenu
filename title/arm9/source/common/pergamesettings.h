#ifndef PERGAMESETTINGS_H
#define PERGAMESETTINGS_H

#include <string>

extern bool perGameSettingsButtons;

extern bool perGameSettings_directBoot;	// Homebrew only
extern int perGameSettings_dsiMode;
extern int perGameSettings_language;
extern int perGameSettings_region;
extern int perGameSettings_boostCpu;
extern int perGameSettings_boostVram;
extern int perGameSettings_cardReadDMA;
extern int perGameSettings_asyncCardRead;
extern int perGameSettings_bootstrapFile;
extern int perGameSettings_wideScreen;
extern int perGameSettings_expandRomSpace;

void loadPerGameSettings(std::string filename);
std::string getSavExtension(void);



#endif //PERGAMESETTINGS_H
