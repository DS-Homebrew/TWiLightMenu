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
#include "common/gl2d.h"
#include "common/tonccpy.h"
#include "graphics/graphics.h"
#include "graphics/fontHandler.h"
#include "ndsheaderbanner.h"
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
#include "icon_sms.h"
#include "icon_gg.h"
#include "icon_md.h"
#include "icon_snes.h"

extern bool extention(const std::string& filename, const char* ext);

extern int theme;
extern int colorMode;
extern bool animateDsiIcons;
extern int consoleModel;

extern u16 convertVramColorToGrayscale(u16 val);

static int iconTexID[2][8];
static int plgTexID;
static int gbaModeTexID;
static int gbTexID;
static int nesTexID;
static int smsTexID;
static int ggTexID;
static int mdTexID;
static int snesTexID;
sNDSHeaderExt ndsHeader;
sNDSBannerExt ndsBanner;

#define TITLE_CACHE_SIZE 0x80

static bool infoFound[2] = {false};
static u16 cachedTitle[2][TITLE_CACHE_SIZE]; 
static char titleToDisplay[2][3][384]; 

static u32 arm9StartSig[4];

static glImage plgIcon[1];

static glImage ndsIcon[2][8][(32 / 32) * (256 / 32)];

static glImage gbaModeIcon[1];
static glImage gbIcon[(32 / 32) * (64 / 32)];
static glImage nesIcon[1];
static glImage smsIcon[1];
static glImage ggIcon[1];
static glImage mdIcon[1];
static glImage snesIcon[1];

u8 *clearTiles;
u16 *blackPalette;
u8 *tilesModified;

void iconTitleInit()
{
	clearTiles = new u8[(32 * 256) / 2]();
	blackPalette = new u16[16*8]();
	tilesModified = new u8[(32 * 256) / 2];
}

static inline void writeBannerText(int num, int textlines, const char* text1, const char* text2, const char* text3)
{
	switch(textlines) {
		case 0:
		default:
			printSmallCentered(false, BOX_PX, iconYpos[num==0 ? 3 : 0]+BOX_PY+BOX_PY_spacing1, text1);
			break;
		case 1:
			printSmallCentered(false, BOX_PX, iconYpos[num==0 ? 3 : 0]+BOX_PY+BOX_PY_spacing2, text1);
			printSmallCentered(false, BOX_PX, iconYpos[num==0 ? 3 : 0]+BOX_PY+BOX_PY_spacing3, text2);
			break;
		case 2:
			printSmallCentered(false, BOX_PX, iconYpos[num==0 ? 3 : 0]+BOX_PY, text1);
			printSmallCentered(false, BOX_PX, iconYpos[num==0 ? 3 : 0]+BOX_PY+BOX_PY_spacing1, text2);
			printSmallCentered(false, BOX_PX, iconYpos[num==0 ? 3 : 0]+BOX_PY+BOX_PY_spacing1*2, text3);
			break;
	}
}

