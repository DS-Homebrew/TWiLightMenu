/*-----------------------------------------------------------------
 Copyright (C) 2005 - 2013
		Michael "Chishm" Chisholm
		Dave "WinterMute" Murphy
		Claudio "sverx"
		Michael "mtheall" Theall

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

------------------------------------------------------------------*/

#include <nds.h>
#include <nds/arm9/dldi.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/stat.h>
#include <gl2d.h>
#include "common/bootstrapsettings.h"
#include "common/systemdetails.h"
#include "common/tonccpy.h"
#include "common/twlmenusettings.h"
#include "fileBrowse.h"
#include "graphics/fontHandler.h"
#include "common/lodepng.h"
#include "common/logging.h"
#include "language.h"
#include "ndsheaderbanner.h"
#include "myDSiMode.h"

#include "iconTitle.h"

#define ICON_POS_X	112
#define ICON_POS_Y	96

#define TITLE_CACHE_SIZE 0x80

// Graphic files
#include "icon_unk.h"
#include "icon_folder.h"
#include "icon_plg.h"
#include "icon_gbamode.h"
#include "icon_gba.h"
#include "icon_gb.h"
#include "icon_nes.h"
#include "icon_sg.h"
#include "icon_sms.h"
#include "icon_gg.h"
#include "icon_md.h"
#include "icon_snes.h"
#include "icon_a26.h"
#include "icon_col.h"
#include "icon_m5.h"
#include "icon_int.h"
#include "icon_pce.h"
#include "icon_ws.h"
#include "icon_ngp.h"
#include "icon_cpc.h"
#include "icon_vid.h"
#include "icon_img.h"
#include "icon_msx.h"
#include "icon_mini.h"
#include "icon_hb.h"

extern u16* colorTable;

extern int cursorPosOnScreen;

static int iconTexID[8];
sNDSHeaderExt ndsHeader;
sNDSBannerExt ndsBanner;

static bool infoFound[8] = {false};
static const char16_t *cachedTitle[8];
static const char16_t *blankTitle = u"";

static u32 arm9StartSig[8];

static glImage ndsIcon[8][(32 / 32) * (256 / 32)];
static u16 _paletteCache[8][16];

u8 *clearTiles;
u16 *blackPalette;
u8 *tilesModified;

static inline void writeBannerText(const int num, std::string_view text, const bool highlighted)
{
	const int ySpacing = (ms().ak_viewMode == TWLSettings::EViewSmallIcon) ? 18 : 38;
	const int xPos = (ms().ak_viewMode == TWLSettings::EViewSmallIcon) ? BOX_PX_SMALL : BOX_PX;
	const int yPos = ((ms().ak_viewMode == TWLSettings::EViewSmallIcon) ? BOX_PY_SMALL : BOX_PY) + (num * ySpacing);
	const FontPalette bannerFontPalette = highlighted ? FontPalette::mainTextHilight : FontPalette::mainText;

	printSmall(false, xPos, yPos - (calcSmallFontHeight(text) / 2), text, Alignment::left, bannerFontPalette);
}

static inline void writeBannerText(const int num, std::u16string_view text, const bool highlighted)
{
	const int ySpacing = (ms().ak_viewMode == TWLSettings::EViewSmallIcon) ? 18 : 38;
	const int xPos = (ms().ak_viewMode == TWLSettings::EViewSmallIcon) ? BOX_PX_SMALL : BOX_PX;
	const int yPos = ((ms().ak_viewMode == TWLSettings::EViewSmallIcon) ? BOX_PY_SMALL : BOX_PY) + (num * ySpacing);
	const FontPalette bannerFontPalette = highlighted ? FontPalette::mainTextHilight : FontPalette::mainText;

	printSmall(false, xPos, yPos - (calcSmallFontHeight(text) / 2), text, Alignment::left, bannerFontPalette);
}

static void convertIconTilesToRaw(u8 *tilesSrc, u8 *tilesNew, bool twl)
{
	const int PY = twl ? 32*8 : 32;
	const int PX = 16;
	const int TILE_SIZE_Y = 8;
	const int TILE_SIZE_X = 4;
	int index = 0;
	for (int tileY = 0; tileY < PY / TILE_SIZE_Y; ++tileY) {
		for (int tileX = 0; tileX < PX / TILE_SIZE_X; ++tileX)
			for (int pY = 0; pY < TILE_SIZE_Y; ++pY)
				for (int pX = 0; pX < TILE_SIZE_X; ++pX)//TILE_SIZE/2 since one u8 equals two pixels (4 bit depth)
					tilesNew[pX + tileX * TILE_SIZE_X + PX * (pY + tileY * TILE_SIZE_Y)] = tilesSrc[index++];
	}
}

void loadIcon(int num, u8 *tilesSrc, u16 *palSrc, bool twl)//(u8(*tilesSrc)[(32 * 32) / 2], u16(*palSrc)[16])
{
	convertIconTilesToRaw(tilesSrc, tilesModified, twl);

	int Ysize = 32;
	int textureSizeY = TEXTURE_SIZE_32;
	if (twl) {
		Ysize = 256;
		textureSizeY = TEXTURE_SIZE_256;
	}

	glDeleteTextures(1, &iconTexID[num]);

	swiCopy(palSrc, _paletteCache[num], 4 * sizeof(u16) | COPY_MODE_COPY | COPY_MODE_WORD);

	iconTexID[num] =
	glLoadTileSet(ndsIcon[num], // pointer to glImage array
				32, // sprite width
				32, // sprite height
				32, // bitmap image width
				Ysize, // bitmap image height
				GL_RGB16, // texture type for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeX for glTexImage2D() in videoGL.h
				textureSizeY, // sizeY for glTexImage2D() in videoGL.h
				GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T | TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT,
				16, // Length of the palette to use (16 colors)
				(u16*) palSrc, // Image palette
				(u8*) tilesModified // Raw image data
				);
}

void loadFolderIcon(int num)
{
	glDeleteTextures(1, &iconTexID[num]);
	swiCopy(icon_folderPal, _paletteCache[num], 4 * sizeof(u16) | COPY_MODE_COPY | COPY_MODE_WORD);
	iconTexID[num] =
	glLoadTileSet(ndsIcon[num], // pointer to glImage array
				32, // sprite width
				32, // sprite height
				32, // bitmap image width
				32, // bitmap image height
				GL_RGB16, // texture type for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeX for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeY for glTexImage2D() in videoGL.h
				TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT,
				16, // Length of the palette to use (16 colors)
				(u16*) icon_folderPal, // Image palette
				(u8*) icon_folderBitmap // Raw image data
				);
}

void loadUnkIcon(int num)
{
	glDeleteTextures(1, &iconTexID[num]);
	swiCopy(icon_unkPal, _paletteCache[num], 4 * sizeof(u16) | COPY_MODE_COPY | COPY_MODE_WORD);
	iconTexID[num] =
	glLoadTileSet(ndsIcon[num], // pointer to glImage array
				32, // sprite width
				32, // sprite height
				32, // bitmap image width
				32, // bitmap image height
				GL_RGB16, // texture type for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeX for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeY for glTexImage2D() in videoGL.h
				TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT,
				16, // Length of the palette to use (16 colors)
				(u16*) icon_unkPal, // Image palette
				(u8*) icon_unkBitmap // Raw image data
				);
}

void loadGBAIcon(int num)
{
	glDeleteTextures(1, &iconTexID[num]);
	swiCopy(icon_gbaPal, _paletteCache[num], 4 * sizeof(u16) | COPY_MODE_COPY | COPY_MODE_WORD);
	iconTexID[num] =
	glLoadTileSet(ndsIcon[num], // pointer to glImage array
				32, // sprite width
				32, // sprite height
				32, // bitmap image width
				32, // bitmap image height
				GL_RGB16, // texture type for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeX for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeY for glTexImage2D() in videoGL.h
				TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT,
				16, // Length of the palette to use (16 colors)
				(u16*) icon_gbaPal, // Image palette
				(u8*) icon_gbaBitmap // Raw image data
				);
}

