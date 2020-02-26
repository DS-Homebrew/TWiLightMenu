/*
    language.h
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

#pragma once
#ifndef _AK_LANGUAGE_H_
#define _AK_LANGUAGE_H_

#include "common/singleton.h"
#include "systemfilenames.h"
#include "common/inifile.h"
#include "common/dsimenusettings.h"
#include <nds.h>
#include "tool/stringtool.h"

extern int setTitleLanguage;

class LanguageFile : public CIniFile
{
private:
  static std::string getIdentifier(DSiMenuPlusPlusSettings::TLanguage language)
  {
    if (language == DSiMenuPlusPlusSettings::ELangDefault)
      language = (DSiMenuPlusPlusSettings::TLanguage)PersonalData->language;
    switch (language)
    {
    case DSiMenuPlusPlusSettings::ELangJapanese:
      return "lang_jp";
    case DSiMenuPlusPlusSettings::ELangEnglish:
      return "lang_en";
    case DSiMenuPlusPlusSettings::ELangFrench:
      return "lang_fr";
    case DSiMenuPlusPlusSettings::ELangSpanish:
      return "lang_es";
    case DSiMenuPlusPlusSettings::ELangGerman:
      return "lang_de";
    case DSiMenuPlusPlusSettings::ELangItalian:
      return "lang_it";
    case DSiMenuPlusPlusSettings::ELangChinese:
      return "lang_cn";
    default:
      return "lang_en";
    }
  }

public:
  LanguageFile(DSiMenuPlusPlusSettings::TLanguage language)
  {
    m_bReadOnly = true;
    LoadIniFile(formatString(SFN_LANGUAGE_TEXT, getIdentifier(language).c_str()));
  }
  ~LanguageFile(){};
};

typedef singleton<LanguageFile, DSiMenuPlusPlusSettings::TLanguage> languageFile_s;
inline LanguageFile &lang() { return languageFile_s::instance(ms().getGuiLanguage()); }
#define LANG(i, t) lang().GetString(i, t, t)

bool stringComp(const std::string &item1, const std::string &item2);
void langInit(void);

#endif //_AK_LANGUAGE_H_
