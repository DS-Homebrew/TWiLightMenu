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
#include "common/singleton.h"
#include "drawing/bmp15.h"
#include "drawing/gdi.h"

class FileIconItem
{
public:
  FileIconItem() : _loaded(false){};
  FileIconItem(const std::string &aFolderName, const std::string &aFileName);
  const std::string &FileName(void) const { return _filename; };
  const std::string &FolderName(void) const { return _foldername; };
  BMP15 &Icon(void);
  bool operator<(const FileIconItem &aValue) const;

private:
  void Load(void);

private:
  bool _loaded;
  std::string _foldername;
  std::string _filename;
  BMP15 _icon;
};

class FileIcons
{
public:
  FileIcons();
  s32 Icon(const std::string &aValue);
  void Draw(s32 idx, u8 x, u8 y, GRAPHICS_ENGINE engine);
  void DrawMem(s32 idx, void *mem);

private:
  std::vector<FileIconItem> _icons;

private:
  typedef std::set<FileIconItem> IconPaths;
  static void LoadFolder(IconPaths &aPaths, const std::string &aFolder);
};

typedef singleton<FileIcons> fileIcons_s;
inline FileIcons &fileIcons() { return fileIcons_s::instance(); }

#endif
