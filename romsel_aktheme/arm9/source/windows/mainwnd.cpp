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
#include "common/cardlaunch.h"
#include "common/systemdetails.h"
#include "common/dsargv.h"
#include "common/flashcardlaunch.h"
#include "common/gbaswitch.h"
#include "common/unlaunchboot.h"
//#include "files.h"

#include "common/inifile.h"
#include "language.h"
#include "common/dsimenusettings.h"
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

MainWnd::MainWnd(s32 x, s32 y, u32 w, u32 h, Window *parent, const std::string &text)
    : Form(x, y, w, h, parent, text), _mainList(NULL), _startMenu(NULL), _startButton(NULL),
      _brightnessButton(NULL), _folderUpButton(NULL), _folderText(NULL), _processL(false)
{
}

MainWnd::~MainWnd()
{
    delete _folderText;
    delete _folderUpButton;
    delete _brightnessButton;
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
    _mainList = new MainList(4, 20, 248, 152, this, "main list");
    _mainList->setRelativePosition(Point(4, 20));
    _mainList->init();
    _mainList->selectChanged.connect(this, &MainWnd::listSelChange);
    _mainList->selectedRowClicked.connect(this, &MainWnd::onMainListSelItemClicked);
    _mainList->selectedRowHeadClicked.connect(this, &MainWnd::onMainListSelItemHeadClicked);
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
    //_startButton = new Button( 0, 172, 48, 18, this, " Start" );
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
    // x = ini.GetInt("brightness btn", "x", 240);
    // y = ini.GetInt("brightness btn", "y", 1);
    // w = ini.GetInt("brightness btn", "w", 16);
    // h = ini.GetInt("brightness btn", "h", 16);
    // _brightnessButton = new Button(x, y, w, h, this, "");
    // _brightnessButton->setRelativePosition(Point(x, y));
    // _brightnessButton->loadAppearance(SFN_BRIGHTNESS_BUTTON);
    // _brightnessButton->pressed.connect(this, &MainWnd::brightnessButtonClicked);
    // addChildWindow(_brightnessButton);

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

    // init startmenu
    _startMenu = new StartMenu(160, 40, 61, 108, this, "start menu");
    //_startMenu->setRelativePosition( Point(160, 40) );
    _startMenu->init();
    _startMenu->itemClicked.connect(this, &MainWnd::startMenuItemClicked);
    _startMenu->hide();
    _startMenu->setRelativePosition(_startMenu->position());
    addChildWindow(_startMenu);
    //windowManager().addWindow( _startMenu );
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
    //         ret = Favorites::AddToFavorites(getSrcFile());
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
    //             ret = Favorites::RemoveFromFavorites(fullPath);
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
            //         case KeyMessage::UI_KEY_X:
            //         {
            //             if (isL)
            //             {
            //                 if (allow)
            //                 {
            //                     DSRomInfo rominfo;
            //                     if (_mainList->getRomInfo(_mainList->selectedRowId(), rominfo) && rominfo.isDSRom() && !rominfo.isHomebrew())
            //                     {
            //                         RomInfoWnd::showCheats(_mainList->getSelectedFullPath());
            //                     }
            //                 }
            //                 _processL = false;
            //             }
            //             else
            //             {

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
    bool ret = false;

    return ret;
}

void MainWnd::onKeyYPressed()
{
    showFileInfo();
}

void MainWnd::onMainListSelItemClicked(u32 index)
{
    onKeyAPressed();
}

void MainWnd::onMainListSelItemHeadClicked(u32 index)
{
    onKeyAPressed();
}

