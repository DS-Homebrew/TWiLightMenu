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


#define TFN_FALLBACK_DSI_UI_DIRECTORY         "nitro:/themes/dsi/dark"
#define TFN_FALLBACK_3DS_UI_DIRECTORY         "nitro:/themes/3ds/light"
#define TFN_FALLBACK_SATURN_UI_DIRECTORY      "nitro:/themes/saturn/default"
#define TFN_FALLBACK_HBLAUNCHER_UI_DIRECTORY  "nitro:/themes/hbLauncher/default"

#define TFN_SYSTEM_UI_DIRECTORY             TFN_SYSTEM_DIR"themes/"
#define TFN_SYSTEM_SOUND_DIRECTORY          "nitro:/sound"
#define TFN_SYSTEM_SOUND_CACHE_DIRECTORY    "/_nds/TWiLightMenu/cache/music"

#define TFN_UI_DIRECTORY              tfn().uiDirectory() + 
#define TFN_FALLBACK_UI_DIRECTORY     tfn().fallbackDirectory() + 

#define TFN_UI_CURRENT_DIRECTORY    TFN_UI_DIRECTORY"/"
#define TFN_THEME_SETTINGS          TFN_UI_DIRECTORY"/theme.ini"

#define TFN_BG_TOPBG                TFN_UI_DIRECTORY"/background/top"
#define TFN_BG_BOTTOMBG             TFN_UI_DIRECTORY"/background/bottom"
#define TFN_BG_BOTTOMBG_MACRO       TFN_UI_DIRECTORY"/background/bottom_macro"
#define TFN_BG_BOTTOMBUBBLEBG       TFN_UI_DIRECTORY"/background/bottom_bubble"
#define TFN_BG_BOTTOMBUBBLEBG_MACRO TFN_UI_DIRECTORY"/background/bottom_bubble_macro"
#define TFN_BG_BOTTOMBG_DS          TFN_UI_DIRECTORY"/background/bottom_ds"
#define TFN_BG_BOTTOMBUBBLEBG_DS    TFN_UI_DIRECTORY"/background/bottom_bubble_ds"
#define TFN_BG_BOTTOMMOVINGBG       TFN_UI_DIRECTORY"/background/bottom_moving"
#define TFN_BG_BOTTOMMOVINGBG_MACRO TFN_UI_DIRECTORY"/background/bottom_moving_macro"
#define TFN_BG_BOTTOMMOVING_LBG     TFN_UI_DIRECTORY"/background/bottom_moving_l"
#define TFN_BG_BOTTOMMOVING_RBG     TFN_UI_DIRECTORY"/background/bottom_moving_r"

#define TFN_UI_LSHOULDER            TFN_UI_DIRECTORY"/ui/Lshoulder"
#define TFN_UI_LSHOULDER_GREYED     TFN_UI_DIRECTORY"/ui/Lshoulder_greyed"
#define TFN_UI_RSHOULDER            TFN_UI_DIRECTORY"/ui/Rshoulder"
#define TFN_UI_RSHOULDER_GREYED     TFN_UI_DIRECTORY"/ui/Rshoulder_greyed"

#define TFN_BATTERY0                TFN_UI_DIRECTORY"/battery/battery0"
#define TFN_BATTERY1                TFN_UI_DIRECTORY"/battery/battery1"
#define TFN_BATTERY2                TFN_UI_DIRECTORY"/battery/battery2"
#define TFN_BATTERY3                TFN_UI_DIRECTORY"/battery/battery3"
#define TFN_BATTERY4                TFN_UI_DIRECTORY"/battery/battery4"
#define TFN_BATTERY1_PURPLE         TFN_UI_DIRECTORY"/battery/battery1purple"
#define TFN_BATTERY2_PURPLE         TFN_UI_DIRECTORY"/battery/battery2purple"
#define TFN_BATTERY3_PURPLE         TFN_UI_DIRECTORY"/battery/battery3purple"
#define TFN_BATTERY4_PURPLE         TFN_UI_DIRECTORY"/battery/battery4purple"
#define TFN_BATTERY_CHARGE          TFN_UI_DIRECTORY"/battery/batterycharge"
#define TFN_BATTERY_CHARGE_BLINK    TFN_UI_DIRECTORY"/battery/batterychargeblink"
#define TFN_BATTERY_FULL            TFN_UI_DIRECTORY"/battery/batteryfull"
#define TFN_BATTERY_FULLDS          TFN_UI_DIRECTORY"/battery/batteryfullDS"
#define TFN_BATTERY_LOW             TFN_UI_DIRECTORY"/battery/batterylow"

