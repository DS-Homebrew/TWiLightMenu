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
	u8 tileWidth, tileHeight;
	u16 tileSize;
	u16 questionMark = 0;
	std::vector<u8> fontTiles;
	std::vector<u8> fontWidths;
	std::vector<u16> fontMap;
	std::vector<u8> characterBuffer;

	u16 getCharIndex(char16_t c);

public:
	static std::u16string utf8to16(std::string_view text);

	FontGraphic() {};
	FontGraphic(const std::string &path, const std::string &fallback);

	u8 height(void) { return tileHeight; }

	int calcWidth(std::string_view text) { return calcWidth(utf8to16(text)); }
	int calcWidth(std::u16string_view text);

	void print(int x, int y, int value, Alignment align) { print(x, y, std::to_string(value), align); }
	void print(int x, int y, std::string_view text, Alignment align) { print(x, y, utf8to16(text), align); }
	void print(int x, int y, std::u16string_view text, Alignment align);
};