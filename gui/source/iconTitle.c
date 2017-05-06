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

#include "srloader_banner.h"

#include "dsmenu_top.h"
#include "threedshome_top.h"
#include "font6x8.h"

// DS Menu theme
#define TITLE_POS_X	(14*8)
#define TITLE_POS_Y	(10*8)

#define ICON_POS_X	40
#define ICON_POS_Y	80

// 3DS HOME Menu theme
#define TITLE_POS_X2	(11*8)
#define TITLE_POS_Y2	(16*8)

#define ICON_POS_X2	112
#define ICON_POS_Y2	56


#define TEXT_WIDTH	((20-4)*8/6)

static int bg2, bg3;
static u16 *sprite;
static tNDSBanner banner;

static inline void writecharRS (int row, int col, u16 car) {
	// get map pointer
	u16 *gfx   = bgGetMapPtr(bg2);
	// get old pair of values from VRAM
	u16 oldval = gfx[row*(512/8/2)+(col/2)];

	// clear the half we will update
	oldval &= (col%2) ? 0x00FF : 0xFF00;
	// apply the updated half
	oldval |= (col%2) ? (car<<8) : car;

	// write back to VRAM
	gfx[row*(512/8/2)+col/2] = oldval;
}

static inline void writeRow (int rownum, const char* text) {
	int i,len,p=0;
	len=strlen(text);

	if (len>TEXT_WIDTH)
		len=TEXT_WIDTH;

	// clear left part
	for (i=0;i<(TEXT_WIDTH-len)/2;i++)
		writecharRS (rownum, i, 0);

	// write centered text
	for (i=(TEXT_WIDTH-len)/2;i<((TEXT_WIDTH-len)/2+len);i++)
		writecharRS (rownum, i, text[p++]-' ');

	// clear right part
	for (i=((TEXT_WIDTH-len)/2+len);i<TEXT_WIDTH;i++)
		writecharRS (rownum, i, 0);
}

static inline void clearIcon (void) {
	dmaFillHalfWords(0, sprite, sizeof(banner.icon));
}

void bannerTitleInit (void) {
	// initialize video mode
	videoSetMode(MODE_4_2D);

	// initialize VRAM banks
	vramSetPrimaryBanks(VRAM_A_MAIN_BG,
	                    VRAM_B_MAIN_SPRITE,
	                    VRAM_C_LCD,
	                    VRAM_D_LCD);
						
	// initialize bg2 as a rotation background and bg3 as a bmp background
	// http://mtheall.com/vram.html#T2=3&RNT2=96&MB2=3&TB2=0&S2=2&T3=6&MB3=1&S3=1
	bg2 = bgInit(2, BgType_Rotation, BgSize_R_512x512,   3, 0);
	bg3 = bgInit(3, BgType_Bmp16,    BgSize_B16_256x256, 1, 0);

	// initialize rotate, scale, and scroll
	bgSetRotateScale(bg3, 0, 1<<8, 1<<8);
	bgSetScroll(bg3, 0, 0);
	bgSetRotateScale(bg2, 0, 8*(1<<8)/6, 1<<8);
	bgSetScroll(bg2, -TITLE_POS_X, -TITLE_POS_Y);

	// clear bg2's map: 512x512 pixels is 64x64 tiles is 4KB
	dmaFillHalfWords(0, bgGetMapPtr(bg2), 4096);
	// load compressed font into bg2's tile data
	decompress(font6x8Tiles, bgGetGfxPtr(bg2), LZ77Vram);

	// load compressed bitmap into bg3
	decompress(srloader_bannerBitmap, bgGetGfxPtr(bg3), LZ77Vram);
	// decompress(sub_bgBitmap, bgGetGfxPtr(bg1), LZ77Vram);

	// load font palette
	dmaCopy(font6x8Pal, BG_PALETTE, font6x8PalLen);

	// apply the bg changes
	bgUpdate();

	// initialize OAM
	oamInit(&oamMain, SpriteMapping_1D_128, false);
	sprite = oamAllocateGfx(&oamMain, SpriteSize_32x32, SpriteColorFormat_16Color);
	dmaFillHalfWords(0, sprite, sizeof(banner.icon));
	oamSet(&oamMain, 0, ICON_POS_X, ICON_POS_Y, 0, 0,
	       SpriteSize_32x32, SpriteColorFormat_16Color, sprite,
	       -1, 0, 0, 0, 0, 0);

	// oam can only be updated during vblank
	swiWaitForVBlank();
	oamUpdate(&oamMain);
}


