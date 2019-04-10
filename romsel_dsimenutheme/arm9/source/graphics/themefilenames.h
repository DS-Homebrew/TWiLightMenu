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


#define TFN_SYSTEM_DIR              "/_nds/TWiLightMenu/%smenu/"


#define TFN_FALLBACK_DSI_UI_DIRECTORY  "nitro:/themes/dsi/dark"
#define TFN_FALLBACK_3DS_UI_DIRECTORY  "nitro:/themes/3ds/light"

#define TFN_SYSTEM_UI_DIRECTORY       TFN_SYSTEM_DIR"themes/"
#define TFN_SYSTEM_SOUND_DIRECTORY    "nitro:/sound"

#define TFN_UI_DIRECTORY              tfn().uiDirectory() + 
#define TFN_FALLBACK_UI_DIRECTORY     tfn().fallbackDirectory() + 

#define TFN_UI_CURRENT_DIRECTORY    TFN_UI_DIRECTORY"/"
#define TFN_THEME_SETTINGS          TFN_UI_DIRECTORY"/theme.ini"

#define TFN_BG_TOPBG                TFN_UI_DIRECTORY"/background/top.grf"
#define TFN_BG_BOTTOMBG             TFN_UI_DIRECTORY"/background/bottom.grf"
#define TFN_BG_BOTTOMBUBBLEBG       TFN_UI_DIRECTORY"/background/bottom_bubble.grf"
#define TFN_BG_BOTTOMBG_DS          TFN_UI_DIRECTORY"/background/bottom_ds.grf"
#define TFN_BG_BOTTOMBUBBLEBG_DS    TFN_UI_DIRECTORY"/background/bottom_bubble_ds.grf"
#define TFN_BG_BOTTOMMOVINGBG       TFN_UI_DIRECTORY"/background/bottom_moving.grf"
#define TFN_BG_BOTTOMMOVING_LBG      TFN_UI_DIRECTORY"/background/bottom_moving_l.grf"
#define TFN_BG_BOTTOMMOVING_RBG      TFN_UI_DIRECTORY"/background/bottom_moving_r.grf"

#define TFN_UI_DATE_TIME_FONT       TFN_UI_DIRECTORY"/ui/date_time_font.bmp"
#define TFN_UI_LSHOULDER            TFN_UI_DIRECTORY"/ui/Lshoulder.bmp"
#define TFN_UI_LSHOULDER_GREYED     TFN_UI_DIRECTORY"/ui/Lshoulder_greyed.bmp"
#define TFN_UI_RSHOULDER            TFN_UI_DIRECTORY"/ui/Rshoulder.bmp"
#define TFN_UI_RSHOULDER_GREYED     TFN_UI_DIRECTORY"/ui/Rshoulder_greyed.bmp"


#define TFN_BATTERY0                TFN_UI_DIRECTORY"/battery/battery0.bmp"
#define TFN_BATTERY1                TFN_UI_DIRECTORY"/battery/battery1.bmp"
#define TFN_BATTERY2                TFN_UI_DIRECTORY"/battery/battery2.bmp"
#define TFN_BATTERY3                TFN_UI_DIRECTORY"/battery/battery3.bmp"
#define TFN_BATTERY4                TFN_UI_DIRECTORY"/battery/battery4.bmp"
#define TFN_BATTERY_CHARGE          TFN_UI_DIRECTORY"/battery/batterycharge.bmp"
#define TFN_BATTERY_CHARGE_BLINK    TFN_UI_DIRECTORY"/battery/batterychargeblink.bmp"
#define TFN_BATTERY_FULL            TFN_UI_DIRECTORY"/battery/batteryfull.bmp"
#define TFN_BATTERY_FULLDS          TFN_UI_DIRECTORY"/battery/batteryfullDS.bmp"
#define TFN_BATTERY_LOW             TFN_UI_DIRECTORY"/battery/batterylow.bmp"

#define TFN_VOLUME0                 TFN_UI_DIRECTORY"/volume/volume0.bmp"
#define TFN_VOLUME1                 TFN_UI_DIRECTORY"/volume/volume1.bmp"
#define TFN_VOLUME2                 TFN_UI_DIRECTORY"/volume/volume2.bmp"
#define TFN_VOLUME3                 TFN_UI_DIRECTORY"/volume/volume3.bmp"
#define TFN_VOLUME4                 TFN_UI_DIRECTORY"/volume/volume4.bmp"

#define TFN_GRF_BIPS                TFN_UI_DIRECTORY"/grf/bips.grf"
#define TFN_GRF_BOX                 TFN_UI_DIRECTORY"/grf/box.grf"

