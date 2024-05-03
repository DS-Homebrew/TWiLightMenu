/*
    rectangle.h
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

#ifndef _AKUI_RECTANGLE_H_
#define _AKUI_RECTANGLE_H_

#include "point.h"

namespace akui
{

class Rect
{
    // ---------------------------------------------------------------------------------------------
    //  Xstruction
    // ---------------------------------------------------------------------------------------------
  public:
    //! default contructor - both points at (0, 0)
    Rect() {}

    //! construction via two points
    Rect(const Point &p1, const Point &p2);

    //! construction from components of the two corner points
    Rect(int x1, int y1, int x2, int y2);

    // ---------------------------------------------------------------------------------------------
    //  Public Interface
    // ---------------------------------------------------------------------------------------------
  public:
    //! returns the position (i.e. lowest value corner) of the rectangle
    Point position() const { return position_; }

    //! returns half the rectangle's size
    Size halfSize() const { return Point(size_.x >> 1, size_.y >> 1); }

    //! the center point
    Point centerPoint() const;

    //! is the passed in point within the rectangle's horizontal edges
    bool isAboveAndBelow(const Point &p) const;

    //! is the passed in point within the rectangle's vertical edges
    bool isLeftAndRightOf(const Point &p) const;

    //! is the passed in point within the rectangle's edges
    bool surrounds(const Point &p) const;

    //! the smallest x coord
    int minX() const { return position_.x; }

    //! the smallest y coord
    int minY() const { return position_.y; }

    //! the largest x coord
    int maxX() const { return position_.x + size_.x; }

    //! the largest y coord
    int maxY() const { return position_.y + size_.y; }

    //! the top right corner point
    Point topRight() const;

    //! the bottom left corner point
    Point bottomLeft() const;

    //! the top left corner point
    Point topLeft() const;

    //! the bottom right corner point
    Point bottomRight() const;

    //! returns the current size
    Point size() const { return size_; }

    //! move to the passed in point
    Rect &setPosition(const Point &p)
    {
        position_ = p;
        return *this;
    }

    //! resize to the passed in size
    Rect &setSize(const Size &s)
    {
        size_ = s;
        return *this;
    }

    //! translate through the passed in offset
    Rect &translateBy(const Point &p);

    //! \brief expands the rectangle in all directions by the passed in amount
    //! \note Accepts negative values
    Rect &expandBy(int amount);

    //! \brief expands the rectangle horizontally by the passed in amount
    //! \note Accepts negative values
    Rect &expandWidthBy(int amount);

    //! \brief expands the rectangle horizontally by the passed in amount
    //! \note Accepts negative values
    Rect &expandHeightBy(int amount);

    //! the height of the rectangle
    int height() const;

    //! the width of the rectangle
    int width() const;

    //! Snaps the size and position of the rectangle to the nearest whole number positions
    Rect &snap();

    // ---------------------------------------------------------------------------------------------
    //  Operators
    // ---------------------------------------------------------------------------------------------
  public:
    //! equality
    bool operator==(const Rect &rect) const;

    //! inequality
    bool operator!=(const Rect &rect) const;

    // ---------------------------------------------------------------------------------------------
    //  Implementation
    // ----------------------------------------------------------------------------------------------
  private:
    //! lowest value corner of the rectangle
    Point position_;

    //! extents of the rectangle
    Point size_;
};

////! output operator
//std::ostream& operator << (std::ostream& os, const Rect& rect);

// *************************************************************************************************
} // namespace akui

#endif //_AKUI_RECTANGLE_H_
