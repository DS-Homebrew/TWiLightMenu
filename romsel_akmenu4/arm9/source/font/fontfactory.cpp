/*
    fontfactory.cpp
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

#include "fontfactory.h"
#include "font_pcf.h"
#include "systemfilenames.h"
#include "language.h"


cFontFactory::cFontFactory() : _font(NULL)
{
}

cFontFactory::~cFontFactory()
{
    if (NULL != _font)
        delete _font;
}

void cFontFactory::makeFont(void)
{
    std::string filename(SFN_FONTS_DIRECTORY + lang().GetString("font", "main", SFN_DEFAULT_FONT));
    _font = new cFontPcf();
    _font->Load(filename.c_str());
}
