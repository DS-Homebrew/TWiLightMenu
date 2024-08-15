#include <nds.h>
#include <stdio.h>
#include <fat.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>

#include "common/inifile.h"

extern bool useTwlCfg;

const char* languageIniPath;

/**
 * Initialize translations.
 * Uses the language ID specified in settings.ui.language.
 */
void langInit(void)
{
	// TODO
}