void loadGBIcon(int num)
{
	glDeleteTextures(1, &iconTexID[num]);
	swiCopy(icon_gbPal, _paletteCache[num], 4 * sizeof(u16) | COPY_MODE_COPY | COPY_MODE_WORD);
	iconTexID[num] =
	glLoadTileSet(ndsIcon[num], // pointer to glImage array
				32, // sprite width
				32, // sprite height
				32, // bitmap image width
				32, // bitmap image height
				GL_RGB16, // texture type for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeX for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeY for glTexImage2D() in videoGL.h
				TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT,
				16, // Length of the palette to use (16 colors)
				(u16*) icon_gbPal, // Image palette
				(u8*) icon_gbBitmap // Raw image data
				);
}

void loadGBCIcon(int num)
{
	glDeleteTextures(1, &iconTexID[num]);
	swiCopy(icon_gbPal, _paletteCache[num], 4 * sizeof(u16) | COPY_MODE_COPY | COPY_MODE_WORD);
	iconTexID[num] =
	glLoadTileSet(ndsIcon[num], // pointer to glImage array
				32, // sprite width
				32, // sprite height
				32, // bitmap image width
				32, // bitmap image height
				GL_RGB16, // texture type for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeX for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeY for glTexImage2D() in videoGL.h
				TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT,
				16, // Length of the palette to use (16 colors)
				(u16*) icon_gbPal, // Image palette
				(u8*) icon_gbBitmap+(32*16) // Raw image data
				);
}

void loadNESIcon(int num)
{
	glDeleteTextures(1, &iconTexID[num]);
	swiCopy(icon_nesPal, _paletteCache[num], 4 * sizeof(u16) | COPY_MODE_COPY | COPY_MODE_WORD);
	iconTexID[num] =
	glLoadTileSet(ndsIcon[num], // pointer to glImage array
				32, // sprite width
				32, // sprite height
				32, // bitmap image width
				32, // bitmap image height
				GL_RGB16, // texture type for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeX for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeY for glTexImage2D() in videoGL.h
				TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT,
				16, // Length of the palette to use (16 colors)
				(u16*) icon_nesPal, // Image palette
				(u8*) icon_nesBitmap // Raw image data
				);
}

void loadSGIcon(int num)
{
	glDeleteTextures(1, &iconTexID[num]);
	swiCopy(icon_sgPal, _paletteCache[num], 4 * sizeof(u16) | COPY_MODE_COPY | COPY_MODE_WORD);
	iconTexID[num] =
	glLoadTileSet(ndsIcon[num], // pointer to glImage array
				32, // sprite width
				32, // sprite height
				32, // bitmap image width
				32, // bitmap image height
				GL_RGB16, // texture type for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeX for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeY for glTexImage2D() in videoGL.h
				TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT,
				16, // Length of the palette to use (16 colors)
				(u16*) icon_sgPal, // Image palette
				(u8*) icon_sgBitmap // Raw image data
				);
}

void loadSMSIcon(int num)
{
	glDeleteTextures(1, &iconTexID[num]);
	swiCopy(icon_smsPal, _paletteCache[num], 4 * sizeof(u16) | COPY_MODE_COPY | COPY_MODE_WORD);
	iconTexID[num] =
	glLoadTileSet(ndsIcon[num], // pointer to glImage array
				32, // sprite width
				32, // sprite height
				32, // bitmap image width
				32, // bitmap image height
				GL_RGB16, // texture type for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeX for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeY for glTexImage2D() in videoGL.h
				TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT,
				16, // Length of the palette to use (16 colors)
				(u16*) icon_smsPal, // Image palette
				(u8*) icon_smsBitmap // Raw image data
				);
}

void loadGGIcon(int num)
{
	glDeleteTextures(1, &iconTexID[num]);
	swiCopy(icon_ggPal, _paletteCache[num], 4 * sizeof(u16) | COPY_MODE_COPY | COPY_MODE_WORD);
	iconTexID[num] =
	glLoadTileSet(ndsIcon[num], // pointer to glImage array
				32, // sprite width
				32, // sprite height
				32, // bitmap image width
				32, // bitmap image height
				GL_RGB16, // texture type for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeX for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeY for glTexImage2D() in videoGL.h
				TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT,
				16, // Length of the palette to use (16 colors)
				(u16*) icon_ggPal, // Image palette
				(u8*) icon_ggBitmap // Raw image data
				);
}

void loadMDIcon(int num)
{
	glDeleteTextures(1, &iconTexID[num]);
	swiCopy(icon_mdPal, _paletteCache[num], 4 * sizeof(u16) | COPY_MODE_COPY | COPY_MODE_WORD);
	iconTexID[num] =
	glLoadTileSet(ndsIcon[num], // pointer to glImage array
				32, // sprite width
				32, // sprite height
				32, // bitmap image width
				32, // bitmap image height
				GL_RGB16, // texture type for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeX for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeY for glTexImage2D() in videoGL.h
				TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT,
				16, // Length of the palette to use (16 colors)
				(u16*) icon_mdPal, // Image palette
				(u8*) icon_mdBitmap // Raw image data
				);
}

void loadSNESIcon(int num)
{
	glDeleteTextures(1, &iconTexID[num]);
	swiCopy(icon_snesPal, _paletteCache[num], 4 * sizeof(u16) | COPY_MODE_COPY | COPY_MODE_WORD);
	iconTexID[num] =
	glLoadTileSet(ndsIcon[num], // pointer to glImage array
				32, // sprite width
				32, // sprite height
				32, // bitmap image width
				32, // bitmap image height
				GL_RGB16, // texture type for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeX for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeY for glTexImage2D() in videoGL.h
				TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT,
				16, // Length of the palette to use (16 colors)
				(u16*) icon_snesPal, // Image palette
				(u8*) icon_snesBitmap // Raw image data
				);
}

void loadPLGIcon(int num)
{
	glDeleteTextures(1, &iconTexID[num]);
	swiCopy(icon_plgPal, _paletteCache[num], 4 * sizeof(u16) | COPY_MODE_COPY | COPY_MODE_WORD);
	iconTexID[num] =
	glLoadTileSet(ndsIcon[num], // pointer to glImage array
				32, // sprite width
				32, // sprite height
				32, // bitmap image width
				32, // bitmap image height
				GL_RGB16, // texture type for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeX for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeY for glTexImage2D() in videoGL.h
				TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT,
				16, // Length of the palette to use (16 colors)
				(u16*) icon_plgPal, // Image palette
				(u8*) icon_plgBitmap // Raw image data
				);
}

void loadA26Icon(int num)
{
	glDeleteTextures(1, &iconTexID[num]);
	swiCopy(icon_a26Pal, _paletteCache[num], 4 * sizeof(u16) | COPY_MODE_COPY | COPY_MODE_WORD);
	iconTexID[num] =
	glLoadTileSet(ndsIcon[num], // pointer to glImage array
				32, // sprite width
				32, // sprite height
				32, // bitmap image width
				32, // bitmap image height
				GL_RGB16, // texture type for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeX for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeY for glTexImage2D() in videoGL.h
				TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT,
				16, // Length of the palette to use (16 colors)
				(u16*) icon_a26Pal, // Image palette
				(u8*) icon_a26Bitmap // Raw image data
				);
}

