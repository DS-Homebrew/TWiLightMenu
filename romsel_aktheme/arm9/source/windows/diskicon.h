/*
    diskicon.h
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

#ifndef _DISKICON_H_
#define _DISKICON_H_

#include <nds.h>
#include "ui/window.h"
#include "drawing/sprite.h"
#include "common/singleton.h"

class DiskIcon : public akui::Window
{

  public:
    DiskIcon();

    ~DiskIcon() {}

  public:
    void draw();

    void turnOn();

    void turnOff();

    akui::Window &loadAppearance(const std::string &aFileName);

    void blink(void);

  protected:
    bool _draw;

    float _lightTime;

    Sprite _icon;
};

typedef singleton<DiskIcon> diskIcon_s;
inline DiskIcon &diskIcon() { return diskIcon_s::instance(); }

#endif //_DISKIOICON_H_
