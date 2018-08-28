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

//#include "files.h"

#include "inifile.h"
#include "language.h"

#include "windows/rominfownd.h"
// #include "helpwnd.h"
// #include "expwnd.h"
// #include "gbaloader.h"
// #include "romlauncher.h"
// #include "sdidentify.h"
// #include "favorites.h"

// #ifdef DEBUG
// #include "iocmn.h"
// #endif

#include <sys/iosupport.h>

using namespace akui;

cMainWnd::cMainWnd(s32 x, s32 y, u32 w, u32 h, cWindow *parent, const std::string &text) : cForm(x, y, w, h, parent, text), _mainList(NULL), _startMenu(NULL), _startButton(NULL),
                                                                                           _brightnessButton(NULL), _folderUpButton(NULL), _folderText(NULL), _processL(false)
{
}

cMainWnd::~cMainWnd()
{
    delete _folderText;
    delete _folderUpButton;
    delete _brightnessButton;
    delete _startButton;
    delete _startMenu;
    delete _mainList;
    windowManager().removeWindow(this);
}

void cMainWnd::init()
{
    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;
    COLOR color = 0;
    std::string file("");
    std::string text("");
    CIniFile ini(SFN_UI_SETTINGS);

    // self init
    dbg_printf("mainwnd init() %08x\n", this);
    loadAppearance(SFN_LOWER_SCREEN_BG);
    windowManager().addWindow(this);

    // init game file list
    //waitMs( 2000 );
    _mainList = new cMainList(4, 20, 248, 152, this, "main list");
    _mainList->setRelativePosition(cPoint(4, 20));
    _mainList->init();
    _mainList->selectChanged.connect(this, &cMainWnd::listSelChange);
    _mainList->selectedRowClicked.connect(this, &cMainWnd::onMainListSelItemClicked);
    _mainList->selectedRowHeadClicked.connect(this, &cMainWnd::onMainListSelItemHeadClicked);
    _mainList->directoryChanged.connect(this, &cMainWnd::onFolderChanged);
    _mainList->animateIcons.connect(this, &cMainWnd::onAnimation);

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
    //_startButton = new cButton( 0, 172, 48, 18, this, " Start" );
    _startButton = new cButton(x, y, w, h, this, text);
    _startButton->setStyle(cButton::press);
    _startButton->setRelativePosition(cPoint(x, y));
    _startButton->loadAppearance(file);
    _startButton->clicked.connect(this, &cMainWnd::startButtonClicked);
    _startButton->setTextColor(color | BIT(15));
    if (!ini.GetInt("start button", "show", 1))
        _startButton->hide();
    addChildWindow(_startButton);

    // // init brightness button
    // x = ini.GetInt("brightness btn", "x", 240);
    // y = ini.GetInt("brightness btn", "y", 1);
    // w = ini.GetInt("brightness btn", "w", 16);
    // h = ini.GetInt("brightness btn", "h", 16);
    // _brightnessButton = new cButton(x, y, w, h, this, "");
    // _brightnessButton->setRelativePosition(cPoint(x, y));
    // _brightnessButton->loadAppearance(SFN_BRIGHTNESS_BUTTON);
    // _brightnessButton->pressed.connect(this, &cMainWnd::brightnessButtonClicked);
    // addChildWindow(_brightnessButton);

    x = ini.GetInt("folderup btn", "x", 0);
    y = ini.GetInt("folderup btn", "y", 2);
    w = ini.GetInt("folderup btn", "w", 32);
    h = ini.GetInt("folderup btn", "h", 16);
    _folderUpButton = new cButton(x, y, w, h, this, "");
    _folderUpButton->setRelativePosition(cPoint(x, y));
    _folderUpButton->loadAppearance(SFN_FOLDERUP_BUTTON);
    _folderUpButton->setSize(cSize(w, h));
    _folderUpButton->pressed.connect(_mainList, &cMainList::backParentDir);
    addChildWindow(_folderUpButton);

    x = ini.GetInt("folder text", "x", 20);
    y = ini.GetInt("folder text", "y", 2);
    w = ini.GetInt("folder text", "w", 160);
    h = ini.GetInt("folder text", "h", 16);
    _folderText = new cStaticText(x, y, w, h, this, "");
    _folderText->setRelativePosition(cPoint(x, y));
    _folderText->setTextColor(ini.GetInt("folder text", "color", 0));
    addChildWindow(_folderText);

    // init startmenu
    _startMenu = new cStartMenu(160, 40, 61, 108, this, "start menu");
    //_startMenu->setRelativePosition( cPoint(160, 40) );
    _startMenu->init();
    _startMenu->itemClicked.connect(this, &cMainWnd::startMenuItemClicked);
    _startMenu->hide();
    _startMenu->setRelativePosition(_startMenu->position());
    addChildWindow(_startMenu);
    //windowManager().addWindow( _startMenu );
    dbg_printf("startMenu %08x\n", _startMenu);

    arrangeChildren();
}

