/*
    msgbox.h
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

#ifndef _AKUI_MESSAGEBOX_H_
#define _AKUI_MESSAGEBOX_H_

#include "window.h"
#include "form.h"
#include "formdesc.h"
#include "button.h"

namespace akui
{

#define MB_OK 1        // 0b00000001
#define MB_CANCEL 2    // 0b00000010
#define MB_OK_CANCEL 3 // 0b00000011
#define MB_YES 4       // 0b00000100
#define MB_NO 8        // 0b00001000
#define MB_YES_NO 12   // 0b00001100
#define MB_HOLD_X 16    // 0b00010000

#define ID_OK 1
#define ID_CANCEL 0
#define ID_YES 1
#define ID_NO 0
#define ID_HOLD_X 0x10

class MessageBox : public Form
{
  public:
    friend u32 messageBox(Window *parent, const std::string &title, const std::string &msg, u32 style);

    MessageBox(s32 x, s32 y, u32 w, u32 h, Window *parent, const std::string &title, const std::string &msg, u32 style);

    ~MessageBox();

  public:
    void draw();

    bool process(const Message &msg);

    Window &loadAppearance(const std::string &aFileName);

    //u32 doModal();

    //u32 msgRet() { return _msgRet; }

  protected:
    void onOK();

    void onCANCEL();

    void onYES() { onOK(); }

    void onNO() { onCANCEL(); }

    void onHOLDX();

    bool processKeyMessage(const KeyMessage &msg);

    bool processTouchMessage(const TouchMessage &msg);

    std::string _msg;

    Point _textPoision;

    u32 _style;

    Button *_buttonOK;
    Button *_buttonCANCEL;
    Button *_buttonYES;
    Button *_buttonNO;
    Button *_buttonHOLD_X;
    FormDesc _renderDesc;
};

u32 messageBox(Window *parent, const std::string &title, const std::string &msg, u32 style);

} // namespace akui

#endif //_MESSAGEBOX_H_
