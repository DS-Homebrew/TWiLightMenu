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
#include "myDSiMode.h"
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
bool updateFrame = true;

static int colonTimer = 0;
static bool showColon = true;

int vblankRefreshCounter = 0;

// bool stopDSiAnim = false;
// bool stopDSiAnimNotif = false;

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

static u16* startButton[2] = {NULL};
static int startButtonW = 0;
static int startButtonH = 0;

static u16* brightnessBtn[2] = {NULL};
static int brightnessBtnW = 0;
static int brightnessBtnH = 0;

static u16* folderUpIcon[2] = {NULL};
static int folderUpIconW = 0;
static int folderUpIconH = 0;

static u16* cardIconBlue[2] = {NULL};
static int cardIconBlueX = 238;
static int cardIconBlueY = 172;
static int cardIconBlueW = 0;
static int cardIconBlueH = 0;

static u16* clockNumbers[2] = {NULL};
static u16* clockNumbers2[2] = {NULL};
static u16* clockColon[2] = {NULL};
static u16* clockColon2[2] = {NULL};
static u16* yearNumbers[2] = {NULL};
static u16* yearNumbers2[2] = {NULL};
static u16* dayNumbers[2] = {NULL};
static u16* dayNumbers2[2] = {NULL};
static u16* weekdayText[2] = {NULL};
static u16* weekdayText2[2] = {NULL};

static int bigClockX = 8;
static int bigClockY = 80;
static int smallClockX = 8;
static int smallClockY = 80;
static int clockNumbersW = 0;
static int clockNumbersH = 0;
static int clockNumbers2W = 0;
static int clockNumbers2H = 0;
static int clockColonW = 0;
static int clockColonH = 0;
static int clockColon2W = 0;
static int clockColon2H = 0;
static int yearNumbersW = 0;
static int yearNumbersH = 0;
static int yearNumbers2W = 0;
static int yearNumbers2H = 0;
static int dayNumbersW = 0;
static int dayNumbersH = 0;
static int dayNumbers2W = 0;
static int dayNumbers2H = 0;
static int weekdayTextW = 0;
static int weekdayTextH = 0;
static int weekdayText2W = 0;
static int weekdayText2H = 0;

static int yearX = 52;
static int yearY = 28;
static int year2X = 52;
static int year2Y = 28;
static bool showYear = false;
static bool showYear2 = false;

static int monthX = 12;
static int monthY = 28;
static int month2X = 12;
static int month2Y = 28;
static bool showMonth = false;
static bool showMonth2 = false;

static int dayxX = 02;
static int dayxY = 28;
static int dayx2X = 02;
static int dayx2Y = 28;
static bool showDayX = false;
static bool showDayX2 = false;

static int dayPositionX = 134;
static int dayPositionY = 34;
static int dayPosition2X = 134;
static int dayPosition2Y = 34;
static int daySizeX = 16;
static int daySizeY = 14;
static int daySize2X = 16;
static int daySize2Y = 14;
static u16 dayHighlightColor = 0xfc00;
static u16 dayHighlightColor2 = 0xfc00;

static int weekdayX = 52;
static int weekdayY = 28;
static int weekday2X = 52;
static int weekday2Y = 28;

u16* colorTable = NULL;

bool displayIcons = false;
int iconsToDisplay = 0;
int smallIconsToDisplay = 0;
static bool iconScaleEnabled = false;
static int iconScaleWait = 0;
static int iconScale = 0;
static bool iconScaleLarge = true;
static int iconScaleDelay = 0;
static int iconShift = 0;

static u16 formFrameColor = RGB15(23,25,4);
static u16 formBodyColor = RGB15(30,29,22);

u16 startBorderColor = 0;
// static u16 windowColorTop = 0;
// static u16 windowColorBottom = 0;
static u16 selectionBarColor1 = 0x5c00;
static u16 selectionBarColor2 = 0x2d60;
static u8 selectionBarOpacity = 100;
static u16* selectionBarBg[2] = {NULL};
static int selectionBarBgW = 0;
static int selectionBarBgH = 0;

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

void resetIconScale(void) {
	iconScaleEnabled = false;
	iconScaleWait = 0;
	iconScale = 0;
	iconScaleLarge = true;
	iconScaleDelay = 0;
	iconShift = 0;
	while (updateFrame) {
		swiWaitForVBlank();
	}
	updateFrame = true;
}

ITCM_CODE void updateSelectionBar(void) {
	static int prevCurPos = 20;
	static int prevViewMode = 4;
	if (prevCurPos == cursorPosOnScreen && prevViewMode == ms().ak_viewMode) {
		return;
	}
	swiWaitForVBlank();
	if (prevCurPos != 20) {
		int h = (prevViewMode != TWLSettings::EViewList) ? 38 : 15;
		if (prevViewMode == TWLSettings::EViewSmallIcon) {
			h = 18;
		}
		// const int hl = h-1;
		const int hl = h;
		for (int y = 19+(prevCurPos*h); y <= 19+hl+(prevCurPos*h); y++) {
			for (int x = 2; x < 256; x++) {
				bottomImageWithBar[0][(y*256)+x] = bottomImage[0][(y*256)+x];
				bottomImageWithBar[1][(y*256)+x] = bottomImage[1][(y*256)+x];
			}
		}
	}

	int h = (ms().ak_viewMode != TWLSettings::EViewList) ? 38 : 15;
	if (ms().ak_viewMode == TWLSettings::EViewSmallIcon) {
		h = 18;
	}
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

	if (selectionBarBg[0] && (ms().ak_viewMode != TWLSettings::EViewList && ms().ak_viewMode != TWLSettings::EViewSmallIcon)) {
		int src = 0;

		for (int y = 19+(cursorPosOnScreen*h); y < 19+(cursorPosOnScreen*h)+selectionBarBgH; y++) {
			for (int x = 2; x < 2+selectionBarBgW; x++) {
				if ((selectionBarBg[0][src] != (0 | BIT(15))) && (x < 256)) {
					bottomImageWithBar[0][(y*256)+x] = selectionBarBg[0][src];
					bottomImageWithBar[1][(y*256)+x] = selectionBarBg[1][src];
				}
				src++;
			}
		}
	}

	prevCurPos = cursorPosOnScreen;
	prevViewMode = ms().ak_viewMode;
}

