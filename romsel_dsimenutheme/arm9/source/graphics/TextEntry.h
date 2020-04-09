#pragma once

#include "FontGraphic.h"

class TextEntry {
public:
	bool large;
	int x, y;
	std::string message;
	Alignment align;

	TextEntry(bool large, int x, int y, const std::string &message, Alignment align);
};