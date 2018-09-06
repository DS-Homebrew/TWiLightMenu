/*
    progresswnd.cpp
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

#include "progresswnd.h"

namespace akui
{

ProgressWnd::ProgressWnd() // s32 x, s32 y, u32 w, u32 h, Window * parent, const std::string & text )
    : Form(0, 0, 0, 0, NULL, ""),
      _bar(0, 0, 180, 24, this, ""),
      _tip(0, 0, 180, 20, this, "")
{
    _renderDesc.setBltMode(BM_MASKBLT);
    setSize(Size(226, 62));
    setPosition(Point(14, 64));
}

ProgressWnd::~ProgressWnd()
{
}

void ProgressWnd::init()
{
    loadAppearance(SFN_PROGRESS_WND_BG);
    addChildWindow(&_bar);
    _bar.setRelativePosition(Point(4, 9));
    _bar.setPercent(0);

    addChildWindow(&_tip);
    _tip.setTextColor(uiSettings().formTextColor);
    _tip.setRelativePosition(Point(4, _size.y - 24));
    _tip.setSize(Size(_size.x - 8, 12));
    arrangeChildren();
    hide();
}

void ProgressWnd::draw()
{
    _renderDesc.draw(windowRectangle(), _engine);
    Form::draw();
    swiWaitForVBlank();
}

bool ProgressWnd::process(const Message &msg)
{
    bool ret = false;
    return ret;
}

Window &ProgressWnd::loadAppearance(const std::string &aFileName)
{
    _renderDesc.loadData(aFileName);
    _bar.loadAppearance(SFN_PROGRESS_BAR_BG);

    return *this;
}

void ProgressWnd::setPercent(u8 percent)
{
    _bar.setPercent(percent);
    windowManager().update();
    gdi().present(GE_MAIN);
}

void ProgressWnd::setTipText(const std::string &tipText)
{
    _tip.setText(tipText);
}

void ProgressWnd::onShow()
{
    _bar.setPercent(0);
    windowManager().addWindow(this);
}

void ProgressWnd::onHide()
{
    _bar.setPercent(0);
    windowManager().removeWindow(this);
}

} // namespace akui
