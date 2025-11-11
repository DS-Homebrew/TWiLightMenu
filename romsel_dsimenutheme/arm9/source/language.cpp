#include <nds.h>
#include <stdio.h>
#include <fat.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <string>

#include "common/twlmenusettings.h"
#include "common/inifile.h"
#include "common/logging.h"

#define STRING(what,def) std::string STR_##what;
#include "language.inl"
#undef STRING

/**
 * Get strings from the ini with special processing
 */
std::string getString(CIniFile &ini, const std::string &item, const std::string &defaultValue) {
	std::string out = ini.GetString("LANGUAGE", item, defaultValue);
	// logPrint("%s: %s\n", item.c_str(), out.c_str());
	if (out == "") {
		// Fallback to default string if translated string is blank
		out = defaultValue;
	}

	// Convert "\n" to actual newlines
	for (uint i = 0; i < out.length() - 1; i++) {
		if (out[i] == '\\') {
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
				case 'd':
				case 'D':
					switch(out[i + 2]) {
						default:
							out = out.substr(0, i) + "" + out.substr(i + 2); // U+E006
							break;
						case 'u':
						case 'U':
							out = out.substr(0, i) + "" + out.substr(i + 3); // U+E079
							break;
						case 'd':
						case 'D':
							out = out.substr(0, i) + "" + out.substr(i + 3); // U+E07A
							break;
						case 'l':
						case 'L':
							out = out.substr(0, i) + "" + out.substr(i + 3); // U+E07B
							break;
						case 'r':
						case 'R':
							out = out.substr(0, i) + "" + out.substr(i + 3); // U+E07C
							break;
						case 'v':
						case 'V':
							out = out.substr(0, i) + "" + out.substr(i + 3); // U+E07D
							break;
						case 'h':
						case 'H':
							out = out.substr(0, i) + "" + out.substr(i + 3); // U+E07E
							break;
					}
				default:
					break;
			}
		} else if (out[i] == '&') {
			if (out.substr(i + 1, 3) == "lrm") {
				out = out.substr(0, i) + "\u200E" + out.substr(i + 4); // Left-to-Right mark
			} else if (out.substr(i + 1, 3) == "rlm") {
				out = out.substr(0, i) + "\u200F" + out.substr(i + 4); // Right-to-Left mark
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
	logPrint("langInit() ");

	char languageIniPath[64];
	snprintf(languageIniPath, sizeof(languageIniPath), "nitro:/languages/%s/language.ini", ms().getGuiLanguageString().c_str());

	CIniFile languageini(languageIniPath);

#define STRING(what,def) STR_##what = getString(languageini, ""#what, def);
#include "language.inl"
#undef STRING

	logPrint("Language inited\n");
}