void loadCOLIcon(int num)
{
	glDeleteTextures(1, &iconTexID[num]);
	swiCopy(icon_colPal, _paletteCache[num], 4 * sizeof(u16) | COPY_MODE_COPY | COPY_MODE_WORD);
	iconTexID[num] =
	glLoadTileSet(ndsIcon[num], // pointer to glImage array
				32, // sprite width
				32, // sprite height
				32, // bitmap image width
				32, // bitmap image height
				GL_RGB16, // texture type for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeX for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeY for glTexImage2D() in videoGL.h
				TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT,
				16, // Length of the palette to use (16 colors)
				(u16*) icon_colPal, // Image palette
				(u8*) icon_colBitmap // Raw image data
				);
}

void loadM5Icon(int num)
{
	glDeleteTextures(1, &iconTexID[num]);
	swiCopy(icon_m5Pal, _paletteCache[num], 4 * sizeof(u16) | COPY_MODE_COPY | COPY_MODE_WORD);
	iconTexID[num] =
	glLoadTileSet(ndsIcon[num], // pointer to glImage array
				32, // sprite width
				32, // sprite height
				32, // bitmap image width
				32, // bitmap image height
				GL_RGB16, // texture type for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeX for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeY for glTexImage2D() in videoGL.h
				TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT,
				16, // Length of the palette to use (16 colors)
				(u16*) icon_m5Pal, // Image palette
				(u8*) icon_m5Bitmap // Raw image data
				);
}

void loadINTIcon(int num)
{
	glDeleteTextures(1, &iconTexID[num]);
	swiCopy(icon_intPal, _paletteCache[num], 4 * sizeof(u16) | COPY_MODE_COPY | COPY_MODE_WORD);
	iconTexID[num] =
	glLoadTileSet(ndsIcon[num], // pointer to glImage array
				32, // sprite width
				32, // sprite height
				32, // bitmap image width
				32, // bitmap image height
				GL_RGB16, // texture type for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeX for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeY for glTexImage2D() in videoGL.h
				TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT,
				16, // Length of the palette to use (16 colors)
				(u16*) icon_intPal, // Image palette
				(u8*) icon_intBitmap // Raw image data
				);
}

void loadPCEIcon(int num)
{
	glDeleteTextures(1, &iconTexID[num]);
	swiCopy(icon_pcePal, _paletteCache[num], 4 * sizeof(u16) | COPY_MODE_COPY | COPY_MODE_WORD);
	iconTexID[num] =
	glLoadTileSet(ndsIcon[num], // pointer to glImage array
				32, // sprite width
				32, // sprite height
				32, // bitmap image width
				32, // bitmap image height
				GL_RGB16, // texture type for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeX for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeY for glTexImage2D() in videoGL.h
				TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT,
				16, // Length of the palette to use (16 colors)
				(u16*) icon_pcePal, // Image palette
				(u8*) icon_pceBitmap // Raw image data
				);
}

void loadWSIcon(int num)
{
	glDeleteTextures(1, &iconTexID[num]);
	swiCopy(icon_wsPal, _paletteCache[num], 4 * sizeof(u16) | COPY_MODE_COPY | COPY_MODE_WORD);
	iconTexID[num] =
	glLoadTileSet(ndsIcon[num], // pointer to glImage array
				32, // sprite width
				32, // sprite height
				32, // bitmap image width
				32, // bitmap image height
				GL_RGB16, // texture type for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeX for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeY for glTexImage2D() in videoGL.h
				TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT,
				16, // Length of the palette to use (16 colors)
				(u16*) icon_wsPal, // Image palette
				(u8*) icon_wsBitmap // Raw image data
				);
}

void loadNGPIcon(int num)
{
	glDeleteTextures(1, &iconTexID[num]);
	swiCopy(icon_ngpPal, _paletteCache[num], 4 * sizeof(u16) | COPY_MODE_COPY | COPY_MODE_WORD);
	iconTexID[num] =
	glLoadTileSet(ndsIcon[num], // pointer to glImage array
				32, // sprite width
				32, // sprite height
				32, // bitmap image width
				32, // bitmap image height
				GL_RGB16, // texture type for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeX for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeY for glTexImage2D() in videoGL.h
				TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT,
				16, // Length of the palette to use (16 colors)
				(u16*) icon_ngpPal, // Image palette
				(u8*) icon_ngpBitmap // Raw image data
				);
}

void loadCPCIcon(int num)
{
	glDeleteTextures(1, &iconTexID[num]);
	swiCopy(icon_cpcPal, _paletteCache[num], 4 * sizeof(u16) | COPY_MODE_COPY | COPY_MODE_WORD);
	iconTexID[num] =
	glLoadTileSet(ndsIcon[num], // pointer to glImage array
				32, // sprite width
				32, // sprite height
				32, // bitmap image width
				32, // bitmap image height
				GL_RGB16, // texture type for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeX for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeY for glTexImage2D() in videoGL.h
				TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT,
				16, // Length of the palette to use (16 colors)
				(u16*) icon_cpcPal, // Image palette
				(u8*) icon_cpcBitmap // Raw image data
				);
}

void loadVIDIcon(int num)
{
	glDeleteTextures(1, &iconTexID[num]);
	swiCopy(icon_vidPal, _paletteCache[num], 4 * sizeof(u16) | COPY_MODE_COPY | COPY_MODE_WORD);
	iconTexID[num] =
	glLoadTileSet(ndsIcon[num], // pointer to glImage array
				32, // sprite width
				32, // sprite height
				32, // bitmap image width
				32, // bitmap image height
				GL_RGB16, // texture type for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeX for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeY for glTexImage2D() in videoGL.h
				TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT,
				16, // Length of the palette to use (16 colors)
				(u16*) icon_vidPal, // Image palette
				(u8*) icon_vidBitmap // Raw image data
				);
}

void loadIMGIcon(int num)
{
	glDeleteTextures(1, &iconTexID[num]);
	swiCopy(icon_imgPal, _paletteCache[num], 4 * sizeof(u16) | COPY_MODE_COPY | COPY_MODE_WORD);
	iconTexID[num] =
	glLoadTileSet(ndsIcon[num], // pointer to glImage array
				32, // sprite width
				32, // sprite height
				32, // bitmap image width
				32, // bitmap image height
				GL_RGB16, // texture type for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeX for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeY for glTexImage2D() in videoGL.h
				TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT,
				16, // Length of the palette to use (16 colors)
				(u16*) icon_imgPal, // Image palette
				(u8*) icon_imgBitmap // Raw image data
				);
}

void loadMSXIcon(int num)
{
	glDeleteTextures(1, &iconTexID[num]);
	swiCopy(icon_msxPal, _paletteCache[num], 4 * sizeof(u16) | COPY_MODE_COPY | COPY_MODE_WORD);
	iconTexID[num] =
	glLoadTileSet(ndsIcon[num], // pointer to glImage array
				32, // sprite width
				32, // sprite height
				32, // bitmap image width
				32, // bitmap image height
				GL_RGB16, // texture type for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeX for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeY for glTexImage2D() in videoGL.h
				TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT,
				16, // Length of the palette to use (16 colors)
				(u16*) icon_msxPal, // Image palette
				(u8*) icon_msxBitmap // Raw image data
				);
}

void loadMINIcon(int num)
{
	glDeleteTextures(1, &iconTexID[num]);
	swiCopy(icon_miniPal, _paletteCache[num], 4 * sizeof(u16) | COPY_MODE_COPY | COPY_MODE_WORD);
	iconTexID[num] =
	glLoadTileSet(ndsIcon[num], // pointer to glImage array
				32, // sprite width
				32, // sprite height
				32, // bitmap image width
				32, // bitmap image height
				GL_RGB16, // texture type for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeX for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeY for glTexImage2D() in videoGL.h
				TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT,
				16, // Length of the palette to use (16 colors)
				(u16*) icon_miniPal, // Image palette
				(u8*) icon_miniBitmap // Raw image data
				);
}

