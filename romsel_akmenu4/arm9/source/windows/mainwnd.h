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
//#include "settingwnd.h"
#include "startmenu.h"

class cMainWnd : public akui::cForm
{
public:

    cMainWnd( s32 x, s32 y, u32 w, u32 h, cWindow * parent, const std::string & text );

    ~cMainWnd();

public:

    bool process( const akui::cMessage & msg );

    cWindow& loadAppearance(const std::string& aFileName );

    void init();

    void draw();

    cWindow* windowBelow(const akui::cPoint & p);

    cMainList * _mainList;

protected:

    void onMainListSelItemClicked( u32 index );

    void onMainListSelItemHeadClicked( u32 index );

    void onKeyAPressed();

    void onKeyBPressed();

    void onKeyXPressed();

    void onKeyYPressed();

    void listSelChange( u32 i );

    void startMenuItemClicked( s16 i );

    void startButtonClicked();

    void brightnessButtonClicked();

    bool processKeyMessage( const akui::cKeyMessage & msg );

    bool processTouchMessage( const akui::cTouchMessage & msg );

    void setParam(void);

    void showSettings(void);

    void onFolderChanged();

    void onAnimation(bool& anAllow);

    void showFileInfo();

    void launchSelected();

    cStartMenu * _startMenu;

    akui::cButton * _startButton;

    akui::cButton * _brightnessButton;

    akui::cButton * _folderUpButton;

    akui::cStaticText * _folderText;

    bool _processL;
};



#endif//_MAINWND_H_
