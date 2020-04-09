#include "TextEntry.h"

TextEntry::TextEntry(bool large, int x, int y, const std::string &message, Alignment align)
	: large(large), x(x), y(y), message(message), align(align) {};
