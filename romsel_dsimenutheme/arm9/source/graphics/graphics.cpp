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
#include <maxmod9.h>
#include <dirent.h>
#include <ctime>
#include "common/gl2d.h"
#include "bios_decompress_callback.h"
#include "FontGraphic.h"

// This is use for the top font.
#include "../include/startborderpal.h"

#include "graphics/ThemeTextures.h"
#include "tool/colortool.h"

#include "queueControl.h"
#include "uvcoord_top_font.h"
#include "uvcoord_date_time_font.h"

#include "../iconTitle.h"
#include "graphics.h"
#include "fontHandler.h"
#include "../ndsheaderbanner.h"
#include "../language.h"
#include "../perGameSettings.h"
#include "common/flashcard.h"
#include "common/dsimenusettings.h"
#include "common/systemdetails.h"

#include "iconHandler.h"
#include "date.h"
#define CONSOLE_SCREEN_WIDTH 32
#define CONSOLE_SCREEN_HEIGHT 24

extern u16 usernameRendered[10];

extern bool whiteScreen;
extern bool fadeType;
extern bool fadeSpeed;
extern bool controlTopBright;
extern bool controlBottomBright;
extern int colorMode;
extern int blfLevel;
int fadeDelay = 0;

extern bool music;
static int musicTime = 0;
static bool waitBeforeMusicPlay = true;
static int waitBeforeMusicPlayTime = 0;

extern int colorRvalue;
extern int colorGvalue;
extern int colorBvalue;

extern bool dropDown;
extern bool redoDropDown;
int dropTime[5] = {0};
int dropSeq[5] = {0};
int dropSpeed[5] = {5};
int dropSpeedChange[5] = {0};
int titleboxYposDropDown[5] = {-85 - 80};
int allowedTitleboxForDropDown = 0;
int delayForTitleboxToDropDown = 0;
extern bool showbubble;
extern bool showSTARTborder;
extern bool isScrolling;
extern bool needToPlayStopSound;
extern int waitForNeedToPlayStopSound;
extern int movingApp;
extern int movingAppYpos;
extern bool movingAppIsDir;
double movingArrowYpos = 59;
bool movingArrowYdirection = true;
bool showMovingArrow = false;

extern bool buttonArrowTouched[2];
extern bool scrollWindowTouched;

extern bool titleboxXmoveleft;
extern bool titleboxXmoveright;

extern bool applaunchprep;

int screenBrightness = 31;

static int colonTimer = 0;

int movetimer = 0;

int titleboxYmovepos = 0;

extern int spawnedtitleboxes;


extern int theme;
extern int subtheme;
std::vector<std::string> photoList;
static std::string photoPath;
int titleboxXmovespeed[8] = {12, 10, 8, 8, 8, 8, 6, 4};
#define titleboxXscrollspeed 8
int titleboxXpos[2] = {0};
int titleboxYpos = 85; // 85, when dropped down
int titlewindowXpos[2] = {0};

bool showLshoulder = false;
bool showRshoulder = false;

int movecloseXpos = 0;

bool showProgressIcon = false;

int progressAnimNum = 0;
int progressAnimDelay = 0;

bool startBorderZoomOut = false;
int startBorderZoomAnimSeq[5] = {0, 1, 2, 1, 0};
int startBorderZoomAnimNum = 0;
int startBorderZoomAnimDelay = 0;

int launchDotX[12] = {0};
int launchDotY[12] = {0};

bool launchDotXMove[12] = {false}; // false = left, true = right
bool launchDotYMove[12] = {false}; // false = up, true = down

int launchDotFrame[12] = {0};
int launchDotCurrentChangingFrame = 0;
bool launchDotDoFrameChange = false;

bool showdialogbox = false;
bool dbox_showIcon = false;
bool dbox_selectMenu = false;
float dbox_movespeed = 22;
float dbox_Ypos = -192;
int bottomScreenBrightness = 255;

int bottomBgState = 0; // 0 = Uninitialized 1 = No Bubble 2 = bubble.

int vblankRefreshCounter = 0;

u16 bmpImageBuffer[256 * 192] = {0};
u16 bgSubBuffer[256 * 192] = {0};

u16 dateFontImage[128 * 16];

static bool rotatingCubesLoaded = false;

bool rocketVideo_playVideo = false;
int rocketVideo_videoYpos = 78;
int rocketVideo_videoFrames = 0xEE;
int rocketVideo_currentFrame = -1;
int rocketVideo_frameDelay = 0;
bool rocketVideo_frameDelayEven = true; // For 24FPS
bool rocketVideo_loadFrame = true;

int bubbleYpos = 80;
int bubbleXpos = 122;

void vramcpy_ui(void *dest, const void *src, int size)
{
	u16 *destination = (u16 *)dest;
	u16 *source = (u16 *)src;
	while (size > 0)
	{
		*destination++ = *source++;
		size -= 2;
	}
}

extern mm_sound_effect snd_stop;
extern mm_sound_effect mus_menu;

void ClearBrightness(void)
{
	fadeType = true;
	screenBrightness = 0;
	swiWaitForVBlank();
	swiWaitForVBlank();
}

bool screenFadedIn(void)
{
	return (screenBrightness == 0);
}

bool screenFadedOut(void)
{
	return (screenBrightness > 24);
}

void beginBgSubModify()
{
	dmaCopyWords(0, BG_GFX_SUB, bgSubBuffer, sizeof(bgSubBuffer));
}

void commitBgSubModify()
{
	DC_FlushRange(bgSubBuffer, sizeof(bgSubBuffer));
	dmaCopyWords(2, bgSubBuffer, BG_GFX_SUB, sizeof(bgSubBuffer));
}

void commitBgSubModifyAsync()
{
	DC_FlushRange(bgSubBuffer, sizeof(bgSubBuffer));
	dmaCopyWordsAsynch(2, bgSubBuffer, BG_GFX_SUB, sizeof(bgSubBuffer));
}

// Ported from PAlib (obsolete)
void SetBrightness(u8 screen, s8 bright)
{
	u16 mode = 1 << 14;

	if (bright < 0)
	{
		mode = 2 << 14;
		bright = -bright;
	}
	if (bright > 31)
		bright = 31;
	*(u16 *)(0x0400006C + (0x1000 * screen)) = bright + mode;
}

