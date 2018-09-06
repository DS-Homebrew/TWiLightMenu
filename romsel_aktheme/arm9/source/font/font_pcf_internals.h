/*
    font_pcf_internals.h
    Copyright (C) 2008-2009 somebody
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
#include <nds.h>
#pragma once
#ifndef __FONT_PCF_INTERNALS_H__
#define __FONT_PCF_INTERNALS_H__


#define PCF_COMPRESSED_METRICS  0x00000100UL

#define PCF_FILE_VERSION ( ( 'p' << 24 ) | ( 'c' << 16 ) | ( 'f' <<  8 ) | 1 )

#define PCF_PROPERTIES        ( 1 << 0 )
#define PCF_ACCELERATORS      ( 1 << 1 )
#define PCF_METRICS           ( 1 << 2 )
#define PCF_BITMAPS           ( 1 << 3 )
#define PCF_INK_METRICS       ( 1 << 4 )
#define PCF_BDF_ENCODINGS     ( 1 << 5 )
#define PCF_SWIDTHS           ( 1 << 6 )
#define PCF_GLYPH_NAMES       ( 1 << 7 )
#define PCF_BDF_ACCELERATORS  ( 1 << 8 )

struct SPcfHeader
{
  u32 iVersion;
  u32 iCount;
};

struct SPcfEntry
{
  u32 iType;
  u32 iFormat;
  u32 iSize;
  u32 iOffset;
};

struct SPcfBitmapsHeader
{
  u32 iFormat;
  u32 iCount;
};

struct PACKED SPcfEncodingsHeader
{
  u32 iFormat;
  u16 iFirstCol;
  u16 iLastCol;
  u16 iFirstRow;
  u16 iLastRow;
  u16 iDefaultChar;
};

struct SPcfCompressedMetric
{
  u8 iLeftSideBearing;
  u8 iRightSideBearing;
  u8 iCharacterWidth;
  u8 iAscent;
  u8 iDescent;
};

struct SPcfAccel
{
  u32 iFormat;
  u8 iNoOverlap;
  u8 iConstantMetrics;
  u8 iTerminalFont;
  u8 iConstantWidth;
  u8 iInkInside;
  u8 iInkMetrics;
  u8 iDrawDirection;
  u8 iUnused;
  u32 iFontAscent;
  u32 iFontDescent;
  u32 iMaxOverlap;
};

#endif
