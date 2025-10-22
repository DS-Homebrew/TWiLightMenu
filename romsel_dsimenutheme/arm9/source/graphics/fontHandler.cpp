#include "fontHandler.h"

#include <nds/arm9/dldi.h>
#include <list>

#include "common/twlmenusettings.h"
#include "common/flashcard.h"
#include "common/logging.h"
#include "common/systemdetails.h"
#include "common/tonccpy.h"
#include "myDSiMode.h"
#include "startborderpal.h"
#include "TextEntry.h"
#include "ThemeConfig.h"
#include "themefilenames.h"
#include "paletteEffects.h"

FontGraphic *smallFont;
FontGraphic *largeFont;
FontGraphic *esrbDescFont;

std::list<TextEntry> topText, bottomText;

bool shouldClear[] = {false, false};

// Checks if any of the specified files exists
// Crashes on CycloDS iEvolution
/*bool fileExists(std::vector<std::string_view> paths) {
	for (const std::string_view &path : paths) {
		if (access(path.data(), F_OK) == 0)
			return true;
	}

	return false;
}*/
bool fileExists(const char* path) {
	if (access(path, F_OK) == 0)
		return true;

	return false;
}

void fontInit() {
	logPrint("fontInit() ");

	// extern u32 rotatingCubesLoaded;
	// const bool useExpansionPak = (sys().isRegularDS() && ((*(u16*)(0x020000C0) != 0 && *(u16*)(0x020000C0) != 0x5A45) || *(vu16*)(0x08240000) == 1) && (*(u16*)(0x020000C0) != 0 || !rotatingCubesLoaded)
	// 						&& (io_dldi_data->ioInterface.features & FEATURE_SLOT_NDS));
	const bool useTileCache = (!dsiFeatures() && !sys().dsDebugRam());

	// Unload fonts if already loaded
	if (smallFont)
		delete smallFont;
	if (largeFont)
		delete largeFont;

	// Load font graphics
	std::string fontPath = std::string(sys().isRunFromSD() ? "sd:" : "fat:") + "/_nds/TWiLightMenu/extras/fonts/" + ms().font;
	std::string defaultPath = std::string(sys().isRunFromSD() ? "sd:" : "fat:") + "/_nds/TWiLightMenu/extras/fonts/Default";
	if (ms().useThemeFont && (fileExists((TFN_FONT_SMALL_DSI).c_str()) || fileExists((TFN_FONT_SMALL).c_str()) || fileExists((TFN_FONT_LARGE_DSI).c_str()) || fileExists((TFN_FONT_LARGE).c_str())))
		fontPath = TFN_FONT_DIRECTORY;
	if (fileExists((fontPath + "/small-dsi.nftr").c_str())) {
		smallFont = new FontGraphic((fontPath + "/small-dsi.nftr").c_str(), useTileCache);
	} else if (fileExists((fontPath + "/small.nftr").c_str())) {
		smallFont = new FontGraphic((fontPath + "/small.nftr").c_str(), useTileCache);
	} else if (fileExists((defaultPath + "/small.nftr").c_str())) {
		smallFont = new FontGraphic((defaultPath + "/small.nftr").c_str(), useTileCache);
	} else {
		smallFont = new FontGraphic("nitro:/graphics/font/small.nftr", useTileCache);
	}
	// If custom small font but no custom large font, use small font as large font
	if ((fileExists((fontPath + "/small-dsi.nftr").c_str()) || fileExists((fontPath + "/small.nftr").c_str())) && (!fileExists((fontPath + "/large-dsi.nftr").c_str()) && !fileExists((fontPath + "/large.nftr").c_str())))
		largeFont = smallFont;
	else {
		if (fileExists((fontPath + "/large-dsi.nftr").c_str())) {
			largeFont = new FontGraphic((fontPath + "/large-dsi.nftr").c_str(), useTileCache);
		} else if (fileExists((fontPath + "/large.nftr").c_str())) {
			largeFont = new FontGraphic((fontPath + "/large.nftr").c_str(), useTileCache);
		} else if (fileExists((defaultPath + "/large.nftr").c_str())) {
			largeFont = new FontGraphic((defaultPath + "/large.nftr").c_str(), useTileCache);
		} else {
			largeFont = new FontGraphic("nitro:/graphics/font/large.nftr", useTileCache);
		}
	}

	extern bool useTwlCfg;
	int themeColor = useTwlCfg ? *(u8*)0x02000444 : PersonalData->theme;

	// Load palettes
	u16 palette[] = {
		tc().fontPalette1(),
		tc().fontPalette2(),
		tc().fontPalette3(),
		tc().fontPalette4(),
		tc().fontPaletteDisabled1(),
		tc().fontPaletteDisabled2(),
		tc().fontPaletteDisabled3(),
		tc().fontPaletteDisabled4(),
		tc().fontPaletteTitlebox1(),
		tc().fontPaletteTitlebox2(),
		tc().fontPaletteTitlebox3(),
		tc().fontPaletteTitlebox4(),
		tc().fontPaletteDialog1(),
		tc().fontPaletteDialog2(),
		tc().fontPaletteDialog3(),
		tc().fontPaletteDialog4(),
		tc().fontPaletteOverlay1(),
		tc().fontPaletteOverlay2(),
		tc().fontPaletteOverlay3(),
		tc().fontPaletteOverlay4(),
		tc().fontPaletteUsername1(),
		tc().fontPaletteUsername2(),
		tc().fontPaletteUsername3(),
		tc().fontPaletteUsername4(),
		tc().fontPaletteDateTime1(),
		tc().fontPaletteDateTime2(),
		tc().fontPaletteDateTime3(),
		tc().fontPaletteDateTime4(),
	};
	u16 paletteSub[] = {
		tc().fontPalette1(),
		tc().fontPalette2(),
		tc().fontPalette3(),
		tc().fontPalette4(),
		tc().fontPaletteDisabled1(),
		tc().fontPaletteDisabled2(),
		tc().fontPaletteDisabled3(),
		tc().fontPaletteDisabled4(),
		tc().fontPaletteTitlebox1(),
		tc().fontPaletteTitlebox2(),
		tc().fontPaletteTitlebox3(),
		tc().fontPaletteTitlebox4(),
		tc().fontPaletteDialog1(),
		tc().fontPaletteDialog2(),
		tc().fontPaletteDialog3(),
		tc().fontPaletteDialog4(),
		tc().fontPaletteOverlay1(),
		tc().fontPaletteOverlay2(),
		tc().fontPaletteOverlay3(),
		tc().fontPaletteOverlay4(),
		tc().fontPaletteUsername1(),
		tc().fontPaletteUsername2(),
		tc().fontPaletteUsername3(),
		tc().fontPaletteUsername4(),
		tc().fontPaletteDateTime1(),
		tc().fontPaletteDateTime2(),
		tc().fontPaletteDateTime3(),
		tc().fontPaletteDateTime4(),
	};
	if (tc().usernameUserPalette()) {
		const int userPalOffset = 4 * (int)FontPalette::name;
		FILE *file = fopen((TFN_PALETTE_USERNAME).c_str(), "rb");
		if (file) {
			fseek(file, themeColor * 4 * sizeof(u16), SEEK_SET);
			fread(palette + userPalOffset, sizeof(u16), 4, file);
			fclose(file);
			// swap palette bytes
			for (int i = 0; i < 4; i++) {
				palette[userPalOffset + i] = (palette[userPalOffset + i] << 8 & 0xFF00) | palette[userPalOffset + i] >> 8;
				paletteSub[userPalOffset + i] = palette[userPalOffset + i];
			}
		}
		else {
			tonccpy(palette + userPalOffset, (tc().usernameEdgeAlpha() ? bmpPal_topSmallFontForAlpha : bmpPal_topSmallFont) + themeColor, 4 * sizeof(u16));
			tonccpy(paletteSub + userPalOffset, bmpPal_topSmallFont + themeColor, 4 * sizeof(u16));
		}
	}
	effectColorModePalette(palette, sizeof(palette) / sizeof(palette[0]));
	effectColorModePalette(paletteSub, sizeof(paletteSub) / sizeof(paletteSub[0]));
	tonccpy(BG_PALETTE, paletteSub, sizeof(palette)); // For bottom screen
	tonccpy(BG_PALETTE_SUB, palette, sizeof(paletteSub)); // For top screen
	logPrint("Font inited\n");
}

void esrbDescFontInit(bool dsFont) {
	esrbDescFont = new FontGraphic(dsFont ? "nitro:/graphics/font/ds.nftr" : "nitro:/graphics/font/small.nftr", false);
}

void esrbDescFontDeinit() {
	if (esrbDescFont)
		delete esrbDescFont;
}

static std::list<TextEntry> &getTextQueue(bool top) {
	return top ? topText : bottomText;
}

FontGraphic *getFont(bool large) {
	return large ? largeFont : smallFont;
}

void updateText(bool top) {
	sassert(!top, "Top screen text must be copied\nmanually.");

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
