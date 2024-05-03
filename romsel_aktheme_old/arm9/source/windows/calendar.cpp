/*
    calendar.cpp
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

#include "windows/calendar.h"
#include "systemfilenames.h"
#include "ui/windowmanager.h"
#include "common/inifile.h"
#include "tool/stringtool.h"
#include "tool/memtool.h"

using namespace akui;

Calendar::Calendar() : Window(NULL, "calendar")
{
    _size = Size(0, 0);
    _position = Point(134, 34);
    _engine = GE_SUB;

    _showYear = false;
    _showMonth = false;
    _showDayX = false;
    _showDay = false;
}

void Calendar::init()
{
    loadAppearance(SFN_UI_SETTINGS);
}

Window &Calendar::loadAppearance(const std::string &aFileName)
{
    // load day number
    _dayNumbers = createBMP15FromFile(SFN_DAY_NUMBERS);

    // load year number
    _yearNumbers = createBMP15FromFile(SFN_YEAR_NUMBERS);

    // load weekday text
    _weekdayText = createBMP15FromFile(SFN_WEEKDAY_TEXT);

    CIniFile ini(aFileName);
    _dayPosition.x = ini.GetInt("calendar day", "x", 134);
    _dayPosition.y = ini.GetInt("calendar day", "y", 34);
    _daySize.x = ini.GetInt("calendar day", "dw", 16);
    _daySize.y = ini.GetInt("calendar day", "dh", 14);
    _dayHighlightColor = ini.GetInt("calendar day", "highlightColor", 0xfc00);
    _showDay = ini.GetInt("calendar day", "show", _showDay);

    _dayxPosition.x = ini.GetInt("calendar dayx", "x", 0);
    _dayxPosition.y = ini.GetInt("calendar dayx", "y", 28);
    _showDayX = ini.GetInt("calendar dayx", "show", _showDayX);

    _monthPosition.x = ini.GetInt("calendar month", "x", 12);
    _monthPosition.y = ini.GetInt("calendar month", "y", 28);
    _showMonth = ini.GetInt("calendar month", "show", _showMonth);

    _yearPosition.x = ini.GetInt("calendar year", "x", 52);
    _yearPosition.y = ini.GetInt("calendar year", "y", 28);
    _showYear = ini.GetInt("calendar year", "show", _showYear);

    _weekdayPosition.x = ini.GetInt("calendar weekday", "x", 52);
    _weekdayPosition.y = ini.GetInt("calendar weekday", "y", 28);
    _showWeekday = ini.GetInt("calendar weekday", "show", _showWeekday);

    return *this;
}

#define IS_LEAP(n) ((!(((n) + 1900) % 400) || (!(((n) + 1900) % 4) && (((n) + 1900) % 100))) != 0)
static u8 daysOfMonth()
{
    return (28 | (((IS_LEAP(datetime().year()) ? 62648028 : 62648012) >> (datetime().month() * 2)) & 3));
}

void Calendar::drawDayNumber(u8 day)
{
    if (day > 31)
        return;

    u8 weekDayOfDay = (((day - 1) % 7) + weekDayOfFirstDay()) % 7;
    u8 x = weekDayOfDay * _daySize.x + _dayPosition.x;
    u8 y = ((day - 1 + weekDayOfFirstDay()) / 7 * _daySize.y) + _dayPosition.y;
    u8 pitch = _dayNumbers.pitch() >> 1;
    u8 w = _dayNumbers.width();
    u8 h = _dayNumbers.height() / 10;
    u8 firstNumber = day / 10;
    u8 secondNumber = day % 10;

    if (day == datetime().day())
        gdi().fillRect(_dayHighlightColor, _dayHighlightColor,
                       x - (_daySize.x / 2 - w), y - (_daySize.y - h) / 2, _daySize.x - 1, _daySize.y - 1, selectedEngine());

    if (_dayNumbers.valid())
    {
        gdi().maskBlt(_dayNumbers.buffer() + firstNumber * pitch * h / 2, x, y, w, h, selectedEngine());
        gdi().maskBlt(_dayNumbers.buffer() + secondNumber * pitch * h / 2, x + w, y, w, h, selectedEngine());
    }
}

void Calendar::drawWeekday(const akui::Point &position, u32 value)
{
    if (!_weekdayText.valid())
        return;

    u8 w = _weekdayText.width();
    u8 h = _weekdayText.height() / 7;
    u8 pitch = _weekdayText.pitch() >> 1;
    u8 x = position.x;
    u8 y = position.y;

    gdi().maskBlt(_weekdayText.buffer() + value * pitch * h / 2, x, y, w, h, selectedEngine());
}

u8 Calendar::weekDayOfFirstDay()
{
    return (datetime().weekday() + 7 - ((datetime().day() - 1) % 7)) % 7;
}

void Calendar::drawNumber(const akui::Point &position, u32 index, u32 value)
{
    if (!_yearNumbers.valid())
        return;

    u8 w = _yearNumbers.width();
    u8 h = _yearNumbers.height() / 10;
    u8 pitch = _yearNumbers.pitch() >> 1;
    u8 x = position.x + index * w;
    u8 y = position.y;

    gdi().maskBlt(_yearNumbers.buffer() + value * pitch * h / 2, x, y, w, h, selectedEngine());
}

void Calendar::drawText(const akui::Point &position, u32 value, u32 factor)
{
    u32 ii = 0;
    while (true)
    {
        u8 number = value / factor;
        value %= factor;
        drawNumber(position, ii, number);
        factor /= 10;
        ii++;
        if (0 == factor)
            break;
    }
}

void Calendar::draw()
{
    if (_showDay)
    {
        for (u8 i = 1; i <= daysOfMonth(); ++i)
        {
            drawDayNumber(i);
        }
    }

    if (_showDayX)
    {
        drawText(_dayxPosition, datetime().day(), 10);
    }
    if (_showMonth)
    {
        drawText(_monthPosition, datetime().month(), 10);
    }
    if (_showYear)
    {
        drawText(_yearPosition, datetime().year(), 1000);
    }
    if (_showWeekday)
    {
        drawWeekday(_weekdayPosition, datetime().weekday());
    }
}
