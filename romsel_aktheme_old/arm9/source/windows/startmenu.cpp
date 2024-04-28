/*
    startmenu.cpp
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

#include "startmenu.h"
#include "ui/windowmanager.h"
#include "systemfilenames.h"
#include "common/inifile.h"
#include "language.h"

using namespace akui;

void StartMenu::init()
{
    CIniFile ini(SFN_UI_SETTINGS);
    if (ini.GetInt("start menu", "showFileOperations", true))
    {
        addItem(START_MENU_ITEM_COPY, LANG("start menu", "Copy"));
        addItem(START_MENU_ITEM_CUT, LANG("start menu", "Cut"));
        addItem(START_MENU_ITEM_PASTE, LANG("start menu", "Paste"));
        addItem(START_MENU_ITEM_DELETE, LANG("start menu", "Delete"));
        addItem(START_MENU_ITEM_HIDE, LANG("start menu", "Hide"));
        //addItem(START_MENU_ITEM_HELP, LANG("start menu", "Help"));

        addItem(START_MENU_ITEM_SETTING, LANG("start menu", "Setting"));
        addItem(START_MENU_ITEM_INFO, LANG("start menu", "Info"));
        // addItem(START_MENU_ITEM_TOOLS, LANG("start menu", "Tools"));
    }
    else
    {
        addItem(START_MENU_ITEM_SETTING, LANG("start menu", "Setting"));
        addItem(START_MENU_ITEM_INFO, LANG("start menu", "Info"));
    }
    loadAppearance(SFN_UI_SETTINGS);
    dbg_printf("startmenu ok\n");
}

bool StartMenu::process(const Message &msg)
{
    if (msg.id() == Message::keyDown)
    {
        KeyMessage &kmsg = (KeyMessage &)msg;
        if (kmsg.keyCode() == KeyMessage::UI_KEY_START)
        {
            hide();
            return false;
        }
    }
    return PopMenu::process(msg);
}

Window &StartMenu::loadAppearance(const std::string &aFileName)
{
    _renderDesc->loadData(SFN_STARTMENU_BG);
    _size = _renderDesc->size();

    CIniFile ini(aFileName);
    //std::string bgFile = ini.GetString( "bg", "file",  );
    int ix = ini.GetInt("start menu", "itemX", 4);
    int iy = ini.GetInt("start menu", "itemY", 12);
    int x = ini.GetInt("start menu", "x", 4);
    int y = ini.GetInt("start menu", "y", 4);
    setPosition(Point(x, y));
    _itemTopLeftPoint = Point(ix, iy);
    _itemHeight = ini.GetInt("start menu", "itemHeight", 16);
    _itemWidth = ini.GetInt("start menu", "itemWidth", 0);
    _barLeft = ini.GetInt("start menu", "barLeft", 2);
    if (_itemWidth == 0 && _barLeft * 2 > _size.x)
        _barLeft = 0;
    return *this;
}