#define TFN_VOLUME0                 TFN_UI_DIRECTORY"/volume/volume0"
#define TFN_VOLUME1                 TFN_UI_DIRECTORY"/volume/volume1"
#define TFN_VOLUME2                 TFN_UI_DIRECTORY"/volume/volume2"
#define TFN_VOLUME3                 TFN_UI_DIRECTORY"/volume/volume3"
#define TFN_VOLUME4                 TFN_UI_DIRECTORY"/volume/volume4"

#define TFN_GRF_BIPS                TFN_UI_DIRECTORY"/grf/bips"
#define TFN_GRF_BOX                 TFN_UI_DIRECTORY"/grf/box"

#define TFN_GRF_BOX_FULL            TFN_UI_DIRECTORY"/grf/box_full"
#define TFN_GRF_BOX_EMPTY           TFN_UI_DIRECTORY"/grf/box_empty"
#define TFN_GRF_CURSOR              TFN_UI_DIRECTORY"/grf/cursor"

#define TFN_GRF_BRACE               TFN_UI_DIRECTORY"/grf/brace"
#define TFN_GRF_BUBBLE              TFN_UI_DIRECTORY"/grf/bubble"
#define TFN_GRF_BUTTON_ARROW        TFN_UI_DIRECTORY"/grf/button_arrow"
#define TFN_GRF_CORNERBUTTON        TFN_UI_DIRECTORY"/grf/cornerbutton"
#define TFN_GRF_DIALOGBOX           TFN_UI_DIRECTORY"/grf/dialogbox"
#define TFN_GRF_FOLDER              TFN_UI_DIRECTORY"/grf/folder"
#define TFN_GRF_ICON_GB             TFN_UI_DIRECTORY"/grf/icon_gb"
#define TFN_GRF_ICON_GBA            TFN_UI_DIRECTORY"/grf/icon_gba"
#define TFN_GRF_ICON_GBAMODE        TFN_UI_DIRECTORY"/grf/icon_gbamode"
#define TFN_GRF_ICON_GG             TFN_UI_DIRECTORY"/grf/icon_gg"
#define TFN_GRF_ICON_MD             TFN_UI_DIRECTORY"/grf/icon_md"
#define TFN_GRF_ICON_NES            TFN_UI_DIRECTORY"/grf/icon_nes"
#define TFN_GRF_ICON_SETTINGS       TFN_UI_DIRECTORY"/grf/icon_settings"
#define TFN_GRF_ICON_MANUAL         TFN_UI_DIRECTORY"/grf/icon_manual"
#define TFN_GRF_ICON_SG             TFN_UI_DIRECTORY"/grf/icon_sg"
#define TFN_GRF_ICON_SMS            TFN_UI_DIRECTORY"/grf/icon_sms"
#define TFN_GRF_ICON_SNES           TFN_UI_DIRECTORY"/grf/icon_snes"
#define TFN_GRF_ICON_PLG            TFN_UI_DIRECTORY"/grf/icon_plg"
#define TFN_GRF_ICON_A26            TFN_UI_DIRECTORY"/grf/icon_a26"
#define TFN_GRF_ICON_COL            TFN_UI_DIRECTORY"/grf/icon_col"
#define TFN_GRF_ICON_M5             TFN_UI_DIRECTORY"/grf/icon_m5"
#define TFN_GRF_ICON_INT            TFN_UI_DIRECTORY"/grf/icon_int"
#define TFN_GRF_ICON_PCE            TFN_UI_DIRECTORY"/grf/icon_pce"
#define TFN_GRF_ICON_WS             TFN_UI_DIRECTORY"/grf/icon_ws"
#define TFN_GRF_ICON_NGP            TFN_UI_DIRECTORY"/grf/icon_ngp"
#define TFN_GRF_ICON_CPC            TFN_UI_DIRECTORY"/grf/icon_cpc"
#define TFN_GRF_ICON_VID            TFN_UI_DIRECTORY"/grf/icon_vid"
#define TFN_GRF_ICON_IMG            TFN_UI_DIRECTORY"/grf/icon_img"
#define TFN_GRF_ICON_UNK            TFN_UI_DIRECTORY"/grf/icon_unk"
#define TFN_GRF_LAUNCH_DOT          TFN_UI_DIRECTORY"/grf/launch_dot"
#define TFN_GRF_MOVING_ARROW        TFN_UI_DIRECTORY"/grf/moving_arrow"
#define TFN_GRF_PROGRESS            TFN_UI_DIRECTORY"/grf/progress"
#define TFN_GRF_SCROLL_WINDOW       TFN_UI_DIRECTORY"/grf/scroll_window"
#define TFN_GRF_SMALL_CART          TFN_UI_DIRECTORY"/grf/small_cart"
#define TFN_GRF_START_BORDER        TFN_UI_DIRECTORY"/grf/start_border"
#define TFN_GRF_START_TEXT          TFN_UI_DIRECTORY"/grf/start_text"
#define TFN_GRF_WIRELESSICONS       TFN_UI_DIRECTORY"/grf/wirelessicons"
#define TFN_RVID_CUBES              TFN_UI_DIRECTORY"/video/3dsRotatingCubes.rvid"
#define TFN_RVID_CUBES_BW           TFN_UI_DIRECTORY"/video/3dsRotatingCubes_bw.rvid"
#define TFN_LZ77_RVID_CUBES         TFN_UI_DIRECTORY"/video/3dsRotatingCubes.lz77.rvid"
#define TFN_LZ77_RVID_CUBES_BW      TFN_UI_DIRECTORY"/video/3dsRotatingCubes_bw.lz77.rvid"

