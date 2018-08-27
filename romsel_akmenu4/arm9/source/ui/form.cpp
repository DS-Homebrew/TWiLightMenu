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
#include "timer.h"
#include "../dbgtool.h"
#include "windowmanager.h"

namespace akui
{

cForm::cForm(s32 x, s32 y, u32 w, u32 h, cWindow *parent, const std::string &text)
    : cWindow(parent, text)
//_renderDesc(NULL)
{
    _size = cSize(w, h);
    _position = cPoint(x, y);
    _modalRet = -1;
}

cForm::~cForm()
{
    //if( _renderDesc )
    //    delete _renderDesc;
}

cForm &cForm::addChildWindow(cWindow *aWindow)
{
    _childWindows.push_back(aWindow);
    aWindow->setPosition(_position + aWindow->relativePosition());
    //layouter_->addWindow(aWindow);
    return *this;
}

cForm &cForm::removeChildWindow(cWindow *aWindow)
{
    _childWindows.remove(aWindow);
    //layouter_->removeWindow(aWindow);
    return *this;
}

cForm &cForm::arrangeChildren()
{
    std::list<cWindow *>::iterator it;
    for (it = _childWindows.begin(); it != _childWindows.end(); ++it)
    {
        (*it)->setPosition(_position + (*it)->relativePosition());
    }
    return *this;
}

void cForm::draw()
{
    std::list<cWindow *>::iterator it;
    for (it = _childWindows.begin(); it != _childWindows.end(); ++it)
    {
        (*it)->render();
    }
}

bool cForm::process(const cMessage &msg)
{
    dbg_printf("cForm::process\n");
    bool ret = false;
    if (isVisible())
    {
        if (msg.id() > cMessage::touchMessageStart && msg.id() < cMessage::touchMessageEnd)
        {
            std::list<cWindow *>::iterator it;
            for (it = _childWindows.begin(); it != _childWindows.end(); ++it)
            {
                cWindow *window = *it;
                ret = window->process(msg);
                if (ret)
                {
                    dbg_printf("(%s) processed\n", window->text().c_str());
                    break;
                }
            }
        }
    }

    // NOTE: cForm does not translate key messages to children in this case

    //if( !ret ) {
    //    dbg_printf("change child focus\n");
    //    if( msg.id() > cMessage::keyMessageStart && msg.id() < cMessage::keyMessageEnd ) {
    //        ret = processKeyMessage( (cKeyMessage &)msg );
    //    }
    //}

    if (!ret)
    {
        ret = cWindow::process(msg);
    }

    return ret;
}

bool cForm::processKeyMessage(const cKeyMessage &msg)
{
    bool ret = false;
    if (msg.id() == cMessage::keyDown)
    {

        if (msg.keyCode() >= 5 && msg.keyCode() <= 8)
        {

            std::list<cWindow *>::iterator it = _childWindows.begin();
            for (it = _childWindows.begin(); it != _childWindows.end(); ++it)
            {
                cWindow *window = *it;
                if (window->isFocused())
                {
                    if (msg.keyCode() == cKeyMessage::UI_KEY_DOWN || msg.keyCode() == cKeyMessage::UI_KEY_RIGHT)
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
                    else if (msg.keyCode() == cKeyMessage::UI_KEY_UP || msg.keyCode() == cKeyMessage::UI_KEY_LEFT)
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

cWindow *cForm::windowBelow(const cPoint &p)
{
    cWindow *ret = cWindow::windowBelow(p); // �ȿ��Լ��ڲ��ڵ�����

    if (ret != 0)
    {
        std::list<cWindow *>::reverse_iterator it;
        for (it = _childWindows.rbegin(); it != _childWindows.rend(); ++it)
        {
            cWindow *window = *it;
            cWindow *cw = window->windowBelow(p);
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

void cForm::onResize()
{
    arrangeChildren();
}

void cForm::onMove()
{
    arrangeChildren();
}

u32 cForm::modalRet()
{
    return _modalRet;
}

u32 cForm::doModal()
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

void cForm::onOK()
{
    _modalRet = 1;
}

void cForm::onCancel()
{
    _modalRet = 0;
}

void cForm::centerScreen()
{
    _position.x = (SCREEN_WIDTH - _size.x) / 2;
    _position.y = (SCREEN_HEIGHT - _size.y) / 2;
}

bool cForm::isActive(void) const
{
    bool result = isFocused();
    for (std::list<cWindow *>::const_iterator it = _childWindows.begin(); !result && it != _childWindows.end(); ++it)
    {
        result = result || (*it)->isFocused();
    }
    return result;
}

cWindow &cForm::disableFocus(void)
{
    for (std::list<cWindow *>::iterator it = _childWindows.begin(); it != _childWindows.end(); ++it)
    {
        (*it)->disableFocus();
    }
    return cWindow::disableFocus();
}

} // namespace akui
