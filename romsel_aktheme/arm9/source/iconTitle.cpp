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
#include "common/flashcard.h"
#include "common/systemdetails.h"
#include "common/tonccpy.h"
#include "common/twlmenusettings.h"
#include "fileBrowse.h"
#include "graphics/fontHandler.h"
#include "common/lodepng.h"
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

static int iconTexID[8];
static int folderTexID;
sNDSHeaderExt ndsHeader;
sNDSBannerExt ndsBanner;

static bool infoFound = false;
static char16_t cachedTitle[TITLE_CACHE_SIZE];

static u32 arm9StartSig[4];

static glImage folderIcon[1];
static glImage ndsIcon[8][(32 / 32) * (256 / 32)];

u8 *clearTiles;
u16 *blackPalette;
u8 *tilesModified;

void iconTitleInit()
{
	clearTiles = new u8[(32 * 256) / 2]();
	blackPalette = new u16[16*8]();
	tilesModified = new u8[(32 * 256) / 2];
}

static inline void writeBannerText(std::string_view text)
{
	const int xPos = BOX_PX;
	const int yPos = BOX_PY;
	const FontPalette bannerFontPalette = FontPalette::black;

	printSmall(false, xPos, yPos - (calcSmallFontHeight(text) / 2), text, Alignment::center, bannerFontPalette);
}

static inline void writeBannerText(std::u16string_view text)
{
	const int xPos = BOX_PX;
	const int yPos = BOX_PY;
	const FontPalette bannerFontPalette = FontPalette::black;

	printSmall(false, xPos, yPos - (calcSmallFontHeight(text) / 2), text, Alignment::center, bannerFontPalette);
}

