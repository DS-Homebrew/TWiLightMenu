/*-----------------------------------------------------------------
 Copyright (C) 2015
	Matthew Scholefield

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

------------------------------------------------------------------*/

#include <nds.h>
#include <gl2d.h>
#include "fileCopy.h"
#include "common/lodepng.h"
#include "bios_decompress_callback.h"
#include "FontGraphic.h"
#include "common/inifile.h"
#include "common/twlmenusettings.h"
#include "common/systemdetails.h"
#include "graphics/color.h"

// Graphic files
#include "icon_manual.h"
#include "iconbox.h"
#include "wirelessicons.h"

#include "../iconTitle.h"
#include "graphics.h"
#include "fontHandler.h"
#include "../ndsheaderbanner.h"
#include "../errorScreen.h"

#define CONSOLE_SCREEN_WIDTH 32
#define CONSOLE_SCREEN_HEIGHT 24

extern bool extension(const std::string_view filename, const std::vector<std::string_view> extensions);

extern bool whiteScreen;
extern bool blackScreen;
extern bool fadeType;
extern bool fadeSpeed;
extern bool controlTopBright;
extern bool controlBottomBright;
int fadeDelay = 0;

extern int colorRvalue;
extern int colorGvalue;
extern int colorBvalue;

int screenBrightness = 0;
bool lcdSwapped = false;
static bool secondBuffer = false;
bool doubleBuffer = true;
bool updateFrame = true;

int vblankRefreshCounter = 0;

extern int spawnedtitleboxes;

extern bool startMenu;

extern int startMenu_cursorPosition;

bool manualIconNextImg = false;

bool showdialogbox = false;
int dialogboxHeight = 0;

int manualTexID, iconboxTexID, wirelessiconTexID;

glImage manualIcon[(32 / 32) * (64 / 32)];
glImage iconboxImage[(64 / 16) * (64 / 16)];
glImage wirelessIcons[(32 / 32) * (64 / 32)];

int bottomBg;

u16 bmpImageBuffer[256*192];
u16 topImage[2][2][256*192];
u16 bottomImage[2][2][256*192];
u16 topImageWithText[2][2][256*192];
u16* colorTable = NULL;

u16 startBorderColor = 0;
static u16 windowColorTop = 0;
static u16 windowColorBottom = 0;

void ClearBrightness(void) {
	fadeType = true;
	screenBrightness = 0;
	swiWaitForVBlank();
	swiWaitForVBlank();
}

bool screenFadedIn(void) { return (screenBrightness == 0); }
bool screenFadedOut(void) { return (screenBrightness > 24); }

bool invertedColors = false;
bool noWhiteFade = false;

// Ported from PAlib (obsolete)
void SetBrightness(u8 screen, s8 bright) {
	if ((invertedColors && bright != 0) || (noWhiteFade && bright > 0)) {
		bright -= bright*2; // Invert brightness to match the inverted colors
	}

	u16 mode = 1 << 14;

	if (bright < 0) {
		mode = 2 << 14;
		bright = -bright;
	}
	if (bright > 31) bright = 31;
	*(vu16*)(0x0400006C + (0x1000 * screen)) = bright + mode;
}

//-------------------------------------------------------
// set up a 2D layer construced of bitmap sprites
// this holds the image when rendering to the top screen
//-------------------------------------------------------

void initSubSprites(void)
{

	oamInit(&oamSub, SpriteMapping_Bmp_2D_256, false);
	int id = 0;

	//set up a 4x3 grid of 64x64 sprites to cover the screen
	for (int y = 0; y < 3; y++)
		for (int x = 0; x < 4; x++) {
			oamSub.oamMemory[id].attribute[0] = ATTR0_BMP | ATTR0_SQUARE | (64 * y);
			oamSub.oamMemory[id].attribute[1] = ATTR1_SIZE_64 | (64 * x);
			oamSub.oamMemory[id].attribute[2] = ATTR2_ALPHA(1) | (8 * 32 * y) | (8 * x);
			++id;
		}

	swiWaitForVBlank();

	oamUpdate(&oamSub);
}

u16 convertToDsBmp(u16 val) {
	val = ((val>>10)&31) | (val&31<<5) | (val&31)<<10 | BIT(15);
	if (colorTable) {
		return colorTable[val % 0x8000] | BIT(15);
	}
	return val;
}

/* u16 convertVramColorToGrayscale(u16 val) {
	u8 b,g,r,max,min;
	b = ((val)>>10)&31;
	g = ((val)>>5)&31;
	r = (val)&31;
	// Value decomposition of hsv
	max = (b > g) ? b : g;
	max = (max > r) ? max : r;

	// Desaturate
	min = (b < g) ? b : g;
	min = (min < r) ? min : r;
	max = (max + min) / 2;

	return 32768|(max<<10)|(max<<5)|(max);
} */

