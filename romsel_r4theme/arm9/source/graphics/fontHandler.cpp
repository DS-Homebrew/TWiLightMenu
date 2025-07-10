#include "fontHandler.h"

#include <nds/arm9/dldi.h>
#include <list>

#include "common/twlmenusettings.h"
#include "common/flashcard.h"
#include "common/systemdetails.h"
#include "common/tonccpy.h"
#include "myDSiMode.h"
#include "TextEntry.h"

extern bool startMenu;
extern u16 startBorderColor;

extern u16* colorTable;
extern u16 topImage[2][2][256*192];
extern u16 topImageWithText[2][2][256*192];

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
		startBorderColor
	};

	// Load font graphics
	smallFont = new FontGraphic({"nitro:/graphics/font/ds.nftr"});

	if (colorTable) {
		palette[3] = colorTable[palette[3] % 0x8000];
		palette[7] = colorTable[palette[7] % 0x8000];
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
		if (ms().theme == TWLSettings::EThemeGBC) {
			// Avoid writing to GBC border
			for (int y = 24; y < 192-24; y++) {
				for (int x = 48; x < 256-48; x++) {
					const int i = (y*256)+x;
					topImageWithText[startMenu][0][i] = (FontGraphic::textBuf[1][i]) ? (BG_PALETTE_SUB[FontGraphic::textBuf[1][i]] | BIT(15)) : topImage[startMenu][0][i];
					topImageWithText[startMenu][1][i] = (FontGraphic::textBuf[1][i]) ? (BG_PALETTE_SUB[FontGraphic::textBuf[1][i]] | BIT(15)) : topImage[startMenu][1][i];
				}
			}
		} else {
			for (int i = 0; i < 256*192; i++) {
				topImageWithText[startMenu][0][i] = (FontGraphic::textBuf[1][i]) ? (BG_PALETTE_SUB[FontGraphic::textBuf[1][i]] | BIT(15)) : topImage[startMenu][0][i];
				topImageWithText[startMenu][1][i] = (FontGraphic::textBuf[1][i]) ? (BG_PALETTE_SUB[FontGraphic::textBuf[1][i]] | BIT(15)) : topImage[startMenu][1][i];
			}
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
