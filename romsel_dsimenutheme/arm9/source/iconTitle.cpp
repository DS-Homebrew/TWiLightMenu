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

#include "iconTitle.h"
#include "common/twlmenusettings.h"
#include "common/bootstrapsettings.h"
#include "common/systemdetails.h"
#include "common/flashcard.h"
#include <gl2d.h>
#include "common/tonccpy.h"
#include "fileBrowse.h"
#include "graphics/fontHandler.h"
#include "graphics/iconHandler.h"
#include "common/lodepng.h"
#include "graphics/queueControl.h"
#include "graphics/ThemeConfig.h"
#include "graphics/ThemeTextures.h"
#include "language.h"
#include "ndsheaderbanner.h"
#include "myDSiMode.h"
#include <ctype.h>
#include <nds.h>
#include <nds/arm9/dldi.h>
#include <stdio.h>
#include <sys/stat.h>
#include <vector>

#define ICON_POS_X 112
#define ICON_POS_Y 96

extern bool showdialogbox;
extern bool dbox_showIcon;
extern bool startMenu;

extern int currentBg;

extern int movingApp;

#define TITLE_CACHE_SIZE 0x80

static bool infoFound[41] = {false};
static const char16_t *cachedTitle[41];
static const char16_t *blankTitle = u"";

static u32 arm9StartSig[4];

u8 tilesModified[(32 * 256) / 2] = {0};

std::vector<std::tuple<u8 *, u16 *, int, bool>> queuedIconUpdateCache;

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
				for (int pX = 0; pX < TILE_SIZE_X; ++pX) // TILE_SIZE/2 since one u8 equals two pixels (4 bit depth)
					tilesNew[pX + tileX * TILE_SIZE_X + PX * (pY + tileY * TILE_SIZE_Y)] = tilesSrc[index++];
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

static inline void loadUnkIcon(int num) { glLoadIcon(num, tex().iconUnknownTexture()->palette(), tex().iconUnknownTexture()->bytes()); }

static inline void clearIcon(int num) { glClearIcon(num); }

void drawIcon(int Xpos, int Ypos, int num) {
	if (num == -1) { // Moving app icon
		glSprite(Xpos, Ypos, bannerFlip[40], &getIcon(6)[bnriconframenumY[40]]);
		if (bnriconPalLine[40] != bnriconPalLoaded[40]) {
			glLoadPalette(6, bnriconTile[40].dsi_palette[bnriconPalLine[40]]);
			bnriconPalLoaded[40] = bnriconPalLine[40];
		}
	} else {
		glSprite(Xpos, Ypos, bannerFlip[num], &getIcon(num % 6)[bnriconframenumY[num]]);
		if (bnriconPalLine[num] != bnriconPalLoaded[num]) {
			glLoadPalette(num % 6, bnriconTile[num].dsi_palette[bnriconPalLine[num]]);
			bnriconPalLoaded[num] = bnriconPalLine[num];
		}
	}
}

