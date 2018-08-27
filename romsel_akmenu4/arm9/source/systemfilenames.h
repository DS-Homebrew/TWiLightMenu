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

#define UI_LANG "lang_en"

#define SFN_SYSTEM_DIR              "sd:/__rpg/"
#define SFN_OFFICIAL_SAVELIST       SFN_SYSTEM_DIR"savelist.bin"
#define SFN_CUSTOM_SAVELIST         SFN_SYSTEM_DIR"gamedata.bin"
#define SFN_LAST_SAVEINFO           SFN_SYSTEM_DIR"lastsave.ini"
#define SFN_LAST_GBA_SAVEINFO       SFN_SYSTEM_DIR"lastgba.ini"
#define SFN_SDCARD_LIST             SFN_SYSTEM_DIR"sdlist.ini"
#define SFN_GLOBAL_SETTINGS         SFN_SYSTEM_DIR"globalsettings.ini"
#define SFN_FAVORITES               SFN_SYSTEM_DIR"favorites.ini"
#if defined(_STORAGE_r4)
#define SFN_BACKLIGHT               SFN_SYSTEM_DIR"backlight.ini"
#endif

#define SFN_UI_DIRECTORY            SFN_SYSTEM_DIR"ui/zelda"
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
#define SFN_CARD_ICON_BLUE          SFN_UI_DIRECTORY"/card_icon_blue.bmp"
#define SFN_PROGRESS_WND_BG         SFN_UI_DIRECTORY"/progress_wnd.bmp"
#define SFN_PROGRESS_BAR_BG         SFN_UI_DIRECTORY"/progress_bar.bmp"
#define SFN_GBAFRAME                SFN_UI_DIRECTORY"/gbaframe.bmp"
#define SFN_UI_ICONS_DIRECTORY      SFN_UI_DIRECTORY"/icons/"

#define SFN_LANGUAGE_DIRECTORY      SFN_SYSTEM_DIR"language/"
#define SFN_LANGUAGE_TEXT           SFN_LANGUAGE_DIRECTORY UI_LANG"/language.txt"

#define SFN_FONTS_DIRECTORY         SFN_SYSTEM_DIR"fonts/"
#define SFN_DEFAULT_FONT            "liberation.pcf"

#define SFN_ICONS_DIRECTORY         SFN_SYSTEM_DIR"icons/"

#define SFN_CHEATS                  SFN_SYSTEM_DIR"cheats/usrcheat.dat"
#endif//_SYSTEMFILENAMES_H_
