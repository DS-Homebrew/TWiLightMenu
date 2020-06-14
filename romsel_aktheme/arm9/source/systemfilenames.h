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
#ifndef _SYSTEMFILENAMES_H_
#define _SYSTEMFILENAMES_H_
#include <string>
#include "common/singleton.h"


#define SFN_SYSTEM_DIR              "/_nds/TWiLightMenu/akmenu/"


#define SFN_FALLBACK_UI_DIRECTORY  "nitro:/themes/zelda"
#define SFN_SYSTEM_UI_DIRECTORY    SFN_SYSTEM_DIR"themes/"
#define SFN_UI_DIRECTORY           sfn().uiDirectory() + 

#define SFN_UI_CURRENT_DIRECTORY    SFN_UI_DIRECTORY"/"
#define SFN_USER_CUSTOM             SFN_UI_DIRECTORY"/custom.ini"
#define SFN_UI_SETTINGS             SFN_UI_DIRECTORY"/uisettings.ini"
#define SFN_UPPER_SCREEN_BG         SFN_UI_DIRECTORY"/upper_screen.bmp"
#define SFN_LOWER_SCREEN_BG         SFN_UI_DIRECTORY"/lower_screen.bmp"
#define SFN_FORM_TITLE_L            SFN_UI_DIRECTORY"/title_left.bmp"
#define SFN_FORM_TITLE_M            SFN_UI_DIRECTORY"/title_bg.bmp"
#define SFN_FORM_TITLE_R            SFN_UI_DIRECTORY"/title_right.bmp"
#define SFN_BUTTON2                 SFN_UI_DIRECTORY"/btn2.bmp"
#define SFN_BUTTON3                 SFN_UI_DIRECTORY"/btn3.bmp"
#define SFN_BUTTON4                 SFN_UI_DIRECTORY"/btn4.bmp"
#define SFN_SPINBUTTON_L            SFN_UI_DIRECTORY"/spin_btn_left.bmp"
#define SFN_SPINBUTTON_R            SFN_UI_DIRECTORY"/spin_btn_right.bmp"
#define SFN_BRIGHTNESS_BUTTON       SFN_UI_DIRECTORY"/brightness.bmp"
#define SFN_FOLDERUP_BUTTON         SFN_UI_DIRECTORY"/folder_up.bmp"
#define SFN_STARTMENU_BG            SFN_UI_DIRECTORY"/menu_bg.bmp"
#define SFN_CLOCK_NUMBERS           SFN_UI_DIRECTORY"/calendar/clock_numbers.bmp"
#define SFN_CLOCK_COLON             SFN_UI_DIRECTORY"/calendar/clock_colon.bmp"
#define SFN_DAY_NUMBERS             SFN_UI_DIRECTORY"/calendar/day_numbers.bmp"
#define SFN_YEAR_NUMBERS            SFN_UI_DIRECTORY"/calendar/year_numbers.bmp"
#define SFN_WEEKDAY_TEXT            SFN_UI_DIRECTORY"/calendar/weekday_text.bmp"
#define SFN_CARD_ICON_BLUE          SFN_UI_DIRECTORY"/card_icon_blue.bmp"
#define SFN_PROGRESS_WND_BG         SFN_UI_DIRECTORY"/progress_wnd.bmp"
#define SFN_PROGRESS_BAR_BG         SFN_UI_DIRECTORY"/progress_bar.bmp"
#define SFN_GBAFRAME                SFN_UI_DIRECTORY"/gbaframe.bmp"
#define SFN_UI_ICONS_DIRECTORY      SFN_UI_DIRECTORY"/icons/"

#define SFN_BATTERY1                SFN_UI_DIRECTORY"/icons/battery1.bmp"
#define SFN_BATTERY2                SFN_UI_DIRECTORY"/icons/battery2.bmp"
#define SFN_BATTERY3                SFN_UI_DIRECTORY"/icons/battery3.bmp"
#define SFN_BATTERY4                SFN_UI_DIRECTORY"/icons/battery4.bmp"
#define SFN_BATTERY_CHARGE          SFN_UI_DIRECTORY"/icons/batterycharge.bmp"
#define SFN_VOLUME0                 SFN_UI_DIRECTORY"/icons/volume0.bmp"
#define SFN_VOLUME1                 SFN_UI_DIRECTORY"/icons/volume1.bmp"
#define SFN_VOLUME2                 SFN_UI_DIRECTORY"/icons/volume2.bmp"
#define SFN_VOLUME3                 SFN_UI_DIRECTORY"/icons/volume3.bmp"
#define SFN_VOLUME4                 SFN_UI_DIRECTORY"/icons/volume4.bmp"

#define SFN_CLOCK_NUMBERS_2         SFN_UI_DIRECTORY"/calendar/clock_numbers_2.bmp"
#define SFN_CLOCK_COLON_2           SFN_UI_DIRECTORY"/calendar/clock_colon_2.bmp"
#define SFN_DAY_NUMBERS_2           SFN_UI_DIRECTORY"/calendar/day_numbers_2.bmp"
#define SFN_YEAR_NUMBERS_2          SFN_UI_DIRECTORY"/calendar/year_numbers_2.bmp"
#define SFN_WEEKDAY_TEXT_2          SFN_UI_DIRECTORY"/calendar/weekday_text_2.bmp"

#define SFN_LIST_BAR_BG             SFN_UI_DIRECTORY"/selection_bar_bg.bmp"

#define SFN_UI_FONT                 SFN_UI_DIRECTORY"/font.pcf"

#define SFN_LANGUAGE_DIRECTORY      "nitro:/language/"
#define SFN_LANGUAGE_TEXT           SFN_LANGUAGE_DIRECTORY "%s/language.ini"

#define SFN_FONTS_DIRECTORY         "nitro:/fonts/"
#define SFN_DEFAULT_FONT            "tahoma.pcf"
#define SFN_FALLBACK_FONT           "nitro:/fonts/tahoma.pcf"


#define SFN_ICONS_DIRECTORY         SFN_SYSTEM_DIR"icons/"

#define SFN_CHEATS                  "/_nds/TWiLightMenu/extras/usrcheat.dat"
#define SFN_CHEAT_DATA              "/_nds/nds-bootstrap/cheatData.bin"

class SystemFilenames
{
    private:
        std::string _uiDirectory;
    public:
        SystemFilenames() {}
        
        ~SystemFilenames(){}

        void initFilenames();
    public: 
        inline const std::string& uiDirectory() { return _uiDirectory; }
  
};

typedef singleton<SystemFilenames> sysFilenames_s;
inline SystemFilenames &sfn() { return sysFilenames_s::instance(); }
#endif//_SYSTEMFILENAMES_H_
