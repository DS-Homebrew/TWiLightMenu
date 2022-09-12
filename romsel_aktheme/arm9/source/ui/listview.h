/*
    listview.h
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

#ifndef _AKUI_LISTVIEW_H_
#define _AKUI_LISTVIEW_H_

#include <string>
#include <vector>
#include "window.h"

namespace akui
{
class ListColumn
{
  public:
    ListColumn() : index(0), width(0), offset(0), text("") {}

    u32 index;
    u16 width;
    u16 offset;
    std::string text;
};

class ListItem
{
  protected:
    std::string _text;
    u32 _lines;

  public:
    ListItem() : _text(""), _lines(1), index(0), column(0), imageId(0), param(0) {}
    u32 index;
    u32 column;
    u32 imageId;
    u32 param;
    const std::string &text(void) const { return _text; }
    u32 lines(void) const { return _lines; }
    void setText(const std::string &text);
};

class ListView : public Window
{
  public:
    ListView(s32 x, s32 y, u32 w, u32 h, Window *parent, const std::string &text, int scrollSpeed);

    virtual ~ListView() {}

    typedef std::vector<ListItem> itemVector; // a column

  public:
    class OwnerDraw
    {
      public:
        size_t _row;
        size_t _col;
        Point _position;
        Size _size;
        s16 _textY;
        u16 _textHeight;
        const char *_text;
        GRAPHICS_ENGINE _engine;
        OwnerDraw(size_t row, size_t col, const Point &position, const Size &size, s16 textY, u16 textHeight, const char *text, GRAPHICS_ENGINE engine) : _row(row), _col(col), _position(position), _size(size), _textY(textY), _textHeight(textHeight), _text(text), _engine(engine){};
    };

  public:

    void draw();

    bool insertColumn(size_t index, const std::string &text, u8 width);

    bool appendRow(const std::vector<std::string> &&text);

    void removeAllRows();

    u32 getRowCount() { return _rows.size(); }

    void setRowHeight(u16 height)
    {
        _rowHeight = height;
        setListViewSize(_size.x, _size.y);
    }

    void setListViewSize(u16 w, u16 h)
    {
        setSize(Size(w, h));
        _visibleRowCount = h / _rowHeight;
        //if ( _visibleRowCount > _rows.size() ) _visibleRowCount = _rows.size();
    }

    void arangeColumnsSize();

    void selectRow(int id);

    u32 firstVisibleRowId(void) { return _firstVisibleRowId; }

    u32 selectedRowId(void) { return _selectedRowId; }

    void selectNext() { selectRow(_selectedRowId + 1); }

    void selectPrev() { selectRow(_selectedRowId - 1); }

    u32 rowBelowPoint(const Point &pt);

    void scrollTo(int id);

    u32 visibleRowCount() { return _visibleRowCount; }

    void setScrollSpeed(int scrollSpeed);

    Window &loadAppearance(const std::string &aFileName);

    bool process(const akui::Message &msg);

    inline void setColors(u16 textColor, u16 textColorHilight, u16 selectionBarColor1, u16 selectionBarColor2)
    {
        _textColor = textColor;
        _textColorHilight = textColorHilight;
        _selectionBarColor1 = selectionBarColor1;
        _selectionBarColor2 = selectionBarColor2;
    };

    void setFirstVisibleIdAndSelectRow(u32 first, u32 row);

    Signal1<u32> selectedRowClicked;

    Signal1<u32> selectChanged;

    Signal1<u32> scrolled;

    Signal1<const OwnerDraw &> ownerDraw;

  protected:
    virtual void onSelectChanged(u32 index) {}

    virtual void onSelectedRowClicked(u32 index) {}

    virtual void onScrolled(u32 index) {}

    bool processTouchMessage(const akui::TouchMessage &msg);

  protected:
    void drawSelectionBar();

    void drawText();

  protected:
    BMP15 _barPic;
    std::string _text;
    u16 _rowHeight;
    u16 _textColor;
    u16 _textColorHilight;
    u16 _selectionBarColor1;
    u16 _selectionBarColor2;
    u16 _selectionBarOpacity;
    bool _showSelectionBarBg;

    u32 _selectedRowId;
    u32 _firstVisibleRowId;
    u32 _visibleRowCount;
    u16 _rowsPerpage;
    bool _touchMovedAfterTouchDown;

    int _scrollSpeed;

    std::vector<ListColumn> _columns;
    std::vector<itemVector> _rows;
};

} // namespace akui
#endif //_LISTVIEW_H_
