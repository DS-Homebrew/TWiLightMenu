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

#include "queueControl.h"
#include "uvcoord_top_font.h"
#include "uvcoord_date_time_font.h"

#include "../iconTitle.h"
#include "graphics.h"
#include "fontHandler.h"
#include "../ndsheaderbanner.h"
#include "../language.h"
#include "../perGameSettings.h"
#include "../flashcard.h"
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
int fadeDelay = 0;

extern bool isRegularDS;

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
int titleboxYposDropDown[5] = {-85-80};
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

int movetimer = 0;

int titleboxYmovepos = 0;

extern int spawnedtitleboxes;

extern bool useGbarunner;

extern int theme;
extern int subtheme;
std::vector<std::string> photoList;
static std::string photoPath;
extern int cursorPosition[2];
extern int pagenum[2];
//int titleboxXmovespeed[8] = {8};
int titleboxXmovespeed[8] = {12, 10, 8, 8, 8, 8, 6, 4};
int titleboxXpos[2] = {0};
int titleboxYpos = 85;	// 85, when dropped down
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

bool launchDotXMove[12] = {false};	// false = left, true = right
bool launchDotYMove[12] = {false};	// false = up, true = down

int launchDotFrame[12] = {0};
int launchDotCurrentChangingFrame = 0;
bool launchDotDoFrameChange = false;

bool showdialogbox = false;
bool dbox_showIcon = false;
bool dbox_selectMenu = false;
float dbox_movespeed = 22;
float dbox_Ypos = -192;
int bottomScreenBrightness = 255;

int bottomBg;

int bottomBgState = 0; // 0 = Uninitialized 1 = No Bubble 2 = bubble.

int vblankRefreshCounter = 0;

static u16 bmpImageBuffer[256*192];

int bubbleYpos = 80;
int bubbleXpos = 122;

void vramcpy_ui (void* dest, const void* src, int size) 
{
	u16* destination = (u16*)dest;
	u16* source = (u16*)src;
	while (size > 0) {
		*destination++ = *source++;
		size-=2;
	}
}

extern mm_sound_effect snd_stop;
extern mm_sound_effect mus_menu;

void ClearBrightness(void) {
	fadeType = true;
	screenBrightness = 0;
	swiWaitForVBlank();
	swiWaitForVBlank();
}

bool screenFadedIn(void) {
	return (screenBrightness == 0);
}

bool screenFadedOut(void) {
	return (screenBrightness > 24);
}

// Ported from PAlib (obsolete)
void SetBrightness(u8 screen, s8 bright) {
	u16 mode = 1 << 14;

	if (bright < 0) {
		mode = 2 << 14;
		bright = -bright;
	}
	if (bright > 31) bright = 31;
	*(u16*)(0x0400006C + (0x1000 * screen)) = bright + mode;
}

