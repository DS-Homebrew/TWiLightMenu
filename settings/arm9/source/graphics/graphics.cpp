#include "graphics.h"
#include <nds.h>

#include "common/fileCopy.h"
#include "common/twlmenusettings.h"
#include "common/systemdetails.h"
#include "common/tonccpy.h"
#include "fontHandler.h"

#include "top_bg.h"
//#include "top_bg_wide.h"
#include "sub_bg.h"
#include "saturn_bg.h"

extern std::string colorLutName;

extern bool fadeType;
//extern bool widescreenEffects;
int screenBrightness = 31;
bool lcdSwapped = false;
extern int currentTheme;
extern bool currentMacroMode;

int frameDelay = 0;
bool frameDelayEven = true; // For 24FPS
u16* colorTable = NULL;

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

	return 32768|(max<<10)|(max<<5)|(max);
} */

// Copys a palette and applies filtering if enabled
void copyPalette(u16 *dst, const u16 *src, int size) {
	if (colorTable) {
		for (int i = 0; i < size; i++) {
			dst[i] = colorTable[src[i] % 0x8000];
		}
	} else {
		tonccpy(dst, src, size);
	}
}

void clearScroller(void) {
	// Reset layer
	dmaCopyWords(0, currentTheme == 4 ? saturn_bgBitmap : sub_bgBitmap , bgGetGfxPtr(7), sub_bgBitmapLen);
}

// TODO: If adding sprites, this would work better as one
void drawScroller(int y, int h, bool onLeft) {
	clearScroller();

	const u8 scroller[4] = {2, 3, 3, 2};
	u8 *dst = (u8*)bgGetGfxPtr(7) + (onLeft ? 2 : 250);
	toncset16(dst + y * 256, 2 | 2 << 8, 2);
	for (int i = 1; i < h - 1; i++) {
		tonccpy(dst + (y + i) * 256, scroller, sizeof(scroller));
	}
	toncset16(dst + (y + h - 1) * 256, 2 | 2 << 8, 2);
}

void vBlankHandler()
{
	if (fadeType) {
		screenBrightness--;
		if (screenBrightness < 0) screenBrightness = 0;
	} else {
		screenBrightness++;
		if (screenBrightness > 31) screenBrightness = 31;
	}
	if (currentMacroMode) {
		SetBrightness(0, lcdSwapped ? (currentTheme == 4 ? -screenBrightness : screenBrightness) : (currentTheme == 4 ? -31 : 31));
		SetBrightness(1, !lcdSwapped ? (currentTheme == 4 ? -screenBrightness : screenBrightness) : (currentTheme == 4 ? -31 : 31));
	} else {
		SetBrightness(0, currentTheme == 4 ? -screenBrightness : screenBrightness);
		SetBrightness(1, currentTheme == 4 ? -screenBrightness : screenBrightness);
	}
}

void graphicsInit() {
	currentTheme = ms().theme;

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

			colorLutName = lutName;
		}
	} else {
		colorLutName = "Default";
	}

	SetBrightness(0, currentTheme == 4 ? -31 : 31);
	SetBrightness(1, currentTheme == 4 && !ms().macroMode ? -31 : 31);
	if (ms().macroMode) {
		powerOff(PM_BACKLIGHT_TOP);
	}

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

	if (currentTheme != 4) {
		tonccpy(bgGetGfxPtr(bg3Main), (/*widescreenEffects ? top_bg_wideBitmap :*/ top_bgBitmap), (/*widescreenEffects ? top_bg_wideBitmapLen :*/ top_bgBitmapLen));
		copyPalette(BG_PALETTE + 0x10, (/*widescreenEffects ? top_bg_widePal :*/ top_bgPal), (/*widescreenEffects ? top_bg_widePalLen :*/ top_bgPalLen));
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
}