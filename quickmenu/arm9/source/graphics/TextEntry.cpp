#include "TextEntry.h"

TextEntry::TextEntry(bool large, bool monospaced, int x, int y, std::string_view message, Alignment align, FontPalette palette)
	: large(large), monospaced(monospaced), x(x), y(y), message(FontGraphic::utf8to16(message)), align(align), palette(palette) {};

TextEntry::TextEntry(bool large, bool monospaced, int x, int y, std::u16string_view message, Alignment align, FontPalette palette)
	: large(large), monospaced(monospaced), x(x), y(y), message(message), align(align), palette(palette) {};