void iconTitleInit (int theme) {
	// initialize video mode
	videoSetMode(MODE_4_2D);

	// initialize VRAM banks
	vramSetPrimaryBanks(VRAM_A_MAIN_BG,
	                    VRAM_B_MAIN_SPRITE,
	                    VRAM_C_LCD,
	                    VRAM_D_LCD);
						
	// initialize bg2 as a rotation background and bg3 as a bmp background
	// http://mtheall.com/vram.html#T2=3&RNT2=96&MB2=3&TB2=0&S2=2&T3=6&MB3=1&S3=1
	bg2 = bgInit(2, BgType_Rotation, BgSize_R_512x512,   3, 0);
	bg3 = bgInit(3, BgType_Bmp16,    BgSize_B16_256x256, 1, 0);

	// initialize rotate, scale, and scroll
	bgSetRotateScale(bg3, 0, 1<<8, 1<<8);
	bgSetScroll(bg3, 0, 0);
	bgSetRotateScale(bg2, 0, 8*(1<<8)/6, 1<<8);
	switch(theme) {
		case 0:
		default:
			bgSetScroll(bg2, -TITLE_POS_X, -TITLE_POS_Y);
			break;
		case 1:
			bgSetScroll(bg2, -TITLE_POS_X2, -TITLE_POS_Y2);
			break;
	}

	// clear bg2's map: 512x512 pixels is 64x64 tiles is 4KB
	dmaFillHalfWords(0, bgGetMapPtr(bg2), 4096);
	// load compressed font into bg2's tile data
	decompress(font6x8Tiles, bgGetGfxPtr(bg2), LZ77Vram);

	// load compressed bitmap into bg3
	switch(theme) {
		case 0:
		default:
			decompress(dsmenu_topBitmap, bgGetGfxPtr(bg3), LZ77Vram);
			break;
		case 1:
			decompress(threedshome_topBitmap, bgGetGfxPtr(bg3), LZ77Vram);
			break;
	}
	// decompress(sub_bgBitmap, bgGetGfxPtr(bg1), LZ77Vram);

	// load font palette
	dmaCopy(font6x8Pal, BG_PALETTE, font6x8PalLen);

	// apply the bg changes
	bgUpdate();

	// initialize OAM
	oamInit(&oamMain, SpriteMapping_1D_128, false);
	sprite = oamAllocateGfx(&oamMain, SpriteSize_32x32, SpriteColorFormat_16Color);
	dmaFillHalfWords(0, sprite, sizeof(banner.icon));
	switch(theme) {
		case 0:
		default:
			oamSet(&oamMain, 0, ICON_POS_X, ICON_POS_Y, 0, 0,
				   SpriteSize_32x32, SpriteColorFormat_16Color, sprite,
				   -1, 0, 0, 0, 0, 0);
			break;
		case 1:
			oamSet(&oamMain, 0, ICON_POS_X2, ICON_POS_Y2, 0, 0,
				   SpriteSize_32x32, SpriteColorFormat_16Color, sprite,
				   -1, 0, 0, 0, 0, 0);
			break;
	}

	// oam can only be updated during vblank
	swiWaitForVBlank();
	oamUpdate(&oamMain);

	// everything's ready :)
	writeRow (0,"Theme loaded.");
	// writeRow (0,"...initializing...");
	// writeRow (1,"==>>> SRLoader+ <<<==");
	// writeRow (2,"(This should vanish.");
	// writeRow (3,"Otherwise, trouble!)");
}


