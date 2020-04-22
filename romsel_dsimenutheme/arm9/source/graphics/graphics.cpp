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

#include "FontGraphic.h"
#include "common/gl2d.h"
#include <ctime>
#include <dirent.h>
#include <maxmod9.h>
#include <nds.h>
#include <nds/arm9/dldi.h>

#include "bubbles.h"	// For HBL theme

// This is use for the top font.
#include "../include/startborderpal.h"

#include "tool/colortool.h"

#include "ThemeConfig.h"
#include "queueControl.h"
#include "uvcoord_date_time_font.h"
#include "uvcoord_top_font.h"

#include "../errorScreen.h"
#include "../iconTitle.h"
#include "../language.h"
#include "../ndsheaderbanner.h"
#include "../perGameSettings.h"
#include "common/dsimenusettings.h"
#include "common/flashcard.h"
#include "common/systemdetails.h"
#include "common/lzss.h"
#include "fontHandler.h"
#include "graphics.h"
#include "graphics/ThemeTextures.h"
#include "graphics/lodepng.h"
#include "themefilenames.h"
#include "date.h"
#include "iconHandler.h"
#include "sound.h"
#include "launchDots.h"
#define CONSOLE_SCREEN_WIDTH 32
#define CONSOLE_SCREEN_HEIGHT 24

static glImage hblBubbles[32 * 64];
static int hblBubblesID = 0;

static int backBubblesYpos_def[4] = {256, 256+56, 256+32, 256+16};
static int frontBubblesYpos_def[3] = {256, 256+64, 256+32};

static int backBubblesYpos[4] = {256, 256+56, 256+32, 256+16};
static int frontBubblesYpos[3] = {256, 256+64, 256+32};

extern u16 usernameRendered[11];

extern bool whiteScreen;
extern bool fadeType;
extern bool fadeSpeed;
extern bool fadeColor;
extern bool controlTopBright;
extern bool controlBottomBright;
int fadeDelay = 0;


extern int colorRvalue;
extern int colorGvalue;
extern int colorBvalue;

extern bool dropDown;
int dropTime[5];
int dropSeq[5];
#define dropSpeedDefine 7
int dropSpeed[5] = {dropSpeedDefine};
int dropSpeedChange[5];
int titleboxYposDropDown[5] = {-85 - 80};
int allowedTitleboxForDropDown = 0;
int delayForTitleboxToDropDown = 0;
extern int currentBg;
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
bool displayGameIcons = false;

extern bool buttonArrowTouched[2];
extern bool scrollWindowTouched;

extern bool titleboxXmoveleft;
extern bool titleboxXmoveright;

extern bool applaunchprep;

int screenBrightness = 31;

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
int titleboxXpos[2];
int titleboxYpos = 85; // 85, when dropped down
int titlewindowXpos[2];

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


// int launchDotFrame[12];
// int launchDotCurrentChangingFrame = 0;
// bool launchDotDoFrameChange = false;

bool showdialogbox = false;
bool dboxInFrame = false;
bool dbox_showIcon = false;
bool dbox_selectMenu = false;
float dbox_movespeed = 22;
float dbox_Ypos = -192;
int bottomScreenBrightness = 255;

int bottomBgState = 0; // 0 = Uninitialized 1 = No Bubble 2 = bubble 3 = moving.

int vblankRefreshCounter = 0;

static bool rotatingCubesLoaded = false;

bool rocketVideo_playVideo = false;
int rocketVideo_videoYpos = 78;
int frameOf60fps = 60;
int rocketVideo_videoFrames = 249;
int rocketVideo_currentFrame = -1;
int rocketVideo_frameDelay = 0;
bool rocketVideo_frameDelayEven = true; // For 24FPS
bool bottomField = false;
bool rocketVideo_loadFrame = true;

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

// Ported from PAlib (obsolete)
void SetBrightness(u8 screen, s8 bright) {
	u16 mode = 1 << 14;

	if (bright < 0) {
		mode = 2 << 14;
		bright = -bright;
	}
	if (bright > 31)
		bright = 31;
	*(u16 *)(0x0400006C + (0x1000 * screen)) = bright + mode;
}

