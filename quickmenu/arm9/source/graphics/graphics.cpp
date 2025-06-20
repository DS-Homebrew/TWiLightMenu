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
#include <nds/arm9/dldi.h>
#include <maxmod9.h>
#include <gl2d.h>
#include "bios_decompress_callback.h"
#include "myDSiMode.h"
#include "fileCopy.h"
#include "common/flashcard.h"
#include "common/systemdetails.h"
#include "common/twlmenusettings.h"
#include "common/logging.h"
#include "language.h"
#include <cmath>

// Graphic files
#include "menu_icons.h"
#include "grit_tileset.h"

#include "cursorpal.h"
#include "usercolors.h"

#include "../iconTitle.h"
#include "graphics.h"
#include "common/lodepng.h"
#include "color.h"
#include "fontHandler.h"
#include "../ndsheaderbanner.h"
#include "../errorScreen.h"
#include "../date.h"

#define CONSOLE_SCREEN_WIDTH 32
#define CONSOLE_SCREEN_HEIGHT 24

#define CONVERT_COLOR(r,g,b) r>>3 | (g>>3)<<5 | (b>>3)<<10 | BIT(15)

extern bool useTwlCfg;

//extern bool widescreenEffects;

extern bool whiteScreen;
extern bool fadeType;
extern bool fadeSpeed;
extern bool controlTopBright;
extern bool controlBottomBright;
int fadeDelay = 0;
int waitUntilFadeOut = 0;

extern int colorRvalue;
extern int colorGvalue;
extern int colorBvalue;

int screenBrightness = 31;

int vblankRefreshCounter = 0;

bool showProgressBar = false;
int progressBarLength = 0;

float cursorTargetTL = 0.0f, cursorTargetTR = 0.0f, cursorTargetBL = 0.0f, cursorTargetBR = 0.0f;
float cursorTL = 0.0f, cursorTR = 0.0f, cursorBL = 0.0f, cursorBR = 0.0f;
float cursorTLPrev = 0.0f, cursorTRPrev = 0.0f, cursorBLPrev = 0.0f, cursorBRPrev = 0.0f;

extern int spawnedtitleboxes;

// extern bool showCursor;
extern bool startMenu;
extern MenuEntry cursorPosition;

extern MenuEntry initialTouchedPosition;
extern MenuEntry currentTouchedPosition;

extern bool pictochatFound;
extern bool dlplayFound;
extern bool gbaBiosFound;
extern bool cardEjected;

int boxArtType[2] = {0};

bool moveIconUp[7] = {false};
int iconYpos[7] = {25, 73, 73, 121, 175, 170, 175};

bool showdialogbox = false;
int dialogboxHeight = 0;

constexpr int calendarXPos = 125;
constexpr int calendarYPos = 31;
constexpr int clockXPos = 13;
constexpr int clockYPos = 45;
constexpr int batteryXPos = 242;
constexpr int batteryYPos = 4;

GRIT_TEXTURE(cornericons, 32, 32, 32, 128, 4);
GRIT_TEXTURE(cursor, 32, 32, 32, 128, 4);
GRIT_TEXTURE(icon_dscard, 32, 32, 32, 64, 2);
GRIT_TEXTURE(icon_gbamode, 32, 32, 32, 64, 2);
GRIT_TEXTURE(icon_settings, 32, 32, 32, 64, 2);
GRIT_TEXTURE(icon_settings_away, 32, 32, 32, 32, 1);
GRIT_TEXTURE(iconbox, 256, 64, 256, 128, 2);
GRIT_TEXTURE(iconbox_pressed, 256, 64, 256, 64, 1);
GRIT_TEXTURE(wirelessicons, 32, 32, 32, 64, 2);
GRIT_TEXTURE(pictochat, 128, 64, 128, 256, 4);
GRIT_TEXTURE(pictochat_jp, 128, 64, 128, 256, 4);
GritTexture<128, 64, 512, 64, dlp_iconsPalLen/sizeof(*dlp_iconsPal), 4> dlp_icons{dlp_iconsPal};

std::array<glImage, 4>* pictochat_images = nullptr;

u16 bmpImageBuffer[256*192] = {0};
u16 topImageBuffer[256*192] = {0};
u16* colorTable = nullptr;

u16 calendarImageBuffer[117*115] = {0};
u16 calendarBigImageBuffer[117*131] = {0};
u16 markerImageBuffer[13*13] = {0};

u16 batteryFullImageBuffer[12*7] = {0};
u16 batteryLowImageBuffer[12*7] = {0};

u16 clockImageBuffer[101*101] = {0};
u16 clockNeedleColor;
u16 clockPinColor;
u16 clockUserColor;

void vramcpy_ui (void* dest, const void* src, int size) 
{
	u16* destination = (u16*)dest;
	u16* source = (u16*)src;
	while (size > 0) {
		*destination++ = *source++;
		size-=2;
	}
}

void ClearBrightness(void) {
	fadeType = true;
	screenBrightness = 0;
	swiWaitForVBlank();
	swiWaitForVBlank();
}

bool screenFadedIn(void) { return (screenBrightness == 0); }
bool screenFadedOut(void) { return (screenBrightness > 24); }

