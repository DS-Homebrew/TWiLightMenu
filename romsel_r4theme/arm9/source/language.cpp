#include <nds.h>
#include <stdio.h>
#include <fat.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>

#include "common/inifile.h"
//#include "common/nitrofs.h"

extern bool useTwlCfg;

const char* languageIniPath;

extern int guiLanguage;
extern int bstrap_language;
extern int titleLanguage;
int setLanguage = 0;
int setGameLanguage = 0;
int setTitleLanguage = 0;

/**
 * Initialize translations.
 * Uses the language ID specified in settings.ui.language.
 */
void langInit(void)
{
	int userLanguage = (useTwlCfg ? *(u8*)0x02000406 : PersonalData->language);

	setLanguage = (guiLanguage == -1) ? userLanguage : guiLanguage;
	setTitleLanguage = (titleLanguage == -1) ? PersonalData->language : titleLanguage;
	setGameLanguage = (bstrap_language == -1) ? userLanguage : bstrap_language;
}
