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

#include "graphics.h"
#include <ctime>
#include <cmath>
#include <dirent.h>
#include <maxmod9.h>
#include <nds/arm9/dldi.h>

#include "../errorScreen.h"
#include "../iconTitle.h"
#include "../language.h"
#include "../ndsheaderbanner.h"
#include "common/twlmenusettings.h"
#include "common/flashcard.h"
#include <gl2d.h>
#include "common/lzss.h"
#include "common/systemdetails.h"
#include "common/my_rumble.h"
#include "common/logging.h"
#include "myDSiMode.h"
#include "date.h"
#include "iconHandler.h"
#include "fileBrowse.h"
#include "fontHandler.h"
#include "graphics/ThemeTextures.h"
#include "common/lodepng.h"
#include "launchDots.h"
#include "queueControl.h"
#include "sound.h"
#include "startborderpal.h"
//#include "ndma.h"
#include "color.h"
#include "ThemeConfig.h"
#include "themefilenames.h"
#include "common/ColorLut.h"
#include "tool/colortool.h"

#include "bubbles.h"	// For HBL theme

#define CONSOLE_SCREEN_WIDTH 32
#define CONSOLE_SCREEN_HEIGHT 24

#define colorsToCache (208*16)

static glImage hblBubbles[32 * 64];
static int hblBubblesID = 0;

static int backBubblesYpos_def[4] = {256, 256+56, 256+32, 256+16};
static int frontBubblesYpos_def[3] = {256, 256+64, 256+32};

static int backBubblesYpos[4] = {256, 256+56, 256+32, 256+16};
static int frontBubblesYpos[3] = {256, 256+64, 256+32};

static float initialDropSpeed = 7.0f;

extern bool whiteScreen;
extern bool fadeType;
extern bool fadeSpeed;
extern bool fadeSleep;
extern bool controlTopBright;
extern bool controlBottomBright;
int fadeDelay = 0;


extern int colorRvalue;
extern int colorGvalue;
extern int colorBvalue;

extern bool dropDown;
int dropDelay[5];
int dropBounce[5];
float dropDownY[5] = { 0.0f };
float dropSpeed[5] = { 0.0f };
int titleboxYposDropDown[5];

extern int currentBg;
extern bool showSTARTborder;
extern bool needToPlayStopSound;
extern int waitForNeedToPlayStopSound;
extern int movingApp;
extern int movingAppYpos;
extern bool movingAppIsDir;
extern bool draggingIcons;
float movingArrowYpos = 59;
bool movingArrowYdirection = true;
bool showMovingArrow = false;
bool displayGameIcons = false;
bool rumblePos = false;

extern bool buttonArrowTouched[2];
extern bool scrollWindowTouched;
extern bool useRumble;

extern bool applaunchprep;

int screenBrightness = 31;
static bool secondBuffer = false;

static int colonTimer = 0;
extern bool showColon;
//static int loadingSoundTimer = 30;

int movetimer = 0;

int titleboxYmovepos = 0;

extern int spawnedtitleboxes;

std::vector<std::string> photoList;
static std::string photoPath;
uint photoWidth, photoHeight;
int titleboxXmovespeed[8] = {12, 10, 8, 8, 8, 8, 6, 4};
#define titleboxXscrollspeed 8
int titleboxXpos[2] = {0};
int titleboxXdest[2] = {0};
int titleboxYpos = 85; // 85, when dropped down
int iconYposOnTitleBox = 12;
int titlewindowXpos[2] = {0};
int titlewindowXdest[2] = {0};
int titleboxXspeed = 3; // higher is SLOWER
int titleboxXspacing = 58;

bool reloadDate = false;
bool reloadTime = false;
bool showLshoulder = false;
bool showRshoulder = false;

bool showProgressIcon = false;
bool showProgressBar = false;
int progressBarLength = 0;
extern bool useTwlCfg;

int progressAnimNum = 0;
int progressAnimDelay = 0;

bool startBorderZoomOut = false;
int startBorderZoomAnimSeq[5] = {0, 1, 2, 1, 0};
int startBorderZoomAnimNum = 0;
int startBorderZoomAnimDelay = 0;


// int launchDotFrame[12];
// int launchDotCurrentChangingFrame = 0;
// bool launchDotDoFrameChange = false;

bool showdialogbox = false;
bool dboxInFrame = false;
bool dboxStopped = true;
bool dbox_showIcon = false;
bool dbox_selectMenu = false;
float dbox_movespeed = 22;
float dbox_Ypos = -192;
int bottomScreenBrightness = 255;

int bottomBgState = 0; // 0 = Uninitialized, 1 = No Bubble, 2 = bubble, 3 = moving.
int prevBottomBgState = 1;

int vblankRefreshCounter = 0;

u32 rotatingCubesLoaded = false;	// u32 used instead of bool, to fix a weird bug

bool rocketVideo_playVideo = false;
int rocketVideo_videoYpos = 78;
int frameOf60fps = 60;
int rocketVideo_videoFrames = 249;
int rocketVideo_currentFrame = -1;
u8 rocketVideo_fps = 25;
u8 rocketVideo_height = 56;
int rocketVideo_frameDelay = 0;
bool rocketVideo_frameDelayEven = true; // For 24FPS
bool rocketVideo_loadFrame = true;
u16* colorTable = NULL;

int bubbleYpos = 80;
int bubbleXpos = 122;

void vramcpy_ui(void *dest, const void *src, int size) {
	u16 *destination = (u16 *)dest;
	u16 *source = (u16 *)src;
	while (size > 0) {
		*destination++ = *source++;
		size -= 2;
	}
}

extern mm_sound_effect snd_stop;
//extern mm_sound_effect snd_loading;
extern mm_sound_effect mus_menu;

void ClearBrightness(void) {
	fadeType = true;
	screenBrightness = 0;
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
	if (bright > 31)
		bright = 31;
	*(vu16*)(0x0400006C + (0x1000 * screen)) = bright + mode;
}

//-------------------------------------------------------
// set up a 2D layer construced of bitmap sprites
// this holds the image when rendering to the top screen
//-------------------------------------------------------

// void initSubSprites(void) {

// 	oamInit(&oamSub, SpriteMapping_Bmp_2D_256, false);
// 	int id = 0;

// 	// set up a 4x3 grid of 64x64 sprites to cover the screen
// 	for (int y = 0; y < 3; y++)
// 		for (int x = 0; x < 4; x++) {
// 			oamSub.oamMemory[id].attribute[0] = ATTR0_BMP | ATTR0_SQUARE | (64 * y);
// 			oamSub.oamMemory[id].attribute[1] = ATTR1_SIZE_64 | (64 * x);
// 			oamSub.oamMemory[id].attribute[2] = ATTR2_ALPHA(1) | (8 * 32 * y) | (8 * x);
// 			++id;
// 		}

// 	swiWaitForVBlank();

// 	oamUpdate(&oamSub);
// }

void bottomBgLoad(int drawBubble, bool init = false) {
	if (init || drawBubble == 0 || (drawBubble == 2 && ms().theme == TWLSettings::ETheme3DS)) {
		if (bottomBgState != 1) {
			tex().drawBottomBg(1);
			bottomBgState = 1;
		}
	} else if (drawBubble == 1) {
		if (bottomBgState != 2) {
			tex().drawBottomBg(2);
			bottomBgState = 2;
		}
	} else if (drawBubble == 2 && ms().theme == TWLSettings::EThemeDSi) {
		if (bottomBgState != 3) {
			tex().drawBottomBg(3);
			bottomBgState = 3;
		}
	}
	if (ms().macroMode && prevBottomBgState != bottomBgState) {
		reloadDate = true;
		reloadTime = true;
		tex().resetCachedVolumeLevel();
		tex().resetCachedBatteryLevel();
		tex().resetProfileName();
		drawCurrentDate();
		drawCurrentTime();
		tex().drawVolumeImageCached();
		tex().drawBatteryImageCached();
		tex().drawProfileName();
		prevBottomBgState = bottomBgState;
	}
}

void bottomBgRefresh() { bottomBgLoad(currentBg, false); }

