/*
    mainwnd.h
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

#ifndef _MAINWND_H_
#define _MAINWND_H_

#include "ui/form.h"
#include "mainlist.h"
#include "ui/button.h"
#include "ui/keymessage.h"
#include "ui/touchmessage.h"
#include "ui/spinbox.h"
#include "settingwnd.h"
#include "startmenu.h"

#include "common/pergamesettings.h"
#include "dsrom.h"

class MainWnd : public akui::Form
{
  public:
    MainWnd(s32 x, s32 y, u32 w, u32 h, Window *parent, const std::string &text);

    ~MainWnd();

  public:
    bool process(const akui::Message &msg);

    Window &loadAppearance(const std::string &aFileName);

    void init();

    void draw();

    Window *windowBelow(const akui::Point &p);

    MainList *_mainList;

    //void bootWidescreen(const char *filename);

  protected:
    void onMainListSelItemClicked(u32 index);

    void onKeyAPressed();

    void onKeyBPressed();

    void onKeyXPressed();

    void onKeyYPressed();

    void listSelChange(u32 i);

    void startMenuItemClicked(s16 i);

    void startButtonClicked();

    void brightnessButtonClicked();

    bool processKeyMessage(const akui::KeyMessage &msg);

    bool processTouchMessage(const akui::TouchMessage &msg);

    void setParam(void);

    void showSettings(void);

    void showManual(void);

    void bootSlot1(void);

    void bootGbaRunner(void);

    void onFolderChanged();

    void onAnimation(bool &anAllow);

    void showFileInfo();

    void launchSelected();

    void bootArgv(DSRomInfo& rominfo);

    void bootBootstrap(PerGameSettings &gameConfig, DSRomInfo& rominfo);

    void bootFlashcard(const std::string &ndsPath, bool usePerGameSettings);

    void bootFile(const std::string &loader, const std::string &fullPath);

    StartMenu *_startMenu;

    akui::Button *_startButton;

    akui::Button *_brightnessButton;

    akui::Button *_batteryIcon;

    akui::Button *_folderUpButton;

    akui::StaticText *_folderText;

    bool _processL;
    
};

std::string apFix(const char *filename, bool isHomebrew);
void bootWidescreen(const char *filename, bool isHomebrew, bool useWidescreen);

#endif //_MAINWND_H_