static void convertIconTilesToRaw(u8 *tilesSrc, u8 *tilesNew, bool twl)
{
	int PY = 32;
	if (twl) PY = 32*8;
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

int loadIcon_loopTimes = 1;

void loadIcon(u8 *tilesSrc, u16 *palSrc, bool twl)//(u8(*tilesSrc)[(32 * 32) / 2], u16(*palSrc)[16])
{
	convertIconTilesToRaw(tilesSrc, tilesModified, twl);

	int Ysize = 32;
	int textureSizeY = TEXTURE_SIZE_32;
	loadIcon_loopTimes = 1;
	if (twl) {
		Ysize = 256;
		textureSizeY = TEXTURE_SIZE_256;
		loadIcon_loopTimes = 8;
	}

	for (int i = 0; i < 8; i++) {
		glDeleteTextures(1, &iconTexID[i]);
	}
	for (int i = 0; i < loadIcon_loopTimes; i++) {
		if (colorTable) {
			for (int i2 = 0; i2 < 16; i2++) {
				*(palSrc+i2+(i*16)) = colorTable[*(palSrc+i2+(i*16))];
			}
		}
		iconTexID[i] =
		glLoadTileSet(ndsIcon[i], // pointer to glImage array
					32, // sprite width
					32, // sprite height
					32, // bitmap image width
					Ysize, // bitmap image height
					GL_RGB16, // texture type for glTexImage2D() in videoGL.h
					TEXTURE_SIZE_32, // sizeX for glTexImage2D() in videoGL.h
					textureSizeY, // sizeY for glTexImage2D() in videoGL.h
					GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T | TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT,
					16, // Length of the palette to use (16 colors)
					(u16*) palSrc+(i*16), // Image palette
					(u8*) tilesModified // Raw image data
					);
	}
}

void loadUnkIcon()
{
	for (int i = 0; i < 8; i++) {
		glDeleteTextures(1, &iconTexID[i]);
	}
	iconTexID[0] =
	glLoadTileSet(ndsIcon[0], // pointer to glImage array
				32, // sprite width
				32, // sprite height
				32, // bitmap image width
				32, // bitmap image height
				GL_RGB16, // texture type for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeX for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeY for glTexImage2D() in videoGL.h
				GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T | TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT,
				16, // Length of the palette to use (16 colors)
				(u16*) icon_unkPal, // Image palette
				(u8*) icon_unkBitmap // Raw image data
				);
}

void loadGBAIcon()
{
	for (int i = 0; i < 8; i++) {
		glDeleteTextures(1, &iconTexID[i]);
	}
	if (ms().gbaBooter == TWLSettings::EGbaGbar2) {
		iconTexID[0] = glLoadTileSet(ndsIcon[0], // pointer to glImage array
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
	} else {
		iconTexID[0] = glLoadTileSet(ndsIcon[0], // pointer to glImage array
					32, // sprite width
					32, // sprite height
					32, // bitmap width
					32, // bitmap height
					GL_RGB16, // texture type for glTexImage2D() in videoGL.h
					TEXTURE_SIZE_32, // sizeX for glTexImage2D() in videoGL.h
					TEXTURE_SIZE_32, // sizeY for glTexImage2D() in videoGL.h
					TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
					16, // Length of the palette to use (16 colors)
					(u16*) icon_gbamodePal, // Load our 16 color tiles palette
					(u8*) icon_gbamodeBitmap // image data generated by GRIT
					);
	}
}

void loadGBIcon()
{
	for (int i = 0; i < 8; i++) {
		glDeleteTextures(1, &iconTexID[i]);
	}
	iconTexID[0] = glLoadTileSet(ndsIcon[0], // pointer to glImage array
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

void loadGBCIcon()
{
	for (int i = 0; i < 8; i++) {
		glDeleteTextures(1, &iconTexID[i]);
	}
	iconTexID[0] = glLoadTileSet(ndsIcon[0], // pointer to glImage array
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

void loadNESIcon()
{
	for (int i = 0; i < 8; i++) {
		glDeleteTextures(1, &iconTexID[i]);
	}
	iconTexID[0] = glLoadTileSet(ndsIcon[0], // pointer to glImage array
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

void loadSGIcon()
{
	for (int i = 0; i < 8; i++) {
		glDeleteTextures(1, &iconTexID[i]);
	}
	iconTexID[0] = glLoadTileSet(ndsIcon[0], // pointer to glImage array
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

void loadSMSIcon()
{
	for (int i = 0; i < 8; i++) {
		glDeleteTextures(1, &iconTexID[i]);
	}
	iconTexID[0] = glLoadTileSet(ndsIcon[0], // pointer to glImage array
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

void loadGGIcon()
{
	for (int i = 0; i < 8; i++) {
		glDeleteTextures(1, &iconTexID[i]);
	}
	iconTexID[0] = glLoadTileSet(ndsIcon[0], // pointer to glImage array
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

void loadMDIcon()
{
	for (int i = 0; i < 8; i++) {
		glDeleteTextures(1, &iconTexID[i]);
	}
	iconTexID[0] = glLoadTileSet(ndsIcon[0], // pointer to glImage array
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

void loadSNESIcon()
{
	for (int i = 0; i < 8; i++) {
		glDeleteTextures(1, &iconTexID[i]);
	}
	iconTexID[0] = glLoadTileSet(ndsIcon[0], // pointer to glImage array
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

void loadPLGIcon()
{
	for (int i = 0; i < 8; i++) {
		glDeleteTextures(1, &iconTexID[i]);
	}
	iconTexID[0] = glLoadTileSet(ndsIcon[0], // pointer to glImage array
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

void loadA26Icon()
{
	for (int i = 0; i < 8; i++) {
		glDeleteTextures(1, &iconTexID[i]);
	}
	iconTexID[0] = glLoadTileSet(ndsIcon[0], // pointer to glImage array
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

void loadCOLIcon()
{
	for (int i = 0; i < 8; i++) {
		glDeleteTextures(1, &iconTexID[i]);
	}
	iconTexID[0] = glLoadTileSet(ndsIcon[0], // pointer to glImage array
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

void loadM5Icon()
{
	for (int i = 0; i < 8; i++) {
		glDeleteTextures(1, &iconTexID[i]);
	}
	iconTexID[0] = glLoadTileSet(ndsIcon[0], // pointer to glImage array
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

void loadINTIcon()
{
	for (int i = 0; i < 8; i++) {
		glDeleteTextures(1, &iconTexID[i]);
	}
	iconTexID[0] = glLoadTileSet(ndsIcon[0], // pointer to glImage array
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

void loadPCEIcon()
{
	for (int i = 0; i < 8; i++) {
		glDeleteTextures(1, &iconTexID[i]);
	}
	iconTexID[0] = glLoadTileSet(ndsIcon[0], // pointer to glImage array
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

void loadWSIcon()
{
	for (int i = 0; i < 8; i++) {
		glDeleteTextures(1, &iconTexID[i]);
	}
	iconTexID[0] = glLoadTileSet(ndsIcon[0], // pointer to glImage array
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

void loadNGPIcon()
{
	for (int i = 0; i < 8; i++) {
		glDeleteTextures(1, &iconTexID[i]);
	}
	iconTexID[0] = glLoadTileSet(ndsIcon[0], // pointer to glImage array
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

void loadCPCIcon()
{
	for (int i = 0; i < 8; i++) {
		glDeleteTextures(1, &iconTexID[i]);
	}
	iconTexID[0] = glLoadTileSet(ndsIcon[0], // pointer to glImage array
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

void loadVIDIcon()
{
	for (int i = 0; i < 8; i++) {
		glDeleteTextures(1, &iconTexID[i]);
	}
	iconTexID[0] = glLoadTileSet(ndsIcon[0], // pointer to glImage array
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

void loadIMGIcon()
{
	for (int i = 0; i < 8; i++) {
		glDeleteTextures(1, &iconTexID[i]);
	}
	iconTexID[0] = glLoadTileSet(ndsIcon[0], // pointer to glImage array
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

void loadMSXIcon()
{
	for (int i = 0; i < 8; i++) {
		glDeleteTextures(1, &iconTexID[i]);
	}
	iconTexID[0] = glLoadTileSet(ndsIcon[0], // pointer to glImage array
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

void loadMINIcon()
{
	for (int i = 0; i < 8; i++) {
		glDeleteTextures(1, &iconTexID[i]);
	}
	iconTexID[0] = glLoadTileSet(ndsIcon[0], // pointer to glImage array
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

void loadHBIcon()
{
	for (int i = 0; i < 8; i++) {
		glDeleteTextures(1, &iconTexID[i]);
	}
	iconTexID[0] = glLoadTileSet(ndsIcon[0], // pointer to glImage array
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

void loadConsoleIcons()
{
	u16* newPalette;

	// Folder
	glDeleteTextures(1, &folderTexID);

	newPalette = (u16*)icon_folderPal;
	if (colorTable) {
		for (int i2 = 0; i2 < 16; i2++) {
			*(newPalette+i2) = colorTable[*(newPalette+i2)];
		}
	}
	folderTexID =
	glLoadTileSet(folderIcon, // pointer to glImage array
				32, // sprite width
				32, // sprite height
				32, // bitmap image width
				32, // bitmap image height
				GL_RGB16, // texture type for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeX for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeY for glTexImage2D() in videoGL.h
				TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT,
				16, // Length of the palette to use (16 colors)
				(u16*) newPalette, // Image palette
				(u8*) icon_folderBitmap // Raw image data
				);


	if (!colorTable) {
		return;
	}

	// GBA
	if (ms().gbaBooter == TWLSettings::EGbaGbar2) {
		newPalette = (u16*)icon_gbaPal;
	} else {
		newPalette = (u16*)icon_gbamodePal;
	}
	for (int i2 = 0; i2 < 16; i2++) {
		*(newPalette+i2) = colorTable[*(newPalette+i2)];
	}

	// GB/GBC
	newPalette = (u16*)icon_gbPal;
	for (int i2 = 0; i2 < 16; i2++) {
		*(newPalette+i2) = colorTable[*(newPalette+i2)];
	}

	// NES
	newPalette = (u16*)icon_nesPal;
	for (int i2 = 0; i2 < 16; i2++) {
		*(newPalette+i2) = colorTable[*(newPalette+i2)];
	}

	// SG
	newPalette = (u16*)icon_sgPal;
	for (int i2 = 0; i2 < 16; i2++) {
		*(newPalette+i2) = colorTable[*(newPalette+i2)];
	}

	// SMS
	newPalette = (u16*)icon_smsPal;
	for (int i2 = 0; i2 < 16; i2++) {
		*(newPalette+i2) = colorTable[*(newPalette+i2)];
	}

	// GG
	newPalette = (u16*)icon_ggPal;
	for (int i2 = 0; i2 < 16; i2++) {
		*(newPalette+i2) = colorTable[*(newPalette+i2)];
	}

	// MD
	newPalette = (u16*)icon_ggPal;
	for (int i2 = 0; i2 < 16; i2++) {
		*(newPalette+i2) = colorTable[*(newPalette+i2)];
	}

	// SNES
	newPalette = (u16*)icon_snesPal;
	for (int i2 = 0; i2 < 16; i2++) {
		*(newPalette+i2) = colorTable[*(newPalette+i2)];
	}

	// DSTWO Plugin
	newPalette = (u16*)icon_plgPal;
	for (int i2 = 0; i2 < 16; i2++) {
		*(newPalette+i2) = colorTable[*(newPalette+i2)];
	}

	// A26
	newPalette = (u16*)icon_a26Pal;
	for (int i2 = 0; i2 < 16; i2++) {
		*(newPalette+i2) = colorTable[*(newPalette+i2)];
	}

	// COL
	newPalette = (u16*)icon_colPal;
	for (int i2 = 0; i2 < 16; i2++) {
		*(newPalette+i2) = colorTable[*(newPalette+i2)];
	}

	// M5
	newPalette = (u16*)icon_m5Pal;
	for (int i2 = 0; i2 < 16; i2++) {
		*(newPalette+i2) = colorTable[*(newPalette+i2)];
	}

	// INT
	newPalette = (u16*)icon_intPal;
	for (int i2 = 0; i2 < 16; i2++) {
		*(newPalette+i2) = colorTable[*(newPalette+i2)];
	}

	// PCE
	newPalette = (u16*)icon_pcePal;
	for (int i2 = 0; i2 < 16; i2++) {
		*(newPalette+i2) = colorTable[*(newPalette+i2)];
	}

	// WS
	newPalette = (u16*)icon_wsPal;
	for (int i2 = 0; i2 < 16; i2++) {
		*(newPalette+i2) = colorTable[*(newPalette+i2)];
	}

	// NGP
	newPalette = (u16*)icon_ngpPal;
	for (int i2 = 0; i2 < 16; i2++) {
		*(newPalette+i2) = colorTable[*(newPalette+i2)];
	}

	// CPC
	newPalette = (u16*)icon_cpcPal;
	for (int i2 = 0; i2 < 16; i2++) {
		*(newPalette+i2) = colorTable[*(newPalette+i2)];
	}

	// Video
	newPalette = (u16*)icon_vidPal;
	for (int i2 = 0; i2 < 16; i2++) {
		*(newPalette+i2) = colorTable[*(newPalette+i2)];
	}

	// Image
	newPalette = (u16*)icon_imgPal;
	for (int i2 = 0; i2 < 16; i2++) {
		*(newPalette+i2) = colorTable[*(newPalette+i2)];
	}

	// MSX
	newPalette = (u16*)icon_msxPal;
	for (int i2 = 0; i2 < 16; i2++) {
		*(newPalette+i2) = colorTable[*(newPalette+i2)];
	}

	// MINI
	newPalette = (u16*)icon_miniPal;
	for (int i2 = 0; i2 < 16; i2++) {
		*(newPalette+i2) = colorTable[*(newPalette+i2)];
	}

	// Homebrew
	newPalette = (u16*)icon_hbPal;
	for (int i2 = 0; i2 < 16; i2++) {
		*(newPalette+i2) = colorTable[*(newPalette+i2)];
	}
}

static void clearIcon()
{
	loadIcon(clearTiles, blackPalette, true);
}

void drawIconFolder(int Xpos, int Ypos) { glSprite(Xpos, Ypos, GL_FLIP_NONE, folderIcon); }
void drawIcon(int Xpos, int Ypos) { glSprite(Xpos, Ypos, bannerFlip, &ndsIcon[bnriconPalLine][bnriconframenumY & 31]); }

void getGameInfo(bool isDir, const char* name)
{
	bnriconPalLine = 0;
	bnriconframenumY = 0;
	bannerFlip = GL_FLIP_NONE;
	bnriconisDSi = false;
	bnrWirelessIcon = 0;
	customIcon = 0;
	toncset(gameTid, 0, 4);
	isTwlm = false;
	isDSiWare = false;
	isHomebrew = false;
	isModernHomebrew = false;
	requiresRamDisk = false;
	requiresDonorRom = false;
	infoFound = false;

	if (ms().showCustomIcons) {
		toncset(&ndsBanner, 0, sizeof(sNDSBannerExt));
		bool customIconGood = false;

		// First try banner bin
		snprintf(customIconPath, sizeof(customIconPath), "%s:/_nds/TWiLightMenu/icons/%s.bin", sdFound() ? "sd" : "fat", name);
		customIcon = (access(customIconPath, F_OK) == 0);
		if (customIcon) {
			customIcon = 2; // custom icon is a banner bin
			FILE *file = fopen(customIconPath, "rb");
			if (file) {
				size_t read = fread(&ndsBanner, 1, sizeof(sNDSBannerExt), file);
				fclose(file);

				if (read >= NDS_BANNER_SIZE_ORIGINAL) {
					customIconGood = true;

					if (ms().animateDsiIcons && read == NDS_BANNER_SIZE_DSi) {
						u16 crc16 = swiCRC16(0xFFFF, ndsBanner.dsi_icon, 0x1180);
						if (ndsBanner.crc[3] == crc16) { // Check if CRC16 is valid
							bnriconisDSi = true;
							grabBannerSequence();
						}
					}

					tonccpy(cachedTitle, ndsBanner.titles[ms().getGameLanguage()], TITLE_CACHE_SIZE*sizeof(u16));

					infoFound = true;
				}
			}
		} else {
			// If no banner bin, try png
			snprintf(customIconPath, sizeof(customIconPath), "%s:/_nds/TWiLightMenu/icons/%s.png", sdFound() ? "sd" : "fat", name);
			customIcon = (access(customIconPath, F_OK) == 0);
			if (customIcon) {
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
						ndsBanner.icon[pos] &= nibble? 0x0f : 0xf0;
						// read color
						u8 r, g, b, a;
						r = image[i*4];
						g = image[i*4+1];
						b = image[i*4+2];
						a = image[i*4+3];
						if (a == 255) {
							// convert to 5-bit bgr
							b /= 8;
							g /= 8;
							r /= 8;
							u16 color = 0x8000 | b<<10 | g<<5 | r;
							// find color in palette
							bool found = false;
							for (uint palIdx = 1; palIdx < colorCount; palIdx++) {
								if (ndsBanner.palette[palIdx] == color) {
									ndsBanner.icon[pos] |= nibble? palIdx<<4 : palIdx;
									found = true;
									break;
								}
							}
							// add color to palette if room available
							if (!found && colorCount < 16) {
								ndsBanner.icon[pos] |= nibble? colorCount<<4 : colorCount;
								ndsBanner.palette[colorCount++] = color;
							}
						}
					}
				}
			}
		}

		if (customIcon && !customIconGood)
			customIcon = -1; // display as unknown
	}

	if (isDir) {
		// banner sequence
		clearBannerSequence();
	} else if (extension(name, {".argv"})) {
		// look through the argv file for the corresponding nds file
		FILE *fp;
		char *line = NULL, *p = NULL;
		size_t size = 0;
		ssize_t rc;

		// open the argv file
		fp = fopen(name, "rb");
		if (fp == NULL) {
			clearBannerSequence();
			fclose(fp);
			return;
		}

		// read each line
		while ((rc = __getline(&line, &size, fp)) > 0) {
			// remove comments
			if ((p = strchr(line, '#')) != NULL)
				*p = 0;

			// skip leading whitespace
			for (p = line; *p && isspace((int) *p); ++p)
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
					clearBannerSequence();
				} else if (S_ISDIR(st.st_mode)) {
					// this is a directory!
					clearBannerSequence();
				} else {
					getGameInfo(false, p);
				}
			} else {
				// this is not an nds/app file!
				clearBannerSequence();
			}
		} else {
			clearBannerSequence();
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
		fread(gameTid, 1, 4, fp);

		fclose(fp);
	} else if (extension(name, {".gba", ".agb", ".mb"})) {
		// this is a gba file!
		FILE *fp;

		// open file for reading info
		fp = fopen(name, "rb");
		if (!fp) {
			fclose(fp);
			return;
		}

		fseek(fp, 0xAC, SEEK_SET);
		fread(gameTid, 1, 4, fp);

		fclose(fp);
	} else if (extension(name, {".nds", ".dsi", ".ids", ".srl", ".app"})) {
		// this is an nds/app file!
		FILE *fp;
		int ret;

		// open file for reading info
		fp = fopen(name, "rb");
		if (fp == NULL) {
			// banner sequence
			clearBannerSequence();
			fclose(fp);
			return;
		}

		
		ret = fseek(fp, 0, SEEK_SET);
		if (ret == 0)
			ret = fread(&ndsHeader, sizeof(ndsHeader), 1, fp); // read if seek succeed
		else
			ret = 0; // if seek fails set to !=1

		if (ret != 1) {
			// try again, but using regular header size
			ret = fseek(fp, 0, SEEK_SET);
			if (ret == 0)
				ret = fread(&ndsHeader, 0x160, 1, fp); // read if seek succeed
			else
				ret = 0; // if seek fails set to !=1

			if (ret != 1) {
				clearBannerSequence();
				fclose(fp);
				return;
			}
		}

		bool dsiEnhancedMbk = (isDSiMode() && *(u32*)0x02FFE1A0 == 0x00403000 && sys().arm7SCFGLocked());

		tonccpy(gameTid, ndsHeader.gameCode, 4);
		isTwlm = (strcmp(gameTid, "SRLA") == 0);
		romVersion = ndsHeader.romversion;
		romUnitCode = ndsHeader.unitCode;
		a7mbk6 = ndsHeader.a7mbk6;

		fseek(fp, ndsHeader.arm9romOffset + ndsHeader.arm9executeAddress - ndsHeader.arm9destination, SEEK_SET);
		fread(arm9StartSig, sizeof(u32), 4, fp);
		if (arm9StartSig[0] == 0xE3A00301
		 && arm9StartSig[1] == 0xE5800208
		 && arm9StartSig[2] == 0xE3A00013
		 && arm9StartSig[3] == 0xE129F000) {
			isHomebrew = true;
			isModernHomebrew = true; // Homebrew is recent (supports reading from SD without a DLDI driver)
			if (ndsHeader.arm7executeAddress >= 0x037F0000 && ndsHeader.arm7destination >= 0x037F0000) {
				if ((ndsHeader.arm9binarySize == 0xC9F68 && ndsHeader.arm7binarySize == 0x12814)	// Colors! v1.1
				|| (ndsHeader.arm9binarySize == 0x1B0864 && ndsHeader.arm7binarySize == 0xDB50)	// Mario Paint Composer DS v2 (Bullet Bill)
				|| (ndsHeader.arm9binarySize == 0xE78FC && ndsHeader.arm7binarySize == 0xF068)		// SnowBros v2.2
				|| (ndsHeader.arm9binarySize == 0xD45C0 && ndsHeader.arm7binarySize == 0x2B7C)		// ikuReader v0.058
				|| (ndsHeader.arm9binarySize == 0x7A124 && ndsHeader.arm7binarySize == 0xEED0)		// PPSEDS r11
				|| (ndsHeader.arm9binarySize == 0x54620 && ndsHeader.arm7binarySize == 0x1538)		// XRoar 0.24fp3
				|| (ndsHeader.arm9binarySize == 0x2C9A8 && ndsHeader.arm7binarySize == 0xFB98)		// NitroGrafx v0.7
				|| (ndsHeader.arm9binarySize == 0x22AE4 && ndsHeader.arm7binarySize == 0xA764)) {	// It's 1975 and this man is about to show you the future
					isModernHomebrew = false; // Have nds-bootstrap load it (in case if it doesn't)
				}
			}
		} else if ((memcmp(ndsHeader.gameTitle, "NDS.TinyFB", 10) == 0)
				 || (memcmp(ndsHeader.gameTitle, "MAGIC FLOOR", 11) == 0)
				 || (memcmp(ndsHeader.gameTitle, "UNLAUNCH.DSI", 12) == 0)) {
			isHomebrew = true;
			isModernHomebrew = true; // No need to use nds-bootstrap
		} else if ((memcmp(ndsHeader.gameTitle, "NMP4BOOT", 8) == 0)
		 || (ndsHeader.arm7executeAddress >= 0x037F0000 && ndsHeader.arm7destination >= 0x037F0000)) {
			isHomebrew = true; // Homebrew is old (requires a DLDI driver to read from SD)
		} else if (ndsHeader.unitCode != 0 && (ndsHeader.accessControl & BIT(4))) {
			isDSiWare = true; // Is a DSiWare game
		}

		if (isHomebrew && !ms().secondaryDevice) {
			if ((ndsHeader.arm9binarySize == 0x98F70 && ndsHeader.arm7binarySize == 0xED94)		// jEnesisDS 0.7.4
			|| (ndsHeader.arm9binarySize == 0x48950 && ndsHeader.arm7binarySize == 0x74C4)			// SNEmulDS06-WIP2
			|| (ndsHeader.arm9binarySize == 0xD45C0 && ndsHeader.arm7binarySize == 0x2B7C)			// ikuReader v0.058
			|| (ndsHeader.arm9binarySize == 0x54620 && ndsHeader.arm7binarySize == 0x1538)) {		// XRoar 0.24fp3
				requiresRamDisk = true;
			}
		}

		if (!isHomebrew) {
			// Check if ROM needs a donor ROM
			if (isDSiMode() && (a7mbk6 == (dsiEnhancedMbk ? 0x080037C0 : 0x00403000) || (ndsHeader.gameCode[0] == 'H' && ndsHeader.arm7binarySize < 0xC000 && ndsHeader.arm7idestination == 0x02E80000 && (REG_MBK9 & 0x00FFFFFF) != 0x00FFFF0F)) && sys().arm7SCFGLocked()) {
				requiresDonorRom = dsiEnhancedMbk ? 51 : 52; // DSi-Enhanced ROM required on CycloDSi, or DSi-Exclusive/DSiWare ROM required on DSiWarehax
				if (ndsHeader.gameCode[0] == 'H' && ndsHeader.arm7binarySize < 0xC000 && ndsHeader.arm7idestination == 0x02E80000) {
					requiresDonorRom += 100;
				} else if (memcmp(ndsHeader.gameCode, "KCX", 3) == 0 && dsiEnhancedMbk) {
					requiresDonorRom = 0;
				}
			} else if (a7mbk6 == 0x080037C0 && ms().secondaryDevice && (!dsiFeatures() || bs().b4dsMode)
			&& (((sys().dsDebugRam() || (dsiFeatures() && bs().b4dsMode == 2)) ? (memcmp(ndsHeader.gameCode, "DME", 3) == 0 || memcmp(ndsHeader.gameCode, "DMD", 3) == 0 || memcmp(ndsHeader.gameCode, "DMP", 3) == 0 || memcmp(ndsHeader.gameCode, "DHS", 3) == 0) : (memcmp(ndsHeader.gameCode, "DMP", 3) == 0 || memcmp(ndsHeader.gameCode, "DHS", 3) == 0))
			|| (ndsHeader.gameCode[0] != 'D' && memcmp(ndsHeader.gameCode, "KCX", 3) != 0 && memcmp(ndsHeader.gameCode, "KAV", 3) != 0 && memcmp(ndsHeader.gameCode, "KNK", 3) != 0 && memcmp(ndsHeader.gameCode, "KE3", 3) != 0))) {
				requiresDonorRom = 51; // SDK5 ROM required
			} else if (memcmp(ndsHeader.gameCode, "AYI", 3) == 0 && ndsHeader.arm7binarySize == 0x25F70) {
				requiresDonorRom = 20; // SDK2.0 ROM required for Yoshi Touch & Go (Europe)
			}
		}

		if (ndsHeader.dsi_flags & BIT(4))
			bnrWirelessIcon = 1;
		else if (ndsHeader.dsi_flags & BIT(3))
			bnrWirelessIcon = 2;

		if (customIcon == 2) { // custom banner bin
			// we're done early, close the file
			fclose(fp);
			return;
		}

		u8 iconCopy[512];
		u16 paletteCopy[16];
		if (customIcon == 1) { // custom png icon
			// copy the icon and palette before they get overwritten
			memcpy(iconCopy, ndsBanner.icon, sizeof(iconCopy));
			memcpy(paletteCopy, ndsBanner.palette, sizeof(paletteCopy));
		}

		if (ndsHeader.bannerOffset == 0) {
			fclose(fp);

			FILE* bannerFile = fopen("nitro:/noinfo.bnr", "rb");
			fread(&ndsBanner, 1, NDS_BANNER_SIZE_ZH_KO, bannerFile);
			fclose(bannerFile);

			tonccpy(cachedTitle, ndsBanner.titles[ms().getGameLanguage()], TITLE_CACHE_SIZE*sizeof(u16));

			// restore png icon
			if (customIcon == 1) {
				memcpy(ndsBanner.icon, iconCopy, sizeof(iconCopy));
				memcpy(ndsBanner.palette, paletteCopy, sizeof(paletteCopy));
			}

			return;
		}
		ret = fseek(fp, ndsHeader.bannerOffset, SEEK_SET);
		if (ret == 0)
			ret = fread(&ndsBanner, sizeof (ndsBanner), 1, fp); // read if seek succeed
		else
			ret = 0; // if seek fails set to !=1

		if (ret != 1) {
			fclose(fp);

			FILE* bannerFile = fopen("nitro:/noinfo.bnr", "rb");
			fread(&ndsBanner, 1, NDS_BANNER_SIZE_ZH_KO, bannerFile);
			fclose(bannerFile);

			tonccpy(cachedTitle, ndsBanner.titles[ms().getGameLanguage()], TITLE_CACHE_SIZE*sizeof(u16));

			// restore png icon
			if (customIcon == 1) {
				memcpy(ndsBanner.icon, iconCopy, sizeof(iconCopy));
				memcpy(ndsBanner.palette, paletteCopy, sizeof(paletteCopy));
			}

			return;
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
		tonccpy(cachedTitle, ndsBanner.titles[currentLang], TITLE_CACHE_SIZE*sizeof(u16));
		infoFound = true;

		// restore png icon
		if (customIcon == 1) {
			memcpy(ndsBanner.icon, iconCopy, sizeof(iconCopy));
			memcpy(ndsBanner.palette, paletteCopy, sizeof(paletteCopy));
			return;
		}
		// banner sequence
		if (ms().animateDsiIcons && ndsBanner.version == NDS_BANNER_VER_DSi) {
			u16 crc16 = swiCRC16(0xFFFF, ndsBanner.dsi_icon, 0x1180);
			if (ndsBanner.crc[3] == crc16) { // Check if CRC16 is valid
				grabBannerSequence();
				bnriconisDSi = true;
			}
		}
	}
}

void iconUpdate(bool isDir, const char* name)
{
	clearText(false);

	const bool isNds = (bnrRomType == 0);

	if (isDir) {
		// icon
		clearIcon();
	} else if (customIcon > 0 || (customIcon && isNds)) {
		if (customIcon == -1) {
			loadUnkIcon();
		} else if (bnriconisDSi) {
			loadIcon(ndsBanner.dsi_icon[0], ndsBanner.dsi_palette[0], true);
		} else {
			loadIcon(ndsBanner.icon, ndsBanner.palette, false);
		}
	} else if (extension(name, {".argv"})) {
		// look through the argv file for the corresponding nds/app file
		FILE *fp;
		char *line = NULL, *p = NULL;
		size_t size = 0;
		ssize_t rc;

		// open the argv file
		fp = fopen(name, "rb");
		if (fp == NULL) {
			clearIcon();
			fclose(fp);
			return;
		}

		// read each line
		while ((rc = __getline(&line, &size, fp)) > 0) {
			// remove comments
			if ((p = strchr(line, '#')) != NULL)
				*p = 0;

			// skip leading whitespace
			for (p = line; *p && isspace((int) *p); ++p)
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
					clearIcon();
				} else if (S_ISDIR(st.st_mode)) {
					// this is a directory!
					clearIcon();
				} else {
					iconUpdate(false, p);
				}
			} else {
				// this is not an nds/app file!
				clearIcon();
			}
		} else {
			clearIcon();
		}
		// clean up the allocated line
		free(line);
	} else if (isNds) {
		// this is an nds/app file!
		FILE *fp;
		unsigned int iconTitleOffset;
		int ret;

		// open file for reading info
		fp = fopen(name, "rb");
		if (fp == NULL) {
			// icon
			clearIcon();
			fclose(fp);
			return;
		}

		
		ret = fseek(fp, offsetof(tNDSHeader, bannerOffset), SEEK_SET);
		if (ret == 0)
			ret = fread(&iconTitleOffset, sizeof (int), 1, fp); // read if seek succeed
		else
			ret = 0; // if seek fails set to !=1

		if (ret != 1) {
			// icon
			loadUnkIcon();
			fclose(fp);
			return;
		}

		if (iconTitleOffset == 0) {
			// icon
			loadUnkIcon();
			fclose(fp);
			return;
		}
		ret = fseek(fp, iconTitleOffset, SEEK_SET);
		if (ret == 0)
			ret = fread(&ndsBanner, sizeof (ndsBanner), 1, fp); // read if seek succeed
		else
			ret = 0; // if seek fails set to !=1

		if (ret != 1) {
			// try again, but using regular banner size
			ret = fseek(fp, iconTitleOffset, SEEK_SET);
			if (ret == 0)
				ret = fread(&ndsBanner, NDS_BANNER_SIZE_ORIGINAL, 1, fp); // read if seek succeed
			else
				ret = 0; // if seek fails set to !=1

			if (ret != 1) {
				// icon
				loadUnkIcon();
				fclose(fp);
				return;
			}
		}

		// close file!
		fclose(fp);

		// icon
		if (bnriconisDSi) {
			loadIcon(ndsBanner.dsi_icon[0], ndsBanner.dsi_palette[0], true);
		} else {
			loadIcon(ndsBanner.icon, ndsBanner.palette, false);
		}
	} else if (bnrRomType == 10) {
		loadA26Icon();
	} else if (bnrRomType == 21) {
		loadMSXIcon();
	} else if (bnrRomType == 13) {
		loadCOLIcon();
	} else if (bnrRomType == 14) {
		loadM5Icon();
	} else if (bnrRomType == 12) {
		loadINTIcon();
	} else if (bnrRomType == 9) {
		loadPLGIcon();
	} else if (bnrRomType == 19) {
		loadVIDIcon();
	} else if (bnrRomType == 20) {
		loadIMGIcon();
	} else if (bnrRomType == 1) {
		loadGBAIcon();
	} else if (bnrRomType == 2) {
		loadGBIcon();
	} else if (bnrRomType == 3) {
		loadGBCIcon();
	} else if (bnrRomType == 4) {
		loadNESIcon();
	} else if (bnrRomType == 15) {
		loadSGIcon();
	} else if (bnrRomType == 5) {
		loadSMSIcon();
	} else if (bnrRomType == 6) {
		loadGGIcon();
	} else if (bnrRomType == 7) {
		loadMDIcon();
	} else if (bnrRomType == 8) {
		loadSNESIcon();
	} else if (bnrRomType == 11) {
		loadPCEIcon();
	} else if (bnrRomType == 16) {
		loadWSIcon();
	} else if (bnrRomType == 17) {
		loadNGPIcon();
	} else if (bnrRomType == 18) {
		loadCPCIcon();
	} else if (bnrRomType == 22) {
		loadMINIcon();
	} else if (bnrRomType == 23) {
		loadHBIcon();
	} else {
		loadUnkIcon();
	}
}

void titleUpdate(bool isDir, const char* name)
{
	clearText(false);

	if (isDir) {
		// text
		if (strcmp(name, "..") == 0) {
			writeBannerText("Back");
		} else {
			writeBannerText(name);
		}
	} else if (extension(name, {".nds", ".dsi", ".ids", ".srl", ".app"}) || infoFound) {
		// this is an nds/app file!
		// or a file with custom banner text
		if (infoFound) {
			writeBannerText(cachedTitle);
		} else {
			writeBannerText(name);
		}
	} else {
		std::vector<std::string> lines;
		lines.push_back(name);

		for (uint i = 0; i < lines.size(); i++) {
			int width = calcSmallFontWidth(lines[i]);
			if (width > 140) {
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
		writeBannerText(out);
	}
}