void loadHBIcon(int num)
{
	glDeleteTextures(1, &iconTexID[num]);
	swiCopy(icon_hbPal, _paletteCache[num], 4 * sizeof(u16) | COPY_MODE_COPY | COPY_MODE_WORD);
	iconTexID[num] =
	glLoadTileSet(ndsIcon[num], // pointer to glImage array
				32, // sprite width
				32, // sprite height
				32, // bitmap image width
				32, // bitmap image height
				GL_RGB16, // texture type for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeX for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeY for glTexImage2D() in videoGL.h
				TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT,
				16, // Length of the palette to use (16 colors)
				(u16*) icon_hbPal, // Image palette
				(u8*) icon_hbBitmap // Raw image data
				);
}

/**
 * Reloads the palette in the given slot from
 * the palette cache.
 */
void glReloadIconPalette(int num) {

	int textureID;
	const u16 *cachedPalette;
	switch (num) {
	default:
		if (BAD_ICON_IDX(num))
			return;
		textureID = iconTexID[num];
		cachedPalette = _paletteCache[num];
		break;
	}
	
	glBindTexture(0, textureID);
	glColorTableEXT(0, 0, 16, 0, 0, cachedPalette);
}

void glLoadPalette(int num, const u16 *_palette) {
	if (!BAD_ICON_IDX(num))
		swiCopy(_palette, _paletteCache[num], 4 * sizeof(u16) | COPY_MODE_COPY | COPY_MODE_WORD);

	glReloadIconPalette(num);
}

/**
 * Reloads all the palettes in the palette cache if
 * they have been corrupted.
 */
void reloadIconPalettes() {
	for (int i = 0; i < NDS_ICON_BANK_COUNT; i++) {
		glReloadIconPalette(i);
	}
}

void loadConsoleIcons()
{
	if (!colorTable) {
		return;
	}

	// Unknown
	u16* newPalette = (u16*)icon_unkPal;
	for (int i2 = 0; i2 < 16; i2++) {
		*(newPalette+i2) = colorTable[*(newPalette+i2) % 0x8000];
	}

	// GBA
	if (ms().gbaBooter == TWLSettings::EGbaGbar2) {
		newPalette = (u16*)icon_gbaPal;
	} else {
		newPalette = (u16*)icon_gbamodePal;
	}
	for (int i2 = 0; i2 < 16; i2++) {
		*(newPalette+i2) = colorTable[*(newPalette+i2) % 0x8000];
	}

	// GB/GBC
	newPalette = (u16*)icon_gbPal;
	for (int i2 = 0; i2 < 16; i2++) {
		*(newPalette+i2) = colorTable[*(newPalette+i2) % 0x8000];
	}

	// NES
	newPalette = (u16*)icon_nesPal;
	for (int i2 = 0; i2 < 16; i2++) {
		*(newPalette+i2) = colorTable[*(newPalette+i2) % 0x8000];
	}

	// SG
	newPalette = (u16*)icon_sgPal;
	for (int i2 = 0; i2 < 16; i2++) {
		*(newPalette+i2) = colorTable[*(newPalette+i2) % 0x8000];
	}

	// SMS
	newPalette = (u16*)icon_smsPal;
	for (int i2 = 0; i2 < 16; i2++) {
		*(newPalette+i2) = colorTable[*(newPalette+i2) % 0x8000];
	}

	// GG
	newPalette = (u16*)icon_ggPal;
	for (int i2 = 0; i2 < 16; i2++) {
		*(newPalette+i2) = colorTable[*(newPalette+i2) % 0x8000];
	}

	// MD
	newPalette = (u16*)icon_ggPal;
	for (int i2 = 0; i2 < 16; i2++) {
		*(newPalette+i2) = colorTable[*(newPalette+i2) % 0x8000];
	}

	// SNES
	newPalette = (u16*)icon_snesPal;
	for (int i2 = 0; i2 < 16; i2++) {
		*(newPalette+i2) = colorTable[*(newPalette+i2) % 0x8000];
	}

	// DSTWO Plugin
	newPalette = (u16*)icon_plgPal;
	for (int i2 = 0; i2 < 16; i2++) {
		*(newPalette+i2) = colorTable[*(newPalette+i2) % 0x8000];
	}

	// A26
	newPalette = (u16*)icon_a26Pal;
	for (int i2 = 0; i2 < 16; i2++) {
		*(newPalette+i2) = colorTable[*(newPalette+i2) % 0x8000];
	}

	// COL
	newPalette = (u16*)icon_colPal;
	for (int i2 = 0; i2 < 16; i2++) {
		*(newPalette+i2) = colorTable[*(newPalette+i2) % 0x8000];
	}

	// M5
	newPalette = (u16*)icon_m5Pal;
	for (int i2 = 0; i2 < 16; i2++) {
		*(newPalette+i2) = colorTable[*(newPalette+i2) % 0x8000];
	}

	// INT
	newPalette = (u16*)icon_intPal;
	for (int i2 = 0; i2 < 16; i2++) {
		*(newPalette+i2) = colorTable[*(newPalette+i2) % 0x8000];
	}

	// PCE
	newPalette = (u16*)icon_pcePal;
	for (int i2 = 0; i2 < 16; i2++) {
		*(newPalette+i2) = colorTable[*(newPalette+i2) % 0x8000];
	}

	// WS
	newPalette = (u16*)icon_wsPal;
	for (int i2 = 0; i2 < 16; i2++) {
		*(newPalette+i2) = colorTable[*(newPalette+i2) % 0x8000];
	}

	// NGP
	newPalette = (u16*)icon_ngpPal;
	for (int i2 = 0; i2 < 16; i2++) {
		*(newPalette+i2) = colorTable[*(newPalette+i2) % 0x8000];
	}

	// CPC
	newPalette = (u16*)icon_cpcPal;
	for (int i2 = 0; i2 < 16; i2++) {
		*(newPalette+i2) = colorTable[*(newPalette+i2) % 0x8000];
	}

	// Video
	newPalette = (u16*)icon_vidPal;
	for (int i2 = 0; i2 < 16; i2++) {
		*(newPalette+i2) = colorTable[*(newPalette+i2) % 0x8000];
	}

	// Image
	newPalette = (u16*)icon_imgPal;
	for (int i2 = 0; i2 < 16; i2++) {
		*(newPalette+i2) = colorTable[*(newPalette+i2) % 0x8000];
	}

	// MSX
	newPalette = (u16*)icon_msxPal;
	for (int i2 = 0; i2 < 16; i2++) {
		*(newPalette+i2) = colorTable[*(newPalette+i2) % 0x8000];
	}

	// MINI
	newPalette = (u16*)icon_miniPal;
	for (int i2 = 0; i2 < 16; i2++) {
		*(newPalette+i2) = colorTable[*(newPalette+i2) % 0x8000];
	}

	// Homebrew
	newPalette = (u16*)icon_hbPal;
	for (int i2 = 0; i2 < 16; i2++) {
		*(newPalette+i2) = colorTable[*(newPalette+i2) % 0x8000];
	}
}

static void clearIcon(int num)
{
	loadIcon(num, clearTiles, blackPalette, true);
}

void convertIconPalette(sNDSBannerExt* ndsBanner) {
	if (!colorTable) return;

	for (int i = 0; i < 16; i++)
		ndsBanner->palette[i] = colorTable[ndsBanner->palette[i] % 0x8000];

	if (ms().animateDsiIcons && ndsBanner->version == NDS_BANNER_VER_DSi) {
		for (int i2 = 0; i2 < 8; i2++) {
			for (int i = 0; i < 16; i++)
				ndsBanner->dsi_palette[i2][i] = colorTable[ndsBanner->dsi_palette[i2][i] % 0x8000];
		}
	}
}

void iconTitleInit()
{
	clearTiles = new u8[(32 * 256) / 2]();
	blackPalette = new u16[16*8]();
	tilesModified = new u8[(32 * 256) / 2];

	for (int i = 0; i < 8; i++) {
		clearIcon(i);
	}
}