#define TFN_FONT_DIRECTORY          TFN_UI_DIRECTORY"/font"
#define TFN_FONT_SMALL              TFN_UI_DIRECTORY"/font/small.nftr"
#define TFN_FONT_SMALL_DS           TFN_UI_DIRECTORY"/font/small-ds.nftr"
#define TFN_FONT_SMALL_DSI          TFN_UI_DIRECTORY"/font/small-dsi.nftr"
#define TFN_FONT_LARGE              TFN_UI_DIRECTORY"/font/large.nftr"
#define TFN_FONT_LARGE_DS           TFN_UI_DIRECTORY"/font/large-ds.nftr"
#define TFN_FONT_LARGE_DSI          TFN_UI_DIRECTORY"/font/large-dsi.nftr"

#define TFN_SOUND_EFFECTBANK        TFN_UI_DIRECTORY"/sound/sfx.bin"
#define TFN_SOUND_BG                TFN_UI_DIRECTORY"/sound/bgm.wav"
#define TFN_SOUND_BG_CACHE          TFN_UI_DIRECTORY"/sound/bgm.pcm.raw"

#define TFN_DEFAULT_SOUND_EFFECTBANK      TFN_SYSTEM_SOUND_DIRECTORY"/defaultfx.bin"
#define TFN_DEFAULT_SOUND_BG              TFN_SYSTEM_SOUND_DIRECTORY"/defaultbg.wav"
#define TFN_DEFAULT_SOUND_BG_CACHE        TFN_SYSTEM_SOUND_CACHE_DIRECTORY"/defaultbg.raw"

#define TFN_SHOP_START_SOUND_BG           TFN_SYSTEM_SOUND_DIRECTORY"/shopbg.start.wav"
#define TFN_SHOP_LOOP_SOUND_BG            TFN_SYSTEM_SOUND_DIRECTORY"/shopbg.loop.wav"
#define TFN_SHOP_START_SOUND_BG_CACHE     TFN_SYSTEM_SOUND_CACHE_DIRECTORY"/shopbg.start.raw"
#define TFN_SHOP_LOOP_SOUND_BG_CACHE      TFN_SYSTEM_SOUND_CACHE_DIRECTORY"/shopbg.loop.raw"

#define TFN_HBL_START_SOUND_BG           TFN_SYSTEM_SOUND_DIRECTORY"/hbl.start.wav"
#define TFN_HBL_LOOP_SOUND_BG            TFN_SYSTEM_SOUND_DIRECTORY"/hbl.loop.wav"
#define TFN_HBL_START_SOUND_BG_CACHE     TFN_SYSTEM_SOUND_CACHE_DIRECTORY"/hbl.start.raw"
#define TFN_HBL_LOOP_SOUND_BG_CACHE      TFN_SYSTEM_SOUND_CACHE_DIRECTORY"/hbl.loop.raw"

#define TFN_SATURN_SOUND_EFFECTBANK       TFN_SYSTEM_SOUND_DIRECTORY"/saturnfx.bin"

