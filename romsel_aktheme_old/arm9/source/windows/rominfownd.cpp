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
#include "windows/cheatwnd.h"
#include "common/dsimenusettings.h"
#include "common/systemdetails.h"
#include "common/pergamesettings.h"
#include "ui/windowmanager.h"
#include "ui/uisettings.h"
#include "language.h"
#include "unicode.h"
#include "ui/binaryfind.h"
#include "common/tonccpy.h"

using namespace akui;

RomInfoWnd::RomInfoWnd(s32 x, s32 y, u32 w, u32 h, Window *parent, const std::string &text)
    : Form(x, y, w, h, parent, text),
      _buttonOK(0, 0, 46, 18, this, "\x01 OK"),
      _buttonGameSettings(0, 0, 76, 18, this, "\x04 Save Type"),
      _buttonCheats(0, 0, 46, 18, this, "\x03 Cheats"),
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
    _buttonGameSettings.setText("\x04 " + LANG("setting window", "setting"));
    _buttonGameSettings.setTextColor(uis().buttonTextColor);
    _buttonGameSettings.loadAppearance(SFN_BUTTON3);
    _buttonGameSettings.clicked.connect(this, &RomInfoWnd::pressGameSettings);
    addChildWindow(&_buttonGameSettings);

    buttonPitch = _buttonGameSettings.size().x + 8;
    s16 nextButtonXone = nextButtonX - buttonPitch;

    _buttonGameSettings.setRelativePosition(Point(nextButtonXone, buttonY));

	if (isDSiMode()) {
		_buttonCheats.setStyle( Button::press );
		_buttonCheats.setText( "\x03 " + LANG( "cheats", "button" ) );
		_buttonCheats.setTextColor( uis().buttonTextColor );
		_buttonCheats.loadAppearance( SFN_BUTTON3 );
		_buttonCheats.clicked.connect( this, &RomInfoWnd::pressCheats );
		addChildWindow( &_buttonCheats );
	}

    buttonPitch = _buttonCheats.size().x + 8;
    nextButtonXone -= buttonPitch;

    _buttonCheats.setRelativePosition( Point(nextButtonXone, buttonY) );

    loadAppearance("");
    arrangeChildren();
}

RomInfoWnd::~RomInfoWnd()
{
}

