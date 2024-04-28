/*
    fileicons.cpp
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

#include "fileicons.h"
#include "systemfilenames.h"
#include "ui/binaryfind.h"
#include "windows/batteryicon.h"
#include "icons.h"
#include <sys/dirent.h>
#include <sys/iosupport.h>
#include <sys/fcntl.h>
FileIconItem::FileIconItem(const std::string &aFolderName, const std::string &aFileName) : _loaded(false), _foldername(aFolderName), _filename(aFileName)
{
}

BMP15 &FileIconItem::Icon(void)
{
  Load();
  return _icon;
}

void FileIconItem::Load(void)
{
  if (!_loaded)
  {
    _icon = createBMP15FromFile(_foldername + _filename + ".bmp");
    _loaded = true;
  }
}

static bool Comp(const FileIconItem &item1, const FileIconItem &item2)
{
  return strcasecmp(item1.FileName().c_str(), item2.FileName().c_str()) < 0;
}

bool FileIconItem::operator<(const FileIconItem &aValue) const
{
  return Comp(*this, aValue);
}

FileIcons::FileIcons()
{
  IconPaths *paths = new IconPaths;
  LoadFolder(*paths, SFN_UI_ICONS_DIRECTORY);
  LoadFolder(*paths, SFN_ICONS_DIRECTORY);
  for (IconPaths::const_iterator it = paths->begin(); it != paths->end(); ++it)
  {
    _icons.push_back(*it);
  }
  delete paths;
}

void FileIcons::LoadFolder(IconPaths &aPaths, const std::string &aFolder)
{
  DIR *dir = opendir(aFolder.c_str());
  if (NULL != dir)
  {
    struct stat st;
    struct dirent *entry;
    char lfn[strlen(aFolder.c_str()) + 256 + 2] = {'\0'}; 

    // char longFilename[MAX_FILENAME_LENGTH];
    while ((entry = readdir(dir)) != NULL)
    {
      snprintf(lfn, sizeof(lfn), "%s/%s", aFolder.c_str(), entry->d_name);
      stat(lfn, &st);
      if ((st.st_mode & S_IFDIR) == 0)
      {
        size_t len = sizeof(entry->d_name);
        if (len > 4)
        {
          char *extName = entry->d_name + len - 4;
          if (strcasecmp(extName, ".bmp") == 0)
          {
            *extName = 0;
            aPaths.insert(FileIconItem(aFolder, std::string(lfn)));
          }
        }
      }
    }
    closedir(dir);
  }
}

s32 FileIcons::Icon(const std::string &aValue)
{
  if (!_icons.size())
    return -1;
  std::vector<FileIconItem>::iterator result = akui::binary_find(_icons.begin(), _icons.end(), FileIconItem("", aValue), Comp);
  if (result == _icons.end())
  {
    return -1;
  }
  BMP15 &image = (*result).Icon();
  return ((image.valid() && image.width() == 32 && image.height() == 32) ? (result - _icons.begin()) : -1);
}

void FileIcons::Draw(s32 idx, u8 x, u8 y, GRAPHICS_ENGINE engine)
{
  gdi().maskBlt(_icons[idx].Icon().buffer(), x, y, 32, 32, engine);
}

void FileIcons::DrawMem(s32 idx, void *mem)
{
  Icons::maskBlt((const u16 *)_icons[idx].Icon().buffer(), (u16 *)mem);
}