#define TFN_GRF_BOX_FULL            TFN_UI_DIRECTORY"/grf/box_full.grf"
#define TFN_GRF_BOX_EMPTY           TFN_UI_DIRECTORY"/grf/box_empty.grf"
#define TFN_GRF_CURSOR              TFN_UI_DIRECTORY"/grf/cursor.grf"

#define TFN_GRF_BRACE               TFN_UI_DIRECTORY"/grf/brace.grf"
#define TFN_GRF_BUBBLE              TFN_UI_DIRECTORY"/grf/bubble.grf"
#define TFN_GRF_BUTTON_ARROW        TFN_UI_DIRECTORY"/grf/button_arrow.grf"
#define TFN_GRF_CORNERBUTTON        TFN_UI_DIRECTORY"/grf/cornerbutton.grf"
#define TFN_GRF_DIALOGBOX           TFN_UI_DIRECTORY"/grf/dialogbox.grf"
#define TFN_GRF_FOLDER              TFN_UI_DIRECTORY"/grf/folder.grf"
#define TFN_GRF_ICON_GB             TFN_UI_DIRECTORY"/grf/icon_gb.grf"
#define TFN_GRF_ICON_GBA            TFN_UI_DIRECTORY"/grf/icon_gba.grf"
#define TFN_GRF_ICON_GBAMODE        TFN_UI_DIRECTORY"/grf/icon_gbamode.grf"
#define TFN_GRF_ICON_GG             TFN_UI_DIRECTORY"/grf/icon_gg.grf"
#define TFN_GRF_ICON_MD             TFN_UI_DIRECTORY"/grf/icon_md.grf"
#define TFN_GRF_ICON_NES            TFN_UI_DIRECTORY"/grf/icon_nes.grf"
#define TFN_GRF_ICON_SETTINGS       TFN_UI_DIRECTORY"/grf/icon_settings.grf"
#define TFN_GRF_ICON_MANUAL         TFN_UI_DIRECTORY"/grf/icon_manual.grf"
#define TFN_GRF_ICON_SMS            TFN_UI_DIRECTORY"/grf/icon_sms.grf"
#define TFN_GRF_ICON_SNES           TFN_UI_DIRECTORY"/grf/icon_snes.grf"
#define TFN_GRF_ICON_UNK            TFN_UI_DIRECTORY"/grf/icon_unk.grf"
#define TFN_GRF_LAUNCH_DOT          TFN_UI_DIRECTORY"/grf/launch_dot.grf"
#define TFN_GRF_MOVING_ARROW        TFN_UI_DIRECTORY"/grf/moving_arrow.grf"
#define TFN_GRF_PROGRESS            TFN_UI_DIRECTORY"/grf/progress.grf"
#define TFN_GRF_SCROLL_WINDOW       TFN_UI_DIRECTORY"/grf/scroll_window.grf"
#define TFN_GRF_SMALL_CART          TFN_UI_DIRECTORY"/grf/small_cart.grf"
#define TFN_GRF_START_BORDER        TFN_UI_DIRECTORY"/grf/start_border.grf"
#define TFN_GRF_START_TEXT          TFN_UI_DIRECTORY"/grf/start_text.grf"
#define TFN_GRF_WIRELESSICONS       TFN_UI_DIRECTORY"/grf/wirelessicons.grf"
#define TFN_RVID_CUBES              TFN_UI_DIRECTORY"/video/3dsRotatingCubes.rvid"

#define TFN_SOUND_EFFECTBANK        TFN_UI_DIRECTORY"/sound/sfx.bin"
#define TFN_SOUND_BG                TFN_UI_DIRECTORY"/sound/bgm.pcm.raw"

#define TFN_DEFAULT_SOUND_EFFECTBANK      TFN_SYSTEM_SOUND_DIRECTORY"/defaultfx.bin"
#define TFN_DEFAULT_SOUND_BG              TFN_SYSTEM_SOUND_DIRECTORY"/defaultbg.pcm.raw"

#define TFN_SHOP_SOUND_EFFECTBANK         TFN_SYSTEM_SOUND_DIRECTORY"/shopfx.bin"
#define TFN_SHOP_SOUND_BG                 TFN_SYSTEM_SOUND_DIRECTORY"/shopbg.pcm.raw"

// We need fallbacks, because unlike akmenu, a bad GRF file could crash the theme
// Leaving the user with no way to fix it well.

#define TFN_FALLBACK_UI_CURRENT_DIRECTORY    TFN_FALLBACK_UI_DIRECTORY"/"
#define TFN_FALLBACK_THEME_SETTINGS          TFN_FALLBACK_UI_DIRECTORY"/theme.ini"

