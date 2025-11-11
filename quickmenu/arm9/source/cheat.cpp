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

#include <nds/arm9/dldi.h>
#include "cheat.h"
#include "common/tonccpy.h"
#include "common/twlmenusettings.h"
#include "common/systemdetails.h"
#include "common/stringtool.h"
#include <algorithm>

#include "ndsheaderbanner.h"
#include "perGameSettings.h"
#include "myDSiMode.h"

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
  while (len--)
  {
    crc^=*p++;
    for (int ii=0;ii<8;++ii) crc=(crc>>1)^((crc&1)?CRCPOLY:0);
  }
  return crc;
}

bool CheatCodelist::parse(const std::string& aFileName)
{
  bool res=false;
  u32 romcrc32,gamecode;
  if (romData(aFileName,gamecode,romcrc32))
  {
    const char* usrcheatPath = sys().isRunFromSD() ? "sd:/_nds/TWiLightMenu/extras/usrcheat.dat" : "fat:/_nds/TWiLightMenu/extras/usrcheat.dat";
    loadPerGameSettings(aFileName.substr(aFileName.find_last_of('/') + 1));
	if (ms().secondaryDevice && !(perGameSettings_useBootstrap == -1 ? ms().useBootstrap : perGameSettings_useBootstrap) && ms().kernelUseable) {
		if ((memcmp(io_dldi_data->friendlyName, "R4iDSN", 6) == 0)
	   || (memcmp(io_dldi_data->friendlyName, "R4iTT", 5) == 0)
     || (memcmp(io_dldi_data->friendlyName, "Acekard AK2", 0xB) == 0)
     || (memcmp(io_dldi_data->friendlyName, "Ace3DS+", 7) == 0)) {
			usrcheatPath = "fat:/_wfwd/cheats/usrcheat.dat";
		}
	}
    FILE* dat=fopen(usrcheatPath,"rb");
    if (dat)
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
  if (strncmp(KHeader,header,12)) return false;

  sDatIndex idx,nidx;

  fseek(aDat,0,SEEK_END);
  long fileSize=ftell(aDat);

  fseek(aDat,0x100,SEEK_SET);
  fread(&nidx,sizeof(nidx),1,aDat);

  bool done=false;

  while (!done)
  {
    tonccpy(&idx,&nidx,sizeof(idx));
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

bool CheatCodelist::parseInternal(FILE* aDat,u32 gamecode,u32 crc32)
{
  // dbg_printf("%x, %x\n",gamecode,crc32);

  _data.clear();

  long dataPos; size_t dataSize;
  if (!searchCheatData(aDat,gamecode,crc32,dataPos,dataSize)) return false;
  fseek(aDat,dataPos,SEEK_SET);

  // dbg_printf("record found: %d\n",dataSize);

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

void CheatCodelist::generateList(void)
{
  _indexes.clear();
  // _List.removeAllRows();

  std::vector<cParsedItem>::iterator itr=_data.begin();
  while (itr!=_data.end())
  {
    std::vector<std::string> row;
    row.push_back("");
    row.push_back((*itr)._title);
    // _List.insertRow(_List.getRowCount(),row);
    _indexes.push_back(itr-_data.begin());
    u32 flags=(*itr)._flags;
    ++itr;
    if ((flags&cParsedItem::EFolder)&&(flags&cParsedItem::EOpen)==0)
    {
      while (((*itr)._flags&cParsedItem::EInFolder)&&itr!=_data.end()) ++itr;
    }
  }
}

bool CheatCodelist::romData(const std::string& aFileName,u32& aGameCode,u32& aCrc32)
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

std::vector<u32> CheatCodelist::getCheats()
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

void CheatCodelist::writeCheatsToFile(const char *path) {
  FILE *file = fopen(path, "wb");
  if (file) {
    std::vector<u32> cheats(getCheats());
    fwrite(cheats.data(),4,cheats.size(),file);
    fwrite("\0\0\0\xCF",4,1,file);
    fclose(file);
  }
}
