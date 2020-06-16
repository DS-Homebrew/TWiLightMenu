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
#include <nds.h>

#include "common/dsimenusettings.h"
#include "common/tonccpy.h"
#include "fontHandler.h"
#include "soundeffect.h"

#include "top_bg.h"
#include "sub_bg.h"
#include "saturn_bg.h"

extern bool fadeType;
int screenBrightness = 31;
static int currentTheme = 0;

int frameOf60fps = 60;
int frameDelay = 0;
bool frameDelayEven = true; // For 24FPS
bool renderFrame = true;

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

void frameRateHandler(void) {
	frameOf60fps++;
	if (frameOf60fps > 60) frameOf60fps = 1;

	if (!renderFrame) {
		frameDelay++;
		switch (ms().fps) {
			case 11:
				renderFrame = (frameDelay == 5+frameDelayEven);
				break;
			case 24:
			//case 25:
				renderFrame = (frameDelay == 2+frameDelayEven);
				break;
			case 48:
				renderFrame = (frameOf60fps != 3
							&& frameOf60fps != 8
							&& frameOf60fps != 13
							&& frameOf60fps != 18
							&& frameOf60fps != 23
							&& frameOf60fps != 28
							&& frameOf60fps != 33
							&& frameOf60fps != 38
							&& frameOf60fps != 43
							&& frameOf60fps != 48
							&& frameOf60fps != 53
							&& frameOf60fps != 58);
				break;
			case 50:
				renderFrame = (frameOf60fps != 3
							&& frameOf60fps != 9
							&& frameOf60fps != 16
							&& frameOf60fps != 22
							&& frameOf60fps != 28
							&& frameOf60fps != 34
							&& frameOf60fps != 40
							&& frameOf60fps != 46
							&& frameOf60fps != 51
							&& frameOf60fps != 58);
				break;
			default:
				renderFrame = (frameDelay == 60/ms().fps);
				break;
		}
	}
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

// Copys a palette and applies filtering if enabled
void copyPalette(u16 *dst, const u16 *src, int size) {
	if(ms().colorMode == 1) { // Grayscale
		for(int i = 0; i < size; i++) {
			dst[i] = convertVramColorToGrayscale(src[i]);
		}
	} else {
		tonccpy(dst, src, size);
	}
}

// TODO: If adding sprites, this would work better as one
void drawScroller(int y, int h) {
	// Reset layer
	dmaCopyWords(0, currentTheme == 4 ? saturn_bgBitmap : sub_bgBitmap , bgGetGfxPtr(7), sub_bgBitmapLen);

	const u8 scroller[4] = {2, 3, 3, 2};
	u8 *dst = (u8*)bgGetGfxPtr(7) + 250;
	toncset16(dst + y * 256, 2 | 2 << 8, 2);
	for(int i = 1; i < h - 1; i++) {
		tonccpy(dst + (y + i) * 256, scroller, sizeof(scroller));
	}
	toncset16(dst + (y + h - 1) * 256, 2 | 2 << 8, 2);
}

void vBlankHandler()
{
	if(fadeType == true) {
		screenBrightness--;
		if (screenBrightness < 0) screenBrightness = 0;
	} else {
		screenBrightness++;
		if (screenBrightness > 31) screenBrightness = 31;
	}
	if (renderFrame) {
		SetBrightness(0, currentTheme == 4 ? -screenBrightness : screenBrightness);
		SetBrightness(1, currentTheme == 4 ? -screenBrightness : screenBrightness);
	}

	if (currentTheme != 4) {
		snd().tickBgMusic();
	}

	updateText(false);
	updateText(true);

	if (renderFrame) {
		frameDelay = 0;
		frameDelayEven = !frameDelayEven;
		renderFrame = false;
	}
}

void graphicsInit() {
	currentTheme = ms().theme;

	SetBrightness(0, currentTheme == 4 ? -31 : 31);
	SetBrightness(1, currentTheme == 4 ? -31 : 31);

	videoSetMode(MODE_5_2D | DISPLAY_BG3_ACTIVE);
	videoSetModeSub(MODE_5_2D | DISPLAY_BG3_ACTIVE);

	vramSetBankA(VRAM_A_MAIN_BG);
	vramSetBankC(VRAM_C_SUB_BG);

	int bg3Main = bgInit(3, BgType_Bmp8, BgSize_B8_256x256, 0, 0);
	bgSetPriority(bg3Main, 3);

	int bg2Main = bgInit(2, BgType_Bmp8, BgSize_B8_256x256, 3, 0);
	bgSetPriority(bg2Main, 0);

	int bg3Sub = bgInitSub(3, BgType_Bmp8, BgSize_B8_256x256, 0, 0);
	bgSetPriority(bg3Sub, 3);

	int bg2Sub = bgInitSub(2, BgType_Bmp8, BgSize_B8_256x256, 3, 0);
	bgSetPriority(bg2Sub, 0);

	if(currentTheme != 4) {
		tonccpy(bgGetGfxPtr(bg3Main), top_bgBitmap, top_bgBitmapLen);
		copyPalette(BG_PALETTE + 0x10, top_bgPal, top_bgPalLen);
		tonccpy(bgGetGfxPtr(bg3Sub), sub_bgBitmap, sub_bgBitmapLen);
		copyPalette(BG_PALETTE_SUB + 0x10, sub_bgPal, sub_bgPalLen);
	} else {
		tonccpy(bgGetGfxPtr(bg3Main), saturn_bgBitmap, saturn_bgBitmapLen);
		copyPalette(BG_PALETTE + 0x10, saturn_bgPal, saturn_bgPalLen);
		tonccpy(bgGetGfxPtr(bg3Sub), saturn_bgBitmap, saturn_bgBitmapLen);
		copyPalette(BG_PALETTE_SUB + 0x10, saturn_bgPal, saturn_bgPalLen);
	}

	irqSet(IRQ_VBLANK, vBlankHandler);
	irqEnable(IRQ_VBLANK);
	irqSet(IRQ_VCOUNT, frameRateHandler);
	irqEnable(IRQ_VCOUNT);
}