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
#include "date.h"
#include "fileCopy.h"
#include "common/lodepng.h"
#include "bios_decompress_callback.h"
#include "FontGraphic.h"
#include "common/inifile.h"
#include "common/twlmenusettings.h"
#include "common/systemdetails.h"
#include "graphics/color.h"

// Graphic files
#include "wirelessicons.h"

#include "../iconTitle.h"
#include "graphics.h"
#include "fontHandler.h"
#include "../ndsheaderbanner.h"
#include "../errorScreen.h"

#define CONSOLE_SCREEN_WIDTH 32
#define CONSOLE_SCREEN_HEIGHT 24

extern bool extension(const std::string_view filename, const std::vector<std::string_view> extensions);

extern bool fadeType;
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

static int colonTimer = 0;
static bool showColon = true;

int frameOf60fps = 60;
int frameDelay = 0;
bool frameDelayEven = true; // For 24FPS
bool renderFrame = true;

extern int spawnedtitleboxes;

extern bool startMenu;

extern int startMenu_cursorPosition;

extern int cursorPosOnScreen;

bool showdialogbox = false;
int dialogboxHeight = 0;

int manualTexID, wirelessiconTexID;

glImage manualIcon[(32 / 32) * (64 / 32)];
glImage wirelessIcons[(32 / 32) * (64 / 32)];

int bottomBg;

u16 bmpImageBuffer[256*192];
u16 topImage[2][256*192];
u16 bottomImage[2][256*192];
u16 topImageWithText[2][256*192];
u16 bottomImageWithBar[2][256*192];

static u16* folderUpIcon[2] = {NULL};
static int folderUpIconW = 0;
static int folderUpIconH = 0;

static u16* clockNumbers[2] = {NULL};
static u16* clockColon[2] = {NULL};
static u16* yearNumbers[2] = {NULL};
static u16* dayNumbers[2] = {NULL};

static int bigClockX = 8;
static int bigClockY = 80;
static int clockNumbersW = 0;
static int clockNumbersH = 0;
static int clockColonW = 0;
static int clockColonH = 0;
static int yearNumbersW = 0;
static int yearNumbersH = 0;
static int dayNumbersW = 0;
static int dayNumbersH = 0;

static int yearX = 52;
static int yearY = 28;
static bool showYear = false;

static int monthX = 12;
static int monthY = 28;
static bool showMonth = false;

static int dayxX = 02;
static int dayxY = 28;
static bool showDayX = false;

static int dayPositionX = 134;
static int dayPositionY = 34;
static int daySizeX = 16;
static int daySizeY = 14;
static u16 dayHighlightColor = 0xfc00;

u16* colorTable = NULL;

bool displayIcons = false;
int iconsToDisplay = 0;
static bool iconScaleEnabled = false;
static int iconScaleWait = 0;
static int iconScale = 0;
static bool iconScaleLarge = true;
static bool iconScaleDelay = false;
static int iconShift = 0;

static u16 formFrameColor = RGB15(23,25,4);
static u16 formBodyColor = RGB15(30,29,22);

u16 startBorderColor = 0;
// static u16 windowColorTop = 0;
// static u16 windowColorBottom = 0;
static u16 selectionBarColor1 = 0x5c00;
static u16 selectionBarColor2 = 0x2d60;
static u8 selectionBarOpacity = 100;

void ClearBrightness(void) {
	fadeType = true;
	screenBrightness = 0;
	swiWaitForVBlank();
	swiWaitForVBlank();
}

bool screenFadedIn(void) { return (screenBrightness == 0); }

bool screenFadedOut(void) { return (screenBrightness > 24); }

// Ported from PAlib (obsolete)
void SetBrightness(u8 screen, s8 bright) {
	u16 mode = 1 << 14;

	if (bright < 0) {
		mode = 2 << 14;
		bright = -bright;
	}
	if (bright > 31) bright = 31;
	*(vu16*)(0x0400006C + (0x1000 * screen)) = bright + mode;
}

