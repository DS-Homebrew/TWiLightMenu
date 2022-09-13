/*---------------------------------------------------------------------------------

	Touch screen control for the ARM7

	Copyright (C) 2005 - 2010
		Michael Noland (joat)
		Jason Rogers (dovoto)
		Dave Murphy (WinterMute)

	This software is provided 'as-is', without any express or implied
	warranty.  In no event will the authors be held liable for any
	damages arising from the use of this software.

	Permission is granted to anyone to use this software for any
	purpose, including commercial applications, and to alter it and
	redistribute it freely, subject to the following restrictions:

	1.	The origin of this software must not be misrepresented; you
			must not claim that you wrote the original software. If you use
			this software in a product, an acknowledgment in the product
			documentation would be appreciated but is not required.
	2.	Altered source versions must be plainly marked as such, and
			must not be misrepresented as being the original software.
	3.	This notice may not be removed or altered from any source
			distribution.

---------------------------------------------------------------------------------*/

#include <nds/ndstypes.h>
#include <nds/system.h>
#include "my_codec.h"
#include <nds/arm7/touch.h>
#include <nds/arm7/input.h>
#include <nds/interrupts.h>

#include <stdlib.h>

static s32 xscale, yscale;
static s32 xoffset, yoffset;

//---------------------------------------------------------------------------------
void my_touchInit() {
//---------------------------------------------------------------------------------

	if (my_cdcIsAvailable()) {
		xscale = ((PersonalData->calX2px - PersonalData->calX1px) << 19) / ((PersonalData->calX2) - (PersonalData->calX1));
		yscale = ((PersonalData->calY2px - PersonalData->calY1px) << 19) / ((PersonalData->calY2) - (PersonalData->calY1));

		xoffset = ((PersonalData->calX1 + PersonalData->calX2) * xscale  - ((PersonalData->calX1px + PersonalData->calX2px) << 19) ) / 2;
		yoffset = ((PersonalData->calY1 + PersonalData->calY2) * yscale  - ((PersonalData->calY1px + PersonalData->calY2px) << 19) ) / 2;

		int oldIME = enterCriticalSection();
		my_cdcTouchInit();
		leaveCriticalSection(oldIME);
	} else {
		touchInit();
	}
}

//---------------------------------------------------------------------------------
bool my_touchPenDown() {
//---------------------------------------------------------------------------------

	bool down;
	int oldIME = enterCriticalSection();
	if (my_cdcIsAvailable()) {
		down = my_cdcTouchPenDown();
	} else {
		down = !(REG_KEYXY & (1<<6));
	}
	leaveCriticalSection(oldIME);
	return down;
}

//---------------------------------------------------------------------------------
// reading pixel position:
//---------------------------------------------------------------------------------
void my_touchReadXY(touchPosition *touchPos) {
//---------------------------------------------------------------------------------

	if (my_cdcIsAvailable()) {
		int oldIME = enterCriticalSection();
		my_cdcTouchRead(touchPos);
		leaveCriticalSection(oldIME);
	} else {
		touchReadXY(touchPos);
		return;
	}

	s16 px = ( touchPos->rawx * xscale - xoffset + xscale/2 ) >>19;
	s16 py = ( touchPos->rawy * yscale - yoffset + yscale/2 ) >>19;

	if (px < 0) px = 0;
	if (py < 0) py = 0;
	if (px > (SCREEN_WIDTH -1)) px = SCREEN_WIDTH -1;
	if (py > (SCREEN_HEIGHT -1)) py = SCREEN_HEIGHT -1;

	touchPos->px = px;
	touchPos->py = py;
}
