/*
    smallclock.h
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

#ifndef _SMALLCLOCK_H_
#define _SMALLCLOCK_H_

#include "ui/window.h"
#include "drawing/bmp15.h"
#include "time/datetime.h"
#include "common/singleton.h"

//class ClockNumber : public
class SmallClock : public akui::Window
{
  public:
    SmallClock();

    ~SmallClock() {}

  public:
    void init();

    void draw();

    void blinkColon();

    akui::Window &loadAppearance(const std::string &aFileName);

  protected:
    void drawNumber(u8 id, u8 number);

    void drawColon();

    BMP15 _numbers;
    BMP15 _colon;

    bool _show;
    bool _colonShow;
    bool _ampmShow;
    akui::Point _ampmPosition;
    COLOR _ampmColor;
};

typedef singleton<SmallClock> smallClock_s;
inline SmallClock &smallClock() { return smallClock_s::instance(); }

#endif //_SMALLCLOCK_H_
