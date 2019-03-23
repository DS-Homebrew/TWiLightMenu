#include <nds.h>
#include <nds/arm9/dldi.h>
#include <maxmod9.h>

#include <stdio.h>
#include <fat.h>
#include <sys/stat.h>
#include <limits.h>

#include <string.h>
#include <unistd.h>
#include "common/gl2d.h"

#include "graphics/graphics.h"

#include "common/nitrofs.h"
#include "nds_loader_arm9.h"
#include "errorScreen.h"

#include "graphics/fontHandler.h"

#include "inifile.h"

#include "soundbank.h"
#include "soundbank_bin.h"

////////////

#include "uvcoord_top_font.h"

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

void printTopText(char topText[64]) {
	// Load username
	char fontPath[64] = {0};
	FILE *file;
	int x = 4;

	for (int c = 0; c < 64; c++) {
		unsigned int charIndex = getTopFontSpriteIndex(topText[c]);
		// 42 characters per line.
		unsigned int texIndex = charIndex / 42;
		sprintf(fontPath, "nitro:/graphics/top_font/small_font_%u.bmp", texIndex);

		file = fopen(fontPath, "rb");

		if (file) {
			// Start loading
			fseek(file, 0xe, SEEK_SET);
			u8 pixelStart = (u8)fgetc(file) + 0xe;
			fseek(file, pixelStart, SEEK_SET);
			for (int y = 13; y >= 0; y--) {
				u16 buffer[512];
				fread(buffer, 2, 0x200, file);
				u16 *src = buffer + (top_font_texcoords[0 + (4 * charIndex)]);

				for (u16 i = 0; i < top_font_texcoords[2 + (4 * charIndex)]; i++) {
					u16 val = *(src++);

					// Blend with pixel
					// const u16 bg =
					//     _bgSubBuffer[(y + 2) * 256 + (i + x)]; // grab the background pixel
					// // Apply palette here.

					// // Magic numbers were found by dumping val to stdout
					// // on case default.
					// switch (val) {
					// // #ff00ff
					// case 0xFC1F:
					// 	break;
					// // #404040
					// case 0xA108:
					// 	val = alphablend(bmpPal_topSmallFont[1 + ((PersonalData->theme) * 16)],
					// 			 bg, 224U);
					// 	break;
					// // #808080
					// case 0xC210:
					// 	// blend the colors with the background to make it look better.
					// 	// Fills in the
					// 	// 1 for light
					// 	val = alphablend(bmpPal_topSmallFont[1 + ((PersonalData->theme) * 16)],
					// 			 bg, 224U);
					// 	break;
					// // #b8b8b8
					// case 0xDEF7:
					// 	// 6 looks good on lighter themes
					// 	// 3 do an average blend twice
					// 	//
					// 	val = alphablend(bmpPal_topSmallFont[3 + ((PersonalData->theme) * 16)],
					// 			 bg, 128U);
					// 	break;
					// default:
					// 	break;
					// }
					if (val != 0xFC1F && val != 0x7C1F) { // Do not render magneta pixel
						BG_GFX_SUB[(y+2)*256+(i+x)] = ((val>>10)&0x1f) | ((val)&(0x1f<<5)) | (val&0x1f)<<10 | BIT(15);
					}
				}
			}
			x += top_font_texcoords[2 + (4 * charIndex)];
		}

		fclose(file);
	}
}