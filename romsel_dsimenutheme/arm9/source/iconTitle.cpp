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
#include "graphics/fontHandler.h"
#include "ndsheaderbanner.h"

#define ICON_POS_X	112
#define ICON_POS_Y	96

static int BOX_PY = 13;
static int BOX_PY_spacing1 = 19;
static int BOX_PY_spacing2 = 9;
static int BOX_PY_spacing3 = 28;

// Graphic files
#include "icon_unk.h"
#include "icon_gb.h"
#include "icon_gbc.h"
#include "icon_nes.h"

extern int theme;

static int iconTexID[6][8];
static int gbTexID;
static int gbcTexID;
static int nesTexID;
sNDSHeaderExt ndsHeader;
sNDSBannerExt ndsBanner;

static glImage ndsIcon[6][8][(32 / 32) * (256 / 32)];

static glImage gbIcon[1];
static glImage gbcIcon[1];
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
			printLargeCentered(false, BOX_PY+BOX_PY_spacing1, text1);
			break;
		case 1:
			printLargeCentered(false, BOX_PY+BOX_PY_spacing2, text1);
			printLargeCentered(false, BOX_PY+BOX_PY_spacing3, text2);
			break;
		case 2:
			printLargeCentered(false, BOX_PY, text1);
			printLargeCentered(false, BOX_PY+BOX_PY_spacing1, text2);
			printLargeCentered(false, BOX_PY+BOX_PY_spacing1*2, text3);
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

void loadIcon(u8 *tilesSrc, u16 *palSrc, int num, bool twl)//(u8(*tilesSrc)[(32 * 32) / 2], u16(*palSrc)[16])
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
				32, // bitmap image height
				GL_RGB16, // texture type for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeX for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeY for glTexImage2D() in videoGL.h
				GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T | TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT,
				16, // Length of the palette to use (16 colors)
				(u16*) icon_gbPal, // Image palette
				(u8*) icon_gbBitmap // Raw image data
				);

	glDeleteTextures(1, &gbcTexID);

	gbcTexID =
	glLoadTileSet(gbcIcon, // pointer to glImage array
				32, // sprite width
				32, // sprite height
				32, // bitmap image width
				32, // bitmap image height
				GL_RGB16, // texture type for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeX for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeY for glTexImage2D() in videoGL.h
				GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T | TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT,
				16, // Length of the palette to use (16 colors)
				(u16*) icon_gbcPal, // Image palette
				(u8*) icon_gbcBitmap // Raw image data
				);
}
void loadNESIcon()
{
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

static void clearIcon(int num)
{
	loadIcon(clearTiles, blackPalette, num, true);
}

void drawIcon(int Xpos, int Ypos, int num)
{
	int num2 = num;
	if(num >= 36) {
		num2 -= 36;
	} else if(num2 >= 30) {
		num2 -= 30;
	} else if(num2 >= 24) {
		num2 -= 24;
	} else if(num2 >= 18) {
		num2 -= 18;
	} else if(num2 >= 12) {
		num2 -= 12;
	} else if(num2 >= 6) {
		num2 -= 6;
	}
	glSprite(Xpos, Ypos, bannerFlip[num], &ndsIcon[num2][bnriconPalLine[num]][bnriconframenumY[num] & 31]);
}

void drawIconGBC(int Xpos, int Ypos, int num)
{
	if(isGBC[num]) {
		glSprite(Xpos, Ypos, GL_FLIP_NONE, gbcIcon);
	} else {
		glSprite(Xpos, Ypos, GL_FLIP_NONE, gbIcon);
	}
}
void drawIconNES(int Xpos, int Ypos)
{
	glSprite(Xpos, Ypos, GL_FLIP_NONE, nesIcon);
}

void getGameInfo(bool isDir, const char* name, int num)
{
	bnriconPalLine[num] = 0;
	bnriconframenumY[num] = 0;
	bannerFlip[num] = GL_FLIP_NONE;
	bnriconisDSi[num] = false;
	launchable[num] = true;
	isHomebrew[num] = false;

	if (isDir)
	{
		// banner sequence
		clearBannerSequence(num);
	}
	else if (strlen(name) >= 5 && strcasecmp(name + strlen(name) - 5, ".argv") == 0)
	{
		// look through the argv file for the corresponding nds file
		FILE *fp;
		char *line = NULL, *p = NULL;
		// size_t size = 0;
		ssize_t rc;

		// open the argv file
		fp = fopen(name, "rb");
		if (fp == NULL)
		{
			clearBannerSequence(num);
			fclose(fp);
			return;
		}

		// done with the file at this point
		fclose(fp);

		if (p && *p)
		{
			// we found an argument
			struct stat st;

			// truncate everything after first argument
			strtok(p, "\n\r\t ");

			if (strlen(p) < 4 || strcasecmp(p + strlen(p) - 4, ".nds") != 0)
			{
				// this is not an nds file!
				clearBannerSequence(num);
			}
			else
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
					getGameInfo(false, p, num);
				}
			}
		}
		else
		{
			clearBannerSequence(num);
		}
		// clean up the allocated line
		free(line);
	}
	else
	{
		// this is an nds file!
		FILE *fp;
		int ret;

		// open file for reading info
		fp = fopen(name, "rb");
		if (fp == NULL)
		{
			// banner sequence
			clearBannerSequence(num);
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

		if(ndsHeader.unitCode == 0x03 && strcmp(ndsHeader.gameCode, "####") != 0) {
			launchable[num] = false;	// Make DSi-Exclusive/DSiWare game unlaunchable
		} else if(ndsHeader.unitCode == 0x02 || ndsHeader.unitCode == 0x03) {
			if(ndsHeader.arm9romOffset == 0x4000 && strcmp(ndsHeader.gameCode, "####") == 0)
				isHomebrew[num] = true;	// If homebrew has DSi-extended header,
											// do not use bootstrap/flashcard's ROM booter to boot it
		}

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

		// banner sequence
		DC_FlushAll();

		if(ndsBanner.version == NDS_BANNER_VER_DSi) {
			grabBannerSequence(num);
			bnriconisDSi[num] = true;
		}
	}
}