#define TFN_FONT_DATE_TIME          TFN_UI_DIRECTORY"/font/date_time.nftr"
#define TFN_FONT_USERNAME           TFN_UI_DIRECTORY"/font/username.nftr"

#define TFN_PALETTE_BIPS                TFN_UI_DIRECTORY"/palettes/bips.bin"
#define TFN_PALETTE_BOX                 TFN_UI_DIRECTORY"/palettes/box.bin"

#define TFN_PALETTE_BOX_FULL            TFN_UI_DIRECTORY"/palettes/box_full.bin"
#define TFN_PALETTE_BOX_EMPTY           TFN_UI_DIRECTORY"/palettes/box_empty.bin"
#define TFN_PALETTE_CURSOR              TFN_UI_DIRECTORY"/palettes/cursor.bin"

#define TFN_PALETTE_BRACE               TFN_UI_DIRECTORY"/palettes/brace.bin"
#define TFN_PALETTE_BUBBLE              TFN_UI_DIRECTORY"/palettes/bubble.bin"
#define TFN_PALETTE_BUTTON_ARROW        TFN_UI_DIRECTORY"/palettes/button_arrow.bin"
#define TFN_PALETTE_CORNERBUTTON        TFN_UI_DIRECTORY"/palettes/cornerbutton.bin"
#define TFN_PALETTE_DIALOGBOX           TFN_UI_DIRECTORY"/palettes/dialogbox.bin"
#define TFN_PALETTE_FOLDER              TFN_UI_DIRECTORY"/palettes/folder.bin"
#define TFN_PALETTE_ICON_GB             TFN_UI_DIRECTORY"/palettes/icon_gb.bin"
#define TFN_PALETTE_ICON_GBA            TFN_UI_DIRECTORY"/palettes/icon_gba.bin"
#define TFN_PALETTE_ICON_GBAMODE        TFN_UI_DIRECTORY"/palettes/icon_gbamode.bin"
#define TFN_PALETTE_ICON_GG             TFN_UI_DIRECTORY"/palettes/icon_gg.bin"
#define TFN_PALETTE_ICON_MD             TFN_UI_DIRECTORY"/palettes/icon_md.bin"
#define TFN_PALETTE_ICON_NES            TFN_UI_DIRECTORY"/palettes/icon_nes.bin"
#define TFN_PALETTE_ICON_SETTINGS       TFN_UI_DIRECTORY"/palettes/icon_settings.bin"
#define TFN_PALETTE_ICON_MANUAL         TFN_UI_DIRECTORY"/palettes/icon_manual.bin"
#define TFN_PALETTE_ICON_SG             TFN_UI_DIRECTORY"/palettes/icon_sg.bin"
#define TFN_PALETTE_ICON_SMS            TFN_UI_DIRECTORY"/palettes/icon_sms.bin"
#define TFN_PALETTE_ICON_SNES           TFN_UI_DIRECTORY"/palettes/icon_snes.bin"
#define TFN_PALETTE_ICON_PLG            TFN_UI_DIRECTORY"/palettes/icon_plg.bin"
#define TFN_PALETTE_ICON_A26            TFN_UI_DIRECTORY"/palettes/icon_a26.bin"
#define TFN_PALETTE_ICON_COL            TFN_UI_DIRECTORY"/palettes/icon_col.bin"
#define TFN_PALETTE_ICON_M5             TFN_UI_DIRECTORY"/palettes/icon_m5.bin"
#define TFN_PALETTE_ICON_INT            TFN_UI_DIRECTORY"/palettes/icon_int.bin"
#define TFN_PALETTE_ICON_PCE            TFN_UI_DIRECTORY"/palettes/icon_pce.bin"
#define TFN_PALETTE_ICON_WS             TFN_UI_DIRECTORY"/palettes/icon_ws.bin"
#define TFN_PALETTE_ICON_NGP            TFN_UI_DIRECTORY"/palettes/icon_ngp.bin"
#define TFN_PALETTE_ICON_CPC            TFN_UI_DIRECTORY"/palettes/icon_cpc.bin"
#define TFN_PALETTE_ICON_VID            TFN_UI_DIRECTORY"/palettes/icon_vid.bin"
#define TFN_PALETTE_ICON_IMG            TFN_UI_DIRECTORY"/palettes/icon_img.bin"
#define TFN_PALETTE_ICON_UNK            TFN_UI_DIRECTORY"/palettes/icon_unk.bin"
#define TFN_PALETTE_LAUNCH_DOT          TFN_UI_DIRECTORY"/palettes/launch_dot.bin"
#define TFN_PALETTE_MOVING_ARROW        TFN_UI_DIRECTORY"/palettes/moving_arrow.bin"
#define TFN_PALETTE_PROGRESS            TFN_UI_DIRECTORY"/palettes/progress.bin"
#define TFN_PALETTE_SCROLL_WINDOW       TFN_UI_DIRECTORY"/palettes/scroll_window.bin"
#define TFN_PALETTE_SMALL_CART          TFN_UI_DIRECTORY"/palettes/small_cart.bin"
#define TFN_PALETTE_START_BORDER        TFN_UI_DIRECTORY"/palettes/start_border.bin"
#define TFN_PALETTE_START_TEXT          TFN_UI_DIRECTORY"/palettes/start_text.bin"
#define TFN_PALETTE_WIRELESSICONS       TFN_UI_DIRECTORY"/palettes/wirelessicons.bin"