ITCM_CODE void displayStartButton(const int x, const int y) {
	if (!startButton[0]) return;

	int src = 0;

	for (int y2 = y; y2 < y+startButtonH; y2++) {
		for (int x2 = x; x2 < x+startButtonW; x2++) {
			if ((startButton[0][src] != (0 | BIT(15))) && x2 >= 0 && x2 < 256 && y2 >= 0 && y2 < 192) {
				bottomImageWithBar[0][(y2*256)+x2] = startButton[0][src];
				bottomImageWithBar[1][(y2*256)+x2] = startButton[1][src];
			}
			src++;
		}
	}

	src = 0;

	// Save to base bottom screen image
	for (int y2 = y; y2 < y+startButtonH; y2++) {
		for (int x2 = x; x2 < x+startButtonW; x2++) {
			if ((startButton[0][src] != (0 | BIT(15))) && x2 >= 0 && x2 < 256 && y2 >= 0 && y2 < 192) {
				bottomImage[0][(y2*256)+x2] = startButton[0][src];
				bottomImage[1][(y2*256)+x2] = startButton[1][src];
			}
			src++;
		}
	}

	delete[] startButton[0];
	delete[] startButton[1];
}

ITCM_CODE void displayBrightnessBtn(const int x, const int y) {
	if (!brightnessBtn[0]) return;

	int src = 0;

	for (int y2 = y; y2 < y+brightnessBtnH; y2++) {
		for (int x2 = x; x2 < x+brightnessBtnW; x2++) {
			if ((brightnessBtn[0][src] != (0 | BIT(15))) && x2 >= 0 && x2 < 256 && y2 >= 0 && y2 < 192) {
				bottomImageWithBar[0][(y2*256)+x2] = brightnessBtn[0][src];
				bottomImageWithBar[1][(y2*256)+x2] = brightnessBtn[1][src];
			}
			src++;
		}
	}

	delete[] brightnessBtn[0];
	delete[] brightnessBtn[1];
}

ITCM_CODE void displayFolderUp(const int x, const int y) {
	if (!folderUpIcon[0]) return;

	int src = 0;

	for (int y2 = y; y2 < y+folderUpIconH; y2++) {
		for (int x2 = x; x2 < x+folderUpIconW; x2++) {
			if ((folderUpIcon[0][src] != (0 | BIT(15))) && x2 >= 0 && x2 < 256 && y2 >= 0 && y2 < 192) {
				bottomImageWithBar[0][(y2*256)+x2] = folderUpIcon[0][src];
				bottomImageWithBar[1][(y2*256)+x2] = folderUpIcon[1][src];
			}
			src++;
		}
	}

	delete[] folderUpIcon[0];
	delete[] folderUpIcon[1];
}

ITCM_CODE void displayDiskIcon(const bool show) {
	if (!cardIconBlue[0]) return;

	static bool prev = false;
	if (prev == show) return;
	prev = show;

	int src = 0;

	for (int y2 = cardIconBlueY; y2 < cardIconBlueY+cardIconBlueH; y2++) {
		for (int x2 = cardIconBlueX; x2 < cardIconBlueX+cardIconBlueW; x2++) {
			if ((cardIconBlue[0][src] != (0 | BIT(15))) && x2 >= 0 && x2 < 256 && y2 >= 0 && y2 < 192) {
				if (show) {
					bottomImageWithBar[0][(y2*256)+x2] = cardIconBlue[0][src];
					bottomImageWithBar[1][(y2*256)+x2] = cardIconBlue[1][src];
				} else {
					bottomImageWithBar[0][(y2*256)+x2] = bottomImage[0][(y2*256)+x2];
					bottomImageWithBar[1][(y2*256)+x2] = bottomImage[1][(y2*256)+x2];
				}
			}
			src++;
		}
	}
}

static std::string loadedTime;

