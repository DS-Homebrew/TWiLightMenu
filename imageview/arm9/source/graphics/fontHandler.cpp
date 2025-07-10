#include "fontHandler.h"

#include <nds/arm9/dldi.h>
#include <list>

#include "common/twlmenusettings.h"
#include "common/flashcard.h"
#include "common/systemdetails.h"
#include "common/tonccpy.h"
#include "myDSiMode.h"
#include "TextEntry.h"

extern u16* colorTable;

FontGraphic *smallFont;
FontGraphic *largeFont;

std::list<TextEntry> bottomText;

bool shouldClear = false;

// Checks if any of the specified files exists
bool fileExists(std::vector<std::string_view> paths) {
	for (const std::string_view &path : paths) {
		if (access(path.data(), F_OK) == 0)
			return true;
	}

	return false;
}

void fontInit() {
	// const bool useExpansionPak = (sys().isRegularDS() && ((*(u16*)(0x020000C0) != 0 && *(u16*)(0x020000C0) != 0x5A45) || *(vu16*)(0x08240000) == 1) && (io_dldi_data->ioInterface.features & FEATURE_SLOT_NDS));
	const bool useTileCache = (!dsiFeatures() && !sys().dsDebugRam());

	// Unload fonts if already loaded
	if (smallFont)
		delete smallFont;
	if (largeFont)
		delete largeFont;

	// Load font graphics
	std::string fontPath = std::string(sys().isRunFromSD() ? "sd:" : "fat:") + "/_nds/TWiLightMenu/extras/fonts/" + ms().font;
	std::string defaultPath = std::string(sys().isRunFromSD() ? "sd:" : "fat:") + "/_nds/TWiLightMenu/extras/fonts/Default";
	smallFont = new FontGraphic({fontPath + "/small-dsi.nftr", fontPath + "/small.nftr", defaultPath + "/small-dsi.nftr", "nitro:/graphics/font/small.nftr"}, useTileCache);
	// If custom small font but no custom large font, use small font as large font
	if (fileExists({fontPath + "/small-dsi.nftr", fontPath + "/small.nftr"}) && !fileExists({fontPath + "/large-dsi.nftr", fontPath + "/large.nftr"}))
		largeFont = smallFont;
	else
		largeFont = new FontGraphic({fontPath + "/large-dsi.nftr", fontPath + "/large.nftr", defaultPath + "/large-dsi.nftr", "nitro:/graphics/font/large.nftr"}, useTileCache);

	// Load palettes
	u16 palette[] = {
		0x0000,
		0x45F1,
		0x2D0A,
		0x1044,
	};
	if (colorTable) {
		for (int i = 1; i < 4; i++) {
			palette[i] = colorTable[palette[i] % 0x8000];
		}
	}
	//tonccpy(BG_PALETTE + 0xF8, palette, sizeof(palette));
	tonccpy(BG_PALETTE_SUB + 0xF8, palette, sizeof(palette));
}

static std::list<TextEntry> &getTextQueue(bool top) {
	return bottomText;
}

FontGraphic *getFont(bool large) {
	return large ? largeFont : smallFont;
}

void updateText(bool top) {
	// Clear before redrawing
	if (shouldClear) {
		dmaFillWords(0, FontGraphic::textBuf, 256 * 192);
		shouldClear = false;
	}

	// Draw text
	auto &text = getTextQueue(top);
	for (auto it = text.begin(); it != text.end(); ++it) {
		FontGraphic *font = getFont(it->large);
		if (font)
			font->print(it->x, it->y, top, it->message, it->align);
	}
	text.clear();

	// Copy buffer to the screen
	extern int bg2Sub;
	tonccpy(bgGetGfxPtr(bg2Sub), FontGraphic::textBuf, 256 * 192);
}

void clearText(bool top) {
	shouldClear = true;
}

void clearText() {
	//clearText(true);
	clearText(false);
}

void printSmall(bool top, int x, int y, std::string_view message, Alignment align) {
	getTextQueue(top).emplace_back(false, x, y, message, align);
}
void printSmall(bool top, int x, int y, std::u16string_view message, Alignment align) {
	getTextQueue(top).emplace_back(false, x, y, message, align);
}

void printLarge(bool top, int x, int y, std::string_view message, Alignment align) {
	getTextQueue(top).emplace_back(true, x, y, message, align);
}
void printLarge(bool top, int x, int y, std::u16string_view message, Alignment align) {
	getTextQueue(top).emplace_back(true, x, y, message, align);
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
