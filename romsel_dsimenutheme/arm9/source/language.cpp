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
	setLanguage = ms().guiLanguage == -1 ? PersonalData->language : ms().guiLanguage;
	setGameLanguage = ms().bstrap_language == -1 ? PersonalData->language : ms().bstrap_language;
}