ITCM_CODE void drawTime(void) {
	if (!clockNumbers[0] && !clockColon[0] && !clockNumbers2[0] && !clockColon2[0]) return;

	// Load time
	std::string currentTime = retTime();
	if (currentTime[0] == ' ')
		currentTime[0] = '0';
	currentTime[2] = showColon ? ':' : ' ';

	if (currentTime == loadedTime)
		return;

	loadedTime = currentTime;

	if (clockNumbers[0] && clockColon[0]) {
		// Draw big clock
		for (int i = 0; i < 5; i++) {
			const int number = (int)currentTime[i]-0x30;
			const bool colon = (number == 10 || number == -0x10);

			const int x = bigClockX + (clockNumbersW * i);
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
		}
	}

	if (clockNumbers2[0] && clockColon2[0]) {
		// Draw small clock
		for (int i = 0; i < 5; i++) {
			const int number = (int)currentTime[i]-0x30;
			const bool colon = (number == 10 || number == -0x10);

			const int x = smallClockX + (clockNumbers2W * i);
			const int y = smallClockY;

			int src = colon ? 0 : ((clockNumbers2W * clockNumbers2H) * number);

			if (colon) {
				for (int y2 = y; y2 < y+clockColon2H; y2++) {
					for (int x2 = x; x2 < x+clockColon2W; x2++) {
						if ((clockColon2[0][src] == (0 | BIT(15))) || (number == -0x10)) {
							topImageWithText[0][(y2*256)+x2] = topImage[0][(y2*256)+x2];
							topImageWithText[1][(y2*256)+x2] = topImage[1][(y2*256)+x2];
						} else {
							topImageWithText[0][(y2*256)+x2] = clockColon2[0][src];
							topImageWithText[1][(y2*256)+x2] = clockColon2[1][src];
						}
						src++;
					}
				}
			} else {
				for (int y2 = y; y2 < y+clockNumbers2H; y2++) {
					for (int x2 = x; x2 < x+clockNumbers2W; x2++) {
						if (clockNumbers2[0][src] == (0 | BIT(15))) {
							topImageWithText[0][(y2*256)+x2] = topImage[0][(y2*256)+x2];
							topImageWithText[1][(y2*256)+x2] = topImage[1][(y2*256)+x2];
						} else {
							topImageWithText[0][(y2*256)+x2] = clockNumbers2[0][src];
							topImageWithText[1][(y2*256)+x2] = clockNumbers2[1][src];
						}
						src++;
					}
				}
			}
		}
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

ITCM_CODE static void drawYearNumber2String(const std::string string, const int len, int x, const int y) {
	for (int i = 0; i < len; i++) {
		const int number = (int)string[i]-0x30;

		int src = (yearNumbers2W * yearNumbers2H) * number;

		for (int y2 = y; y2 < y+yearNumbers2H; y2++) {
			for (int x2 = x; x2 < x+yearNumbers2W; x2++) {
				if (yearNumbers2[0][src] == (0 | BIT(15))) {
					topImageWithText[0][(y2*256)+x2] = topImage[0][(y2*256)+x2];
					topImageWithText[1][(y2*256)+x2] = topImage[1][(y2*256)+x2];
				} else {
					topImageWithText[0][(y2*256)+x2] = yearNumbers2[0][src];
					topImageWithText[1][(y2*256)+x2] = yearNumbers2[1][src];
				}
				src++;
			}
		}
		x += yearNumbers2W;
	}
}

static std::string loadedYear;

ITCM_CODE void drawYear(void) {
	if ((!yearNumbers[0] && !yearNumbers2[0]) || (!showYear && !showYear2)) return;

	// Load year
	std::string currentYear = retYear();

	if (currentYear == loadedYear)
		return;

	loadedYear = currentYear;

	if (yearNumbers[0] && showYear) {
		drawYearNumberString(currentYear, 4, yearX, yearY);
	}
	if (yearNumbers2[0] && showYear2) {
		drawYearNumber2String(currentYear, 4, year2X, year2Y);
	}
}

static std::string loadedMonth;

ITCM_CODE void drawMonth(void) {
	if ((!yearNumbers[0] && !yearNumbers2[0]) || (!showMonth && !showMonth2)) return;

	// Load month
	std::string currentMonth = retMonth();

	if (currentMonth == loadedMonth)
		return;

	loadedMonth = currentMonth;

	if (yearNumbers[0] && showMonth) {
		drawYearNumberString(currentMonth, 2, monthX, monthY);
	}
	if (yearNumbers2[0] && showMonth2) {
		drawYearNumber2String(currentMonth, 2, month2X, month2Y);
	}
}

static std::string loadedDayX;

ITCM_CODE void drawDayX(void) {
	if ((!yearNumbers[0] && !yearNumbers2[0]) || (!showDayX && !showDayX2)) return;

	// Load day
	std::string currentDayX = retDay();

	if (currentDayX == loadedDayX)
		return;

	loadedDayX = currentDayX;

	if (yearNumbers[0] && showDayX) {
		drawYearNumberString(currentDayX, 2, dayxX, dayxY);
	}
	if (yearNumbers2[0] && showDayX2) {
		drawYearNumber2String(currentDayX, 2, dayx2X, dayx2Y);
	}
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
static u8 oldDays2X[32] = {0};
static u8 oldDays2Y[32] = {0};
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

static void clearDayNumber2(u8 day)
{
	if (day > 31)
		return;

	const u8 x = oldDays2X[day];
	const u8 y = oldDays2Y[day];

	for (int y2 = y; y2 < y+daySize2Y-1; y2++) {
		for (int x2 = x; x2 < x+daySize2X-1; x2++) {
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

static void drawDayNumber2(u8 day)
{
	if (day > 31)
		return;

	const u8 weekDayOfDay = (((day - 1) % 7) + weekDayOfFirstDay()) % 7;
	const u8 x = weekDayOfDay *daySize2X + dayPosition2X;
	const u8 y = ((day - 1 + weekDayOfFirstDay()) / 7 * daySize2Y) + dayPosition2Y;
	const u8 firstNumber = day / 10;
	const u8 secondNumber = day % 10;

	const int x3 = x - (daySize2X / 2 - dayNumbers2W);
	const int y3 = y - (daySize2Y - dayNumbers2H) / 2;
	if (day == dateDay()) {
		for (int y2 = y3; y2 < y3+daySize2Y-1; y2++) {
			for (int x2 = x3; x2 < x3+daySize2X-1; x2++) {
				topImageWithText[0][(y2*256)+x2] = dayHighlightColor2;
				topImageWithText[1][(y2*256)+x2] = dayHighlightColor2;
			}
		}
	} else {
		for (int y2 = y; y2 < y+daySize2Y-1; y2++) {
			for (int x2 = x; x2 < x+daySize2X-1; x2++) {
				topImageWithText[0][(y2*256)+x2] = topImage[0][(y2*256)+x2];
				topImageWithText[1][(y2*256)+x2] = topImage[1][(y2*256)+x2];
			}
		}
	}

	for (int i = 0; i < 2; i++) {
		int src = (dayNumbers2W * dayNumbers2H) * ((i == 1) ? secondNumber : firstNumber);

		const int x3 = (i == 1) ? (x+dayNumbers2W) : x;
		for (int y2 = y; y2 < y+dayNumbers2H; y2++) {
			for (int x2 = x3; x2 < x3+dayNumbers2W; x2++) {
				if (dayNumbers2[0][src] != (0 | BIT(15))) {
					topImageWithText[0][(y2*256)+x2] = dayNumbers2[0][src];
					topImageWithText[1][(y2*256)+x2] = dayNumbers2[1][src];
				}
				src++;
			}
		}
	}

	oldDays2X[day] = x;
	oldDays2Y[day] = y;
}

static std::string loadedDay;

void drawDay(void) {
	if (!dayNumbers[0] && !dayNumbers2[0]) return;

	// Load day
	std::string currentDay = retDay();

	if (currentDay == loadedDay)
		return;

	loadedDay = currentDay;

	if (dayNumbers[0]) {
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
	}
	if (dayNumbers2[0]) {
		if (oldDaysSaved && (dateMonth() != oldMonth || dateYear() != oldYear)) {
			for (u8 i = 1; i <= oldDaysOfMonth; ++i)
			{
				clearDayNumber2(i);
			}
		}

		for (u8 i = 1; i <= daysOfMonth(); ++i)
		{
			drawDayNumber2(i);
		}
	}

	oldMonth = dateMonth();
	oldYear = dateYear();
	oldDaysOfMonth = daysOfMonth();
	oldDaysSaved = true;
}

static u8 loadedWeekday = 0xFF;

ITCM_CODE void drawWeekday(void) {
	if (!weekdayText[0] && !weekdayText2[0]) return;

	u8 currentWeekday = dateWeekday();

	if (currentWeekday == loadedWeekday)
		return;

	loadedWeekday = currentWeekday;

	if (weekdayText[0]) {
		int src = (weekdayTextW * weekdayTextH) * currentWeekday;

		for (int y2 = weekdayY; y2 < weekdayY+weekdayTextH; y2++) {
			for (int x2 = weekdayX; x2 < weekdayX+weekdayTextW; x2++) {
				if (weekdayText[0][src] == (0 | BIT(15))) {
					bottomImageWithBar[0][(y2*256)+x2] = bottomImage[0][(y2*256)+x2];
					bottomImageWithBar[1][(y2*256)+x2] = bottomImage[1][(y2*256)+x2];
				} else {
					bottomImageWithBar[0][(y2*256)+x2] = weekdayText[0][src];
					bottomImageWithBar[1][(y2*256)+x2] = weekdayText[1][src];
				}
				src++;
			}
		}
	}
	if (weekdayText2[0]) {
		int src = (weekdayText2W * weekdayText2H) * currentWeekday;

		for (int y2 = weekday2Y; y2 < weekday2Y+weekdayText2H; y2++) {
			for (int x2 = weekday2X; x2 < weekday2X+weekdayText2W; x2++) {
				if (weekdayText2[0][src] == (0 | BIT(15))) {
					bottomImageWithBar[0][(y2*256)+x2] = bottomImage[0][(y2*256)+x2];
					bottomImageWithBar[1][(y2*256)+x2] = bottomImage[1][(y2*256)+x2];
				} else {
					bottomImageWithBar[0][(y2*256)+x2] = weekdayText2[0][src];
					bottomImageWithBar[1][(y2*256)+x2] = weekdayText2[1][src];
				}
				src++;
			}
		}
	}
}

enum class ImageType {
	bottom,
	top,
	startButton,
	brightness,
	folderUp,
	cardIconBlue,
	clockNumbers,
	clockNumbers2,
	clockColon,
	clockColon2,
	yearNumbers,
	yearNumbers2,
	dayNumbers,
	dayNumbers2,
	weekdayText,
	weekdayText2,
	selectionBarBg
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
	if (type < ImageType::startButton && width <= 254) {
		// Adjust X position
		for (int i = width; i < 256; i += 2) {
			xPos++;
		}
	}

	int yPos = 0;
	if (type < ImageType::startButton && height <= 190) {
		// Adjust Y position
		for (int i = height; i < 192; i += 2) {
			yPos++;
		}
	}

	if (type == ImageType::startButton) {
		startButtonW = (int)width;
		startButtonH = (int)height;

		startButton[0] = new u16[width*height];
		startButton[1] = new u16[width*height];
	} else if (type == ImageType::brightness) {
		brightnessBtnW = (int)width;
		brightnessBtnH = (int)height;

		brightnessBtn[0] = new u16[width*height];
		brightnessBtn[1] = new u16[width*height];
	} else if (type == ImageType::folderUp) {
		folderUpIconW = (int)width;
		folderUpIconH = (int)height;

		folderUpIcon[0] = new u16[width*height];
		folderUpIcon[1] = new u16[width*height];
	} else if (type == ImageType::cardIconBlue) {
		cardIconBlueW = (int)width;
		cardIconBlueH = (int)height;

		cardIconBlue[0] = new u16[width*height];
		cardIconBlue[1] = new u16[width*height];
	} else if (type == ImageType::clockNumbers) {
		clockNumbersW = (int)width;
		clockNumbersH = (int)height/10;

		clockNumbers[0] = new u16[width*height];
		clockNumbers[1] = new u16[width*height];
	} else if (type == ImageType::clockNumbers2) {
		clockNumbers2W = (int)width;
		clockNumbers2H = (int)height/10;

		clockNumbers2[0] = new u16[width*height];
		clockNumbers2[1] = new u16[width*height];
	} else if (type == ImageType::clockColon) {
		clockColonW = (int)width;
		clockColonH = (int)height;

		clockColon[0] = new u16[width*height];
		clockColon[1] = new u16[width*height];
	} else if (type == ImageType::clockColon2) {
		clockColon2W = (int)width;
		clockColon2H = (int)height;

		clockColon2[0] = new u16[width*height];
		clockColon2[1] = new u16[width*height];
	} else if (type == ImageType::yearNumbers) {
		yearNumbersW = (int)width;
		yearNumbersH = (int)height/10;

		yearNumbers[0] = new u16[width*height];
		yearNumbers[1] = new u16[width*height];
	} else if (type == ImageType::yearNumbers2) {
		yearNumbers2W = (int)width;
		yearNumbers2H = (int)height/10;

		yearNumbers2[0] = new u16[width*height];
		yearNumbers2[1] = new u16[width*height];
	} else if (type == ImageType::dayNumbers) {
		dayNumbersW = (int)width;
		dayNumbersH = (int)height/10;

		dayNumbers[0] = new u16[width*height];
		dayNumbers[1] = new u16[width*height];
	} else if (type == ImageType::dayNumbers2) {
		dayNumbers2W = (int)width;
		dayNumbers2H = (int)height/10;

		dayNumbers2[0] = new u16[width*height];
		dayNumbers2[1] = new u16[width*height];
	} else if (type == ImageType::weekdayText) {
		weekdayTextW = (int)width;
		weekdayTextH = (int)height/7;

		weekdayText[0] = new u16[width*height];
		weekdayText[1] = new u16[width*height];
	} else if (type == ImageType::weekdayText2) {
		weekdayText2W = (int)width;
		weekdayText2H = (int)height/7;

		weekdayText2[0] = new u16[width*height];
		weekdayText2[1] = new u16[width*height];
	} else if (type == ImageType::selectionBarBg) {
		selectionBarBgW = (int)width;
		selectionBarBgH = (int)height;

		selectionBarBg[0] = new u16[width*height];
		selectionBarBg[1] = new u16[width*height];
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
			if (colorTable && ((type < ImageType::startButton) || (color != (0 | BIT(15))))) {
				color = colorTable[color % 0x8000] | BIT(15);
			}
			if (type == ImageType::selectionBarBg) {
				selectionBarBg[0][(y*width)+x] = color;
			} else if (type == ImageType::weekdayText2) {
				weekdayText2[0][(y*width)+x] = color;
			} else if (type == ImageType::weekdayText) {
				weekdayText[0][(y*width)+x] = color;
			} else if (type == ImageType::dayNumbers2) {
				dayNumbers2[0][(y*width)+x] = color;
			} else if (type == ImageType::dayNumbers) {
				dayNumbers[0][(y*width)+x] = color;
			} else if (type == ImageType::yearNumbers2) {
				yearNumbers2[0][(y*width)+x] = color;
			} else if (type == ImageType::yearNumbers) {
				yearNumbers[0][(y*width)+x] = color;
			} else if (type == ImageType::clockColon2) {
				clockColon2[0][(y*width)+x] = color;
			} else if (type == ImageType::clockColon) {
				clockColon[0][(y*width)+x] = color;
			} else if (type == ImageType::clockNumbers2) {
				clockNumbers2[0][(y*width)+x] = color;
			} else if (type == ImageType::clockNumbers) {
				clockNumbers[0][(y*width)+x] = color;
			} else if (type == ImageType::cardIconBlue) {
				cardIconBlue[0][(y*width)+x] = color;
			} else if (type == ImageType::folderUp) {
				folderUpIcon[0][(y*width)+x] = color;
			} else if (type == ImageType::brightness) {
				brightnessBtn[0][(y*width)+x] = color;
			} else if (type == ImageType::startButton) {
				startButton[0][(y*width)+x] = color;
			} else if (type == ImageType::top) {
				topImage[0][(xPos+x+(y*256))+(yPos*256)] = color;
			} else {
				bottomImage[0][(xPos+x+(y*256))+(yPos*256)] = color;
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
			if (colorTable && ((type < ImageType::startButton) || (color != (0 | BIT(15))))) {
				color = colorTable[color % 0x8000] | BIT(15);
			}
			if (type == ImageType::selectionBarBg) {
				selectionBarBg[1][(y*width)+x] = color;
			} else if (type == ImageType::weekdayText2) {
				weekdayText2[1][(y*width)+x] = color;
			} else if (type == ImageType::weekdayText) {
				weekdayText[1][(y*width)+x] = color;
			} else if (type == ImageType::dayNumbers2) {
				dayNumbers2[1][(y*width)+x] = color;
			} else if (type == ImageType::dayNumbers) {
				dayNumbers[1][(y*width)+x] = color;
			} else if (type == ImageType::yearNumbers2) {
				yearNumbers2[1][(y*width)+x] = color;
			} else if (type == ImageType::yearNumbers) {
				yearNumbers[1][(y*width)+x] = color;
			} else if (type == ImageType::clockColon2) {
				clockColon2[1][(y*width)+x] = color;
			} else if (type == ImageType::clockColon) {
				clockColon[1][(y*width)+x] = color;
			} else if (type == ImageType::clockNumbers2) {
				clockNumbers2[1][(y*width)+x] = color;
			} else if (type == ImageType::clockNumbers) {
				clockNumbers[1][(y*width)+x] = color;
			} else if (type == ImageType::cardIconBlue) {
				cardIconBlue[1][(y*width)+x] = color;
			} else if (type == ImageType::folderUp) {
				folderUpIcon[1][(y*width)+x] = color;
			} else if (type == ImageType::brightness) {
				brightnessBtn[1][(y*width)+x] = color;
			} else if (type == ImageType::startButton) {
				startButton[1][(y*width)+x] = color;
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
		if (type == ImageType::startButton) {
			renderWidth = (int)width;
			dst = (startButton[0] + (height-1) * width);
			dst2 = (startButton[1] + (height-1) * width);
		} else if (type == ImageType::brightness) {
			renderWidth = (int)width;
			dst = (brightnessBtn[0] + (height-1) * width);
			dst2 = (brightnessBtn[1] + (height-1) * width);
		} else if (type == ImageType::folderUp) {
			renderWidth = (int)width;
			dst = (folderUpIcon[0] + (height-1) * width);
			dst2 = (folderUpIcon[1] + (height-1) * width);
		} else if (type == ImageType::cardIconBlue) {
			renderWidth = (int)width;
			dst = (cardIconBlue[0] + (height-1) * width);
			dst2 = (cardIconBlue[1] + (height-1) * width);
		} else if (type == ImageType::clockNumbers) {
			renderWidth = (int)width;
			dst = (clockNumbers[0] + (height-1) * width);
			dst2 = (clockNumbers[1] + (height-1) * width);
		} else if (type == ImageType::clockNumbers2) {
			renderWidth = (int)width;
			dst = (clockNumbers2[0] + (height-1) * width);
			dst2 = (clockNumbers2[1] + (height-1) * width);
		} else if (type == ImageType::clockColon) {
			renderWidth = (int)width;
			dst = (clockColon[0] + (height-1) * width);
			dst2 = (clockColon[1] + (height-1) * width);
		} else if (type == ImageType::clockColon2) {
			renderWidth = (int)width;
			dst = (clockColon2[0] + (height-1) * width);
			dst2 = (clockColon2[1] + (height-1) * width);
		} else if (type == ImageType::yearNumbers) {
			renderWidth = (int)width;
			dst = (yearNumbers[0] + (height-1) * width);
			dst2 = (yearNumbers[1] + (height-1) * width);
		} else if (type == ImageType::yearNumbers2) {
			renderWidth = (int)width;
			dst = (yearNumbers2[0] + (height-1) * width);
			dst2 = (yearNumbers2[1] + (height-1) * width);
		} else if (type == ImageType::dayNumbers) {
			renderWidth = (int)width;
			dst = (dayNumbers[0] + (height-1) * width);
			dst2 = (dayNumbers[1] + (height-1) * width);
		} else if (type == ImageType::dayNumbers2) {
			renderWidth = (int)width;
			dst = (dayNumbers2[0] + (height-1) * width);
			dst2 = (dayNumbers2[1] + (height-1) * width);
		} else if (type == ImageType::weekdayText) {
			renderWidth = (int)width;
			dst = (weekdayText[0] + (height-1) * width);
			dst2 = (weekdayText[1] + (height-1) * width);
		} else if (type == ImageType::weekdayText2) {
			renderWidth = (int)width;
			dst = (weekdayText2[0] + (height-1) * width);
			dst2 = (weekdayText2[1] + (height-1) * width);
		} else if (type == ImageType::selectionBarBg) {
			renderWidth = (int)width;
			dst = (selectionBarBg[0] + (height-1) * width);
			dst2 = (selectionBarBg[1] + (height-1) * width);
		}
		u16 *src = bmpImageBuffer;
		for (uint y = 0; y < height; y++, dst -= renderWidth, dst2 -= renderWidth) {
			for (uint x = 0; x < width; x++) {
				u16 val = *(src++);
				if (type >= ImageType::startButton && val == 0) {
					u16 color = 0 | BIT(15);
					*(dst + x) = color;
					*(dst2 + x) = color;
				} else {
					u16 color = ((val >> (rgb565 ? 11 : 10)) & 0x1F) | ((val >> (rgb565 ? 1 : 0)) & (0x1F << 5)) | (val & 0x1F) << 10 | BIT(15);
					if (colorTable) {
						color = colorTable[color % 0x8000] | BIT(15);
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
			if (colorTable && ((type < ImageType::startButton) || (pixelBuffer[i] != (0 | BIT(15))))) {
				pixelBuffer[i] = colorTable[pixelBuffer[i] % 0x8000] | BIT(15);
			}
		}
		u8 *bmpImageBuffer = new u8[width * height];
		fread(bmpImageBuffer, 1, width * height, file);

		int x = 0;
		int y = height-1;
		for (u32 i = 0; i < width*height; i++) {
			const u16 color = pixelBuffer[bmpImageBuffer[i]];
			if (type == ImageType::selectionBarBg) {
				selectionBarBg[0][(y*width)+x] = color;
				selectionBarBg[1][(y*width)+x] = color;
			} else if (type == ImageType::weekdayText2) {
				weekdayText2[0][(y*width)+x] = color;
				weekdayText2[1][(y*width)+x] = color;
			} else if (type == ImageType::weekdayText) {
				weekdayText[0][(y*width)+x] = color;
				weekdayText[1][(y*width)+x] = color;
			} else if (type == ImageType::dayNumbers2) {
				dayNumbers2[0][(y*width)+x] = color;
				dayNumbers2[1][(y*width)+x] = color;
			} else if (type == ImageType::dayNumbers) {
				dayNumbers[0][(y*width)+x] = color;
				dayNumbers[1][(y*width)+x] = color;
			} else if (type == ImageType::yearNumbers2) {
				yearNumbers2[0][(y*width)+x] = color;
				yearNumbers2[1][(y*width)+x] = color;
			} else if (type == ImageType::yearNumbers) {
				yearNumbers[0][(y*width)+x] = color;
				yearNumbers[1][(y*width)+x] = color;
			} else if (type == ImageType::clockColon2) {
				clockColon2[0][(y*width)+x] = color;
				clockColon2[1][(y*width)+x] = color;
			} else if (type == ImageType::clockColon) {
				clockColon[0][(y*width)+x] = color;
				clockColon[1][(y*width)+x] = color;
			} else if (type == ImageType::clockNumbers2) {
				clockNumbers2[0][(y*width)+x] = color;
				clockNumbers2[1][(y*width)+x] = color;
			} else if (type == ImageType::clockNumbers) {
				clockNumbers[0][(y*width)+x] = color;
				clockNumbers[1][(y*width)+x] = color;
			} else if (type == ImageType::cardIconBlue) {
				cardIconBlue[0][(y*width)+x] = color;
				cardIconBlue[1][(y*width)+x] = color;
			} else if (type == ImageType::folderUp) {
				folderUpIcon[0][(y*width)+x] = color;
				folderUpIcon[1][(y*width)+x] = color;
			} else if (type == ImageType::brightness) {
				brightnessBtn[0][(y*width)+x] = color;
				brightnessBtn[1][(y*width)+x] = color;
			} else if (type == ImageType::startButton) {
				startButton[0][(y*width)+x] = color;
				startButton[1][(y*width)+x] = color;
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
			if (colorTable && ((type < ImageType::startButton) || (monoPixel[i] != (0 | BIT(15))))) {
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
				if (type == ImageType::selectionBarBg) {
					selectionBarBg[0][(y*width)+x] = color;
					selectionBarBg[1][(y*width)+x] = color;
				} else if (type == ImageType::weekdayText2) {
					weekdayText2[0][(y*width)+x] = color;
					weekdayText2[1][(y*width)+x] = color;
				} else if (type == ImageType::weekdayText) {
					weekdayText[0][(y*width)+x] = color;
					weekdayText[1][(y*width)+x] = color;
				} else if (type == ImageType::dayNumbers2) {
					dayNumbers2[0][(y*width)+x] = color;
					dayNumbers2[1][(y*width)+x] = color;
				} else if (type == ImageType::dayNumbers) {
					dayNumbers[0][(y*width)+x] = color;
					dayNumbers[1][(y*width)+x] = color;
				} else if (type == ImageType::yearNumbers2) {
					yearNumbers2[0][(y*width)+x] = color;
					yearNumbers2[1][(y*width)+x] = color;
				} else if (type == ImageType::yearNumbers) {
					yearNumbers[0][(y*width)+x] = color;
					yearNumbers[1][(y*width)+x] = color;
				} else if (type == ImageType::clockColon2) {
					clockColon2[0][(y*width)+x] = color;
					clockColon2[1][(y*width)+x] = color;
				} else if (type == ImageType::clockColon) {
					clockColon[0][(y*width)+x] = color;
					clockColon[1][(y*width)+x] = color;
				} else if (type == ImageType::clockNumbers2) {
					clockNumbers2[0][(y*width)+x] = color;
					clockNumbers2[1][(y*width)+x] = color;
				} else if (type == ImageType::clockNumbers) {
					clockNumbers[0][(y*width)+x] = color;
					clockNumbers[1][(y*width)+x] = color;
				} else if (type == ImageType::cardIconBlue) {
					cardIconBlue[0][(y*width)+x] = color;
					cardIconBlue[1][(y*width)+x] = color;
				} else if (type == ImageType::folderUp) {
					folderUpIcon[0][(y*width)+x] = color;
					folderUpIcon[1][(y*width)+x] = color;
				} else if (type == ImageType::brightness) {
					brightnessBtn[0][(y*width)+x] = color;
					brightnessBtn[1][(y*width)+x] = color;
				} else if (type == ImageType::startButton) {
					startButton[0][(y*width)+x] = color;
					startButton[1][(y*width)+x] = color;
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
			topImage[0][(xPos+x+(y*256))+(yPos*256)] = res;
		} else {
			bottomImage[0][(xPos+x+(y*256))+(yPos*256)] = res;
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
	if (fadeType) {
		screenBrightness--;
		if (screenBrightness < 0) screenBrightness = 0;
	} else {
		screenBrightness++;
		if (screenBrightness > 31) screenBrightness = 31;
	}
	if (ms().macroMode) {
		SetBrightness(0, lcdSwapped ? screenBrightness : 31);
		SetBrightness(1, !lcdSwapped ? screenBrightness : 31);
	} else {
		if (controlBottomBright) SetBrightness(0, screenBrightness);
		if (controlTopBright) SetBrightness(1, screenBrightness);
	}

	static bool showdialogboxPrev = showdialogbox;
	static int dialogboxHeightPrev = dialogboxHeight;
	static bool displayIconsPrev = displayIcons;

	if (showdialogboxPrev != showdialogbox) {
		showdialogboxPrev = showdialogbox;
		updateFrame = true;
	}

	if (showdialogbox && (dialogboxHeightPrev != dialogboxHeight)) {
		dialogboxHeightPrev = dialogboxHeight;
		updateFrame = true;
	}

	if (displayIconsPrev != displayIcons) {
		displayIconsPrev = displayIcons;
		updateFrame = true;
	}

	if (displayIcons) {
		if (ms().ak_viewMode == TWLSettings::EViewSmallIcon && smallIconsToDisplay > 0) {
			// Playback animated icons
			for (int i = 0; i < smallIconsToDisplay; i++) {
				if (bnriconisDSi[i] && playBannerSequence(i)) {
					updateFrame = true;
				}
			}
		} else if (ms().ak_viewMode != TWLSettings::EViewList && iconsToDisplay > 0) {
			// Playback animated icons
			for (int i = 0; i < iconsToDisplay; i++) {
				if (bnriconisDSi[i] && playBannerSequence(i)) {
					updateFrame = true;
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
					updateFrame = true;
				}
				if (iconScaleDelay++ == 2) {
					iconScaleDelay = 0;
				}
			} else if (ms().ak_zoomIcons) {
				if (iconScaleWait++ == 60) {
					iconScaleWait = 0;
					iconScaleEnabled = true;
				}
			}
		}
	}

	// Blink colon once per second
	if (colonTimer >= 60) {
		colonTimer = 0;
		showColon = !showColon;
	}

	colonTimer++;

	if (updateFrame) {
		glBegin2D();

		// glColor(RGB15(31, 31-(3*blfLevel), 31-(6*blfLevel)));
		glColor(RGB15(31, 31, 31));

		if (displayIcons) {
			if (ms().ak_viewMode == TWLSettings::EViewSmallIcon && smallIconsToDisplay > 0) {
				for (int i = 0; i < smallIconsToDisplay; i++) {
					drawIcon(i, 5, (20+(i*18)), (1 << 11));
				}
			} else if (ms().ak_viewMode != TWLSettings::EViewList && iconsToDisplay > 0) {
				for (int i = 0; i < iconsToDisplay; i++) {
					/* if (cursorPosOnScreen == i) {
						glBoxFilled(2, 19+(i*38), 253, 19+37+(i*38), selectionBarColor1); // Draw selection bar
					} */
					if ((i == cursorPosOnScreen) && (iconScale > 0)) {
						drawIcon(i, 5-iconShift, 22+(i*38)-iconShift, (1 << 12)+iconScale);
					} else {
						drawIcon(i, 5, 22+(i*38), 0);
					}
					// if (bnrWirelessIcon > 0) glSprite(24, 12, GL_FLIP_NONE, &wirelessIcons[(bnrWirelessIcon-1) & 31]);
				}
			}
		}
		if (showdialogbox) {
			glBoxFilled(15, 71, 241, 121+(dialogboxHeight*12), formFrameColor);
			// glBoxFilledGradient(16, 72, 240, 86, windowColorTop, windowColorBottom, windowColorBottom, windowColorTop);
			glBoxFilled(16, 88, 240, 120+(dialogboxHeight*12), formBodyColor);
		}

		glEnd2D();
		GFX_FLUSH = 0;
		updateFrame = false;
	}

	if (vblankRefreshCounter >= REFRESH_EVERY_VBLANKS) {
		if (!showdialogbox) {
			reloadIconPalettes();
		}
		vblankRefreshCounter = 0;
	} else {
		vblankRefreshCounter++;
	}

	if (doubleBuffer) {
		dmaCopyHalfWordsAsynch(0, topImageWithText[secondBuffer], BG_GFX_SUB, 0x18000);
		dmaCopyHalfWordsAsynch(1, bottomImageWithBar[secondBuffer], BG_GFX, 0x18000);
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

	lcdMainOnBottom();
	lcdSwapped = true;

	u16 white = 0xFFFF;
	if (colorTable) {
		white = colorTable[0x7FFF] | BIT(15);
	}

	dmaFillHalfWords(white, BG_GFX, 0x18000);
	dmaFillHalfWords(white, BG_GFX_SUB, 0x18000);
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
		std::string themePathNested = themePath + "/" + ms().ak_theme;
		if (access(themePathNested.c_str(), F_OK) == 0) {
			themePath = themePathNested;
		}
	}
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

	if (sys().isRegularDS() || (dsiFeatures() && !sys().i2cBricked() && ms().consoleModel < 2)) {
		std::string pathBrightness;
		if (access((themePath + "/brightness.bmp").c_str(), F_OK) == 0) {
			pathBrightness = themePath + "/brightness.bmp";
		} else {
			pathBrightness = "nitro:/themes/zelda/brightness.bmp";
		}

		loadBmp(ImageType::brightness, pathBrightness.c_str());
	}

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
		std::string fileStartButton = ini.GetString("start button", "file", "none");

		if (fileStartButton != "none") {
			std::string pathStartButton;
			if (access((themePath + "/" + fileStartButton).c_str(), F_OK) == 0) {
				pathStartButton = themePath + "/" + fileStartButton;
			} else {
				pathStartButton = "nitro:/themes/zelda/" + fileStartButton;
			}

			if (extension(pathStartButton.c_str(), {".bmp"})) {
				loadBmp(ImageType::startButton, pathStartButton.c_str());
			}
		}

		std::string pathCardIcon;
		if (access((themePath + "/card_icon_blue.bmp").c_str(), F_OK) == 0) {
			pathCardIcon = themePath + "/card_icon_blue.bmp";
		} else {
			pathCardIcon = "nitro:/themes/zelda/card_icon_blue.bmp";
		}

		loadBmp(ImageType::cardIconBlue, pathCardIcon.c_str());

		cardIconBlueX = ini.GetInt("disk icon", "x", cardIconBlueX);
		cardIconBlueY = ini.GetInt("disk icon", "y", cardIconBlueY);

		formFrameColor = ini.GetInt("global settings", "formFrameColor", formFrameColor);
		formBodyColor = ini.GetInt("global settings", "formBodyColor", formBodyColor);

		selectionBarColor1 = ini.GetInt("main list", "selectionBarColor1", RGB15(16, 20, 24)) | BIT(15);
		selectionBarColor2 = ini.GetInt("main list", "selectionBarColor2", RGB15(20, 25, 0)) | BIT(15);
		selectionBarOpacity = ini.GetInt("main list", "selectionBarOpacity", 100);
		if (colorTable) {
			selectionBarColor1 = colorTable[selectionBarColor1 % 0x8000];
			selectionBarColor2 = colorTable[selectionBarColor2 % 0x8000];
		}

		if (ini.GetInt("main list", "showSelectionBarBg", false)) {
			std::string pathSelectionBarBg;
			if (access((themePath + "/selection_bar_bg.bmp").c_str(), F_OK) == 0) {
				pathSelectionBarBg = themePath + "/selection_bar_bg.bmp";
			} else {
				pathSelectionBarBg = "nitro:/themes/zelda/selection_bar_bg.bmp";
			}

			loadBmp(ImageType::selectionBarBg, pathSelectionBarBg.c_str());
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

			if (ini.GetInt("small clock", "show", 0)) {
				smallClockX = ini.GetInt("small clock", "x", smallClockX);
				smallClockY = ini.GetInt("small clock", "y", smallClockY);

				std::string pathClockNumbers;
				if (access((themePath + "/calendar/clock_numbers_2.bmp").c_str(), F_OK) == 0) {
					pathClockNumbers = themePath + "/calendar/clock_numbers_2.bmp";
				} else {
					pathClockNumbers = "nitro:/themes/zelda/calendar/clock_numbers_2.bmp";
				}

				loadBmp(ImageType::clockNumbers2, pathClockNumbers.c_str());

				if (access((themePath + "/calendar/clock_colon_2.bmp").c_str(), F_OK) == 0) {
					pathClockNumbers = themePath + "/calendar/clock_colon_2.bmp";
				} else {
					pathClockNumbers = "nitro:/themes/zelda/calendar/clock_colon_2.bmp";
				}

				loadBmp(ImageType::clockColon2, pathClockNumbers.c_str());
			}

			showYear = ini.GetInt("calendar year", "show", showYear);
			showYear2 = ini.GetInt("calendar year 2", "show", showYear2);
			showMonth = ini.GetInt("calendar month", "show", showMonth);
			showMonth2 = ini.GetInt("calendar month 2", "show", showMonth2);
			showDayX = ini.GetInt("calendar dayx", "show", showDayX);
			showDayX2 = ini.GetInt("calendar dayx 2", "show", showDayX2);

			if (showYear) {
				yearX = ini.GetInt("calendar year", "x", yearX);
				yearY = ini.GetInt("calendar year", "y", yearY);
			}
			if (showYear2) {
				year2X = ini.GetInt("calendar year 2", "x", year2X);
				year2Y = ini.GetInt("calendar year 2", "y", year2Y);
			}
			if (showMonth) {
				monthX = ini.GetInt("calendar month", "x", monthX);
				monthY = ini.GetInt("calendar month", "y", monthY);
			}
			if (showMonth2) {
				month2X = ini.GetInt("calendar month 2", "x", month2X);
				month2Y = ini.GetInt("calendar month 2", "y", month2Y);
			}
			if (showDayX) {
				dayxX = ini.GetInt("calendar dayx", "x", dayxX);
				dayxY = ini.GetInt("calendar dayx", "y", dayxY);
			}
			if (showDayX2) {
				dayx2X = ini.GetInt("calendar dayx 2", "x", dayx2X);
				dayx2Y = ini.GetInt("calendar dayx 2", "y", dayx2Y);
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
			if (showYear2 || showMonth2 || showDayX2) {
				std::string pathYearNumbers;
				if (access((themePath + "/calendar/year_numbers_2.bmp").c_str(), F_OK) == 0) {
					pathYearNumbers = themePath + "/calendar/year_numbers_2.bmp";
				} else {
					pathYearNumbers = "nitro:/themes/zelda/calendar/year_numbers_2.bmp";
				}

				loadBmp(ImageType::yearNumbers2, pathYearNumbers.c_str());
			}

			if (ini.GetInt("calendar day", "show", 0)) {
				dayPositionX = ini.GetInt("calendar day", "x", dayPositionX);
				dayPositionY = ini.GetInt("calendar day", "y", dayPositionY);
				daySizeX = ini.GetInt("calendar day", "dw", daySizeX);
				daySizeY = ini.GetInt("calendar day", "dh", daySizeY);
				dayHighlightColor = ini.GetInt("calendar day", "highlightColor", dayHighlightColor) | BIT(15);
				if (colorTable) {
					dayHighlightColor = colorTable[dayHighlightColor % 0x8000];
				}

				std::string pathDayNumbers;
				if (access((themePath + "/calendar/day_numbers.bmp").c_str(), F_OK) == 0) {
					pathDayNumbers = themePath + "/calendar/day_numbers.bmp";
				} else {
					pathDayNumbers = "nitro:/themes/zelda/calendar/day_numbers.bmp";
				}

				loadBmp(ImageType::dayNumbers, pathDayNumbers.c_str());
			}
			if (ini.GetInt("calendar day 2", "show", 0)) {
				dayPosition2X = ini.GetInt("calendar day 2", "x", dayPosition2X);
				dayPosition2Y = ini.GetInt("calendar day 2", "y", dayPosition2Y);
				daySize2X = ini.GetInt("calendar day 2", "dw", daySize2X);
				daySize2Y = ini.GetInt("calendar day 2", "dh", daySize2Y);
				dayHighlightColor2 = ini.GetInt("calendar day 2", "highlightColor", dayHighlightColor2) | BIT(15);
				if (colorTable) {
					dayHighlightColor2 = colorTable[dayHighlightColor2 % 0x8000];
				}

				std::string pathDayNumbers;
				if (access((themePath + "/calendar/day_numbers_2.bmp").c_str(), F_OK) == 0) {
					pathDayNumbers = themePath + "/calendar/day_numbers_2.bmp";
				} else {
					pathDayNumbers = "nitro:/themes/zelda/calendar/day_numbers_2.bmp";
				}

				loadBmp(ImageType::dayNumbers2, pathDayNumbers.c_str());
			}

			if (ini.GetInt("calendar weekday", "show", 0)) {
				weekdayX = ini.GetInt("calendar weekday", "x", weekdayX);
				weekdayY = ini.GetInt("calendar weekday", "y", weekdayY);

				std::string pathWeekdayText;
				if (access((themePath + "/calendar/weekday_text.bmp").c_str(), F_OK) == 0) {
					pathWeekdayText = themePath + "/calendar/weekday_text.bmp";
				} else {
					pathWeekdayText = "nitro:/themes/zelda/calendar/weekday_text.bmp";
				}

				loadBmp(ImageType::weekdayText, pathWeekdayText.c_str());
			}
			if (ini.GetInt("calendar weekday 2", "show", 0)) {
				weekday2X = ini.GetInt("calendar weekday 2", "x", weekday2X);
				weekday2Y = ini.GetInt("calendar weekday 2", "y", weekday2Y);

				std::string pathWeekdayText;
				if (access((themePath + "/calendar/weekday_text_2.bmp").c_str(), F_OK) == 0) {
					pathWeekdayText = themePath + "/calendar/weekday_text_2.bmp";
				} else {
					pathWeekdayText = "nitro:/themes/zelda/calendar/weekday_text_2.bmp";
				}

				loadBmp(ImageType::weekdayText2, pathWeekdayText.c_str());
			}
		}
	}

	// Initialize the bottom background
	// bottomBg = bgInit(2, BgType_ExRotation, BgSize_ER_256x256, 0,1);

	startBorderColor = RGB15(colorRvalue/8, colorGvalue/8, colorBvalue/8) | BIT(15); // Bit 15 is needed for the color to display on the top screen
	// windowColorTop = RGB15(0, 0, 31);
	// windowColorBottom = RGB15(0, 0, 15);
	if (colorTable) {
		startBorderColor = colorTable[startBorderColor % 0x8000] | BIT(15);
		// windowColorTop = colorTable[windowColorTop % 0x8000];
		// windowColorBottom = colorTable[windowColorBottom % 0x8000];
	}

	u16* newPalette = (u16*)wirelessiconsPal;
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