static void convertIconTilesToRaw(u8 *tilesSrc, u8 *tilesNew, bool twl)
{
	int PY = 32;
	if(twl) PY = 32*8;
	const int PX = 16;
	const int TILE_SIZE_Y = 8;
	const int TILE_SIZE_X = 4;
	int index = 0;
	for (int tileY = 0; tileY < PY / TILE_SIZE_Y; ++tileY)
	{
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
	if(twl) {
		Ysize = 256;
		textureSizeY = TEXTURE_SIZE_256;
		loadIcon_loopTimes = 8;
	}

	for (int i = 0; i < 8; i++) {
		glDeleteTextures(1, &iconTexID[num][i]);
	}
	for (int i = 0; i < loadIcon_loopTimes; i++) {
		if (colorMode == 1) {
			for (int i2 = 0; i2 < 16; i2++) {
				*(palSrc+i2+(i*16)) = convertVramColorToGrayscale(*(palSrc+i2+(i*16)));
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

void loadConsoleIcons()
{
	u16* newPalette;

	// DSTWO Plugin
	glDeleteTextures(1, &plgTexID);
	
	newPalette = (u16*)icon_plgPal;
	if (colorMode == 1) {
		for (int i2 = 0; i2 < 16; i2++) {
			*(newPalette+i2) = convertVramColorToGrayscale(*(newPalette+i2));
		}
	}
	plgTexID =
	glLoadTileSet(plgIcon, // pointer to glImage array
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

	if (!isDSiMode()) {
		// GBA Mode
		newPalette = (u16*)icon_gbamodePal;
		if (colorMode == 1) {
			for (int i2 = 0; i2 < 16; i2++) {
				*(newPalette+i2) = convertVramColorToGrayscale(*(newPalette+i2));
			}
		}
		gbaModeTexID =
		glLoadTileSet(gbaModeIcon, // pointer to glImage array
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
	if (colorMode == 1) {
		for (int i2 = 0; i2 < 16; i2++) {
			*(newPalette+i2) = convertVramColorToGrayscale(*(newPalette+i2));
		}
	}
	gbTexID =
	glLoadTileSet(gbIcon, // pointer to glImage array
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
	if (colorMode == 1) {
		for (int i2 = 0; i2 < 16; i2++) {
			*(newPalette+i2) = convertVramColorToGrayscale(*(newPalette+i2));
		}
	}
	nesTexID =
	glLoadTileSet(nesIcon, // pointer to glImage array
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

	// SMS
	glDeleteTextures(1, &smsTexID);
	
	newPalette = (u16*)icon_smsPal;
	if (colorMode == 1) {
		for (int i2 = 0; i2 < 16; i2++) {
			*(newPalette+i2) = convertVramColorToGrayscale(*(newPalette+i2));
		}
	}
	smsTexID =
	glLoadTileSet(smsIcon, // pointer to glImage array
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
	if (colorMode == 1) {
		for (int i2 = 0; i2 < 16; i2++) {
			*(newPalette+i2) = convertVramColorToGrayscale(*(newPalette+i2));
		}
	}
	ggTexID =
	glLoadTileSet(ggIcon, // pointer to glImage array
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
	if (colorMode == 1) {
		for (int i2 = 0; i2 < 16; i2++) {
			*(newPalette+i2) = convertVramColorToGrayscale(*(newPalette+i2));
		}
	}
	mdTexID =
	glLoadTileSet(mdIcon, // pointer to glImage array
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
	if (colorMode == 1) {
		for (int i2 = 0; i2 < 16; i2++) {
			*(newPalette+i2) = convertVramColorToGrayscale(*(newPalette+i2));
		}
	}
	snesTexID =
	glLoadTileSet(snesIcon, // pointer to glImage array
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
}

static void clearIcon(int num)
{
	loadIcon(num, clearTiles, blackPalette, true);
}

void drawIcon(int num, int Xpos, int Ypos)
{
	glSprite(Xpos, Ypos, bannerFlip[num], &ndsIcon[num][bnriconPalLine[num]][bnriconframenumY[num] & 31]);
}

void drawIconPlg(int Xpos, int Ypos)
{
	glSprite(Xpos, Ypos, GL_FLIP_NONE, plgIcon);
}
void drawIconGBA(int Xpos, int Ypos)
{
	glSprite(Xpos, Ypos, GL_FLIP_NONE, gbaModeIcon);
}
void drawIconGB(int Xpos, int Ypos)
{
	glSprite(Xpos, Ypos, GL_FLIP_NONE, &gbIcon[0 & 31]);
}
void drawIconGBC(int Xpos, int Ypos)
{
	glSprite(Xpos, Ypos, GL_FLIP_NONE, &gbIcon[1 & 31]);
}
void drawIconNES(int Xpos, int Ypos)
{
	glSprite(Xpos, Ypos, GL_FLIP_NONE, nesIcon);
}
void drawIconSMS(int Xpos, int Ypos)
{
	glSprite(Xpos, Ypos, GL_FLIP_NONE, smsIcon);
}
void drawIconGG(int Xpos, int Ypos)
{
	glSprite(Xpos, Ypos, GL_FLIP_NONE, ggIcon);
}
void drawIconMD(int Xpos, int Ypos)
{
	glSprite(Xpos, Ypos, GL_FLIP_NONE, mdIcon);
}
void drawIconSNES(int Xpos, int Ypos)
{
	glSprite(Xpos, Ypos, GL_FLIP_NONE, snesIcon);
}

void loadFixedBanner(bool isSlot1) {
	/* Banner fixes start here */
	u32 bannersize = 0;

	/*FILE* bannerFile = fopen("sd:/_nds/TWiLightMenu/slot1.bnr", "wb");
	bannersize = NDS_BANNER_SIZE_ORIGINAL;
	fwrite(&ndsBanner, 1, bannersize, bannerFile);
	fclose(bannerFile);*/

	// Fire Emblem - Heroes of Light and Shadow
	if (ndsBanner.crc[3] == 0xD8F4) {
		// Use fixed banner.
		FILE *fixedBannerFile =
		    fopen("nitro:/fixedbanners/Fire Emblem - Heroes of Light and Shadow (J) (Eng).bnr", "rb");
		bannersize = NDS_BANNER_SIZE_DSi;
		fread(&ndsBanner, 1, bannersize, fixedBannerFile);
		fclose(fixedBannerFile);
	} else // Pokemon Black Version
	    if (ndsBanner.crc[0] != 0x4683 && ndsBanner.crc[0] != 0xA251 && ndsBanner.crc[3] == 0xEE5D) {
		// Use fixed banner.
		FILE *fixedBannerFile = fopen("nitro:/fixedbanners/Pokemon Black Version.bnr", "rb");
		bannersize = NDS_BANNER_SIZE_DSi;
		fread(&ndsBanner, 1, bannersize, fixedBannerFile);
		fclose(fixedBannerFile);
	} else // Pokemon Blaze Black (Clean Version)
	    if (ndsBanner.crc[0] == 0x4683 && ndsBanner.crc[3] == 0xEE5D) {
		// Use fixed banner.
		FILE *fixedBannerFile = fopen("nitro:/fixedbanners/Pokemon Blaze Black (Clean Version).bnr", "rb");
		bannersize = NDS_BANNER_SIZE_DSi;
		fread(&ndsBanner, 1, bannersize, fixedBannerFile);
		fclose(fixedBannerFile);
	} else // Pokemon Blaze Black (Full Version)
	    if (ndsBanner.crc[0] == 0xA251 && ndsBanner.crc[3] == 0xEE5D) {
		// Use fixed banner.
		FILE *fixedBannerFile = fopen("nitro:/fixedbanners/Pokemon Blaze Black (Full Version).bnr", "rb");
		bannersize = NDS_BANNER_SIZE_DSi;
		fread(&ndsBanner, 1, bannersize, fixedBannerFile);
		fclose(fixedBannerFile);
	} else // Pokemon White Version
	    if (ndsBanner.crc[0] != 0x77F4 && ndsBanner.crc[0] != 0x9CA8 && ndsBanner.crc[3] == 0x0C88) {
		// Use fixed banner.
		FILE *fixedBannerFile = fopen("nitro:/fixedbanners/Pokemon White Version.bnr", "rb");
		bannersize = NDS_BANNER_SIZE_DSi;
		fread(&ndsBanner, 1, bannersize, fixedBannerFile);
		fclose(fixedBannerFile);
	} else // Pokemon Volt White (Clean Version)
	    if (ndsBanner.crc[0] == 0x77F4 && ndsBanner.crc[3] == 0x0C88) {
		// Use fixed banner.
		FILE *fixedBannerFile = fopen("nitro:/fixedbanners/Pokemon Volt White (Clean Version).bnr", "rb");
		bannersize = NDS_BANNER_SIZE_DSi;
		fread(&ndsBanner, 1, bannersize, fixedBannerFile);
		fclose(fixedBannerFile);
	} else // Pokemon Volt White (Full Version)
	    if (ndsBanner.crc[0] == 0x9CA8 && ndsBanner.crc[3] == 0x0C88) {
		// Use fixed banner.
		FILE *fixedBannerFile = fopen("nitro:/fixedbanners/Pokemon Volt White (Full Version).bnr", "rb");
		bannersize = NDS_BANNER_SIZE_DSi;
		fread(&ndsBanner, 1, bannersize, fixedBannerFile);
		fclose(fixedBannerFile);
	} else // Pokemon Black Version 2
	    if (ndsBanner.crc[3] == 0x2CA3) {
		// Use fixed banner.
		FILE *fixedBannerFile = fopen("nitro:/fixedbanners/Pokemon Black Version 2.bnr", "rb");
		bannersize = NDS_BANNER_SIZE_DSi;
		fread(&ndsBanner, 1, bannersize, fixedBannerFile);
		fclose(fixedBannerFile);
	} else // Pokemon White Version 2
	    if (ndsBanner.crc[3] == 0x3B18) {
		// Use fixed banner.
		FILE *fixedBannerFile = fopen("nitro:/fixedbanners/Pokemon White Version 2.bnr", "rb");
		bannersize = NDS_BANNER_SIZE_DSi;
		fread(&ndsBanner, 1, bannersize, fixedBannerFile);
		fclose(fixedBannerFile);
	} else if (isSlot1 && memcmp(ndsHeader.gameCode, "ALXX", 4) == 0) {
		cardRead(0x75600, &arm9StartSig, 0x10);
		if (arm9StartSig[0] == 0xE58D0008
		 && arm9StartSig[1] == 0xE1500005
		 && arm9StartSig[2] == 0xBAFFFFC5
		 && arm9StartSig[3] == 0xE59D100C)
		{
			// It's a SuperCard DSTWO, so use correct banner.
			cardRead(0x1843400, &ndsBanner, NDS_BANNER_SIZE_ORIGINAL);
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
	isDSiWare[num] = false;
	isHomebrew[num] = false;
	isModernHomebrew[num] = false;
	infoFound[num] = false;

	if (extention(name, ".argv")) {
		// look through the argv file for the corresponding nds file
		FILE *fp;
		char *line = NULL, *p = NULL;
		size_t size = 0;
		ssize_t rc;

		// open the argv file
		fp = fopen(name, "rb");
		if (fp == NULL)
		{
			clearBannerSequence(num);
			fclose(fp);
			return;
		}

		// read each line
		while ((rc = __getline(&line, &size, fp)) > 0)
		{
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

		if (p && *p)
		{
			// we found an argument
			struct stat st;

			// truncate everything after first argument
			strtok(p, "\n\r\t ");

			if (extention(p, ".nds")
			 || extention(p, ".dsi")
			 || extention(p, ".ids")
			 || extention(p, ".srl")
			 || extention(p, ".app"))
			{
				// let's see if this is a file or directory
				rc = stat(p, &st);
				if (rc != 0)
				{
					// stat failed
					clearBannerSequence(num);
				}
				else if (S_ISDIR(st.st_mode))
				{
					// this is a directory!
					clearBannerSequence(num);
				}
				else
				{
					getGameInfo(num, false, p);
				}
			}
			else
			{
				// this is not an nds/app file!
				clearBannerSequence(num);
			}
		}
		else
		{
			clearBannerSequence(num);
		}
		// clean up the allocated line
		free(line);
	}
	else if (strcmp(name, "slot1")==0
			 || extention(name, ".nds")
			 || extention(name, ".dsi")
			 || extention(name, ".ids")
			 || extention(name, ".srl")
			 || extention(name, ".app"))
	{
		// this is an nds/app file!
		FILE *fp;
		int ret;
		bool isSlot1 = (strcmp(name, "slot1") == 0);

		if (isSlot1)
		{
			cardRead(0, &ndsHeader, sizeof(ndsHeader));
		}
		else
		{
			// open file for reading info
			fp = fopen(name, "rb");
			if (fp == NULL)
			{
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

			fseek(fp, (ndsHeader.arm9romOffset <= 0x200 ? ndsHeader.arm9romOffset : ndsHeader.arm9romOffset+0x800), SEEK_SET);
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
					|| (ndsHeader.arm9binarySize == 0xD45C0 && ndsHeader.arm7binarySize == 0x2B7C)		// ikuReader v0.058
					|| (ndsHeader.arm9binarySize == 0x54620 && ndsHeader.arm7binarySize == 0x1538)		// XRoar 0.24fp3
					|| (ndsHeader.arm9binarySize == 0x2C9A8 && ndsHeader.arm7binarySize == 0xFB98)		// NitroGrafx v0.7
					|| (ndsHeader.arm9binarySize == 0x22AE4 && ndsHeader.arm7binarySize == 0xA764)) {	// It's 1975 and this man is about to show you the future
						isModernHomebrew[num] = false; // Have nds-bootstrap load it (in case if it doesn't)
					}
				}
			} else if ((memcmp(ndsHeader.gameTitle, "NDS.TinyFB", 10) == 0)
					 || (memcmp(ndsHeader.gameTitle, "UNLAUNCH.DSI", 12) == 0)) {
				isHomebrew[num] = true;
				isModernHomebrew[num] = true; // No need to use nds-bootstrap
			} else if ((memcmp(ndsHeader.gameTitle, "NMP4BOOT", 8) == 0)
			 || (ndsHeader.arm7executeAddress >= 0x037F0000 && ndsHeader.arm7destination >= 0x037F0000)) {
				isHomebrew[num] = true; // Homebrew is old (requires a DLDI driver to read from SD)
			} else if ((ndsHeader.gameCode[0] == 0x48 && ndsHeader.makercode[0] != 0 && ndsHeader.makercode[1] != 0)
			 || (ndsHeader.gameCode[0] == 0x4B && ndsHeader.makercode[0] != 0 && ndsHeader.makercode[1] != 0)
			 || (ndsHeader.gameCode[0] == 0x5A && ndsHeader.makercode[0] != 0 && ndsHeader.makercode[1] != 0)
			 || (ndsHeader.gameCode[0] == 0x42 && ndsHeader.gameCode[1] == 0x38 && ndsHeader.gameCode[2] == 0x38))
			{ if (ndsHeader.unitCode != 0)
				isDSiWare[num] = true; // Is a DSiWare game
			}
		}

		if (ndsHeader.dsi_flags & BIT(4))
			bnrWirelessIcon[num] = 1;
		else if (ndsHeader.dsi_flags & BIT(3))
			bnrWirelessIcon[num] = 2;

		if (ndsHeader.bannerOffset == 0)
		{
			if (!isSlot1)
				fclose(fp);

			FILE* bannerFile = fopen("nitro:/noinfo.bnr", "rb");
			fread(&ndsBanner, 1, NDS_BANNER_SIZE_ZH_KO, bannerFile);
			fclose(bannerFile);

			tonccpy(cachedTitle[num], ndsBanner.titles[setGameLanguage], TITLE_CACHE_SIZE*sizeof(u16));

			return;
		}
		if (isSlot1)
		{
			if ((ndsCardHeader.bannerOffset > 0) && cardInited)
			{
				cardRead(ndsCardHeader.bannerOffset, &ndsBanner, NDS_BANNER_SIZE_DSi);
			}
			else
			{
				FILE* bannerFile = fopen("nitro:/noinfo.bnr", "rb");
				fread(&ndsBanner, 1, NDS_BANNER_SIZE_ZH_KO, bannerFile);
				fclose(bannerFile);

				tonccpy(cachedTitle[num], ndsBanner.titles[setGameLanguage], TITLE_CACHE_SIZE*sizeof(u16));

				return;
			}
		}
		else
		{
			ret = fseek(fp, ndsHeader.bannerOffset, SEEK_SET);
			if (ret == 0)
				ret = fread(&ndsBanner, NDS_BANNER_SIZE_DSi, 1, fp); // read if seek succeed
			else
				ret = 0; // if seek fails set to !=1

			if (ret != 1)
			{
				// try again, but using regular banner size
				ret = fseek(fp, ndsHeader.bannerOffset, SEEK_SET);
				if (ret == 0)
					ret = fread(&ndsBanner, NDS_BANNER_SIZE_ORIGINAL, 1, fp); // read if seek succeed
				else
					ret = 0; // if seek fails set to !=1

				if (ret != 1)
				{
					fclose(fp);

					FILE* bannerFile = fopen("nitro:/noinfo.bnr", "rb");
					fread(&ndsBanner, 1, NDS_BANNER_SIZE_ZH_KO, bannerFile);
					fclose(bannerFile);

					tonccpy(cachedTitle[num], ndsBanner.titles[setGameLanguage], TITLE_CACHE_SIZE*sizeof(u16));

					return;
				}
			}

			// close file!
			fclose(fp);
		}

		loadFixedBanner(isSlot1);

		DC_FlushAll();

		int currentLang = 0;
		if (ndsBanner.version == NDS_BANNER_VER_ZH || ndsBanner.version == NDS_BANNER_VER_ZH_KO || ndsBanner.version == NDS_BANNER_VER_DSi) {
			currentLang = setGameLanguage;
		} else {
			currentLang = setTitleLanguage;
		}
		while (ndsBanner.titles[currentLang][0] == 0) {
			if (currentLang == 0) break;
			currentLang--;
		}
		tonccpy(cachedTitle[num], ndsBanner.titles[currentLang], TITLE_CACHE_SIZE*sizeof(u16));
		infoFound[num] = true;

		// banner sequence
		if(animateDsiIcons && ndsBanner.version == NDS_BANNER_VER_DSi) {
			grabBannerSequence(num);
			bnriconisDSi[num] = true;
		}
	}
}

void iconUpdate(int num, bool isDir, const char* name)
{
	clearText(false);

	if (extention(name, ".argv")) {
		// look through the argv file for the corresponding nds/app file
		FILE *fp;
		char *line = NULL, *p = NULL;
		size_t size = 0;
		ssize_t rc;

		// open the argv file
		fp = fopen(name, "rb");
		if (fp == NULL)
		{
			clearIcon(num);
			fclose(fp);
			return;
		}

		// read each line
		while ((rc = __getline(&line, &size, fp)) > 0)
		{
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

		if (p && *p)
		{
			// we found an argument
			struct stat st;

			// truncate everything after first argument
			strtok(p, "\n\r\t ");

			if (extention(p, ".nds")
			 || extention(p, ".dsi")
			 || extention(p, ".ids")
			 || extention(p, ".srl")
			 || extention(p, ".app"))
			{
				// let's see if this is a file or directory
				rc = stat(p, &st);
				if (rc != 0)
				{
					// stat failed
					clearIcon(num);
				}
				else if (S_ISDIR(st.st_mode))
				{
					// this is a directory!
					clearIcon(num);
				}
				else
				{
					iconUpdate(num, false, p);
				}
			}
			else
			{
				// this is not an nds/app file!
				clearIcon(num);
			}
		}
		else
		{
			clearIcon(num);
		}
		// clean up the allocated line
		free(line);
	} else if (strcmp(name, "slot1")==0
			 || extention(name, ".nds")
			 || extention(name, ".dsi")
			 || extention(name, ".ids")
			 || extention(name, ".srl")
			 || extention(name, ".app"))
	{
		// this is an nds/app file!

		// icon
		DC_FlushAll();
		if(animateDsiIcons && ndsBanner.version == NDS_BANNER_VER_DSi) {
			loadIcon(num, ndsBanner.dsi_icon[0], ndsBanner.dsi_palette[0], true);
		} else {
			loadIcon(num, ndsBanner.icon, ndsBanner.palette, false);
		}
	}
}

void titleUpdate(int num, bool isDir, const char* name)
{
	if (extention(name, ".plg")
	 || extention(name, ".rvid")
	 || extention(name, ".mp4")
	 || extention(name, ".gba")
	 || extention(name, ".gb")
	 || extention(name, ".sgb")
	 || extention(name, ".gbc")
	 || extention(name, ".nes")
	 || extention(name, ".fds")
	 || extention(name, ".sms")
	 || extention(name, ".gg")
	 || extention(name, ".gen")
	 || extention(name, ".smc")
	 || extention(name, ".sfc")
	 || extention(name, ".a26"))
	{
		writeBannerText(num, 0, name, "", "");
	}
	else
	{
		// this is an nds/app file!

		// turn unicode into ascii (kind of)
		// and convert 0x0A into 0x00
		int bannerlines = 0;
		// The index of the character array
		int charIndex = 0;
		for (int i = 0; i < TITLE_CACHE_SIZE; i++)
		{
			// todo: fix crash on titles that are too long (homebrew)
			if ((cachedTitle[num][i] == 0x000A) || (cachedTitle[num][i] == 0xFFFF)) {
				titleToDisplay[num][bannerlines][charIndex] = 0;
				bannerlines++;
				charIndex = 0;
			} else if (cachedTitle[num][i] <= 0x007F) { // ASCII are one UTF-8 character
				titleToDisplay[num][bannerlines][charIndex++] = cachedTitle[num][i];
			} else if (cachedTitle[num][i] <= 0x07FF) { // 0x0080 - 0x07FF are two UTF-8 characters
				titleToDisplay[num][bannerlines][charIndex++] = (0xC0 | ((cachedTitle[num][i] & 0x7C0) >> 6));
				titleToDisplay[num][bannerlines][charIndex++] = (0x80 | (cachedTitle[num][i] & 0x03F));
			} else { // 0x0800 - 0xFFFF take three UTF-8 characters, we don't need to handle higher as we're coming from single UTF-16 chars
				titleToDisplay[num][bannerlines][charIndex++] = (0xE0 | ((cachedTitle[num][i] & 0xF000) >> 12));
				titleToDisplay[num][bannerlines][charIndex++] = (0x80 | ((cachedTitle[num][i] & 0x0FC0) >> 6));
				titleToDisplay[num][bannerlines][charIndex++] = (0x80 | (cachedTitle[num][i] & 0x003F));
			}
		}

		// text
		if (infoFound[num]) {
			writeBannerText(num, bannerlines, titleToDisplay[num][0], titleToDisplay[num][1], titleToDisplay[num][2]);
		} else {
			printSmallCentered(false, BOX_PX, iconYpos[num==0 ? 3 : 0]+BOX_PY+BOX_PY_spacing2, name);
			printSmallCentered(false, BOX_PX, iconYpos[num==0 ? 3 : 0]+BOX_PY+BOX_PY_spacing3, titleToDisplay[num][0]);
		}
	}
}
