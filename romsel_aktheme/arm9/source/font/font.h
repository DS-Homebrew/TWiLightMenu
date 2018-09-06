/*
    font.h
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
#ifndef __FONT_H__
#define __FONT_H__

#include <nds.h>
#include <string>

class Font
{
public:
  Font();
  virtual ~Font();

public:
  u32 getStringScreenWidth(const char *str, size_t len);
  std::string breakLine(const std::string &text, u32 maxLineWidth);

public:
  virtual void Info(const char *aString, u32 *aWidth, u32 *aSymbolCount) = 0;
  virtual void Draw(u16 *mem, s16 x, s16 y, const u8 *aText, u16 color) = 0;
  virtual bool Load(const char *aFileName) = 0;
  virtual u32 FontRAM(void);
};

#endif //_FONT_H_
