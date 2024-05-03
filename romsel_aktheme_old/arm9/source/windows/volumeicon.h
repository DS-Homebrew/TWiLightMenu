/*
    volumeicon.h
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

#ifndef _VOLUMEICON_H_
#define _VOLUMEICON_H_

#include <nds.h>
#include "ui/window.h"
#include "drawing/sprite.h"
#include "common/singleton.h"

class VolumeIcon : public akui::Window
{

  public:
    VolumeIcon();

    ~VolumeIcon() {}

  public:
    void draw();

    void drawTop();

    void drawBottom();

    void drawIcon(const BMP15 &icon);

    akui::Window& loadAppearance(const std::string &filename) { return *this; };

  protected:
    bool _draw;

    float _lightTime;

    Sprite _icon;

    BMP15 _volIcon0;
    BMP15 _volIcon1;
    BMP15 _volIcon2;
    BMP15 _volIcon3;
    BMP15 _volIcon4;
};

typedef singleton<VolumeIcon> volumeIcon_s;
inline VolumeIcon &volumeIcon() { return volumeIcon_s::instance(); }

#endif //_VOLUMEICON_H_
