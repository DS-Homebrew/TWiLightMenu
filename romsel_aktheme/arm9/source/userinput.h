/*
    userinput.h
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

#ifndef __USERINPUT_H__
#define __USERINPUT_H__

#include <nds.h>

typedef struct T_INPUT
{
    struct touchPositionRelative
    {
      s16 px;
      s16 py;
    };
    u32 keysHeld;
    u32 keysUp;
    u32 keysDown;
    u32 keysDownRepeat;
    touchPosition touchPt;
    touchPositionRelative movedPt;
    bool touchDown;
    bool touchUp;
    bool touchHeld;
    bool touchMoved;
    bool operator==( const T_INPUT & src ) {
        return keysHeld == src.keysHeld
            && keysUp == src.keysUp
            && keysDown == src.keysDown
            && keysDownRepeat == src.keysDownRepeat
            && touchPt.px == src.touchPt.px
            && touchPt.py == src.touchPt.py
            && movedPt.px == src.movedPt.px
            && movedPt.py == src.movedPt.py
            && touchDown == src.touchDown
            && touchUp == src.touchUp
            && touchHeld == src.touchHeld
            && touchMoved == src.touchMoved;
    }
} INPUT;

void initInput();
INPUT & updateInput();
INPUT & getInput();
u32 getInputIdleMs();
void resetInputIdle(void);
bool processInput( INPUT & inputs  );

#endif//_INPUT_H_
