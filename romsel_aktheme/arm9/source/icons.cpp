/*
    icons.cpp
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

#include "icons.h"

void Icons::maskBlt(const u16 *aSrc, u16 *aDst)
{
  for (u32 ii = 0; ii < 32; ++ii)
  {
    for (u32 jj = 0; jj < 16; ++jj)
    {
      if (((*(u32 *)aSrc) & 0x80008000) == 0x80008000)
      {
        *(u32 *)aDst = *(u32 *)aSrc;
      }
      else
      {
        if (aSrc[0] & 0x8000)
          aDst[0] = aSrc[0];
        if (aSrc[1] & 0x8000)
          aDst[1] = aSrc[1];
      }
      aSrc += 2;
      aDst += 2;
    }
  }
}
