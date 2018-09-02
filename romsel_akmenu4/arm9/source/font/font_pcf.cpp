/*
    font_pcf.cpp
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

#include "font_pcf.h"
#include "font_pcf_internals.h"
#include "language.h"
#include <fcntl.h>
#include <unistd.h>

FontPcf::FontPcf() : Font(), iGlyphs(NULL), iData(NULL), iCount(0), iDataSize(0), iHeight(0), iAscent(0), iDescent(0)
{
  //FIXME: test on nds
  //printf("%d\n",sizeof(FontPcf::SGlyph));
}

FontPcf::~FontPcf()
{
  delete[] iGlyphs;
  delete[] iData;
}

void FontPcf::Info(const char *aString, u32 *aWidth, u32 *aSymbolCount)
{
  u32 len;
  u32 code = utf8toucs2((const u8 *)aString, &len);
  if (aSymbolCount)
    *aSymbolCount = len;
  if (aWidth)
  {
    s32 index = Search(code);
    *aWidth = (index >= 0) ? iGlyphs[index].iWidth : ((code > 0 && code < 8) ? 10 : 0);
  }
}

bool FontPcf::ParseAccels(FILE *aFont, u32 aSize, u32 aOffset)
{
  bool res = false;
  if (fseek(aFont, aOffset, SEEK_SET) != 0)
    return false;
  SPcfAccel header;
  if (fread(&header, sizeof(header), 1, aFont) != 1)
    return false;
  res = true;
  iAscent = header.iFontAscent;
  iDescent = header.iFontDescent;
  iHeight = iAscent + iDescent;
  return res;
}

bool FontPcf::ParseBitmaps(FILE* aFont, u32 aSize, u32 aOffset)
{
  bool res = false;
  if (fseek(aFont, aOffset, SEEK_SET) != 0)
    return false;
  SPcfBitmapsHeader header;
  if (fread(&header, sizeof(header), 1, aFont) != 1)
    return false;
  iGlyphs = new (std::nothrow) SGlyph[header.iCount];
  if (!iGlyphs)
    return false;
  iCount = header.iCount;
  iDataSize = aSize - sizeof(SPcfBitmapsHeader) - header.iCount * sizeof(u32) - 16;
  iData = new (std::nothrow) u8[iDataSize];
  if (iData)
  {
    u32 *offsets = new (std::nothrow) u32[header.iCount];
    if (offsets)
    {
      do
      {
        if (fread(offsets, sizeof(u32) * header.iCount, 1, aFont) != 1)
          break;
        for (u32 ii = 0; ii < header.iCount; ii++)
        {
          iGlyphs[ii].iOffset = offsets[ii];
        }
        if (fseek(aFont, 16, SEEK_CUR) != 0)
          break;
        if (fread(iData, iDataSize, 1, aFont) != 1)
          break;
        res = true;
      } while (false);
      delete[] offsets;
    }
  }
  return res;
}

bool FontPcf::ParseMetrics(FILE *aFont, u32 aSize, u32 aOffset)
{
  bool res = false;
  if (fseek(aFont, aOffset, SEEK_SET) != 0)
    return false;
  u8 *buffer = new (std::nothrow) u8[aSize];
  if (buffer)
  {
    if (fread(buffer, aSize, 1, aFont) == 1)
    {
      u32 format = *(u32 *)buffer;
      if (format & PCF_COMPRESSED_METRICS)
      {
        u32 count = *(u16 *)(buffer + 4);
        if (count == iCount)
        {
          SPcfCompressedMetric *metrics = (SPcfCompressedMetric *)(buffer + 6);
          for (u32 ii = 0; ii < iCount; ii++)
          {
            iGlyphs[ii].iWidth = metrics[ii].iCharacterWidth - 0x80;
            iGlyphs[ii].iLeft = metrics[ii].iLeftSideBearing - 0x80;
            iGlyphs[ii].iRight = metrics[ii].iRightSideBearing - 0x80;
            iGlyphs[ii].iAscent = metrics[ii].iAscent - 0x80;
            iGlyphs[ii].iDescent = metrics[ii].iDescent - 0x80;
            //printf("0x%08x: w=%d,l=%d,r=%d,a=%d,d=%d,offset=0x%08x\n",iGlyphs[ii].iCode,iGlyphs[ii].iWidth,iGlyphs[ii].iLeft,iGlyphs[ii].iRight,iGlyphs[ii].iAscent,iGlyphs[ii].iDescent,iGlyphs[ii].iOffset);
          }
          res = true;
        }
      }
    }
    delete[] buffer;
  }
  return res;
}

bool FontPcf::ParseEncodings(FILE *aFont, u32 aSize, u32 aOffset)
{
  bool res = false;
  if (fseek(aFont, aOffset, SEEK_SET) != 0)
    return false;
  u8 *buffer = new (std::nothrow) u8[aSize];
  if (buffer)
  {
    if (fread(buffer, aSize, 1, aFont) == 1)
    {
      SPcfEncodingsHeader &header = *(SPcfEncodingsHeader *)(buffer);
      u32 nencoding = (header.iLastCol - header.iFirstCol + 1) * (header.iLastRow - header.iFirstRow + 1);
      u16 *codes = (u16 *)(buffer + sizeof(SPcfEncodingsHeader));
      for (u32 ii = 0; ii < nencoding; ii++)
      {
        if (codes[ii] != 0xffff)
        {
          iGlyphs[codes[ii]].iCode = (((ii / (header.iLastCol - header.iFirstCol + 1)) + header.iFirstRow) * 256) + ((ii % (header.iLastCol - header.iFirstCol + 1)) + header.iFirstCol);
        }
      }
      res = true;
    }
    delete[] buffer;
  }
  return res;
}

int FontPcf::Compare(const void *a, const void *b)
{
  return (static_cast<const SGlyph *>(a)->iCode - static_cast<const SGlyph *>(b)->iCode);
}

bool FontPcf::Load(const char *aFileName)
{
  bool res = false;
  FILE *font = fopen(aFileName, "rb");
  if (font)
  {
    do
    {
      SPcfHeader header;
      if (fread(&header, sizeof(header), 1, font) != 1)
        break;
      if (header.iVersion != PCF_FILE_VERSION)
        break;
      SPcfEntry entries[header.iCount];
      if (fread(entries, sizeof(SPcfEntry) * header.iCount, 1, font) != 1)
        break;
      s32 accelsIndex = -1, bitmapsIndex = -1, metricsIndex = -1, encodingsIndex = -1;
      for (u32 ii = 0; ii < header.iCount; ii++)
      {
        if (entries[ii].iType == PCF_ACCELERATORS)
          accelsIndex = ii;
        else if (entries[ii].iType == PCF_BITMAPS)
          bitmapsIndex = ii;
        else if (entries[ii].iType == PCF_METRICS)
          metricsIndex = ii;
        else if (entries[ii].iType == PCF_BDF_ENCODINGS)
          encodingsIndex = ii;
      }
      if (accelsIndex >= 0 && bitmapsIndex >= 0 && metricsIndex >= 0 && encodingsIndex >= 0)
      {
        if (!ParseAccels(font, entries[accelsIndex].iSize, entries[accelsIndex].iOffset))
          break;
        if (!ParseBitmaps(font, entries[bitmapsIndex].iSize, entries[bitmapsIndex].iOffset))
          break;
        if (!ParseMetrics(font, entries[metricsIndex].iSize, entries[metricsIndex].iOffset))
          break;
        if (!ParseEncodings(font, entries[encodingsIndex].iSize, entries[encodingsIndex].iOffset))
          break;
        if (lang().GetInt("font", "sort", 0))
          qsort(iGlyphs, iCount, sizeof(SGlyph), Compare);
        res = true;
      }
    } while (false);
    fclose(font);
  }
  return res;
}

s32 FontPcf::Search(u16 aCode)
{
  s32 result = SearchInternal(aCode);
  if (result < 0 && aCode > ' ')
    result = SearchInternal('?');
  return result;
}

s32 FontPcf::SearchInternal(u16 aCode)
{
  s32 low = 0, high = iCount - 1, curr;
  while (true)
  {
    curr = (low + high) / 2;
    //curr=((aCode-iGlyphs[low].iCode)*(high-low)/(iGlyphs[high].iCode-iGlyphs[low].iCode))+low;
    if (aCode < iGlyphs[curr].iCode)
    {
      high = curr - 1;
    }
    else if (aCode > iGlyphs[curr].iCode)
    {
      low = curr + 1;
    }
    else
    {
      return curr;
    }
    if (low > high)
      return -1;
  }
}

void FontPcf::DrawInternal(u16 *mem, s16 x, s16 y, const u8 *data, u16 color, u32 width, u32 height)
{
  u32 byteW = width;
  byteW = byteW / 8 + (byteW & 7 ? 1 : 0);
  for (u32 ii = 0; ii < height; ii++)
  {
    u32 cur_width = width;
    for (u32 jj = 0; jj < byteW; jj++)
    {
      u32 curr = *data++;
      u32 top = (cur_width > 8) ? 8 : cur_width;
      for (u32 kk = 0; kk < top; kk++)
      {
        if (curr & (1 << kk))
        {
          if (y + ii >= 0 && x + jj >= 0)
          {
            *(mem + x + (jj << 3) + kk + ((y + ii) << 8)) = color;
          }
        }
      }
      cur_width -= 8;
    }
  }
}

void FontPcf::Draw(u16 *mem, s16 x, s16 y, const u8 *aText, u16 color)
{
  if (!(iData && iGlyphs))
    return;
  u32 code = utf8toucs2(aText, NULL);
  s32 index = Search(code);
  if (index >= 0)
  {
    x += iGlyphs[index].iLeft;
    y += -iGlyphs[index].iAscent + iAscent;
    DrawInternal(mem, x, y, iData + iGlyphs[index].iOffset, color, -iGlyphs[index].iLeft + iGlyphs[index].iRight, iGlyphs[index].iAscent + iGlyphs[index].iDescent);
  }
  else if (code > 0 && code < 8)
  {
    y += -9 + iAscent;
    u8 special[] =
        {
            0x7c,
            0,
            0xfe,
            0,
            0xef,
            1,
            0xd7,
            1,
            0xd7,
            1,
            0x83,
            1,
            0xbb,
            1,
            0xfe,
            0,
            0x7c,
            0,
            0x7c,
            0,
            0xfe,
            0,
            0xc3,
            1,
            0xbb,
            1,
            0xc3,
            1,
            0xbb,
            1,
            0xc3,
            1,
            0xfe,
            0,
            0x7c,
            0,
            0x7c,
            0,
            0xfe,
            0,
            0xbb,
            1,
            0xd7,
            1,
            0xef,
            1,
            0xd7,
            1,
            0xbb,
            1,
            0xfe,
            0,
            0x7c,
            0,
            0x7c,
            0,
            0xfe,
            0,
            0xbb,
            1,
            0xd7,
            1,
            0xef,
            1,
            0xef,
            1,
            0xef,
            1,
            0xfe,
            0,
            0x7c,
            0,
            0xf8,
            1,
            0xfc,
            1,
            0xf6,
            1,
            0xf7,
            1,
            0xf7,
            1,
            0xf7,
            1,
            0x87,
            1,
            0xff,
            1,
            0xff,
            1,
            0x3f,
            0,
            0x7f,
            0,
            0xc3,
            0,
            0xbb,
            1,
            0xc3,
            1,
            0xdb,
            1,
            0xbb,
            1,
            0xff,
            1,
            0xff,
            1,
            0x38,
            0,
            0x38,
            0,
            0x38,
            0,
            0xff,
            1,
            0xef,
            1,
            0xff,
            1,
            0x38,
            0,
            0x38,
            0,
            0x38,
            0,
        };
    DrawInternal(mem, x, y, special + (code - 1) * 18, color, 9, 9);
  }
}

#define ONE_BYTE_MASK 0x80
#define ONE_BYTE_SIGN 0x00
#define TWO_BYTE_MASK 0xc0e0
#define TWO_BYTE_SIGN 0x80c0
#define THREE_BYTE_MASK 0xc0c0f0
#define THREE_BYTE_SIGN 0x8080e0
#define FIRST_BYTE 0xff
#define SECOND_BYTE 0xff00
#define THIRD_BYTE 0xff0000

u32 FontPcf::utf8toucs2(const u8 *aSource, u32 *aLength)
{
  u32 data = aSource[0], len = 1, res = '?';
  if ((data & ONE_BYTE_MASK) == ONE_BYTE_SIGN)
  {
    res = data;
  }
  else
  {
    data += aSource[1] * 0x100;
    if ((data & TWO_BYTE_MASK) == TWO_BYTE_SIGN)
    {
      res = data & ~TWO_BYTE_MASK;
      res = ((res & SECOND_BYTE) >> 8) | ((res & FIRST_BYTE) << 6);
      len = 2;
    }
    else
    {
      data += aSource[2] * 0x10000;
      if ((data & THREE_BYTE_MASK) == THREE_BYTE_SIGN)
      {
        res = data & ~THREE_BYTE_MASK;
        res = ((res & FIRST_BYTE) << 12) | ((res & SECOND_BYTE) >> 2) | ((res & THIRD_BYTE) >> 16);
        len = 3;
      }
    }
  }
  if (aLength)
    *aLength = len;
  return res;
}

u32 FontPcf::FontRAM(void)
{
  return iDataSize + sizeof(SGlyph) * iCount;
}
