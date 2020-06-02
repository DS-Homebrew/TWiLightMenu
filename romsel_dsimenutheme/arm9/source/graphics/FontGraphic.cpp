#include "FontGraphic.h"

#include "common/tonccpy.h"

FontGraphic::FontGraphic(const std::string &path, const std::string &fallback) {
	FILE *file = fopen(path.c_str(), "rb");
	if(!file) {
		file = fopen(fallback.c_str(), "rb");
	}

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

		// Allocate character buffer
		characterBuffer = std::vector<u8>(tileWidth * tileHeight);
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

uint FontGraphic::calcWidth(std::u16string_view text) {
	uint x = 0;

	for(auto c : text) {
		u16 index = getCharIndex(c);
		x += fontWidths[(index * 3) + 2];
	}

	return x;
}

ITCM_CODE void FontGraphic::print(int x, int y, std::u16string_view text, Alignment align) {
	// Adjust x for alignment
	switch(align) {
		case Alignment::left: {
			break;
		} case Alignment::center: {
			size_t newline = text.find('\n');
			while(newline != text.npos) {
				print(x, y, text.substr(0, newline), align);
				text = text.substr(newline + 1);
				newline = text.find('\n');
				y += tileHeight;
			}
			x = ((256 - calcWidth(text)) / 2) + x;
			break;
		} case Alignment::right: {
			x = x - calcWidth(text);
			break;
		}
	}
	const int xStart = x;

	// Loop through string and print it
	for(auto c : text) {
		if(c == '\n') {
			x = xStart;
			y += tileHeight;
			continue;
		}

		u16 index = getCharIndex(c);
		for(int i = 0; i < tileSize; i++) {
			u8 tile = fontTiles[i + (index * tileSize)];
			characterBuffer[(i * 4)]     = (tile >> 6 & 3);
			characterBuffer[(i * 4) + 1] = (tile >> 4 & 3);
			characterBuffer[(i * 4) + 2] = (tile >> 2 & 3);
			characterBuffer[(i * 4) + 3] = (tile      & 3);
		}

		if(x + fontWidths[(index * 3) + 2] > 256)
			return;

		u8 *dst = (u8*)bgGetGfxPtr(2) + x + fontWidths[(index * 3)];
		for(int i = 0; i < tileHeight; i++) {
			tonccpy(dst + ((y + i) * 256), &characterBuffer[i * tileWidth], tileWidth);
		}

		x += fontWidths[(index * 3) + 2];
	}
}
