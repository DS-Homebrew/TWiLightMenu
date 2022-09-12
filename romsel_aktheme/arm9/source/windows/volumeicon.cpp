/*
    volumeicon.cpp
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

#include "volumeicon.h"
#include "drawing/bmp15.h"
#include "common/inifile.h"
#include "systemfilenames.h"
#include "tool/memtool.h"
#include "tool/timetool.h"
#include "common/systemdetails.h"

using namespace akui;

VolumeIcon::VolumeIcon() : Window(NULL, "volumeicon")
{
    CIniFile ini(SFN_UI_SETTINGS);
    _size = Size(0, 0);
    _position = Point(0, 0);
    if (ini.GetInt("volume icon", "screen", true)) {
        _engine = GE_SUB;
    } else {
        _engine = GE_MAIN;
    }

    _volIcon0 =  createBMP15FromFile(SFN_VOLUME0);
    _volIcon1 =  createBMP15FromFile(SFN_VOLUME1);
    _volIcon2 =  createBMP15FromFile(SFN_VOLUME2);
    _volIcon3 =  createBMP15FromFile(SFN_VOLUME3);
    _volIcon4 =  createBMP15FromFile(SFN_VOLUME4);

    _icon.init(1);
    _icon.setPosition(226, 174);
    _icon.setPriority(3);
    _icon.setBufferOffset(16);
    if (ini.GetInt("volume icon", "show", false))
    {
        _icon.show();
    }

    fillMemory(_icon.buffer(), 32 * 32 * 2, 0x00000000);
}

void VolumeIcon::draw()
{
    CIniFile ini(SFN_UI_SETTINGS);
    if (ini.GetInt("volume icon", "show", false))
    {
        u8 volumeLevel = sys().volumeStatus();
        
        if (volumeLevel >= 0x1C && volumeLevel < 0x20) {
            drawIcon(_volIcon4);
        } else if (volumeLevel >= 0x11 && volumeLevel < 0x1C) {
            drawIcon(_volIcon3);
        } else if (volumeLevel >= 0x07 && volumeLevel < 0x11) {
            drawIcon(_volIcon2);
        } else if (volumeLevel > 0x00 && volumeLevel < 0x07) {
            drawIcon(_volIcon1);
        } else {
            drawIcon(_volIcon0);
        }
    }
}

void VolumeIcon::drawIcon(const BMP15 &icon)
{

    CIniFile ini(SFN_UI_SETTINGS);

    u16 x = ini.GetInt("volume icon", "x", 238);
    u16 y = ini.GetInt("volume icon", "y", 172);
    _icon.setPosition(x, y);

    if (ini.GetInt("volume icon", "screen", true)) {
    gdi().maskBlt(icon.buffer(), x, y, icon.width(), icon.height(), _engine);
    } else {
        u32 pitch = icon.pitch() >> 1;
	    for (u8 i = 0; i < icon.height(); ++i)
	    {
	        for (u8 j = 0; j < icon.width(); ++j)
	        {
	            ((u16 *)_icon.buffer())[i * 32 + j] = ((u16 *)icon.buffer())[i * pitch + j];
	        }
	    }
    }

    dbg_printf("cVolumeIcon::drawIcon ok %d\n", icon.valid());
}
