/*
    calendar.h
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

#ifndef _CALENDAR_2_H_
#define _CALENDAR_2_H_

#include "ui/window.h"
#include "drawing/bmp15.h"
#include "time/datetime.h"
#include "common/singleton.h"
#include "ui/point.h"


class Calendar_2 : public akui::Window
{
public:

    Calendar_2();

    ~Calendar_2() {}

public:

    void init();

    void draw();

    akui::Window & loadAppearance(const std::string& aFileName );

protected:

    u8 weekDayOfFirstDay();

    void drawDayNumber( u8 day );
    void drawText( const akui::Point& position, u32 value, u32 factor );
    void drawNumber( const akui::Point& position, u32 index, u32 value );
    void drawWeekday( const akui::Point& position, u32 value );

    akui::Point _dayPosition;
    akui::Size _daySize;
    akui::Point _dayxPosition;
    akui::Point _monthPosition;
    akui::Point _yearPosition;
    akui::Point _weekdayPosition;
    COLOR _dayHighlightColor;
    BMP15 _dayNumbers;       // index 10 means colon
    BMP15 _yearNumbers;
    BMP15 _weekdayText;

    bool _showYear_2;
    bool _showMonth_2;
    bool _showDayX_2;
    bool _showDay_2;
    bool _showWeekday_2;

    bool _colonShow_2;
};


typedef singleton< Calendar_2 > calendar_2_s;
inline Calendar_2 & calendar_2() { return calendar_2_s::instance(); }


#endif//_CALENDAR_2_H_