void updateCursorTargetPos(void) { 
	switch (cursorPosition) {
		case MenuEntry::INVALID:
			cursorTargetTL = 0.0f;
			cursorTargetBL = 0.0f;
			cursorTargetTR = 0.0f;
			cursorTargetBR = 0.0f;
			break;
		case MenuEntry::CART:
			cursorTargetTL = 31.0f;
			cursorTargetBL = 23.0f;
			cursorTargetTR = 213.0f;
			cursorTargetBR = 61.0f;
			//drawCursorRect(31, 23, 213, 61);
			break;
		case MenuEntry::PICTOCHAT:
			cursorTargetTL = 31.0f;
			cursorTargetBL = 71.0f;
			cursorTargetTR = 117.0f;
			cursorTargetBR = 109.0f;
			//drawCursorRect(31, 71, 117, 109);
			break;
		case MenuEntry::DOWNLOADPLAY:
			cursorTargetTL = 127.0f;
			cursorTargetBL = 71.0f;
			cursorTargetTR = 213.0f;
			cursorTargetBR = 109.0f;
			//drawCursorRect(127, 71, 213, 109);
			break;
		case MenuEntry::GBA:
			cursorTargetTL = 31.0f;
			cursorTargetBL = 119.0f;
			cursorTargetTR = 213.0f;
			cursorTargetBR = 157.0f;
			//drawCursorRect(31, 119, 213, 157);
			break;
		case MenuEntry::BRIGHTNESS:
			cursorTargetTL = 0.0f;
			cursorTargetBL = 167.0f;
			cursorTargetTR = 20.0f;
			cursorTargetBR = 182.0f;
			//drawCursorRect(0, 167, 20, 182);
			break;
		case MenuEntry::SETTINGS:
			cursorTargetTL = 112.0f;
			cursorTargetBL = 167.0f;
			cursorTargetTR = 132.0f;
			cursorTargetBR = 182.0f;
			//drawCursorRect(112, 167, 132, 182);
			break;
		case MenuEntry::MANUAL:
			cursorTargetTL = 225.0f;
			cursorTargetBL = 167.0f;
			cursorTargetTR = 245.0f;
			cursorTargetBR = 182.0f;
			//drawCursorRect(225, 167, 245, 182);
			break;
	}
}

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

static void bootModeIconLoad() {
	if (ms().macroMode) return;

	const char* filePath = (ms().autorun || (isDSiMode() && !flashcardFound() && ms().autostartSlot1)) ? "nitro:/graphics/icons/bootauto.png" : "nitro:/graphics/icons/bootmanual.png";

	constexpr int posX = 226;
	constexpr int posY = 2;

	u16 imageBuffer[11*11] = { 0 };

	FILE* file = fopen(filePath, "rb");
	if (file) {
		// Start loading
		std::vector<unsigned char> image;
		unsigned width, height;
		lodepng::decode(image, width, height, filePath);
		for (unsigned i=0;i<image.size()/4;i++) {
			imageBuffer[i] = image[i*4]>>3 | (image[(i*4)+1]>>3)<<5 | (image[(i*4)+2]>>3)<<10 | BIT(15);
			if (colorTable) {
				imageBuffer[i] = colorTable[imageBuffer[i] % 0x8000] | BIT(15);
			}
		}

		u16* src = imageBuffer;
		for (int y = 0; y < 11; y++) {
			for (int x = 0; x < 11; x++) {
				BG_GFX_SUB[(posY+y)*256+(posX+x)] = *src;
				src++;
			}
		}
	}

	fclose(file);
}

void batteryIconLoad() {
	if (ms().macroMode) return;

	// Load full battery icon

	const char* filePath = "nitro:/graphics/battery/batteryfull.png";
	if (dsiFeatures() && !sys().i2cBricked() && ms().consoleModel < 2 && ms().powerLedColor) {
		filePath = "nitro:/graphics/battery/batteryfullPurple.png";
	} else if (sys().isRegularDS()) {
		filePath = "nitro:/graphics/battery/batteryfullDS.png";
	}

	FILE* file = fopen(filePath, "rb");

	if (file) {
		// Start loading
		std::vector<unsigned char> image;
		unsigned width, height;
		lodepng::decode(image, width, height, filePath);

		int x = batteryXPos, y = batteryYPos;
		int xEnd = x + 12;
		for (unsigned i=0;i<image.size()/4;i++) {
			u16 color = image[i*4]>>3 | (image[(i*4)+1]>>3)<<5 | (image[(i*4)+2]>>3)<<10 | BIT(15);
			if (colorTable) {
				color = colorTable[color % 0x8000] | BIT(15);
			}
			
			if (image[(i*4)+3] == 0) {
				batteryFullImageBuffer[i] = bmpImageBuffer[(y*256)+x];
			} else {
				batteryFullImageBuffer[i] = color;
			}

			x++;
			if (x == xEnd) {
				x = batteryXPos;
				y++;
			}
		}
	}

	fclose(file);

	// Load low battery icon

	filePath = "nitro:/graphics/battery/batterylow.png";
	if (dsiFeatures() && !sys().i2cBricked() && ms().consoleModel < 2 && ms().powerLedColor) {
		filePath = "nitro:/graphics/battery/batteryfullPurple.png";
	}

	file = fopen(filePath, "rb");
	if (file) {
		// Start loading
		std::vector<unsigned char> image;
		unsigned width, height;
		lodepng::decode(image, width, height, filePath);

		int x = batteryXPos, y = batteryYPos;
		int xEnd = x + 12;
		for (unsigned i=0;i<image.size()/4;i++) {
			u16 color = image[i*4]>>3 | (image[(i*4)+1]>>3)<<5 | (image[(i*4)+2]>>3)<<10 | BIT(15);
			if (colorTable) {
				color = colorTable[color % 0x8000] | BIT(15);
			}
			
			if (image[(i*4)+3] == 0) {
				batteryLowImageBuffer[i] = bmpImageBuffer[(y*256)+x];
			} else {
				batteryLowImageBuffer[i] = color;
			}

			x++;
			if (x == xEnd) {
				x = batteryXPos;
				y++;
			}
		}
	}

	fclose(file);
}

static bool isBatteryLow(void) {
	const u8 batteryLevel = sys().batteryStatus();
	if (batteryLevel & BIT(7)) // charging
		return false;

	if (batteryLevel <= 0x3)
		return true;
	else
		return false;
}

void batteryIconDraw(bool blink) {
	if (ms().macroMode) return;

	bool low = isBatteryLow();
	if (low && blink) {
		for (int y = 0; y < 7; y++) {
			for (int x = 0; x < 12; x++) {
				BG_GFX_SUB[(batteryYPos+y)*256+(batteryXPos+x)] = bmpImageBuffer[(batteryYPos+y)*256+(batteryXPos+x)];
			}
		}
		return;
	}

	u16* src = batteryFullImageBuffer;
	if (low)
		src = batteryLowImageBuffer;

	for (int y = 0; y < 7; y++) {
		for (int x = 0; x < 12; x++) {
			BG_GFX_SUB[(batteryYPos+y)*256+(batteryXPos+x)] = *src;
			src++;
		}
	}
}

