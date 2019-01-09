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
    _size = Size(0, 0);
    _position = Point(0, 0);
    _engine = GE_MAIN;
    _icon.init(2);
    _icon.setPosition(226, 174);
    _icon.setPriority(3);
    _icon.setBufferOffset(32);
    _icon.show();

    fillMemory(_icon.buffer(), 32 * 32 * 2, 0x00000000);
}

void BatteryIcon::draw()
{
    u32 batteryLevel = getBatteryLevel();

    if (batteryLevel & 1<<7) {
         batteryIcon().loadAppearance(SFN_BATTERY_CHARGE);
     } else if (batteryLevel & 1<<3) {
         batteryIcon().loadAppearance(SFN_BATTERY4);
     } else if (batteryLevel & 1<<2) {
         batteryIcon().loadAppearance(SFN_BATTERY3);
     } else if (batteryLevel & 1<<1) {
         batteryIcon().loadAppearance(SFN_BATTERY2);
     } else if (batteryLevel & 1<<0) {
         batteryIcon().loadAppearance(SFN_BATTERY1);
     } else {
         batteryIcon().loadAppearance(SFN_BATTERY1);
     }
}

Window &BatteryIcon::loadAppearance(const std::string &aFileName)
{

    CIniFile ini(SFN_UI_SETTINGS);

    u16 x = ini.GetInt("battery icon", "x", 238);
    u16 y = ini.GetInt("battery icon", "y", 172);

    _icon.setPosition(x, y);

    BMP15 icon = createBMP15FromFile(aFileName);

    u32 pitch = icon.pitch() >> 1;
    for (u8 i = 0; i < icon.height(); ++i)
    {
        for (u8 j = 0; j < icon.width(); ++j)
        {
            ((u16 *)_icon.buffer())[i * 32 + j] = ((u16 *)icon.buffer())[i * pitch + j];
        }
    }
    dbg_printf("cBatteryIcon::loadAppearance ok %d\n", icon.valid());
    return *this;
}
