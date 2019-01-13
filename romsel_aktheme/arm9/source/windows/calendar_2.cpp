/*
    calendar_2.cpp
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

#include "windows/calendar_2.h"
#include "systemfilenames.h"
#include "ui/windowmanager.h"
#include "common/inifile.h"
#include "tool/stringtool.h"
#include "tool/memtool.h"

using namespace akui;

Calendar_2::Calendar_2() : Window(NULL, "calendar")
{
    _size = Size(0, 0);
    _position = Point(134, 34);
    _engine = GE_SUB;

    _showYear_2 = false;
    _showMonth_2 = false;
    _showDayX_2 = false;
    _showDay_2 = false;
}

void Calendar_2::init()
{
    loadAppearance(SFN_UI_SETTINGS);
}

Window &Calendar_2::loadAppearance(const std::string &aFileName)
{
    // load day number
    _dayNumbers = createBMP15FromFile(SFN_DAY_NUMBERS_2);

    // load year number
    _yearNumbers = createBMP15FromFile(SFN_YEAR_NUMBERS_2);

    // load weekday text
    _weekdayText = createBMP15FromFile(SFN_WEEKDAY_TEXT_2);

    CIniFile ini(aFileName);
    _dayPosition.x = ini.GetInt("calendar day 2", "x", 134);
    _dayPosition.y = ini.GetInt("calendar day 2", "y", 34);
    _daySize.x = ini.GetInt("calendar day 2", "dw", 16);
    _daySize.y = ini.GetInt("calendar day 2", "dh", 14);
    _dayHighlightColor = ini.GetInt("calendar day 2", "highlightColor", 0xfc00);
    _showDay_2 = ini.GetInt("calendar day 2", "show", _showDay_2);

    _dayxPosition.x = ini.GetInt("calendar dayx 2", "x", 0);
    _dayxPosition.y = ini.GetInt("calendar dayx 2", "y", 28);
    _showDayX_2 = ini.GetInt("calendar dayx 2", "show", _showDayX_2);

    _monthPosition.x = ini.GetInt("calendar month 2", "x", 12);
    _monthPosition.y = ini.GetInt("calendar month 2", "y", 28);
    _showMonth_2 = ini.GetInt("calendar month 2", "show", _showMonth_2);

    _yearPosition.x = ini.GetInt("calendar year 2", "x", 52);
    _yearPosition.y = ini.GetInt("calendar year 2", "y", 28);
    _showYear_2 = ini.GetInt("calendar year 2", "show", _showYear_2);

    _weekdayPosition.x = ini.GetInt("calendar weekday 2", "x", 52);
    _weekdayPosition.y = ini.GetInt("calendar weekday 2", "y", 28);
    _showWeekday_2 = ini.GetInt("calendar weekday 2", "show", _showWeekday_2);

    return *this;
}

#define IS_LEAP(n) ((!(((n) + 1900) % 400) || (!(((n) + 1900) % 4) && (((n) + 1900) % 100))) != 0)
static u8 daysOfMonth()
{
    return (28 | (((IS_LEAP(datetime().year()) ? 62648028 : 62648012) >> (datetime().month() * 2)) & 3));
}

void Calendar_2::drawDayNumber(u8 day)
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

void Calendar_2::drawWeekday(const akui::Point &position, u32 value)
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


u8 Calendar_2::weekDayOfFirstDay()
{
    return (datetime().weekday() + 7 - ((datetime().day() - 1) % 7)) % 7;
}

void Calendar_2::drawNumber(const akui::Point &position, u32 index, u32 value)
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

void Calendar_2::drawText(const akui::Point &position, u32 value, u32 factor)
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

void Calendar_2::draw()
{
    if (_showDay_2)
    {
        for (u8 i = 1; i <= daysOfMonth(); ++i)
        {
            drawDayNumber(i);
        }
    }

    if (_showDayX_2)
    {
        drawText(_dayxPosition, datetime().day(), 10);
    }
    if (_showMonth_2)
    {
        drawText(_monthPosition, datetime().month(), 10);
    }
    if (_showYear_2)
    {
        drawText(_yearPosition, datetime().year(), 1000);
    }
    if (_showWeekday_2)
    {
        drawWeekday(_weekdayPosition, datetime().weekday());
    }
}
