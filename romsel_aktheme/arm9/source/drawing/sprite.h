/*
    sprite.h
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
#ifndef _SPRITE_H_
#define _SPRITE_H_

#include <nds.h>
#include <nds/arm9/sprite.h>

enum SPRITE_SIZE
{
    SS_SIZE_8 = ATTR1_SIZE_8,
    SS_SIZE_16 = ATTR1_SIZE_16,
    SS_SIZE_32 = ATTR1_SIZE_32,
    SS_SIZE_64 = ATTR1_SIZE_64
};

enum SPRITE_SHAPE
{
    SS_SHAPE_SQUARE = ATTR0_SQUARE,
    SS_SHAPE_WIDE = ATTR0_WIDE,
    SS_SHAPE_TALL = ATTR0_TALL
};


class Sprite
{
public:

    Sprite() { init(0); }

    Sprite( u8 id );

    ~Sprite();

    static void sysinit();
    void init( u16 id );
    //void update();
    void setAlpha( u8 alpha );
    void setPosition( u16 x, u8 y );
    void setSize( SPRITE_SIZE size );
    void setShape( SPRITE_SHAPE shape );
    void setBufferOffset( u32 offset );
    void setScale( float scaleX, float scaleY );
    void setRotation( float angle );
    void show();
    void hide();
    bool visible();
    void setPriority( u8 priority );

    u16 * buffer();

    //cSprite & operator=( const Sprite & src );

protected:

    SpriteEntry * _entry;
    SpriteRotation * _affine;
    u8 _id;
    SPRITE_SIZE _size;
    SPRITE_SHAPE _shape;
    u16 _x;
    u8 _y;
    float _scaleX;
    float _scaleY;
    float _rotation;
    u8 _alpha;
    u8 _priority;
    u16 _bufferOffset;
};


//class SpritePool
//{
//public:
//    Sprite * createSprite( SPRITE_SIZE size, SPRITE_SHAPE shape );
//
//    void destroySprite( Sprite * pointer );
//
//};

#endif//_SPRITE_H_
