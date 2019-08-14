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

#include "common/dsimenusettings.h"
#include "common/flashcard.h"
#include "common/gl2d.h"
#include "graphics/fontHandler.h"
#include "graphics/iconHandler.h"
#include "graphics/queueControl.h"
#include "graphics/ThemeTextures.h"
#include "language.h"
#include "ndsheaderbanner.h"
#include "common/tonccpy.h"
#include <ctype.h>
#include <nds.h>
#include <stdio.h>
#include <sys/stat.h>

#define LEFT_ALIGN 70
#define ICON_POS_X 112
#define ICON_POS_Y 96

static int BOX_PY = 11;
static int BOX_PY_spacing1 = 19;
static int BOX_PY_spacing2 = 9;
static int BOX_PY_spacing3 = 28;

extern bool showdialogbox;
extern bool startMenu;


extern int currentBg;

extern int movingApp;

extern bool extention(const std::string& filename, const char* ext);

sNDSHeaderExt ndsHeader;
sNDSBannerExt ndsBanner;

#define TITLE_CACHE_SIZE 0x80

static bool infoFound[41] = {false};
static u16 cachedTitle[41][TITLE_CACHE_SIZE];
static char titleToDisplay[3][384];

static u32 arm9StartSig[4];

u8 tilesModified[(32 * 256) / 2] = {0};

std::vector<std::tuple<u8 *, u16 *, int, bool>> queuedIconUpdateCache;

void writeBannerText(int textlines, const char *text1, const char *text2, const char *text3) {
	if (ms().theme == 1) {
		switch (textlines) {
		case 0:
		default:
			printSmallCentered(false, BOX_PY + BOX_PY_spacing1, text1);
			break;
		case 1:
			printSmallCentered(false, BOX_PY + BOX_PY_spacing2, text1);
			printSmallCentered(false, BOX_PY + BOX_PY_spacing3, text2);
			break;
		case 2:
			printSmallCentered(false, BOX_PY, text1);
			printSmallCentered(false, BOX_PY + BOX_PY_spacing1, text2);
			printSmallCentered(false, BOX_PY + BOX_PY_spacing1 * 2, text3);
			break;
		}
	} else {
		switch (textlines) {
		case 0:
		default:
			printLargeCentered(false, BOX_PY + BOX_PY_spacing1, text1);
			break;
		case 1:
			printLargeCentered(false, BOX_PY + BOX_PY_spacing2, text1);
			printLargeCentered(false, BOX_PY + BOX_PY_spacing3, text2);
			break;
		case 2:
			printLargeCentered(false, BOX_PY, text1);
			printLargeCentered(false, BOX_PY + BOX_PY_spacing1, text2);
			printLargeCentered(false, BOX_PY + BOX_PY_spacing1 * 2, text3);
			break;
		}
	}
}

static void convertIconTilesToRaw(u8 *tilesSrc, u8 *tilesNew, bool twl) {
	int PY = 32;
	if (twl)
		PY = 32 * 8;
	const int PX = 16;
	const int TILE_SIZE_Y = 8;
	const int TILE_SIZE_X = 4;
	int index = 0;
	for (int tileY = 0; tileY < PY / TILE_SIZE_Y; ++tileY) {
		for (int tileX = 0; tileX < PX / TILE_SIZE_X; ++tileX)
			for (int pY = 0; pY < TILE_SIZE_Y; ++pY)
				for (int pX = 0; pX < TILE_SIZE_X;
				     ++pX) // TILE_SIZE/2 since one u8 equals two pixels (4 bit depth)
					tilesNew[pX + tileX * TILE_SIZE_X + PX * (pY + tileY * TILE_SIZE_Y)] =
					    tilesSrc[index++];
	}
}

/**
 * Queue the icon update.
 */
void deferLoadIcon(u8 *tilesSrc, u16 *palSrc, int num, bool twl) {
	queuedIconUpdateCache.emplace_back(std::move(std::make_tuple(tilesSrc, palSrc, num, twl)));
}

/**
 * This is called in graphics/vblank handler to process
 * any deferred updates.
 */