void iconUpdate(bool isDir, const char* name, int num)
{
	if(num >= 36) {
		num -= 36;
	} else if(num >= 30) {
		num -= 30;
	} else if(num >= 24) {
		num -= 24;
	} else if(num >= 18) {
		num -= 18;
	} else if(num >= 12) {
		num -= 12;
	} else if(num >= 6) {
		num -= 6;
	}
	clearText(false);

	if (isDir)
	{
		// icon
		clearIcon(num);
	}
	else if (strlen(name) >= 5 && strcasecmp(name + strlen(name) - 5, ".argv") == 0)
	{
		// look through the argv file for the corresponding nds file
		FILE *fp;
		char *line = NULL, *p = NULL;
		// size_t size = 0;
		ssize_t rc;

		// open the argv file
		fp = fopen(name, "rb");
		if (fp == NULL)
		{
			clearIcon(num);
			fclose(fp);
			return;
		}

		// done with the file at this point
		fclose(fp);

		if (p && *p)
		{
			// we found an argument
			struct stat st;

			// truncate everything after first argument
			strtok(p, "\n\r\t ");

			if (strlen(p) < 4 || strcasecmp(p + strlen(p) - 4, ".nds") != 0)
			{
				// this is not an nds file!
				clearIcon(num);
			}
			else
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
					iconUpdate(false, p, num);
				}
			}
		}
		else
		{
			clearIcon(num);
		}
		// clean up the allocated line
		free(line);
	}
	else
	{
		// this is an nds file!
		FILE *fp;
		unsigned int iconTitleOffset;
		int ret;

		// open file for reading info
		fp = fopen(name, "rb");
		if (fp == NULL)
		{
			// icon
			clearIcon(num);
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
			loadUnkIcon(num);
			fclose(fp);
			return;
		}

		if (iconTitleOffset == 0)
		{
			// icon
			loadUnkIcon(num);
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
				loadUnkIcon(num);
				fclose(fp);
				return;
			}
		}

		// close file!
		fclose(fp);

		// icon
		DC_FlushAll();
		if(ndsBanner.version == NDS_BANNER_VER_DSi) {
			loadIcon(ndsBanner.dsi_icon[0], ndsBanner.dsi_palette[0], num, true);
		} else {
			loadIcon(ndsBanner.icon, ndsBanner.palette, num, false);
		}
	}
}

void titleUpdate(bool isDir, const char* name)
{
	clearText(false);
	if (theme == 1) {
		BOX_PY = 39;
		BOX_PY_spacing1 = 17;
		BOX_PY_spacing2 = 7;
		BOX_PY_spacing3 = 26;
	}

	if (isDir)
	{
		// text
		writeBannerText(1, name, "[directory]", "");
	}
	else if (strcasecmp(name + strlen(name) - 3, ".gb") == 0 ||
				strcasecmp (name + strlen(name) - 4, ".sgb") == 0 ||
				strcasecmp (name + strlen(name) - 4, ".gbc") == 0 ||
				strcasecmp (name + strlen(name) - 4, ".nes") == 0 ||
				strcasecmp (name + strlen(name) - 4, ".fds") == 0  )
	{
		writeBannerText(0, name, "", "");
	}
	else if (strlen(name) >= 5 && strcasecmp(name + strlen(name) - 5, ".argv") == 0)
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

		if (p && *p)
		{
			// we found an argument
			struct stat st;

			// truncate everything after first argument
			strtok(p, "\n\r\t ");

			if (strlen(p) < 4 || strcasecmp(p + strlen(p) - 4, ".nds") != 0)
			{
				// this is not an nds file!
				writeBannerText(0, "(invalid argv file!)", "", "");
			}
			else
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
					writeBannerText(0, "(invalid argv file!)", "", "");
				}
				else
				{
					titleUpdate(false, p);
				}
			}
		}
		else
		{
			writeBannerText(0, "(no argument!)", "", "");
		}
		// clean up the allocated line
		free(line);
	}
	else
	{
		// this is an nds file!
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

		// turn unicode into ascii (kind of)
		// and convert 0x0A into 0x00
		char *p = (char*) ndsBanner.titles[PersonalData->language];
		int bannerlines = 0;
		for (unsigned int i = 0; i < sizeof (ndsBanner.titles[PersonalData->language]); i += 2)
		{
			if ((p[i] == 0x0A) || (p[i] == 0xFF)) {
				p[i / 2] = 0;
				bannerlines++;
			} else if (p[i] == 0xE9) {
				p[i / 2] = 0x65;	// Replace bugged "é" with regular "e"
			} else {
				p[i / 2] = p[i];
			}
		}

		// text
		switch(bannerlines) {
			case 0:
			default:
				printLargeCentered(false, BOX_PY+BOX_PY_spacing1, p);
				break;
			case 1:
				printLargeCentered(false, BOX_PY+BOX_PY_spacing2, p);
				p += strlen(p) + 1;
				printLargeCentered(false, BOX_PY+BOX_PY_spacing3, p);
				break;
			case 2:
				printLargeCentered(false, BOX_PY, p);
				p += strlen(p) + 1;
				printLargeCentered(false, BOX_PY+BOX_PY_spacing1, p);
				p += strlen(p) + 1;
				printLargeCentered(false, BOX_PY+BOX_PY_spacing1*2, p);
				break;
		}
		
	}
}
