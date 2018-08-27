/*
    globalsettings.h
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

#ifndef _GLOBALSETTINGS_H_
#define _GLOBALSETTINGS_H_

#include <nds.h>
#include <string>
#include "singleton.h"
#include "tool/fifotool.h"

class cGlobalSettings
{
public:
  enum TScrollSpeed
  {
    EScrollFast=4,
    EScrollMedium=10,
    EScrollSlow=16
  };
  enum TViewMode
  {
    EViewList=0,
    EViewIcon=1,
    EViewInternal=2
  };
  enum TSlot2Mode
  {
    ESlot2Ask=0,
    ESlot2Gba=1,
    ESlot2Nds=2
  };
public:

    cGlobalSettings();

    ~cGlobalSettings();

public:

    void loadSettings();
    void saveSettings();
    void updateSafeMode(void);
    static u32 CopyBufferSize(void);
    void nextBrightness(void);
public:
    u8          fontHeight;
    u8          language;
    u8          fileListType;
    u8          romTrim;
    std::string langDirectory;
    std::string uiName;
    std::string startupFolder;
    bool        showHiddenFiles;
    bool        enterLastDirWhenBoot;
    int         scrollSpeed;
    bool        showGbaRoms;
    int         viewMode;
    bool        gbaSleepHack;
    bool        gbaAutoSave;
    bool        Animation;
    bool        cheats;
    bool        softreset;
    bool        dma;
    bool        sdsave;
    bool        cheatDB;
    int         slot2mode;
    bool        saveExt;
    bool        safeMode;
    bool        show12hrClock;
    bool        autorunWithLastRom;
    bool        homebrewreset;
};


typedef t_singleton< cGlobalSettings > globalSettings_s;
inline cGlobalSettings & gs() { return globalSettings_s::instance(); }

#endif//_GLOBALSETTINGS_H_