void gbaModeIconLoad(bool bottomScreen) {
	if (ms().macroMode) return;

	char filePath[256];
	snprintf(filePath, sizeof(filePath), "nitro:/graphics/icons/gba%s.png", bottomScreen ? "bottom" : "top");

	constexpr int posX = 210;
	constexpr int posY = 2;

	u16 imageBuffer[11*11] = {0};

	FILE* file = fopen(filePath, "rb");
	if (file) {
		// Start loading
		std::vector<unsigned char> image;
		unsigned width, height;
		lodepng::decode(image, width, height, filePath);
		for (unsigned i=0;i<image.size()/4;i++) {
			imageBuffer[i] = image[i*4]>>3 | (image[(i*4)+1]>>3)<<5 | (image[(i*4)+2]>>3)<<10 | BIT(15);
			if (colorTable) {
				imageBuffer[i] = colorTable[imageBuffer[i] % 0x8000] | BIT(15);
			}
		}

		u16* src = imageBuffer;
		for (int y = 0; y < 11; y++) {
			for (int x = 0; x < 11; x++) {
				BG_GFX_SUB[(posY+y)*256+(posX+x)] = *src;
				src++;
			}
		}
	}

	fclose(file);
}

void bottomBgLoad() {
	std::string bottomBGFile = "nitro:/graphics/bottombg.png";

	char temp[256];
	char tempNested[256];

	switch (ms().theme) {
		case TWLSettings::EThemeDSi: // DSi Theme
			sprintf(temp, "%s:/_nds/TWiLightMenu/dsimenu/themes/%s/quickmenu/bottombg.png", sdFound() ? "sd" : "fat", (ms().dsi_theme.c_str()));
			sprintf(tempNested, "%s:/_nds/TWiLightMenu/dsimenu/themes/%s/%s/quickmenu/bottombg.png", sdFound() ? "sd" : "fat", (ms().dsi_theme.c_str()), (ms().dsi_theme.c_str()));
			break;
		case TWLSettings::ETheme3DS:
			sprintf(temp, "%s:/_nds/TWiLightMenu/3dsmenu/themes/%s/quickmenu/bottombg.png", sdFound() ? "sd" : "fat", (ms()._3ds_theme.c_str()));
			sprintf(tempNested, "%s:/_nds/TWiLightMenu/3dsmenu/themes/%s/%s/quickmenu/bottombg.png", sdFound() ? "sd" : "fat", (ms()._3ds_theme.c_str()), (ms()._3ds_theme.c_str()));
			break;
		case TWLSettings::EThemeR4:
			sprintf(temp, "%s:/_nds/TWiLightMenu/r4menu/themes/%s/quickmenu/bottombg.png", sdFound() ? "sd" : "fat", (ms().r4_theme.c_str()));
			sprintf(tempNested, "%s:/_nds/TWiLightMenu/r4menu/themes/%s/%s/quickmenu/bottombg.png", sdFound() ? "sd" : "fat", (ms().r4_theme.c_str()), (ms().r4_theme.c_str()));
			break;
		case TWLSettings::EThemeWood:
			sprintf(temp, "%s:/_nds/TWiLightMenu/akmenu/themes/%s/quickmenu/bottombg.png", sdFound() ? "sd" : "fat", (ms().ak_theme.c_str()));
			sprintf(tempNested, "%s:/_nds/TWiLightMenu/akmenu/themes/%s/%s/quickmenu/bottombg.png", sdFound() ? "sd" : "fat", (ms().ak_theme.c_str()), (ms().ak_theme.c_str()));
			break;
		case TWLSettings::EThemeSaturn:
			// sprintf(temp, "nitro:/graphics/bottombg_saturn.png");
			// break;
		case TWLSettings::EThemeHBL:
		case TWLSettings::EThemeGBC:
			temp[0] = '\0';
			tempNested[0] = '\0';
			break;
	}

	if (temp[0] != '\0' && access(temp, F_OK) == 0)
		bottomBGFile = std::string(temp);
	else if (tempNested[0] != '\0' && access(tempNested, F_OK) == 0)
		bottomBGFile = std::string(tempNested);

	std::vector<unsigned char> image;
	uint imageWidth, imageHeight;
	lodepng::decode(image, imageWidth, imageHeight, bottomBGFile);
	if (imageWidth > 256 || imageHeight > 192)	return;

	for (uint i=0;i<image.size()/4;i++) {
		bmpImageBuffer[i] = image[i*4]>>3 | (image[(i*4)+1]>>3)<<5 | (image[(i*4)+2]>>3)<<10 | BIT(15);
		if (colorTable) {
			bmpImageBuffer[i] = colorTable[bmpImageBuffer[i] % 0x8000] | BIT(15);
		}
	}

	// Start loading
	u16* src = bmpImageBuffer;
	int x = 0;
	int y = 0;
	for (int i=0; i<256*192; i++) {
		if (x >= 256) {
			x = 0;
			y++;
		}
		BG_GFX[y*256+x] = *(src++);
		x++;
	}
}

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