void iconTitleUpdate (int isdir, const char* name) {
	writeRow (0,name);
	writeRow (1,"");
	writeRow (2,"");
	writeRow (3,"");

	if (isdir) {
		// text
		writeRow (2,"[directory]");
		// icon
		clearIcon();
	} else if(strlen(name) >= 5 && strcasecmp(name + strlen(name) - 5, ".argv") == 0) {
		// look through the argv file for the corresponding nds file
		FILE    *fp;
		char    *line = NULL, *p = NULL;
		size_t  size = 0;
		ssize_t rc;

		// open the argv file
		fp = fopen(name,"rb");
		if(fp == NULL) {
			writeRow(2, "(can't open file!)");
			clearIcon();
			fclose(fp); return;
		}

		// read each line
		while((rc = __getline(&line, &size, fp)) > 0) {
			// remove comments
			if((p = strchr(line, '#')) != NULL)
				*p = 0;

			// skip leading whitespace
			for(p = line; *p && isspace((int)*p); ++p)
			  ;

			if(*p)
				break;
		}

		// done with the file at this point
		fclose(fp);

		if(p && *p) {
			// we found an argument
			struct stat st;

			// truncate everything after first argument
			strtok(p, "\n\r\t ");

			if(strlen(p) < 4 || strcasecmp(p + strlen(p) - 4, ".nds") != 0) {
				// this is not an nds file!
				writeRow(2, "(invalid argv file!)");
				clearIcon();
			} else {
				// let's see if this is a file or directory
				rc = stat(p, &st);
				if(rc != 0) {
					// stat failed
					writeRow(2, "(can't find argument!)");
					clearIcon();
				} else if(S_ISDIR(st.st_mode)) {
					// this is a directory!
					writeRow(2, "(invalid argv file!)");
					clearIcon();
				} else {
					iconTitleUpdate(false, p);
				}
			}
		} else {
			writeRow(2, "(no argument!)");
			clearIcon();
		}
		// clean up the allocated line
		free(line);
	} else {
		// this is an nds file!
		FILE *fp;
		unsigned int Icon_title_offset;
		int ret;

		// open file for reading info
		fp=fopen (name,"rb");
		if (fp==NULL) {
			// text
			writeRow (2,"(can't open file!)");
			// icon
			clearIcon();
			fclose (fp); return;
		}

		ret=fseek (fp, offsetof(tNDSHeader, bannerOffset), SEEK_SET);
		if (ret==0)
			ret=fread (&Icon_title_offset, sizeof(int), 1, fp); // read if seek succeed
		else
			ret=0;  // if seek fails set to !=1

		if (ret!=1) {
			// text
			writeRow (2,"(can't read file!)");
			// icon
			clearIcon();
			fclose (fp); return;
		}

		if (Icon_title_offset==0) {
			// text
			writeRow (2,"(no title/icon)");
			// icon
			clearIcon();
			fclose (fp); return;
		}

		ret=fseek (fp,Icon_title_offset,SEEK_SET);
		if (ret==0)
			ret=fread (&banner, sizeof(banner), 1, fp); // read if seek succeed
		else
			ret=0;  // if seek fails set to !=1

		if (ret!=1) {
			// text
			writeRow (2,"(can't read icon/title!)");
			// icon
			clearIcon();
			fclose (fp); return;
		}

		// close file!
		fclose (fp);

		// turn unicode into ascii (kind of)
		// and convert 0x0A into 0x00
		int i;
		char *p = (char*)banner.titles[0];
		for (i = 0; i < sizeof(banner.titles[0]); i = i+2) {
			if ((p[i] == 0x0A) || (p[i] == 0xFF))
				p[i/2] = 0;
			else
				p[i/2] = p[i];
		}

		// text
		for(i = 0; i < 3; ++i) {
			writeRow (i+1, p);
			p += strlen(p)+1;
		}

		// icon
		DC_FlushAll();
		dmaCopy(banner.icon,    sprite,         sizeof(banner.icon));
		dmaCopy(banner.palette, SPRITE_PALETTE, sizeof(banner.palette));
	}
}