void drawBubble(const glImage *images) { glSprite(bubbleXpos, bubbleYpos, GL_FLIP_NONE, &images[0]); }

void drawDbox() {
	for (int y = 0; y < 192 / 16; y++) {
		for (int x = 0; x < 256 / 16; x++) {
			int i = y * 16 + x;
			glSprite(x * 16, dbox_Ypos + y * 16, GL_FLIP_NONE, &tex().dialogboxImage()[i & 255]);
		}
	}
}

void reloadDboxPalette() { tex().reloadPalDialogBox(); }

u8 *rotatingCubesLocation = (u8*)NULL;

void frameRateHandler(void) {
	frameOf60fps++;
	if (frameOf60fps > 60) frameOf60fps = 1;

	if (!rocketVideo_playVideo)
		return;
	if (!rocketVideo_loadFrame) {
		rocketVideo_frameDelay++;
		switch (rocketVideo_fps) {
			case 11:
				rocketVideo_loadFrame = (rocketVideo_frameDelay == 5+rocketVideo_frameDelayEven);
				break;
			case 24:
				rocketVideo_loadFrame = (rocketVideo_frameDelay == 2+rocketVideo_frameDelayEven);
				break;
			case 25:
				rocketVideo_loadFrame = (frameOf60fps == 1
									  || frameOf60fps == 3
									  || frameOf60fps == 5
									  || frameOf60fps == 7
									  || frameOf60fps == 9
									  || frameOf60fps == 11
									  || frameOf60fps == 14
									  || frameOf60fps == 16
									  || frameOf60fps == 19
									  || frameOf60fps == 21
									  || frameOf60fps == 24
									  || frameOf60fps == 26
									  || frameOf60fps == 29
									  || frameOf60fps == 31
									  || frameOf60fps == 34
									  || frameOf60fps == 36
									  || frameOf60fps == 39
									  || frameOf60fps == 41
									  || frameOf60fps == 44
									  || frameOf60fps == 46
									  || frameOf60fps == 49
									  || frameOf60fps == 51
									  || frameOf60fps == 54
									  || frameOf60fps == 56
									  || frameOf60fps == 59);
				break;
			case 48:
				rocketVideo_loadFrame = (frameOf60fps != 3
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
				rocketVideo_loadFrame = (frameOf60fps != 3
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
				rocketVideo_loadFrame = (rocketVideo_frameDelay == 60/rocketVideo_fps);
				break;
		}
	}
}

void playRotatingCubesVideo(void) {
	if (!rocketVideo_playVideo || !rocketVideo_loadFrame)
		return;

	dmaCopyWordsAsynch(1, rotatingCubesLocation+(rocketVideo_currentFrame*(0x200*rocketVideo_height)), (u16*)BG_GFX_SUB+(256*rocketVideo_videoYpos), 0x200*rocketVideo_height);

	rocketVideo_currentFrame++;
	if (rocketVideo_currentFrame > rocketVideo_videoFrames) {
		rocketVideo_currentFrame = 0;
	}
	rocketVideo_frameDelay = 0;
	rocketVideo_frameDelayEven = !rocketVideo_frameDelayEven;
	rocketVideo_loadFrame = false;
}

void vBlankHandler() {
	execQueue();		   // Execute any actions queued during last vblank.
	execDeferredIconUpdates(); // Update any icons queued during last vblank.

	if (waitForNeedToPlayStopSound > 0) {
		waitForNeedToPlayStopSound++;
		if (waitForNeedToPlayStopSound == 5) {
			waitForNeedToPlayStopSound = 0;
		}
		needToPlayStopSound = false;
	}

	static bool updateFrame = true;
	static bool whiteScreenPrev = whiteScreen;
	static int currentBgPrev = currentBg;
	static bool showSTARTborderPrev = showSTARTborder;
	static bool displayGameIconsPrev = displayGameIcons;
	static bool showProgressIconPrev = showProgressIcon;
	static bool showProgressBarPrev = showProgressBar;
	static int progressBarLengthPrev = progressBarLength;
	static bool dbox_showIconPrev = dbox_showIcon;
	static int movingAppYposPrev = movingAppYpos;

	if (whiteScreenPrev != whiteScreen) {
		whiteScreenPrev = whiteScreen;
		updateFrame = true;
	}

	if (currentBgPrev != currentBg) {
		currentBgPrev = currentBg;
		updateFrame = true;
	}

	if (showSTARTborderPrev != showSTARTborder) {
		showSTARTborderPrev = showSTARTborder;
		updateFrame = true;
	}

	if (displayGameIconsPrev != displayGameIcons) {
		displayGameIconsPrev = displayGameIcons;
		updateFrame = true;
	}

	if (showProgressIconPrev != showProgressIcon) {
		showProgressIconPrev = showProgressIcon;
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

	if (dbox_showIconPrev != dbox_showIcon) {
		dbox_showIconPrev = dbox_showIcon;
		updateFrame = true;
	}

	// Move title box/window closer to destination if moved
	if (ms().theme == TWLSettings::EThemeDSi && !draggingIcons && !scrollWindowTouched) {
		if (titleboxXpos[ms().secondaryDevice] > titleboxXdest[ms().secondaryDevice]) {
			titleboxXpos[ms().secondaryDevice] -= titleboxXspeed;
			if (titleboxXpos[ms().secondaryDevice] < titleboxXdest[ms().secondaryDevice]) {
				titleboxXpos[ms().secondaryDevice] = titleboxXdest[ms().secondaryDevice];
			}
			updateFrame = true;
		} else if (titleboxXpos[ms().secondaryDevice] < titleboxXdest[ms().secondaryDevice]) {
			titleboxXpos[ms().secondaryDevice] += titleboxXspeed;
			if (titleboxXpos[ms().secondaryDevice] > titleboxXdest[ms().secondaryDevice]) {
				titleboxXpos[ms().secondaryDevice] = titleboxXdest[ms().secondaryDevice];
			}
			updateFrame = true;
		}

		if (titlewindowXpos[ms().secondaryDevice] > titlewindowXdest[ms().secondaryDevice]) {
			titlewindowXpos[ms().secondaryDevice] -= std::max((titlewindowXpos[ms().secondaryDevice] - titlewindowXdest[ms().secondaryDevice]) / titleboxXspeed, 1);
			updateFrame = true;
		} else if (titlewindowXpos[ms().secondaryDevice] < titlewindowXdest[ms().secondaryDevice]) {
			titlewindowXpos[ms().secondaryDevice] += std::max((titlewindowXdest[ms().secondaryDevice] - titlewindowXpos[ms().secondaryDevice]) / titleboxXspeed, 1);
			updateFrame = true;
		}
	} else if (ms().theme != TWLSettings::EThemeSaturn) {
		if (titleboxXpos[ms().secondaryDevice] > titleboxXdest[ms().secondaryDevice]) {
			titleboxXpos[ms().secondaryDevice] -= std::max((titleboxXpos[ms().secondaryDevice] - titleboxXdest[ms().secondaryDevice]) / titleboxXspeed, 1);
			updateFrame = true;
		} else if (titleboxXpos[ms().secondaryDevice] < titleboxXdest[ms().secondaryDevice]) {
			titleboxXpos[ms().secondaryDevice] += std::max((titleboxXdest[ms().secondaryDevice] - titleboxXpos[ms().secondaryDevice]) / titleboxXspeed, 1);
			updateFrame = true;
		}

		if (titlewindowXpos[ms().secondaryDevice] > titlewindowXdest[ms().secondaryDevice]) {
			titlewindowXpos[ms().secondaryDevice] -= std::max((titlewindowXpos[ms().secondaryDevice] - titlewindowXdest[ms().secondaryDevice]) / titleboxXspeed, 1);
			updateFrame = true;
		} else if (titlewindowXpos[ms().secondaryDevice] < titlewindowXdest[ms().secondaryDevice]) {
			titlewindowXpos[ms().secondaryDevice] += std::max((titlewindowXdest[ms().secondaryDevice] - titlewindowXpos[ms().secondaryDevice]) / titleboxXspeed, 1);
			updateFrame = true;
		}
	} else if (titleboxXpos[ms().secondaryDevice] != titleboxXdest[ms().secondaryDevice]) { // In saturn theme just move instantly
		titleboxXpos[ms().secondaryDevice] = titleboxXdest[ms().secondaryDevice];
		titlewindowXpos[ms().secondaryDevice] = titlewindowXdest[ms().secondaryDevice];
		updateFrame = true;
	}

	if (ms().theme == TWLSettings::ETheme3DS && rotatingCubesLoaded) {
		playRotatingCubesVideo();
	}

	if (fadeType) {
		if (!fadeDelay) {
			screenBrightness -= fadeSleep ? 1 : 1+(ms().theme<4 && fadeSpeed);
			if (screenBrightness < 0)
				screenBrightness = 0;
		}
		if (!fadeSpeed) {
			fadeDelay++;
			if (fadeDelay == 3)
				fadeDelay = 0;
		} else {
			fadeDelay = 0;
		}
	} else {
		if (!fadeDelay) {
			screenBrightness += fadeSleep ? 1 : 1+(ms().theme<4 && fadeSpeed);
			if (screenBrightness > 31)
				screenBrightness = 31;
		}
		if (!fadeSpeed) {
			fadeDelay++;
			if (fadeDelay == 3)
				fadeDelay = 0;
		} else {
			fadeDelay = 0;
		}
	}

	if (controlBottomBright)
		SetBrightness(0, tc().darkLoading() ? -screenBrightness : screenBrightness);
	if (controlTopBright && !ms().macroMode)
		SetBrightness(1, tc().darkLoading() ? -screenBrightness : screenBrightness);

	if (showdialogbox) {
		// Dialogbox moving into view...
		dboxInFrame = true;
		if (ms().theme == TWLSettings::ETheme3DS) {
			dbox_movespeed = 0;
			if (dbox_Ypos != 0) {
				dbox_Ypos = 0;
				updateFrame = true;
			}
		}
		if (dbox_movespeed <= 1) {
			if (dbox_Ypos >= 0) {
				// dbox stopped
				if (!dboxStopped || dbox_Ypos != 0) {
					dboxStopped = true;
					dbox_movespeed = 0;
					dbox_Ypos = 0;
					bottomScreenBrightness = 127;
					REG_BLDY = (0b0100 << 1);
					updateFrame = true;
				}
			} else {
				// dbox moving into view
				dbox_movespeed = 1;
			}
		} else {
			// Dbox decel
			dboxStopped = false;
			dbox_movespeed -= 1.25;
			bottomScreenBrightness -= 7;
			if (bottomScreenBrightness < 127) {
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
		if (dbox_movespeed) {
			dbox_Ypos += dbox_movespeed;
			updateFrame = true;
		}
	} else {
		// Dialogbox moving down...
		if (ms().theme == TWLSettings::ETheme3DS || dbox_Ypos <= -192 || dbox_Ypos >= 192) {
			if (dboxStopped || dbox_Ypos != -192) {
				dboxInFrame = false;
				dboxStopped = false;
				dbox_movespeed = 22;
				dbox_Ypos = -192;
				bottomScreenBrightness = 255;
				REG_BLDY = 0;
				updateFrame = true;
			}
		} else {
			dbox_movespeed += 1;
			dbox_Ypos += dbox_movespeed;
			bottomScreenBrightness += 7;
			if (bottomScreenBrightness > 255) {
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

			updateFrame = true;
		}
	}

	if (!whiteScreen && dropDown && ms().theme == TWLSettings::EThemeDSi) { // perform dropdown anim in the DSi theme
		static constexpr float gravity = 0.375f;
		static constexpr float restitution = 0.37f;
		
		int i2 = CURPOS - 2;
		if (i2 < 0)
			i2 += 5;

		for (int i = i2, d = 0; i < i2 + 5; i++, d++) {
			if (dropDelay[d] > 0) continue; // Skip if not ready

			int b = i % 5;
			if (dropBounce[b] < 3) {
				dropSpeed[b] += gravity;

				if (dropDownY[b] + dropSpeed[b] >= 0.0f) { // We collided, now bounce
					dropDownY[b] = 0.0f;
					dropSpeed[b] *= -restitution;
					dropBounce[b]++;
				} else
					dropDownY[b] += dropSpeed[b]; // Otherwise, move Y

				// Update drawing position
				titleboxYposDropDown[b] = (int)(floorf(dropDownY[b]));
				updateFrame = true;
			}
			else if (dropBounce[b] < 4) { // Stop dropdown
				dropDownY[b] = 0.0f;
				titleboxYposDropDown[b] = 0;
				dropBounce[b]++;
			}
		}

		// Update delays
		for (int i = 0; i < 5; i++) {
			if (dropDelay[i] > 0)
				dropDelay[i]--;
		}
	}

	if (movingApp != -1 && ms().theme == TWLSettings::EThemeDSi && showMovingArrow) {
		if (movingArrowYdirection) {
			movingArrowYpos += 0.33;
			if (movingArrowYpos > 67)
				movingArrowYdirection = false;
		} else {
			movingArrowYpos -= 0.33;
			if (movingArrowYpos < 59)
				movingArrowYdirection = true;
		}
		updateFrame = true;
	}

	if (movingAppYposPrev != movingAppYpos) {
		movingAppYposPrev = movingAppYpos;
		updateFrame = true;
	}

	if (applaunchprep && titleboxYmovepos < 192) {
		titleboxYmovepos += 5;
		updateFrame = true;
	}

	if (ms().theme == TWLSettings::EThemeHBL) {
		// Back bubbles
		for (int i = 0; i < 4; i++) {
			backBubblesYpos[i]--;
			if (backBubblesYpos[i] < -16) {
				backBubblesYpos[i] = backBubblesYpos_def[i];
			}
		}
		// Front bubbles
		for (int i = 0; i < 3; i++) {
			frontBubblesYpos[i] -= 2;
			if (frontBubblesYpos[i] < -32) {
				frontBubblesYpos[i] = frontBubblesYpos_def[i];
			}
		}
		updateFrame = true;
	}

	// Blink colon once per second
	if (colonTimer >= 60) {
		colonTimer = 0;
		showColon = !showColon;
	}

	colonTimer++;

	if (showProgressIcon) {
		/*loadingSoundTimer++;

		if (loadingSoundTimer >= 60) {
			loadingSoundTimer = 0;
			mmEffectEx(&snd_loading);
		}*/

		progressAnimDelay++;
		if (progressAnimDelay == 3) {
			progressAnimNum++;
			if (progressAnimNum > 7)
				progressAnimNum = 0;
			progressAnimDelay = 0;
			updateFrame = true;
		}
	}

	if (displayGameIcons || dbox_showIcon) {
		// Playback animated icons
		for (int i = 0; i < ((movingApp != -1) ? 41 : 40); i++) {
			if (bnriconisDSi[i] && playBannerSequence(i) && !updateFrame) {
				updateFrame = (displayGameIcons && (ms().theme != TWLSettings::EThemeSaturn)) ? ((i >= CURPOS-2 && i <= CURPOS+2) || i == 40) : (i == CURPOS);
			}
		}
	}

	if (ms().theme == TWLSettings::ETheme3DS) {
		startBorderZoomAnimDelay++;
		if (startBorderZoomAnimDelay == 8) {
			startBorderZoomAnimNum++;
			if (startBorderZoomAnimSeq[startBorderZoomAnimNum] == 0) {
				startBorderZoomAnimNum = 0;
			}
			startBorderZoomAnimDelay = 0;
			updateFrame = true;
		}
	} else if (startBorderZoomOut) {
		if (useRumble) {
			rumblePos = !rumblePos;
			my_setRumble(rumblePos);
		}
		startBorderZoomAnimNum++;
		if (startBorderZoomAnimSeq[startBorderZoomAnimNum] == 0) {
			if (useRumble) {
				rumblePos = false;
				my_setRumble(rumblePos);
			}
			startBorderZoomAnimNum = 0;
			startBorderZoomOut = false;
		}
		updateFrame = true;
	} else {
		startBorderZoomAnimNum = 0;
	}

	// if (applaunchprep && ms().theme == TWLSettings::EThemeDSi && launchDotDoFrameChange) {
	// 	launchDotFrame[0]--;
	// 	if (launchDotCurrentChangingFrame >= 1)
	// 		launchDotFrame[1]--;
	// 	if (launchDotCurrentChangingFrame >= 2)
	// 		launchDotFrame[2]--;
	// 	if (launchDotCurrentChangingFrame >= 3)
	// 		launchDotFrame[3]--;
	// 	if (launchDotCurrentChangingFrame >= 4)
	// 		launchDotFrame[4]--;
	// 	if (launchDotCurrentChangingFrame >= 5)
	// 		launchDotFrame[5]--;
	// 	if (launchDotCurrentChangingFrame >= 6)
	// 		launchDotFrame[6]--;
	// 	if (launchDotCurrentChangingFrame >= 7)
	// 		launchDotFrame[7]--;
	// 	if (launchDotCurrentChangingFrame >= 8)
	// 		launchDotFrame[8]--;
	// 	if (launchDotCurrentChangingFrame >= 9)
	// 		launchDotFrame[9]--;
	// 	if (launchDotCurrentChangingFrame >= 10)
	// 		launchDotFrame[10]--;
	// 	if (launchDotCurrentChangingFrame >= 11)
	// 		launchDotFrame[11]--;
	// 	for (int i = 0; i < 12; i++) {
	// 		if (launchDotFrame[i] < 0)
	// 			launchDotFrame[i] = 0;
	// 	}
	// 	launchDotCurrentChangingFrame++;
	// 	if (launchDotCurrentChangingFrame > 11)
	// 		launchDotCurrentChangingFrame = 11;
	// }
	// if (applaunchprep && ms().theme == TWLSettings::EThemeDSi)
	// 	launchDotDoFrameChange = !launchDotDoFrameChange;

	if (updateFrame) {
		glBegin2D();

		int bg_R = bottomScreenBrightness / 8;
		int bg_G = (bottomScreenBrightness / 8);
		int bg_B = (bottomScreenBrightness / 8);

		if (!invertedColors) {
			glColor(RGB15(bg_R, bg_G, bg_B));
		}

		if (ms().theme == TWLSettings::EThemeHBL) {
			// Back bubbles
			int bubbleXpos = 16;
			for (int i = 0; i < 4; i++) {
				glSprite(bubbleXpos, backBubblesYpos[i], GL_FLIP_NONE, &hblBubbles[3]);
				bubbleXpos += 64;
			}
			// Front bubbles
			bubbleXpos = 32;
			for (int i = 0; i < 3; i++) {
				glSprite(bubbleXpos, frontBubblesYpos[i], GL_FLIP_NONE, &hblBubbles[4]);
				glSprite(bubbleXpos+16, frontBubblesYpos[i], GL_FLIP_NONE, &hblBubbles[5]);
				glSprite(bubbleXpos, frontBubblesYpos[i]+16, GL_FLIP_NONE, &hblBubbles[6]);
				glSprite(bubbleXpos+16, frontBubblesYpos[i]+16, GL_FLIP_NONE, &hblBubbles[7]);
				bubbleXpos += 64;
			}
		}

		if (ms().theme == TWLSettings::EThemeDSi) {
			int bipXpos = 27;
			glSprite(16 + titlewindowXpos[ms().secondaryDevice], 171, GL_FLIP_NONE,
				 tex().scrollwindowImage());
			for (int i = 0; i < 40; i++) {
				if (i < spawnedtitleboxes) {
					if (bnrSysSettings[i]) {
						glSprite(bipXpos, 178, GL_FLIP_NONE, &tex().bipsImage()[2]);
					} else {
						glSprite(bipXpos, 178, GL_FLIP_NONE, tex().bipsImage());
					}
				} else {
					glSprite(bipXpos, 178, GL_FLIP_NONE, &tex().bipsImage()[1]);
				}
				bipXpos += 5;
			}
			glSprite(16 + titlewindowXpos[ms().secondaryDevice], 171, GL_FLIP_NONE,
				 &tex().buttonarrowImage()[2 + scrollWindowTouched]);
			glSprite(0, 171, GL_FLIP_NONE, &tex().buttonarrowImage()[0 + buttonArrowTouched[0]]);
			glSprite(224, 171, GL_FLIP_H, &tex().buttonarrowImage()[0 + buttonArrowTouched[1]]);
			glSprite(66 - titleboxXpos[ms().secondaryDevice], 81, GL_FLIP_NONE, tex().braceImage());
		}

		if (displayGameIcons) {
			// for (int i = 0; i < 11; i++) {
			// 		glSprite(76 + launchDotX[i], 69 + launchDotY[i], GL_FLIP_NONE,
			// 			&tex().launchdotImage()[5 & 15]);

			// 			//  &tex().launchdotImage()[(launchDotFrame[i]) & 15]);
			// 		if (launchDotX[i] == 0)
			// 			launchDotXMove[i] = true;
			// 		if (launchDotX[i] == 88)
			// 			launchDotXMove[i] = false;
			// 		if (launchDotY[i] == 0)
			// 			launchDotYMove[i] = true;
			// 		if (launchDotY[i] == 88)
			// 			launchDotYMove[i] = false;
			// 		if (launchDotXMove[i] == false) {
			// 			launchDotX[i]--;
			// 		} else if (launchDotXMove[i] == true) {
			// 			launchDotX[i]++;
			// 		}
			// 		if (launchDotYMove[i] == false) {
			// 			launchDotY[i]--;
			// 		} else if (launchDotYMove[i] == true) {
			// 			launchDotY[i]++;
			// 		}
			// }

			int spawnedboxXpos = 96;
			int iconXpos = 112;

			int realCurPos = (titleboxXpos[ms().secondaryDevice] + 32) / titleboxXspacing;
			int titleboxOffset = (realCurPos * titleboxXspacing) - (titleboxXpos[ms().secondaryDevice]);

			int maxIconNumber = (ms().theme == TWLSettings::EThemeSaturn ? 0 : 3);
			for (int pos = std::max(CURPOS - maxIconNumber, 0); pos <= std::min(CURPOS + maxIconNumber, 39); pos++) {
				int i = pos;

				// Stop drawing if the rest of the boxes are empty (with hide empty boxes enabled)
				if (ms().hideEmptyBoxes && (i != 0 && i >= spawnedtitleboxes - (movingApp != -1)))
					break;

				if (movingApp == -1) {
					spawnedboxXpos = 96 + pos * titleboxXspacing;
					iconXpos = 112 + pos * titleboxXspacing;

					if (ms().theme == TWLSettings::EThemeDSi) {
						if (titleboxOffset >= 22 && titleboxOffset <= 32 && (i == realCurPos || i == realCurPos - 1)) {
							int adjust = 0;
							if (i == realCurPos)
								adjust = (titleboxOffset - 22) * 7 / 10;
							else if (i == realCurPos - 1)
								adjust = -((10 - (titleboxOffset - 22)) * 7 / 10);

							spawnedboxXpos += adjust;
							iconXpos += adjust;
						} else if (i < realCurPos) {
							spawnedboxXpos -= 7;
							iconXpos -= 7;
						} else if (i > realCurPos) {
							spawnedboxXpos += 7;
							iconXpos += 7;
						}
					}
				} else {
					spawnedboxXpos = 96 + 38 + pos * titleboxXspacing;
					iconXpos = 112 + 38 + pos * titleboxXspacing;

					if (i >= movingApp - (PAGENUM * 40))
						i++;
				}

				if (i < spawnedtitleboxes) {
					if (isDirectory[i]) {
						if (ms().theme == TWLSettings::ETheme3DS)
							glSprite(spawnedboxXpos - titleboxXpos[ms().secondaryDevice],
									 titleboxYpos, GL_FLIP_NONE,
									 tex().folderImage());
						else
							glSprite(spawnedboxXpos - titleboxXpos[ms().secondaryDevice],
									 (titleboxYpos - 3) + titleboxYposDropDown[i % 5],
									 GL_FLIP_NONE, tex().folderImage());
						if (customIcon[i])
							drawIcon(iconXpos - titleboxXpos[ms().secondaryDevice],
									 (titleboxYpos + iconYposOnTitleBox) + titleboxYposDropDown[i % 5],
									 i);
					} else if (!applaunchprep || CURPOS != i) { // Only draw the icon if we're not launching the selcted app
						if (!bnrSysSettings[i]) {
							if (ms().theme == TWLSettings::ETheme3DS) {
								glSprite(spawnedboxXpos - titleboxXpos[ms().secondaryDevice],
										 titleboxYpos, GL_FLIP_NONE,
										 tex().boxfullImage());
							} else {
								glSprite(spawnedboxXpos - titleboxXpos[ms().secondaryDevice],
										 titleboxYpos + titleboxYposDropDown[i % 5],
										 GL_FLIP_NONE, &tex().boxfullImage()[0]);
							}
						}
						if (bnrSysSettings[i])
							glSprite(spawnedboxXpos - titleboxXpos[ms().secondaryDevice],
									 (titleboxYpos - 1) + titleboxYposDropDown[i % 5],
									 GL_FLIP_NONE, &tex().settingsImage()[1]);
						else
							drawIcon(iconXpos - titleboxXpos[ms().secondaryDevice],
										(titleboxYpos + iconYposOnTitleBox) + titleboxYposDropDown[i % 5],
										i);
					}
				} else {
					// Empty box
					if (ms().theme > TWLSettings::EThemeDSi) {
						glSprite(spawnedboxXpos - titleboxXpos[ms().secondaryDevice],
								 titleboxYpos + titleboxYposDropDown[i % 5],
								 GL_FLIP_NONE, tex().boxemptyImage());
					} else {
						glSprite(spawnedboxXpos - titleboxXpos[ms().secondaryDevice],
								 titleboxYpos + titleboxYposDropDown[i % 5],
								 GL_FLIP_NONE, &tex().boxfullImage()[1]);
					}
				}
			}

			if (movingApp != -1) {
				if (movingAppIsDir) {
					if (ms().theme == TWLSettings::ETheme3DS)
						glSprite(96, titleboxYpos - movingAppYpos, GL_FLIP_NONE, tex().folderImage());
					else
						glSprite(96, titleboxYpos - movingAppYpos + titleboxYposDropDown[movingApp % 5],
								 GL_FLIP_NONE, tex().folderImage());
					if (customIcon[movingApp])
						drawIcon(112,
								 (titleboxYpos + iconYposOnTitleBox) - movingAppYpos + titleboxYposDropDown[movingApp % 5],
								 -1);
				} else {
					if (!bnrSysSettings[movingApp]) {
						if (ms().theme == TWLSettings::ETheme3DS) {
							glSprite(96, titleboxYpos - movingAppYpos, GL_FLIP_NONE,
									 tex().boxfullImage());
						} else {
							glSprite(96,
									 titleboxYpos - movingAppYpos + titleboxYposDropDown[movingApp % 5],
									 GL_FLIP_NONE, &tex().boxfullImage()[0]);
						}
					}
					if (bnrSysSettings[movingApp])
						glSprite(96,
								 (titleboxYpos - 1) - movingAppYpos + titleboxYposDropDown[movingApp % 5],
								 GL_FLIP_NONE, &tex().settingsImage()[1]);
					else
						drawIcon(112, (titleboxYpos + iconYposOnTitleBox) - movingAppYpos + titleboxYposDropDown[movingApp % 5], -1);
				}
			}

			if (ms().theme == TWLSettings::EThemeDSi) {
				glSprite(spawnedboxXpos + titleboxXspacing + 20 - titleboxXpos[ms().secondaryDevice], 81, GL_FLIP_H, tex().braceImage());
			}

			if (movingApp != -1 && ms().theme == TWLSettings::EThemeDSi && showMovingArrow) {
				glSprite(115, movingArrowYpos, GL_FLIP_NONE, tex().movingArrowImage());
			}
		}

		// Top icons for 3DS theme
		if (ms().theme == TWLSettings::ETheme3DS) {
			int topIconXpos = 116;
			for (int i = 0; i < 3; i++) {
				topIconXpos -= 16;
			}
			if ((isDSiMode() && sdFound()) || bothSDandFlashcard()) {
				//for (int i = 0; i < 2; i++) {
					topIconXpos -= 16;
				//}
				if (ms().secondaryDevice) {
					glSprite(topIconXpos, 1, GL_FLIP_NONE, &tex().smallCartImage()[2]); // SD card
				} else {
					glSprite(topIconXpos, 1, GL_FLIP_NONE,
						 &tex().smallCartImage()[(REG_SCFG_MC == 0x11) ? 1 : 0]); // Slot-1 card
				}
				topIconXpos += 32;
			} else if (ms().gbaBooter == TWLSettings::EGbaNativeGbar2 && (io_dldi_data->ioInterface.features & FEATURE_SLOT_NDS)) {
				// for (int i = 0; i < 2; i++) {
					topIconXpos -= 16;
				//}
				//if (ms().gbaBooter == TWLSettings::EGbaGbar2) {
				//	drawSmallIconGBA(topIconXpos, 1); // GBARunner2
				//} else {
					glSprite(topIconXpos, 1, GL_FLIP_NONE, &tex().smallCartImage()[3]); // GBA Mode
				//}
				topIconXpos += 32;
			}
			if (io_dldi_data->ioInterface.features & FEATURE_SLOT_GBA) {
				topIconXpos -= 16;
				glSprite(topIconXpos, 1, GL_FLIP_NONE,
					 &tex().smallCartImage()[0]); // Slot-1 card
				topIconXpos += 32;
			}
			glSprite(topIconXpos, 1, GL_FLIP_NONE, &tex().smallCartImage()[5]); // Pictochat
			topIconXpos += 32;
			glSprite(topIconXpos, 1, GL_FLIP_NONE, &tex().smallCartImage()[6]); // DS Download Play
			topIconXpos += 32;
			glSprite(topIconXpos, 1, GL_FLIP_NONE, &tex().smallCartImage()[7]); // Internet Browser
			topIconXpos += 32;
			glSprite(topIconXpos, 1, GL_FLIP_NONE, &tex().smallCartImage()[4]); // Manual

			// Replace by baked-in backgrounds on 3DS.

			// glSprite(0, 0, GL_FLIP_NONE, &tex().cornerButtonImage()[0]);
			// if (!sys().isRegularDS())
			// 	glSprite(256 - 44, 0, GL_FLIP_NONE, &tex().cornerButtonImage()[1]);
		}

		if (applaunchprep) {
			if (isDirectory[CURPOS]) {
				glSprite(96, 87 - titleboxYmovepos, GL_FLIP_NONE, tex().folderImage());
				if (customIcon[CURPOS])
					drawIcon(112, 96 - titleboxYmovepos, CURPOS);
			} else {
				if (!bnrSysSettings[CURPOS]) {
					glSprite(96, 84 - titleboxYmovepos, GL_FLIP_NONE, tex().boxfullImage());
				}
				if (bnrSysSettings[CURPOS])
					glSprite(96, 83 - titleboxYmovepos, GL_FLIP_NONE, &tex().settingsImage()[1]);
				else
					drawIcon(112, 96 - titleboxYmovepos, CURPOS);
			}
			// Draw dots after selecting a game/app
			dots().drawAuto();
		}
		if (showSTARTborder && displayGameIcons && (ms().theme < 4)) {
			glSprite(96, tc().startBorderRenderY(), GL_FLIP_NONE,
				 &tex().startbrdImage()[startBorderZoomAnimSeq[startBorderZoomAnimNum] &
							(tc().startBorderSpriteH() - 1)]);
			glSprite(96 + tc().startBorderSpriteW(), tc().startBorderRenderY(), GL_FLIP_H,
				 &tex().startbrdImage()[startBorderZoomAnimSeq[startBorderZoomAnimNum] &
							(tc().startBorderSpriteH() - 1)]);
			if (bnrWirelessIcon[CURPOS] > 0)
				glSprite(96, tc().startBorderRenderY(), GL_FLIP_NONE,
					 &tex().wirelessIcons()[(bnrWirelessIcon[CURPOS] - 1) & 31]);

			if (currentBg == 1 && tc().playStopSound() && needToPlayStopSound &&
				waitForNeedToPlayStopSound == 0) {
				snd().playStop();
				waitForNeedToPlayStopSound = 1;
				needToPlayStopSound = false;
			}
			// if (ms().theme == TWLSettings::EThemeDSi) {
				// glSprite(96, tc().startBorderRenderY(), GL_FLIP_NONE,
				// 	 &tex().startbrdImage()[startBorderZoomAnimSeq[startBorderZoomAnimNum] &
				// (tc().startBorderSpriteH() - 1)]); glSprite(96 + tc().startBorderSpriteW(),
				// tc().startBorderRenderY(), GL_FLIP_H,
				// 	 &tex().startbrdImage()[startBorderZoomAnimSeq[startBorderZoomAnimNum] &
				// (tc().startBorderSpriteH() - 1)]); if (bnrWirelessIcon[CURPOS] > 0) 	glSprite(96,
				// tc().startBorderRenderY(), GL_FLIP_NONE,
				// 		 &tex().wirelessIcons()[(bnrWirelessIcon[CURPOS] - 1) & 31]);
			// }
		}

		// Refresh the background layer.
		if (currentBg == 1 && ms().theme != TWLSettings::EThemeSaturn && ms().theme != TWLSettings::EThemeHBL)
			drawBubble(tex().bubbleImage());
		if (showSTARTborder && displayGameIcons && ms().theme == TWLSettings::EThemeDSi) {
			glSprite(96, tc().startTextRenderY(), GL_FLIP_NONE, &tex().startImage()[ms().getGameLanguage()]);
		}

		glColor(RGB15(31, 31, 31));
		if (dbox_Ypos != -192 || (ms().theme == TWLSettings::EThemeSaturn && currentBg == 1) || ms().theme == TWLSettings::EThemeHBL) {
			// Draw the dialog box.
			if (ms().theme != TWLSettings::EThemeSaturn && ms().theme != TWLSettings::EThemeHBL) drawDbox();
			if (dbox_showIcon && !isDirectory[CURPOS]) {
				drawIcon(ms().rtl() ? 256 - 56 : 24,
							((ms().theme == TWLSettings::EThemeSaturn || ms().theme == TWLSettings::EThemeHBL) ? 0 : dbox_Ypos) + 24,
							CURPOS);
			}
			if (dbox_selectMenu) {
				int selIconYpos = 96;
				int selIconXpos = ms().rtl() ? 256 - 56 : 32;
				if (ms().kioskMode) {
					if (sdFound()) {
						for (int i = 0; i < 3; i++) {
							selIconYpos -= 14;
						}
					} else {
						for (int i = 0; i < 2; i++) {
							selIconYpos -= 14;
						}
					}
				} else {
					if (sdFound()) {
						for (int i = 0; i < 4; i++) {
							selIconYpos -= 14;
						}
					} else {
						for (int i = 0; i < 3; i++) {
							selIconYpos -= 14;
						}
					}
				}
				if (!sys().isRegularDS()) {
					glSprite(selIconXpos, (ms().theme == TWLSettings::EThemeSaturn ? 0 : dbox_Ypos) + selIconYpos, GL_FLIP_NONE,
						 &tex().cornerButtonImage()[1]); // System Menu
					selIconYpos += 28;
				}
				if (!ms().kioskMode) {
					glSprite(selIconXpos, (ms().theme == TWLSettings::EThemeSaturn ? 0 : dbox_Ypos) + selIconYpos, GL_FLIP_NONE,
						 &tex().cornerButtonImage()[0]); // Settings
					selIconYpos += 28;
				}
				if (sdFound()) {
					if (ms().secondaryDevice) {
						glSprite(selIconXpos, (ms().theme == TWLSettings::EThemeSaturn ? 0 : dbox_Ypos) + selIconYpos, GL_FLIP_NONE,
							 &tex().smallCartImage()[2]); // SD card
					} else {
						glSprite(
							selIconXpos, (ms().theme == TWLSettings::EThemeSaturn ? 0 : dbox_Ypos) + selIconYpos, GL_FLIP_NONE,
							&tex().smallCartImage()[(REG_SCFG_MC == 0x11) ? 1
												  : 0]); // Slot-1 card
					}
					selIconYpos += 28;
				}
				if (io_dldi_data->ioInterface.features & FEATURE_SLOT_GBA) {
					glSprite(
						selIconXpos, (ms().theme == TWLSettings::EThemeSaturn ? 0 : dbox_Ypos) + selIconYpos, GL_FLIP_NONE,
						&tex().smallCartImage()[0]); // Slot-1 card
					selIconYpos += 28;
				} else if (sys().isRegularDS() && ms().gbaBooter != 2) {
				/*	drawSmallIconGBA(selIconXpos, (ms().theme == TWLSettings::EThemeSaturn ? 0 : dbox_Ypos) + selIconYpos); // GBARunner2
				} else {*/
					glSprite(selIconXpos, (ms().theme == TWLSettings::EThemeSaturn ? 0 : dbox_Ypos) + selIconYpos, GL_FLIP_NONE,
						 &tex().smallCartImage()[3]); // GBA Mode
					selIconYpos += 28;
				}
				glSprite(selIconXpos, (ms().theme == TWLSettings::EThemeSaturn ? 0 : dbox_Ypos) + selIconYpos, GL_FLIP_NONE,
					 tex().manualImage()); // Manual
			}
		}

		// Show button_arrowPals (debug feature)
		/*for (int i = 0; i < 16; i++) {
				for (int i2 = 0; i2 < 16; i2++) {
					glBox(i2,i,i2,i,button_arrowPals[(i*16)+i2]);
				}
			}*/
		if (whiteScreen) {
			u16 fillColor = tc().darkLoading() ? RGB15(0, 0, 0) : RGB15(31, 31, 31);
			if (colorTable) fillColor = colorTable[fillColor % 0x8000];
			glBoxFilled(0, 0, 256, 192, fillColor);
		}
		if (showProgressIcon && ms().theme != TWLSettings::EThemeSaturn) {
			glSprite(ms().rtl() ? 16 : 224, 152, GL_FLIP_NONE, &tex().progressImage()[progressAnimNum]);
		}
		if (showProgressBar) {
			if (progressBarLength > 192) progressBarLength = 192;
			int barXpos = ms().rtl() ? 256 - 19 : 19;
			int barYpos = 157;
			if (ms().theme == TWLSettings::EThemeSaturn) {
				barXpos += ms().rtl() ? -12 : 12;
				barYpos += 12;
			}
			extern int getFavoriteColor(void);
			u16 fillColor = tc().progressBarUserPalette() ? progressBarColors[getFavoriteColor()] : tc().progressBarColor();
			u16 fillColorBack = tc().darkLoading() ? RGB15(6, 6, 6) : RGB15(23, 23, 23);
			if (colorTable) {
				fillColor = colorTable[fillColor % 0x8000];
				fillColorBack = colorTable[fillColorBack % 0x8000];
			}
			if (ms().rtl()) {
				glBoxFilled(barXpos, barYpos, barXpos-192, barYpos+5, fillColorBack);
				if (progressBarLength > 0) {
					glBoxFilled(barXpos, barYpos, barXpos-progressBarLength, barYpos+5, fillColor);
				}
			} else {
				glBoxFilled(barXpos, barYpos, barXpos+192, barYpos+5, fillColorBack);
				if (progressBarLength > 0) {
					glBoxFilled(barXpos, barYpos, barXpos+progressBarLength, barYpos+5, fillColor);
				}
			}
		}

		//}
		glEnd2D();
		GFX_FLUSH = 0;
		updateFrame = false;
	}

	if (vblankRefreshCounter >= REFRESH_EVERY_VBLANKS) {
		if (showdialogbox && dbox_Ypos == -192) {
			// Reload the dialog box palettes here...
			reloadDboxPalette();
		} else if (!showdialogbox) {
			reloadIconPalettes();
		}
		vblankRefreshCounter = 0;
	} else {
		vblankRefreshCounter++;
	}

	if (boxArtColorDeband) {
		//ndmaCopyWordsAsynch(0, tex().frameBuffer(secondBuffer), BG_GFX, 0x18000);
		dmaCopyHalfWordsAsynch(1, tex().frameBufferBot(secondBuffer), BG_GFX_SUB, 0x18000);
		secondBuffer = !secondBuffer;
	}

	bottomBgRefresh(); // Refresh the background image on vblank
}

static bool currentPhotoIsBootstrap = false;
static std::string currentPhotoPath;
static int currentBootstrapPhoto = 0;

void loadPhoto(const std::string &path, const bool bufferOnly);
void loadBootstrapScreenshot(FILE *file, const bool bufferOnly);

bool loadPhotoList() {
	if (!tex().photoBuffer()) {
		return false;
	}

	DIR *dir;
	struct dirent *ent;
	std::string photoDir;
	std::string dirPath = sys().isRunFromSD() ? "sd:/_nds/TWiLightMenu/dsimenu/photos/" : "fat:/_nds/TWiLightMenu/dsimenu/photos/";
	std::vector<std::string> photoList;

	if ((dir = opendir(dirPath.c_str())) == NULL) {
		dirPath = sys().isRunFromSD() ? "fat:/_nds/TWiLightMenu/dsimenu/photos/" : "sd:/_nds/TWiLightMenu/dsimenu/photos/";
		dir = opendir(dirPath.c_str());
	}

	if (dir) {
		/* print all the files and directories within directory */
		while ((ent = readdir(dir)) != NULL) {
			photoDir = ent->d_name;
			if (photoDir == ".." || photoDir == "..." || photoDir == "." || photoDir.substr(0, 2) == "._" ||
				photoDir.substr(photoDir.find_last_of(".") + 1) != "png")
				continue;

			// Reallocation here, but prevents our vector from being filled with garbage
			photoList.emplace_back(dirPath + photoDir);
		}
		closedir(dir);
		if (photoList.size() > 0) {
			currentPhotoPath = photoList[rand() / ((RAND_MAX + 1u) / photoList.size())];
			loadPhoto(currentPhotoPath, false);
			currentPhotoIsBootstrap = false;
			return true;
		}
	}

	// If no photos found, try find a bootstrap screenshot
	FILE *file = fopen(sys().isRunFromSD() ? "sd:/_nds/nds-bootstrap/screenshots.tar" : "fat:/_nds/nds-bootstrap/screenshots.tar", "rb");
	if (!file)
		file = fopen(sys().isRunFromSD() ? "fat:/_nds/nds-bootstrap/screenshots.tar" : "sd:/_nds/nds-bootstrap/screenshots.tar", "rb");
	
	if (file) {
		std::vector<int> screenshots;
		fseek(file, 0x200, SEEK_SET);
		for (int i = 0; i < 50; i++) {
			if (fgetc(file) == 'B')
				screenshots.push_back(i);
			fseek(file, 0x18400 - 1, SEEK_CUR);
		}

		if (screenshots.size() > 0) {
			currentBootstrapPhoto = screenshots[rand() % screenshots.size()];
			fseek(file, 0x200 + 0x18400 * currentBootstrapPhoto, SEEK_SET);
			loadBootstrapScreenshot(file, false);
			currentPhotoIsBootstrap = true;
			return true;
		}
	}

	// If no photos or screenshots found, then draw the default
	char path[64];
	snprintf(path, sizeof(path), "nitro:/languages/%s/photo_default.png", ms().getGuiLanguageString().c_str());
	currentPhotoPath = path;
	loadPhoto(path, false);
	currentPhotoIsBootstrap = false;
	return true;
}

void reloadPhoto() {
	if (!currentPhotoIsBootstrap) {
		loadPhoto(currentPhotoPath, true);
		return;
	}

	// If no photos found, try find a bootstrap screenshot
	FILE *file = fopen(sys().isRunFromSD() ? "sd:/_nds/nds-bootstrap/screenshots.tar" : "fat:/_nds/nds-bootstrap/screenshots.tar", "rb");
	if (!file)
		file = fopen(sys().isRunFromSD() ? "fat:/_nds/nds-bootstrap/screenshots.tar" : "sd:/_nds/nds-bootstrap/screenshots.tar", "rb");
	
	if (!file) {
		return;
	}

	std::vector<int> screenshots;
	fseek(file, 0x200, SEEK_SET);
	for (int i = 0; i < 50; i++) {
		if (fgetc(file) == 'B')
			screenshots.push_back(i);
		fseek(file, 0x18400 - 1, SEEK_CUR);
	}

	if (screenshots.size() > 0) {
		fseek(file, 0x200 + 0x18400 * currentBootstrapPhoto, SEEK_SET);
		loadBootstrapScreenshot(file, true);
	}
}

void loadPhoto(const std::string &path, const bool bufferOnly) {
	std::vector<unsigned char> image;
	bool alternatePixel = false;

	lodepng::decode(image, photoWidth, photoHeight, path);

	if (photoWidth > 208 || photoHeight > 156) {
		image.clear();
		// Image is too big, load the default
		lodepng::decode(image, photoWidth, photoHeight, "nitro:/graphics/photo_default.png");
	}

	for (uint i=0;i<image.size()/4;i++) {
		u8 pixelAdjustInfo = 0;
		if (boxArtColorDeband) {
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
		}
		u16 color = image[i*4]>>3 | (image[(i*4)+1]>>3)<<5 | (image[(i*4)+2]>>3)<<10 | BIT(15);
		if (image[(i*4)+3] == 255) {
			tex().photoBuffer()[i] = color;
		} else {
			tex().photoBuffer()[i] = alphablend(color, 0, image[(i*4)+3]);
		}
		if (colorTable) {
			tex().photoBuffer()[i] = colorTable[tex().photoBuffer()[i] % 0x8000] | BIT(15);
		}
		if (boxArtColorDeband) {
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
			color = image[i*4]>>3 | (image[(i*4)+1]>>3)<<5 | (image[(i*4)+2]>>3)<<10 | BIT(15);
			if (image[(i*4)+3] == 255) {
				tex().photoBuffer2()[i] = color;
			} else {
				tex().photoBuffer2()[i] = alphablend(color, 0, image[(i*4)+3]);
			}
			if (colorTable) {
				tex().photoBuffer2()[i] = colorTable[tex().photoBuffer2()[i] % 0x8000] | BIT(15);
			}
			if ((i % photoWidth) == photoWidth-1) alternatePixel = !alternatePixel;
			alternatePixel = !alternatePixel;
		}
	}

	if (bufferOnly) {
		return;
	}

	u16 *bgSubBuffer = tex().beginBgSubModify();
	u16* bgSubBuffer2 = tex().bgSubBuffer2();

	// Fill area with black
	for (int y = 24; y < 180; y++) {
		dmaFillHalfWords(0x8000, bgSubBuffer + (y * 256) + 24, 208 * 2);
		if (boxArtColorDeband) {
			dmaFillHalfWords(0x8000, bgSubBuffer2 + (y * 256) + 24, 208 * 2);
		}
	}

	// Start loading
	u16 *src = tex().photoBuffer();
	u16 *src2 = tex().photoBuffer2();
	uint startX = 24 + (208 - photoWidth) / 2;
	uint y = 24 + ((156 - photoHeight) / 2);
	uint x = startX;
	for (uint i = 0; i < photoWidth * photoHeight; i++) {
		if (x >= startX + photoWidth) {
			x = startX;
			y++;
		}
		bgSubBuffer[y * 256 + x] = *(src++);
		if (boxArtColorDeband) {
			bgSubBuffer2[y * 256 + x] = *(src2++);
		}
		x++;
	}
	tex().commitBgSubModify();
}

void loadBootstrapScreenshot(FILE *file, const bool bufferOnly) {
	// Simple check to ensure we're seeked to a BMP
	if (fgetc(file) != 'B' || fgetc(file) != 'M')
		return;

	photoWidth = 208;
	photoHeight = 156;

	// All nds-bootstrap screenshots should be this size, if it's not something's wrong
	u32 bmpSize;
	fread(&bmpSize, 4, 1, file);
	if (bmpSize != 0x18046)
		return;

	fseek(file, 0x40, SEEK_CUR);
	u16 *buffer = new u16[256 * 192];
	fread(buffer, 2, 256 * 192, file);

	u16 *bgSubBuffer = tex().beginBgSubModify();
	u16* bgSubBuffer2 = tex().bgSubBuffer2();

	if (!bufferOnly) {
		// Fill area with black
		for (int y = 24; y < 180; y++) {
			dmaFillHalfWords(0x8000, bgSubBuffer + (y * 256) + 24, 208 * 2);
		}
	}

	// Start loading
	for (uint row = 0; row < photoHeight; row++) {
		for (uint col = 0; col < photoWidth; col++) {
			u16 val = buffer[(row * 12 / 10) * 256 + (col * 12 / 10)];

			// RGB 565 -> BGR 5551
			val = ((val >> 11) & 0x1F) | ((val & (0x1F << 6)) >> 1) | ((val & 0x1F) << 10) | BIT(15);
			/* if (colorTable) {
				val = colorTable[val % 0x8000] | BIT(15); // TODO: Remove this when nds-bootstrap supports color modes
			} */

			u8 y = photoHeight - row - 1;
			if (!bufferOnly) {
				bgSubBuffer[(24 + y) * 256 + 24 + col] = val;
			}
			tex().photoBuffer()[y * photoWidth + col] = val;
			if (boxArtColorDeband) {
				if (!bufferOnly) {
					bgSubBuffer2[(24 + y) * 256 + 24 + col] = val;
				}
				tex().photoBuffer2()[y * photoWidth + col] = val;
			}
		}
	}
	if (!bufferOnly) {
		tex().commitBgSubModify();
	}

	delete[] buffer;
}

static std::string loadedDate;

ITCM_CODE void drawCurrentDate() {
	// Load date
	std::string currentDate = getDate();
	if (currentDate == loadedDate && !reloadDate)
		return;

	loadedDate = currentDate;

	ms().macroMode ? tex().drawDateTimeMacro(loadedDate.c_str(), tc().dateRenderX(), tc().dateRenderY(), true) : tex().drawDateTime(loadedDate.c_str(), tc().dateRenderX(), tc().dateRenderY(), true);

	reloadDate = false;
}

static std::string loadedTime;

ITCM_CODE void drawCurrentTime() {
	// Load time
	std::string currentTime = retTime();
	if (currentTime[0] == ' ')
		currentTime[0] = '0';
	currentTime[2] = showColon ? ':' : ' ';

	if (currentTime == loadedTime && !reloadTime)
		return;

	loadedTime = currentTime;

	ms().macroMode ? tex().drawDateTimeMacro(currentTime.c_str(), tc().timeRenderX(), tc().timeRenderY(), false) : tex().drawDateTime(currentTime.c_str(), tc().timeRenderX(), tc().timeRenderY(), false);

	reloadTime = false;
}

void clearBoxArt() {
	if (ms().macroMode) return;
	tex().drawOverBoxArt(photoWidth, photoHeight);
}

// static char videoFrameFilename[256];

void graphicsInit() {
	logPrint("graphicsInit()\n");

	// for (int i = 0; i < 12; i++) {
	// 	launchDotFrame[i] = 5;
	// }

	if (ms().theme == TWLSettings::EThemeDSi) {
		titleboxXspeed = 8;
		iconYposOnTitleBox = 13;
	}

	for (int i = 0; i < 5; i++) {
		dropBounce[i] = 0;
		dropSpeed[i] = initialDropSpeed;

		if (ms().theme == TWLSettings::ETheme3DS || ms().theme == TWLSettings::EThemeSaturn || ms().theme == TWLSettings::EThemeHBL) {
			dropDownY[i] = 0.0f;
			titleboxYposDropDown[i] = 0;
		} else {
			dropDownY[i] = -85.0f - 80.0f;
			titleboxYposDropDown[i] = dropDownY[i];
		}
	}
	int dropDownType = rand() % 4;
	switch (dropDownType) {
		// Left to Right
		case 0:
			for (int i = 0; i < 5; i++) dropDelay[i] = 6 + i * 4;
			break;

		// Right to Left
		case 1:
			for (int i = 0; i < 5; i++) dropDelay[i] = 6 + (4-i) * 4;
			break;

		// V-Shape (\/)
		case 2:
			dropDelay[2] = 8;
			for (int i = 1; i <= 2; i++) {
				dropDelay[2+i] = 8+(i*3);
				dropDelay[2-i] = 8+(i*3);
			}
			break;

		// Inverted V-Shape (/\)
		case 3:
		default:
			dropDelay[2] = 14;
			for (int i = 1; i <= 2; i++) {
				dropDelay[2+i] = 14-(i*3);
				dropDelay[2-i] = 14-(i*3);
			}
			break;
	}
	dropDown = false;

	titleboxXpos[0] = ms().cursorPosition[0] * titleboxXspacing;
	titleboxXdest[0] = ms().cursorPosition[0] * titleboxXspacing;
	titlewindowXpos[0] = ms().cursorPosition[0] * 5;
	titlewindowXdest[0] = ms().cursorPosition[0] * 5;
	titleboxXpos[1] = ms().cursorPosition[1] * titleboxXspacing;
	titleboxXdest[1] = ms().cursorPosition[1] * titleboxXspacing;
	titlewindowXpos[1] = ms().cursorPosition[1] * 5;
	titlewindowXdest[1] = ms().cursorPosition[1] * 5;

	SetBrightness(0, (ms().theme == TWLSettings::EThemeSaturn || ms().theme == TWLSettings::EThemeHBL) ? -31 : 31);
	SetBrightness(1, (ms().theme == TWLSettings::EThemeSaturn || ms().theme == TWLSettings::EThemeHBL) && !ms().macroMode ? -31 : 31);

	// videoSetup() Called here before.
	// REG_BLDCNT = BLEND_SRC_BG3 | BLEND_FADE_BLACK;

	// swiWaitForVBlank();
	titleboxYpos = tc().titleboxRenderY();
	bubbleYpos = tc().bubbleTipRenderY();
	bubbleXpos = tc().bubbleTipRenderX();

	if (ms().theme == TWLSettings::ETheme3DS) {
		//tex().load3DSTheme();
		rocketVideo_videoYpos = tc().rotatingCubesRenderY();
	}

	if (!ms().macroMode) {
		tex().drawTopBg();
	}
	bottomBgLoad(false, true);
	// consoleDemoInit();

	if (ms().theme != TWLSettings::EThemeSaturn && ms().theme != TWLSettings::EThemeHBL) {
		tex().drawProfileName();
	}

	drawCurrentDate();
	drawCurrentTime();

	// printf("drawn bgload");
	// while (1) {}
	if (!ms().macroMode && ms().showPhoto && tc().renderPhoto()) {
		srand(time(NULL));
		loadPhotoList();
	}

	if (ms().theme == TWLSettings::EThemeHBL) {
		u16* newPalette = (u16*)bubblesPal;
		if (colorTable) {
			for (int i2 = 0; i2 < 6; i2++) {
				*(newPalette+i2) = colorTable[*(newPalette+i2) % 0x8000];
			}
		}

		// Load the texture here.
		hblBubblesID = glLoadTileSet(hblBubbles,   // pointer to glImage array
					   16,	       // sprite width
					   16,	       // sprite height
					   32,	       // bitmap width
					   64,	       // bitmap height
					   GL_RGB16,	   // texture type for glTexImage2D() in videoGL.h
					   TEXTURE_SIZE_32, // sizeX for glTexImage2D() in videoGL.h
					   TEXTURE_SIZE_64, // sizeY for glTexImage2D() in videoGL.h
					   TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
					   6,	    // Length of the palette to use (6 colors)
					   (u16 *)newPalette, // Load our 16 color tiles palette
					   (u8 *)bubblesBitmap   // image data generated by GRIT
		);
	}

	tex().drawVolumeImageCached();
	tex().drawBatteryImageCached();
	irqSet(IRQ_VBLANK, vBlankHandler);
	irqEnable(IRQ_VBLANK);
	irqSet(IRQ_VCOUNT, frameRateHandler);
	irqEnable(IRQ_VCOUNT);
	// consoleDemoInit();
}