// No longer used.
// void drawBG(glImage *images)
// {
// 	for (int y = 0; y < 256 / 16; y++)
// 	{
// 		for (int x = 0; x < 256 / 16; x++)
// 		{
// 			int i = y * 16 + x;
// 			glSprite(x * 16, y * 16, GL_FLIP_NONE, &images[i & 255]);
// 		}
// 	}
// }

static void loadBmp(const bool top, const int startMenu, const char* filename) {
	FILE* file = fopen(filename, "rb");
	if (!file)
		return;

	// Read width & height
	fseek(file, 0x12, SEEK_SET);
	u32 width, height;
	fread(&width, 1, sizeof(width), file);
	fread(&height, 1, sizeof(height), file);

	if (width > 256 || height > 192) {
		fclose(file);
		return;
	}

	int xPos = 0;
	if (width <= 254) {
		// Adjust X position
		for (int i = width; i < 256; i += 2) {
			xPos++;
		}
	}

	int yPos = 0;
	if (height <= 190) {
		// Adjust Y position
		for (int i = height; i < 192; i += 2) {
			yPos++;
		}
	}

	fseek(file, 0x1C, SEEK_SET);
	u8 bitsPerPixel = fgetc(file);
	fseek(file, 0xE, SEEK_SET);
	u8 headerSize = fgetc(file);
	bool rgb565 = false;
	if (headerSize == 0x38) {
		// Check the upper byte green mask for if it's got 5 or 6 bits
		fseek(file, 0x2C, SEEK_CUR);
		rgb565 = fgetc(file) == 0x07;
		fseek(file, headerSize - 0x2E, SEEK_CUR);
	} else {
		fseek(file, headerSize - 1, SEEK_CUR);
	}
	if (bitsPerPixel == 24 || bitsPerPixel == 32) { // 24-bit or 32-bit
		int bits = (bitsPerPixel == 32) ? 4 : 3;

		u8 *bmpImageBuffer = new u8[(width * height)*bits];
		fread(bmpImageBuffer, bits, width * height, file);

		bool alternatePixel = false;
		int x = 0;
		int y = height-1;
		u8 pixelAdjustInfo = 0;
		for (u32 i = 0; i < width*height; i++) {
			pixelAdjustInfo = 0;
			if (alternatePixel) {
				if (bmpImageBuffer[(i*bits)] >= 0x4 && bmpImageBuffer[(i*bits)] < 0xFC) {
					bmpImageBuffer[(i*bits)] += 0x4;
					pixelAdjustInfo |= BIT(0);
				}
				if (bmpImageBuffer[(i*bits)+1] >= 0x4 && bmpImageBuffer[(i*bits)+1] < 0xFC) {
					bmpImageBuffer[(i*bits)+1] += 0x4;
					pixelAdjustInfo |= BIT(1);
				}
				if (bmpImageBuffer[(i*bits)+2] >= 0x4 && bmpImageBuffer[(i*bits)+2] < 0xFC) {
					bmpImageBuffer[(i*bits)+2] += 0x4;
					pixelAdjustInfo |= BIT(2);
				}
			}
			u16 color = bmpImageBuffer[(i*bits)+2]>>3 | (bmpImageBuffer[(i*bits)+1]>>3)<<5 | (bmpImageBuffer[i*bits]>>3)<<10 | BIT(15);
			if (colorTable) {
				color = colorTable[color % 0x8000] | BIT(15);
			}
			if (top) {
				topImage[startMenu][0][(xPos+x+(y*256))+(yPos*256)] = color;
			} else {
				bottomImage[startMenu][0][(xPos+x+(y*256))+(yPos*256)] = color;
			}
			if (alternatePixel) {
				if (pixelAdjustInfo & BIT(0)) {
					bmpImageBuffer[(i*bits)] -= 0x4;
				}
				if (pixelAdjustInfo & BIT(1)) {
					bmpImageBuffer[(i*bits)+1] -= 0x4;
				}
				if (pixelAdjustInfo & BIT(2)) {
					bmpImageBuffer[(i*bits)+2] -= 0x4;
				}
			} else {
				if (bmpImageBuffer[(i*bits)] >= 0x4 && bmpImageBuffer[(i*bits)] < 0xFC) {
					bmpImageBuffer[(i*bits)] += 0x4;
				}
				if (bmpImageBuffer[(i*bits)+1] >= 0x4 && bmpImageBuffer[(i*bits)+1] < 0xFC) {
					bmpImageBuffer[(i*bits)+1] += 0x4;
				}
				if (bmpImageBuffer[(i*bits)+2] >= 0x4 && bmpImageBuffer[(i*bits)+2] < 0xFC) {
					bmpImageBuffer[(i*bits)+2] += 0x4;
				}
			}
			color = bmpImageBuffer[(i*bits)+2]>>3 | (bmpImageBuffer[(i*bits)+1]>>3)<<5 | (bmpImageBuffer[i*bits]>>3)<<10 | BIT(15);
			if (colorTable) {
				color = colorTable[color % 0x8000] | BIT(15);
			}
			if (top) {
				topImage[startMenu][1][(xPos+x+(y*256))+(yPos*256)] = color;
			} else {
				bottomImage[startMenu][1][(xPos+x+(y*256))+(yPos*256)] = color;
			}
			x++;
			if (x == (int)width) {
				alternatePixel = !alternatePixel;
				x=0;
				y--;
			}
			alternatePixel = !alternatePixel;
		}
		delete[] bmpImageBuffer;
	} else if (bitsPerPixel == 16) { // 16-bit
		u16 *bmpImageBuffer = new u16[width * height];
		fread(bmpImageBuffer, 2, width * height, file);
		u16 *dst = (top ? topImage[startMenu][0] : bottomImage[startMenu][0]) + ((191 - ((192 - height) / 2)) * 256) + (256 - width) / 2;
		u16 *dst2 = (top ? topImage[startMenu][1] : bottomImage[startMenu][1]) + ((191 - ((192 - height) / 2)) * 256) + (256 - width) / 2;
		u16 *src = bmpImageBuffer;
		for (uint y = 0; y < height; y++, dst -= 256, dst2 -= 256) {
			for (uint x = 0; x < width; x++) {
				u16 val = *(src++);
				u16 color = ((val >> (rgb565 ? 11 : 10)) & 0x1F) | ((val >> (rgb565 ? 1 : 0)) & (0x1F << 5)) | (val & 0x1F) << 10 | BIT(15);
				if (colorTable) {
					color = colorTable[color % 0x8000] | BIT(15);
				}
				*(dst + x) = color;
				*(dst2 + x) = color;
			}
		}

		delete[] bmpImageBuffer;
	} else if (bitsPerPixel == 8) { // 8-bit
		u16* pixelBuffer = new u16[256];
		for (int i = 0; i < 256; i++) {
			u8 pixelB = 0;
			u8 pixelG = 0;
			u8 pixelR = 0;
			u8 unk = 0;
			fread(&pixelB, 1, 1, file);
			fread(&pixelG, 1, 1, file);
			fread(&pixelR, 1, 1, file);
			fread(&unk, 1, 1, file);
			pixelBuffer[i] = pixelR>>3 | (pixelG>>3)<<5 | (pixelB>>3)<<10 | BIT(15);
			if (colorTable) {
				pixelBuffer[i] = colorTable[pixelBuffer[i] % 0x8000] | BIT(15);
			}
		}
		u8 *bmpImageBuffer = new u8[width * height];
		fread(bmpImageBuffer, 1, width * height, file);

		int x = 0;
		int y = height-1;
		for (u32 i = 0; i < width*height; i++) {
			if (top) {
				topImage[startMenu][0][(xPos+x+(y*256))+(yPos*256)] = pixelBuffer[bmpImageBuffer[i]];
				topImage[startMenu][1][(xPos+x+(y*256))+(yPos*256)] = pixelBuffer[bmpImageBuffer[i]];
			} else {
				bottomImage[startMenu][0][(xPos+x+(y*256))+(yPos*256)] = pixelBuffer[bmpImageBuffer[i]];
				bottomImage[startMenu][1][(xPos+x+(y*256))+(yPos*256)] = pixelBuffer[bmpImageBuffer[i]];
			}
			x++;
			if (x == (int)width) {
				x=0;
				y--;
			}
		}
		delete[] pixelBuffer;
		delete[] bmpImageBuffer;
	} else if (bitsPerPixel == 1) { // 1-bit
		u16 monoPixel[2] = {0};
		for (int i = 0; i < 2; i++) {
			u8 pixelB = 0;
			u8 pixelG = 0;
			u8 pixelR = 0;
			u8 unk = 0;
			fread(&pixelB, 1, 1, file);
			fread(&pixelG, 1, 1, file);
			fread(&pixelR, 1, 1, file);
			fread(&unk, 1, 1, file);
			monoPixel[i] = pixelR>>3 | (pixelG>>3)<<5 | (pixelB>>3)<<10 | BIT(15);
			if (colorTable) {
				monoPixel[i] = colorTable[monoPixel[i] % 0x8000] | BIT(15);
			}
		}
		u8 *bmpImageBuffer = new u8[(width * height)/8];
		fread(bmpImageBuffer, 1, (width * height)/8, file);

		int x = 0;
		int y = height-1;
		for (u32 i = 0; i < (width*height)/8; i++) {
			for (int b = 7; b >= 0; b--) {
				const u16 color = monoPixel[(bmpImageBuffer[i] & (BIT(b))) ? 1 : 0];
				if (top) {
					topImage[startMenu][0][(xPos+x+(y*256))+(yPos*256)] = color;
					topImage[startMenu][1][(xPos+x+(y*256))+(yPos*256)] = color;
				} else {
					bottomImage[startMenu][0][(xPos+x+(y*256))+(yPos*256)] = color;
					bottomImage[startMenu][1][(xPos+x+(y*256))+(yPos*256)] = color;
				}
				x++;
				if (x == (int)width) {
					x=0;
					y--;
				}
			}
		}
		delete[] bmpImageBuffer;
	}
	fclose(file);
}

