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

#include "graphics.h"
#include "../errorScreen.h"
#include "fontHandler.h"
#include "common/tonccpy.h"
#include "common/twlmenusettings.h"
#include "graphics/gif.hpp"
#include "graphics/lodepng.h"

#include <nds.h>

extern bool fadeType;
extern bool fadeSpeed;
extern bool controlTopBright;
extern bool controlBottomBright;
int fadeDelay = 0;

int screenBrightness = 31;
bool isPng = false;

u16* pngDsImageBuffer[2];

int bg3Sub;
int bg2Main;
int bg3Main;
int bg2Sub;

void ClearBrightness(void) {
	fadeType = true;
	screenBrightness = 0;
	swiWaitForVBlank();
	swiWaitForVBlank();
}

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

	return BIT(15)|(max<<10)|(max<<5)|(max);
}

void vBlankHandler() {
	if(fadeType == true) {
		if(!fadeDelay) {
			screenBrightness--;
			if (screenBrightness < 0) screenBrightness = 0;
		}
		if (!fadeSpeed) {
			fadeDelay++;
			if (fadeDelay == 3) fadeDelay = 0;
		} else {
			fadeDelay = 0;
		}
	} else {
		if(!fadeDelay) {
			screenBrightness++;
			if (screenBrightness > 31) screenBrightness = 31;
		}
		if (!fadeSpeed) {
			fadeDelay++;
			if (fadeDelay == 3) fadeDelay = 0;
		} else {
			fadeDelay = 0;
		}
	}
	if (controlTopBright) SetBrightness(0, screenBrightness);
	if (controlBottomBright && !ms().macroMode) SetBrightness(1, screenBrightness);

	if (isPng) {
		static bool secondBuffer = false;
		dmaCopyHalfWordsAsynch(0, pngDsImageBuffer[secondBuffer], BG_GFX, (256*192)*2);
		secondBuffer = !secondBuffer;
	}

	//updateText(true);
	//updateText(false);
}

void imageLoad(const char* filename) {
	if (isPng) {
		std::vector<unsigned char> image;
		unsigned width, height;
		lodepng::decode(image, width, height, filename);
		if (width != 256 || height > 192) return;

		int yPos = 0;
		if (height <= 190) {
			// Adjust Y position
			for (int i = height; i < 192; i += 2) {
				yPos++;
			}
		}

		bool alternatePixel = false;
		for(unsigned i=0;i<image.size()/4;i++) {
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
			pngDsImageBuffer[0][i+(yPos*256)] = image[i*4]>>3 | (image[(i*4)+1]>>3)<<5 | (image[(i*4)+2]>>3)<<10 | BIT(15);
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
			pngDsImageBuffer[1][i+(yPos*256)] = image[i*4]>>3 | (image[(i*4)+1]>>3)<<5 | (image[(i*4)+2]>>3)<<10 | BIT(15);
			if ((i % 256) == 255) alternatePixel = !alternatePixel;
			alternatePixel = !alternatePixel;
		}
		return;
	}

	Gif gif(filename, false, false, true);
	std::vector<u8> pageImage = gif.frame(0).image.imageData;
	int width = gif.frame(0).descriptor.w;
	int height = gif.frame(0).descriptor.h;
	if (width != 256 || height > 192) return;

	int yPos = 0;
	if (height <= 190) {
		// Adjust Y position
		for (int i = height; i < 192; i += 2) {
			yPos++;
		}
	}

	tonccpy(BG_PALETTE, gif.gct().data(), std::min(0xF6u, gif.gct().size()) * 2);
	dmaCopyWordsAsynch(0, pageImage.data(), bgGetGfxPtr(bg3Main)+(yPos*256), 256*192);
}

void bgLoad(void) {
	if (ms().macroMode) {
		return;
	}

	Gif gif("nitro:/graphics/bg.gif", false, false, true);
	const auto &frame = gif.frame(0);
	u16 *dst = bgGetGfxPtr(bg3Sub);

	tonccpy(BG_PALETTE_SUB, gif.gct().data(), gif.gct().size() * 2);

	for(uint i = 0; i < frame.image.imageData.size() - 2; i += 2) {
		toncset16(dst++, (frame.image.imageData[i]) | (frame.image.imageData[i + 1]) << 8, 1);
	}
}

void graphicsInit() {
	*(u16*)(0x0400006C) |= BIT(14);
	*(u16*)(0x0400006C) &= BIT(15);
	SetBrightness(0, 31);
	SetBrightness(1, 31);

	////////////////////////////////////////////////////////////
	videoSetMode(MODE_5_2D);
	videoSetModeSub(MODE_5_2D);

	vramSetBankA(VRAM_A_MAIN_BG);
	//vramSetBankB(VRAM_B_MAIN_BG); // May be needed for larger images
	vramSetBankC(VRAM_C_SUB_BG);

	if (isPng) {
		//bg3Main = bgInit(3, BgType_Bmp16, BgSize_B16_256x256, 0, 0);

		videoSetMode(MODE_5_2D | DISPLAY_BG3_ACTIVE);

		REG_BG3CNT = BG_MAP_BASE(0) | BG_BMP16_256x256 | BG_PRIORITY(0);
		REG_BG3X = 0;
		REG_BG3Y = 0;
		REG_BG3PA = 1<<8;
		REG_BG3PB = 0;
		REG_BG3PC = 0;
		REG_BG3PD = 1<<8;

		pngDsImageBuffer[0] = new u16[256*192];
		pngDsImageBuffer[1] = new u16[256*192];
		toncset16(pngDsImageBuffer[0], 0, 256*192);
		toncset16(pngDsImageBuffer[1], 0, 256*192);
	} else {
		bg3Main = bgInit(3, BgType_Bmp8, BgSize_B8_256x256, 0, 0);
	}
	bgSetPriority(bg3Main, 3);

	//bg2Main = bgInit(2, BgType_Bmp8, BgSize_B8_256x256, 3, 0);
	//bgSetPriority(bg2Main, 0);

	bg3Sub = bgInitSub(3, BgType_Bmp8, BgSize_B8_256x256, 0, 0);
	bgSetPriority(bg3Sub, 3);

	bg2Sub = bgInitSub(2, BgType_Bmp8, BgSize_B8_256x256, 3, 0);
	bgSetPriority(bg2Sub, 0);

	if (ms().macroMode) {
		lcdMainOnBottom();
	}

	irqSet(IRQ_VBLANK, vBlankHandler);
	irqEnable(IRQ_VBLANK);
}