void execDeferredIconUpdates() {
	for (auto arg : queuedIconUpdateCache) {
		u8 *tilesSrc;
		u16 *palSrc;
		int num;
		bool twl;
		std::tie(tilesSrc, palSrc, num, twl) = arg;
		convertIconTilesToRaw(tilesSrc, tilesModified, twl);
		glLoadIcon(num, (u16 *)palSrc, (u8 *)tilesModified, twl ? TWL_TEX_HEIGHT : 32);
	}
	queuedIconUpdateCache.clear();
}

//(u8(*tilesSrc)[(32 * 32) / 2], u16(*palSrc)[16])
void loadIcon(u8 *tilesSrc, u16 *palSrc, int num, bool twl) {
	// Hack to prevent glitched icons on startup.
	if (currentBg == 1) {
		deferLoadIcon(tilesSrc, palSrc, num, twl);
	} else {
		convertIconTilesToRaw(tilesSrc, tilesModified, twl);
		glLoadIcon(num, (u16 *)palSrc, (u8 *)tilesModified, twl ? TWL_TEX_HEIGHT : 32);
	}
}

void loadUnkIcon(int num) { glLoadIcon(num, tex().iconUnknownTexture()->palette(), tex().iconUnknownTexture()->bytes()); }

static void clearIcon(int num) { glClearIcon(num); }

void drawIcon(int Xpos, int Ypos, int num) {
	int num2 = num;
	if (num == -1) {
		num2 = 6;
		num = movingApp;
	} else if (num >= 36) {
		num2 -= 36;
	} else if (num2 >= 30) {
		num2 -= 30;
	} else if (num2 >= 24) {
		num2 -= 24;
	} else if (num2 >= 18) {
		num2 -= 18;
	} else if (num2 >= 12) {
		num2 -= 12;
	} else if (num2 >= 6) {
		num2 -= 6;
	}
	// glSprite(Xpos, Ypos, bannerFlip[num], &ndsIcon[num2][bnriconPalLine[num]][bnriconframenumY[num]]);
	glSprite(Xpos, Ypos, GL_FLIP_NONE, &getIcon(num2)[bnriconframenumY[num2 == 6 ? 40 : num]]);
}

void drawIconGBA(int Xpos, int Ypos) { glSprite(Xpos, Ypos, GL_FLIP_NONE, &getIcon(GBA_ICON)[0 & 31]); }
void drawSmallIconGBA(int Xpos, int Ypos) { glSprite(Xpos, Ypos, GL_FLIP_NONE, &getIcon(GBA_ICON)[1 & 31]); }
void drawIconGB(int Xpos, int Ypos) { glSprite(Xpos, Ypos, GL_FLIP_NONE, &getIcon(GBC_ICON)[0 & 31]); }
void drawIconGBC(int Xpos, int Ypos) { glSprite(Xpos, Ypos, GL_FLIP_NONE, &getIcon(GBC_ICON)[1 & 31]); }
void drawIconNES(int Xpos, int Ypos) { glSprite(Xpos, Ypos, GL_FLIP_NONE, getIcon(NES_ICON)); }
void drawIconSMS(int Xpos, int Ypos) { glSprite(Xpos, Ypos, GL_FLIP_NONE, getIcon(SMS_ICON)); }
void drawIconGG(int Xpos, int Ypos) { glSprite(Xpos, Ypos, GL_FLIP_NONE, getIcon(GG_ICON)); }
void drawIconMD(int Xpos, int Ypos) { glSprite(Xpos, Ypos, GL_FLIP_NONE, getIcon(MD_ICON)); }
void drawIconSNES(int Xpos, int Ypos) { glSprite(Xpos, Ypos, GL_FLIP_NONE, getIcon(SNES_ICON)); }
void drawIconPLG(int Xpos, int Ypos) { glSprite(Xpos, Ypos, GL_FLIP_NONE, getIcon(PLG_ICON)); }