void RomInfoWnd::draw()
{
    _renderDesc.draw(windowRectangle(), _engine);

    if (_romInfo.isBannerAnimated() && ms().animateDsiIcons)
    {
        int seqIdx = seq().allocate_sequence(
            _romInfo.saveInfo().gameCode,
            _romInfo.animatedIcon().sequence);

        int bmpIdx = seq()._dsiIconSequence[seqIdx]._bitmapIndex;
        int palIdx = seq()._dsiIconSequence[seqIdx]._paletteIndex;
        bool flipH = seq()._dsiIconSequence[seqIdx]._flipH;
        bool flipV = seq()._dsiIconSequence[seqIdx]._flipV;

        _romInfo.drawDSiAnimatedRomIcon(position().x + 8, position().y + 24, bmpIdx, palIdx, flipH, flipV, selectedEngine());
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
        case KeyMessage::UI_KEY_X:
            if (_buttonCheats.isVisible())
            {
                pressCheats();
            }
            // else if (_buttonFlash.isVisible())
            // {
            //     pressFlash();
            // }
            ret = true;
            break;
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

    SettingWnd settingWnd(0, 0, 252, 188, this, LANG("game settings", "Per Game Settings"));

    PerGameSettings settingsIni(_filenameText);
    std::vector<std::string> _values;

    if (!_romInfo.isHomebrew())
    {
		if (ms().useBootstrap || !ms().secondaryDevice) {
			_values.push_back(LANG("game settings", "Default")); // -2 => 0
			_values.push_back(LANG("game settings", "System")); // -1 => 1
			_values.push_back(LANG("game settings", "Japanese"));
			_values.push_back(LANG("game settings", "English"));
			_values.push_back(LANG("game settings", "French"));
			_values.push_back(LANG("game settings", "German"));
			_values.push_back(LANG("game settings", "Italian"));
			_values.push_back(LANG("game settings", "Spanish"));
			_values.push_back(LANG("game settings", "Chinese"));
			_values.push_back(LANG("game settings", "Korean"));

			settingWnd.addSettingItem(LANG("game settings", "Language"), _values, settingsIni.language + 2); // Default is -2
			_values.clear();

			_values.push_back("0");
			_values.push_back("1");
			_values.push_back("2");
			_values.push_back("3");
			_values.push_back("4");
			_values.push_back("5");
			_values.push_back("6");
			_values.push_back("7");
			_values.push_back("8");
			_values.push_back("9");

			settingWnd.addSettingItem(LANG("game settings", "Save number"), _values, settingsIni.saveNo);
			_values.clear();
		}

		if ((isDSiMode() && ms().useBootstrap) || !ms().secondaryDevice) {
			_values.push_back(LANG("game settings", "Default")); // -1 => 0
			_values.push_back(LANG("game settings", "DS mode")); // 0 => 1
			_values.push_back(LANG("game settings", "DSi mode")); // 1 => 2
			_values.push_back(LANG("game settings", "DSi mode (Forced)")); // 2 => 3

			settingWnd.addSettingItem(LANG("game settings", "Run in"), _values, settingsIni.dsiMode + 1);
			_values.clear();
		}

		if ((REG_SCFG_EXT != 0) || !ms().secondaryDevice) {
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
		}

		if (ms().useBootstrap || !ms().secondaryDevice) {
			if ((isDSiMode() || !ms().secondaryDevice) && _romInfo.saveInfo().arm9destination != 0x02004000
			&& _romInfo.saveInfo().gameSdkVersion >= 0x2008000 && _romInfo.saveInfo().gameSdkVersion < 0x5000000) {
				_values.push_back(LANG("game settings", "Auto")); // -1 => 0
				_values.push_back(LANG("game settings", "Off")); // 0 => 1
				_values.push_back(LANG("game settings", "On")); // 1 => 2            

				settingWnd.addSettingItem(LANG("game settings", "Heap Shrink"), _values, settingsIni.heapShrink + 1);
				_values.clear();
			}

			_values.push_back(LANG("game settings", "Default")); // -1 => 0
			_values.push_back(LANG("game settings", "Release")); // 0 => 1
			_values.push_back(LANG("game settings", "Nightly")); // 1 => 2            

			settingWnd.addSettingItem(LANG("game settings", "Bootstrap File"), _values, settingsIni.bootstrapFile + 1);
			_values.clear();
		}
    }
    else
    {

        _values.push_back(LANG("game settings", "Default"));
        _values.push_back(LANG("game settings", "No"));
        _values.push_back(LANG("game settings", "Yes"));

        settingWnd.addSettingItem(LANG("game settings", "Direct Boot"), _values, settingsIni.directBoot + 1);
        _values.clear();

		if (!ms().secondaryDevice) {
			_values.push_back("None");
			_values.push_back("0");
			_values.push_back("1");
			_values.push_back("2");
			_values.push_back("3");
			_values.push_back("4");
			_values.push_back("5");
			_values.push_back("6");
			_values.push_back("7");
			_values.push_back("8");
			_values.push_back("9");

			settingWnd.addSettingItem(LANG("game settings", "RAM disk"), _values, settingsIni.ramDiskNo + 1);
			_values.clear();
		}

		if (isDSiMode() || !ms().secondaryDevice) {
			_values.push_back(LANG("game settings", "Default")); // -1 => 0
			_values.push_back(LANG("game settings", "DS mode")); // 0 => 1
			_values.push_back(LANG("game settings", "DSi mode")); // 1 => 2

			settingWnd.addSettingItem(LANG("game settings", "Run in"), _values, settingsIni.dsiMode + 1);
			_values.clear();
		}

		if ((REG_SCFG_EXT != 0) || !ms().secondaryDevice) {
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
		}

		if (!ms().secondaryDevice) {
			_values.push_back(LANG("game settings", "Default")); // -1 => 0
			_values.push_back(LANG("game settings", "Release")); // 0 => 1
			_values.push_back(LANG("game settings", "Nightly")); // 1 => 2            

			settingWnd.addSettingItem(LANG("game settings", "Bootstrap File"), _values, settingsIni.bootstrapFile + 1);
			_values.clear();
		}
	}

    _settingWnd = &settingWnd;
    u32 ret = settingWnd.doModal();

    dbg_printf("SETRET %i\n", ret);
    if (ret == ID_OK)
    {
        dbg_printf("FN: %s\n", _filenameText.c_str());
		int selection = 0;
        if (!_romInfo.isHomebrew())
        {
			if (ms().useBootstrap || !ms().secondaryDevice) {
				settingsIni.language = (PerGameSettings::TLanguage)(settingWnd.getItemSelection(0, selection) - 2);
                selection++;
                settingsIni.saveNo = (int)settingWnd.getItemSelection(0, selection);
			}
            if (isDSiMode() || !ms().secondaryDevice) {
                selection++;
                settingsIni.dsiMode = (PerGameSettings::TDefaultBool)(settingWnd.getItemSelection(0, selection) - 1);
            }
            if ((REG_SCFG_EXT != 0) || !ms().secondaryDevice) {
                selection++;
                settingsIni.boostCpu = (PerGameSettings::TDefaultBool)(settingWnd.getItemSelection(0, selection) - 1);
                selection++;
                settingsIni.boostVram = (PerGameSettings::TDefaultBool)(settingWnd.getItemSelection(0, selection) - 1);
            }
			if (ms().useBootstrap || !ms().secondaryDevice) {
				selection++;
				settingsIni.heapShrink = (PerGameSettings::TDefaultBool)(settingWnd.getItemSelection(0, selection) - 1);
				selection++;
				settingsIni.bootstrapFile = (PerGameSettings::TDefaultBool)(settingWnd.getItemSelection(0, selection) - 1);
			}
        }
        else
        {
            settingsIni.directBoot = (PerGameSettings::TDefaultBool)(settingWnd.getItemSelection(0, selection) - 1);
			if (!ms().secondaryDevice) {
                selection++;
                settingsIni.ramDiskNo = (int)(settingWnd.getItemSelection(0, selection) - 1);
			}
            if (isDSiMode() || !ms().secondaryDevice) {
                selection++;
                settingsIni.dsiMode = (PerGameSettings::TDefaultBool)(settingWnd.getItemSelection(0, selection) - 1);
			}
            if ((REG_SCFG_EXT != 0) || !ms().secondaryDevice) {
                selection++;
                settingsIni.boostCpu = (PerGameSettings::TDefaultBool)(settingWnd.getItemSelection(0, selection) - 1);
                selection++;
                settingsIni.boostVram = (PerGameSettings::TDefaultBool)(settingWnd.getItemSelection(0, selection) - 1);
            }
			if (ms().useBootstrap || !ms().secondaryDevice) {
				selection++;
				settingsIni.bootstrapFile = (PerGameSettings::TDefaultBool)(settingWnd.getItemSelection(0, selection) - 1);
			}
		}
        settingsIni.saveSettings();
    }
    _settingWnd = NULL;
    if (ret == ID_CANCEL)
        return;
}

void RomInfoWnd::pressCheats(void)
{
  if (!isDSiMode()||!_romInfo.isDSRom()||_romInfo.isHomebrew()) return;
  showCheats(_fullName);
}

void RomInfoWnd::showCheats(const std::string& aFileName)
{
  u32 w=256;
  u32 h=179;

  CheatWnd cheatWnd((256-w)/2,(192-h)/2,w,h,NULL,LANG("cheats","title"));
  if (cheatWnd.parse(aFileName)) cheatWnd.doModal();
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
        sizeUnit = " B";
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
    auto bannerTitle = _romInfo.banner().title;
    _romInfoText = unicode_to_local_string(bannerTitle, 128, NULL);

    _buttonGameSettings.hide();

    if (_romInfo.isDSRom() && !_romInfo.isHomebrew())
    {
        addCode();
    }
    if (!_romInfo.isDSiWare() && (_romInfo.isDSRom() || _romInfo.isHomebrew()))
    {
        _buttonGameSettings.show();
    }

    _buttonCheats.hide();

    if (!_romInfo.isDSiWare() && _romInfo.isDSRom())
    {
        _buttonCheats.show();
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
    tonccpy(gameCode, _romInfo.saveInfo().gameCode, sizeof(_romInfo.saveInfo().gameCode));
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