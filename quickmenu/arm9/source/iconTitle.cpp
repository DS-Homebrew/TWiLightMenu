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
#include "common/systemdetails.h"
#include "graphics/graphics.h"
#include "graphics/fontHandler.h"
#include "common/lodepng.h"
#include "common/logging.h"
#include "ndsheaderbanner.h"
#include "myDSiMode.h"
#include "language.h"
#include "read_card.h"

#include "extension.h"

#include "iconTitle.h"

#define ICON_POS_X	112
#define ICON_POS_Y	96

// Graphic files

#include "graphics/grit_tileset.h"

#include "icon_unk.h"
#include "icon_plg.h"
#include "icon_gba.h"
#include "icon_gb.h"
#include "icon_gbc.h"
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

static int iconTexID[2];
sNDSHeaderExt ndsHeader;
sNDSBannerExt ndsBanner;

#define TITLE_CACHE_SIZE 0x80

static bool infoFound[2] = {false};
static char16_t cachedTitle[2][TITLE_CACHE_SIZE];

static u32 arm9StartSig[4];

static glImage ndsIcon[2][(32 / 32) * (256 / 32)];
static u16 dsi_palette[2][8][16];
static u16 _paletteCache[2][16];

#define CONSOLE_ICON(name) GRIT_BITMAP(name, 32, 32, 32, 32, 1)

CONSOLE_ICON(icon_gba);
CONSOLE_ICON(icon_gb);
CONSOLE_ICON(icon_gbc);
CONSOLE_ICON(icon_nes);
CONSOLE_ICON(icon_sg);
CONSOLE_ICON(icon_sms);
CONSOLE_ICON(icon_gg);
CONSOLE_ICON(icon_snes);
CONSOLE_ICON(icon_plg);
CONSOLE_ICON(icon_a26);
CONSOLE_ICON(icon_col);
CONSOLE_ICON(icon_m5);
CONSOLE_ICON(icon_int);
CONSOLE_ICON(icon_pce);
CONSOLE_ICON(icon_ws);
CONSOLE_ICON(icon_ngp);
CONSOLE_ICON(icon_cpc);
CONSOLE_ICON(icon_vid);
CONSOLE_ICON(icon_img);
CONSOLE_ICON(icon_msx);
CONSOLE_ICON(icon_mini);
CONSOLE_ICON(icon_hb);
CONSOLE_ICON(icon_md);
CONSOLE_ICON(icon_unk);

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
	if (colorTable) {
		for (int i = 0; i < (twl ? 16*8 : 16); i++) {
			palSrc[i] = colorTable[palSrc[i] % 0x8000];
		}
	}
	if (twl) {
		for (int i = 0; i < 8; i++) {
			tonccpy(dsi_palette[num][i], palSrc+(16*i), 16*sizeof(u16));
		}
	}

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

template<typename T>
void loadIconTileset(T& icon, int num) {
	glDeleteTextures(1, &iconTexID[num]);
	
	iconTexID[num] = LoadTilesetTextureCachePalette(icon, ndsIcon[num], _paletteCache[num]);
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

	RemapPalette(icon_gba, colorTable);
	RemapPalette(icon_gb, colorTable);
	RemapPalette(icon_gbc, colorTable);
	RemapPalette(icon_nes, colorTable);
	RemapPalette(icon_sg, colorTable);
	RemapPalette(icon_sms, colorTable);
	RemapPalette(icon_gg, colorTable);
	RemapPalette(icon_snes, colorTable);
	RemapPalette(icon_plg, colorTable);
	RemapPalette(icon_a26, colorTable);
	RemapPalette(icon_col, colorTable);
	RemapPalette(icon_m5, colorTable);
	RemapPalette(icon_pce, colorTable);
	RemapPalette(icon_ws, colorTable);
	RemapPalette(icon_ngp, colorTable);
	RemapPalette(icon_cpc, colorTable);
	RemapPalette(icon_vid, colorTable);
	RemapPalette(icon_img, colorTable);
	RemapPalette(icon_msx, colorTable);
	RemapPalette(icon_mini, colorTable);
	RemapPalette(icon_hb, colorTable);
	RemapPalette(icon_md, colorTable);
	RemapPalette(icon_unk, colorTable);
}

static void clearIcon(int num)
{
	loadIcon(num, clearTiles, blackPalette, true);
}

void drawIcon(int num, int Xpos, int Ypos) {
	glSprite(Xpos, Ypos, bannerFlip[num], &ndsIcon[num][bnriconframenumY[num] & 31]);
	if (bnriconPalLine[num] != bnriconPalLoaded[num]) {
		glLoadPalette(num, dsi_palette[num][bnriconPalLine[num]]);
		bnriconPalLoaded[num] = bnriconPalLine[num];
	}
}

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

