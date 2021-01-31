#include "FontGraphic.h"

#include "common/tonccpy.h"

static u8* lastUsedLoc = (u8*)0x08000000;

u8 FontGraphic::textBuf[2][256 * 192];

bool FontGraphic::isStrongRTL(char16_t c) {
	return (c >= 0x0590 && c <= 0x05FF) || c == 0x200F;
}

bool FontGraphic::isWeak(char16_t c) {
	return c < 'A' || (c > 'Z' && c < 'a') || (c > 'z' && c < 127);
}

bool FontGraphic::isNumber(char16_t c) {
	return c >= '0' && c <= '9';
}

FontGraphic::FontGraphic(const std::vector<std::string> &paths, const bool useExpansionPak) {
	FILE *file = nullptr;
	for(const auto &path : paths) {
		file = fopen(path.c_str(), "rb");
		if(file)
			break;
	}

	if(file) {
		if (useExpansionPak && *(u16*)(0x020000C0) == 0 && lastUsedLoc == (u8*)0x08000000) {
			lastUsedLoc += 0x01000000;
		}

		// Get file size
		fseek(file, 0, SEEK_END);
		u32 fileSize = ftell(file);

		// Skip font info
		fseek(file, 0x14, SEEK_SET);
		fseek(file, fgetc(file)-1, SEEK_CUR);

		// Load glyph info
		u32 chunkSize;
		fread(&chunkSize, 4, 1, file);
		tileWidth = fgetc(file);
		tileHeight = fgetc(file);
		fread(&tileSize, 2, 1, file);

		// Load character glyphs
		int tileAmount = ((chunkSize-0x10)/tileSize);
		if (!useExpansionPak) {
			fontTilesVector = std::vector<u8>(tileSize*tileAmount);
			fontTiles = fontTilesVector.data();
		} else {
			fontTiles = lastUsedLoc;
		}
		fseek(file, 4, SEEK_CUR);
		fread(fontTiles, tileSize, tileAmount, file);

		// Fix top row
		for(int i=0;i<tileAmount;i++) {
			fontTiles[i*tileSize] = 0;
			fontTiles[i*tileSize+1] = 0;
			fontTiles[i*tileSize+2] = 0;
		}

		// Load character widths
		fseek(file, 0x24, SEEK_SET);
		u32 locHDWC;
		fread(&locHDWC, 4, 1, file);
		fseek(file, locHDWC-4, SEEK_SET);
		fread(&chunkSize, 4, 1, file);
		fseek(file, 8, SEEK_CUR);
		if (!useExpansionPak) {
			fontWidthsVector = std::vector<u8>(3*tileAmount);
			fontWidths = fontWidthsVector.data();
		} else {
			fontWidths = fontTiles+(tileSize*tileAmount);
			lastUsedLoc = fontWidths+((3*tileAmount)*4);
		}
		fread(fontWidths, 3, tileAmount, file);

		// Load character maps
		fontMap = std::vector<u16>(tileAmount);
		fseek(file, 0x28, SEEK_SET);
		u32 locPAMC, mapType;
		fread(&locPAMC, 4, 1, file);

		while(locPAMC < fileSize) {
			u16 firstChar, lastChar;
			fseek(file, locPAMC, SEEK_SET);
			fread(&firstChar, 2, 1, file);
			fread(&lastChar, 2, 1, file);
			fread(&mapType, 4, 1, file);
			fread(&locPAMC, 4, 1, file);

			switch(mapType) {
				case 0: {
					u16 firstTile;
					fread(&firstTile, 2, 1, file);
					for(unsigned i=firstChar;i<=lastChar;i++) {
						fontMap[firstTile+(i-firstChar)] = i;
					}
					break;
				} case 1: {
					for(int i=firstChar;i<=lastChar;i++) {
						u16 tile;
						fread(&tile, 2, 1, file);
						fontMap[tile] = i;
					}
					break;
				} case 2: {
					u16 groupAmount;
					fread(&groupAmount, 2, 1, file);
					for(int i=0;i<groupAmount;i++) {
						u16 charNo, tileNo;
						fread(&charNo, 2, 1, file);
						fread(&tileNo, 2, 1, file);
						fontMap[tileNo] = charNo;
					}
					break;
				}
			}
		}
		fclose(file);
		questionMark = getCharIndex(0xFFFD);
		if(questionMark == 0)
			questionMark = getCharIndex('?');
	}
}

u16 FontGraphic::getCharIndex(char16_t c) {
	// Try a binary search
	int left = 0;
	int right = fontMap.size();

	while(left <= right) {
		int mid = left + ((right - left) / 2);
		if(fontMap[mid] == c) {
			return mid;
		}

		if(fontMap[mid] < c) {
			left = mid + 1;
		} else {
			right = mid - 1;
		}
	}

	return questionMark;
}

std::u16string FontGraphic::utf8to16(std::string_view text) {
	std::u16string out;
	for(uint i=0;i<text.size();) {
		char16_t c;
		if(!(text[i] & 0x80)) {
			c = text[i++];
		} else if((text[i] & 0xE0) == 0xC0) {
			c  = (text[i++] & 0x1F) << 6;
			c |=  text[i++] & 0x3F;
		} else if((text[i] & 0xF0) == 0xE0) {
			c  = (text[i++] & 0x0F) << 12;
			c |= (text[i++] & 0x3F) << 6;
			c |=  text[i++] & 0x3F;
		} else {
			i++; // out of range or something (This only does up to 0xFFFF since it goes to a U16 anyways)
		}
		out += c;
	}
	return out;
}

