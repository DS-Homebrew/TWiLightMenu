/*
    fontfactory.h
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


#pragma once
#ifndef _FONT_FACTORY_H_
#define _FONT_FACTORY_H_

#include <nds.h>
#include "../singleton.h"
#include "akfont.h"



class cFontFactory
{
public:

    cFontFactory();

    ~cFontFactory();

    void makeFont(void);

    cFont & font() { return *_font; }

protected:

    cFont * _font;
};

typedef t_singleton< cFontFactory > fontFactory_s;
inline cFontFactory & fontFactory() { return fontFactory_s::instance(); }
inline cFont & font() { return fontFactory_s::instance().font(); }

#endif