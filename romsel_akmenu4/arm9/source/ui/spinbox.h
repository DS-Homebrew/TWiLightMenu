/*
    spinbox.h
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

#ifndef _SPINBOX_H_
#define _SPINBOX_H_

#include "form.h"
#include "button.h"
#include "statictext.h"
#include <vector>
#include <string>

namespace akui
{

class SpinBox : public Form
{

  public:
    SpinBox(s32 x, s32 y, u32 w, u32 h, Window *parent, const std::string &text);
    ~SpinBox();

  public:
    void draw();

    Window &loadAppearance(const std::string &aFileName);

    u32 selectedItemId() { return _selectedItemId; }

    void selectItem(u32 id);

    void selectNext();

    void selectPrev();

    void insertItem(const std::string &item, u32 position);

    void removeItem(u32 position);

    void setTextColor(COLOR color);

    void onCmponentClicked();
    Signal1<SpinBox *> componentClicked;
    Signal1<SpinBox *> changed;

  protected:
    void onResize();

    void onMove();

    void onGainedFocus();

    void arrangeButton();

    void arrangeText();

  protected:
    COLOR _focusedColor;
    COLOR _normalColor;
    COLOR _frameColor;
    Button _prevButton;
    Button _nextButton;
    StaticText _itemText;
    u32 _selectedItemId;
    std::vector<std::string> _items;
};

} // namespace akui

#endif //_SPINBOX_H_
