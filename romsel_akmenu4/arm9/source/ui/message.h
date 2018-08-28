/*
    message.h
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

#ifndef _AKUI_MESSAGE_H_
#define _AKUI_MESSAGE_H_

namespace akui
{

class Message
{
    // ---------------------------------------------------------------------------------------------
    //  public enumerations
    // ---------------------------------------------------------------------------------------------
  public:
    //! all possible messages have a unique id
    enum Id
    {
        keyMessageStart = 0,
        keyDown, //!< \sa KeyMessage
        keyUp,   //!< \sa KeyMessage
        keyMessageEnd,
        touchMessageStart,
        touchMove, //!< \sa TouchMessage
        touchDown, //!< \sa TouchMessage
        touchUp,   //!< \sa TouchMessage
        touchMessageEnd
        // more to come...
    };

    // ---------------------------------------------------------------------------------------------
    //  Construction / Destruction
    // ---------------------------------------------------------------------------------------------
  public:
    //! constructor requires id
    Message(Id anId) : _id(anId) {}

    //! destructor
    virtual ~Message() {}

    // ---------------------------------------------------------------------------------------------
    //  Accessors
    // ---------------------------------------------------------------------------------------------
  public:
    //! returns the id of the message
    Id id() const { return _id; }

    // ---------------------------------------------------------------------------------------------
    //  Implementation
    // ---------------------------------------------------------------------------------------------
  protected:
    Id _id;
};

} // namespace akui

#endif //_AKUI_MESSAGE_H_
