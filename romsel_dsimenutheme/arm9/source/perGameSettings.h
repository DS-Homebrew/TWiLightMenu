#ifndef PERGAMESETTINGS_H
#define PERGAMESETTINGS_H

#include <string>

extern bool perGameSettingsButtons;

extern bool perGameSettings_directBoot;	// Homebrew only
extern int perGameSettings_dsiMode;
extern int perGameSettings_dsPhatColors;
extern int perGameSettings_language;
extern int perGameSettings_region;
extern int perGameSettings_ramDiskNo;
extern int perGameSettings_boostCpu;
extern int perGameSettings_boostVram;
extern int perGameSettings_cardReadDMA;
extern int perGameSettings_asyncCardRead;
extern int perGameSettings_bootstrapFile;
extern int perGameSettings_wideScreen;
extern int perGameSettings_expandRomSpace;
extern int perGameSettings_dsiwareBooter;
extern int perGameSettings_useBootstrap;
extern int perGameSettings_useBootstrapCheat;
extern int perGameSettings_saveRelocation;

extern char fileCounter[8];

void loadPerGameSettings(std::string filename);
void savePerGameSettings(std::string filename);
bool checkIfShowAPMsg (std::string filename);
void dontShowAPMsgAgain (std::string filename);
bool checkIfShowRAMLimitMsg (std::string filename);
void dontShowRAMLimitMsgAgain (std::string filename);
bool checkIfDSiMode (std::string filename);
void perGameSettings(std::string filename, bool* dsiBinariesFound, bool* dsiBinariesChecked);
std::string getSavExtension(void);
std::string getPubExtension(void);
std::string getPrvExtension(void);
std::string getImgExtension(int number);



#endif //PERGAMESETTINGS_H
