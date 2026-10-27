#include "fontHandler.h"

#include <nds/arm9/dldi.h>
#include <algorithm>
#include <list>

#include "common/twlmenusettings.h"
#include "common/flashcard.h"
#include "common/inifile.h"
#include "common/systemdetails.h"
#include "common/tonccpy.h"
#include "myDSiMode.h"
#include "TextEntry.h"

extern bool startMenu;
extern u16 startBorderColor;

extern u16* colorTable;
extern u16 topImage[2][256*192];
extern u16 topImageWithText[2][256*192];

FontGraphic *smallFont;
FontGraphic *largeFont;
FontGraphic *esrbDescFont;

static bool esrbDescFontIsDs = false;

std::list<TextEntry> topText, bottomText;

bool shouldClear[] = {false, false};

void fontInit() {
	// Unload fonts if already loaded
	if (smallFont)
		delete smallFont;
	if (largeFont)
		delete largeFont;

	extern std::string iniPath;
	extern std::string customIniPath;
	CIniFile ini( iniPath.c_str() );
	CIniFile customIni( customIniPath.c_str() );

	u16 palette[] = {
		0x0000,
		0x0000,
		0x0000,
		0x0000,
		0x0000,
		0x0000,
		0x0000,
		0xFFFF,
		0x0000,
		0x0000,
		0x0000,
		startBorderColor,
		0x0000,
		0x0000,
		0x0000,
		ini.GetInt("global settings", "formTextColor", RGB15(17,12,0)),
		0x0000,
		0x0000,
		0x0000,
		ini.GetInt("global settings", "formTitleTextColor", RGB15(11,11,11)),
		0x0000,
		0x0000,
		0x0000,
		ini.GetInt("global settings", "buttonTextColor", RGB15(17,12,0)),
		0x0000,
		0x0000,
		0x0000,
		ini.GetInt("global settings", "spinBoxTextColor", RGB15(31,31,31)),
		0x0000,
		0x0000,
		0x0000,
		ini.GetInt("global settings", "spinBoxTextHiLightColor", RGB15(31,31,31)),
		0x0000,
		0x0000,
		0x0000,
		ini.GetInt("global settings", "listTextColor", 0),
		0x0000,
		0x0000,
		0x0000,
		ini.GetInt("global settings", "listTextHighLightColor", 0),
		0x0000,
		0x0000,
		0x0000,
		ini.GetInt("global settings", "popMenuTextColor", RGB15(0,0,0)),
		0x0000,
		0x0000,
		0x0000,
		ini.GetInt("global settings", "popMenuTextHighLightColor", RGB15(31,31,31)),
		0x0000,
		0x0000,
		0x0000,
		ini.GetInt("start button", "textColor", 0x7fff),
		0x0000,
		0x0000,
		0x0000,
		ini.GetInt("main list", "textColor", RGB15(7, 7, 7)),
		0x0000,
		0x0000,
		0x0000,
		ini.GetInt("main list", "textColorHilight", RGB15(31, 0, 31)),
		0x0000,
		0x0000,
		0x0000,
		ini.GetInt("folder text", "color", 0),
		0x0000,
		0x0000,
		0x0000,
		customIni.GetInt("user name", "color", 0),
		0x0000,
		0x0000,
		0x0000,
		customIni.GetInt("custom text", "color", 0),
		0x0000,
		0x0000,
		0x0000,
		ini.GetInt("free space", "color", 0)
	};

	// Load font graphics
	smallFont = new FontGraphic({"nitro:/graphics/font/ds.nftr"});

	if (colorTable) {
		palette[3] = colorTable[palette[3] % 0x8000];
		palette[7] = colorTable[palette[7] % 0x8000];
		palette[15] = colorTable[palette[15] % 0x8000]; // formTextColor
		palette[19] = colorTable[palette[19] % 0x8000]; // formTitleTextColor
		palette[23] = colorTable[palette[23] % 0x8000]; // buttonTextColor
		palette[27] = colorTable[palette[27] % 0x8000]; // spinBoxTextColor
		palette[31] = colorTable[palette[31] % 0x8000]; // spinBoxTextHiLightColor
		palette[35] = colorTable[palette[35] % 0x8000]; // listTextColor
		palette[39] = colorTable[palette[39] % 0x8000]; // listTextHighLightColor
		palette[43] = colorTable[palette[43] % 0x8000]; // popMenuTextColor
		palette[47] = colorTable[palette[47] % 0x8000]; // popMenuTextHighLightColor
		palette[51] = colorTable[palette[51] % 0x8000]; // start button: textColor
		palette[55] = colorTable[palette[55] % 0x8000]; // main list: textColor
		palette[59] = colorTable[palette[59] % 0x8000]; // main list: textColorHilight
		palette[63] = colorTable[palette[63] % 0x8000]; // folder text: color
		palette[67] = colorTable[palette[67] % 0x8000]; // user name: color
		palette[71] = colorTable[palette[71] % 0x8000]; // custom text: color
		palette[75] = colorTable[palette[75] % 0x8000]; // free space: color
	}
	// Load palettes
	tonccpy(BG_PALETTE, palette, sizeof(palette));
	tonccpy(BG_PALETTE_SUB, palette, sizeof(palette));
}

