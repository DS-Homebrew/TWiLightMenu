/*
    language.cpp
    Copyright (C) 2007 Acekard, www.acekard.com
    Copyright (C) 2007-2009 somebody
    Copyright (C) 2009 yellow wood goblin

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "language.h"
#include <nds.h>
#include <stdio.h>
#include <fat.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>

#include "common/dsimenusettings.h"
#include "common/inifile.h"

const char* languageIniPath;

int setTitleLanguage = 0;

void langInit(void)
{
	printf("langInit\n");
	if (ms().titleLanguage == -1) {
		setTitleLanguage = PersonalData->language;
	} else {
		setTitleLanguage = ms().titleLanguage;
	}
}

bool stringComp(const std::string &item1, const std::string &item2)
{
  return strcasecmp(item1.c_str(), item2.c_str()) < 0;
}
