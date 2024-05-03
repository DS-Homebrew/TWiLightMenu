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
#include "common/systemdetails.h"

using namespace akui;

BatteryIcon::BatteryIcon() : Window(NULL, "batteryicon")
{
    CIniFile ini(SFN_UI_SETTINGS);
    _size = Size(0, 0);
    _position = Point(0, 0);
    if (ini.GetInt("battery icon", "screen", true)) {
        _engine = GE_SUB;
    } else {
        _engine = GE_MAIN;
    }
    
    _batteryCharge = createBMP15FromFile(SFN_BATTERY_CHARGE);
    _battery1 = createBMP15FromFile(SFN_BATTERY1);
    _battery2 = createBMP15FromFile(SFN_BATTERY2);
    _battery3 = createBMP15FromFile(SFN_BATTERY3);
    _battery4 = createBMP15FromFile(SFN_BATTERY4);

    _icon.init(1);
    _icon.setPosition(226, 174);
    _icon.setPriority(3);
    _icon.setBufferOffset(16);
    if (ini.GetInt("battery icon", "show", false))
    {
        _icon.show();
    }

    fillMemory(_icon.buffer(), 32 * 32 * 2, 0x00000000);
}

void BatteryIcon::draw()
{
    CIniFile ini(SFN_UI_SETTINGS);
    if (ini.GetInt("battery icon", "show", false)) {
        u8 batteryLevel = sys().batteryStatus();

		if (isDSiMode()) {
			if (batteryLevel & BIT(7)) {
				drawIcon(_batteryCharge);
			} else if (batteryLevel == 0xF) {
				drawIcon(_battery4);
			} else if (batteryLevel == 0xB) {
				drawIcon(_battery3);
			} else if (batteryLevel == 0x7) {
				drawIcon(_battery2);
			} else if (batteryLevel == 0x3 || batteryLevel == 0x1) {
				drawIcon(_battery1);
			} else {
				drawIcon(_batteryCharge);
			}
		} else {
			if (batteryLevel & BIT(0)) {
				drawIcon(_battery1);
			} else {
				drawIcon(_battery4);
			}
		}
    }
}

void BatteryIcon::drawIcon(BMP15 &icon)
{

    CIniFile ini(SFN_UI_SETTINGS);

    u16 x = ini.GetInt("battery icon", "x", 238);
    u16 y = ini.GetInt("battery icon", "y", 172);
    _icon.setPosition(x, y);

    if (ini.GetInt("battery icon", "screen", true)) {
        gdi().maskBlt(icon.buffer(), x, y, icon.width(), icon.height(), _engine);
    }

    dbg_printf("cBatteryIcon::drawIcon ok %d\n", icon.valid());
}