void moveIconClose(int num) {
	if (titleboxXmoveleft) {
		movecloseXpos = 0;
		if (movetimer == 1) {
			if (CURPOS - 2 == num)
				movecloseXpos = 1;
			else if (CURPOS + 2 == num)
				movecloseXpos = -2;
		} else if (movetimer == 2) {
			if (CURPOS - 2 == num)
				movecloseXpos = 1;
			else if (CURPOS + 2 == num)
				movecloseXpos = -2;
		} else if (movetimer == 3) {
			if (CURPOS - 2 == num)
				movecloseXpos = 2;
			else if (CURPOS + 2 == num)
				movecloseXpos = -3;
		} else if (movetimer == 4) {
			if (CURPOS - 2 == num)
				movecloseXpos = 2;
			else if (CURPOS + 2 == num)
				movecloseXpos = -3;
		} else if (movetimer == 5) {
			if (CURPOS - 2 == num)
				movecloseXpos = 3;
			else if (CURPOS + 2 == num)
				movecloseXpos = -4;
		} else if (movetimer == 6) {
			if (CURPOS - 2 == num)
				movecloseXpos = 4;
			else if (CURPOS + 2 == num)
				movecloseXpos = -5;
		} else if (movetimer == 7) {
			if (CURPOS - 2 == num)
				movecloseXpos = 5;
			else if (CURPOS + 2 == num)
				movecloseXpos = -6;
		} else if (movetimer == 8) {
			if (CURPOS - 2 == num)
				movecloseXpos = 6;
			else if (CURPOS + 2 == num)
				movecloseXpos = -7;
		}
	}
	if (titleboxXmoveright) {
		movecloseXpos = 0;
		if (movetimer == 1) {
			if (CURPOS - 2 == num)
				movecloseXpos = 2;
			else if (CURPOS + 2 == num)
				movecloseXpos = -1;
		} else if (movetimer == 2) {
			if (CURPOS - 2 == num)
				movecloseXpos = 2;
			else if (CURPOS + 2 == num)
				movecloseXpos = -1;
		} else if (movetimer == 3) {
			if (CURPOS - 2 == num)
				movecloseXpos = 3;
			else if (CURPOS + 2 == num)
				movecloseXpos = -2;
		} else if (movetimer == 4) {
			if (CURPOS - 2 == num)
				movecloseXpos = 3;
			else if (CURPOS + 2 == num)
				movecloseXpos = -2;
		} else if (movetimer == 5) {
			if (CURPOS - 2 == num)
				movecloseXpos = 4;
			else if (CURPOS + 2 == num)
				movecloseXpos = -3;
		} else if (movetimer == 6) {
			if (CURPOS - 2 == num)
				movecloseXpos = 5;
			else if (CURPOS + 2 == num)
				movecloseXpos = -4;
		} else if (movetimer == 7) {
			if (CURPOS - 2 == num)
				movecloseXpos = 6;
			else if (CURPOS + 2 == num)
				movecloseXpos = -5;
		} else if (movetimer == 8) {
			if (CURPOS - 2 == num)
				movecloseXpos = 7;
			else if (CURPOS + 2 == num)
				movecloseXpos = -6;
		}
	}
	if (!titleboxXmoveleft || !titleboxXmoveright) {
		if (CURPOS - 2 == num)
			movecloseXpos = 6;
		else if (CURPOS + 2 == num)
			movecloseXpos = -6;
		else
			movecloseXpos = 0;
	}
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
	if (init || drawBubble == 0 || (drawBubble == 2 && ms().theme == 1)) {
		if (bottomBgState != 1) {
			tex().drawBottomBg(1);
			bottomBgState = 1;
		}
	} else if (drawBubble == 1) {
		if (bottomBgState != 2) {
			tex().drawBottomBg(2);
			bottomBgState = 2;
		}
	} else if (drawBubble == 2 && ms().theme == 0) {
		if (bottomBgState != 3) {
			tex().drawBottomBg(3);
			bottomBgState = 3;
		}
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

static u8 *rotatingCubesLocation = (u8 *)0x02700000;

void rvidFrameRateHandler(void) {
	if (!rocketVideo_playVideo)
		return;
	if (!rocketVideo_loadFrame) {
		frameOf60fps++;
		if (frameOf60fps > 60) frameOf60fps = 1;

		rocketVideo_frameDelay++;
		// 50FPS
		rocketVideo_loadFrame =   (frameOf60fps != 3
								&& frameOf60fps != 9
								&& frameOf60fps != 16
								&& frameOf60fps != 22
								&& frameOf60fps != 28
								&& frameOf60fps != 34
								&& frameOf60fps != 40
								&& frameOf60fps != 46
								&& frameOf60fps != 51
								&& frameOf60fps != 58);
	}
}

void playRotatingCubesVideo(void) {
	if (!rocketVideo_playVideo)
		return;

	if (rocketVideo_loadFrame) {
		DC_FlushRange((void*)(rotatingCubesLocation + (rocketVideo_currentFrame * 0x7000)), 0x7000);
		for (int v = 0; v < 56; v += 2) {
			dmaCopyWords(1, rotatingCubesLocation+(rocketVideo_currentFrame*(0x200*56)+(0x200*v)+(0x200*bottomField)), (u16*)BG_GFX_SUB+(256*(rocketVideo_videoYpos+v+bottomField)), 0x200);
			if (v == 54) {
				dmaCopyWordsAsynch(1, rotatingCubesLocation+(rocketVideo_currentFrame*(0x200*56)+(0x200*v)+(0x200*bottomField)), (u16*)BG_GFX_SUB+(256*(rocketVideo_videoYpos+v+1+bottomField)), 0x200);
			} else {
				dmaCopyWords(1, rotatingCubesLocation+(rocketVideo_currentFrame*(0x200*56)+(0x200*v)+(0x200*bottomField)), (u16*)BG_GFX_SUB+(256*(rocketVideo_videoYpos+v+1+bottomField)), 0x200);
			}
		}

		if (bottomField) {
			rocketVideo_currentFrame++;
			if (rocketVideo_currentFrame > rocketVideo_videoFrames) {
				rocketVideo_currentFrame = 0;
			}
		}
		bottomField = !bottomField;
		rocketVideo_frameDelay = 0;
		rocketVideo_frameDelayEven = !rocketVideo_frameDelayEven;
		rocketVideo_loadFrame = false;
	}
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

	if (ms().theme == 1 && rotatingCubesLoaded) {
		playRotatingCubesVideo();
	}

	glBegin2D();
	{
		if (fadeType == true) {
			if (!fadeDelay) {
				screenBrightness -= 1+(ms().theme<4 && fadeSpeed);
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
				screenBrightness += 1+(ms().theme<4 && fadeSpeed);
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
			SetBrightness(0, fadeColor ? screenBrightness : -screenBrightness);
		if (controlTopBright)
			SetBrightness(1, fadeColor ? screenBrightness : -screenBrightness);

		if (showdialogbox) {
			// Dialogbox moving into view...
			dboxInFrame = true;
			if (dbox_movespeed <= 1) {
				if (dbox_Ypos >= 0) {
					// dbox stopped
					dbox_movespeed = 0;
					dbox_Ypos = 0;
					bottomScreenBrightness = 127;
					REG_BLDY = (0b0100 << 1);
				} else {
					// dbox moving into view
					dbox_movespeed = 1;
				}
			} else {
				// Dbox decel
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
			dbox_Ypos += dbox_movespeed;
		} else {
			// Dialogbox moving down...
			if (dbox_Ypos <= -192 || dbox_Ypos >= 192) {
				dboxInFrame = false;
				dbox_movespeed = 22;
				dbox_Ypos = -192;
				bottomScreenBrightness = 255;
				REG_BLDY = 0;
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
			}
		}

		if (titleboxXmoveleft) {
			if (ms().theme == 4) {
				titleboxXpos[ms().secondaryDevice] -= 64;
				titlewindowXpos[ms().secondaryDevice] -= 8;
				titleboxXmoveleft = false;
			} else if (movetimer == 8) {
				//	if (currentBg && theme == 0) mmEffectEx(&snd_stop);
				needToPlayStopSound = true;
				startBorderZoomOut = true;
				titlewindowXpos[ms().secondaryDevice] -= 1;
				movetimer++;
			} else if (movetimer < 8) {
				titleboxXpos[ms().secondaryDevice] -=
					(isScrolling ? titleboxXscrollspeed : titleboxXmovespeed[movetimer]);
				if (movetimer == 0 || movetimer == 2 || movetimer == 4 || movetimer == 6)
					titlewindowXpos[ms().secondaryDevice] -= 1;
				movetimer++;
			} else {
				titleboxXmoveleft = false;
				movetimer = 0;
			}
		} else if (titleboxXmoveright) {
			if (ms().theme == 4) {
				titleboxXpos[ms().secondaryDevice] += 64;
				titlewindowXpos[ms().secondaryDevice] += 8;
				titleboxXmoveright = false;
			} else if (movetimer == 8) {
				//	if (currentBg && theme == 0) mmEffectEx(&snd_stop);
				needToPlayStopSound = true;
				startBorderZoomOut = true;
				titlewindowXpos[ms().secondaryDevice] += 1;
				movetimer++;
			} else if (movetimer < 8) {
				titleboxXpos[ms().secondaryDevice] +=
					(isScrolling ? titleboxXscrollspeed : titleboxXmovespeed[movetimer]);
				if (movetimer == 0 || movetimer == 2 || movetimer == 4 || movetimer == 6)
					titlewindowXpos[ms().secondaryDevice] += 1;
				movetimer++;
			} else {
				titleboxXmoveright = false;
				movetimer = 0;
			}
		}

		if (!whiteScreen && dropDown && ms().theme == 0) {
			int i2 = CURPOS - 2;
			if (i2 < 0)
				i2 += 5;
			for (int i = i2; i <= allowedTitleboxForDropDown + i2; i++) {
				if (dropSeq[i % 5] == 0) {
					titleboxYposDropDown[i % 5] += dropSpeed[i % 5];
					if (titleboxYposDropDown[i % 5] > 0)
						dropSeq[i % 5] = 1;
				} else if (dropSeq[i % 5] == 1) {
					titleboxYposDropDown[i % 5] -= dropSpeed[i % 5];
					dropTime[i % 5]++;
					dropSpeedChange[i % 5]++;
					if (dropTime[i % 5] >= 15) {
						dropSpeedChange[i % 5] = -1;
						dropSeq[i % 5] = 2;
					}
					if (dropSpeedChange[i % 5] == 2) {
						dropSpeed[i % 5]--;
						if (dropSpeed[i % 5] < 0)
							dropSpeed[i % 5] = 0;
						dropSpeedChange[i % 5] = -1;
					}
				} else if (dropSeq[i % 5] == 2) {
					titleboxYposDropDown[i % 5] += dropSpeed[i % 5];
					if (titleboxYposDropDown[i % 5] >= 0) {
						dropSeq[i % 5] = 3;
						titleboxYposDropDown[i % 5] = 0;
					}
					dropSpeedChange[i % 5]++;
					if (dropSpeedChange[i % 5] == 1) {
						dropSpeed[i % 5]++;
						if (dropSpeed[i % 5] > 6)
							dropSpeed[i % 5] = 6;
						dropSpeedChange[i % 5] = -1;
					}
				} else if (dropSeq[i % 5] == 3) {
					titleboxYposDropDown[i % 5] = 0;
				}
			}

			delayForTitleboxToDropDown++;
			if (delayForTitleboxToDropDown >= 5) {
				allowedTitleboxForDropDown++;
				if (allowedTitleboxForDropDown > 4)
					allowedTitleboxForDropDown = 4;
				delayForTitleboxToDropDown = 0;
			}
		}

		int bg_R = bottomScreenBrightness / 8;
		int bg_G = (bottomScreenBrightness / 8) - (3 * ms().blfLevel);
		if (bg_G < 0)
			bg_G = 0;
		int bg_B = (bottomScreenBrightness / 8) - (6 * ms().blfLevel);
		if (bg_B < 0)
			bg_B = 0;

		glColor(RGB15(bg_R, bg_G, bg_B));

		if (ms().theme == 5) {
			// Back bubbles
			int bubbleXpos = 16;
			for (int i = 0; i < 4; i++) {
				glSprite(bubbleXpos, backBubblesYpos[i], GL_FLIP_NONE, &hblBubbles[3]);
				bubbleXpos += 64;
				backBubblesYpos[i]--;
				if (backBubblesYpos[i] < -16) {
					backBubblesYpos[i] = backBubblesYpos_def[i];
				}
			}
			// Front bubbles
			bubbleXpos = 32;
			for (int i = 0; i < 3; i++) {
				glSprite(bubbleXpos, frontBubblesYpos[i], GL_FLIP_NONE, &hblBubbles[4]);
				glSprite(bubbleXpos+16, frontBubblesYpos[i], GL_FLIP_NONE, &hblBubbles[5]);
				glSprite(bubbleXpos, frontBubblesYpos[i]+16, GL_FLIP_NONE, &hblBubbles[6]);
				glSprite(bubbleXpos+16, frontBubblesYpos[i]+16, GL_FLIP_NONE, &hblBubbles[7]);
				bubbleXpos += 64;
				frontBubblesYpos[i] -= 2;
				if (frontBubblesYpos[i] < -32) {
					frontBubblesYpos[i] = frontBubblesYpos_def[i];
				}
			}
		}

		if (ms().theme == 0) {
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
				} else
					glSprite(bipXpos, 178, GL_FLIP_NONE, &tex().bipsImage()[1]);
				bipXpos += 5;
			}
			glSprite(16 + titlewindowXpos[ms().secondaryDevice], 171, GL_FLIP_NONE,
				 &tex().buttonarrowImage()[2 + scrollWindowTouched]);
			glSprite(0, 171, GL_FLIP_NONE, &tex().buttonarrowImage()[0 + buttonArrowTouched[0]]);
			glSprite(224, 171, GL_FLIP_H, &tex().buttonarrowImage()[0 + buttonArrowTouched[1]]);
			glSprite(72 - titleboxXpos[ms().secondaryDevice], 81, GL_FLIP_NONE, tex().braceImage());
		}

	if (displayGameIcons) {
		int spawnedboxXpos = 96;
		int iconXpos = 112;

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

		if (movingApp != -1) {
			if (movingAppIsDir) {
				if (ms().theme == 1)
					glSprite(96, titleboxYpos - movingAppYpos, GL_FLIP_NONE, tex().folderImage());
				else
					glSprite(96, titleboxYpos - movingAppYpos + titleboxYposDropDown[movingApp % 5],
						 GL_FLIP_NONE, tex().folderImage());
			} else {
				if (!bnrSysSettings[movingApp]) {
					if (ms().theme == 1) {
						glSprite(96, titleboxYpos - movingAppYpos, GL_FLIP_NONE,
							 tex().boxfullImage());
					} else {
						glSprite(96,
							 titleboxYpos - movingAppYpos +
								 titleboxYposDropDown[movingApp % 5],
							 GL_FLIP_NONE, &tex().boxfullImage()[0]);
					}
				}
				if (bnrSysSettings[movingApp])
					glSprite(96,
						 (titleboxYpos - 1) - movingAppYpos +
							 titleboxYposDropDown[movingApp % 5],
						 GL_FLIP_NONE, &tex().settingsImage()[1]);
				else if (bnrRomType[movingApp] == 9)
					drawIconPLG(112, (titleboxYpos + 12) - movingAppYpos +
								  titleboxYposDropDown[movingApp % 5]);
				else if (bnrRomType[movingApp] == 8)
					drawIconSNES(112, (titleboxYpos + 12) - movingAppYpos +
								  titleboxYposDropDown[movingApp % 5]);
				else if (bnrRomType[movingApp] == 7)
					drawIconMD(112, (titleboxYpos + 12) - movingAppYpos +
								titleboxYposDropDown[movingApp % 5]);
				else if (bnrRomType[movingApp] == 6)
					drawIconGG(112, (titleboxYpos + 12) - movingAppYpos +
								titleboxYposDropDown[movingApp % 5]);
				else if (bnrRomType[movingApp] == 5)
					drawIconSMS(112, (titleboxYpos + 12) - movingAppYpos +
								 titleboxYposDropDown[movingApp % 5]);
				else if (bnrRomType[movingApp] == 4)
					drawIconNES(112, (titleboxYpos + 12) - movingAppYpos +
								 titleboxYposDropDown[movingApp % 5]);
				else if (bnrRomType[movingApp] == 3)
					drawIconGBC(112, (titleboxYpos + 12) - movingAppYpos +
								 titleboxYposDropDown[movingApp % 5]);
				else if (bnrRomType[movingApp] == 2)
					drawIconGB(112, (titleboxYpos + 12) - movingAppYpos +
								titleboxYposDropDown[movingApp % 5]);
				else if (bnrRomType[movingApp] == 1)
					drawIconGBA(112, (titleboxYpos + 12) - movingAppYpos +
								titleboxYposDropDown[movingApp % 5]);
				else
					drawIcon(112,
						 (titleboxYpos + 12) - movingAppYpos +
							 titleboxYposDropDown[movingApp % 5],
						 -1);
			}
		}

		int maxIconNumber = (ms().theme == 4 ? 0 : 3);
		for (int i = 0; i < 40; i++) {
			int movingAppXFix = 0;
			if (CURPOS <= (movingApp - (PAGENUM * 40))) {
				if (i == CURPOS - 2)
					movingAppXFix = -20;
				else if (i == CURPOS - 1)
					movingAppXFix = -5;
				else if (i == CURPOS)
					movingAppXFix = 5;
				else if (CURPOS < (movingApp - (PAGENUM * 40)) - 1) {
					if (i == CURPOS + 1)
						movingAppXFix = 20;
				} else if (CURPOS == (movingApp - (PAGENUM * 40)) - 1) {
					if (i == CURPOS + 1)
						movingAppXFix = 20;
					else if (i == CURPOS + 2)
						movingAppXFix = 20;
				} else {
					if (i == CURPOS + 1)
						movingAppXFix = 5;
					else if (i == CURPOS + 2)
						movingAppXFix = 20;
				}
			} else {
				if (CURPOS == (movingApp - (PAGENUM * 40)) + 1) {
					if (i == CURPOS - 2)
						movingAppXFix = -20;
					else if (i == CURPOS - 1)
						movingAppXFix = -5;
				} else {
					if (i == CURPOS - 2)
						movingAppXFix = -20;
					else if (i == CURPOS - 1)
						movingAppXFix = -20;
				}
				if (i == CURPOS)
					movingAppXFix = -5;
				else if (i == CURPOS + 1)
					movingAppXFix = 5;
				else if (i == CURPOS + 2)
					movingAppXFix = 20;
			}

			if (ms().theme == 0) {
				moveIconClose(i);
			} else {
				movecloseXpos = 0;
			}
			if (i >= CURPOS - maxIconNumber && i <= CURPOS + maxIconNumber) {
				if (i < spawnedtitleboxes) {
					if (isDirectory[i]) {
						if (movingApp != -1) {
							int j = i;
							if (i > movingApp - (PAGENUM * 40))
								j--;
							if (ms().theme == 1)
								glSprite((j * 2496 / 39) + 128 -
										 titleboxXpos[ms().secondaryDevice] +
										 movingAppXFix,
									 titleboxYpos, GL_FLIP_NONE,
									 tex().folderImage());
							else
								glSprite((j * 2496 / 39) + 128 -
										 titleboxXpos[ms().secondaryDevice] +
										 movingAppXFix,
									 (titleboxYpos - 3) +
										 titleboxYposDropDown[i % 5],
									 GL_FLIP_NONE, tex().folderImage());
						} else {
							if (ms().theme == 1)
								glSprite(spawnedboxXpos -
										 titleboxXpos[ms().secondaryDevice] +
										 movecloseXpos,
									 titleboxYpos, GL_FLIP_NONE,
									 tex().folderImage());
							else
								glSprite(spawnedboxXpos -
										 titleboxXpos[ms().secondaryDevice] +
										 movecloseXpos,
									 (titleboxYpos - 3) +
										 titleboxYposDropDown[i % 5],
									 GL_FLIP_NONE, tex().folderImage());
						}
					} else if (!applaunchprep ||
						   CURPOS !=
							   i) { // Only draw the icon if we're not launching the selcted app
						if (movingApp != -1) {
							int j = i;
							if (i > movingApp - (PAGENUM * 40))
								j--;
							if (j == -1)
								continue;
							if (!bnrSysSettings[i]) {
								if (ms().theme == 1) {
									glSprite(
										(j * 2496 / 39) + 128 -
										titleboxXpos[ms().secondaryDevice] +
										movingAppXFix,
										titleboxYpos, GL_FLIP_NONE,
										tex().boxfullImage());
								} else {
									glSprite(
										(j * 2496 / 39) + 128 -
										titleboxXpos[ms().secondaryDevice] +
										movingAppXFix,
										titleboxYpos + titleboxYposDropDown[i % 5],
										GL_FLIP_NONE, &tex().boxfullImage()[0]);
								}
							}
							if (bnrSysSettings[i])
								glSprite((j * 2496 / 39) + 128 -
										 titleboxXpos[ms().secondaryDevice] +
										 movingAppXFix,
									 (titleboxYpos - 1) +
										 titleboxYposDropDown[i % 5],
									 GL_FLIP_NONE, &tex().settingsImage()[1]);
							else if (bnrRomType[i] == 9)
								drawIconPLG((j * 2496 / 39) + 144 -
										 titleboxXpos[ms().secondaryDevice] +
										 movingAppXFix,
										 (titleboxYpos + 12) +
										 titleboxYposDropDown[i % 5]);
							else if (bnrRomType[i] == 8)
								drawIconSNES((j * 2496 / 39) + 144 -
										 titleboxXpos[ms().secondaryDevice] +
										 movingAppXFix,
										 (titleboxYpos + 12) +
										 titleboxYposDropDown[i % 5]);
							else if (bnrRomType[i] == 7)
								drawIconMD((j * 2496 / 39) + 144 -
										   titleboxXpos[ms().secondaryDevice] +
										   movingAppXFix,
									   (titleboxYpos + 12) +
										   titleboxYposDropDown[i % 5]);
							else if (bnrRomType[i] == 6)
								drawIconGG((j * 2496 / 39) + 144 -
										   titleboxXpos[ms().secondaryDevice] +
										   movingAppXFix,
									   (titleboxYpos + 12) +
										   titleboxYposDropDown[i % 5]);
							else if (bnrRomType[i] == 5)
								drawIconSMS((j * 2496 / 39) + 144 -
										titleboxXpos[ms().secondaryDevice] +
										movingAppXFix,
										(titleboxYpos + 12) +
										titleboxYposDropDown[i % 5]);
							else if (bnrRomType[i] == 4)
								drawIconNES((j * 2496 / 39) + 144 -
										titleboxXpos[ms().secondaryDevice] +
										movingAppXFix,
										(titleboxYpos + 12) +
										titleboxYposDropDown[i % 5]);
							else if (bnrRomType[i] == 3)
								drawIconGBC((j * 2496 / 39) + 144 -
										titleboxXpos[ms().secondaryDevice] +
										movingAppXFix,
										(titleboxYpos + 12) +
										titleboxYposDropDown[i % 5]);
							else if (bnrRomType[i] == 2)
								drawIconGB((j * 2496 / 39) + 144 -
										   titleboxXpos[ms().secondaryDevice] +
										   movingAppXFix,
									   (titleboxYpos + 12) +
										   titleboxYposDropDown[i % 5]);
							else if (bnrRomType[i] == 1)
								drawIconGBA((j * 2496 / 39) + 144 -
										   titleboxXpos[ms().secondaryDevice] +
										   movingAppXFix,
									   (titleboxYpos + 12) +
										   titleboxYposDropDown[i % 5]);
							else
								drawIcon((j * 2496 / 39) + 144 -
										 titleboxXpos[ms().secondaryDevice] +
										 movingAppXFix,
									 (titleboxYpos + 12) +
										 titleboxYposDropDown[i % 5],
									 i);
						} else {
							if (!bnrSysSettings[i]) {
								if (ms().theme == 1) {
									glSprite(spawnedboxXpos -
											 titleboxXpos[ms().secondaryDevice],
										 titleboxYpos, GL_FLIP_NONE,
										 tex().boxfullImage());
								} else {
									glSprite(
										spawnedboxXpos -
										titleboxXpos[ms().secondaryDevice] +
										movecloseXpos,
										titleboxYpos + titleboxYposDropDown[i % 5],
										GL_FLIP_NONE, &tex().boxfullImage()[0]);
								}
							}
							if (bnrSysSettings[i])
								glSprite(spawnedboxXpos -
										 titleboxXpos[ms().secondaryDevice] +
										 movecloseXpos,
									 (titleboxYpos - 1) +
										 titleboxYposDropDown[i % 5],
									 GL_FLIP_NONE, &tex().settingsImage()[1]);
							else if (bnrRomType[i] == 9)
								drawIconPLG(
									iconXpos - titleboxXpos[ms().secondaryDevice] +
									movecloseXpos,
									(titleboxYpos + 12) + titleboxYposDropDown[i % 5]);
							else if (bnrRomType[i] == 8)
								drawIconSNES(
									iconXpos - titleboxXpos[ms().secondaryDevice] +
									movecloseXpos,
									(titleboxYpos + 12) + titleboxYposDropDown[i % 5]);
							else if (bnrRomType[i] == 7)
								drawIconMD(
									iconXpos - titleboxXpos[ms().secondaryDevice] +
									movecloseXpos,
									(titleboxYpos + 12) + titleboxYposDropDown[i % 5]);
							else if (bnrRomType[i] == 6)
								drawIconGG(
									iconXpos - titleboxXpos[ms().secondaryDevice] +
									movecloseXpos,
									(titleboxYpos + 12) + titleboxYposDropDown[i % 5]);
							else if (bnrRomType[i] == 5)
								drawIconSMS(
									iconXpos - titleboxXpos[ms().secondaryDevice] +
									movecloseXpos,
									(titleboxYpos + 12) + titleboxYposDropDown[i % 5]);
							else if (bnrRomType[i] == 4)
								drawIconNES(
									iconXpos - titleboxXpos[ms().secondaryDevice] +
									movecloseXpos,
									(titleboxYpos + 12) + titleboxYposDropDown[i % 5]);
							else if (bnrRomType[i] == 3)
								drawIconGBC(
									iconXpos - titleboxXpos[ms().secondaryDevice] +
									movecloseXpos,
									(titleboxYpos + 12) + titleboxYposDropDown[i % 5]);
							else if (bnrRomType[i] == 2)
								drawIconGB(
									iconXpos - titleboxXpos[ms().secondaryDevice] +
									movecloseXpos,
									(titleboxYpos + 12) + titleboxYposDropDown[i % 5]);
							else if (bnrRomType[i] == 1)
								drawIconGBA(
									iconXpos - titleboxXpos[ms().secondaryDevice] +
									movecloseXpos,
									(titleboxYpos + 12) + titleboxYposDropDown[i % 5]);
							else
								drawIcon(iconXpos - titleboxXpos[ms().secondaryDevice] +
										 movecloseXpos,
									 (titleboxYpos + 12) +
										 titleboxYposDropDown[i % 5],
									 i);
						}
					}
				} else {
					// Empty box
					if (movingApp != -1) {
						if (ms().theme > 0) {
							glSprite(((i - 1) * 2496 / 39) + 128 -
									 titleboxXpos[ms().secondaryDevice] + movingAppXFix,
								 titleboxYpos + titleboxYposDropDown[i % 5],
								 GL_FLIP_NONE, tex().boxemptyImage());
						} else {
							glSprite(((i - 1) * 2496 / 39) + 128 -
									 titleboxXpos[ms().secondaryDevice] + movingAppXFix,
								 titleboxYpos + titleboxYposDropDown[i % 5],
								 GL_FLIP_NONE, &tex().boxfullImage()[1]);
						}
					} else {
						if (ms().theme > 0) {
							glSprite(spawnedboxXpos - titleboxXpos[ms().secondaryDevice],
								 titleboxYpos + titleboxYposDropDown[i % 5],
								 GL_FLIP_NONE, tex().boxemptyImage());
						} else {
							glSprite(spawnedboxXpos - titleboxXpos[ms().secondaryDevice] +
									 movecloseXpos,
								 titleboxYpos + titleboxYposDropDown[i % 5],
								 GL_FLIP_NONE, &tex().boxfullImage()[1]);
						}
					}
				}
			}
			spawnedboxXpos += 64;
			iconXpos += 64;
		}
		if (ms().theme == 0) {
			glSprite(spawnedboxXpos + 10 - titleboxXpos[ms().secondaryDevice], 81, GL_FLIP_H,
				 tex().braceImage());
		}

		if (movingApp != -1 && ms().theme==0 && showMovingArrow) {
			if (movingArrowYdirection) {
				movingArrowYpos += 0.33;
				if (movingArrowYpos > 67)
					movingArrowYdirection = false;
			} else {
				movingArrowYpos -= 0.33;
				if (movingArrowYpos < 59)
					movingArrowYdirection = true;
			}
			glSprite(115, movingArrowYpos, GL_FLIP_NONE, tex().movingArrowImage());
		}
	}
		// Top icons for 3DS theme
		if (ms().theme == 1) {
			int topIconXpos = 116;
			if (isDSiMode() && sdFound()) {
				for (int i = 0; i < 2; i++) {
					topIconXpos -= 14;
				}
				if (ms().secondaryDevice) {
					glSprite(topIconXpos, 1, GL_FLIP_NONE, &tex().smallCartImage()[2]); // SD card
				} else {
					glSprite(topIconXpos, 1, GL_FLIP_NONE,
						 &tex().smallCartImage()[(REG_SCFG_MC == 0x11) ? 1 : 0]); // Slot-1 card
				}
				topIconXpos += 28;
				drawSmallIconGBA(topIconXpos, 1); // GBARunner2
			} else {
				// for (int i = 0; i < 2; i++) {
					topIconXpos -= 14;
				//}
				if (ms().useGbarunner) {
					drawSmallIconGBA(topIconXpos, 1); // GBARunner2
				} else {
					glSprite(topIconXpos, 1, GL_FLIP_NONE, &tex().smallCartImage()[3]); // GBA Mode
				}
			}
			topIconXpos += 28;
			glSprite(topIconXpos, 1, GL_FLIP_NONE, &tex().smallCartImage()[4]); // Manual

			// Replace by baked-in backgrounds on 3DS.

			// glSprite(0, 0, GL_FLIP_NONE, &tex().cornerButtonImage()[0]);
			// if (!sys().isRegularDS())
			// 	glSprite(256 - 44, 0, GL_FLIP_NONE, &tex().cornerButtonImage()[1]);
		}

		if (applaunchprep) {

			if (isDirectory[CURPOS]) {
				glSprite(96, 87 - titleboxYmovepos, GL_FLIP_NONE, tex().folderImage());
			} else {
				if (!bnrSysSettings[CURPOS]) {
					glSprite(96, 84 - titleboxYmovepos, GL_FLIP_NONE, tex().boxfullImage());
				}
				if (bnrSysSettings[CURPOS])
					glSprite(84, 83 - titleboxYmovepos, GL_FLIP_NONE, &tex().settingsImage()[1]);
				else if (bnrRomType[CURPOS] == 9)
					drawIconPLG(112, 96 - titleboxYmovepos);
				else if (bnrRomType[CURPOS] == 8)
					drawIconSNES(112, 96 - titleboxYmovepos);
				else if (bnrRomType[CURPOS] == 7)
					drawIconMD(112, 96 - titleboxYmovepos);
				else if (bnrRomType[CURPOS] == 6)
					drawIconGG(112, 96 - titleboxYmovepos);
				else if (bnrRomType[CURPOS] == 5)
					drawIconSMS(112, 96 - titleboxYmovepos);
				else if (bnrRomType[CURPOS] == 4)
					drawIconNES(112, 96 - titleboxYmovepos);
				else if (bnrRomType[CURPOS] == 3)
					drawIconGBC(112, 96 - titleboxYmovepos);
				else if (bnrRomType[CURPOS] == 2)
					drawIconGB(112, 96 - titleboxYmovepos);
				else if (bnrRomType[CURPOS] == 1)
					drawIconGBA(112, 96 - titleboxYmovepos);
				else
					drawIcon(112, 96 - titleboxYmovepos, CURPOS);
			}
			// Draw dots after selecting a game/app

			dots().drawAuto();

			titleboxYmovepos += 5;
		}
		if (showSTARTborder && displayGameIcons && (ms().theme < 4) && (!isScrolling || ms().theme == 1)) {
			glSprite(96, tc().startBorderRenderY(), GL_FLIP_NONE,
				 &tex().startbrdImage()[startBorderZoomAnimSeq[startBorderZoomAnimNum] &
							(tc().startBorderSpriteH() - 1)]);
			glSprite(96 + tc().startBorderSpriteW(), tc().startBorderRenderY(), GL_FLIP_H,
				 &tex().startbrdImage()[startBorderZoomAnimSeq[startBorderZoomAnimNum] &
							(tc().startBorderSpriteH() - 1)]);
			if (bnrWirelessIcon[CURPOS] > 0)
				glSprite(96, tc().startBorderRenderY(), GL_FLIP_NONE,
					 &tex().wirelessIcons()[(bnrWirelessIcon[CURPOS] - 1) & 31]);

			if (ms().theme == 0) {
				if (currentBg == 1 && ms().theme == 0 && needToPlayStopSound &&
					waitForNeedToPlayStopSound == 0) {
					// mmEffectEx(&snd_stop);
					snd().playStop();
					waitForNeedToPlayStopSound = 1;
					needToPlayStopSound = false;
				}
				// glSprite(96, tc().startBorderRenderY(), GL_FLIP_NONE,
				// 	 &tex().startbrdImage()[startBorderZoomAnimSeq[startBorderZoomAnimNum] &
				// (tc().startBorderSpriteH() - 1)]); glSprite(96 + tc().startBorderSpriteW(),
				// tc().startBorderRenderY(), GL_FLIP_H,
				// 	 &tex().startbrdImage()[startBorderZoomAnimSeq[startBorderZoomAnimNum] &
				// (tc().startBorderSpriteH() - 1)]); if (bnrWirelessIcon[CURPOS] > 0) 	glSprite(96,
				// tc().startBorderRenderY(), GL_FLIP_NONE,
				// 		 &tex().wirelessIcons()[(bnrWirelessIcon[CURPOS] - 1) & 31]);
			}
		}

		// Refresh the background layer.
		if (currentBg == 1 && ms().theme != 4 && ms().theme != 5)
			drawBubble(tex().bubbleImage());
		if (showSTARTborder && displayGameIcons && ms().theme == 0 && !isScrolling) {
			glSprite(96, tc().startTextRenderY(), GL_FLIP_NONE, &tex().startImage()[setLanguage]);
		}

		glColor(RGB15(31, 31 - (3 * ms().blfLevel), 31 - (6 * ms().blfLevel)));
		if (dbox_Ypos != -192 || (ms().theme == 4 && currentBg == 1) || ms().theme == 5) {
			// Draw the dialog box.
			if (ms().theme != 4 && ms().theme != 5) drawDbox();
			if (dbox_showIcon && !isDirectory[CURPOS]) {
				drawIcon(24, ((ms().theme == 4 || ms().theme == 5) ? 0 : dbox_Ypos) + 24, CURPOS);
			}
			if (dbox_selectMenu) {
				int selIconYpos = 96;
				if (isDSiMode() && memcmp(io_dldi_data->friendlyName, "CycloDS iEvolution", 18) != 0) {
					for (int i = 0; i < 4; i++) {
						selIconYpos -= 14;
					}
				} else {
					for (int i = 0; i < 3; i++) {
						selIconYpos -= 14;
					}
				}
				if (!sys().isRegularDS()) {
					glSprite(32, (ms().theme == 4 ? 0 : dbox_Ypos) + selIconYpos, GL_FLIP_NONE,
						 &tex().cornerButtonImage()[1]); // System Menu
					selIconYpos += 28;
				}
				glSprite(32, (ms().theme == 4 ? 0 : dbox_Ypos) + selIconYpos, GL_FLIP_NONE,
					 &tex().cornerButtonImage()[0]); // Settings
				selIconYpos += 28;
				if (isDSiMode() && memcmp(io_dldi_data->friendlyName, "CycloDS iEvolution", 18) != 0) {
					if (ms().secondaryDevice) {
						glSprite(32, (ms().theme == 4 ? 0 : dbox_Ypos) + selIconYpos, GL_FLIP_NONE,
							 &tex().smallCartImage()[2]); // SD card
					} else {
						glSprite(
							32, (ms().theme == 4 ? 0 : dbox_Ypos) + selIconYpos, GL_FLIP_NONE,
							&tex().smallCartImage()[(REG_SCFG_MC == 0x11) ? 1
												  : 0]); // Slot-1 card
					}
					selIconYpos += 28;
				}
				/*if (ms().useGbarunner) {
					drawSmallIconGBA(32, (ms().theme == 4 ? 0 : dbox_Ypos) + selIconYpos); // GBARunner2
				} else {
					glSprite(32, (ms().theme == 4 ? 0 : dbox_Ypos) + selIconYpos, GL_FLIP_NONE,
						 &tex().smallCartImage()[3]); // GBA Mode
				}
				selIconYpos += 28;*/
				glSprite(32, (ms().theme == 4 ? 0 : dbox_Ypos) + selIconYpos, GL_FLIP_NONE,
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
			glBoxFilled(0, 0, 256, 192, RGB15(31, 31 - (3 * ms().blfLevel), 31 - (6 * ms().blfLevel)));
			if (showProgressIcon)
				glSprite(224, 152, GL_FLIP_NONE, &tex().progressImage()[progressAnimNum]);
		}

		if (vblankRefreshCounter >= REFRESH_EVERY_VBLANKS) {
			if (showdialogbox && dbox_Ypos == -192) {
				// Reload the dialog box palettes here...
				reloadDboxPalette();
			} else if (!showdialogbox) {

				reloadIconPalettes();
				reloadFontPalettes();
			}
			vblankRefreshCounter = 0;
		} else {
			vblankRefreshCounter++;
		}
		updateText(false);
		//}
	}
	glEnd2D();
	GFX_FLUSH = 0;

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
		}
	}
	if (displayGameIcons || dbox_showIcon) {
		// Playback animated icons
		for (int i = 0; i < 41; i++) {
			if (bnriconisDSi[i] == true) {
				playBannerSequence(i);
			}
		}
	}

	if (ms().theme == 1) {
		startBorderZoomAnimDelay++;
		if (startBorderZoomAnimDelay == 8) {
			startBorderZoomAnimNum++;
			if (startBorderZoomAnimSeq[startBorderZoomAnimNum] == 0) {
				startBorderZoomAnimNum = 0;
			}
			startBorderZoomAnimDelay = 0;
		}
	} else if (startBorderZoomOut) {
		startBorderZoomAnimNum++;
		if (startBorderZoomAnimSeq[startBorderZoomAnimNum] == 0) {
			startBorderZoomAnimNum = 0;
			startBorderZoomOut = false;
		}
	} else {
		startBorderZoomAnimNum = 0;
	}

	// if (applaunchprep && ms().theme == 0 && launchDotDoFrameChange) {
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
	// if (applaunchprep && ms().theme == 0)
	// 	launchDotDoFrameChange = !launchDotDoFrameChange;

	bottomBgRefresh(); // Refresh the background image on vblank
}

void loadPhoto(const std::string &path);

void loadPhotoList() {
	DIR *dir;
	struct dirent *ent;
	std::string photoDir;
	std::string dirPath = "sd:/_nds/TWiLightMenu/dsimenu/photos/";
	std::vector<std::string> photoList;

	if ((dir = opendir(dirPath.c_str())) == NULL) {
		dirPath = "fat:/_nds/TWiLightMenu/dsimenu/photos/";
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
		if(photoList.size() > 0) {
			loadPhoto(photoList[rand() / ((RAND_MAX + 1u) / photoList.size())]);
			return;
		}
	}
	// If dir not opened or no photos found, then draw the default
	loadPhoto("nitro:/graphics/photo_default.png");
}

void loadPhoto(const std::string &path) {
	std::vector<unsigned char> image;

	lodepng::decode(image, photoWidth, photoHeight, path);

	if(photoWidth > 208 || photoHeight > 156) {
		image.clear();
		// Image is too big, load the default
		lodepng::decode(image, photoWidth, photoHeight, "nitro:/graphics/photo_default.png");
	}

	for(uint i=0;i<image.size()/4;i++) {
		tex().photoBuffer()[i] = image[i*4]>>3 | (image[(i*4)+1]>>3)<<5 | (image[(i*4)+2]>>3)<<10 | BIT(15);
		if (ms().colorMode == 1) {
			tex().photoBuffer()[i] = convertVramColorToGrayscale(tex().photoBuffer()[i]);
		}
	}

	u16 *bgSubBuffer = tex().beginBgSubModify();

	// Fill area with black
	for(int y = 24; y < 180; y++) {
		dmaFillHalfWords(0x8000, bgSubBuffer + (y * 256) + 24, 208 * 2);
	}

	// Start loading
	u16 *src = tex().photoBuffer();
	uint startX = 24 + (208 - photoWidth) / 2;
	uint y = 24 + ((156 - photoHeight) / 2);
	uint x = startX;
	for (uint i = 0; i < photoWidth * photoHeight; i++) {
		if (x >= startX + photoWidth) {
			x = startX;
			y++;
		}
		bgSubBuffer[y * 256 + x] = *(src++);
		x++;
	}
	tex().commitBgSubModify();
}

// Load photo without overwriting shoulder button images
void loadPhotoPart() {
	u16 *bgSubBuffer = tex().beginBgSubModify();

	// Fill area with black
	for(int y = 24; y < 172; y++) {
		dmaFillHalfWords(0x8000, bgSubBuffer + (y * 256) + 24, 208 * 2);
	}

	// Start loading
	u16 *src = tex().photoBuffer();
	uint startX = 24 + (208 - photoWidth) / 2;
	uint y = 24 + ((156 - photoHeight) / 2);
	uint x = startX;
	for (uint i = 0; i < photoWidth * photoHeight; i++) {
		if (x >= startX + photoWidth) {
			x = startX;
			y++;
			if(y >= 172)	break;
		}
		bgSubBuffer[y * 256 + x] = *(src++);
		x++;
	}
	tex().commitBgSubModify();
}

static std::string loadedDate;

void drawCurrentDate() {
	// Load date
	int x = (ms().theme >= 4 ? 122 : 162);
	if (ms().theme == 5) {
		x -= 28;
	}
	char date[6];

	if (!GetDate(FORMAT_MD, date, sizeof(date)))
		return;

	std::string currentDate = date;
	if (currentDate == loadedDate)
		return;

	loadedDate = date;

	tex().drawDateTime(date, x, 15, 5, NULL);
}

static std::string loadedTime;
static int hourWidth;
static bool initialClockDraw = true;

void drawCurrentTime() {
	// Load time
	int x = (ms().theme >= 4 ? 162 : 200);
	if (ms().theme == 5) {
		x -= 28;
	}
	char time[10];
	std::string currentTime = RetTime();
	if (currentTime != loadedTime) {
		loadedTime = currentTime;
		if (currentTime.substr(0, 1) == " ")
			currentTime = "0" + currentTime.substr(1);
		sprintf(time, currentTime.c_str());

		int howManyToDraw = 5;
		if (initialClockDraw) {
			initialClockDraw = false;
		} else {
			if (currentTime.substr(3, 2) != "00") {
				strcpy(time, currentTime.substr(3, 2).c_str());
				howManyToDraw = 2;
				x = hourWidth;
			}
		}
		tex().drawDateTime(time, x, 15, howManyToDraw, &hourWidth);
	}
}

void drawClockColon() {
	// Load time
	int x = (ms().theme >= 4 ? 176 : 214);
	if (ms().theme == 5) {
		x -= 28;
	}
	int imgY = 15;
	char colon[1];

	// Blink the ':' once per second.
	if (colonTimer >= 60) {
		colonTimer = 0;
		std::string currentColon = showColon ? ":" : ";";
		sprintf(colon, currentColon.c_str());
		tex().drawDateTime(colon, x, imgY, 1, NULL);
		showColon = !showColon;
	}
}

void clearBoxArt() {
	if (!tc().renderPhoto()) {
		// tex().drawTopBg();
		// tex().drawProfileName();
		// tex().drawBatteryImageCached();
		// tex().drawVolumeImageCached();
		// tex().drawShoulders(showLshoulder, showR);
		// drawCurrentDate();
		// drawCurrentTime();
		// drawClockColon();
		tex().drawTopBgAvoidingShoulders();
	} else {
		loadPhotoPart();
	}
}

// static char videoFrameFilename[256];

void loadRotatingCubes() {
	bool rvidCompressed = false;
	std::string cubes = TFN_RVID_CUBES;
	if (isDSiMode()) {
		rvidCompressed = true;
		cubes = TFN_LZ77_RVID_CUBES;
		if (ms().colorMode == 1) {
			cubes = TFN_LZ77_RVID_CUBES_BW;
		}
	} else {
		if (ms().colorMode == 1) {
			cubes = TFN_RVID_CUBES_BW;
		}
	}
	FILE *videoFrameFile = fopen(cubes.c_str(), "rb");

	if (!videoFrameFile && isDSiMode()) {
		// Fallback to uncompressed RVID
		rvidCompressed = false;
		cubes = TFN_RVID_CUBES;
		if (ms().colorMode == 1) {
			cubes = TFN_RVID_CUBES_BW;
		}
		videoFrameFile = fopen(cubes.c_str(), "rb");
	}

	// if (!videoFrameFile) {
	// 	videoFrameFile = fopen(std::string(TFN_FALLBACK_RVID_CUBES).c_str(), "rb");
	// }
	// FILE* videoFrameFile;

	/*for (u8 selectedFrame = 0; selectedFrame <= rocketVideo_videoFrames; selectedFrame++) {
		if (selectedFrame < 0x10) {
			snprintf(videoFrameFilename, sizeof(videoFrameFilename),
	"nitro:/video/3dsRotatingCubes/0x0%x.bmp", (int)selectedFrame); } else { snprintf(videoFrameFilename,
	sizeof(videoFrameFilename), "nitro:/video/3dsRotatingCubes/0x%x.bmp", (int)selectedFrame);
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

	if (videoFrameFile) {
		bool doRead = false;
		if (!rvidCompressed) {
			fseek(videoFrameFile, 0x200, SEEK_SET);
		}

		if (isDSiMode()) {
			doRead = true;
		} else if (sys().isRegularDS()) {
			sysSetCartOwner(BUS_OWNER_ARM9); // Allow arm9 to access GBA ROM (or in this case, the DS Memory
							 // Expansion Pak)
			*(vu32 *)(0x08240000) = 1;
			if (*(vu32 *)(0x08240000) == 1) {
				// Set to load video into DS Memory Expansion Pak
				rotatingCubesLocation = (u8 *)0x09000000;
				doRead = true;
			}
		}

		if (doRead) {
			if (rvidCompressed) {
				fread((void*)0x02480000, 1, 0x200000, videoFrameFile);
				LZ77_Decompress((u8*)0x02480000, (u8*)rotatingCubesLocation);
			} else {
				fread(rotatingCubesLocation, 1, 0x700000, videoFrameFile);
			}
			rotatingCubesLoaded = true;
			rocketVideo_playVideo = true;
		}
	}
}
void graphicsInit() {
	printf("graphicsInit()\n");

	// for (int i = 0; i < 12; i++) {
	// 	launchDotFrame[i] = 5;
	// }

	for (int i = 0; i < 5; i++) {
		dropTime[i] = 0;
		dropSeq[i] = 0;
		dropSpeed[i] = dropSpeedDefine;
		dropSpeedChange[i] = 0;
		if (ms().theme == 1 || ms().theme == 4 || ms().theme == 5)
			titleboxYposDropDown[i] = 0;
		else
			titleboxYposDropDown[i] = -85 - 80;
	}

	allowedTitleboxForDropDown = 0;
	delayForTitleboxToDropDown = 0;
	dropDown = false;


	titleboxXpos[0] = ms().cursorPosition[0] * 64;
	titlewindowXpos[0] = ms().cursorPosition[0] * 5;
	titleboxXpos[1] = ms().cursorPosition[1] * 64;
	titlewindowXpos[1] = ms().cursorPosition[1] * 5;

	SetBrightness(0, (ms().theme == 4 || ms().theme == 5) ? -31 : 31);
	SetBrightness(1, (ms().theme == 4 || ms().theme == 5) ? -31 : 31);

	// videoSetup() Called here before.
	// REG_BLDCNT = BLEND_SRC_BG3 | BLEND_FADE_BLACK;

	if (isDSiMode()) {
		loadSdRemovedImage();
	}

	// swiWaitForVBlank();
	titleboxYpos = tc().titleboxRenderY();
	bubbleYpos = tc().bubbleTipRenderY();
	bubbleXpos = tc().bubbleTipRenderX();

	if (ms().theme == 1) {
		//tex().load3DSTheme();
		rocketVideo_videoYpos = tc().rotatingCubesRenderY();
		loadRotatingCubes();
	}

	tex().drawTopBg();
	if (ms().theme != 4 && ms().theme != 5) {
		tex().drawProfileName();
	}

	drawCurrentDate();
	drawCurrentTime();
	drawClockColon();


	bottomBgLoad(false, true);
	// consoleDemoInit();

	// printf("drawn bgload");
	// while(1) {}
	if (tc().renderPhoto()) {
		srand(time(NULL));
		loadPhotoList();
	}

	if (ms().theme == 5) {
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
					   (u16 *)bubblesPal, // Load our 16 color tiles palette
					   (u8 *)bubblesBitmap   // image data generated by GRIT
		);
	}

	tex().drawVolumeImageCached();
	tex().drawBatteryImageCached();
	irqSet(IRQ_VBLANK, vBlankHandler);
	irqEnable(IRQ_VBLANK);
	irqSet(IRQ_VCOUNT, rvidFrameRateHandler);
	irqEnable(IRQ_VCOUNT);
	// consoleDemoInit();
}
