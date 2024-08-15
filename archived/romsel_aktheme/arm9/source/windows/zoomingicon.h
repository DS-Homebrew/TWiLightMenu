/*
    zoomingicon.h
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

#ifndef _ZOOMINGICON_H_
#define _ZOOMINGICON_H_

#include <nds.h>
#include "drawing/sprite.h"
#include "ui/animation.h"

class ZoomingIcon : public Animation
{
public:

    ZoomingIcon();

    ~ZoomingIcon();

public:

    void update();

    void show();

    void hide();

    void reset();

    void setScale( float scale );

    void setPosition( u8 x, u8 y );

    void setBufferChanged();

    u16 * buffer() { return _buffer; }

public:

    Sprite _sprite;

    u16 _buffer[32*32];
    u16 _x;
    u8 _y;
    float _scale;
    bool _needUpdateBuffer;

};



#endif//_ZOOMINGICON_H_
