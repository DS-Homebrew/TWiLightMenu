#include "TextEntry.h"

TextEntry::TextEntry(bool large, int x, int y, std::string_view message, Alignment align, FontPalette palette)
	: large(large), x(x), y(y), message(FontGraphic::utf8to16(message)), align(align), palette(palette) {};

TextEntry::TextEntry(bool large, int x, int y, std::u16string_view message, Alignment align, FontPalette palette)
	: large(large), x(x), y(y), message(message), align(align), palette(palette) {};
