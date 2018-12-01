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
#include "graphics/fontHandler.h"
#include "ndsheaderbanner.h"
#include "language.h"

#define ICON_POS_X	112
#define ICON_POS_Y	96

#define BOX_PX				73
#define BOX_PY				32
#define BOX_PY_spacing1		8
#define BOX_PY_spacing2		4
#define BOX_PY_spacing3		12

// Graphic files
#include "icon_unk.h"
#include "icon_gbamode.h"
#include "icon_gba.h"
#include "icon_gb.h"
#include "icon_nes.h"

extern int theme;
extern bool useGbarunner;
extern bool animateDsiIcons;

static int iconTexID[8];
static int gbaTexID;
static int gbTexID;
static int nesTexID;
sNDSHeaderExt ndsHeader;
sNDSBannerExt ndsBanner;

static glImage ndsIcon[8][(32 / 32) * (256 / 32)];

static glImage gbaIcon[1];
static glImage gbIcon[(32 / 32) * (64 / 32)];
static glImage nesIcon[1];

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
	switch(textlines) {
		case 0:
		default:
			printSmall(false, BOX_PX, BOX_PY+BOX_PY_spacing1, text1);
			break;
		case 1:
			printSmall(false, BOX_PX, BOX_PY+BOX_PY_spacing2, text1);
			printSmall(false, BOX_PX, BOX_PY+BOX_PY_spacing3, text2);
			break;
		case 2:
			printSmall(false, BOX_PX, BOX_PY, text1);
			printSmall(false, BOX_PX, BOX_PY+BOX_PY_spacing1, text2);
			printSmall(false, BOX_PX, BOX_PY+BOX_PY_spacing1*2, text3);
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

void loadIcon(u8 *tilesSrc, u16 *palSrc, bool twl)//(u8(*tilesSrc)[(32 * 32) / 2], u16(*palSrc)[16])
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
		glDeleteTextures(1, &iconTexID[i]);
	}
	for (int i = 0; i < loadIcon_loopTimes; i++) {
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

void loadGBCIcon()
{
	glDeleteTextures(1, &gbTexID);
	
	gbTexID =
	glLoadTileSet(gbIcon, // pointer to glImage array
				32, // sprite width
				32, // sprite height
				32, // bitmap image width
				64, // bitmap image height
				GL_RGB16, // texture type for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeX for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_64, // sizeY for glTexImage2D() in videoGL.h
				GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T | TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT,
				16, // Length of the palette to use (16 colors)
				(u16*) icon_gbPal, // Image palette
				(u8*) icon_gbBitmap // Raw image data
				);
}
void loadNESIcon()
{
	glDeleteTextures(1, &gbaTexID);
	
	if (useGbarunner) {
		gbaTexID =
		glLoadTileSet(gbaIcon, // pointer to glImage array
					32, // sprite width
					32, // sprite height
					32, // bitmap image width
					32, // bitmap image height
					GL_RGB16, // texture type for glTexImage2D() in videoGL.h
					TEXTURE_SIZE_32, // sizeX for glTexImage2D() in videoGL.h
					TEXTURE_SIZE_32, // sizeY for glTexImage2D() in videoGL.h
					GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T | TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT,
					16, // Length of the palette to use (16 colors)
					(u16*) icon_gbaPal, // Image palette
					(u8*) icon_gbaBitmap // Raw image data
					);
	} else {
		gbaTexID =
		glLoadTileSet(gbaIcon, // pointer to glImage array
					32, // sprite width
					32, // sprite height
					32, // bitmap width
					32, // bitmap height
					GL_RGB16, // texture type for glTexImage2D() in videoGL.h
					TEXTURE_SIZE_32, // sizeX for glTexImage2D() in videoGL.h
					TEXTURE_SIZE_32, // sizeY for glTexImage2D() in videoGL.h
					GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T | TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
					16, // Length of the palette to use (16 colors)
					(u16*) icon_gbamodePal, // Load our 16 color tiles palette
					(u8*) icon_gbamodeBitmap // image data generated by GRIT
					);
	}

	glDeleteTextures(1, &nesTexID);
	
	nesTexID =
	glLoadTileSet(nesIcon, // pointer to glImage array
				32, // sprite width
				32, // sprite height
				32, // bitmap image width
				32, // bitmap image height
				GL_RGB16, // texture type for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeX for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeY for glTexImage2D() in videoGL.h
				GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T | TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT,
				16, // Length of the palette to use (16 colors)
				(u16*) icon_nesPal, // Image palette
				(u8*) icon_nesBitmap // Raw image data
				);
}

static void clearIcon()
{
	loadIcon(clearTiles, blackPalette, true);
}

void drawIcon(int Xpos, int Ypos)
{
	glSprite(Xpos, Ypos, bannerFlip, &ndsIcon[bnriconPalLine][bnriconframenumY & 31]);
}

void drawIconGBA(int Xpos, int Ypos)
{
	glSprite(Xpos, Ypos, GL_FLIP_NONE, gbaIcon);
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

void loadFixedBanner(void) {
	/* Banner fixes start here */
	u32 bannersize = 0;

	// Fire Emblem - Heroes of Light and Shadow (English Translation)
	if(ndsBanner.crc[0] == 0xECF9
	&& ndsBanner.crc[1] == 0xD18F
	&& ndsBanner.crc[2] == 0xE22A
	&& ndsBanner.crc[3] == 0xD8F4)
	{
		// Use fixed banner.
		FILE* fixedBannerFile = fopen("nitro:/fixedbanners/Fire Emblem - Heroes of Light and Shadow (J) (Eng).bnr", "rb");
		bannersize = NDS_BANNER_SIZE_DSi;
		fread(&ndsBanner, 1, bannersize, fixedBannerFile);
		fclose(fixedBannerFile);
	} else // Pokemon Black Version
	if(ndsBanner.crc[0] == 0x4A19
	&& ndsBanner.crc[1] == 0x40AD
	&& ndsBanner.crc[2] == 0x5641
	&& ndsBanner.crc[3] == 0xEE5D)
	{
		// Use fixed banner.
		FILE* fixedBannerFile = fopen("nitro:/fixedbanners/Pokemon Black Version.bnr", "rb");
		bannersize = NDS_BANNER_SIZE_DSi;
		fread(&ndsBanner, 1, bannersize, fixedBannerFile);
		fclose(fixedBannerFile);
	} else // Pokemon Blaze Black (Clean Version)
	if(ndsBanner.crc[0] == 0x4683
	&& ndsBanner.crc[1] == 0x40AD
	&& ndsBanner.crc[2] == 0x5641
	&& ndsBanner.crc[3] == 0xEE5D)
	{
		// Use fixed banner.
		FILE* fixedBannerFile = fopen("nitro:/fixedbanners/Pokemon Blaze Black (Clean Version).bnr", "rb");
		bannersize = NDS_BANNER_SIZE_DSi;
		fread(&ndsBanner, 1, bannersize, fixedBannerFile);
		fclose(fixedBannerFile);
	} else // Pokemon Blaze Black (Full Version)
	if(ndsBanner.crc[0] == 0xA251
	&& ndsBanner.crc[1] == 0x40AD
	&& ndsBanner.crc[2] == 0x5641
	&& ndsBanner.crc[3] == 0xEE5D)
	{
		// Use fixed banner.
		FILE* fixedBannerFile = fopen("nitro:/fixedbanners/Pokemon Blaze Black (Full Version).bnr", "rb");
		bannersize = NDS_BANNER_SIZE_DSi;
		fread(&ndsBanner, 1, bannersize, fixedBannerFile);
		fclose(fixedBannerFile);
	} else // Pokemon White Version
	if(ndsBanner.crc[0] == 0xE249
	&& ndsBanner.crc[1] == 0x5C94
	&& ndsBanner.crc[2] == 0xBF18
	&& ndsBanner.crc[3] == 0x0C88)
	{
		// Use fixed banner.
		FILE* fixedBannerFile = fopen("nitro:/fixedbanners/Pokemon White Version.bnr", "rb");
		bannersize = NDS_BANNER_SIZE_DSi;
		fread(&ndsBanner, 1, bannersize, fixedBannerFile);
		fclose(fixedBannerFile);
	} else // Pokemon Volt White (Clean Version)
	if(ndsBanner.crc[0] == 0x77F4
	&& ndsBanner.crc[1] == 0x5C94
	&& ndsBanner.crc[2] == 0xBF18
	&& ndsBanner.crc[3] == 0x0C88)
	{
		// Use fixed banner.
		FILE* fixedBannerFile = fopen("nitro:/fixedbanners/Pokemon Volt White (Clean Version).bnr", "rb");
		bannersize = NDS_BANNER_SIZE_DSi;
		fread(&ndsBanner, 1, bannersize, fixedBannerFile);
		fclose(fixedBannerFile);
	} else // Pokemon Volt White (Full Version)
	if(ndsBanner.crc[0] == 0x9CA8
	&& ndsBanner.crc[1] == 0x5C94
	&& ndsBanner.crc[2] == 0xBF18
	&& ndsBanner.crc[3] == 0x0C88)
	{
		// Use fixed banner.
		FILE* fixedBannerFile = fopen("nitro:/fixedbanners/Pokemon Volt White (Full Version).bnr", "rb");
		bannersize = NDS_BANNER_SIZE_DSi;
		fread(&ndsBanner, 1, bannersize, fixedBannerFile);
		fclose(fixedBannerFile);
	} else // Pokemon Black Version 2
	if(ndsBanner.crc[0] == 0xF996
	&& ndsBanner.crc[1] == 0xD784
	&& ndsBanner.crc[2] == 0xA257
	&& ndsBanner.crc[3] == 0x2CA3)
	{
		// Use fixed banner.
		FILE* fixedBannerFile = fopen("nitro:/fixedbanners/Pokemon Black Version 2.bnr", "rb");
		bannersize = NDS_BANNER_SIZE_DSi;
		fread(&ndsBanner, 1, bannersize, fixedBannerFile);
		fclose(fixedBannerFile);
	} else // Pokemon White Version 2
	if(ndsBanner.crc[0] == 0xA487
	&& ndsBanner.crc[1] == 0xF58C
	&& ndsBanner.crc[2] == 0xAF9E
	&& ndsBanner.crc[3] == 0x3B18)
	{
		// Use fixed banner.
		FILE* fixedBannerFile = fopen("nitro:/fixedbanners/Pokemon White Version 2.bnr", "rb");
		bannersize = NDS_BANNER_SIZE_DSi;
		fread(&ndsBanner, 1, bannersize, fixedBannerFile);
		fclose(fixedBannerFile);
	}
}

void getGameInfo(bool isDir, const char* name)
{
	bnriconPalLine = 0;
	bnriconframenumY = 0;
	bannerFlip = GL_FLIP_NONE;
	bnriconisDSi = false;
	bnrWirelessIcon = 0;
	isDSiWare = false;
	isHomebrew = 0;

	if (isDir)
	{
		// banner sequence
		clearBannerSequence();
	}
	else if ((strlen(name) >= 5 && strcasecmp(name + strlen(name) - 5, ".argv") == 0)
		|| (strlen(name) >= 5 && strcasecmp(name + strlen(name) - 5, ".ARGV") == 0)
		|| (strlen(name) >= 10 && strcasecmp(name + strlen(name) - 10, ".launcharg") == 0)
		|| (strlen(name) >= 10 && strcasecmp(name + strlen(name) - 10, ".LAUNCHARG") == 0))
	{
		// look through the argv file for the corresponding nds file
		FILE *fp;
		char *line = NULL, *p = NULL;
		size_t size = 0;
		ssize_t rc;

		// open the argv file
		fp = fopen(name, "rb");
		if (fp == NULL)
		{
			clearBannerSequence();
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

		if ((strlen(name) >= 10 && strcasecmp(name + strlen(name) - 10, ".launcharg") == 0)
		|| (strlen(name) >= 10 && strcasecmp(name + strlen(name) - 10, ".LAUNCHARG") == 0))
		{
			// Search for .app
			char appPath[256];
			for (u8 appVer = 0; appVer <= 0xFF; appVer++)
			{
				if (appVer > 0xF) {
					snprintf(appPath, sizeof(appPath), "%scontent/000000%x.app", p, appVer);
				} else {
					snprintf(appPath, sizeof(appPath), "%scontent/0000000%x.app", p, appVer);
				}
				if (access(appPath, F_OK) == 0)
				{
					p = appPath;
					break;
				}
				if (appVer == 0xFF) {
					p = NULL;
				}
			}
		}

		if (p && *p)
		{
			// we found an argument
			struct stat st;

			// truncate everything after first argument
			strtok(p, "\n\r\t ");

			if ((strlen(p) >= 4 && strcasecmp(p + strlen(p) - 4, ".nds") == 0)
			|| (strlen(p) >= 4 && strcasecmp(p + strlen(p) - 4, ".NDS") == 0)
			|| (strlen(p) >= 4 && strcasecmp(p + strlen(p) - 4, ".dsi") == 0)
			|| (strlen(p) >= 4 && strcasecmp(p + strlen(p) - 4, ".DSI") == 0)
			|| (strlen(p) >= 4 && strcasecmp(p + strlen(p) - 4, ".ids") == 0)
			|| (strlen(p) >= 4 && strcasecmp(p + strlen(p) - 4, ".IDS") == 0)
			|| (strlen(p) >= 4 && strcasecmp(p + strlen(p) - 4, ".app") == 0)
			|| (strlen(p) >= 4 && strcasecmp(p + strlen(p) - 4, ".APP") == 0))
			{
				// let's see if this is a file or directory
				rc = stat(p, &st);
				if (rc != 0)
				{
					// stat failed
					clearBannerSequence();
				}
				else if (S_ISDIR(st.st_mode))
				{
					// this is a directory!
					clearBannerSequence();
				}
				else
				{
					getGameInfo(false, p);
				}
			}
			else
			{
				// this is not an nds/app file!
				clearBannerSequence();
			}
		}
		else
		{
			clearBannerSequence();
		}
		// clean up the allocated line
		free(line);
	}
	else if ((strlen(name) >= 4 && strcasecmp(name + strlen(name) - 4, ".nds") == 0)
			|| (strlen(name) >= 4 && strcasecmp(name + strlen(name) - 4, ".NDS") == 0)
			|| (strlen(name) >= 4 && strcasecmp(name + strlen(name) - 4, ".dsi") == 0)
			|| (strlen(name) >= 4 && strcasecmp(name + strlen(name) - 4, ".DSI") == 0)
			|| (strlen(name) >= 4 && strcasecmp(name + strlen(name) - 4, ".ids") == 0)
			|| (strlen(name) >= 4 && strcasecmp(name + strlen(name) - 4, ".IDS") == 0)
			|| (strlen(name) >= 4 && strcasecmp(name + strlen(name) - 4, ".app") == 0)
			|| (strlen(name) >= 4 && strcasecmp(name + strlen(name) - 4, ".APP") == 0))
	{
		// this is an nds/app file!
		FILE *fp;
		int ret;

		// open file for reading info
		fp = fopen(name, "rb");
		if (fp == NULL)
		{
			// banner sequence
			clearBannerSequence();
			fclose(fp);
			return;
		}

		
		ret = fseek(fp, 0, SEEK_SET);
		if (ret == 0)
			ret = fread(&ndsHeader, sizeof (ndsHeader), 1, fp); // read if seek succeed
		else
			ret = 0; // if seek fails set to !=1

		if (ret != 1)
		{
			fclose(fp);
			return;
		}

		if ((ndsHeader.unitCode == 0x03 && ndsHeader.arm7binarySize > 0x20000)
		|| (ndsHeader.unitCode == 0x03 && ndsHeader.gameCode[0] == 0x48)
		|| (ndsHeader.unitCode == 0x03 && ndsHeader.arm7binarySize == 0x151BC)) {
			isDSiWare = true;	// Is a DSi-Exclusive/DSiWare game
		} else if ((ndsHeader.unitCode >= 0x02
		&& ndsHeader.arm9romOffset == 0x4000 && ndsHeader.arm7binarySize < 0x20000)
		|| (ndsHeader.arm9romOffset == 0x200 && ndsHeader.arm7destination == 0x02380000)) {
			isHomebrew = 2;		// Homebrew is recent (may have DSi-extended header)
		} else if (ndsHeader.arm7executeAddress >= 0x037F0000 && ndsHeader.arm7destination >= 0x037F0000) {
			isHomebrew = 1;		// Homebrew has no DSi-extended header
		}

		if (ndsHeader.dsi_flags == 0x10) bnrWirelessIcon = 1;
		else if (ndsHeader.dsi_flags == 0x0B) bnrWirelessIcon = 2;

		if (ndsHeader.bannerOffset == 0)
		{
			fclose(fp);
			return;
		}
		ret = fseek(fp, ndsHeader.bannerOffset, SEEK_SET);
		if (ret == 0)
			ret = fread(&ndsBanner, sizeof (ndsBanner), 1, fp); // read if seek succeed
		else
			ret = 0; // if seek fails set to !=1

		if (ret != 1)
		{
			fclose(fp);
			return;
		}

		// close file!
		fclose(fp);

		loadFixedBanner();

		// banner sequence
		DC_FlushAll();

		if(animateDsiIcons && ndsBanner.version == NDS_BANNER_VER_DSi) {
			grabBannerSequence();
			bnriconisDSi = true;
		}
	}
}

void iconUpdate(bool isDir, const char* name)
{
	clearText(false);

	if (isDir)
	{
		// icon
		clearIcon();
	}
	else if ((strlen(name) >= 5 && strcasecmp(name + strlen(name) - 5, ".argv") == 0)
		|| (strlen(name) >= 5 && strcasecmp(name + strlen(name) - 5, ".ARGV") == 0)
		|| (strlen(name) >= 10 && strcasecmp(name + strlen(name) - 10, ".launcharg") == 0)
		|| (strlen(name) >= 10 && strcasecmp(name + strlen(name) - 10, ".LAUNCHARG") == 0))
	{
		// look through the argv file for the corresponding nds/app file
		FILE *fp;
		char *line = NULL, *p = NULL;
		size_t size = 0;
		ssize_t rc;

		// open the argv file
		fp = fopen(name, "rb");
		if (fp == NULL)
		{
			clearIcon();
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

		if ((strlen(name) >= 10 && strcasecmp(name + strlen(name) - 10, ".launcharg") == 0)
		|| (strlen(name) >= 10 && strcasecmp(name + strlen(name) - 10, ".LAUNCHARG") == 0))
		{
			// Search for .app
			char appPath[256];
			for (u8 appVer = 0; appVer <= 0xFF; appVer++)
			{
				if (appVer > 0xF) {
					snprintf(appPath, sizeof(appPath), "%scontent/000000%x.app", p, appVer);
				} else {
					snprintf(appPath, sizeof(appPath), "%scontent/0000000%x.app", p, appVer);
				}
				if (access(appPath, F_OK) == 0)
				{
					p = appPath;
					break;
				}
				if (appVer == 0xFF) {
					p = NULL;
				}
			}
		}

		if (p && *p)
		{
			// we found an argument
			struct stat st;

			// truncate everything after first argument
			strtok(p, "\n\r\t ");

			if ((strlen(p) >= 4 && strcasecmp(p + strlen(p) - 4, ".nds") == 0)
			|| (strlen(p) >= 4 && strcasecmp(p + strlen(p) - 4, ".NDS") == 0)
			|| (strlen(p) >= 4 && strcasecmp(p + strlen(p) - 4, ".dsi") == 0)
			|| (strlen(p) >= 4 && strcasecmp(p + strlen(p) - 4, ".DSI") == 0)
			|| (strlen(p) >= 4 && strcasecmp(p + strlen(p) - 4, ".ids") == 0)
			|| (strlen(p) >= 4 && strcasecmp(p + strlen(p) - 4, ".IDS") == 0)
			|| (strlen(p) >= 4 && strcasecmp(p + strlen(p) - 4, ".app") == 0)
			|| (strlen(p) >= 4 && strcasecmp(p + strlen(p) - 4, ".APP") == 0))
			{
				// let's see if this is a file or directory
				rc = stat(p, &st);
				if (rc != 0)
				{
					// stat failed
					clearIcon();
				}
				else if (S_ISDIR(st.st_mode))
				{
					// this is a directory!
					clearIcon();
				}
				else
				{
					iconUpdate(false, p);
				}
			}
			else
			{
				// this is not an nds/app file!
				clearIcon();
			}
		}
		else
		{
			clearIcon();
		}
		// clean up the allocated line
		free(line);
	}
	else if ((strlen(name) >= 4 && strcasecmp(name + strlen(name) - 4, ".nds") == 0)
			|| (strlen(name) >= 4 && strcasecmp(name + strlen(name) - 4, ".NDS") == 0)
			|| (strlen(name) >= 4 && strcasecmp(name + strlen(name) - 4, ".dsi") == 0)
			|| (strlen(name) >= 4 && strcasecmp(name + strlen(name) - 4, ".DSI") == 0)
			|| (strlen(name) >= 4 && strcasecmp(name + strlen(name) - 4, ".ids") == 0)
			|| (strlen(name) >= 4 && strcasecmp(name + strlen(name) - 4, ".IDS") == 0)
			|| (strlen(name) >= 4 && strcasecmp(name + strlen(name) - 4, ".app") == 0)
			|| (strlen(name) >= 4 && strcasecmp(name + strlen(name) - 4, ".APP") == 0))
	{
		// this is an nds/app file!
		FILE *fp;
		unsigned int iconTitleOffset;
		int ret;

		// open file for reading info
		fp = fopen(name, "rb");
		if (fp == NULL)
		{
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

		if (ret != 1)
		{
			// icon
			loadUnkIcon();
			fclose(fp);
			return;
		}

		if (iconTitleOffset == 0)
		{
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

		if (ret != 1)
		{
			// try again, but using regular banner size
			ret = fseek(fp, iconTitleOffset, SEEK_SET);
			if (ret == 0)
				ret = fread(&ndsBanner, NDS_BANNER_SIZE_ORIGINAL, 1, fp); // read if seek succeed
			else
				ret = 0; // if seek fails set to !=1

			if (ret != 1)
			{
				// icon
				loadUnkIcon();
				fclose(fp);
				return;
			}
		}

		// close file!
		fclose(fp);

		loadFixedBanner();

		// icon
		DC_FlushAll();
		if(animateDsiIcons && ndsBanner.version == NDS_BANNER_VER_DSi) {
			loadIcon(ndsBanner.dsi_icon[0], ndsBanner.dsi_palette[0], true);
		} else {
			loadIcon(ndsBanner.icon, ndsBanner.palette, false);
		}
	}
}

void titleUpdate(bool isDir, const char* name)
{
	clearText(false);
	
	if (isDir)
	{
		// text
		if (strcmp(name, "..") == 0) {
			writeBannerText(0, "Back", "", "");
		} else {
			writeBannerText(0, name, "", "");
		}
	}
	else if (strcasecmp(name + strlen(name) - 3, ".gb") == 0 ||
				strcasecmp (name + strlen(name) - 4, ".GB") == 0 ||
				strcasecmp (name + strlen(name) - 4, ".sgb") == 0 ||
				strcasecmp (name + strlen(name) - 4, ".SGB") == 0 ||
				strcasecmp (name + strlen(name) - 4, ".gbc") == 0 ||
				strcasecmp (name + strlen(name) - 4, ".GBC") == 0 ||
				strcasecmp (name + strlen(name) - 4, ".nes") == 0 ||
				strcasecmp (name + strlen(name) - 4, ".NES") == 0 ||
				strcasecmp (name + strlen(name) - 4, ".fds") == 0 ||
				strcasecmp (name + strlen(name) - 4, ".FDS") == 0  )
	{
		writeBannerText(0, name, "", "");
	}
	else if ((strlen(name) >= 5 && strcasecmp(name + strlen(name) - 5, ".argv") == 0)
		|| (strlen(name) >= 5 && strcasecmp(name + strlen(name) - 5, ".ARGV") == 0)
		|| (strlen(name) >= 10 && strcasecmp(name + strlen(name) - 10, ".launcharg") == 0)
		|| (strlen(name) >= 10 && strcasecmp(name + strlen(name) - 10, ".LAUNCHARG") == 0))
	{
		// look through the argv file for the corresponding nds/app file
		FILE *fp;
		char *line = NULL, *p = NULL;
		size_t size = 0;
		ssize_t rc;

		// open the argv file
		fp = fopen(name, "rb");
		if (fp == NULL)
		{
			writeBannerText(0, "(can't open file!)", "", "");
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

		if ((strlen(name) >= 10 && strcasecmp(name + strlen(name) - 10, ".launcharg") == 0)
		|| (strlen(name) >= 10 && strcasecmp(name + strlen(name) - 10, ".LAUNCHARG") == 0))
		{
			// Search for .app
			char appPath[256];
			for (u8 appVer = 0; appVer <= 0xFF; appVer++)
			{
				if (appVer > 0xF) {
					snprintf(appPath, sizeof(appPath), "%scontent/000000%x.app", p, appVer);
				} else {
					snprintf(appPath, sizeof(appPath), "%scontent/0000000%x.app", p, appVer);
				}
				if (access(appPath, F_OK) == 0)
				{
					p = appPath;
					break;
				}
				if (appVer == 0xFF) {
					p = NULL;
				}
			}
		}

		if (p && *p)
		{
			// we found an argument
			struct stat st;

			// truncate everything after first argument
			strtok(p, "\n\r\t ");

			if ((strlen(p) >= 4 && strcasecmp(p + strlen(p) - 4, ".nds") == 0)
			|| (strlen(p) >= 4 && strcasecmp(p + strlen(p) - 4, ".NDS") == 0)
			|| (strlen(p) >= 4 && strcasecmp(p + strlen(p) - 4, ".dsi") == 0)
			|| (strlen(p) >= 4 && strcasecmp(p + strlen(p) - 4, ".DSI") == 0)
			|| (strlen(p) >= 4 && strcasecmp(p + strlen(p) - 4, ".ids") == 0)
			|| (strlen(p) >= 4 && strcasecmp(p + strlen(p) - 4, ".IDS") == 0)
			|| (strlen(p) >= 4 && strcasecmp(p + strlen(p) - 4, ".app") == 0)
			|| (strlen(p) >= 4 && strcasecmp(p + strlen(p) - 4, ".APP") == 0))
			{
				// let's see if this is a file or directory
				rc = stat(p, &st);
				if (rc != 0)
				{
					// stat failed
					writeBannerText(0, "(can't find argument!)", "", "");
				}
				else if (S_ISDIR(st.st_mode))
				{
					// this is a directory!
					writeBannerText(1, "(invalid argv file!)", "This is a directory.", "");
				}
				else
				{
					titleUpdate(false, p);
				}
			}
			else
			{
				// this is not an nds/app file!
				writeBannerText(1, "(invalid argv file!)", "No .nds/.app file.", "");
			}
		}
		else
		{
			writeBannerText(0, "(no argument!)", "", "");
		}
		// clean up the allocated line
		free(line);
	}
	else if ((strlen(name) >= 4 && strcasecmp(name + strlen(name) - 4, ".nds") == 0)
			|| (strlen(name) >= 4 && strcasecmp(name + strlen(name) - 4, ".NDS") == 0)
			|| (strlen(name) >= 4 && strcasecmp(name + strlen(name) - 4, ".dsi") == 0)
			|| (strlen(name) >= 4 && strcasecmp(name + strlen(name) - 4, ".DSI") == 0)
			|| (strlen(name) >= 4 && strcasecmp(name + strlen(name) - 4, ".ids") == 0)
			|| (strlen(name) >= 4 && strcasecmp(name + strlen(name) - 4, ".IDS") == 0)
			|| (strlen(name) >= 4 && strcasecmp(name + strlen(name) - 4, ".app") == 0)
			|| (strlen(name) >= 4 && strcasecmp(name + strlen(name) - 4, ".APP") == 0))
	{
		// this is an nds/app file!
		FILE *fp;
		unsigned int iconTitleOffset;
		int ret;

		// open file for reading info
		fp = fopen(name, "rb");
		if (fp == NULL)
		{
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

		if (ret != 1)
		{
			// text
			writeBannerText(0, "(can't read file!)", "", "");
			fclose(fp);
			return;
		}

		if (iconTitleOffset == 0)
		{
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

		if (ret != 1)
		{
			// try again, but using regular banner size
			ret = fseek(fp, iconTitleOffset, SEEK_SET);
			if (ret == 0)
				ret = fread(&ndsBanner, NDS_BANNER_SIZE_ORIGINAL, 1, fp); // read if seek succeed
			else
				ret = 0; // if seek fails set to !=1

			if (ret != 1)
			{
				// text
				writeBannerText(1, name, "(can't read icon/title!)", "");
				fclose(fp);
				return;
			}
		}

		// close file!
		fclose(fp);

		loadFixedBanner();

		// turn unicode into ascii (kind of)
		// and convert 0x0A into 0x00
		char *p = (char*) ndsBanner.titles[setGameLanguage];
		int bannerlines = 0;
		for (unsigned int i = 0; i < sizeof (ndsBanner.titles[setGameLanguage]); i += 2)
		{
			if ((p[i] == 0x0A) || (p[i] == 0xFF)) {
				p[i / 2] = 0;
				bannerlines++;
			} else if (p[i] == 0xE9) {
				p[i / 2] = 0x65;	// Replace bugged "�" with regular "e"
			} else {
				p[i / 2] = p[i];
			}
		}

		// text
		switch(bannerlines) {
			case 0:
			default:
				printSmall(false, BOX_PX, BOX_PY+BOX_PY_spacing1, p);
				break;
			case 1:
				printSmall(false, BOX_PX, BOX_PY+BOX_PY_spacing2, p);
				p += strlen(p) + 1;
				printSmall(false, BOX_PX, BOX_PY+BOX_PY_spacing3, p);
				break;
			case 2:
				printSmall(false, BOX_PX, BOX_PY, p);
				p += strlen(p) + 1;
				printSmall(false, BOX_PX, BOX_PY+BOX_PY_spacing1, p);
				p += strlen(p) + 1;
				printSmall(false, BOX_PX, BOX_PY+BOX_PY_spacing1*2, p);
				break;
		}
		
	}
}
