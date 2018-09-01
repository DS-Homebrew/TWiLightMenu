/*
    bigclock.cpp
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

#include "bigclock.h"
#include "tool/stringtool.h"
#include "systemfilenames.h"
#include "ui/windowmanager.h"
#include "common/inifile.h"
#include "common/dsimenusettings.h"

using namespace akui;

BigClock::BigClock() : Window(NULL, "big clock")
{
    _size = Size(0, 0);
    _position = Point(8, 80);
    _engine = GE_SUB;
    _show = false;
    _colonShow = false;
    _ampmShow = false;
    _ampmColor = RGB15(17, 12, 0);
}

void BigClock::init()
{
    loadAppearance(SFN_UI_SETTINGS);
}

Window &BigClock::loadAppearance(const std::string &aFileName)
{
    CIniFile ini(aFileName);
    _position.x = ini.GetInt("big clock", "x", 8);
    _position.y = ini.GetInt("big clock", "y", 80);
    _show = ini.GetInt("big clock", "show", _show);

    _ampmPosition.x = ini.GetInt("am pm", "x", 8);
    _ampmPosition.y = ini.GetInt("am pm", "y", 80);
    _ampmShow = ini.GetInt("am pm", "show", _ampmShow);
    _ampmColor = ini.GetInt("am pm", "color", _ampmColor);

    _numbers = createBMP15FromFile(SFN_CLOCK_NUMBERS);
    _colon = createBMP15FromFile(SFN_CLOCK_COLON);

    return *this;
}

void BigClock::drawNumber(u8 id, u8 number)
{
    if (number > 10)
        return;

    u8 x = _position.x + id * (_numbers.width() + 2);
    if (id > 2)
    { // minute number
        x -= 8;
    }
    if (_numbers.valid())
    {
        u8 w = _numbers.width();
        u8 h = _numbers.height() / 10;
        u8 pitch = _numbers.pitch() >> 1;
        gdi().maskBlt(_numbers.buffer() + number * pitch * h / 2, x, _position.y, w, h, selectedEngine());
    }
}

void BigClock::drawColon()
{
    u8 x = _position.x + 2 * _numbers.width();
    if (_colon.valid())
    {
        gdi().maskBlt(_colon.buffer(),
                      x, _position.y, _colon.width(), _colon.height(), selectedEngine());
    }
}

void BigClock::draw()
{
    if (!_show)
        return;
    u8 hours = datetime().hours();
    u8 minutes = datetime().minutes();
    const char *ampm = (hours < 12) ? "AM" : "PM";
    if (ms().show12hrClock)
    {
        if (hours > 12)
            hours -= 12;
        if (hours == 0)
            hours = 12;
    }

    u8 number1 = hours / 10;
    u8 number2 = hours % 10;
    u8 number3 = minutes / 10;
    u8 number4 = minutes % 10;
    //u8 number5 = datetime().seconds() / 10;
    //u8 number6 = datetime().seconds() % 10;

    drawNumber(0, number1);
    drawNumber(1, number2);
    drawNumber(3, number3);
    drawNumber(4, number4);
    if (_colonShow)
        drawColon();
    if (ms().show12hrClock && _ampmShow)
    {
        gdi().setPenColor(_ampmColor, _engine);
        gdi().textOut(_ampmPosition.x, _ampmPosition.y, ampm, _engine);
    }
}

void BigClock::blinkColon()
{
    _colonShow = !_colonShow;
}