void loadFixedBanner(void) {
	/* Banner fixes start here */
	u32 bannersize = 0;

	// Fire Emblem - Heroes of Light and Shadow (English Translation)
	if (ndsBanner.crc[0] == 0xECF9 && ndsBanner.crc[1] == 0xD18F && ndsBanner.crc[2] == 0xE22A &&
	    ndsBanner.crc[3] == 0xD8F4) {
		// Use fixed banner.
		FILE *fixedBannerFile =
		    fopen("nitro:/fixedbanners/Fire Emblem - Heroes of Light and Shadow (J) (Eng).bnr", "rb");
		bannersize = NDS_BANNER_SIZE_DSi;
		fread(&ndsBanner, 1, bannersize, fixedBannerFile);
		fclose(fixedBannerFile);
	} else // Pokemon Black Version
	    if (ndsBanner.crc[0] == 0x4A19 && ndsBanner.crc[1] == 0x40AD && ndsBanner.crc[2] == 0x5641 &&
		ndsBanner.crc[3] == 0xEE5D) {
		// Use fixed banner.
		FILE *fixedBannerFile = fopen("nitro:/fixedbanners/Pokemon Black Version.bnr", "rb");
		bannersize = NDS_BANNER_SIZE_DSi;
		fread(&ndsBanner, 1, bannersize, fixedBannerFile);
		fclose(fixedBannerFile);
	} else // Pokemon Blaze Black (Clean Version)
	    if (ndsBanner.crc[0] == 0x4683 && ndsBanner.crc[1] == 0x40AD && ndsBanner.crc[2] == 0x5641 &&
		ndsBanner.crc[3] == 0xEE5D) {
		// Use fixed banner.
		FILE *fixedBannerFile = fopen("nitro:/fixedbanners/Pokemon Blaze Black (Clean Version).bnr", "rb");
		bannersize = NDS_BANNER_SIZE_DSi;
		fread(&ndsBanner, 1, bannersize, fixedBannerFile);
		fclose(fixedBannerFile);
	} else // Pokemon Blaze Black (Full Version)
	    if (ndsBanner.crc[0] == 0xA251 && ndsBanner.crc[1] == 0x40AD && ndsBanner.crc[2] == 0x5641 &&
		ndsBanner.crc[3] == 0xEE5D) {
		// Use fixed banner.
		FILE *fixedBannerFile = fopen("nitro:/fixedbanners/Pokemon Blaze Black (Full Version).bnr", "rb");
		bannersize = NDS_BANNER_SIZE_DSi;
		fread(&ndsBanner, 1, bannersize, fixedBannerFile);
		fclose(fixedBannerFile);
	} else // Pokemon White Version
	    if (ndsBanner.crc[0] == 0xE249 && ndsBanner.crc[1] == 0x5C94 && ndsBanner.crc[2] == 0xBF18 &&
		ndsBanner.crc[3] == 0x0C88) {
		// Use fixed banner.
		FILE *fixedBannerFile = fopen("nitro:/fixedbanners/Pokemon White Version.bnr", "rb");
		bannersize = NDS_BANNER_SIZE_DSi;
		fread(&ndsBanner, 1, bannersize, fixedBannerFile);
		fclose(fixedBannerFile);
	} else // Pokemon Volt White (Clean Version)
	    if (ndsBanner.crc[0] == 0x77F4 && ndsBanner.crc[1] == 0x5C94 && ndsBanner.crc[2] == 0xBF18 &&
		ndsBanner.crc[3] == 0x0C88) {
		// Use fixed banner.
		FILE *fixedBannerFile = fopen("nitro:/fixedbanners/Pokemon Volt White (Clean Version).bnr", "rb");
		bannersize = NDS_BANNER_SIZE_DSi;
		fread(&ndsBanner, 1, bannersize, fixedBannerFile);
		fclose(fixedBannerFile);
	} else // Pokemon Volt White (Full Version)
	    if (ndsBanner.crc[0] == 0x9CA8 && ndsBanner.crc[1] == 0x5C94 && ndsBanner.crc[2] == 0xBF18 &&
		ndsBanner.crc[3] == 0x0C88) {
		// Use fixed banner.
		FILE *fixedBannerFile = fopen("nitro:/fixedbanners/Pokemon Volt White (Full Version).bnr", "rb");
		bannersize = NDS_BANNER_SIZE_DSi;
		fread(&ndsBanner, 1, bannersize, fixedBannerFile);
		fclose(fixedBannerFile);
	} else // Pokemon Black Version 2
	    if (ndsBanner.crc[0] == 0xF996 && ndsBanner.crc[1] == 0xD784 && ndsBanner.crc[2] == 0xA257 &&
		ndsBanner.crc[3] == 0x2CA3) {
		// Use fixed banner.
		FILE *fixedBannerFile = fopen("nitro:/fixedbanners/Pokemon Black Version 2.bnr", "rb");
		bannersize = NDS_BANNER_SIZE_DSi;
		fread(&ndsBanner, 1, bannersize, fixedBannerFile);
		fclose(fixedBannerFile);
	} else // Pokemon White Version 2
	    if (ndsBanner.crc[0] == 0xA487 && ndsBanner.crc[1] == 0xF58C && ndsBanner.crc[2] == 0xAF9E &&
		ndsBanner.crc[3] == 0x3B18) {
		// Use fixed banner.
		FILE *fixedBannerFile = fopen("nitro:/fixedbanners/Pokemon White Version 2.bnr", "rb");
		bannersize = NDS_BANNER_SIZE_DSi;
		fread(&ndsBanner, 1, bannersize, fixedBannerFile);
		fclose(fixedBannerFile);
	}
}

