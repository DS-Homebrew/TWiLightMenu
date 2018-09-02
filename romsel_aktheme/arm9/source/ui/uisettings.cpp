/*
    uisettings.cpp
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
#include "uisettings.h"
#include "common/inifile.h"

UISettings::UISettings()
{
    showCalendar = true;
    formFrameColor = RGB15(23,25,4);
    formBodyColor = RGB15(30,29,22);
    formTextColor = RGB15(17,12,0);
    formTitleTextColor = RGB15(11,11,11);
    buttonTextColor = RGB15(17,12,0);
    spinBoxNormalColor = RGB15(0,0,31);
    spinBoxFocusColor = RGB15(0,31,0);
    spinBoxTextColor = RGB15(31,31,31);
    spinBoxTextHighLightColor = RGB15(31,31,31);
    spinBoxFrameColor = RGB15(11,11,11);
    listViewBarColor1 = RGB15(0,11,19);
    listViewBarColor2 = RGB15(0,5,9);
    listTextColor = 0;
    listTextHighLightColor = 0;
    popMenuTextColor = RGB15(0,0,0);
    popMenuTextHighLightColor = RGB15(31,31,31);
    popMenuBarColor = RGB15(0,11,19);
    thickness = 1;
}

UISettings::~UISettings()
{
}

void UISettings::loadSettings()
{
    CIniFile ini( SFN_UI_SETTINGS );

    showCalendar = ini.GetInt( "global settings", "showCalendar", showCalendar );
    formFrameColor = ini.GetInt( "global settings", "formFrameColor", formFrameColor );
    formBodyColor = ini.GetInt( "global settings", "formBodyColor", formBodyColor );
    formTextColor = ini.GetInt( "global settings", "formTextColor", formTextColor );
    formTitleTextColor = ini.GetInt( "global settings", "formTitleTextColor", formTitleTextColor );
    buttonTextColor = ini.GetInt( "global settings", "buttonTextColor", buttonTextColor );
    spinBoxNormalColor = ini.GetInt( "global settings", "spinBoxNormalColor", spinBoxNormalColor );
    spinBoxFocusColor = ini.GetInt( "global settings", "spinBoxFocusColor", spinBoxFocusColor );
    spinBoxTextColor = ini.GetInt( "global settings", "spinBoxTextColor", spinBoxTextColor );
    spinBoxTextHighLightColor = ini.GetInt( "global settings", "spinBoxTextHiLightColor", spinBoxTextHighLightColor );
    spinBoxFrameColor = ini.GetInt( "global settings", "spinBoxFrameColor", spinBoxFrameColor );
    listViewBarColor1 = ini.GetInt( "global settings", "listViewBarColor1", listViewBarColor1 );
    listViewBarColor2 = ini.GetInt( "global settings", "listViewBarColor2", listViewBarColor2 );
    listTextColor = ini.GetInt( "global settings", "listTextColor", listTextColor );
    listTextHighLightColor = ini.GetInt( "global settings", "listTextHighLightColor", listTextHighLightColor );
    popMenuTextColor = ini.GetInt( "global settings", "popMenuTextColor", popMenuTextColor );
    popMenuTextHighLightColor = ini.GetInt( "global settings", "popMenuTextHighLightColor", popMenuTextHighLightColor );
    popMenuBarColor = ini.GetInt( "global settings", "popMenuBarColor", popMenuBarColor );
    thickness = ini.GetInt( "global settings", "thickness", thickness );
}
