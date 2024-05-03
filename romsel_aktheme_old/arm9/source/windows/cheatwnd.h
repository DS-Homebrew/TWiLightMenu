/*
    cheatwnd.h
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

#ifndef __CHEATWND_H__
#define __CHEATWND_H__

#include "ui/form.h"
#include "ui/formdesc.h"
#include "ui/statictext.h"
#include "ui/button.h"
#include "ui/listview.h"

class CheatWnd: public akui::Form
{
  public:
    CheatWnd(s32 x,s32 y,u32 w,u32 h,Window* parent,const std::string& text);
    ~CheatWnd();
    bool parse(const std::string& aFileName);
    static bool searchCheatData(FILE* aDat,u32 gamecode,u32 crc32,long& aPos,size_t& aSize);
    static bool romData(const std::string& aFileName,u32& aGameCode,u32& aCrc32);
    void writeCheatsToFile(const char* path);
  protected:
    void draw();
    bool process(const akui::Message& msg);
    Window& loadAppearance(const std::string& aFileName);
  protected:
    bool processKeyMessage(const akui::KeyMessage& msg);
    void onItemClicked(u32 index);
    void onSelect(void);
    void onDeselectAll(void);
    void onInfo(void);
    void onGenerate(void);
    void onCancel(void);
    void onDraw(const akui::ListView::OwnerDraw& data);
    void drawMark(const akui::ListView::OwnerDraw& od,u16 width);
    void generateList(void);
    akui::Button _buttonDeselect;
    akui::Button _buttonInfo;
    akui::Button _buttonGenerate;
    akui::Button _buttonCancel;
    akui::FormDesc _renderDesc;
    akui::ListView _List;
  protected:
    bool parseInternal(FILE* aDat,u32 gamecode,u32 crc32);
    void deselectFolder(size_t anIndex);
  private:
    struct sDatIndex
    {
      u32 _gameCode;
      u32 _crc32;
      u64 _offset;
    };
    class cParsedItem
    {
      public:
        std::string _title;
        std::string _comment;
        std::vector<u32> _cheat;
        u32 _flags;
        u32 _offset;
        cParsedItem(const std::string& title,const std::string& comment,u32 flags,u32 offset=0):_title(title),_comment(comment),_flags(flags),_offset(offset) {};
        enum
        {
          EFolder=1,
          EInFolder=2,
          EOne=4,
          ESelected=8,
          EOpen=16
        };
    };
    enum
    {
      EIconColumn=0,
      ETextColumn=1,
      EIconWidth=15,
      EFolderWidth=11,
      ERowHeight=15,
      EFolderTop=3,
      ESelectTop=5
    };
  private:
    std::vector<cParsedItem> _data;
    std::vector<size_t> _indexes;
    std::string _fileName;
  public:
    std::vector<u32> getCheats();
};

#endif