auto getMenuEntryTexture(MenuEntry entry) {
	switch(entry) {
		case MenuEntry::CART:
			if(isDSiMode() && cardEjected)
				return &iconbox.images[1];
			if(initialTouchedPosition == MenuEntry::CART) {
				if(currentTouchedPosition != MenuEntry::CART)
					return &iconbox.images[1];
				return &iconbox_pressed.images[0];
			}
			return &iconbox.images[0];
		case MenuEntry::PICTOCHAT:
			if(!pictochatFound)
				return &(*pictochat_images)[2];
			if(initialTouchedPosition == MenuEntry::PICTOCHAT) {
				if(currentTouchedPosition != MenuEntry::PICTOCHAT)
					return &(*pictochat_images)[2];
				return &(*pictochat_images)[1];
			}
			return &(*pictochat_images)[0];
		case MenuEntry::DOWNLOADPLAY:
			if(!dlplayFound)
				return &dlp_icons.images[3];
			if(initialTouchedPosition == MenuEntry::DOWNLOADPLAY) {
				if(currentTouchedPosition != MenuEntry::DOWNLOADPLAY)
					return &dlp_icons.images[2];
				return &dlp_icons.images[1];
			}
			return &dlp_icons.images[0];
		case MenuEntry::GBA:
		{
			bool hasGbaCart = sys().isRegularDS() && ((io_dldi_data->ioInterface.features & FEATURE_SLOT_GBA) || (((u8*)GBAROM)[0xB2] == 0x96));
			if(hasGbaCart || sdFound()) {
				if(initialTouchedPosition == MenuEntry::GBA) {
					if(currentTouchedPosition != MenuEntry::GBA)
						return &iconbox.images[1];
					return &iconbox_pressed.images[0];
				}
				return &iconbox.images[0];
			}
			return &iconbox.images[1];
		}
		case MenuEntry::BRIGHTNESS:
			return &cornericons.images[0];
		case MenuEntry::SETTINGS:
			if(initialTouchedPosition == MenuEntry::SETTINGS) {
				if(currentTouchedPosition != MenuEntry::SETTINGS)
					return &icon_settings_away.images[0];
				return &icon_settings.images[1];
			}
			return &icon_settings.images[0];
		case MenuEntry::MANUAL:
			return &cornericons.images[3];
		case MenuEntry::INVALID:
			break;
	}
	__builtin_unreachable();
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
		if (fadeSpeed || waitUntilFadeOut == 15) {
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
		} else {
			waitUntilFadeOut++;
		}
	}

	static bool updateFrame = true;
	static bool whiteScreenPrev = whiteScreen;
	static bool showProgressBarPrev = showProgressBar;
	static int progressBarLengthPrev = progressBarLength;
	// static bool showCursorPrev = showCursor;
	static bool startMenuPrev = startMenu;

	if (whiteScreenPrev != whiteScreen) {
		whiteScreenPrev = whiteScreen;
		updateFrame = true;
	}

	if (showProgressBarPrev != showProgressBar) {
		showProgressBarPrev = showProgressBar;
		updateFrame = true;
	}

	if (progressBarLengthPrev != progressBarLength) {
		progressBarLengthPrev = progressBarLength;
		updateFrame = true;
	}

	/* if (showCursorPrev != showCursor) {
		showCursorPrev = showCursor;
		updateFrame = true;
	} */

	if (startMenuPrev != startMenu) {
		startMenuPrev = startMenu;
		updateFrame = true;
	}

	if (!whiteScreen && startMenu) {
		// Playback animated icons
		for (int i = 0; i < 2; i++) {
			if (bnriconisDSi[i] && playBannerSequence(i)) {
				updateFrame = true;
			}
		}

		for (int i = 0; i < 7; i++) {
			if (moveIconUp[i]) {
				iconYpos[i] -= 6;
				updateFrame = true;
			}
		}
	}

	constexpr float swiftness = 0.25f;
	cursorTL += (cursorTargetTL - cursorTL) * swiftness;
	cursorBL += (cursorTargetBL - cursorBL) * swiftness;
	cursorTR += (cursorTargetTR - cursorTR) * swiftness;
	cursorBR += (cursorTargetBR - cursorBR) * swiftness;

	if (cursorTLPrev != cursorTL) {
		cursorTLPrev = cursorTL;
		updateFrame = true;
	}

	if (cursorBLPrev != cursorBL) {
		cursorBLPrev = cursorBL;
		updateFrame = true;
	}

	if (cursorTRPrev != cursorTR) {
		cursorTRPrev = cursorTR;
		updateFrame = true;
	}

	if (cursorBRPrev != cursorBR) {
		cursorBRPrev = cursorBR;
		updateFrame = true;
	}

	if (updateFrame) {
	  glBegin2D();
	  {
		if (controlBottomBright) SetBrightness(0, screenBrightness);
		if (controlTopBright && !ms().macroMode) SetBrightness(1, screenBrightness);

		// glColor(RGB15(31, 31-(3*ms().blfLevel), 31-(6*ms().blfLevel)));
		glColor(RGB15(31, 31, 31));
		if (whiteScreen) {
			glBoxFilled(0, 0, 256, 192, RGB15(31, 31, 31));
			if (showProgressBar) {
				if (progressBarLength > 192) progressBarLength = 192;
				int barXpos = 31;
				int barYpos = 169;
				glBoxFilled(barXpos, barYpos, barXpos+192, barYpos+5, RGB15(23, 23, 23));
				if (progressBarLength > 0) {
					glBoxFilled(barXpos, barYpos, barXpos+progressBarLength, barYpos+5, RGB15(0, 0, 31));
				}
			}
		} else if (startMenu) {
			glSprite(33, iconYpos[0], GL_FLIP_NONE, getMenuEntryTexture(MenuEntry::CART));
			if (isDSiMode() && cardEjected) {
				//glSprite(33, iconYpos[0], GL_FLIP_NONE, &iconboxImage[(REG_SCFG_MC == 0x11) ? 1 : 0]);
				//glSprite(40, iconYpos[0]+6, GL_FLIP_NONE, &icon_dscard.images[(REG_SCFG_MC == 0x11) ? 1 : 0]);
				glSprite(40, iconYpos[0]+6, GL_FLIP_NONE, &icon_dscard.images[1]);
			} else {
				if ((isDSiMode() && !flashcardFound() && sys().arm7SCFGLocked()) || (io_dldi_data->ioInterface.features & FEATURE_SLOT_GBA)) {
					glSprite(40, iconYpos[0]+6, GL_FLIP_NONE, &icon_dscard.images[0]);
				} else drawIcon(1, 40, iconYpos[0]+6);
				if (bnrWirelessIcon[1] > 0) glSprite(207, iconYpos[0]+30, GL_FLIP_NONE, &wirelessicons.images[(bnrWirelessIcon[0]-1) & 31]);
			}
			glSprite(33, iconYpos[1], GL_FLIP_NONE, getMenuEntryTexture(MenuEntry::PICTOCHAT));
			glSprite(129, iconYpos[2], GL_FLIP_NONE, getMenuEntryTexture(MenuEntry::DOWNLOADPLAY));
			glSprite(33, iconYpos[3], GL_FLIP_NONE, getMenuEntryTexture(MenuEntry::GBA));
			int num = (io_dldi_data->ioInterface.features & FEATURE_SLOT_GBA) ? 1 : 0;
			if (num == 0 && ms().gbaBooter == TWLSettings::EGbaNativeGbar2) {
				glSprite(40, iconYpos[3]+6, GL_FLIP_NONE, &icon_gbamode.images[(((u8*)GBAROM)[0xB2] == 0x96) ? 0 : 1]);
			}
			else drawIcon(num, 40, iconYpos[3]+6);
			if (sys().isRegularDS() || (dsiFeatures() && !sys().i2cBricked() && ms().consoleModel < 2)) {
				glSprite(10, iconYpos[4], GL_FLIP_NONE, getMenuEntryTexture(MenuEntry::BRIGHTNESS));
			}
			if (bnrWirelessIcon[num] > 0) glSprite(207, iconYpos[3]+30, GL_FLIP_NONE, &wirelessicons.images[(bnrWirelessIcon[1]-1) & 31]);
			if (!ms().kioskMode) {
				glSprite(117, iconYpos[5], GL_FLIP_NONE, getMenuEntryTexture(MenuEntry::SETTINGS));
			}
			glSprite(235, iconYpos[6], GL_FLIP_NONE, getMenuEntryTexture(MenuEntry::MANUAL));

			// Draw cursor
			// if (showCursor) {
				auto drawCursorRect = [](int x1, int y1, int x2, int y2) {
						glSprite(x1, y1, GL_FLIP_NONE, &cursor.images[0]);
						glSprite(x2, y1, GL_FLIP_NONE, &cursor.images[1]);
						glSprite(x1, y2, GL_FLIP_NONE, &cursor.images[2]);
						glSprite(x2, y2, GL_FLIP_NONE, &cursor.images[3]);
				};
				
				updateCursorTargetPos();
				
				drawCursorRect(std::roundf(cursorTL), std::roundf(cursorBL), std::roundf(cursorTR), std::roundf(cursorBR));
			// }
		}
		/*if (showdialogbox) {
			glBoxFilled(15, 79, 241, 129+(dialogboxHeight*8), RGB15(0, 0, 0));
			glBoxFilledGradient(16, 80, 240, 94, RGB15(0, 0, 31), RGB15(0, 0, 15), RGB15(0, 0, 15), RGB15(0, 0, 31));
			glBoxFilled(16, 96, 240, 128+(dialogboxHeight*8), RGB15(31, 31, 31));
		}*/
	  }
	  glEnd2D();
	  GFX_FLUSH = 0;
	}

	if (vblankRefreshCounter >= REFRESH_EVERY_VBLANKS) {
		if (!whiteScreen && startMenu) {
			reloadIconPalettes();
		}
		vblankRefreshCounter = 0;
	} else {
		vblankRefreshCounter++;
	}
}

