/*
    form.h
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

#ifndef _AKUI_FORM_H_
#define _AKUI_FORM_H_

#include <list>
#include "rectangle.h"
#include "window.h"
#include "bitmapdesc.h"

namespace akui
{

class cForm : public cWindow
{

  public:
    cForm(s32 x, s32 y, u32 w, u32 h, cWindow *parent, const std::string &text);

    ~cForm();

  public:
    u32 doModal();

    cForm &addChildWindow(cWindow *aWindow);

    cForm &removeChildWindow(cWindow *aWindow);

    cForm &arrangeChildren();

    void draw();

    //cWindow& loadAppearance(const std::string& aFileName );

    bool process(const cMessage &msg);

    cWindow *windowBelow(const cPoint &p);

    u32 modalRet();

    void centerScreen();

    bool isActive(void) const;

    cWindow &disableFocus(void);

  protected:
    virtual void onOK();

    virtual void onCancel();

    void onResize();

    void onMove();

    bool processKeyMessage(const cKeyMessage &msg);

    std::list<cWindow *> _childWindows;

    //cFormDesc * _renderDesc;
    u32 _modalRet;
};

} // namespace akui

#endif //_AKUI_FORM_H_
