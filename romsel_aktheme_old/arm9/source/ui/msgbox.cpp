/*
    msgbox.cpp
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
#include "msgbox.h"
#include "font/fontfactory.h"
#include "language.h"

namespace akui
{

MessageBox::MessageBox(s32 x, s32 y, u32 w, u32 h, Window *parent, const std::string &title, const std::string &msg, u32 style)
    : Form(x, y, w, h, parent, title)
{
    u32 largestLineWidth = 0;
    size_t pos1 = 0;
    size_t pos2 = 0;
    size_t lineCount = 0;

    std::string breakedMsg = font().breakLine(msg, 192);

    pos1 = breakedMsg.find('\n');
    while (breakedMsg.npos != pos1)
    {
        ++lineCount;
        u32 lineWidth = font().getStringScreenWidth(&breakedMsg[pos2], pos1 - pos2);
        if (largestLineWidth < lineWidth)
            largestLineWidth = lineWidth;
        pos2 = pos1;
        pos1 = breakedMsg.find('\n', pos2 + 1);
        dbg_printf("line w %d\n", lineWidth);
    }

    _size.x = largestLineWidth + 48;
    if (_size.x < 192)
        _size.x = 192;
    if (_size.x > 256)
        _size.x = 256;
    if (_size.y > 192)
        _size.y = 192;
    if (_size.x & 1)
        --_size.x; // 4 byte align, for speed optimization
    _size.y = lineCount * SYSTEM_FONT_HEIGHT + 60;
    _position.x = (SCREEN_WIDTH - _size.x) / 2;
    if (_position.x & 1)
        --_position.x;
    _position.y = (SCREEN_HEIGHT - _size.y) / 2;
    _textPoision.x = _position.x + (_size.x - largestLineWidth) / 2;
    _textPoision.y = _position.y + 24;
    dbg_printf("_size.x %d largestLineWidth %d\n", _size.x, largestLineWidth);

    _text = title;
    _msg = breakedMsg;
    _style = style;

    _buttonOK = NULL;
    _buttonCANCEL = NULL;
    _buttonYES = NULL;
    _buttonNO = NULL;
    _buttonHOLD_X = NULL;

    _buttonOK = new Button(0, 0, 46, 18, this, "\x01 OK");
    _buttonOK->setText("\x01 " + LANG("message box", "ok"));
    _buttonOK->setStyle(Button::press);
    _buttonOK->hide();
    _buttonOK->loadAppearance(SFN_BUTTON3);
    _buttonOK->setStyle(Button::press);
    _buttonOK->clicked.connect(this, &MessageBox::onOK);
    addChildWindow(_buttonOK);

    _buttonCANCEL = new Button(0, 0, 46, 18, this, "\x02 Cancel");
    _buttonCANCEL->setText("\x02 " + LANG("message box", "cancel"));
    _buttonCANCEL->setStyle(Button::press);
    _buttonCANCEL->hide();
    _buttonCANCEL->loadAppearance(SFN_BUTTON3);
    _buttonCANCEL->clicked.connect(this, &MessageBox::onCANCEL);
    addChildWindow(_buttonCANCEL);

    _buttonYES = new Button(0, 0, 46, 18, this, "\x01 Yes");
    _buttonYES->setText("\x01 " + LANG("message box", "yes"));
    _buttonYES->setStyle(Button::press);
    _buttonYES->hide();
    _buttonYES->loadAppearance(SFN_BUTTON3);
    _buttonYES->clicked.connect(this, &MessageBox::onOK);
    addChildWindow(_buttonYES);

    _buttonNO = new Button(0, 0, 46, 18, this, "\x02 No");
    _buttonNO->setText("\x02 " + LANG("message box", "no"));
    _buttonNO->setStyle(Button::press);
    _buttonNO->hide();
    _buttonNO->loadAppearance(SFN_BUTTON3);
    _buttonNO->clicked.connect(this, &MessageBox::onCANCEL);
    addChildWindow(_buttonNO);

    _buttonHOLD_X = new Button(0, 0, 46, 18, this, "\x03 Don't show");
    _buttonHOLD_X->setText("\x03 " + LANG("message box", "Don't show"));
    _buttonHOLD_X->setStyle(Button::press);
    _buttonHOLD_X->hide();
    _buttonHOLD_X->loadAppearance(SFN_BUTTON4);
    _buttonHOLD_X->clicked.connect(this, &MessageBox::onHOLDX);
    addChildWindow(_buttonHOLD_X);

    s16 nextButtonX = size().x;
    s16 buttonPitch = 60;
    s16 buttonY = size().y - _buttonNO->size().y - 4;

    if (_style & MB_NO)
    {
        buttonPitch = _buttonNO->size().x + 8;
        nextButtonX -= buttonPitch;
        _buttonNO->setRelativePosition(Point(nextButtonX, buttonY));
        _buttonNO->show();
    }

    if (_style & MB_YES)
    {
        buttonPitch = _buttonYES->size().x + 8;
        nextButtonX -= buttonPitch;
        _buttonYES->setRelativePosition(Point(nextButtonX, buttonY));
        _buttonYES->show();
    }

    if (_style & MB_CANCEL)
    {
        buttonPitch = _buttonCANCEL->size().x + 8;
        nextButtonX -= buttonPitch;
        _buttonCANCEL->setRelativePosition(Point(nextButtonX, buttonY));
        _buttonCANCEL->show();
    }

    if (_style & MB_OK)
    {
        buttonPitch = _buttonOK->size().x + 8;
        nextButtonX -= buttonPitch;
        _buttonOK->setRelativePosition(Point(nextButtonX, buttonY));
        _buttonOK->show();
    }

    if (_style & MB_HOLD_X)
    {
        buttonPitch = _buttonHOLD_X->size().x + 8;
        nextButtonX -= buttonPitch;
        _buttonHOLD_X->setRelativePosition(Point(nextButtonX, buttonY));
        _buttonHOLD_X->show();
    }

    arrangeChildren();

    loadAppearance("");
}

MessageBox::~MessageBox()
{
    delete _buttonOK;
    delete _buttonCANCEL;
    delete _buttonYES;
    delete _buttonNO;
    delete _buttonHOLD_X;
}

void MessageBox::onOK()
{
    _modalRet = ID_OK;
}

void MessageBox::onHOLDX()
{
    _modalRet = ID_HOLD_X;
}

void MessageBox::onCANCEL()
{
    _modalRet = ID_CANCEL;
}

bool MessageBox::process(const Message &msg)
{
    bool ret = false;
    if (isVisible())
    {
        ret = Form::process(msg);
        if (!ret)
        {
            if (msg.id() > Message::keyMessageStart && msg.id() < Message::keyMessageEnd)
            {
                ret = processKeyMessage((KeyMessage &)msg);
            }
            else if (msg.id() > Message::touchMessageStart && msg.id() < Message::touchMessageEnd)
            {
                ret = processTouchMessage((TouchMessage &)msg);
            }
        }
    }

    return ret;
}

bool MessageBox::processKeyMessage(const KeyMessage &msg)
{
    bool ret = false;
    if (msg.id() == Message::keyDown && !(_style & MB_HOLD_X))
    {
        switch (msg.keyCode())
        {
        case KeyMessage::UI_KEY_A:
            onOK();
            ret = true;
            return true;
            break;
        case KeyMessage::UI_KEY_B:
            onCANCEL();
            ret = true;
            return true;
            break;
        }
    }
    else if (msg.id() == Message::keyDown && (_style & MB_HOLD_X))
    {
        switch (msg.keyCode())
        {
        case KeyMessage::UI_KEY_A:
            onOK();
            ret = true;
            return true;
            break;
        case KeyMessage::UI_KEY_B:
            onCANCEL();
            ret = true;
            return true;
            break;
        case KeyMessage::UI_KEY_X:
            onHOLDX();
            ret = true;
            return true;
            break;
        }
    }
    return ret;
}

bool MessageBox::processTouchMessage(const TouchMessage &msg)
{
    return false;
}

void MessageBox::draw()
{
    _renderDesc.draw(windowRectangle(), _engine);
    Form::draw();

    // draw message text
    gdi().setPenColor(uiSettings().formTextColor, _engine);
    gdi().textOut(_textPoision.x, _textPoision.y, _msg.c_str(), _engine);
}

Window &MessageBox::loadAppearance(const std::string &aFileName)
{
    _renderDesc.loadData(
        SFN_FORM_TITLE_L, SFN_FORM_TITLE_R, SFN_FORM_TITLE_M);

    _renderDesc.setTitleText(_text);
    return *this;
}

u32 messageBox(Window *parent, const std::string &title, const std::string &msg, u32 style)
{
    // check point ���������ֵĶԻ�����ʧ����ͼ������
    MessageBox msgbox(12, 36, 232, 120, parent, title, msg, style);
    //cMessageBox msgbox( 0, 0, 256, 192, parent, text, style );

    return msgbox.doModal();
}

} // namespace akui
