/*
    systemfilenames.h
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
#ifndef _THEMEFILENAMES_H_
#define _THEMEFILENAMES_H_
#include <string>
#include "common/singleton.h"


#define TFN_SYSTEM_DIR              "/_nds/TWiLightMenu/akmenu/"


#define TFN_FALLBACK_UI_DIRECTORY  "nitro:/themes/zelda"
#define TFN_SYSTEM_UI_DIRECTORY    TFN_SYSTEM_DIR"themes/"
#define TFN_UI_DIRECTORY           sfn().uiDirectory() + 

#define TFN_UI_CURRENT_DIRECTORY    TFN_UI_DIRECTORY"/"
#define TFN_USER_CUSTOM             TFN_UI_DIRECTORY"/custom.ini"
#define TFN_UI_SETTINGS             TFN_UI_DIRECTORY"/uisettings.ini"
#define TFN_UPPER_SCREEN_BG         TFN_UI_DIRECTORY"/upper_screen.bmp"
#define TFN_LOWER_SCREEN_BG         TFN_UI_DIRECTORY"/lower_screen.bmp"
#define TFN_FORM_TITLE_L            TFN_UI_DIRECTORY"/title_left.bmp"
#define TFN_FORM_TITLE_M            TFN_UI_DIRECTORY"/title_bg.bmp"
#define TFN_FORM_TITLE_R            TFN_UI_DIRECTORY"/title_right.bmp"
#define TFN_BUTTON2                 TFN_UI_DIRECTORY"/btn2.bmp"
#define TFN_BUTTON3                 TFN_UI_DIRECTORY"/btn3.bmp"
#define TFN_BUTTON4                 TFN_UI_DIRECTORY"/btn4.bmp"
#define TFN_SPINBUTTON_L            TFN_UI_DIRECTORY"/spin_btn_left.bmp"
#define TFN_SPINBUTTON_R            TFN_UI_DIRECTORY"/spin_btn_right.bmp"
#define TFN_BRIGHTNESS_BUTTON       TFN_UI_DIRECTORY"/brightness.bmp"
#define TFN_FOLDERUP_BUTTON         TFN_UI_DIRECTORY"/folder_up.bmp"
#define TFN_STARTMENU_BG            TFN_UI_DIRECTORY"/menu_bg.bmp"
#define TFN_CLOCK_NUMBERS           TFN_UI_DIRECTORY"/calendar/clock_numbers.bmp"
#define TFN_CLOCK_COLON             TFN_UI_DIRECTORY"/calendar/clock_colon.bmp"
#define TFN_DAY_NUMBERS             TFN_UI_DIRECTORY"/calendar/day_numbers.bmp"
#define TFN_YEAR_NUMBERS            TFN_UI_DIRECTORY"/calendar/year_numbers.bmp"
#define TFN_WEEKDAY_TEXT            TFN_UI_DIRECTORY"/calendar/weekday_text.bmp"
#define TFN_CARD_ICON_BLUE          TFN_UI_DIRECTORY"/card_icon_blue.bmp"
#define TFN_PROGRESS_WND_BG         TFN_UI_DIRECTORY"/progress_wnd.bmp"
#define TFN_PROGRESS_BAR_BG         TFN_UI_DIRECTORY"/progress_bar.bmp"
#define TFN_GBAFRAME                TFN_UI_DIRECTORY"/gbaframe.bmp"
#define TFN_UI_ICONS_DIRECTORY      TFN_UI_DIRECTORY"/icons/"

#define TFN_BATTERY1                TFN_UI_DIRECTORY"/icons/battery1.bmp"
#define TFN_BATTERY2                TFN_UI_DIRECTORY"/icons/battery2.bmp"
#define TFN_BATTERY3                TFN_UI_DIRECTORY"/icons/battery3.bmp"
#define TFN_BATTERY4                TFN_UI_DIRECTORY"/icons/battery4.bmp"
#define TFN_BATTERY_CHARGE          TFN_UI_DIRECTORY"/icons/batterycharge.bmp"
#define TFN_VOLUME0                 TFN_UI_DIRECTORY"/icons/volume0.bmp"
#define TFN_VOLUME1                 TFN_UI_DIRECTORY"/icons/volume1.bmp"
#define TFN_VOLUME2                 TFN_UI_DIRECTORY"/icons/volume2.bmp"
#define TFN_VOLUME3                 TFN_UI_DIRECTORY"/icons/volume3.bmp"
#define TFN_VOLUME4                 TFN_UI_DIRECTORY"/icons/volume4.bmp"

#define TFN_CLOCK_NUMBERS_2         TFN_UI_DIRECTORY"/calendar/clock_numbers_2.bmp"
#define TFN_CLOCK_COLON_2           TFN_UI_DIRECTORY"/calendar/clock_colon_2.bmp"
#define TFN_DAY_NUMBERS_2           TFN_UI_DIRECTORY"/calendar/day_numbers_2.bmp"
#define TFN_YEAR_NUMBERS_2          TFN_UI_DIRECTORY"/calendar/year_numbers_2.bmp"
#define TFN_WEEKDAY_TEXT_2          TFN_UI_DIRECTORY"/calendar/weekday_text_2.bmp"

#define TFN_LIST_BAR_BG             TFN_UI_DIRECTORY"/selection_bar_bg.bmp"

#define TFN_LANGUAGE_DIRECTORY      "nitro:/language/"
#define TFN_LANGUAGE_TEXT           TFN_LANGUAGE_DIRECTORY "%s/language.txt"

#define TFN_FONTS_DIRECTORY         "nitro:/fonts/"
#define TFN_DEFAULT_FONT            "tahoma.pcf"
#define TFN_FALLBACK_FONT           "nitro:/fonts/tahoma.pcf"


#define TFN_ICONS_DIRECTORY         TFN_SYSTEM_DIR"icons/"

#define TFN_CHEATS                  "/_nds/TWiLightMenu/extras/usrcheat.dat"

class ThemeFilenames
{
    private:
        std::string _uiDirectory;
    public:
        ThemeFilenames();
        
        ~ThemeFilenames(){}
    public: 
        inline const std::string& uiDirectory() { return _uiDirectory; }
  
};

typedef singleton<ThemeFilenames> sysFilenames_s;
inline ThemeFilenames &tfn() { return sysFilenames_s::instance(); }
#endif//_THEMEFILENAMES_H_