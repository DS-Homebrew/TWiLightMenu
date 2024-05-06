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
  static std::string getIdentifier(TWLSettings::TLanguage language, bool useTwlCfg)
  {
    if (language == TWLSettings::ELangDefault)
      language = (TWLSettings::TLanguage)(useTwlCfg ? *(u8*)0x02000406 : PersonalData->language);
    switch (language)
    {
    case TWLSettings::ELangJapanese:
      return "lang_ja";
    case TWLSettings::ELangEnglish:
      return "lang_en";
    case TWLSettings::ELangFrench:
      return "lang_fr";
    case TWLSettings::ELangGerman:
      return "lang_de";
    case TWLSettings::ELangItalian:
      return "lang_it";
    case TWLSettings::ELangSpanish:
      return "lang_es";
    case TWLSettings::ELangChineseS:
      return "lang_zh-CN";
    case TWLSettings::ELangKorean:
      return "lang_ko";
    case TWLSettings::ELangChineseT:
      return "lang_zh-TW";
    case TWLSettings::ELangPolish:
      return "lang_pl";
    case TWLSettings::ELangPortuguese:
      return "lang_pt";
    case TWLSettings::ELangRussian:
      return "lang_ru";
    case TWLSettings::ELangSwedish:
      return "lang_sv";
    case TWLSettings::ELangDanish:
      return "lang_da";
    case TWLSettings::ELangTurkish:
      return "lang_tr";
    case TWLSettings::ELangUkrainian:
      return "lang_uk";
    case TWLSettings::ELangHungarian:
      return "lang_hu";
    case TWLSettings::ELangNorwegian:
      return "lang_no";
    case TWLSettings::ELangHebrew:
      return "lang_he";
    case TWLSettings::ELangDutch:
      return "lang_nl";
    case TWLSettings::ELangIndonesian:
      return "lang_id";
    case TWLSettings::ELangGreek:
      return "lang_el";
    case TWLSettings::ELangBulgarian:
      return "lang_bg";
    case TWLSettings::ELangRomanian:
      return "lang_ro";
    case TWLSettings::ELangArabic:
      return "lang_ar";
    case TWLSettings::ELangPortugueseBrazil:
      return "lang_pt-BR";
    case TWLSettings::ELangVietnamese:
      return "lang_vi";
    case TWLSettings::ELangValencian:
      return "lang_val";
    case TWLSettings::ELangCatalan:
      return "lang_ca";
    case TWLSettings::ELangRyukyuan:
      return "lang_ry";
    case TWLSettings::ELangCzech:
      return "lang_cs";
    case TWLSettings::ELangFinnish:
      return "lang_fi";
    default:
      return "lang_en";
    }
  }

public:
  LanguageFile(TWLSettings::TLanguage language)
  {
	extern bool useTwlCfg;
    m_bReadOnly = true;
    LoadIniFile(formatString(SFN_LANGUAGE_TEXT, getIdentifier(language, useTwlCfg).c_str()));
  }
  ~LanguageFile(){};
};

typedef singleton<LanguageFile, TWLSettings::TLanguage> languageFile_s;
inline LanguageFile &lang() { return languageFile_s::instance(ms().getGuiLanguage()); }
#define LANG(i, t) lang().GetString(i, t, t)

bool stringComp(const std::string &item1, const std::string &item2);
void langInit(void);

#endif //_AK_LANGUAGE_H_