void moveIconClose(int num) {
	if (titleboxXmoveleft) {
		movecloseXpos = 0;
		if(movetimer == 1) {
			if (cursorPosition[secondaryDevice]-2 == num) movecloseXpos = 1;
			else if (cursorPosition[secondaryDevice]+2 == num) movecloseXpos = -2;
		} else if(movetimer == 2) {
			if (cursorPosition[secondaryDevice]-2 == num) movecloseXpos = 1;
			else if (cursorPosition[secondaryDevice]+2 == num) movecloseXpos = -2;
		} else if(movetimer == 3) {
			if (cursorPosition[secondaryDevice]-2 == num) movecloseXpos = 2;
			else if (cursorPosition[secondaryDevice]+2 == num) movecloseXpos = -3;
		} else if(movetimer == 4) {
			if (cursorPosition[secondaryDevice]-2 == num) movecloseXpos = 2;
			else if (cursorPosition[secondaryDevice]+2 == num) movecloseXpos = -3;
		} else if(movetimer == 5) {
			if (cursorPosition[secondaryDevice]-2 == num) movecloseXpos = 3;
			else if (cursorPosition[secondaryDevice]+2 == num) movecloseXpos = -4;
		} else if(movetimer == 6) {
			if (cursorPosition[secondaryDevice]-2 == num) movecloseXpos = 4;
			else if (cursorPosition[secondaryDevice]+2 == num) movecloseXpos = -5;
		} else if(movetimer == 7) {
			if (cursorPosition[secondaryDevice]-2 == num) movecloseXpos = 5;
			else if (cursorPosition[secondaryDevice]+2 == num) movecloseXpos = -6;
		} else if(movetimer == 8) {
			if (cursorPosition[secondaryDevice]-2 == num) movecloseXpos = 6;
			else if (cursorPosition[secondaryDevice]+2 == num) movecloseXpos = -7;
		}
	}
	if (titleboxXmoveright) {
		movecloseXpos = 0;
		if(movetimer == 1) {
			if (cursorPosition[secondaryDevice]-2 == num) movecloseXpos = 2;
			else if (cursorPosition[secondaryDevice]+2 == num) movecloseXpos = -1;
		} else if(movetimer == 2) {
			if (cursorPosition[secondaryDevice]-2 == num) movecloseXpos = 2;
			else if (cursorPosition[secondaryDevice]+2 == num) movecloseXpos = -1;
		} else if(movetimer == 3) {
			if (cursorPosition[secondaryDevice]-2 == num) movecloseXpos = 3;
			else if (cursorPosition[secondaryDevice]+2 == num) movecloseXpos = -2;
		} else if(movetimer == 4) {
			if (cursorPosition[secondaryDevice]-2 == num) movecloseXpos = 3;
			else if (cursorPosition[secondaryDevice]+2 == num) movecloseXpos = -2;
		} else if(movetimer == 5) {
			if (cursorPosition[secondaryDevice]-2 == num) movecloseXpos = 4;
			else if (cursorPosition[secondaryDevice]+2 == num) movecloseXpos = -3;
		} else if(movetimer == 6) {
			if (cursorPosition[secondaryDevice]-2 == num) movecloseXpos = 5;
			else if (cursorPosition[secondaryDevice]+2 == num) movecloseXpos = -4;
		} else if(movetimer == 7) {
			if (cursorPosition[secondaryDevice]-2 == num) movecloseXpos = 6;
			else if (cursorPosition[secondaryDevice]+2 == num) movecloseXpos = -5;
		} else if(movetimer == 8) {
			if (cursorPosition[secondaryDevice]-2 == num) movecloseXpos = 7;
			else if (cursorPosition[secondaryDevice]+2 == num) movecloseXpos = -6;
		}
	}
	if(!titleboxXmoveleft || !titleboxXmoveright) {
		if (cursorPosition[secondaryDevice]-2 == num) movecloseXpos = 6;
		else if (cursorPosition[secondaryDevice]+2 == num) movecloseXpos = -6;
		else movecloseXpos = 0;
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

void bottomBgLoad(bool drawBubble, bool init = false) {
	if (init || (!drawBubble && bottomBgState == 2)) {
		tex().drawBg(bottomBg);
		// Set that we've not drawn the bubble.
		bottomBgState = 1;
	} else if (drawBubble && bottomBgState == 1){
		tex().drawBubbleBg(bottomBg);
		// Set that we've drawn the bubble.
		bottomBgState = 2;
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
			glSprite(x * 16, dbox_Ypos+y * 16, GL_FLIP_NONE, &tex().dialogboxImage()[i & 255]);
		}
	}
}


void reloadDboxPalette() {
	tex().reloadPalDialogBox();
}

void vBlankHandler()
{
	execQueue(); // Execute any actions queued during last vblank.
	execDeferredIconUpdates(); // Update any icons queued during last vblank.
	if (theme == 1 && waitBeforeMusicPlay) {
		if (waitBeforeMusicPlayTime == 60) {
			mmEffectEx(&mus_menu);
			waitBeforeMusicPlay = false;
		} else {
			waitBeforeMusicPlayTime++;
		}
	} else {
		waitBeforeMusicPlay = false;
	}
	
	if (music && !waitBeforeMusicPlay) {
		musicTime++;
		if (musicTime == 60*49) {	// Length of music file in seconds (60*ss)
			mmEffectEx(&mus_menu);
			musicTime = 0;
		}
	}

	if (waitForNeedToPlayStopSound > 0) {
		waitForNeedToPlayStopSound++;
		if (waitForNeedToPlayStopSound == 5) {
			waitForNeedToPlayStopSound = 0;
		}
		needToPlayStopSound = false;
	}

	glBegin2D();
	{
		if(fadeType == true) {
			if(!fadeDelay) {
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
			if(!fadeDelay) {
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
		if (controlBottomBright) SetBrightness(0, screenBrightness);
		if (controlTopBright) SetBrightness(1, screenBrightness);

		if (showdialogbox) {
			// Dialogbox moving into view...
			if (dbox_movespeed <= 1) {
				if (dbox_Ypos >= 0) {
					// dbox stopped
					dbox_movespeed = 0;
					dbox_Ypos = 0;
					bottomScreenBrightness = 127;
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
			}
			dbox_Ypos += dbox_movespeed;
		} else {
			// Dialogbox moving down...
			if (dbox_Ypos <= -192 || dbox_Ypos >= 192) {
				dbox_movespeed = 22;
				dbox_Ypos = -192;
				bottomScreenBrightness = 255;
			} else {
				dbox_movespeed += 1;
				dbox_Ypos += dbox_movespeed;
				bottomScreenBrightness += 7;
				if (bottomScreenBrightness > 255) {
					bottomScreenBrightness = 255;
				}
			}
		}

		if (titleboxXmoveleft) {
			if (movetimer == 8) {
			//	if (showbubble && theme == 0) mmEffectEx(&snd_stop);
				needToPlayStopSound = true;
				startBorderZoomOut = true;
				titlewindowXpos[secondaryDevice] -= 1;
				movetimer++;
			} else if (movetimer < 8) {
				titleboxXpos[secondaryDevice] -= titleboxXmovespeed[movetimer];
				if(movetimer==0 || movetimer==2 || movetimer==4 || movetimer==6 ) titlewindowXpos[secondaryDevice] -= 1;
				movetimer++;
			} else {
				titleboxXmoveleft = false;
				movetimer = 0;
			}
		} else if (titleboxXmoveright) {
			if (movetimer == 8) {
			//	if (showbubble && theme == 0) mmEffectEx(&snd_stop);
				needToPlayStopSound = true;
				startBorderZoomOut = true;
				titlewindowXpos[secondaryDevice] += 1;
				movetimer++;
			} else if (movetimer < 8) {
				titleboxXpos[secondaryDevice] += titleboxXmovespeed[movetimer];
				if(movetimer==0 || movetimer==2 || movetimer==4 || movetimer==6 ) titlewindowXpos[secondaryDevice] += 1;
				movetimer++;
			} else {
				titleboxXmoveright = false;
				movetimer = 0;
			}
		}

		if (redoDropDown && theme == 0) {
			for (int i = 0; i < 5; i++) {
				dropTime[i] = 0;
				dropSeq[i] = 0;
				dropSpeed[i] = 5;
				dropSpeedChange[i] = 0;
				titleboxYposDropDown[i] = -85-80;
			}
			allowedTitleboxForDropDown = 0;
			delayForTitleboxToDropDown = 0;
			dropDown = false;
			redoDropDown = false;
		}

		if (!whiteScreen && dropDown && theme == 0) {
			for (int i = 0; i <= allowedTitleboxForDropDown; i++) {
				if (dropSeq[i] == 0) {
					titleboxYposDropDown[i] += dropSpeed[i];
					if (titleboxYposDropDown[i] > 0) dropSeq[i] = 1;
				} else if (dropSeq[i] == 1) {
					titleboxYposDropDown[i] -= dropSpeed[i];
					dropTime[i]++;
					dropSpeedChange[i]++;
					if (dropTime[i] >= 15) {
						dropSpeedChange[i] = -1;
						dropSeq[i] = 2;
					}
					if (dropSpeedChange[i] == 2) {
						dropSpeed[i]--;
						if (dropSpeed[i] < 0) dropSpeed[i] = 0;
						dropSpeedChange[i] = -1;
					}
				} else if (dropSeq[i] == 2) {
					titleboxYposDropDown[i] += dropSpeed[i];
					if (titleboxYposDropDown[i] >= 0) {
						dropSeq[i] = 3;
						titleboxYposDropDown[i] = 0;
					}
					dropSpeedChange[i]++;
					if (dropSpeedChange[i] == 1) {
						dropSpeed[i]++;
						if (dropSpeed[i] > 4) dropSpeed[i] = 4;
						dropSpeedChange[i] = -1;
					}
				} else if (dropSeq[i] == 3) {
					titleboxYposDropDown[i] = 0;
				}
			}

			delayForTitleboxToDropDown++;
			if (delayForTitleboxToDropDown >= 5) {
				allowedTitleboxForDropDown++;
				if (allowedTitleboxForDropDown > 4) allowedTitleboxForDropDown = 4;
				delayForTitleboxToDropDown = 0;
			}
		}

		//if (renderingTop)
		//{
			/*glBoxFilledGradient(0, -64, 256, 112,
						  RGB15(colorRvalue,colorGvalue,colorBvalue), RGB15(0,0,0), RGB15(0,0,0), RGB15(colorRvalue,colorGvalue,colorBvalue)
						);
			glBoxFilledGradient(0, 112, 256, 192,
						  RGB15(0,0,0), RGB15(colorRvalue,colorGvalue,colorBvalue), RGB15(colorRvalue,colorGvalue,colorBvalue), RGB15(0,0,0)
						);
			drawBG(mainBgImage);
			glSprite(-2, 172, GL_FLIP_NONE, &shoulderImage[0 & 31]);
			glSprite(178, 172, GL_FLIP_NONE, &shoulderImage[1 & 31]);
			if (whiteScreen) glBoxFilled(0, 0, 256, 192, RGB15(31, 31, 31));
			updateText(renderingTop);
			glColor(RGB15(31, 31, 31));*/
		//}
		//else
		//{


		//	drawBG(subBgImage);
		//	if (!showbubble && theme==0) glSprite(0, 29, GL_FLIP_NONE, ndsimenutextImage);

		glColor(RGB15(bottomScreenBrightness/8, bottomScreenBrightness/8, bottomScreenBrightness/8));
			if (theme==0) {
				int bipXpos = 27;
				glSprite(16+titlewindowXpos[secondaryDevice], 171, GL_FLIP_NONE, tex().scrollwindowImage());
				for(int i = 0; i < 40; i++) {
					if (i < spawnedtitleboxes) glSprite(bipXpos, 178, GL_FLIP_NONE, tex().bipsImage());
					else glSprite(bipXpos, 178, GL_FLIP_NONE, &tex().bipsImage()[1 & 31]);
					bipXpos += 5;
				}
				glSprite(16+titlewindowXpos[secondaryDevice], 171, GL_FLIP_NONE, &tex().buttonarrowImage()[2+scrollWindowTouched]);
				glSprite(0, 171, GL_FLIP_NONE, &tex().buttonarrowImage()[0+buttonArrowTouched[0]]);
				glSprite(224, 171, GL_FLIP_H, &tex().buttonarrowImage()[0+buttonArrowTouched[1]]);
				glSprite(72-titleboxXpos[secondaryDevice], 81, GL_FLIP_NONE, tex().braceImage());
			}
			int spawnedboxXpos = 96;
			int iconXpos = 112;

			if(movingApp!=-1) {
				if(movingAppIsDir) {
					if (theme == 1) glSprite(96, titleboxYpos-movingAppYpos, GL_FLIP_NONE, tex().folderImage());
					else glSprite(96, titleboxYpos-movingAppYpos+titleboxYposDropDown[movingApp % 5], GL_FLIP_NONE, tex().folderImage());
				} else {
					if (theme == 1) {
						glSprite(96, titleboxYpos-movingAppYpos, GL_FLIP_NONE, tex().boxfullImage());
					} else { 
						glSprite(96, titleboxYpos-movingAppYpos+titleboxYposDropDown[movingApp % 5], GL_FLIP_NONE, &tex().boxfullImage()[0 & 63]);
					}
					if (bnrRomType[movingApp] == 7) drawIconSNES(112, (titleboxYpos+12)-movingAppYpos+titleboxYposDropDown[movingApp % 5]);
					else if (bnrRomType[movingApp] == 6) drawIconMD(112, (titleboxYpos+12)-movingAppYpos+titleboxYposDropDown[movingApp % 5]);
					else if (bnrRomType[movingApp] == 5) drawIconGG(112, (titleboxYpos+12)-movingAppYpos+titleboxYposDropDown[movingApp % 5]);
					else if (bnrRomType[movingApp] == 4) drawIconSMS(112, (titleboxYpos+12)-movingAppYpos+titleboxYposDropDown[movingApp % 5]);
					else if (bnrRomType[movingApp] == 3) drawIconNES(112, (titleboxYpos+12)-movingAppYpos+titleboxYposDropDown[movingApp % 5]);
					else if (bnrRomType[movingApp] == 2) drawIconGBC(112, (titleboxYpos+12)-movingAppYpos+titleboxYposDropDown[movingApp % 5]);
					else if (bnrRomType[movingApp] == 1) drawIconGB(112, (titleboxYpos+12)-movingAppYpos+titleboxYposDropDown[movingApp % 5]);
					else drawIcon(112, (titleboxYpos+12)-movingAppYpos+titleboxYposDropDown[movingApp % 5], -1);
				}
			}

			for(int i = 0; i < 40; i++) {
				if (theme == 0) {
					moveIconClose(i);
				} else {
					movecloseXpos = 0;
				}
				if (i < spawnedtitleboxes) {
					if (isDirectory[i]) {
						if(movingApp!=-1) {
							int j = i;
								if(i>movingApp-(pagenum[secondaryDevice]*40))	j--;
							if (theme == 1) glSprite((j*2496/39)+128-titleboxXpos[secondaryDevice], titleboxYpos, GL_FLIP_NONE, tex().folderImage());
							else glSprite((j*2496/39)+128-titleboxXpos[secondaryDevice], (titleboxYpos-3)+titleboxYposDropDown[i % 5], GL_FLIP_NONE, tex().folderImage());
						} else {
							if (theme == 1) glSprite(spawnedboxXpos-titleboxXpos[secondaryDevice]+movecloseXpos, titleboxYpos, GL_FLIP_NONE, tex().folderImage());
							else glSprite(spawnedboxXpos-titleboxXpos[secondaryDevice]+movecloseXpos, (titleboxYpos-3)+titleboxYposDropDown[i % 5], GL_FLIP_NONE, tex().folderImage());
						}
					} else if (!applaunchprep || cursorPosition[secondaryDevice] != i){ // Only draw the icon if we're not launching the selcted app
						if(movingApp!=-1) {
							int j = i;
							if(i>movingApp-(pagenum[secondaryDevice]*40))	j--;
							if(j==-1)	continue;
							if (theme == 1) {
								glSprite((j*2496/39)+128-titleboxXpos[secondaryDevice], titleboxYpos, GL_FLIP_NONE, tex().boxfullImage());
							} else { 
								glSprite((j*2496/39)+128-titleboxXpos[secondaryDevice], titleboxYpos+titleboxYposDropDown[i % 5], GL_FLIP_NONE, &tex().boxfullImage()[0 & 63]);
							}
							if (bnrRomType[i] == 7) drawIconSNES((j*2496/39)+144-titleboxXpos[secondaryDevice], (titleboxYpos+12)+titleboxYposDropDown[i % 5]);
							else if (bnrRomType[i] == 6) drawIconMD((j*2496/39)+144-titleboxXpos[secondaryDevice], (titleboxYpos+12)+titleboxYposDropDown[i % 5]);
							else if (bnrRomType[i] == 5) drawIconGG((j*2496/39)+144-titleboxXpos[secondaryDevice], (titleboxYpos+12)+titleboxYposDropDown[i % 5]);
							else if (bnrRomType[i] == 4) drawIconSMS((j*2496/39)+144-titleboxXpos[secondaryDevice], (titleboxYpos+12)+titleboxYposDropDown[i % 5]);
							else if (bnrRomType[i] == 3) drawIconNES((j*2496/39)+144-titleboxXpos[secondaryDevice], (titleboxYpos+12)+titleboxYposDropDown[i % 5]);
							else if (bnrRomType[i] == 2) drawIconGBC((j*2496/39)+144-titleboxXpos[secondaryDevice], (titleboxYpos+12)+titleboxYposDropDown[i % 5]);
							else if (bnrRomType[i] == 1) drawIconGB((j*2496/39)+144-titleboxXpos[secondaryDevice], (titleboxYpos+12)+titleboxYposDropDown[i % 5]);
							else drawIcon((j*2496/39)+144-titleboxXpos[secondaryDevice], (titleboxYpos+12)+titleboxYposDropDown[i % 5], i);
						} else {
							if (theme == 1) {
								glSprite(spawnedboxXpos-titleboxXpos[secondaryDevice], titleboxYpos, GL_FLIP_NONE, tex().boxfullImage());
							} else { 
								glSprite(spawnedboxXpos-titleboxXpos[secondaryDevice]+movecloseXpos, titleboxYpos+titleboxYposDropDown[i % 5], GL_FLIP_NONE, &tex().boxfullImage()[0 & 63]);
							}
							if (bnrRomType[i] == 7) drawIconSNES(iconXpos-titleboxXpos[secondaryDevice]+movecloseXpos, (titleboxYpos+12)+titleboxYposDropDown[i % 5]);
							else if (bnrRomType[i] == 6) drawIconMD(iconXpos-titleboxXpos[secondaryDevice]+movecloseXpos, (titleboxYpos+12)+titleboxYposDropDown[i % 5]);
							else if (bnrRomType[i] == 5) drawIconGG(iconXpos-titleboxXpos[secondaryDevice]+movecloseXpos, (titleboxYpos+12)+titleboxYposDropDown[i % 5]);
							else if (bnrRomType[i] == 4) drawIconSMS(iconXpos-titleboxXpos[secondaryDevice]+movecloseXpos, (titleboxYpos+12)+titleboxYposDropDown[i % 5]);
							else if (bnrRomType[i] == 3) drawIconNES(iconXpos-titleboxXpos[secondaryDevice]+movecloseXpos, (titleboxYpos+12)+titleboxYposDropDown[i % 5]);
							else if (bnrRomType[i] == 2) drawIconGBC(iconXpos-titleboxXpos[secondaryDevice]+movecloseXpos, (titleboxYpos+12)+titleboxYposDropDown[i % 5]);
							else if (bnrRomType[i] == 1) drawIconGB(iconXpos-titleboxXpos[secondaryDevice]+movecloseXpos, (titleboxYpos+12)+titleboxYposDropDown[i % 5]);
							else drawIcon(iconXpos-titleboxXpos[secondaryDevice]+movecloseXpos, (titleboxYpos+12)+titleboxYposDropDown[i % 5], i);
						}
					}
				} else {
					// Empty box
					if(movingApp!=-1) {
						if (theme == 1) {
							glSprite(((i-1)*2496/39)+128-titleboxXpos[secondaryDevice], titleboxYpos+titleboxYposDropDown[i % 5], GL_FLIP_NONE, tex().boxemptyImage());
						} else {
							glSprite(((i-1)*2496/39)+128-titleboxXpos[secondaryDevice], titleboxYpos+titleboxYposDropDown[i % 5], GL_FLIP_NONE, &tex().boxfullImage()[1 & 63]);
						}
					} else {
						if (theme == 1) {
							glSprite(spawnedboxXpos-titleboxXpos[secondaryDevice], titleboxYpos+titleboxYposDropDown[i % 5], GL_FLIP_NONE, tex().boxemptyImage());
						} else {
							glSprite(spawnedboxXpos-titleboxXpos[secondaryDevice]+movecloseXpos, titleboxYpos+titleboxYposDropDown[i % 5], GL_FLIP_NONE, &tex().boxfullImage()[1 & 63]);
						}
					}
				}
				spawnedboxXpos += 64;
				iconXpos += 64;
			}
			if (theme == 0) {
				glSprite(spawnedboxXpos+10-titleboxXpos[secondaryDevice], 81, GL_FLIP_H, tex().braceImage());
			}

			if(movingApp!=-1 && !theme && showMovingArrow) {
				if(movingArrowYdirection) {
					movingArrowYpos += 0.33;
					if(movingArrowYpos>67)
						movingArrowYdirection = false;
				} else {
					movingArrowYpos -= 0.33;
					if(movingArrowYpos<59)
						movingArrowYdirection = true;
				}
				glSprite(115, movingArrowYpos, GL_FLIP_NONE, tex().movingArrowImage());	
			}
			// Top icons for 3DS theme
			if (theme==1) {
				int topIconXpos = 116;
				if (isDSiMode() && sdFound()) {
					//for (int i = 0; i < 4; i++) {
						topIconXpos -= 14;
					//}
					if (secondaryDevice) {
						glSprite(topIconXpos, 1, GL_FLIP_NONE, &tex().smallCartImage()[2]);	// SD card
					} else {
						glSprite(topIconXpos, 1, GL_FLIP_NONE, &tex().smallCartImage()[(REG_SCFG_MC == 0x11) ? 1 : 0]);	// Slot-1 card
					}
					topIconXpos += 28;
					drawSmallIconGBA(topIconXpos, 1);	// GBARunner2
				} else {
					//for (int i = 0; i < 3; i++) {
					//	topIconXpos -= 14;
					//}
					if (useGbarunner) {
						drawSmallIconGBA(topIconXpos, 1);	// GBARunner2
					} else {
						glSprite(topIconXpos, 1, GL_FLIP_NONE, &tex().smallCartImage()[3]);	// GBA Mode
					}
				}
				glSprite(0, 0, GL_FLIP_NONE, &tex().cornerButtonImage()[0]);
				if (!isRegularDS) glSprite(256-44, 0, GL_FLIP_NONE, &tex().cornerButtonImage()[1]);
			}

			if (applaunchprep && theme==0) {
				
				if (isDirectory[cursorPosition[secondaryDevice]]) {
					glSprite(96, 87-titleboxYmovepos, GL_FLIP_NONE, tex().folderImage());
				} else {
					glSprite(96, 84-titleboxYmovepos, GL_FLIP_NONE, tex().boxfullImage());
					if (bnrRomType[cursorPosition[secondaryDevice]] == 7) drawIconSNES(112, 96-titleboxYmovepos);
					else if (bnrRomType[cursorPosition[secondaryDevice]] == 6) drawIconMD(112, 96-titleboxYmovepos);
					else if (bnrRomType[cursorPosition[secondaryDevice]] == 5) drawIconGG(112, 96-titleboxYmovepos);
					else if (bnrRomType[cursorPosition[secondaryDevice]] == 4) drawIconSMS(112, 96-titleboxYmovepos);
					else if (bnrRomType[cursorPosition[secondaryDevice]] == 3) drawIconNES(112, 96-titleboxYmovepos);
					else if (bnrRomType[cursorPosition[secondaryDevice]] == 2) drawIconGBC(112, 96-titleboxYmovepos);
					else if (bnrRomType[cursorPosition[secondaryDevice]] == 1) drawIconGB(112, 96-titleboxYmovepos);
					else drawIcon(112, 96-titleboxYmovepos, cursorPosition[secondaryDevice]);
				}
				// Draw dots after selecting a game/app
				for (int i = 0; i < 11; i++) {
					glSprite(76+launchDotX[i], 69+launchDotY[i], GL_FLIP_NONE, &tex().launchdotImage()[(launchDotFrame[i]) & 15]);
					if (launchDotX[i] == 0) launchDotXMove[i] = true;
					if (launchDotX[i] == 88) launchDotXMove[i] = false;
					if (launchDotY[i] == 0) launchDotYMove[i] = true;
					if (launchDotY[i] == 88) launchDotYMove[i] = false;
					if (launchDotXMove[i] == false) {
						launchDotX[i]--;
					} else if (launchDotXMove[i] == true) {
						launchDotX[i]++;
					}
					if (launchDotYMove[i] == false) {
						launchDotY[i]--;
					} else if (launchDotYMove[i] == true) {
						launchDotY[i]++;
					}
				}
				titleboxYmovepos += 5;
			}
			if (showSTARTborder) {
				if (theme == 1) {
					glSprite(96, 92, GL_FLIP_NONE, &tex().startbrdImage()[startBorderZoomAnimSeq[startBorderZoomAnimNum] & 63]);
					glSprite(96+32, 92, GL_FLIP_H, &tex().startbrdImage()[startBorderZoomAnimSeq[startBorderZoomAnimNum] & 63]);
					if (bnrWirelessIcon[cursorPosition[secondaryDevice]] > 0) glSprite(96, 92, GL_FLIP_NONE, &tex().wirelessIcons()[(bnrWirelessIcon[cursorPosition[secondaryDevice]]-1) & 31]);
				} else if (!isScrolling) {
					if (showbubble && theme == 0 && needToPlayStopSound && waitForNeedToPlayStopSound == 0) {
						mmEffectEx(&snd_stop);
						waitForNeedToPlayStopSound = 1;
						needToPlayStopSound = false;
					}
					glSprite(96, 81, GL_FLIP_NONE, &tex().startbrdImage()[startBorderZoomAnimSeq[startBorderZoomAnimNum] & 79]);
					glSprite(96+32, 81, GL_FLIP_H, &tex().startbrdImage()[startBorderZoomAnimSeq[startBorderZoomAnimNum] & 79]);
					if (bnrWirelessIcon[cursorPosition[secondaryDevice]] > 0) glSprite(96, 81, GL_FLIP_NONE, &tex().wirelessIcons()[(bnrWirelessIcon[cursorPosition[secondaryDevice]]-1) & 31]);
				}
			}

			// Refresh the background layer.
			bottomBgLoad(showbubble);
			if (showbubble) drawBubble(tex().bubbleImage());
			if (showSTARTborder && theme == 0 && !isScrolling) glSprite(96, 144, GL_FLIP_NONE, &tex().startImage()[setLanguage]);

			glColor(RGB15(31, 31, 31));
			if (dbox_Ypos != -192) {
				// Draw the dialog box.
				drawDbox();
				if (dbox_showIcon && !isDirectory[cursorPosition[secondaryDevice]]) {
					drawIcon(24, dbox_Ypos+24, cursorPosition[secondaryDevice]);
				}
				if (dbox_selectMenu) {
					int selIconYpos = 96;
					if (isDSiMode() && sdFound()) {
						for (int i = 0; i < 4; i++) {
							selIconYpos -= 14;
						}
					} else {
						for (int i = 0; i < 3; i++) {
							selIconYpos -= 14;
						}
					}
					if (!isRegularDS) {
						glSprite(32, dbox_Ypos+selIconYpos, GL_FLIP_NONE, &tex().cornerButtonImage()[1]);	// System Menu
						selIconYpos += 28;
					}
					glSprite(32, dbox_Ypos+selIconYpos, GL_FLIP_NONE, &tex().cornerButtonImage()[0]);	// Settings
					selIconYpos += 28;
					if (isDSiMode() && sdFound()) {
						if (secondaryDevice) {
							glSprite(32, dbox_Ypos+selIconYpos, GL_FLIP_NONE, &tex().smallCartImage()[2]);	// SD card
						} else {
							glSprite(32, dbox_Ypos+selIconYpos, GL_FLIP_NONE, &tex().smallCartImage()[(REG_SCFG_MC == 0x11) ? 1 : 0]);	// Slot-1 card
						}
						selIconYpos += 28;
					}
					if (useGbarunner) {
						drawSmallIconGBA(32, dbox_Ypos+selIconYpos);	// GBARunner2
					} else {
						glSprite(32, dbox_Ypos+selIconYpos, GL_FLIP_NONE, &tex().smallCartImage()[3]);	// GBA Mode
					}
				}
			}
			// Show button_arrowPals (debug feature)
			/*for (int i = 0; i < 16; i++) {
				for (int i2 = 0; i2 < 16; i2++) {
					glBox(i2,i,i2,i,button_arrowPals[(i*16)+i2]);
				}
			}*/
			if (whiteScreen) {
				glBoxFilled(0, 0, 256, 192, RGB15(31, 31, 31));
				if (showProgressIcon) glSprite(224, 152, GL_FLIP_NONE, &tex().progressImage()[progressAnimNum]);
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

	if (showProgressIcon) {
		progressAnimDelay++;
		if (progressAnimDelay == 3) {
			progressAnimNum++;
			if (progressAnimNum > 7) progressAnimNum = 0;
			progressAnimDelay = 0;
		}
	}
	if (!whiteScreen) {
		// Playback animated icons
		for (int i = 0; i < 41; i++) {
			if(bnriconisDSi[i]==true) {
				playBannerSequence(i);
			}
		}
	}
	if (theme == 1) {
		startBorderZoomAnimDelay++;
		if (startBorderZoomAnimDelay == 8) {
			startBorderZoomAnimNum++;
			if(startBorderZoomAnimSeq[startBorderZoomAnimNum] == 0) {
				startBorderZoomAnimNum = 0;
			}
			startBorderZoomAnimDelay = 0;
		}
	} else if (startBorderZoomOut) {
		startBorderZoomAnimNum++;
		if(startBorderZoomAnimSeq[startBorderZoomAnimNum] == 0) {
			startBorderZoomAnimNum = 0;
			startBorderZoomOut = false;
		}
	} else {
		startBorderZoomAnimNum = 0;
	}
	if (applaunchprep && theme==0 && launchDotDoFrameChange) {
		launchDotFrame[0]--;
		if (launchDotCurrentChangingFrame >= 1) launchDotFrame[1]--;
		if (launchDotCurrentChangingFrame >= 2) launchDotFrame[2]--;
		if (launchDotCurrentChangingFrame >= 3) launchDotFrame[3]--;
		if (launchDotCurrentChangingFrame >= 4) launchDotFrame[4]--;
		if (launchDotCurrentChangingFrame >= 5) launchDotFrame[5]--;
		if (launchDotCurrentChangingFrame >= 6) launchDotFrame[6]--;
		if (launchDotCurrentChangingFrame >= 7) launchDotFrame[7]--;
		if (launchDotCurrentChangingFrame >= 8) launchDotFrame[8]--;
		if (launchDotCurrentChangingFrame >= 9) launchDotFrame[9]--;
		if (launchDotCurrentChangingFrame >= 10) launchDotFrame[10]--;
		if (launchDotCurrentChangingFrame >= 11) launchDotFrame[11]--;
		for (int i = 0; i < 12; i++) {
			if (launchDotFrame[i] < 0) launchDotFrame[i] = 0;
		}
		launchDotCurrentChangingFrame++;
		if (launchDotCurrentChangingFrame > 11) launchDotCurrentChangingFrame = 11;
	}
	if (applaunchprep && theme==0) launchDotDoFrameChange = !launchDotDoFrameChange;
}

void clearBmpScreen() {
	u16 val = 0xFFFF;
	for (int i = 0; i < 256*192; i++) {
		BG_GFX_SUB[i] = ((val>>10)&0x1f) | ((val)&(0x1f<<5)) | (val&0x1f)<<10 | BIT(15);
	}
}

void loadBoxArt(const char* filename) {
	FILE* file = fopen(filename, "rb");
	if (!file) file = fopen("nitro:/graphics/boxart_unknown.bmp", "rb");

	if (file) {
		// Start loading
		fseek(file, 0xe, SEEK_SET);
		u8 pixelStart = (u8)fgetc(file) + 0xe;
		fseek(file, pixelStart, SEEK_SET);
		fread(bmpImageBuffer, 2, 0x7800, file);
		u16* src = bmpImageBuffer;
		int x = 64;
		int y = 40+114;
		for (int i=0; i<128*115; i++) {
			if (x >= 64+128) {
				x = 64;
				y--;
			}
			u16 val = *(src++);
			BG_GFX_SUB[y*256+x] = ((val>>10)&0x1f) | ((val)&(0x1f<<5)) | (val&0x1f)<<10 | BIT(15);
			x++;
		}
	}

	fclose(file);
}

static int loadedVolumeImage = -1;

static const char *volume4ImagePath;
static const char *volume3ImagePath;
static const char *volume2ImagePath;
static const char *volume1ImagePath;
static const char *volume0ImagePath;

void setVolumeImagePaths(void) {
	switch (subtheme) {
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

void loadVolumeImage(void) {
	if (!isDSiMode())
		return;

	u8 volumeLevel = *(u8*)(0x027FF000);
	const char *volumeImagePath;

	if (volumeLevel >= 0x1C && volumeLevel < 0x20) {
		if (loadedVolumeImage == 4) return;
		volumeImagePath = volume4ImagePath;
		loadedVolumeImage = 4;
	} else if (volumeLevel >= 0x11 && volumeLevel < 0x1C) {
		if (loadedVolumeImage == 3) return;
		volumeImagePath = volume3ImagePath;
		loadedVolumeImage = 3;
	} else if (volumeLevel >= 0x07 && volumeLevel < 0x11) {
		if (loadedVolumeImage == 2) return;
		volumeImagePath = volume2ImagePath;
		loadedVolumeImage = 2;
	} else if (volumeLevel > 0x00 && volumeLevel < 0x07) {
		if (loadedVolumeImage == 1) return;
		volumeImagePath = volume1ImagePath;
		loadedVolumeImage = 1;
	} else if (volumeLevel == 0x00) {
		if (loadedVolumeImage == 0) return;
		volumeImagePath = volume0ImagePath;
		loadedVolumeImage = 0;
	} else {
		return;
	}

	FILE* file = fopen(volumeImagePath, "rb");

	if (file) {
		// Start loading
		fseek(file, 0xe, SEEK_SET);
		u8 pixelStart = (u8)fgetc(file) + 0xe;
		fseek(file, pixelStart, SEEK_SET);
		fread(bmpImageBuffer, 2, 0x200, file);
		u16* src = bmpImageBuffer;
		int x = 4;
		int y = 5+11;
		for (int i=0; i<18*12; i++) {
			if (x >= 4+18) {
				x = 4;
				y--;
			}
			u16 val = *(src++);
			if (val != 0x7C1F) {	// Do not render magneta pixel
				BG_GFX_SUB[y*256+x] = ((val>>10)&0x1f) | ((val)&(0x1f<<5)) | (val&0x1f)<<10 | BIT(15);
			}
			x++;
		}
	}

	fclose(file);
}

static int loadedBatteryImage = -1;

static const char *batteryChargeImagePath;
static const char *battery4ImagePath;
static const char *battery3ImagePath;
static const char *battery2ImagePath;
static const char *battery1ImagePath;
static const char *batteryFullImagePath;
static const char *batteryLowImagePath;

void setBatteryImagePaths(void) {
	switch (subtheme) {
		case 0:	
			batteryChargeImagePath = (theme == 1) ? "nitro:/graphics/batterycharge.bmp" : "nitro:/graphics/dark_batterycharge.bmp";
			battery4ImagePath = (theme == 1) ? "nitro:/graphics/battery4.bmp" : "nitro:/graphics/dark_battery4.bmp";
			battery3ImagePath = (theme == 1) ? "nitro:/graphics/battery3.bmp" : "nitro:/graphics/dark_battery3.bmp";
			battery2ImagePath = (theme == 1) ? "nitro:/graphics/battery2.bmp" : "nitro:/graphics/dark_battery2.bmp";
			battery1ImagePath = (theme == 1) ? "nitro:/graphics/battery1.bmp" : "nitro:/graphics/dark_battery1.bmp";
			if (isRegularDS) {
				batteryFullImagePath = (theme == 1) ? "nitro:/graphics/batteryfullDS.bmp" : "nitro:/graphics/dark_batteryfullDS.bmp";
			} else {
				batteryFullImagePath = (theme == 1) ? "nitro:/graphics/batteryfull.bmp" : "nitro:/graphics/dark_batteryfull.bmp";
			}
			batteryLowImagePath = (theme == 1) ? "nitro:/graphics/batterylow.bmp" : "nitro:/graphics/dark_batterylow.bmp";
			break;
		case 1:
		default:
			batteryChargeImagePath = "nitro:/graphics/batterycharge.bmp";
			battery4ImagePath = "nitro:/graphics/battery4.bmp";
			battery3ImagePath = "nitro:/graphics/battery3.bmp";
			battery2ImagePath = "nitro:/graphics/battery2.bmp";
			battery1ImagePath = "nitro:/graphics/battery1.bmp";
			batteryFullImagePath = isRegularDS ? "nitro:/graphics/batteryfullDS.bmp" : "nitro:/graphics/batteryfull.bmp";
			batteryLowImagePath = "nitro:/graphics/batterylow.bmp";
			break;
		case 2:
			batteryChargeImagePath = "nitro:/graphics/red_batterycharge.bmp";
			battery4ImagePath = "nitro:/graphics/red_battery4.bmp";
			battery3ImagePath = "nitro:/graphics/red_battery3.bmp";
			battery2ImagePath = "nitro:/graphics/red_battery2.bmp";
			battery1ImagePath = "nitro:/graphics/red_battery1.bmp";
			batteryFullImagePath = isRegularDS ? "nitro:/graphics/red_batteryfullDS.bmp" : "nitro:/graphics/red_batteryfull.bmp";
			batteryLowImagePath = "nitro:/graphics/red_batterylow.bmp";
			break;
		case 3:
			batteryChargeImagePath = "nitro:/graphics/blue_batterycharge.bmp";
			battery4ImagePath = "nitro:/graphics/blue_battery4.bmp";
			battery3ImagePath = "nitro:/graphics/blue_battery3.bmp";
			battery2ImagePath = "nitro:/graphics/blue_battery2.bmp";
			battery1ImagePath = "nitro:/graphics/blue_battery1.bmp";
			batteryFullImagePath = isRegularDS ? "nitro:/graphics/blue_batteryfullDS.bmp" : "nitro:/graphics/blue_batteryfull.bmp";
			batteryLowImagePath = "nitro:/graphics/blue_batterylow.bmp";
			break;
		case 4:
			batteryChargeImagePath = "nitro:/graphics/green_batterycharge.bmp";
			battery4ImagePath = "nitro:/graphics/green_battery4.bmp";
			battery3ImagePath = "nitro:/graphics/green_battery3.bmp";
			battery2ImagePath = "nitro:/graphics/green_battery2.bmp";
			battery1ImagePath = "nitro:/graphics/green_battery1.bmp";
			batteryFullImagePath = isRegularDS ? "nitro:/graphics/green_batteryfullDS.bmp" : "nitro:/graphics/green_batteryfull.bmp";
			batteryLowImagePath = "nitro:/graphics/green_batterylow.bmp";
			break;
		case 5:
			batteryChargeImagePath = "nitro:/graphics/yellow_batterycharge.bmp";
			battery4ImagePath = "nitro:/graphics/yellow_battery4.bmp";
			battery3ImagePath = "nitro:/graphics/yellow_battery3.bmp";
			battery2ImagePath = "nitro:/graphics/yellow_battery2.bmp";
			battery1ImagePath = "nitro:/graphics/yellow_battery1.bmp";
			batteryFullImagePath = isRegularDS ? "nitro:/graphics/yellow_batteryfullDS.bmp" : "nitro:/graphics/yellow_batteryfull.bmp";
			batteryLowImagePath = "nitro:/graphics/yellow_batterylow.bmp";
			break;
		case 6:
			batteryChargeImagePath = "nitro:/graphics/pink_batterycharge.bmp";
			battery4ImagePath = "nitro:/graphics/pink_battery4.bmp";
			battery3ImagePath = "nitro:/graphics/pink_battery3.bmp";
			battery2ImagePath = "nitro:/graphics/pink_battery2.bmp";
			battery1ImagePath = "nitro:/graphics/pink_battery1.bmp";
			batteryFullImagePath = isRegularDS ? "nitro:/graphics/pink_batteryfullDS.bmp" : "nitro:/graphics/pink_batteryfull.bmp";
			batteryLowImagePath = "nitro:/graphics/pink_batterylow.bmp";
			break;
		case 7:
			batteryChargeImagePath = "nitro:/graphics/purple_batterycharge.bmp";
			battery4ImagePath = "nitro:/graphics/purple_battery4.bmp";
			battery3ImagePath = "nitro:/graphics/purple_battery3.bmp";
			battery2ImagePath = "nitro:/graphics/purple_battery2.bmp";
			battery1ImagePath = "nitro:/graphics/purple_battery1.bmp";
			batteryFullImagePath = isRegularDS ? "nitro:/graphics/purple_batteryfullDS.bmp" : "nitro:/graphics/purple_batteryfull.bmp";
			batteryLowImagePath = "nitro:/graphics/purple_batterylow.bmp";
			break;
	}
}

void loadBatteryImage(void) {
	u8 batteryLevel = *(u8*)(0x027FF001);
	const char *batteryImagePath;

	if (isDSiMode()) {
		if (batteryLevel & BIT(7)) {
			if (loadedBatteryImage == 7) return;
			batteryImagePath = batteryChargeImagePath;
			loadedBatteryImage = 7;
		} else if (batteryLevel == 0xF) {
			if (loadedBatteryImage == 4) return;
			batteryImagePath = battery4ImagePath;
			loadedBatteryImage = 4;
		} else if (batteryLevel == 0xB) {
			if (loadedBatteryImage == 3) return;
			batteryImagePath = battery3ImagePath;
			loadedBatteryImage = 3;
		} else if (batteryLevel == 0x7) {
			if (loadedBatteryImage == 2) return;
			batteryImagePath = battery2ImagePath;
			loadedBatteryImage = 2;
		} else if (batteryLevel == 0x3 || batteryLevel == 0x1) {
			if (loadedBatteryImage == 1) return;
			batteryImagePath = battery1ImagePath;
			loadedBatteryImage = 1;
		} else {
			return;
		}
	} else {
		if (batteryLevel & BIT(0)) {
			if (loadedBatteryImage == 1) return;
			batteryImagePath = batteryLowImagePath;
			loadedBatteryImage = 1;
		} else {
			if (loadedBatteryImage == 0) return;
			batteryImagePath = batteryFullImagePath;
			loadedBatteryImage = 0;
		}
	}

	FILE* file = fopen(batteryImagePath, "rb");

	if (file) {
		// Start loading
		fseek(file, 0xe, SEEK_SET);
		u8 pixelStart = (u8)fgetc(file) + 0xe;
		fseek(file, pixelStart, SEEK_SET);
		fread(bmpImageBuffer, 2, 0x200, file);
		u16* src = bmpImageBuffer;
		int x = 235;
		int y = 5+10;
		for (int i=0; i<18*11; i++) {
			if (x >= 235+18) {
				x = 235;
				y--;
			}
			u16 val = *(src++);
			if (val != 0x7C1F) {	// Do not render magneta pixel
				BG_GFX_SUB[y*256+x] = ((val>>10)&0x1f) | ((val)&(0x1f<<5)) | (val&0x1f)<<10 | BIT(15);
			}
			x++;
		}
	}

	fclose(file);
}

void loadPhotoList()
{
	DIR *dir;
	struct dirent *ent;
	std::string photoDir;
	bool dirOpen = false;
	std::string dirPath;
	if ((dir = opendir("sd:/_nds/TWiLightMenu/dsimenu/photos/")) != NULL) {
		dirOpen = true;
		dirPath = "sd:/_nds/TWiLightMenu/dsimenu/photos/";
	} else if ((dir = opendir("fat:/_nds/TWiLightMenu/dsimenu/photos/")) != NULL) {
		dirOpen = true;
		dirPath = "fat:/_nds/TWiLightMenu/dsimenu/photos/";
	}

	if(dirOpen) {
		/* print all the files and directories within directory */
		while ((ent = readdir(dir)) != NULL)
		{
			// Reallocation here, but prevents our vector from being filled with

			photoDir = ent->d_name;
			if (photoDir == ".." || photoDir == "..." || photoDir == "." || photoDir.substr(0,2) == "._" || photoDir.substr(photoDir.find_last_of(".") + 1) != "bmp") continue;

			photoList.emplace_back(dirPath+photoDir);
		}
		closedir(dir);
	}

	photoPath = photoList[rand()/((RAND_MAX+1u)/photoList.size())];
}

void loadPhoto() {
	FILE* file = fopen(photoPath.c_str(), "rb");
	if (!file) file = fopen("nitro:/graphics/photo_default.bmp", "rb");

	if (file) {
		// Start loading
		fseek(file, 0xe, SEEK_SET);
		u8 pixelStart = (u8)fgetc(file) + 0xe;
		fseek(file, pixelStart, SEEK_SET);
		fread(bmpImageBuffer, 2, 0x10000, file);
		u16* src = bmpImageBuffer;
		int x = 24;
		int y = 24+155;
		for (int i=0; i<208*156; i++) {
			if (x >= 24+208) {
				x = 24;
				y--;
			}
			u16 val = *(src++);
			BG_GFX_SUB[y*256+x] = ((val>>10)&0x1f) | ((val)&(0x1f<<5)) | (val&0x1f)<<10 | BIT(15);
			x++;
		}
	}

	fclose(file);
}

// Load photo without overwriting shoulder button images
void loadPhotoPart() {
	FILE* file = fopen(photoPath.c_str(), "rb");
	if (!file) file = fopen("nitro:/graphics/photo_default.bmp", "rb");

	if (file) {
		// Start loading
		fseek(file, 0xe, SEEK_SET);
		u8 pixelStart = (u8)fgetc(file) + 0xe;
		fseek(file, pixelStart, SEEK_SET);
		fread(bmpImageBuffer, 2, 0x10000, file);
		u16* src = bmpImageBuffer;
		int x = 24;
		int y = 24+155;
		for (int i=0; i<208*156; i++) {
			if (x >= 24+208) {
				x = 24;
				y--;
			}
			u16 val = *(src++);
			if (y <= 24+147) {
				BG_GFX_SUB[y*256+x] = ((val>>10)&0x1f) | ((val)&(0x1f<<5)) | (val&0x1f)<<10 | BIT(15);
			}
			x++;
		}
	}

	fclose(file);
}

void loadBMP(const char* filename) {
	FILE* file = fopen(filename, "rb");

	if (file) {
		// Start loading
		fseek(file, 0xe, SEEK_SET);
		u8 pixelStart = (u8)fgetc(file) + 0xe;
		fseek(file, pixelStart, SEEK_SET);
		fread(bmpImageBuffer, 2, 0x1A000, file);
		u16* src = bmpImageBuffer;
		int x = 0;
		int y = 191;
		for (int i=0; i<256*192; i++) {
			if (x >= 256) {
				x = 0;
				y--;
			}
			u16 val = *(src++);
			if (val != 0xFC1F) {	// Do not render magneta pixel
				BG_GFX_SUB[y*256+x] = ((val>>10)&0x1f) | ((val)&(0x1f<<5)) | (val&0x1f)<<10 | BIT(15);
			}
			x++;
		}
	}

	fclose(file);
}

// Load .bmp file without overwriting shoulder button images or username
void loadBMPPart(const char* filename) {
	FILE* file = fopen(filename, "rb");

	if (file) {
		// Start loading
		fseek(file, 0xe, SEEK_SET);
		u8 pixelStart = (u8)fgetc(file) + 0xe;
		fseek(file, pixelStart, SEEK_SET);
		fread(bmpImageBuffer, 2, 0x1A000, file);
		u16* src = bmpImageBuffer;
		int x = 0;
		int y = 191;
		for (int i=0; i<256*192; i++) {
			if (x >= 256) {
				x = 0;
				y--;
			}
			u16 val = *(src++);
			if (y >= 32 && y <= 167 && val != 0xFC1F) {
				BG_GFX_SUB[y*256+x] = ((val>>10)&0x1f) | ((val)&(0x1f<<5)) | (val&0x1f)<<10 | BIT(15);
			}
			x++;
		}
	}

	fclose(file);
}

void loadShoulders() {
	FILE* file;

	// Draw L shoulder
	if (showLshoulder)
	{ 
		file = fopen(tex().shoulderLPath.c_str(), "rb");
	} else {
		file = fopen(tex().shoulderLGreyPath.c_str(), "rb");
	}

	if (file) {
		// Start loading
		fseek(file, 0xe, SEEK_SET);
		u8 pixelStart = (u8)fgetc(file) + 0xe;
		fseek(file, pixelStart, SEEK_SET);
		for (int y=19; y>=0; y--) {
			u16 buffer[78];
			fread(buffer, 2, 0x4E, file);
			u16* src = buffer;
			for (int i=0; i<78; i++) {
				u16 val = *(src++);
				if (val != 0xFC1F) {	// Do not render magneta pixel
					BG_GFX_SUB[(y+172)*256+i] = ((val>>10)&0x1f) | ((val)&(0x1f<<5)) | (val&0x1f)<<10 | BIT(15);
				}
			}
		}
	}

	fclose(file);

	// Draw R shoulder

	if (showRshoulder)
	{ 
		file = fopen(tex().shoulderRPath.c_str(), "rb");
	} else {
		file = fopen(tex().shoulderRGreyPath.c_str(), "rb");
	}
	
	if (file) {
		// Start loading
		fseek(file, 0xe, SEEK_SET);
		u8 pixelStart = (u8)fgetc(file) + 0xe;
		fseek(file, pixelStart, SEEK_SET);
		for (int y=19; y>=0; y--) {
			u16 buffer[78];
			fread(buffer, 2, 0x4E, file);
			u16* src = buffer;
			for (int i=0; i<78; i++) {
				u16 val = *(src++);
				if (val != 0xFC1F) {	// Do not render magneta pixel
					BG_GFX_SUB[(y+172)*256+(i+178)] = ((val>>10)&0x1f) | ((val)&(0x1f<<5)) | (val&0x1f)<<10 | BIT(15);
				}
			}
		}
	}

	fclose(file);
}

/**
 * Get the index in the UV coordinate array where the letter appears
 */
unsigned int getTopFontSpriteIndex(const u16 letter) {
	unsigned int spriteIndex = 0;
	for (unsigned int i = 0; i < TOP_FONT_NUM_IMAGES; i++) {
		if (top_utf16_lookup_table[i] == letter) {
			spriteIndex = i;
		}
	}
	return spriteIndex;
}

unsigned int getDateTimeFontSpriteIndex(const u16 letter) {
	unsigned int spriteIndex = 0;
	for (unsigned int i = 0; i < DATE_TIME_FONT_NUM_IMAGES; i++) {
		if (date_time_utf16_lookup_table[i] == letter) {
			spriteIndex = i;
		}
	}
	return spriteIndex;
}

//   xrrrrrgggggbbbbb according to http://problemkaputt.de/gbatek.htm#dsvideobgmodescontrol
#define MASK_RB      0b0111110000011111
#define MASK_G       0b0000001111100000 
#define MASK_MUL_RB  0b0111110000011111000000 
#define MASK_MUL_G   0b0000001111100000000000 
#define MAX_ALPHA        64 // 6bits+1 with rounding

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

void topBgLoad() {
	loadBMP(tex().topBgPath.c_str());

	// Load username
	char fontPath[64];
	FILE* file;
	int x = (isDSiMode() ? 28 : 4); 

	for (int c = 0; c < 10; c++) {
		unsigned int charIndex = getTopFontSpriteIndex(usernameRendered[c]);
		// 42 characters per line.
		unsigned int texIndex = charIndex / 42;
		sprintf(fontPath, "nitro:/graphics/top_font/small_font_%u.bmp", texIndex);
		
		file = fopen(fontPath, "rb");

		if (file) {
			// Start loading
			fseek(file, 0xe, SEEK_SET);
			u8 pixelStart = (u8)fgetc(file) + 0xe;
			fseek(file, pixelStart, SEEK_SET);
			for (int y=15; y>=0; y--) {
				u16 buffer[512];
				fread(buffer, 2, 0x200, file);
				u16* src = buffer+(top_font_texcoords[0+(4*charIndex)]);

				for (u16 i=0; i < top_font_texcoords[2+(4*charIndex)]; i++) {
					u16 val = *(src++);
					u16 bg = BG_GFX_SUB[(y+2)*256+(i+x)]; //grab the background pixel
					// Apply palette here.
					
					// Magic numbers were found by dumping val to stdout
					// on case default.
					switch (val) {
						// #ff00ff
						case 0xFC1F:
							break;
						// #414141
						case 0xA108:
							val = bmpPal_topSmallFont[1+((PersonalData->theme)*16)];
							break;
						case 0xC210:
							// blend the colors with the background to make it look better.
							val = alphablend(bmpPal_topSmallFont[2+((PersonalData->theme)*16)], bg, 48);
							break;
						case 0xDEF7:
							val = alphablend(bmpPal_topSmallFont[3+((PersonalData->theme)*16)], bg, 64);
						default:
							break;
					}
					if (val != 0xFC1F) {	// Do not render magneta pixel
						BG_GFX_SUB[(y+2)*256+(i+x)] = ((val>>10)&0x1f) | ((val)&(0x1f<<5)) | (val&0x1f)<<10 | BIT(15);
					}
				}
			}
			x += top_font_texcoords[2+(4*charIndex)];
		}

		fclose(file);
	}
}

void loadDate() {
	// Load date
	char fontPath[64];
	FILE* file;
	int x = 162;
	char date[6];

	GetDate(FORMAT_MD, date, sizeof(date));

	for (int c = 0; c < 5; c++) {
		unsigned int charIndex = getDateTimeFontSpriteIndex(date[c]);
		switch (theme) {
			case 0:
			default:
				if (subtheme == 7) sprintf(fontPath, "nitro:/graphics/top_font/purple_date_time_font.bmp");
				else if (subtheme == 6) sprintf(fontPath, "nitro:/graphics/top_font/pink_date_time_font.bmp");
				else if (subtheme == 5) sprintf(fontPath, "nitro:/graphics/top_font/yellow_date_time_font.bmp");
				else if (subtheme == 4) sprintf(fontPath, "nitro:/graphics/top_font/green_date_time_font.bmp");
				else if (subtheme == 3) sprintf(fontPath, "nitro:/graphics/top_font/blue_date_time_font.bmp");
				else if (subtheme == 2) sprintf(fontPath, "nitro:/graphics/top_font/red_date_time_font.bmp");
				else if (subtheme == 1) sprintf(fontPath, "nitro:/graphics/top_font/date_time_font.bmp");
				else sprintf(fontPath, "nitro:/graphics/top_font/dark_date_time_font.bmp");
				break;
			case 1:
				sprintf(fontPath, "nitro:/graphics/top_font/date_time_font.bmp");
				break;
		}

		file = fopen(fontPath, "rb");

		if (file) {
			// Start date
			fseek(file, 0xe, SEEK_SET);
			u8 pixelStart = (u8)fgetc(file) + 0xe;
			fseek(file, pixelStart, SEEK_SET);
			for (int y=14; y>=6; y--) {
				u16 buffer[512];
				fread(buffer, 2, 0x200, file);
				u16* src = buffer+(date_time_font_texcoords[0+(4*charIndex)]);

				for (u16 i=0; i < date_time_font_texcoords[2+(4*charIndex)]; i++) {
					u16 val = *(src++);
					if (val != 0xFC1F) {	// Do not render magneta pixel
						BG_GFX_SUB[(y)*256+(i+x)] = ((val>>10)&0x1f) | ((val)&(0x1f<<5)) | (val&0x1f)<<10 | BIT(15);
					}
				}
			}
			x += date_time_font_texcoords[2+(4*charIndex)];
		}

		fclose(file);
	}
}

static std::string loadedTime;
static int hourWidth;
static bool initialClockDraw = true;

void loadTime() {
	// Load time
	char fontPath[64];
	FILE* file;
	int x = 200;
	char time[10];
	std::string currentTime = RetTime();
	currentTime.replace(2, 1, " ");
	if(currentTime != loadedTime) {
		loadedTime = currentTime;
		if(currentTime.substr(0,1) == " ")
			currentTime = "0" + currentTime.substr(1);
		sprintf(time, currentTime.c_str());

		int	howManyToDraw = 5;
		if(initialClockDraw) {
			initialClockDraw = false;
		} else {
			if(currentTime.substr(3,2) != "00") {
				strcpy(time, currentTime.substr(3,2).c_str());
				howManyToDraw = 2;
				x = hourWidth;
			}
		}

		for (int c = 0; c < howManyToDraw; c++) {
			unsigned int charIndex = getDateTimeFontSpriteIndex(time[c]);
			switch (theme) {
				case 0:
				default:
					if (subtheme == 7) sprintf(fontPath, "nitro:/graphics/top_font/purple_date_time_font.bmp");
					else if (subtheme == 6) sprintf(fontPath, "nitro:/graphics/top_font/pink_date_time_font.bmp");
					else if (subtheme == 5) sprintf(fontPath, "nitro:/graphics/top_font/yellow_date_time_font.bmp");
					else if (subtheme == 4) sprintf(fontPath, "nitro:/graphics/top_font/green_date_time_font.bmp");
					else if (subtheme == 3) sprintf(fontPath, "nitro:/graphics/top_font/blue_date_time_font.bmp");
					else if (subtheme == 2) sprintf(fontPath, "nitro:/graphics/top_font/red_date_time_font.bmp");
					else if (subtheme == 1) sprintf(fontPath, "nitro:/graphics/top_font/date_time_font.bmp");
					else sprintf(fontPath, "nitro:/graphics/top_font/dark_date_time_font.bmp");
					break;
				case 1:
					sprintf(fontPath, "nitro:/graphics/top_font/date_time_font.bmp");
					break;
			}

			file = fopen(fontPath, "rb");

			if (file) {
				// Start loading
				fseek(file, 0xe, SEEK_SET);
				u8 pixelStart = (u8)fgetc(file) + 0xe;
				fseek(file, pixelStart, SEEK_SET);
				for (int y=14; y>=6; y--) {
					u16 buffer[512];
					fread(buffer, 2, 0x200, file);
					u16* src = buffer+(date_time_font_texcoords[0+(4*charIndex)]);

					for (u16 i=0; i < date_time_font_texcoords[2+(4*charIndex)]; i++) {
						u16 val = *(src++);
						if (val != 0xFC1F) {	// Do not render magneta pixel
							BG_GFX_SUB[(y)*256+(i+x)] = ((val>>10)&0x1f) | ((val)&(0x1f<<5)) | (val&0x1f)<<10 | BIT(15);
						}
					}
				}
				x += date_time_font_texcoords[2+(4*charIndex)];
				if(c == 2) hourWidth = x;
			}
		}

		fclose(file);
	}
}

static std::string loadedColon;

void loadClockColon() {
	// Load time
	char fontPath[64];
	FILE* file;
	int x = 214;
	char colon[5];
	std::string currentColon = RetTime().substr(2,1);
	if(currentColon != ":")	currentColon = ";";
	if(currentColon != loadedColon) {
		loadedColon = currentColon;
		sprintf(colon, currentColon.c_str());

		for (int c = 0; c < 1; c++) {
			unsigned int charIndex = getDateTimeFontSpriteIndex(colon[c]);
			switch (theme) {
				case 0:
				default:
					if (subtheme == 7) sprintf(fontPath, "nitro:/graphics/top_font/purple_date_time_font.bmp");
					else if (subtheme == 6) sprintf(fontPath, "nitro:/graphics/top_font/pink_date_time_font.bmp");
					else if (subtheme == 5) sprintf(fontPath, "nitro:/graphics/top_font/yellow_date_time_font.bmp");
					else if (subtheme == 4) sprintf(fontPath, "nitro:/graphics/top_font/green_date_time_font.bmp");
					else if (subtheme == 3) sprintf(fontPath, "nitro:/graphics/top_font/blue_date_time_font.bmp");
					else if (subtheme == 2) sprintf(fontPath, "nitro:/graphics/top_font/red_date_time_font.bmp");
					else if (subtheme == 1) sprintf(fontPath, "nitro:/graphics/top_font/date_time_font.bmp");
					else sprintf(fontPath, "nitro:/graphics/top_font/dark_date_time_font.bmp");
					break;
				case 1:
					sprintf(fontPath, "nitro:/graphics/top_font/date_time_font.bmp");
					break;
			}

			file = fopen(fontPath, "rb");

			if (file) {
				// Start loading
				fseek(file, 0xe, SEEK_SET);
				u8 pixelStart = (u8)fgetc(file) + 0xe;
				fseek(file, pixelStart, SEEK_SET);
				for (int y=14; y>=6; y--) {
					u16 buffer[512];
					fread(buffer, 2, 0x200, file);
					u16* src = buffer+(date_time_font_texcoords[0+(4*charIndex)]);

					for (u16 i=0; i < date_time_font_texcoords[2+(4*charIndex)]; i++) {
						u16 val = *(src++);
						if (val != 0xFC1F) {	// Do not render magneta pixel
							BG_GFX_SUB[(y)*256+(i+x)] = ((val>>10)&0x1f) | ((val)&(0x1f<<5)) | (val&0x1f)<<10 | BIT(15);
						}
					}
				}
				x += date_time_font_texcoords[2+(4*charIndex)];
				if(c == 2) hourWidth = x;
			}
		}

		fclose(file);
	}
}

void clearBoxArt() {
	if (theme == 1) {
		loadBMPPart("nitro:/graphics/3ds_top.bmp");
	} else {
		loadPhotoPart();
	}
}

void graphicsInit()
{
	
	for (int i = 0; i < 12; i++) {
		launchDotFrame[i] = 5;
	}

	for (int i = 0; i < 5; i++) {
		dropTime[i] = 0;
		dropSeq[i] = 0;
		dropSpeed[i] = 5;
		dropSpeedChange[i] = 0;
		if (theme == 1) titleboxYposDropDown[i] = 0;
		else titleboxYposDropDown[i] = -85-80;
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

	titleboxXpos[0] = cursorPosition[0]*64;
	titlewindowXpos[0] = cursorPosition[0]*5;
	titleboxXpos[1] = cursorPosition[1]*64;
	titlewindowXpos[1] = cursorPosition[1]*5;

	*(u16*)(0x0400006C) |= BIT(14);
	*(u16*)(0x0400006C) &= BIT(15);
	SetBrightness(0, 31);
	SetBrightness(1, 31);

	////////////////////////////////////////////////////////////
	videoSetMode(MODE_5_3D | DISPLAY_BG2_ACTIVE);
	videoSetModeSub(MODE_3_2D | DISPLAY_BG3_ACTIVE);

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
	
	REG_BG3CNT_SUB = BG_MAP_BASE(0) | BG_BMP16_256x256 | BG_PRIORITY(0);
	REG_BG3X_SUB = 0;
	REG_BG3Y_SUB = 0;
	REG_BG3PA_SUB = 1<<8;
	REG_BG3PB_SUB = 0;
	REG_BG3PC_SUB = 0;
	REG_BG3PD_SUB = 1<<8;

	if (theme < 1) {
		srand(time(NULL));
		loadPhotoList();
		loadPhoto();
	}

	// Initialize the bottom background
	bottomBg = bgInit(2, BgType_ExRotation, BgSize_ER_256x256, 0,1);

	swiWaitForVBlank();

	if (theme == 1) {
		tex().load3DSTheme();
		titleboxYpos = 96;
		bubbleYpos += 18;
		bubbleXpos += 3;
		topBgLoad();
		loadDate();
		loadTime();
		loadClockColon();
		bottomBgLoad(false, true);
	} else {
		switch(subtheme) {
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
		loadDate();
		loadTime();
		loadClockColon();
		bottomBgLoad(false, true);
	}

	setVolumeImagePaths();
	loadVolumeImage();
	setBatteryImagePaths();
	loadBatteryImage();

	irqSet(IRQ_VBLANK, vBlankHandler);
	irqEnable(IRQ_VBLANK);
	//consoleDemoInit();


}