static void loadPng(const bool top, const int startMenu, const std::string filename) {
	std::vector<unsigned char> image;
	unsigned width, height;
	lodepng::decode(image, width, height, filename);
	if (width > 256 || height > 192) return;

	int xPos = 0;
	if (width <= 254) {
		// Adjust X position
		for (int i = width; i < 256; i += 2) {
			xPos++;
		}
	}

	int yPos = 0;
	if (height <= 190) {
		// Adjust Y position
		for (int i = height; i < 192; i += 2) {
			yPos++;
		}
	}

	bool alternatePixel = false;
	int x = 0;
	int y = 0;
	u8 pixelAdjustInfo = 0;
	for (unsigned i=0;i<image.size()/4;i++) {
		pixelAdjustInfo = 0;
		if (alternatePixel) {
			if (image[(i*4)] >= 0x4 && image[(i*4)] < 0xFC) {
				image[(i*4)] += 0x4;
				pixelAdjustInfo |= BIT(0);
			}
			if (image[(i*4)+1] >= 0x4 && image[(i*4)+1] < 0xFC) {
				image[(i*4)+1] += 0x4;
				pixelAdjustInfo |= BIT(1);
			}
			if (image[(i*4)+2] >= 0x4 && image[(i*4)+2] < 0xFC) {
				image[(i*4)+2] += 0x4;
				pixelAdjustInfo |= BIT(2);
			}
			if (image[(i*4)+3] >= 0x4 && image[(i*4)+3] < 0xFC) {
				image[(i*4)+3] += 0x4;
				pixelAdjustInfo |= BIT(3);
			}
		}
		u16 res = 0;
		if (image[(i*4)+3] > 0) {
			u16 color = image[i*4]>>3 | (image[(i*4)+1]>>3)<<5 | (image[(i*4)+2]>>3)<<10 | BIT(15);
			if (colorTable) {
				color = colorTable[color % 0x8000] | BIT(15);
			}
			res = alphablend(color, colorTable ? colorTable[0] : 0, image[(i*4)+3]);
		}
		if (top) {
			topImage[startMenu][0][(xPos+x+(y*256))+(yPos*256)] = res;
		} else {
			bottomImage[startMenu][0][(xPos+x+(y*256))+(yPos*256)] = res;
		}
		if (alternatePixel) {
			if (pixelAdjustInfo & BIT(0)) {
				image[(i*4)] -= 0x4;
			}
			if (pixelAdjustInfo & BIT(1)) {
				image[(i*4)+1] -= 0x4;
			}
			if (pixelAdjustInfo & BIT(2)) {
				image[(i*4)+2] -= 0x4;
			}
			if (pixelAdjustInfo & BIT(3)) {
				image[(i*4)+3] -= 0x4;
			}
		} else {
			if (image[(i*4)] >= 0x4 && image[(i*4)] < 0xFC) {
				image[(i*4)] += 0x4;
			}
			if (image[(i*4)+1] >= 0x4 && image[(i*4)+1] < 0xFC) {
				image[(i*4)+1] += 0x4;
			}
			if (image[(i*4)+2] >= 0x4 && image[(i*4)+2] < 0xFC) {
				image[(i*4)+2] += 0x4;
			}
			if (image[(i*4)+3] >= 0x4 && image[(i*4)+3] < 0xFC) {
				image[(i*4)+3] += 0x4;
			}
		}
		res = 0;
		if (image[(i*4)+3] > 0) {
			u16 color = image[i*4]>>3 | (image[(i*4)+1]>>3)<<5 | (image[(i*4)+2]>>3)<<10 | BIT(15);
			if (colorTable) {
				color = colorTable[color % 0x8000] | BIT(15);
			}
			res = alphablend(color, colorTable ? colorTable[0] : 0, image[(i*4)+3]);
		}
		if (top) {
			topImage[startMenu][1][(xPos+x+(y*256))+(yPos*256)] = res;
		} else {
			bottomImage[startMenu][1][(xPos+x+(y*256))+(yPos*256)] = res;
		}
		x++;
		if ((unsigned)x == width) {
			alternatePixel = !alternatePixel;
			x=0;
			y++;
		}
		alternatePixel = !alternatePixel;
	}
}

