/*
    mainlist.h
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

#ifndef _MAINLIST_H_
#define _MAINLIST_H_

#include <nds.h>
#include "ui/listview.h"
#include "ui/sigslot.h"
#include "ui/keymessage.h"
#include "ui/touchmessage.h"
#include "dsrom.h"
#include "ui/zoomingicon.h"

#define SD_ROOT "/"

class cMainList : public akui::cListView
{
public:
  enum VIEW_MODE
  {
    VM_LIST = 0,
    VM_ICON,
    VM_INTERNAL
  };

  enum //COLUMN_LIST
  {
    ICON_COLUMN = 0,
    SHOWNAME_COLUMN = 1,
    INTERNALNAME_COLUMN = 2,
    REALNAME_COLUMN = 3,
    SAVETYPE_COLUMN = 4,
    FILESIZE_COLUMN = 5
  };

public:
  cMainList(s32 x, s32 y, u32 w, u32 h, cWindow *parent, const std::string &text);

  ~cMainList();

public:
  int init();

  void addDirEntry(int pos, const std::string row1, const std::string row2, const std::string path, const std::string &bannerKey, const u8 *banner);

  bool enterDir(const std::string &dirName);

  void backParentDir();

  void refresh();

  std::string getCurrentDir();

  bool getRomInfo(u32 rowIndex, DSRomInfo &info) const;

  void setRomInfo(u32 rowIndex, const DSRomInfo &info);

  void setViewMode(VIEW_MODE mode);

  std::string getSelectedFullPath();

  std::string getSelectedShowName();

  VIEW_MODE getViewMode() { return _viewMode; }

  void arrangeIcons();

  akui::Signal1<u32> selectedRowHeadClicked;

  akui::Signal0 directoryChanged;

  akui::Signal1<bool &> animateIcons;

public:
  bool IsFavorites(void);

  void SwitchShowAllFiles(void);

protected:
  void draw();

  void drawIcons();

  enum
  {
    POSITION = 0,
    CONTENT = 1
  };

  void updateActiveIcon(bool updateContent);
  void updateInternalNames(void);

protected:
  void onSelectedRowClicked(u32 index);

  void onSelectChanged(u32 index);

  void onScrolled(u32 index);

protected:
  VIEW_MODE _viewMode;

  std::string _currentDir;

  std::vector<std::string> _extnameFilter;

  std::vector<DSRomInfo> _romInfoList;

  cZoomingIcon _activeIcon;

  float _activeIconScale;

  bool _showAllFiles;
};

#endif //_MAINLIST_H_
