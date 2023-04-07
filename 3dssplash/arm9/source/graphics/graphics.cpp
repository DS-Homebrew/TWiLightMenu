#include "graphics.h"
#include "common/tonccpy.h"
#include "common/twlmenusettings.h"
#include "graphics/gif.hpp"

#include <nds.h>

extern bool fadeType;
extern bool controlTopBright;
extern bool controlBottomBright;
int fadeDelay = 0;

int screenBrightness = 31;
bool bottomBgLoaded = false;

int bg3Sub;
int bg3Main;

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
	if (fadeType == true) {
		screenBrightness--;
		if (screenBrightness < 0) screenBrightness = 0;
	} else {
		screenBrightness++;
		if (screenBrightness > 31) screenBrightness = 31;
	}
	if (controlTopBright) SetBrightness(0, bottomBgLoaded ? -screenBrightness : screenBrightness);
	if (controlBottomBright && !ms().macroMode) SetBrightness(1, bottomBgLoaded ? -screenBrightness : screenBrightness);
}

void bgLoad(void) {
	if (ms().macroMode) {
		return;
	}

	Gif gif ("nitro:/graphics/nintendo.gif", false, false);
	gif.displayFrame();
	bottomBgLoaded = true;
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
	vramSetBankC(VRAM_C_SUB_BG);

	bg3Main = bgInit(3, BgType_Bmp8, BgSize_B8_256x256, 0, 0);
	bgSetPriority(bg3Main, 3);

	bg3Sub = bgInitSub(3, BgType_Bmp8, BgSize_B8_256x256, 0, 0);
	bgSetPriority(bg3Sub, 3);

	if (ms().macroMode) {
		lcdMainOnBottom();
	}

	irqSet(IRQ_VBLANK, vBlankHandler);
	irqEnable(IRQ_VBLANK);
}
