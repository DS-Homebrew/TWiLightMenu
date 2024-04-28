/*
    cheatwnd.cpp
    Portions copyright (C) 2008 Normmatt, www.normmatt.com, Smiths (www.emuholic.com)
    Portions copyright (C) 2008 bliss (bliss@hanirc.org)
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

#include "cheatwnd.h"
#include "ui/uisettings.h"
#include "ui/windowmanager.h"
#include "language.h"
#include "ui/msgbox.h"
#include "gamecode.h"
#include <sys/stat.h>
#include <algorithm>

using namespace akui;

#define CRCPOLY 0xedb88320
static u32 crc32(const u8* p,size_t len)
{
  u32 crc=-1;
  while (len--)
  {
    crc^=*p++;
    for (int ii=0;ii<8;++ii) crc=(crc>>1)^((crc&1)?CRCPOLY:0);
  }
  return crc;
}


CheatWnd::CheatWnd(s32 x,s32 y,u32 w,u32 h,Window* parent,const std::string& text):
Form(x,y,w,h,parent,text),
_buttonDeselect(0,0,46,18,this,"\x05 Deselect"),
_buttonInfo(0,0,46,18,this,"\x04 Info"),
_buttonGenerate(0,0,46,18,this,"\x03 OK"),
_buttonCancel(0,0,46,18,this,"\x02 Cancel"),
_List(0,0,w-8,h-44,this,"cheat tree", ms().ak_scrollSpeed)
{
  s16 buttonY=size().y-_buttonCancel.size().y-4;

  _buttonCancel.setStyle(Button::press);
  _buttonCancel.setText("\x02 "+LANG("setting window","cancel"));
  _buttonCancel.setTextColor(uis().buttonTextColor);
  _buttonCancel.loadAppearance(SFN_BUTTON3);
  _buttonCancel.clicked.connect(this,&CheatWnd::onCancel);
  addChildWindow(&_buttonCancel);

  _buttonGenerate.setStyle(Button::press);
  _buttonGenerate.setText( "\x03 " + LANG( "setting window", "ok" ) );
  _buttonGenerate.setTextColor(uis().buttonTextColor);
  _buttonGenerate.loadAppearance(SFN_BUTTON2);
  _buttonGenerate.clicked.connect(this,&CheatWnd::onGenerate);
  addChildWindow(&_buttonGenerate);

  _buttonInfo.setStyle(Button::press);
  _buttonInfo.setText( "\x04 " + LANG( "cheats", "info" ) );
  _buttonInfo.setTextColor(uis().buttonTextColor);
  _buttonInfo.loadAppearance(SFN_BUTTON3);
  _buttonInfo.clicked.connect(this,&CheatWnd::onInfo);
  addChildWindow(&_buttonInfo);

  _buttonDeselect.setStyle(Button::press);
  _buttonDeselect.setText( "\x05 " + LANG( "cheats", "deselect" ) );
  _buttonDeselect.setTextColor(uis().buttonTextColor);
  _buttonDeselect.loadAppearance(SFN_BUTTON4);
  _buttonDeselect.clicked.connect(this,&CheatWnd::onDeselectAll);
  addChildWindow(&_buttonDeselect);

  s16 nextButtonX=size().x;
  s16 buttonPitch=_buttonCancel.size().x+4;
  nextButtonX-=buttonPitch;
  _buttonCancel.setRelativePosition(Point(nextButtonX,buttonY));

  buttonPitch=_buttonGenerate.size().x+4;
  nextButtonX-=buttonPitch;
  _buttonGenerate.setRelativePosition(Point(nextButtonX,buttonY));

  buttonPitch=_buttonInfo.size().x+4;
  nextButtonX-=buttonPitch;
  _buttonInfo.setRelativePosition(Point(nextButtonX,buttonY));

  buttonPitch=_buttonDeselect.size().x+4;
  nextButtonX-=buttonPitch;
  _buttonDeselect.setRelativePosition(Point(nextButtonX,buttonY));

  _List.setRelativePosition(Point(4, 20));

  CIniFile ini(SFN_UI_SETTINGS);
  if (ini.GetInt("cheat menu","textColor",-1)!=-1)
  {
    _List.setColors(ini.GetInt("cheat menu","textColor",RGB15(7,7,7)),ini.GetInt("cheat menu","textColorHilight",RGB15(31,0,31)),ini.GetInt("cheat menu","selectionBarColor1",RGB15(16,20,24)),ini.GetInt("cheat menu","selectionBarColor2",RGB15(20,25,0)));
  }
  else
  {
    _List.setColors(ini.GetInt("main list","textColor",RGB15(7,7,7)),ini.GetInt("main list","textColorHilight",RGB15(31,0,31)),ini.GetInt("main list","selectionBarColor1",RGB15(16,20,24)),ini.GetInt("main list","selectionBarColor2",RGB15(20,25,0)));
  }

  _List.insertColumn(0,"icon",EIconWidth);
  _List.insertColumn(1,"showName",_List.size().x+2-EIconWidth);
  _List.arangeColumnsSize();
  _List.setRowHeight(ERowHeight);
  _List.selectedRowClicked.connect(this,&CheatWnd::onItemClicked);
  _List.ownerDraw.connect(this,&CheatWnd::onDraw);
  addChildWindow(&_List);

  loadAppearance("");
  arrangeChildren();
}

CheatWnd::~CheatWnd()
{}

void CheatWnd::draw()
{
  _renderDesc.draw(windowRectangle(),_engine);
  Form::draw();
}

bool CheatWnd::process(const akui::Message& msg)
{
  bool ret=false;
  ret=Form::process(msg);
  if (!ret)
  {
    if (msg.id()>Message::keyMessageStart&&msg.id()<Message::keyMessageEnd)
    {
      ret=processKeyMessage((KeyMessage&)msg);
    }
  }
  return ret;
}

bool CheatWnd::processKeyMessage(const KeyMessage& msg)
{
  bool ret=false;
  if (msg.id() == Message::keyDown)
  {
    switch(msg.keyCode())
    {
      case KeyMessage::UI_KEY_DOWN:
        _List.selectNext();
        ret=true;
        break;
      case KeyMessage::UI_KEY_UP:
        _List.selectPrev();
        ret=true;
        break;
      case KeyMessage::UI_KEY_LEFT:
        {
          size_t ii = _List.selectedRowId();
          while (ii-- > 0)
          {
            if (_data[_indexes[ii]]._flags&cParsedItem::EFolder) break;
          }
          _List.selectRow(ii);
        }
        ret=true;
        break;
      case KeyMessage::UI_KEY_RIGHT:
        {
          size_t ii=_List.selectedRowId(),top=_List.getRowCount();
          while (++ii < top)
          {
            if (_data[_indexes[ii]]._flags&cParsedItem::EFolder) break;
          }
          _List.selectRow(ii);
        }
        ret=true;
        break;
      case KeyMessage::UI_KEY_A:
        onSelect();
        ret=true;
        break;
      case KeyMessage::UI_KEY_B:
        onCancel();
        ret=true;
        break;
      case KeyMessage::UI_KEY_X:
        onGenerate();
        ret=true;
        break;
      case KeyMessage::UI_KEY_Y:
        onInfo();
        ret=true;
        break;
      case KeyMessage::UI_KEY_L:
        onDeselectAll();
        ret=true;
        break;
      default:
        break;
    }
  }
  return ret;
}

Window& CheatWnd::loadAppearance(const std::string& aFileName )
{
  _renderDesc.loadData(SFN_FORM_TITLE_L,SFN_FORM_TITLE_R,SFN_FORM_TITLE_M);
  _renderDesc.setTitleText(_text);
  return *this;
}

void CheatWnd::onItemClicked(u32 index)
{
  (void)index;
  onSelect();
}

void CheatWnd::onSelect(void)
{
  size_t index=_indexes[_List.selectedRowId()];
  if (_data[index]._flags&cParsedItem::EFolder)
  {
    _data[index]._flags^=cParsedItem::EOpen;
    u32 first=_List.firstVisibleRowId(),row=_List.selectedRowId();
    generateList();
    _List.setFirstVisibleIdAndSelectRow(first,row);
  }
  else
  {
    bool deselect=(_data[index]._flags&(cParsedItem::EOne|cParsedItem::ESelected))==(cParsedItem::EOne|cParsedItem::ESelected);
    if (_data[index]._flags&cParsedItem::EOne) deselectFolder(index);
    if (!deselect) _data[index]._flags^=cParsedItem::ESelected;
  }
}

void CheatWnd::onDeselectAll(void)
{
  std::vector<cParsedItem>::iterator itr=_data.begin();
  while (itr!=_data.end())
  {
    (*itr)._flags&=~cParsedItem::ESelected;
    ++itr;
  }
}

void CheatWnd::onInfo(void)
{
  size_t index=_indexes[_List.selectedRowId()];
  std::string body(_data[index]._title);
  body+="\n\n";
  body+=_data[index]._comment;
  messageBox(this,LANG("cheats","title"),body,MB_OK);
}

void CheatWnd::onCancel(void)
{
  Form::onCancel();
}

static void updateDB(u8 value,u32 offset,FILE* db)
{
  u8 oldvalue;
  if (!db) return;
  if (!offset) return;
  if (fseek(db,offset,SEEK_SET)) return;
  if (fread(&oldvalue,sizeof(oldvalue),1,db)!=1) return;
  if (oldvalue!=value)
  {
    if (fseek(db,offset,SEEK_SET)) return;
    fwrite(&value,sizeof(value),1,db);
  }
}

void CheatWnd::onGenerate(void)
{
  FILE* db=fopen(SFN_CHEATS,"r+b");
  if (db)
  {
    std::vector<cParsedItem>::iterator itr=_data.begin();
    while (itr!=_data.end())
    {
      updateDB(((*itr)._flags&cParsedItem::ESelected)?1:0,(*itr)._offset,db);
      ++itr;
    }
    fclose(db);
  }
  Form::onOK();
}

void CheatWnd::drawMark(const ListView::OwnerDraw& od,u16 width)
{
  u16 color=gdi().getPenColor(od._engine);
  u16 size=od._size.y-ESelectTop*2;
  gdi().fillRect(color,color,od._position.x+((width-size)>>1)-1,od._position.y+ESelectTop,size,size,od._engine);
}

void CheatWnd::onDraw(const ListView::OwnerDraw& od)
{
  size_t index=_indexes[od._row];
  u32 flags=_data[index]._flags;
  if (od._col==EIconColumn)
  {
    if (flags&cParsedItem::EFolder)
    {
      u16 size=od._size.y-EFolderTop*2;
      s16 x1=od._position.x+((od._size.x-size)>>1)-1,x2=x1+(size>>1),y1=od._position.y+EFolderTop,y2=y1+(size>>1);
      gdi().frameRect(x1,y1,size,size,od._engine);
      gdi().drawLine(x1,y2,x1+size,y2,od._engine);
      if (!(flags&cParsedItem::EOpen)) gdi().drawLine(x2,y1,x2,y1+size,od._engine);
    }
    else if (!(flags&cParsedItem::EInFolder))
    {
      if (flags&cParsedItem::ESelected) drawMark(od,od._size.x);
    }
  }
  else if (od._col==ETextColumn)
  {
    if (flags&cParsedItem::EInFolder)
    {
      if (flags&cParsedItem::ESelected) drawMark(od,EFolderWidth);
    }
    s16 x=od._position.x; u16 w=od._size.x;
    if (flags&cParsedItem::EInFolder)
    {
      x+=EFolderWidth;
      w-=EFolderWidth;
    }
    gdi().textOutRect(x,od._textY,w,od._textHeight,od._text,od._engine);
  }
}

bool CheatWnd::parse(const std::string& aFileName)
{
  bool res=false;
  _fileName=aFileName;
  u32 romcrc32,gamecode;
  if (romData(_fileName,gamecode,romcrc32))
  {
    FILE* dat=fopen(SFN_CHEATS,"rb");
    if (dat)
    {
      res=parseInternal(dat,gamecode,romcrc32);
      fclose(dat);
    }
  }
  return res;
}

bool CheatWnd::romData(const std::string& aFileName,u32& aGameCode,u32& aCrc32)
{
  bool res=false;
  FILE* rom=fopen(aFileName.c_str(),"rb");
  if (rom)
  {
    u8 header[512];
    if (1==fread(header,sizeof(header),1,rom))
    {
      aCrc32=crc32(header,sizeof(header));
      aGameCode=gamecode((const char*)(header+12));
      res=true;
    }
    fclose(rom);
  }
  return res;
}

bool CheatWnd::searchCheatData(FILE* aDat,u32 gamecode,u32 crc32,long& aPos,size_t& aSize)
{
  aPos=0;
  aSize=0;
  const char* KHeader="R4 CheatCode";
  char header[12];
  fread(header,12,1,aDat);
  if (strncmp(KHeader,header,12)) return false;

  sDatIndex idx,nidx;

  fseek(aDat,0,SEEK_END);
  long fileSize=ftell(aDat);

  fseek(aDat,0x100,SEEK_SET);
  fread(&nidx,sizeof(nidx),1,aDat);

  bool done=false;

  while (!done)
  {
    memcpy(&idx,&nidx,sizeof(idx));
    fread(&nidx,sizeof(nidx),1,aDat);
    if (gamecode==idx._gameCode&&crc32==idx._crc32)
    {
      aSize=((nidx._offset)?nidx._offset:fileSize)-idx._offset;
      aPos=idx._offset;
      done=true;
    }
    if (!nidx._offset) done=true;
  }
  return (aPos&&aSize);
}

bool CheatWnd::parseInternal(FILE* aDat,u32 gamecode,u32 crc32)
{
  dbg_printf("%x, %x\n",gamecode,crc32);

  _data.clear();

  long dataPos; size_t dataSize;
  if (!searchCheatData(aDat,gamecode,crc32,dataPos,dataSize)) return false;
  fseek(aDat,dataPos,SEEK_SET);

  dbg_printf("record found: %d\n",dataSize);

  char* buffer=(char*)malloc(dataSize);
  if (!buffer) return false;
  fread(buffer,dataSize,1,aDat);
  char* gameTitle=buffer;

  u32* ccode=(u32*)(((u32)gameTitle+strlen(gameTitle)+4)&~3);
  u32 cheatCount=*ccode;
  cheatCount&=0x0fffffff;
  ccode+=9;

  u32 cc=0;
  while (cc<cheatCount)
  {
    u32 folderCount=1;
    char* folderName=NULL;
    char* folderNote=NULL;
    u32 flagItem=0;
    if ((*ccode>>28)&1)
    {
      flagItem|=cParsedItem::EInFolder;
      if ((*ccode>>24)==0x11) flagItem|=cParsedItem::EOne;
      folderCount=*ccode&0x00ffffff;
      folderName=(char*)((u32)ccode+4);
      folderNote=(char*)((u32)folderName+strlen(folderName)+1);
      _data.push_back(cParsedItem(folderName,folderNote,cParsedItem::EFolder));
      cc++;
      ccode=(u32*)(((u32)folderName+strlen(folderName)+1+strlen(folderNote)+1+3)&~3);
    }

    u32 selectValue=cParsedItem::ESelected;
    for (size_t ii=0;ii<folderCount;++ii)
    {
      char* cheatName=(char*)((u32)ccode+4);
      char* cheatNote=(char*)((u32)cheatName+strlen(cheatName)+1);
      u32* cheatData=(u32*)(((u32)cheatNote+strlen(cheatNote)+1+3)&~3);
      u32 cheatDataLen=*cheatData++;

      if (cheatDataLen)
      {
        _data.push_back(cParsedItem(cheatName,cheatNote,flagItem|((*ccode&0xff000000)?selectValue:0),dataPos+(((char*)ccode+3)-buffer)));
        if ((*ccode&0xff000000)&&(flagItem&cParsedItem::EOne)) selectValue=0;
        _data.back()._cheat.resize(cheatDataLen);
        tonccpy(_data.back()._cheat.data(),cheatData,cheatDataLen*4);
      }
      cc++;
      ccode=(u32*)((u32)ccode+(((*ccode&0x00ffffff)+1)*4));
    }
  }
  free(buffer);
  generateList();
  return true;
}

void CheatWnd::generateList(void)
{
  _indexes.clear();
  _List.removeAllRows();

  std::vector<cParsedItem>::iterator itr=_data.begin();
  while (itr!=_data.end())
  {
    std::vector<std::string> row;
    row.push_back("");
    row.push_back((*itr)._title);
    _List.appendRow(std::move(row));
    _indexes.push_back(itr-_data.begin());
    u32 flags=(*itr)._flags;
    ++itr;
    if ((flags&cParsedItem::EFolder)&&(flags&cParsedItem::EOpen)==0)
    {
      while (((*itr)._flags&cParsedItem::EInFolder)&&itr!=_data.end()) ++itr;
    }
  }
}

void CheatWnd::deselectFolder(size_t anIndex)
{
  std::vector<cParsedItem>::iterator itr=_data.begin()+anIndex;
  while (--itr>=_data.begin())
  {
    if ((*itr)._flags&cParsedItem::EFolder)
    {
      ++itr;
      break;
    }
  }
  while (((*itr)._flags&cParsedItem::EInFolder)&&itr!=_data.end())
  {
    (*itr)._flags&=~cParsedItem::ESelected;
    ++itr;
  }
}

std::vector<u32> CheatWnd::getCheats()
{
  std::vector<u32> cheats;
  for (uint i=0;i<_data.size();i++)
  {
    if (_data[i]._flags&cParsedItem::ESelected)
    {
      cheats.insert(cheats.end(),_data[i]._cheat.begin(),_data[i]._cheat.end());
    }
  }
  return cheats;
}

void CheatWnd::writeCheatsToFile(const char* path) {
  FILE *file = fopen(path, "wb");
  if (file) {
    std::vector<u32> cheats(getCheats());
    fwrite(cheats.data(),4,cheats.size(),file);
    fwrite("\0\0\0\xCF",4,1,file);
    fclose(file);
  }
}
