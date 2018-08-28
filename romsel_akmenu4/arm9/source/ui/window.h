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

class cWindow : public SlotHolder
{
  public:
    explicit cWindow(cWindow *aParent = 0, const std::string &aText = "");
    virtual ~cWindow();

  public:
    cWindow &setWindowRectangle(const cRect &rect);

    cRect windowRectangle() const;

    //! The window text is a string with a context sensitive interpretation. It can be the
    //! label of a button or the title of a frame window etc.
    const std::string &text() const { return _text; }

    //! \brief Sets the window text; a string with a context sensitive
    //! interpretation. It can be the label of a button or the title of a
    //! frame window etc.
    cWindow &setText(const std::string &aText);

    //! returns the dimensions of the window in pixels
    cSize size() const { return _size; }

    //! sets the dimensions of the window in pixels
    cWindow &setSize(const cSize &aSize);

    //! returns the position of the window in app window coords
    const cPoint &position() const { return _position; }

    const cPoint &relativePosition() const { return _relative_position; }
    cWindow &setRelativePosition(const cPoint &rPosition)
    {
        _relative_position = rPosition;
        return *this;
    }

    //! sets the position of the window in app window coords
    cWindow &setPosition(const cPoint &aPosition);

    //! returns \c true if this window currently has the focus
    bool isFocused() const;

    //! sets the focus to this window
    cWindow &enableFocused();

    //! removes the focus from this window
    cWindow &disableFocused();

  public:
    //! \brief Returns a pointer to the window below the passed in point
    //!
    //! The window manager calls this on top level windows to determine
    //! which window is currently below the mouse cursor. Each derived class
    //! which has child windows must overwrite this and pass the call on to
    //! the children.
    virtual cWindow *windowBelow(const cPoint &p);

    //! makes the window visible \sa hide
    cWindow &show();

    //! makes the window invisible \sa show
    cWindow &hide();

    //! returns true iff the window is currently visible
    bool isVisible() const { return _isVisible; }

    //! \brief Returns true if the passed in window pointer points to either
    //! this window or a child window of this window.
    //!
    //! Each derived class that has child windows must overwrite this and
    //! pass the call on to the children.
    virtual bool doesHierarchyContain(cWindow *aWindow) const;

    //! returns the top level window belonging to this window
    cWindow *topLevelWindow() const;

    //! returns a pointer to this window's parent
    cWindow *parent() const { return _parent; }

    void setParent(cWindow *parent) { _parent = parent; }

    //! Loads a descriptor for this individual window instance from an xml file
    virtual cWindow &loadAppearance(const std::string &aFileName) = 0;

    virtual void update() {}

    virtual bool process(const cMessage &msg);

    bool isFocusable() { return _isFocusable; }

    virtual cWindow &disableFocus(void)
    {
        _isFocusable = false;
        return *this;
    }

    cWindow &render();

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
    cWindow *_parent; //!< The window's parent (or 0 if this window has no parent)

    //MessageListeners messageListeners_; //!< The message listeners attached to this window

    std::string _text;         //!< The window text
    cSize _size;               //!< The size of the window
    cPoint _position;          //!< The position of the window
    cPoint _relative_position; //!< The position of the window
    bool _isVisible;           //!< The visiblility flag
    bool _isSizeSetByUser;     //!< Whether the user has explicitly set the window's size
    bool _isFocusable;

  protected:
    GRAPHICS_ENGINE _engine;
};

} // namespace akui
#endif //_AKUI_WINDOW_H_
