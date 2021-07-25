#include "fontHandler.h"

#include <nds/arm9/dldi.h>
#include <list>

// #include "common/systemdetails.h"
#include "common/tonccpy.h"
#include "flashcard.h"
#include "myDSiMode.h"
#include "TextEntry.h"

FontGraphic *smallFont;
FontGraphic *largeFont;

std::list<TextEntry> topText, bottomText;

bool shouldClear[] = {false, false};

extern bool isRegularDS;
// extern std::string font;

void fontInit() {
	bool useExpansionPak = (isRegularDS && ((*(u16*)(0x020000C0) != 0 && *(u16*)(0x020000C0) != 0x5A45) || *(vu16*)(0x08240000) == 1) && (io_dldi_data->ioInterface.features & FEATURE_SLOT_NDS));

	// Unload fonts if already loaded
	if(smallFont)
		delete smallFont;
	if(largeFont)
		delete largeFont;

	// Load font graphics
	smallFont = new FontGraphic({"nitro:/graphics/font/small.nftr"}, useExpansionPak);

	// Load palettes
	u16 palette[] = {
		0x0000,
		0xDEF7,
		0xC631,
		0xA108,
	};
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
	if(top)
		return;

	// Clear before redrawing
	if(shouldClear[top]) {
		dmaFillWords(0, FontGraphic::textBuf[top], 256 * 192);
		shouldClear[top] = false;
	}

	// Draw text
	auto &text = getTextQueue(top);
	for(auto it = text.begin(); it != text.end(); ++it) {
		FontGraphic *font = getFont(it->large);
		if(font)
			font->print(it->x, it->y, top, it->message, it->align);
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
	if(smallFont)
		return smallFont->calcWidth(text);
	return 0;
}
int calcSmallFontWidth(std::u16string_view text) {
	if(smallFont)
		return smallFont->calcWidth(text);
	return 0;
}

int calcLargeFontWidth(std::string_view text) {
	if(largeFont)
		return largeFont->calcWidth(text);
	return 0;
}
int calcLargeFontWidth(std::u16string_view text) {
	if(largeFont)
		return largeFont->calcWidth(text);
	return 0;
}

int calcSmallFontHeight(std::string_view text) { return calcSmallFontHeight(FontGraphic::utf8to16(text)); }
int calcSmallFontHeight(std::u16string_view text) {
	if(smallFont) {
		int lines = 1;
		for(auto c : text) {
			if(c == '\n')
				lines++;
		}
		return lines * smallFont->height();
	}

	return 0;
}

int calcLargeFontHeight(std::string_view text) { return calcLargeFontHeight(FontGraphic::utf8to16(text)); }
int calcLargeFontHeight(std::u16string_view text) {
	if(largeFont) {
		int lines = 1;
		for(auto c : text) {
			if(c == '\n')
				lines++;
		}
		return lines * largeFont->height();
	}

	return 0;
}

u8 smallFontHeight(void) {
	if(smallFont)
		return smallFont->height();
	return 0;
}

u8 largeFontHeight(void) {
	if(largeFont)
		return largeFont->height();
	return 0;
}
