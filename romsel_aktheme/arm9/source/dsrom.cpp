/*
    dsrom.cpp
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

#include <nds/arm9/dldi.h>

#include "dsrom.h"
#include "tool/dbgtool.h"
#include "fileicons.h"
#include "icons.h"
#include "language.h"
#include "common/dsimenusettings.h"
#include "common/module_params.h"
#include "common/ndsheader.h"
#include "common/dsargv.h"

#include "nds_banner_bin.h"
#include "unknown_nds_banner_bin.h"
#include "gbarom_banner_bin.h"
#include "icon_bg_bin.h"
#include "gamecode.h"

#include "common/tonccpy.h"
#include <memory>

static u32 arm9Sig[3][4];

DSRomInfo &DSRomInfo::operator=(const DSRomInfo &src)
{
	tonccpy(&_banner, &src._banner, sizeof(_banner));
	tonccpy(&_saveInfo, &src._saveInfo, sizeof(_saveInfo));

	_dsiIcon = std::make_unique<tDSiAnimatedIcon>();
	tonccpy(_dsiIcon.get(), src._dsiIcon.get(), sizeof(tDSiAnimatedIcon));


	_isDSRom = src._isDSRom;
	_isHomebrew = src._isHomebrew;
	_isGbaRom = src._isGbaRom;
	_fileName = src._fileName;
	_romVersion = src._romVersion;
	_extIcon = src._extIcon;
	_hasExtendedBinaries = src._hasExtendedBinaries;
	_isDSiWare = src._isDSiWare;
	_isBannerAnimated = src._isBannerAnimated;
	_isArgv = src._isArgv;
	_requiresDonorRom = src._requiresDonorRom;
	
	return *this;
}

DSRomInfo::DSRomInfo(const DSRomInfo &src)
: _isDSRom(EFalse), _isHomebrew(EFalse), _hasExtendedBinaries(EFalse), _isDSiWare(EFalse), 
  _isGbaRom(EFalse), 
  _isBannerAnimated(EFalse),
  _isArgv(EFalse),
  _requiresDonorRom(0),
  _extIcon(-1), 
  _romVersion(0)
{
	tonccpy(&_banner, &src._banner, sizeof(_banner));
	tonccpy(&_saveInfo, &src._saveInfo, sizeof(_saveInfo));

	_dsiIcon = std::make_unique<tDSiAnimatedIcon>();
	tonccpy(_dsiIcon.get(), src._dsiIcon.get(), sizeof(tDSiAnimatedIcon));
	
	_isDSRom = src._isDSRom;
	_isHomebrew = src._isHomebrew;
	_isGbaRom = src._isGbaRom;
	_fileName = src._fileName;
	_romVersion = src._romVersion;
	_extIcon = src._extIcon;
	_hasExtendedBinaries = src._hasExtendedBinaries;
	_isDSiWare = src._isDSiWare;
	_isBannerAnimated = src._isBannerAnimated;
	_isArgv = src._isArgv;
	_requiresDonorRom = src._requiresDonorRom;
}

bool DSRomInfo::loadDSRomInfo(const std::string &filename, bool loadBanner)
{
	_isDSRom = EFalse;
	_isHomebrew = EFalse;
	FILE *f = fopen(filename.c_str(), "rb");
	if (NULL == f) {
		return false;
	}

	sNDSHeaderExt header;
	if (fread(&header, sizeof(header), 1, f) != 1) {
		if (fread(&header, 0x160, 1, f) != 1) {
			dbg_printf("read rom header fail\n");
			_banner.crc = ((tNDSBanner*)unknown_nds_banner_bin)->crc;
			tonccpy(_banner.icon,((tNDSBanner*)unknown_nds_banner_bin)->icon, sizeof(_banner.icon));
			tonccpy(_banner.palette,((tNDSBanner*)unknown_nds_banner_bin)->palette, sizeof(_banner.palette));
			tonccpy(_banner.title, ((tNDSBanner*)unknown_nds_banner_bin)->titles[setTitleLanguage], sizeof(_banner.title));
			fclose(f);
			return false;
		}
	}

	///////// ROM Header /////////
	u16 crc = header.headerCRC16;
	if (crc != header.headerCRC16) {
		dbg_printf("%s rom header crc error\n", filename.c_str());
		
		_banner.crc = ((tNDSBanner*)unknown_nds_banner_bin)->crc;
		tonccpy(_banner.icon,((tNDSBanner*)unknown_nds_banner_bin)->icon, sizeof(_banner.icon));
		tonccpy(_banner.palette,((tNDSBanner*)unknown_nds_banner_bin)->palette, sizeof(_banner.palette));
		tonccpy(_banner.title, ((tNDSBanner*)unknown_nds_banner_bin)->titles[setTitleLanguage], sizeof(_banner.title));		fclose(f);
		return true;
	} else {
		_isDSRom = ETrue;
		_isDSiWare = EFalse;
		_hasExtendedBinaries = ETrue;
		_requiresDonorRom = 0;
		bool usingFlashcard = (!isDSiMode() && ms().secondaryDevice);
		bool hasCycloDSi = (memcmp(io_dldi_data->friendlyName, "CycloDS iEvolution", 18) == 0);
		switch (header.arm7binarySize) {
			case 0x22B40:
			case 0x22BCC:
				if (usingFlashcard || hasCycloDSi) _requiresDonorRom = 51;
				break;
			case 0x23708:
			case 0x2378C:
			case 0x237F0:
				if (usingFlashcard || hasCycloDSi) _requiresDonorRom = 5;
				break;
			case 0x23CAC:
				if (usingFlashcard || hasCycloDSi) _requiresDonorRom = 20;
				break;
			case 0x24DA8:
			case 0x24F50:
				_requiresDonorRom = 2;
				break;
			case 0x2434C:
			case 0x2484C:
			case 0x249DC:
			case 0x25D04:
			case 0x25D94:
			case 0x25FFC:
				if (usingFlashcard || hasCycloDSi) _requiresDonorRom = 3;
				break;
			case 0x27618:
			case 0x2762C:
			case 0x29CEC:
				_requiresDonorRom = 5;
				break;
			default:
				break;
		}

		fseek(f, (header.arm9romOffset <= 0x200 ? header.arm9romOffset : header.arm9romOffset+0x800), SEEK_SET);
		fread(arm9Sig[0], sizeof(u32), 4, f);
		if (header.arm9romOffset <= 0x200 && arm9Sig[0] == 0 && arm9Sig[1] == 0 && arm9Sig[2] == 0 && arm9Sig[3] == 0)
		{
			fseek(f, header.arm9romOffset+0x800, SEEK_SET);
			fread(arm9Sig[0], sizeof(u32), 4, f);
		}
		if (arm9Sig[0][0] == 0xE3A00301
		 && arm9Sig[0][1] == 0xE5800208
		 && arm9Sig[0][2] == 0xE3A00013
		 && arm9Sig[0][3] == 0xE129F000)
		{
			_isDSiWare = ETrue;
			_isHomebrew = ETrue;
			if ((u32)header.arm7destination >= 0x037F0000 && (u32)header.arm7executeAddress >= 0x037F0000)
			{
				if ((header.arm9binarySize == 0xC9F68 && header.arm7binarySize == 0x12814)	// Colors! v1.1
				|| (header.arm9binarySize == 0x1B0864 && header.arm7binarySize == 0xDB50)	// Mario Paint Composer DS v2 (Bullet Bill)
				|| (header.arm9binarySize == 0xD45C0 && header.arm7binarySize == 0x2B7C)	// ikuReader v0.058
				|| (header.arm9binarySize == 0x7A124 && header.arm7binarySize == 0xEED0)	// PPSEDS r11
				|| (header.arm9binarySize == 0x54620 && header.arm7binarySize == 0x1538)	// XRoar 0.24fp3
				|| (header.arm9binarySize == 0x2C9A8 && header.arm7binarySize == 0xFB98)	// NitroGrafx v0.7
				|| (header.arm9binarySize == 0x22AE4 && header.arm7binarySize == 0xA764))	// It's 1975 and this man is about to show you the future
				{
					_isDSiWare = EFalse; // Have nds-bootstrap load it
				}
			}
		}
		else if ((memcmp(header.gameTitle, "NDS.TinyFB", 10) == 0)
			   || (memcmp(header.gameTitle, "UNLAUNCH.DSI", 12) == 0))
		{
			_isDSiWare = ETrue;
			_isHomebrew = ETrue;
		}
		else if ((memcmp(header.gameTitle, "NMP4BOOT", 8) == 0)
		 || ((u32)header.arm7destination >= 0x037F0000 && (u32)header.arm7executeAddress >= 0x037F0000))
		{
			_isHomebrew = ETrue;
		}
		else if ((header.gameCode[0] == 0x48 && header.makercode[0] != 0 && header.makercode[1] != 0)
		 || (header.gameCode[0] == 0x4B && header.makercode[0] != 0 && header.makercode[1] != 0)
		 || (header.gameCode[0] == 0x5A && header.makercode[0] != 0 && header.makercode[1] != 0)
		 || (header.gameCode[0] == 0x42 && header.gameCode[1] == 0x38 && header.gameCode[2] == 0x38))
		{ if (header.unitCode != 0)
		  {
			dbg_printf("DSIWAREFOUND Is DSiWare!\n");
			_isDSiWare = ETrue;

			_saveInfo.dsiPrvSavSize = header.prvSavSize;
			_saveInfo.dsiPubSavSize = header.pubSavSize;
			_saveInfo.dsiTid[0] = header.dsi_tid;
			_saveInfo.dsiTid[1] = header.dsi_tid2;
		  }
		}
		if ((memcmp(header.gameCode, "KPP", 3) == 0
		  || memcmp(header.gameCode, "KPF", 3) == 0)
		&& (!isDSiMode() || ms().dsiWareBooter || ms().consoleModel > 0)) {
			_isDSiWare = EFalse;
		}
	}

	if (_isHomebrew == EFalse && header.unitCode != 0)
	{
		fseek(f, 0x8000, SEEK_SET);
		fread(arm9Sig[0], sizeof(u32), 4, f);
		fseek(f, header.arm9iromOffset, SEEK_SET);
		fread(arm9Sig[1], sizeof(u32), 4, f);
		fseek(f, header.arm7iromOffset, SEEK_SET);
		fread(arm9Sig[2], sizeof(u32), 4, f);
		for (int i = 1; i < 3; i++) {
			if (arm9Sig[i][0] == arm9Sig[0][0]
			 && arm9Sig[i][1] == arm9Sig[0][1]
			 && arm9Sig[i][2] == arm9Sig[0][2]
			 && arm9Sig[i][3] == arm9Sig[0][3]) {
				_hasExtendedBinaries = EFalse;
			}
			if (arm9Sig[i][0] == 0
			 && arm9Sig[i][1] == 0
			 && arm9Sig[i][2] == 0
			 && arm9Sig[i][3] == 0) {
				_hasExtendedBinaries = EFalse;
			}
			if (arm9Sig[i][0] == 0xFFFFFFFF
			 && arm9Sig[i][1] == 0xFFFFFFFF
			 && arm9Sig[i][2] == 0xFFFFFFFF
			 && arm9Sig[i][3] == 0xFFFFFFFF) {
				_hasExtendedBinaries = EFalse;
			}
		}
		if (header.arm9iromOffset == 0 || header.arm7iromOffset == 0)
		{
			_hasExtendedBinaries = EFalse;
		}
	}

	///////// saveInfo /////////
	tonccpy(_saveInfo.gameTitle, header.gameTitle, 12);
	tonccpy(_saveInfo.gameCode, header.gameCode, 4);
	_saveInfo.arm9destination = header.arm9destination;
	///// SDK Version /////

	if (_isHomebrew == EFalse)
	{
		_saveInfo.gameSdkVersion = getModuleParams(&header, f)->sdk_version;
		dbg_printf("SDK: %X\n", _saveInfo.gameSdkVersion);
	}
	else
	{
		_saveInfo.gameSdkVersion = 0;
	}

	_saveInfo.gameCRC = header.headerCRC16;
	_romVersion = header.romversion;

	///////// banner /////////
	if (header.bannerOffset != 0)
	{
		fseek(f, header.bannerOffset, SEEK_SET);
		sNDSBannerExt banner;
		int bannerSize;
		int currentLang = 0;

		// Rely on short-circuiting to get a clean if statement.

		// If we get a full size banner, then continue as so, setting bannerSize appropriately.
		// Otherwise, if we read an invalid amount of bytes, read a DS size header.
		if ((fseek(f, header.bannerOffset, SEEK_SET) == 0 && (bannerSize = fread(&banner, 1, sizeof(banner), f)) == sizeof(banner)) || (fseek(f, header.bannerOffset, SEEK_SET) == 0 && (bannerSize = fread(&banner, 1, NDS_BANNER_SIZE_ORIGINAL, f)) == NDS_BANNER_SIZE_ORIGINAL))
		{
			u16 dsiCrc16 = swiCRC16(0xFFFF, banner.dsi_icon, 0x1180);
			// Check for DSi Banner.
			if (bannerSize == NDS_BANNER_SIZE_DSi && banner.version == NDS_BANNER_VER_DSi && banner.crc[3] == dsiCrc16)
			{
				dbg_printf("DSi Banner Found!");
				_isBannerAnimated = ETrue;
				_dsiIcon = std::make_unique<tDSiAnimatedIcon>();

				tonccpy(_dsiIcon->icon_frames, banner.dsi_icon, sizeof(banner.dsi_icon));
				tonccpy(_dsiIcon->palette_frames, banner.dsi_palette, sizeof(banner.dsi_palette));
				tonccpy(_dsiIcon->sequence, banner.dsi_seq, sizeof(banner.dsi_seq));
			}
				_banner.crc = ((tNDSBanner*)&banner)->crc;
				tonccpy(_banner.icon, &banner.icon, sizeof(_banner.icon));
				tonccpy(_banner.palette,&banner.palette, sizeof(_banner.palette));
				if (banner.version == NDS_BANNER_VER_ZH || banner.version == NDS_BANNER_VER_ZH_KO || banner.version == NDS_BANNER_VER_DSi)
				{
					currentLang = ms().getGuiLanguage();
				}
				else
				{
					currentLang = setTitleLanguage;
				}
				while (&banner.titles[currentLang][0] == 0)
				{
					if (currentLang == 0) break;
					currentLang--;
				}
				tonccpy(_banner.title, &banner.titles[currentLang], sizeof(_banner.title));
		}
		else
		{
			_banner.crc = ((tNDSBanner*)nds_banner_bin)->crc;
			tonccpy(_banner.icon,((tNDSBanner*)nds_banner_bin)->icon, sizeof(_banner.icon));
			tonccpy(_banner.palette,((tNDSBanner*)nds_banner_bin)->palette, sizeof(_banner.palette));
			if (((tNDSBanner*)nds_banner_bin)->version == NDS_BANNER_VER_ZH
			 || ((tNDSBanner*)nds_banner_bin)->version == NDS_BANNER_VER_ZH_KO
			 || ((tNDSBanner*)nds_banner_bin)->version == NDS_BANNER_VER_DSi)
			{
				currentLang = ms().getGuiLanguage();
			}
			else
			{
				currentLang = setTitleLanguage;
			}
			while (((tNDSBanner*)nds_banner_bin)->titles[currentLang][0] == 0)
			{
				if (currentLang == 0) break;
				currentLang--;
			}
			tonccpy(_banner.title, ((tNDSBanner*)nds_banner_bin)->titles[currentLang], sizeof(_banner.title));
		}
	}
	else
	{
		_banner.crc = ((tNDSBanner*)nds_banner_bin)->crc;
		tonccpy(_banner.icon,((tNDSBanner*)nds_banner_bin)->icon, sizeof(_banner.icon));
		tonccpy(_banner.palette,((tNDSBanner*)nds_banner_bin)->palette, sizeof(_banner.palette));
		tonccpy(_banner.title, ((tNDSBanner*)nds_banner_bin)->titles[ms().getGuiLanguage()], sizeof(_banner.title));
	}

	fclose(f);

	return true;
}

void DSRomInfo::drawDSRomIcon(u8 x, u8 y, GRAPHICS_ENGINE engine)
{
	if (_extIcon >= 0) {
		fileIcons().Draw(_extIcon, x, y, engine);
		return;
	}
	bool skiptransparent = false;
	switch (_saveInfo.getIcon()) {
		case SAVE_INFO_EX_ICON_TRANSPARENT:
			break;
		case SAVE_INFO_EX_ICON_AS_IS:
			skiptransparent = true;
			break;
		case SAVE_INFO_EX_ICON_FIRMWARE:
			gdi().maskBlt(icon_bg_bin, x, y, 32, 32, engine);
			break;
	}
	for (int tile = 0; tile < 16; ++tile) {
		for (int pixel = 0; pixel < 32; ++pixel) {
			u8 a_byte = _banner.icon[(tile << 5) + pixel];

			//int px = (tile & 3) * 8 + (2 * pixel & 7);
			//int py = (tile / 4) * 8 + (2 * pixel / 8);
			int px = ((tile & 3) << 3) + ((pixel << 1) & 7);
			int py = ((tile >> 2) << 3) + (pixel >> 2);

			u8 idx1 = (a_byte & 0xf0) >> 4;
			if (skiptransparent || 0 != idx1)
			{
				gdi().setPenColor(_banner.palette[idx1], engine);
				gdi().drawPixel(px + 1 + x, py + y, engine);
			}

			u8 idx2 = (a_byte & 0x0f);
			if (skiptransparent || 0 != idx2)
			{
				gdi().setPenColor(_banner.palette[idx2], engine);
				gdi().drawPixel(px + x, py + y, engine);
			}
		}
	}
}

void DSRomInfo::drawDSiAnimatedRomIcon(u8 x, u8 y, u8 frame, u8 palette, bool flipH, bool flipV, GRAPHICS_ENGINE engine)
{
	if (_extIcon >= 0) {
		fileIcons().Draw(_extIcon, x, y, engine);
		return;
	}
	if (_isBannerAnimated != ETrue) {
		return drawDSRomIcon(x, y, engine);
	}

	bool skiptransparent = false;
	switch (_saveInfo.getIcon()) {
		case SAVE_INFO_EX_ICON_TRANSPARENT:
			break;
		case SAVE_INFO_EX_ICON_AS_IS:
			skiptransparent = true;
			break;
		case SAVE_INFO_EX_ICON_FIRMWARE:
			gdi().maskBlt(icon_bg_bin, x, y, 32, 32, engine);
			break;
	}
	for (int tile = 0; tile < 16; ++tile) {
		for (int pixel = 0; pixel < 32; ++pixel) {
			u8 a_byte = _dsiIcon->icon_frames[frame][(tile << 5) + pixel];

			//int px = (tile & 3) * 8 + (2 * pixel & 7);
			//int py = (tile / 4) * 8 + (2 * pixel / 8);
			int px = ((tile & 3) << 3) + ((pixel << 1) & 7);
			int py = ((tile >> 2) << 3) + (pixel >> 2);
			int drawX = flipH ? 39 - (px + x) : (px + x);
			int drawY = flipV ? 31 + (-py + y) : (py + y);
			// 32 by 32.
			u8 idx1 = (a_byte & 0xf0) >> 4;
			if (skiptransparent || 0 != idx1)
			{
				
				gdi().setPenColor(_dsiIcon->palette_frames[palette][idx1], engine);

				gdi().drawPixel(drawX, drawY, engine);
			}

			u8 idx2 = (a_byte & 0x0f);
			if (skiptransparent || 0 != idx2)
			{
				gdi().setPenColor(_dsiIcon->palette_frames[palette][idx2], engine);
				gdi().drawPixel(drawX + (flipH ? 1 : - 1), drawY, engine);
			}
		}
	}
}

void DSRomInfo::drawDSRomIconMem(void *mem)
{
	if (_extIcon >= 0) {
		fileIcons().DrawMem(_extIcon, mem);
		return;
	}
	u16 *pmem = (u16 *)mem;
	bool skiptransparent = false;
	switch (_saveInfo.getIcon()) {
		case SAVE_INFO_EX_ICON_TRANSPARENT:
			break;
		case SAVE_INFO_EX_ICON_AS_IS:
			skiptransparent = true;
			break;
		case SAVE_INFO_EX_ICON_FIRMWARE:
			Icons::maskBlt((const u16 *)icon_bg_bin, pmem);
			break;
	}
	for (int tile = 0; tile < 16; ++tile) {
		for (int pixel = 0; pixel < 32; ++pixel) {
			u8 a_byte = _banner.icon[(tile << 5) + pixel];

			//int px = (tile & 3) * 8 + (2 * pixel & 7);
			//int py = (tile / 4) * 8 + (2 * pixel / 8);
			int px = ((tile & 3) << 3) + ((pixel << 1) & 7);
			int py = ((tile >> 2) << 3) + (pixel >> 2);

			u8 idx1 = (a_byte & 0xf0) >> 4;
			if (skiptransparent || idx1 != 0) {
				pmem[py * 32 + px + 1] = _banner.palette[idx1] | BIT(15);
				//gdi().setPenColor(_banner.palette[idx1]);
				//gdi().drawPixel(px + 1 + x, py + y, engine);
			}

			u8 idx2 = (a_byte & 0x0f);
			if (skiptransparent || 0 != idx2) {
				pmem[py * 32 + px]  = _banner.palette[idx2] | BIT(15);
				//gdi().setPenColor(_banner.palette[idx2]);
				//gdi().drawPixel(px+x, py+y, engine);
			}
		}
	}
}

void DSRomInfo::drawDSiAnimatedRomIconMem(void *mem, u8 frame, u8 palette, bool flipH, bool flipV)
{
	if (_extIcon >= 0) {
		fileIcons().DrawMem(_extIcon, mem);
		return;
	}
	load();

	if (_isBannerAnimated != ETrue) {
		return drawDSRomIconMem(mem);
	}

	u16 *pmem = (u16 *)mem;
	bool skiptransparent = false;
	switch (_saveInfo.getIcon()) {
		case SAVE_INFO_EX_ICON_TRANSPARENT:
			break;
		case SAVE_INFO_EX_ICON_AS_IS:
			skiptransparent = true;
			break;
		case SAVE_INFO_EX_ICON_FIRMWARE:
			Icons::maskBlt((const u16 *)icon_bg_bin, pmem);
			break;
	}
	for (int tile = 0; tile < 16; ++tile) {
		for (int pixel = 0; pixel < 32; ++pixel) {
			u8 a_byte = _dsiIcon->icon_frames[frame][(tile << 5) + pixel];

			//int px = (tile & 3) * 8 + (2 * pixel & 7);
			//int py = (tile / 4) * 8 + (2 * pixel / 8);
			int px = ((tile & 3) << 3) + ((pixel << 1) & 7);
			int py = ((tile >> 2) << 3) + (pixel >> 2);

			int drawX =  flipH ? 30 - (px) : px + 1;
			int drawY =  flipV ? 31 + (-py) : py;

			u8 idx1 = (a_byte & 0xf0) >> 4;
			if (skiptransparent || 0 != idx1) {
				pmem[drawY * 32 + drawX] = _dsiIcon->palette_frames[palette][idx1] | BIT(15);
				//gdi().setPenColor(_banner.palette[idx1]);
				//gdi().drawPixel(px + 1 + x, py + y, engine);
			}

			u8 idx2 = (a_byte & 0x0f);
			if (skiptransparent || 0 != idx2) {
				pmem[drawY * 32 + drawX + (flipH ? 1 : - 1)] = _dsiIcon->palette_frames[palette][idx2] | BIT(15);
				//gdi().setPenColor(_banner.palette[idx2]);
				//gdi().drawPixel(px + x, py + y, engine);
			}
		}
	}
}

bool DSRomInfo::loadGbaRomInfo(const std::string &filename)
{
	_isGbaRom = EFalse;
	FILE *gbaFile = fopen(filename.c_str(), "rb");
	if (gbaFile) {
		sGBAHeader header;
		fread(&header, 1, sizeof(header), gbaFile);
		fclose(gbaFile);
		if (header.is96h == 0x96) {
			_isGbaRom = ETrue;
			tonccpy(_saveInfo.gameCode, header.gamecode, 4);
			_romVersion = header.version;

			_banner.crc = ((tNDSBanner*)gbarom_banner_bin)->crc;
			tonccpy(_banner.icon,((tNDSBanner*)gbarom_banner_bin)->icon, sizeof(_banner.icon));
			tonccpy(_banner.palette,((tNDSBanner*)gbarom_banner_bin)->palette, sizeof(_banner.palette));
			tonccpy(_banner.title, ((tNDSBanner*)gbarom_banner_bin)->titles[setTitleLanguage], sizeof(_banner.title));
			return true;
		}
	}
	return false;
}

bool DSRomInfo::loadArgv(const std::string &filename)
{
	_isArgv = EFalse;
	_isDSRom = EMayBe;
	_isDSiWare = EMayBe;
	ArgvFile argv(filename);
	if (argv.launchPath().empty()) 
	{
		dbg_printf("empty launch path!\n");
		return false;
	}
	_isArgv = ETrue;
	return loadDSRomInfo(argv.launchPath(), true);
}

void DSRomInfo::load(void)
{
	if (_isArgv == EMayBe)
		loadArgv(_fileName);

	else if (_isDSRom == EMayBe)
		loadDSRomInfo(_fileName, true);

	if (_isGbaRom == EMayBe)
		loadGbaRomInfo(_fileName);
}

const tNDSCompactedBanner &DSRomInfo::banner(void)
{
	load();
	return (tNDSCompactedBanner &)_banner;
}

const tDSiAnimatedIcon &DSRomInfo::animatedIcon(void)
{
	load();
	return *_dsiIcon;
}

SAVE_INFO_EX &DSRomInfo::saveInfo(void)
{
	load();
	return _saveInfo;
}

u8 DSRomInfo::version(void)
{
	load();
	return _romVersion;
}

bool DSRomInfo::isDSRom(void)
{
	load();
	return (_isDSRom == ETrue);
}

bool DSRomInfo::hasExtendedBinaries(void)
{
	load();
	return (_hasExtendedBinaries == ETrue);
}

bool DSRomInfo::isDSiWare(void)
{
	load();
	return (_isDSiWare == ETrue);
}

bool DSRomInfo::isArgv(void)
{
	load();
	return (_isArgv == ETrue);
}

int DSRomInfo::requiresDonorRom(void)
{
	load();
	return _requiresDonorRom;
}

bool DSRomInfo::isBannerAnimated(void)
{
	load();
	return (_isBannerAnimated == ETrue);
}

bool DSRomInfo::isHomebrew(void)
{
	load();
	return (_isHomebrew == ETrue);
}

bool DSRomInfo::isGbaRom(void)
{
	load();
	return (_isGbaRom == ETrue);
}

void DSRomInfo::setExtIcon(const std::string &aValue)
{
	_extIcon = fileIcons().Icon(aValue);
};

void DSRomInfo::setBanner(const std::string &anExtIcon, const u8 *aBanner)
{
	setExtIcon(anExtIcon);
	_banner.crc = ((tNDSBanner*)aBanner)->crc;
	tonccpy(_banner.icon,((tNDSBanner*)aBanner)->icon, sizeof(_banner.icon));
	tonccpy(_banner.palette,((tNDSBanner*)aBanner)->palette, sizeof(_banner.palette));
	tonccpy(_banner.title, ((tNDSBanner*)aBanner)->titles[setTitleLanguage], sizeof(_banner.title));
}
