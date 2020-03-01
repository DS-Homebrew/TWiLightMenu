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

const char *languageIniPath;

int setLanguage = 0;

std::string ConvertFromUTF8(const std::string& input) {
	std::string res;
	for(size_t i = 0; i < input.size();) {
		auto c = input[i];
		if((c & 0x80) == 0) {
			res += c;
			i++;
		} else if((c & 0xe0) == 0xc0) {
			res += ((((unsigned)c & 0x1f) << 6) | ((unsigned)input[i + 1] & 0x3f));
			i += 2;
		}
		//characters not rendered by the actual font as they are bigger than a char
		else if((c & 0xf0) == 0xe0) {
			i += 3;
		} else if((c & 0xf8) == 0xf0) {
			i += 4;
		}
	}
	return res;
}

/**
 * Initialize translations.
 * Uses the language ID specified in settings.ui.language.
 */
void langInit(void)
{
	switch (ms().getGuiLanguage()) {
		case 0:
			languageIniPath = "nitro:/languages/japanese.ini";
			break;
		case 1:
		case 6:
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

#define STRING(what,def) STR_##what = ConvertFromUTF8(languageini.GetString("LANGUAGE", ""#what, def));
#include "language.inl"
#undef STRING
}
