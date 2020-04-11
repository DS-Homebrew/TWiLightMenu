#include "FontGraphic.h"

#include "common/tonccpy.h"

FontGraphic::FontGraphic(const std::string &path) {
	FILE *file = fopen(path.c_str(), "rb");

	if(file) {
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
		fontTiles = std::vector<u8>(tileSize*tileAmount);
		fseek(file, 4, SEEK_CUR);
		fread(fontTiles.data(), tileSize, tileAmount, file);

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
		fontWidths = std::vector<u8>(3*tileAmount);
		fread(fontWidths.data(), 3, tileAmount, file);

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

	// If that doesn't find the char, do a linear search
	for(unsigned int i=0;i<fontMap.size();i++) {
		if(fontMap[i] == c)	return i;
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
	int x = 0;

	for(uint i = 0; i < text.size(); i++) {
		u16 index = getCharIndex(text[i]);
		x += fontWidths[(index * 3) + 2];
	}

	return x;
}

void FontGraphic::print(int x, int y, std::u16string_view text, Alignment align) {
	// Adjust x for alignment
	switch(align) {
		case Alignment::left:
			break;
		case Alignment::center:
			x = ((256 - calcWidth(text)) / 2) + x;
			break;
		case Alignment::right:
			x = x - calcWidth(text);
	}
	const int xStart = x;

	// Loop through string and print it
	for(uint i = 0; i < text.size(); i++) {
		if(text[i] == '\n') {
			x = xStart;
			y += tileHeight;
			continue;
		}

		int index = getCharIndex(text[i]);
		u8 bitmap[tileWidth * tileHeight];
		for(int i=0;i<tileSize;i++) {
			bitmap[(i * 4)]     = (fontTiles[i + (index * tileSize)] >> 6 & 3);
			bitmap[(i * 4) + 1] = (fontTiles[i + (index * tileSize)] >> 4 & 3);
			bitmap[(i * 4) + 2] = (fontTiles[i + (index * tileSize)] >> 2 & 3);
			bitmap[(i * 4) + 3] = (fontTiles[i + (index * tileSize)]      & 3);
		}

		if(x + fontWidths[(index * 3) + 2] > 256) {
			x = xStart;
			y += tileHeight;
		}

		u8 *dst = (u8*)bgGetGfxPtr(2); // BG2 Main
		for(int i = 0; i < tileHeight; i++) {
			tonccpy(dst + ((y + i) * 256 + x + fontWidths[(index * 3)]), bitmap + ((i * tileWidth)), tileWidth);
		}

		x += fontWidths[(index * 3) + 2];
		// if( i++ > 3)	return;
	}
}