static void clockNeedleDraw(int angle, u32 length, u16 color) {
	if (ms().macroMode) return;
	
	constexpr float PI = 3.1415926535897f;
	
	// Find coords from angle & length
	int x0 = clockXPos + 50;
	int y0 = clockYPos + 50; 
	
	float radians = (float)(angle%360) * (PI / 180.0f);
	int x1 = x0 + std::cos(radians) * length;
	int y1 = y0 - std::sin(radians) * length;

	// Draw line using Bresenham's line algorithm
	int dx = abs(x1 - x0);
	int dy = -abs(y1 - y0);

	int stepX = x0 < x1 ? 1 : -1;
	int stepY = y0 < y1 ? 1 : -1;

	int error = (dx + dy);
	int error2;

	while (true) {
		BG_GFX_SUB[y0*256+x0] = color;
		BG_GFX_SUB[(y0+1)*256+x0] = color;
		BG_GFX_SUB[y0*256+(x0-1)] = color;
		BG_GFX_SUB[(y0+1)*256+(x0-1)] = color;

		if (x0 == x1 && y0 == y1) break;

		error2 = error * 2;

		if (error2 >= dy) {
			if (x0 == x1) break;
			error += dy;
			x0 += stepX;
		}
		
		if (error2 <= dx) {
			if (y0 == y1) break;
			error += dx;
			y0 += stepY;
		}
	}
}

static void markerLoad(void) {
	char filePath[256];
	snprintf(filePath, sizeof(filePath), "nitro:/graphics/calendar/marker/%i.png", getFavoriteColor());
	FILE* file = fopen(filePath, "rb");

	if (file) {
		// Start loading
		std::vector<unsigned char> image;
		unsigned width, height;
		lodepng::decode(image, width, height, filePath);
		for (unsigned i=0;i<image.size()/4;i++) {
			markerImageBuffer[i] = image[i*4]>>3 | (image[(i*4)+1]>>3)<<5 | (image[(i*4)+2]>>3)<<10 | BIT(15);
			if (colorTable) {
				markerImageBuffer[i] = colorTable[markerImageBuffer[i] % 0x8000] | BIT(15);
			}
		}
	}

	fclose(file);
}

static void markerDraw(int x, int y) {
	u16* src = markerImageBuffer;

	int dstX;
	for (int yy = 0; yy < 13; yy++) {
		dstX = x;
		for (int xx = 0; xx < 13; xx++) {
			BG_GFX_SUB[y*256+dstX] = *src;
			dstX++;
			src++;
		}
		y++;
	}
}

