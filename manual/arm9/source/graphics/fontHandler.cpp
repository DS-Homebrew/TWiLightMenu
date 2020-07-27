#include "fontHandler.h"
#include <list>

#include "common/tonccpy.h"
#include "TextEntry.h"

FontGraphic smallFont;
FontGraphic largeFont;

std::list<TextEntry> topText, bottomText;

bool shouldClear[] = {false, false};


void fontInit() {
	printf("fontInit()\n");

	// Load font graphics
	smallFont = FontGraphic("nitro:/graphics/font/small.nftr", "");
	largeFont = FontGraphic("nitro:/graphics/font/large.nftr", "");

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

FontGraphic &getFont(bool large) {
	return large ? largeFont : smallFont;
}

void updateText(bool top) {
	// Clear before redrawing
	if(shouldClear[top]) {
		dmaFillWords(0, FontGraphic::textBuf[top], 256 * 192);
		shouldClear[top] = false;
	}

	// Draw text
	auto &text = getTextQueue(top);
	for(auto it = text.begin(); it != text.end(); ++it) {
		getFont(it->large).print(it->x, it->y, top, it->message, it->align);
	}
	text.clear();

	// Copy buffer to the screen
	tonccpy(bgGetGfxPtr(top ? 2 : 6), FontGraphic::textBuf[top], 256 * 64);
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
	return smallFont.calcWidth(text);
}
int calcSmallFontWidth(std::u16string_view text) {
	return smallFont.calcWidth(text);
}

int calcLargeFontWidth(std::string_view text) {
	return largeFont.calcWidth(text);
}
int calcLargeFontWidth(std::u16string_view text) {
	return largeFont.calcWidth(text);
}

int calcSmallFontHeight(std::string_view text) { return calcSmallFontHeight(FontGraphic::utf8to16(text)); }
int calcSmallFontHeight(std::u16string_view text) {
	int lines = 1;
	for(auto c : text) {
		if(c == '\n')
			lines++;
	}
	return lines * smallFont.height();
}

int calcLargeFontHeight(std::string_view text) { return calcLargeFontHeight(FontGraphic::utf8to16(text)); }
int calcLargeFontHeight(std::u16string_view text) {
	int lines = 1;
	for(auto c : text) {
		if(c == '\n')
			lines++;
	}
	return lines * largeFont.height();
}

u8 smallFontHeight(void) {
	return smallFont.height();
}

u8 largeFontHeight(void) {
	return largeFont.height();
}