#define TFN_PALETTE_USERNAME            TFN_UI_DIRECTORY"/palettes/username.bin"

// We need fallbacks, because unlike akmenu, a bad GRF file could crash the theme
// Leaving the user with no way to fix it well.

#define TFN_FALLBACK_UI_CURRENT_DIRECTORY    TFN_FALLBACK_UI_DIRECTORY"/"
#define TFN_FALLBACK_THEME_SETTINGS          TFN_FALLBACK_UI_DIRECTORY"/theme.ini"

#define TFN_FALLBACK_BG_TOPBG                TFN_FALLBACK_UI_DIRECTORY"/background/top.grf"
#define TFN_FALLBACK_BG_BOTTOMBG             TFN_FALLBACK_UI_DIRECTORY"/background/bottom.grf"
#define TFN_FALLBACK_BG_BOTTOMBUBBLEBG       TFN_FALLBACK_UI_DIRECTORY"/background/bottom_bubble.grf"
#define TFN_FALLBACK_BG_BOTTOMBUBBLEBG_MACRO TFN_FALLBACK_UI_DIRECTORY"/background/bottom_bubble_macro.grf"
#define TFN_FALLBACK_BG_BOTTOMMOVINGBG       TFN_FALLBACK_UI_DIRECTORY"/background/bottom_moving.grf"
#define TFN_FALLBACK_BG_BOTTOMMOVING_LBG     TFN_FALLBACK_UI_DIRECTORY"/background/bottom_moving_l.grf"
#define TFN_FALLBACK_BG_BOTTOMMOVING_RBG     TFN_FALLBACK_UI_DIRECTORY"/background/bottom_moving_r.grf"

#define TFN_FALLBACK_BG_BOTTOMBG_DS          TFN_FALLBACK_UI_DIRECTORY"/background/bottom_ds.grf"
#define TFN_FALLBACK_BG_BOTTOMBUBBLEBG_DS    TFN_FALLBACK_UI_DIRECTORY"/background/bottom_bubble_ds.grf"

#define TFN_FALLBACK_UI_LSHOULDER            TFN_FALLBACK_UI_DIRECTORY"/ui/Lshoulder.png"
#define TFN_FALLBACK_UI_LSHOULDER_GREYED     TFN_FALLBACK_UI_DIRECTORY"/ui/Lshoulder_greyed.png"
#define TFN_FALLBACK_UI_RSHOULDER            TFN_FALLBACK_UI_DIRECTORY"/ui/Rshoulder.png"
#define TFN_FALLBACK_UI_RSHOULDER_GREYED     TFN_FALLBACK_UI_DIRECTORY"/ui/Rshoulder_greyed.png"