void frameRateHandler(void) {
	frameOf60fps++;
	if (frameOf60fps > 60) frameOf60fps = 1;

	if (!renderFrame) {
		frameDelay++;
		switch (ms().fps) {
			case 11:
				renderFrame = (frameDelay == 5+frameDelayEven);
				break;
			case 24:
			//case 25:
				renderFrame = (frameDelay == 2+frameDelayEven);
				break;
			case 48:
				renderFrame = (frameOf60fps != 3
							&& frameOf60fps != 8
							&& frameOf60fps != 13
							&& frameOf60fps != 18
							&& frameOf60fps != 23
							&& frameOf60fps != 28
							&& frameOf60fps != 33
							&& frameOf60fps != 38
							&& frameOf60fps != 43
							&& frameOf60fps != 48
							&& frameOf60fps != 53
							&& frameOf60fps != 58);
				break;
			case 50:
				renderFrame = (frameOf60fps != 3
							&& frameOf60fps != 9
							&& frameOf60fps != 16
							&& frameOf60fps != 22
							&& frameOf60fps != 28
							&& frameOf60fps != 34
							&& frameOf60fps != 40
							&& frameOf60fps != 46
							&& frameOf60fps != 51
							&& frameOf60fps != 58);
				break;
			default:
				renderFrame = (frameDelay == 60/ms().fps);
				break;
		}
	}
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
		return colorTable[val];
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

void resetIconScale(void) {
	iconScaleEnabled = false;
	iconScaleWait = 0;
	iconScale = 0;
	iconScaleLarge = true;
	iconScaleDelay = false;
	iconShift = 0;
}

ITCM_CODE void updateSelectionBar(void) {
	static int prevCurPos = 20;
	static int prevViewMode = 3;
	if (prevCurPos == cursorPosOnScreen && prevViewMode == ms().ak_viewMode) {
		return;
	}
	swiWaitForVBlank();
	if (prevCurPos != 20) {
		const int h = (prevViewMode != TWLSettings::EViewList) ? 38 : 11;
		const int hl = h-1;
		for (int y = 19+(prevCurPos*h); y <= 19+hl+(prevCurPos*h); y++) {
			for (int x = 2; x <= 253; x++) {
				bottomImageWithBar[0][(y*256)+x] = bottomImage[0][(y*256)+x];
				bottomImageWithBar[1][(y*256)+x] = bottomImage[1][(y*256)+x];
			}
		}
	}

	const int h = (ms().ak_viewMode != TWLSettings::EViewList) ? 38 : 11;
	const int hl = h-1;
	bool color2 = false;
	if (selectionBarOpacity == 100) {
		for (int y = 19+(cursorPosOnScreen*h); y <= 19+hl+(cursorPosOnScreen*h); y++) {
			for (int x = 2; x <= 253; x++) {
				const u16 color = color2 ? selectionBarColor2 : selectionBarColor1;
				bottomImageWithBar[0][(y*256)+x] = color;
				bottomImageWithBar[1][(y*256)+x] = color;
				color2 = !color2;
			}
			color2 = !color2;
		}
	} else {
		const u8 alpha = ((selectionBarOpacity * 32) / 25) * 2;
		for (int y = 19+(cursorPosOnScreen*h); y <= 19+hl+(cursorPosOnScreen*h); y++) {
			for (int x = 2; x <= 253; x++) {
				const u16 color = color2 ? selectionBarColor2 : selectionBarColor1;
				bottomImageWithBar[0][(y*256)+x] = alphablend(color, bottomImage[0][(y*256)+x], alpha);
				if (bottomImage[1][(y*256)+x] == bottomImage[0][(y*256)+x]) {
					bottomImageWithBar[1][(y*256)+x] = bottomImageWithBar[0][(y*256)+x];
				} else {
					bottomImageWithBar[1][(y*256)+x] = alphablend(color, bottomImage[1][(y*256)+x], alpha);
				}
				color2 = !color2;
			}
			color2 = !color2;
		}
	}

	prevCurPos = cursorPosOnScreen;
	prevViewMode = ms().ak_viewMode;
}

ITCM_CODE void displayFolderUp(const int x, const int y) {
	if (!folderUpIcon[0]) return;

	int src = 0;

	for (int y2 = y; y2 < y+folderUpIconH; y2++) {
		for (int x2 = x; x2 < x+folderUpIconW; x2++) {
			if (folderUpIcon[0][src] != (0 | BIT(15))) {
				bottomImageWithBar[0][(y2*256)+x2] = folderUpIcon[0][src];
				bottomImageWithBar[1][(y2*256)+x2] = folderUpIcon[1][src];
			}
			src++;
		}
	}

	delete[] folderUpIcon[0];
	delete[] folderUpIcon[1];
}

static std::string loadedTime;

ITCM_CODE void drawTime(void) {
	if (!clockNumbers[0] || !clockColon[0]) return;

	// Load time
	std::string currentTime = retTime();
	if (currentTime[0] == ' ')
		currentTime[0] = '0';
	currentTime[2] = showColon ? ':' : ' ';

	if (currentTime == loadedTime)
		return;

	loadedTime = currentTime;

	int x = bigClockX;

	for (int i = 0; i < 5; i++) {
		const int number = (int)currentTime[i]-0x30;
		const bool colon = (number == 10 || number == -0x10);

		const int y = bigClockY;

		int src = colon ? 0 : ((clockNumbersW * clockNumbersH) * number);

		if (colon) {
			for (int y2 = y; y2 < y+clockColonH; y2++) {
				for (int x2 = x; x2 < x+clockColonW; x2++) {
					if ((clockColon[0][src] == (0 | BIT(15))) || (number == -0x10)) {
						topImageWithText[0][(y2*256)+x2] = topImage[0][(y2*256)+x2];
						topImageWithText[1][(y2*256)+x2] = topImage[1][(y2*256)+x2];
					} else {
						topImageWithText[0][(y2*256)+x2] = clockColon[0][src];
						topImageWithText[1][(y2*256)+x2] = clockColon[1][src];
					}
					src++;
				}
			}
		} else {
			for (int y2 = y; y2 < y+clockNumbersH; y2++) {
				for (int x2 = x; x2 < x+clockNumbersW; x2++) {
					if (clockNumbers[0][src] == (0 | BIT(15))) {
						topImageWithText[0][(y2*256)+x2] = topImage[0][(y2*256)+x2];
						topImageWithText[1][(y2*256)+x2] = topImage[1][(y2*256)+x2];
					} else {
						topImageWithText[0][(y2*256)+x2] = clockNumbers[0][src];
						topImageWithText[1][(y2*256)+x2] = clockNumbers[1][src];
					}
					src++;
				}
			}
		}
		x += colon ? clockColonW : clockNumbersW;
	}
}

ITCM_CODE static void drawYearNumberString(const std::string string, const int len, int x, const int y) {
	for (int i = 0; i < len; i++) {
		const int number = (int)string[i]-0x30;

		int src = (yearNumbersW * yearNumbersH) * number;

		for (int y2 = y; y2 < y+yearNumbersH; y2++) {
			for (int x2 = x; x2 < x+yearNumbersW; x2++) {
				if (yearNumbers[0][src] == (0 | BIT(15))) {
					topImageWithText[0][(y2*256)+x2] = topImage[0][(y2*256)+x2];
					topImageWithText[1][(y2*256)+x2] = topImage[1][(y2*256)+x2];
				} else {
					topImageWithText[0][(y2*256)+x2] = yearNumbers[0][src];
					topImageWithText[1][(y2*256)+x2] = yearNumbers[1][src];
				}
				src++;
			}
		}
		x += yearNumbersW;
	}
}

static std::string loadedYear;

ITCM_CODE void drawYear(void) {
	if (!yearNumbers[0] || !showYear) return;

	// Load year
	std::string currentYear = retYear();

	if (currentYear == loadedYear)
		return;

	loadedYear = currentYear;

	drawYearNumberString(currentYear, 4, yearX, yearY);
}

static std::string loadedMonth;

ITCM_CODE void drawMonth(void) {
	if (!yearNumbers[0] || !showMonth) return;

	// Load month
	std::string currentMonth = retMonth();

	if (currentMonth == loadedMonth)
		return;

	loadedMonth = currentMonth;

	drawYearNumberString(currentMonth, 2, monthX, monthY);
}

static std::string loadedDayX;

ITCM_CODE void drawDayX(void) {
	if (!yearNumbers[0] || !showDayX) return;

	// Load day
	std::string currentDayX = retDay();

	if (currentDayX == loadedDayX)
		return;

	loadedDayX = currentDayX;

	drawYearNumberString(currentDayX, 2, dayxX, dayxY);
}

static u8 oldMonth = 0;
static u8 oldDaysOfMonth = 0;
static u16 oldYear = 0;

#define IS_LEAP(n) ((!(((n) + 1900) % 400) || (!(((n) + 1900) % 4) && (((n) + 1900) % 100))) != 0)
static u8 daysOfMonth()
{
    return (28 | (((IS_LEAP(dateYear()) ? 62648028 : 62648012) >> (dateMonth() * 2)) & 3));
}

static u8 weekDayOfFirstDay()
{
    return (dateWeekday() + 7 - ((dateDay() - 1) % 7)) % 7;
}

static u8 oldDaysX[32] = {0};
static u8 oldDaysY[32] = {0};
static bool oldDaysSaved = false;

static void clearDayNumber(u8 day)
{
	if (day > 31)
		return;

	const u8 x = oldDaysX[day];
	const u8 y = oldDaysY[day];

	for (int y2 = y; y2 < y+daySizeY-1; y2++) {
		for (int x2 = x; x2 < x+daySizeX-1; x2++) {
			topImageWithText[0][(y2*256)+x2] = topImage[0][(y2*256)+x2];
			topImageWithText[1][(y2*256)+x2] = topImage[1][(y2*256)+x2];
		}
	}
}

static void drawDayNumber(u8 day)
{
	if (day > 31)
		return;

	const u8 weekDayOfDay = (((day - 1) % 7) + weekDayOfFirstDay()) % 7;
	const u8 x = weekDayOfDay *daySizeX + dayPositionX;
	const u8 y = ((day - 1 + weekDayOfFirstDay()) / 7 * daySizeY) + dayPositionY;
	const u8 firstNumber = day / 10;
	const u8 secondNumber = day % 10;

	const int x3 = x - (daySizeX / 2 - dayNumbersW);
	const int y3 = y - (daySizeY - dayNumbersH) / 2;
	if (day == dateDay()) {
		for (int y2 = y3; y2 < y3+daySizeY-1; y2++) {
			for (int x2 = x3; x2 < x3+daySizeX-1; x2++) {
				topImageWithText[0][(y2*256)+x2] = dayHighlightColor;
				topImageWithText[1][(y2*256)+x2] = dayHighlightColor;
			}
		}
	} else {
		for (int y2 = y; y2 < y+daySizeY-1; y2++) {
			for (int x2 = x; x2 < x+daySizeX-1; x2++) {
				topImageWithText[0][(y2*256)+x2] = topImage[0][(y2*256)+x2];
				topImageWithText[1][(y2*256)+x2] = topImage[1][(y2*256)+x2];
			}
		}
	}

	for (int i = 0; i < 2; i++) {
		int src = (dayNumbersW * dayNumbersH) * ((i == 1) ? secondNumber : firstNumber);

		const int x3 = (i == 1) ? (x+dayNumbersW) : x;
		for (int y2 = y; y2 < y+dayNumbersH; y2++) {
			for (int x2 = x3; x2 < x3+dayNumbersW; x2++) {
				if (dayNumbers[0][src] != (0 | BIT(15))) {
					topImageWithText[0][(y2*256)+x2] = dayNumbers[0][src];
					topImageWithText[1][(y2*256)+x2] = dayNumbers[1][src];
				}
				src++;
			}
		}
	}

	oldDaysX[day] = x;
	oldDaysY[day] = y;
}

static std::string loadedDay;

void drawDay(void) {
	if (!dayNumbers[0]) return;

	// Load day
	std::string currentDay = retDay();

	if (currentDay == loadedDay)
		return;

	loadedDay = currentDay;

	if (oldDaysSaved && (dateMonth() != oldMonth || dateYear() != oldYear)) {
		for (u8 i = 1; i <= oldDaysOfMonth; ++i)
		{
			clearDayNumber(i);
		}
	}

	for (u8 i = 1; i <= daysOfMonth(); ++i)
	{
		drawDayNumber(i);
	}

	oldMonth = dateMonth();
	oldYear = dateYear();
	oldDaysOfMonth = daysOfMonth();
	oldDaysSaved = true;
}

enum class ImageType {
	bottom,
	top,
	folderUp,
	clockNumbers,
	clockColon,
	yearNumbers,
	dayNumbers
};

static void loadBmp(const ImageType type, const char* filename) {
	FILE* file = fopen(filename, "rb");
	if (!file)
		return;

	// Read width & height
	fseek(file, 0x12, SEEK_SET);
	u32 width, height;
	fread(&width, 1, sizeof(width), file);
	fread(&height, 1, sizeof(height), file);

	if (width > 256 || (height > 192 && type < ImageType::clockNumbers)) {
		fclose(file);
		return;
	}

	int xPos = 0;
	if (type < ImageType::folderUp && width <= 254) {
		// Adjust X position
		for (int i = width; i < 256; i += 2) {
			xPos++;
		}
	}

	int yPos = 0;
	if (type < ImageType::folderUp && height <= 190) {
		// Adjust Y position
		for (int i = height; i < 192; i += 2) {
			yPos++;
		}
	}

	if (type == ImageType::folderUp) {
		folderUpIconW = (int)width;
		folderUpIconH = (int)height;

		folderUpIcon[0] = new u16[width*height];
		folderUpIcon[1] = new u16[width*height];
	} else if (type == ImageType::clockNumbers) {
		clockNumbersW = (int)width;
		clockNumbersH = (int)height/10;

		clockNumbers[0] = new u16[width*height];
		clockNumbers[1] = new u16[width*height];
	} else if (type == ImageType::clockColon) {
		clockColonW = (int)width;
		clockColonH = (int)height;

		clockColon[0] = new u16[width*height];
		clockColon[1] = new u16[width*height];
	} else if (type == ImageType::yearNumbers) {
		yearNumbersW = (int)width;
		yearNumbersH = (int)height/10;

		yearNumbers[0] = new u16[width*height];
		yearNumbers[1] = new u16[width*height];
	} else if (type == ImageType::dayNumbers) {
		dayNumbersW = (int)width;
		dayNumbersH = (int)height/10;

		dayNumbers[0] = new u16[width*height];
		dayNumbers[1] = new u16[width*height];
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
				if (bmpImageBuffer[(i*bits)] >= 0x4) {
					bmpImageBuffer[(i*bits)] -= 0x4;
					pixelAdjustInfo |= BIT(0);
				}
				if (bmpImageBuffer[(i*bits)+1] >= 0x4) {
					bmpImageBuffer[(i*bits)+1] -= 0x4;
					pixelAdjustInfo |= BIT(1);
				}
				if (bmpImageBuffer[(i*bits)+2] >= 0x4) {
					bmpImageBuffer[(i*bits)+2] -= 0x4;
					pixelAdjustInfo |= BIT(2);
				}
			}
			u16 color = bmpImageBuffer[(i*bits)+2]>>3 | (bmpImageBuffer[(i*bits)+1]>>3)<<5 | (bmpImageBuffer[i*bits]>>3)<<10 | BIT(15);
			if (colorTable && ((type < ImageType::folderUp) || (color != (0 | BIT(15))))) {
				color = colorTable[color];
			}
			if (type == ImageType::dayNumbers) {
				dayNumbers[0][(y*width)+x] = color;
			} else if (type == ImageType::yearNumbers) {
				yearNumbers[0][(y*width)+x] = color;
			} else if (type == ImageType::clockColon) {
				clockColon[0][(y*width)+x] = color;
			} else if (type == ImageType::clockNumbers) {
				clockNumbers[0][(y*width)+x] = color;
			} else if (type == ImageType::folderUp) {
				folderUpIcon[0][(y*width)+x] = color;
			} else if (type == ImageType::top) {
				topImage[0][(xPos+x+(y*256))+(yPos*256)] = color;
			} else {
				bottomImage[0][(xPos+x+(y*256))+(yPos*256)] = color;
			}
			if (alternatePixel) {
				if (pixelAdjustInfo & BIT(0)) {
					bmpImageBuffer[(i*bits)] += 0x4;
				}
				if (pixelAdjustInfo & BIT(1)) {
					bmpImageBuffer[(i*bits)+1] += 0x4;
				}
				if (pixelAdjustInfo & BIT(2)) {
					bmpImageBuffer[(i*bits)+2] += 0x4;
				}
			} else {
				if (bmpImageBuffer[(i*bits)] >= 0x4) {
					bmpImageBuffer[(i*bits)] -= 0x4;
				}
				if (bmpImageBuffer[(i*bits)+1] >= 0x4) {
					bmpImageBuffer[(i*bits)+1] -= 0x4;
				}
				if (bmpImageBuffer[(i*bits)+2] >= 0x4) {
					bmpImageBuffer[(i*bits)+2] -= 0x4;
				}
			}
			color = bmpImageBuffer[(i*bits)+2]>>3 | (bmpImageBuffer[(i*bits)+1]>>3)<<5 | (bmpImageBuffer[i*bits]>>3)<<10 | BIT(15);
			if (colorTable && ((type < ImageType::folderUp) || (color != (0 | BIT(15))))) {
				color = colorTable[color];
			}
			if (type == ImageType::dayNumbers) {
				dayNumbers[1][(y*width)+x] = color;
			} else if (type == ImageType::yearNumbers) {
				yearNumbers[1][(y*width)+x] = color;
			} else if (type == ImageType::clockColon) {
				clockColon[1][(y*width)+x] = color;
			} else if (type == ImageType::clockNumbers) {
				clockNumbers[1][(y*width)+x] = color;
			} else if (type == ImageType::folderUp) {
				folderUpIcon[1][(y*width)+x] = color;
			} else if (type == ImageType::top) {
				topImage[1][(xPos+x+(y*256))+(yPos*256)] = color;
			} else {
				bottomImage[1][(xPos+x+(y*256))+(yPos*256)] = color;
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
		// u16 *bmpImageBuffer = new u16[width * height];
		fread(bmpImageBuffer, 2, width * height, file);
		int renderWidth = 256;
		u16 *dst = ((type == ImageType::top) ? topImage[0] : bottomImage[0]) + ((191 - ((192 - height) / 2)) * 256) + (256 - width) / 2;
		u16 *dst2 = ((type == ImageType::top) ? topImage[1] : bottomImage[1]) + ((191 - ((192 - height) / 2)) * 256) + (256 - width) / 2;
		if (type == ImageType::folderUp) {
			renderWidth = (int)width;
			dst = (folderUpIcon[0] + (height-1) * width);
			dst2 = (folderUpIcon[1] + (height-1) * width);
		} else if (type == ImageType::clockNumbers) {
			renderWidth = (int)width;
			dst = (clockNumbers[0] + (height-1) * width);
			dst2 = (clockNumbers[1] + (height-1) * width);
		} else if (type == ImageType::clockColon) {
			renderWidth = (int)width;
			dst = (clockColon[0] + (height-1) * width);
			dst2 = (clockColon[1] + (height-1) * width);
		} else if (type == ImageType::yearNumbers) {
			renderWidth = (int)width;
			dst = (yearNumbers[0] + (height-1) * width);
			dst2 = (yearNumbers[1] + (height-1) * width);
		} else if (type == ImageType::dayNumbers) {
			renderWidth = (int)width;
			dst = (dayNumbers[0] + (height-1) * width);
			dst2 = (dayNumbers[1] + (height-1) * width);
		}
		u16 *src = bmpImageBuffer;
		for (uint y = 0; y < height; y++, dst -= renderWidth, dst2 -= renderWidth) {
			for (uint x = 0; x < width; x++) {
				u16 val = *(src++);
				if (type >= ImageType::folderUp && val == 0) {
					u16 color = 0 | BIT(15);
					*(dst + x) = color;
					*(dst2 + x) = color;
				} else {
					u16 color = ((val >> (rgb565 ? 11 : 10)) & 0x1F) | ((val >> (rgb565 ? 1 : 0)) & (0x1F << 5)) | (val & 0x1F) << 10 | BIT(15);
					if (colorTable) {
						color = colorTable[color];
					}
					*(dst + x) = color;
					*(dst2 + x) = color;
				}
			}
		}

		// delete[] bmpImageBuffer;
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
			if (colorTable && ((type < ImageType::folderUp) || (pixelBuffer[i] != (0 | BIT(15))))) {
				pixelBuffer[i] = colorTable[pixelBuffer[i]];
			}
		}
		u8 *bmpImageBuffer = new u8[width * height];
		fread(bmpImageBuffer, 1, width * height, file);

		int x = 0;
		int y = height-1;
		for (u32 i = 0; i < width*height; i++) {
			const u16 color = pixelBuffer[bmpImageBuffer[i]];
			if (type == ImageType::dayNumbers) {
				dayNumbers[0][(y*width)+x] = color;
				dayNumbers[1][(y*width)+x] = color;
			} else if (type == ImageType::yearNumbers) {
				yearNumbers[0][(y*width)+x] = color;
				yearNumbers[1][(y*width)+x] = color;
			} else if (type == ImageType::clockColon) {
				clockColon[0][(y*width)+x] = color;
				clockColon[1][(y*width)+x] = color;
			} else if (type == ImageType::clockNumbers) {
				clockNumbers[0][(y*width)+x] = color;
				clockNumbers[1][(y*width)+x] = color;
			} else if (type == ImageType::folderUp) {
				folderUpIcon[0][(y*width)+x] = color;
				folderUpIcon[1][(y*width)+x] = color;
			} else if (type == ImageType::top) {
				topImage[0][(xPos+x+(y*256))+(yPos*256)] = color;
				topImage[1][(xPos+x+(y*256))+(yPos*256)] = color;
			} else {
				bottomImage[0][(xPos+x+(y*256))+(yPos*256)] = color;
				bottomImage[1][(xPos+x+(y*256))+(yPos*256)] = color;
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
			if (colorTable && ((type < ImageType::folderUp) || (monoPixel[i] != (0 | BIT(15))))) {
				monoPixel[i] = colorTable[monoPixel[i]];
			}
		}
		u8 *bmpImageBuffer = new u8[(width * height)/8];
		fread(bmpImageBuffer, 1, (width * height)/8, file);

		int x = 0;
		int y = height-1;
		for (u32 i = 0; i < (width*height)/8; i++) {
			for (int b = 7; b >= 0; b--) {
				const u16 color = monoPixel[(bmpImageBuffer[i] & (BIT(b))) ? 1 : 0];
				if (type == ImageType::dayNumbers) {
					dayNumbers[0][(y*width)+x] = color;
					dayNumbers[1][(y*width)+x] = color;
				} else if (type == ImageType::yearNumbers) {
					yearNumbers[0][(y*width)+x] = color;
					yearNumbers[1][(y*width)+x] = color;
				} else if (type == ImageType::clockColon) {
					clockColon[0][(y*width)+x] = color;
					clockColon[1][(y*width)+x] = color;
				} else if (type == ImageType::clockNumbers) {
					clockNumbers[0][(y*width)+x] = color;
					clockNumbers[1][(y*width)+x] = color;
				} else if (type == ImageType::folderUp) {
					folderUpIcon[0][(y*width)+x] = color;
					folderUpIcon[1][(y*width)+x] = color;
				} else if (type == ImageType::top) {
					topImage[0][(xPos+x+(y*256))+(yPos*256)] = color;
					topImage[1][(xPos+x+(y*256))+(yPos*256)] = color;
				} else {
					bottomImage[0][(xPos+x+(y*256))+(yPos*256)] = color;
					bottomImage[1][(xPos+x+(y*256))+(yPos*256)] = color;
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

static void loadPng(const bool top, const std::string filename) {
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
			if (image[(i*4)] >= 0x4) {
				image[(i*4)] -= 0x4;
				pixelAdjustInfo |= BIT(0);
			}
			if (image[(i*4)+1] >= 0x4) {
				image[(i*4)+1] -= 0x4;
				pixelAdjustInfo |= BIT(1);
			}
			if (image[(i*4)+2] >= 0x4) {
				image[(i*4)+2] -= 0x4;
				pixelAdjustInfo |= BIT(2);
			}
		}
		u16 res = 0;
		if (image[(i*4)+3] > 0) {
			u16 color = image[i*4]>>3 | (image[(i*4)+1]>>3)<<5 | (image[(i*4)+2]>>3)<<10 | BIT(15);
			if (colorTable) {
				color = colorTable[color];
			}
			res = alphablend(color, colorTable ? colorTable[0] : 0, image[(i*4)+3]);
		}
		if (top) {
			topImage[0][(xPos+x+(y*256))+(yPos*256)] = res;
		} else {
			bottomImage[0][(xPos+x+(y*256))+(yPos*256)] = res;
		}
		if (alternatePixel) {
			if (pixelAdjustInfo & BIT(0)) {
				image[(i*4)] += 0x4;
			}
			if (pixelAdjustInfo & BIT(1)) {
				image[(i*4)+1] += 0x4;
			}
			if (pixelAdjustInfo & BIT(2)) {
				image[(i*4)+2] += 0x4;
			}
		} else {
			if (image[(i*4)] >= 0x4) {
				image[(i*4)] -= 0x4;
			}
			if (image[(i*4)+1] >= 0x4) {
				image[(i*4)+1] -= 0x4;
			}
			if (image[(i*4)+2] >= 0x4) {
				image[(i*4)+2] -= 0x4;
			}
		}
		res = 0;
		if (image[(i*4)+3] > 0) {
			u16 color = image[i*4]>>3 | (image[(i*4)+1]>>3)<<5 | (image[(i*4)+2]>>3)<<10 | BIT(15);
			if (colorTable) {
				color = colorTable[color];
			}
			res = alphablend(color, colorTable ? colorTable[0] : 0, image[(i*4)+3]);
		}
		if (top) {
			topImage[1][(xPos+x+(y*256))+(yPos*256)] = res;
		} else {
			bottomImage[1][(xPos+x+(y*256))+(yPos*256)] = res;
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
	glBegin2D();
	{
		if (fadeType) {
			screenBrightness--;
			if (screenBrightness < 0) screenBrightness = 0;
		} else {
			screenBrightness++;
			if (screenBrightness > 31) screenBrightness = 31;
		}
		if (renderFrame) {
			if (ms().macroMode) {
				SetBrightness(0, lcdSwapped ? screenBrightness : 31);
				SetBrightness(1, !lcdSwapped ? screenBrightness : 31);
			} else {
				if (controlBottomBright) SetBrightness(0, screenBrightness);
				if (controlTopBright) SetBrightness(1, screenBrightness);
			}
		}

		// glColor(RGB15(31, 31-(3*blfLevel), 31-(6*blfLevel)));
		glColor(RGB15(31, 31, 31));

		if (displayIcons && iconsToDisplay > 0) {
			for (int i = 0; i < iconsToDisplay; i++) {
				/* if (cursorPosOnScreen == i) {
					glBoxFilled(2, 19+(i*38), 253, 19+37+(i*38), selectionBarColor1); // Draw selection bar
				} */
				if ((i == cursorPosOnScreen) && (iconScale > 0)) {
					if (isDirectory[i] && !customIcon[i]) drawIconFolder(5-iconShift, 22+(i*38)-iconShift, (1 << 12)+iconScale);
					else drawIcon(i, 5-iconShift, 22+(i*38)-iconShift, (1 << 12)+iconScale);
				} else {
					if (isDirectory[i] && !customIcon[i]) drawIconFolder(5, 22+(i*38), 0);
					else drawIcon(i, 5, 22+(i*38), 0);
				}
				// if (bnrWirelessIcon > 0) glSprite(24, 12, GL_FLIP_NONE, &wirelessIcons[(bnrWirelessIcon-1) & 31]);
				// Playback animated icons
				if (bnriconisDSi[i]) {
					playBannerSequence(i);
				}
			}
			if (iconScaleEnabled) {
				if (!iconScaleDelay) {
					if (iconScaleLarge) {
						iconScale += 110;
						if (iconScale == 110) {
							iconShift = 1;
						} else if (iconScale == 330) {
							iconShift = 2;
						} else if (iconScale == 550) {
							iconShift = 3;
						} else if (iconScale == 660) {
							iconScaleLarge = false;
						}
					} else {
						iconScale -= 110;
						if (iconScale == 330) {
							iconShift = 2;
						} else if (iconScale == 110) {
							iconShift = 1;
						} else if (iconScale == 0) {
							iconShift = 0;
							iconScaleLarge = true;
						}
					}
				}
				iconScaleDelay = !iconScaleDelay;
			} else if (ms().ak_zoomIcons) {
				iconScaleWait++;
				if (iconScaleWait == 60) {
					iconScaleWait = 0;
					iconScaleEnabled = true;
				}
			}
		}
		if (showdialogbox) {
			glBoxFilled(15, 71, 241, 121+(dialogboxHeight*12), formFrameColor);
			// glBoxFilledGradient(16, 72, 240, 86, windowColorTop, windowColorBottom, windowColorBottom, windowColorTop);
			glBoxFilled(16, 88, 240, 120+(dialogboxHeight*12), formBodyColor);
		}
	}
	glEnd2D();
	GFX_FLUSH = 0;

	// Blink colon once per second
	if (colonTimer >= 60) {
		colonTimer = 0;
		showColon = !showColon;
	}

	colonTimer++;

	if (doubleBuffer) {
		dmaCopyHalfWordsAsynch(0, topImageWithText[secondBuffer], BG_GFX_SUB, 0x18000);
		dmaCopyHalfWordsAsynch(1, bottomImageWithBar[secondBuffer], BG_GFX, 0x18000);
		secondBuffer = !secondBuffer;
	}

	frameDelay = 0;
	frameDelayEven = !frameDelayEven;
	renderFrame = false;
}

void graphicsInit()
{	
	*(u16*)(0x0400006C) |= BIT(14);
	*(u16*)(0x0400006C) &= BIT(15);
	SetBrightness(0, 31);
	SetBrightness(1, 31);

	if (ms().colorMode != "Default") {
		char colorTablePath[256];
		sprintf(colorTablePath, "%s:/_nds/colorLut/%s.lut", (sys().isRunFromSD() ? "sd" : "fat"), ms().colorMode.c_str());

		if (getFileSize(colorTablePath) == 0x20000) {
			colorTable = new u16[0x20000/sizeof(u16)];

			FILE* file = fopen(colorTablePath, "rb");
			fread(colorTable, 1, 0x20000, file);
			fclose(file);
		}
	}

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

	lcdMainOnBottom();
	lcdSwapped = true;

	dmaFillWords(0xFFFFFFFF, BG_GFX, 0x18000);
	dmaFillWords(0xFFFFFFFF, BG_GFX_SUB, 0x18000);
	SetBrightness(0, 0);
	SetBrightness(1, 0);
}

void graphicsLoad()
{	
	if (isDSiMode()) {
		loadSdRemovedImage();
	}

	std::string themePath = std::string(sys().isRunFromSD() ? "sd:" : "fat:") + "/_nds/TWiLightMenu/akmenu/themes/" + ms().ak_theme;
	{
		std::string pathTop;
		if (access((themePath + "/upper_screen.bmp").c_str(), F_OK) == 0) {
			pathTop = themePath + "/upper_screen.bmp";
		} else if (access((themePath + "/upper_screen.png").c_str(), F_OK) == 0) {
			pathTop = themePath + "/upper_screen.png";
		} else {
			pathTop = "nitro:/themes/zelda/upper_screen.bmp";
		}

		std::string pathBottom;
		if (access((themePath + "/lower_screen.bmp").c_str(), F_OK) == 0) {
			pathBottom = themePath + "/lower_screen.bmp";
		} else if (access((themePath + "/lower_screen.png").c_str(), F_OK) == 0) {
			pathBottom = themePath + "/lower_screen.png";
		} else {
			pathBottom = "nitro:/themes/zelda/lower_screen.bmp";
		}

		if (extension(pathTop.c_str(), {".png"})) {
			loadPng(true, pathTop);
		} else {
			loadBmp(ImageType::top, pathTop.c_str());
		}
		if (extension(pathBottom.c_str(), {".png"})) {
			loadPng(false, pathBottom);
		} else {
			loadBmp(ImageType::bottom, pathBottom.c_str());
		}
	}

	dmaCopyHalfWordsAsynch(0, topImage[0], topImageWithText[0], 0x18000);
	dmaCopyHalfWordsAsynch(1, topImage[1], topImageWithText[1], 0x18000);
	dmaCopyHalfWordsAsynch(2, bottomImage[0], bottomImageWithBar[0], 0x18000);
	dmaCopyHalfWordsAsynch(3, bottomImage[1], bottomImageWithBar[1], 0x18000);

	if (ms().showDirectories) {
		std::string pathFolderUp;
		if (access((themePath + "/folder_up.bmp").c_str(), F_OK) == 0) {
			pathFolderUp = themePath + "/folder_up.bmp";
		} else {
			pathFolderUp = "nitro:/themes/zelda/folder_up.bmp";
		}

		loadBmp(ImageType::folderUp, pathFolderUp.c_str());
	}

	extern std::string iniPath;
	extern std::string customIniPath;
	iniPath = themePath + "/uisettings.ini";
	customIniPath = themePath + "/custom.ini";
	if (access(iniPath.c_str(), F_OK) != 0) {
		iniPath = "nitro:/themes/zelda/uisettings.ini";
		customIniPath = "nitro:/themes/zelda/custom.ini";
	}

	{
		CIniFile ini( iniPath.c_str() );
		formFrameColor = ini.GetInt("global settings", "formFrameColor", formFrameColor);
		formBodyColor = ini.GetInt("global settings", "formBodyColor", formBodyColor);

		selectionBarColor1 = ini.GetInt("main list", "selectionBarColor1", RGB15(16, 20, 24)) | BIT(15);
		selectionBarColor2 = ini.GetInt("main list", "selectionBarColor2", RGB15(20, 25, 0)) | BIT(15);
		selectionBarOpacity = ini.GetInt("main list", "selectionBarOpacity", 100);
		if (colorTable) {
			selectionBarColor1 = colorTable[selectionBarColor1];
		}

		if (!ms().macroMode && ini.GetInt("global settings", "showCalendar", true)) {
			if (ini.GetInt("big clock", "show", 0)) {
				bigClockX = ini.GetInt("big clock", "x", bigClockX);
				bigClockY = ini.GetInt("big clock", "y", bigClockY);

				std::string pathClockNumbers;
				if (access((themePath + "/calendar/clock_numbers.bmp").c_str(), F_OK) == 0) {
					pathClockNumbers = themePath + "/calendar/clock_numbers.bmp";
				} else {
					pathClockNumbers = "nitro:/themes/zelda/calendar/clock_numbers.bmp";
				}

				loadBmp(ImageType::clockNumbers, pathClockNumbers.c_str());

				if (access((themePath + "/calendar/clock_colon.bmp").c_str(), F_OK) == 0) {
					pathClockNumbers = themePath + "/calendar/clock_colon.bmp";
				} else {
					pathClockNumbers = "nitro:/themes/zelda/calendar/clock_colon.bmp";
				}

				loadBmp(ImageType::clockColon, pathClockNumbers.c_str());
			}

			showYear = ini.GetInt("calendar year", "show", showYear);
			showMonth = ini.GetInt("calendar month", "show", showMonth);
			showDayX = ini.GetInt("calendar dayx", "show", showDayX);

			if (showYear) {
				yearX = ini.GetInt("calendar year", "x", yearX);
				yearY = ini.GetInt("calendar year", "y", yearY);
			}
			if (showMonth) {
				monthX = ini.GetInt("calendar month", "x", monthX);
				monthY = ini.GetInt("calendar month", "y", monthY);
			}
			if (showDayX) {
				dayxX = ini.GetInt("calendar dayx", "x", dayxX);
				dayxY = ini.GetInt("calendar dayx", "y", dayxY);
			}

			if (showYear || showMonth || showDayX) {
				std::string pathYearNumbers;
				if (access((themePath + "/calendar/year_numbers.bmp").c_str(), F_OK) == 0) {
					pathYearNumbers = themePath + "/calendar/year_numbers.bmp";
				} else {
					pathYearNumbers = "nitro:/themes/zelda/calendar/year_numbers.bmp";
				}

				loadBmp(ImageType::yearNumbers, pathYearNumbers.c_str());
			}

			if (ini.GetInt("calendar day", "show", 0)) {
				dayPositionX = ini.GetInt("calendar day", "x", dayPositionX);
				dayPositionY = ini.GetInt("calendar day", "y", dayPositionY);
				daySizeX = ini.GetInt("calendar day", "dw", daySizeX);
				daySizeY = ini.GetInt("calendar day", "dh", daySizeY);
				dayHighlightColor = ini.GetInt("calendar day", "highlightColor", dayHighlightColor) | BIT(15);

				std::string pathDayNumbers;
				if (access((themePath + "/calendar/day_numbers.bmp").c_str(), F_OK) == 0) {
					pathDayNumbers = themePath + "/calendar/day_numbers.bmp";
				} else {
					pathDayNumbers = "nitro:/themes/zelda/calendar/day_numbers.bmp";
				}

				loadBmp(ImageType::dayNumbers, pathDayNumbers.c_str());
			}
		}
	}

	// Initialize the bottom background
	// bottomBg = bgInit(2, BgType_ExRotation, BgSize_ER_256x256, 0,1);

	startBorderColor = RGB15(colorRvalue/8, colorGvalue/8, colorBvalue/8) | BIT(15); // Bit 15 is needed for the color to display on the top screen
	// windowColorTop = RGB15(0, 0, 31);
	// windowColorBottom = RGB15(0, 0, 15);
	if (colorTable) {
		startBorderColor = colorTable[startBorderColor];
		// windowColorTop = colorTable[windowColorTop];
		// windowColorBottom = colorTable[windowColorBottom];
	}

	u16* newPalette = (u16*)wirelessiconsPal;
	if (colorTable) {
		for (int i2 = 0; i2 < 16; i2++) {
			*(newPalette+i2) = colorTable[*(newPalette+i2)];
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

	while (dmaBusy(0) || dmaBusy(1) || dmaBusy(2) || dmaBusy(3)) swiDelay(100);

	irqSet(IRQ_VBLANK, vBlankHandler);
	irqEnable(IRQ_VBLANK);
	irqSet(IRQ_VCOUNT, frameRateHandler);
	irqEnable(IRQ_VCOUNT);
}