void vBlankHandler()
{
	if (fadeType) {
		if (!fadeDelay) {
			screenBrightness--;
			if (screenBrightness < 0) screenBrightness = 0;
		}
		if (!fadeSpeed) {
			fadeDelay++;
			if (fadeDelay == 3) fadeDelay = 0;
		} else {
			fadeDelay = 0;
		}
	} else {
		if (!fadeDelay) {
			screenBrightness++;
			if (screenBrightness > 31) screenBrightness = 31;
		}
		if (!fadeSpeed) {
			fadeDelay++;
			if (fadeDelay == 3) fadeDelay = 0;
		} else {
			fadeDelay = 0;
		}
	}
	if (ms().macroMode) {
		SetBrightness(0, lcdSwapped ? (ms().theme==6 ? -screenBrightness : screenBrightness) : (ms().theme==6 ? -31 : 31));
		SetBrightness(1, !lcdSwapped ? (ms().theme==6 ? -screenBrightness : screenBrightness) : (ms().theme==6 ? -31 : 31));
	} else {
		if (controlBottomBright) SetBrightness(0, ms().theme==6 ? -screenBrightness : screenBrightness);
		if (controlTopBright) SetBrightness(1, ms().theme==6 ? -screenBrightness : screenBrightness);
	}

	static bool whiteScreenPrev = whiteScreen;
	static bool blackScreenPrev = blackScreen;
	static bool startMenuPrev = startMenu;
	static bool showdialogboxPrev = showdialogbox;
	static int dialogboxHeightPrev = dialogboxHeight;

	if (whiteScreenPrev != whiteScreen) {
		whiteScreenPrev = whiteScreen;
		updateFrame = true;
	}

	if (blackScreenPrev != blackScreen) {
		blackScreenPrev = blackScreen;
		updateFrame = true;
	}

	if (startMenuPrev != startMenu) {
		startMenuPrev = startMenu;
		updateFrame = true;
	}

	if (showdialogboxPrev != showdialogbox) {
		showdialogboxPrev = showdialogbox;
		updateFrame = true;
	}

	if (showdialogbox && (dialogboxHeightPrev != dialogboxHeight)) {
		dialogboxHeightPrev = dialogboxHeight;
		updateFrame = true;
	}

	if (startMenu) {
		manualIconNextImg = !manualIconNextImg;
		updateFrame = true;
	} else if (bnriconisDSi && playBannerSequence()) {
		updateFrame = true;
	}

	if (updateFrame) {
		glBegin2D();

		// glColor(RGB15(31, 31-(3*blfLevel), 31-(6*blfLevel)));
		glColor(RGB15(31, 31, 31));
		const u16 black = colorTable ? colorTable[RGB15(0, 0, 0) % 0x8000] : RGB15(0, 0, 0);
		const u16 white = colorTable ? colorTable[RGB15(31, 31, 31) % 0x8000] : RGB15(31, 31, 31);

	  if (ms().theme != TWLSettings::EThemeGBC) {
		if (startMenu) {
			glBox(10+(startMenu_cursorPosition*82), 62, 81+(startMenu_cursorPosition*82), 132, startBorderColor);
			glSprite(232, 2, GL_FLIP_NONE, &manualIcon[manualIconNextImg]);
		} else {
			glBoxFilled(35, 23, 217, 64, black);
			glBoxFilled(77, 24, 216, 63, white);
			glSprite(36, 24, GL_FLIP_NONE, iconboxImage);
			drawIcon(40, 28);
			if (bnrWirelessIcon > 0) glSprite(24, 12, GL_FLIP_NONE, &wirelessIcons[(bnrWirelessIcon-1) & 31]);
		}
	  }
		if (showdialogbox) {
			glBoxFilled(15, 71, 241, 121+(dialogboxHeight*12), black);
			glBoxFilledGradient(16, 72, 240, 86, windowColorTop, windowColorBottom, windowColorBottom, windowColorTop);
			glBoxFilled(16, 88, 240, 120+(dialogboxHeight*12), white);
		}
		if (whiteScreen) {
			glBoxFilled(0, 0, 256, 192, white);
		} else if (blackScreen) {
			glBoxFilled(0, 0, 256, 192, black);
		}

		glEnd2D();
		GFX_FLUSH = 0;
		updateFrame = false;
	}

	if (vblankRefreshCounter >= REFRESH_EVERY_VBLANKS) {
		if (!startMenu && !showdialogbox) {
			reloadIconPalettes();
		}
		vblankRefreshCounter = 0;
	} else {
		vblankRefreshCounter++;
	}

	if (doubleBuffer) {
		extern bool startMenu;
		dmaCopyHalfWordsAsynch(0, topImageWithText[startMenu][secondBuffer], BG_GFX_SUB, 0x18000);
		dmaCopyHalfWordsAsynch(1, bottomImage[startMenu][secondBuffer], BG_GFX, 0x18000);
		secondBuffer = !secondBuffer;
	}
}

