/*
    font.cpp
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

#include "font.h"

#include <nds.h>
#include <string>
#include <string.h>
#include <stdio.h>

#include "drawing/gdi.h"
#include "systemfilenames.h"


Font::Font()
{
}

Font::~Font()
{
}

u32 Font::getStringScreenWidth(const char *str, size_t len)
{
    if (NULL == str || 0 == len)
        return 0;

    size_t strLen = strlen(str);
    if (len > strLen)
        len = strLen;

    const char *endstr = str + len;
    u32 width = 0;
    const char *p = str;
    while (*p != 0 && p < endstr)
    {
        u32 ww, add;
        Info(p, &ww, &add);
        width += ww;
        p += add;
    }
    return width;
}

std::string Font::breakLine(const std::string &text, u32 maxLineWidth)
{
    if (0 == maxLineWidth)
        return text;

    std::string ret;



    const char *p = text.c_str();
    bool hasSpace = false;
    u32 tempWidth = 0;

    while (*p != 0)
    {
        u32 ww, add;
        Info(p, &ww, &add);

        if (' ' == *p)
            hasSpace = true;
        tempWidth += ww;
        if (tempWidth > maxLineWidth)
        {
            if (hasSpace)
            {
                u32 lastSpacePos = ret.find_last_of(' ');
                ret[lastSpacePos] = '\n';
                tempWidth = getStringScreenWidth(
                    text.c_str() + lastSpacePos, (size_t)(p - text.c_str()) - lastSpacePos);
                hasSpace = false;
            }
            else
            {
                ret.push_back('\n');
                tempWidth = 0;
            }
        }
        for (u32 ii = 0; ii < add; ii++)
            ret.push_back(*p++);
    }
    if (ret[ret.length() - 1] != '\n')
        ret.push_back('\n');

    return ret;
}

u32 Font::FontRAM(void)
{
    return 0;
}