void cMainWnd::draw()
{
    cForm::draw();
}

void cMainWnd::listSelChange(u32 i)
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

void cMainWnd::startMenuItemClicked(s16 i)
{
    dbg_printf("start menu item %d\n", i);
    //messageBox( this, "Power Off", "Are you sure you want to turn off ds?", MB_YES | MB_NO );


#pragma region copypaste
    // ------------------- Copy and Paste ---
    // if (START_MENU_ITEM_COPY == i)
    // {
    //     if ("" == _mainList->getSelectedFullPath())
    //         return;
    //     struct stat st;
    //     stat(_mainList->getSelectedFullPath().c_str(), &st);
    //     if (st.st_mode & S_IFDIR)
    //     {
    //         messageBox(this, LANG("no copy dir", "title"), LANG("no copy dir", "text"), MB_YES | MB_NO);
    //         return;
    //     }
    //     setSrcFile(_mainList->getSelectedFullPath(), SFM_COPY);
    // }

    // else if (START_MENU_ITEM_CUT == i)
    // {
    //     if ("" == _mainList->getSelectedFullPath())
    //         return;
    //     struct stat st;
    //     stat(_mainList->getSelectedFullPath().c_str(), &st);
    //     if (st.st_mode & S_IFDIR)
    //     {
    //         messageBox(this, LANG("no copy dir", "title"), LANG("no copy dir", "text"), MB_YES | MB_NO);
    //         return;
    //     }
    //     setSrcFile(_mainList->getSelectedFullPath(), SFM_CUT);
    // }

    // else if (START_MENU_ITEM_PASTE == i)
    // {
    //     bool ret = false;
    //     if (_mainList->IsFavorites())
    //     {
    //         ret = cFavorites::AddToFavorites(getSrcFile());
    //     }
    //     else
    //     {
    //         ret = copyOrMoveFile(_mainList->getCurrentDir());
    //     }
    //     if (ret) // refresh current directory
    //         _mainList->enterDir(_mainList->getCurrentDir());
    // }

    // else if (START_MENU_ITEM_DELETE == i)
    // {
    //     std::string fullPath = _mainList->getSelectedFullPath();
    //     if ("" != fullPath)
    //     {
    //         bool ret = false;
    //         if (_mainList->IsFavorites())
    //         {
    //             ret = cFavorites::RemoveFromFavorites(fullPath);
    //         }
    //         else
    //         {
    //             ret = deleteFile(fullPath);
    //         }
    //         if (ret)
    //             _mainList->enterDir(_mainList->getCurrentDir());
    //     }
    // }
#pragma endregion
    
    if (START_MENU_ITEM_SETTING == i)
    {
        showSettings();
    }

    else if (START_MENU_ITEM_INFO == i)
    {
        showFileInfo();
    }

    // else if (START_MENU_ITEM_HELP == i)
    // {
    //     CIniFile ini(SFN_UI_SETTINGS); //(256-)/2,(192-128)/2, 220, 128
    //     u32 w = 200;
    //     u32 h = 160;
    //     w = ini.GetInt("help window", "w", w);
    //     h = ini.GetInt("help window", "h", h);
    //     cHelpWnd *helpWnd = new cHelpWnd((256 - w) / 2, (192 - h) / 2, w, h, this, LANG("help window", "title"));
    //     helpWnd->doModal();
    //     delete helpWnd;
    // }
    // else if (START_MENU_ITEM_TOOLS == i)
    // {
    //     u32 w = 250;
    //     u32 h = 130;
    //     cExpWnd expWnd((256 - w) / 2, (192 - h) / 2, w, h, NULL, LANG("exp window", "title"));
    //     expWnd.doModal();
    // }
}

