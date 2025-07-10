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
#include <gl2d.h>
#include "common/tonccpy.h"
#include "fileBrowse.h"
#include "graphics/fontHandler.h"
#include "graphics/iconHandler.h"
#include "common/lodepng.h"
#include "common/logging.h"
#include "graphics/paletteEffects.h"
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
static inline void loadGBAIcon(int num) { glLoadIcon(num, tex().iconGBATexture()->palette(), tex().iconGBATexture()->bytes()); }
static inline void loadGBIcon(int num) { glLoadIcon(num, tex().iconGBTexture()->palette(), tex().iconGBTexture()->bytes()); }
static inline void loadGBCIcon(int num) { glLoadIcon(num, tex().iconGBTexture()->palette(), tex().iconGBTexture()->bytes()+(32*16)); }
static inline void loadNESIcon(int num) { glLoadIcon(num, tex().iconNESTexture()->palette(), tex().iconNESTexture()->bytes()); }
static inline void loadSGIcon(int num) { glLoadIcon(num, tex().iconSGTexture()->palette(), tex().iconSGTexture()->bytes()); }
static inline void loadSMSIcon(int num) { glLoadIcon(num, tex().iconSMSTexture()->palette(), tex().iconSMSTexture()->bytes()); }
static inline void loadGGIcon(int num) { glLoadIcon(num, tex().iconGGTexture()->palette(), tex().iconGGTexture()->bytes()); }
static inline void loadMDIcon(int num) { glLoadIcon(num, tex().iconMDTexture()->palette(), tex().iconMDTexture()->bytes()); }
static inline void loadSNESIcon(int num) { glLoadIcon(num, tex().iconSNESTexture()->palette(), tex().iconSNESTexture()->bytes()); }
static inline void loadPLGIcon(int num) { glLoadIcon(num, tex().iconPLGTexture()->palette(), tex().iconPLGTexture()->bytes()); }
static inline void loadA26Icon(int num) { glLoadIcon(num, tex().iconA26Texture()->palette(), tex().iconA26Texture()->bytes()); }
static inline void loadCOLIcon(int num) { glLoadIcon(num, tex().iconCOLTexture()->palette(), tex().iconCOLTexture()->bytes()); }
static inline void loadM5Icon(int num) { glLoadIcon(num, tex().iconM5Texture()->palette(), tex().iconM5Texture()->bytes()); }
static inline void loadINTIcon(int num) { glLoadIcon(num, tex().iconINTTexture()->palette(), tex().iconINTTexture()->bytes()); }
static inline void loadPCEIcon(int num) { glLoadIcon(num, tex().iconPCETexture()->palette(), tex().iconPCETexture()->bytes()); }
static inline void loadWSIcon(int num) { glLoadIcon(num, tex().iconWSTexture()->palette(), tex().iconWSTexture()->bytes()); }
static inline void loadNGPIcon(int num) { glLoadIcon(num, tex().iconNGPTexture()->palette(), tex().iconNGPTexture()->bytes()); }
static inline void loadCPCIcon(int num) { glLoadIcon(num, tex().iconCPCTexture()->palette(), tex().iconCPCTexture()->bytes()); }
static inline void loadVIDIcon(int num) { glLoadIcon(num, tex().iconVIDTexture()->palette(), tex().iconVIDTexture()->bytes()); }
static inline void loadIMGIcon(int num) { glLoadIcon(num, tex().iconIMGTexture()->palette(), tex().iconIMGTexture()->bytes()); }
static inline void loadMSXIcon(int num) { glLoadIcon(num, tex().iconMSXTexture()->palette(), tex().iconMSXTexture()->bytes()); }
static inline void loadMINIcon(int num) { glLoadIcon(num, tex().iconMINITexture()->palette(), tex().iconMINITexture()->bytes()); }
static inline void loadHBIcon(int num) { glLoadIcon(num, tex().iconHBTexture()->palette(), tex().iconHBTexture()->bytes()); }

static inline void clearIcon(int num) { glClearIcon(num); }