void drawIconGBA(int Xpos, int Ypos) { glSprite(Xpos, Ypos, GL_FLIP_NONE, &getIcon(GBA_ICON)[0 & 31]); }
void drawSmallIconGBA(int Xpos, int Ypos) { glSprite(Xpos, Ypos, GL_FLIP_NONE, &getIcon(GBA_ICON)[1 & 31]); }
void drawIconGB(int Xpos, int Ypos) { glSprite(Xpos, Ypos, GL_FLIP_NONE, &getIcon(GBC_ICON)[0 & 31]); }
void drawIconGBC(int Xpos, int Ypos) { glSprite(Xpos, Ypos, GL_FLIP_NONE, &getIcon(GBC_ICON)[1 & 31]); }
void drawIconNES(int Xpos, int Ypos) { glSprite(Xpos, Ypos, GL_FLIP_NONE, getIcon(NES_ICON)); }
void drawIconSG(int Xpos, int Ypos) { glSprite(Xpos, Ypos, GL_FLIP_NONE, getIcon(SG_ICON)); }
void drawIconSMS(int Xpos, int Ypos) { glSprite(Xpos, Ypos, GL_FLIP_NONE, getIcon(SMS_ICON)); }
void drawIconGG(int Xpos, int Ypos) { glSprite(Xpos, Ypos, GL_FLIP_NONE, getIcon(GG_ICON)); }
void drawIconMD(int Xpos, int Ypos) { glSprite(Xpos, Ypos, GL_FLIP_NONE, getIcon(MD_ICON)); }
void drawIconSNES(int Xpos, int Ypos) { glSprite(Xpos, Ypos, GL_FLIP_NONE, getIcon(SNES_ICON)); }
void drawIconPLG(int Xpos, int Ypos) { glSprite(Xpos, Ypos, GL_FLIP_NONE, getIcon(PLG_ICON)); }
void drawIconA26(int Xpos, int Ypos) { glSprite(Xpos, Ypos, GL_FLIP_NONE, getIcon(A26_ICON)); }
void drawIconCOL(int Xpos, int Ypos) { glSprite(Xpos, Ypos, GL_FLIP_NONE, getIcon(COL_ICON)); }
void drawIconM5(int Xpos, int Ypos) { glSprite(Xpos, Ypos, GL_FLIP_NONE, getIcon(M5_ICON)); }
void drawIconINT(int Xpos, int Ypos) { glSprite(Xpos, Ypos, GL_FLIP_NONE, getIcon(INT_ICON)); }
void drawIconPCE(int Xpos, int Ypos) { glSprite(Xpos, Ypos, GL_FLIP_NONE, getIcon(PCE_ICON)); }
void drawIconWS(int Xpos, int Ypos) { glSprite(Xpos, Ypos, GL_FLIP_NONE, getIcon(WS_ICON)); }
void drawIconNGP(int Xpos, int Ypos) { glSprite(Xpos, Ypos, GL_FLIP_NONE, getIcon(NGP_ICON)); }
void drawIconCPC(int Xpos, int Ypos) { glSprite(Xpos, Ypos, GL_FLIP_NONE, getIcon(CPC_ICON)); }
void drawIconVID(int Xpos, int Ypos) { glSprite(Xpos, Ypos, GL_FLIP_NONE, getIcon(VID_ICON)); }
void drawIconIMG(int Xpos, int Ypos) { glSprite(Xpos, Ypos, GL_FLIP_NONE, getIcon(IMG_ICON)); }

void drawRomIcon(int Xpos, int Ypos, int num, int romType) {
	if (num == -1 ? customIcon[40] : customIcon[num])
		drawIcon(Xpos, Ypos, num);
	else if (romType == 20)
		drawIconIMG(Xpos, Ypos);
	else if (romType == 19)
		drawIconVID(Xpos, Ypos);
	else if (romType == 18)
		drawIconCPC(Xpos, Ypos);
	else if (romType == 17)
		drawIconNGP(Xpos, Ypos);
	else if (romType == 16)
		drawIconWS(Xpos, Ypos);
	else if (romType == 15)
		drawIconSG(Xpos, Ypos);
	else if (romType == 14)
		drawIconM5(Xpos, Ypos);
	else if (romType == 13)
		drawIconCOL(Xpos, Ypos);
	else if (romType == 12)
		drawIconINT(Xpos, Ypos);
	else if (romType == 11)
		drawIconPCE(Xpos, Ypos);
	else if (romType == 10)
		drawIconA26(Xpos, Ypos);
	else if (romType == 9)
		drawIconPLG(Xpos, Ypos);
	else if (romType == 8)
		drawIconSNES(Xpos, Ypos);
	else if (romType == 7)
		drawIconMD(Xpos, Ypos);
	else if (romType == 6)
		drawIconGG(Xpos, Ypos);
	else if (romType == 5)
		drawIconSMS(Xpos, Ypos);
	else if (romType == 4)
		drawIconNES(Xpos, Ypos);
	else if (romType == 3)
		drawIconGBC(Xpos, Ypos);
	else if (romType == 2)
		drawIconGB(Xpos, Ypos);
	else if (romType == 1)
		drawIconGBA(Xpos, Ypos);
	else
		drawIcon(Xpos, Ypos, num);
}

void clearTitle(int num) {
	cachedTitle[num] = blankTitle;
}