void cMainWnd::startButtonClicked()
{
    if (_startMenu->isVisible())
    {
        _startMenu->hide();
    }
    else
    {
        if (!gs().safeMode)
            _startMenu->show();
    }
}

cWindow &cMainWnd::loadAppearance(const std::string &aFileName)
{
    return *this;
}

bool cMainWnd::process(const cMessage &msg)
{
    if (_startMenu->isVisible())
        return _startMenu->process(msg);

    bool ret = false;

    ret = cForm::process(msg);

    if (!ret)
    {
        if (msg.id() > cMessage::keyMessageStart && msg.id() < cMessage::keyMessageEnd)
        {
            ret = processKeyMessage((cKeyMessage &)msg);
        }

        if (msg.id() > cMessage::touchMessageStart && msg.id() < cMessage::touchMessageEnd)
        {
            ret = processTouchMessage((cTouchMessage &)msg);
        }
    }
    return ret;
}

bool cMainWnd::processKeyMessage(const cKeyMessage &msg)
{
    bool ret = false, isL = msg.shift() & cKeyMessage::UI_SHIFT_L;
    bool allow = !gs().safeMode;
    if (msg.id() == cMessage::keyDown)
    {
        switch (msg.keyCode())
        {
        case cKeyMessage::UI_KEY_DOWN:
            _mainList->selectNext();
            ret = true;
            break;
        case cKeyMessage::UI_KEY_UP:
            _mainList->selectPrev();
            ret = true;
            break;

        case cKeyMessage::UI_KEY_LEFT:
            _mainList->selectRow(_mainList->selectedRowId() - _mainList->visibleRowCount());
            ret = true;
            break;

        case cKeyMessage::UI_KEY_RIGHT:
            _mainList->selectRow(_mainList->selectedRowId() + _mainList->visibleRowCount());
            ret = true;
            break;
        case cKeyMessage::UI_KEY_A:
            onKeyAPressed();
            ret = true;
            break;
        case cKeyMessage::UI_KEY_B:
            onKeyBPressed();
            ret = true;
            break;
        case cKeyMessage::UI_KEY_Y:
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
//         case cKeyMessage::UI_KEY_X:
//         {
//             if (isL)
//             {
//                 if (allow)
//                 {
//                     DSRomInfo rominfo;
//                     if (_mainList->getRomInfo(_mainList->selectedRowId(), rominfo) && rominfo.isDSRom() && !rominfo.isHomebrew())
//                     {
//                         cRomInfoWnd::showCheats(_mainList->getSelectedFullPath());
//                     }
//                 }
//                 _processL = false;
//             }
//             else
//             {
// // #if defined(_STORAGE_rpg)
// //                 const std::string dir = _mainList->getCurrentDir();
// //                 if (dir.length() < 5)
// //                 {
// //                     _mainList->enterDir("sd:/");
// //                 }
// //                 else if (dir.substr(0, 5) == "sd:")
// //                 {
// //                     _mainList->enterDir("fat1:/");
// //                 }
// //                 else
// //                 {
// //                     _mainList->enterDir("sd:/");
// //                 }
// // #elif defined(_STORAGE_r4) || defined(_STORAGE_ak2i) || defined(_STORAGE_r4idsn)
// //                 _mainList->enterDir("favorites:/");
// // #endif
//             }
//             ret = true;
//             break;
//         }
        case cKeyMessage::UI_KEY_START:
            startButtonClicked();
            ret = true;
            break;
        case cKeyMessage::UI_KEY_SELECT:
            if (isL)
            {
                if (allow)
                    _mainList->SwitchShowAllFiles();
                _processL = false;
            }
            else
            {
                if (allow)
                    _mainList->setViewMode((cMainList::VIEW_MODE)((_mainList->getViewMode() + 1) % 3));
            }
            ret = true;
            break;
        case cKeyMessage::UI_KEY_L:
            _processL = true;
            ret = true;
            break;
        case cKeyMessage::UI_KEY_R:
           // brightnessButtonClicked();
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
    if (msg.id() == cMessage::keyUp)
    {
        switch (msg.keyCode())
        {
        case cKeyMessage::UI_KEY_L:
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

bool cMainWnd::processTouchMessage(const cTouchMessage &msg)
{
    bool ret = false;

    return ret;
}

void cMainWnd::onKeyYPressed()
{
    if (gs().safeMode)
        return;
    showFileInfo();
}

void cMainWnd::onMainListSelItemClicked(u32 index)
{
    onKeyAPressed();
}

void cMainWnd::onMainListSelItemHeadClicked(u32 index)
{
    onKeyAPressed();
}

void cMainWnd::onKeyAPressed()
{
    launchSelected();
}

void cMainWnd::launchSelected()
{
    dbg_printf("Launch.");
    std::string fullPath = _mainList->getSelectedFullPath();

    if (fullPath[fullPath.size() - 1] == '/')
    {
        _mainList->enterDir(fullPath);
        return;
    }

//     DSRomInfo rominfo;
//     if (!_mainList->getRomInfo(_mainList->selectedRowId(), rominfo))
//         return;

//     //rominfo.loadDSRomInfo( fullPath, false );

//     if (rominfo.isGbaRom())
//     {
//         CGbaLoader(fullPath).Load(false, false);
//         return;
//     }

//     if (!rominfo.isDSRom())
//         return;

//     dbg_printf("(%s)\n", fullPath.c_str());
//     dbg_printf("%d\n", fullPath[fullPath.size() - 1]);

//     std::string title, text;
//     bool show = true;
//     switch (launchRom(fullPath, rominfo, rominfo.isHomebrew() && "akmenu4.nds" == _mainList->getSelectedShowName()))
//     {
// #if defined(_STORAGE_rpg)
//     case ELaunchSDOnly:
//         title = LANG("sd save", "title");
//         text = LANG("sd save", "text");
//         break;
//     case ELaunchRestoreFail:
//         title = LANG("restore save fail", "title");
//         text = LANG("restore save fail", "text");
//         break;
// #endif
// #if defined(_STORAGE_rpg) || defined(_STORAGE_ak2i)
//     case ELaunchSlowSD:
//     {
//         std::string model = sdidGetSDManufacturerName() + " " + sdidGetSDName();
//         title = LANG("unsupported sd", "title");
//         text = LANG("unsupported sd", "text");
//         text = formatString(text.c_str(), model.c_str());
//     }
//     break;
// #endif
//     case ELaunchNoFreeSpace:
//         title = LANG("no free space", "title");
//         text = LANG("no free space", "text");
//         break;
//     default:
//         show = false;
//         break;
//     }
//     if (show)
//         messageBox(this, title, text, MB_OK);
}

void cMainWnd::onKeyBPressed()
{
    _mainList->backParentDir();
}

void cMainWnd::setParam(void)
{
    /*
    cSettingWnd settingWnd(0, 0, 252, 188, NULL, LANG("system setting", "title"));

    //page 1: system
    std::string currentUIStyle = gs().uiName;
    std::vector<std::string> _values;
    u32 uiIndex = 0, langIndex = 0;
    //user interface style
    _values.clear();
    std::vector<std::string> uiNames;
    DIR_ITER *dir = diropen(SFN_UI_DIRECTORY);
    if (NULL != dir)
    {
        struct stat st;
        char longFilename[MAX_FILENAME_LENGTH];
        while (dirnext(dir, longFilename, &st) == 0)
        {
            std::string lfn(longFilename);
            if (lfn != ".." && lfn != ".")
                _values.push_back(lfn);
        }
        dirclose(dir);
        dir = NULL;
    }
    else
    {
        _values.push_back(gs().uiName);
    }
    std::sort(_values.begin(), _values.end());
    for (size_t ii = 0; ii < _values.size(); ++ii)
    {
        if (0 == stricmp(_values[ii].c_str(), gs().uiName.c_str()))
            uiIndex = ii;
    }
    uiNames = _values;
    settingWnd.addSettingItem(LANG("ui style", "text"), _values, uiIndex);

    //language
    _values.clear();
    std::vector<std::string> langNames;
    dir = diropen(SFN_LANGUAGE_DIRECTORY);
    if (NULL != dir)
    {
        struct stat st;
        char longFilename[MAX_FILENAME_LENGTH];
        while (dirnext(dir, longFilename, &st) == 0)
        {
            std::string lfn(longFilename);
            if (lfn != ".." && lfn != ".")
                _values.push_back(lfn);
        }
        dirclose(dir);
        dir = NULL;
    }
    else
    {
        _values.push_back(gs().langDirectory);
    }
    std::sort(_values.begin(), _values.end());
    for (size_t ii = 0; ii < _values.size(); ++ii)
    {
        if (0 == stricmp(_values[ii].c_str(), gs().langDirectory.c_str()))
            langIndex = ii;
    }
    langNames = _values;
    settingWnd.addSettingItem(LANG("language", "text"), _values, langIndex);

    //file list type
    _values.clear();
    for (size_t ii = 0; ii < 3; ++ii)
    {
        std::string itemName = formatString("item%d", ii);
        _values.push_back(LANG("filelist type", itemName));
    }
    settingWnd.addSettingItem(LANG("filelist type", "text"), _values, gs().fileListType);

    _values.clear();
    _values.push_back(LANG("switches", "Disable"));
    _values.push_back(LANG("switches", "Enable"));
    settingWnd.addSettingItem(LANG("system setting", "safe mode"), _values, gs().safeMode);

    //page 2: interface
    settingWnd.addSettingTab(LANG("interface settings", "title"));
    size_t scrollSpeed = 0;
    switch (gs().scrollSpeed)
    {
    case cGlobalSettings::EScrollFast:
        scrollSpeed = 0;
        break;
    case cGlobalSettings::EScrollMedium:
        scrollSpeed = 1;
        break;
    case cGlobalSettings::EScrollSlow:
        scrollSpeed = 2;
        break;
    }
    _values.clear();
    _values.push_back(LANG("scrolling", "fast"));
    _values.push_back(LANG("scrolling", "medium"));
    _values.push_back(LANG("scrolling", "slow"));
    settingWnd.addSettingItem(LANG("interface settings", "scrolling speed"), _values, scrollSpeed);
    _values.clear();
    _values.push_back(LANG("interface settings", "oldschool"));
    _values.push_back(LANG("interface settings", "modern"));
    _values.push_back(LANG("interface settings", "internal"));
    settingWnd.addSettingItem(LANG("interface settings", "filelist style"), _values, gs().viewMode);
    _values.clear();
    _values.push_back(LANG("switches", "Disable"));
    _values.push_back(LANG("switches", "Enable"));
    settingWnd.addSettingItem(LANG("interface settings", "animation"), _values, gs().Animation);
    settingWnd.addSettingItem(LANG("interface settings", "12 hour"), _values, gs().show12hrClock);

    //page 3: filesystem
    settingWnd.addSettingTab(LANG("file settings", "title"));
    _values.clear();
    _values.push_back(LANG("switches", "Disable"));
    _values.push_back(LANG("switches", "Enable"));
    settingWnd.addSettingItem(LANG("file settings", "show hidden files"), _values, gs().showHiddenFiles);
    settingWnd.addSettingItem(LANG("rom trim", "text"), _values, gs().romTrim);
    _values.clear();
    _values.push_back(".nds.sav");
    _values.push_back(".sav");
    settingWnd.addSettingItem(LANG("file settings", "save extension"), _values, gs().saveExt);

    //page 4: patches
    settingWnd.addSettingTab(LANG("setting window", "patches"));
    _values.clear();
    _values.push_back(LANG("switches", "Disable"));
    _values.push_back(LANG("switches", "Enable"));
    settingWnd.addSettingItem(LANG("patches", "cheating system"), _values, gs().cheats);
    settingWnd.addSettingItem(LANG("patches", "reset in game"), _values, gs().softreset);
    settingWnd.addSettingItem(LANG("patches", "homebrew reset"), _values, gs().homebrewreset);
#if defined(_STORAGE_rpg) || defined(_STORAGE_ak2i)
    settingWnd.addSettingItem(LANG("patches", "dma"), _values, gs().dma);
#endif
#if defined(_STORAGE_rpg)
    settingWnd.addSettingItem(LANG("patches", "sd save"), _values, gs().sdsave);
#endif

    //page 5: gba
    settingWnd.addSettingTab(LANG("gba settings", "title"));
    _values.clear();
    _values.push_back(LANG("switches", "Disable"));
    _values.push_back(LANG("switches", "Enable"));
    settingWnd.addSettingItem(LANG("gba settings", "sleephack"), _values, gs().gbaSleepHack);
    settingWnd.addSettingItem(LANG("gba settings", "autosave"), _values, gs().gbaAutoSave);
    _values.clear();
    _values.push_back(LANG("gba settings", "modeask"));
    _values.push_back(LANG("gba settings", "modegba"));
    _values.push_back(LANG("gba settings", "modends"));
    settingWnd.addSettingItem(LANG("gba settings", "mode"), _values, gs().slot2mode);

    u32 ret = settingWnd.doModal();
    if (ID_CANCEL == ret)
        return;

    //page 1: system
    u32 uiIndexAfter = settingWnd.getItemSelection(0, 0);
    u32 langIndexAfter = settingWnd.getItemSelection(0, 1);
    gs().fileListType = settingWnd.getItemSelection(0, 2);
    gs().safeMode = settingWnd.getItemSelection(0, 3);

    //page 2: interface
    switch (settingWnd.getItemSelection(1, 0))
    {
    case 0:
        gs().scrollSpeed = cGlobalSettings::EScrollFast;
        break;
    case 1:
        gs().scrollSpeed = cGlobalSettings::EScrollMedium;
        break;
    case 2:
        gs().scrollSpeed = cGlobalSettings::EScrollSlow;
        break;
    }
    gs().viewMode = settingWnd.getItemSelection(1, 1);
    gs().Animation = settingWnd.getItemSelection(1, 2);
    gs().show12hrClock = settingWnd.getItemSelection(1, 3);

    //page 3: filesystem
    gs().showHiddenFiles = settingWnd.getItemSelection(2, 0);
    gs().romTrim = settingWnd.getItemSelection(2, 1);
    gs().saveExt = settingWnd.getItemSelection(2, 2);

    //page 4: patches
    gs().cheats = settingWnd.getItemSelection(3, 0);
    gs().softreset = settingWnd.getItemSelection(3, 1);
    gs().homebrewreset = settingWnd.getItemSelection(3, 2);
#if defined(_STORAGE_rpg) || defined(_STORAGE_ak2i)
    gs().dma = settingWnd.getItemSelection(3, 3);
#endif
#if defined(_STORAGE_rpg)
    gs().sdsave = settingWnd.getItemSelection(3, 4);
#endif

    //page 5: gba
    gs().gbaSleepHack = settingWnd.getItemSelection(4, 0);
    gs().gbaAutoSave = settingWnd.getItemSelection(4, 1);
    gs().slot2mode = settingWnd.getItemSelection(4, 2);

    if (uiIndex != uiIndexAfter)
    {
        u32 ret = messageBox(this,
                             LANG("ui style changed", "title"),
                             LANG("ui style changed", "text"), MB_YES | MB_NO);
        if (ID_YES == ret)
        {
            gs().uiName = uiNames[uiIndexAfter];
            gs().langDirectory = langNames[langIndexAfter];
            gs().saveSettings();
#if defined(_STORAGE_rpg)
            loadRom("sd:/akmenu4.nds", 0, 0, 0);
#elif defined(_STORAGE_r4)
            loadRom("sd:/_ds_menu.dat", "", 0, 0, 0);
#elif defined(_STORAGE_ak2i)
            loadRom("sd:/akmenu4.nds", "", 0, 0, 0);
#elif defined(_STORAGE_r4idsn)
            loadRom("sd:/_dsmenu.dat", "", 0, 0, 0);
#endif
        }
    }

    if (langIndex != langIndexAfter)
    {
        u32 ret = messageBox(this,
                             LANG("language changed", "title"),
                             LANG("language changed", "text"), MB_YES | MB_NO);
        if (ID_YES == ret)
        {
            gs().langDirectory = langNames[langIndexAfter];
            gs().saveSettings();
#if defined(_STORAGE_rpg)
            loadRom("sd:/akmenu4.nds", 0, 0, 0);
#elif defined(_STORAGE_r4)
            loadRom("sd:/_ds_menu.dat", "", 0, 0, 0);
#elif defined(_STORAGE_ak2i)
            loadRom("sd:/akmenu4.nds", "", 0, 0, 0);
#elif defined(_STORAGE_r4idsn)
            loadRom("sd:/_dsmenu.dat", "", 0, 0, 0);
#endif
        }
    }

    gs().saveSettings();
    _mainList->setViewMode((cMainList::VIEW_MODE)gs().viewMode);*/
}

void cMainWnd::showSettings(void)
{
//     // if (gs().safeMode)
//     //     return;
//     // u8 currentFileListType = gs().fileListType, currentShowHiddenFiles = gs().showHiddenFiles;
//    // setParam();
//     if (gs().fileListType != currentFileListType || gs().showHiddenFiles != currentShowHiddenFiles)
//     {
//         _mainList->enterDir(_mainList->getCurrentDir());
//     }
}

void cMainWnd::showFileInfo()
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

    cRomInfoWnd *romInfoWnd = new cRomInfoWnd((256 - w) / 2, (192 - h) / 2, w, h, this, LANG("rom info", "title"));
    std::string showName = _mainList->getSelectedShowName();
    std::string fullPath = _mainList->getSelectedFullPath();
    romInfoWnd->setFileInfo(fullPath, showName);
    romInfoWnd->setRomInfo(rominfo);
    romInfoWnd->doModal();
    rominfo = romInfoWnd->getRomInfo();
    _mainList->setRomInfo(_mainList->selectedRowId(), rominfo);

    delete romInfoWnd;
}

void cMainWnd::onFolderChanged()
{
    resetInputIdle();
    std::string dirShowName = _mainList->getCurrentDir();

    if (dirShowName.substr(0, 5) == SD_ROOT)
        dirShowName.replace(0, 4, "SD");
    

    dbg_printf("%s\n", _mainList->getSelectedFullPath().c_str());

    _folderText->setText(dirShowName);
}

void cMainWnd::onAnimation(bool &anAllow)
{
    if (_startMenu->isVisible())
        anAllow = false;
    else if (windowManager().currentWindow() != this)
        anAllow = false;
}

cWindow *cMainWnd::windowBelow(const cPoint &p)
{
    cWindow *wbp = cForm::windowBelow(p);
    if (_startMenu->isVisible() && wbp != _startButton)
        wbp = _startMenu;
    return wbp;
}
