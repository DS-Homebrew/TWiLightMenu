/*
    button.h
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

#ifndef _AKUI_BUTTON_H_
#define _AKUI_BUTTON_H_

#include "rectangle.h"
#include "window.h"
#include "renderdesc.h"
#include "drawing/bmp15.h"

namespace akui
{

class ButtonDesc;

class Button : public Window
{

  public:
    enum State
    {
        up = 0,
        down = 1
    };

    enum Style
    {
        single = 0,
        press = 1,
        toggle = 2
    };

    enum Alignment
    {
        left,
        center,
        right
    };

    Button(s32 x, s32 y, u32 w, u32 h, Window *parent, const std::string &text);

    ~Button();

  public:
    void draw();

    Window &loadAppearance(const std::string &aFileName);

    bool process(const Message &msg);

    State state() { return _state; }

    void setTextColor(COLOR color) { _textColor = color; }

    COLOR textColor() { return _textColor; }

    void setStyle(Style style) { _style = style; }

    Style style() { return _style; }

    void setAlignment(Alignment alignment) { _alignment = alignment; }

    Alignment alignment() { return _alignment; }

    void onPressed();

    void onReleased();

    void onClicked();

    Signal0 clicked;

    Signal0 pressed;

  protected:
    bool processTouchMessage(const akui::TouchMessage &msg);

    bool _captured;

    State _state;

    COLOR _textColor;

    ButtonDesc *_renderDesc;

    Style _style;

    Alignment _alignment;
};

// form desc��ֻ���𻭱���
class ButtonDesc : public RenderDesc
{
  public:
    ButtonDesc();

    ~ButtonDesc();

  public:
    ButtonDesc &setButton(Button *button)
    {
        _button = button;
        return *this;
    }

    void draw(const Rect &area, GRAPHICS_ENGINE engine) const;

    void loadData(const std::string &filename);

  protected:
    Button *_button;
    BMP15 _background;
    COLOR _textColor;
};
} // namespace akui

#endif //_AKUI_BUTTON_H_
