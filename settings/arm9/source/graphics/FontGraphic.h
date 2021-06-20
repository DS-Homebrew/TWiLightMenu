#pragma once

#include <nds.h>
#include <string>
#include <string_view>
#include <vector>

enum class Alignment {
	left,
	center,
	right,
};

class FontGraphic {
private:
	static bool isStrongRTL(char16_t c);
	static bool isWeak(char16_t c);
	static bool isNumber(char16_t c);

	static u8 *lastUsedLoc;

	bool useExpansionPak = false;
	u8 tileWidth = 0, tileHeight = 0;
	u16 tileSize = 0;
	int tileAmount = 0;
	u16 questionMark = 0;
	u8 *fontTiles = nullptr;
	u8 *fontWidths = nullptr;
	u16 *fontMap = nullptr;

	u16 getCharIndex(char16_t c);

public:
	static u8 textBuf[2][256 * 192];

	static std::u16string utf8to16(std::string_view text);

	FontGraphic(const std::vector<std::string> &paths, const bool useExpansionPak);

	~FontGraphic(void);

	u8 height(void) { return tileHeight; }

	int calcWidth(std::string_view text) { return calcWidth(utf8to16(text)); }
	int calcWidth(std::u16string_view text);

	void print(int x, int y, bool top, int value, Alignment align, bool rtl = false) { print(x, y, top, std::to_string(value), align, rtl); }
	void print(int x, int y, bool top, std::string_view text, Alignment align, bool rtl = false) { print(x, y, top, utf8to16(text), align, rtl); }
	void print(int x, int y, bool top, std::u16string_view text, Alignment align, bool rtl = false);
};