void clearTitle(int num) {
	for (int i = 0; i < TITLE_CACHE_SIZE; i++) {
		cachedTitle[num][i] = 0;
	}
}

void getGameInfo(bool isDir, const char *name, int num) {
	if (num == -1)
		num = 40;

	bnriconPalLine[num] = 0;
	bnriconframenumY[num] = 0;
	bannerFlip[num] = GL_FLIP_NONE;
	bnriconisDSi[num] = false;
	bnrWirelessIcon[num] = 0;
	isDSiWare[num] = false;
	isHomebrew[num] = 0;
	infoFound[num] = false;

	if (isDir) {
		clearTitle(num);
		clearBannerSequence(num); // banner sequence
	} else if (extention(name, ".argv")) {
		// look through the argv file for the corresponding nds file
		FILE *fp;
		char *line = NULL, *p = NULL;
		size_t size = 0;
		ssize_t rc;

		// open the argv file
		fp = fopen(name, "rb");
		if (fp == NULL) {
			clearTitle(num);
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

			if (extention(p, ".nds")
			 || extention(p, ".dsi")
			 || extention(p, ".ids")
			 || extention(p, ".app")) {
				// let's see if this is a file or directory
				rc = stat(p, &st);
				if (rc != 0) {
					// stat failed
					clearTitle(num);
					clearBannerSequence(num);
				} else if (S_ISDIR(st.st_mode)) {
					// this is a directory!
					clearTitle(num);
					clearBannerSequence(num);
				} else {
					getGameInfo(false, p, num);
				}
			} else {
				// this is not an nds/app file!
				clearTitle(num);
				clearBannerSequence(num);
			}
		} else {
			clearTitle(num);
			clearBannerSequence(num);
		}
		// clean up the allocated line
		free(line);
	} else if (extention(name, ".nds")
			 || extention(name, ".dsi")
			 || extention(name, ".ids")
			 || extention(name, ".app")) {
		// this is an nds/app file!
		FILE *fp;
		int ret;

		// open file for reading info
		fp = fopen(name, "rb");
		if (fp == NULL) {
			clearTitle(num);
			clearBannerSequence(num); // banner sequence
			fclose(fp);
			return;
		}

		ret = fseek(fp, 0, SEEK_SET);
		if (ret == 0)
			ret = fread(&ndsHeader, sizeof(ndsHeader), 1, fp); // read if seek succeed
		else
			ret = 0; // if seek fails set to !=1

		if (ret != 1) {
			clearTitle(num);
			clearBannerSequence(num);
			fclose(fp);
			return;
		}

		if (num < 40) {
			tonccpy(gameTid[num], ndsHeader.gameCode, 4);
			headerCRC[num] = ndsHeader.headerCRC16;
		}

		fseek(fp, (ndsHeader.arm9romOffset == 0x200 ? ndsHeader.arm9romOffset : ndsHeader.arm9romOffset+0x800), SEEK_SET);
		fread(arm9StartSig, sizeof(u32), 4, fp);
		if (arm9StartSig[0] == 0xE3A00301
		 && arm9StartSig[1] == 0xE5800208
		 && arm9StartSig[2] == 0xE3A00013
		 && arm9StartSig[3] == 0xE129F000) {
			isHomebrew[num] = 2; // Homebrew is recent (supports reading from SD without a DLDI driver)
			if (ndsHeader.arm7executeAddress >= 0x037F0000 && ndsHeader.arm7destination >= 0x037F0000) {
				if ((ndsHeader.arm9binarySize == 0xC9F68 && ndsHeader.arm7binarySize == 0x12814)	// Colors! v1.1
				|| (ndsHeader.arm9binarySize == 0x1B0864 && ndsHeader.arm7binarySize == 0xDB50)	// Mario Paint Composer DS v2 (Bullet Bill)
				|| (ndsHeader.arm9binarySize == 0x2C9A8 && ndsHeader.arm7binarySize == 0xFB98)) {	// NitroGrafx v0.7
					isHomebrew[num] = 1; // Have nds-bootstrap load it (in case if non-DLDI SD read doesn't work)
				}
			}
		} else if (ndsHeader.arm7executeAddress >= 0x037F0000 && ndsHeader.arm7destination >= 0x037F0000) {
			isHomebrew[num] = 1; // Homebrew is old (requires a DLDI driver to read from SD)
		} else if ((ndsHeader.gameCode[0] == 0x48 && ndsHeader.makercode[0] != 0 && ndsHeader.makercode[1] != 0)
		 || (ndsHeader.gameCode[0] == 0x4B && ndsHeader.makercode[0] != 0 && ndsHeader.makercode[1] != 0)
		 || (ndsHeader.gameCode[0] == 0x5A && ndsHeader.makercode[0] != 0 && ndsHeader.makercode[1] != 0)
		 || (ndsHeader.gameCode[0] == 0x42 && ndsHeader.gameCode[1] == 0x38 && ndsHeader.gameCode[2] == 0x38))
		{
			if ((ms().consoleModel == 0 && ndsHeader.unitCode == 0x02) || ndsHeader.unitCode == 0x03)
				isDSiWare[num] = true; // Is a DSiWare game
		}

		bnrSysSettings[num] =
		    (ndsHeader.gameCode[0] == 0x48 && ndsHeader.gameCode[1] == 0x4E && ndsHeader.gameCode[2] == 0x42);

		if (ndsHeader.dsi_flags & BIT(4))
			bnrWirelessIcon[num] = 1;
		else if (ndsHeader.dsi_flags & BIT(3))
			bnrWirelessIcon[num] = 2;

		if (ndsHeader.bannerOffset == 0) {
			fclose(fp);

			FILE *bannerFile = fopen("nitro:/noinfo.bnr", "rb");
			fread(&ndsBanner, 1, NDS_BANNER_SIZE_ZH_KO, bannerFile);
			fclose(bannerFile);

			tonccpy(bnriconTile[num], (char *)&ndsBanner, 0x23C0);

			for (int i = 0; i < 128; i++) {
				cachedTitle[num][i] = ndsBanner.titles[setGameLanguage][i];
			}

			return;
		}
		ret = fseek(fp, ndsHeader.bannerOffset, SEEK_SET);
		if (ret == 0)
			ret = fread(&ndsBanner, sizeof(ndsBanner), 1, fp); // read if seek succeed
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

				FILE *bannerFile = fopen("nitro:/noinfo.bnr", "rb");
				fread(&ndsBanner, 1, NDS_BANNER_SIZE_ZH_KO, bannerFile);
				fclose(bannerFile);

				tonccpy(bnriconTile[num], (char *)&ndsBanner, 0x23C0);

				for (int i = 0; i < TITLE_CACHE_SIZE; i++) {
					cachedTitle[num][i] = ndsBanner.titles[setGameLanguage][i];
				}

				return;
			}
		}

		// close file!
		fclose(fp);

		loadFixedBanner();

		DC_FlushAll();

		tonccpy(bnriconTile[num], (char *)&ndsBanner, 0x23C0);

		for (int i = 0; i < TITLE_CACHE_SIZE; i++) {
			cachedTitle[num][i] = ndsBanner.titles[setGameLanguage][i];
		}
		infoFound[num] = true;

		// banner sequence
		if (ms().animateDsiIcons && ndsBanner.version == NDS_BANNER_VER_DSi) {
			grabBannerSequence(num);
			bnriconisDSi[num] = true;
		}
	}
}

