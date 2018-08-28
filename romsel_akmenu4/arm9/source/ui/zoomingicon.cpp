/*
    zoomingicon.cpp
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

#include "zoomingicon.h"
#include "drawing/gdi.h"

ZoomingIcon::ZoomingIcon()
{
    _x = 0;
    _y = 0;
    _scale = 1.f;
    _needUpdateBuffer = false;

    _sprite.init(0);
    _sprite.setPriority(1);
    _sprite.setAlpha(15);
    _sprite.show();
}

ZoomingIcon::~ZoomingIcon()
{
}

void ZoomingIcon::setScale(float scale)
{
    _scale = scale;
}

void ZoomingIcon::setPosition(u8 x, u8 y)
{
    _x = x;
    _y = y;
}

void ZoomingIcon::setBufferChanged()
{
    _needUpdateBuffer = true;
}

void ZoomingIcon::update()
{
    static float scaleFactor = 0.015;
    if (_visible)
    {
        _scale += scaleFactor;
        if (_scale > 1.2 || _scale < 0.9)
            scaleFactor *= -1;
        _sprite.setScale(_scale, _scale);
        if (!_sprite.visible())
            _sprite.show();
    }
    else
    {
        _scale = 1.0;
        scaleFactor = 0.015;
        _sprite.setScale(1.f, 1.f);
        if (_sprite.visible())
            _sprite.hide();
    }

    _sprite.setScale(_scale, _scale);
    _sprite.setPosition(_x, _y);

    if (_needUpdateBuffer)
    {
        DC_FlushRange(_buffer, 32 * 32 * 2);
        dmaCopy(_buffer, _sprite.buffer(), 32 * 32 * 2);
        DC_InvalidateRange(_sprite.buffer(), 32 * 32 * 2);
        _needUpdateBuffer = false;
    }
}

void ZoomingIcon::show()
{
    if (!_visible)
    {
        _visible = true;
        gdi().maskBlt(_buffer, _x, _y, 32, 32, GE_MAIN); // sprite only available on main engine
    }
}

void ZoomingIcon::hide()
{
    if (_visible)
    {
        _visible = false;
        gdi().maskBlt(_buffer, _x, _y, 32, 32, GE_MAIN); // sprite only available on main engine
    }
}

void ZoomingIcon::reset()
{
    _scale = 1.f;
}