void MainWnd::onKeyAPressed()
{
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

void MainWnd::bootBootstrap(PerGameSettings &gameConfig, DSRomInfo &rominfo)
{
    dbg_printf("%s", _mainList->getSelectedShowName().c_str());
    std::string fullPath = _mainList->getSelectedFullPath();

    BootstrapConfig config(fullPath, std::string((char *)rominfo.saveInfo().gameCode), rominfo.saveInfo().gameSdkVersion);

    config.asyncPrefetch(gameConfig.asyncPrefetch == PerGameSettings::EDefault ? ms().bstrap_asyncPrefetch : (bool)gameConfig.asyncPrefetch)
        .cpuBoost(gameConfig.boostCpu == PerGameSettings::EDefault ? ms().boostCpu : (bool)gameConfig.boostCpu)
        .vramBoost(gameConfig.boostVram == PerGameSettings::EDefault ? ms().boostVram : (bool)gameConfig.boostVram)
        .soundFix(gameConfig.soundFix == PerGameSettings::EDefault ? ms().soundFix : (bool)gameConfig.soundFix)
        .nightlyBootstrap(ms().bootstrapFile);

    // GameConfig is default, global is not default
    if (gameConfig.language == PerGameSettings::ELangDefault && ms().bstrap_language != DSiMenuPlusPlusSettings::ELangDefault)
    {
        config.language(ms().bstrap_language);
    }
    // GameConfig is system, or global is defaut
    else if (gameConfig.language == PerGameSettings::ELangSystem || ms().bstrap_language == DSiMenuPlusPlusSettings::ELangDefault)
    {
        config.language(PersonalData->language);
    }
    else
    // gameConfig is not default
    {
        config.language(gameConfig.language);
    }

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

void MainWnd::bootFlashcard(const std::string &fullPath)
{
    int err = loadGameOnFlashcard(fullPath.c_str());
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

// void MainWnd::bootUnlaunch(DSRomInfo& rominfo)
// {

// }

void MainWnd::launchSelected()
{
    dbg_printf("Launch.");
    std::string fullPath = _mainList->getSelectedFullPath();

    if (fullPath[fullPath.size() - 1] == '/')
    {
        _mainList->enterDir(fullPath);
        return;
    }

    ms().romfolder = _mainList->getCurrentDir();
    ms().saveSettings();

    DSRomInfo rominfo;
    if (!_mainList->getRomInfo(_mainList->selectedRowId(), rominfo))
        return;

    // Launch DSiWare
    if (rominfo.isDSiWare() && rominfo.isArgv())
    {
        ms().launchType = DSiMenuPlusPlusSettings::ENoLaunch;
        ms().saveSettings();
        dsiLaunch(rominfo.saveInfo().dsiTid);
        return;
    }

    // Todo: Don't boot on 3DS.
    if (rominfo.isDSiWare() && !sys().flashcardUsed())
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

        if (!unlaunch.prepare())
        {
            messageBox(this, LANG("game launch", "Unlaunch Error"), LANG("game launch", "bootthis exists"), MB_OK);
            progressWnd().hide();
            return;
        }
        ms().launchType = DSiMenuPlusPlusSettings::EDSiWareLaunch;
        ms().saveSettings();
        progressWnd().hide();
        messageBox(this, LANG("game launch", "unlaunch boot"), LANG("game launch", "unlaunch instructions"), MB_HOLD_X);
        unlaunch.launch();
    }

    if (rominfo.isDSRom())
    {
        ms().launchType = DSiMenuPlusPlusSettings::ESDFlashcardLaunch;
        ms().saveSettings();
        PerGameSettings gameConfig(_mainList->getSelectedShowName());
        // Direct Boot for homebrew.
        if (gameConfig.directBoot && rominfo.isHomebrew())
        {
            bootArgv(rominfo);
            return;
        }

        else if (sys().flashcardUsed())
        {
            dbg_printf("Flashcard Launch: %s\n", fullPath.c_str());
            bootFlashcard(fullPath);
            return;
        }
        else
        {
            bootBootstrap(gameConfig, rominfo);
            return;
        }
    }

    std::string extension;
    size_t lastDotPos = fullPath.find_last_of('.');
    if (fullPath.npos != lastDotPos)
        extension = fullPath.substr(lastDotPos);

    // NES Launch
    if (extension == ".nes")
    {
        ms().launchType = DSiMenuPlusPlusSettings::ENESDSLaunch;
        ms().saveSettings();
        if (sys().flashcardUsed())
        {
            bootFile(NESDS_FC, fullPath);
        }
        else
        {
            bootFile(NESDS_SD, fullPath);
        }
    }

    // GB Launch
    if (extension == ".gb" || extension == ".gbc")
    {
        ms().launchType = DSiMenuPlusPlusSettings::EGameYobLaunch;
        ms().saveSettings();
        if (sys().flashcardUsed())
        {
            bootFile(GAMEYOB_FC, fullPath);
        }
        else
        {
            bootFile(GAMEYOB_SD, fullPath);
        }
        // gb
    }
}

void MainWnd::onKeyBPressed()
{
    _mainList->backParentDir();
}

void MainWnd::showSettings(void)
{
    dbg_printf("Launch titleandsettings...");
    LoaderConfig settingsLoader(DSIMENUPP_SETTINGS_SRL, DSIMENUPP_INI);
    settingsLoader.option("SRLOADER", "GOTOSETTINGS", true);

    if (int err = settingsLoader.launch())
    {
        std::string errorString = formatString(LANG("game launch", "error").c_str(), err);
        messageBox(this, LANG("game launch", "NDS Bootstrap Error"), errorString, MB_OK);
    }
}

void MainWnd::bootSlot1(void)
{
    dbg_printf("Launch Slot1..\n");
    if (!ms().slot1LaunchMethod || sys().arm7SCFGLocked())
    {
        cardLaunch();
        return;
    }

    LoaderConfig slot1Loader(SLOT1_SRL, DSIMENUPP_INI);
    if (int err = slot1Loader.launch())
    {
       std::string errorString = formatString(LANG("game launch", "error").c_str(), err);
        messageBox(this, LANG("game launch", "NDS Bootstrap Error"), errorString, MB_OK);
    }
}

void MainWnd::bootGbaRunner(void)
{
    if (sys().flashcardUsed() && ms().useGbarunner)
    {
        bootFlashcard(GBARUNNER_FC);
        return;
    }

    if (sys().flashcardUsed() && !ms().useGbarunner)
    {
        gbaSwitch();
        return;
    }

    BootstrapConfig gbaRunner(GBARUNNER_BOOTSTRAP, "", 0);
    if (int err = gbaRunner.launch())
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

    if (dirShowName.substr(0, 1) == SD_ROOT)
        dirShowName.replace(0, 1, "SD:/");

    if (!strncmp(dirShowName.c_str(), "^*::", 2))
    {

        if (dirShowName == SPATH_TITLEANDSETTINGS)
        {
            showSettings();
        }

        if (dirShowName == SPATH_SLOT1)
        {
            if (!ms().slot1LaunchMethod || sys().arm7SCFGLocked())
            {
                cardLaunch();
            }
            else
            {
                bootSlot1();
            }
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
