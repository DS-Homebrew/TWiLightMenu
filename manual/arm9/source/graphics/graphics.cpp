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
#include "userpal.h"
#include "../errorScreen.h"
#include "fontHandler.h"
#include "fileCopy.h"
#include "common/tonccpy.h"
#include "common/twlmenusettings.h"
#include "common/systemdetails.h"
#include "graphics/gif.hpp"

#include <nds.h>

extern bool fadeType;
extern bool controlTopBright;
extern bool controlBottomBright;

// u8 bgColor1 = 0xF6;
// u8 bgColor2 = 0xF7;

int screenBrightness = 31;
bool updatePalMidFrame = true;
bool leaveTopBarIntact = false;

u16 bmpImageBuffer[256*192] = {0};
u16 topBarPal[10] = {0}; // For both font and top bar palettes
static u16 pagePal[256] = {0};
u16* colorTable = NULL;
std::vector<u8> pageImage;

extern int pageYpos;
extern int pageYsize;

int bg3Sub;
int bg2Main;
int bg3Main;
int bg2Sub;

bool screenFadedIn(void) { return (screenBrightness == 0); }
bool screenFadedOut(void) { return (screenBrightness > 24); }

bool invertedColors = false;
bool noWhiteFade = false;

// Ported from PAlib (obsolete)
void SetBrightness(u8 screen, s8 bright) {
	if ((invertedColors && bright != 0) || (noWhiteFade && bright > 0)) {
		bright -= bright*2; // Invert brightness to match the inverted colors
	}

	u16 mode = 1 << 14;

	if (bright < 0) {
		mode = 2 << 14;
		bright = -bright;
	}
	if (bright > 31) bright = 31;
	*(vu16*)(0x0400006C + (0x1000 * screen)) = bright + mode;
}

/* u16 convertVramColorToGrayscale(u16 val) {
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
} */

void vBlankHandler() {
	if (fadeType) {
		screenBrightness--;
		if (screenBrightness < 0) screenBrightness = 0;
	} else {
		screenBrightness++;
		if (screenBrightness > 31) screenBrightness = 31;
	}

	if (leaveTopBarIntact) {
		if (controlTopBright) SetBrightness(0, 0);
		if (controlBottomBright && !ms().macroMode) SetBrightness(1, ms().macroMode ? 0 : screenBrightness);
		tonccpy(BG_PALETTE + 0xF6, topBarPal, 10 * 2);
		while (REG_VCOUNT != 18);
		if (controlTopBright) SetBrightness(0, screenBrightness);
		tonccpy(BG_PALETTE, pagePal, 256 * 2);
		return;
	}

	if (controlTopBright) SetBrightness(0, screenBrightness);
	if (controlBottomBright && !ms().macroMode) SetBrightness(1, screenBrightness);

	if (updatePalMidFrame) {
		tonccpy(BG_PALETTE + 0xF6, topBarPal, 10 * 2);
		while (REG_VCOUNT != 18);
		tonccpy(BG_PALETTE, pagePal, 256 * 2);
	}
}

void pageLoad(const std::string &filename) {
	Gif gif (filename.c_str(), false, false, true);
	pageImage = gif.frame(0).image.imageData;
	pageYsize = gif.frame(0).descriptor.h;

	while (!screenFadedOut()) { swiWaitForVBlank(); }

	/* tonccpy(BG_PALETTE, gif.gct().data(), std::min(0xF6u, gif.gct().size()) * 2);
	if (colorTable) {
		for (int i = 0; i < (int)std::min(0xF6u, gif.gct().size()); i++) {
			BG_PALETTE[i] = colorTable[BG_PALETTE[i] % 0x8000];
		}
	} */
	tonccpy(pagePal, gif.gct().data(), gif.gct().size() * 2);
	if (colorTable) {
		for (int i = 0; i < (int)gif.gct().size(); i++) {
			pagePal[i] = colorTable[pagePal[i] % 0x8000];
		}
	}
	if (!ms().macroMode) {
		tonccpy(BG_PALETTE_SUB, pagePal, gif.gct().size() * 2);
	}

	dmaCopyWordsAsynch(0, pageImage.data(), bgGetGfxPtr(bg3Main)+(9*256), 174*256);
	if (!ms().macroMode) dmaCopyWordsAsynch(1, pageImage.data()+(174*256), bgGetGfxPtr(bg3Sub), 192*256);

	fadeType = true; // Fade in from white
	while (!screenFadedIn()) { swiWaitForVBlank(); }

	while (dmaBusy(0) || dmaBusy(1));
}

