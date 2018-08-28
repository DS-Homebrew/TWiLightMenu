/*
    fileicons.h
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

#ifndef __FILEICONS_H__
#define __FILEICONS_H__

#include <vector>
#include <set>
#include "singleton.h"
#include "drawing/bmp15.h"
#include "drawing/gdi.h"

class cFileIconItem
{
public:
  cFileIconItem() : _loaded(false){};
  cFileIconItem(const std::string &aFolderName, const std::string &aFileName);
  const std::string &FileName(void) const { return _filename; };
  const std::string &FolderName(void) const { return _foldername; };
  cBMP15 &Icon(void);
  bool operator<(const cFileIconItem &aValue) const;

private:
  void Load(void);

private:
  bool _loaded;
  std::string _foldername;
  std::string _filename;
  cBMP15 _icon;
};

class cFileIcons
{
public:
  cFileIcons();
  s32 Icon(const std::string &aValue);
  void Draw(s32 idx, u8 x, u8 y, GRAPHICS_ENGINE engine);
  void DrawMem(s32 idx, void *mem);

private:
  std::vector<cFileIconItem> _icons;

private:
  typedef std::set<cFileIconItem> cIconPaths;
  static void LoadFolder(cIconPaths &aPaths, const std::string &aFolder);
};

typedef t_singleton<cFileIcons> fileIcons_s;
inline cFileIcons &fileIcons() { return fileIcons_s::instance(); }

#endif
