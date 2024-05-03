/*
    progresswnd.h
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

#ifndef _PROGRESSWND_H_
#define _PROGRESSWND_H_

#include "ui.h"
#include "progressbar.h"
#include "common/singleton.h"

namespace akui
{

class ProgressWnd : public Form
{

  public:
    ProgressWnd(); // s32 x, s32 y, u32 w, u32 h, Window * parent, const std::string & text );

    ~ProgressWnd();

  public:
    void init();

    void draw();

    bool process(const Message &msg);

    Window &loadAppearance(const std::string &aFileName);

    void setPercent(u8 percent);

    void setTipText(const std::string &tipText);

  protected:
    void onShow();

    void onHide();

    ProgressBar _bar;

    StaticText _tip;

    BitmapDesc _renderDesc;
};

} // namespace akui

typedef singleton<akui::ProgressWnd> progressWnd_s;
inline akui::ProgressWnd &progressWnd() { return progressWnd_s::instance(); }

#endif //_PROGRESSWND_H_