void drawIcon(int num, int Xpos, int Ypos, s32 scale) {
	(scale == 0) ? glSprite(Xpos, Ypos, bannerFlip[num], &ndsIcon[num][bnriconframenumY[num] & 31]) : glSpriteScale(Xpos, Ypos, scale, bannerFlip[num], &ndsIcon[num][bnriconframenumY[num] & 31]);
	if (bnriconPalLine[num] != bnriconPalLoaded[num]) {
		glLoadPalette(num, bnriconTile[num].dsi_palette[bnriconPalLine[num]]);
		bnriconPalLoaded[num] = bnriconPalLine[num];
	}
}

void clearTitle(int num) {
	cachedTitle[num] = blankTitle;
}

/* void copyGameInfo(int numDst, int numSrc)
{
	bnriconPalLine[numDst] = bnriconPalLine[numSrc];
	bnriconPalLoaded[numDst] = bnriconPalLoaded[numSrc];
	bnriconframenumY[numDst] = bnriconframenumY[numSrc];
	bannerFlip[numDst] = bannerFlip[numSrc];
	bnrWirelessIcon[numDst] = bnrWirelessIcon[numSrc];
	customIcon[numDst] = customIcon[numSrc];
	tonccpy(gameTid[numDst], gameTid[numSrc], 4);
	isValid[numDst] = isValid[numSrc];
	isTwlm[numDst] = isTwlm[numSrc];
	isDSiWare[numDst] = isDSiWare[numSrc];
	isHomebrew[numDst] = isHomebrew[numSrc];
	isModernHomebrew[numDst] = isModernHomebrew[numSrc];
	requiresRamDisk[numDst] = requiresRamDisk[numSrc];
	requiresDonorRom[numDst] = requiresDonorRom[numSrc];
	infoFound[numDst] = infoFound[numSrc];

	romVersion[numDst] = romVersion[numSrc];
	romUnitCode[numDst] = romUnitCode[numSrc];
	a7mbk6[numDst] = a7mbk6[numSrc];
	requiresRamDisk[numDst] = requiresRamDisk[numSrc];
	bnrWirelessIcon[numDst] = bnrWirelessIcon[numSrc];

	bnriconTile[numDst] = bnriconTile[numSrc];
	cachedTitle[numDst] = cachedTitle[numSrc];
	if (bnriconisDSi[numSrc]) {
		copyBannerSequence(numDst, numSrc);
	}

	bnriconisDSi[numDst] = bnriconisDSi[numSrc];
} */

