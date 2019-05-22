#include "topText.h"
#include <nds.h>
#include <stdio.h>
#include "uvcoord_top_font.h"
#include "graphics/fontHandler.h"

extern u16 smallFontCache[512*160];

/**
 * Get the index in the UV coordinate array where the letter appears
 */
unsigned int getTopFontSpriteIndex(const u16 letter) {
	unsigned int spriteIndex = 0;
	long int left = 0;
	long int right = TOP_FONT_NUM_IMAGES;
	long int mid = 0;

	while (left <= right) {
		mid = left + ((right - left) / 2);
		if (top_utf16_lookup_table[mid] == letter) {
			spriteIndex = mid;
			break;
		}

		if (top_utf16_lookup_table[mid] < letter) {
			left = mid + 1;
		} else {
			right = mid - 1;
		}
	}
	return spriteIndex;
}
/**
 * Prints texts to the top screen
 * topText is the text that will be printed 
 */
void printTopText(char topText[48]) {
	int x = 4;

	for (int c = 0; c < 48; c++) {
		unsigned int charIndex = getTopFontSpriteIndex(topText[c]);

		for (int y = 0; y < 16; y++) {
			int currentCharIndex = ((512*((charIndex/42)*16+y))+top_font_texcoords[0+(4*charIndex)]);

			for (u16 i = 0; i < top_font_texcoords[2 + (4 * charIndex)]; i++) {
				if (smallFontCache[currentCharIndex+i] != 0xFC1F && smallFontCache[currentCharIndex] != 0x7C1F) { // Do not render magneta pixel
					BG_GFX_SUB[(y)*256+(i+x)] = smallFontCache[currentCharIndex+i];
				}
			}
		}
		x += top_font_texcoords[2 + (4 * charIndex)];
	}
}