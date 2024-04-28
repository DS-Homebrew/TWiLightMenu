/*
    rominfownd.h
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

#ifndef _ROMINFOWND_H_
#define _ROMINFOWND_H_

#include "ui/form.h"
#include "ui/formdesc.h"
#include "ui/spinbox.h"
#include "ui/statictext.h"
#include "ui/message.h"
#include "settingwnd.h"
#include "tool/stringtool.h"
#include <string>
#include "dsrom.h"

class RomInfoWnd : public akui::Form
{
  public:
    RomInfoWnd(s32 x, s32 y, u32 w, u32 h, Window *parent, const std::string &text);

    ~RomInfoWnd();

  public:
    void draw();

    bool process(const akui::Message &msg);

    Window &loadAppearance(const std::string &aFileName);

    void setFileInfo(const std::string &fullName, const std::string &showName);

    void setRomInfo(const DSRomInfo &romInfo);

    const DSRomInfo &getRomInfo();

    // static void showPerGameSettings(const std::string &aFileName);
    
    static void showCheats(const std::string &aFileName);

  protected:
    void setDiskInfo(void);

    // void pressFlash(void);

    void pressGameSettings(void);

    // void pressCopy(void);

    void pressCheats(void);

    // void pressCopySlot(void);

    bool processKeyMessage(const akui::KeyMessage &msg);

    void onOK();

    void onShow();

    void addCode(void);

    akui::Button _buttonOK;

    akui::Button _buttonGameSettings;

    // akui::Button _buttonFlash;

    // akui::Button _buttonCopy;

    akui::Button _buttonCheats;

    akui::FormDesc _renderDesc;

    DSRomInfo _romInfo;

    std::string _romInfoText;

    std::string _filenameText;

    std::string _fileDateText;

    std::string _fileSizeText;

    std::string _saveTypeText;

    std::string _fullName;

    u32 _size;

    SettingWnd *_settingWnd;
};

#endif //_ROMINFOWND_H_
