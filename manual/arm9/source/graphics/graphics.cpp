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
#include "graphics/lodepng.h"

#include <nds.h>

extern bool fadeType;
extern bool fadeSpeed;
extern bool controlTopBright;
extern bool controlBottomBright;
extern int colorMode;
extern int blfLevel;
int fadeDelay = 0;

int bgColor1 = 0x6F7B;
int bgColor2 = 0x77BD;

int screenBrightness = 31;

std::vector<u16> pageImage;

extern int pageYpos;
extern int pageYsize;

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

u16 convertToDsBmp(u16 val) {
	if (colorMode == 1) {
		u16 newVal = ((val>>10)&31) | (val&31<<5) | (val&31)<<10 | BIT(15);

		u8 b,g,r,max,min;
		b = ((newVal)>>10)&31;
		g = ((newVal)>>5)&31;
		r = (newVal)&31;
		// Value decomposition of hsv
		max = (b > g) ? b : g;
		max = (max > r) ? max : r;

		// Desaturate
		min = (b < g) ? b : g;
		min = (min < r) ? min : r;
		max = (max + min) / 2;

		newVal = 32768|(max<<10)|(max<<5)|(max);

		b = ((newVal)>>10)&(31-6*blfLevel);
		g = ((newVal)>>5)&(31-3*blfLevel);
		r = (newVal)&31;

		return 32768|(b<<10)|(g<<5)|(r);
	} else {
		return ((val>>10)&31) | (val&(31-3*blfLevel)<<5) | (val&(31-6*blfLevel))<<10 | BIT(15);
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
	if (controlBottomBright) SetBrightness(0, screenBrightness);
	if (controlTopBright) SetBrightness(1, screenBrightness);

	updateText(true);
	// updateText(false);
}

void pageLoad(const std::string &filename) {
	std::vector<unsigned char> image;
	unsigned width, height;
	lodepng::decode(image, width, height, filename);
	pageYsize = height;
	pageImage = std::vector<u16>((pageYsize + 192) * width);
	for(unsigned i=0;i<image.size()/4;i++) {
		pageImage[i] = image[i*4]>>3 | (image[(i*4)+1]>>3)<<5 | (image[(i*4)+2]>>3)<<10 | BIT(15);
	}

	dmaCopyWordsAsynch(0, pageImage.data(), bgGetGfxPtr(bg3Main)+(18*256), 0x15C00);
	dmaCopyWordsAsynch(1, pageImage.data()+(174*256), bgGetGfxPtr(bg3Sub), 0x18000);
	for(int i=0;i<192;i++) {
		if(i%2)	dmaFillHalfWords(((bgColor1>>10)&0x1f) | ((bgColor1)&(0x1f<<5)) | (bgColor1&0x1f)<<10 | BIT(15), pageImage.data()+(pageYsize*256)+(i*256), 512);
		else	dmaFillHalfWords(((bgColor2>>10)&0x1f) | ((bgColor2)&(0x1f<<5)) | (bgColor2&0x1f)<<10 | BIT(15), pageImage.data()+(pageYsize*256)+(i*256), 512);
	}
}

void pageScroll(void) {
	dmaCopyWordsAsynch(0, pageImage.data()+(pageYpos*256), bgGetGfxPtr(bg3Main)+(18*256), 0x15C00);
	dmaCopyWordsAsynch(1, pageImage.data()+((174+pageYpos)*256), bgGetGfxPtr(bg3Sub), 0x18000);
	while (dmaBusy(0) || dmaBusy(1));
}

void topBarLoad(void) {
	std::vector<unsigned char> image;
	unsigned width, height;
	lodepng::decode(image, width, height, "nitro:/graphics/topbar.png");
	for(unsigned i=0;i<image.size()/4;i++) {
		bgGetGfxPtr(bg3Main)[i] = image[i*4]>>3 | (image[(i*4)+1]>>3)<<5 | (image[(i*4)+2]>>3)<<10 | BIT(15);
	}
}

void graphicsInit() {
	*(u16*)(0x0400006C) |= BIT(14);
	*(u16*)(0x0400006C) &= BIT(15);
	SetBrightness(0, 31);
	SetBrightness(1, 31);

	////////////////////////////////////////////////////////////
	videoSetMode(MODE_5_2D | DISPLAY_BG3_ACTIVE);
	videoSetModeSub(MODE_5_2D | DISPLAY_BG3_ACTIVE);

	vramSetBankA(VRAM_A_MAIN_BG);
	vramSetBankC(VRAM_C_SUB_BG);

	bg3Main = bgInit(3, BgType_Bmp16, BgSize_B16_256x256, 0, 0);
	bgSetPriority(bg3Sub, 3);

	bg2Main = bgInit(2, BgType_Bmp8, BgSize_B8_256x256, 6, 0);
	bgSetPriority(bg2Main, 0);

	bg3Sub = bgInitSub(3, BgType_Bmp16, BgSize_B16_256x256, 0, 0);
	bgSetPriority(bg3Main, 3);

	// bg2Sub = bgInitSub(2, BgType_Bmp8, BgSize_B8_256x256, 6, 0);
	// bgSetPriority(bg2Sub, 0);

	irqSet(IRQ_VBLANK, vBlankHandler);
	irqEnable(IRQ_VBLANK);
}
