#include "fontHandler.h"

#include <nds/arm9/dldi.h>
#include <list>

#include "common/twlmenusettings.h"
#include "common/flashcard.h"
#include "common/systemdetails.h"
#include "common/tonccpy.h"
#include "myDSiMode.h"
#include "TextEntry.h"
#include "color.h"

extern u16* colorTable;

FontGraphic *smallFont;
FontGraphic *largeFont;

std::list<TextEntry> topText, bottomText;

bool shouldClear[] = {false, false};

void fontInit() {
	// const bool useExpansionPak = (sys().isRegularDS() && ((*(u16*)(0x020000C0) != 0 && *(u16*)(0x020000C0) != 0x5A45) || *(vu16*)(0x08240000) == 1) && (io_dldi_data->ioInterface.features & FEATURE_SLOT_NDS));
	const bool useTileCache = (!dsiFeatures() && !sys().dsDebugRam());

	// Unload fonts if already loaded
	if (smallFont)
		delete smallFont;
	if (largeFont)
		delete largeFont;

	u16 palette[] = {
		0x0000,
		0xDEF7,
		0xC631,
		0xA108,
		0x0000,
		0xEB5A,
		0xDEF7,
		0xD294,
	};

	// Load font graphics
	if (ms().dsClassicCustomFont) {
		std::string fontPath = std::string(sys().isRunFromSD() ? "sd:" : "fat:") + "/_nds/TWiLightMenu/extras/fonts/" + ms().font;
		std::string defaultPath = std::string(sys().isRunFromSD() ? "sd:" : "fat:") + "/_nds/TWiLightMenu/extras/fonts/Default";
		smallFont = new FontGraphic({fontPath + "/small-dsi.nftr", fontPath + "/small.nftr", defaultPath + "/small-dsi.nftr", "nitro:/graphics/font/small.nftr"}, useTileCache);
	} else {
		smallFont = new FontGraphic({"nitro:/graphics/font/small.nftr"}, false);
		palette[3] = 0x94A5;
	}

	if (colorTable) {
		for (int i = 1; i < 8; i++) {
			palette[i] = colorTable[palette[i]];
		}
	}
	// Load palettes
	tonccpy(BG_PALETTE, palette, sizeof(palette));
	tonccpy(BG_PALETTE_SUB, palette, sizeof(palette));
}

static std::list<TextEntry> &getTextQueue(bool top) {
	return top ? topText : bottomText;
}

FontGraphic *getFont(bool large) {
	return large ? largeFont : smallFont;
}

void updateText(bool top) {
	// Remove if adding top support
	if (top)
		return;

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

	// Copy buffer to the screen
	tonccpy(bgGetGfxPtr(top ? 6 : 2), FontGraphic::textBuf[top], 256 * 192);
}

void clearText(bool top) {
	shouldClear[top] = true;
}

void clearText() {
	clearText(true);
	clearText(false);
}

void printTopSmall(int xPos, int yPos, std::string_view str) {
	toncset16(FontGraphic::textBuf[1], 0, 256 * smallFont->height());
	smallFont->print(0, 0, true, str, Alignment::left, FontPalette::regular);
	int width = smallFont->calcWidth(str);
	xPos -= width/2;

	for (int y = 0; y < smallFont->height() && yPos + y < SCREEN_HEIGHT; y++) {
		if (yPos + y < 0) continue;
		for (int x = 0; x < width && xPos + x < SCREEN_WIDTH; x++) {
			if (xPos + x < 0) continue;
			int px = FontGraphic::textBuf[1][y * 256 + x];
			u16 bg = BG_GFX_SUB[(yPos + y) * 256 + (xPos + x)];
			u16 val = px ? alphablend(BG_PALETTE[px], bg, (px % 4) < 2 ? 128 : 224) : bg;

			BG_GFX_SUB[(yPos + y) * 256 + (xPos + x)] = val;
		}
	}
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