void getGameInfo(bool isDir, const char *name, int num, bool fromArgv) {
	if (num == -1)
		num = 40;

	bnriconPalLine[num] = 0;
	bnriconPalLoaded[num] = 0;
	bnriconframenumY[num] = 0;
	bannerFlip[num] = GL_FLIP_NONE;
	bnrWirelessIcon[num] = 0;
	isDSiWare[num] = false;
	isHomebrew[num] = false;
	isModernHomebrew[num] = false;
	requiresRamDisk[num] = false;
	requiresDonorRom[num] = false;
	if (!fromArgv) {
		bnriconisDSi[num] = false;
		customIcon[num] = 0;
		infoFound[num] = false;
	}

	if (ms().showCustomIcons && customIcon[num] < 2) {
		sNDSBannerExt &banner = bnriconTile[num];
		bool argvHadPng = customIcon[num] == 1;
		u8 iconCopy[512];
		u16 paletteCopy[16];
		if (argvHadPng) { // custom png icon from argv
			// copy the icon and palette before they get overwritten
			memcpy(iconCopy, banner.icon, sizeof(iconCopy));
			memcpy(paletteCopy, banner.palette, sizeof(paletteCopy));
		}
		toncset(&banner, 0, sizeof(sNDSBannerExt));
		bool customIconGood = false;

		// First try banner bin
		snprintf(customIconPath, sizeof(customIconPath), "%s:/_nds/TWiLightMenu/icons/%s.bin", sdFound() ? "sd" : "fat", name);
		if (access(customIconPath, F_OK) == 0) {
			customIcon[num] = 2; // custom icon is a banner bin
			FILE *file = fopen(customIconPath, "rb");
			if (file) {
				size_t read = fread(&banner, 1, sizeof(sNDSBannerExt), file);
				fclose(file);

				// restore png icon
				if (argvHadPng) {
					memcpy(banner.icon, iconCopy, sizeof(iconCopy));
					memcpy(banner.palette, paletteCopy, sizeof(paletteCopy));
				}

				if (read >= NDS_BANNER_SIZE_ORIGINAL) {
					customIconGood = true;

					if (!argvHadPng && ms().animateDsiIcons && read == NDS_BANNER_SIZE_DSi) {
						u16 crc16 = swiCRC16(0xFFFF, banner.dsi_icon, 0x1180);
						if (banner.crc[3] == crc16) { // Check if CRC16 is valid
							bnriconisDSi[num] = true;
							grabBannerSequence(num);
						}
					}

					int currentLang = 0;
					if (banner.version == NDS_BANNER_VER_ZH || banner.version == NDS_BANNER_VER_ZH_KO || banner.version == NDS_BANNER_VER_DSi) {
						currentLang = ms().getGameLanguage();
					} else {
						currentLang = ms().getTitleLanguage();
					}
					while (banner.titles[currentLang][0] == 0 || (banner.titles[currentLang][0] == 0x20 && banner.titles[currentLang][1] == 0)) {
						if (currentLang == 0) break;
						currentLang--;
					}
					cachedTitle[num] = (char16_t*)&banner.titles[currentLang];
					infoFound[num] = true;
				}
			}
		} else if (customIcon[num] == 0) {
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
						banner.icon[pos] &= nibble? 0x0f : 0xf0;
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
								if (banner.palette[palIdx] == color) {
									banner.icon[pos] |= nibble? palIdx<<4 : palIdx;
									found = true;
									break;
								}
							}
							// add color to palette if room available
							if (!found && colorCount < 16) {
								banner.icon[pos] |= nibble? colorCount<<4 : colorCount;
								banner.palette[colorCount++] = color;
							}
						}
					}
				}
			}
		}

		if (customIcon[num] && !customIconGood)
			customIcon[num] = -1; // display as unknown
	}

	if (isDir) {
		clearTitle(num);
		if (customIcon[num] != 2)
			clearBannerSequence(num); // banner sequence
	} else if (extension(name, {".argv"})) {
		// look through the argv file for the corresponding nds file
		FILE *fp;
		char *line = NULL, *p = NULL;
		size_t size = 0;
		ssize_t rc;

		// open the argv file
		fp = fopen(name, "rb");
		if (!fp) {
			clearTitle(num);
			if (customIcon[num] != 2)
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

			if (extension(p, {".nds", ".dsi", ".ids", ".srl", ".app"})) {
				// let's see if this is a file or directory
				rc = stat(p, &st);
				if (rc != 0) {
					// stat failed
					clearTitle(num);
					if (customIcon[num] != 2)
						clearBannerSequence(num);
				} else if (S_ISDIR(st.st_mode)) {
					// this is a directory!
					clearTitle(num);
					if (customIcon[num] != 2)
						clearBannerSequence(num);
				} else {
					getGameInfo(false, p, num, true);
				}
			} else {
				// this is not an nds/app file!
				clearTitle(num);
				if (customIcon[num] != 2)
					clearBannerSequence(num);
			}
		} else {
			clearTitle(num);
			if (customIcon[num] != 2)
				clearBannerSequence(num);
		}
		// clean up the allocated line
		free(line);
	} else if (extension(name, {".nds", ".dsi", ".ids", ".srl", ".app"})) {
		// this is an nds/app file!
		FILE *fp;

		// open file for reading info
		fp = fopen(name, "rb");
		if (!fp) {
			clearTitle(num);
			if (customIcon[num] != 2)
				clearBannerSequence(num); // banner sequence
			fclose(fp);
			return;
		}

		sNDSHeaderExt ndsHeader;

		if (!fread(&ndsHeader, sizeof(ndsHeader), 1, fp)) {
			// try again, but using regular header size
			fseek(fp, 0, SEEK_SET);
			if (!fread(&ndsHeader, 0x160, 1, fp)) {
				clearTitle(num);
				if (customIcon[num] != 2)
					clearBannerSequence(num);
				fclose(fp);
				return;
			}
		}

		if (num < 40) {
			tonccpy(gameTid[num], ndsHeader.gameCode, 4);
			isTwlm[num] = (strcmp(gameTid[num], "SRLA") == 0);
			romVersion[num] = ndsHeader.romversion;
			unitCode[num] = ndsHeader.unitCode;
			headerCRC[num] = ndsHeader.headerCRC16;
			a7mbk6[num] = ndsHeader.a7mbk6;
		}

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

		if (isHomebrew[num] == true && !ms().secondaryDevice && num < 40) {
			if ((ndsHeader.arm9binarySize == 0x98F70 && ndsHeader.arm7binarySize == 0xED94)		// jEnesisDS 0.7.4
			|| (ndsHeader.arm9binarySize == 0x48950 && ndsHeader.arm7binarySize == 0x74C4)			// SNEmulDS06-WIP2
			|| (ndsHeader.arm9binarySize == 0xD45C0 && ndsHeader.arm7binarySize == 0x2B7C)			// ikuReader v0.058
			|| (ndsHeader.arm9binarySize == 0x54620 && ndsHeader.arm7binarySize == 0x1538)) {		// XRoar 0.24fp3
				requiresRamDisk[num] = true;
			}
		}

		if (num < 40 && !isHomebrew[num]) {
			// Check if ROM needs a donor ROM
			bool dsiEnhancedMbk = (isDSiMode() && *(u32*)0x02FFE1A0 == 0x00403000 && sys().arm7SCFGLocked());
			if (isDSiMode() && (a7mbk6[num] == (dsiEnhancedMbk ? 0x080037C0 : 0x00403000) || (ndsHeader.gameCode[0] == 'H' && ndsHeader.arm7binarySize < 0xC000 && ndsHeader.arm7idestination == 0x02E80000 && (REG_MBK9 & 0x00FFFFFF) != 0x00FFFF0F)) && sys().arm7SCFGLocked()) {
				requiresDonorRom[num] = dsiEnhancedMbk ? 51 : 52; // DSi-Enhanced ROM required on CycloDSi, or DSi-Exclusive/DSiWare ROM required on DSiWarehax
				if (ndsHeader.gameCode[0] == 'H' && ndsHeader.arm7binarySize < 0xC000 && ndsHeader.arm7idestination == 0x02E80000) {
					requiresDonorRom[num] += 100;
				}
			} else if (ndsHeader.gameCode[0] != 'D' && memcmp(ndsHeader.gameCode, "KCX", 3) != 0 && memcmp(ndsHeader.gameCode, "KAV", 3) != 0 && memcmp(ndsHeader.gameCode, "KNK", 3) != 0 && a7mbk6[num] == 0x080037C0 && ms().secondaryDevice && (!dsiFeatures() || bs().b4dsMode)) {
				requiresDonorRom[num] = 51; // DSi-Enhanced ROM required
			} else if (memcmp(ndsHeader.gameCode, "AYI", 3) == 0 && ndsHeader.arm7binarySize == 0x25F70) {
				requiresDonorRom[num] = 20; // SDK2.0 ROM required for Yoshi Touch & Go (Europe)
			}
		}

		bnrSysSettings[num] = (ndsHeader.gameCode[0] == 0x48 && ndsHeader.gameCode[1] == 0x4E && ndsHeader.gameCode[2] == 0x42);

		if (ndsHeader.dsi_flags & BIT(4))
			bnrWirelessIcon[num] = 1;
		else if (ndsHeader.dsi_flags & BIT(3))
			bnrWirelessIcon[num] = 2;
		
		if (customIcon[num] == 2) { // custom banner bin
			// we're done early, close the file
			fclose(fp);
			return;
		}

		sNDSBannerExt &ndsBanner = bnriconTile[num];

		u8 iconCopy[512];
		u16 paletteCopy[16];
		if (customIcon[num] == 1) { // custom png icon
			// copy the icon and palette before they get overwritten
			memcpy(iconCopy, ndsBanner.icon, sizeof(iconCopy));
			memcpy(paletteCopy, ndsBanner.palette, sizeof(paletteCopy));
		}

		if (ndsHeader.bannerOffset == 0) {
			fclose(fp);

			// If no custom icon, display as unknown
			if (customIcon[num] == 0)
				customIcon[num] = -1;

			return;
		}
		fseek(fp, ndsHeader.bannerOffset, SEEK_SET);
		if (!fread(&ndsBanner, sizeof(ndsBanner), 1, fp)) {
			// try again, but using regular banner size
			fseek(fp, ndsHeader.bannerOffset, SEEK_SET);
			if (!fread(&ndsBanner, NDS_BANNER_SIZE_ORIGINAL, 1, fp)) {
				fclose(fp);

				// If no custom icon, display as unknown
				if (customIcon[num] == 0)
					customIcon[num] = -1;

				return;
			}
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

		cachedTitle[num] = (char16_t*)&ndsBanner.titles[currentLang];

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

void iconUpdate(bool isDir, const char *name, int num) {
	int spriteIdx = num == -1 ? 6 : num % 6;
	if (num == -1)
		num = 40;

	if (customIcon[num]) {
		sNDSBannerExt &ndsBanner = bnriconTile[num];
		if (customIcon[num] == -1) {
			loadUnkIcon(spriteIdx);
		} else if (bnriconisDSi[num]) {
			loadIcon(ndsBanner.dsi_icon[0], ndsBanner.dsi_palette[bnriconPalLine[num]], spriteIdx, true);
			bnriconPalLoaded[num] = bnriconPalLine[num];
		} else {
			loadIcon(ndsBanner.icon, ndsBanner.palette, spriteIdx, false);
		}
	} else if (isDir) {
		clearIcon(spriteIdx);
	} else if (extension(name, {".argv"})) {
		// look through the argv file for the corresponding nds file
		FILE *fp;
		char *line = NULL, *p = NULL;
		size_t size = 0;
		ssize_t rc;

		// open the argv file
		fp = fopen(name, "rb");
		if (fp == NULL) {
			clearIcon(spriteIdx);
			fclose(fp);
			return;
		}

		// read each line
		while ((rc = __getline(&line, &size, fp)) > 0) {
			// remove comments
			if ((p = strchr(line, '#')) != NULL)
				*p = 0;

			// skip leading whitespace
			for (p = line; *p && isspace((int)*p); ++p);

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
					clearIcon(spriteIdx);
				} else if (S_ISDIR(st.st_mode)) {
					// this is a directory!
					clearIcon(spriteIdx);
				} else {
					iconUpdate(false, p, spriteIdx);
				}
			} else {
				// this is not an nds/app file!
				clearIcon(spriteIdx);
			}
		} else {
			clearIcon(spriteIdx);
		}
		// clean up the allocated line
		free(line);
	} else if (extension(name, {".nds", ".dsi", ".ids", ".srl", ".app"})) {
		// this is an nds/app file!
		sNDSBannerExt &ndsBanner = bnriconTile[num];
		if (bnriconisDSi[num]) {
			loadIcon(ndsBanner.dsi_icon[0], ndsBanner.dsi_palette[bnriconPalLine[num]], spriteIdx, true);
			bnriconPalLoaded[num] = bnriconPalLine[num];
		} else {
			loadIcon(ndsBanner.icon, ndsBanner.palette, spriteIdx, false);
		}
	}
}

