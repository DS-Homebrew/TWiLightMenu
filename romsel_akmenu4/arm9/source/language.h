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

#include "singleton.h"
#include "systemfilenames.h"
#include "inifile.h"

class LanguageFile : public CIniFile
{
public:
  LanguageFile()
  {
    m_bReadOnly = true;
    LoadIniFile(SFN_LANGUAGE_TEXT);
  }
  ~LanguageFile(){};
};

typedef t_singleton<LanguageFile> languageFile_s;
inline LanguageFile &lang() { return languageFile_s::instance(); }
#define LANG(i, t) lang().GetString(i, t, t)

bool stringComp(const std::string &item1, const std::string &item2);

#endif//_AK_LANGUAGE_H_
