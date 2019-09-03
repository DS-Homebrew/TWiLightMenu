/*
    batteryicon.h
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

#ifndef _BATTERYICON_H_
#define _BATTERYICON_H_

#include <nds.h>
#include "ui/window.h"
#include "drawing/sprite.h"
#include "common/singleton.h"

class BatteryIcon : public akui::Window
{

  public:
    BatteryIcon();

    ~BatteryIcon() {}

    u32 waitForBattery = 11;

  public:
    void draw();

    void drawIcon(BMP15 &icon);

    akui::Window &loadAppearance(const std::string &aFileName) { return *this; }

  protected:
    BMP15 _batteryCharge;
    BMP15 _battery1;
    BMP15 _battery2;
    BMP15 _battery3;
    BMP15 _battery4;

    bool _draw;

    float _lightTime;

    Sprite _icon;

};

typedef singleton<BatteryIcon> batteryIcon_s;
inline BatteryIcon &batteryIcon() { return batteryIcon_s::instance(); }

#endif // _BATTERYICON_H_
