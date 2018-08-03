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

#include "TextEntry.h"
#include "TextPane.h"
#include "FontGraphic.h"

#pragma once

void fontInit();
void updateText(bool top);
void clearText(bool top);
void clearText();
void printSmall(bool top, int x, int y, const u16* message);
void printSmallCentered(bool top, int y, const u16* message);
void printLarge(bool top, int x, int y, const u16* message);
void printLargeCentered(bool top, int y, const u16* message);

void printSmallAscii(bool top, int x, int y, const char* message);
void printSmallCenteredAscii(bool top, int y, const char* message);
void printLargeAscii(bool top, int x, int y, const char* message);
void printLargeCenteredAscii(bool top, int y, const char* message);

int calcSmallFontWidth(const u16* text);
int calcLargeFontWidth(const u16* text);

int calcSmallFontWidthAscii(const char* text);
int calcLargeFontWidthAscii(const char* text);
void animateTextIn(bool top);
void scrollTextVert(bool top, bool up, TextEntry &newEntry);
TextEntry *getPreviousTextEntry(bool top);
TextPane &createTextPane(int startX, int startY, int shownElements);
FontGraphic &getFont(bool large);
void waitForPanesToClear();