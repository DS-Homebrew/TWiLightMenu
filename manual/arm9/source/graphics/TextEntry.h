#pragma once

#include "FontGraphic.h"

class TextEntry {
public:
	bool large;
	int x, y;
	std::u16string message;
	Alignment align;

	TextEntry(bool large, int x, int y, std::string_view message, Alignment align);
	TextEntry(bool large, int x, int y, std::u16string_view message, Alignment align);
};
