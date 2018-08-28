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
#include "tool/stringtool.h"
#include <string>
#include "dsrom.h"

class cRomInfoWnd : public akui::cForm
{
  public:
    cRomInfoWnd(s32 x, s32 y, u32 w, u32 h, cWindow *parent, const std::string &text);

    ~cRomInfoWnd();

  public:
    void draw();

    bool process(const akui::cMessage &msg);

    cWindow &loadAppearance(const std::string &aFileName);

    void setFileInfo(const std::string &fullName, const std::string &showName);

    void setRomInfo(const DSRomInfo &romInfo);

    const DSRomInfo &getRomInfo();

    void setSaves(const std::vector<std::string> *saves);

    bool SlotExists(u8 slot);

    static void showCheats(const std::string &aFileName);

  protected:
    void setDiskInfo(void);

    // void pressFlash(void);

    // void pressSaveType(void);

    // void pressCopy(void);

    // void pressCheats(void);

    // void pressCopySlot(void);

    bool processKeyMessage(const akui::cKeyMessage &msg);

    void onOK();

    void onShow();

    void addCode(void);

    akui::cButton _buttonOK;

    akui::cButton _buttonSaveType;

    akui::cButton _buttonFlash;

    akui::cButton _buttonCopy;

    akui::cButton _buttonCheats;

    akui::cFormDesc _renderDesc;

    DSRomInfo _romInfo;

    std::string _romInfoText;

    std::string _filenameText;

    std::string _fileDateText;

    std::string _fileSizeText;

    std::string _saveTypeText;

    std::string _fullName;

    u32 _size;

    //cSettingWnd *_settingWnd;

    const std::vector<std::string> *_saves;
};

#endif //_ROMINFOWND_H_
