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

	//printf("langInit\n");
	setLanguage = (ms().guiLanguage == TWLSettings::ELangDefault) ? userLanguage : ms().guiLanguage;
	setTitleLanguage = (ms().titleLanguage == TWLSettings::ELangDefault) ? PersonalData->language : ms().titleLanguage;
	setGameLanguage = (ms().gameLanguage == TWLSettings::ELangDefault) ? userLanguage : ms().gameLanguage;

	switch (setLanguage) {
		case TWLSettings::ELangJapanese:
			languageIniPath = "nitro:/languages/japanese.ini";
			break;
		case TWLSettings::ELangEnglish:
		default:
			languageIniPath = "nitro:/languages/english.ini";
			break;
		case TWLSettings::ELangFrench:
			languageIniPath = "nitro:/languages/french.ini";
			break;
		case TWLSettings::ELangGerman:
			languageIniPath = "nitro:/languages/german.ini";
			break;
		case TWLSettings::ELangItalian:
			languageIniPath = "nitro:/languages/italian.ini";
			break;
		case TWLSettings::ELangSpanish:
			languageIniPath = "nitro:/languages/spanish.ini";
			break;
		case TWLSettings::ELangChineseS:
			languageIniPath = "nitro:/languages/chinese_s.ini";
			break;
		case TWLSettings::ELangKorean:
			languageIniPath = "nitro:/languages/korean.ini";
			break;
		case TWLSettings::ELangChineseT:
			languageIniPath = "nitro:/languages/chinese_t.ini";
			break;
		case TWLSettings::ELangPolish:
			languageIniPath = "nitro:/languages/polish.ini";
			break;
		case TWLSettings::ELangPortuguese:
			languageIniPath = "nitro:/languages/portuguese.ini";
			break;
		case TWLSettings::ELangRussian:
			languageIniPath = "nitro:/languages/russian.ini";
			break;
		case TWLSettings::ELangSwedish:
			languageIniPath = "nitro:/languages/swedish.ini";
			break;
		case TWLSettings::ELangDanish:
			languageIniPath = "nitro:/languages/danish.ini";
			break;
		case TWLSettings::ELangTurkish:
			languageIniPath = "nitro:/languages/turkish.ini";
			break;
		case TWLSettings::ELangUkrainian:
			languageIniPath = "nitro:/languages/ukrainian.ini";
			break;
	}

	CIniFile languageini(languageIniPath);

#define STRING(what,def) STR_##what = getString(languageini, ""#what, def);
#include "language.inl"
#undef STRING
}
