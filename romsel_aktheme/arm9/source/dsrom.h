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
#include "saveinfo.h"
#include "drawing/gdi.h"
#include "common/ndsheader.h"
#include "common/tonccpy.h"
#include "unknown_banner_bin.h"
#include <memory>

using std::unique_ptr;

typedef struct {
  u8 icon_frames[8][512];
  u16 palette_frames[8][16];	//!< Palette for each DSi icon frame.
	u16 sequence[64];
} tDSiAnimatedIcon;

typedef struct {
  u8 icon[512];
  u16 title[128];
  u16 palette[16];
  u16 crc;
} tNDSCompactedBanner;

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
  tNDSCompactedBanner _banner;
  SAVE_INFO_EX _saveInfo;
  TBool _isDSRom;
  TBool _isHomebrew;
  TBool _hasExtendedBinaries;
  TBool _isDSiWare;
  TBool _isGbaRom;
  TBool _isBannerAnimated;
  TBool _isArgv;
  int _requiresDonorRom;
  std::string _fileName;
  s32 _extIcon;
  u8 _romVersion;
  unique_ptr<tDSiAnimatedIcon> _dsiIcon;

private:
  void load(void);
  bool loadGbaRomInfo(const std::string &filename);
  bool loadArgv(const std::string &filename);
  bool loadDSRomInfo(const std::string &filename, bool loadBanner);

public:
  DSRomInfo() : _isDSRom(EFalse), _isHomebrew(EFalse), _hasExtendedBinaries(EFalse), _isDSiWare(EFalse), 
  _isGbaRom(EFalse), 
  _isBannerAnimated(EFalse),
  _isArgv(EFalse),
  _requiresDonorRom(0),
  _extIcon(-1), 
  _romVersion(0)
  {
    toncset(&_banner, 0, sizeof(_banner));
    toncset(&_saveInfo, 0, sizeof(_saveInfo));
    // toncset(&_dsiIcon, 0, sizeof(_dsiIcon));
    // memset(&_dsiPalette, 0, sizeof(_dsiPalette));
    // memset(&_dsiIcon, 0, sizeof(_dsiIcon));

  }

  DSRomInfo(const DSRomInfo&);
  virtual ~DSRomInfo() { }

public:
  void drawDSRomIcon(u8 x, u8 y, GRAPHICS_ENGINE engine);
  void drawDSiAnimatedRomIcon(u8 x, u8 y, u8 frame, u8 palette, bool flipH, bool flipV, GRAPHICS_ENGINE engine);

  void drawDSRomIconMem(void *mem);
  void drawDSiAnimatedRomIconMem(void *mem, u8 frame, u8 palette, bool flipH, bool flipV);

  const tNDSCompactedBanner &banner(void);
  const tDSiAnimatedIcon &animatedIcon(void);
  SAVE_INFO_EX &saveInfo(void);
  u8 version(void);
  void setExtIcon(const std::string &aValue);
  inline bool isExtIcon(void) { return _extIcon >= 0; };
  bool isDSRom(void);
  bool isHomebrew(void);
  bool isGbaRom(void);
  bool hasExtendedBinaries(void);
  bool isDSiWare(void);
  bool isBannerAnimated(void);
  bool isArgv(void);
  int requiresDonorRom(void);

  DSRomInfo &operator=(const DSRomInfo &src);
  void MayBeArgv(const std::string &filename)
  {
    _isDSRom = EMayBe;
    _isHomebrew = EMayBe;
    _isDSiWare = EMayBe;
    _isArgv = EMayBe;
    _fileName = filename;
  }
  void MayBeDSRom(const std::string &filename)
  {
    _isDSRom = EMayBe;
    _isHomebrew = EMayBe;
    _isDSiWare = EMayBe;
    _hasExtendedBinaries = EMayBe;
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
