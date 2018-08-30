/*
    mainlist.cpp
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

//�

#include <sys/dirent.h>
#define ATTRIB_HID 0x02
#include "mainlist.h"
//#include "files.h"
#include "windows/startmenu.h"
#include "systemfilenames.h"
#include "bootstrap_support/systemdetails.h"
#include "ui/windowmanager.h"
#include "tool/timetool.h"
#include "tool/memtool.h"
#include "tool/dbgtool.h"
#include "inifile.h"
#include "unknown_banner_bin.h"
#include "nds_save_banner_bin.h"
#include "nand_banner_bin.h"
#include "microsd_banner_bin.h"
#include "gba_banner_bin.h"
#include "gbarom_banner_bin.h"
#include "sysmenu_banner_bin.h"
#include "settings_banner_bin.h"
#include "folder_banner_bin.h"
#include "ui/progresswnd.h"
#include "language.h"
#include "unicode.h"



using namespace akui;

MainList::MainList(s32 x, s32 y, u32 w, u32 h, Window *parent, const std::string &text)
    : ListView(x, y, w, h, parent, text, gs().scrollSpeed), _showAllFiles(false)
{
    _viewMode = VM_LIST;
    _activeIconScale = 1;
    _activeIcon.hide();
    _activeIcon.update();
    animationManager().addAnimation(&_activeIcon);
    dbg_printf("_activeIcon.init\n");
}

MainList::~MainList()
{
}

int MainList::init()
{
    CIniFile ini(SFN_UI_SETTINGS);
    _textColor = ini.GetInt("main list", "textColor", RGB15(7, 7, 7));
    _textColorHilight = ini.GetInt("main list", "textColorHilight", RGB15(31, 0, 31));
    _selectionBarColor1 = ini.GetInt("main list", "selectionBarColor1", RGB15(16, 20, 24));
    _selectionBarColor2 = ini.GetInt("main list", "selectionBarColor2", RGB15(20, 25, 0));
    _selectionBarOpacity = ini.GetInt("main list", "selectionBarOpacity", 100);

    insertColumn(ICON_COLUMN, "icon", 0);
    insertColumn(SHOWNAME_COLUMN, "showName", 0);
    insertColumn(INTERNALNAME_COLUMN, "internalName", 0);
    insertColumn(REALNAME_COLUMN, "realName", 0); // hidden column for contain real filename
    insertColumn(SAVETYPE_COLUMN, "saveType", 0);
    insertColumn(FILESIZE_COLUMN, "fileSize", 0);

    setViewMode((MainList::VIEW_MODE)gs().viewMode);

    _activeIcon.hide();

    return 1;
}

static bool itemSortComp(const ListView::itemVector &item1, const ListView::itemVector &item2)
{
    const std::string &fn1 = item1[MainList::REALNAME_COLUMN].text();
    const std::string &fn2 = item2[MainList::REALNAME_COLUMN].text();
    if ("../" == fn1)
        return true;
    if ("../" == fn2)
        return false;
    if ('/' == fn1[fn1.size() - 1] && '/' == fn2[fn2.size() - 1])
        return fn1 < fn2;
    if ('/' == fn1[fn1.size() - 1])
        return true;
    if ('/' == fn2[fn2.size() - 1])
        return false;

    return fn1 < fn2;
}

static bool extnameFilter(const std::vector<std::string> &extNames, std::string extName)
{
    if (0 == extNames.size())
        return true;

    for (size_t i = 0; i < extName.size(); ++i)
        extName[i] = tolower(extName[i]);

    for (size_t i = 0; i < extNames.size(); ++i)
    {
        if (extName == extNames[i])
        {
            return true;
        }
    }
    return false;
}

void MainList::addDirEntry(int pos, const std::string row1, const std::string row2, const std::string path, const std::string &bannerKey, const u8 *banner)
{
    std::vector<std::string> a_row;
    a_row.push_back(""); // make a space for icon
    DSRomInfo rominfo;

    a_row.push_back(row1);
    a_row.push_back(row2);
    a_row.push_back(path);

    if (!bannerKey.empty())
    {
        rominfo.setBanner(bannerKey, banner);
    }

    insertRow(pos, a_row);
    _romInfoList.push_back(rominfo);
}

bool MainList::enterDir(const std::string &dirName)
{
    dbg_printf("Enter Dir: %s\n", dirName.c_str());
    if (SPATH_ROOT == dirName) // select RPG or SD card
    {
        removeAllRows();
        _romInfoList.clear();

		if (!sys().flashcardUsed()) {
			// TODO: Show "SD Card" if consoleModel is < 3, else show "microSD Card"
			addDirEntry(0, LANG("mainlist", "SD Card"), "", SD_ROOT, "usd", microsd_banner_bin);
		} else {
			if (sys().isRegularDS()) {
				addDirEntry(0, LANG("mainlist", "microSD Card"), "", SD_ROOT, "usd", microsd_banner_bin);
			} else {
				addDirEntry(0, LANG("mainlist", "SLOT-1 microSD Card"), "", SD_ROOT, "usd", microsd_banner_bin);
			}
		}
        addDirEntry(1, "GBARunner2", "", SPATH_GBARUNNER, "gbarunner", gbarom_banner_bin);
        if (!sys().flashcardUsed()) {
			addDirEntry(2, "SLOT-1 Card", "", SPATH_SLOT1, "slot1", nand_banner_bin);
            addDirEntry(3, "System Menu", "", SPATH_SYSMENU, "sysmenu", sysmenu_banner_bin);
			addDirEntry(4, "Settings", "", SPATH_TITLEANDSETTINGS, "titleandsettings", settings_banner_bin);
			//addDirEntry(5, "System Settings", "", SPATH_SYSTEMSETTINGS, "systemsettings", settings_banner_bin);
		} else {
			addDirEntry(2, "Settings", "", SPATH_TITLEANDSETTINGS, "titleandsettings", settings_banner_bin);
		}
        _currentDir = SPATH_ROOT;
        directoryChanged();
        return true;
    }

    if (!strncmp(dirName.c_str(), "^*::", 2)) {
        dbg_printf("Special directory entered");
        _currentDir = dirName;
        directoryChanged();
        return true;
    }

    DIR *dir = NULL;

    dir = opendir(dirName.c_str());

    if (dir == NULL)
    {
        if (SD_ROOT == dirName)
        {
            std::string title = LANG("sd card error", "title");
            std::string sdError = LANG("sd card error", "text");
            messageBox(NULL, title, sdError, MB_OK);
        }
        dbg_printf("Unable to open directory<%s>.\n", dirName.c_str());
        return false;
    }

    removeAllRows();
    _romInfoList.clear();

    std::vector<std::string> extNames;
    // Push launchargs here.
    extNames.push_back(".nds");
    extNames.push_back(".ids");

    if (gs().showGbaRoms > 0)
        extNames.push_back(".gba");
    if (gs().fileListType > 0)
        extNames.push_back(".sav");
    if (_showAllFiles || gs().fileListType > 1)
        extNames.clear();
    std::vector<std::string> savNames;
    savNames.push_back(".sav");

    struct stat st;
    dirent *direntry;
    std::string extName;
    u8 attr = 0;
    char lfnBuf[256];
    // list dir
    {
        cwl();
        if (dir)
        {
            while ((direntry = readdir(dir)) != NULL)
            {
                snprintf(lfnBuf, sizeof(lfnBuf), "%s/%s", dirName.c_str(), direntry->d_name);
                stat(lfnBuf, &st);
                attr = st.st_spare1;
                std::string lfn(direntry->d_name);

                // st.st_mode & S_IFDIR indicates a directory
                size_t lastDotPos = lfn.find_last_of('.');
                if (lfn.npos != lastDotPos)
                    extName = lfn.substr(lastDotPos);
                else
                    extName = "";

                dbg_printf("%s: %s %s\n", (st.st_mode & S_IFDIR ? " DIR" : "FILE"), lfnBuf, extName.c_str());
                bool showThis = (st.st_mode & S_IFDIR) ? (strcmp(lfn.c_str(), ".") && strcmp(lfn.c_str(), "..")) : extnameFilter(extNames, extName);
                showThis = showThis && (_showAllFiles || gs().showHiddenFiles || !(attr & ATTRIB_HID));

                if (showThis)
                {
                    u32 row_count = getRowCount();
                    size_t insertPos(row_count);

                    std::string real_name = dirName + lfn;
                    if (st.st_mode & S_IFDIR)
                    {
                        real_name += "/";
                    }

                    addDirEntry(insertPos, lfn, "", real_name, "", NULL);
                }
            }
            closedir(dir);
        }
        std::sort(_rows.begin(), _rows.end(), itemSortComp);

        for (size_t ii = 0; ii < _rows.size(); ++ii)
        {

            DSRomInfo &rominfo = _romInfoList[ii];
            std::string filename = _rows[ii][REALNAME_COLUMN].text();
            size_t lastDotPos = filename.find_last_of('.');
            if (filename.npos != lastDotPos)
                extName = filename.substr(lastDotPos);
            else
                extName = "";
            for (size_t jj = 0; jj < extName.size(); ++jj)
                extName[jj] = tolower(extName[jj]);

            if ('/' == filename[filename.size() - 1])
            {
                rominfo.setBanner("folder", folder_banner_bin);
            }
            else
            {
                bool allowExt = true, allowUnknown = false;
                if (".gba" == extName)
                {
                    rominfo.MayBeGbaRom(filename);
                }
                else if (".nds" != extName && ".ids" != extName)
                {
                    memcpy(&rominfo.banner(), unknown_banner_bin, sizeof(tNDSBanner));
                    allowUnknown = true;
                }
                else
                {
                    rominfo.MayBeDSRom(filename);
                    allowExt = false;
                }
                rominfo.setExtIcon(_rows[ii][SHOWNAME_COLUMN].text());
                if (allowExt && extName.length() && !rominfo.isExtIcon())
                    rominfo.setExtIcon(extName.substr(1));
                if (allowUnknown && !rominfo.isExtIcon())
                    rominfo.setExtIcon("unknown");
            }
        }
        _currentDir = dirName;
    }

    directoryChanged();

    return true;
}

void MainList::onSelectChanged(u32 index)
{
    dbg_printf("%s\n", _rows[index][3].text().c_str());
}

void MainList::onSelectedRowClicked(u32 index)
{
    const INPUT &input = getInput();
    //dbg_printf("%d %d", input.touchPt.px, _position.x );
    if (input.touchPt.px > _position.x && input.touchPt.px < _position.x + 32)
        selectedRowHeadClicked(index);
}

void MainList::onScrolled(u32 index)
{
    _activeIconScale = 1;
    //updateActiveIcon( CONTENT );
}

void MainList::backParentDir()
{
    if ("..." == _currentDir)
        return;

    dbg_printf("CURDIR:\"%s\"\n", _currentDir.c_str());
    if ("" == _currentDir || SD_ROOT == _currentDir)
    {
        dbg_printf("Entering HOME\n");
        enterDir(SPATH_ROOT);
        return;
    }

    size_t pos = _currentDir.rfind("/", _currentDir.size() - 2);
    std::string parentDir = _currentDir.substr(0, pos + 1);
    dbg_printf("%s->%s\n", _currentDir.c_str(), parentDir.c_str());

    std::string oldCurrentDir = _currentDir;

    if (enterDir(parentDir))
    { // select last entered director
        for (size_t i = 0; i < _rows.size(); ++i)
        {
            if (parentDir + _rows[i][SHOWNAME_COLUMN].text() == oldCurrentDir)
            {
                selectRow(i);
            }
        }
    }
}

std::string MainList::getSelectedFullPath()
{
    if (!_rows.size())
        return std::string("");
    return _rows[_selectedRowId][REALNAME_COLUMN].text();
}

std::string MainList::getSelectedShowName()
{
    if (!_rows.size())
        return std::string("");
    return _rows[_selectedRowId][SHOWNAME_COLUMN].text();
}

bool MainList::getRomInfo(u32 rowIndex, DSRomInfo &info) const
{
    if (rowIndex < _romInfoList.size())
    {
        info = _romInfoList[rowIndex];
        return true;
    }
    else
    {
        return false;
    }
}

void MainList::setRomInfo(u32 rowIndex, const DSRomInfo &info)
{
    if (!_romInfoList[rowIndex].isDSRom())
        return;

    if (rowIndex < _romInfoList.size())
    {
        _romInfoList[rowIndex] = info;
    }
}

void MainList::draw()
{
    updateInternalNames();
    ListView::draw();
    updateActiveIcon(POSITION);
    drawIcons();
}

void MainList::drawIcons()
{
    if (VM_LIST != _viewMode)
    {
        size_t total = _visibleRowCount;
        if (total > _rows.size() - _firstVisibleRowId)
            total = _rows.size() - _firstVisibleRowId;

        for (size_t i = 0; i < total; ++i)
        {
            if (_firstVisibleRowId + i == _selectedRowId)
            {
                if (_activeIcon.visible())
                {
                    continue;
                }
            }
            s32 itemX = _position.x + 1;
            s32 itemY = _position.y + i * _rowHeight + ((_rowHeight - 32) >> 1) - 1;
            _romInfoList[_firstVisibleRowId + i].drawDSRomIcon(itemX, itemY, _engine);
        }
    }
}

void MainList::setViewMode(VIEW_MODE mode)
{
    if (!_columns.size())
        return;
    _viewMode = mode;
    switch (_viewMode)
    {
    case VM_LIST:
        _columns[ICON_COLUMN].width = 0;
        _columns[SHOWNAME_COLUMN].width = 250;
        _columns[INTERNALNAME_COLUMN].width = 0;
        arangeColumnsSize();
        setRowHeight(15);
        break;
    case VM_ICON:
        _columns[ICON_COLUMN].width = 36;
        _columns[SHOWNAME_COLUMN].width = 214;
        _columns[INTERNALNAME_COLUMN].width = 0;
        arangeColumnsSize();
        setRowHeight(38);
        break;
    case VM_INTERNAL:
        _columns[ICON_COLUMN].width = 36;
        _columns[SHOWNAME_COLUMN].width = 0;
        _columns[INTERNALNAME_COLUMN].width = 214;
        arangeColumnsSize();
        setRowHeight(38);
        break;
    }
    scrollTo(_selectedRowId - _visibleRowCount + 1);
}

void MainList::updateActiveIcon(bool updateContent)
{
    const INPUT &temp = getInput();
    bool allowAnimation = true;
    animateIcons(allowAnimation);

    //do not show active icon when hold key to list files. Otherwise the icon will not show correctly.
    if (getInputIdleMs() > 1000 && VM_LIST != _viewMode && allowAnimation && _romInfoList.size() && 0 == temp.keysHeld && gs().Animation)
    {
        if (!_activeIcon.visible())
        {
            u8 backBuffer[32 * 32 * 2];
            zeroMemory(backBuffer, 32 * 32 * 2);
            _romInfoList[_selectedRowId].drawDSRomIconMem(backBuffer);
            memcpy(_activeIcon.buffer(), backBuffer, 32 * 32 * 2);
            _activeIcon.setBufferChanged();

            s32 itemX = _position.x;
            s32 itemY = _position.y + (_selectedRowId - _firstVisibleRowId) * _rowHeight + ((_rowHeight - 32) >> 1) - 1;
            _activeIcon.setPosition(itemX, itemY);
            _activeIcon.show();
            dbg_printf("sel %d ac ico x %d y %d\n", _selectedRowId, itemX, itemY);
            for (u8 i = 0; i < 8; ++i)
                dbg_printf("%02x", backBuffer[i]);
            dbg_printf("\n");
        }
    }
    else
    {
        if (_activeIcon.visible())
        {
            _activeIcon.hide();
            cwl();
        }
    }
}

std::string MainList::getCurrentDir()
{
    return _currentDir;
}

void MainList::updateInternalNames(void)
{
    if (_viewMode == VM_INTERNAL)
    {
        size_t total = _visibleRowCount;
        if (total > _rows.size() - _firstVisibleRowId)
            total = _rows.size() - _firstVisibleRowId;
        for (size_t ii = 0; ii < total; ++ii)
        {
            if (0 == _rows[_firstVisibleRowId + ii][INTERNALNAME_COLUMN].text().length())
            {
                if (_romInfoList[_firstVisibleRowId + ii].isDSRom())
                {
                    _rows[_firstVisibleRowId + ii][INTERNALNAME_COLUMN].setText(unicode_to_local_string(_romInfoList[_firstVisibleRowId + ii].banner().titles[gs().language], 128, NULL));
                }
                else
                {
                    _rows[_firstVisibleRowId + ii][INTERNALNAME_COLUMN].setText(_rows[_firstVisibleRowId + ii][SHOWNAME_COLUMN].text());
                }
            }
        }
    }
}

void MainList::SwitchShowAllFiles(void)
{
    _showAllFiles = !_showAllFiles;
    enterDir(getCurrentDir());
}
