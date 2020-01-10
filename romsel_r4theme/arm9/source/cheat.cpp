/*
    cheat.cpp
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

#include "cheat.h"
#include "flashcard.h"
#include "common/dbgtool.h"
#include "common/stringtool.h"
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>

#include "ndsheaderbanner.h"
#include "iconTitle.h"
#include "graphics/fontHandler.h"
#include "errorScreen.h"
#include "common/tonccpy.h"

extern int dialogboxHeight;
extern bool useBootstrap;

CheatCodelist::~CheatCodelist(void) {}

inline u32 gamecode(const char *aGameCode)
{
    u32 gameCode;
    memcpy(&gameCode, aGameCode, sizeof(gameCode));
    return gameCode;
}

#define CRCPOLY 0xedb88320
static u32 crc32(const u8* p,size_t len)
{
  u32 crc=-1;
  while(len--)
  {
    crc^=*p++;
    for(int ii=0;ii<8;++ii) crc=(crc>>1)^((crc&1)?CRCPOLY:0);
  }
  return crc;
}

bool CheatCodelist::parse(const std::string& aFileName)
{
  bool res=false;
  u32 romcrc32,gamecode;
  if(romData(aFileName,gamecode,romcrc32))
  {
    FILE* dat=fopen((sdFound() || !secondaryDevice) ? "sd:/_nds/TWiLightMenu/extras/usrcheat.dat" : "fat:/_nds/TWiLightMenu/extras/usrcheat.dat","rb");
    if(dat)
    {
      res=parseInternal(dat,gamecode,romcrc32);
      fclose(dat);
    }
  }
  return res;
}

bool CheatCodelist::searchCheatData(FILE* aDat,u32 gamecode,u32 crc32,long& aPos,size_t& aSize)
{
  aPos=0;
  aSize=0;
  const char* KHeader="R4 CheatCode";
  char header[12];
  fread(header,12,1,aDat);
  if(memcmp(KHeader,header,12)) return false;

  sDatIndex idx,nidx;

  fseek(aDat,0,SEEK_END);
  long fileSize=ftell(aDat);

  fseek(aDat,0x100,SEEK_SET);
  fread(&nidx,sizeof(nidx),1,aDat);

  bool done=false;

  while(!done)
  {
    tonccpy(&idx,&nidx,sizeof(idx));
    fread(&nidx,sizeof(nidx),1,aDat);
    if(gamecode==idx._gameCode&&crc32==idx._crc32)
    {
      aSize=((nidx._offset)?nidx._offset:fileSize)-idx._offset;
      aPos=idx._offset;
      done=true;
    }
    if(!nidx._offset) done=true;
  }
  return (aPos&&aSize);
}

bool CheatCodelist::parseInternal(FILE* aDat,u32 gamecode,u32 crc32)
{
  dbg_printf("%x, %x\n",gamecode,crc32);

  _data.clear();

  long dataPos; size_t dataSize;
  if(!searchCheatData(aDat,gamecode,crc32,dataPos,dataSize)) return false;
  fseek(aDat,dataPos,SEEK_SET);

  dbg_printf("record found: %d\n",dataSize);

  char* buffer=(char*)malloc(dataSize);
  if(!buffer) return false;
  fread(buffer,dataSize,1,aDat);
  char* gameTitle=buffer;

  u32* ccode=(u32*)(((u32)gameTitle+strlen(gameTitle)+4)&~3);
  u32 cheatCount=*ccode;
  cheatCount&=0x0fffffff;
  ccode+=9;

  u32 cc=0;
  while(cc<cheatCount)
  {
    u32 folderCount=1;
    char* folderName=NULL;
    char* folderNote=NULL;
    u32 flagItem=0;
    if((*ccode>>28)&1)
    {
      flagItem|=cParsedItem::EInFolder;
      if((*ccode>>24)==0x11) flagItem|=cParsedItem::EOne;
      folderCount=*ccode&0x00ffffff;
      folderName=(char*)((u32)ccode+4);
      folderNote=(char*)((u32)folderName+strlen(folderName)+1);
      _data.push_back(cParsedItem(folderName,folderNote,cParsedItem::EFolder));
      cc++;
      ccode=(u32*)(((u32)folderName+strlen(folderName)+1+strlen(folderNote)+1+3)&~3);
    }

    u32 selectValue=cParsedItem::ESelected;
    for(size_t ii=0;ii<folderCount;++ii)
    {
      char* cheatName=(char*)((u32)ccode+4);
      char* cheatNote=(char*)((u32)cheatName+strlen(cheatName)+1);
      u32* cheatData=(u32*)(((u32)cheatNote+strlen(cheatNote)+1+3)&~3);
      u32 cheatDataLen=*cheatData++;

      if(cheatDataLen)
      {
        _data.push_back(cParsedItem(cheatName,cheatNote,flagItem|((*ccode&0xff000000)?selectValue:0),dataPos+(((char*)ccode+3)-buffer)));
        if((*ccode&0xff000000)&&(flagItem&cParsedItem::EOne)) selectValue=0;
        for(size_t jj=0;jj<cheatDataLen;++jj)
        {
          _data.back()._cheat+=formatString("%08X",*(cheatData+jj));
          _data.back()._cheat+=((jj+1)%2)?" ":"\n";
        }
        if(cheatDataLen%2) _data.back()._cheat+="\n";
      }
      cc++;
      ccode=(u32*)((u32)ccode+(((*ccode&0x00ffffff)+1)*4));
    }
  }
  free(buffer);
  generateList();
  return true;
}

void CheatCodelist::generateList(void)
{
  _indexes.clear();
  // _List.removeAllRows();

  std::vector<cParsedItem>::iterator itr=_data.begin();
  while(itr!=_data.end())
  {
    std::vector<std::string> row;
    row.push_back("");
    row.push_back((*itr)._title);
    // _List.insertRow(_List.getRowCount(),row);
    _indexes.push_back(itr-_data.begin());
    u32 flags=(*itr)._flags;
    ++itr;
    if((flags&cParsedItem::EFolder)&&(flags&cParsedItem::EOpen)==0)
    {
      while(((*itr)._flags&cParsedItem::EInFolder)&&itr!=_data.end()) ++itr;
    }
  }
}

bool CheatCodelist::romData(const std::string& aFileName,u32& aGameCode,u32& aCrc32)
{
  bool res=false;
  FILE* rom=fopen(aFileName.c_str(),"rb");
  if(rom)
  {
    u8 header[512];
    if(1==fread(header,sizeof(header),1,rom))
    {
      aCrc32=crc32(header,sizeof(header));
      aGameCode=gamecode((const char*)(header+12));
      res=true;
    }
    fclose(rom);
  }
  return res;
}

std::string CheatCodelist::getCheats()
{
  std::string cheats;
  for(uint i=0;i< _data.size();i++)
  {
    if(_data[i]._flags&cParsedItem::ESelected)
    {
      cheats += _data[i]._cheat.substr(0, _data[i]._cheat.size());
    }
  }
  std::replace( cheats.begin(), cheats.end(), '\n', ' ');
  return cheats;
}

void CheatCodelist::drawCheatList(std::vector<CheatCodelist::cParsedItem>& list, uint curPos, uint screenPos) {
  for(uint i=0;(int)i<(dialogboxHeight+2) && i<list.size();i++) {
    if(list[screenPos+i]._flags&cParsedItem::EFolder) {
      if(screenPos+i == curPos) {
        printSmall(false, 27, 100+(i*8), ">");
        printSmall(false, 35, 100+(i*8), list[screenPos+i]._title.c_str());
      } else {
        printSmall(false, 22, 100+(i*8), ">");
        printSmall(false, 30, 100+(i*8), list[screenPos+i]._title.c_str());
      }
    } else {
      if(list[screenPos+i]._flags&cParsedItem::ESelected) {
        printSmall(false, 16, 99+(i*8), "x");
      }
      if(screenPos+i == curPos) {
        printSmall(false, 25, 100+(i*8), "-");
        printSmall(false, 32, 100+(i*8), list[screenPos+i]._title.c_str());
      } else {
        printSmall(false, 21, 100+(i*8), "-");
        printSmall(false, 28, 100+(i*8), list[screenPos+i]._title.c_str());
      }
    }
  }
}

void CheatCodelist::selectCheats(std::string filename)
{
  int pressed = 0;
  int held = 0;

  clearText();

  int oldDialogboxHeight = dialogboxHeight;
  dialogboxHeight = 6;

  titleUpdate(isDirectory, filename.c_str());
  printLargeCentered(false, 84, "Cheats");
  printSmallCentered(false, 100, "Loading...");

  parse(filename);

  bool cheatsFound = true;
  // If no cheats are found
  if(_data.size() == 0) {
    cheatsFound = false;
    clearText();
    titleUpdate(isDirectory, filename.c_str());
    printLargeCentered(false, 84, "Cheats");
    printSmallCentered(false, 100, "No cheats found");
    printSmallCentered(false, 160, "B: Back");

    while(1) {
      scanKeys();
      pressed = keysDownRepeat();
      checkSdEject();
      swiWaitForVBlank();
      if(pressed & KEY_B) {
        break;
      }
    }
  }

  // Get cheat folders
  std::vector<CheatCodelist::cParsedItem> cheatFolders;
  for(uint i=0;i<_data.size();i++) {
    if(_data[i]._flags&cParsedItem::EFolder) {
      cheatFolders.push_back(_data[i]);
    }
  }

  // Make list of all cheats not in folders and folders
  std::vector<CheatCodelist::cParsedItem> currentList;
  for(uint i=0;i<_data.size();i++) {
    if(!(_data[i]._flags&cParsedItem::EInFolder)) {
      currentList.push_back(_data[i]);
    }
  }

  int mainListCurPos = -1, mainListScreenPos = -1,
      cheatWnd_cursorPosition = 0, cheatWnd_screenPosition = 0;

  while(cheatsFound) {
    clearText();
    titleUpdate(isDirectory, filename.c_str());
    printLargeCentered(false, 84, "Cheats");

    // Print bottom text
    if(currentList[cheatWnd_cursorPosition]._comment != "") {
      if(currentList[cheatWnd_cursorPosition]._flags&cParsedItem::EFolder) {
        printSmallCentered(false, 167, "A: Open Y: Info X: Save B: Cancl");
      } else if(currentList[cheatWnd_cursorPosition]._flags&cParsedItem::ESelected) {
        printSmallCentered(false, 167, "A: Deslct Y: Info X: Save B: Cancl");
      } else {
        printSmallCentered(false, 167, "A: Slct Y: Info X: Save B: Cancl");
      }
    } else {
      if(currentList[cheatWnd_cursorPosition]._flags&cParsedItem::EFolder) {
        printSmallCentered(false, 167, "A: Open X: Save B: Cancel");
      } else if(currentList[cheatWnd_cursorPosition]._flags&cParsedItem::ESelected) {
        printSmallCentered(false, 167, "A: Deselect X: Save B: Cancel");
      } else {
        printSmallCentered(false, 167, "A: Select X: Save B: Cancel");
      }
    }

    // Scroll screen if needed
    if(cheatWnd_cursorPosition < cheatWnd_screenPosition) {
      cheatWnd_screenPosition = cheatWnd_cursorPosition;
    }
    if(cheatWnd_cursorPosition > cheatWnd_screenPosition + (dialogboxHeight+2) - 1) {
      cheatWnd_screenPosition = cheatWnd_cursorPosition - (dialogboxHeight+2) + 1;
    }

    drawCheatList(currentList, cheatWnd_cursorPosition, cheatWnd_screenPosition);

  do {
    scanKeys();
    pressed = keysDown();
    held = keysDownRepeat();
    checkSdEject();
    swiWaitForVBlank();
  } while(!pressed && !held);
    if(held & KEY_UP) {
      if(cheatWnd_cursorPosition>0) {
        cheatWnd_cursorPosition--;
      }
    } else if(held & KEY_DOWN) {
      if(cheatWnd_cursorPosition<((int)currentList.size()-1)) {
        cheatWnd_cursorPosition++;
      }
    } else if(held & KEY_LEFT) {
      cheatWnd_cursorPosition -= (cheatWnd_cursorPosition > (dialogboxHeight+2) ? (dialogboxHeight+2) : cheatWnd_cursorPosition);
    } else if(held & KEY_RIGHT) {
      cheatWnd_cursorPosition += (cheatWnd_cursorPosition < (int)(currentList.size()-(dialogboxHeight+2)) ? (dialogboxHeight+2) : currentList.size()-cheatWnd_cursorPosition-1);
    } else if(pressed & KEY_A) {
      if(currentList[cheatWnd_cursorPosition]._flags&cParsedItem::EFolder) {
        for(uint i=0;i<_data.size();i++) {
          if(_data[i]._title == currentList[cheatWnd_cursorPosition]._title) {
            currentList.clear();
            for(uint j=i+1;!(_data[j]._flags&cParsedItem::EFolder) && j<_data.size();j++) {
              currentList.push_back(_data[j]);
            }
            mainListCurPos = cheatWnd_cursorPosition;
            mainListScreenPos = cheatWnd_screenPosition;
            cheatWnd_cursorPosition = 0;
            break;
          }
        }
      } else {
        for(uint i=0;i<_data.size();i++) {
          if(_data[i]._title == currentList[cheatWnd_cursorPosition]._title) {
            if(_data[i]._flags&cParsedItem::ESelected) {
              _data[i]._flags&=~cParsedItem::ESelected;
              currentList[cheatWnd_cursorPosition]._flags&=~cParsedItem::ESelected;
            } else {
              _data[i]._flags^=cParsedItem::ESelected;
              currentList[cheatWnd_cursorPosition]._flags^=cParsedItem::ESelected;
            }
          }
        }
      }
    }
    if(pressed & KEY_B) {
      if(mainListCurPos != -1) {
        currentList.clear();
        for(uint i=0;i<_data.size();i++) {
          if(!(_data[i]._flags&cParsedItem::EInFolder)) {
            currentList.push_back(_data[i]);
          }
        }
        cheatWnd_cursorPosition = mainListCurPos;
        cheatWnd_screenPosition = mainListScreenPos;
        mainListCurPos = -1;
      } else {
        break;
      }
    }
    if(pressed & KEY_X) {
      clearText();
      titleUpdate(isDirectory, filename.c_str());
      printLargeCentered(false, 84, "Cheats");
      printSmallCentered(false, 100, "Saving...");
      onGenerate();
      break;
    }
    if(pressed & KEY_Y) {
      if(currentList[cheatWnd_cursorPosition]._comment != "") {
        clearText();
        titleUpdate(isDirectory, filename.c_str());
        printLargeCentered(false, 84, "Cheats");

        std::vector<std::string> _topText;
        std::string _topTextStr(currentList[cheatWnd_cursorPosition]._comment);
        std::vector<std::string> words;
        std::size_t pos;

        // Process comment to stay within the box
        while((pos = _topTextStr.find(' ')) != std::string::npos) {
          words.push_back(_topTextStr.substr(0, pos));
          _topTextStr = _topTextStr.substr(pos + 1);
        }
        if(_topTextStr.size())
          words.push_back(_topTextStr);
        std::string temp;
        _topText.clear();
        for(auto word : words) {
          int width = calcLargeFontWidth((temp + " " + word).c_str());
          if(width > 220) {
            _topText.push_back(temp);
            temp = word;
          } else {
            temp += " " + word;
          }
        }
        if(temp.size())
          _topText.push_back(temp);
        
        // Print comment
        for (int i = 0; i < (int)_topText.size(); i++) {
          printSmallCentered(false, 100 + (i*8), _topText[i].c_str());
        }

        // Print 'Back' text
        printSmallCentered(false, 167, "B: Back");
        while(1) {
          scanKeys();
          pressed = keysDown();
          checkSdEject();
          swiWaitForVBlank();
          if(pressed & KEY_B) {
            break;
          }
        }
      }
    }
  }
  dialogboxHeight = oldDialogboxHeight;
}

static void updateDB(u8 value,u32 offset,FILE* db)
{
  u8 oldvalue;
  if(!db) return;
  if(!offset) return;
  if(fseek(db,offset,SEEK_SET)) return;
  if(fread(&oldvalue,sizeof(oldvalue),1,db)!=1) return;
  if(oldvalue!=value)
  {
    if(fseek(db,offset,SEEK_SET)) return;
    fwrite(&value,sizeof(value),1,db);
  }
}

void CheatCodelist::onGenerate(void)
{
  FILE* db=fopen((sdFound() || !secondaryDevice) ? "sd:/_nds/TWiLightMenu/extras/usrcheat.dat" : "fat:/_nds/TWiLightMenu/extras/usrcheat.dat","r+b");
  if(db)
  {
    std::vector<cParsedItem>::iterator itr=_data.begin();
    while(itr!=_data.end())
    {
      updateDB(((*itr)._flags&cParsedItem::ESelected)?1:0,(*itr)._offset,db);
      ++itr;
    }
    fclose(db);
  }
}

void writeCheatsToFile(std::string data, const char* path) {
  std::fstream fs;
  fs.open(path, std::ios::binary | std::fstream::out);
  std::stringstream str;
  u32 value;
  while(1) {
    str.clear();
    str << data.substr(0, data.find(" "));
    str >> std::hex >> value;
    fs.write(reinterpret_cast<char*>(&value),sizeof(value));
    data = data.substr(data.find(" ")+1);
    if((int)data.find(" ") == -1) break;
  }
  fs.write("\0\0\0\xCF", 4);
  fs.close();
}
