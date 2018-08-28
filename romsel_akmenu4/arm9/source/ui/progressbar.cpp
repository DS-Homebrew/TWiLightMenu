/*
    progressbar.cpp
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

#include "progressbar.h"

namespace akui
{

ProgressBar::ProgressBar(s32 x, s32 y, u32 w, u32 h, Window *parent, const std::string &text)
    : Window(parent, text)
{
    setSize(Size(w, h));
    setPosition(Point(x, y));
}

ProgressBar::~ProgressBar()
{
}

void ProgressBar::draw()
{
    //draw frame

    //draw left

    //draw right

    //draw bar body
    u8 width = _percent * size().x / 100;
    if (_barBmp.valid())
    {
        gdi().maskBlt(_barBmp.buffer(),
                      _barBmp.pitch() >> 1, _barBmp.height(),
                      _position.x, _position.y, width, _barBmp.height(), _engine);
    }
    else
    {
        u16 color1 = 0xfc00;
        u16 color2 = 0x800f;
        for (u8 i = 0; i < size().y; ++i)
        {
            if (i & 1)
                gdi().fillRect(
                    color1, color2, position().x, position().y + i, width, 1, _engine);
            else
                gdi().fillRect(
                    color2, color1, position().x, position().y + i, width, 1, _engine);
        }
    }
}

Window &ProgressBar::loadAppearance(const std::string &aFileName)
{
    _barBmp = createBMP15FromFile(aFileName);
    setSize(Size(_barBmp.width(), _barBmp.height()));
    return *this;
}

void ProgressBar::setPercent(u8 percent)
{
    _percent = percent;
}

} // namespace akui
