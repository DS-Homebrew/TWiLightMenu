/*
    mainwnd.cpp
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

#include "tool/dbgtool.h"
#include "ui/windowmanager.h"
#include "mainwnd.h"
#include "ui/msgbox.h"
#include "systemfilenames.h"

#include "time/datetime.h"
#include "time/timer.h"

#include "tool/timetool.h"
#include "tool/fifotool.h"

#include "ui/progresswnd.h"
#include "common/bootstrapconfig.h"
#include "common/loaderconfig.h"
#include "common/pergamesettings.h"
#include "common/cardlaunch.h"
#include "common/systemdetails.h"
#include "common/dsargv.h"
#include "common/flashcard.h"
#include "common/flashcardlaunch.h"
#include "common/gbaswitch.h"
#include "common/unlaunchboot.h"
#include "common/files.h"
#include "common/filecopy.h"
#include "common/nds_loader_arm9.h"
#include "incompatibleGameMap.h"

#include "common/inifile.h"
#include "language.h"
#include "common/dsimenusettings.h"
#include "windows/rominfownd.h"

#include "sr_data_srllastran.h"

#include <nds/arm9/dldi.h>
#include <sys/iosupport.h>

using namespace akui;

MainWnd::MainWnd(s32 x, s32 y, u32 w, u32 h, Window *parent, const std::string &text)
    : Form(x, y, w, h, parent, text), _mainList(NULL), _startMenu(NULL), _startButton(NULL),
      _brightnessButton(NULL), _batteryIcon(NULL), _folderUpButton(NULL),  _folderText(NULL), _processL(false)
{
}

MainWnd::~MainWnd()
{
    delete _folderText;
    delete _folderUpButton;
    delete _brightnessButton;
    delete _batteryIcon;
    delete _startButton;
    delete _startMenu;
    delete _mainList;
    windowManager().removeWindow(this);
}

void MainWnd::init()
{
    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;
    bool showBatt = 0;
    COLOR color = 0;
    std::string file("");
    std::string text("");
    CIniFile ini(SFN_UI_SETTINGS);

    // self init
    dbg_printf("mainwnd init() %08x\n", this);
    loadAppearance(SFN_LOWER_SCREEN_BG);
    windowManager().addWindow(this);

    // init game file list
    _mainList = new MainList(4, 20, 248, 152, this, "main list");
    _mainList->setRelativePosition(Point(4, 20));
    _mainList->init();
    _mainList->selectChanged.connect(this, &MainWnd::listSelChange);
    _mainList->selectedRowClicked.connect(this, &MainWnd::onMainListSelItemClicked);
    _mainList->directoryChanged.connect(this, &MainWnd::onFolderChanged);
    _mainList->animateIcons.connect(this, &MainWnd::onAnimation);

    addChildWindow(_mainList);
    dbg_printf("mainlist %08x\n", _mainList);

    //waitMs( 1000 );

    // init start button
    x = ini.GetInt("start button", "x", 0);
    y = ini.GetInt("start button", "y", 172);
    w = ini.GetInt("start button", "w", 48);
    h = ini.GetInt("start button", "h", 10);
    color = ini.GetInt("start button", "textColor", 0x7fff);
    file = ini.GetString("start button", "file", "none");
    text = ini.GetString("start button", "text", "START");
    if (file != "none")
    {
        file = SFN_UI_CURRENT_DIRECTORY + file;
    }
    if (text == "ini")
    {
        text = LANG("start menu", "START");
    }
    _startButton = new Button(x, y, w, h, this, text);
    _startButton->setStyle(Button::press);
    _startButton->setRelativePosition(Point(x, y));
    _startButton->loadAppearance(file);
    _startButton->clicked.connect(this, &MainWnd::startButtonClicked);
    _startButton->setTextColor(color | BIT(15));
    if (!ini.GetInt("start button", "show", 1))
        _startButton->hide();
    addChildWindow(_startButton);

    // // init brightness button
	
	    x = ini.GetInt("battery icon", "x", 238);
	    y = ini.GetInt("battery icon", "y", 172);
	    showBatt = ini.GetInt("battery icon", "show", 0);

	
	    if (showBatt)
	    {
            if (!ini.GetInt("battery icon", "screen", true))
            {
                _batteryIcon = new Button(x, y, w, h, this, "");
                _batteryIcon->setRelativePosition(Point(x,y));

                u8 batteryLevel = sys().batteryStatus();

				if (isDSiMode()) {
					if (batteryLevel & BIT(7)) {
						_batteryIcon->loadAppearance(SFN_BATTERY_CHARGE);
					} else if (batteryLevel == 0xF) {
						_batteryIcon->loadAppearance(SFN_BATTERY4);
					} else if (batteryLevel == 0xB) {
						_batteryIcon->loadAppearance(SFN_BATTERY3);
					} else if (batteryLevel == 0x7) {
						_batteryIcon->loadAppearance(SFN_BATTERY2);
					} else if (batteryLevel == 0x3 || batteryLevel == 0x1) {
						_batteryIcon->loadAppearance(SFN_BATTERY1);
					} else {
						_batteryIcon->loadAppearance(SFN_BATTERY_CHARGE);
					}
				} else {
					if (batteryLevel & BIT(0)) {
						_batteryIcon->loadAppearance(SFN_BATTERY1);
					} else {
						_batteryIcon->loadAppearance(SFN_BATTERY4);
					}
				}
        
                addChildWindow(_batteryIcon);
            }
	    }

    x = ini.GetInt("folderup btn", "x", 0);
    y = ini.GetInt("folderup btn", "y", 2);
    w = ini.GetInt("folderup btn", "w", 32);
    h = ini.GetInt("folderup btn", "h", 16);
    _folderUpButton = new Button(x, y, w, h, this, "");
    _folderUpButton->setRelativePosition(Point(x, y));
    _folderUpButton->loadAppearance(SFN_FOLDERUP_BUTTON);
    _folderUpButton->setSize(Size(w, h));
    _folderUpButton->pressed.connect(_mainList, &MainList::backParentDir);
    addChildWindow(_folderUpButton);

    x = ini.GetInt("folder text", "x", 20);
    y = ini.GetInt("folder text", "y", 2);
    w = ini.GetInt("folder text", "w", 160);
    h = ini.GetInt("folder text", "h", 16);
    _folderText = new StaticText(x, y, w, h, this, "");
    _folderText->setRelativePosition(Point(x, y));
    _folderText->setTextColor(ini.GetInt("folder text", "color", 0));
    addChildWindow(_folderText);
    
    if (!ms().showDirectories) {
        _folderText->hide();
    }
    // init startmenu
    _startMenu = new StartMenu(160, 40, 61, 108, this, "start menu");
    _startMenu->init();
    _startMenu->itemClicked.connect(this, &MainWnd::startMenuItemClicked);
    _startMenu->hide();
    _startMenu->setRelativePosition(_startMenu->position());
    addChildWindow(_startMenu);
    dbg_printf("startMenu %08x\n", _startMenu);

    arrangeChildren();
}

void MainWnd::draw()
{
    Form::draw();
}

void MainWnd::listSelChange(u32 i)
{
    // #ifdef DEBUG
    //     //dbg_printf( "main list item %d\n", i );
    //     DSRomInfo info;
    //     if (_mainList->getRomInfo(i, info))
    //     {
    //         char title[13] = {};
    //         memcpy(title, info.saveInfo().gameTitle, 12);
    //         char code[5] = {};
    //         memcpy(code, info.saveInfo().gameCode, 4);
    //         u16 crc = swiCRC16(0xffff, ((unsigned char *)&(info.banner())) + 32, 0x840 - 32);
    //         dbg_printf("%s %s %04x %d %04x/%04x\n",
    //                    title, code, info.saveInfo().gameCRC, info.isDSRom(), info.banner().crc, crc);
    //         //dbg_printf("sizeof banner %08x\n", sizeof( info.banner() ) );
    //     }
    // #endif //DEBUG
}

void MainWnd::startMenuItemClicked(s16 i)
{
    CIniFile ini(SFN_UI_SETTINGS);
    if (!ini.GetInt("start menu", "showFileOperations", true)) i += 4;
    
    dbg_printf("start menu item %d\n", i);

    // ------------------- Copy and Paste ---
    if (START_MENU_ITEM_COPY == i)
    {
        if (_mainList->getSelectedFullPath() == "")
            return;
        struct stat st;
        stat(_mainList->getSelectedFullPath().c_str(), &st);
        if (st.st_mode & S_IFDIR)
        {
            messageBox(this, LANG("no copy dir", "title"), LANG("no copy dir", "text"), MB_YES | MB_NO);
            return;
        }
        setSrcFile(_mainList->getSelectedFullPath(), SFM_COPY);
    }

    else if (START_MENU_ITEM_CUT == i)
    {
        if (_mainList->getSelectedFullPath() == "")
            return;
        struct stat st;
        stat(_mainList->getSelectedFullPath().c_str(), &st);
        if (st.st_mode & S_IFDIR)
        {
            messageBox(this, LANG("no copy dir", "title"), LANG("no copy dir", "text"), MB_YES | MB_NO);
            return;
        }
        setSrcFile(_mainList->getSelectedFullPath(), SFM_CUT);
    }

    else if (START_MENU_ITEM_PASTE == i)
    {
        bool ret = false;
        ret = copyOrMoveFile(_mainList->getCurrentDir());
        if (ret) // refresh current directory
            _mainList->enterDir(_mainList->getCurrentDir());
    }

    else if (START_MENU_ITEM_HIDE == i)
    {
        std::string fullPath = _mainList->getSelectedFullPath();
        if (fullPath != "")
        {
            bool ret = false;
            ret = hideFile(fullPath);
            if (ret)
                _mainList->enterDir(_mainList->getCurrentDir());
        }
    }

    else if (START_MENU_ITEM_DELETE == i)
    {
        std::string fullPath = _mainList->getSelectedFullPath();
        if (fullPath != "" && !ms().preventDeletion)
        {
            bool ret = false;
            ret = deleteFile(fullPath);
            if (ret)
                _mainList->enterDir(_mainList->getCurrentDir());
        }
    }

    if (START_MENU_ITEM_SETTING == i)
    {
        showSettings();
    }

    else if (START_MENU_ITEM_INFO == i)
    {
        showFileInfo();
    }
}

void MainWnd::startButtonClicked()
{
    if (_startMenu->isVisible())
    {
        _startMenu->hide();
    }
    else
    {
        _startMenu->show();
    }
}

Window &MainWnd::loadAppearance(const std::string &aFileName)
{
    return *this;
}

bool MainWnd::process(const Message &msg)
{
    if (_startMenu->isVisible())
        return _startMenu->process(msg);

    bool ret = false;

    ret = Form::process(msg);

    if (!ret)
    {
        if (msg.id() > Message::keyMessageStart && msg.id() < Message::keyMessageEnd)
        {
            ret = processKeyMessage((KeyMessage &)msg);
        }

        if (msg.id() > Message::touchMessageStart && msg.id() < Message::touchMessageEnd)
        {
            ret = processTouchMessage((TouchMessage &)msg);
        }
    }
    return ret;
}

bool MainWnd::processKeyMessage(const KeyMessage &msg)
{
    bool ret = false, isL = msg.shift() & KeyMessage::UI_SHIFT_L;
    if (msg.id() == Message::keyDown)
    {
        switch (msg.keyCode())
        {
        case KeyMessage::UI_KEY_DOWN:
            _mainList->selectNext();
            ret = true;
            break;
        case KeyMessage::UI_KEY_UP:
            _mainList->selectPrev();
            ret = true;
            break;

        case KeyMessage::UI_KEY_LEFT:
            _mainList->selectRow(_mainList->selectedRowId() - _mainList->visibleRowCount());
            ret = true;
            break;

        case KeyMessage::UI_KEY_RIGHT:
            _mainList->selectRow(_mainList->selectedRowId() + _mainList->visibleRowCount());
            ret = true;
            break;
        case KeyMessage::UI_KEY_A:
            onKeyAPressed();
            ret = true;
            break;
        case KeyMessage::UI_KEY_B:
            onKeyBPressed();
            ret = true;
            break;
        case KeyMessage::UI_KEY_Y:
            if (isL)
            {
                showSettings();
                _processL = false;
            }
            else
            {
                onKeyYPressed();
            }
            ret = true;
            break;

        case KeyMessage::UI_KEY_START:
            startButtonClicked();
            ret = true;
            break;
        case KeyMessage::UI_KEY_SELECT:
            if (isL)
            {
                _mainList->SwitchShowAllFiles();
                _processL = false;
            }
            else
            {
                _mainList->setViewMode((MainList::VIEW_MODE)((_mainList->getViewMode() + 1) % 3));
                ms().ak_viewMode = _mainList->getViewMode();
                ms().saveSettings();
            }
            ret = true;
            break;
        case KeyMessage::UI_KEY_L:
            _processL = true;
            ret = true;
            break;
        case KeyMessage::UI_KEY_R:
#ifdef DEBUG
            gdi().switchSubEngineMode();
            gdi().present(GE_SUB);
#endif //DEBUG
            ret = true;
            break;
        default:
        {
        }
        };
    }
    if (msg.id() == Message::keyUp)
    {
        switch (msg.keyCode())
        {
        case KeyMessage::UI_KEY_L:
            if (_processL)
            {
                _mainList->backParentDir();
                _processL = false;
            }
            ret = true;
            break;
        }
    }
    return ret;
}

bool MainWnd::processTouchMessage(const TouchMessage &msg)
{
    return false;
}

void MainWnd::onKeyYPressed()
{
    showFileInfo();
}

void MainWnd::onMainListSelItemClicked(u32 index)
{
    onKeyAPressed();
}

void MainWnd::onKeyAPressed()
{
    cwl();
    launchSelected();
}

void bootstrapSaveHandler()
{
    progressWnd().setPercent(50);
    progressWnd().update();
}

void bootstrapLaunchHandler()
{
    progressWnd().setPercent(90);
    progressWnd().update();
}

void MainWnd::bootArgv(DSRomInfo &rominfo)
{
    std::string fullPath = _mainList->getSelectedFullPath();
    std::string launchPath = fullPath;
    std::vector<const char *> cargv{};
    if (rominfo.isArgv())
    {
        ArgvFile argv(fullPath);
        launchPath = argv.launchPath();
        for (auto &string : argv.launchArgs())
            cargv.push_back(&string.front());
    }

    LoaderConfig config(fullPath, "");
    progressWnd().setTipText(LANG("game launch", "Please wait"));
    progressWnd().update();
    progressWnd().show();

    int err = config.launch(0, cargv.data());

    if (err)
    {
        std::string errorString = formatString(LANG("game launch", "error").c_str(), err);
        messageBox(this, LANG("game launch", "ROM Start Error"), errorString, MB_OK);
        progressWnd().hide();
    }
}

//void MainWnd::apFix(const char *filename)
std::string apFix(const char *filename, bool isHomebrew)
{
	if (flashcardFound()) {
		remove("fat:/_nds/nds-bootstrap/apFix.ips");
	}

	if (isHomebrew) {
		return "";
	}

	bool ipsFound = false;
	char ipsPath[256];
	snprintf(ipsPath, sizeof(ipsPath), "%s:/_nds/TWiLightMenu/extras/apfix/%s.ips", sdFound() ? "sd" : "fat", filename);
	ipsFound = (access(ipsPath, F_OK) == 0);

	char game_TID[5];
	u16 headerCRC16 = 0;

	if (!ipsFound) {
		FILE *f_nds_file = fopen(filename, "rb");

		fseek(f_nds_file, offsetof(sNDSHeaderExt, gameCode), SEEK_SET);
		fread(game_TID, 1, 4, f_nds_file);
		fseek(f_nds_file, offsetof(sNDSHeaderExt, headerCRC16), SEEK_SET);
		fread(&headerCRC16, sizeof(u16), 1, f_nds_file);
		fclose(f_nds_file);
		game_TID[4] = 0;

		snprintf(ipsPath, sizeof(ipsPath), "%s:/_nds/TWiLightMenu/extras/apfix/%s-%X.ips", sdFound() ? "sd" : "fat", game_TID, headerCRC16);
		ipsFound = (access(ipsPath, F_OK) == 0);
	}

	if (ipsFound) {
		if (ms().secondaryDevice && sdFound()) {
			mkdir("fat:/_nds", 0777);
			mkdir("fat:/_nds/nds-bootstrap", 0777);
			fcopy(ipsPath, "fat:/_nds/nds-bootstrap/apFix.ips");
			return "fat:/_nds/nds-bootstrap/apFix.ips";
		}
		return ipsPath;
	} else {
		bool cheatVer = false;

		FILE *file = fopen(sdFound() ? "sd:/_nds/TWiLightMenu/extras/apfix.pck" : "fat:/_nds/TWiLightMenu/extras/apfix.pck", "rb");
		if (file) {
			char buf[5] = {0};
			fread(buf, 1, 4, file);
			if (strcmp(buf, ".PCK") != 0) // Invalid file
				return "";

			u32 fileCount;
			fread(&fileCount, 1, sizeof(fileCount), file);

			u32 offset = 0, size = 0;

			// Try binary search for the game
			int left = 0;
			int right = fileCount;

			while (left <= right) {
				int mid = left + ((right - left) / 2);
				fseek(file, 16 + mid * 16, SEEK_SET);
				fread(buf, 1, 4, file);
				nocashMessage(buf);
				int cmp = strcmp(buf, game_TID);
				if (cmp == 0) { // TID matches, check CRC
					u16 crc;
					fread(&crc, 1, sizeof(crc), file);

					if (crc == headerCRC16) { // CRC matches
						fread(&offset, 1, sizeof(offset), file);
						fread(&size, 1, sizeof(size), file);
						cheatVer = fgetc(file) & 1;
						break;
					} else if (crc < headerCRC16) {
						left = mid + 1;
					} else {
						right = mid - 1;
					}
				} else if (cmp < 0) {
					left = mid + 1;
				} else {
					right = mid - 1;
				}
			}

			if (offset > 0 && size > 0) {
				fseek(file, offset, SEEK_SET);
				u8 *buffer = new u8[size];
				fread(buffer, 1, size, file);

				if (flashcardFound()) {
					mkdir("fat:/_nds", 0777);
					mkdir("fat:/_nds/nds-bootstrap", 0777);
				}
				snprintf(ipsPath, sizeof(ipsPath), "%s:/_nds/nds-bootstrap/apFix%s", ms().secondaryDevice ? "fat" : "sd", cheatVer ? "Cheat.bin" : ".ips");
				FILE *out = fopen(ipsPath, "wb");
				if (out) {
					fwrite(buffer, 1, size, out);
					fclose(out);
				}
				delete[] buffer;
				fclose(file);
				return ipsPath;
			}

			fclose(file);
		}
	}

	return "";
}

bool checkIfAPPatch(const char *filename) {
	char game_TID[5];
	u16 headerCRC16 = 0;

	FILE *f_nds_file = fopen(filename, "rb");

	fseek(f_nds_file, offsetof(sNDSHeaderExt, gameCode), SEEK_SET);
	fread(game_TID, 1, 4, f_nds_file);
	fseek(f_nds_file, offsetof(sNDSHeaderExt, headerCRC16), SEEK_SET);
	fread(&headerCRC16, sizeof(u16), 1, f_nds_file);
	fclose(f_nds_file);
	game_TID[4] = 0;

	FILE *file = fopen(sdFound() ? "sd:/_nds/TWiLightMenu/extras/apfix.pck" : "fat:/_nds/TWiLightMenu/extras/apfix.pck", "rb");
	if (file) {
		char buf[5] = {0};
		fread(buf, 1, 4, file);
		if (strcmp(buf, ".PCK") != 0) // Invalid file
			return "";

		u32 fileCount;
		fread(&fileCount, 1, sizeof(fileCount), file);

		// Try binary search for the game
		int left = 0;
		int right = fileCount;

		while (left <= right) {
			int mid = left + ((right - left) / 2);
			fseek(file, 16 + mid * 16, SEEK_SET);
			fread(buf, 1, 4, file);
			int cmp = strcmp(buf, game_TID);
			if (cmp == 0) { // TID matches, check CRC
				u16 crc;
				fread(&crc, 1, sizeof(crc), file);

				if (crc == headerCRC16) { // CRC matches
					fclose(file);
					return true;
				} else if (crc < headerCRC16) {
					left = mid + 1;
				} else {
					right = mid - 1;
				}
			} else if (cmp < 0) {
				left = mid + 1;
			} else {
				right = mid - 1;
			}
		}

		fclose(file);
	}

	return false;
}

sNDSHeader ndsCart;

//void MainWnd::bootWidescreen(const char *filename)
void bootWidescreen(const char *filename, bool isHomebrew, bool useWidescreen)
{
	remove("/_nds/nds-bootstrap/wideCheatData.bin");

	if (sys().arm7SCFGLocked() || ms().consoleModel < 2 || !useWidescreen
	|| (access("sd:/_nds/TWiLightMenu/TwlBg/Widescreen.cxi", F_OK) != 0)) {
		return;
	}
	
	bool wideCheatFound = false;
	char wideBinPath[256];
	if (ms().launchType[ms().secondaryDevice] == TWLSettings::ESDFlashcardLaunch) {
		snprintf(wideBinPath, sizeof(wideBinPath), "sd:/_nds/TWiLightMenu/extras/widescreen/%s.bin", filename);
		wideCheatFound = (access(wideBinPath, F_OK) == 0);
	}

	char game_TID[5];
	u16 headerCRC16 = 0;

	if (ms().slot1Launched) {
		// Reset Slot-1 to allow reading card header
		sysSetCardOwner (BUS_OWNER_ARM9);
		disableSlot1();
		for (int i = 0; i < 25; i++) { swiWaitForVBlank(); }
		enableSlot1();
		for (int i = 0; i < 15; i++) { swiWaitForVBlank(); }

		cardReadHeader((uint8*)&ndsCart);

		memcpy(game_TID, ndsCart.gameCode, 4);
		game_TID[4] = 0;
		headerCRC16 = ndsCart.headerCRC16;

		snprintf(wideBinPath, sizeof(wideBinPath), "sd:/_nds/TWiLightMenu/extras/widescreen/%s-%X.bin", game_TID, ndsCart.headerCRC16);
		wideCheatFound = (access(wideBinPath, F_OK) == 0);
	} else if (!wideCheatFound) {
		FILE *f_nds_file = fopen(filename, "rb");

		fseek(f_nds_file, offsetof(sNDSHeaderExt, gameCode), SEEK_SET);
		fread(game_TID, 1, 4, f_nds_file);
		fseek(f_nds_file, offsetof(sNDSHeaderExt, headerCRC16), SEEK_SET);
		fread(&headerCRC16, sizeof(u16), 1, f_nds_file);
		fclose(f_nds_file);
		game_TID[4] = 0;

		snprintf(wideBinPath, sizeof(wideBinPath), "sd:/_nds/TWiLightMenu/extras/widescreen/%s-%X.bin", game_TID, headerCRC16);
		wideCheatFound = (access(wideBinPath, F_OK) == 0);
	}

	if (isHomebrew) {
		if (!ms().homebrewHasWide) return;

		// Prepare for reboot into 16:10 TWL_FIRM
		mkdir("sd:/luma", 0777);
		mkdir("sd:/luma/sysmodules", 0777);
		if ((access("sd:/luma/sysmodules/TwlBg.cxi", F_OK) == 0)
		&& (rename("sd:/luma/sysmodules/TwlBg.cxi", "sd:/luma/sysmodules/TwlBg_bak.cxi") != 0)) {
			//resultText = "Failed to backup custom TwlBg.";
		} else {
			if (fcopy("sd:/_nds/TWiLightMenu/TwlBg/Widescreen.cxi", "sd:/luma/sysmodules/TwlBg.cxi") == 0) {
				irqDisable(IRQ_VBLANK);				// Fix the throwback to 3DS HOME Menu bug
				memcpy((u32 *)0x02000300, sr_data_srllastran, 0x020);
				fifoSendValue32(FIFO_USER_02, 1); // Reboot in 16:10 widescreen
				swiWaitForVBlank();
			} else {
				//resultText = "Failed to reboot TwlBg in widescreen.";
			}
		}
		return;
	}

	if (wideCheatFound) {
		//const char* resultText;
		mkdir("/_nds", 0777);
		mkdir("/_nds/nds-bootstrap", 0777);
		if (fcopy(wideBinPath, "/_nds/nds-bootstrap/wideCheatData.bin") == 0) {
			// Prepare for reboot into 16:10 TWL_FIRM
			mkdir("sd:/luma", 0777);
			mkdir("sd:/luma/sysmodules", 0777);
			if ((access("sd:/luma/sysmodules/TwlBg.cxi", F_OK) == 0)
			&& (rename("sd:/luma/sysmodules/TwlBg.cxi", "sd:/luma/sysmodules/TwlBg_bak.cxi") != 0)) {
				//resultText = "Failed to backup custom TwlBg.";
			} else {
				if (fcopy("sd:/_nds/TWiLightMenu/TwlBg/Widescreen.cxi", "sd:/luma/sysmodules/TwlBg.cxi") == 0) {
					irqDisable(IRQ_VBLANK);				// Fix the throwback to 3DS HOME Menu bug
					memcpy((u32 *)0x02000300, sr_data_srllastran, 0x020);
					fifoSendValue32(FIFO_USER_02, 1); // Reboot in 16:10 widescreen
					swiWaitForVBlank();
				} else {
					//resultText = "Failed to reboot TwlBg in widescreen.";
				}
			}
			rename("sd:/luma/sysmodules/TwlBg_bak.cxi", "sd:/luma/sysmodules/TwlBg.cxi");
		} else {
			//resultText = "Failed to copy widescreen code for the game.";
		}
		remove("/_nds/nds-bootstrap/wideCheatData.bin");
		//messageBox(this, LANG("game launch", "Widescreen Error"), resultText, MB_OK);	// Does not work
	} else {
		FILE *file = fopen(sdFound() ? "sd:/_nds/TWiLightMenu/extras/widescreen.pck" : "fat:/_nds/TWiLightMenu/extras/widescreen.pck", "rb");
		if (file) {
			char buf[5] = {0};
			fread(buf, 1, 4, file);
			if (strcmp(buf, ".PCK") != 0) // Invalid file
				return;

			u32 fileCount;
			fread(&fileCount, 1, sizeof(fileCount), file);

			u32 offset = 0, size = 0;

			// Try binary search for the game
			int left = 0;
			int right = fileCount;

			while (left <= right) {
				int mid = left + ((right - left) / 2);
				fseek(file, 16 + mid * 16, SEEK_SET);
				fread(buf, 1, 4, file);
				int cmp = strcmp(buf, game_TID);
				if (cmp == 0) { // TID matches, check CRC
					u16 crc;
					fread(&crc, 1, sizeof(crc), file);

					if (crc == headerCRC16) { // CRC matches
						fread(&offset, 1, sizeof(offset), file);
						fread(&size, 1, sizeof(size), file);
						break;
					} else if (crc < headerCRC16) {
						left = mid + 1;
					} else {
						right = mid - 1;
					}
				} else if (cmp < 0) {
					left = mid + 1;
				} else {
					right = mid - 1;
				}
			}

			if (offset > 0 && size > 0) {
				fseek(file, offset, SEEK_SET);
				u8 *buffer = new u8[size];
				fread(buffer, 1, size, file);

				if (flashcardFound()) {
					mkdir("fat:/_nds", 0777);
					mkdir("fat:/_nds/nds-bootstrap", 0777);
				}
				snprintf(wideBinPath, sizeof(wideBinPath), "%s:/_nds/nds-bootstrap/wideCheatData.bin", ms().secondaryDevice ? "fat" : "sd");
				FILE *out = fopen(wideBinPath, "wb");
				if (out) {
					fwrite(buffer, 1, size, out);
					fclose(out);
				}
				delete[] buffer;
				fclose(file);
				return;
			}

			fclose(file);
		}
	}
}

void MainWnd::bootBootstrap(PerGameSettings &gameConfig, DSRomInfo &rominfo)
{
    dbg_printf("%s", _mainList->getSelectedShowName().c_str());
    std::string fileName = _mainList->getSelectedShowName();
    std::string fullPath = _mainList->getSelectedFullPath();

    BootstrapConfig config(fileName, fullPath, std::string((char *)rominfo.saveInfo().gameCode), rominfo.saveInfo().gameSdkVersion, gameConfig.heapShrink);

    config.dsiMode(rominfo.isDSiWare() ? true : (gameConfig.dsiMode == PerGameSettings::EDefault ? DEFAULT_DSI_MODE : (int)gameConfig.dsiMode))
		  .saveNo((int)gameConfig.saveNo)
		  .ramDiskNo((int)gameConfig.ramDiskNo)
		  .cpuBoost(gameConfig.boostCpu == PerGameSettings::EDefault ? DEFAULT_BOOST_CPU : (bool)gameConfig.boostCpu)
		  .vramBoost(gameConfig.boostVram == PerGameSettings::EDefault ? DEFAULT_BOOST_VRAM : (bool)gameConfig.boostVram)
		  .nightlyBootstrap(gameConfig.bootstrapFile == PerGameSettings::EDefault ? ms().bootstrapFile : (bool)gameConfig.bootstrapFile)
		  .wideScreen(gameConfig.wideScreen == PerGameSettings::EDefault ? ms().wideScreen : (bool)gameConfig.wideScreen);

    // GameConfig is default, global is not default
    if (gameConfig.language == PerGameSettings::ELangDefault && ms().gameLanguage != TWLSettings::ELangDefault)
    {
        config.language(ms().gameLanguage);
    }
    // GameConfig is system, or global is defaut
    else if (gameConfig.language == PerGameSettings::ELangSystem || ms().gameLanguage == TWLSettings::ELangDefault)
    {
        config.language(-1);
    }
    else
    // gameConfig is not default
    {
        config.language(gameConfig.language);
    }

	char gameTid[5] = {0};
	tonccpy(gameTid, rominfo.saveInfo().gameCode, 4);

	FILE *f_nds_file = fopen(fullPath.c_str(), "rb");
	u16 headerCRC16 = 0;
	fseek(f_nds_file, offsetof(sNDSHeaderExt, headerCRC16), SEEK_SET);
	fread(&headerCRC16, sizeof(u16), 1, f_nds_file);
	fclose(f_nds_file);

	if (!rominfo.isDSiWare()) {
		bool proceedToLaunch = true;

		if (!isDSiMode() && ms().secondaryDevice) {
			// TODO: If the list gets large enough, switch to bsearch().
			for (unsigned int i = 0; i < sizeof(incompatibleGameListB4DS)/sizeof(incompatibleGameListB4DS[0]); i++) {
				if (memcmp(gameTid, incompatibleGameListB4DS[i], 3) == 0) {
					// Found match
					proceedToLaunch = false;
					break;
				}
			}
		}

		if (proceedToLaunch) {
			// TODO: If the list gets large enough, switch to bsearch().
			for (unsigned int i = 0; i < sizeof(incompatibleGameList)/sizeof(incompatibleGameList[0]); i++) {
				if (memcmp(gameTid, incompatibleGameList[i], 3) == 0) {
					// Found match
					proceedToLaunch = false;
					break;
				}
			}
		}

		if (!proceedToLaunch) {
			int optionPicked = messageBox(this, LANG("game launch", "Compatibility Warning"), "This game is known to not run. If there's an nds-bootstrap version that fixes this, please ignore this message.", MB_OK | MB_CANCEL);
			progressWnd().hide();

			scanKeys();
			int pressed = keysHeld();

			if (pressed & KEY_B || optionPicked == ID_CANCEL)
			{
				return;
			}
		}
	}

	if (!rominfo.isDSiWare() && rominfo.requiresDonorRom()) {
		const char* pathDefine = "DONOR_NDS_PATH";
		const char* msg = "This game requires a donor ROM to run. Please switch the theme, and set an existing DS SDK5 game as a donor ROM.";
		if (rominfo.requiresDonorRom()==20) {
			pathDefine = "DONORE2_NDS_PATH";
			msg = "This game requires a donor ROM to run. Please switch the theme, and set an existing early SDK2 game as a donor ROM.";
		} else if (rominfo.requiresDonorRom()==2) {
			pathDefine = "DONOR2_NDS_PATH";
			msg = "This game requires a donor ROM to run. Please switch the theme, and set an existing late SDK2 game as a donor ROM.";
		} else if (rominfo.requiresDonorRom()==3) {
			pathDefine = "DONOR3_NDS_PATH";
			msg = "This game requires a donor ROM to run. Please switch the theme, and set an existing early SDK3 game as a donor ROM.";
		} else if (rominfo.requiresDonorRom()==51) {
			pathDefine = "DONORTWL_NDS_PATH";
			msg = "This game requires a donor ROM to run. Please switch the theme, and set an existing DSi-Enhanced game as a donor ROM.";
		}
		std::string donorRomPath;
		const char* bootstrapinipath = (sdFound() ? "sd:/_nds/nds-bootstrap.ini" : "fat:/_nds/nds-bootstrap.ini");
		CIniFile bootstrapini(bootstrapinipath);
		donorRomPath = bootstrapini.GetString("NDS-BOOTSTRAP", pathDefine, "");
		if (donorRomPath == "" || access(donorRomPath.c_str(), F_OK) != 0) {
			messageBox(this, LANG("game launch", "NDS Bootstrap Error"), msg, MB_OK);
			progressWnd().hide();
			return;
		}
	}

	if ((gameConfig.dsiMode == PerGameSettings::EDefault ? DEFAULT_DSI_MODE : (int)gameConfig.dsiMode)
	 && !rominfo.isDSiWare() && !rominfo.hasExtendedBinaries()) {
		messageBox(this, LANG("game launch", "NDS Bootstrap Error"), "The DSi binaries are missing. Please get a clean dump of this ROM, or start in DS mode.", MB_OK);
		progressWnd().hide();
		return;
	}

	bool hasAP = false;
	bool hasAP1 = false;
    PerGameSettings settingsIni(_mainList->getSelectedShowName().c_str());

	char ipsPath[256];
	sprintf(ipsPath, "%s:/_nds/TWiLightMenu/extras/apfix/%s-%X.ips", sdFound() ? "sd" : "fat", gameTid, headerCRC16);

	if (settingsIni.checkIfShowAPMsg() && !(access(ipsPath, F_OK) == 0 || checkIfAPPatch(_mainList->getSelectedFullPath().c_str()))) {
		// Check for SDK4-5 ROMs that don't have AP measures.
		if ((memcmp(rominfo.saveInfo().gameCode, "AZLJ", 4) == 0)	// Girls Mode (JAP version of Style Savvy)
		 || (memcmp(rominfo.saveInfo().gameCode, "YEEJ", 4) == 0)	// Inazuma Eleven (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "CNSX", 4) == 0)	// Naruto Shippuden: Naruto vs Sasuke (Europe)
		 || (memcmp(rominfo.saveInfo().gameCode, "BH2J", 4) == 0))	// Super Scribblenauts (Japan)
		{
			hasAP = false;
		}
		else
		// Check for ROMs that have AP measures.
		if ((memcmp(rominfo.saveInfo().gameCode, "VETP", 4) == 0)	// 1000 Cooking Recipes from Elle a Table (Europe)
		 || (memcmp(rominfo.saveInfo().gameCode, "CQQP", 4) == 0)	// AFL Mascot Manor (Australia)
		 || (memcmp(rominfo.saveInfo().gameCode, "CA5E", 4) == 0)	// Again: Interactive Crime Novel (USA)
		 || (memcmp(rominfo.saveInfo().gameCode, "TAKJ", 4) == 0)	// All Kamen Rider: Rider Generation (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "BKCE", 4) == 0)	// America's Test Kitchen: Let's Get Cooking (USA)
		 || (memcmp(rominfo.saveInfo().gameCode, "A3PJ", 4) == 0)	// Anpanman to Touch de Waku Waku Training (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "B2AK", 4) == 0)	// Aranuri: Badachingudeulkkwa hamkke Mandeuneun Sesang (Korea)
		 || (memcmp(rominfo.saveInfo().gameCode, "BB4J", 4) == 0)	// Battle Spirits Digital Starter (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "CYJJ", 4) == 0)	// Blood of Bahamut (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "TBSJ", 4) == 0)	// Byoutai Seiri DS: Image Dekiru! Shikkan, Shoujou to Care (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "C5YJ", 4) == 0)	// Chocobo to Mahou no Ehon: Majo to Shoujo to 5-nin no Yuusha (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "C6HK", 4) == 0)	// Chuldong! Rescue Force DS (Korea)
		 || (memcmp(rominfo.saveInfo().gameCode, "CCTJ", 4) == 0)	// Cid to Chocobo no Fushigi na Dungeon: Toki Wasure no Meikyuu DS+ (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "CLPD", 4) == 0)	// Club Penguin: Elite Penguin Force (Germany)
		 || (memcmp(rominfo.saveInfo().gameCode, "BQ6J", 4) == 0)	// Cocoro no Cocoron (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "BQIJ", 4) == 0)	// Cookin' Idol I! My! Mine!: Game de Hirameki! Kirameki! Cooking (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "B3CJ", 4) == 0)	// Cooking Mama 3 (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "TMCP", 4) == 0)	// Cooking Mama World: Combo Pack: Volume 1 (Europe)
		 || (memcmp(rominfo.saveInfo().gameCode, "TMDP", 4) == 0)	// Cooking Mama World: Combo Pack: Volume 2 (Europe)
		 || (memcmp(rominfo.saveInfo().gameCode, "BJ8P", 4) == 0)	// Cooking Mama World: Hobbies & Fun (Europe)
		 || (memcmp(rominfo.saveInfo().gameCode, "VCPJ", 4) == 0)	// Cosmetick Paradise: Kirei no Mahou (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "VCTJ", 4) == 0)	// Cosmetick Paradise: Princess Life (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "BQBJ", 4) == 0)	// Crayon Shin-chan: Obaka Dainin Den: Susume! Kasukabe Ninja Tai! (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "BUCJ", 4) == 0)	// Crayon Shin-chan: Shock Gahn!: Densetsu o Yobu Omake Daiketsusen!! (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "BDNJ", 4) == 0)	// Cross Treasures (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "TPGJ", 4) == 0)	// Dengeki Gakuen RPG: Cross of Venus Special (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "BLEJ", 4) == 0)	// Digimon Story: Lost Evolution (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "TBFJ", 4) == 0)	// Digimon Story: Super Xros Wars: Blue (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "TLTJ", 4) == 0)	// Digimon Story: Super Xros Wars: Red (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "BVIJ", 4) == 0)	// Dokonjou Shougakusei Bon Biita: Hadaka no Choujou Ketsusen!!: Biita vs Dokuro Dei! (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "TDBJ", 4) == 0)	// Dragon Ball Kai: Ultimate Butou Den (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "B2JJ", 4) == 0)	// Dragon Quest Monsters: Joker 2: Professional (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "YVKK", 4) == 0)	// DS Vitamin: Widaehan Bapsang: Malhaneun! Geongangyori Giljabi (Korea)
		 || (memcmp(rominfo.saveInfo().gameCode, "B3LJ", 4) == 0)	// Eigo de Tabisuru: Little Charo (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "VL3J", 4) == 0)	// Elminage II: Sousei no Megami to Unmei no Daichi: DS Remix (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "THMJ", 4) == 0)	// FabStyle (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "VI2J", 4) == 0)	// Fire Emblem: Shin Monshou no Nazo: Hikari to Kage no Eiyuu (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "BFPJ", 4) == 0)	// Fresh PreCure!: Asobi Collection (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "B4FJ", 4) == 0)	// Fushigi no Dungeon: Fuurai no Shiren 4: Kami no Hitomi to Akuma no Heso (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "B5FJ", 4) == 0)	// Fushigi no Dungeon: Fuurai no Shiren 5: Fortune Tower to Unmei no Dice (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "BG3J", 4) == 0)	// G.G Series Collection+ (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "BRQJ", 4) == 0)	// Gendai Daisenryaku DS: Isshoku Sokuhatsu, Gunji Balance Houkai (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "VMMJ", 4) == 0)	// Gokujou!! Mecha Mote Iinchou: MM My Best Friend! (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "BM7J", 4) == 0)	// Gokujou!! Mecha Mote Iinchou: MM Town de Miracle Change! (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "BXOJ", 4) == 0)	// Gyakuten Kenji 2 (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "BQFJ", 4) == 0)	// HeartCatch PreCure!: Oshare Collection (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "AWIK", 4) == 0)	// Hotel Duskui Bimil (Korea)
		 || (memcmp(rominfo.saveInfo().gameCode, "YHGJ", 4) == 0)	// Houkago Shounen (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "BRYJ", 4) == 0)	// Hudson x GReeeeN: Live! DeeeeS! (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "YG4K", 4) == 0)	// Hwansangsuhojeon: Tierkreis (Korea)
		 || (memcmp(rominfo.saveInfo().gameCode, "BZ2J", 4) == 0)	// Imasugu Tsukaeru Mamechishiki: Quiz Zatsugaku-ou DS (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "BEZJ", 4) == 0)	// Inazuma Eleven 3: Sekai e no Chousen!!: Bomber (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "BE8J", 4) == 0)	// Inazuma Eleven 3: Sekai e no Chousen!!: Spark (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "BOEJ", 4) == 0)	// Inazuma Eleven 3: Sekai e no Chousen!!: The Ogre (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "BJKJ", 4) == 0)	// Ippan Zaidan Houjin Nihon Kanji Shuujukudo Kentei Kikou Kounin: Kanjukuken DS (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "BIMJ", 4) == 0)	// Iron Master: The Legendary Blacksmith (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "CDOK", 4) == 0)	// Iron Master: Wanggugui Yusangwa Segaeui Yeolsoe (Korea)
		 || (memcmp(rominfo.saveInfo().gameCode, "YROK", 4) == 0)	// Isanghan Naraui Princess (Korea)
		 || (memcmp(rominfo.saveInfo().gameCode, "BRGJ", 4) == 0)	// Ishin no Arashi: Shippuu Ryouma Den (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "UXBP", 4) == 0)	// Jam with the Band (Europe)
		 || (memcmp(rominfo.saveInfo().gameCode, "YEOK", 4) == 0)	// Jeoldaepiryo: Yeongsugeo 1000 DS (Korea)
		 || (memcmp(rominfo.saveInfo().gameCode, "YE9K", 4) == 0)	// Jeoldaeuwi: Yeongdaneo 1900 DS (Korea)
		 || (memcmp(rominfo.saveInfo().gameCode, "B2OK", 4) == 0)	// Jeongukmin Model Audition Superstar DS (Korea)
		 || (memcmp(rominfo.saveInfo().gameCode, "YRCK", 4) == 0)	// Jjangguneun Monmallyeo: Cinemaland Chalkak Chalkak Daesodong! (Korea)
		 || (memcmp(rominfo.saveInfo().gameCode, "CL4K", 4) == 0)	// Jjangguneun Monmallyeo: Mallangmallang Gomuchalheuk Daebyeonsin! (Korea)
		 || (memcmp(rominfo.saveInfo().gameCode, "BOBJ", 4) == 0)	// Kaidan Restaurant: Ura Menu 100-sen (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "TK2J", 4) == 0)	// Kaidan Restaurant: Zoku! Shin Menu 100-sen (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "BA7J", 4) == 0)	// Kaibou Seirigaku DS: Touch de Hirogaru! Jintai no Kouzou to Kinou (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "BKXJ", 4) == 0)	// Kaijuu Busters (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "BYVJ", 4) == 0)	// Kaijuu Busters Powered (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "TGKJ", 4) == 0)	// Kaizoku Sentai Gokaiger: Atsumete Henshin! 35 Sentai! (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "B8RJ", 4) == 0)	// Kamen Rider Battle: GanbaRide: Card Battle Taisen (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "BKHJ", 4) == 0)	// Kamonohashikamo.: Aimai Seikatsu no Susume (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "BKPJ", 4) == 0)	// Kanshuu: Shuukan Pro Wrestling: Pro Wrestling Kentei DS (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "BEKJ", 4) == 0)	// Kanzen Taiou Saishin Kako Mondai: Nijishiken Taisaku: Eiken Kanzenban (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "BQJJ", 4) == 0)	// Kawaii Koneko DS 3 (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "BKKJ", 4) == 0)	// Keroro RPG: Kishi to Musha to Densetsu no Kaizoku (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "BKSJ", 4) == 0)	// Keshikasu-kun: Battle Kasu-tival (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "BKTJ", 4) == 0)	// Kimi ni Todoke: Sodateru Omoi (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "TK9J", 4) == 0)	// Kimi ni Todoke: Special (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "TKTJ", 4) == 0)	// Kimi ni Todoke: Tsutaeru Kimochi (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "CKDJ", 4) == 0)	// Kindaichi Shounen no Jikenbo: Akuma no Satsujin Koukai (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "VCGJ", 4) == 0)	// Kirakira Rhythm Collection (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "BCKJ", 4) == 0)	// Kochira Katsushika Ku Kameari Kouen Mae Hashutsujo: Kateba Tengoku! Makereba Jigoku!: Ryoutsu-ryuu Ikkakusenkin Daisakusen! (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "BCXJ", 4) == 0)	// Kodawari Saihai Simulation: Ochanoma Pro Yakyuu DS: 2010 Nendo Ban (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "VKPJ", 4) == 0)	// Korg DS-10+ Synthesizer Limited Edition (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "BZMJ", 4) == 0)	// Korg M01 Music Workstation (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "BTAJ", 4) == 0)	// Lina no Atelier: Strahl no Renkinjutsushi (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "BCDK", 4) == 0)	// Live-On Card Live-R DS (Korea)
		 || (memcmp(rominfo.saveInfo().gameCode, "VLIP", 4) == 0)	// Lost Identities (Europe)
		 || (memcmp(rominfo.saveInfo().gameCode, "BOXJ", 4) == 0)	// Love Plus+ (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "BL3J", 4) == 0)	// Lupin Sansei: Shijou Saidai no Zunousen (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "YNOK", 4) == 0)	// Mabeopcheonjamun DS (Korea)
		 || (memcmp(rominfo.saveInfo().gameCode, "BCJK", 4) == 0)	// Mabeopcheonjamun DS 2 (Korea)
		 || (memcmp(rominfo.saveInfo().gameCode, "ANMK", 4) == 0)	// Maeilmaeil Deoukdeo!: DS Dunoe Training (Korea)
		 || (memcmp(rominfo.saveInfo().gameCode, "TCYE", 4) == 0)	// Mama's Combo Pack: Volume 1 (USA)
		 || (memcmp(rominfo.saveInfo().gameCode, "TCZE", 4) == 0)	// Mama's Combo Pack: Volume 2 (USA)
		 || (memcmp(rominfo.saveInfo().gameCode, "ANMK", 4) == 0)	// Marie-Antoinette and the American War of Independence: Episode 1: The Brotherhood of the Wolf (Europe)
		 || (memcmp(rominfo.saveInfo().gameCode, "BA5K", 4) == 0)	// Mario & Luigi RPG: Siganui Partner (Korea)
		 || (memcmp(rominfo.saveInfo().gameCode, "C6OJ", 4) == 0)	// Medarot DS: Kabuto Ver. (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "BQWJ", 4) == 0)	// Medarot DS: Kuwagata Ver. (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "BBJJ", 4) == 0)	// Metal Fight Beyblade: Baku Shin Susanoo Shuurai! (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "TKNJ", 4) == 0)	// Meitantei Conan: Aoki Houseki no Rondo (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "TMKJ", 4) == 0)	// Meitantei Conan: Kako Kara no Zensou Kyoku (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "TMXJ", 4) == 0)	// Metal Max 2: Reloaded (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "C34J", 4) == 0)	// Mini Yonku DS (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "BWCJ", 4) == 0)	// Minna no Conveni (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "BQUJ", 4) == 0)	// Minna no Suizokukan (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "BQVJ", 4) == 0)	// Minna to Kimi no Piramekino! (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "B2WJ", 4) == 0)	// Moe Moe 2-ji Taisen(ryaku) Two: Yamato Nadeshiko (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "BWRJ", 4) == 0)	// Momotarou Dentetsu: World (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "CZZK", 4) == 0)	// Monmallineun 3-gongjuwa Hamkkehaneun: Geurimyeonsang Yeongdaneo Amgibeop (Korea)
		 || (memcmp(rominfo.saveInfo().gameCode, "B3IJ", 4) == 0)	// Motto! Stitch! DS: Rhythm de Rakugaki Daisakusen (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "C6FJ", 4) == 0)	// Mugen no Frontier Exceed: Super Robot Taisen OG Saga (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "B74J", 4) == 0)	// Nanashi no Geemu Me (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "TNRJ", 4) == 0)	// Nora to Toki no Koubou: Kiri no Mori no Majo (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "YNRK", 4) == 0)	// Naruto Jilpungjeon: Daenantu! Geurimja Bunsinsul (Korea)
		 || (memcmp(rominfo.saveInfo().gameCode, "BKJJ", 4) == 0)	// Nazotte Oboeru: Otona no Kanji Renshuu: Kaiteiban (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "BPUJ", 4) == 0)	// Nettou! Powerful Koushien (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "TJ7J", 4) == 0)	// New Horizon: English Course 3 (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "TJ8J", 4) == 0)	// New Horizon: English Course 2 (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "TJ9J", 4) == 0)	// New Horizon: English Course 1 (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "BETJ", 4) == 0)	// Nihon Keizai Shinbunsha Kanshuu: Shiranai Mama dewa Son wo Suru: 'Mono ya Okane no Shikumi' DS (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "YCUP", 4) == 0)	// Nintendo Presents: Crossword Collection (Europe)
		 || (memcmp(rominfo.saveInfo().gameCode, "B2KJ", 4) == 0)	// Ni no Kuni: Shikkoku no Madoushi (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "BNCJ", 4) == 0)	// Nodame Cantabile: Tanoshii Ongaku no Jikan Desu (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "CQKP", 4) == 0)	// NRL Mascot Mania (Australia)
		 || (memcmp(rominfo.saveInfo().gameCode, "BO4J", 4) == 0)	// Ochaken no Heya DS 4 (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "BOYJ", 4) == 0)	// Odoru Daisousa-sen: The Game: Sensuikan ni Sennyuu Seyo! (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "B62J", 4) == 0)	// Okaeri! Chibi-Robo!: Happy Rich Oosouji! (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "TGBJ", 4) == 0)	// One Piece Gigant Battle 2: Shin Sekai (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "BOKJ", 4) == 0)	// Ookami to Koushinryou: Umi o Wataru Kaze (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "TKDJ", 4) == 0)	// Ore-Sama Kingdom: Koi mo Manga mo Debut o Mezase! Doki Doki Love Lesson (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "TFTJ", 4) == 0)	// Original Story from Fairy Tail: Gekitotsu! Kardia Daiseidou (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "BHQJ", 4) == 0)	// Otona no Renai Shousetsu: DS Harlequin Selection (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "BIPJ", 4) == 0)	// Pen1 Grand Prix: Penguin no Mondai Special (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "BO9J", 4) == 0)	// Penguin no Mondai: The World (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "B42J", 4) == 0)	// Pet Shop Monogatari DS 2 (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "BVGE", 4) == 0)	// Petz: Bunnyz Bunch (USA)
		 || (memcmp(rominfo.saveInfo().gameCode, "BLLE", 4) == 0)	// Petz: Catz Playground (USA)
		 || (memcmp(rominfo.saveInfo().gameCode, "BUFE", 4) == 0)	// Petz: Puppyz & Kittenz (USA)
		 || (memcmp(rominfo.saveInfo().gameCode, "VFBE", 4) == 0)	// Petz Fantasy: Moonlight Magic (USA)
		 || (memcmp(rominfo.saveInfo().gameCode, "VTPV", 4) == 0)	// Phineas and Ferb: 2 Disney Games (Europe)
		 || (memcmp(rominfo.saveInfo().gameCode, "B5VE", 4) == 0)	// Phineas and Ferb: Across the 2nd Dimension (USA)
		 || (memcmp(rominfo.saveInfo().gameCode, "YFTK", 4) == 0)	// Pokemon Bulgasaui Dungeon: Siganui Tamheomdae (Korea)
		 || (memcmp(rominfo.saveInfo().gameCode, "YFYK", 4) == 0)	// Pokemon Bulgasaui Dungeon: Eodumui Tamheomdae (Korea)
		 || (memcmp(rominfo.saveInfo().gameCode, "BPPJ", 4) == 0)	// PostPet DS: Yumemiru Momo to Fushigi no Pen (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "BONJ", 4) == 0)	// Powerful Golf (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "VPTJ", 4) == 0)	// Power Pro Kun Pocket 12 (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "VPLJ", 4) == 0)	// Power Pro Kun Pocket 13 (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "VP4J", 4) == 0)	// Power Pro Kun Pocket 14 (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "B2YK", 4) == 0)	// Ppiyodamari DS (Korea)
		 || (memcmp(rominfo.saveInfo().gameCode, "B4NK", 4) == 0)	// Princess Angel: Baeguiui Cheonsa (Korea)
		 || (memcmp(rominfo.saveInfo().gameCode, "C4WK", 4) == 0)	// Princess Bakery (Korea)
		 || (memcmp(rominfo.saveInfo().gameCode, "CP4K", 4) == 0)	// Princess Maker 4: Special Edition (Korea)
		 || (memcmp(rominfo.saveInfo().gameCode, "C29J", 4) == 0)	// Pro Yakyuu Famista DS 2009 (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "BF2J", 4) == 0)	// Pro Yakyuu Famista DS 2010 (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "B89J", 4) == 0)	// Pro Yakyuu Team o Tsukurou! 2 (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "BU9J", 4) == 0)	// Pucca: Power Up (Europe)
		 || (memcmp(rominfo.saveInfo().gameCode, "TP4J", 4) == 0)	// Puyo Puyo!!: Puyopuyo 20th Anniversary (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "BYOJ", 4) == 0)	// Puyo Puyo 7 (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "BHXJ", 4) == 0)	// Quiz! Hexagon II (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "BQ2J", 4) == 0)	// Quiz Magic Academy DS: Futatsu no Jikuuseki (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "YRBK", 4) == 0)	// Ragnarok DS (Korea)
		 || (memcmp(rominfo.saveInfo().gameCode, "TEDJ", 4) == 0)	// Red Stone DS: Akaki Ishi ni Michibikareshi Mono-tachi (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "B35J", 4) == 0)	// Rekishi Simulation Game: Sangokushi DS 3 (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "BUKJ", 4) == 0)	// Rekishi Taisen: Gettenka: Tenkaichi Battle Royale (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "YLZK", 4) == 0)	// Rhythm Sesang (Korea)
		 || (memcmp(rominfo.saveInfo().gameCode, "BKMJ", 4) == 0)	// Rilakkuma Rhythm: Mattari Kibun de Dararan Ran (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "B6XJ", 4) == 0)	// Rockman EXE: Operate Shooting Star (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "V29J", 4) == 0)	// RPG Tkool DS (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "VEBJ", 4) == 0)	// RPG Tsukuru DS+: Create The New World (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "ARFK", 4) == 0)	// Rune Factory: Sinmokjjangiyagi (Korea)
		 || (memcmp(rominfo.saveInfo().gameCode, "CSGJ", 4) == 0)	// SaGa 2: Hihou Densetsu: Goddess of Destiny (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "BZ3J", 4) == 0)	// SaGa 3: Jikuu no Hasha: Shadow or Light (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "CBEJ", 4) == 0)	// Saibanin Suiri Game: Yuuzai x Muzai (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "B59J", 4) == 0)	// Sakusaku Jinkou Kokyuu Care Training DS (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "BSWJ", 4) == 0)	// Saka Tsuku DS: World Challenge 2010 (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "B3GJ", 4) == 0)	// SD Gundam Sangoku Den: Brave Battle Warriors: Shin Militia Taisen (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "B7XJ", 4) == 0)	// Seitokai no Ichizon: DS Suru Seitokai (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "CQ2J", 4) == 0)	// Sengoku Spirits: Gunshi Den (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "CQ3J", 4) == 0)	// Sengoku Spirits: Moushou Den (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "YR4J", 4) == 0)	// Sengoku Spirits: Shukun Den (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "B5GJ", 4) == 0)	// Shin Sengoku Tenka Touitsu: Gunyuu-tachi no Souran (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "C36J", 4) == 0)	// Sloane to MacHale no Nazo no Story (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "B2QJ", 4) == 0)	// Sloane to MacHale no Nazo no Story 2 (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "A3YK", 4) == 0)	// Sonic Rush Adventure (Korea)
		 || (memcmp(rominfo.saveInfo().gameCode, "TFLJ", 4) == 0)	// Sora no Otoshimono Forte: Dreamy Season (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "YW4K", 4) == 0)	// Spectral Force: Genesis (Korea)
		 || (memcmp(rominfo.saveInfo().gameCode, "B22J", 4) == 0)	// Strike Witches 2: Iyasu, Naosu, Punipuni Suru (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "BYQJ", 4) == 0)	// Suisui Physical Assessment Training DS (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "TPQJ", 4) == 0)	// Suite PreCure: Melody Collection (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "CS7J", 4) == 0)	// Summon Night X: Tears Crown (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "C2YJ", 4) == 0)	// Supa Robo Gakuen (Japan) Nazotoki Adventure (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "BRWJ", 4) == 0)	// Super Robot Taisen L (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "BROJ", 4) == 0)	// Super Robot Taisen OG Saga: Masou Kishin: The Lord of Elemental (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "C5IJ", 4) == 0)	// Tago Akira no Atama no Taisou: Dai-1-shuu: Nazotoki Sekai Isshuu Ryokou (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "C52J", 4) == 0)	// Tago Akira no Atama no Taisou: Dai-2-shuu: Ginga Oudan (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "BQ3J", 4) == 0)	// Tago Akira no Atama no Taisou: Dai-3-shuu: Fushigi no Kuni no Nazotoki Otogibanashi (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "BQ4J", 4) == 0)	// Tago Akira no Atama no Taisou: Dai-4-shuu: Time Machine no Nazotoki Daibouken (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "B3DJ", 4) == 0)	// Taiko no Tatsujin DS: Dororon! Yookai Daikessen!! (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "B7KJ", 4) == 0)	// Tamagotch no Narikiri Challenge (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "BGVJ", 4) == 0)	// Tamagotch no Narikiri Channel (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "BG5J", 4) == 0)	// Tamagotch no Pichi Pichi Omisetchi (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "TGCJ", 4) == 0)	// Tamagotchi Collection (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "BQ9J", 4) == 0)	// Tekipaki Kyuukyuu Kyuuhen Training DS (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "B5KJ", 4) == 0)	// Tenkaichi: Sengoku Lovers DS (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "TENJ", 4) == 0)	// Tennis no Ouji-sama: Gyutto! Dokidoki Survival: Umi to Yama no Love Passion (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "BTGJ", 4) == 0)	// Tennis no Ouji-sama: Motto Gakuensai no Ouji-sama: More Sweet Edition (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "VIMJ", 4) == 0)	// The Idolm@ster: Dearly Stars (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "B6KP", 4) == 0)	// Tinker Bell + Tinker Bell and the Lost Treasure (Europe)
		 || (memcmp(rominfo.saveInfo().gameCode, "TKGJ", 4) == 0)	// Tobidase! Kagaku-kun: Chikyuu Daitanken! Nazo no Chinkai Seibutsu ni Idome! (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "CT5K", 4) == 0)	// TOEIC DS: Haru 10-bun Yakjeomgeukbok +200 (Korea)
		 || (memcmp(rominfo.saveInfo().gameCode, "AEYK", 4) == 0)	// TOEIC Test DS Training (Korea)
		 || (memcmp(rominfo.saveInfo().gameCode, "BT5J", 4) == 0)	// TOEIC Test Super Coach@DS (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "TQ5J", 4) == 0)	// Tokumei Sentai Go Busters (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "CVAJ", 4) == 0)	// Tokyo Twilight Busters: Kindan no Ikenie Teito Jigokuhen (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "CZXK", 4) == 0)	// Touch Man to Man: Gichoyeongeo (Korea)
		 || (memcmp(rominfo.saveInfo().gameCode, "BUQJ", 4) == 0)	// Treasure Report: Kikai Jikake no Isan (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "C2VJ", 4) == 0)	// Tsukibito (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "BH6J", 4) == 0)	// TV Anime Fairy Tail: Gekitou! Madoushi Kessen (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "CUHJ", 4) == 0)	// Umihara Kawase Shun: Second Edition Kanzen Ban (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "TBCJ", 4) == 0)	// Usavich: Game no Jikan (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "BPOJ", 4) == 0)	// Utacchi (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "BXPJ", 4) == 0)	// Winnie the Pooh: Kuma no Puu-san: 100 Acre no Mori no Cooking Book (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "BWYJ", 4) == 0)	// Wizardry: Boukyaku no Isan (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "BWZJ", 4) == 0)	// Wizardry: Inochi no Kusabi (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "BWWJ", 4) == 0)	// WiZmans World (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "BYNJ", 4) == 0)	// Yamakawa Shuppansha Kanshuu: Shousetsu Nihonshi B: Shin Sougou Training Plus (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "BYSJ", 4) == 0)	// Yamakawa Shuppansha Kanshuu: Shousetsu Sekaishi B: Shin Sougou Training Plus (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "B5DJ", 4) == 0)	// Yamanote-sen Meimei 100 Shuunen Kinen: Densha de Go!: Tokubetsu Hen: Fukkatsu! Shouwa no Yamanote-sen (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "BYMJ", 4) == 0)	// Yumeiro Patissiere: My Sweets Cooking (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "BZQJ", 4) == 0)	// Zac to Ombra: Maboroshi no Yuuenchi (Japan)
		 || (memcmp(rominfo.saveInfo().gameCode, "BZBJ", 4) == 0))	// Zombie Daisuki (Japan)
		{
			hasAP = true;
		} else {
			static const char ap_list[][4] = {
				"YBN",	// 100 Classic Books
				"VAL",	// Alice in Wonderland
				"VAA",	// Art Academy
				"C7U",	// Battle of Giants: Dragons
				"BIG",	// Battle of Giants: Mutant Insects
				"BBU",	// Beyblade: Metal Fusion
				"BRZ",	// Beyblade: Metal Masters
				"YBU",	// Blue Dragon: Awakened Shadow
				"VKH",	// Brainstorm Series: Treasure Chase
				"BDU",	// C.O.P.: The Recruit
				"BDY",	// Call of Duty: Black Ops
				"TCM",	// Camping Mama: Outdoor Adventures
				"VCM",	// Camp Rock: The Final Jam
				"BQN",	// Captain America: Super Soldier
				"B2B",	// Captain Tsubasa
				"VCA",	// Cars 2
				"VMY",	// Chronicles of Mystery: The Secret Tree of Life
				"YQU",	// Chrono Trigger
				"CY9",	// Club Penguin: EPF: Herbert's Revenge
				"BQ8",	// Crafting Mama
				"VAO",	// Crime Lab: Body of Evidence
				"BD2",	// Deca Sports DS
				"BDE",	// Dementium II
				"BDB",	// Dragon Ball: Origins 2
				"YV5",	// Dragon Quest V: Hand of the Heavenly Bride
				"YVI",	// Dragon Quest VI: Realms of Revelation
				"YDQ",	// Dragon Quest IX: Sentinels of the Starry Skies
				"CJR",	// Dragon Quest Monsters: Joker 2
				"BEL",	// Element Hunters
				"BJ3",	// Etrian Odyssey III: The Drowned City
				"CFI",	// Final Fantasy Crystal Chronicles: Echoes of Time
				"BFX",	// Final Fantasy: The 4 Heroes of Light
				"VDE",	// Fossil Fighters Champions
				"BJC",	// GoldenEye 007
				"BO5",	// Golden Sun: Dark Dawn
				"YGX",	// Grand Theft Auto: Chinatown Wars
				"BGT",	// Ghost Trick: Phantom Detective
				"B7H",	// Harry Potter and the Deathly Hallows: Part 1
				"BU8",	// Harry Potter and the Deathly Hallows: Part 2
				"BKU",	// Harvest Moon DS: The Tale of Two Towns
				"YEE",	// Inazuma Eleven
				"BEE",	// Inazuma Eleven 2: Blizzard
				"BEB",	// Inazuma Eleven 2: Firestorm
				"B86",	// Jewels of the Ages
				"YKG",	// Kindgom Hearts: 358/2 Days
				"BK9",	// Kindgom Hearts: Re-coded
				"VKG",	// Korg DS-10+ Synthesizer
				"BQP",	// KuruKuru Princess: Tokimeki Figure
				"YLU", 	// Last Window: The Secret of Cape West
				"BSD",	// Lufia: Curse of the Sinistrals
				"YMP",	// MapleStory DS
				"CLJ",	// Mario & Luigi: Bowser's Inside Story
				"COL",	// Mario & Sonic at the Olympic Winter Games
				"V2G",	// Mario vs. Donkey Kong: Mini-Land Mayhem!
				"B6Z",	// Mega Man Zero Collection
				"BVN",	// Michael Jackson: The Experience
				"CHN",	// Might & Magic: Clash of Heroes
				"BNQ",	// Murder in Venice
				"BFL",	// MySims: Sky Heroes
				"CNS",	// Naruto Shippuden: Naruto vs Sasuke
				"BSK",	// Nine Hours: Nine Persons: Nine Doors
				"BOJ",	// One Piece: Gigant Battle!
				"BOO",	// Ookami Den
				"VFZ",	// Petz: Fantasy
				"BNR",	// Petz: Nursery
				"B3U",	// Petz: Nursery 2
				"C24",	// Phantasy Star 0
				"BZF",	// Phineas and Ferb: Across the 2nd Dimension
				"VPF",	// Phineas and Ferb: Ride Again
				"IPK",	// Pokemon HeartGold Version
				"IPG",	// Pokemon SoulSilver Version
				"IRA",	// Pokemon Black Version
				"IRB",	// Pokemon White Version
				"IRE",	// Pokemon Black Version 2
				"IRD",	// Pokemon White Version 2
				"VPY",	// Pokemon Conquest
				"B3R",	// Pokemon Ranger: Guardian Signs
				"VPP",	// Prince of Persia: The Forgotten Sands
				"BLF",	// Professor Layton and the Last Specter
				"C3J",	// Professor Layton and the Unwound Future
				"BKQ",	// Pucca: Power Up
				"VRG",	// Rabbids Go Home: A Comedy Adventure
				"BRJ",	// Radiant Hostoria
				"B3X",	// River City: Soccer Hooligans
				"BRM",	// Rooms: The Main Building
				"TDV",	// Shin Megami Tensei: Devil Survivor 2
				"BMT",	// Shin Megami Tensei: Strange Journey
				"VCD",	// Solatorobo: Red the Hunter
				"BXS",	// Sonic Colors
				"VSN",	// Sonny with a Chance
				"B2U",	// Sports Collection
				"CLW",	// Star Wars: The Clone Wars: Jedi Alliance
				"AZL",	// Style Savvy
				"BH2",	// Super Scribblenauts
				"B6T",	// Tangled
				"B4T",	// Tetris Party Deluxe
				"BKI",	// The Legend of Zelda: Spirit Tracks
				"VS3",	// The Sims 3
				"BZU",	// The Smurfs
				"TR2",	// The Smurfs 2
				"BS8",	// The Sorcerer's Apprentice
				"BTU",	// Tinker Bell and the Great Fairy Rescue
				"CCU",	// Tomodachi Collection
				"VT3",	// Toy Story 3
				"VTE",	// TRON: Evolution
				"B3V",	// Vampire Moon: The Mystery of the Hidden Sun
				"BW4",	// Wizards of Waverly Place: Spellbound
				"BYX",	// Yu-Gi-Oh! 5D's: World Championship 2010: Reverse of Arcadia
				"BYY",	// Yu-Gi-Oh! 5D's: World Championship 2011: Over the Nexus
			};
			// TODO: If the list gets large enough, switch to bsearch().
			for (unsigned int i = 0; i < sizeof(ap_list)/sizeof(ap_list[0]); i++) {
				if (memcmp(rominfo.saveInfo().gameCode, ap_list[i], 3) == 0) {
					// Found a match.
					hasAP = true;
					break;
				}
			}

			static const char ap_list2[][4] = {
				"VID",	// Imagine: Resort Owner
				"TAD",	// Kirby: Mass Attack
			};
			// TODO: If the list gets large enough, switch to bsearch().
			for (unsigned int i = 0; i < sizeof(ap_list2)/sizeof(ap_list2[0]); i++) {
				if (memcmp(rominfo.saveInfo().gameCode, ap_list2[i], 3) == 0) {
					// Found a match.
					hasAP1 = true;
					break;
				}
			}

		}

        int optionPicked = 0;

		if (hasAP)
		{
			optionPicked = messageBox(this, LANG("game launch", "ap warning"), LANG("game launch", "ap msg"), MB_OK | MB_HOLD_X | MB_CANCEL);
		}

		if (hasAP1)
		{
			optionPicked = messageBox(this, LANG("game launch", "ap warning"), LANG("game launch", "ap msg1"), MB_OK | MB_HOLD_X | MB_CANCEL);
		}

		scanKeys();
		int pressed = keysHeld();

		if (pressed & KEY_X || optionPicked == ID_HOLD_X)
		{
			settingsIni.dontShowAPMsgAgain();
		}
		if (!hasAP || pressed & KEY_A || pressed & KEY_X || optionPicked == ID_HOLD_X || optionPicked == ID_OK)
		{
			// Event handlers for progress window.
			config.onSaveCreated(bootstrapSaveHandler)
				.onConfigSaved(bootstrapLaunchHandler);

			progressWnd().setTipText(LANG("game launch", "Please wait"));
			progressWnd().update();
			progressWnd().show();

			int err = config.launch();
			if (err)
			{
				std::string errorString = formatString(LANG("game launch", "error").c_str(), err);
				messageBox(this, LANG("game launch", "NDS Bootstrap Error"), errorString, MB_OK);
				progressWnd().hide();
			}
		}
	} else {
		// Event handlers for progress window.
		config.onSaveCreated(bootstrapSaveHandler)
			.onConfigSaved(bootstrapLaunchHandler);

		progressWnd().setTipText(LANG("game launch", "Please wait"));
		progressWnd().update();
		progressWnd().show();

		int err = config.launch();
		if (err)
		{
			std::string errorString = formatString(LANG("game launch", "error").c_str(), err);
			messageBox(this, LANG("game launch", "NDS Bootstrap Error"), errorString, MB_OK);
			progressWnd().hide();
		}
	}
}

void MainWnd::bootFlashcard(const std::string &ndsPath, bool usePerGameSettings)
{
    int err = loadGameOnFlashcard(ndsPath.c_str(), usePerGameSettings);
    if (err)
    {
        std::string errorString = formatString(LANG("game launch", "error").c_str(), err);
        messageBox(this, LANG("game launch", "Flashcard Error"), errorString, MB_OK);
    }
}

void MainWnd::bootFile(const std::string &loader, const std::string &fullPath)
{
    LoaderConfig config(loader, "");
    std::vector<const char *> argv{};
    argv.emplace_back(loader.c_str());
    argv.emplace_back(fullPath.c_str());
    int err = config.launch(argv.size(), argv.data());
    if (err)
    {
        std::string errorString = formatString(LANG("game launch", "error").c_str(), err);
        messageBox(this, LANG("game launch", "Launch Error"), errorString, MB_OK);
        progressWnd().hide();
    }
}

void MainWnd::launchSelected()
{
    cwl();
    dbg_printf("Launch.");
    std::string fullPath = _mainList->getSelectedFullPath();

    cwl();
    if (fullPath[fullPath.size() - 1] == '/')
    {
        cwl();
        _mainList->enterDir(fullPath);
        return;
    }

	if (!ms().gotosettings && ms().consoleModel < 2 && ms().previousUsedDevice && bothSDandFlashcard()) {
		if (access("sd:/_nds/TWiLightMenu/tempDSiWare.dsi", F_OK) == 0) {
			remove("sd:/_nds/TWiLightMenu/tempDSiWare.dsi");
		}
		if (access("sd:/_nds/TWiLightMenu/tempDSiWare.pub", F_OK) == 0) {
			remove("sd:/_nds/TWiLightMenu/tempDSiWare.pub");
		}
		if (access("sd:/_nds/TWiLightMenu/tempDSiWare.prv", F_OK) == 0) {
			remove("sd:/_nds/TWiLightMenu/tempDSiWare.prv");
		}
	}

    ms().romfolder[ms().secondaryDevice] = _mainList->getCurrentDir();
    ms().previousUsedDevice = ms().secondaryDevice;
    ms().romPath[ms().secondaryDevice] = fullPath;
    ms().slot1Launched = false;
    ms().saveSettings();

    DSRomInfo rominfo;
    if (!_mainList->getRomInfo(_mainList->selectedRowId(), rominfo)) {
        return;
	}

	chdir(_mainList->getCurrentDir().c_str());

    // Launch DSiWare
    if (!rominfo.isHomebrew() && rominfo.isDSiWare() && (isDSiMode() || sdFound()) && ms().consoleModel == 0 && !ms().dsiWareBooter)
    {
        // Unlaunch boot here....
        UnlaunchBoot unlaunch(fullPath, rominfo.saveInfo().dsiPubSavSize, rominfo.saveInfo().dsiPrvSavSize);

        // Roughly associated with 50%, 90%
        unlaunch.onPrvSavCreated(bootstrapSaveHandler)
            .onPubSavCreated(bootstrapLaunchHandler);

            
        progressWnd().setPercent(0);
        progressWnd().setTipText(LANG("game launch", "Preparing Unlaunch Boot"));
        progressWnd().update();
        progressWnd().show();

        if (unlaunch.prepare())
        {
			progressWnd().hide();
            messageBox(this, LANG("game launch", "unlaunch boot"), LANG("game launch", "unlaunch instructions"), MB_OK);
        }
        ms().launchType[ms().secondaryDevice] = TWLSettings::EDSiWareLaunch;
        ms().saveSettings();
        progressWnd().hide();
        unlaunch.launch();
    }

    if (rominfo.isDSRom())
    {
        PerGameSettings gameConfig(_mainList->getSelectedShowName());
        // Direct Boot for homebrew.
        if (rominfo.isDSiWare() || (gameConfig.directBoot && rominfo.isHomebrew()))
        {
			std::string fileName = _mainList->getSelectedShowName();
			std::string fullPath = _mainList->getSelectedFullPath();

			BootstrapConfig config(fileName, fullPath, std::string((char *)rominfo.saveInfo().gameCode), rominfo.saveInfo().gameSdkVersion, gameConfig.heapShrink);

			ms().homebrewHasWide = (rominfo.saveInfo().gameCode[0] == 'W' || rominfo.version() == 0x57);
			ms().launchType[ms().secondaryDevice] = TWLSettings::ESDFlashcardDirectLaunch;
			ms().saveSettings();
			bootWidescreen(fileName.c_str(), true, (gameConfig.wideScreen == PerGameSettings::EDefault ? ms().wideScreen : (bool)gameConfig.wideScreen));
            bootArgv(rominfo);
            return;
        }

        else if (ms().useBootstrap || !ms().secondaryDevice)
        {
			ms().launchType[ms().secondaryDevice] = TWLSettings::ESDFlashcardLaunch;
			ms().saveSettings();
            bootBootstrap(gameConfig, rominfo);
            return;
        }
        else
        {
			ms().launchType[ms().secondaryDevice] = TWLSettings::ESDFlashcardLaunch;
			ms().saveSettings();
            dbg_printf("Flashcard Launch: %s\n", fullPath.c_str());
            bootFlashcard(fullPath, true);
            return;
        }
    }

    std::string extension;
    size_t lastDotPos = fullPath.find_last_of('.');
    if (fullPath.npos != lastDotPos)
        extension = fullPath.substr(lastDotPos);

    // DSTWO Plugin Launch
    if (extension == ".plg" && ms().secondaryDevice && memcmp(io_dldi_data->friendlyName, "DSTWO(Slot-1)", 0xD) == 0)
    {
        ms().launchType[ms().secondaryDevice] = TWLSettings::ESDFlashcardLaunch;
        ms().saveSettings();

		// Print .plg path without "fat:" at the beginning
		char ROMpathDS2[256];
		for (int i = 0; i < 252; i++) {
			ROMpathDS2[i] = fullPath[4+i];
			if (fullPath[4+i] == '\x00') break;
		}

		CIniFile dstwobootini( "fat:/_dstwo/twlm.ini" );
		dstwobootini.SetString("boot_settings", "file", ROMpathDS2);
		dstwobootini.SaveIniFile( "fat:/_dstwo/twlm.ini" );

        bootFile(BOOTPLG_SRL, fullPath);
	}

	const char *ndsToBoot;

    // RVID Launch
    if (extension == ".rvid")
    {
        ms().homebrewArg = fullPath;
        ms().launchType[ms().secondaryDevice] = TWLSettings::ERVideoLaunch;
        ms().saveSettings();

		ndsToBoot = RVIDPLAYER_SD;
		if (!isDSiMode() || access(ndsToBoot, F_OK) != 0) {
			ndsToBoot = RVIDPLAYER_FC;
		}

        bootFile(ndsToBoot, fullPath);
    }

    // MPEG4 Launch
    if (extension == ".mp4")
    {
        ms().homebrewArg = fullPath;
        ms().launchType[ms().secondaryDevice] = TWLSettings::EMPEG4Launch;
        ms().saveSettings();

		ndsToBoot = MPEG4PLAYER_SD;
		if (!isDSiMode() || access(ndsToBoot, F_OK) != 0) {
			ndsToBoot = MPEG4PLAYER_FC;
		}

        bootFile(ndsToBoot, fullPath);
    }

    // GBA Launch
    if (extension == ".gba")
	{
        ms().homebrewArg = fullPath;
        ms().launchType[ms().secondaryDevice] = TWLSettings::ESDFlashcardLaunch;
        ms().saveSettings();
		if (ms().secondaryDevice)
        {
			if (REG_SCFG_EXT != 0)
			{
				ndsToBoot = ms().consoleModel>0 ? GBARUNNER_3DS : GBARUNNER_DSI;
				if (access(ndsToBoot, F_OK) != 0) {
					ndsToBoot = ms().consoleModel>0 ? GBARUNNER_3DS_FC : GBARUNNER_DSI_FC;
				}
			}
			else
			{
				ndsToBoot = ms().gbar2DldiAccess ? GBARUNNER_A7_SD : GBARUNNER_A9_SD;
				if (access(ndsToBoot, F_OK) != 0) {
					ndsToBoot = ms().gbar2DldiAccess ? GBARUNNER_A7 : GBARUNNER_A9;
				}
			}

            bootFile(ndsToBoot, fullPath);
		}
		else
		{
			std::string bootstrapPath = (ms().bootstrapFile ? BOOTSTRAP_NIGHTLY_HB : BOOTSTRAP_RELEASE_HB);

			std::vector<char*> argarray;
			argarray.push_back(strdup(bootstrapPath.c_str()));
			argarray.at(0) = (char*)bootstrapPath.c_str();

			const char* gbar2Path = ms().consoleModel>0 ? GBARUNNER_3DS : GBARUNNER_DSI;
			if (sys().arm7SCFGLocked()) {
				gbar2Path = ms().consoleModel>0 ? GBARUNNER_3DS_NODSP : GBARUNNER_DSI_NODSP;
			}

			LoaderConfig gen(bootstrapPath, BOOTSTRAP_INI);
			gen.option("NDS-BOOTSTRAP", "NDS_PATH", gbar2Path)
			   .option("NDS-BOOTSTRAP", "HOMEBREW_ARG", fullPath)
			   .option("NDS-BOOTSTRAP", "RAM_DRIVE_PATH", "")
			   .option("NDS-BOOTSTRAP", "GUI_LANGUAGE", ms().getGuiLanguageString())
			   .option("NDS-BOOTSTRAP", "LANGUAGE", ms().gameLanguage)
			   .option("NDS-BOOTSTRAP", "DSI_MODE", 0)
			   .option("NDS-BOOTSTRAP", "BOOST_CPU", 1)
			   .option("NDS-BOOTSTRAP", "BOOST_VRAM", 0);
			if (int err = gen.launch(argarray.size(), (const char **)&argarray[0], false))
			{
				std::string errorString = formatString(LANG("game launch", "error").c_str(), err);
				messageBox(this, LANG("game launch", "nds-bootstrap error"), errorString, MB_OK);
			}
		}
	}

    // A26 Launch
    if (extension == ".a26")
    {
        ms().homebrewArg = fullPath;
        ms().launchType[ms().secondaryDevice] = TWLSettings::EStellaDSLaunch;
        ms().saveSettings();

		ndsToBoot = STELLADS_SD;
		if (!isDSiMode() || access(ndsToBoot, F_OK) != 0) {
			ndsToBoot = STELLADS_FC;
		}

        bootFile(ndsToBoot, fullPath);
    }

    // NES Launch
    if (extension == ".nes" || extension == ".fds")
    {
        ms().homebrewArg = fullPath;
        ms().launchType[ms().secondaryDevice] = TWLSettings::ENESDSLaunch;
        ms().saveSettings();

		ndsToBoot = (ms().secondaryDevice ? NESDS_SD : NESTWL_SD);
		if (!isDSiMode() || access(ndsToBoot, F_OK) != 0) {
			ndsToBoot = NESDS_FC;
		}

        bootFile(ndsToBoot, fullPath);
    }

    // GB Launch
    if (extension == ".gb" || extension == ".gbc")
    {
        ms().homebrewArg = fullPath;
        ms().launchType[ms().secondaryDevice] = TWLSettings::EGameYobLaunch;
        ms().saveSettings();

		ndsToBoot = GAMEYOB_SD;
		if (!isDSiMode() || access(ndsToBoot, F_OK) != 0) {
			ndsToBoot = GAMEYOB_FC;
		}

        bootFile(ndsToBoot, fullPath);
    }

    // SMS/GG Launch
    if (extension == ".sms" || extension == ".gg")
    {
		mkdir(ms().secondaryDevice ? "fat:/data" : "sd:/data", 0777);
		mkdir(ms().secondaryDevice ? "fat:/data/s8ds" : "sd:/data/s8ds", 0777);

		ms().homebrewArg = fullPath;
		if (!ms().secondaryDevice && !sys().arm7SCFGLocked() && ms().smsGgInRam)
		{
			ms().launchType[ms().secondaryDevice] = TWLSettings::ESDFlashcardLaunch;
			ms().saveSettings();

			std::string bootstrapPath = (ms().bootstrapFile ? BOOTSTRAP_NIGHTLY_HB : BOOTSTRAP_RELEASE_HB);

			std::vector<char*> argarray;
			argarray.push_back(strdup(bootstrapPath.c_str()));
			argarray.at(0) = (char*)bootstrapPath.c_str();

			LoaderConfig gen(bootstrapPath, BOOTSTRAP_INI);
			gen.option("NDS-BOOTSTRAP", "NDS_PATH", S8DS07_ROM)
			   .option("NDS-BOOTSTRAP", "HOMEBREW_ARG", "")
			   .option("NDS-BOOTSTRAP", "RAM_DRIVE_PATH", fullPath)
			   .option("NDS-BOOTSTRAP", "GUI_LANGUAGE", ms().getGuiLanguageString())
			   .option("NDS-BOOTSTRAP", "LANGUAGE", ms().gameLanguage)
			   .option("NDS-BOOTSTRAP", "DSI_MODE", 0)
			   .option("NDS-BOOTSTRAP", "BOOST_CPU", 1)
			   .option("NDS-BOOTSTRAP", "BOOST_VRAM", 0);
			if (int err = gen.launch(argarray.size(), (const char **)&argarray[0], false))
			{
				std::string errorString = formatString(LANG("game launch", "error").c_str(), err);
				messageBox(this, LANG("game launch", "nds-bootstrap error"), errorString, MB_OK);
			}
		}
		else
		{
			ms().launchType[ms().secondaryDevice] = TWLSettings::ES8DSLaunch;
			ms().saveSettings();

			ndsToBoot = S8DS_ROM;
			if (!isDSiMode() || access(ndsToBoot, F_OK) != 0) {
				ndsToBoot = S8DS_FC;
			}

			bootFile(ndsToBoot, fullPath);
		}
    }
	
    // GEN Launch
    if (extension == ".gen")
	{
		bool usePicoDrive = ((isDSiMode() && sdFound() && sys().arm7SCFGLocked())
			|| ms().showMd==2 || (ms().showMd==3 && getFileSize(fullPath) > 0x300000));
        ms().homebrewArg = fullPath;
        ms().launchType[ms().secondaryDevice] = (usePicoDrive ? TWLSettings::EPicoDriveTWLLaunch : TWLSettings::ESDFlashcardLaunch);
        ms().saveSettings();
		if (usePicoDrive || ms().secondaryDevice)
        {
			ndsToBoot = usePicoDrive ? PICODRIVETWL_ROM : JENESISDS_ROM;
			if (!isDSiMode() || access(ndsToBoot, F_OK) != 0) {
				ndsToBoot = usePicoDrive ? PICODRIVETWL_FC : JENESISDS_FC;
			}

            bootFile(ndsToBoot, fullPath);
		}
		else
		{
			std::string bootstrapPath = (ms().bootstrapFile ? BOOTSTRAP_NIGHTLY_HB : BOOTSTRAP_RELEASE_HB);

			std::vector<char*> argarray;
			argarray.push_back(strdup(bootstrapPath.c_str()));
			argarray.at(0) = (char*)bootstrapPath.c_str();

			LoaderConfig gen(bootstrapPath, BOOTSTRAP_INI);
			gen.option("NDS-BOOTSTRAP", "NDS_PATH", JENESISDS_ROM)
			   .option("NDS-BOOTSTRAP", "HOMEBREW_ARG", "fat:/ROM.BIN")
			   .option("NDS-BOOTSTRAP", "RAM_DRIVE_PATH", fullPath)
			   .option("NDS-BOOTSTRAP", "GUI_LANGUAGE", ms().getGuiLanguageString())
			   .option("NDS-BOOTSTRAP", "LANGUAGE", ms().gameLanguage)
			   .option("NDS-BOOTSTRAP", "DSI_MODE", 0)
			   .option("NDS-BOOTSTRAP", "BOOST_CPU", 1)
			   .option("NDS-BOOTSTRAP", "BOOST_VRAM", 0);
			if (int err = gen.launch(argarray.size(), (const char **)&argarray[0], false))
			{
				std::string errorString = formatString(LANG("game launch", "error").c_str(), err);
				messageBox(this, LANG("game launch", "nds-bootstrap error"), errorString, MB_OK);
			}
		}
	}

    // SNES Launch
    if (extension == ".smc" || extension == ".sfc")
	{
        ms().homebrewArg = fullPath;
        ms().launchType[ms().secondaryDevice] = TWLSettings::ESDFlashcardLaunch;
        ms().saveSettings();
		if (ms().secondaryDevice)
        {
			ndsToBoot = SNEMULDS_ROM;
			if (!isDSiMode() || access(ndsToBoot, F_OK) != 0) {
				ndsToBoot = SNEMULDS_FC;
			}

            bootFile(ndsToBoot, fullPath);
		}
		else
		{
			std::string bootstrapPath = (ms().bootstrapFile ? BOOTSTRAP_NIGHTLY_HB : BOOTSTRAP_RELEASE_HB);

			std::vector<char*> argarray;
			argarray.push_back(strdup(bootstrapPath.c_str()));
			argarray.at(0) = (char*)bootstrapPath.c_str();

			LoaderConfig snes(bootstrapPath, BOOTSTRAP_INI);
			snes.option("NDS-BOOTSTRAP", "NDS_PATH", SNEMULDS_ROM)
				.option("NDS-BOOTSTRAP", "HOMEBREW_ARG", "fat:/snes/ROM.SMC")
				.option("NDS-BOOTSTRAP", "RAM_DRIVE_PATH", fullPath)
				.option("NDS-BOOTSTRAP", "GUI_LANGUAGE", ms().getGuiLanguageString())
			    .option("NDS-BOOTSTRAP", "LANGUAGE", ms().gameLanguage)
			    .option("NDS-BOOTSTRAP", "DSI_MODE", 0)
				.option("NDS-BOOTSTRAP", "BOOST_CPU", 0)
			    .option("NDS-BOOTSTRAP", "BOOST_VRAM", 0);
			if (int err = snes.launch(argarray.size(), (const char **)&argarray[0], false))
			{
				std::string errorString = formatString(LANG("game launch", "error").c_str(), err);
				messageBox(this, LANG("game launch", "nds-bootstrap error"), errorString, MB_OK);
			}
		}
	}

    // PCE Launch
    if (extension == ".pce")
	{
        ms().homebrewArg = fullPath;
        ms().launchType[ms().secondaryDevice] = TWLSettings::ESDFlashcardLaunch;
        ms().saveSettings();
		if (ms().secondaryDevice)
        {
			ndsToBoot = NITROGRAFX_ROM;
			if (access(ndsToBoot, F_OK) != 0) {
				ndsToBoot = NITROGRAFX_FC;
			}

            bootFile(ndsToBoot, fullPath);
		}
		else
		{
			std::string bootstrapPath = (ms().bootstrapFile ? BOOTSTRAP_NIGHTLY_HB : BOOTSTRAP_RELEASE_HB);

			std::vector<char*> argarray;
			argarray.push_back(strdup(bootstrapPath.c_str()));
			argarray.at(0) = (char*)bootstrapPath.c_str();

			LoaderConfig snes(bootstrapPath, BOOTSTRAP_INI);
			snes.option("NDS-BOOTSTRAP", "NDS_PATH", NITROGRAFX_ROM)
			   .option("NDS-BOOTSTRAP", "HOMEBREW_ARG", fullPath)
			   .option("NDS-BOOTSTRAP", "RAM_DRIVE_PATH", "")
			    .option("NDS-BOOTSTRAP", "GUI_LANGUAGE", ms().getGuiLanguageString())
			    .option("NDS-BOOTSTRAP", "LANGUAGE", ms().gameLanguage)
			    .option("NDS-BOOTSTRAP", "DSI_MODE", 0)
				.option("NDS-BOOTSTRAP", "BOOST_CPU", 1)
			    .option("NDS-BOOTSTRAP", "BOOST_VRAM", 0);
			if (int err = snes.launch(argarray.size(), (const char **)&argarray[0], false))
			{
				std::string errorString = formatString(LANG("game launch", "error").c_str(), err);
				messageBox(this, LANG("game launch", "nds-bootstrap error"), errorString, MB_OK);
			}
		}
	}
}

void MainWnd::onKeyBPressed()
{
    _mainList->backParentDir();
}

void MainWnd::showSettings(void)
{
    dbg_printf("Launch settings...");
	if (!isDSiMode()) {
		chdir("fat:/");
	} else if (sdFound()) {
		chdir("sd:/");
	}
    LoaderConfig settingsLoader(DSIMENUPP_SETTINGS_SRL, DSIMENUPP_INI);

    if (int err = settingsLoader.launch())
    {
        std::string errorString = formatString(LANG("game launch", "error").c_str(), err);
        messageBox(this, LANG("game launch", "NDS Bootstrap Error"), errorString, MB_OK);
    }
}

void MainWnd::showManual(void)
{
    dbg_printf("Launch manual...");
	if (sdFound()) {
		chdir("sd:/");
	}
    LoaderConfig manualLoader(TWLMENUPP_MANUAL_SRL, DSIMENUPP_INI);

    if (int err = manualLoader.launch())
    {
        std::string errorString = formatString(LANG("game launch", "error").c_str(), err);
        messageBox(this, LANG("game launch", "NDS Bootstrap Error"), errorString, MB_OK);
    }
}

void MainWnd::bootSlot1(void)
{
    dbg_printf("Launch Slot1..\n");
    ms().slot1Launched = true;
    ms().saveSettings();

    if (ms().slot1LaunchMethod==0 || sys().arm7SCFGLocked())
    {
        cardLaunch();
        return;
    }
	else if (ms().slot1LaunchMethod==2)
	{
        // Unlaunch boot here....
        UnlaunchBoot unlaunch("cart:", 0, 0);

        // Roughly associated with 50%, 90%
        unlaunch.onPrvSavCreated(bootstrapSaveHandler)
            .onPubSavCreated(bootstrapLaunchHandler);

            
        progressWnd().setPercent(0);
        progressWnd().setTipText(LANG("game launch", "Preparing Unlaunch Boot"));
        progressWnd().update();
        progressWnd().show();

        unlaunch.prepare();
		progressWnd().hide();
        unlaunch.launch();
	}

	bootWidescreen(NULL, false, ms().wideScreen);
	if (sdFound()) {
		chdir("sd:/");
	}
    LoaderConfig slot1Loader(SLOT1_SRL, DSIMENUPP_INI);
    if (int err = slot1Loader.launch())
    {
        std::string errorString = formatString(LANG("game launch", "error").c_str(), err);
        messageBox(this, LANG("game launch", "nds-bootstrap error"), errorString, MB_OK);
    }
}

void MainWnd::bootGbaRunner(void)
{
    if (ms().secondaryDevice && ms().useGbarunner)
    {
		if (ms().useBootstrap)
		{
			if (isDSiMode())
			{
				bootFile(ms().consoleModel>0 ? GBARUNNER_3DS_FC : GBARUNNER_DSI_FC, "");
			}
			else
			{
				bootFile(ms().gbar2DldiAccess ? GBARUNNER_A7 : GBARUNNER_A9, "");
			}
		}
		else
		{
			if (isDSiMode())
			{
				bootFlashcard(ms().consoleModel>0 ? GBARUNNER_3DS_FC : GBARUNNER_DSI_FC, false);
			}
			else
			{
				bootFlashcard(ms().gbar2DldiAccess ? GBARUNNER_A7 : GBARUNNER_A9, false);
			}
		}
        return;
    }

    if (!isDSiMode() && !ms().useGbarunner)
    {
        gbaSwitch();
        return;
    }

	std::string bootstrapPath = (ms().bootstrapFile ? BOOTSTRAP_NIGHTLY_HB : BOOTSTRAP_RELEASE_HB);

	std::vector<char*> argarray;
	argarray.push_back(strdup(bootstrapPath.c_str()));
	argarray.at(0) = (char*)bootstrapPath.c_str();

    LoaderConfig gbaRunner(bootstrapPath, BOOTSTRAP_INI);
	gbaRunner.option("NDS-BOOTSTRAP", "NDS_PATH", ms().consoleModel>0 ? GBARUNNER_3DS : GBARUNNER_DSI)
			 .option("NDS-BOOTSTRAP", "HOMEBREW_ARG", "")
			 .option("NDS-BOOTSTRAP", "RAM_DRIVE_PATH", "")
			 .option("NDS-BOOTSTRAP", "GUI_LANGUAGE", ms().getGuiLanguageString())
			 .option("NDS-BOOTSTRAP", "LANGUAGE", ms().gameLanguage)
			 .option("NDS-BOOTSTRAP", "DSI_MODE", 0)
			 .option("NDS-BOOTSTRAP", "BOOST_CPU", 1)
			 .option("NDS-BOOTSTRAP", "BOOST_VRAM", 0);
    if (int err = gbaRunner.launch(argarray.size(), (const char **)&argarray[0], false))
    {
        std::string errorString = formatString(LANG("game launch", "error").c_str(), err);
        messageBox(this, LANG("game launch", "NDS Bootstrap Error"), errorString, MB_OK);
    }
}

void MainWnd::showFileInfo()
{
    DSRomInfo rominfo;
    if (!_mainList->getRomInfo(_mainList->selectedRowId(), rominfo))
    {
        return;
    }

    dbg_printf("show '%s' info\n", _mainList->getSelectedFullPath().c_str());

    CIniFile ini(SFN_UI_SETTINGS); //(256-)/2,(192-128)/2, 220, 128
    u32 w = 240;
    u32 h = 144;
    w = ini.GetInt("rom info window", "w", w);
    h = ini.GetInt("rom info window", "h", h);

    RomInfoWnd *romInfoWnd = new RomInfoWnd((256 - w) / 2, (192 - h) / 2, w, h, this, LANG("rom info", "title"));
    std::string showName = _mainList->getSelectedShowName();
    std::string fullPath = _mainList->getSelectedFullPath();
    romInfoWnd->setFileInfo(fullPath, showName);
    romInfoWnd->setRomInfo(rominfo);
    romInfoWnd->doModal();
    rominfo = romInfoWnd->getRomInfo();
    _mainList->setRomInfo(_mainList->selectedRowId(), rominfo);

    delete romInfoWnd;
}

void MainWnd::onFolderChanged()
{
    resetInputIdle();
    std::string dirShowName = _mainList->getCurrentDir();


    if (!strncmp(dirShowName.c_str(), "^*::", 2))
    {

        if (dirShowName == SPATH_TITLEANDSETTINGS)
        {
            showSettings();
        }

        if (dirShowName == SPATH_MANUAL)
        {
            showManual();
        }

        if (dirShowName == SPATH_SLOT1)
        {
            bootSlot1();
        }

        if (dirShowName == SPATH_GBARUNNER)
        {
            bootGbaRunner();
        }

        if (dirShowName == SPATH_SYSMENU)
        {
            dsiSysMenuLaunch();
        }

        if (dirShowName == SPATH_SYSTEMSETTINGS)
        {
            dsiLaunchSystemSettings();
        }
        dirShowName.clear();
    }

    dbg_printf("%s\n", _mainList->getSelectedFullPath().c_str());

    _folderText->setText(dirShowName);
}

void MainWnd::onAnimation(bool &anAllow)
{
    if (_startMenu->isVisible())
        anAllow = false;
    else if (windowManager().currentWindow() != this)
        anAllow = false;
}

Window *MainWnd::windowBelow(const Point &p)
{
    Window *wbp = Form::windowBelow(p);
    if (_startMenu->isVisible() && wbp != _startButton)
        wbp = _startMenu;
    return wbp;
}