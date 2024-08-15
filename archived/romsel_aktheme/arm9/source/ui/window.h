/*
    window.h
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

#ifndef _AKUI_WINDOW_H_
#define _AKUI_WINDOW_H_

#include <string>
#include "userinput.h"
#include "drawing/gdi.h"
#include "point.h"
#include "rectangle.h"
#include "sigslot.h"
#include "keymessage.h"
#include "touchmessage.h"
#include "tool/dbgtool.h"

namespace akui
{

class Window : public SlotHolder
{
  public:
    explicit Window(Window *aParent = 0, const std::string &aText = "");
    virtual ~Window();

  public:
    Window &setWindowRectangle(const Rect &rect);

    Rect windowRectangle() const;

    //! The window text is a string with a context sensitive interpretation. It can be the
    //! label of a button or the title of a frame window etc.
    const std::string &text() const { return _text; }

    //! \brief Sets the window text; a string with a context sensitive
    //! interpretation. It can be the label of a button or the title of a
    //! frame window etc.
    Window &setText(const std::string &aText);

    //! returns the dimensions of the window in pixels
    Size size() const { return _size; }

    //! sets the dimensions of the window in pixels
    Window &setSize(const Size &aSize);

    //! returns the position of the window in app window coords
    const Point &position() const { return _position; }

    const Point &relativePosition() const { return _relative_position; }
    Window &setRelativePosition(const Point &rPosition)
    {
        _relative_position = rPosition;
        return *this;
    }

    //! sets the position of the window in app window coords
    Window &setPosition(const Point &aPosition);

    //! returns \c true if this window currently has the focus
    bool isFocused() const;

    //! sets the focus to this window
    Window &enableFocused();

    //! removes the focus from this window
    Window &disableFocused();

  public:
    //! \brief Returns a pointer to the window below the passed in point
    //!
    //! The window manager calls this on top level windows to determine
    //! which window is currently below the mouse cursor. Each derived class
    //! which has child windows must overwrite this and pass the call on to
    //! the children.
    virtual Window *windowBelow(const Point &p);

    //! makes the window visible \sa hide
    Window &show();

    //! makes the window invisible \sa show
    Window &hide();

    //! returns true iff the window is currently visible
    bool isVisible() const { return _isVisible; }

    //! \brief Returns true if the passed in window pointer points to either
    //! this window or a child window of this window.
    //!
    //! Each derived class that has child windows must overwrite this and
    //! pass the call on to the children.
    virtual bool doesHierarchyContain(Window *aWindow) const;

    //! returns the top level window belonging to this window
    Window *topLevelWindow() const;

    //! returns a pointer to this window's parent
    Window *parent() const { return _parent; }

    void setParent(Window *parent) { _parent = parent; }

    //! Loads a descriptor for this individual window instance from an xml file
    virtual Window &loadAppearance(const std::string &aFileName) = 0;

    virtual void update() {}

    virtual bool process(const Message &msg);

    bool isFocusable() { return _isFocusable; }

    virtual Window &disableFocus(void)
    {
        _isFocusable = false;
        return *this;
    }

    Window &render();

    void setEngine(GRAPHICS_ENGINE engine) { _engine = engine; }

    GRAPHICS_ENGINE selectedEngine() { return _engine; }

  protected:
    virtual void draw() = 0;

    //! \brief called when the window is shown, derived classes can override this to
    //! react to the window becoming visible
    virtual void onShow() {}

    //! \brief called when the window is hidden, derived classes can override this to
    //! react to the window being hidden
    virtual void onHide()
    { /*dbg_printf("%s default onHide()\n", _text.c_str() );*/
    }

    //! Called when the window receives the focus
    virtual void onGainedFocus()
    { /*dbg_printf("%s get FOCUS\n", _text.c_str() );*/
    }

    //! Called when the window loses the focus
    virtual void onLostFocus() { dbg_printf("%s lost FOCUS\n", _text.c_str()); }

    //! \brief called when the window is resized, derived classes can override this to
    //! react to the window size changing
    virtual void onResize()
    { /*dbg_printf("%s on resize\n", _text.c_str() );*/
    }

    //! \brief called when the window is moved, derived classes can override this to
    //! react to the window position changing
    virtual void onMove()
    { /*dbg_printf("%s on move\n", _text.c_str() ); */
    }

    //! \brief called when the window text changes, derived classes can override this to
    //! react to the window text changing
    virtual void onTextChanged() {}

  protected:
    Window *_parent; //!< The window's parent (or 0 if this window has no parent)

    //MessageListeners messageListeners_; //!< The message listeners attached to this window

    std::string _text;         //!< The window text
    Size _size;               //!< The size of the window
    Point _position;          //!< The position of the window
    Point _relative_position; //!< The position of the window
    bool _isVisible;           //!< The visiblility flag
    bool _isSizeSetByUser;     //!< Whether the user has explicitly set the window's size
    bool _isFocusable;

  protected:
    GRAPHICS_ENGINE _engine;
};

} // namespace akui
#endif //_AKUI_WINDOW_H_
