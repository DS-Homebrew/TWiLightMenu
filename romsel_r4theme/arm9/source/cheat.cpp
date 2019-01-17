/*
    NitroHax -- Cheat tool for the Nintendo DS
    Copyright (C) 2008  Michael "Chishm" Chisholm

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <nds.h>

#include "cheat.h"
#include "tool/dbgtool.h"
#include "tool/stringtool.h"
#include <stdio.h>
#include <algorithm>
#include <iterator>

static const char HEX_CHARACTERS[] = "0123456789ABCDEFabcdef";

CheatFolder::~CheatFolder()
{
	for (std::vector<CheatBase*>::iterator curItem = contents.begin(); curItem != contents.end(); curItem++) {
		delete (*curItem);
	}
}

void CheatFolder::enableAll (bool enabled)
{
	if (allowOneOnly && enabled) {
		return;
	}
	for (std::vector<CheatBase*>::iterator curItem = contents.begin(); curItem != contents.end(); curItem++) {
		CheatCode* cheatCode = dynamic_cast<CheatCode*>(*curItem);
		if (cheatCode) {
			cheatCode->setEnabled (enabled);
		}
	}
}

void CheatFolder::enablingSubCode (void)
{
	if (allowOneOnly) {
		enableAll (false);
	}
}	

std::list<CheatWord> CheatFolder::getEnabledCodeData (void)
{
    #ifdef DEBUG
    nocashMessage("CheatFolder::getEnabledCodeData");
    #endif

	std::list<CheatWord> codeData;
	CheatCode* cheatCode;
	
	for (std::vector<CheatBase*>::iterator curItem = contents.begin(); curItem != contents.end(); curItem++) {
		std::list<CheatWord> curCodeData = (*curItem)->getEnabledCodeData();
		cheatCode = dynamic_cast<CheatCode*>(*curItem);
		if (cheatCode && cheatCode->isMaster()) {
			codeData.insert( codeData.begin(), curCodeData.begin(), curCodeData.end());  
		} else {
			codeData.insert( codeData.end(), curCodeData.begin(), curCodeData.end());  
		}
	}
	
	return codeData;
}

std::list<CheatWord> CheatCode::getEnabledCodeData (void)
{
    nocashMessage("CheatCode::getEnabledCodeData");
    
	std::list<CheatWord> codeData;
	if (enabled) {
		codeData = cheatData;
	}
	
	return codeData;
}

void CheatCode::setCodeData (const std::string& data) 
{
	const char* codeData = data.c_str();
	char codeinfo[30];
	int value;
	int codeLen = strlen (codeData);
	int codePos = 0;
	int readNum = 1;
	
	const char ALWAYS_ON[] = "always_on";
	const char ON[] = "on";
	const char MASTER[] = "master";
	
	if (sscanf (codeData, "%29s", codeinfo) > 0) {
		if (strcmp(codeinfo, ALWAYS_ON) == 0) {
			always_on = true;
			enabled = true;
			codePos += strlen (ALWAYS_ON);
		} else if (strcmp(codeinfo, ON) == 0) {
			enabled = true;
			codePos += strlen (ON);
		} else if (strcmp(codeinfo, MASTER) == 0) {
			master = true;
			codePos += strlen (MASTER);
		}
	}
	
	while ((codePos < codeLen) && (readNum > 0)) {
		// Move onto the next hexadecimal value
		codePos += strcspn (codeData + codePos, HEX_CHARACTERS);
		readNum = sscanf (codeData + codePos, "%x", &value);
		if (readNum > 0) {
			cheatData.push_back (value);
			codePos += CODE_WORD_LEN;
		} else {
			readNum = sscanf (codeData + codePos, "%29s", codeinfo);
			if (readNum > 0) {
				codePos += strlen (codeinfo);
			}
		}
	}
	
	if (master && (cheatData.size() >= 2)) {
		if ((*(cheatData.begin()) & 0xFF000000) == 0xCF000000) {
			// Master code meant for Nitro Hax
			always_on = true;
			enabled = true;
		} else if ((cheatData.size() >= 18) && (*(cheatData.begin()) == 0x00000000)) {
			// Master code meant for the Action Replay
			// Convert it for Nitro Hax
			CheatWord relocDest;
			std::list<CheatWord>::iterator i = cheatData.begin();
			std::advance (i, 13);
			relocDest = *i;
			cheatData.clear();
			cheatData.push_back (CHEAT_ENGINE_RELOCATE);
			cheatData.push_back (relocDest);
			enabled = true;
		}
	}
}

void CheatCode::toggleEnabled (void)
{
	if (!enabled && getParent()) {
		getParent()->enablingSubCode();
	}
	if (!always_on) {
		enabled = !enabled;
	}
}


void CheatGame::setGameid (const std::string& id)
{
	const char* idData = id.c_str();
	const char* crcData;
	
	headerCRC = 0;
	if (id.size() < 9) {
		return;
	}
	gameid[0] = id.at(0);
	gameid[1] = id.at(1);
	gameid[2] = id.at(2);
	gameid[3] = id.at(3);
	
	headerCRC = 0;
	crcData = strpbrk (idData+4, HEX_CHARACTERS);
	if (crcData) {
		sscanf (crcData, "%x", &headerCRC);
		// CRC is inverted in the cheat list
		headerCRC = ~headerCRC;
	}
}


CheatCodelist::~CheatCodelist(void) 
{
	for (std::vector<CheatBase*>::iterator curItem = getContents().begin(); curItem != getContents().end(); curItem++) {
		delete (*curItem);
	}
} 

#define BUFFER_SIZE 1024
std::string CheatCodelist::nextToken (FILE* fp, TOKEN_TYPE& tokenType)
{
	char tokenData[BUFFER_SIZE];
	std::string token;
	
	if (fscanf(fp, " <%1023[^>]", tokenData) > 0) {
		if (tokenData[0] == '/') {
			tokenType = TOKEN_TAG_END;
			token += &tokenData[1];
		} else {
			tokenType = TOKEN_TAG_START;
			token += tokenData;
		}
		while (fscanf(fp, "%1023[^>]", tokenData) > 0) {
			token += tokenData;
		} ;
		fscanf(fp, ">");
		if (tokenType == TOKEN_TAG_START && token.at(token.size() -1 ) == '/') {
			token.resize (token.size() -1);
			tokenType = TOKEN_TAG_SINGLE;
		}
	} else if (fscanf(fp, "%1023[^<]", tokenData) > 0) {
		tokenType = TOKEN_DATA;
		do {
			token += tokenData;
		} while (fscanf(fp, "%1023[^<]", tokenData) > 0);
		if (token.empty()) {
			return token;
		}
	} else {
		return token;
	}

	return token;
}
	
bool CheatCodelist::load (FILE* fp, const char gameid[4], uint32_t headerCRC, bool filter)
{
    #ifdef DEBUG
    nocashMessage("CheatCodelist::load");
    #endif

	enum {state_normal, state_name, state_note, state_codes, state_gameid, state_allowedon} state = state_normal;
	CheatBase* curItem = this;
	CheatBase* newItem;
	CheatCode* cheatCode;
	CheatFolder* cheatFolder;
	CheatGame* cheatGame;
	std::string token;
	TOKEN_TYPE tokenType;
	int depth = 0;
	bool done = false;
	
    #ifdef DEBUG
    nocashMessage("CheatCodelist::load : search for codelist TOKEN_TAG_START");
    #endif
    
	do	
	token = nextToken (fp, tokenType);
	while (!token.empty() && (tokenType != TOKEN_TAG_START || token != "codelist")) ;
	
	if (token != "codelist") {
        #ifdef DEBUG        
        nocashMessage("CheatCodelist::load : codelist TOKEN_TAG_START not found");
        #endif
		return false;
	}
	depth ++;
    
    #ifdef DEBUG
    nocashMessage("CheatCodelist::load : codelist TOKEN_TAG_START found");
    #endif
	
	while (!token.empty() && !done) {
		token = nextToken (fp, tokenType);
		switch (tokenType) {
			case TOKEN_DATA:
				switch (state) {
					case state_name:
                        #ifdef DEBUG
                        nocashMessage("CheatCodelist::load : state_name");
                        nocashMessage(token.c_str());
                        #endif
						curItem->name = token;
						break;
					case state_note:
                        #ifdef DEBUG
                        nocashMessage("CheatCodelist::load : state_note");
                        #endif
						curItem->note = token;
						break;
					case state_codes:
                        #ifdef DEBUG
                        nocashMessage("CheatCodelist::load : state_codes");
                        #endif
						cheatCode = dynamic_cast<CheatCode*>(curItem); 
						if (cheatCode) {
							cheatCode->setCodeData (token);
						}
						break;
					case state_gameid:
                        #ifdef DEBUG
                        nocashMessage("CheatCodelist::load : state_gameid");
                        nocashMessage(token.c_str());
                        #endif
						cheatGame = dynamic_cast<CheatGame*>(curItem); 
						if (cheatGame) {
							cheatGame->setGameid (token);
						}
						break;
					case state_allowedon:
                        #ifdef DEBUG
                        nocashMessage("CheatCodelist::load : state_allowedon");
                        nocashMessage(token.c_str());
                        #endif
						cheatFolder = dynamic_cast<CheatFolder*>(curItem); 
						if (cheatFolder) {
							cheatFolder->setAllowOneOnly (!(token == "0"));
						}
						break;
					case state_normal:
						break;
				}
				break;
			case TOKEN_TAG_START:
				depth++;
				if (token == "game") {
					cheatGame = new CheatGame (this);					
					curItem = cheatGame;
				} else if (token == "folder") {
					cheatFolder = dynamic_cast<CheatFolder*>(curItem);
					if (cheatFolder) {
						newItem = new CheatFolder (cheatFolder);
						cheatFolder->addItem (newItem);
						curItem = newItem;
					} 
				} else if (token == "cheat") {
					cheatFolder = dynamic_cast<CheatFolder*>(curItem);
					if (cheatFolder) {
						newItem = new CheatCode (cheatFolder);
						cheatFolder->addItem (newItem);
						curItem = newItem;
					} 
				} else if (token == "codelist") {
					// Should only occur at top level
					curItem = this;
					depth = 1;
				} else if (token == "name") {
					state = state_name;
				} else if (token == "note") {
					state = state_note;
				} else if (token == "codes") {
					state = state_codes;
				} else if (token == "gameid") {
					state = state_gameid;
				} else if (token == "allowedon") {
					state = state_allowedon;
				}				
				break;
			case TOKEN_TAG_END:
				if ( (token == "folder") ||
					 (token == "cheat")
				) {					
					newItem = curItem->getParent();
					if (newItem) {
						curItem = newItem;
					}
				} else if (token == "subscription") {
					done = true;
				} else  if (token == "game") {
					if(!filter || ((CheatGame*)curItem)->checkGameid(gameid, headerCRC)) {
						this->addItem (curItem);
						newItem = curItem->getParent();
						if (newItem) {
							curItem = newItem;
						}
					} else {
						newItem = curItem->getParent();
						delete (curItem);
						if (newItem) {
							curItem = newItem;
						}
					}
				}
				state = state_normal;			
				depth--;
				break;
			case TOKEN_TAG_SINGLE:
				break;
		}
	}
	
	this->name = "Cheat list";
	return true;
}

CheatGame* CheatCodelist::getGame (const char gameid[4], uint32_t headerCRC)
{
    nocashMessage("CheatCodelist::getGame");

	for (std::vector<CheatBase*>::iterator curItem = contents.begin(); curItem != contents.end(); curItem++) {
		CheatGame* game = dynamic_cast<CheatGame*>(*curItem);
		if (game && game->checkGameid(gameid, headerCRC)) {
			return game;
		}
	}
	
    nocashMessage("CheatCodelist::getGame NULL");
	return NULL;
}

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
    FILE* dat=fopen("sd:/_nds/usrcheat.dat","rb");
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
  if(strncmp(KHeader,header,12)) return false;

  sDatIndex idx,nidx;

  fseek(aDat,0,SEEK_END);
  long fileSize=ftell(aDat);

  fseek(aDat,0x100,SEEK_SET);
  fread(&nidx,sizeof(nidx),1,aDat);

  bool done=false;

  while(!done)
  {
    memcpy(&idx,&nidx,sizeof(idx));
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
