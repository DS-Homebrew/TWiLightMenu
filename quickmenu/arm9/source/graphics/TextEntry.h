#pragma once

#include "FontGraphic.h"

class TextEntry {
public:
	bool large;
	bool monospaced;
	int x, y;
	std::u16string message;
	Alignment align;
	FontPalette palette;

	TextEntry(bool large, bool monospaced, int x, int y, std::string_view message, Alignment align, FontPalette palette);
	TextEntry(bool large, bool monospaced, int x, int y, std::u16string_view message, Alignment align, FontPalette palette);
};
