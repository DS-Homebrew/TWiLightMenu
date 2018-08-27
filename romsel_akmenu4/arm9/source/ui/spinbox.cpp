/*
    spinbox.cpp
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
#include "spinbox.h"
#include "../font/fontfactory.h"

//#include "windowmanager.h"

namespace akui
{

cSpinBox::cSpinBox(s32 x, s32 y, u32 w, u32 h, cWindow *parent, const std::string &text)
    : cForm(x, y, w, h, parent, text),
      _prevButton(0, 0, 0, 0, this, ""),
      _nextButton(0, 0, 0, 0, this, ""),
      _itemText(0, 0, 0, 0, this, "spinbox")
{
    _normalColor = uiSettings().spinBoxNormalColor; //RGB15( 0, 0, 31 );
    _focusedColor = uiSettings().spinBoxFocusColor; //RGB15( 0, 31, 0 );
    _frameColor = uiSettings().spinBoxFrameColor;
    _itemText.setTextColor(uiSettings().spinBoxTextColor);

    _prevButton.pressed.connect(this, &cSpinBox::selectPrev);
    _prevButton.pressed.connect(this, &cSpinBox::onCmponentClicked);
    _nextButton.pressed.connect(this, &cSpinBox::selectNext);
    _nextButton.pressed.connect(this, &cSpinBox::onCmponentClicked);

    addChildWindow(&_itemText);
    addChildWindow(&_prevButton);
    addChildWindow(&_nextButton);

    _itemText.setTextColor(RGB15(31, 31, 31));

    u8 cx = 0;
    _prevButton.setSize(cSize(18, 18));
    _prevButton.setRelativePosition(cPoint(cx, 0));

    cx = 0 + _prevButton.windowRectangle().width();
    _itemText.setRelativePosition(cPoint(cx, 0));
    _itemText.setSize(cSize(w - 18 * 2, 18));

    cx = windowRectangle().width() - _nextButton.windowRectangle().width();
    _nextButton.setSize(cSize(18, 18));
    _nextButton.setRelativePosition(cPoint(cx, 0));

    selectItem(0);
}

cSpinBox::~cSpinBox()
{
}

void cSpinBox::selectItem(u32 id)
{
    // danger !!!! it may cause system halt when cSpinBox destruct
    //windowManager().setFocusedWindow( this );
    _selectedItemId = id;
    if (_selectedItemId >= _items.size())
        return;

    _itemText.setText(_items[_selectedItemId]);

    //s32 textWidth = _items[_selectedItemId].length() * 6;
    //s32 textHeight = 12;
    //_itemText.setRelativePosition( cPoint((_size.x - textWidth) >> 1, (_size.y - textHeight) >> 1) );

    arrangeButton();
    arrangeText();
    arrangeChildren();
    changed(this);
}

void cSpinBox::selectNext()
{
    if (_items.size() - 1 == _selectedItemId)
        return;

    selectItem(_selectedItemId + 1);
}

void cSpinBox::selectPrev()
{
    if (0 == _selectedItemId)
        return;

    selectItem(_selectedItemId - 1);
}

void cSpinBox::insertItem(const std::string &item, u32 position)
{
    if (position > _items.size())
        return;

    _items.insert(_items.begin() + position, item);
}

void cSpinBox::removeItem(u32 position)
{
    if (position > _items.size() - 1)
        return;

    _items.erase(_items.begin() + position);
}

void cSpinBox::setTextColor(COLOR color)
{
}

void cSpinBox::draw()
{
    // draw bar
    u16 barColor = _normalColor;
    if (isActive())
    {
        barColor = _focusedColor;
        _itemText.setTextColor(uiSettings().spinBoxTextHighLightColor);
    }
    else
    {
        _itemText.setTextColor(uiSettings().spinBoxTextColor);
    }

    u8 bodyX1 = _prevButton.position().x + _prevButton.size().x;
    u8 fillWidth = windowRectangle().size().x - _nextButton.size().x - _prevButton.size().x;
    gdi().setPenColor(barColor, _engine);
    gdi().fillRect(barColor, barColor, bodyX1, _position.y, fillWidth, _prevButton.size().y, selectedEngine());
    gdi().setPenColor(_frameColor, _engine);
    gdi().frameRect(bodyX1, _position.y, fillWidth, _prevButton.size().y, uiSettings().thickness, selectedEngine());

    // draw previous button
    _prevButton.draw();

    // draw text
    _itemText.draw();

    // draw next button
    _nextButton.draw();
}

cWindow &cSpinBox::loadAppearance(const std::string &aFileName)
{
    _prevButton.loadAppearance(SFN_SPINBUTTON_L);
    _nextButton.loadAppearance(SFN_SPINBUTTON_R);

    return *this;
}

void cSpinBox::onGainedFocus()
{
}

void cSpinBox::onResize()
{
    dbg_printf("spin box on resize\n");
    arrangeButton();
    arrangeText();
    arrangeChildren();
}

void cSpinBox::onMove()
{
    arrangeButton();
    arrangeText();
    arrangeChildren();
}

void cSpinBox::arrangeText()
{
    s32 textWidth = _items.size() ? font().getStringScreenWidth(_items[_selectedItemId].c_str(), _items[_selectedItemId].length()) : 0;
    s32 textHeight = SYSTEM_FONT_HEIGHT;
    if (textWidth > _itemText.size().x)
        textWidth = _itemText.size().x;

    _itemText.setRelativePosition(cPoint((_size.x - textWidth) >> 1, (_size.y - textHeight) >> 1));
}

void cSpinBox::arrangeButton()
{
    u8 x = 0;
    _prevButton.setSize(cSize(_prevButton.size().x, _size.y));
    _prevButton.setRelativePosition(cPoint(x, (_size.y - _prevButton.size().y) / 2));

    x = _prevButton.size().x;
    _itemText.setRelativePosition(cPoint(x, (_size.y - SYSTEM_FONT_HEIGHT) / 2));

    x = size().x - _nextButton.size().x;
    _nextButton.setSize(cSize(_nextButton.size().x, _size.y));
    _nextButton.setRelativePosition(cPoint(x, (_size.y - _nextButton.size().y)));
}

void cSpinBox::onCmponentClicked()
{
    componentClicked(this);
}

} // namespace akui