int FontGraphic::calcWidth(std::u16string_view text) {
	uint x = 0;

	for(auto c : text) {
		u16 index = getCharIndex(c);
		x += fontWidths[(index * 3) + 2];
	}

	return x;
}

ITCM_CODE void FontGraphic::print(int x, int y, bool top, std::u16string_view text, Alignment align, bool rtl) {
	// If RTL isn't forced, check for RTL text
	if(!rtl) {
		for(const auto c : text) {
			if(isStrongRTL(c)) {
				rtl = true;
				break;
			}
		}
	}
	auto ltrBegin = text.end(), ltrEnd = text.end();

	// Adjust x for alignment
	switch(align) {
		case Alignment::left: {
			break;
		} case Alignment::center: {
			size_t newline = text.find('\n');
			while(newline != text.npos) {
				print(x, y, top, text.substr(0, newline), align, rtl);
				text = text.substr(newline + 1);
				newline = text.find('\n');
				y += tileHeight;
			}

			x = ((256 - calcWidth(text)) / 2) + x;
			break;
		} case Alignment::right: {
			size_t newline = text.find('\n');
			while(newline != text.npos) {
				print(x - calcWidth(text.substr(0, newline)), y, top, text.substr(0, newline), Alignment::left, rtl);
				text = text.substr(newline + 1);
				newline = text.find('\n');
				y += tileHeight;
			}
			x = x - calcWidth(text);
			break;
		}
	}
	const int xStart = x;

	// Loop through string and print it
	for(auto it = (rtl ? text.end() - 1 : text.begin()); true; it += (rtl ? -1 : 1)) {
		// If we hit the end of the string in an LTR section of an RTL
		// string, it may not be done, if so jump back to printing RTL
		if(it == (rtl ? text.begin() - 1 : text.end())) {
			if(ltrBegin == text.end() || (ltrBegin == text.begin() && ltrEnd == text.end())) {
				break;
			} else {
				it = ltrBegin;
				ltrBegin = text.end();
				rtl = true;
			}
		}

		// If at the end of an LTR section within RTL, jump back to the RTL
		if(it == ltrEnd && ltrBegin != text.end()) {
			if(ltrBegin == text.begin() && (!isWeak(*ltrBegin) || isNumber(*ltrBegin)))
				break;

			it = ltrBegin;
			ltrBegin = text.end();
			rtl = true;
		// If in RTL and hit a non-RTL character that's not punctuation, switch to LTR
		} else if(rtl && !isStrongRTL(*it) && (!isWeak(*it) || isNumber(*it))) {
			// Save where we are as the end of the LTR section
			ltrEnd = it + 1;

			// Go back until an RTL character or the start of the string
			bool allNumbers = true;
			while(!isStrongRTL(*it) && it != text.begin()) {
				// Check for if the LTR section is only numbers,
				// if so they won't be removed from the end
				if(allNumbers && !isNumber(*it) && !isWeak(*it))
					allNumbers = false;
				it--;
			}

			// Save where we are to return to after printing the LTR section
			ltrBegin = it;

			// If on an RTL char right now, add one
			if(isStrongRTL(*it)) {
				it++;
			}

			// Remove all punctuation and, if the section isn't only numbers,
			// numbers from the end of the LTR section
			if(allNumbers) {
				while(isWeak(*it) && !isNumber(*it)) {
					if(it != text.begin())
						ltrBegin++;
					it++;
				}
			} else {
				while(isWeak(*it)) {
					if(it != text.begin())
						ltrBegin++;
					it++;
				}
			}

			rtl = false;
		}

		if(*it == '\n') {
			x = xStart;
			y += tileHeight;
			continue;
		}

		// Brackets are flipped in RTL
		u16 index;
		if(rtl) {
			switch(*it) {
				case '(':
					index = getCharIndex(')');
					break;
				case ')':
					index = getCharIndex('(');
					break;
				case '[':
					index = getCharIndex(']');
					break;
				case ']':
					index = getCharIndex('[');
					break;
				case '<':
					index = getCharIndex('>');
					break;
				case '>':
					index = getCharIndex('<');
					break;
				default:
					index = getCharIndex(*it);
					break;
			}
		} else {
			index = getCharIndex(*it);
		}

		// Don't draw off screen chars
		if(x >= 0 && x + fontWidths[(index * 3) + 2] < 256 && y >= 0 && y + tileHeight < 192) {
			u8 *dst = textBuf[top] + x + fontWidths[(index * 3)];
			for(int i = 0; i < tileHeight; i++) {
				for(int j = 0; j < tileWidth; j++) {
					u8 px = fontTiles[(index * tileSize) + (i * tileWidth + j) / 4] >> ((3 - ((i * tileWidth + j) % 4)) * 2) & 3;
					if(px)
						dst[(y + i) * 256 + j] = px;
				}
			}
		}

		x += fontWidths[(index * 3) + 2];
	}
}
