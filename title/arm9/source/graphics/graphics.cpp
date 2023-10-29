/*-----------------------------------------------------------------
 Copyright (C) 2015
	Matthew Scholefield

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
#include <nds/dma.h>
#include <maxmod9.h>
#include "bios_decompress_callback.h"
#include "common/twlmenusettings.h"
#include "common/systemdetails.h"
#include "common/gl2d.h"
#include "common/tonccpy.h"
#include "graphics.h"
#include "common/ColorLut.h"
#include "common/lodepng.h"

#define CONSOLE_SCREEN_WIDTH 32
#define CONSOLE_SCREEN_HEIGHT 24

extern bool fadeType;
extern bool fadeColor;
bool controlTopBright = true;
bool controlBottomBright = true;
int screenBrightness = 31;

bool twlMenuSplash = false;
extern void twlMenuVideo_loadTopGraphics(void);
extern void twlMenuVideo_topGraphicRender(void);

bool doubleBuffer = false;
bool doubleBufferTop = true;
bool secondBuffer = false;

u16 frameBuffer[2][256*192];
u16 frameBufferBot[2][256*192];
u16* dsiSplashLocation = (u16*)0x02600000;

// Ported from PAlib (obsolete)
void SetBrightness(u8 screen, s8 bright) {
	u16 mode = 1 << 14;

	if (bright < 0) {
		mode = 2 << 14;
		bright = -bright;
	}
	if (bright > 31) bright = 31;
	*(u16*)(0x0400006C + (0x1000 * screen)) = bright + mode;
}

u16 convertVramColorToGrayscale(u16 val) {
	u8 b,g,r,max,min;
	b = ((val)>>10)&31;
	g = ((val)>>5)&31;
	r = (val)&31;
	// Value decomposition of hsv
	max = (b > g) ? b : g;
	max = (max > r) ? max : r;

	// Desaturate
	min = (b < g) ? b : g;
	min = (min < r) ? min : r;
	max = (max + min) / 2;

	return 32768|(max<<10)|(max<<5)|(max);
}

void vBlankHandler() {
	if (fadeType == true) {
		screenBrightness--;
		if (screenBrightness < 0) screenBrightness = 0;
	} else {
		screenBrightness++;
		if (screenBrightness > 31) screenBrightness = 31;
	}
	if (controlTopBright) SetBrightness(0, fadeColor ? screenBrightness : -screenBrightness);
	if (controlBottomBright && !ms().macroMode) SetBrightness(1, fadeColor ? screenBrightness : -screenBrightness);
	if (doubleBuffer) {
		if (doubleBufferTop) {
			dmaCopyWordsAsynch(0, frameBuffer[secondBuffer], BG_GFX, 0x18000);
		}
		dmaCopyWordsAsynch(1, frameBufferBot[secondBuffer], BG_GFX_SUB, 0x18000);
		secondBuffer = !secondBuffer;
	}
}

void LoadBMP(void) {
	dmaFillHalfWords(0, BG_GFX, 0x18000);
	dmaFillHalfWords(0, BG_GFX_SUB, 0x18000);

	std::vector<unsigned char> image;
	unsigned width, height;

	lodepng::decode(image, width, height, (sys().isDSPhat() || ms().colorMode == 2) ? "nitro:/graphics/logoPhat_rocketrobz.png" : "nitro:/graphics/logo_rocketrobz.png");
	bool alternatePixel = false;
	for (unsigned i=0;i<image.size()/4;i++) {
		image[(i*4)+3] = 0;
		if (alternatePixel) {
			if (image[(i*4)] >= 0x4) {
				image[(i*4)] -= 0x4;
				image[(i*4)+3] |= BIT(0);
			}
			if (image[(i*4)+1] >= 0x4) {
				image[(i*4)+1] -= 0x4;
				image[(i*4)+3] |= BIT(1);
			}
			if (image[(i*4)+2] >= 0x4) {
				image[(i*4)+2] -= 0x4;
				image[(i*4)+3] |= BIT(2);
			}
		}
		u16 color = image[i*4]>>3 | (image[(i*4)+1]>>3)<<5 | (image[(i*4)+2]>>3)<<10 | BIT(15);
		if (ms().macroMode) {
			frameBuffer[0][i] = color;
		} else {
			frameBufferBot[0][i] = color;
		}
		if (alternatePixel) {
			if (image[(i*4)+3] & BIT(0)) {
				image[(i*4)] += 0x4;
			}
			if (image[(i*4)+3] & BIT(1)) {
				image[(i*4)+1] += 0x4;
			}
			if (image[(i*4)+3] & BIT(2)) {
				image[(i*4)+2] += 0x4;
			}
		} else {
			if (image[(i*4)] >= 0x4) {
				image[(i*4)] -= 0x4;
			}
				if (image[(i*4)+1] >= 0x4) {
				image[(i*4)+1] -= 0x4;
			}
			if (image[(i*4)+2] >= 0x4) {
				image[(i*4)+2] -= 0x4;
			}
		}
		color = image[i*4]>>3 | (image[(i*4)+1]>>3)<<5 | (image[(i*4)+2]>>3)<<10 | BIT(15);
		if (ms().macroMode) {
			frameBuffer[1][i] = color;
		} else {
			frameBufferBot[1][i] = color;
		}
		if ((i % 256) == 255) alternatePixel = !alternatePixel;
		alternatePixel = !alternatePixel;
	}
	image.clear();
	if (ms().colorMode > 0) {
		#define colorsToCache (256*32)

		u16* prevColor = new u16[colorsToCache];
		u16* colorConv = new u16[colorsToCache];
		u16* prevColor2 = new u16[colorsToCache];
		u16* colorConv2 = new u16[colorsToCache];

		int colorsCached = 0;
		int colorsCached2 = 0;
		int accessCounter = 0;
		int accessCounter2 = 0;

		for (int i=0; i<256*192; i++) {
			u16 color = frameBufferBot[0][i];
			u16 color2 = frameBufferBot[1][i];
			if (ms().macroMode) {
				color = frameBuffer[0][i];
				color2 = frameBuffer[1][i];
			}

			int c = 0;
			int c2 = 0;
			bool cachedColorFound = false;
			bool cachedColorFound2 = false;
			if (i > 0) {
				for (c = 0; c < colorsCached; c++) {
					if (prevColor[c] == color) {
						cachedColorFound = true;
						break;
					}
				}
				for (c2 = 0; c2 < colorsCached2; c2++) {
					if (prevColor2[c2] == color2) {
						cachedColorFound2 = true;
						break;
					}
				}
			}
			if (!cachedColorFound) {
				c = accessCounter;

				if (ms().colorMode == 2) {
					colorConv[c] = convertDSColorToPhat(color);
				} else if (ms().colorMode == 1) {
					colorConv[c] = convertVramColorToGrayscale(color);
				}
				colorsCached++;
				if (colorsCached > colorsToCache) colorsCached = colorsToCache;
				accessCounter++;
				if (accessCounter >= colorsToCache) accessCounter = 0;

				prevColor[c] = color;
			}
			if (!cachedColorFound2) {
				c2 = accessCounter2;

				if (color2 == color) {
					colorConv2[c2] = colorConv[c];
				} else {
					if (ms().colorMode == 2) {
						colorConv2[c2] = convertDSColorToPhat(color2);
					} else if (ms().colorMode == 1) {
						colorConv2[c2] = convertVramColorToGrayscale(color2);
					}
				}
				colorsCached2++;
				if (colorsCached2 > colorsToCache) colorsCached2 = colorsToCache;
				accessCounter2++;
				if (accessCounter2 >= colorsToCache) accessCounter2 = 0;

				prevColor2[c2] = color2;
			}
			color = colorConv[c];
			color2 = colorConv2[c2];

			if (ms().macroMode) {
				frameBuffer[0][i] = color;
				frameBuffer[1][i] = color2;
			} else {
				frameBufferBot[0][i] = color;
				frameBufferBot[1][i] = color2;
			}
		}

		delete[] prevColor;
		delete[] colorConv;
		delete[] prevColor2;
		delete[] colorConv2;
	}
	doubleBuffer = true;
	if (ms().macroMode) {
		fadeType = true;
		for (int i = 0; i < 60 * 3; i++) {
			scanKeys();
			if ((keysHeld() & KEY_START) || (keysHeld() & KEY_SELECT) || (keysHeld() & KEY_TOUCH)) break;
			swiWaitForVBlank();
		}
		fadeType = false;
		for (int i = 0; i < 25; i++) {
			swiWaitForVBlank();
		}
		doubleBuffer = false;
		swiWaitForVBlank();
		dmaFillHalfWords(0, BG_GFX, 0x18000);
	}

	doubleBufferTop = false;
}

static bool graphicIrqRunning = false;

void runGraphicIrq(void) {
	if (graphicIrqRunning) return;

	*(u16*)(0x0400006C) |= BIT(14);
	*(u16*)(0x0400006C) &= BIT(15);
	SetBrightness(0, 31);
	SetBrightness(1, 31);
	if (ms().macroMode) {
		lcdMainOnBottom();
		powerOff(PM_BACKLIGHT_TOP);
	}

	irqSet(IRQ_VBLANK, vBlankHandler);
	irqEnable(IRQ_VBLANK);

	graphicIrqRunning = true;
}

bool screenFadedIn(void) { return (screenBrightness == 0); }

bool screenFadedOut(void) { return (screenBrightness > 24); }

void loadTitleGraphics() {
	videoSetMode(MODE_5_3D | DISPLAY_BG3_ACTIVE);
	videoSetModeSub(MODE_3_2D | DISPLAY_BG3_ACTIVE);

	// sprites
	vramSetBankA(VRAM_A_TEXTURE);
	vramSetBankB(VRAM_B_TEXTURE);
	vramSetBankC(VRAM_C_SUB_BG_0x06200000);
	vramSetBankD(VRAM_D_MAIN_BG_0x06000000);
	vramSetBankE(VRAM_E_TEX_PALETTE);
	vramSetBankF(VRAM_F_TEX_PALETTE_SLOT4);
	vramSetBankG(VRAM_G_MAIN_SPRITE);
	vramSetBankH(VRAM_H_SUB_BG_EXT_PALETTE);
	vramSetBankI(VRAM_I_SUB_SPRITE_EXT_PALETTE);

	REG_BG3CNT = BG_MAP_BASE(0) | BG_BMP16_256x256 | BG_PRIORITY(0);
	REG_BG3X = 0;
	REG_BG3Y = 0;
	REG_BG3PA = 1<<8;
	REG_BG3PB = 0;
	REG_BG3PC = 0;
	REG_BG3PD = 1<<8;
	REG_BLDCNT = BLEND_SRC_BG3 | BLEND_FADE_BLACK;
	REG_BLDY = 0;

	REG_BG3CNT_SUB = BG_MAP_BASE(0) | BG_BMP16_256x256 | BG_PRIORITY(0);
	REG_BG3X_SUB = 0;
	REG_BG3Y_SUB = 0;
	REG_BG3PA_SUB = 1<<8;
	REG_BG3PB_SUB = 0;
	REG_BG3PC_SUB = 0;
	REG_BG3PD_SUB = 1<<8;

	// Clear the background palettes
	toncset16(BG_PALETTE, 0, 256);
	toncset16(BG_PALETTE_SUB, 0, 256);

	twlMenuVideo_loadTopGraphics();

	// Display TWiLightMenu++ logo
	LoadBMP();
}