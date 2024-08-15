/*
    form.cpp
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
#include "form.h"
#include "time/timer.h"
#include "tool/dbgtool.h"
#include "windowmanager.h"

namespace akui
{

Form::Form(s32 x, s32 y, u32 w, u32 h, Window *parent, const std::string &text)
    : Window(parent, text)
//_renderDesc(NULL)
{
    _size = Size(w, h);
    _position = Point(x, y);
    _modalRet = -1;
}

Form::~Form()
{
    //if ( _renderDesc )
    //    delete _renderDesc;
}

Form &Form::addChildWindow(Window *aWindow)
{
    _childWindows.push_back(aWindow);
    aWindow->setPosition(_position + aWindow->relativePosition());
    //layouter_->addWindow(aWindow);
    return *this;
}

Form &Form::removeChildWindow(Window *aWindow)
{
    _childWindows.remove(aWindow);
    //layouter_->removeWindow(aWindow);
    return *this;
}

Form &Form::arrangeChildren()
{
    std::list<Window *>::iterator it;
    for (it = _childWindows.begin(); it != _childWindows.end(); ++it)
    {
        (*it)->setPosition(_position + (*it)->relativePosition());
    }
    return *this;
}

void Form::draw()
{
    std::list<Window *>::iterator it;
    for (it = _childWindows.begin(); it != _childWindows.end(); ++it)
    {
        (*it)->render();
    }
}

bool Form::process(const Message &msg)
{
    dbg_printf("cForm::process\n");
    bool ret = false;
    if (isVisible())
    {
        if (msg.id() > Message::touchMessageStart && msg.id() < Message::touchMessageEnd)
        {
            std::list<Window *>::iterator it;
            for (it = _childWindows.begin(); it != _childWindows.end(); ++it)
            {
                Window *window = *it;
                ret = window->process(msg);
                if (ret)
                {
                    dbg_printf("(%s) processed\n", window->text().c_str());
                    break;
                }
            }
        }
    }

    // NOTE: Form does not translate key messages to children in this case

    //if ( !ret ) {
    //    dbg_printf("change child focus\n");
    //    if ( msg.id() > Message::keyMessageStart && msg.id() < Message::keyMessageEnd ) {
    //        ret = processKeyMessage( (KeyMessage &)msg );
    //    }
    //}

    if (!ret)
    {
        ret = Window::process(msg);
    }

    return ret;
}

bool Form::processKeyMessage(const KeyMessage &msg)
{
    bool ret = false;
    if (msg.id() == Message::keyDown)
    {

        if (msg.keyCode() >= 5 && msg.keyCode() <= 8)
        {

            std::list<Window *>::iterator it = _childWindows.begin();
            for (it = _childWindows.begin(); it != _childWindows.end(); ++it)
            {
                Window *window = *it;
                if (window->isFocused())
                {
                    if (msg.keyCode() == KeyMessage::UI_KEY_DOWN || msg.keyCode() == KeyMessage::UI_KEY_RIGHT)
                    {
                        ++it;
                        if (it == _childWindows.end())
                            it = _childWindows.begin();
                        if ((*it)->isVisible())
                        {
                            windowManager().setFocusedWindow((*it));
                            ret = true;
                            break;
                        }
                    }
                    else if (msg.keyCode() == KeyMessage::UI_KEY_UP || msg.keyCode() == KeyMessage::UI_KEY_LEFT)
                    {
                        if (it == _childWindows.begin())
                        {
                            it = _childWindows.end();
                        }
                        --it;
                        if ((*it)->isVisible())
                        {
                            windowManager().setFocusedWindow((*it));
                            ret = true;
                            break;
                        }
                    }
                }
            }
            if (_childWindows.end() == it)
            {
                if (_childWindows.front()->isVisible())
                {
                    windowManager().setFocusedWindow(_childWindows.front());
                    ret = true;
                }
            }
        }
    }
    return ret;
}

Window *Form::windowBelow(const Point &p)
{
    Window *ret = Window::windowBelow(p); // �ȿ��Լ��ڲ��ڵ�����

    if (ret != 0)
    {
        std::list<Window *>::reverse_iterator it;
        for (it = _childWindows.rbegin(); it != _childWindows.rend(); ++it)
        {
            Window *window = *it;
            Window *cw = window->windowBelow(p);
            //dbg_printf( "check child (%s)\n", window->text().c_str() );
            if (cw != 0)
            {
                ret = cw;
                break;
            }
        }
    }

    return ret;
}

void Form::onResize()
{
    arrangeChildren();
}

void Form::onMove()
{
    arrangeChildren();
}

u32 Form::modalRet()
{
    return _modalRet;
}

u32 Form::doModal()
{
    windowManager().addWindow(this);
    show();

    do
    { // manually update system loop
        timer().updateFps();
        INPUT &inputs = updateInput();
        processInput(inputs);
        windowManager().update();
        gdi().present(GE_MAIN);
        //dbg_printf( "modal window looping\n" );
    } while (modalRet() == (u32)-1);

    windowManager().removeWindow(this);
    return modalRet();
}

void Form::onOK()
{
    _modalRet = 1;
}

void Form::onCancel()
{
    _modalRet = 0;
}

void Form::centerScreen()
{
    _position.x = (SCREEN_WIDTH - _size.x) / 2;
    _position.y = (SCREEN_HEIGHT - _size.y) / 2;
}

bool Form::isActive(void) const
{
    bool result = isFocused();
    for (std::list<Window *>::const_iterator it = _childWindows.begin(); !result && it != _childWindows.end(); ++it)
    {
        result = result || (*it)->isFocused();
    }
    return result;
}

Window &Form::disableFocus(void)
{
    for (std::list<Window *>::iterator it = _childWindows.begin(); it != _childWindows.end(); ++it)
    {
        (*it)->disableFocus();
    }
    return Window::disableFocus();
}

} // namespace akui