void getGameInfo(int num, bool isDir, const char* name, bool fromArgv)
{
	bnriconPalLine[num] = 0;
	bnriconPalLoaded[num] = 0;
	bnriconframenumY[num] = 0;
	bannerFlip[num] = GL_FLIP_NONE;
	bnrWirelessIcon[num] = 0;
	isDSiWare[num] = false;
	isHomebrew[num] = true;
	isModernHomebrew[num] = true;
	if (!fromArgv) {
		bnriconisDSi[num] = false;
		customIcon[num] = 0;
		infoFound[num] = false;
	}

	if (ms().showCustomIcons && customIcon[num] < 2 && (!fromArgv || customIcon[num] <= 0)) {
		toncset(&ndsBanner, 0, sizeof(sNDSBannerExt));
		bool customIconGood = false;

		// First try banner bin
		snprintf(customIconPath, sizeof(customIconPath), "%s:/_nds/TWiLightMenu/icons/%s.bin", sys().isRunFromSD() ? "sd" : "fat", name);
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
						ndsBanner.icon[pos] &= nibble? 0x0f : 0xf0;
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

	if (extension(name, {".argv"})) {
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

			if (extension(p, {".nds", ".dsi", ".ids", ".srl", ".app"})) {
				// let's see if this is a file or directory
				rc = stat(p, &st);
				if (rc != 0) {
					// stat failed
					clearBannerSequence(num);
				} else if (S_ISDIR(st.st_mode)) {
					// this is a directory!
					clearBannerSequence(num);
				} else {
					getGameInfo(num, false, p, true);
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
	} else if ((strcmp(name, "slot1") == 0) || extension(name, {".nds", ".dsi", ".ids", ".srl", ".app"})) {
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

	logPrint("iconUpdate: ");

	const auto isNds = (bnrRomType[num] == ROM_TYPE_NDS);

	if (customIcon[num] > 0 || (customIcon[num] && isNds)) {
		if (customIcon[num] == -1) {
			logPrint(isDir ? "Custom icon invalid!" : "Banner not found or custom icon invalid!");
			loadIconTileset(icon_unk, num);
		} else if (bnriconisDSi[num]) {
			logPrint("Custom icon found!");
			loadIcon(num, ndsBanner.dsi_icon[0], ndsBanner.dsi_palette[0], true);
		} else {
			logPrint("Custom icon found!");
			loadIcon(num, ndsBanner.icon, ndsBanner.palette, false);
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
					logPrint("Icon not found!");
					clearIcon(num);
				} else if (S_ISDIR(st.st_mode)) {
					// this is a directory!
					logPrint("Folder found!");
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
		logPrint("NDS icon found!");
		if (bnriconisDSi[num]) {
			loadIcon(num, ndsBanner.dsi_icon[0], ndsBanner.dsi_palette[0], true);
		} else {
			loadIcon(num, ndsBanner.icon, ndsBanner.palette, false);
		}
	} else if (bnrRomType[num] == ROM_TYPE_A26) {
		loadIconTileset(icon_a26, num);
	} else if (bnrRomType[num] == ROM_TYPE_MSX) {
		loadIconTileset(icon_msx, num);
	} else if (bnrRomType[num] == ROM_TYPE_COL) {
		loadIconTileset(icon_col, num);
	} else if (bnrRomType[num] == ROM_TYPE_M5) {
		loadIconTileset(icon_m5, num);
	} else if (bnrRomType[num] == ROM_TYPE_INT) {
		loadIconTileset(icon_int, num);
	} else if (bnrRomType[num] == ROM_TYPE_PLG) {
		loadIconTileset(icon_plg, num);
	} else if (bnrRomType[num] == ROM_TYPE_VID) {
		loadIconTileset(icon_vid, num);
	} else if (bnrRomType[num] == ROM_TYPE_IMG) {
		loadIconTileset(icon_img, num);
	} else if (bnrRomType[num] == ROM_TYPE_GBA) {
		loadIconTileset(icon_gba, num);
	} else if (bnrRomType[num] == ROM_TYPE_GB) {
		loadIconTileset(icon_gb, num);
	} else if (bnrRomType[num] == ROM_TYPE_GBC) {
		loadIconTileset(icon_gbc, num);
	} else if (bnrRomType[num] == ROM_TYPE_NES) {
		loadIconTileset(icon_nes, num);
	} else if (bnrRomType[num] == ROM_TYPE_SG) {
		loadIconTileset(icon_sg, num);
	} else if (bnrRomType[num] == ROM_TYPE_SMS) {
		loadIconTileset(icon_sms, num);
	} else if (bnrRomType[num] == ROM_TYPE_GG) {
		loadIconTileset(icon_gg, num);
	} else if (bnrRomType[num] == ROM_TYPE_MD) {
		loadIconTileset(icon_md, num);
	} else if (bnrRomType[num] == ROM_TYPE_SNES) {
		loadIconTileset(icon_snes, num);
	} else if (bnrRomType[num] == ROM_TYPE_PCE) {
		loadIconTileset(icon_pce, num);
	} else if (bnrRomType[num] == ROM_TYPE_WS) {
		loadIconTileset(icon_ws, num);
	} else if (bnrRomType[num] == ROM_TYPE_NGP) {
		loadIconTileset(icon_ngp, num);
	} else if (bnrRomType[num] == ROM_TYPE_CPC) {
		loadIconTileset(icon_cpc, num);
	} else if (bnrRomType[num] == ROM_TYPE_MINI) {
		loadIconTileset(icon_mini, num);
	} else if (bnrRomType[num] == ROM_TYPE_HB) {
		loadIconTileset(icon_hb, num);
	} else {
		loadIconTileset(icon_unk, num);
	}
	logPrint("\n");
}

void titleUpdate(int num, bool top, bool isDir, const char* name)
{
	if ((strcmp(name, "slot1") == 0) || infoFound[num]) {
		// this is an nds/app file!
		// or a file with custom banner text
		if (infoFound[num]) {
			printSmall(false, BOX_PX, iconYpos[top ? 0 : 3] + BOX_PY - (calcSmallFontHeight(cachedTitle[num]) / 2), cachedTitle[num], Alignment::center);
		} else {
			printSmall(false, BOX_PX, iconYpos[top ? 0 : 3] + BOX_PY - (calcSmallFontHeight(name) / 2), name, Alignment::center);
		}
	} else {
		std::vector<std::string> lines;
		if (ms().filenameDisplay == 0) {
			std::string nameString = name;
			std::string nameSubstr = nameString.substr(0, nameString.rfind('.'));
			lines.push_back(nameSubstr);
		} else {
			lines.push_back(name);
		}

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
