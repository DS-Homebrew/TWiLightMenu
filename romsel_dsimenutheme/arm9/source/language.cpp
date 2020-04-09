#include <nds.h>
#include <stdio.h>
#include <fat.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <string>

#include "common/dsimenusettings.h"
#include "common/inifile.h"

#define STRING(what,def) std::string STR_##what = ""#what;
#include "language.inl"
#undef STRING

extern bool useTwlCfg;

const char *languageIniPath;

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
	setTitleLanguage = (ms().titleLanguage == -1) ? PersonalData->language : ms().titleLanguage;
	setGameLanguage = (ms().bstrap_language == -1) ? userLanguage : ms().bstrap_language;

	switch (setLanguage) {
		case 0:
			languageIniPath = "nitro:/languages/japanese.ini";
			break;
		case 1:
		case 6:
		case 7:
		default:
			languageIniPath = "nitro:/languages/english.ini";
			break;
		case 2:
			languageIniPath = "nitro:/languages/french.ini";
			break;
		case 3:
			languageIniPath = "nitro:/languages/german.ini";
			break;
		case 4:
			languageIniPath = "nitro:/languages/italian.ini";
			break;
		case 5:
			languageIniPath = "nitro:/languages/spanish.ini";
			break;
	}

	CIniFile languageini(languageIniPath);

#define STRING(what,def) STR_##what = languageini.GetString("LANGUAGE", ""#what, def);
#include "language.inl"
#undef STRING
}
