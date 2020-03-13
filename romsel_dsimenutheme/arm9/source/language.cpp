#include <nds.h>
#include <stdio.h>
#include <fat.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>

#include "common/dsimenusettings.h"
#include "common/inifile.h"

extern bool useTwlCfg;

const char* languageIniPath;

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

	//printf("langInit\n");
	setLanguage = (ms().guiLanguage == -1) ? userLanguage : ms().guiLanguage;
	setTitleLanguage = (ms().titleLanguage == -1) ? userLanguage : ms().titleLanguage;
	setGameLanguage = (ms().bstrap_language == -1) ? userLanguage : ms().bstrap_language;
}