void getGameInfo(int num, int fileOffset, bool isDir, const char* name, bool fromArgv)
{
	bnriconPalLine[num] = 0;
	bnriconPalLoaded[num] = 0;
	bnriconframenumY[num] = 0;
	bannerFlip[num] = GL_FLIP_NONE;
	bnrWirelessIcon[num] = 0;
	toncset(gameTid[num], 0, 4);
	isValid[num] = false;
	isTwlm[num] = false;
	isDSiWare[num] = false;
	isHomebrew[num] = true;
	isModernHomebrew[num] = true;
	requiresRamDisk[num] = false;
	requiresDonorRom[num] = false;
	if (!fromArgv) {
		bnriconisDSi[num] = false;
		customIcon[num] = 0;
		infoFound[num] = false;
	}

	if (ms().showCustomIcons && !preloadedBannerIconFound(fileOffset) && customIcon[num] < 2 && (!fromArgv || customIcon[num] <= 0)) {
		sNDSBannerExt &banner = bnriconTile[num];
		const bool argvHadPng = customIcon[num] == 1;
		u8 iconCopy[512];
		u16 paletteCopy[16];
		if (argvHadPng) { // custom png icon from argv
			// copy the icon and palette before they get overwritten
			memcpy(iconCopy, banner.icon, sizeof(iconCopy));
			memcpy(paletteCopy, banner.palette, sizeof(paletteCopy));
		}
		toncset(&banner, 0, sizeof(sNDSBannerExt));
		bool customIconGood = false;

		// First try banner bin
		snprintf(customIconPath, sizeof(customIconPath), "%s:/_nds/TWiLightMenu/icons/%s.bin", sys().isRunFromSD() ? "sd" : "fat", name);
		if (access(customIconPath, F_OK) == 0) {
			customIcon[num] = 2; // custom icon is a banner bin
			FILE *file = fopen(customIconPath, "rb");
			if (file) {
				size_t read = fread(&banner, 1, sizeof(sNDSBannerExt), file);
				fclose(file);

				// restore png icon
				if (argvHadPng) {
					memcpy(banner.icon, iconCopy, sizeof(iconCopy));
					memcpy(banner.palette, paletteCopy, sizeof(paletteCopy));
					if (ndsBanner.version == NDS_BANNER_VER_DSi) {
						ndsBanner.version = NDS_BANNER_VER_ZH_KO;
					}
				}

				if (read >= NDS_BANNER_SIZE_ORIGINAL) {
					customIconGood = true;
					tonccpy(getPreloadedBannerIcon(fileOffset), &banner, sizeof(sNDSBannerExt));
					if (dsiFeatures()) {
						bannerIconPreloaded[fileOffset] = true;
					}
				}
			}
		} else if (customIcon[num] == 0) {
			// If no banner bin, try png
			snprintf(customIconPath, sizeof(customIconPath), "%s:/_nds/TWiLightMenu/icons/%s.png", sys().isRunFromSD() ? "sd" : "fat", name);
			customIcon[num] = (access(customIconPath, F_OK) == 0);
			if (customIcon[num]) {
				std::vector<unsigned char> image;
				uint imageWidth, imageHeight;
				lodepng::decode(image, imageWidth, imageHeight, customIconPath);
				if (imageWidth <= 32 && imageHeight <= 32) {
					customIconGood = true;

					// if smaller than 32x32, center it
					int xOfs = (32 - imageWidth) / 2;
					int yOfs = (32 - imageHeight) / 2;

					uint colorCount = 1;
					for (uint i = 0; i < image.size()/4; i++) {
						// calculate byte and nibble position of pixel in tiled banner icon
						uint x = xOfs + (i%imageWidth), y = yOfs + (i/imageWidth);
						uint tileX = x/8, tileY = y/8;
						uint offX = x%8, offY = y%8;
						uint pos = tileX*32 + tileY*128 + offX/2 + offY*4;
						bool nibble = offX%2;
						// clear pixel (using transparent palette slot)
						banner.icon[pos] &= nibble? 0x0f : 0xf0;
						// read color
						if (image[i*4+3] == 255) {
							// convert to bgr565
							const u16 green = (image[i*4+1]>>2)<<5;
							u16 color = image[i*4]>>3 | (image[i*4+2]>>3)<<10;
							if (green & BIT(5)) {
								color |= BIT(15);
							}
							for (int g = 6; g <= 10; g++) {
								if (green & BIT(g)) {
									color |= BIT(g-1);
								}
							}
							// find color in palette
							bool found = false;
							for (uint palIdx = 1; palIdx < colorCount; palIdx++) {
								if (banner.palette[palIdx] == color) {
									banner.icon[pos] |= nibble? palIdx<<4 : palIdx;
									found = true;
									break;
								}
							}
							// add color to palette if room available
							if (!found && colorCount < 16) {
								banner.icon[pos] |= nibble? colorCount<<4 : colorCount;
								banner.palette[colorCount++] = color;
							}
						}
	
						convertIconPalette(&banner);
					}
				}
			}
		}

		if (customIcon[num] && !customIconGood)
			customIcon[num] = -1; // display as unknown
	}

	if (isDir) {
		clearTitle(num);
		if (customIcon[num] != 2)
			clearBannerSequence(num); // banner sequence
	} else if (extension(name, {".argv"})) {
		// look through the argv file for the corresponding nds file
		FILE *fp;
		char *line = NULL, *p = NULL;
		size_t size = 0;
		ssize_t rc;

		// open the argv file
		fp = fopen(name, "rb");
		if (!fp) {
			clearTitle(num);
			if (customIcon[num] != 2)
				clearBannerSequence(num);
			fclose(fp);
			return;
		}

		// read each line
		while ((rc = __getline(&line, &size, fp)) > 0) {
			// remove comments
			if ((p = strchr(line, '#')) != NULL)
				*p = 0;

			// skip leading whitespace
			for (p = line; *p && isspace((int)*p); ++p)
				;

			if (*p)
				break;
		}

		// done with the file at this point
		fclose(fp);

		if (p && *p) {
			// we found an argument
			struct stat st;

			// truncate everything after first argument
			strtok(p, "\n\r\t ");

			if (extension(p, {".nds", ".dsi", ".ids", ".srl", ".app"})) {
				// let's see if this is a file or directory
				rc = stat(p, &st);
				if (rc != 0) {
					// stat failed
					clearTitle(num);
					if (customIcon[num] != 2)
						clearBannerSequence(num);
				} else if (S_ISDIR(st.st_mode)) {
					// this is a directory!
					clearTitle(num);
					if (customIcon[num] != 2)
						clearBannerSequence(num);
				} else {
					getGameInfo(num, fileOffset, true, p, true);
				}
			} else {
				// this is not an nds/app file!
				clearTitle(num);
				if (customIcon[num] != 2)
					clearBannerSequence(num);
			}
		} else {
			clearTitle(num);
			if (customIcon[num] != 2)
				clearBannerSequence(num);
		}
		// clean up the allocated line
		free(line);
	} else if (extension(name, {".gbc"})) {
		// this is a gbc file!
		FILE *fp;

		// open file for reading info
		fp = fopen(name, "rb");
		if (!fp) {
			fclose(fp);
			return;
		}

		fseek(fp, 0x13F, SEEK_SET);
		fread(gameTid[num], 1, 4, fp);

		fclose(fp);
	} else if (extension(name, {".agb", ".gba", ".mb"})) {
		// this is a gba file!
		FILE *fp;

		// open file for reading info
		fp = fopen(name, "rb");
		if (!fp) {
			fclose(fp);
			return;
		}

		fseek(fp, 0xAC, SEEK_SET);
		fread(gameTid[num], 1, 4, fp);

		fclose(fp);
	} else if (extension(name, {".nds", ".dsi", ".ids", ".srl", ".app"})) {
		// this is an nds/app file!
		// open file for reading info
		FILE *fp = fopen(name, "rb");
		if (!fp) {
			clearTitle(num);
			if (customIcon[num] != 2)
				clearBannerSequence(num); // banner sequence
			fclose(fp);
			return;
		}

		sNDSHeaderExt ndsHeader;

		if (preloadedHeaderFound(fileOffset)) {
			tonccpy(&ndsHeader, getPreloadedHeader(fileOffset), sizeof(sNDSHeaderExt));
		} else if (!fread(&ndsHeader, sizeof(sNDSHeaderExt), 1, fp)) {
			// try again, but using regular header size
			fseek(fp, 0, SEEK_SET);
			if (!fread(&ndsHeader, 0x160, 1, fp)) {
				clearTitle(num);
				if (customIcon[num] != 2)
					clearBannerSequence(num);
				fclose(fp);
				return;
			}
		}

		if (!preloadedHeaderFound(fileOffset)) {
			tonccpy(getPreloadedHeader(fileOffset), &ndsHeader, sizeof(sNDSHeaderExt));
			if (dsiFeatures()) {
				headerPreloaded[fileOffset] = true;
			}
		}

		tonccpy(gameTid[num], ndsHeader.gameCode, 4);
		isValid[num] = (ndsHeader.arm9destination >= 0x02000000 && ndsHeader.arm9destination < 0x03000000 && ndsHeader.arm9executeAddress >= 0x02000000 && ndsHeader.arm9executeAddress < 0x03000000);
		isTwlm[num] = (strcmp(gameTid[num], "SRLA") == 0);
		romVersion[num] = ndsHeader.romversion;
		romUnitCode[num] = ndsHeader.unitCode;
		a7mbk6[num] = ndsHeader.a7mbk6;

		fseek(fp, ndsHeader.arm9romOffset + ndsHeader.arm9executeAddress - ndsHeader.arm9destination, SEEK_SET);
		fread(arm9StartSig, sizeof(u32), 4, fp);
		if ((arm9StartSig[0] == 0xE3A0C301 || (arm9StartSig[0] >= 0xEA000000 && arm9StartSig[0] < 0xEC000000 /* If title contains cracktro or extra splash */))
		  && arm9StartSig[1] == 0xE58CC208) {
			// Title seems to be developed with Nintendo SDK, verify
			if ((arm9StartSig[2] >= 0xEB000000 && arm9StartSig[2] < 0xEC000000) // SDK 2 & TWL SDK 5
			 && (arm9StartSig[3] >= 0xE3A00000 && arm9StartSig[3] < 0xE3A01000)) {
				isHomebrew[num] = false;
				isModernHomebrew[num] = false;
			} else
			if (arm9StartSig[2] == 0xE1DC00B6 // SDK 3-5
			 && arm9StartSig[3] == 0xE3500000) {
				isHomebrew[num] = false;
				isModernHomebrew[num] = false;
			} else
			if (arm9StartSig[2] == 0xEAFFFFFF // SDK 4 (HM DS Cute)
			 && arm9StartSig[3] == 0xE1DC00B6) {
				isHomebrew[num] = false;
				isModernHomebrew[num] = false;
			}
		} else if (strncmp(gameTid[num], "HNA", 3) == 0) {
			// Modcrypted
			isHomebrew[num] = false;
			isModernHomebrew[num] = false;
		}

		if (isHomebrew[num]) {
			if (arm9StartSig[0] == 0xE3A00301
			 && arm9StartSig[1] == 0xE5800208
			 && arm9StartSig[2] == 0xE3A00013
			 && arm9StartSig[3] == 0xE129F000) {
				// isModernHomebrew[num] = true; // Homebrew is recent (supports reading from SD without a DLDI driver)
				if (ndsHeader.arm7executeAddress >= 0x037F0000 && ndsHeader.arm7destination >= 0x037F0000) {
					if ((ndsHeader.arm9binarySize == 0xC9F68 && ndsHeader.arm7binarySize == 0x12814)	// Colors! v1.1
					|| (ndsHeader.arm9binarySize == 0x1B0864 && ndsHeader.arm7binarySize == 0xDB50)	// Mario Paint Composer DS v2 (Bullet Bill)
					|| (ndsHeader.arm9binarySize == 0xE78FC && ndsHeader.arm7binarySize == 0xF068)		// SnowBros v2.2
					|| (ndsHeader.arm9binarySize == 0xD45C0 && ndsHeader.arm7binarySize == 0x2B7C)		// ikuReader v0.058
					|| (ndsHeader.arm9binarySize == 0x7A124 && ndsHeader.arm7binarySize == 0xEED0)		// PPSEDS r11
					|| (ndsHeader.arm9binarySize == 0x54620 && ndsHeader.arm7binarySize == 0x1538)		// XRoar 0.24fp3
					|| (ndsHeader.arm9binarySize == 0x2C9A8 && ndsHeader.arm7binarySize == 0xFB98)		// NitroGrafx v0.7
					|| (ndsHeader.arm9binarySize == 0x22AE4 && ndsHeader.arm7binarySize == 0xA764)) {	// It's 1975 and this man is about to show you the future
						isModernHomebrew[num] = false; // Have nds-bootstrap load it (in case if it doesn't)
					}
				}
			} else if ((ndsHeader.unitCode == 0) && ((memcmp(ndsHeader.gameTitle, "NMP4BOOT", 8) == 0)
			 || (ndsHeader.arm7executeAddress >= 0x037F0000 && ndsHeader.arm7destination >= 0x037F0000))) {
				isModernHomebrew[num] = false; // Homebrew is old (requires a DLDI driver to read from SD)
			}
			if (!ms().secondaryDevice && num < 40) {
				if ((ndsHeader.arm9binarySize == 0x98F70 && ndsHeader.arm7binarySize == 0xED94)		// jEnesisDS 0.7.4
				|| (ndsHeader.arm9binarySize == 0x48950 && ndsHeader.arm7binarySize == 0x74C4)			// SNEmulDS06-WIP2
				|| (ndsHeader.arm9binarySize == 0xD45C0 && ndsHeader.arm7binarySize == 0x2B7C)			// ikuReader v0.058
				|| (ndsHeader.arm9binarySize == 0x54620 && ndsHeader.arm7binarySize == 0x1538)) {		// XRoar 0.24fp3
					requiresRamDisk[num] = true;
				}
			}
		} else if (ndsHeader.unitCode != 0 && (ndsHeader.accessControl & BIT(4))) {
			isDSiWare[num] = true; // Is a DSiWare game
		}

		if (!isHomebrew[num]) {
			// Check if ROM needs a donor ROM
			bool dsiEnhancedMbk = (isDSiMode() && *(u32*)0x02FFE1A0 == 0x00403000 && sys().arm7SCFGLocked());
			if (isDSiMode() && (a7mbk6[num] == (dsiEnhancedMbk ? 0x080037C0 : 0x00403000) || (ndsHeader.gameCode[0] == 'H' && ndsHeader.arm7binarySize < 0xC000 && ndsHeader.arm7idestination == 0x02E80000 && (REG_MBK9 & 0x00FFFFFF) != 0x00FFFF0F)) && sys().arm7SCFGLocked()) {
				requiresDonorRom[num] = dsiEnhancedMbk ? 51 : 52; // DSi-Enhanced ROM required on CycloDSi, or DSi-Exclusive/DSiWare ROM required on DSiWarehax
				if (ndsHeader.gameCode[0] == 'H' && ndsHeader.arm7binarySize < 0xC000 && ndsHeader.arm7idestination == 0x02E80000) {
					requiresDonorRom[num] += 100;
				} else if (memcmp(ndsHeader.gameCode, "KCX", 3) == 0 && dsiEnhancedMbk) {
					requiresDonorRom[num] = 0;
				}
			} else if (a7mbk6[num] == 0x080037C0 && ms().secondaryDevice && (!dsiFeatures() || bs().b4dsMode)
			&& (((sys().dsDebugRam() || (dsiFeatures() && bs().b4dsMode == 2)) ? (memcmp(ndsHeader.gameCode, "DME", 3) == 0 || memcmp(ndsHeader.gameCode, "DMF", 3) == 0 || memcmp(ndsHeader.gameCode, "DMD", 3) == 0 || memcmp(ndsHeader.gameCode, "DMP", 3) == 0 || memcmp(ndsHeader.gameCode, "DHS", 3) == 0 || memcmp(ndsHeader.gameCode, "DSY", 3) == 0) : (memcmp(ndsHeader.gameCode, "DMF", 3) == 0 || memcmp(ndsHeader.gameCode, "DMP", 3) == 0 || memcmp(ndsHeader.gameCode, "DHS", 3) == 0))
			|| (ndsHeader.gameCode[0] != 'D' && memcmp(ndsHeader.gameCode, "KCX", 3) != 0 && memcmp(ndsHeader.gameCode, "KAV", 3) != 0 && memcmp(ndsHeader.gameCode, "KNK", 3) != 0 && memcmp(ndsHeader.gameCode, "KE3", 3) != 0))) {
				requiresDonorRom[num] = 51; // SDK5 ROM required
			} else if (memcmp(ndsHeader.gameCode, "AYI", 3) == 0 && ndsHeader.arm7binarySize == 0x25F70) {
				requiresDonorRom[num] = 20; // SDK2.0 ROM required for Yoshi Touch & Go (Europe)
			}
		}

		if (ndsHeader.dsi_flags & BIT(4))
			bnrWirelessIcon[num] = 1;
		else if (ndsHeader.dsi_flags & BIT(3))
			bnrWirelessIcon[num] = 2;

		sNDSBannerExt &ndsBanner = bnriconTile[num];

		u8 iconCopy[512];
		u16 paletteCopy[16];
		if (preloadedBannerIconFound(fileOffset)) {
			tonccpy(&ndsBanner, getPreloadedBannerIcon(fileOffset), sizeof(sNDSBannerExt));
		} else {
			if (customIcon[num] == 1) { // custom png icon
				// copy the icon and palette before they get overwritten
				memcpy(iconCopy, ndsBanner.icon, sizeof(iconCopy));
				memcpy(paletteCopy, ndsBanner.palette, sizeof(paletteCopy));
			}

			if (customIcon[num] != 2 && ndsHeader.bannerOffset == 0) {
				fclose(fp);

				// If no custom icon, display as unknown
				if (customIcon[num] == 0)
					customIcon[num] = -1;

				return;
			}

			fseek(fp, ndsHeader.bannerOffset, SEEK_SET);
			if (!fread(&ndsBanner, sizeof(ndsBanner), 1, fp)) {
				// try again, but using regular banner size
				fseek(fp, ndsHeader.bannerOffset, SEEK_SET);
				if (!fread(&ndsBanner, NDS_BANNER_SIZE_ORIGINAL, 1, fp)) {
					fclose(fp);

					// If no custom icon, display as unknown
					if (customIcon[num] == 0)
						customIcon[num] = -1;

					return;
				}
			}
		}

		// close file!
		fclose(fp);

		int currentLang = 0;
		if (ndsBanner.version == NDS_BANNER_VER_ZH || ndsBanner.version == NDS_BANNER_VER_ZH_KO || ndsBanner.version == NDS_BANNER_VER_DSi) {
			currentLang = ms().getGameLanguage();
		} else {
			currentLang = ms().getTitleLanguage();
		}
		while (ndsBanner.titles[currentLang][0] == 0 || (ndsBanner.titles[currentLang][0] == 0x20 && ndsBanner.titles[currentLang][1] == 0)) {
			if (currentLang == 0) break;
			currentLang--;
		}

		if (!preloadedBannerIconFound(fileOffset)) {
			// restore png icon
			if (customIcon[num] == 1) {
				tonccpy(ndsBanner.icon, iconCopy, sizeof(iconCopy));
				tonccpy(ndsBanner.palette, paletteCopy, sizeof(paletteCopy));
				if (ndsBanner.version == NDS_BANNER_VER_DSi) {
					ndsBanner.version = NDS_BANNER_VER_ZH_KO;
				}
			}

			tonccpy(getPreloadedBannerIcon(fileOffset), &ndsBanner, sizeof(sNDSBannerExt));
			if (dsiFeatures()) {
				bannerIconPreloaded[fileOffset] = true;
			}
		}

		if (ms().ak_viewMode == TWLSettings::EViewSmallIcon) {
			for (int i = 0; i < 128; i++) {
				if (ndsBanner.titles[currentLang][i] == 0x0A) {
					// ndsBanner.titles[currentLang][i] = 0x2022; // TODO: Add spaces around the centered dot
					ndsBanner.titles[currentLang][i] = '-';
				}
			}
		}

		cachedTitle[num] = (char16_t*)&ndsBanner.titles[currentLang];
		infoFound[num] = true;

		if (customIcon[num] == 1) {
			return;
		}

		// banner sequence
		if (ms().animateDsiIcons && ndsBanner.version == NDS_BANNER_VER_DSi) {
			u16 crc16 = swiCRC16(0xFFFF, ndsBanner.dsi_icon, 0x1180);
			convertIconPalette(&ndsBanner);
			if (ndsBanner.crc[3] == crc16) { // Check if CRC16 is valid
				grabBannerSequence(num);
				bnriconisDSi[num] = true;
			}
		} else {
			convertIconPalette(&ndsBanner);
		}
	}
}

