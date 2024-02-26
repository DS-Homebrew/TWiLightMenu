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
#include <stdio.h>
#include <ctype.h>
#include <sys/stat.h>
#include <gl2d.h>
#include "common/tonccpy.h"
#include "common/twlmenusettings.h"
#include "common/flashcard.h"
#include "graphics/graphics.h"
#include "graphics/fontHandler.h"
#include "common/lodepng.h"
#include "ndsheaderbanner.h"
#include "myDSiMode.h"
#include "language.h"
#include "read_card.h"

#include "iconTitle.h"

#define ICON_POS_X	112
#define ICON_POS_Y	96

// Graphic files
#include "icon_unk.h"
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

extern bool extension(const std::string& filename, const char* ext);

extern u16* colorTable;

static int iconTexID[2][8];
sNDSHeaderExt ndsHeader;
sNDSBannerExt ndsBanner;

#define TITLE_CACHE_SIZE 0x80

static bool infoFound[2] = {false};
static char16_t cachedTitle[2][TITLE_CACHE_SIZE];

static u32 arm9StartSig[4];

static glImage ndsIcon[2][8][(32 / 32) * (256 / 32)];

u8 *clearTiles;
u16 *blackPalette;
u8 *tilesModified;

void iconTitleInit()
{
	clearTiles = new u8[(32 * 256) / 2]();
	blackPalette = new u16[16*8]();
	tilesModified = new u8[(32 * 256) / 2];
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

void loadIcon(int num, u8 *tilesSrc, u16 *palSrc, bool twl)//(u8(*tilesSrc)[(32 * 32) / 2], u16(*palSrc)[16])
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
		glDeleteTextures(1, &iconTexID[num][i]);
	}
	for (int i = 0; i < loadIcon_loopTimes; i++) {
		if (colorTable) {
			for (int i2 = 0; i2 < 16; i2++) {
				*(palSrc+i2+(i*16)) = colorTable[*(palSrc+i2+(i*16))];
			}
		}
		iconTexID[num][i] =
		glLoadTileSet(ndsIcon[num][i], // pointer to glImage array
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

void loadUnkIcon(int num)
{
	for (int i = 0; i < 8; i++) {
		glDeleteTextures(1, &iconTexID[num][i]);
	}
	iconTexID[num][0] =
	glLoadTileSet(ndsIcon[num][0], // pointer to glImage array
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
	for (int i = 0; i < 8; i++) {
		glDeleteTextures(1, &iconTexID[num][i]);
	}
	iconTexID[num][0] =
	glLoadTileSet(ndsIcon[num][0], // pointer to glImage array
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
	for (int i = 0; i < 8; i++) {
		glDeleteTextures(1, &iconTexID[num][i]);
	}
	iconTexID[num][0] =
	glLoadTileSet(ndsIcon[num][0], // pointer to glImage array
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
	for (int i = 0; i < 8; i++) {
		glDeleteTextures(1, &iconTexID[num][i]);
	}
	iconTexID[num][0] =
	glLoadTileSet(ndsIcon[num][0], // pointer to glImage array
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
	for (int i = 0; i < 8; i++) {
		glDeleteTextures(1, &iconTexID[num][i]);
	}
	iconTexID[num][0] =
	glLoadTileSet(ndsIcon[num][0], // pointer to glImage array
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
	for (int i = 0; i < 8; i++) {
		glDeleteTextures(1, &iconTexID[num][i]);
	}
	iconTexID[num][0] =
	glLoadTileSet(ndsIcon[num][0], // pointer to glImage array
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
	for (int i = 0; i < 8; i++) {
		glDeleteTextures(1, &iconTexID[num][i]);
	}
	iconTexID[num][0] =
	glLoadTileSet(ndsIcon[num][0], // pointer to glImage array
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
	for (int i = 0; i < 8; i++) {
		glDeleteTextures(1, &iconTexID[num][i]);
	}
	iconTexID[num][0] =
	glLoadTileSet(ndsIcon[num][0], // pointer to glImage array
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
	for (int i = 0; i < 8; i++) {
		glDeleteTextures(1, &iconTexID[num][i]);
	}
	iconTexID[num][0] =
	glLoadTileSet(ndsIcon[num][0], // pointer to glImage array
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
	for (int i = 0; i < 8; i++) {
		glDeleteTextures(1, &iconTexID[num][i]);
	}
	iconTexID[num][0] =
	glLoadTileSet(ndsIcon[num][0], // pointer to glImage array
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
	for (int i = 0; i < 8; i++) {
		glDeleteTextures(1, &iconTexID[num][i]);
	}
	iconTexID[num][0] =
	glLoadTileSet(ndsIcon[num][0], // pointer to glImage array
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
	for (int i = 0; i < 8; i++) {
		glDeleteTextures(1, &iconTexID[num][i]);
	}
	iconTexID[num][0] =
	glLoadTileSet(ndsIcon[num][0], // pointer to glImage array
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
	for (int i = 0; i < 8; i++) {
		glDeleteTextures(1, &iconTexID[num][i]);
	}
	iconTexID[num][0] =
	glLoadTileSet(ndsIcon[num][0], // pointer to glImage array
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
	for (int i = 0; i < 8; i++) {
		glDeleteTextures(1, &iconTexID[num][i]);
	}
	iconTexID[num][0] =
	glLoadTileSet(ndsIcon[num][0], // pointer to glImage array
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
	for (int i = 0; i < 8; i++) {
		glDeleteTextures(1, &iconTexID[num][i]);
	}
	iconTexID[num][0] =
	glLoadTileSet(ndsIcon[num][0], // pointer to glImage array
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
	for (int i = 0; i < 8; i++) {
		glDeleteTextures(1, &iconTexID[num][i]);
	}
	iconTexID[num][0] =
	glLoadTileSet(ndsIcon[num][0], // pointer to glImage array
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
	for (int i = 0; i < 8; i++) {
		glDeleteTextures(1, &iconTexID[num][i]);
	}
	iconTexID[num][0] =
	glLoadTileSet(ndsIcon[num][0], // pointer to glImage array
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
	for (int i = 0; i < 8; i++) {
		glDeleteTextures(1, &iconTexID[num][i]);
	}
	iconTexID[num][0] =
	glLoadTileSet(ndsIcon[num][0], // pointer to glImage array
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
	for (int i = 0; i < 8; i++) {
		glDeleteTextures(1, &iconTexID[num][i]);
	}
	iconTexID[num][0] =
	glLoadTileSet(ndsIcon[num][0], // pointer to glImage array
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
	for (int i = 0; i < 8; i++) {
		glDeleteTextures(1, &iconTexID[num][i]);
	}
	iconTexID[num][0] =
	glLoadTileSet(ndsIcon[num][0], // pointer to glImage array
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
	for (int i = 0; i < 8; i++) {
		glDeleteTextures(1, &iconTexID[num][i]);
	}
	iconTexID[num][0] =
	glLoadTileSet(ndsIcon[num][0], // pointer to glImage array
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
	for (int i = 0; i < 8; i++) {
		glDeleteTextures(1, &iconTexID[num][i]);
	}
	iconTexID[num][0] =
	glLoadTileSet(ndsIcon[num][0], // pointer to glImage array
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
	for (int i = 0; i < 8; i++) {
		glDeleteTextures(1, &iconTexID[num][i]);
	}
	iconTexID[num][0] =
	glLoadTileSet(ndsIcon[num][0], // pointer to glImage array
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

void loadConsoleIcons()
{
	if (!colorTable) {
		return;
	}

	u16* newPalette;

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
}

static void clearIcon(int num)
{
	loadIcon(num, clearTiles, blackPalette, true);
}

void drawIcon(int num, int Xpos, int Ypos) { glSprite(Xpos, Ypos, bannerFlip[num], &ndsIcon[num][bnriconPalLine[num]][bnriconframenumY[num] & 31]); }

void loadFixedBanner(bool isSlot1) {
	if (isSlot1 && memcmp(ndsHeader.gameCode, "ALXX", 4) == 0) {
		u16 alxxBannerCrc = 0;
		cardRead(0x75600, &arm9StartSig, 0x10);
		cardRead(0x174602, &alxxBannerCrc, sizeof(u16));
		if ((arm9StartSig[0] == 0xE58D0008
		 && arm9StartSig[1] == 0xE1500005
		 && arm9StartSig[2] == 0xBAFFFFC5
		 && arm9StartSig[3] == 0xE59D100C)
		 || alxxBannerCrc != 0xBA52) {
			// It's a SuperCard DSTWO, so use correct banner.
			//cardRead(0x1843400, &ndsBanner, NDS_BANNER_SIZE_ORIGINAL);
			FILE *fixedBannerFile = fopen("nitro:/fixedbanners/SuperCard DSTWO.bnr", "rb");
			fread(&ndsBanner, 1, NDS_BANNER_SIZE_ORIGINAL, fixedBannerFile);
			fclose(fixedBannerFile);
		}
	}
}

void getGameInfo(int num, bool isDir, const char* name)
{
	bnriconPalLine[num] = 0;
	bnriconframenumY[num] = 0;
	bannerFlip[num] = GL_FLIP_NONE;
	bnriconisDSi[num] = false;
	bnrWirelessIcon[num] = 0;
	customIcon[num] = 0;
	isDSiWare[num] = false;
	isHomebrew[num] = false;
	isModernHomebrew[num] = false;
	infoFound[num] = false;

	if (ms().showCustomIcons) {
		toncset(&ndsBanner, 0, sizeof(sNDSBannerExt));
		bool customIconGood = false;

		// First try banner bin
		snprintf(customIconPath, sizeof(customIconPath), "%s:/_nds/TWiLightMenu/icons/%s.bin", sdFound() ? "sd" : "fat", name);
		customIcon[num] = (access(customIconPath, F_OK) == 0);
		if (customIcon[num]) {
			customIcon[num] = 2; // custom icon is a banner bin
			FILE *file = fopen(customIconPath, "rb");
			if (file) {
				size_t read = fread(&ndsBanner, 1, sizeof(sNDSBannerExt), file);
				fclose(file);

				if (read >= NDS_BANNER_SIZE_ORIGINAL) {
					customIconGood = true;

					if (ms().animateDsiIcons && read == NDS_BANNER_SIZE_DSi) {
						u16 crc16 = swiCRC16(0xFFFF, ndsBanner.dsi_icon, 0x1180);
						if (ndsBanner.crc[3] == crc16) { // Check if CRC16 is valid
							bnriconisDSi[num] = true;
							grabBannerSequence(num);
						}
					}

					tonccpy(cachedTitle[num], ndsBanner.titles[ms().getGameLanguage()], TITLE_CACHE_SIZE*sizeof(u16));
					
					infoFound[num] = true;
				}
			}
		} else {
			// If no banner bin, try png
			snprintf(customIconPath, sizeof(customIconPath), "%s:/_nds/TWiLightMenu/icons/%s.png", sdFound() ? "sd" : "fat", name);
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

		if (customIcon[num] && !customIconGood)
			customIcon[num] = -1; // display as unknown
	}

	if (extension(name, ".argv")) {
		// look through the argv file for the corresponding nds file
		FILE *fp;
		char *line = NULL, *p = NULL;
		size_t size = 0;
		ssize_t rc;

		// open the argv file
		fp = fopen(name, "rb");
		if (fp == NULL) {
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

			if (extension(p, ".nds")
			 || extension(p, ".dsi")
			 || extension(p, ".ids")
			 || extension(p, ".srl")
			 || extension(p, ".app")) {
				// let's see if this is a file or directory
				rc = stat(p, &st);
				if (rc != 0) {
					// stat failed
					clearBannerSequence(num);
				} else if (S_ISDIR(st.st_mode)) {
					// this is a directory!
					clearBannerSequence(num);
				} else {
					getGameInfo(num, false, p);
				}
			} else {
				// this is not an nds/app file!
				clearBannerSequence(num);
			}
		} else {
			clearBannerSequence(num);
		}
		// clean up the allocated line
		free(line);
	} else if (strcmp(name, "slot1")==0
			 || extension(name, ".nds")
			 || extension(name, ".dsi")
			 || extension(name, ".ids")
			 || extension(name, ".srl")
			 || extension(name, ".app")) {
		// this is an nds/app file!
		FILE *fp = NULL;
		int ret;
		bool isSlot1 = (strcmp(name, "slot1") == 0);

		if (isSlot1) {
			cardRead(0, &ndsHeader, sizeof(ndsHeader));
		} else {
			// open file for reading info
			fp = fopen(name, "rb");
			if (fp == NULL) {
				// banner sequence
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
					fclose(fp);
					return;
				}
			}

			tonccpy(gameTid[num], ndsHeader.gameCode, 4);
			romVersion[num] = ndsHeader.romversion;
			unitCode[num] = ndsHeader.unitCode;
			headerCRC[num] = ndsHeader.headerCRC16;

			fseek(fp, ndsHeader.arm9romOffset + ndsHeader.arm9executeAddress - ndsHeader.arm9destination, SEEK_SET);
			fread(arm9StartSig, sizeof(u32), 4, fp);
			if (arm9StartSig[0] == 0xE3A00301
			 && arm9StartSig[1] == 0xE5800208
			 && arm9StartSig[2] == 0xE3A00013
			 && arm9StartSig[3] == 0xE129F000) {
				isHomebrew[num] = true;
				isModernHomebrew[num] = true; // Homebrew is recent (supports reading from SD without a DLDI driver)
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
			} else if ((memcmp(ndsHeader.gameTitle, "NDS.TinyFB", 10) == 0)
					 || (memcmp(ndsHeader.gameTitle, "MAGIC FLOOR", 11) == 0)
					 || (memcmp(ndsHeader.gameTitle, "UNLAUNCH.DSI", 12) == 0)) {
				isHomebrew[num] = true;
				isModernHomebrew[num] = true; // No need to use nds-bootstrap
			} else if ((memcmp(ndsHeader.gameTitle, "NMP4BOOT", 8) == 0)
			 || (ndsHeader.arm7executeAddress >= 0x037F0000 && ndsHeader.arm7destination >= 0x037F0000)) {
				isHomebrew[num] = true; // Homebrew is old (requires a DLDI driver to read from SD)
			} else if (ndsHeader.unitCode != 0 && (ndsHeader.accessControl & BIT(4))) {
				isDSiWare[num] = true; // Is a DSiWare game
			}
		}

		if (ndsHeader.dsi_flags & BIT(4))
			bnrWirelessIcon[num] = 1;
		else if (ndsHeader.dsi_flags & BIT(3))
			bnrWirelessIcon[num] = 2;

		if (customIcon[num] == 2) { // custom banner bin
			// we're done early
			if (!isSlot1)
				fclose(fp);
			return;
		}
		
		u8 iconCopy[512];
		u16 paletteCopy[16];
		if (customIcon[num] == 1) { // custom png icon
			// copy the icon and palette before they get overwritten
			memcpy(iconCopy, ndsBanner.icon, sizeof(iconCopy));
			memcpy(paletteCopy, ndsBanner.palette, sizeof(paletteCopy));
		}

		if (ndsHeader.bannerOffset == 0) {
			if (!isSlot1)
				fclose(fp);

			FILE* bannerFile = fopen("nitro:/noinfo.bnr", "rb");
			fread(&ndsBanner, 1, NDS_BANNER_SIZE_ZH_KO, bannerFile);
			fclose(bannerFile);

			tonccpy(cachedTitle[num], ndsBanner.titles[ms().getGameLanguage()], TITLE_CACHE_SIZE*sizeof(u16));

			// restore png icon
			if (customIcon[num] == 1) {
				memcpy(ndsBanner.icon, iconCopy, sizeof(iconCopy));
				memcpy(ndsBanner.palette, paletteCopy, sizeof(paletteCopy));
			}

			return;
		}
		if (isSlot1) {
			if ((ndsCardHeader.bannerOffset > 0) && cardInited) {
				cardRead(ndsCardHeader.bannerOffset, &ndsBanner, NDS_BANNER_SIZE_DSi);
			} else {
				FILE* bannerFile = fopen("nitro:/noinfo.bnr", "rb");
				fread(&ndsBanner, 1, NDS_BANNER_SIZE_ZH_KO, bannerFile);
				fclose(bannerFile);

				tonccpy(cachedTitle[num], ndsBanner.titles[ms().getGameLanguage()], TITLE_CACHE_SIZE*sizeof(u16));

				// restore png icon
				if (customIcon[num] == 1) {
					memcpy(ndsBanner.icon, iconCopy, sizeof(iconCopy));
					memcpy(ndsBanner.palette, paletteCopy, sizeof(paletteCopy));
				}

				return;
			}
		} else {
			ret = fseek(fp, ndsHeader.bannerOffset, SEEK_SET);
			if (ret == 0)
				ret = fread(&ndsBanner, NDS_BANNER_SIZE_DSi, 1, fp); // read if seek succeed
			else
				ret = 0; // if seek fails set to !=1

			if (ret != 1) {
				// try again, but using regular banner size
				ret = fseek(fp, ndsHeader.bannerOffset, SEEK_SET);
				if (ret == 0)
					ret = fread(&ndsBanner, NDS_BANNER_SIZE_ORIGINAL, 1, fp); // read if seek succeed
				else
					ret = 0; // if seek fails set to !=1

				if (ret != 1) {
					fclose(fp);

					FILE* bannerFile = fopen("nitro:/noinfo.bnr", "rb");
					fread(&ndsBanner, 1, NDS_BANNER_SIZE_ZH_KO, bannerFile);
					fclose(bannerFile);

					tonccpy(cachedTitle[num], ndsBanner.titles[ms().getGameLanguage()], TITLE_CACHE_SIZE*sizeof(u16));

					// restore png icon
					if (customIcon[num] == 1) {
						memcpy(ndsBanner.icon, iconCopy, sizeof(iconCopy));
						memcpy(ndsBanner.palette, paletteCopy, sizeof(paletteCopy));
					}

					return;
				}
			}

			// close file!
			fclose(fp);
		}

		loadFixedBanner(isSlot1);

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
		tonccpy(cachedTitle[num], ndsBanner.titles[currentLang], TITLE_CACHE_SIZE*sizeof(u16));
		infoFound[num] = true;

		// restore png icon
		if (customIcon[num] == 1) {
			memcpy(ndsBanner.icon, iconCopy, sizeof(iconCopy));
			memcpy(ndsBanner.palette, paletteCopy, sizeof(paletteCopy));
			return;
		}
		// banner sequence
		if (ms().animateDsiIcons && ndsBanner.version == NDS_BANNER_VER_DSi) {
			u16 crc16 = swiCRC16(0xFFFF, ndsBanner.dsi_icon, 0x1180);
			if (ndsBanner.crc[3] == crc16) { // Check if CRC16 is valid
				grabBannerSequence(num);
				bnriconisDSi[num] = true;
			}
		}
	}
}

void iconUpdate(int num, bool isDir, const char* name)
{
	clearText(false);

	const bool isNds = (bnrRomType[num] == 0);

	if (customIcon[num] > 0 || (customIcon[num] && isNds)) {
		if (customIcon[num] == -1) {
			loadUnkIcon(num);
		} else if (bnriconisDSi[num]) {
			loadIcon(num, ndsBanner.dsi_icon[0], ndsBanner.dsi_palette[0], true);
		} else {
			loadIcon(num, ndsBanner.icon, ndsBanner.palette, false);
		}
	} else if (extension(name, ".argv")) {
		// look through the argv file for the corresponding nds/app file
		FILE *fp;
		char *line = NULL, *p = NULL;
		size_t size = 0;
		ssize_t rc;

		// open the argv file
		fp = fopen(name, "rb");
		if (fp == NULL) {
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

			if (extension(p, ".nds")
			 || extension(p, ".dsi")
			 || extension(p, ".ids")
			 || extension(p, ".srl")
			 || extension(p, ".app")) {
				// let's see if this is a file or directory
				rc = stat(p, &st);
				if (rc != 0) {
					// stat failed
					clearIcon(num);
				} else if (S_ISDIR(st.st_mode)) {
					// this is a directory!
					clearIcon(num);
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

		// icon
		if (bnriconisDSi[num]) {
			loadIcon(num, ndsBanner.dsi_icon[0], ndsBanner.dsi_palette[0], true);
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
	} else {
		loadUnkIcon(num);
	}
}

void titleUpdate(int num, bool top, bool isDir, const char* name)
{
	if (strcmp(name, "slot1")==0
	 || extension(name, ".nds")
	 || extension(name, ".dsi")
	 || extension(name, ".ids")
	 || extension(name, ".srl")
	 || extension(name, ".app")
	 || infoFound[num]) {
		// this is an nds/app file!
		// or a file with custom banner text
		if (infoFound[num]) {
			printSmall(false, BOX_PX, iconYpos[top ? 0 : 3] + BOX_PY - (calcSmallFontHeight(cachedTitle[num]) / 2), cachedTitle[num], Alignment::center);
		} else {
			printSmall(false, BOX_PX, iconYpos[top ? 0 : 3] + BOX_PY - (calcSmallFontHeight(name) / 2), name, Alignment::center);
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
		printSmall(false, BOX_PX, iconYpos[top ? 0 : 3] + BOX_PY - (calcSmallFontHeight(out) / 2), out, Alignment::center);
	}
}