static void calendarTextDraw(const Datetime& now) {
	printSmallMonospaced(true, 56, calendarYPos+3, getDateYear(), Alignment::center);

	Datetime firstDay(now.getYear(), now.getMonth(), 1);
	int startWeekday = firstDay.getWeekDay();

	// Draw weekdays
	{
		printTiny(true, 0*16+8, calendarYPos+20, STR_TWO_LETTER_SUNDAY,    Alignment::center, FontPalette::white);
		printTiny(true, 1*16+8, calendarYPos+20, STR_TWO_LETTER_MONDAY,    Alignment::center, FontPalette::white);
		printTiny(true, 2*16+8, calendarYPos+20, STR_TWO_LETTER_TUESDAY,   Alignment::center, FontPalette::white);
		printTiny(true, 3*16+8, calendarYPos+20, STR_TWO_LETTER_WEDNESDAY, Alignment::center, FontPalette::white);
		printTiny(true, 4*16+8, calendarYPos+20, STR_TWO_LETTER_THURSDAY,  Alignment::center, FontPalette::white);
		printTiny(true, 5*16+8, calendarYPos+20, STR_TWO_LETTER_FRIDAY,    Alignment::center, FontPalette::white);
		printTiny(true, 6*16+8, calendarYPos+20, STR_TWO_LETTER_SATURDAY,  Alignment::center, FontPalette::white);
	}

	// Draw marker
	{
		int myPos = (startWeekday + now.getDay() - 1) / 7;
		markerDraw(calendarXPos+now.getWeekDay()*16+4, calendarYPos+myPos*16+34);
	}

	// Draw dates
	{
		int date = 1;
		int end = startWeekday+firstDay.getMonthDays();
		for (int i = startWeekday; i < end; ++i) {
			int cxPos = i % 7;
			int cyPos = i / 7;

			FontPalette fontColor = cxPos == 0 ? FontPalette::sunday : cxPos == 6 ? FontPalette::saturday : FontPalette::regular;
			printTiny(true, cxPos*16+8, calendarYPos+cyPos*16+36, std::to_string(date), Alignment::center, fontColor);

			date++;
		}
	}

	// Copy to background
	updateTopTextArea(calendarXPos, calendarYPos, 113, 131);
}

void calendarDraw() {
	if (ms().macroMode) return;

	int calendarHeight = 115;
	u16* src = calendarImageBuffer;

	Datetime datetime = Datetime::now();
	Datetime firstDay(datetime.getYear(), datetime.getMonth(), 1);

	// If the dates exceed the small calendar then use the big calendar
	if (firstDay.getWeekDay() + firstDay.getMonthDays() > 7*5) {
		calendarHeight = 131;
		src = calendarBigImageBuffer;
	}

	int xDst = calendarXPos;
	int yDst = calendarYPos;
	for (int yy = 0; yy < 131; yy++) {
		xDst = calendarXPos;
		for (int xx = 0; xx < 117; xx++) {
			if (yy < calendarHeight)
				BG_GFX_SUB[yDst*256+xDst] = *(src++);
			else
				BG_GFX_SUB[yDst*256+xDst] = topImageBuffer[yDst*256+xDst]; // clear bottom for the small calendar

			xDst++;
		}
		yDst++;
	}

	calendarTextDraw(datetime);
}

void calendarLoad(void) {
	if (ms().macroMode) return;

	markerLoad();

	// Small calendar
	int calendarX = calendarXPos;
	int calendarY = calendarYPos;

	const char* filePath = "nitro:/graphics/calendar/calendar.png";
	FILE* file = fopen(filePath, "rb");

	if (file) {
		// Start loading
		std::vector<unsigned char> image;
		unsigned width, height;
		lodepng::decode(image, width, height, filePath);
		for (unsigned i=0;i<image.size()/4;i++) {
			calendarImageBuffer[i] = image[i*4]>>3 | (image[(i*4)+1]>>3)<<5 | (image[(i*4)+2]>>3)<<10 | BIT(15);
			if (colorTable) {
				calendarImageBuffer[i] = colorTable[calendarImageBuffer[i] % 0x8000] | BIT(15);
			}

			calendarImageBuffer[i] = alphablend(calendarImageBuffer[i], topImageBuffer[(calendarY*256)+calendarX], image[(i*4)+3]);

			calendarX++;
			if (calendarX >= calendarXPos + 117) {
				calendarX = calendarXPos;
				calendarY++;
			}
		}
	}

	fclose(file);
	
	// Big calendar
	calendarX = calendarXPos;
	calendarY = calendarYPos;

	filePath = "nitro:/graphics/calendar/calendarbig.png";
	file = fopen(filePath, "rb");

	if (file) {
		// Start loading
		std::vector<unsigned char> image;
		unsigned width, height;
		lodepng::decode(image, width, height, filePath);
		for (unsigned i=0;i<image.size()/4;i++) {
			calendarBigImageBuffer[i] = image[i*4]>>3 | (image[(i*4)+1]>>3)<<5 | (image[(i*4)+2]>>3)<<10 | BIT(15);
			if (colorTable) {
				calendarBigImageBuffer[i] = colorTable[calendarBigImageBuffer[i] % 0x8000] | BIT(15);
			}

			calendarBigImageBuffer[i] = alphablend(calendarBigImageBuffer[i], topImageBuffer[(calendarY*256)+calendarX], image[(i*4)+3]);

			calendarX++;
			if (calendarX >= calendarXPos + 117) {
				calendarX = calendarXPos;
				calendarY++;
			}
		}
	}

	fclose(file);

	calendarDraw();
}

void clockLoad(void) {
	if (ms().macroMode) return;

	const char* filePath = "nitro:/graphics/clock.png";
	FILE* file = fopen(filePath, "rb");

	int clockX = clockXPos;
	int clockY = clockYPos;

	if (file) {
		// Start loading
		std::vector<unsigned char> image;
		unsigned width, height;
		lodepng::decode(image, width, height, filePath);
		for (unsigned i=0;i<image.size()/4;i++) {
			clockImageBuffer[i] = image[i*4]>>3 | (image[(i*4)+1]>>3)<<5 | (image[(i*4)+2]>>3)<<10 | BIT(15);
			if (colorTable) {
				clockImageBuffer[i] = colorTable[clockImageBuffer[i] % 0x8000] | BIT(15);
			}

			clockImageBuffer[i] = alphablend(clockImageBuffer[i], topImageBuffer[(clockY*256)+clockX], image[(i*4)+3]);

			clockX++;
			if (clockX >= clockXPos + 101) {
				clockX = clockXPos;
				clockY++;
			}
		}

		clockDraw();
	}
	fclose(file);
}