void iconUpdate(int num, bool isDir, const char* name)
{
	logPrint("iconUpdate: ");

	const bool isNds = (bnrRomType[num] == 0);

	if (customIcon[num] > 0 || (customIcon[num] && !isDir && isNds)) {
		sNDSBannerExt &ndsBanner = bnriconTile[num];
		if (customIcon[num] == -1) {
			logPrint(isDir ? "Custom icon invalid!" : "Banner not found or custom icon invalid!");
			loadUnkIcon(num);
		} else if (bnriconisDSi[num]) {
			logPrint("Custom icon found!");
			loadIcon(num, ndsBanner.dsi_icon[0], ndsBanner.dsi_palette[bnriconPalLine[num]], true);
			bnriconPalLoaded[num] = bnriconPalLine[num];
		} else {
			logPrint("Custom icon found!");
			loadIcon(num, ndsBanner.icon, ndsBanner.palette, false);
		}
	} else if (isDir) {
		logPrint("Folder found!");
		loadFolderIcon(num);
	} else if (extension(name, {".argv"})) {
		// look through the argv file for the corresponding nds/app file
		FILE *fp;
		char *line = NULL, *p = NULL;
		size_t size = 0;
		ssize_t rc;

		// open the argv file
		fp = fopen(name, "rb");
		if (fp == NULL) {
			logPrint("Icon not found!\n");
			clearIcon(num);
			fclose(fp);
			return;
		}

		// read each line
		while ((rc = __getline(&line, &size, fp)) > 0) {
			// remove comments
			if ((p = strchr(line, '#')) != NULL)
				*p = 0;

			// skip leading whitespace
			for (p = line; *p && isspace((int)*p); ++p);

			if (*p)
				break;
		}

		// done with the file at this point
		fclose(fp);

		if (p && *p) {
			// we found an argument
			struct stat st;

			// truncate everything after first argument
			strtok(p, "\n\r\t ");

			if (extension(p, {".nds", ".dsi", ".ids", ".srl", ".app"})) {
				// let's see if this is a file or directory
				rc = stat(p, &st);
				if (rc != 0) {
					// stat failed
					logPrint("Icon not found!");
					clearIcon(num);
				} else if (S_ISDIR(st.st_mode)) {
					// this is a directory!
					logPrint("Folder found!");
					loadFolderIcon(num);
				} else {
					iconUpdate(num, false, p);
				}
			} else {
				// this is not an nds/app file!
				clearIcon(num);
			}
		} else {
			clearIcon(num);
		}
		// clean up the allocated line
		free(line);
	} else if (isNds) {
		// this is an nds/app file!
		logPrint("NDS icon found!");
		sNDSBannerExt &ndsBanner = bnriconTile[num];
		if (bnriconisDSi[num]) {
			loadIcon(num, ndsBanner.dsi_icon[0], ndsBanner.dsi_palette[bnriconPalLine[num]], true);
			bnriconPalLoaded[num] = bnriconPalLine[num];
		} else {
			loadIcon(num, ndsBanner.icon, ndsBanner.palette, false);
		}
	} else if (bnrRomType[num] == 10) {
		loadA26Icon(num);
	} else if (bnrRomType[num] == 21) {
		loadMSXIcon(num);
	} else if (bnrRomType[num] == 13) {
		loadCOLIcon(num);
	} else if (bnrRomType[num] == 14) {
		loadM5Icon(num);
	} else if (bnrRomType[num] == 12) {
		loadINTIcon(num);
	} else if (bnrRomType[num] == 9) {
		loadPLGIcon(num);
	} else if (bnrRomType[num] == 19) {
		loadVIDIcon(num);
	} else if (bnrRomType[num] == 20) {
		loadIMGIcon(num);
	} else if (bnrRomType[num] == 1) {
		loadGBAIcon(num);
	} else if (bnrRomType[num] == 2) {
		loadGBIcon(num);
	} else if (bnrRomType[num] == 3) {
		loadGBCIcon(num);
	} else if (bnrRomType[num] == 4) {
		loadNESIcon(num);
	} else if (bnrRomType[num] == 15) {
		loadSGIcon(num);
	} else if (bnrRomType[num] == 5) {
		loadSMSIcon(num);
	} else if (bnrRomType[num] == 6) {
		loadGGIcon(num);
	} else if (bnrRomType[num] == 7) {
		loadMDIcon(num);
	} else if (bnrRomType[num] == 8) {
		loadSNESIcon(num);
	} else if (bnrRomType[num] == 11) {
		loadPCEIcon(num);
	} else if (bnrRomType[num] == 16) {
		loadWSIcon(num);
	} else if (bnrRomType[num] == 17) {
		loadNGPIcon(num);
	} else if (bnrRomType[num] == 18) {
		loadCPCIcon(num);
	} else if (bnrRomType[num] == 22) {
		loadMINIcon(num);
	} else if (bnrRomType[num] == 23) {
		loadHBIcon(num);
	} else {
		loadUnkIcon(num);
	}
	logPrint("\n");
}

