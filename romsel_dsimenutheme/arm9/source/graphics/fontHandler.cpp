#include "fontHandler.h"
#include <list>

#include "common/tonccpy.h"
#include "TextEntry.h"

FontGraphic smallFont;
FontGraphic largeFont;

std::list<TextEntry> topText, bottomText;


void fontInit() {
	printf("fontInit()\n");

	smallFont = FontGraphic("nitro:/graphics/font/small.nftr");

	//Do the same with our bigger texture
	largeFont = FontGraphic("nitro:/graphics/font/large.nftr");
}

static std::list<TextEntry> &getTextQueue(bool top) {
	return top ? topText : bottomText;
}

FontGraphic &getFont(bool large) {
	return large ? largeFont : smallFont;
}

void updateText(bool top) {
	// Clear before redrawing
	dmaFillWords(0, bgGetGfxPtr(2), 256 * 192);

	// Draw text
	auto &text = getTextQueue(top);
	for(auto it = text.begin(); it != text.end(); ++it) {
		getFont(it->large).print(it->x, it->y, it->message, it->align);
	}
}

void clearText(bool top) {
	std::list<TextEntry> &text = getTextQueue(top);
	for (auto it = text.begin(); it != text.end(); ++it) {
		// if (it->immune)
		// 	continue;
		it = text.erase(it);
		--it;
	}
}

void clearText() {
	clearText(true);
	clearText(false);
}

void printSmall(bool top, int x, int y, const std::string &message, Alignment align) {
	getTextQueue(top).emplace_back(false, x, y, message.c_str(), align);
}

void printLarge(bool top, int x, int y, const std::string &message, Alignment align) {
	getTextQueue(top).emplace_back(true, x, y, message.c_str(), align);
}

int calcSmallFontWidth(const char *text) {
	return smallFont.calcWidth(text);
}

int calcLargeFontWidth(const char *text) {
	return largeFont.calcWidth(text);
}