void esrbDescFontInit(bool dsFont) {
	if (dsFont) {
		esrbDescFont = smallFont;
		esrbDescFontIsDs = true;
		return;
	}
	esrbDescFont = new FontGraphic({"nitro:/graphics/font/small.nftr"});
}

void esrbDescFontDeinit() {
	if (!esrbDescFont) return;

	if (esrbDescFontIsDs) {
		esrbDescFont = NULL;
		esrbDescFontIsDs = false;
		return;
	}
	delete esrbDescFont;
}

static std::list<TextEntry> &getTextQueue(bool top) {
	return top ? topText : bottomText;
}

FontGraphic *getFont(bool large) {
	return large ? largeFont : smallFont;
}

void updateText(bool top) {
	// Clear before redrawing
	if (shouldClear[top]) {
		dmaFillWords(0, FontGraphic::textBuf[top], 256 * 192);
		shouldClear[top] = false;
	}

	// Draw text
	auto &text = getTextQueue(top);
	for (auto it = text.begin(); it != text.end(); ++it) {
		FontGraphic *font = getFont(it->large);
		if (font)
			font->print(it->x, it->y, top, it->message, it->align, it->palette);
	}
	text.clear();

	if (top) {
		// Copy buffer to the top screen
		for (int i = 0; i < 256*192; i++) {
			topImageWithText[0][i] = (FontGraphic::textBuf[1][i]) ? (BG_PALETTE_SUB[FontGraphic::textBuf[1][i]] | BIT(15)) : topImage[0][i];
			topImageWithText[1][i] = (FontGraphic::textBuf[1][i]) ? (BG_PALETTE_SUB[FontGraphic::textBuf[1][i]] | BIT(15)) : topImage[1][i];
		}
		return;
	}

	// Copy buffer to the bottom screen
	tonccpy(bgGetGfxPtr(2), FontGraphic::textBuf[0], 256 * 192);
}

void updateTextImg(u16* img, bool top) {
	if (top)	return;

	// Clear before redrawing
	if (shouldClear[top]) {
		dmaFillWords(0, FontGraphic::textBuf[top], 256 * 192);
		shouldClear[top] = false;
	}

	// Draw text
	auto &text = getTextQueue(top);
	for (auto it = text.begin(); it != text.end(); ++it) {
		if (esrbDescFont)
			esrbDescFont->print(it->x, it->y, top, it->message, it->align, it->palette);
	}
	text.clear();

	u16 palette[] = {
		0x0000,
		0x6718,
		0x4A32,
		0x1064,
	};

	// Copy buffer to the image
	for (int i = 0; i < 256 * 192; i++) {
		if (FontGraphic::textBuf[top][i] != 0) {
			//img[i] = top ? BG_PALETTE[FontGraphic::textBuf[true][i]] : BG_PALETTE_SUB[FontGraphic::textBuf[false][i]];
			img[i] = palette[FontGraphic::textBuf[top][i]];
		}
	}
}