void writeBannerText(std::string_view text) { writeBannerText(FontGraphic::utf8to16(text)); }
void writeBannerText(std::u16string text) {
	// Split to lines since DS game titles have manual line breaks
	std::vector<std::u16string> lines;
	size_t newline = text.find('\n');
	while (newline != text.npos) {
		lines.push_back(text.substr(0, newline));
		text = text.substr(newline + 1);
		newline = text.find('\n');
	}
	lines.push_back(text.data());
	
	// Insert line breaks if lines are too long
	for (uint i = 0; i < lines.size(); i++) {
		int width = tc().titleboxTextLarge() ? calcLargeFontWidth(lines[i]) : calcSmallFontWidth(lines[i]);
		if (width > tc().titleboxTextW()) {
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

	// Trim to the max lines if too big
	if ((int)lines.size() > tc().titleboxMaxLines())
		lines.resize(tc().titleboxMaxLines());

	// Re-combine to a single string
	std::u16string out;
	for (auto line : lines) {
		out += line + u'\n';
	}
	if (tc().titleboxTextLarge() && !ms().macroMode) {
		printLarge(false, 0, tc().titleboxTextY() - (((lines.size() - 1) * largeFontHeight()) / 2), out, Alignment::center, FontPalette::titlebox);
	} else {
		printSmall(false, 0, tc().titleboxTextY() - (((lines.size() - 1) * smallFontHeight()) / 2), out, Alignment::center, FontPalette::titlebox);
	}
}

static inline void writeDialogTitle(std::u16string text) {
	int lines = 0;
	for (auto c : text) {
		if (c == '\n') {
			lines++;
		}
	}

	printLarge(false, ms().rtl() ? 256 - 70 : 70, 31 - (lines * largeFontHeight() / 2), text, ms().rtl() ? Alignment::right : Alignment::left, FontPalette::dialog);
}
static inline void writeDialogTitleFolder(std::u16string text) {
	int lines = 0;
	for (auto c : text) {
		if (c == '\n') {
			lines++;
		}
	}

	printLarge(false, 0, tc().titleboxTextY() - (lines * largeFontHeight() / 2), text, Alignment::center, FontPalette::dialog);
}

static inline std::u16string splitLongDialogTitle(std::string_view text) {
	std::vector<std::u16string> lines;
	lines.push_back(FontGraphic::utf8to16(text));

	for (uint i = 0; i < lines.size(); i++) {
		int width = calcLargeFontWidth(lines[i]);
		if (width > 256 - 78) {
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

	std::u16string out;
	for (auto line : lines) {
		out += line + u'\n';
	}
	out.erase(out.end()-1);
	return out;
}

void titleUpdate(bool isDir, std::string_view name, int num) {
	bool theme_showdialogbox = (showdialogbox || (ms().theme == TWLSettings::EThemeSaturn && currentBg == 1) || (ms().theme == TWLSettings::EThemeHBL && dbox_showIcon));
	if (isDir) {
		if (theme_showdialogbox) {
			writeDialogTitleFolder(splitLongDialogTitle(name == ".." ? STR_BACK : name));
		} else {
			writeBannerText(name == ".." ? STR_BACK : name);
		}
	} else if (infoFound[num] || extension(name, {".nds", ".dsi", ".ids", ".srl", ".app"})) {
		// this is an nds/app file!
		// or a file with custom banner text
		if (theme_showdialogbox) {
			infoFound[num] ? writeDialogTitle(cachedTitle[num]) : writeDialogTitle(u"???");
		} else {
			infoFound[num] ? writeBannerText(cachedTitle[num]) : writeBannerText(name);
		}
	} else {
		if (theme_showdialogbox) {
			writeDialogTitle(splitLongDialogTitle(name.substr(0, name.rfind('.'))));
		} else {
			writeBannerText(name.substr(0, name.rfind('.')));
		}
	}
}