#define TFN_FALLBACK_BATTERY0                TFN_FALLBACK_UI_DIRECTORY"/battery/battery0.png"
#define TFN_FALLBACK_BATTERY1                TFN_FALLBACK_UI_DIRECTORY"/battery/battery1.png"
#define TFN_FALLBACK_BATTERY2                TFN_FALLBACK_UI_DIRECTORY"/battery/battery2.png"
#define TFN_FALLBACK_BATTERY3                TFN_FALLBACK_UI_DIRECTORY"/battery/battery3.png"
#define TFN_FALLBACK_BATTERY4                TFN_FALLBACK_UI_DIRECTORY"/battery/battery4.png"
#define TFN_FALLBACK_BATTERY1_PURPLE         TFN_FALLBACK_UI_DIRECTORY"/battery/battery1purple.png"
#define TFN_FALLBACK_BATTERY2_PURPLE         TFN_FALLBACK_UI_DIRECTORY"/battery/battery2purple.png"
#define TFN_FALLBACK_BATTERY3_PURPLE         TFN_FALLBACK_UI_DIRECTORY"/battery/battery3purple.png"
#define TFN_FALLBACK_BATTERY4_PURPLE         TFN_FALLBACK_UI_DIRECTORY"/battery/battery4purple.png"
#define TFN_FALLBACK_BATTERY_CHARGE          TFN_FALLBACK_UI_DIRECTORY"/battery/batterycharge.png"
#define TFN_FALLBACK_BATTERY_CHARGE_BLINK    TFN_FALLBACK_UI_DIRECTORY"/battery/batterychargeblink.png"
#define TFN_FALLBACK_BATTERY_FULL            TFN_FALLBACK_UI_DIRECTORY"/battery/batteryfull.png"
#define TFN_FALLBACK_BATTERY_FULLDS          TFN_FALLBACK_UI_DIRECTORY"/battery/batteryfullDS.png"
#define TFN_FALLBACK_BATTERY_LOW             TFN_FALLBACK_UI_DIRECTORY"/battery/batterylow.png"

#define TFN_FALLBACK_VOLUME0                 TFN_FALLBACK_UI_DIRECTORY"/volume/volume0.png"
#define TFN_FALLBACK_VOLUME1                 TFN_FALLBACK_UI_DIRECTORY"/volume/volume1.png"
#define TFN_FALLBACK_VOLUME2                 TFN_FALLBACK_UI_DIRECTORY"/volume/volume2.png"
#define TFN_FALLBACK_VOLUME3                 TFN_FALLBACK_UI_DIRECTORY"/volume/volume3.png"
#define TFN_FALLBACK_VOLUME4                 TFN_FALLBACK_UI_DIRECTORY"/volume/volume4.png"

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
#define TFN_FALLBACK_GRF_ICON_SG             TFN_FALLBACK_UI_DIRECTORY"/grf/icon_sg.grf"
#define TFN_FALLBACK_GRF_ICON_SMS            TFN_FALLBACK_UI_DIRECTORY"/grf/icon_sms.grf"
#define TFN_FALLBACK_GRF_ICON_SNES           TFN_FALLBACK_UI_DIRECTORY"/grf/icon_snes.grf"
#define TFN_FALLBACK_GRF_ICON_PLG            TFN_FALLBACK_UI_DIRECTORY"/grf/icon_plg.grf"
#define TFN_FALLBACK_GRF_ICON_A26            TFN_FALLBACK_UI_DIRECTORY"/grf/icon_a26.grf"
#define TFN_FALLBACK_GRF_ICON_COL            TFN_FALLBACK_UI_DIRECTORY"/grf/icon_col.grf"
#define TFN_FALLBACK_GRF_ICON_M5             TFN_FALLBACK_UI_DIRECTORY"/grf/icon_m5.grf"
#define TFN_FALLBACK_GRF_ICON_INT            TFN_FALLBACK_UI_DIRECTORY"/grf/icon_int.grf"
#define TFN_FALLBACK_GRF_ICON_PCE            TFN_FALLBACK_UI_DIRECTORY"/grf/icon_pce.grf"
#define TFN_FALLBACK_GRF_ICON_WS             TFN_FALLBACK_UI_DIRECTORY"/grf/icon_ws.grf"
#define TFN_FALLBACK_GRF_ICON_NGP            TFN_FALLBACK_UI_DIRECTORY"/grf/icon_ngp.grf"
#define TFN_FALLBACK_GRF_ICON_CPC            TFN_FALLBACK_UI_DIRECTORY"/grf/icon_cpc.grf"
#define TFN_FALLBACK_GRF_ICON_VID            TFN_FALLBACK_UI_DIRECTORY"/grf/icon_vid.grf"
#define TFN_FALLBACK_GRF_ICON_IMG            TFN_FALLBACK_UI_DIRECTORY"/grf/icon_img.grf"
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

#define TFN_FALLBACK_FONT_DATE_TIME          "nitro:/graphics/font/date_time.nftr"


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