#define TFN_FALLBACK_BG_TOPBG                TFN_FALLBACK_UI_DIRECTORY"/background/top.grf"
#define TFN_FALLBACK_BG_BOTTOMBG             TFN_FALLBACK_UI_DIRECTORY"/background/bottom.grf"
#define TFN_FALLBACK_BG_BOTTOMBUBBLEBG       TFN_FALLBACK_UI_DIRECTORY"/background/bottom_bubble.grf"
#define TFN_FALLBACK_BG_BOTTOMMOVINGBG       TFN_FALLBACK_UI_DIRECTORY"/background/bottom_moving.grf"
#define TFN_FALLBACK_BG_BOTTOMMOVING_LBG     TFN_FALLBACK_UI_DIRECTORY"/background/bottom_moving_l.grf"
#define TFN_FALLBACK_BG_BOTTOMMOVING_RBG     TFN_FALLBACK_UI_DIRECTORY"/background/bottom_moving_r.grf"

#define TFN_FALLBACK_BG_BOTTOMBG_DS          TFN_FALLBACK_UI_DIRECTORY"/background/bottom_ds.grf"
#define TFN_FALLBACK_BG_BOTTOMBUBBLEBG_DS    TFN_FALLBACK_UI_DIRECTORY"/background/bottom_bubble_ds.grf"

#define TFN_FALLBACK_UI_DATE_TIME_FONT       TFN_FALLBACK_UI_DIRECTORY"/ui/date_time_font.bmp"
#define TFN_FALLBACK_UI_LSHOULDER            TFN_FALLBACK_UI_DIRECTORY"/ui/Lshoulder.bmp"
#define TFN_FALLBACK_UI_LSHOULDER_GREYED     TFN_FALLBACK_UI_DIRECTORY"/ui/Lshoulder_greyed.bmp"
#define TFN_FALLBACK_UI_RSHOULDER            TFN_FALLBACK_UI_DIRECTORY"/ui/Rshoulder.bmp"
#define TFN_FALLBACK_UI_RSHOULDER_GREYED     TFN_FALLBACK_UI_DIRECTORY"/ui/Rshoulder_greyed.bmp"

#define TFN_FALLBACK_BATTERY0                TFN_FALLBACK_UI_DIRECTORY"/battery/battery0.bmp"
#define TFN_FALLBACK_BATTERY1                TFN_FALLBACK_UI_DIRECTORY"/battery/battery1.bmp"
#define TFN_FALLBACK_BATTERY2                TFN_FALLBACK_UI_DIRECTORY"/battery/battery2.bmp"
#define TFN_FALLBACK_BATTERY3                TFN_FALLBACK_UI_DIRECTORY"/battery/battery3.bmp"
#define TFN_FALLBACK_BATTERY4                TFN_FALLBACK_UI_DIRECTORY"/battery/battery4.bmp"
#define TFN_FALLBACK_BATTERY_CHARGE          TFN_FALLBACK_UI_DIRECTORY"/battery/batterycharge.bmp"
#define TFN_FALLBACK_BATTERY_CHARGE_BLINK    TFN_FALLBACK_UI_DIRECTORY"/battery/batterychargeblink.bmp"
#define TFN_FALLBACK_BATTERY_FULL            TFN_FALLBACK_UI_DIRECTORY"/battery/batteryfull.bmp"
#define TFN_FALLBACK_BATTERY_FULLDS          TFN_FALLBACK_UI_DIRECTORY"/battery/batteryfullDS.bmp"
#define TFN_FALLBACK_BATTERY_LOW             TFN_FALLBACK_UI_DIRECTORY"/battery/batterylow.bmp"

#define TFN_FALLBACK_VOLUME0                 TFN_FALLBACK_UI_DIRECTORY"/volume/volume0.bmp"
#define TFN_FALLBACK_VOLUME1                 TFN_FALLBACK_UI_DIRECTORY"/volume/volume1.bmp"
#define TFN_FALLBACK_VOLUME2                 TFN_FALLBACK_UI_DIRECTORY"/volume/volume2.bmp"
#define TFN_FALLBACK_VOLUME3                 TFN_FALLBACK_UI_DIRECTORY"/volume/volume3.bmp"
#define TFN_FALLBACK_VOLUME4                 TFN_FALLBACK_UI_DIRECTORY"/volume/volume4.bmp"

