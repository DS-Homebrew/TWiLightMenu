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

#include "dsrom.h"
#include "tool/dbgtool.h"
#include "fileicons.h"
#include "icons.h"
#include "common/module_params.h"
#include "common/ndsheader.h"
#include "common/dsargv.h"
#include "common/fixedbanners.h"

#include "nds_banner_bin.h"
#include "unknown_nds_banner_bin.h"
#include "gbarom_banner_bin.h"
#include "icon_bg_bin.h"
#include "gamecode.h"

DSRomInfo &DSRomInfo::operator=(const DSRomInfo &src)
{
    memcpy(&_banner, &src._banner, sizeof(_banner));
    memcpy(&_saveInfo, &src._saveInfo, sizeof(_saveInfo));
    memcpy(&_dsiIcon, &src._dsiIcon, sizeof(_dsiIcon));

    _isDSRom = src._isDSRom;
    _isHomebrew = src._isHomebrew;
    _isGbaRom = src._isGbaRom;
    _fileName = src._fileName;
    _romVersion = src._romVersion;
    _extIcon = src._extIcon;
    _isDSiWare = src._isDSiWare;
    _isBannerAnimated = src._isBannerAnimated;
    _isArgv = src._isArgv;
    
    return *this;
}

bool DSRomInfo::loadDSRomInfo(const std::string &filename, bool loadBanner)
{
    _isDSRom = EFalse;
    _isHomebrew = EFalse;
    FILE *f = fopen(filename.c_str(), "rb");
    if (NULL == f)
    {
        return false;
    }

    sNDSHeaderExt header;
    if (1 != fread(&header, sizeof(header), 1, f))
    {
        dbg_printf("read rom header fail\n");
        memcpy(&_banner, unknown_nds_banner_bin, sizeof(_banner));
        fclose(f);
        return false;
    }

    ///////// ROM Header /////////
    u16 crc = header.headerCRC16;
    if (crc != header.headerCRC16)
    {
        dbg_printf("%s rom header crc error\n", filename.c_str());
        memcpy(&_banner, unknown_nds_banner_bin, sizeof(_banner));
        fclose(f);
        return true;
    }
    else
    {
        _isDSRom = ETrue;
        _isDSiWare = EFalse;

        if ((header.unitCode == 0x03 && (header.arm7binarySize > 0x20000 || _isArgv == ETrue))
        || (header.unitCode == 0x03 && header.gameCode[0] == 0x48)
		|| (header.unitCode == 0x03 && header.arm7binarySize == 0x151BC))
		{
            dbg_printf("DSIWAREFOUND Is DSiWare!\n");
            _isDSiWare = ETrue;

            _saveInfo.dsiPrvSavSize = header.prvSavSize;
            _saveInfo.dsiPubSavSize = header.pubSavSize;
            _saveInfo.dsiTid[0] = header.dsi_tid;
            _saveInfo.dsiTid[1] = header.dsi_tid2;
        }
        else if ((header.unitCode >= 0x02 && header.arm9romOffset == 0x4000 && header.arm7binarySize < 0x20000) || (header.arm9romOffset == 0x200 && (u32)header.arm7destination == 0x02380000))
        {
            // Homebrew with DSiWare Extended header
            _isDSiWare = ETrue;
            _isHomebrew = ETrue;
        }
        else if (((u32)header.arm7destination >= 0x037F0000 && (u32)header.arm7executeAddress >= 0x037F0000) || (0x23232323 == gamecode(header.gameCode))) // ####
        {
            _isDSiWare = EFalse;
            _isHomebrew = ETrue;
        }
    }

    ///////// saveInfo /////////
    memcpy(_saveInfo.gameTitle, header.gameTitle, 12);
    memcpy(_saveInfo.gameCode, header.gameCode, 4);
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

        // Rely on short-circuiting to get a clean if statement.

        // If we get a full size banner, then continue as so, setting bannerSize appropriately.
        // Otherwise, if we read an invalid amount of bytes, read a DS size header.
        if ((fseek(f, header.bannerOffset, SEEK_SET) == 0 && (bannerSize = fread(&banner, 1, sizeof(banner), f)) == sizeof(banner)) || (fseek(f, header.bannerOffset, SEEK_SET) == 0 && (bannerSize = fread(&banner, 1, NDS_BANNER_SIZE_ORIGINAL, f)) == NDS_BANNER_SIZE_ORIGINAL))
        {
            // Check for DSi Banner.
            if (bannerSize == NDS_BANNER_SIZE_DSi && banner.version == NDS_BANNER_VER_DSi)
            {
                dbg_printf("DSi Banner Found!");
                _isBannerAnimated = ETrue;
                fixBanner(&banner);
                memcpy(_dsiIcon.icon_frames, banner.dsi_icon, sizeof(banner.dsi_icon));
                memcpy(_dsiIcon.palette_frames, banner.dsi_palette, sizeof(banner.dsi_palette));
                memcpy(_dsiIcon.sequence, banner.dsi_seq, sizeof(banner.dsi_seq));

                
            }
            memcpy(&_banner, &banner, sizeof(_banner));
        }
        else
        {
            memcpy(&_banner, nds_banner_bin, sizeof(_banner));
        }
    }
    else
    {
        //dbg_printf( "%s has no banner\n", filename );
        memcpy(&_banner, nds_banner_bin, sizeof(_banner));
    }

    fclose(f);

    return true;
}

