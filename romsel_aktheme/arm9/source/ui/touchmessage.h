/*
    touchmessage.h
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

#ifndef _AKUI_TOUCHMESSAGE_H_
#define _AKUI_TOUCHMESSAGE_H_

#include "message.h"

namespace akui
{

//! Message caused by interaction with the mouse
class TouchMessage : public Message
{
    // ---------------------------------------------------------------------------------------------
    //  Construction / Destruction
    // ---------------------------------------------------------------------------------------------
  public:
    //! construction requires the id and the mouse position
    TouchMessage(Id anId, const Point &aPosition) : Message(anId),
                                                      _position(aPosition)
    {
    }

    //! destructor
    virtual ~TouchMessage() {}

    // ---------------------------------------------------------------------------------------------
    //  Accessors
    // ---------------------------------------------------------------------------------------------
  public:
    //! returns the position of the mouse cursor when the message occured
    Point position() const { return _position; }

    //! convenience method for getting the mouse x position
    s32 x() const { return position().x; }

    //! convenience method for getting the mouse y position
    s32 y() const { return position().y; }

    // ---------------------------------------------------------------------------------------------
    //  Implementation
    // ---------------------------------------------------------------------------------------------
  protected:
    Point _position;
};

} // namespace akui

#endif //_AKUI_TOUCHMESSAGE_H_
