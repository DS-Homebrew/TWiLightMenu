#include <nds.h>
#include <stdio.h>
#include <fat.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>

#include "common/dsimenusettings.h"
#include "common/inifile.h"

const char* languageIniPath;

int setLanguage = 0;
int setGameLanguage = 0;

/**
 * Initialize translations.
 * Uses the language ID specified in settings.ui.language.
 */
void langInit(void)
{
	if (ms().guiLanguage == -1) {
		setLanguage = PersonalData->language;
	} else {
		setLanguage = ms().guiLanguage;
	}

	if (ms().bstrap_language == -1) {
		setGameLanguage = PersonalData->language;
	} else {
		setGameLanguage = ms().bstrap_language;
	}
}
