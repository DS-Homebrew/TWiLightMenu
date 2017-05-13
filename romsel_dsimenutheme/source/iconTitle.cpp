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

#define ICON_POS_X	112
#define ICON_POS_Y	100

#define BOX_PY (2*8)

static int iconTexID;
static tNDSBanner banner;

static glImage icon[1];

u8 *clearTiles;
u16 *blackPalette;
u8 *tilesModified;

void iconTitleInit()
{
	clearTiles = new u8[(32 * 32) / 2]();
	blackPalette = new u16[16]();
	tilesModified = new u8[(32 * 32) / 2];
}

static inline void writeRow(int rownum, const char* text)
{
	printLargeCentered(false, BOX_PY + 15 * rownum, text);
}

static void convertIconTilesToRaw(u8 *tilesSrc, u8 *tilesNew)
{
	const int PY = 32;
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

void loadIcon(u8 *tilesSrc, u16 *palSrc)//(u8(*tilesSrc)[(32 * 32) / 2], u16(*palSrc)[16])
{
	convertIconTilesToRaw(tilesSrc, tilesModified);

	glDeleteTextures(1, &iconTexID);
	iconTexID =
			glLoadTileSet(icon, // pointer to glImage array
						32, // sprite width
						32, // sprite height
						32, // bitmap image width
						32, // bitmap image height
						GL_RGB16, // texture type for glTexImage2D() in videoGL.h
						TEXTURE_SIZE_32, // sizeX for glTexImage2D() in videoGL.h
						TEXTURE_SIZE_32, // sizeY for glTexImage2D() in videoGL.h
						GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T | TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT,
						16, // Length of the palette to use (16 colors)
						(u16*) palSrc, // Image palette
						(u8*) tilesModified // Raw image data
						);
}

static void clearIcon(void)
{
	loadIcon(clearTiles, blackPalette);
}

void drawIcon()
{
	glSprite(ICON_POS_X, ICON_POS_Y, GL_FLIP_NONE, icon);
}

void iconTitleUpdate(bool isDir, const char* name)
{
	clearText(false);

	if (isDir)
	{
		writeRow(0, name);
		// text
		writeRow(2, "[directory]");
		// icon
		clearIcon();
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
			writeRow(2, "(can't open file!)");
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

		if (p && *p)
		{
			// we found an argument
			struct stat st;

			// truncate everything after first argument
			strtok(p, "\n\r\t ");

			if (strlen(p) < 4 || strcasecmp(p + strlen(p) - 4, ".nds") != 0)
			{
				// this is not an nds file!
				writeRow(2, "(invalid argv file!)");
				clearIcon();
			}
			else
			{
				// let's see if this is a file or directory
				rc = stat(p, &st);
				if (rc != 0)
				{
					// stat failed
					writeRow(2, "(can't find argument!)");
					clearIcon();
				}
				else if (S_ISDIR(st.st_mode))
				{
					// this is a directory!
					writeRow(2, "(invalid argv file!)");
					clearIcon();
				}
				else
				{
					iconTitleUpdate(false, p);
				}
			}
		}
		else
		{
			writeRow(2, "(no argument!)");
			clearIcon();
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
			writeRow(2, "(can't open file!)");
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
			// text
			writeRow(2, "(can't read file!)");
			// icon
			clearIcon();
			fclose(fp);
			return;
		}

		if (iconTitleOffset == 0)
		{
			writeRow(0, name);
			// text
			writeRow(2, "(no title/icon)");
			// icon
			clearIcon();
			fclose(fp);
			return;
		}
		ret = fseek(fp, iconTitleOffset, SEEK_SET);
		if (ret == 0)
			ret = fread(&banner, sizeof (banner), 1, fp); // read if seek succeed
		else
			ret = 0; // if seek fails set to !=1

		if (ret != 1)
		{
			// text
			writeRow(2, "(can't read icon/title!)");
			// icon
			clearIcon();
			fclose(fp);
			return;
		}

		// close file!
		fclose(fp);

		// turn unicode into ascii (kind of)
		// and convert 0x0A into 0x00
		char *p = (char*) banner.titles[0];
		for (unsigned int i = 0; i < sizeof (banner.titles[0]); i += 2)
		{
			if ((p[i] == 0x0A) || (p[i] == 0xFF))
				p[i / 2] = 0;
			else
				p[i / 2] = p[i];
		}

		// text
		for (int i = 0; i < 3; ++i)
		{
			writeRow(i, p);
			p += strlen(p) + 1;
		}

		// icon
		DC_FlushAll();
		loadIcon(banner.icon, banner.palette);
	}
}
