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

#ifndef _CALENDAR_H_
#define _CALENDAR_H_

#include "ui/window.h"
#include "bmp15.h"
#include "datetime.h"
#include "singleton.h"
#include "ui/point.h"


class cCalendar : public akui::cWindow
{
public:

    cCalendar();

    ~cCalendar() {}

public:

    void init();

    void draw();

    akui::cWindow & loadAppearance(const std::string& aFileName );

protected:

    u8 weekDayOfFirstDay();

    void drawDayNumber( u8 day );
    void drawText( const akui::cPoint& position, u32 value, u32 factor );
    void drawNumber( const akui::cPoint& position, u32 index, u32 value );

    akui::cPoint _dayPosition;
    akui::cSize _daySize;
    akui::cPoint _dayxPosition;
    akui::cPoint _monthPosition;
    akui::cPoint _yearPosition;
    COLOR _dayHighlightColor;
    cBMP15 _dayNumbers;       // index 10 means colon
    cBMP15 _yearNumbers;

    bool _showYear;
    bool _showMonth;
    bool _showDayX;
    bool _showDay;

    bool _colonShow;
};


typedef t_singleton< cCalendar > calendar_s;
inline cCalendar & calendar() { return calendar_s::instance(); }


#endif//_CALENDAR_H_