void convertIconPalette(sNDSBannerExt* ndsBanner) {
	effectColorModePalette(ndsBanner->palette, 16);

	if (ms().animateDsiIcons && ndsBanner->version == NDS_BANNER_VER_DSi) {
		for (int i = 0; i < 8; i++) {
			effectColorModePalette(ndsBanner->dsi_palette[i], 16);
		}
	}
}

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
	toncset(gameTid[num], 0, 4);
	isValid[num] = false;
	isTwlm[num] = false;
	isUnlaunch[num] = false;
	isDSiWare[num] = false;
	isHomebrew[num] = true;
	isModernHomebrew[num] = true;
	requiresRamDisk[num] = false;
	requiresDonorRom[num] = false;
	if (!fromArgv) {
		bnriconisDSi[num] = false;
		customIcon[num] = 0;
		infoFound[num] = false;
	}

	if (ms().showCustomIcons && customIcon[num] < 2 && (!fromArgv || customIcon[num] <= 0)) {
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
		snprintf(customIconPath, sizeof(customIconPath), "%s:/_nds/TWiLightMenu/icons/%s.bin", sys().isRunFromSD() ? "sd" : "fat", name);
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
					convertIconPalette(&banner);
				}
			}
		} else if (customIcon[num] == 0) {
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
						banner.icon[pos] &= nibble? 0x0f : 0xf0;
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
	
						convertIconPalette(&banner);
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
	} else if (extension(name, {".gbc"})) {
		// this is a gbc file!
		FILE *fp;

		// open file for reading info
		fp = fopen(name, "rb");
		if (!fp) {
			fclose(fp);
			return;
		}

		fseek(fp, 0x13F, SEEK_SET);
		fread(gameTid[num], 1, 4, fp);

		fclose(fp);
	} else if (extension(name, {".agb", ".gba", ".mb"})) {
		// this is a gba file!
		FILE *fp;

		// open file for reading info
		fp = fopen(name, "rb");
		if (!fp) {
			fclose(fp);
			return;
		}

		fseek(fp, 0xAC, SEEK_SET);
		fread(gameTid[num], 1, 4, fp);

		fclose(fp);
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
			isValid[num] = (ndsHeader.arm9destination >= 0x02000000 && ndsHeader.arm9destination < 0x03000000 && ndsHeader.arm9executeAddress >= 0x02000000 && ndsHeader.arm9executeAddress < 0x03000000);
			isTwlm[num] = (strcmp(gameTid[num], "SRLA") == 0);
			isUnlaunch[num] = (memcmp(ndsHeader.gameTitle, "UNLAUNCH.DSI", 12) == 0);
			romVersion[num] = ndsHeader.romversion;
			unitCode[num] = ndsHeader.unitCode;
			headerCRC[num] = ndsHeader.headerCRC16;
			a7mbk6[num] = ndsHeader.a7mbk6;
		}

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
			if (!ms().secondaryDevice && num < 40) {
				if ((ndsHeader.arm9binarySize == 0x98F70 && ndsHeader.arm7binarySize == 0xED94)		// jEnesisDS 0.7.4
				|| (ndsHeader.arm9binarySize == 0x48950 && ndsHeader.arm7binarySize == 0x74C4)			// SNEmulDS06-WIP2
				|| (ndsHeader.arm9binarySize == 0xD45C0 && ndsHeader.arm7binarySize == 0x2B7C)			// ikuReader v0.058
				|| (ndsHeader.arm9binarySize == 0x54620 && ndsHeader.arm7binarySize == 0x1538)) {		// XRoar 0.24fp3
					requiresRamDisk[num] = true;
				}
			}
		} else if (ndsHeader.unitCode != 0 && (ndsHeader.accessControl & BIT(4))) {
			isDSiWare[num] = true; // Is a DSiWare game
		}

		if (num < 40 && !isHomebrew[num]) {
			// Check if ROM needs a donor ROM
			bool dsiEnhancedMbk = (isDSiMode() && *(u32*)0x02FFE1A0 == 0x00403000 && sys().arm7SCFGLocked());
			if (isDSiMode() && (a7mbk6[num] == (dsiEnhancedMbk ? 0x080037C0 : 0x00403000) || (ndsHeader.gameCode[0] == 'H' && ndsHeader.arm7binarySize < 0xC000 && ndsHeader.arm7idestination == 0x02E80000 && (REG_MBK9 & 0x00FFFFFF) != 0x00FFFF0F)) && sys().arm7SCFGLocked()) {
				requiresDonorRom[num] = dsiEnhancedMbk ? 51 : 52; // DSi-Enhanced ROM required on CycloDSi, or DSi-Exclusive/DSiWare ROM required on DSiWarehax
				if (ndsHeader.gameCode[0] == 'H' && ndsHeader.arm7binarySize < 0xC000 && ndsHeader.arm7idestination == 0x02E80000) {
					requiresDonorRom[num] += 100;
				} else if (memcmp(ndsHeader.gameCode, "KCX", 3) == 0 && dsiEnhancedMbk) {
					requiresDonorRom[num] = 0;
				}
			} else if (a7mbk6[num] == 0x080037C0 && ms().secondaryDevice && (!dsiFeatures() || bs().b4dsMode)
			&& (((sys().dsDebugRam() || (dsiFeatures() && bs().b4dsMode == 2)) ? (memcmp(ndsHeader.gameCode, "DME", 3) == 0 || memcmp(ndsHeader.gameCode, "DMF", 3) == 0 || memcmp(ndsHeader.gameCode, "DMD", 3) == 0 || memcmp(ndsHeader.gameCode, "DMP", 3) == 0 || memcmp(ndsHeader.gameCode, "DHS", 3) == 0 || memcmp(ndsHeader.gameCode, "DSY", 3) == 0) : (memcmp(ndsHeader.gameCode, "DMF", 3) == 0 || memcmp(ndsHeader.gameCode, "DMP", 3) == 0 || memcmp(ndsHeader.gameCode, "DHS", 3) == 0))
			|| (ndsHeader.gameCode[0] != 'D' && memcmp(ndsHeader.gameCode, "KCX", 3) != 0 && memcmp(ndsHeader.gameCode, "KAV", 3) != 0 && memcmp(ndsHeader.gameCode, "KNK", 3) != 0 && memcmp(ndsHeader.gameCode, "KE3", 3) != 0))) {
				requiresDonorRom[num] = 51; // SDK5 ROM required
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
			convertIconPalette(&ndsBanner);
			if (ndsBanner.crc[3] == crc16) { // Check if CRC16 is valid
				grabBannerSequence(num);
				bnriconisDSi[num] = true;
			}
		} else {
			convertIconPalette(&ndsBanner);
		}
	}
}

