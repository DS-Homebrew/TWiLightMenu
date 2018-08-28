/*
    bitmapdesc.cpp
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

#include "bitmapdesc.h"
#include "tool/memtool.h"
#include "tool/dbgtool.h"

namespace akui
{

///////////////////////////////// desc ////////////////
BitmapDesc::BitmapDesc()
{
    _bltmode = BM_BITBLT;
}

BitmapDesc::~BitmapDesc()
{
}

void BitmapDesc::draw(const Rect &area, GRAPHICS_ENGINE engine) const
{
    if (_background.valid())
    {
        if (BM_BITBLT == _bltmode)
            gdi().bitBlt(_background.buffer(),
                         area.position().x, area.position().y,
                         _background.width(), _background.height(), engine);
        else
            gdi().maskBlt(_background.buffer(),
                          area.position().x, area.position().y,
                          _background.width(), _background.height(), engine);
    }
}

void BitmapDesc::loadData(const std::string &filename)
{
    if (!_background.valid())
    {
        _background = createBMP15FromFile(filename);
    }
}

Size BitmapDesc::size()
{
    if (_background.valid())
        return Size(_background.width(), _background.height());
    else
        return Size(0, 0);
}

} // namespace akui
