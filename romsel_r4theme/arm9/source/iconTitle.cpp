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
#include "icon_vid.h"
#include "icon_img.h"
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

extern u16 convertVramColorToGrayscale(u16 val);

static int iconTexID[8];
static int folderTexID;
static int vidTexID;
static int imgTexID;
static int plgTexID;
static int gbaTexID;
static int gbTexID;
static int nesTexID;
static int sgTexID;
static int smsTexID;
static int ggTexID;
static int mdTexID;
static int snesTexID;
static int a26TexID;
static int colTexID;
static int m5TexID;
static int intTexID;
static int pceTexID;
static int wsTexID;
static int ngpTexID;
static int cpcTexID;
sNDSHeaderExt ndsHeader;
sNDSBannerExt ndsBanner;

static char titleToDisplay[3][384]; 

static u32 arm9StartSig[4];

static glImage folderIcon[1];
static glImage vidIcon[1];
static glImage imgIcon[1];
static glImage plgIcon[1];

static glImage ndsIcon[8][(32 / 32) * (256 / 32)];

static glImage gbaIcon[1];
static glImage gbIcon[(32 / 32) * (64 / 32)];
static glImage nesIcon[1];
static glImage sgIcon[1];
static glImage smsIcon[1];
static glImage ggIcon[1];
static glImage mdIcon[1];
static glImage snesIcon[1];
static glImage a26Icon[1];
static glImage colIcon[1];
static glImage m5Icon[1];
static glImage intIcon[1];
static glImage pceIcon[1];
static glImage wsIcon[1];
static glImage ngpIcon[1];
static glImage cpcIcon[1];

u8 *clearTiles;
u16 *blackPalette;
u8 *tilesModified;

void iconTitleInit()
{
	clearTiles = new u8[(32 * 256) / 2]();
	blackPalette = new u16[16*8]();
	tilesModified = new u8[(32 * 256) / 2];
}