void iconUpdate(bool isDir, const char *name, int num) {
	logPrint("iconUpdate: ");

	int spriteIdx = num == -1 ? 6 : num % 6;
	if (num == -1)
		num = 40;

	const bool isNds = (bnrRomType[num] == 0);

	if (customIcon[num] > 0 || (customIcon[num] && !isDir && isNds)) {
		sNDSBannerExt &ndsBanner = bnriconTile[num];
		if (customIcon[num] == -1) {
			logPrint(isDir ? "Custom icon invalid!" : "Banner not found or custom icon invalid!");
			loadUnkIcon(spriteIdx);
		} else if (bnriconisDSi[num]) {
			logPrint("Custom icon found!");
			loadIcon(ndsBanner.dsi_icon[0], ndsBanner.dsi_palette[bnriconPalLine[num]], spriteIdx, true);
			bnriconPalLoaded[num] = bnriconPalLine[num];
		} else {
			logPrint("Custom icon found!");
			loadIcon(ndsBanner.icon, ndsBanner.palette, spriteIdx, false);
		}
	} else if (isDir) {
		logPrint("Folder found!");
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
			logPrint("Icon not found!\n");
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
					logPrint("Icon not found!");
					clearIcon(spriteIdx);
				} else if (S_ISDIR(st.st_mode)) {
					// this is a directory!
					logPrint("Folder found!");
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
	} else if (isNds) {
		// this is an nds/app file!
		logPrint("NDS icon found!");
		sNDSBannerExt &ndsBanner = bnriconTile[num];
		if (bnriconisDSi[num]) {
			loadIcon(ndsBanner.dsi_icon[0], ndsBanner.dsi_palette[bnriconPalLine[num]], spriteIdx, true);
			bnriconPalLoaded[num] = bnriconPalLine[num];
		} else {
			loadIcon(ndsBanner.icon, ndsBanner.palette, spriteIdx, false);
		}
	} else if (bnrRomType[num] == 10) {
		tex().loadIconA26Texture();
		loadA26Icon(spriteIdx);
	} else if (bnrRomType[num] == 21) {
		tex().loadIconMSXTexture();
		loadMSXIcon(spriteIdx);
	} else if (bnrRomType[num] == 13) {
		tex().loadIconCOLTexture();
		loadCOLIcon(spriteIdx);
	} else if (bnrRomType[num] == 14) {
		tex().loadIconM5Texture();
		loadM5Icon(spriteIdx);
	} else if (bnrRomType[num] == 12) {
		tex().loadIconINTTexture();
		loadINTIcon(spriteIdx);
	} else if (bnrRomType[num] == 9) {
		tex().loadIconPLGTexture();
		loadPLGIcon(spriteIdx);
	} else if (bnrRomType[num] == 19) {
		tex().loadIconVIDTexture();
		loadVIDIcon(spriteIdx);
	} else if (bnrRomType[num] == 20) {
		tex().loadIconIMGTexture();
		loadIMGIcon(spriteIdx);
	} else if (bnrRomType[num] == 1) {
		tex().loadIconGBATexture();
		loadGBAIcon(spriteIdx);
	} else if (bnrRomType[num] == 2) {
		tex().loadIconGBTexture();
		loadGBIcon(spriteIdx);
	} else if (bnrRomType[num] == 3) {
		tex().loadIconGBTexture();
		loadGBCIcon(spriteIdx);
	} else if (bnrRomType[num] == 4) {
		tex().loadIconNESTexture();
		loadNESIcon(spriteIdx);
	} else if (bnrRomType[num] == 15) {
		tex().loadIconSGTexture();
		loadSGIcon(spriteIdx);
	} else if (bnrRomType[num] == 5) {
		tex().loadIconSMSTexture();
		loadSMSIcon(spriteIdx);
	} else if (bnrRomType[num] == 6) {
		tex().loadIconGGTexture();
		loadGGIcon(spriteIdx);
	} else if (bnrRomType[num] == 7) {
		tex().loadIconMDTexture();
		loadMDIcon(spriteIdx);
	} else if (bnrRomType[num] == 8) {
		tex().loadIconSNESTexture();
		loadSNESIcon(spriteIdx);
	} else if (bnrRomType[num] == 11) {
		tex().loadIconPCETexture();
		loadPCEIcon(spriteIdx);
	} else if (bnrRomType[num] == 16) {
		tex().loadIconWSTexture();
		loadWSIcon(spriteIdx);
	} else if (bnrRomType[num] == 17) {
		tex().loadIconNGPTexture();
		loadNGPIcon(spriteIdx);
	} else if (bnrRomType[num] == 18) {
		tex().loadIconCPCTexture();
		loadCPCIcon(spriteIdx);
	} else if (bnrRomType[num] == 22) {
		tex().loadIconMINITexture();
		loadMINIcon(spriteIdx);
	} else if (bnrRomType[num] == 23) {
		tex().loadIconHBTexture();
		loadHBIcon(spriteIdx);
	} else {
		loadUnkIcon(spriteIdx);
	}
	logPrint("\n");
}

