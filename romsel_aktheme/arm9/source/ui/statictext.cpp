/*
    statictext.cpp
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

#include "ui.h"
#include "statictext.h"
#include "drawing/gdi.h"

namespace akui
{

StaticText::StaticText(s32 x, s32 y, u32 w, u32 h, Window *parent, const std::string &text)
    : Window(parent, text)
{
    _position.x = x;
    _position.y = y;
    _size.x = w;
    _size.y = h;
    _textColor = uiSettings().formTextColor; //(RGB15(31,31,31))
}

StaticText::~StaticText()
{
}

void StaticText::draw()
{
    gdi().setPenColor(_textColor, _engine);
    gdi().textOutRect(_position.x, _position.y, _size.x, _size.y, _text.c_str(), selectedEngine());
}

Window &StaticText::loadAppearance(const std::string &aFileName)
{
    return *this;
}

void StaticText::setTextColor(COLOR color)
{
    _textColor = color;
}

} // namespace akui
