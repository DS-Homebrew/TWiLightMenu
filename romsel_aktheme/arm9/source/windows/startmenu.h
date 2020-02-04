/*
    startmenu.h
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

#pragma once
#ifndef _STARTMENU_H_
#define _STARTMENU_H_

#include "ui/popmenu.h"

#define START_MENU_ITEM_COPY 0
#define START_MENU_ITEM_CUT 1
#define START_MENU_ITEM_DELETE 2
#define START_MENU_ITEM_HIDE 3
#define START_MENU_ITEM_PASTE 4
//#define START_MENU_ITEM_HELP 6

#define START_MENU_ITEM_SETTING 5
#define START_MENU_ITEM_INFO 6
// #define START_MENU_ITEM_TOOLS 2

class StartMenu : public akui::PopMenu
{
  public:
    StartMenu(s32 x, s32 y, u32 w, u32 h, Window *parent, const std::string &text) : PopMenu(x, y, w, h, parent, text)
    {
    }

    ~StartMenu() {}

    void init();

    bool process(const akui::Message &msg);

    Window &loadAppearance(const std::string &aFileName);
};

#endif //_STARTMENU_H_