void iconUpdate(bool isDir, const char *name, int num) {
	int num2 = num;
	if (num == -1) {
		num2 = 6;
		num = 40;
	} else if (num >= 36) {
		num2 -= 36;
	} else if (num >= 30) {
		num2 -= 30;
	} else if (num >= 24) {
		num2 -= 24;
	} else if (num >= 18) {
		num2 -= 18;
	} else if (num >= 12) {
		num2 -= 12;
	} else if (num >= 6) {
		num2 -= 6;
	}

	if (isDir) {
		// icon
		clearIcon(num2);
	} else if (extention(name, ".argv")) {
		// look through the argv file for the corresponding nds file
		FILE *fp;
		char *line = NULL, *p = NULL;
		size_t size = 0;
		ssize_t rc;

		// open the argv file
		fp = fopen(name, "rb");
		if (fp == NULL) {
			clearIcon(num2);
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

			if (extention(p, ".nds")
			 || extention(p, ".dsi")
			 || extention(p, ".ids")
			 || extention(p, ".app")) {
				// let's see if this is a file or directory
				rc = stat(p, &st);
				if (rc != 0) {
					// stat failed
					clearIcon(num2);
				} else if (S_ISDIR(st.st_mode)) {
					// this is a directory!
					clearIcon(num2);
				} else {
					iconUpdate(false, p, num2);
				}
			} else {
				// this is not an nds/app file!
				clearIcon(num2);
			}
		} else {
			clearIcon(num2);
		}
		// clean up the allocated line
		free(line);
	} else if (extention(name, ".nds")
			 || extention(name, ".dsi")
			 || extention(name, ".ids")
			 || extention(name, ".app")) {
		// this is an nds/app file!
		tonccpy((char *)&ndsBanner, bnriconTile[num], 0x23C0);

		// icon
		DC_FlushAll();
		if (ms().animateDsiIcons && ndsBanner.version == NDS_BANNER_VER_DSi) {
			loadIcon(ndsBanner.dsi_icon[0], ndsBanner.dsi_palette[0], num2, true);
		} else {
			loadIcon(ndsBanner.icon, ndsBanner.palette, num2, false);
		}
	}
}

