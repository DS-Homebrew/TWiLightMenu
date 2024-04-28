/*
    saveinfo.h
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

#ifndef _SAVEINFO_H_
#define _SAVEINFO_H_

#include <nds.h>
#include <vector>
#include <string>
#include "common/singleton.h"

typedef struct _SAVE_INFO_T
{
  u8 gameTitle[12];
  u8 gameCode[4];
  u32 dsiTid[2];
  u32 arm9destination;
  PACKED u16 gameCRC;
  u32 gameSdkVersion;
  u32 dsiPubSavSize;
  u32 dsiPrvSavSize;
  u8 saveType;
} SAVE_INFO;

enum SAVE_TYPE
{
  ST_UNKNOWN = 0,
  ST_NOSAVE,
  ST_4K,
  ST_64K,
  ST_512K,
  ST_2M,
  ST_4M,
  ST_8M,
  ST_NEW,
  ST_AUTO,
  ST_1M,
  ST_16M,
  ST_32M,
  ST_64M
};

enum DISPLAY_SAVE_TYPE
{
  D_ST_UNKNOWN = 0,
  D_ST_NOSAVE,
  D_ST_4K,
  D_ST_64K,
  D_ST_512K,
  D_ST_1M,
  D_ST_2M,
  D_ST_4M,
  D_ST_8M,
  D_ST_16M,
  D_ST_32M,
  D_ST_64M
};

//flags
#define SAVE_INFO_EX_RUMBLE 0x03
#define SAVE_INFO_EX_DOWNLOAD_PLAY 0x04
#define SAVE_INFO_EX_SOFT_RESET 0x08
#define SAVE_INFO_EX_CHEAT 0x10
#define SAVE_INFO_EX_SLOT_MASK 0x60
#define SAVE_INFO_EX_SLOT_SHIFT 5
#define SAVE_INFO_EX_DMA 0x80
#define SAVE_INFO_EX_COMPARE_SIZE 18
//flags2
#define SAVE_INFO_EX_GLOBAL_DOWNLOAD_PLAY 0x00000001
#define SAVE_INFO_EX_GLOBAL_SOFT_RESET 0x00000002
#define SAVE_INFO_EX_GLOBAL_CHEAT 0x00000004
#define SAVE_INFO_EX_GLOBAL_DMA 0x00000008
#define SAVE_INFO_EX_GLOBAL_MASK 0x0000000f
#define SAVE_INFO_EX_PROTECTION 0x00000010
#define SAVE_INFO_EX_LINKAGE 0x00000020
#define SAVE_INFO_EX_ICON_MASK 0x000000c0
#define SAVE_INFO_EX_ICON_SHIFT 6
#define SAVE_INFO_EX_ICON_TRANSPARENT 0
#define SAVE_INFO_EX_ICON_AS_IS 1
#define SAVE_INFO_EX_ICON_FIRMWARE 2
#define SAVE_INFO_EX_SD_SAVE 0x00000100
#define SAVE_INFO_EX_GLOBAL_SD_SAVE 0x00000200
#define SAVE_INFO_EX_LANGUAGE_MASK 0x00001c00
#define SAVE_INFO_EX_LANGUAGE_SHIFT 10

typedef struct SAVE_INFO_EX_T
{
  u8 gameTitle[12];
  u8 gameCode[4];
  u16 gameCRC;
  u32 arm9destination;
  u8 saveType;
  u8 flags;
  u32 flags2;
  u32 reserved[2];

  u32 dsiTid[2];
  u32 dsiPubSavSize;
  u32 dsiPrvSavSize;
  u32 gameSdkVersion;

  u8 getRumble(void) { return flags & SAVE_INFO_EX_RUMBLE; };
  u8 getDownloadPlay(void) { return getFlag(SAVE_INFO_EX_DOWNLOAD_PLAY, SAVE_INFO_EX_GLOBAL_DOWNLOAD_PLAY, false); };
  u8 getSoftReset(void) { return getFlag(SAVE_INFO_EX_SOFT_RESET, SAVE_INFO_EX_GLOBAL_SOFT_RESET, false); };
  u8 getCheat(void) { return getFlag(SAVE_INFO_EX_CHEAT, SAVE_INFO_EX_GLOBAL_CHEAT, false); };
  u8 getSlot(void) { return (flags & SAVE_INFO_EX_SLOT_MASK) >> SAVE_INFO_EX_SLOT_SHIFT; };
  u8 getDMA(void) { return getFlag(SAVE_INFO_EX_DMA, SAVE_INFO_EX_GLOBAL_DMA, false); };
  u8 getProtection(void) { return (flags2 & SAVE_INFO_EX_PROTECTION) ? 1 : 0; };
  u8 getLinkage(void) { return (flags2 & SAVE_INFO_EX_LINKAGE) ? 1 : 0; };
  u8 getIcon(void) { return (flags2 & SAVE_INFO_EX_ICON_MASK) >> SAVE_INFO_EX_ICON_SHIFT; };
  u8 getSDSave(void) { return getFlag(SAVE_INFO_EX_SD_SAVE, SAVE_INFO_EX_GLOBAL_SD_SAVE, true); };
  u8 getLanguage(void) { return (flags2 & SAVE_INFO_EX_LANGUAGE_MASK) >> SAVE_INFO_EX_LANGUAGE_SHIFT; }
  bool isDownloadPlay(void) { return getState(SAVE_INFO_EX_DOWNLOAD_PLAY, SAVE_INFO_EX_GLOBAL_DOWNLOAD_PLAY, false, false); };
  
  bool isProtection(void) { return (flags2 & SAVE_INFO_EX_PROTECTION) ? true : false; };
  bool isLinkage(void) { return (flags2 & SAVE_INFO_EX_LINKAGE) ? true : false; };
 
  u8 getFlag(u32 personal, u32 global, bool style)
  {
    return (flags2 & global) ? 2 : ((style ? (flags2 & personal) : (flags & personal)) ? 1 : 0);
  }
 
  bool getState(u32 personal, u32 global, bool globalState, bool style)
  {
    switch (getFlag(personal, global, style))
    {
    case 0:
      return false;
    case 1:
      return true;
    default:
      return globalState;
    }
  }
  
} SAVE_INFO_EX;

#define SAVE_INFO_EX_HEADER_MAGIC 0x42474b41

typedef struct SAVE_INFO_EX_HEADER_T
{
  u32 marker;
  u32 itemSize;
  u32 itemCount;
  u32 reserved;
} SAVE_INFO_EX_HEADER;

#endif //_SAVEINFO_H_
