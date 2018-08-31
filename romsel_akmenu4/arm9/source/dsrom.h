/*
    dsrom.h
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

#ifndef _DSROM_H_
#define _DSROM_H_

#include <nds.h>
#include <string.h>
#include "savemngr.h"
#include "drawing/gdi.h"
#include "bootstrap_support/ndsheader.h"


typedef struct {
  u8 icon_frames[8][512];
  u16 palette_frames[8][16];	//!< Palette for each DSi icon frame.
	u16 sequence[64];
} tDSiAnimatedIcon;

class DSRomInfo
{
private:
  enum TBool
  {
    ETrue,
    EFalse,
    EMayBe
  };

private:
  tNDSBanner _banner;
  SAVE_INFO_EX _saveInfo;
  TBool _isDSRom;
  TBool _isHomebrew;
  TBool _isDSiWare;
  TBool _isGbaRom;
  TBool _isBannerAnimated;
  std::string _fileName;
  s32 _extIcon;
  u8 _romVersion;
  tDSiAnimatedIcon _dsiIcon;
  

private:
  void load(void);
  bool loadGbaRomInfo(const std::string &filename);
  bool loadDSRomInfo(const std::string &filename, bool loadBanner);

public:
  DSRomInfo() : _isDSRom(EFalse), _isHomebrew(EFalse), _isGbaRom(EFalse), _extIcon(-1), _romVersion(0), 
  _isDSiWare(EFalse), 
  _isBannerAnimated(EFalse)
  {
    //memcpy(&_banner,unknown_banner_bin,unknown_banner_bin_size);
    memset(&_banner, 0, sizeof(_banner));
    memset(&_saveInfo, 0, sizeof(_saveInfo));
    memset(&_dsiIcon, 0, sizeof(_dsiIcon));
    // memset(&_dsiPalette, 0, sizeof(_dsiPalette));
    // memset(&_dsiIcon, 0, sizeof(_dsiIcon));

  }

public:
  void drawDSRomIcon(u8 x, u8 y, GRAPHICS_ENGINE engine);
  void drawDSiAnimatedRomIcon(u8 x, u8 y, u8 frame, u8 palette, GRAPHICS_ENGINE engine);

  void drawDSRomIconMem(void *mem);
  tNDSBanner &banner(void);
  tDSiAnimatedIcon &animatedIcon(void);
  SAVE_INFO_EX &saveInfo(void);
  u8 version(void);
  void setExtIcon(const std::string &aValue);
  inline bool isExtIcon(void) { return _extIcon >= 0; };
  bool isDSRom(void);
  bool isHomebrew(void);
  bool isGbaRom(void);
  bool isDSiWare(void);
  bool isBannerAnimated(void);
  DSRomInfo &operator=(const DSRomInfo &src);
  void MayBeDSRom(const std::string &filename)
  {
    _isDSRom = EMayBe;
    _isHomebrew = EMayBe;
    _isDSiWare = EMayBe;
    _fileName = filename;
  };
  void MayBeGbaRom(const std::string &filename)
  {
    _isGbaRom = EMayBe;
    _fileName = filename;
  };
  void setBanner(const std::string &anExtIcon, const u8 *aBanner);
};

#endif //_DSROM_H_
