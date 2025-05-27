#pragma once

#include <array>
#include <map>
#include <nds.h>
#include <string>
#include <string_view>
#include <vector>

#define tileCacheCount 128

enum class Alignment {
	left,
	center,
	right,
};
enum class FontPalette {
	regular = 0,
	disabled = 1,
	white = 2,
	sunday = 3,
	saturday = 4,
};

class FontGraphic {
private:
	static std::map<char16_t, std::array<char16_t, 3>> arabicPresentationForms;

	static bool isArabic(char16_t c);
	static bool isStrongRTL(char16_t c);
	static bool isWeak(char16_t c);
	static bool isNumber(char16_t c);

	static char16_t arabicForm(char16_t current, char16_t prev, char16_t next);

	FILE* file = nullptr;
	bool useTileCache = false;
	u8 tileOffset = 0;
	u8 tileWidth = 0, tileHeight = 0;
	u16 tileSize = 0;
	int tileAmount = 0;
	u16 questionMark = 0;
	u16 indexCache[tileCacheCount] = {0xFFFF};
	bool cacheAllocated[tileCacheCount] = {false};
	u8 nextCachePos = 0xFF;
	u8 *fontTiles = nullptr;
	u8 *fontWidths = nullptr;
	u16 *fontMap = nullptr;
	u8 fontFixedWidth = 8;

	u16 getCharIndex(char16_t c);

public:
	static u8 textBuf[2][256 * 192];

	static std::u16string utf8to16(std::string_view text);

	FontGraphic(const std::vector<std::string> &paths, const bool set_useTileCache);

	~FontGraphic(void);

	u8 height(void) { return tileHeight; }
	u8 fixedWidth(void) { return fontFixedWidth; }

	void setFixedWidthChar(char character);

	int calcWidth(std::string_view text, bool monospaced = false) { return calcWidth(utf8to16(text), monospaced); }
	int calcWidth(std::u16string_view text, bool monospaced = false);

	void print(int x, int y, bool top, int value, Alignment align, FontPalette palette, bool rtl = false, bool monospaced = false) { print(x, y, top, std::to_string(value), align, palette, rtl, monospaced); }
	void print(int x, int y, bool top, std::string_view text, Alignment align, FontPalette palette, bool rtl = false, bool monospaced = false) { print(x, y, top, utf8to16(text), align, palette, rtl, monospaced); }
	void print(int x, int y, bool top, std::u16string_view text, Alignment align, FontPalette palette, bool rtl = false, bool monospaced = false);
};
