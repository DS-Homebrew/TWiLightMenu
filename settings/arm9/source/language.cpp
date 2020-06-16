#include <nds.h>
#include <stdio.h>
#include <fat.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <string>

#include "common/inifile.h"
#include "common/dsimenusettings.h"

#define STRING(what,def) std::string STR_##what = ""#what;
#include "language.inl"
#undef STRING

extern bool useTwlCfg;

const char *languageIniPath;

/**
 * Get strings from the ini with special processing
 */
std::string getString(CIniFile &ini, const std::string &item, const std::string &defaultValue) {
	std::string out = ini.GetString("LANGUAGE", item, defaultValue);

	// Convert "\n" to actual newlines
	for(uint i = 0; i < out.length() - 1; i++) {
		if(out[i] == '\\') {
			switch(out[i + 1]) {
				case 'n':
				case 'N':
					out = out.substr(0, i) + '\n' + out.substr(i + 2);
					break;
				case 'a':
				case 'A':
					out = out.substr(0, i) + "" + out.substr(i + 2); // U+E000
					break;
				case 'b':
				case 'B':
					out = out.substr(0, i) + "" + out.substr(i + 2); // U+E001
					break;
				case 'x':
				case 'X':
					out = out.substr(0, i) + "" + out.substr(i + 2); // U+E002
					break;
				case 'y':
				case 'Y':
					out = out.substr(0, i) + "" + out.substr(i + 2); // U+E003
					break;
				case 'l':
				case 'L':
					out = out.substr(0, i) + "" + out.substr(i + 2); // U+E004
					break;
				case 'r':
				case 'R':
					out = out.substr(0, i) + "" + out.substr(i + 2); // U+E005
					break;
				default:
					break;
			}
		}
	}

	return out;
}

/**
 * Initialize translations.
 * Uses the language ID specified in settings.ui.language.
 */
void langInit(void)
{
	int userLanguage = (useTwlCfg ? *(u8*)0x02000406 : PersonalData->language);
	int setLanguage = (ms().getGuiLanguage() == -1) ? userLanguage : ms().getGuiLanguage();

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

#define STRING(what,def) STR_##what = getString(languageini, ""#what, def);
#include "language.inl"
#undef STRING
}