void moveIconClose(int num)
{
	if (titleboxXmoveleft)
	{
		movecloseXpos = 0;
		if (movetimer == 1)
		{
			if (ms().cursorPosition[ms().secondaryDevice] - 2 == num)
				movecloseXpos = 1;
			else if (ms().cursorPosition[ms().secondaryDevice] + 2 == num)
				movecloseXpos = -2;
		}
		else if (movetimer == 2)
		{
			if (ms().cursorPosition[ms().secondaryDevice] - 2 == num)
				movecloseXpos = 1;
			else if (ms().cursorPosition[ms().secondaryDevice] + 2 == num)
				movecloseXpos = -2;
		}
		else if (movetimer == 3)
		{
			if (ms().cursorPosition[ms().secondaryDevice] - 2 == num)
				movecloseXpos = 2;
			else if (ms().cursorPosition[ms().secondaryDevice] + 2 == num)
				movecloseXpos = -3;
		}
		else if (movetimer == 4)
		{
			if (ms().cursorPosition[ms().secondaryDevice] - 2 == num)
				movecloseXpos = 2;
			else if (ms().cursorPosition[ms().secondaryDevice] + 2 == num)
				movecloseXpos = -3;
		}
		else if (movetimer == 5)
		{
			if (ms().cursorPosition[ms().secondaryDevice] - 2 == num)
				movecloseXpos = 3;
			else if (ms().cursorPosition[ms().secondaryDevice] + 2 == num)
				movecloseXpos = -4;
		}
		else if (movetimer == 6)
		{
			if (ms().cursorPosition[ms().secondaryDevice] - 2 == num)
				movecloseXpos = 4;
			else if (ms().cursorPosition[ms().secondaryDevice] + 2 == num)
				movecloseXpos = -5;
		}
		else if (movetimer == 7)
		{
			if (ms().cursorPosition[ms().secondaryDevice] - 2 == num)
				movecloseXpos = 5;
			else if (ms().cursorPosition[ms().secondaryDevice] + 2 == num)
				movecloseXpos = -6;
		}
		else if (movetimer == 8)
		{
			if (ms().cursorPosition[ms().secondaryDevice] - 2 == num)
				movecloseXpos = 6;
			else if (ms().cursorPosition[ms().secondaryDevice] + 2 == num)
				movecloseXpos = -7;
		}
	}
	if (titleboxXmoveright)
	{
		movecloseXpos = 0;
		if (movetimer == 1)
		{
			if (ms().cursorPosition[ms().secondaryDevice] - 2 == num)
				movecloseXpos = 2;
			else if (ms().cursorPosition[ms().secondaryDevice] + 2 == num)
				movecloseXpos = -1;
		}
		else if (movetimer == 2)
		{
			if (ms().cursorPosition[ms().secondaryDevice] - 2 == num)
				movecloseXpos = 2;
			else if (ms().cursorPosition[ms().secondaryDevice] + 2 == num)
				movecloseXpos = -1;
		}
		else if (movetimer == 3)
		{
			if (ms().cursorPosition[ms().secondaryDevice] - 2 == num)
				movecloseXpos = 3;
			else if (ms().cursorPosition[ms().secondaryDevice] + 2 == num)
				movecloseXpos = -2;
		}
		else if (movetimer == 4)
		{
			if (ms().cursorPosition[ms().secondaryDevice] - 2 == num)
				movecloseXpos = 3;
			else if (ms().cursorPosition[ms().secondaryDevice] + 2 == num)
				movecloseXpos = -2;
		}
		else if (movetimer == 5)
		{
			if (ms().cursorPosition[ms().secondaryDevice] - 2 == num)
				movecloseXpos = 4;
			else if (ms().cursorPosition[ms().secondaryDevice] + 2 == num)
				movecloseXpos = -3;
		}
		else if (movetimer == 6)
		{
			if (ms().cursorPosition[ms().secondaryDevice] - 2 == num)
				movecloseXpos = 5;
			else if (ms().cursorPosition[ms().secondaryDevice] + 2 == num)
				movecloseXpos = -4;
		}
		else if (movetimer == 7)
		{
			if (ms().cursorPosition[ms().secondaryDevice] - 2 == num)
				movecloseXpos = 6;
			else if (ms().cursorPosition[ms().secondaryDevice] + 2 == num)
				movecloseXpos = -5;
		}
		else if (movetimer == 8)
		{
			if (ms().cursorPosition[ms().secondaryDevice] - 2 == num)
				movecloseXpos = 7;
			else if (ms().cursorPosition[ms().secondaryDevice] + 2 == num)
				movecloseXpos = -6;
		}
	}
	if (!titleboxXmoveleft || !titleboxXmoveright)
	{
		if (ms().cursorPosition[ms().secondaryDevice] - 2 == num)
			movecloseXpos = 6;
		else if (ms().cursorPosition[ms().secondaryDevice] + 2 == num)
			movecloseXpos = -6;
		else
			movecloseXpos = 0;
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
		for (int x = 0; x < 4; x++)
		{
			oamSub.oamMemory[id].attribute[0] = ATTR0_BMP | ATTR0_SQUARE | (64 * y);
			oamSub.oamMemory[id].attribute[1] = ATTR1_SIZE_64 | (64 * x);
			oamSub.oamMemory[id].attribute[2] = ATTR2_ALPHA(1) | (8 * 32 * y) | (8 * x);
			++id;
		}

	swiWaitForVBlank();

	oamUpdate(&oamSub);
}

u16 convertToDsBmp(u16 val)
{
	if (colorMode == 1)
	{
		u16 newVal = ((val >> 10) & 31) | (val & 31 << 5) | (val & 31) << 10 | BIT(15);

		u8 b, g, r, max, min;
		b = ((newVal) >> 10) & 31;
		g = ((newVal) >> 5) & 31;
		r = (newVal)&31;
		// Value decomposition of hsv
		max = (b > g) ? b : g;
		max = (max > r) ? max : r;

		// Desaturate
		min = (b < g) ? b : g;
		min = (min < r) ? min : r;
		max = (max + min) / 2;

		newVal = 32768 | (max << 10) | (max << 5) | (max);

		b = ((newVal) >> 10) & (31 - 6 * blfLevel);
		g = ((newVal) >> 5) & (31 - 3 * blfLevel);
		r = (newVal)&31;

		return 32768 | (b << 10) | (g << 5) | (r);
	}
	else
	{
		return ((val >> 10) & 31) | (val & (31 - 3 * blfLevel) << 5) | (val & (31 - 6 * blfLevel)) << 10 | BIT(15);
	}
}

void bottomBgLoad(bool drawBubble, bool init = false)
{
	if (init || (!drawBubble && bottomBgState == 2))
	{
		tex().drawBg();
		// Set that we've not drawn the bubble.
		bottomBgState = 1;
	}
	else if (drawBubble && bottomBgState == 1)
	{
		tex().drawBubbleBg();
		// Set that we've drawn the bubble.
		bottomBgState = 2;
	}
}

void bottomBgRefresh()
{
	bottomBgLoad(showbubble, false);
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

void drawBubble(const glImage *images)
{
	glSprite(bubbleXpos, bubbleYpos, GL_FLIP_NONE, &images[0]);
}

void drawDbox()
{
	for (int y = 0; y < 192 / 16; y++)
	{
		for (int x = 0; x < 256 / 16; x++)
		{
			int i = y * 16 + x;
			glSprite(x * 16, dbox_Ypos + y * 16, GL_FLIP_NONE, &tex().dialogboxImage()[i & 255]);
		}
	}
}

void reloadDboxPalette()
{
	tex().reloadPalDialogBox();
}

static void *rotatingCubesLocation = (void *)0x02700000;

void playRotatingCubesVideo(void)
{
	if (rocketVideo_playVideo)
	{
		if (!rocketVideo_loadFrame)
		{
			rocketVideo_frameDelay++;
			rocketVideo_loadFrame = (rocketVideo_frameDelay == 2 + rocketVideo_frameDelayEven);
		}

		if (rocketVideo_loadFrame)
		{
			rocketVideo_currentFrame++;

			if (rocketVideo_currentFrame > rocketVideo_videoFrames)
			{
				rocketVideo_currentFrame = 0;
			}

			DC_FlushRange(rotatingCubesLocation + (rocketVideo_currentFrame * 0x7000), 0x7000);
			dmaCopyWordsAsynch(1, rotatingCubesLocation + (rocketVideo_currentFrame * 0x7000), (u16 *)BG_GFX_SUB + (256 * rocketVideo_videoYpos), 0x7000);

			if (colorMode == 1)
			{
				beginBgSubModify();
				for (u16 i = 0; i < 256 * 56; i++)
				{
					bgSubBuffer[(rocketVideo_videoYpos * 256) + i] = convertVramColorToGrayscale(bgSubBuffer[(rocketVideo_videoYpos * 256) + i]);
				}
				commitBgSubModifyAsync();
			}
			rocketVideo_frameDelay = 0;
			rocketVideo_frameDelayEven = !rocketVideo_frameDelayEven;
			rocketVideo_loadFrame = false;
		}
	}
}

void vBlankHandler()
{
	execQueue();			   // Execute any actions queued during last vblank.
	execDeferredIconUpdates(); // Update any icons queued during last vblank.

	if (theme == 1 && waitBeforeMusicPlay)
	{
		if (waitBeforeMusicPlayTime == 60 * 3)
		{
			mmEffectEx(&mus_menu);
			waitBeforeMusicPlay = false;
		}
		else
		{
			waitBeforeMusicPlayTime++;
		}
	}
	else
	{
		waitBeforeMusicPlay = false;
	}

	if (music && !waitBeforeMusicPlay)
	{
		musicTime++;
		if (musicTime == 60 * 49)
		{ // Length of music file in seconds (60*ss)
			mmEffectEx(&mus_menu);
			musicTime = 0;
		}
	}

	if (waitForNeedToPlayStopSound > 0)
	{
		waitForNeedToPlayStopSound++;
		if (waitForNeedToPlayStopSound == 5)
		{
			waitForNeedToPlayStopSound = 0;
		}
		needToPlayStopSound = false;
	}

	if (theme == 1 && rotatingCubesLoaded)
	{
		playRotatingCubesVideo();
	}

	glBegin2D();
	{
		if (fadeType == true)
		{
			if (!fadeDelay)
			{
				screenBrightness--;
				if (screenBrightness < 0)
					screenBrightness = 0;
			}
			if (!fadeSpeed)
			{
				fadeDelay++;
				if (fadeDelay == 3)
					fadeDelay = 0;
			}
			else
			{
				fadeDelay = 0;
			}
		}
		else
		{
			if (!fadeDelay)
			{
				screenBrightness++;
				if (screenBrightness > 31)
					screenBrightness = 31;
			}
			if (!fadeSpeed)
			{
				fadeDelay++;
				if (fadeDelay == 3)
					fadeDelay = 0;
			}
			else
			{
				fadeDelay = 0;
			}
		}
		if (controlBottomBright)
			SetBrightness(0, screenBrightness);
		if (controlTopBright)
			SetBrightness(1, screenBrightness);

		if (showdialogbox)
		{
			// Dialogbox moving into view...
			if (dbox_movespeed <= 1)
			{
				if (dbox_Ypos >= 0)
				{
					// dbox stopped
					dbox_movespeed = 0;
					dbox_Ypos = 0;
					bottomScreenBrightness = 127;
					REG_BLDY = (0b0100 << 1);
				}
				else
				{
					// dbox moving into view
					dbox_movespeed = 1;
				}
			}
			else
			{
				// Dbox decel
				dbox_movespeed -= 1.25;
				bottomScreenBrightness -= 7;
				if (bottomScreenBrightness < 127)
				{
					bottomScreenBrightness = 127;
				}
				if (bottomScreenBrightness > 231)
					REG_BLDY = 0;
				if (bottomScreenBrightness > 199 && bottomScreenBrightness <= 231)
					REG_BLDY = (0b0001 << 1);
				if (bottomScreenBrightness > 167 && bottomScreenBrightness <= 199)
					REG_BLDY = (0b0010 << 1);
				if (bottomScreenBrightness > 135 && bottomScreenBrightness <= 167)
					REG_BLDY = (0b0011 << 1);
				if (bottomScreenBrightness > 103 && bottomScreenBrightness <= 135)
					REG_BLDY = (0b0100 << 1);
			}
			dbox_Ypos += dbox_movespeed;
		}
		else
		{
			// Dialogbox moving down...
			if (dbox_Ypos <= -192 || dbox_Ypos >= 192)
			{
				dbox_movespeed = 22;
				dbox_Ypos = -192;
				bottomScreenBrightness = 255;
				REG_BLDY = 0;
			}
			else
			{
				dbox_movespeed += 1;
				dbox_Ypos += dbox_movespeed;
				bottomScreenBrightness += 7;
				if (bottomScreenBrightness > 255)
				{
					bottomScreenBrightness = 255;
				}
				if (bottomScreenBrightness > 231)
					REG_BLDY = 0;
				if (bottomScreenBrightness > 199 && bottomScreenBrightness <= 231)
					REG_BLDY = (0b0001 << 1);
				if (bottomScreenBrightness > 167 && bottomScreenBrightness <= 199)
					REG_BLDY = (0b0010 << 1);
				if (bottomScreenBrightness > 135 && bottomScreenBrightness <= 167)
					REG_BLDY = (0b0011 << 1);
				if (bottomScreenBrightness > 103 && bottomScreenBrightness <= 135)
					REG_BLDY = (0b0100 << 1);
			}
		}

		if (titleboxXmoveleft)
		{
			if (movetimer == 8)
			{
				//	if (showbubble && theme == 0) mmEffectEx(&snd_stop);
				needToPlayStopSound = true;
				startBorderZoomOut = true;
				titlewindowXpos[ms().secondaryDevice] -= 1;
				movetimer++;
			}
			else if (movetimer < 8)
			{
				titleboxXpos[ms().secondaryDevice] -= (isScrolling ? titleboxXscrollspeed : titleboxXmovespeed[movetimer]);
				if (movetimer == 0 || movetimer == 2 || movetimer == 4 || movetimer == 6)
					titlewindowXpos[ms().secondaryDevice] -= 1;
				movetimer++;
			}
			else
			{
				buttonArrowTouched[0] = false;
				titleboxXmoveleft = false;
				movetimer = 0;
			}
		}
		else if (titleboxXmoveright)
		{
			if (movetimer == 8)
			{
				//	if (showbubble && theme == 0) mmEffectEx(&snd_stop);
				needToPlayStopSound = true;
				startBorderZoomOut = true;
				titlewindowXpos[ms().secondaryDevice] += 1;
				movetimer++;
			}
			else if (movetimer < 8)
			{
				titleboxXpos[ms().secondaryDevice] += (isScrolling ? titleboxXscrollspeed : titleboxXmovespeed[movetimer]);
				if (movetimer == 0 || movetimer == 2 || movetimer == 4 || movetimer == 6)
					titlewindowXpos[ms().secondaryDevice] += 1;
				movetimer++;
			}
			else
			{
				buttonArrowTouched[1] = false;
				titleboxXmoveright = false;
				movetimer = 0;
			}
		}

		if (redoDropDown && theme == 0)
		{
			for (int i = 0; i < 5; i++)
			{
				dropTime[i] = 0;
				dropSeq[i] = 0;
				dropSpeed[i] = 5;
				dropSpeedChange[i] = 0;
				titleboxYposDropDown[i] = -85 - 80;
			}
			allowedTitleboxForDropDown = 0;
			delayForTitleboxToDropDown = 0;
			dropDown = false;
			redoDropDown = false;
		}

		if (!whiteScreen && dropDown && theme == 0)
		{
			for (int i = 0; i <= allowedTitleboxForDropDown; i++)
			{
				if (dropSeq[i] == 0)
				{
					titleboxYposDropDown[i] += dropSpeed[i];
					if (titleboxYposDropDown[i] > 0)
						dropSeq[i] = 1;
				}
				else if (dropSeq[i] == 1)
				{
					titleboxYposDropDown[i] -= dropSpeed[i];
					dropTime[i]++;
					dropSpeedChange[i]++;
					if (dropTime[i] >= 15)
					{
						dropSpeedChange[i] = -1;
						dropSeq[i] = 2;
					}
					if (dropSpeedChange[i] == 2)
					{
						dropSpeed[i]--;
						if (dropSpeed[i] < 0)
							dropSpeed[i] = 0;
						dropSpeedChange[i] = -1;
					}
				}
				else if (dropSeq[i] == 2)
				{
					titleboxYposDropDown[i] += dropSpeed[i];
					if (titleboxYposDropDown[i] >= 0)
					{
						dropSeq[i] = 3;
						titleboxYposDropDown[i] = 0;
					}
					dropSpeedChange[i]++;
					if (dropSpeedChange[i] == 1)
					{
						dropSpeed[i]++;
						if (dropSpeed[i] > 4)
							dropSpeed[i] = 4;
						dropSpeedChange[i] = -1;
					}
				}
				else if (dropSeq[i] == 3)
				{
					titleboxYposDropDown[i] = 0;
				}
			}

			delayForTitleboxToDropDown++;
			if (delayForTitleboxToDropDown >= 5)
			{
				allowedTitleboxForDropDown++;
				if (allowedTitleboxForDropDown > 4)
					allowedTitleboxForDropDown = 4;
				delayForTitleboxToDropDown = 0;
			}
		}

		int bg_R = bottomScreenBrightness / 8;
		int bg_G = (bottomScreenBrightness / 8) - (3 * blfLevel);
		if (bg_G < 0)
			bg_G = 0;
		int bg_B = (bottomScreenBrightness / 8) - (6 * blfLevel);
		if (bg_B < 0)
			bg_B = 0;

		glColor(RGB15(bg_R, bg_G, bg_B));

		if (theme == 0)
		{
			int bipXpos = 27;
			glSprite(16 + titlewindowXpos[ms().secondaryDevice], 171, GL_FLIP_NONE, tex().scrollwindowImage());
			for (int i = 0; i < 40; i++)
			{
				if (i < spawnedtitleboxes)
				{
					if (bnrSysSettings[i])
					{
						glSprite(bipXpos, 178, GL_FLIP_NONE, &tex().bipsImage()[2]);
					}
					else
					{
						glSprite(bipXpos, 178, GL_FLIP_NONE, tex().bipsImage());
					}
				}
				else
					glSprite(bipXpos, 178, GL_FLIP_NONE, &tex().bipsImage()[1]);
				bipXpos += 5;
			}
			glSprite(16 + titlewindowXpos[ms().secondaryDevice], 171, GL_FLIP_NONE, &tex().buttonarrowImage()[2 + scrollWindowTouched]);
			glSprite(0, 171, GL_FLIP_NONE, &tex().buttonarrowImage()[0 + buttonArrowTouched[0]]);
			glSprite(224, 171, GL_FLIP_H, &tex().buttonarrowImage()[0 + buttonArrowTouched[1]]);
			glSprite(72 - titleboxXpos[ms().secondaryDevice], 81, GL_FLIP_NONE, tex().braceImage());
		}
		int spawnedboxXpos = 96;
		int iconXpos = 112;

		if (movingApp != -1)
		{
			if (movingAppIsDir)
			{
				if (theme == 1)
					glSprite(96, titleboxYpos - movingAppYpos, GL_FLIP_NONE, tex().folderImage());
				else
					glSprite(96, titleboxYpos - movingAppYpos + titleboxYposDropDown[movingApp % 5], GL_FLIP_NONE, tex().folderImage());
			}
			else
			{
				if (!bnrSysSettings[movingApp])
				{
					if (theme == 1)
					{
						glSprite(96, titleboxYpos - movingAppYpos, GL_FLIP_NONE, tex().boxfullImage());
					}
					else
					{
						glSprite(96, titleboxYpos - movingAppYpos + titleboxYposDropDown[movingApp % 5], GL_FLIP_NONE, &tex().boxfullImage()[0]);
					}
				}
				if (bnrSysSettings[movingApp])
					glSprite(96, (titleboxYpos - 1) - movingAppYpos + titleboxYposDropDown[movingApp % 5], GL_FLIP_NONE, &tex().settingsImage()[1]);
				else if (bnrRomType[movingApp] == 7)
					drawIconSNES(112, (titleboxYpos + 12) - movingAppYpos + titleboxYposDropDown[movingApp % 5]);
				else if (bnrRomType[movingApp] == 6)
					drawIconMD(112, (titleboxYpos + 12) - movingAppYpos + titleboxYposDropDown[movingApp % 5]);
				else if (bnrRomType[movingApp] == 5)
					drawIconGG(112, (titleboxYpos + 12) - movingAppYpos + titleboxYposDropDown[movingApp % 5]);
				else if (bnrRomType[movingApp] == 4)
					drawIconSMS(112, (titleboxYpos + 12) - movingAppYpos + titleboxYposDropDown[movingApp % 5]);
				else if (bnrRomType[movingApp] == 3)
					drawIconNES(112, (titleboxYpos + 12) - movingAppYpos + titleboxYposDropDown[movingApp % 5]);
				else if (bnrRomType[movingApp] == 2)
					drawIconGBC(112, (titleboxYpos + 12) - movingAppYpos + titleboxYposDropDown[movingApp % 5]);
				else if (bnrRomType[movingApp] == 1)
					drawIconGB(112, (titleboxYpos + 12) - movingAppYpos + titleboxYposDropDown[movingApp % 5]);
				else
					drawIcon(112, (titleboxYpos + 12) - movingAppYpos + titleboxYposDropDown[movingApp % 5], -1);
			}
		}

		for (int i = 0; i < 40; i++)
		{
			if (theme == 0)
			{
				moveIconClose(i);
			}
			else
			{
				movecloseXpos = 0;
			}
			if (i >= ms().cursorPosition[ms().secondaryDevice] - 3 && i <= ms().cursorPosition[ms().secondaryDevice] + 3)
			{
				if (i < spawnedtitleboxes)
				{
					if (isDirectory[i])
					{
						if (movingApp != -1)
						{
							int j = i;
							if (i > movingApp - (ms().pagenum[ms().secondaryDevice] * 40))
								j--;
							if (theme == 1)
								glSprite((j * 2496 / 39) + 128 - titleboxXpos[ms().secondaryDevice], titleboxYpos, GL_FLIP_NONE, tex().folderImage());
							else
								glSprite((j * 2496 / 39) + 128 - titleboxXpos[ms().secondaryDevice], (titleboxYpos - 3) + titleboxYposDropDown[i % 5], GL_FLIP_NONE, tex().folderImage());
						}
						else
						{
							if (theme == 1)
								glSprite(spawnedboxXpos - titleboxXpos[ms().secondaryDevice] + movecloseXpos, titleboxYpos, GL_FLIP_NONE, tex().folderImage());
							else
								glSprite(spawnedboxXpos - titleboxXpos[ms().secondaryDevice] + movecloseXpos, (titleboxYpos - 3) + titleboxYposDropDown[i % 5], GL_FLIP_NONE, tex().folderImage());
						}
					}
					else if (!applaunchprep || ms().cursorPosition[ms().secondaryDevice] != i)
					{ // Only draw the icon if we're not launching the selcted app
						if (movingApp != -1)
						{
							int j = i;
							if (i > movingApp - (ms().pagenum[ms().secondaryDevice] * 40))
								j--;
							if (j == -1)
								continue;
							if (!bnrSysSettings[i])
							{
								if (theme == 1)
								{
									glSprite((j * 2496 / 39) + 128 - titleboxXpos[ms().secondaryDevice], titleboxYpos, GL_FLIP_NONE, tex().boxfullImage());
								}
								else
								{
									glSprite((j * 2496 / 39) + 128 - titleboxXpos[ms().secondaryDevice], titleboxYpos + titleboxYposDropDown[i % 5], GL_FLIP_NONE, &tex().boxfullImage()[0]);
								}
							}
							if (bnrSysSettings[i])
								glSprite((j * 2496 / 39) + 128 - titleboxXpos[ms().secondaryDevice], (titleboxYpos - 1) + titleboxYposDropDown[i % 5], GL_FLIP_NONE, &tex().settingsImage()[1]);
							else if (bnrRomType[i] == 7)
								drawIconSNES((j * 2496 / 39) + 144 - titleboxXpos[ms().secondaryDevice], (titleboxYpos + 12) + titleboxYposDropDown[i % 5]);
							else if (bnrRomType[i] == 6)
								drawIconMD((j * 2496 / 39) + 144 - titleboxXpos[ms().secondaryDevice], (titleboxYpos + 12) + titleboxYposDropDown[i % 5]);
							else if (bnrRomType[i] == 5)
								drawIconGG((j * 2496 / 39) + 144 - titleboxXpos[ms().secondaryDevice], (titleboxYpos + 12) + titleboxYposDropDown[i % 5]);
							else if (bnrRomType[i] == 4)
								drawIconSMS((j * 2496 / 39) + 144 - titleboxXpos[ms().secondaryDevice], (titleboxYpos + 12) + titleboxYposDropDown[i % 5]);
							else if (bnrRomType[i] == 3)
								drawIconNES((j * 2496 / 39) + 144 - titleboxXpos[ms().secondaryDevice], (titleboxYpos + 12) + titleboxYposDropDown[i % 5]);
							else if (bnrRomType[i] == 2)
								drawIconGBC((j * 2496 / 39) + 144 - titleboxXpos[ms().secondaryDevice], (titleboxYpos + 12) + titleboxYposDropDown[i % 5]);
							else if (bnrRomType[i] == 1)
								drawIconGB((j * 2496 / 39) + 144 - titleboxXpos[ms().secondaryDevice], (titleboxYpos + 12) + titleboxYposDropDown[i % 5]);
							else
								drawIcon((j * 2496 / 39) + 144 - titleboxXpos[ms().secondaryDevice], (titleboxYpos + 12) + titleboxYposDropDown[i % 5], i);
						}
						else
						{
							if (!bnrSysSettings[i])
							{
								if (theme == 1)
								{
									glSprite(spawnedboxXpos - titleboxXpos[ms().secondaryDevice], titleboxYpos, GL_FLIP_NONE, tex().boxfullImage());
								}
								else
								{
									glSprite(spawnedboxXpos - titleboxXpos[ms().secondaryDevice] + movecloseXpos, titleboxYpos + titleboxYposDropDown[i % 5], GL_FLIP_NONE, &tex().boxfullImage()[0]);
								}
							}
							if (bnrSysSettings[i])
								glSprite(spawnedboxXpos - titleboxXpos[ms().secondaryDevice] + movecloseXpos, (titleboxYpos - 1) + titleboxYposDropDown[i % 5], GL_FLIP_NONE, &tex().settingsImage()[1]);
							else if (bnrRomType[i] == 7)
								drawIconSNES(iconXpos - titleboxXpos[ms().secondaryDevice] + movecloseXpos, (titleboxYpos + 12) + titleboxYposDropDown[i % 5]);
							else if (bnrRomType[i] == 6)
								drawIconMD(iconXpos - titleboxXpos[ms().secondaryDevice] + movecloseXpos, (titleboxYpos + 12) + titleboxYposDropDown[i % 5]);
							else if (bnrRomType[i] == 5)
								drawIconGG(iconXpos - titleboxXpos[ms().secondaryDevice] + movecloseXpos, (titleboxYpos + 12) + titleboxYposDropDown[i % 5]);
							else if (bnrRomType[i] == 4)
								drawIconSMS(iconXpos - titleboxXpos[ms().secondaryDevice] + movecloseXpos, (titleboxYpos + 12) + titleboxYposDropDown[i % 5]);
							else if (bnrRomType[i] == 3)
								drawIconNES(iconXpos - titleboxXpos[ms().secondaryDevice] + movecloseXpos, (titleboxYpos + 12) + titleboxYposDropDown[i % 5]);
							else if (bnrRomType[i] == 2)
								drawIconGBC(iconXpos - titleboxXpos[ms().secondaryDevice] + movecloseXpos, (titleboxYpos + 12) + titleboxYposDropDown[i % 5]);
							else if (bnrRomType[i] == 1)
								drawIconGB(iconXpos - titleboxXpos[ms().secondaryDevice] + movecloseXpos, (titleboxYpos + 12) + titleboxYposDropDown[i % 5]);
							else
								drawIcon(iconXpos - titleboxXpos[ms().secondaryDevice] + movecloseXpos, (titleboxYpos + 12) + titleboxYposDropDown[i % 5], i);
						}
					}
				}
				else
				{
					// Empty box
					if (movingApp != -1)
					{
						if (theme == 1)
						{
							glSprite(((i - 1) * 2496 / 39) + 128 - titleboxXpos[ms().secondaryDevice], titleboxYpos + titleboxYposDropDown[i % 5], GL_FLIP_NONE, tex().boxemptyImage());
						}
						else
						{
							glSprite(((i - 1) * 2496 / 39) + 128 - titleboxXpos[ms().secondaryDevice], titleboxYpos + titleboxYposDropDown[i % 5], GL_FLIP_NONE, &tex().boxfullImage()[1 & 63]);
						}
					}
					else
					{
						if (theme == 1)
						{
							glSprite(spawnedboxXpos - titleboxXpos[ms().secondaryDevice], titleboxYpos + titleboxYposDropDown[i % 5], GL_FLIP_NONE, tex().boxemptyImage());
						}
						else
						{
							glSprite(spawnedboxXpos - titleboxXpos[ms().secondaryDevice] + movecloseXpos, titleboxYpos + titleboxYposDropDown[i % 5], GL_FLIP_NONE, &tex().boxfullImage()[1 & 63]);
						}
					}
				}
			}
			spawnedboxXpos += 64;
			iconXpos += 64;
		}
		if (theme == 0)
		{
			glSprite(spawnedboxXpos + 10 - titleboxXpos[ms().secondaryDevice], 81, GL_FLIP_H, tex().braceImage());
		}

		if (movingApp != -1 && !theme && showMovingArrow)
		{
			if (movingArrowYdirection)
			{
				movingArrowYpos += 0.33;
				if (movingArrowYpos > 67)
					movingArrowYdirection = false;
			}
			else
			{
				movingArrowYpos -= 0.33;
				if (movingArrowYpos < 59)
					movingArrowYdirection = true;
			}
			glSprite(115, movingArrowYpos, GL_FLIP_NONE, tex().movingArrowImage());
		}
		// Top icons for 3DS theme
		if (theme == 1)
		{
			int topIconXpos = 116;
			if (isDSiMode() && sdFound())
			{
				//for (int i = 0; i < 4; i++) {
				topIconXpos -= 14;
				//}
				if (ms().secondaryDevice)
				{
					glSprite(topIconXpos, 1, GL_FLIP_NONE, &tex().smallCartImage()[2]); // SD card
				}
				else
				{
					glSprite(topIconXpos, 1, GL_FLIP_NONE, &tex().smallCartImage()[(REG_SCFG_MC == 0x11) ? 1 : 0]); // Slot-1 card
				}
				topIconXpos += 28;
				drawSmallIconGBA(topIconXpos, 1); // GBARunner2
			}
			else
			{
				//for (int i = 0; i < 3; i++) {
				//	topIconXpos -= 14;
				//}
				if (ms().useGbarunner)
				{
					drawSmallIconGBA(topIconXpos, 1); // GBARunner2
				}
				else
				{
					glSprite(topIconXpos, 1, GL_FLIP_NONE, &tex().smallCartImage()[3]); // GBA Mode
				}
			}
			glSprite(0, 0, GL_FLIP_NONE, &tex().cornerButtonImage()[0]);
			if (!sys().isRegularDS())
				glSprite(256 - 44, 0, GL_FLIP_NONE, &tex().cornerButtonImage()[1]);
		}

		if (applaunchprep && theme == 0)
		{

			if (isDirectory[ms().cursorPosition[ms().secondaryDevice]])
			{
				glSprite(96, 87 - titleboxYmovepos, GL_FLIP_NONE, tex().folderImage());
			}
			else
			{
				if (!bnrSysSettings[ms().cursorPosition[ms().secondaryDevice]])
				{
					glSprite(96, 84 - titleboxYmovepos, GL_FLIP_NONE, tex().boxfullImage());
				}
				if (bnrSysSettings[ms().cursorPosition[ms().secondaryDevice]])
					glSprite(84, 83 - titleboxYmovepos, GL_FLIP_NONE, &tex().settingsImage()[1]);
				else if (bnrRomType[ms().cursorPosition[ms().secondaryDevice]] == 7)
					drawIconSNES(112, 96 - titleboxYmovepos);
				else if (bnrRomType[ms().cursorPosition[ms().secondaryDevice]] == 6)
					drawIconMD(112, 96 - titleboxYmovepos);
				else if (bnrRomType[ms().cursorPosition[ms().secondaryDevice]] == 5)
					drawIconGG(112, 96 - titleboxYmovepos);
				else if (bnrRomType[ms().cursorPosition[ms().secondaryDevice]] == 4)
					drawIconSMS(112, 96 - titleboxYmovepos);
				else if (bnrRomType[ms().cursorPosition[ms().secondaryDevice]] == 3)
					drawIconNES(112, 96 - titleboxYmovepos);
				else if (bnrRomType[ms().cursorPosition[ms().secondaryDevice]] == 2)
					drawIconGBC(112, 96 - titleboxYmovepos);
				else if (bnrRomType[ms().cursorPosition[ms().secondaryDevice]] == 1)
					drawIconGB(112, 96 - titleboxYmovepos);
				else
					drawIcon(112, 96 - titleboxYmovepos, ms().cursorPosition[ms().secondaryDevice]);
			}
			// Draw dots after selecting a game/app
			for (int i = 0; i < 11; i++)
			{
				glSprite(76 + launchDotX[i], 69 + launchDotY[i], GL_FLIP_NONE, &tex().launchdotImage()[(launchDotFrame[i]) & 15]);
				if (launchDotX[i] == 0)
					launchDotXMove[i] = true;
				if (launchDotX[i] == 88)
					launchDotXMove[i] = false;
				if (launchDotY[i] == 0)
					launchDotYMove[i] = true;
				if (launchDotY[i] == 88)
					launchDotYMove[i] = false;
				if (launchDotXMove[i] == false)
				{
					launchDotX[i]--;
				}
				else if (launchDotXMove[i] == true)
				{
					launchDotX[i]++;
				}
				if (launchDotYMove[i] == false)
				{
					launchDotY[i]--;
				}
				else if (launchDotYMove[i] == true)
				{
					launchDotY[i]++;
				}
			}
			titleboxYmovepos += 5;
		}
		if (showSTARTborder)
		{
			if (theme == 1)
			{
				glSprite(96, 92, GL_FLIP_NONE, &tex().startbrdImage()[startBorderZoomAnimSeq[startBorderZoomAnimNum] & 63]);
				glSprite(96 + 32, 92, GL_FLIP_H, &tex().startbrdImage()[startBorderZoomAnimSeq[startBorderZoomAnimNum] & 63]);
				if (bnrWirelessIcon[ms().cursorPosition[ms().secondaryDevice]] > 0)
					glSprite(96, 92, GL_FLIP_NONE, &tex().wirelessIcons()[(bnrWirelessIcon[ms().cursorPosition[ms().secondaryDevice]] - 1) & 31]);
			}
			else if (!isScrolling)
			{
				if (showbubble && theme == 0 && needToPlayStopSound && waitForNeedToPlayStopSound == 0)
				{
					mmEffectEx(&snd_stop);
					waitForNeedToPlayStopSound = 1;
					needToPlayStopSound = false;
				}
				glSprite(96, 81, GL_FLIP_NONE, &tex().startbrdImage()[startBorderZoomAnimSeq[startBorderZoomAnimNum] & 79]);
				glSprite(96 + 32, 81, GL_FLIP_H, &tex().startbrdImage()[startBorderZoomAnimSeq[startBorderZoomAnimNum] & 79]);
				if (bnrWirelessIcon[ms().cursorPosition[ms().secondaryDevice]] > 0)
					glSprite(96, 81, GL_FLIP_NONE, &tex().wirelessIcons()[(bnrWirelessIcon[ms().cursorPosition[ms().secondaryDevice]] - 1) & 31]);
			}
		}

		// Refresh the background layer.
		if (showbubble)
			drawBubble(tex().bubbleImage());
		if (showSTARTborder && theme == 0 && !isScrolling)
			glSprite(96, 144, GL_FLIP_NONE, &tex().startImage()[setLanguage]);

		glColor(RGB15(31, 31 - (3 * blfLevel), 31 - (6 * blfLevel)));
		if (dbox_Ypos != -192)
		{
			// Draw the dialog box.
			drawDbox();
			if (dbox_showIcon && !isDirectory[ms().cursorPosition[ms().secondaryDevice]])
			{
				drawIcon(24, dbox_Ypos + 24, ms().cursorPosition[ms().secondaryDevice]);
			}
			if (dbox_selectMenu)
			{
				int selIconYpos = 96;
				if (isDSiMode() && sdFound())
				{
					for (int i = 0; i < 4; i++)
					{
						selIconYpos -= 14;
					}
				}
				else
				{
					for (int i = 0; i < 3; i++)
					{
						selIconYpos -= 14;
					}
				}
				if (!sys().isRegularDS())
				{
					glSprite(32, dbox_Ypos + selIconYpos, GL_FLIP_NONE, &tex().cornerButtonImage()[1]); // System Menu
					selIconYpos += 28;
				}
				glSprite(32, dbox_Ypos + selIconYpos, GL_FLIP_NONE, &tex().cornerButtonImage()[0]); // Settings
				selIconYpos += 28;
				if (isDSiMode() && sdFound())
				{
					if (ms().secondaryDevice)
					{
						glSprite(32, dbox_Ypos + selIconYpos, GL_FLIP_NONE, &tex().smallCartImage()[2]); // SD card
					}
					else
					{
						glSprite(32, dbox_Ypos + selIconYpos, GL_FLIP_NONE, &tex().smallCartImage()[(REG_SCFG_MC == 0x11) ? 1 : 0]); // Slot-1 card
					}
					selIconYpos += 28;
				}
				if (ms().useGbarunner)
				{
					drawSmallIconGBA(32, dbox_Ypos + selIconYpos); // GBARunner2
				}
				else
				{
					glSprite(32, dbox_Ypos + selIconYpos, GL_FLIP_NONE, &tex().smallCartImage()[3]); // GBA Mode
				}
			}
		}
		// Show button_arrowPals (debug feature)
		/*for (int i = 0; i < 16; i++) {
				for (int i2 = 0; i2 < 16; i2++) {
					glBox(i2,i,i2,i,button_arrowPals[(i*16)+i2]);
				}
			}*/
		if (whiteScreen)
		{
			glBoxFilled(0, 0, 256, 192, RGB15(31, 31 - (3 * blfLevel), 31 - (6 * blfLevel)));
			if (showProgressIcon)
				glSprite(224, 152, GL_FLIP_NONE, &tex().progressImage()[progressAnimNum]);
		}

		if (vblankRefreshCounter >= REFRESH_EVERY_VBLANKS)
		{
			if (showdialogbox && dbox_Ypos == -192)
			{
				// Reload the dialog box palettes here...
				reloadDboxPalette();
			}
			else if (!showdialogbox)
			{
				if (theme == 1)
				{
					// on other themes, reloadDboxPalettes also reloads cornerbutton palettes
					tex().reloadPal3dsCornerButton();
				}
				reloadIconPalettes();
				reloadFontPalettes();
			}
			vblankRefreshCounter = 0;
		}
		else
		{
			vblankRefreshCounter++;
		}
		updateText(false);
		//}
	}
	glEnd2D();
	GFX_FLUSH = 0;

	colonTimer++;

	if (showProgressIcon)
	{
		progressAnimDelay++;
		if (progressAnimDelay == 3)
		{
			progressAnimNum++;
			if (progressAnimNum > 7)
				progressAnimNum = 0;
			progressAnimDelay = 0;
		}
	}
	if (!whiteScreen)
	{
		// Playback animated icons
		for (int i = 0; i < 41; i++)
		{
			if (bnriconisDSi[i] == true)
			{
				playBannerSequence(i);
			}
		}
	}

	if (theme == 1)
	{
		startBorderZoomAnimDelay++;
		if (startBorderZoomAnimDelay == 8)
		{
			startBorderZoomAnimNum++;
			if (startBorderZoomAnimSeq[startBorderZoomAnimNum] == 0)
			{
				startBorderZoomAnimNum = 0;
			}
			startBorderZoomAnimDelay = 0;
		}
	}
	else if (startBorderZoomOut)
	{
		startBorderZoomAnimNum++;
		if (startBorderZoomAnimSeq[startBorderZoomAnimNum] == 0)
		{
			startBorderZoomAnimNum = 0;
			startBorderZoomOut = false;
		}
	}
	else
	{
		startBorderZoomAnimNum = 0;
	}
	if (applaunchprep && theme == 0 && launchDotDoFrameChange)
	{
		launchDotFrame[0]--;
		if (launchDotCurrentChangingFrame >= 1)
			launchDotFrame[1]--;
		if (launchDotCurrentChangingFrame >= 2)
			launchDotFrame[2]--;
		if (launchDotCurrentChangingFrame >= 3)
			launchDotFrame[3]--;
		if (launchDotCurrentChangingFrame >= 4)
			launchDotFrame[4]--;
		if (launchDotCurrentChangingFrame >= 5)
			launchDotFrame[5]--;
		if (launchDotCurrentChangingFrame >= 6)
			launchDotFrame[6]--;
		if (launchDotCurrentChangingFrame >= 7)
			launchDotFrame[7]--;
		if (launchDotCurrentChangingFrame >= 8)
			launchDotFrame[8]--;
		if (launchDotCurrentChangingFrame >= 9)
			launchDotFrame[9]--;
		if (launchDotCurrentChangingFrame >= 10)
			launchDotFrame[10]--;
		if (launchDotCurrentChangingFrame >= 11)
			launchDotFrame[11]--;
		for (int i = 0; i < 12; i++)
		{
			if (launchDotFrame[i] < 0)
				launchDotFrame[i] = 0;
		}
		launchDotCurrentChangingFrame++;
		if (launchDotCurrentChangingFrame > 11)
			launchDotCurrentChangingFrame = 11;
	}
	if (applaunchprep && theme == 0)
		launchDotDoFrameChange = !launchDotDoFrameChange;
	bottomBgRefresh(); // Refresh the background image on vblank
}

void clearBmpScreen()
{
	beginBgSubModify();
	u16 val = 0xFFFF;
	for (int i = 0; i < 256 * 192; i++)
	{
		bgSubBuffer[i] = ((val >> 10) & 31) | (val & (31 - 3 * blfLevel) << 5) | (val & (31 - 6 * blfLevel)) << 10 | BIT(15);
	}
	commitBgSubModify();
}

void loadBoxArt(const char *filename)
{
	FILE *file = fopen(filename, "rb");
	if (!file)
		file = fopen("nitro:/graphics/boxart_unknown.bmp", "rb");

	if (file)
	{
		// Start loading
		beginBgSubModify();
		fseek(file, 0xe, SEEK_SET);
		u8 pixelStart = (u8)fgetc(file) + 0xe;
		fseek(file, pixelStart, SEEK_SET);
		fread(bmpImageBuffer, 2, 0x7800, file);
		u16 *src = bmpImageBuffer;
		int x = 64;
		int y = 40 + 114;
		for (int i = 0; i < 128 * 115; i++)
		{
			if (x >= 64 + 128)
			{
				x = 64;
				y--;
			}
			u16 val = *(src++);
			bgSubBuffer[y * 256 + x] = convertToDsBmp(val);
			x++;
		}
		commitBgSubModify();
	}

	fclose(file);
}

static int loadedVolumeImage = -1;

static const char *volume4ImagePath;
static const char *volume3ImagePath;
static const char *volume2ImagePath;
static const char *volume1ImagePath;
static const char *volume0ImagePath;

void setVolumeImagePaths(void)
{
	switch (subtheme)
	{
	case 0:
		volume4ImagePath = (theme == 1) ? "nitro:/graphics/3ds_volume4.bmp" : "nitro:/graphics/dark_volume4.bmp";
		volume3ImagePath = (theme == 1) ? "nitro:/graphics/3ds_volume3.bmp" : "nitro:/graphics/dark_volume3.bmp";
		volume2ImagePath = (theme == 1) ? "nitro:/graphics/3ds_volume2.bmp" : "nitro:/graphics/dark_volume2.bmp";
		volume1ImagePath = (theme == 1) ? "nitro:/graphics/3ds_volume1.bmp" : "nitro:/graphics/dark_volume1.bmp";
		volume0ImagePath = (theme == 1) ? "nitro:/graphics/3ds_volume0.bmp" : "nitro:/graphics/dark_volume0.bmp";
		break;
	case 1:
	default:
		volume4ImagePath = "nitro:/graphics/volume4.bmp";
		volume3ImagePath = "nitro:/graphics/volume3.bmp";
		volume2ImagePath = "nitro:/graphics/volume2.bmp";
		volume1ImagePath = "nitro:/graphics/volume1.bmp";
		volume0ImagePath = "nitro:/graphics/volume0.bmp";
		break;
	case 2:
		volume4ImagePath = "nitro:/graphics/red_volume4.bmp";
		volume3ImagePath = "nitro:/graphics/red_volume3.bmp";
		volume2ImagePath = "nitro:/graphics/red_volume2.bmp";
		volume1ImagePath = "nitro:/graphics/red_volume1.bmp";
		volume0ImagePath = "nitro:/graphics/red_volume0.bmp";
		break;
	case 3:
		volume4ImagePath = "nitro:/graphics/blue_volume4.bmp";
		volume3ImagePath = "nitro:/graphics/blue_volume3.bmp";
		volume2ImagePath = "nitro:/graphics/blue_volume2.bmp";
		volume1ImagePath = "nitro:/graphics/blue_volume1.bmp";
		volume0ImagePath = "nitro:/graphics/blue_volume0.bmp";
		break;
	case 4:
		volume4ImagePath = "nitro:/graphics/green_volume4.bmp";
		volume3ImagePath = "nitro:/graphics/green_volume3.bmp";
		volume2ImagePath = "nitro:/graphics/green_volume2.bmp";
		volume1ImagePath = "nitro:/graphics/green_volume1.bmp";
		volume0ImagePath = "nitro:/graphics/green_volume0.bmp";
		break;
	case 5:
		volume4ImagePath = "nitro:/graphics/yellow_volume4.bmp";
		volume3ImagePath = "nitro:/graphics/yellow_volume3.bmp";
		volume2ImagePath = "nitro:/graphics/yellow_volume2.bmp";
		volume1ImagePath = "nitro:/graphics/yellow_volume1.bmp";
		volume0ImagePath = "nitro:/graphics/yellow_volume0.bmp";
		break;
	case 6:
		volume4ImagePath = "nitro:/graphics/pink_volume4.bmp";
		volume3ImagePath = "nitro:/graphics/pink_volume3.bmp";
		volume2ImagePath = "nitro:/graphics/pink_volume2.bmp";
		volume1ImagePath = "nitro:/graphics/pink_volume1.bmp";
		volume0ImagePath = "nitro:/graphics/pink_volume0.bmp";
		break;
	case 7:
		volume4ImagePath = "nitro:/graphics/purple_volume4.bmp";
		volume3ImagePath = "nitro:/graphics/purple_volume3.bmp";
		volume2ImagePath = "nitro:/graphics/purple_volume2.bmp";
		volume1ImagePath = "nitro:/graphics/purple_volume1.bmp";
		volume0ImagePath = "nitro:/graphics/purple_volume0.bmp";
		break;
	}
}

void loadVolumeImage(void)
{
	if (!isDSiMode())
		return;

	u8 volumeLevel = *(u8 *)(0x023FF000);
	const char *volumeImagePath;

	if (volumeLevel >= 0x1C && volumeLevel < 0x20)
	{
		if (loadedVolumeImage == 4)
			return;
		volumeImagePath = volume4ImagePath;
		loadedVolumeImage = 4;
	}
	else if (volumeLevel >= 0x11 && volumeLevel < 0x1C)
	{
		if (loadedVolumeImage == 3)
			return;
		volumeImagePath = volume3ImagePath;
		loadedVolumeImage = 3;
	}
	else if (volumeLevel >= 0x07 && volumeLevel < 0x11)
	{
		if (loadedVolumeImage == 2)
			return;
		volumeImagePath = volume2ImagePath;
		loadedVolumeImage = 2;
	}
	else if (volumeLevel > 0x00 && volumeLevel < 0x07)
	{
		if (loadedVolumeImage == 1)
			return;
		volumeImagePath = volume1ImagePath;
		loadedVolumeImage = 1;
	}
	else if (volumeLevel == 0x00)
	{
		if (loadedVolumeImage == 0)
			return;
		volumeImagePath = volume0ImagePath;
		loadedVolumeImage = 0;
	}
	else
	{
		return;
	}

	beginBgSubModify();

	const u16 *src = tex().volumeTexture(loadedVolumeImage)->texture();
	int x = 4;
	int y = 5 + 11;
	for (int i = 0; i < 18 * 12; i++)
	{
		if (x >= 4 + 18)
		{
			x = 4;
			y--;
		}
		u16 val = *(src++);
		if (val != 0x7C1F)
		{ // Do not render magneta pixel
			bgSubBuffer[y * 256 + x] = convertToDsBmp(val);
		}
		x++;
	}
	commitBgSubModify();
}

static int loadedBatteryImage = -1;


void loadBatteryImage(void)
{
	u8 batteryLevel = *(u8 *)(0x023FF001);

	if (isDSiMode())
	{
		if (batteryLevel & BIT(7))
		{
			if (loadedBatteryImage == 7)
				return;
			loadedBatteryImage = 7;
		}
		else if (batteryLevel == 0xF)
		{
			if (loadedBatteryImage == 4)
				return;
			loadedBatteryImage = 4;
		}
		else if (batteryLevel == 0xB)
		{
			if (loadedBatteryImage == 3)
				return;
			loadedBatteryImage = 3;
		}
		else if (batteryLevel == 0x7)
		{
			if (loadedBatteryImage == 2)
				return;
			loadedBatteryImage = 2;
		}
		else if (batteryLevel == 0x3 || batteryLevel == 0x1)
		{
			if (loadedBatteryImage == 1)
				return;
			loadedBatteryImage = 1;
		}
		else
		{
			return;
		}
	}
	else
	{
		if (batteryLevel & BIT(0))
		{
			if (loadedBatteryImage == 1)
				return;
			loadedBatteryImage = 1;
		}
		else
		{
			if (loadedBatteryImage == 0)
				return;
			loadedBatteryImage = 0;
		}
	}

	// Start loading
	beginBgSubModify();
	const u16 *src = tex().batteryTexture(loadedBatteryImage, isDSiMode(), sys().isRegularDS())->texture();
	int x = 235;
	int y = 5 + 10;
	for (int i = 0; i < 18 * 11; i++)
	{
		if (x >= 235 + 18)
		{
			x = 235;
			y--;
		}
		u16 val = *(src++);
		if (val != 0x7C1F)
		{ // Do not render magneta pixel
			bgSubBuffer[y * 256 + x] = convertToDsBmp(val);
		}
		x++;
	}
	commitBgSubModify();
}

void loadPhotoList()
{
	DIR *dir;
	struct dirent *ent;
	std::string photoDir;
	bool dirOpen = false;
	std::string dirPath;
	if ((dir = opendir("sd:/_nds/TWiLightMenu/dsimenu/photos/")) != NULL)
	{
		dirOpen = true;
		dirPath = "sd:/_nds/TWiLightMenu/dsimenu/photos/";
	}
	else if ((dir = opendir("fat:/_nds/TWiLightMenu/dsimenu/photos/")) != NULL)
	{
		dirOpen = true;
		dirPath = "fat:/_nds/TWiLightMenu/dsimenu/photos/";
	}

	if (dirOpen)
	{
		/* print all the files and directories within directory */
		while ((ent = readdir(dir)) != NULL)
		{

			photoDir = ent->d_name;
			if (photoDir == ".." || photoDir == "..." || photoDir == "." || photoDir.substr(0, 2) == "._" || photoDir.substr(photoDir.find_last_of(".") + 1) != "bmp")
				continue;

			// Reallocation here, but prevents our vector from being filled with garbage
			photoList.emplace_back(dirPath + photoDir);
		}
		closedir(dir);
		photoPath = photoList[rand() / ((RAND_MAX + 1u) / photoList.size())];
	}
}

void loadPhoto()
{
	FILE *file = fopen(photoPath.c_str(), "rb");
	if (!file)
		file = fopen("nitro:/graphics/photo_default.bmp", "rb");

	if (file)
	{
		beginBgSubModify();
		// Start loading
		fseek(file, 0xe, SEEK_SET);
		u8 pixelStart = (u8)fgetc(file) + 0xe;
		fseek(file, pixelStart, SEEK_SET);
		fread(bmpImageBuffer, 2, 0x10000, file);
		u16 *src = bmpImageBuffer;
		int x = 24;
		int y = 24 + 155;
		for (int i = 0; i < 208 * 156; i++)
		{
			if (x >= 24 + 208)
			{
				x = 24;
				y--;
			}
			u16 val = *(src++);
			bgSubBuffer[y * 256 + x] = convertToDsBmp(val);
			x++;
		}
		commitBgSubModify();
	}

	fclose(file);
}

// Load photo without overwriting shoulder button images
void loadPhotoPart()
{
	FILE *file = fopen(photoPath.c_str(), "rb");
	if (!file)
		file = fopen("nitro:/graphics/photo_default.bmp", "rb");

	if (file)
	{
		// Start loading
		beginBgSubModify();
		fseek(file, 0xe, SEEK_SET);
		u8 pixelStart = (u8)fgetc(file) + 0xe;
		fseek(file, pixelStart, SEEK_SET);
		fread(bmpImageBuffer, 2, 0x10000, file);
		u16 *src = bmpImageBuffer;
		int x = 24;
		int y = 24 + 155;
		for (int i = 0; i < 208 * 156; i++)
		{
			if (x >= 24 + 208)
			{
				x = 24;
				y--;
			}
			u16 val = *(src++);
			if (y <= 24 + 147)
			{
				bgSubBuffer[y * 256 + x] = convertToDsBmp(val);
			}
			x++;
		}
		commitBgSubModify();
	}

	fclose(file);
}

void drawTopBg()
{

	beginBgSubModify();
	const u16 *src = tex().topBackgroundTexture()->texture();
	int x = 0;
	int y = 191;
	for (int i = 0; i < 256 * 192; i++)
	{
		if (x >= 256)
		{
			x = 0;
			y--;
		}
		u16 val = *(src++);
		if (val != 0xFC1F)
		{ // Do not render magneta pixel
			bgSubBuffer[y * 256 + x] = convertToDsBmp(val);
		}
		x++;
	}
	commitBgSubModify();
}

// Load .bmp file without overwriting shoulder button images or username
void loadBMPPart(const char *filename)
{
	FILE *file = fopen(filename, "rb");

	if (file)
	{
		// Start loading
		beginBgSubModify();
		fseek(file, 0xe, SEEK_SET);
		u8 pixelStart = (u8)fgetc(file) + 0xe;
		fseek(file, pixelStart, SEEK_SET);
		fread(bmpImageBuffer, 2, 0x1A000, file);
		u16 *src = bmpImageBuffer;
		int x = 0;
		int y = 191;
		for (int i = 0; i < 256 * 192; i++)
		{
			if (x >= 256)
			{
				x = 0;
				y--;
			}
			u16 val = *(src++);
			if (y >= 32 && y <= 167 && val != 0xFC1F)
			{
				bgSubBuffer[y * 256 + x] = convertToDsBmp(val);
			}
			x++;
		}
		commitBgSubModify();
	}

	fclose(file);
}

void loadShoulders()
{

	beginBgSubModify();
	const u16 *rightSrc = showRshoulder ? tex().rightShoulderTexture()->texture()
										: tex().rightShoulderGreyedTexture()->texture();

	const u16 *leftSrc = showLshoulder ? tex().leftShoulderTexture()->texture()
									   : tex().leftShoulderGreyedTexture()->texture();
	for (int y = 19; y >= 0; y--)
	{
		// Draw R Shoulders
		for (int i = 0; i < 78; i++)
		{
			u16 val = *(rightSrc++);
			if (val != 0xFC1F)
			{ // Do not render magneta pixel
				bgSubBuffer[(y + 172) * 256 + (i + 178)] = convertToDsBmp(val);
			}
		}
	}

	for (int y = 19; y >= 0; y--)
	{
		// Draw L Shoulders
		for (int i = 0; i < 78; i++)
		{
			u16 val = *(leftSrc++);
			if (val != 0xFC1F)
			{ // Do not render magneta pixel
				bgSubBuffer[(y + 172) * 256 + i] = convertToDsBmp(val);
			}
		}
	}

	commitBgSubModify();
}

/**
 * Get the index in the UV coordinate array where the letter appears
 */
unsigned int getTopFontSpriteIndex(const u16 letter)
{
	unsigned int spriteIndex = 0;
	long int left = 0;
	long int right = TOP_FONT_NUM_IMAGES;
	long int mid = 0;

	while (left <= right)
	{
		mid = left + ((right - left) / 2);
		if (top_utf16_lookup_table[mid] == letter)
		{
			spriteIndex = mid;
			break;
		}

		if (top_utf16_lookup_table[mid] < letter)
		{
			left = mid + 1;
		}
		else
		{
			right = mid - 1;
		}
	}
	return spriteIndex;
}

unsigned int getDateTimeFontSpriteIndex(const u16 letter)
{
	unsigned int spriteIndex = 0;
	long int left = 0;
	long int right = DATE_TIME_FONT_NUM_IMAGES;
	long int mid = 0;

	while (left <= right)
	{
		mid = left + ((right - left) / 2);
		if (date_time_utf16_lookup_table[mid] == letter)
		{
			spriteIndex = mid;
			break;
		}

		if (date_time_utf16_lookup_table[mid] < letter)
		{
			left = mid + 1;
		}
		else
		{
			right = mid - 1;
		}
	}
	return spriteIndex;
}

//   xrrrrrgggggbbbbb according to http://problemkaputt.de/gbatek.htm#dsvideobgmodescontrol
#define MASK_RB 0b0111110000011111
#define MASK_G 0b0000001111100000
#define MASK_MUL_RB 0b0111110000011111000000
#define MASK_MUL_G 0b0000001111100000000000
#define MAX_ALPHA 64 // 6bits+1 with rounding

/**
 * Adapted from https://stackoverflow.com/questions/18937701/
 * applies alphablending with the given
 * RGB555 foreground, RGB555 background, and alpha from 
 * 0 to 128 (0, 1.0).
 * The lower the alpha the more transparent, but
 * this function does not produce good results at the extremes 
 * (near 0 or 128).
 */
inline u16 alphablend(u16 fg, u16 bg, u8 alpha)
{

	// alpha for foreground multiplication
	// convert from 8bit to (6bit+1) with rounding
	// will be in [0..64] inclusive
	alpha = (alpha + 2) >> 2;
	// "beta" for background multiplication; (6bit+1);
	// will be in [0..64] inclusive
	u8 beta = MAX_ALPHA - alpha;
	// so (0..64)*alpha + (0..64)*beta always in 0..64

	return (u16)((
					 ((alpha * (u32)(fg & MASK_RB) + beta * (u32)(bg & MASK_RB)) & MASK_MUL_RB) |
					 ((alpha * (fg & MASK_G) + beta * (bg & MASK_G)) & MASK_MUL_G)) >>
				 5);
}

void topBgLoad()
{
	drawTopBg();

	// Load username
	char fontPath[64];
	FILE *file;
	int x = (isDSiMode() ? 28 : 4);

	for (int c = 0; c < 10; c++)
	{
		unsigned int charIndex = getTopFontSpriteIndex(usernameRendered[c]);
		// 42 characters per line.
		unsigned int texIndex = charIndex / 42;
		sprintf(fontPath, "nitro:/graphics/top_font/small_font_%u.bmp", texIndex);

		file = fopen(fontPath, "rb");

		if (file)
		{
			beginBgSubModify();
			// Start loading
			fseek(file, 0xe, SEEK_SET);
			u8 pixelStart = (u8)fgetc(file) + 0xe;
			fseek(file, pixelStart, SEEK_SET);
			for (int y = 15; y >= 0; y--)
			{
				u16 buffer[512];
				fread(buffer, 2, 0x200, file);
				u16 *src = buffer + (top_font_texcoords[0 + (4 * charIndex)]);

				for (u16 i = 0; i < top_font_texcoords[2 + (4 * charIndex)]; i++)
				{
					u16 val = *(src++);
					u16 bg = bgSubBuffer[(y + 2) * 256 + (i + x)]; //grab the background pixel
					// Apply palette here.

					// Magic numbers were found by dumping val to stdout
					// on case default.
					switch (val)
					{
					// #ff00ff
					case 0xFC1F:
						break;
					// #414141
					case 0xA108:
						val = bmpPal_topSmallFont[1 + ((PersonalData->theme) * 16)];
						break;
					case 0xC210:
						// blend the colors with the background to make it look better.
						val = alphablend(bmpPal_topSmallFont[2 + ((PersonalData->theme) * 16)], bg, 48);
						break;
					case 0xDEF7:
						val = alphablend(bmpPal_topSmallFont[3 + ((PersonalData->theme) * 16)], bg, 64);
					default:
						break;
					}
					if (val != 0xFC1F)
					{ // Do not render magneta pixel
						bgSubBuffer[(y + 2) * 256 + (i + x)] = convertToDsBmp(val);
					}
				}
			}
			x += top_font_texcoords[2 + (4 * charIndex)];
			commitBgSubModify();
		}

		fclose(file);
	}
}

void loadDateFont()
{
	const u16 *src = tex().dateTimeFontTexture()->texture();
	int x = 0;
	int y = 15;
	for (int i = 0; i < 128 * 16; i++)
	{
		if (x >= 128)
		{
			x = 0;
			y--;
		}
		u16 val = *(src++);
		if (val != 0x7C1F)
		{ // Do not render magneta pixel
			dateFontImage[y * 128 + x] = convertToDsBmp(val);
		}
		else
		{
			dateFontImage[y * 128 + x] = 0x7C1F;
		}
		x++;
	}
}

static std::string loadedDate;

void loadDate()
{
	// Load date
	int x = 162;
	char date[6];

	if (!GetDate(FORMAT_MD, date, sizeof(date)))
		return;

	std::string currentDate = date;
	if (currentDate == loadedDate)
		return;

	loadedDate = date;

	beginBgSubModify();
	for (int c = 0; c < 5; c++)
	{
		int imgY = 15;

		unsigned int charIndex = getDateTimeFontSpriteIndex(date[c]);
		// Start date
		for (int y = 14; y >= 6; y--)
		{
			for (u16 i = 0; i < date_time_font_texcoords[2 + (4 * charIndex)]; i++)
			{
				if (dateFontImage[(imgY * 128) + (date_time_font_texcoords[0 + (4 * charIndex)] + i)] != 0x7C1F)
				{ // Do not render magneta pixel
					bgSubBuffer[y * 256 + (i + x)] = dateFontImage[(imgY * 128) + (date_time_font_texcoords[0 + (4 * charIndex)] + i)];
				}
			}
			imgY--;
		}
		x += date_time_font_texcoords[2 + (4 * charIndex)];
	}
	commitBgSubModify();
}

static std::string loadedTime;
static int hourWidth;
static bool initialClockDraw = true;

void loadTime()
{
	// Load time
	int x = 200;
	char time[10];
	std::string currentTime = RetTime();
	if (currentTime != loadedTime)
	{
		loadedTime = currentTime;
		if (currentTime.substr(0, 1) == " ")
			currentTime = "0" + currentTime.substr(1);
		sprintf(time, currentTime.c_str());

		int howManyToDraw = 5;
		if (initialClockDraw)
		{
			initialClockDraw = false;
		}
		else
		{
			if (currentTime.substr(3, 2) != "00")
			{
				strcpy(time, currentTime.substr(3, 2).c_str());
				howManyToDraw = 2;
				x = hourWidth;
			}
		}

		beginBgSubModify();
		for (int c = 0; c < howManyToDraw; c++)
		{
			int imgY = 15;

			unsigned int charIndex = getDateTimeFontSpriteIndex(time[c]);

			for (int y = 14; y >= 6; y--)
			{
				for (u16 i = 0; i < date_time_font_texcoords[2 + (4 * charIndex)]; i++)
				{
					if (dateFontImage[(imgY * 128) + (date_time_font_texcoords[0 + (4 * charIndex)] + i)] != 0x7C1F)
					{ // Do not render magneta pixel
						bgSubBuffer[y * 256 + (i + x)] = dateFontImage[(imgY * 128) + (date_time_font_texcoords[0 + (4 * charIndex)] + i)];
					}
				}
				imgY--;
			}
			x += date_time_font_texcoords[2 + (4 * charIndex)];
			if (c == 2)
				hourWidth = x;
		}
		commitBgSubModify();
	}
}

static bool showColon = true;

void loadClockColon()
{
	// Load time
	int x = 214;
	int imgY = 15;
	char colon[1];

	// Blink the ':' once per second.
	if (colonTimer >= 60)
	{
		colonTimer = 0;
		std::string currentColon = showColon ? ":" : ";";
		sprintf(colon, currentColon.c_str());
		beginBgSubModify();
		unsigned int charIndex = getDateTimeFontSpriteIndex(colon[0]);

		for (int y = 14; y >= 6; y--)
		{
			for (u16 i = 0; i < date_time_font_texcoords[2 + (4 * charIndex)]; i++)
			{
				if (dateFontImage[(imgY * 128) + (date_time_font_texcoords[0 + (4 * charIndex)] + i)] != 0x7C1F)
				{ // Do not render magneta pixel
					bgSubBuffer[y * 256 + (i + x)] = dateFontImage[(imgY * 128) + (date_time_font_texcoords[0 + (4 * charIndex)] + i)];
				}
			}
			imgY--;
		}
		x += date_time_font_texcoords[2 + (4 * charIndex)];
		commitBgSubModify();
		showColon = !showColon;
	}
}

void clearBoxArt()
{
	if (theme == 1)
	{
		loadBMPPart("nitro:/graphics/3ds_top.bmp");
	}
	else
	{
		loadPhotoPart();
	}
}

//static char videoFrameFilename[256];

void loadRotatingCubes()
{
	FILE *videoFrameFile = fopen("nitro:/video/3dsRotatingCubes.rvid", "rb");
	//FILE* videoFrameFile;

	/*for (u8 selectedFrame = 0; selectedFrame <= rocketVideo_videoFrames; selectedFrame++) {
		if (selectedFrame < 0x10) {
			snprintf(videoFrameFilename, sizeof(videoFrameFilename), "nitro:/video/3dsRotatingCubes/0x0%x.bmp", (int)selectedFrame);
		} else {
			snprintf(videoFrameFilename, sizeof(videoFrameFilename), "nitro:/video/3dsRotatingCubes/0x%x.bmp", (int)selectedFrame);
		}
		videoFrameFile = fopen(videoFrameFilename, "rb");

		if (videoFrameFile) {
			// Start loading
			fseek(videoFrameFile, 0xe, SEEK_SET);
			u8 pixelStart = (u8)fgetc(videoFrameFile) + 0xe;
			fseek(videoFrameFile, pixelStart, SEEK_SET);
			fread(bmpImageBuffer, 2, 0x7000, videoFrameFile);
			u16* src = bmpImageBuffer;
			int x = 0;
			int y = 55;
			for (int i=0; i<256*56; i++) {
				if (x >= 256) {
					x = 0;
					y--;
				}
				u16 val = *(src++);
				renderedImageBuffer[y*256+x] = convertToDsBmp(val);
				x++;
			}
		}
		fclose(videoFrameFile);
		memcpy(rotatingCubesLocation+(selectedFrame*0x7000), renderedImageBuffer, 0x7000);
	}*/

	if (videoFrameFile)
	{
		bool doRead = false;

		if (isDSiMode())
		{
			doRead = true;
		}
		else if (sys().isRegularDS() && colorMode == 0)
		{
			sysSetCartOwner(BUS_OWNER_ARM9); // Allow arm9 to access GBA ROM (or in this case, the DS Memory Expansion Pak)
			*(vu32 *)(0x08240000) = 1;
			if (*(vu32 *)(0x08240000) == 1)
			{
				// Set to load video into DS Memory Expansion Pak
				rotatingCubesLocation = (void *)0x09000000;
				doRead = true;
			}
		}
		if (doRead)
		{
			fread(rotatingCubesLocation, 1, 0x690000, videoFrameFile);
			rotatingCubesLoaded = true;
			rocketVideo_playVideo = true;
		}
	}
}

void graphicsInit()
{

	for (int i = 0; i < 12; i++)
	{
		launchDotFrame[i] = 5;
	}

	for (int i = 0; i < 5; i++)
	{
		dropTime[i] = 0;
		dropSeq[i] = 0;
		dropSpeed[i] = 5;
		dropSpeedChange[i] = 0;
		if (theme == 1)
			titleboxYposDropDown[i] = 0;
		else
			titleboxYposDropDown[i] = -85 - 80;
	}

	allowedTitleboxForDropDown = 0;
	delayForTitleboxToDropDown = 0;
	dropDown = false;
	redoDropDown = false;

	launchDotXMove[0] = false;
	launchDotYMove[0] = true;
	launchDotX[0] = 44;
	launchDotY[0] = 0;
	launchDotXMove[1] = false;
	launchDotYMove[1] = true;
	launchDotX[1] = 28;
	launchDotY[1] = 16;
	launchDotXMove[2] = false;
	launchDotYMove[2] = true;
	launchDotX[2] = 12;
	launchDotY[2] = 32;
	launchDotXMove[3] = true;
	launchDotYMove[3] = true;
	launchDotX[3] = 4;
	launchDotY[3] = 48;
	launchDotXMove[4] = true;
	launchDotYMove[4] = true;
	launchDotX[4] = 20;
	launchDotY[4] = 64;
	launchDotXMove[5] = true;
	launchDotYMove[5] = true;
	launchDotX[5] = 36;
	launchDotY[5] = 80;
	launchDotXMove[6] = true;
	launchDotYMove[6] = false;
	launchDotX[6] = 52;
	launchDotY[6] = 80;
	launchDotXMove[7] = true;
	launchDotYMove[7] = false;
	launchDotX[7] = 68;
	launchDotY[7] = 64;
	launchDotXMove[8] = true;
	launchDotYMove[8] = false;
	launchDotX[8] = 84;
	launchDotY[8] = 48;
	launchDotXMove[9] = false;
	launchDotYMove[9] = false;
	launchDotX[9] = 76;
	launchDotY[9] = 36;
	launchDotXMove[10] = false;
	launchDotYMove[10] = false;
	launchDotX[10] = 60;
	launchDotY[10] = 20;
	launchDotX[11] = 44;
	launchDotY[11] = 0;

	titleboxXpos[0] = ms().cursorPosition[0] * 64;
	titlewindowXpos[0] = ms().cursorPosition[0] * 5;
	titleboxXpos[1] = ms().cursorPosition[1] * 64;
	titlewindowXpos[1] = ms().cursorPosition[1] * 5;

	*(u16 *)(0x0400006C) |= BIT(14);
	*(u16 *)(0x0400006C) &= BIT(15);
	SetBrightness(0, 31);
	SetBrightness(1, 31);

	////////////////////////////////////////////////////////////
	videoSetMode(MODE_5_3D | DISPLAY_BG3_ACTIVE);
	videoSetModeSub(MODE_3_2D | DISPLAY_BG3_ACTIVE);

	// Initialize gl2d
	glScreen2D();
	// Make gl2d render on transparent stage.
	glClearColor(31, 31, 31, 0);
	glDisable(GL_CLEAR_BMP);

	// Clear the GL texture state
	glResetTextures();

	// Set up enough texture memory for our textures
	// Bank A is just 128kb and we are using 194 kb of
	// sprites
	vramSetBankA(VRAM_A_TEXTURE);
	vramSetBankB(VRAM_B_TEXTURE);
	vramSetBankC(VRAM_C_SUB_BG_0x06200000);
	vramSetBankD(VRAM_D_MAIN_BG_0x06000000);
	vramSetBankE(VRAM_E_TEX_PALETTE);
	vramSetBankF(VRAM_F_TEX_PALETTE_SLOT4);
	vramSetBankG(VRAM_G_TEX_PALETTE_SLOT5); // 16Kb of palette ram, and font textures take up 8*16 bytes.
	vramSetBankH(VRAM_H_SUB_BG_EXT_PALETTE);
	vramSetBankI(VRAM_I_SUB_SPRITE_EXT_PALETTE);

	//	vramSetBankH(VRAM_H_SUB_BG_EXT_PALETTE); // Not sure this does anything...
	lcdMainOnBottom();

	REG_BG3CNT = BG_MAP_BASE(0) | BG_BMP16_256x256 | BG_PRIORITY(0);
	REG_BG3X = 0;
	REG_BG3Y = 0;
	REG_BG3PA = 1 << 8;
	REG_BG3PB = 0;
	REG_BG3PC = 0;
	REG_BG3PD = 1 << 8;

	REG_BG3CNT_SUB = BG_MAP_BASE(0) | BG_BMP16_256x256 | BG_PRIORITY(0);
	REG_BG3X_SUB = 0;
	REG_BG3Y_SUB = 0;
	REG_BG3PA_SUB = 1 << 8;
	REG_BG3PB_SUB = 0;
	REG_BG3PC_SUB = 0;
	REG_BG3PD_SUB = 1 << 8;

	REG_BLDCNT = BLEND_SRC_BG3 | BLEND_FADE_BLACK;

	swiWaitForVBlank();

	if (theme == 1)
	{
		tex().load3DSTheme();
		titleboxYpos = 96;
		bubbleYpos += 18;
		bubbleXpos += 3;
		topBgLoad();
		loadDate();
		loadTime();
		loadClockColon();
		bottomBgLoad(false, true);
		loadRotatingCubes();
	}
	else
	{
		switch (subtheme)
		{
		default:
		case 0:
			tex().loadDSiDarkTheme();
			break;
		case 1:
			tex().loadDSiWhiteTheme();
			break;
		case 2:
			tex().loadDSiRedTheme();
			break;
		case 3:
			tex().loadDSiBlueTheme();
			break;
		case 4:
			tex().loadDSiGreenTheme();
			break;
		case 5:
			tex().loadDSiYellowTheme();
			break;
		case 6:
			tex().loadDSiPinkTheme();
			break;
		case 7:
			tex().loadDSiPurpleTheme();
			break;
		}

		topBgLoad();
		loadDateFont();
		loadDate();
		loadTime();
		loadClockColon();

		if (theme < 1)
		{
			srand(time(NULL));
			loadPhotoList();
			loadPhoto();
		}

		bottomBgLoad(false, true);
	}

	setVolumeImagePaths();
	loadVolumeImage();
	loadBatteryImage();
	irqSet(IRQ_VBLANK, vBlankHandler);
	irqEnable(IRQ_VBLANK);
	//consoleDemoInit();
}