void writeBannerText(std::string_view name, std::string_view text) { writeBannerText(FontGraphic::utf8to16(name), FontGraphic::utf8to16(text)); }
void writeBannerText(std::string_view name, std::u16string text) { writeBannerText(FontGraphic::utf8to16(name), text); }
void writeBannerText(std::u16string name, std::u16string text) {
	if (ms().filenameDisplay == 2) {
		text = name;
	}
	const bool nameAndTextMatch = (name == text);

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
	if (tc().titleboxTextLarge() && !ms().macroMode && ((ms().filenameDisplay == 0) || nameAndTextMatch)) {
		printLarge(false, 0, tc().titleboxTextY() - (((lines.size() - 1) * largeFontHeight()) / 2), out, Alignment::center, FontPalette::titlebox);
	} else if (tc().titleboxTextLarge() && (ms().filenameDisplay == 1) && !ms().macroMode && !nameAndTextMatch) {
		printSmall(false, 8, tc().titleboxTextY() - (smallFontHeight() * 1.5), name, Alignment::left, FontPalette::titlebox);
		printSmall(false, 0, (tc().titleboxTextY() + (smallFontHeight() / 1.5)) - (((lines.size() - 1) * smallFontHeight()) / 2), out, Alignment::center, FontPalette::titlebox);
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
	const bool theme_showdialogbox = (showdialogbox || (ms().theme == TWLSettings::EThemeSaturn && currentBg == 1) || (ms().theme == TWLSettings::EThemeHBL && dbox_showIcon));
	if (isDir) {
		if (theme_showdialogbox) {
			writeDialogTitleFolder(splitLongDialogTitle(name == ".." ? STR_BACK : name));
		} else {
			writeBannerText(name, name == ".." ? STR_BACK : name);
		}
	} else {
		if (theme_showdialogbox) {
			infoFound[num] ? writeDialogTitle(cachedTitle[num]) : writeDialogTitle(u"???");
		} else {
			if ((ms().filenameDisplay == 0) && !infoFound[num]) {
				std::string_view nameSubstr = name.substr(0, name.rfind('.'));
				writeBannerText(nameSubstr, nameSubstr);
			} else {
				infoFound[num] ? writeBannerText(name, cachedTitle[num]) : writeBannerText(name, name);
			}
		}
	}
}