void DSRomInfo::drawDSRomIcon(u8 x, u8 y, GRAPHICS_ENGINE engine)
{
    if (_extIcon >= 0)
    {
        fileIcons().Draw(_extIcon, x, y, engine);
        return;
    }
    bool skiptransparent = false;
    switch (_saveInfo.getIcon())
    {
    case SAVE_INFO_EX_ICON_TRANSPARENT:
        break;
    case SAVE_INFO_EX_ICON_AS_IS:
        skiptransparent = true;
        break;
    case SAVE_INFO_EX_ICON_FIRMWARE:
        gdi().maskBlt(icon_bg_bin, x, y, 32, 32, engine);
        break;
    }
    for (int tile = 0; tile < 16; ++tile)
    {
        for (int pixel = 0; pixel < 32; ++pixel)
        {
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
    if (_extIcon >= 0)
    {
        fileIcons().Draw(_extIcon, x, y, engine);
        return;
    }
    if (_isBannerAnimated != ETrue)
    {
        return drawDSRomIcon(x, y, engine);
    }

    bool skiptransparent = false;
    switch (_saveInfo.getIcon())
    {
    case SAVE_INFO_EX_ICON_TRANSPARENT:
        break;
    case SAVE_INFO_EX_ICON_AS_IS:
        skiptransparent = true;
        break;
    case SAVE_INFO_EX_ICON_FIRMWARE:
        gdi().maskBlt(icon_bg_bin, x, y, 32, 32, engine);
        break;
    }
    for (int tile = 0; tile < 16; ++tile)
    {
        for (int pixel = 0; pixel < 32; ++pixel)
        {
            u8 a_byte = _dsiIcon.icon_frames[frame][(tile << 5) + pixel];

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
                
                gdi().setPenColor(_dsiIcon.palette_frames[palette][idx1], engine);

                gdi().drawPixel(drawX, drawY, engine);
            }

            u8 idx2 = (a_byte & 0x0f);
            if (skiptransparent || 0 != idx2)
            {
                gdi().setPenColor(_dsiIcon.palette_frames[palette][idx2], engine);
                gdi().drawPixel(drawX + (flipH ? 1 : - 1), drawY, engine);
            }
        }
    }
}

void DSRomInfo::drawDSRomIconMem(void *mem)
{
    if (_extIcon >= 0)
    {
        fileIcons().DrawMem(_extIcon, mem);
        return;
    }
    u16 *pmem = (u16 *)mem;
    bool skiptransparent = false;
    switch (_saveInfo.getIcon())
    {
    case SAVE_INFO_EX_ICON_TRANSPARENT:
        break;
    case SAVE_INFO_EX_ICON_AS_IS:
        skiptransparent = true;
        break;
    case SAVE_INFO_EX_ICON_FIRMWARE:
        Icons::maskBlt((const u16 *)icon_bg_bin, pmem);
        break;
    }
    for (int tile = 0; tile < 16; ++tile)
    {
        for (int pixel = 0; pixel < 32; ++pixel)
        {
            u8 a_byte = _banner.icon[(tile << 5) + pixel];

            //int px = (tile & 3) * 8 + (2 * pixel & 7);
            //int py = (tile / 4) * 8 + (2 * pixel / 8);
            int px = ((tile & 3) << 3) + ((pixel << 1) & 7);
            int py = ((tile >> 2) << 3) + (pixel >> 2);

         
            u8 idx1 = (a_byte & 0xf0) >> 4;
            if (skiptransparent || 0 != idx1)
            {
              pmem[py * 32 + px + 1] = _banner.palette[idx1] | BIT(15);
                //gdi().setPenColor( _banner.palette[idx1] );
                //gdi().drawPixel( px+1+x, py+y, engine );
            }

            u8 idx2 = (a_byte & 0x0f);
            if (skiptransparent || 0 != idx2)
            {
                pmem[py * 32 + px]  = _banner.palette[idx2] | BIT(15);
                //gdi().setPenColor( _banner.palette[idx2] );
                //gdi().drawPixel( px+x, py+y, engine );
            }
        }
    }
}

void DSRomInfo::drawDSiAnimatedRomIconMem(void *mem, u8 frame, u8 palette, bool flipH, bool flipV)
{
    if (_extIcon >= 0)
    {
        fileIcons().DrawMem(_extIcon, mem);
        return;
    }
    load();

    if (_isBannerAnimated != ETrue)
    {
        return drawDSRomIconMem(mem);
    }

    u16 *pmem = (u16 *)mem;
    bool skiptransparent = false;
    switch (_saveInfo.getIcon())
    {
    case SAVE_INFO_EX_ICON_TRANSPARENT:
        break;
    case SAVE_INFO_EX_ICON_AS_IS:
        skiptransparent = true;
        break;
    case SAVE_INFO_EX_ICON_FIRMWARE:
        Icons::maskBlt((const u16 *)icon_bg_bin, pmem);
        break;
    }
    for (int tile = 0; tile < 16; ++tile)
    {
        for (int pixel = 0; pixel < 32; ++pixel)
        {
            u8 a_byte = _dsiIcon.icon_frames[frame][(tile << 5) + pixel];

            //int px = (tile & 3) * 8 + (2 * pixel & 7);
            //int py = (tile / 4) * 8 + (2 * pixel / 8);
            int px = ((tile & 3) << 3) + ((pixel << 1) & 7);
            int py = ((tile >> 2) << 3) + (pixel >> 2);

            int drawX =  flipH ? 30 - (px) : px + 1;
            int drawY =  flipV ? 31 + (-py) : py;

            u8 idx1 = (a_byte & 0xf0) >> 4;
            if (skiptransparent || 0 != idx1)
            {
                pmem[drawY * 32 + drawX] = _dsiIcon.palette_frames[palette][idx1] | BIT(15);
                //gdi().setPenColor( _banner.palette[idx1] );
                //gdi().drawPixel( px+1+x, py+y, engine );
            }

            u8 idx2 = (a_byte & 0x0f);
            if (skiptransparent || 0 != idx2)
            {
                 
                pmem[drawY * 32 + drawX + (flipH ? 1 : - 1)] = _dsiIcon.palette_frames[palette][idx2] | BIT(15);
                //gdi().setPenColor( _banner.palette[idx2] );
                //gdi().drawPixel( px+x, py+y, engine );
            }
        }
    }
}

bool DSRomInfo::loadGbaRomInfo(const std::string &filename)
{
    _isGbaRom = EFalse;
    FILE *gbaFile = fopen(filename.c_str(), "rb");
    if (gbaFile)
    {
        sGBAHeader header;
        fread(&header, 1, sizeof(header), gbaFile);
        fclose(gbaFile);
        if (header.is96h == 0x96)
        {
            _isGbaRom = ETrue;
            memcpy(_saveInfo.gameCode, header.gamecode, 4);
            _romVersion = header.version;
            memcpy(&_banner, gbarom_banner_bin, sizeof(tNDSBanner));
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

tNDSBanner &DSRomInfo::banner(void)
{
    load();
    return (tNDSBanner &)_banner;
}

tDSiAnimatedIcon &DSRomInfo::animatedIcon(void)
{
    load();
    return _dsiIcon;
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
    return (_isDSRom == ETrue) ? true : false;
}

bool DSRomInfo::isDSiWare(void)
{
    load();
    return (_isDSiWare == ETrue) ? true : false;
}

bool DSRomInfo::isArgv(void)
{
    load();
    return (_isArgv == ETrue) ? true : false;
}

bool DSRomInfo::isBannerAnimated(void)
{
    load();
    return (_isBannerAnimated == ETrue) ? true : false;
}

bool DSRomInfo::isHomebrew(void)
{
    load();
    return (_isHomebrew == ETrue) ? true : false;
}

bool DSRomInfo::isGbaRom(void)
{
    load();
    return (_isGbaRom == ETrue) ? true : false;
}

void DSRomInfo::setExtIcon(const std::string &aValue)
{
    _extIcon = fileIcons().Icon(aValue);
};

void DSRomInfo::setBanner(const std::string &anExtIcon, const u8 *aBanner)
{
    setExtIcon(anExtIcon);
    memcpy(&banner(), aBanner, sizeof(tNDSBanner));
}
