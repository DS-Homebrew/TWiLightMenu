/*
    font_pcf.h
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

/*
  very base pcf font implementation
  supported only fonts with ...
*/


#pragma once
#ifndef __FONT_PCF_H__
#define __FONT_PCF_H__

#include "font.h"
#include <cstdio>

//#define SMALL 1

class FontPcf: public Font
{
  private:
#ifdef SMALL
    struct PACKED SGlyph
    {
      unsigned iOffset:19;
      unsigned iWidth:5;
      signed iLeft:6;
      signed iRight:6;
      signed iAscent:6;
      signed iDescent:6;
      unsigned iCode:16;
    };
#else
    struct SGlyph
    {
      u32 iOffset;
      u16 iCode;
      u8 iWidth;
      u8 iReserved;
      s8 iLeft;
      s8 iRight;
      s8 iAscent;
      s8 iDescent;
    };
#endif
  private:
    SGlyph* iGlyphs;
    u8* iData;
    u32 iCount;
    u32 iDataSize;
    u32 iHeight;
    u32 iAscent;
    u32 iDescent;
  private:
    bool ParseAccels(FILE* aFont,u32 aSize,u32 aOffset);
    bool ParseBitmaps(FILE* aFont,u32 aSize,u32 aOffset);
    bool ParseMetrics(FILE* aFont,u32 aSize,u32 aOffset);
    bool ParseEncodings(FILE* aFont,u32 aSize,u32 aOffset);
    s32 SearchInternal(u16 aCode);
    s32 Search(u16 aCode);
    static u32 utf8toucs2(const u8* aSource,u32* aLength);
    void DrawInternal(u16* mem,s16 x,s16 y,const u8* data,u16 color,u32 width,u32 height);
    static int Compare(const void* a,const void* b);
  public:
    FontPcf();
    ~FontPcf();
    bool Load(const char* aFileName);
    void Draw(u16* mem,s16 x,s16 y,const u8* aText,u16 color);
    void Info(const char* aString,u32* aWidth,u32* aSymbolCount);
    u32 FontRAM(void);
};

#endif
