/*
    formdesc.h
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

#ifndef _FORMDESC_H_
#define _FORMDESC_H_

#include <string>
#include "renderdesc.h"
#include "drawing/bmp15.h"

namespace akui
{

class FormDesc : public RenderDesc
{
  public:
    FormDesc();

    ~FormDesc();

  public:
    void draw(const Rect &area, GRAPHICS_ENGINE engine) const;

    void loadData(const std::string &topleftBmpFile,
                  const std::string &toprightBmpFile,
                  const std::string &middleBmpFile);

    void setTitleText(const std::string &text);

  protected:
    BMP15 _topleft;
    BMP15 _middle;
    BMP15 _topright;

    COLOR _bodyColor;
    COLOR _frameColor;

    std::string _titleText;
};

} // namespace akui

#endif //_FORMDESC_H_
