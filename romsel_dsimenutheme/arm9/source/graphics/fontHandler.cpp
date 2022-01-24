#include "fontHandler.h"

#include <nds/arm9/dldi.h>
#include <list>

#include "common/twlmenusettings.h"
#include "common/flashcard.h"
#include "common/systemdetails.h"
#include "common/tonccpy.h"
#include "myDSiMode.h"
#include "TextEntry.h"
#include "ThemeConfig.h"

FontGraphic *smallFont;
FontGraphic *largeFont;
FontGraphic *esrbDescFont;

std::list<TextEntry> topText, bottomText;

bool shouldClear[] = {false, false};

void fontInit() {
	extern u32 rotatingCubesLoaded;
	bool useExpansionPak = (sys().isRegularDS() && ((*(u16*)(0x020000C0) != 0 && *(u16*)(0x020000C0) != 0x5A45) || *(vu16*)(0x08240000) == 1) && (*(u16*)(0x020000C0) != 0 || !rotatingCubesLoaded)
							&& (io_dldi_data->ioInterface.features & FEATURE_SLOT_NDS));

	// Unload fonts if already loaded
	if(smallFont)
		delete smallFont;
	if(largeFont)
		delete largeFont;

	// Load font graphics
	std::string fontPath = std::string(sdFound() ? "sd:" : "fat:") + "/_nds/TWiLightMenu/extras/fonts/" + ms().font;
	smallFont = new FontGraphic({fontPath + (((dsiFeatures() && !sys().arm7SCFGLocked()) || sys().dsDebugRam() || useExpansionPak) ? "/small-dsi.nftr" : "/small-ds.nftr"), fontPath + "/small.nftr", "nitro:/graphics/font/small.nftr"}, useExpansionPak);
	largeFont = new FontGraphic({fontPath + (((dsiFeatures() && !sys().arm7SCFGLocked()) || sys().dsDebugRam() || useExpansionPak) ? "/large-dsi.nftr" : "/large-ds.nftr"), fontPath + "/large.nftr", "nitro:/graphics/font/large.nftr"}, useExpansionPak);

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

void fontReinit() {
	// Unload fonts if already loaded
	if(smallFont)
		delete smallFont;
	if(largeFont)
		delete largeFont;

	// Load font graphics
	std::string fontPath = std::string("fat:") + "/_nds/TWiLightMenu/extras/fonts/" + ms().font;
	smallFont = new FontGraphic({fontPath + "/small-ds.nftr", fontPath + "/small.nftr", "nitro:/graphics/font/small.nftr"}, false);
	largeFont = new FontGraphic({fontPath + "/large-ds.nftr", fontPath + "/large.nftr", "nitro:/graphics/font/large.nftr"}, false);
}

void esrbDescFontInit(bool dsFont) {
	esrbDescFont = new FontGraphic({dsFont ? "nitro:/graphics/font/ds.nftr" : "nitro:/graphics/font/small.nftr"}, false);
}

void esrbDescFontDeinit() {
	if(esrbDescFont)
		delete esrbDescFont;
}

static std::list<TextEntry> &getTextQueue(bool top) {
	return top ? topText : bottomText;
}

FontGraphic *getFont(bool large) {
	return large ? largeFont : smallFont;
}

void updateText(bool top) {
	if(top)	return; // Assign some VRAM and remove this to be able to print to top

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

void updateTextImg(u16* img, bool top) {
	if(top)	return;

	// Clear before redrawing
	if(shouldClear[top]) {
		dmaFillWords(0, FontGraphic::textBuf[top], 256 * 192);
		shouldClear[top] = false;
	}

	// Draw text
	auto &text = getTextQueue(top);
	for(auto it = text.begin(); it != text.end(); ++it) {
		if(esrbDescFont)
			esrbDescFont->print(it->x, it->y, top, it->message, it->align);
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