void graphicsInit()
{	
	char currentSettingPath[40];
	sprintf(currentSettingPath, "%s:/_nds/colorLut/currentSetting.txt", (sys().isRunFromSD() ? "sd" : "fat"));

	if (access(currentSettingPath, F_OK) == 0) {
		// Load color LUT
		char lutName[128] = {0};
		FILE* file = fopen(currentSettingPath, "rb");
		fread(lutName, 1, 128, file);
		fclose(file);

		char colorTablePath[256];
		sprintf(colorTablePath, "%s:/_nds/colorLut/%s.lut", (sys().isRunFromSD() ? "sd" : "fat"), lutName);

		if (getFileSize(colorTablePath) == 0x10000) {
			colorTable = new u16[0x10000/sizeof(u16)];

			FILE* file = fopen(colorTablePath, "rb");
			fread(colorTable, 1, 0x10000, file);
			fclose(file);

			const u16 color0 = colorTable[0] | BIT(15);
			const u16 color7FFF = colorTable[0x7FFF] | BIT(15);

			invertedColors =
			  (color0 >= 0xF000 && color0 <= 0xFFFF
			&& color7FFF >= 0x8000 && color7FFF <= 0x8FFF);
			if (!invertedColors) noWhiteFade = (color7FFF < 0xF000);
		}
	}

	*(vu16*)(0x0400006C) |= BIT(14);
	*(vu16*)(0x0400006C) &= BIT(15);
	SetBrightness(0, 31);
	SetBrightness(1, 31);

	////////////////////////////////////////////////////////////
	videoSetMode(MODE_5_3D);
	videoSetModeSub(MODE_3_2D);

	// Initialize gl2d
	glScreen2D();
	// Make gl2d render on transparent stage.
	glClearColor(31,31,31,0);
	glDisable(GL_CLEAR_BMP);

	// Clear the GL texture state
	glResetTextures();

	// Set up enough texture memory for our textures
	// Bank A is just 128kb and we are using 194 kb of
	// sprites
	vramSetBankA(VRAM_A_MAIN_BG);
	vramSetBankB(VRAM_B_MAIN_BG);
	vramSetBankC(VRAM_C_SUB_BG);
	vramSetBankD(VRAM_D_TEXTURE);
	vramSetBankE(VRAM_E_TEX_PALETTE);
	vramSetBankF(VRAM_F_TEX_PALETTE_SLOT4);
	vramSetBankG(VRAM_G_TEX_PALETTE_SLOT5); // 16Kb of palette ram, and font textures take up 8*16 bytes.
	vramSetBankH(VRAM_H_SUB_BG_EXT_PALETTE);
	vramSetBankI(VRAM_I_SUB_SPRITE_EXT_PALETTE);

	lcdMainOnBottom();

	int bg3Main = bgInit(3, BgType_Bmp16, BgSize_B16_256x256, 0, 0);
	bgSetPriority(bg3Main, 3);

	int bg2Main = bgInit(2, BgType_Bmp8, BgSize_B8_256x256, 7, 0);
	bgSetPriority(bg2Main, 0);

	int bg3Sub = bgInitSub(3, BgType_Bmp16, BgSize_B16_256x256, 0, 0);
	bgSetPriority(bg3Sub, 3);

	// int bg2Sub = bgInitSub(2, BgType_Bmp8, BgSize_B8_256x256, 3, 0);
	// bgSetPriority(bg2Sub, 0);

	bgSetPriority(0, 1); // Set 3D to below text

	if (ms().macroMode && ms().theme == TWLSettings::EThemeGBC) {
		lcdMainOnTop();
		lcdSwapped = false;
	} else {
		lcdMainOnBottom();
		lcdSwapped = true;
	}

	// Make screens black
	u16 black = 0x8000;
	if (colorTable) {
		black = colorTable[0] | BIT(15);
	}

	dmaFillHalfWords(black, BG_GFX, 0x18000);
	dmaFillHalfWords(black, BG_GFX_SUB, 0x18000);
	SetBrightness(0, 0);
	SetBrightness(1, 0);
}