static inline void writeDialogTitle(int textlines, const char *text1, const char *text2, const char *text3) {
	// Ensure that the font isn't corrupted.
	switch (textlines) {
	case 0:
	default:
		printLarge(false, LEFT_ALIGN, BOX_PY + BOX_PY_spacing1, text1);
		break;
	case 1:
		printLarge(false, LEFT_ALIGN, BOX_PY + BOX_PY_spacing2, text1);
		printLarge(false, LEFT_ALIGN, BOX_PY + BOX_PY_spacing3, text2);
		break;
	case 2:
		printLarge(false, LEFT_ALIGN, BOX_PY, text1);
		printLarge(false, LEFT_ALIGN, BOX_PY + BOX_PY_spacing1, text2);
		printLarge(false, LEFT_ALIGN, BOX_PY + BOX_PY_spacing1 * 2, text3);
		break;
	}
}

void titleUpdate(bool isDir, const char *name, int num) {
	clearText(false);
	if (showdialogbox || (ms().theme == 4 && currentBg == 1)) {
		BOX_PY = 14;
		BOX_PY_spacing1 = 17;
		BOX_PY_spacing2 = 7;
		BOX_PY_spacing3 = 26;
	} else if (ms().theme == 1) {
		BOX_PY = 39;
		BOX_PY_spacing1 = 16;
		BOX_PY_spacing2 = 8;
		BOX_PY_spacing3 = 25;
	} else {
		BOX_PY = 11;
		BOX_PY_spacing1 = 19;
		BOX_PY_spacing2 = 9;
		BOX_PY_spacing3 = 28;
	}

	/*if (startMenu) {
		if (ms().startMenu_cursorPosition == 0) {
			writeBannerText(0, "Settings", "", "");
		} else if (ms().startMenu_cursorPosition == 1) {
			if (!flashcardFound()) {
				if (REG_SCFG_MC == 0x11) {
					writeBannerText(1, "There is nothing inserted in", "the Game Card slot.", "");
				} else {
					writeBannerText(1, "Launch Slot-1 card", "(NTR carts only)", "");
				}
			} else {
				if (ms().useGbarunner) {
					writeBannerText(0, "Start GBARunner2", "", "");
				} else {
					writeBannerText(0, "Start GBA Mode", "", "");
				}
			}
		} else if (ms().startMenu_cursorPosition == 2) {
			writeBannerText(0, "Start GBARunner2", "", "");
		}
		return;
	}*/

	if (isDir) {
		// text
		if (strcmp(name, "..") == 0) {
			writeBannerText(0, "Back", "", "");
		} else {
			writeBannerText(0, name, "", "");
		}
	} else if (extention(name, ".plg")
	 || extention(name, ".rvid")
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
	 || extention(name, ".sfc"))
	{
		writeBannerText(0, name, "", "");
	} else {
		// this is an nds/app file!

		// turn unicode into ascii (kind of)
		// and convert 0x0A into 0x00
		int bannerlines = 0;
		// The index of the character array
		int charIndex = 0;
		for (int i = 0; i < TITLE_CACHE_SIZE; i++) {
			// todo: fix crash on titles that are too long (homebrew)
			if ((cachedTitle[num][i] == 0x000A) || (cachedTitle[num][i] == 0xFFFF)) {
				titleToDisplay[bannerlines][charIndex] = 0;
				bannerlines++;
				charIndex = 0;
			} else if (cachedTitle[num][i] <= 0x00FF) { // ASCII+Latin Extended Range is 0x0 to 0xFF.
				// Handle ASCII here
				titleToDisplay[bannerlines][charIndex] = cachedTitle[num][i];
				charIndex++;
			} else {
				// We need to split U16 into two characters here.
				char lowerBits = cachedTitle[num][i] & 0xFF;
				char higherBits = cachedTitle[num][i] >> 8;
				// Since we have UTF16LE, assemble in FontGraphic the other way.
				// We will need to peek in FontGraphic.cpp since the higher bit is significant.
				titleToDisplay[bannerlines][charIndex] = UTF16_SIGNAL_BYTE;
				// 0x0F signal bit to treat the next two characters as UTF
				titleToDisplay[bannerlines][charIndex + 1] = lowerBits;
				titleToDisplay[bannerlines][charIndex + 2] = higherBits;
				charIndex += 3;
			}
		}

		// text
		if (showdialogbox || (ms().theme == 4 && currentBg == 1) || infoFound[num]) {
			if (showdialogbox || (ms().theme == 4 && currentBg == 1)) {
				writeDialogTitle(bannerlines, titleToDisplay[0], titleToDisplay[1], titleToDisplay[2]);
			} else {
				writeBannerText(bannerlines, titleToDisplay[0], titleToDisplay[1], titleToDisplay[2]);
			}
		} else if (ms().theme == 1) {
			printSmallCentered(false, BOX_PY + BOX_PY_spacing2, name);
			printSmallCentered(false, BOX_PY + BOX_PY_spacing3, titleToDisplay[0]);
		} else {
			printLargeCentered(false, BOX_PY + BOX_PY_spacing2, name);
			printLargeCentered(false, BOX_PY + BOX_PY_spacing3, titleToDisplay[0]);
		}
	}
}
