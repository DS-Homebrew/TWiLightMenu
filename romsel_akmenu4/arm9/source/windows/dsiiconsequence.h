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

#ifndef _DSI_SEQUENCE_H_
#define _DSI_SEQUENCE_H_

#include <nds.h>
#include "drawing/sprite.h"

// Bitmasks from 
// http://problemkaputt.de/gbatek.htm#dscartridgeicontitle
#define SEQ_FLIPV(i) ((i & 0b1000000000000000) >> 15)
#define SEQ_FLIPH(i) ((i & 0b0100000000000000) >> 14)
#define SEQ_PAL(i) ((i & 0b0011100000000000) >> 11)
#define SEQ_BMP(i) ((i & 0b0000011100000000) >> 8)
#define SEQ_DUR(i) ((i & 0b0000000011111111) >> 0)

class DSiIconSequence : public Animation
{
  public:
    DSiIconSequence();

    ~DSiIconSequence();

  public:
    void update() override;

    void reset();

    void setSequenceChanged();

    u16 *sequence() { return _sequence; }

    u8 *gameTid() { return _gameTid; }

  public:
    u8 _gameTid[4];
    u16 _sequence[64];
    bool _flipH;
    bool _flipV;
    size_t _currentSequenceIndex;
    size_t _updatesUntilNewIndex;
    size_t _bitmapIndex;
    size_t _paletteIndex;
    bool _needUpdateSequence;
};

#endif //_ZOOMINGICON_H_
