/*
    statictext.h
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

#ifndef _STATICTEXT_H_
#define _STATICTEXT_H_

#include "window.h"
#include "drawing/bmp15.h"

namespace akui
{

class StaticText : public Window
{
  public:
    StaticText(s32 x, s32 y, u32 w, u32 h, Window *parent, const std::string &text);

    ~StaticText();

  public:
    void draw();

    Window &loadAppearance(const std::string &aFileName);

    void setTextColor(COLOR color);

  protected:
    COLOR _textColor;
};

} // namespace akui

#endif //_STATICTEXT_H_