void pageScroll(void) {
	dmaCopyWordsAsynch(0, pageImage.data()+(pageYpos*256), bgGetGfxPtr(bg3Main)+(9*256), 174*256);
	if (!ms().macroMode) dmaCopyWordsAsynch(1, pageImage.data()+((174+pageYpos)*256), bgGetGfxPtr(bg3Sub), 192*256);
	while (dmaBusy(0) || dmaBusy(1));
}

void topBarLoad(void) {
	Gif gif ("nitro:/graphics/topbar.gif", false, false, true);
	const auto &frame = gif.frame(0);
	u16 *dst = bgGetGfxPtr(bg3Main);

	extern bool useTwlCfg;
	int favoriteColor = (int)(useTwlCfg ? *(u8*)0x02000444 : PersonalData->theme);
	if (favoriteColor < 0 || favoriteColor >= 16) favoriteColor = 0; // Invalid color found, so default to gray

	/* tonccpy(BG_PALETTE + 0xFC, gif.gct().data(), gif.gct().size() * 2);
	if (colorTable) {
		for (int i = 0xFC; i < (int)0xFC + gif.gct().size(); i++) {
			BG_PALETTE[i] = colorTable[BG_PALETTE[i] % 0x8000];
		}
	} */
	tonccpy(topBarPal+4, gif.gct().data(), gif.gct().size() * 2);
	topBarPal[4+4] = palUserFont[favoriteColor][1];
	topBarPal[4+5] = palUserFont[favoriteColor][0];
	if (colorTable) {
		for (int i = 0; i < 6; i++) {
			topBarPal[4+i] = colorTable[topBarPal[i+4] % 0x8000];
		}
	}

	for (uint i = 0; i < frame.image.imageData.size(); i += 2) {
		toncset16(dst++, (frame.image.imageData[i] + 0xFA) | (frame.image.imageData[i + 1] + 0xFA) << 8, 1);
	}
	for (int i = 0; i < 256/2; i++) {
		toncset16(dst++, 0xFEFE, 1);
	}
	for (int i = 0; i < 256/2; i++) {
		toncset16(dst++, 0xFFFF, 1);
	}
}

void graphicsInit() {
	char currentSettingPath[40];
	sprintf(currentSettingPath, "%s:/_nds/colorLut/currentSetting.txt", (sys().isRunFromSD() ? "sd" : "fat"));

	if (access(currentSettingPath, F_OK) == 0) {
		// Load color LUT
		char lutName[128] = {0};
		FILE* file = fopen(currentSettingPath, "rb");
		fread(lutName, 1, 128, file);
		fclose(file);

		char colorTablePath[256];
		sprintf(colorTablePath, "%s:/_nds/colorLut/%s.lut", (sys().isRunFromSD() ? "sd" : "fat"), lutName);

		if (getFileSize(colorTablePath) == 0x10000) {
			colorTable = new u16[0x10000/sizeof(u16)];

			FILE* file = fopen(colorTablePath, "rb");
			fread(colorTable, 1, 0x10000, file);
			fclose(file);

			const u16 color0 = colorTable[0] | BIT(15);
			const u16 color7FFF = colorTable[0x7FFF] | BIT(15);

			invertedColors =
			  (color0 >= 0xF000 && color0 <= 0xFFFF
			&& color7FFF >= 0x8000 && color7FFF <= 0x8FFF);
			if (!invertedColors) noWhiteFade = (color7FFF < 0xF000);

			vramSetBankD(VRAM_D_LCD);
			tonccpy(VRAM_D, colorTable, 0x10000); // Copy LUT to VRAM
			delete[] colorTable; // Free up RAM space
			colorTable = VRAM_D;
		}
	}

	*(vu16*)(0x0400006C) |= BIT(14);
	*(vu16*)(0x0400006C) &= BIT(15);
	SetBrightness(0, 31);
	SetBrightness(1, 31);

	////////////////////////////////////////////////////////////
	videoSetMode(MODE_5_2D);
	videoSetModeSub(MODE_5_2D);

	vramSetBankA(VRAM_A_MAIN_BG);
	vramSetBankC(VRAM_C_SUB_BG);

	bg3Main = bgInit(3, BgType_Bmp8, BgSize_B8_256x256, 0, 0);
	bgSetPriority(bg3Main, 3);

	bg2Main = bgInit(2, BgType_Bmp8, BgSize_B8_256x256, 3, 0);
	bgSetPriority(bg2Main, 0);

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