// Redraws one box of the top screen's text, so the clock, date and box art around it are kept
void printTopBox(int x, int y, int w, int h, std::string_view left, std::string_view center, FontPalette palette) {
	// Glyphs can reach past the box, so keep the text buffer's rows as they were outside it
	const int rows = std::min(std::max(h, 1 + smallFont->height()), 192 - y);
	std::vector<u8> saved(FontGraphic::textBuf[1] + (y*256), FontGraphic::textBuf[1] + ((y+rows)*256));

	for (int y2 = y; y2 < y+h; y2++) {
		toncset(FontGraphic::textBuf[1] + (y2*256) + x, 0, w);
	}

	smallFont->print(x+2, y+1, true, left, Alignment::left, palette);
	smallFont->print(x+(w/2)-128, y+1, true, center, Alignment::center, palette);

	for (int y2 = y; y2 < y+h; y2++) {
		for (int x2 = x; x2 < x+w; x2++) {
			const int i = (y2*256)+x2;
			topImageWithText[0][i] = (FontGraphic::textBuf[1][i]) ? (BG_PALETTE_SUB[FontGraphic::textBuf[1][i]] | BIT(15)) : topImage[0][i];
			topImageWithText[1][i] = (FontGraphic::textBuf[1][i]) ? (BG_PALETTE_SUB[FontGraphic::textBuf[1][i]] | BIT(15)) : topImage[1][i];
		}
	}

	for (int y2 = y; y2 < y+rows; y2++) {
		for (int x2 = 0; x2 < 256; x2++) {
			if (y2 < y+h && x2 >= x && x2 < x+w) continue;
			FontGraphic::textBuf[1][(y2*256)+x2] = saved[((y2-y)*256)+x2];
		}
	}
}

void clearText(bool top) {
	shouldClear[top] = true;
}

void clearText() {
	clearText(true);
	clearText(false);
}

void printSmall(bool top, int x, int y, std::string_view message, Alignment align, FontPalette palette) {
	getTextQueue(top).emplace_back(false, x, y, message, align, palette);
}
void printSmall(bool top, int x, int y, std::u16string_view message, Alignment align, FontPalette palette) {
	getTextQueue(top).emplace_back(false, x, y, message, align, palette);
}

void printLarge(bool top, int x, int y, std::string_view message, Alignment align, FontPalette palette) {
	getTextQueue(top).emplace_back(true, x, y, message, align, palette);
}
void printLarge(bool top, int x, int y, std::u16string_view message, Alignment align, FontPalette palette) {
	getTextQueue(top).emplace_back(true, x, y, message, align, palette);
}

int calcSmallFontWidth(std::string_view text) {
	if (smallFont)
		return smallFont->calcWidth(text);
	return 0;
}
int calcSmallFontWidth(std::u16string_view text) {
	if (smallFont)
		return smallFont->calcWidth(text);
	return 0;
}

int calcLargeFontWidth(std::string_view text) {
	if (largeFont)
		return largeFont->calcWidth(text);
	return 0;
}
int calcLargeFontWidth(std::u16string_view text) {
	if (largeFont)
		return largeFont->calcWidth(text);
	return 0;
}

int calcSmallFontHeight(std::string_view text) { return calcSmallFontHeight(FontGraphic::utf8to16(text)); }
int calcSmallFontHeight(std::u16string_view text) {
	if (smallFont) {
		int lines = 1;
		for (auto c : text) {
			if (c == '\n')
				lines++;
		}
		return lines * smallFont->height();
	}

	return 0;
}

int calcLargeFontHeight(std::string_view text) { return calcLargeFontHeight(FontGraphic::utf8to16(text)); }
int calcLargeFontHeight(std::u16string_view text) {
	if (largeFont) {
		int lines = 1;
		for (auto c : text) {
			if (c == '\n')
				lines++;
		}
		return lines * largeFont->height();
	}

	return 0;
}

u8 smallFontHeight(void) {
	if (smallFont)
		return smallFont->height();
	return 0;
}

u8 largeFontHeight(void) {
	if (largeFont)
		return largeFont->height();
	return 0;
}
