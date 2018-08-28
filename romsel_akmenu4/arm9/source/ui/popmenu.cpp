/*
    popmenu.cpp
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

//�

#include "ui.h"
#include "popmenu.h"
#include "windowmanager.h"

namespace akui
{

PopMenu::PopMenu(s32 x, s32 y, u32 w, u32 h, Window *parent, const std::string &text)
    : Window(parent, text)
{
    _size = Size(w, h);
    _position = Point(x, y);

    _selectedItemIndex = 0;
    _itemHeight = 0;
    _itemWidth = 0;
    _barLeft = 2;

    _textColor = uiSettings().popMenuTextColor;
    _textHighLightColor = uiSettings().popMenuTextHighLightColor;
    _barColor = uiSettings().popMenuBarColor;

    _renderDesc = new BitmapDesc();
    _renderDesc->setBltMode(BM_MASKBLT);

    _skipTouch = false;
}

PopMenu::~PopMenu()
{
    if (NULL != _renderDesc)
        delete _renderDesc;
}

void PopMenu::popup()
{
    show();
    return;
}

void PopMenu::addItem(size_t index, const std::string &itemText)
{
    if (index > _items.size())
        index = _items.size();
    _items.insert(_items.begin() + index, itemText);
}

void PopMenu::removeItem(size_t index)
{
    if (index > _items.size() - 1)
        index = _items.size() - 1;
    _items.erase(_items.begin() + index);
}

size_t PopMenu::itemCount()
{
    return _items.size();
}

void PopMenu::clearItem()
{
    _items.clear();
}

void PopMenu::draw()
{
    _renderDesc->draw(windowRectangle(), selectedEngine());
    drawItems();
}

void PopMenu::drawItems()
{
    // ѭ������item���֣����� selected ���־��Ȼ���ѡ����
    for (size_t i = 0; i < _items.size(); ++i)
    {
        s16 itemX = _position.x + _itemTopLeftPoint.x;
        s16 itemY = _position.y + i * _itemHeight + _itemTopLeftPoint.y;
        if (_selectedItemIndex == (s16)i)
        {
            s16 barX = _position.x + _barLeft;
            s16 barY = itemY - 2;
            gdi().setPenColor(_barColor, _engine);
            gdi().fillRect(_barColor, _barColor, barX, barY, barWidth(), _itemHeight, _engine);
            gdi().setPenColor(_textHighLightColor, _engine);
        }
        else
        {
            gdi().setPenColor(_textColor, _engine);
        }
        gdi().textOut(itemX, itemY, _items[i].c_str(), _engine);
    }
}

s16 PopMenu::barWidth(void)
{
    return _itemWidth ? _itemWidth : (_size.x - 2 * _barLeft);
}

bool PopMenu::process(const Message &msg)
{
    bool ret = false;
    if (isVisible())
    {
        //ret = Form::process( msg );
        if (msg.id() > Message::keyMessageStart && msg.id() < Message::keyMessageEnd)
        {
            ret = processKeyMessage((KeyMessage &)msg);
        }
        else if (msg.id() > Message::touchMessageStart && msg.id() < Message::touchMessageEnd)
        {
            ret = processTouchMessage((TouchMessage &)msg);
        }
    }

    // PopMenu process all KEY messages while it is showing
    // derived classes can override this feature
    return ret;
}

bool PopMenu::processKeyMessage(const KeyMessage &msg)
{
    bool ret = false;
    switch (msg.keyCode())
    {
    case KeyMessage::UI_KEY_DOWN:
        _selectedItemIndex += 1;
        if (_selectedItemIndex > (s16)_items.size() - 1)
            _selectedItemIndex = 0;
        ret = true;
        break;
    case KeyMessage::UI_KEY_UP:
        _selectedItemIndex -= 1;
        if (_selectedItemIndex < 0)
            _selectedItemIndex = (s16)_items.size() - 1;
        ret = true;
        break;
    case KeyMessage::UI_KEY_A:
        // do something by ( _selectedItemIndex )
        hide();
        itemClicked(_selectedItemIndex);
        ret = true;
        break;
    case KeyMessage::UI_KEY_B:
        hide();
        ret = true;
        break;
    };

    return ret;
}

bool PopMenu::processTouchMessage(const TouchMessage &msg)
{
    bool ret = false;
    if (msg.id() == Message::touchUp)
    {
        if (windowBelow(Point(msg.position().x, msg.position().y)) && !_skipTouch)
        {
            hide();
            itemClicked(_selectedItemIndex);
        }
        else
            hide();

        _skipTouch = false;
        ret = true;
    }
    if (msg.id() == Message::touchMove || msg.id() == Message::touchDown)
    {
        const INPUT &input = getInput();
        size_t item = itemBelowPoint(Point(input.touchPt.px, input.touchPt.py));
        if ((size_t)-1 == item)
            _skipTouch = true;
        else
            _selectedItemIndex = item;
        ret = true;
    }

    return ret;
}

u32 PopMenu::itemBelowPoint(const Point &pt)
{
    Point menuPos(position().x + _barLeft, position().y + _itemTopLeftPoint.y - 2);
    Size menuSize(barWidth(), _itemHeight * _items.size());
    Rect rect(menuPos, menuPos + menuSize);

    if (rect.surrounds(pt))
    {
        u32 item = (pt.y - menuPos.y) / _itemHeight;
        if (item > _items.size() - 1)
            item = _items.size() - 1;
        return item;
    }
    return (u32)-1;
}

void PopMenu::onShow()
{
    _selectedItemIndex = 0;
}

} // namespace akui