#define TFN_FALLBACK_GRF_BIPS                TFN_FALLBACK_UI_DIRECTORY"/grf/bips.grf"
#define TFN_FALLBACK_GRF_BOX                 TFN_FALLBACK_UI_DIRECTORY"/grf/box.grf"
#define TFN_FALLBACK_GRF_BRACE               TFN_FALLBACK_UI_DIRECTORY"/grf/brace.grf"
#define TFN_FALLBACK_GRF_BUBBLE              TFN_FALLBACK_UI_DIRECTORY"/grf/bubble.grf"
#define TFN_FALLBACK_GRF_BUTTON_ARROW        TFN_FALLBACK_UI_DIRECTORY"/grf/button_arrow.grf"
#define TFN_FALLBACK_GRF_CORNERBUTTON        TFN_FALLBACK_UI_DIRECTORY"/grf/cornerbutton.grf"
#define TFN_FALLBACK_GRF_DIALOGBOX           TFN_FALLBACK_UI_DIRECTORY"/grf/dialogbox.grf"
#define TFN_FALLBACK_GRF_FOLDER              TFN_FALLBACK_UI_DIRECTORY"/grf/folder.grf"
#define TFN_FALLBACK_GRF_ICON_GB             TFN_FALLBACK_UI_DIRECTORY"/grf/icon_gb.grf"
#define TFN_FALLBACK_GRF_ICON_GBA            TFN_FALLBACK_UI_DIRECTORY"/grf/icon_gba.grf"
#define TFN_FALLBACK_GRF_ICON_GBAMODE        TFN_FALLBACK_UI_DIRECTORY"/grf/icon_gbamode.grf"
#define TFN_FALLBACK_GRF_ICON_GG             TFN_FALLBACK_UI_DIRECTORY"/grf/icon_gg.grf"
#define TFN_FALLBACK_GRF_ICON_MD             TFN_FALLBACK_UI_DIRECTORY"/grf/icon_md.grf"
#define TFN_FALLBACK_GRF_ICON_NES            TFN_FALLBACK_UI_DIRECTORY"/grf/icon_nes.grf"
#define TFN_FALLBACK_GRF_ICON_SETTINGS       TFN_FALLBACK_UI_DIRECTORY"/grf/icon_settings.grf"
#define TFN_FALLBACK_GRF_ICON_MANUAL         TFN_FALLBACK_UI_DIRECTORY"/grf/icon_manual.grf"
#define TFN_FALLBACK_GRF_ICON_SMS            TFN_FALLBACK_UI_DIRECTORY"/grf/icon_sms.grf"
#define TFN_FALLBACK_GRF_ICON_SNES           TFN_FALLBACK_UI_DIRECTORY"/grf/icon_snes.grf"
#define TFN_FALLBACK_GRF_ICON_UNK            TFN_FALLBACK_UI_DIRECTORY"/grf/icon_unk.grf"
#define TFN_FALLBACK_GRF_LAUNCH_DOT          TFN_FALLBACK_UI_DIRECTORY"/grf/launch_dot.grf"
#define TFN_FALLBACK_GRF_MOVING_ARROW        TFN_FALLBACK_UI_DIRECTORY"/grf/moving_arrow.grf"
#define TFN_FALLBACK_GRF_PROGRESS            TFN_FALLBACK_UI_DIRECTORY"/grf/progress.grf"
#define TFN_FALLBACK_GRF_SCROLL_WINDOW       TFN_FALLBACK_UI_DIRECTORY"/grf/scroll_window.grf"
#define TFN_FALLBACK_GRF_SMALL_CART          TFN_FALLBACK_UI_DIRECTORY"/grf/small_cart.grf"
#define TFN_FALLBACK_GRF_START_BORDER        TFN_FALLBACK_UI_DIRECTORY"/grf/start_border.grf"
#define TFN_FALLBACK_GRF_START_TEXT          TFN_FALLBACK_UI_DIRECTORY"/grf/start_text.grf"
#define TFN_FALLBACK_GRF_WIRELESSICONS       TFN_FALLBACK_UI_DIRECTORY"/grf/wirelessicons.grf"

#define TFN_FALLBACK_RVID_CUBES              TFN_FALLBACK_UI_DIRECTORY"/video/3dsRotatingCubes.rvid"

#define TFN_FALLBACK_GRF_BOX_FULL            TFN_FALLBACK_UI_DIRECTORY"/grf/box_full.grf"
#define TFN_FALLBACK_GRF_BOX_EMPTY           TFN_FALLBACK_UI_DIRECTORY"/grf/box_empty.grf"
#define TFN_FALLBACK_GRF_CURSOR              TFN_FALLBACK_UI_DIRECTORY"/grf/cursor.grf"


class ThemeFilenames
{
    private:
        std::string _uiDirectory;
        std::string _fallbackDirectory;
    public:
        ThemeFilenames();
        ~ThemeFilenames(){}
    public: 
        inline const std::string& uiDirectory() { return _uiDirectory; }
        inline const std::string& fallbackDirectory() { return _fallbackDirectory; }

  
};

typedef singleton<ThemeFilenames> themeFilenames_s;
inline ThemeFilenames &tfn() { return themeFilenames_s::instance(); }
#endif//_THEMEFILENAMES_H_