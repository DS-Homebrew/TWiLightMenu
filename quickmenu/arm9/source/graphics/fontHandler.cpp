#include "fontHandler.h"

#include <nds/arm9/dldi.h>
#include <list>

#include "common/twlmenusettings.h"
#include "common/flashcard.h"
#include "common/logging.h"
#include "common/systemdetails.h"
#include "common/tonccpy.h"
#include "myDSiMode.h"
#include "TextEntry.h"
#include "color.h"
#include "usercolors.h"

extern u16* colorTable;

FontGraphic *smallFont;
FontGraphic *tinyFont;
FontGraphic *esrbDescFont;

std::list<TextEntry> topText, bottomText;

bool shouldClear[] = {false, false};

void fontInit() {
	logPrint("fontInit() ");

	// const bool useExpansionPak = (sys().isRegularDS() && ((*(u16*)(0x020000C0) != 0 && *(u16*)(0x020000C0) != 0x5A45) || *(vu16*)(0x08240000) == 1) && (io_dldi_data->ioInterface.features & FEATURE_SLOT_NDS));
	const bool useTileCache = (!dsiFeatures() && !sys().dsDebugRam());

	// Unload fonts if already loaded
	if (smallFont)
		delete smallFont;
	if (tinyFont)
		delete tinyFont;

	u16 palette[] = {
		0x0000, // Regular (dark gray)
		0xDEF7,
		0xC631,
		0xA108,
		0x0000, // Disabled (light gray)
		0xEB5A,
		0xDEF7,
		0xD294,
		0x0000, // Top Bar (lightened user color)
		0x0000,
		0x0000,
		userColorsTopBar[getFavoriteColor()],
		0x0000, // Sunday (red)
		0x0000,
		0x0000,
		0x800F,
		0x0000, // Saturday (blue)
		0x0000,
		0x0000,
		0xC000,
	};

	// Load font graphics
	if (ms().dsClassicCustomFont) {
		std::string fontPath = std::string(sys().isRunFromSD() ? "sd:" : "fat:") + "/_nds/TWiLightMenu/extras/fonts/" + ms().font;
		std::string defaultPath = std::string(sys().isRunFromSD() ? "sd:" : "fat:") + "/_nds/TWiLightMenu/extras/fonts/Default";
		smallFont = new FontGraphic({fontPath + "/small-dsi.nftr", fontPath + "/small.nftr", defaultPath + "/small-dsi.nftr", "nitro:/graphics/font/ds.nftr"}, useTileCache);
	} else {
		smallFont = new FontGraphic({"nitro:/graphics/font/ds.nftr"}, false);
		palette[3] = 0x94A5;
	}
	tinyFont = new FontGraphic({"nitro:/graphics/font/tiny.nftr"}, false);
	tinyFont->setFixedWidthChar('A'); // tiny font has tiny fixed-width

	if (colorTable) {
		for (uint i = 1; i < sizeof(palette)/sizeof(u16); i++) {
			palette[i] = colorTable[palette[i] % 0x8000] | BIT(15);
		}
	}
	// Load palettes
	tonccpy(BG_PALETTE, palette, sizeof(palette));
	tonccpy(BG_PALETTE_SUB, palette, sizeof(palette));
	logPrint("Font inited\n");
}

void esrbDescFontInit(bool dsFont) {
	esrbDescFont = new FontGraphic({dsFont ? "nitro:/graphics/font/ds.nftr" : "nitro:/graphics/font/small.nftr"}, false);
}

void esrbDescFontDeinit() {
	if (esrbDescFont)
		delete esrbDescFont;
}

static std::list<TextEntry> &getTextQueue(bool top) {
	return top ? topText : bottomText;
}

FontGraphic *getFont(bool tiny) {
	return tiny ? tinyFont : smallFont;
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
			font->print(it->x, it->y, top, it->message, it->align, it->palette, false, it->monospaced);
	}
	text.clear();

	// Copy buffer to the screen (top screen must be copied manually to background)
	if (!top) tonccpy(bgGetGfxPtr(top ? 6 : 2), FontGraphic::textBuf[top], 256 * 192);
}

void updateTopTextArea(int x, int y, int width, int height, u16 *restoreBuf) {
	// Clear only the affected rows
	dmaFillWords(0, FontGraphic::textBuf[1] + y * 256, height * 256);
	shouldClear[1] = false;
	updateText(true);
	// Manual copy to background layer only within box bounds
	for (int yy = y; yy < y + height; yy++) {
		for (int xx = x; xx < x + width; xx++) {
			int idx = yy * 256 + xx;
			int px = FontGraphic::textBuf[1][idx];
			if (px || restoreBuf) BG_GFX_SUB[idx] = px ? BG_PALETTE_SUB[px] : restoreBuf[idx];
		}
	}
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
	getTextQueue(top).emplace_back(false, false, x, y, message, align, palette);
}
void printSmall(bool top, int x, int y, std::u16string_view message, Alignment align, FontPalette palette) {
	getTextQueue(top).emplace_back(false, false, x, y, message, align, palette);
}

void printSmallMonospaced(bool top, int x, int y, std::string_view message, Alignment align, FontPalette palette) {
	getTextQueue(top).emplace_back(false, true, x, y, message, align, palette);
}
void printSmallMonospaced(bool top, int x, int y, std::u16string_view message, Alignment align, FontPalette palette) {
	getTextQueue(top).emplace_back(false, true, x, y, message, align, palette);
}

void printTiny(bool top, int x, int y, std::string_view message, Alignment align, FontPalette palette) {
	getTextQueue(top).emplace_back(true, false, x, y, message, align, palette);
}
void printTiny(bool top, int x, int y, std::u16string_view message, Alignment align, FontPalette palette) {
	getTextQueue(top).emplace_back(true, false, x, y, message, align, palette);
}

void printTinyMonospaced(bool top, int x, int y, std::string_view message, Alignment align, FontPalette palette) {
	getTextQueue(top).emplace_back(true, true, x, y, message, align, palette);
}
void printTinyMonospaced(bool top, int x, int y, std::u16string_view message, Alignment align, FontPalette palette) {
	getTextQueue(top).emplace_back(true, true, x, y, message, align, palette);
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

int calcTinyFontWidth(std::string_view text) {
	if (tinyFont)
		return tinyFont->calcWidth(text);
	return 0;
}
int calcTinyFontWidth(std::u16string_view text) {
	if (tinyFont)
		return tinyFont->calcWidth(text);
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

int calcTinyFontHeight(std::string_view text) { return calcTinyFontHeight(FontGraphic::utf8to16(text)); }
int calcTinyFontHeight(std::u16string_view text) {
	if (tinyFont) {
		int lines = 1;
		for (auto c : text) {
			if (c == '\n')
				lines++;
		}
		return lines * tinyFont->height();
	}

	return 0;
}

u8 smallFontHeight(void) {
	if (smallFont)
		return smallFont->height();
	return 0;
}

u8 tinyFontHeight(void) {
	if (tinyFont)
		return tinyFont->height();
	return 0;
}
