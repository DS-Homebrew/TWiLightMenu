/*
    formdesc.cpp
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

#include "ui.h"
#include "formdesc.h"


namespace akui
{

// �߿���ɫ��b5c71f
//    23, 25, 4
// ���ڱ���ɫ��eeebae
// 30, 29, 22

FormDesc::FormDesc()
{
    _bodyColor = uiSettings().formBodyColor;   //RGB15(30,29,22);
    _frameColor = uiSettings().formFrameColor; //RGB15(23,25,4);
}

FormDesc::~FormDesc()
{
}

void FormDesc::draw(const Rect &area, GRAPHICS_ENGINE engine) const
{
    if (_topleft.valid())
    {
        gdi().maskBlt(
            _topleft.buffer(), area.position().x, area.position().y, _topleft.width(), _topleft.height(), engine);
    }

    if (_middle.valid())
    {
        for (u32 i = 0; i < _middle.height(); ++i)
        {
            COLOR lineColor = _middle.buffer()[i] & 0xFFFF;
            gdi().setPenColor(lineColor, engine);
            gdi().fillRect(lineColor, lineColor, area.position().x + _topleft.width(),
                           area.position().y + i,
                           area.size().x - _topleft.width() - _topright.width(),
                           1, engine);
        }
    }

    if (_topright.valid())
    {
        gdi().maskBlt(_topright.buffer(),
                      area.position().x + area.size().x - _topright.width(), area.position().y,
                      _topright.width(), _topright.height(), engine);
    }

    if (_titleText != "")
    {
        gdi().setPenColor(uiSettings().formTitleTextColor, engine);
        gdi().textOut(area.position().x + 8,
                      area.position().y + ((_topleft.height() - SYSTEM_FONT_HEIGHT) >> 1) + 1,
                      _titleText.c_str(), engine);
    }

    gdi().setPenColor(_bodyColor, engine);
    gdi().fillRect(_bodyColor, _bodyColor, area.topLeft().x, area.topLeft().y + _topleft.height(),
                   area.width(), area.height() - _topleft.height(), engine);

    gdi().setPenColor(_frameColor, engine);
    gdi().frameRect(area.topLeft().x, area.topLeft().y + _topleft.height(),
                    area.width(), area.height() - _topleft.height(), uiSettings().thickness, engine);
}

void FormDesc::loadData(const std::string &topleftBmpFile,
                         const std::string &toprightBmpFile,
                         const std::string &middleBmpFile)
{
    _topleft = createBMP15FromFile(topleftBmpFile);
    _topright = createBMP15FromFile(toprightBmpFile);
    _middle = createBMP15FromFile(middleBmpFile);
}

void FormDesc::setTitleText(const std::string &text)
{
    _titleText = text;
}

} // namespace akui
