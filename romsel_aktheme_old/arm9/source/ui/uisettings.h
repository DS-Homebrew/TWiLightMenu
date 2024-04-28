/*
    uisettings.h
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

#ifndef _UISETTINGS_H_
#define _UISETTINGS_H_

#include <nds.h>
#include "drawing/gdi.h"
#include "common/singleton.h"

class UISettings
{
  public:
    UISettings();

    ~UISettings();

  public:
    void loadSettings();

  public:
    bool showCalendar;

    COLOR formFrameColor;
    COLOR formBodyColor;
    COLOR formTextColor;
    COLOR formTitleTextColor;
    COLOR buttonTextColor;
    COLOR spinBoxNormalColor;
    COLOR spinBoxFocusColor;
    COLOR spinBoxTextColor;
    COLOR spinBoxTextHighLightColor;
    COLOR spinBoxFrameColor;
    COLOR listViewBarColor1;
    COLOR listViewBarColor2;
    COLOR listTextColor;
    COLOR listTextHighLightColor;
    COLOR popMenuTextColor;
    COLOR popMenuTextHighLightColor;
    COLOR popMenuBarColor;

    u32 thickness;
};

typedef singleton<UISettings> uiSettings_s;
inline UISettings &uiSettings() { return uiSettings_s::instance(); }
inline UISettings &uis() { return uiSettings_s::instance(); }

#endif //_UISETTINGS_H_
