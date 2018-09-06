/*
    timer.h
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

#pragma once
#ifndef _TIMER_H_
#define _TIMER_H_

#include <nds.h>
#include "common/singleton.h"

class Timer
{
  public:
    Timer();

    ~Timer() {}

  public:
    void initTimer();

    double updateTimer();

    double updateFps();

    double getFps();

    double getTime();

    vu64 getTick();

    double tickToUs(u64 tick);

  private:
    static void timerInterruptHandler();
    double _lastTime;
    double _currentTime;
    static vu64 _overFlow;
    static constexpr double _factor = 1.f / (33.514 * 1000000.f);
    double _fps;
    u32 _fpsCounter;
};

typedef singleton<Timer> timer_s;
inline Timer &timer() { return timer_s::instance(); }

#endif //_TIMER_H_
