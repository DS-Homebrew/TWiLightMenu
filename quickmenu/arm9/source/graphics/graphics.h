/*-----------------------------------------------------------------
 Copyright (C) 2015
	Matthew Scholefield

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

------------------------------------------------------------------*/
#pragma once
#define REFRESH_EVERY_VBLANKS 60
#include "date.h"

extern bool moveIconUp[7];
extern int iconYpos[7];

extern u16* colorTable;

bool screenFadedIn(void);
bool screenFadedOut(void);
void SetBrightness(u8 screen, s8 bright);
void initSubSprites(void);
void bottomBgLoad(void);
//void loadBoxArt(const char* filename, bool secondaryDevice);
void graphicsInit();
void topBgLoad(void);
void topBarLoad(void);
void clockLoad(void);
void clockDraw(void);
void calendarLoad(void);
void calendarDraw(void);
void gbaModeIconLoad(bool screen);
void drawDateTime(bool date, bool showTimeColon = false);
void startRendering(bool top);
bool isRenderingTop();

void batteryIconLoad(void);
void batteryIconDraw(bool blink);

template<typename T> inline const T abs(T const & x)
{
	return (x < 0) ? -x : x;
}

enum class MenuEntry {
	CART,
	PICTOCHAT,
	DOWNLOADPLAY,
	GBA,
	BRIGHTNESS,
	SETTINGS,
	MANUAL,
	INVALID,
};
