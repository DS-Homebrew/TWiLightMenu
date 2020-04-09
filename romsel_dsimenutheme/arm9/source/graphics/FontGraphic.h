#pragma once

#include <nds.h>
#include <string>
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
	u16 getCharIndex(char16_t c);
	static std::u16string utf8to16(const std::string &text);

public:
	FontGraphic() {};
	FontGraphic(const std::string &path);

	int height(void) { return tileHeight; }

	int calcWidth(const std::string &text) { return calcWidth(utf8to16(text)); }
	int calcWidth(const std::u16string &text);

	void print(int x, int y, int value, Alignment align) { print(x, y, std::to_string(value), align); }
	void print(int x, int y, const std::string &text, Alignment align) { print(x, y, utf8to16(text), align); }
	void print(int x, int y, const std::u16string &text, Alignment align);
};