void titleUpdate(int num, bool isDir, const char* name, const bool highlighted)
{
	if (isDir && (strcmp(name, "..") == 0)) {
		// text
		writeBannerText(num, "Back", highlighted);
	} else if ((ms().ak_viewMode >= TWLSettings::EViewInternal) && infoFound[num]) {
		// this is an nds/app file!
		// or a file with custom banner text
		writeBannerText(num, cachedTitle[num], highlighted);
	} else if (ms().ak_viewMode == TWLSettings::EViewList || ms().ak_viewMode == TWLSettings::EViewSmallIcon) {
		if ((ms().filenameDisplay == 0) && (ms().ak_viewMode == TWLSettings::EViewSmallIcon) && !infoFound[num]) {
			std::string nameString = name;
			std::string nameSubstr = nameString.substr(0, nameString.rfind('.'));
			writeBannerText(num, nameSubstr, highlighted);
		} else {
			writeBannerText(num, name, highlighted);
		}
	} else {
		std::vector<std::string> lines;
		if ((ms().filenameDisplay == 0) && (ms().ak_viewMode >= TWLSettings::EViewInternal || ms().ak_viewMode == TWLSettings::EViewSmallIcon) && !infoFound[num]) {
			std::string nameString = name;
			std::string nameSubstr = nameString.substr(0, nameString.rfind('.'));
			lines.push_back(nameSubstr);
		} else {
			lines.push_back(name);
		}

		for (uint i = 0; i < lines.size(); i++) {
			int width = calcSmallFontWidth(lines[i]);
			if (width > 214) {
				int mid = lines[i].length() / 2;
				bool foundSpace = false;
				for (uint j = 0; j < lines[i].length() / 2; j++) {
					if (lines[i][mid + j] == ' ') {
						lines.insert(lines.begin() + i, lines[i].substr(0, mid + j));
						lines[i + 1] = lines[i + 1].substr(mid + j + 1);
						i--;
						foundSpace = true;
						break;
					} else if (lines[i][mid - j] == ' ') {
						lines.insert(lines.begin() + i, lines[i].substr(0, mid - j));
						lines[i + 1] = lines[i + 1].substr(mid - j + 1);
						i--;
						foundSpace = true;
						break;
					}
				}
				if (!foundSpace) {
					lines.insert(lines.begin() + i, lines[i].substr(0, mid));
					lines[i + 1] = lines[i + 1].substr(mid);
					i--;
				}
			}
		}

		std::string out;
		for (auto line : lines) {
			out += line + '\n';
		}
		out.pop_back();
		writeBannerText(num, out, highlighted);
	}
}
