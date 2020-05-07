#include "fontHandler.h"
#include <list>

#include "common/tonccpy.h"
#include "TextEntry.h"
#include "ThemeConfig.h"
#include "themefilenames.h"

FontGraphic smallFont;
FontGraphic largeFont;

std::list<TextEntry> topText, bottomText;

bool shouldClear[] = {false, false};


void fontInit() {
	printf("fontInit()\n");

	// Load font graphics
	smallFont = FontGraphic(TFN_FONT_SMALL, TFN_FALLBACK_FONT_SMALL);
	largeFont = FontGraphic(TFN_FONT_LARGE, TFN_FALLBACK_FONT_LARGE);

	// Load palettes
	u16 palette[] = {
		tc().fontPalette1(),
		tc().fontPalette2(),
		tc().fontPalette3(),
		tc().fontPalette4(),
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
	if(shouldClear[top] && /* remove when adding top drawing */ !top) {
		dmaFillWords(0, bgGetGfxPtr(2), 256 * 192);
		shouldClear[top] = false;
	}

	// Draw text
	auto &text = getTextQueue(top);
	for(auto it = text.begin(); it != text.end(); ++it) {
		getFont(it->large).print(it->x, it->y, it->message, it->align);
	}
	text.clear();
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

void printLarge(bool top, int x, int y, std::string_view message, Alignment align) {
	getTextQueue(top).emplace_back(true, x, y, message, align);
}

uint calcSmallFontWidth(std::string_view text) {
	return smallFont.calcWidth(text);
}

uint calcLargeFontWidth(std::string_view text) {
	return largeFont.calcWidth(text);
}

u8 smallFontHeight(void) {
	return smallFont.height();
}

u8 largeFontHeight(void) {
	return largeFont.height();
}
