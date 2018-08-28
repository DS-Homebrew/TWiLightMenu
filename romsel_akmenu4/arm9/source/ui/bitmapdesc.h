/*
    bitmapdesc.h
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

#ifndef _BMPWINDOWDESC_H_
#define _BMPWINDOWDESC_H_

#include <string>
#include "renderdesc.h"
#include "drawing/bmp15.h"

namespace akui
{

// bitmap desc��ֻ���𻭱���
enum BLTMODE
{
    BM_BITBLT,
    BM_MASKBLT
};

class BitmapDesc : public RenderDesc
{
  public:
    BitmapDesc();

    ~BitmapDesc();

  public:
    void setBltMode(BLTMODE bltmode) { _bltmode = bltmode; }

    void draw(const Rect &area, GRAPHICS_ENGINE engine) const;

    void loadData(const std::string &filename);

    Size size();

  protected:
    BMP15 _background;

    BLTMODE _bltmode;
};

} // namespace akui

#endif //_BMPWINDOWDESC_H_