void clockDraw() {
	if (ms().macroMode) return;

	Datetime time = Datetime::now();

	u16* src = clockImageBuffer;
		
	int dstX = clockXPos;
	int dstY = clockYPos;
	for (int yy = 0; yy < 101; yy++) {
		dstX = clockXPos;
		for (int xx = 0; xx < 101; xx++) {
			BG_GFX_SUB[dstY*256+dstX] = *(src++);
			dstX++;
		}
		dstY++;
	}
	
	float h = (float)time.getHour() + (float)time.getMinute() / 60.0f;

	clockNeedleDraw(90-(h * 30), 24, clockNeedleColor); // hour
	clockNeedleDraw(90-(time.getMinute() * 6), 30, clockNeedleColor); // minute
	clockNeedleDraw(90-(time.getSecond() * 6), 36, clockUserColor); // second

	// draw clock pin
	for (int yy = clockYPos+48; yy < clockYPos+48+5; yy++) {
		for (int xx = clockXPos+48; xx < clockXPos+48+5; xx++) {
			BG_GFX_SUB[yy*256+xx] = clockPinColor;
		}
	}
}

// No longer used.
/*void loadBoxArt(const char* filename, bool secondaryDevice) {
	if (ms().macroMode) return;

	if (access(filename, F_OK) != 0) {
		switch (boxArtType[secondaryDevice]) {
			case -1:
				return;
			case 0:
			default:
				filename = "nitro:/graphics/boxart_unknown.png";
				break;
			case 1:
				filename = "nitro:/graphics/boxart_unknown1.png";
				break;
			case 2:
				filename = "nitro:/graphics/boxart_unknown2.png";
				break;
			case 3:
				filename = "nitro:/graphics/boxart_unknown3.png";
				break;
		}
	}

	std::vector<unsigned char> image;
	uint imageXpos, imageYpos, imageWidth, imageHeight;
	lodepng::decode(image, imageWidth, imageHeight, filename);
	if (imageWidth > 256 || imageHeight > 192)	return;

	imageXpos = (256-imageWidth)/2;
	imageYpos = (192-imageHeight)/2;

	int photoXstart = imageXpos;
	int photoXend = imageXpos+imageWidth;
	int photoX = photoXstart;
	int photoY = imageYpos;

	for (uint i=0;i<image.size()/4;i++) {
		u16 color = image[i*4]>>3 | (image[(i*4)+1]>>3)<<5 | (image[(i*4)+2]>>3)<<10 | BIT(15);
		if (colorTable) {
			color = colorTable[color % 0x8000] | BIT(15);
		}
		if (image[(i*4)+3] == 0) {
			bmpImageBuffer[i] = color;
		} else {
			bmpImageBuffer[i] = alphablend(color, topImageBuffer[(photoY*256)+photoX], image[(i*4)+3]);
		}
		photoX++;
		if (photoX == photoXend) {
			photoX = photoXstart;
			photoY++;
		}
	}

	// Re-load top BG (excluding top bar)
	u16* src = topImageBuffer+(256*16);
	int x = 0;
	int y = 16;
	for (int i=256*16; i<256*192; i++) {
		if (x >= 256) {
			x = 0;
			y++;
		}
		BG_GFX_SUB[y*256+x] = *(src++);
		x++;
	}

	src = bmpImageBuffer;
	for (uint y = 0; y < imageHeight; y++) {
		for (uint x = 0; x < imageWidth; x++) {
			BG_GFX_SUB[(y+imageYpos) * 256 + imageXpos + x] = *(src++);
		}
	}
}*/

void topBgLoad(void) {
	if (ms().macroMode) return;

	char filePath[256];
	sprintf(filePath, "nitro:/graphics/topbg.png");

	char temp[256];
	char tempNested[256];

	switch (ms().theme) {
		case TWLSettings::EThemeDSi: // DSi Theme
			sprintf(temp, "%s:/_nds/TWiLightMenu/dsimenu/themes/%s/quickmenu/topbg.png", sdFound() ? "sd" : "fat", (ms().dsi_theme.c_str()));
			sprintf(tempNested, "%s:/_nds/TWiLightMenu/dsimenu/themes/%s/%s/quickmenu/topbg.png", sdFound() ? "sd" : "fat", (ms().dsi_theme.c_str()), (ms().dsi_theme.c_str()));
			break;
		case TWLSettings::ETheme3DS:
			sprintf(temp, "%s:/_nds/TWiLightMenu/3dsmenu/themes/%s/quickmenu/topbg.png", sdFound() ? "sd" : "fat", (ms()._3ds_theme.c_str()));
			sprintf(tempNested, "%s:/_nds/TWiLightMenu/3dsmenu/themes/%s/%s/quickmenu/topbg.png", sdFound() ? "sd" : "fat", (ms()._3ds_theme.c_str()), (ms()._3ds_theme.c_str()));
			break;
		case TWLSettings::EThemeR4:
			sprintf(temp, "%s:/_nds/TWiLightMenu/r4menu/themes/%s/quickmenu/topbg.png", sdFound() ? "sd" : "fat", (ms().r4_theme.c_str()));
			sprintf(tempNested, "%s:/_nds/TWiLightMenu/r4menu/themes/%s/%s/quickmenu/topbg.png", sdFound() ? "sd" : "fat", (ms().r4_theme.c_str()), (ms().r4_theme.c_str()));
			break;
		case TWLSettings::EThemeWood:
			sprintf(temp, "%s:/_nds/TWiLightMenu/akmenu/themes/%s/quickmenu/topbg.png", sdFound() ? "sd" : "fat", (ms().ak_theme.c_str()));
			sprintf(tempNested, "%s:/_nds/TWiLightMenu/akmenu/themes/%s/%s/quickmenu/topbg.png", sdFound() ? "sd" : "fat", (ms().ak_theme.c_str()), (ms().ak_theme.c_str()));
			break;
		case TWLSettings::EThemeSaturn:
			// sprintf(temp, "nitro:/graphics/topbg_saturn.png");
			// break;
		case TWLSettings::EThemeHBL:
		case TWLSettings::EThemeGBC:
			temp[0] = '\0';
			tempNested[0] = '\0';
			break;
	}

	if (temp[0] != '\0' && access(temp, F_OK) == 0)
		sprintf(filePath, "%s", temp);
	else if (tempNested[0] != '\0' && access(tempNested, F_OK) == 0)
		sprintf(filePath, "%s", tempNested);

	std::vector<unsigned char> image;
	uint imageWidth, imageHeight;
	lodepng::decode(image, imageWidth, imageHeight, std::string(filePath));
	if (imageWidth > 256 || imageHeight > 192)	return;

	for (uint i=0;i<image.size()/4;i++) {
		topImageBuffer[i] = image[i*4]>>3 | (image[(i*4)+1]>>3)<<5 | (image[(i*4)+2]>>3)<<10 | BIT(15);
		if (colorTable) {
			topImageBuffer[i] = colorTable[topImageBuffer[i] % 0x8000] | BIT(15);
		}
	}

	// Start loading
	u16* src = topImageBuffer;
	int x = 0;
	int y = 0;
	for (int i=0; i<256*192; i++) {
		if (x >= 256) {
			x = 0;
			y++;
		}
		BG_GFX_SUB[y*256+x] = *(src++);
		x++;
	}
}

