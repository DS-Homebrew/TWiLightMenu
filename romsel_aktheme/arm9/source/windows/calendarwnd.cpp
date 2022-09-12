/*
    calendarwnd.cpp
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

#include "calendarwnd.h"
#include "drawing/gdi.h"
#include "drawing/bmp15.h"
#include "systemfilenames.h"

using namespace akui;

CalendarWnd::CalendarWnd() : Form( 0, 0, 256, 192, NULL, "calendar window" )
{

}

CalendarWnd::~CalendarWnd()
{

}

void CalendarWnd::init()
{
    setEngine( GE_SUB );
    loadAppearance( SFN_UPPER_SCREEN_BG );
}

Window& CalendarWnd::loadAppearance(const std::string& aFileName )
{
    _background = createBMP15FromFile( aFileName );
    return *this;
}


void CalendarWnd::draw()
{
    if ( _background.valid() )
        gdi().bitBlt( _background.buffer(), 0, 0, 256, 192, selectedEngine() );
}
