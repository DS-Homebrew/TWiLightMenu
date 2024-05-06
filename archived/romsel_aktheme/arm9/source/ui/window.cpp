/*
    window.cpp
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
#include "window.h"
#include "windowmanager.h"

namespace akui {

Window::Window(Window* aParent, const std::string& aText) :
    _parent(aParent),
    _text(aText),
    _size(Size(0, 0)),
    _position(Point(0, 0)),
    _relative_position(Point(0, 0)),
    _isVisible(true),
    _isSizeSetByUser(false),
    _isFocusable(true),
    _engine( GE_MAIN )
{
}

Window::~Window()
{
    if ( isFocused() )
        windowManager().setFocusedWindow( NULL );
}

Window& Window::setWindowRectangle(const Rect& rect)
{
    setSize(rect.size());
    setPosition(rect.position());
    return *this;
}




Rect Window::windowRectangle() const
{
    return Rect(position(), position() + size());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


bool Window::isFocused() const
{
    return windowManager().focusedWindow() == this;
}


Window& Window::enableFocused()
{
    onGainedFocus();
    return *this;
}

Window& Window::disableFocused()
{
    onLostFocus();
    return *this;
}

Window* Window::windowBelow(const Point & p)
{
    Window* ret = 0;
    if (isVisible())
    {
        if (windowRectangle().surrounds(p)) ret = this;
    }
    return ret;
}


Window& Window::show()
{
    _isVisible = true;
    onShow();
    return *this;

}

Window& Window::hide()
{
    _isVisible = false;
    onHide();
    return *this;

}

bool Window::doesHierarchyContain(Window* aWindow) const
{
    return (aWindow == this);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

Window* Window::topLevelWindow() const
{
    Window* ret = (Window*)(this);
    Window* test = ret;
    while (test != 0)
    {
        ret = test;
        test = ret->parent();
    }
    return ret;
}

bool Window::process( const Message & msg )
{
    //dbg_printf("%08x call default process()\n", this );
    return false;
}

Window& Window::render()
{
    ////dbg_printf("cWindow::render this is %08x\n", this );
    if ( isVisible() )
        draw();
    return *this;
}

Window& Window::setSize(const Size& aSize)
{
    _size = aSize;
    onResize();
    _isSizeSetByUser = true;
    return *this;
}

Window& Window::setPosition(const Point& aPosition)
{
    _position = aPosition;
    onMove();
    return *this;
}

Window& Window::setText(const std::string& aText)
{
    _text = aText;
    onTextChanged();
    return *this;
}

} // namespace akui
