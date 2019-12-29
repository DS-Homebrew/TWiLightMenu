#include <nds.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

extern int guiLanguage;
extern int bstrap_language;
int setLanguage = 0;
int setGameLanguage = 0;

/**
 * Initialize translations.
 * Uses the language ID specified in settings.ui.language.
 */
void langInit(void)
{
	setLanguage = guiLanguage == -1 ? PersonalData->language : guiLanguage;
	setGameLanguage = bstrap_language == -1 ? PersonalData->language : bstrap_language;
}