void topBarLoad(void) {
	if (ms().macroMode) return;

	char filePath[256];
	snprintf(filePath, sizeof(filePath), "nitro:/graphics/%s/%i.png", "topbar", getFavoriteColor());
	FILE* file = fopen(filePath, "rb");

	if (file) {
		// Start loading
		std::vector<unsigned char> image;
		unsigned width, height;
		lodepng::decode(image, width, height, filePath);
		for (unsigned i=0;i<image.size()/4;i++) {
			bmpImageBuffer[i] = image[i*4]>>3 | (image[(i*4)+1]>>3)<<5 | (image[(i*4)+2]>>3)<<10 | BIT(15);
			if (colorTable) {
				bmpImageBuffer[i] = colorTable[bmpImageBuffer[i] % 0x8000] | BIT(15);
			}
		}
		u16* src = bmpImageBuffer;
		int x = 0;
		int y = 0;
		for (int i = 0; i < 256 * 16; i++) {
			if (x >= 256) {
				x = 0;
				y++;
			}
			BG_GFX_SUB[y*256+x] = *(src++);
			x++;
		}
	}

	fclose(file);

	char16_t username[11] = {0};
	memcpy(username, useTwlCfg ? (s16 *)0x02000448 : PersonalData->name, 10 * sizeof(char16_t));
	printTiny(true, 3, 3, username, Alignment::left, FontPalette::white);
	updateTopTextArea(3, 3, calcTinyFontWidth(username), tinyFontHeight(), bmpImageBuffer);

	drawDateTime(true);
	drawDateTime(false);

	bootModeIconLoad();
}

void drawDateTime(bool date, bool showTimeColon) {
	std::string text = date ? getDate() : retTime();
	if (!date && !showTimeColon) text[2] = ' ';

	const int posX = date ? 204 : 172;
	if (text.length() <= 5)
		printTinyMonospaced(true, posX, 3, text, Alignment::right, FontPalette::white);
	else
		// If datetime exceeds 5 characters, don't print it as monospaced to avoid overflow.
		printTiny(true, posX+2, 3, text, Alignment::right, FontPalette::white); 
		
	updateTopTextArea(posX - 27, 3, 27, tinyFontHeight(), bmpImageBuffer);
}

void graphicsInit()
{
	logPrint("graphicsInit()\n");

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
	videoSetModeSub(MODE_5_2D);

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

	/*if (widescreenEffects) {
		// Add black bars to left and right sides
		s16 c = cosLerp(0) >> 4;
		REG_BG3PA_SUB = ( c * 315)>>8;
		REG_BG3X_SUB = -29 << 8;
	}*/

	swiWaitForVBlank();
	
	auto getDlpBitmapOffset = []{
		using Lang = TWLSettings::TLanguage;
		switch(ms().getGuiLanguage()) {
		case Lang::ELangEnglish:
		default:
			return 0;
		case Lang::ELangFrench:
			return 2;
		case Lang::ELangGerman:
			return 1;
		case Lang::ELangItalian:
			return 4;
		case Lang::ELangSpanish:
			return 3;
		case Lang::ELangJapanese:
			return 5;
		case Lang::ELangKorean:
			return 6;
		case Lang::ELangChineseS:
			return 7;
		}
	};
	
	dlp_icons.bitmap = dlp_iconsBitmap + 0x1000 * getDlpBitmapOffset();

	cursor.palette = (u16*)cursorPals+(getFavoriteColor()*16);

	if (colorTable) {
		RemapPalette(cornericons, colorTable);
		RemapPalette(cursor, colorTable);
		RemapPalette(icon_dscard, colorTable);
		RemapPalette(icon_gbamode, colorTable);
		RemapPalette(icon_settings, colorTable);
		RemapPalette(icon_settings_away, colorTable);
		RemapPalette(iconbox, colorTable);
		RemapPalette(iconbox_pressed, colorTable);
		RemapPalette(wirelessicons, colorTable);
		RemapPalette(dlp_icons, colorTable);
		RemapPalette(pictochat, colorTable);
		RemapPalette(pictochat_jp, colorTable);
	}

	LoadTileset(cornericons);
	LoadTileset(cursor);
	LoadTileset(icon_dscard);
	LoadTileset(icon_gbamode);
	LoadTileset(icon_settings);
	LoadTileset(icon_settings_away);
	LoadTileset(iconbox);
	LoadTileset(iconbox_pressed);
	LoadTileset(wirelessicons);
	LoadTileset(dlp_icons);
	if (ms().getGuiLanguage() == TWLSettings::TLanguage::ELangJapanese) {
		LoadTileset(pictochat_jp);
		pictochat_images = &pictochat_jp.images;
	} else {
		LoadTileset(pictochat);
		pictochat_images = &pictochat.images;
	}

	loadConsoleIcons();

	updateCursorTargetPos();
	cursorTL = cursorTargetTL;
	cursorBL = cursorTargetBL;
	cursorTR = cursorTargetTR;
	cursorBR = cursorTargetBR;

	clockNeedleColor = CONVERT_COLOR(121,121,121);
	clockPinColor = CONVERT_COLOR(73, 73, 73);
	clockUserColor = userColors[getFavoriteColor()];
	if (colorTable) {
		clockNeedleColor = colorTable[clockNeedleColor % 0x8000] | BIT(15);
		clockPinColor = colorTable[clockPinColor % 0x8000] | BIT(15);
		clockUserColor = colorTable[clockUserColor % 0x8000] | BIT(15);
	}

	irqSet(IRQ_VBLANK, vBlankHandler);
	irqEnable(IRQ_VBLANK);
}
