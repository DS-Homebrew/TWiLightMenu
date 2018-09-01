/*
    rominfownd.cpp
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

#include <sys/stat.h>
#include "rominfownd.h"
#include "systemfilenames.h"
#include "ui/msgbox.h"
#include "windows/dsiiconsequence.h"
#include "bootstrap_support/dsimenusettings.h"
#include "bootstrap_support/pergamesettings.h"
#include "ui/windowmanager.h"
#include "ui/uisettings.h"
#include "language.h"
#include "unicode.h"
#include "ui/binaryfind.h"

using namespace akui;

RomInfoWnd::RomInfoWnd(s32 x, s32 y, u32 w, u32 h, Window *parent, const std::string &text)
    : Form(x, y, w, h, parent, text),
      _buttonOK(0, 0, 46, 18, this, "\x01 OK"),
      _buttonGameSettings(0, 0, 76, 18, this, "\x04 Save Type"),
      _settingWnd(NULL)
//   _saves(NULL)
{
    s16 buttonY = size().y - _buttonOK.size().y - 4;

    _buttonOK.setStyle(Button::press);
    _buttonOK.setText("\x01 " + LANG("setting window", "ok"));
    _buttonOK.setTextColor(uis().buttonTextColor);
    _buttonOK.loadAppearance(SFN_BUTTON3);
    _buttonOK.clicked.connect(this, &RomInfoWnd::onOK);
    addChildWindow(&_buttonOK);

    s16 nextButtonX = size().x;

    s16 buttonPitch = _buttonOK.size().x + 8;
    nextButtonX -= buttonPitch;

    _buttonOK.setRelativePosition(Point(nextButtonX, buttonY));
    _buttonGameSettings.setStyle(Button::press);
    _buttonGameSettings.setText("\x04 " + LANG("setting window", "Settings"));
    _buttonGameSettings.setTextColor(uis().buttonTextColor);
    _buttonGameSettings.loadAppearance(SFN_BUTTON3);
    _buttonGameSettings.clicked.connect(this, &RomInfoWnd::pressGameSettings);
    addChildWindow(&_buttonGameSettings);

    buttonPitch = _buttonGameSettings.size().x + 8;
    s16 nextButtonXone = nextButtonX - buttonPitch;

    _buttonGameSettings.setRelativePosition(Point(nextButtonXone, buttonY));

    loadAppearance("");
    arrangeChildren();
}

RomInfoWnd::~RomInfoWnd()
{
}

void RomInfoWnd::draw()
{
    _renderDesc.draw(windowRectangle(), _engine);

    if (_romInfo.isBannerAnimated())
    {
        int seqIdx = seq().allocate_sequence(
            _romInfo.saveInfo().gameCode,
            _romInfo.animatedIcon().sequence);

        int bmpIdx = seq()._dsiIconSequence[seqIdx]._bitmapIndex;
        int palIdx = seq()._dsiIconSequence[seqIdx]._paletteIndex;
        _romInfo.drawDSiAnimatedRomIcon(position().x + 8, position().y + 24, bmpIdx, palIdx, selectedEngine());
    }
    else
    {
        _romInfo.drawDSRomIcon(position().x + 8, position().y + 24, selectedEngine());
    }

    gdi().setPenColor(uiSettings().formTextColor, selectedEngine());
    gdi().textOutRect(position().x + 48, position().y + 22, size().x - 40, 40, _romInfoText.c_str(), selectedEngine());

    gdi().textOutRect(position().x + 8, position().y + 64, size().x - 8, 40, _filenameText.c_str(), selectedEngine());
    gdi().textOutRect(position().x + 8, position().y + 64 + 14, size().x - 8, 40, _fileDateText.c_str(), selectedEngine());
    gdi().textOutRect(position().x + 8, position().y + 64 + 14 + 14, size().x - 8, 40, _fileSizeText.c_str(), selectedEngine());
    gdi().textOutRect(position().x + 8, position().y + 64 + 14 + 14 + 14, size().x - 8, 40, _saveTypeText.c_str(), selectedEngine());

    Form::draw();
    // Powersaving loop
    swiWaitForVBlank();
}

bool RomInfoWnd::process(const akui::Message &msg)
{
    bool ret = false;

    ret = Form::process(msg);

    if (!ret)
    {
        if (msg.id() > Message::keyMessageStart && msg.id() < Message::keyMessageEnd)
        {
            ret = processKeyMessage((KeyMessage &)msg);
        }
    }
    return ret;
}

bool RomInfoWnd::processKeyMessage(const KeyMessage &msg)
{
    bool ret = false;
    if (msg.id() == Message::keyDown)
    {
        switch (msg.keyCode())
        {
        case KeyMessage::UI_KEY_A:
        case KeyMessage::UI_KEY_B:
            onOK();
            ret = true;
            break;
        case KeyMessage::UI_KEY_Y:
            pressGameSettings();
            ret = true;
            break;
        // case KeyMessage::UI_KEY_X:
        //     // if (_buttonCheats.isVisible())
        //     // {
        //     //     pressCheats();
        //     // }
        //     // else if (_buttonFlash.isVisible())
        //     // {
        //     //     pressFlash();
        //     // }
        //     ret = true;
        //     break;
        // case KeyMessage::UI_KEY_L:
        //     pressCopy();
        //     ret = true;
        //     break;
        default:
        {
        }
        };
    }

    return ret;
}

void RomInfoWnd::pressGameSettings(void)
{
    if (!_romInfo.isDSRom() && !_romInfo.isHomebrew())
        return;
    if (_romInfo.isDSiWare())
        return;

    SettingWnd settingWnd(0, 0, 252, 188, this, "Per Game Settings");

    PerGameSettings settingsIni(_filenameText);
    std::vector<std::string> _values;

    if (!_romInfo.isHomebrew())
    {
        _values.push_back(LANG("game settings", "Default")); // -2 => 0
        _values.push_back(LANG("game settings", "System")); // -1 => 1
        _values.push_back(LANG("game settings", "Japanese"));
        _values.push_back(LANG("game settings", "English"));
        _values.push_back(LANG("game settings", "French"));
        _values.push_back(LANG("game settings", "German"));
        _values.push_back(LANG("game settings", "Italian"));
        _values.push_back(LANG("game settings", "Spanish"));

        settingWnd.addSettingItem(LANG("game settings", "Language"), _values, settingsIni.language + 2); // Default is -2
        _values.clear();

        _values.push_back(LANG("game settings", "Default")); // -1 => 0
        _values.push_back(LANG("game settings", "67MHz (NTR)")); // 0 => 1
        _values.push_back(LANG("game settings", "133MHz (TWL)")); // 1 => 2

        settingWnd.addSettingItem(LANG("game settings", "CPU Frequency"), _values, settingsIni.boostCpu + 1);
        _values.clear();

        _values.push_back(LANG("game settings", "Default")); // -1 => 0
        _values.push_back(LANG("game settings", "Off")); // 0 => 1
        _values.push_back(LANG("game settings", "On")); // 1 => 2

        settingWnd.addSettingItem(LANG("game settings", "Boost VRAM"), _values, settingsIni.boostVram + 1);
        _values.clear();

        _values.push_back(LANG("game settings", "Default")); // -1 => 0
        _values.push_back(LANG("game settings", "Off")); // 0 => 1
        _values.push_back(LANG("game settings", "On")); // 1 => 2

        settingWnd.addSettingItem(LANG("game settings", "Sound Fix"), _values, settingsIni.soundFix + 1);
        _values.clear();

        _values.push_back(LANG("game settings", "Default")); // -1 => 0
        _values.push_back(LANG("game settings", "Off")); // 0 => 1
        _values.push_back(LANG("game settings", "On")); // 1 => 2

        settingWnd.addSettingItem(LANG("game settings", "Async Prefetch"), _values, settingsIni.asyncPrefetch + 1);
        _values.clear();
    }
    else
    {

        _values.push_back(LANG("game settings", "Default"));
        _values.push_back(LANG("game settings", "Use Bootstrap"));
        _values.push_back(LANG("game settings", "Direct Boot"));

        settingWnd.addSettingItem(LANG("game settings", "Direct Boot"), _values, settingsIni.directBoot + 1);
        _values.clear();
    }

    _settingWnd = &settingWnd;
    u32 ret = settingWnd.doModal();

    dbg_printf("SETRET %i\n", ret);
    if (ret == ID_OK)
    {
        dbg_printf("FN: %s\n", _filenameText.c_str());
        if (!_romInfo.isHomebrew())
        {
            settingsIni.language = (PerGameSettings::TLanguage)(settingWnd.getItemSelection(0, 0) - 2);
            settingsIni.boostCpu = (PerGameSettings::TDefaultBool)(settingWnd.getItemSelection(0, 1) - 1);
            settingsIni.boostVram = (PerGameSettings::TDefaultBool)(settingWnd.getItemSelection(0, 2) - 1);
            settingsIni.soundFix = (PerGameSettings::TDefaultBool)(settingWnd.getItemSelection(0, 3) - 1);
            settingsIni.asyncPrefetch = (PerGameSettings::TDefaultBool)(settingWnd.getItemSelection(0, 4) - 1);
        }
        else
        {
            settingsIni.directBoot = (PerGameSettings::TDefaultBool)(settingWnd.getItemSelection(0, 0) - 1);
        }
        settingsIni.saveSettings();
    }
    _settingWnd = NULL;
    if (ret == ID_CANCEL)
        return;
}

Window &RomInfoWnd::loadAppearance(const std::string &aFileName)
{
    _renderDesc.loadData(SFN_FORM_TITLE_L, SFN_FORM_TITLE_R, SFN_FORM_TITLE_M);
    _renderDesc.setTitleText(_text);
    return *this;
}

static std::string getFriendlyFileSizeString(u64 size)
{
    std::string fileSize;
    std::string sizeUnit;
    if (size < 1024)
    {
        fileSize = formatString("%d", size);
        sizeUnit = " Byte";
    }
    else
    {
        u32 divider;
        if (size < 1024 * 1024)
        {
            divider = 1024;
            sizeUnit = " KB";
        }
        else if (size < 1024 * 1024 * 1024)
        {
            divider = 1024 * 1024;
            sizeUnit = " MB";
        }
        else
        {
            divider = 1024 * 1024 * 1024;
            sizeUnit = " GB";
        }
        fileSize = formatString("%d.%02d", (u32)(size / divider), (u32)((size * 100 + (divider >> 1)) / divider % 100));
    }
    return fileSize + sizeUnit;
}

void RomInfoWnd::setFileInfo(const std::string &fullName, const std::string &showName)
{
    _fullName = fullName;

    if ("" == showName)
    {
        dbg_printf("show name %s\n", showName.c_str());
        return;
    }

    struct stat st;
    if (-1 == stat(fullName.c_str(), &st))
    {
        return;
    }

    if ("/" == fullName)
    {
        // setDiskInfo();
        return;
    }

    _filenameText = showName;

    //dbg_printf("st.st_mtime %d\n", st.st_mtime );
    //struct tm * filetime = localtime(&st.st_mtime);

    struct tm *filetime = gmtime(&st.st_mtime);

    _fileDateText = formatString(LANG("rom info", "file date").c_str(),
                                 filetime->tm_year + 1900, filetime->tm_mon + 1, filetime->tm_mday,
                                 filetime->tm_hour, filetime->tm_min, filetime->tm_sec);

    dbg_printf("st.st_mtime %d\n", (u32)st.st_mtime);
    dbg_printf("%d-%d-%d %02d:%02d:%02d\n",
               filetime->tm_year + 1900, filetime->tm_mon + 1, filetime->tm_mday,
               filetime->tm_hour, filetime->tm_min, filetime->tm_sec);

    if ('/' == fullName[fullName.size() - 1])
    {
        _fileSizeText = "";
    }
    else
    {
        _fileSizeText = formatString(LANG("rom info", "file size").c_str(), getFriendlyFileSizeString(st.st_size).c_str());
    }
    _size = st.st_size;
}

void RomInfoWnd::setRomInfo(const DSRomInfo &romInfo)
{
    _romInfo = romInfo;

    _romInfoText = unicode_to_local_string(_romInfo.banner().titles[ms().guiLanguage()], 128, NULL);

    _buttonGameSettings.hide();

    if (_romInfo.isDSRom() && !_romInfo.isHomebrew())
    {
        addCode();
    }
    if (!_romInfo.isDSiWare() && (_romInfo.isDSRom() || _romInfo.isHomebrew()))
    {
        _buttonGameSettings.show();
    }
}

const DSRomInfo &RomInfoWnd::getRomInfo()
{
    return _romInfo;
}

void RomInfoWnd::onOK()
{
    Form::onOK();
}

void RomInfoWnd::onShow()
{
    centerScreen();
}

void RomInfoWnd::addCode(void)
{
    char gameCode[5];
    memcpy(gameCode, _romInfo.saveInfo().gameCode, sizeof(_romInfo.saveInfo().gameCode));
    gameCode[4] = 0;
    // if (_saveTypeText.length())
    //     _saveTypeText += ", ";
    _saveTypeText += formatString(LANG("rom info", "game code").c_str(), gameCode);
    if (_romInfo.version() > 0)
    {
        _saveTypeText += formatString("v%02d", _romInfo.version());
    }
    _saveTypeText += formatString(", SDK %X", (_romInfo.saveInfo().gameSdkVersion & 0xF000000) >> 0x18);
}

// #if defined(_STORAGE_rpg)
// #define ITEM_SAVETYPE 0, 0
// #define ITEM_DOWNLOADPLAY 0, 1
// #define ITEM_DMA 0, 2
// #define ITEM_PROTECTION 0, 3
// #define ITEM_LINKAGE 0, 4
// #define ITEM_RUMBLE 0, 5

// #define ITEM_CHEATS 1, 0
// #define ITEM_SOFTRESET 1, 1
// #define ITEM_SDSAVE 1, 2
// #define ITEM_SAVESLOT 1, 3
// #define ITEM_ICON 1, 4
// #define ITEM_LANGUAGE 1, 5
// #elif defined(_STORAGE_r4) || defined(_STORAGE_r4idsn)
// #define ITEM_SAVETYPE 0, 0
// #define ITEM_LINKAGE 0, 1
// #define ITEM_RUMBLE 0, 2

// #define ITEM_CHEATS 1, 0
// #define ITEM_SOFTRESET 1, 1
// #define ITEM_SAVESLOT 1, 2
// #define ITEM_ICON 1, 3
// #define ITEM_LANGUAGE 1, 4
// #elif defined(_STORAGE_ak2i)
// #define ITEM_SAVETYPE 0, 0
// #define ITEM_DOWNLOADPLAY 0, 1
// #define ITEM_DMA 0, 2
// #define ITEM_PROTECTION 0, 3
// #define ITEM_LINKAGE 0, 4
// #define ITEM_RUMBLE 0, 5

// #define ITEM_CHEATS 1, 0
// #define ITEM_SOFTRESET 1, 1
// #define ITEM_SAVESLOT 1, 2
// #define ITEM_ICON 1, 3
// #define ITEM_LANGUAGE 1, 4
// #endif