void graphicsLoad()
{	
	if (isDSiMode()) {
		loadSdRemovedImage();
	}

	// BG_PALETTE_SUB[255] = RGB15(31, 31-(3*blfLevel), 31-(6*blfLevel));
	BG_PALETTE_SUB[255] = RGB15(31, 31, 31);

	if (ms().theme == TWLSettings::EThemeGBC) {
		uint imageWidth, imageHeight;
		std::vector<unsigned char> image;

		lodepng::decode(image, imageWidth, imageHeight, "nitro:/graphics/gbcborder.png");

		for (uint i=0; i<image.size()/4; i++) {
			topImage[0][0][i] = image[i*4]>>3 | (image[(i*4)+1]>>3)<<5 | (image[(i*4)+2]>>3)<<10 | BIT(15);
			if (colorTable) {
				topImage[0][0][i] = colorTable[topImage[0][0][i] % 0x8000];
			}
			topImage[0][1][i] = topImage[0][0][i];
			topImage[1][0][i] = topImage[0][0][i];
			topImage[1][1][i] = topImage[0][0][i];
		}

		FILE* fileTop = fopen("nitro:/themes/gbnp/bg.bmp", "rb");

		if (fileTop) {
			// Start loading
			fseek(fileTop, 0xe, SEEK_SET);
			u8 pixelStart = (u8)fgetc(fileTop) + 0xe;
			fseek(fileTop, pixelStart, SEEK_SET);
			fread(bmpImageBuffer, 1, 0xB40A, fileTop);
			u16* src = bmpImageBuffer;
			int x = 48;
			int y = 167;
			for (int i=0; i<160*144; i++) {
				if (x >= 48+160) {
					x = 48;
					y--;
				}
				u16 val = *(src++);
				topImage[0][0][y*256+x] = convertToDsBmp(val);
				topImage[0][1][y*256+x] = topImage[0][0][y*256+x];
				topImage[1][0][y*256+x] = topImage[0][0][y*256+x];
				topImage[1][1][y*256+x] = topImage[0][0][y*256+x];
				x++;
			}
		}
		fclose(fileTop);
	} else
	for (int startMenu = 0; startMenu < 2; startMenu++) {
		std::string themePath = std::string(sys().isRunFromSD() ? "sd:" : "fat:") + "/_nds/TWiLightMenu/r4menu/themes/" + ms().r4_theme;
		{
			std::string themePathNested = themePath + "/" + ms().r4_theme;
			if (access(themePathNested.c_str(), F_OK) == 0) {
				themePath = themePathNested;
			}
		}
		std::string pathTop;
		if (startMenu) {
			if (access((themePath + "/logo.bmp").c_str(), F_OK) == 0) {
				pathTop = themePath + "/logo.bmp";
			} else if (access((themePath + "/logo.png").c_str(), F_OK) == 0) {
				pathTop = themePath + "/logo.png";
			} else {
				pathTop = "nitro:/themes/theme1/logo.png";
			}
		} else {
			if (access((themePath + "/bckgrd_1.bmp").c_str(), F_OK) == 0) {
				pathTop = themePath + "/bckgrd_1.bmp";
			} else if (access((themePath + "/bckgrd_1.png").c_str(), F_OK) == 0) {
				pathTop = themePath + "/bckgrd_1.png";
			} else {
				pathTop = "nitro:/themes/theme1/bckgrd_1.png";
			}
		}

		std::string pathBottom;
		if (startMenu) {
			if (access((themePath + "/icons.bmp").c_str(), F_OK) == 0) {
				pathBottom = themePath + "/icons.bmp";
			} else if (access((themePath + "/icons.png").c_str(), F_OK) == 0) {
				pathBottom = themePath + "/icons.png";
			} else {
				pathBottom = "nitro:/themes/theme1/icons.png";
			}
		} else {
			if (access((themePath + "/bckgrd_2.bmp").c_str(), F_OK) == 0) {
				pathBottom = themePath + "/bckgrd_2.bmp";
			} else if (access((themePath + "/bckgrd_2.png").c_str(), F_OK) == 0) {
				pathBottom = themePath + "/bckgrd_2.png";
			} else {
				pathBottom = "nitro:/themes/theme1/bckgrd_2.png";
			}
		}

		if (extension(pathTop.c_str(), {".png"})) {
			loadPng(true, startMenu, pathTop);
		} else {
			loadBmp(true, startMenu, pathTop.c_str());
		}
		if (extension(pathBottom.c_str(), {".png"})) {
			loadPng(false, startMenu, pathBottom);
		} else {
			loadBmp(false, startMenu, pathBottom.c_str());
		}
	}

	dmaCopyHalfWordsAsynch(0, topImage[0][0], topImageWithText[0][0], 0x18000);
	dmaCopyHalfWordsAsynch(1, topImage[0][1], topImageWithText[0][1], 0x18000);
	dmaCopyHalfWordsAsynch(2, topImage[1][0], topImageWithText[1][0], 0x18000);
	dmaCopyHalfWordsAsynch(3, topImage[1][1], topImageWithText[1][1], 0x18000);

	// Initialize the bottom background
	// bottomBg = bgInit(2, BgType_ExRotation, BgSize_ER_256x256, 0,1);

	startBorderColor = RGB15(colorRvalue/8, colorGvalue/8, colorBvalue/8) | BIT(15); // Bit 15 is needed for the color to display on the top screen
	windowColorTop = RGB15(0, 0, 31);
	windowColorBottom = RGB15(0, 0, 15);
	if (colorTable) {
		startBorderColor = colorTable[startBorderColor % 0x8000] | BIT(15);
		windowColorTop = colorTable[windowColorTop % 0x8000];
		windowColorBottom = colorTable[windowColorBottom % 0x8000];
	}

	/*if (subtheme >= 0 && subtheme < 12) {
		icon1TexID = glLoadTileSet(icon1Image, // pointer to glImage array
								73, // sprite width
								72, // sprite height
								128, // bitmap width
								72, // bitmap height
								GL_RGB256, // texture type for glTexImage2D() in videoGL.h
								TEXTURE_SIZE_128, // sizeX for glTexImage2D() in videoGL.h
								TEXTURE_SIZE_128, // sizeY for glTexImage2D() in videoGL.h
								TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
								256, // Length of the palette to use (16 colors)
								(u16*) icon1Pal, // Load our 16 color tiles palette
								(u8*) icon1Bitmap // image data generated by GRIT
								);
		icon2TexID = glLoadTileSet(icon2Image, // pointer to glImage array
								73, // sprite width
								72, // sprite height
								128, // bitmap width
								72, // bitmap height
								GL_RGB256, // texture type for glTexImage2D() in videoGL.h
								TEXTURE_SIZE_128, // sizeX for glTexImage2D() in videoGL.h
								TEXTURE_SIZE_128, // sizeY for glTexImage2D() in videoGL.h
								TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
								256, // Length of the palette to use (16 colors)
								(u16*) icon2Pal, // Load our 16 color tiles palette
								(u8*) icon2Bitmap // image data generated by GRIT
								);
		icon3TexID = glLoadTileSet(icon3Image, // pointer to glImage array
								73, // sprite width
								72, // sprite height
								128, // bitmap width
								72, // bitmap height
								GL_RGB256, // texture type for glTexImage2D() in videoGL.h
								TEXTURE_SIZE_128, // sizeX for glTexImage2D() in videoGL.h
								TEXTURE_SIZE_128, // sizeY for glTexImage2D() in videoGL.h
								TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
								256, // Length of the palette to use (16 colors)
								(u16*) icon3Pal, // Load our 16 color tiles palette
								(u8*) icon3Bitmap // image data generated by GRIT
								);
	}*/

	u16* newPalette = (u16*)icon_manualPal;
	if (colorTable) {
		for (int i2 = 0; i2 < 16; i2++) {
			*(newPalette+i2) = colorTable[*(newPalette+i2) % 0x8000];
		}
	}

	manualTexID = glLoadTileSet(manualIcon, // pointer to glImage array
							32, // sprite width
							32, // sprite height
							32, // bitmap width
							64, // bitmap height
							GL_RGB16, // texture type for glTexImage2D() in videoGL.h
							TEXTURE_SIZE_32, // sizeX for glTexImage2D() in videoGL.h
							TEXTURE_SIZE_64, // sizeY for glTexImage2D() in videoGL.h
							TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
							16, // Length of the palette to use (16 colors)
							(u16*) newPalette, // Load our 16 color tiles palette
							(u8*) icon_manualBitmap // image data generated by GRIT
							);

	newPalette = (u16*)iconboxPal;
	if (colorTable) {
		for (int i2 = 0; i2 < 16; i2++) {
			*(newPalette+i2) = colorTable[*(newPalette+i2) % 0x8000];
		}
	}

	iconboxTexID = glLoadTileSet(iconboxImage, // pointer to glImage array
							40, // sprite width
							40, // sprite height
							64, // bitmap width
							64, // bitmap height
							GL_RGB16, // texture type for glTexImage2D() in videoGL.h
							TEXTURE_SIZE_64, // sizeX for glTexImage2D() in videoGL.h
							TEXTURE_SIZE_64, // sizeY for glTexImage2D() in videoGL.h
							TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
							16, // Length of the palette to use (16 colors)
							(u16*) newPalette, // Load our 16 color tiles palette
							(u8*) iconboxBitmap // image data generated by GRIT
							);

	newPalette = (u16*)wirelessiconsPal;
	if (colorTable) {
		for (int i2 = 0; i2 < 16; i2++) {
			*(newPalette+i2) = colorTable[*(newPalette+i2) % 0x8000];
		}
	}

	wirelessiconTexID = glLoadTileSet(wirelessIcons, // pointer to glImage array
							32, // sprite width
							32, // sprite height
							32, // bitmap width
							64, // bitmap height
							GL_RGB16, // texture type for glTexImage2D() in videoGL.h
							TEXTURE_SIZE_32, // sizeX for glTexImage2D() in videoGL.h
							TEXTURE_SIZE_64, // sizeY for glTexImage2D() in videoGL.h
							TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
							16, // Length of the palette to use (16 colors)
							(u16*) newPalette, // Load our 16 color tiles palette
							(u8*) wirelessiconsBitmap // image data generated by GRIT
							);

	loadConsoleIcons();
	allocateBannerIconsToPreload();

	while (dmaBusy(0) || dmaBusy(1) || dmaBusy(2) || dmaBusy(3)) swiDelay(100);

	irqSet(IRQ_VBLANK, vBlankHandler);
	irqEnable(IRQ_VBLANK);
}
