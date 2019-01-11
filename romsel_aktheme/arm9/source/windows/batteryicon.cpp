/*
    batteryicon.cpp
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

#include "batteryicon.h"
#include "drawing/bmp15.h"
#include "common/inifile.h"
#include "systemfilenames.h"
#include "tool/memtool.h"
#include "tool/timetool.h"

using namespace akui;

BatteryIcon::BatteryIcon() : Window(NULL, "batteryicon")
{
    CIniFile ini(SFN_UI_SETTINGS);
    _size = Size(0, 0);
    _position = Point(0, 0);
    if(ini.GetInt("battery icon", "screen", true)) {
        _engine = GE_SUB;
    } else {
        _engine = GE_MAIN;
    }
    _icon.init(1);
    _icon.setPosition(226, 174);
    _icon.setPriority(3);
    _icon.setBufferOffset(16);
    if(ini.GetInt("battery icon", "show", false))
    {
        _icon.show();
    }

    fillMemory(_icon.buffer(), 32 * 32 * 2, 0x00000000);
}

void BatteryIcon::draw()
{
    CIniFile ini(SFN_UI_SETTINGS);
    if(ini.GetInt("battery icon", "show", false)) {
        u8 batteryLevel = *(u8*)(0x027FF001);

        if (batteryLevel & BIT(7)) {
            loadAppearance(SFN_BATTERY_CHARGE);
        } else if (batteryLevel & BIT(3)) {
            loadAppearance(SFN_BATTERY4);
        } else if (batteryLevel & BIT(2)) {
            loadAppearance(SFN_BATTERY3);
        } else if (batteryLevel & BIT(1)) {
            loadAppearance(SFN_BATTERY2);
        } else if (batteryLevel & BIT(0)) {
            loadAppearance(SFN_BATTERY1);
        } else {
            loadAppearance(SFN_BATTERY_CHARGE);
        }
    }
}

Window &BatteryIcon::loadAppearance(const std::string &aFileName)
{

    CIniFile ini(SFN_UI_SETTINGS);

    u16 x = ini.GetInt("battery icon", "x", 238);
    u16 y = ini.GetInt("battery icon", "y", 172);
    _icon.setPosition(x, y);

    BMP15 icon = createBMP15FromFile(aFileName);

    if(ini.GetInt("battery icon", "screen", true)) {
        gdi().maskBlt(icon.buffer(), x, y, icon.width(), icon.height(), _engine);
    }

    dbg_printf("cBatteryIcon::loadAppearance ok %d\n", icon.valid());
    return *this;
}