static inline void writeBannerText(int textlines, const char* text1, const char* text2, const char* text3)
{
	if (ms().theme == TWLSettings::EThemeGBC) {
		char textAdjusted[26];
		snprintf(textAdjusted, 25, text1);
		for (int i = 15; i <= 17; i++) {
			iprintf ("\x1b[%d;8H", i);
			iprintf ("                  ");
		}
		iprintf ("\x1b[15;8H");
		iprintf (textAdjusted);
		if (textlines >= 1) {
			snprintf(textAdjusted, 25, text2);
			iprintf ("\x1b[16;8H");
			iprintf (textAdjusted);
		}
		if (textlines >= 2) {
			snprintf(textAdjusted, 25, text3);
			iprintf ("\x1b[17;8H");
			iprintf (textAdjusted);
		}
		return;
	}
	switch(textlines) {
		case 0:
		default:
			printSmallCentered(false, BOX_PX, BOX_PY+BOX_PY_spacing1, text1);
			break;
		case 1:
			printSmallCentered(false, BOX_PX, BOX_PY+BOX_PY_spacing2, text1);
			printSmallCentered(false, BOX_PX, BOX_PY+BOX_PY_spacing3, text2);
			break;
		case 2:
			printSmallCentered(false, BOX_PX, BOX_PY, text1);
			printSmallCentered(false, BOX_PX, BOX_PY+BOX_PY_spacing1, text2);
			printSmallCentered(false, BOX_PX, BOX_PY+BOX_PY_spacing1*2, text3);
			break;
	}
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
		if (ms().colorMode == 1) {
			for (int i2 = 0; i2 < 16; i2++) {
				*(palSrc+i2+(i*16)) = convertVramColorToGrayscale(*(palSrc+i2+(i*16)));
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

void loadConsoleIcons()
{
	u16* newPalette;

	// Folder
	glDeleteTextures(1, &folderTexID);

	newPalette = (u16*)icon_folderPal;
	if (ms().colorMode == 1) {
		for (int i2 = 0; i2 < 16; i2++) {
			*(newPalette+i2) = convertVramColorToGrayscale(*(newPalette+i2));
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

	// Video
	glDeleteTextures(1, &vidTexID);

	newPalette = (u16*)icon_vidPal;
	if (ms().colorMode == 1) {
		for (int i2 = 0; i2 < 16; i2++) {
			*(newPalette+i2) = convertVramColorToGrayscale(*(newPalette+i2));
		}
	}
	vidTexID =
	glLoadTileSet(vidIcon, // pointer to glImage array
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
				(u8*) icon_vidBitmap // Raw image data
				);

	// Image
	glDeleteTextures(1, &imgTexID);

	newPalette = (u16*)icon_imgPal;
	if (ms().colorMode == 1) {
		for (int i2 = 0; i2 < 16; i2++) {
			*(newPalette+i2) = convertVramColorToGrayscale(*(newPalette+i2));
		}
	}
	imgTexID =
	glLoadTileSet(imgIcon, // pointer to glImage array
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
				(u8*) icon_imgBitmap // Raw image data
				);

	// DSTWO Plugin
	glDeleteTextures(1, &plgTexID);

	newPalette = (u16*)icon_plgPal;
	if (ms().colorMode == 1) {
		for (int i2 = 0; i2 < 16; i2++) {
			*(newPalette+i2) = convertVramColorToGrayscale(*(newPalette+i2));
		}
	}
	plgTexID = glLoadTileSet(plgIcon, // pointer to glImage array
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
				(u8*) icon_plgBitmap // Raw image data
				);

	// GBA
	glDeleteTextures(1, &gbaTexID);
	
	if (ms().gbaBooter == TWLSettings::EGbaGbar2) {
		newPalette = (u16*)icon_gbaPal;
		if (ms().colorMode == 1) {
			for (int i2 = 0; i2 < 16; i2++) {
				*(newPalette+i2) = convertVramColorToGrayscale(*(newPalette+i2));
			}
		}
		gbaTexID = glLoadTileSet(gbaIcon, // pointer to glImage array
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
					(u8*) icon_gbaBitmap // Raw image data
					);
	} else {
		newPalette = (u16*)icon_gbamodePal;
		if (ms().colorMode == 1) {
			for (int i2 = 0; i2 < 16; i2++) {
				*(newPalette+i2) = convertVramColorToGrayscale(*(newPalette+i2));
			}
		}
		gbaTexID = glLoadTileSet(gbaIcon, // pointer to glImage array
					32, // sprite width
					32, // sprite height
					32, // bitmap width
					32, // bitmap height
					GL_RGB16, // texture type for glTexImage2D() in videoGL.h
					TEXTURE_SIZE_32, // sizeX for glTexImage2D() in videoGL.h
					TEXTURE_SIZE_32, // sizeY for glTexImage2D() in videoGL.h
					TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
					16, // Length of the palette to use (16 colors)
					(u16*) newPalette, // Load our 16 color tiles palette
					(u8*) icon_gbamodeBitmap // image data generated by GRIT
					);
	}

	// GB/GBC
	glDeleteTextures(1, &gbTexID);
	
	newPalette = (u16*)icon_gbPal;
	if (ms().colorMode == 1) {
		for (int i2 = 0; i2 < 16; i2++) {
			*(newPalette+i2) = convertVramColorToGrayscale(*(newPalette+i2));
		}
	}
	gbTexID = glLoadTileSet(gbIcon, // pointer to glImage array
				32, // sprite width
				32, // sprite height
				32, // bitmap image width
				64, // bitmap image height
				GL_RGB16, // texture type for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeX for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_64, // sizeY for glTexImage2D() in videoGL.h
				TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT,
				16, // Length of the palette to use (16 colors)
				(u16*) newPalette, // Image palette
				(u8*) icon_gbBitmap // Raw image data
				);

	// NES
	glDeleteTextures(1, &nesTexID);

	newPalette = (u16*)icon_nesPal;
	if (ms().colorMode == 1) {
		for (int i2 = 0; i2 < 16; i2++) {
			*(newPalette+i2) = convertVramColorToGrayscale(*(newPalette+i2));
		}
	}
	nesTexID = glLoadTileSet(nesIcon, // pointer to glImage array
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
				(u8*) icon_nesBitmap // Raw image data
				);

	// SG
	glDeleteTextures(1, &sgTexID);
	
	newPalette = (u16*)icon_sgPal;
	if (ms().colorMode == 1) {
		for (int i2 = 0; i2 < 16; i2++) {
			*(newPalette+i2) = convertVramColorToGrayscale(*(newPalette+i2));
		}
	}
	sgTexID =
	glLoadTileSet(sgIcon, // pointer to glImage array
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
				(u8*) icon_sgBitmap // Raw image data
				);

	// SMS
	glDeleteTextures(1, &smsTexID);
	
	newPalette = (u16*)icon_smsPal;
	if (ms().colorMode == 1) {
		for (int i2 = 0; i2 < 16; i2++) {
			*(newPalette+i2) = convertVramColorToGrayscale(*(newPalette+i2));
		}
	}
	smsTexID = glLoadTileSet(smsIcon, // pointer to glImage array
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
				(u8*) icon_smsBitmap // Raw image data
				);

	// GG
	glDeleteTextures(1, &ggTexID);
	
	newPalette = (u16*)icon_ggPal;
	if (ms().colorMode == 1) {
		for (int i2 = 0; i2 < 16; i2++) {
			*(newPalette+i2) = convertVramColorToGrayscale(*(newPalette+i2));
		}
	}
	ggTexID = glLoadTileSet(ggIcon, // pointer to glImage array
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
				(u8*) icon_ggBitmap // Raw image data
				);

	// MD
	glDeleteTextures(1, &mdTexID);
	
	newPalette = (u16*)icon_mdPal;
	if (ms().colorMode == 1) {
		for (int i2 = 0; i2 < 16; i2++) {
			*(newPalette+i2) = convertVramColorToGrayscale(*(newPalette+i2));
		}
	}
	mdTexID = glLoadTileSet(mdIcon, // pointer to glImage array
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
				(u8*) icon_mdBitmap // Raw image data
				);

	// SNES
	glDeleteTextures(1, &snesTexID);
	
	newPalette = (u16*)icon_snesPal;
	if (ms().colorMode == 1) {
		for (int i2 = 0; i2 < 16; i2++) {
			*(newPalette+i2) = convertVramColorToGrayscale(*(newPalette+i2));
		}
	}
	snesTexID = glLoadTileSet(snesIcon, // pointer to glImage array
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
				(u8*) icon_snesBitmap // Raw image data
				);

	// A26
	glDeleteTextures(1, &a26TexID);
	
	newPalette = (u16*)icon_a26Pal;
	if (ms().colorMode == 1) {
		for (int i2 = 0; i2 < 16; i2++) {
			*(newPalette+i2) = convertVramColorToGrayscale(*(newPalette+i2));
		}
	}
	a26TexID =
	glLoadTileSet(a26Icon, // pointer to glImage array
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
				(u8*) icon_a26Bitmap // Raw image data
				);

	// COL
	glDeleteTextures(1, &colTexID);
	
	newPalette = (u16*)icon_colPal;
	if (ms().colorMode == 1) {
		for (int i2 = 0; i2 < 16; i2++) {
			*(newPalette+i2) = convertVramColorToGrayscale(*(newPalette+i2));
		}
	}
	colTexID =
	glLoadTileSet(colIcon, // pointer to glImage array
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
				(u8*) icon_colBitmap // Raw image data
				);

	// M5
	glDeleteTextures(1, &m5TexID);
	
	newPalette = (u16*)icon_m5Pal;
	if (ms().colorMode == 1) {
		for (int i2 = 0; i2 < 16; i2++) {
			*(newPalette+i2) = convertVramColorToGrayscale(*(newPalette+i2));
		}
	}
	m5TexID =
	glLoadTileSet(m5Icon, // pointer to glImage array
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
				(u8*) icon_m5Bitmap // Raw image data
				);

	// INT
	glDeleteTextures(1, &intTexID);
	
	newPalette = (u16*)icon_intPal;
	if (ms().colorMode == 1) {
		for (int i2 = 0; i2 < 16; i2++) {
			*(newPalette+i2) = convertVramColorToGrayscale(*(newPalette+i2));
		}
	}
	intTexID =
	glLoadTileSet(intIcon, // pointer to glImage array
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
				(u8*) icon_intBitmap // Raw image data
				);

	// PCE
	glDeleteTextures(1, &pceTexID);
	
	newPalette = (u16*)icon_pcePal;
	if (ms().colorMode == 1) {
		for (int i2 = 0; i2 < 16; i2++) {
			*(newPalette+i2) = convertVramColorToGrayscale(*(newPalette+i2));
		}
	}
	pceTexID =
	glLoadTileSet(pceIcon, // pointer to glImage array
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
				(u8*) icon_pceBitmap // Raw image data
				);

	// WS
	glDeleteTextures(1, &wsTexID);
	
	newPalette = (u16*)icon_wsPal;
	if (ms().colorMode == 1) {
		for (int i2 = 0; i2 < 16; i2++) {
			*(newPalette+i2) = convertVramColorToGrayscale(*(newPalette+i2));
		}
	}
	wsTexID =
	glLoadTileSet(wsIcon, // pointer to glImage array
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
				(u8*) icon_wsBitmap // Raw image data
				);

	// NGP
	glDeleteTextures(1, &ngpTexID);
	
	newPalette = (u16*)icon_ngpPal;
	if (ms().colorMode == 1) {
		for (int i2 = 0; i2 < 16; i2++) {
			*(newPalette+i2) = convertVramColorToGrayscale(*(newPalette+i2));
		}
	}
	ngpTexID =
	glLoadTileSet(ngpIcon, // pointer to glImage array
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
				(u8*) icon_ngpBitmap // Raw image data
				);

	// CPC
	glDeleteTextures(1, &cpcTexID);

	newPalette = (u16*)icon_cpcPal;
	if (ms().colorMode == 1) {
		for (int i2 = 0; i2 < 16; i2++) {
			*(newPalette+i2) = convertVramColorToGrayscale(*(newPalette+i2));
		}
	}
	cpcTexID =
	glLoadTileSet(cpcIcon, // pointer to glImage array
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
				(u8*) icon_cpcBitmap // Raw image data
				);
}

static void clearIcon()
{
	loadIcon(clearTiles, blackPalette, true);
}

void drawIcon(int Xpos, int Ypos) { glSprite(Xpos, Ypos, bannerFlip, &ndsIcon[bnriconPalLine][bnriconframenumY & 31]); }

void drawIconFolder(int Xpos, int Ypos) { glSprite(Xpos, Ypos, GL_FLIP_NONE, folderIcon); }
void drawIconVID(int Xpos, int Ypos) { glSprite(Xpos, Ypos, GL_FLIP_NONE, vidIcon); }
void drawIconIMG(int Xpos, int Ypos) { glSprite(Xpos, Ypos, GL_FLIP_NONE, imgIcon); }
void drawIconPlg(int Xpos, int Ypos) { glSprite(Xpos, Ypos, GL_FLIP_NONE, plgIcon); }
void drawIconGBA(int Xpos, int Ypos) { glSprite(Xpos, Ypos, GL_FLIP_NONE, gbaIcon); }
void drawIconGB(int Xpos, int Ypos) { glSprite(Xpos, Ypos, GL_FLIP_NONE, &gbIcon[0 & 31]); }
void drawIconGBC(int Xpos, int Ypos) { glSprite(Xpos, Ypos, GL_FLIP_NONE, &gbIcon[1 & 31]); }
void drawIconNES(int Xpos, int Ypos) { glSprite(Xpos, Ypos, GL_FLIP_NONE, nesIcon); }
void drawIconSG(int Xpos, int Ypos) { glSprite(Xpos, Ypos, GL_FLIP_NONE, sgIcon); }
void drawIconSMS(int Xpos, int Ypos) { glSprite(Xpos, Ypos, GL_FLIP_NONE, smsIcon); }
void drawIconGG(int Xpos, int Ypos) { glSprite(Xpos, Ypos, GL_FLIP_NONE, ggIcon); }
void drawIconMD(int Xpos, int Ypos) { glSprite(Xpos, Ypos, GL_FLIP_NONE, mdIcon); }
void drawIconSNES(int Xpos, int Ypos) { glSprite(Xpos, Ypos, GL_FLIP_NONE, snesIcon); }
void drawIconA26(int Xpos, int Ypos) { glSprite(Xpos, Ypos, GL_FLIP_NONE, a26Icon); }
void drawIconCOL(int Xpos, int Ypos) { glSprite(Xpos, Ypos, GL_FLIP_NONE, colIcon); }
void drawIconM5(int Xpos, int Ypos) { glSprite(Xpos, Ypos, GL_FLIP_NONE, m5Icon); }
void drawIconINT(int Xpos, int Ypos) { glSprite(Xpos, Ypos, GL_FLIP_NONE, intIcon); }
void drawIconPCE(int Xpos, int Ypos) { glSprite(Xpos, Ypos, GL_FLIP_NONE, pceIcon); }
void drawIconWS(int Xpos, int Ypos) { glSprite(Xpos, Ypos, GL_FLIP_NONE, wsIcon); }
void drawIconNGP(int Xpos, int Ypos) { glSprite(Xpos, Ypos, GL_FLIP_NONE, ngpIcon); }
void drawIconCPC(int Xpos, int Ypos) { glSprite(Xpos, Ypos, GL_FLIP_NONE, cpcIcon); }

void getGameInfo(bool isDir, const char* name)
{
	bnriconPalLine = 0;
	bnriconframenumY = 0;
	bannerFlip = GL_FLIP_NONE;
	bnriconisDSi = false;
	bnrWirelessIcon = 0;
	customIcon = 0;
	isDSiWare = false;
	isHomebrew = false;
	isModernHomebrew = false;
	requiresRamDisk = false;
	requiresDonorRom = false;

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

		isTwlm = (strncmp(ndsHeader.gameCode, "SRLA", 4) == 0);
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
				}
			} else if (ndsHeader.gameCode[0] != 'D' && memcmp(ndsHeader.gameCode, "KCX", 3) != 0 && memcmp(ndsHeader.gameCode, "KAV", 3) != 0 && memcmp(ndsHeader.gameCode, "KNK", 3) != 0 && a7mbk6 == 0x080037C0 && ms().secondaryDevice && (!dsiFeatures() || bs().b4dsMode)) {
				requiresDonorRom = 51; // DSi-Enhanced ROM required
			} else if (memcmp(ndsHeader.gameCode, "AYI", 3) == 0 && ndsHeader.arm7binarySize == 0x25F70) {
				requiresDonorRom = 20; // SDK2.0 ROM required
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
			return;
		}
		ret = fseek(fp, ndsHeader.bannerOffset, SEEK_SET);
		if (ret == 0)
			ret = fread(&ndsBanner, sizeof (ndsBanner), 1, fp); // read if seek succeed
		else
			ret = 0; // if seek fails set to !=1

		if (ret != 1) {
			fclose(fp);
			return;
		}

		// close file!
		fclose(fp);

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
	if (ms().theme == TWLSettings::EThemeGBC) {
		return;
	} else {
		clearText(false);
	}

	if (isDir) {
		// icon
		clearIcon();
	} else if (customIcon) {
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
	} else if (extension(name, {".nds", ".dsi", ".ids", ".srl", ".app"})) {
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
	}
}

void titleUpdate(bool isDir, const char* name)
{
	if (ms().theme != TWLSettings::EThemeGBC) {
		clearText(false);
	}

	if (isDir) {
		// text
		if (strcmp(name, "..") == 0) {
			writeBannerText(0, "Back", "", "");
		} else {
			writeBannerText(0, name, "", "");
		}
	} else if (!extension(name, {".nds", ".dsi", ".ids", ".srl", ".app", ".argv"}) && customIcon != 2) {
		std::vector<std::string> lines;
		lines.push_back(name);

		for (uint i = 0; i < lines.size(); i++) {
			int width = calcSmallFontWidth(lines[i].c_str());
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

		int lineCount = lines.size();
		if (lineCount > 3) lineCount = 3;
		strcpy(titleToDisplay[0], lines[0].c_str());
		strcpy(titleToDisplay[1], lineCount > 1 ? lines[1].c_str() : "");
		strcpy(titleToDisplay[2], lineCount > 2 ? lines[2].c_str() : "");

		writeBannerText(lineCount-1, titleToDisplay[0], titleToDisplay[1], titleToDisplay[2]);
	} else if (extension(name, {".argv"})) {
		// look through the argv file for the corresponding nds/app file
		FILE *fp;
		char *line = NULL, *p = NULL;
		size_t size = 0;
		ssize_t rc;

		// open the argv file
		fp = fopen(name, "rb");
		if (fp == NULL) {
			writeBannerText(0, "(can't open file!)", "", "");
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
					writeBannerText(0, "(can't find argument!)", "", "");
				} else if (S_ISDIR(st.st_mode)) {
					// this is a directory!
					writeBannerText(1, "(invalid argv file!)", "This is a directory.", "");
				} else {
					titleUpdate(false, p);
				}
			} else {
				// this is not an nds/app file!
				writeBannerText(1, "(invalid argv file!)", "No .nds/.app file.", "");
			}
		} else {
			writeBannerText(0, "(no argument!)", "", "");
		}
		// clean up the allocated line
		free(line);
	} else if (extension(name, {".nds", ".dsi", ".ids", ".srl", ".app"}) || customIcon == 2) {
		// this is an nds/app file!
		// or a file with custom banner text
		if (customIcon != 2) {
			FILE *fp;
			unsigned int iconTitleOffset;
			int ret;

			// open file for reading info
			fp = fopen(name, "rb");
			if (fp == NULL) {
				// text
				writeBannerText(0, "(can't open file!)", "", "");
				fclose(fp);
				return;
			}

			ret = fseek(fp, offsetof(tNDSHeader, bannerOffset), SEEK_SET);
			if (ret == 0)
				ret = fread(&iconTitleOffset, sizeof (int), 1, fp); // read if seek succeed
			else
				ret = 0; // if seek fails set to !=1

			if (ret != 1) {
				// text
				writeBannerText(0, "(can't read file!)", "", "");
				fclose(fp);
				return;
			}

			if (iconTitleOffset == 0) {
				// text
				writeBannerText(1, name, "(no title/icon)", "");
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
					// text
					writeBannerText(1, name, "(can't read icon/title!)", "");
					fclose(fp);
					return;
				}
			}

			// close file!
			fclose(fp);
		}

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

		// turn unicode into ascii (kind of)
		// and convert 0x0A into 0x00
		int bannerlines = 0;
		// The index of the character array
		int charIndex = 0;
		for (int i = 0; i < TITLE_CACHE_SIZE; i++) {
			// todo: fix crash on titles that are too long (homebrew)
			if ((ndsBanner.titles[currentLang][i] == 0x000A) || (ndsBanner.titles[currentLang][i] == 0xFFFF)) {
				titleToDisplay[bannerlines][charIndex] = 0;
				bannerlines++;
				charIndex = 0;
			} else if (ndsBanner.titles[currentLang][i] <= 0x007F) { // ASCII are one UTF-8 character
				titleToDisplay[bannerlines][charIndex++] = ndsBanner.titles[currentLang][i];
			} else if (ndsBanner.titles[currentLang][i] <= 0x07FF) { // 0x0080 - 0x07FF are two UTF-8 characters
				titleToDisplay[bannerlines][charIndex++] = (0xC0 | ((ndsBanner.titles[currentLang][i] & 0x7C0) >> 6));
				titleToDisplay[bannerlines][charIndex++] = (0x80 | (ndsBanner.titles[currentLang][i] & 0x03F));
			} else { // 0x0800 - 0xFFFF take three UTF-8 characters, we don't need to handle higher as we're coming from single UTF-16 chars
				titleToDisplay[bannerlines][charIndex++] = (0xE0 | ((ndsBanner.titles[currentLang][i] & 0xF000) >> 12));
				titleToDisplay[bannerlines][charIndex++] = (0x80 | ((ndsBanner.titles[currentLang][i] & 0x0FC0) >> 6));
				titleToDisplay[bannerlines][charIndex++] = (0x80 | (ndsBanner.titles[currentLang][i] & 0x003F));
			}
		}

		// text
		//if (infoFound[num]) {
			writeBannerText(bannerlines, titleToDisplay[0], titleToDisplay[1], titleToDisplay[2]);
		//} else {
		//	printSmallCentered(false, BOX_PX, BOX_PY+BOX_PY_spacing2, name);
		//	printSmallCentered(false, BOX_PX, BOX_PY+BOX_PY_spacing3, titleToDisplay[0]);
		//}
	}
}
