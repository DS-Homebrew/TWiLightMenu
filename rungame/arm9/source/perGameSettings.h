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
extern int perGameSettings_swiHaltHook;
extern int perGameSettings_bootstrapFile;
extern int perGameSettings_wideScreen;
extern int perGameSettings_expandRomSpace;
extern int perGameSettings_dsiwareBooter;
extern int perGameSettings_useBootstrap;

void loadPerGameSettings(std::string filename);
std::string getSavExtension(void);
std::string getPubExtension(void);
std::string getPrvExtension(void);



#endif //PERGAMESETTINGS_H
