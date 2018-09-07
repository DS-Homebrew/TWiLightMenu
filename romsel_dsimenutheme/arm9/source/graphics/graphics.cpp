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
#include "common/gl2d.h"
#include "bios_decompress_callback.h"
#include "FontGraphic.h"

// Graphic files
#include "bottom.h"
#include "bottom_bubble.h"

#include "org_bottom.h"
#include "org_bottom_bubble.h"

#include "red_bottom.h"
#include "red_bottom_bubble.h"

#include "blue_bottom.h"
#include "blue_bottom_bubble.h"

#include "green_bottom.h"
#include "green_bottom_bubble.h"

#include "yellow_bottom.h"
#include "yellow_bottom_bubble.h"

#include "pink_bottom.h"
#include "pink_bottom_bubble.h"

#include "purple_bottom.h"
#include "purple_bottom_bubble.h"

#include "_3ds_bottom.h"
#include "_3ds_bottom_bubble.h"

// #include "dialogbox.h"
// #include "nintendo_dsi_menu.h"
// #include "org_nintendo_dsi_menu.h"
// #include "bubble.h"
// #include "org_bubble.h"
// #include "red_bubble.h"
// #include "blue_bubble.h"
// #include "green_bubble.h"
// #include "yellow_bubble.h"
// #include "pink_bubble.h"
// #include "purple_bubble.h"
// #include "_3ds_bubble.h"
// #include "progress.h"
// #include "bips.h"
// #include "scroll_window.h"
// #include "org_scroll_window.h"
// #include "red_scroll_window.h"
// #include "blue_scroll_window.h"
// #include "green_scroll_window.h"
// #include "yellow_scroll_window.h"
// #include "pink_scroll_window.h"
// #include "purple_scroll_window.h"
// #include "button_arrow.h"
// #include "launch_dot.h"
// #include "start_text.h"
// #include "start_border.h"
#include "../include/startborderpal.h"
// #include "_3ds_cursor.h"
// #include "brace.h"
// #include "org_brace.h"
// #include "red_brace.h"
// #include "blue_brace.h"
// #include "green_brace.h"
// #include "yellow_brace.h"
// #include "pink_brace.h"
// #include "purple_brace.h"
// #include "icon_settings.h"
// #include "org_icon_settings.h"
// #include "box.h"
// #include "org_box.h"
// #include "red_box.h"
// #include "blue_box.h"
// #include "green_box.h"
// #include "yellow_box.h"
// #include "pink_box.h"
// #include "purple_box.h"
// #include "_3ds_box_full.h"
// #include "_3ds_box_empty.h"
// #include "folder.h"
// #include "org_folder.h"
// #include "_3ds_folder.h"
// #include "wirelessicons.h"

#include "graphics/ThemeTextures.h"

#include "queueControl.h"
#include "uvcoord_top_font.h"

#include "../iconTitle.h"
#include "graphics.h"
#include "fontHandler.h"
#include "../ndsheaderbanner.h"
#include "../language.h"
#include "../perGameSettings.h"
#include "../flashcard.h"
#include "iconHandler.h"
#define CONSOLE_SCREEN_WIDTH 32
#define CONSOLE_SCREEN_HEIGHT 24

extern u16 usernameRendered[10];

extern bool whiteScreen;
extern bool fadeType;
extern bool fadeSpeed;
extern bool controlTopBright;
extern bool controlBottomBright;
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
int titleboxYposDropDown[5] = {-85-80};
int allowedTitleboxForDropDown = 0;
int delayForTitleboxToDropDown = 0;
extern bool showbubble;
extern bool showSTARTborder;
extern bool isScrolling;
extern bool needToPlayStopSound;
extern int waitForNeedToPlayStopSound;

extern bool titleboxXmoveleft;
extern bool titleboxXmoveright;

extern bool applaunchprep;

int screenBrightness = 31;

int movetimer = 0;

int titleboxYmovepos = 0;

extern int spawnedtitleboxes;

extern bool startMenu;

extern int theme;
extern int subtheme;
extern int cursorPosition[2];
extern int startMenu_cursorPosition;
extern int pagenum[2];
//int titleboxXmovespeed[8] = {8};
int titleboxXmovespeed[8] = {12, 10, 8, 8, 8, 8, 6, 4};
int titleboxXpos[2] = {0};
int startMenu_titleboxXpos;
int titleboxYpos = 85;	// 85, when dropped down
int titlewindowXpos[2] = {0};
int startMenu_titlewindowXpos;

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
float dbox_movespeed = 22;
float dbox_Ypos = -192;

int bottomBg;

int bottomBgState = 0; // 0 = Uninitialized 1 = No Bubble 2 = bubble.

// int subBgTexID, mainBgTexID, shoulderTexID, ndsimenutextTexID, bubbleTexID, progressTexID, dialogboxTexID, wirelessiconTexID;
// int bipsTexID, scrollwindowTexID, buttonarrowTexID, launchdotTexID, startTexID, startbrdTexID, settingsTexID, braceTexID, boxfullTexID, boxemptyTexID, folderTexID;

// //glImage subBgImage[(256 / 16) * (256 / 16)];
// //glImage mainBgImage[(256 / 16) * (256 / 16)];
// //glImage shoulderImage[(128 / 16) * (64 / 32)];
// //glImage ndsimenutextImage[(256 / 16) * (32 / 16)];

// glImage tex().bubbleImage()[1];
// glImage tex().progressImage()[(16 / 16) * (128 / 16)];
// glImage tex().dialogboxImage()[(256 / 16) * (256 / 16)];
// glImage tex().bipsImage()[(8 / 8) * (32 / 8)];
// glImage tex().scrollwindowImage()[(32 / 16) * (32 / 16)];
// glImage tex().buttonarrowImage()[(32 / 32) * (64 / 32)];
// glImage tex().launchdotImage()[(16 / 16) * (96 / 16)];
// glImage tex().startImage()[(64 / 16) * (128 / 16)];
// glImage tex().startbrdImage()[(32 / 32) * (256 / 80)];
// glImage tex().startbrdImage()[(32 / 32) * (192 / 64)];
// glImage tex().braceImage()[(16 / 16) * (128 / 16)];
// glImage tex().settingsImage()[(64 / 16) * (128 / 64)];
// glImage boxfullImage[(64 / 16) * (128 / 64)];
// glImage boxemptyImage[(64 / 16) * (64 / 16)];
// glImage tex().folderImage()[(64 / 16) * (64 / 16)];
// glImage tex().wirelessIcons()[(32 / 32) * (64 / 32)];


int vblankRefreshCounter = 0;

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
	swiIntrWait(0, 1);
	swiIntrWait(0, 1);
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

void startMenu_moveIconClose(int num) {
	if (titleboxXmoveleft) {
		movecloseXpos = 0;
		if(movetimer == 1) {
			if (startMenu_cursorPosition-2 == num) movecloseXpos = 1;
			else if (startMenu_cursorPosition+2 == num) movecloseXpos = -2;
		} else if(movetimer == 2) {
			if (startMenu_cursorPosition-2 == num) movecloseXpos = 1;
			else if (startMenu_cursorPosition+2 == num) movecloseXpos = -2;
		} else if(movetimer == 3) {
			if (startMenu_cursorPosition-2 == num) movecloseXpos = 2;
			else if (startMenu_cursorPosition+2 == num) movecloseXpos = -3;
		} else if(movetimer == 4) {
			if (startMenu_cursorPosition-2 == num) movecloseXpos = 2;
			else if (startMenu_cursorPosition+2 == num) movecloseXpos = -3;
		} else if(movetimer == 5) {
			if (startMenu_cursorPosition-2 == num) movecloseXpos = 3;
			else if (startMenu_cursorPosition+2 == num) movecloseXpos = -4;
		} else if(movetimer == 6) {
			if (startMenu_cursorPosition-2 == num) movecloseXpos = 4;
			else if (startMenu_cursorPosition+2 == num) movecloseXpos = -5;
		} else if(movetimer == 7) {
			if (startMenu_cursorPosition-2 == num) movecloseXpos = 5;
			else if (startMenu_cursorPosition+2 == num) movecloseXpos = -6;
		} else if(movetimer == 8) {
			if (startMenu_cursorPosition-2 == num) movecloseXpos = 6;
			else if (startMenu_cursorPosition+2 == num) movecloseXpos = -7;
		}
	}
	if (titleboxXmoveright) {
		movecloseXpos = 0;
		if(movetimer == 1) {
			if (startMenu_cursorPosition-2 == num) movecloseXpos = 2;
			else if (startMenu_cursorPosition+2 == num) movecloseXpos = -1;
		} else if(movetimer == 2) {
			if (startMenu_cursorPosition-2 == num) movecloseXpos = 2;
			else if (startMenu_cursorPosition+2 == num) movecloseXpos = -1;
		} else if(movetimer == 3) {
			if (startMenu_cursorPosition-2 == num) movecloseXpos = 3;
			else if (startMenu_cursorPosition+2 == num) movecloseXpos = -2;
		} else if(movetimer == 4) {
			if (startMenu_cursorPosition-2 == num) movecloseXpos = 3;
			else if (startMenu_cursorPosition+2 == num) movecloseXpos = -2;
		} else if(movetimer == 5) {
			if (startMenu_cursorPosition-2 == num) movecloseXpos = 4;
			else if (startMenu_cursorPosition+2 == num) movecloseXpos = -3;
		} else if(movetimer == 6) {
			if (startMenu_cursorPosition-2 == num) movecloseXpos = 5;
			else if (startMenu_cursorPosition+2 == num) movecloseXpos = -4;
		} else if(movetimer == 7) {
			if (startMenu_cursorPosition-2 == num) movecloseXpos = 6;
			else if (startMenu_cursorPosition+2 == num) movecloseXpos = -5;
		} else if(movetimer == 8) {
			if (startMenu_cursorPosition-2 == num) movecloseXpos = 7;
			else if (startMenu_cursorPosition+2 == num) movecloseXpos = -6;
		}
	}
	if(!titleboxXmoveleft || !titleboxXmoveright) {
		if (startMenu_cursorPosition-2 == num) movecloseXpos = 6;
		else if (startMenu_cursorPosition+2 == num) movecloseXpos = -6;
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

	swiIntrWait(0, 1);

	oamUpdate(&oamSub);
}

void bottomBgLoad(bool drawBubble, bool init = false) {
	if (init || (!drawBubble && bottomBgState == 2)) {
		if (theme == 1) {
			dmaCopy(_3ds_bottomTiles, bgGetGfxPtr(bottomBg), _3ds_bottomTilesLen);
			dmaCopy(_3ds_bottomPal, BG_PALETTE, _3ds_bottomPalLen);
			dmaCopy(_3ds_bottomMap, bgGetMapPtr(bottomBg), _3ds_bottomMapLen);
		} else if (subtheme == 7) {
			dmaCopy(purple_bottomTiles, bgGetGfxPtr(bottomBg), purple_bottomTilesLen);
			dmaCopy(purple_bottomPal, BG_PALETTE, purple_bottomPalLen);
			dmaCopy(purple_bottomMap, bgGetMapPtr(bottomBg), purple_bottomMapLen);
		} else if (subtheme == 6) {
			dmaCopy(pink_bottomTiles, bgGetGfxPtr(bottomBg), pink_bottomTilesLen);
			dmaCopy(pink_bottomPal, BG_PALETTE, pink_bottomPalLen);
			dmaCopy(pink_bottomMap, bgGetMapPtr(bottomBg), pink_bottomMapLen);
		} else if (subtheme == 5) {
			dmaCopy(yellow_bottomTiles, bgGetGfxPtr(bottomBg), yellow_bottomTilesLen);
			dmaCopy(yellow_bottomPal, BG_PALETTE, yellow_bottomPalLen);
			dmaCopy(yellow_bottomMap, bgGetMapPtr(bottomBg), yellow_bottomMapLen);
		} else if (subtheme == 4) {
			dmaCopy(green_bottomTiles, bgGetGfxPtr(bottomBg), green_bottomTilesLen);
			dmaCopy(green_bottomPal, BG_PALETTE, green_bottomPalLen);
			dmaCopy(green_bottomMap, bgGetMapPtr(bottomBg), green_bottomMapLen);
		} else if (subtheme == 3) {
			dmaCopy(blue_bottomTiles, bgGetGfxPtr(bottomBg), blue_bottomTilesLen);
			dmaCopy(blue_bottomPal, BG_PALETTE, blue_bottomPalLen);
			dmaCopy(blue_bottomMap, bgGetMapPtr(bottomBg), blue_bottomMapLen);
		} else if (subtheme == 2) {
			dmaCopy(red_bottomTiles, bgGetGfxPtr(bottomBg), red_bottomTilesLen);
			dmaCopy(red_bottomPal, BG_PALETTE, red_bottomPalLen);
			dmaCopy(red_bottomMap, bgGetMapPtr(bottomBg), red_bottomMapLen);
		} else if (subtheme == 1) {
			dmaCopy(org_bottomTiles, bgGetGfxPtr(bottomBg), org_bottomTilesLen);
			dmaCopy(org_bottomPal, BG_PALETTE, org_bottomPalLen);
			dmaCopy(org_bottomMap, bgGetMapPtr(bottomBg), org_bottomMapLen);
		} else {
			dmaCopy(bottomTiles, bgGetGfxPtr(bottomBg), bottomTilesLen);
			dmaCopy(bottomPal, BG_PALETTE, bottomPalLen);
			dmaCopy(bottomMap, bgGetMapPtr(bottomBg), bottomMapLen);
		}
		// Set that we've not drawn the bubble.
		bottomBgState = 1;
	} else if (drawBubble && bottomBgState == 1){
		if (theme == 1) {
			dmaCopy(_3ds_bottom_bubbleTiles, bgGetGfxPtr(bottomBg), _3ds_bottom_bubbleTilesLen);
			dmaCopy(_3ds_bottom_bubblePal, BG_PALETTE, _3ds_bottom_bubblePalLen);
			dmaCopy(_3ds_bottom_bubbleMap, bgGetMapPtr(bottomBg), _3ds_bottom_bubbleMapLen);
		} else if (subtheme == 7) {
			dmaCopy(purple_bottom_bubbleTiles, bgGetGfxPtr(bottomBg), purple_bottom_bubbleTilesLen);
			dmaCopy(purple_bottom_bubblePal, BG_PALETTE, purple_bottom_bubblePalLen);
			dmaCopy(purple_bottom_bubbleMap, bgGetMapPtr(bottomBg), purple_bottom_bubbleMapLen);
		} else if (subtheme == 6) {
			dmaCopy(pink_bottom_bubbleTiles, bgGetGfxPtr(bottomBg), pink_bottom_bubbleTilesLen);
			dmaCopy(pink_bottom_bubblePal, BG_PALETTE, pink_bottom_bubblePalLen);
			dmaCopy(pink_bottom_bubbleMap, bgGetMapPtr(bottomBg), pink_bottom_bubbleMapLen);
		} else if (subtheme == 5) {
			dmaCopy(yellow_bottom_bubbleTiles, bgGetGfxPtr(bottomBg), yellow_bottom_bubbleTilesLen);
			dmaCopy(yellow_bottom_bubblePal, BG_PALETTE, yellow_bottom_bubblePalLen);
			dmaCopy(yellow_bottom_bubbleMap, bgGetMapPtr(bottomBg), yellow_bottom_bubbleMapLen);
		} else if (subtheme == 4) {
			dmaCopy(green_bottom_bubbleTiles, bgGetGfxPtr(bottomBg), green_bottom_bubbleTilesLen);
			dmaCopy(green_bottom_bubblePal, BG_PALETTE, green_bottom_bubblePalLen);
			dmaCopy(green_bottom_bubbleMap, bgGetMapPtr(bottomBg), green_bottom_bubbleMapLen);
		} else if (subtheme == 3) {
			dmaCopy(blue_bottom_bubbleTiles, bgGetGfxPtr(bottomBg), blue_bottom_bubbleTilesLen);
			dmaCopy(blue_bottom_bubblePal, BG_PALETTE, blue_bottom_bubblePalLen);
			dmaCopy(blue_bottom_bubbleMap, bgGetMapPtr(bottomBg), blue_bottom_bubbleMapLen);
		} else if (subtheme == 2) {
			dmaCopy(red_bottom_bubbleTiles, bgGetGfxPtr(bottomBg), red_bottom_bubbleTilesLen);
			dmaCopy(red_bottom_bubblePal, BG_PALETTE, red_bottom_bubblePalLen);
			dmaCopy(red_bottom_bubbleMap, bgGetMapPtr(bottomBg), red_bottom_bubbleMapLen);
		} else if (subtheme == 1) {
			dmaCopy(org_bottom_bubbleTiles, bgGetGfxPtr(bottomBg), org_bottom_bubbleTilesLen);
			dmaCopy(org_bottom_bubblePal, BG_PALETTE, org_bottom_bubblePalLen);
			dmaCopy(org_bottom_bubbleMap, bgGetMapPtr(bottomBg), org_bottom_bubbleMapLen);
		} else {
			dmaCopy(bottom_bubbleTiles, bgGetGfxPtr(bottomBg), bottom_bubbleTilesLen);
			dmaCopy(bottom_bubblePal, BG_PALETTE, bottom_bubblePalLen);
			dmaCopy(bottom_bubbleMap, bgGetMapPtr(bottomBg), bottom_bubbleMapLen);
		}
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
	// glBindTexture(0, dialogboxTexID);
	// glColorSubTableEXT(0, 0, 4, 0, 0, (u16*) dialogboxPal);
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
			// Dialogbox moving up...
			if (dbox_movespeed <= 1) {
				if (dbox_Ypos >= 0) {
					// dbox stopped
					dbox_movespeed = 0;
					dbox_Ypos = 0;
				} else {
					// dbox moving up
					dbox_movespeed = 1;
				}
			} else {
				// Dbox decel
				dbox_movespeed -= 1.25;
			}
			dbox_Ypos += dbox_movespeed;
		} else {
			// Dialogbox moving down...
			if (dbox_Ypos <= -192 || dbox_Ypos >= 192) {
				dbox_movespeed = 22;
				dbox_Ypos = -192;
			} else {
				dbox_movespeed += 1;
				dbox_Ypos += dbox_movespeed;
			}
		}

		if (titleboxXmoveleft) {
			if(startMenu) {
				if (movetimer == 8) {
				//	if (showbubble && theme == 0) mmEffectEx(&snd_stop);
					needToPlayStopSound = true;
					startBorderZoomOut = true;
					startMenu_titlewindowXpos -= 1;
					movetimer++;
				} else if (movetimer < 8) {
					startMenu_titleboxXpos -= titleboxXmovespeed[movetimer];
					if(movetimer==0 || movetimer==2 || movetimer==4 || movetimer==6 ) startMenu_titlewindowXpos -= 1;
					movetimer++;
				} else {
					titleboxXmoveleft = false;
					movetimer = 0;
				}
			} else {
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
			}
		} else if (titleboxXmoveright) {
			if(startMenu) {
				if (movetimer == 8) {
					// if (showbubble && theme == 0) mmEffectEx(&snd_stop);
					needToPlayStopSound = true;
					startBorderZoomOut = true;
					startMenu_titlewindowXpos += 1;
					movetimer++;
				} else if (movetimer < 8) {
					startMenu_titleboxXpos += titleboxXmovespeed[movetimer];
					if(movetimer==0 || movetimer==2 || movetimer==4 || movetimer==6 ) startMenu_titlewindowXpos += 1;
					movetimer++;
				} else {
					titleboxXmoveright = false;
					movetimer = 0;
				}
			} else {
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

			if (theme==0) {
				glColor(RGB15(31, 31, 31));
				int bipXpos = 27;
				if(startMenu) {
					glSprite(16+startMenu_titlewindowXpos, 171, GL_FLIP_NONE, tex().scrollwindowImage());
					for(int i = 0; i < 40; i++) {
						if (i == 0) glSprite(bipXpos, 178, GL_FLIP_NONE, &tex().bipsImage()[2 & 31]);
						else if (i == 1) glSprite(bipXpos, 178, GL_FLIP_NONE, tex().bipsImage());
						else glSprite(bipXpos, 178, GL_FLIP_NONE, &tex().bipsImage()[1 & 31]);
						bipXpos += 5;
					}
					glSprite(16+startMenu_titlewindowXpos, 171, GL_FLIP_NONE, &tex().buttonarrowImage()[1]);
				} else {
					glSprite(16+titlewindowXpos[secondaryDevice], 171, GL_FLIP_NONE, tex().scrollwindowImage());
					for(int i = 0; i < 40; i++) {
						if (i < spawnedtitleboxes) glSprite(bipXpos, 178, GL_FLIP_NONE, tex().bipsImage());
						else glSprite(bipXpos, 178, GL_FLIP_NONE, &tex().bipsImage()[1 & 31]);
						bipXpos += 5;
					}
					glSprite(16+titlewindowXpos[secondaryDevice], 171, GL_FLIP_NONE, &tex().buttonarrowImage()[1]);
				}
				glSprite(0, 171, GL_FLIP_NONE, &tex().buttonarrowImage()[0]);
				glSprite(224, 171, GL_FLIP_H, &tex().buttonarrowImage()[0]);
				glColor(RGB15(31, 31, 31));
				if (startMenu) {
					glSprite(72-startMenu_titleboxXpos, 81, GL_FLIP_NONE, tex().braceImage());
				} else {
					glSprite(72-titleboxXpos[secondaryDevice], 81, GL_FLIP_NONE, tex().braceImage());
				}
			}
			int spawnedboxXpos = 96;
			int iconXpos = 112;
			if(startMenu) {
				for(int i = 0; i < 40; i++) {
					if (theme == 0) {
						startMenu_moveIconClose(i);
					} else {
						movecloseXpos = 0;
					}
					if (i == 0 &&  (!applaunchprep || startMenu_cursorPosition != i)) {
						glSprite(spawnedboxXpos-startMenu_titleboxXpos+movecloseXpos, (titleboxYpos-1)+titleboxYposDropDown[i % 5], GL_FLIP_NONE, &tex().settingsImage()[1 & 63]);
					} else if (i == 1 && (!applaunchprep || startMenu_cursorPosition != i)) {
						if (!flashcardFound()) {
							glSprite(spawnedboxXpos-startMenu_titleboxXpos+movecloseXpos, titleboxYpos+titleboxYposDropDown[i % 5], GL_FLIP_NONE, &tex().settingsImage()[0 & 63]);
						} else {
							if (theme == 1) glSprite(spawnedboxXpos-startMenu_titleboxXpos+movecloseXpos, titleboxYpos, GL_FLIP_NONE, tex().boxfullImage());
							else glSprite(spawnedboxXpos-startMenu_titleboxXpos+movecloseXpos, (titleboxYpos)+titleboxYposDropDown[i % 5], GL_FLIP_NONE, &tex().boxfullImage()[0 & 63]);
							drawIconGBA(iconXpos-startMenu_titleboxXpos+movecloseXpos, (titleboxYpos+12)+titleboxYposDropDown[i % 5]);
						}
					} else if (i == 2 && !flashcardFound() && (!applaunchprep || startMenu_cursorPosition != i)) {
						if (theme == 1) glSprite(spawnedboxXpos-startMenu_titleboxXpos+movecloseXpos, titleboxYpos, GL_FLIP_NONE, tex().boxfullImage());
						else glSprite(spawnedboxXpos-startMenu_titleboxXpos+movecloseXpos, titleboxYpos+titleboxYposDropDown[i % 5], GL_FLIP_NONE, &tex().boxfullImage()[0 & 63]);
						drawIconGBA(iconXpos-startMenu_titleboxXpos+movecloseXpos, (titleboxYpos+12)+titleboxYposDropDown[i % 5]);
					} else if (!applaunchprep || startMenu_cursorPosition != i){
						if (theme == 1) {
							glSprite(spawnedboxXpos-startMenu_titleboxXpos, titleboxYpos, GL_FLIP_NONE, tex().boxemptyImage());
						} else {
							glSprite(spawnedboxXpos-startMenu_titleboxXpos+movecloseXpos, titleboxYpos+titleboxYposDropDown[i % 5], GL_FLIP_NONE, &tex().boxfullImage()[1 & 63]);
						}
					}
					spawnedboxXpos += 64;
					iconXpos += 64;
				}
				if (theme == 0) glSprite(spawnedboxXpos+10-startMenu_titleboxXpos, 81, GL_FLIP_H, tex().braceImage());
			} else {
				for(int i = 0; i < 40; i++) {
					if (theme == 0) {
						moveIconClose(i);
					} else {
						movecloseXpos = 0;
					}
					if (i < spawnedtitleboxes) {
						if (isDirectory[i]) {
							if (theme == 1) glSprite(spawnedboxXpos-titleboxXpos[secondaryDevice]+movecloseXpos, titleboxYpos, GL_FLIP_NONE, tex().folderImage());
							else glSprite(spawnedboxXpos-titleboxXpos[secondaryDevice]+movecloseXpos, (titleboxYpos-3)+titleboxYposDropDown[i % 5], GL_FLIP_NONE, tex().folderImage());
						} else if (!applaunchprep || cursorPosition[secondaryDevice] != i){ // Only draw the icon if we're not launching the selcted app
							if (theme == 1) {
								glSprite(spawnedboxXpos-titleboxXpos[secondaryDevice], titleboxYpos, GL_FLIP_NONE, tex().boxfullImage());
							} else { 
								glSprite(spawnedboxXpos-titleboxXpos[secondaryDevice]+movecloseXpos, titleboxYpos+titleboxYposDropDown[i % 5], GL_FLIP_NONE, &tex().boxfullImage()[0 & 63]);
							}
							if (bnrRomType[i] == 3) drawIconNES(iconXpos-titleboxXpos[secondaryDevice]+movecloseXpos, (titleboxYpos+12)+titleboxYposDropDown[i % 5]);
							else if (bnrRomType[i] == 2) drawIconGBC(iconXpos-titleboxXpos[secondaryDevice]+movecloseXpos, (titleboxYpos+12)+titleboxYposDropDown[i % 5]);
							else if (bnrRomType[i] == 1) drawIconGB(iconXpos-titleboxXpos[secondaryDevice]+movecloseXpos, (titleboxYpos+12)+titleboxYposDropDown[i % 5]);
							
							else drawIcon(iconXpos-titleboxXpos[secondaryDevice]+movecloseXpos, (titleboxYpos+12)+titleboxYposDropDown[i % 5], i);
						}
					} else {
						// Empty box
						if (theme == 1) {
							glSprite(spawnedboxXpos-titleboxXpos[secondaryDevice], titleboxYpos+titleboxYposDropDown[i % 5], GL_FLIP_NONE, tex().boxemptyImage());
						} else {
							glSprite(spawnedboxXpos-titleboxXpos[secondaryDevice]+movecloseXpos, titleboxYpos+titleboxYposDropDown[i % 5], GL_FLIP_NONE, &tex().boxfullImage()[1 & 63]);
						}
					}
					spawnedboxXpos += 64;
					iconXpos += 64;
				}
				if (theme == 0) glSprite(spawnedboxXpos+10-titleboxXpos[secondaryDevice], 81, GL_FLIP_H, tex().braceImage());
			}
			if (applaunchprep && theme==0) {
				
				if(startMenu) {
					if (startMenu_cursorPosition == 0) {
						glSprite(96, 83-titleboxYmovepos, GL_FLIP_NONE, &tex().settingsImage()[1 & 63]);
					} else if (startMenu_cursorPosition == 1) {
						if (!flashcardFound()) {
							glSprite(96, 83-titleboxYmovepos, GL_FLIP_NONE, &tex().settingsImage()[0 & 63]);
						} else {
							glSprite(96, 84-titleboxYmovepos, GL_FLIP_NONE, tex().boxfullImage());
							drawIconGBA(112, 96-titleboxYmovepos);
						}
					} else if (startMenu_cursorPosition == 2) {
						glSprite(96, 84-titleboxYmovepos, GL_FLIP_NONE, tex().boxfullImage());
						drawIconGBA(112, 96-titleboxYmovepos);
					}
				} else {
					if (isDirectory[cursorPosition[secondaryDevice]]) {
						glSprite(96, 87-titleboxYmovepos, GL_FLIP_NONE, tex().folderImage());
					} else {
						glSprite(96, 84-titleboxYmovepos, GL_FLIP_NONE, tex().boxfullImage());
						if (bnrRomType[cursorPosition[secondaryDevice]] == 3) drawIconNES(112, 96-titleboxYmovepos);
						else if (bnrRomType[cursorPosition[secondaryDevice]] == 2) drawIconGBC(112, 96-titleboxYmovepos);
						else if (bnrRomType[cursorPosition[secondaryDevice]] == 1) drawIconGB(112, 96-titleboxYmovepos);
						else drawIcon(112, 96-titleboxYmovepos, cursorPosition[secondaryDevice]);
					}
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
					glColor(RGB15(31, 31, 31));
					if (!startMenu) {
						if (bnrWirelessIcon[cursorPosition[secondaryDevice]] > 0) glSprite(96, 92, GL_FLIP_NONE, &tex().wirelessIcons()[(bnrWirelessIcon[cursorPosition[secondaryDevice]]-1) & 31]);
					}
				} else if (!isScrolling) {
					if (showbubble && theme == 0 && needToPlayStopSound && waitForNeedToPlayStopSound == 0) {
						mmEffectEx(&snd_stop);
						waitForNeedToPlayStopSound = 1;
						needToPlayStopSound = false;
					}
					glSprite(96, 81, GL_FLIP_NONE, &tex().startbrdImage()[startBorderZoomAnimSeq[startBorderZoomAnimNum] & 79]);
					glSprite(96+32, 81, GL_FLIP_H, &tex().startbrdImage()[startBorderZoomAnimSeq[startBorderZoomAnimNum] & 79]);
					glColor(RGB15(31, 31, 31));
					if (!startMenu) {
						if (bnrWirelessIcon[cursorPosition[secondaryDevice]] > 0) glSprite(96, 81, GL_FLIP_NONE, &tex().wirelessIcons()[(bnrWirelessIcon[cursorPosition[secondaryDevice]]-1) & 31]);
					}
				}
			}

			// Refresh the background layer.
			bottomBgLoad(showbubble);
			if (showbubble) drawBubble(tex().bubbleImage());
			if (showSTARTborder && theme == 0 && !isScrolling) glSprite(96, 144, GL_FLIP_NONE, &tex().startImage()[setLanguage]);
			if (dbox_Ypos != -192) {
				// Draw the dialog box.
				drawDbox();
				if (!isDirectory[cursorPosition[secondaryDevice]]) drawIcon(24, dbox_Ypos+20, cursorPosition[secondaryDevice]);
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
			glColor(RGB15(31, 31, 31));
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
		for (int i = 0; i < 40; i++) {
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
		for (int y=114; y>=0; y--) {
			u16 buffer[128];
			fread(buffer, 2, 0x80, file);
			u16* src = buffer;
			for (int i=0; i<128; i++) {
				u16 val = *(src++);
				BG_GFX_SUB[(y+40)*256+(i+64)] = ((val>>10)&0x1f) | ((val)&(0x1f<<5)) | (val&0x1f)<<10 | BIT(15);
			}
		}
	}

	fclose(file);
}

void loadPhoto() {
	FILE* file = fopen("fat:/_nds/dsimenuplusplus/photo.bmp", "rb");
	if (!file) file = fopen("sd:/_nds/dsimenuplusplus/photo.bmp", "rb");
	if (!file) file = fopen("nitro:/graphics/photo_default.bmp", "rb");

	if (file) {
		// Start loading
		fseek(file, 0xe, SEEK_SET);
		u8 pixelStart = (u8)fgetc(file) + 0xe;
		fseek(file, pixelStart, SEEK_SET);
		for (int y=155; y>=0; y--) {
			u16 buffer[208];
			fread(buffer, 2, 0xD0, file);
			u16* src = buffer;
			for (int i=0; i<208; i++) {
				u16 val = *(src++);
				BG_GFX_SUB[(y+24)*256+(i+24)] = ((val>>10)&0x1f) | ((val)&(0x1f<<5)) | (val&0x1f)<<10 | BIT(15);
			}
		}
	}

	fclose(file);
}

// Load photo without overwriting shoulder button images
void loadPhotoPart() {
	FILE* file = fopen("fat:/_nds/dsimenuplusplus/photo.bmp", "rb");
	if (!file) file = fopen("sd:/_nds/dsimenuplusplus/photo.bmp", "rb");
	if (!file) file = fopen("nitro:/graphics/photo_default.bmp", "rb");

	if (file) {
		// Start loading
		fseek(file, 0xe, SEEK_SET);
		u8 pixelStart = (u8)fgetc(file) + 0xe;
		fseek(file, pixelStart, SEEK_SET);
		for (int y=155; y>=0; y--) {
			u16 buffer[208];
			fread(buffer, 2, 0xD0, file);
			if (y <= 147) {
				u16* src = buffer;
				for (int i=0; i<208; i++) {
					u16 val = *(src++);
					BG_GFX_SUB[(y+24)*256+(i+24)] = ((val>>10)&0x1f) | ((val)&(0x1f<<5)) | (val&0x1f)<<10 | BIT(15);
				}
			}
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
		for (int y=191; y>=0; y--) {
			u16 buffer[256];
			fread(buffer, 2, 0x100, file);
			u16* src = buffer;
			for (int i=0; i<256; i++) {
				u16 val = *(src++);
				if (val != 0xFC1F) {	// Do not render magneta pixel
					BG_GFX_SUB[y*256+i] = ((val>>10)&0x1f) | ((val)&(0x1f<<5)) | (val&0x1f)<<10 | BIT(15);
				}
			}
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
		for (int y=191; y>=32; y--) {
			u16 buffer[256];
			fread(buffer, 2, 0x100, file);
			if (y <= 167) {
				u16* src = buffer;
				for (int i=0; i<256; i++) {
					u16 val = *(src++);
					if (val != 0xFC1F) {	// Do not render magneta pixel
						BG_GFX_SUB[y*256+i] = ((val>>10)&0x1f) | ((val)&(0x1f<<5)) | (val&0x1f)<<10 | BIT(15);
					}
				}
			}
		}
	}

	fclose(file);
}

void loadShoulders() {
	FILE* file;

	// Draw L shoulder
	if (subtheme == 7) {
		if (showLshoulder) {
			file = fopen("nitro:/graphics/purple_Lshoulder.bmp", "rb");
		} else {
			file = fopen("nitro:/graphics/purple_Lshoulder_greyed.bmp", "rb");
		}
	} else if (subtheme == 6) {
		if (showLshoulder) {
			file = fopen("nitro:/graphics/pink_Lshoulder.bmp", "rb");
		} else {
			file = fopen("nitro:/graphics/pink_Lshoulder_greyed.bmp", "rb");
		}
	} else if (subtheme == 5) {
		if (showLshoulder) {
			file = fopen("nitro:/graphics/yellow_Lshoulder.bmp", "rb");
		} else {
			file = fopen("nitro:/graphics/yellow_Lshoulder_greyed.bmp", "rb");
		}
	} else if (subtheme == 4) {
		if (showLshoulder) {
			file = fopen("nitro:/graphics/green_Lshoulder.bmp", "rb");
		} else {
			file = fopen("nitro:/graphics/green_Lshoulder_greyed.bmp", "rb");
		}
	} else if (subtheme == 3) {
		if (showLshoulder) {
			file = fopen("nitro:/graphics/blue_Lshoulder.bmp", "rb");
		} else {
			file = fopen("nitro:/graphics/blue_Lshoulder_greyed.bmp", "rb");
		}
	} else if (subtheme == 2) {
		if (showLshoulder) {
			file = fopen("nitro:/graphics/red_Lshoulder.bmp", "rb");
		} else {
			file = fopen("nitro:/graphics/red_Lshoulder_greyed.bmp", "rb");
		}
	} else if (theme == 1 || subtheme == 1) {
		if (showLshoulder) {
			file = fopen("nitro:/graphics/Lshoulder.bmp", "rb");
		} else {
			file = fopen("nitro:/graphics/Lshoulder_greyed.bmp", "rb");
		}
	} else {
		if (showLshoulder) {
			file = fopen("nitro:/graphics/dark_Lshoulder.bmp", "rb");
		} else {
			file = fopen("nitro:/graphics/dark_Lshoulder_greyed.bmp", "rb");
		}
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
	if (subtheme == 7) {
		if (showRshoulder) {
			file = fopen("nitro:/graphics/purple_Rshoulder.bmp", "rb");
		} else {
			file = fopen("nitro:/graphics/purple_Rshoulder_greyed.bmp", "rb");
		}
	} else if (subtheme == 6) {
		if (showRshoulder) {
			file = fopen("nitro:/graphics/pink_Rshoulder.bmp", "rb");
		} else {
			file = fopen("nitro:/graphics/pink_Rshoulder_greyed.bmp", "rb");
		}
	} else if (subtheme == 5) {
		if (showRshoulder) {
			file = fopen("nitro:/graphics/yellow_Rshoulder.bmp", "rb");
		} else {
			file = fopen("nitro:/graphics/yellow_Rshoulder_greyed.bmp", "rb");
		}
	} else if (subtheme == 4) {
		if (showRshoulder) {
			file = fopen("nitro:/graphics/green_Rshoulder.bmp", "rb");
		} else {
			file = fopen("nitro:/graphics/green_Rshoulder_greyed.bmp", "rb");
		}
	} else if (subtheme == 3) {
		if (showRshoulder) {
			file = fopen("nitro:/graphics/blue_Rshoulder.bmp", "rb");
		} else {
			file = fopen("nitro:/graphics/blue_Rshoulder_greyed.bmp", "rb");
		}
	} else if (subtheme == 2) {
		if (showRshoulder) {
			file = fopen("nitro:/graphics/red_Rshoulder.bmp", "rb");
		} else {
			file = fopen("nitro:/graphics/red_Rshoulder_greyed.bmp", "rb");
		}
	} else if (theme == 1 || subtheme == 1) {
		if (showRshoulder) {
			file = fopen("nitro:/graphics/Rshoulder.bmp", "rb");
		} else {
			file = fopen("nitro:/graphics/Rshoulder_greyed.bmp", "rb");
		}
	} else {
		if (showRshoulder) {
			file = fopen("nitro:/graphics/dark_Rshoulder.bmp", "rb");
		} else {
			file = fopen("nitro:/graphics/dark_Rshoulder_greyed.bmp", "rb");
		}
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
	if (theme == 1) {
		loadBMP("nitro:/graphics/3ds_top.bmp");
	} else if (subtheme == 7) {
		loadBMP("nitro:/graphics/purple_top.bmp");
	} else if (subtheme == 6) {
		loadBMP("nitro:/graphics/pink_top.bmp");
	} else if (subtheme == 5) {
		loadBMP("nitro:/graphics/yellow_top.bmp");
	} else if (subtheme == 4) {
		loadBMP("nitro:/graphics/green_top.bmp");
	} else if (subtheme == 3) {
		loadBMP("nitro:/graphics/blue_top.bmp");
	} else if (subtheme == 2) {
		loadBMP("nitro:/graphics/red_top.bmp");
	} else if (subtheme == 1) {
		loadBMP("nitro:/graphics/org_top.bmp");
	} else {
		loadBMP("nitro:/graphics/top.bmp");
	}

	// Load username
	char fontPath[64];
	FILE* file;
	int x = 24; 

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

				for (int i=0; i < top_font_texcoords[2+(4*charIndex)]; i++) {
					u16 val = *(src++);
					u16 bg = BG_GFX_SUB[(y+1)*256+(i+x)]; //grab the background pixel
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
						BG_GFX_SUB[(y+1)*256+(i+x)] = ((val>>10)&0x1f) | ((val)&(0x1f<<5)) | (val&0x1f)<<10 | BIT(15);
					}
				}
			}
			x += top_font_texcoords[2+(4*charIndex)];
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
	startMenu_titleboxXpos = startMenu_cursorPosition*64;
	startMenu_titlewindowXpos = startMenu_cursorPosition*5;
	
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

	if (theme < 1) loadPhoto();
	topBgLoad();

	// Initialize the bottom background
	bottomBg = bgInit(2, BgType_ExRotation, BgSize_ER_256x256, 0,1);
	
	bottomBgLoad(false, true);
	swiIntrWait(0, 1);
	tex().loadDSiDarkTheme();

	if (theme == 1) {
		tex().load3DSTheme();
		titleboxYpos = 96;
		bubbleYpos += 18;
		bubbleXpos += 3;
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
	}

	// progressTexID = glLoadTileSet(tex().progressImage(), // pointer to glImage array
	// 						16, // sprite width
	// 						16, // sprite height
	// 						16, // bitmap width
	// 						128, // bitmap height
	// 						GL_RGB16, // texture type for glTexImage2D() in videoGL.h
	// 						TEXTURE_SIZE_16, // sizeX for glTexImage2D() in videoGL.h
	// 						TEXTURE_SIZE_128, // sizeY for glTexImage2D() in videoGL.h
	// 						TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
	// 						9, // Length of the palette to use (9 colors)
	// 						(u16*) progressPal, // Load our 16 color tiles palette
	// 						(u8*) progressBitmap // image data generated by GRIT
	// 						);

	// dialogboxTexID = glLoadTileSet(tex().dialogboxImage(), // pointer to glImage array
	// 						16, // sprite width
	// 						16, // sprite height
	// 						256, // bitmap width
	// 						192, // bitmap height
	// 						GL_RGB16, // texture type for glTexImage2D() in videoGL.h
	// 						TEXTURE_SIZE_256, // sizeX for glTexImage2D() in videoGL.h
	// 						TEXTURE_SIZE_256, // sizeY for glTexImage2D() in videoGL.h
	// 						TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
	// 						4, // Length of the palette to use (16 colors)
	// 						(u16*) dialogboxPal, // Load our 16 color tiles palette
	// 						(u8*) dialogboxBitmap // image data generated by GRIT
	// 						);
	

	// 	bubbleTexID = glLoadTileSet(tex().bubbleImage(), // pointer to glImage array
	// 							7, // sprite width
	// 							7, // sprite height
	// 							8, // bitmap width
	// 							8, // bitmap height
	// 							GL_RGB16, // texture type for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_8, // sizeX for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_8, // sizeY for glTexImage2D() in videoGL.h
	// 							TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
	// 							12, // Length of the palette to use (16 colors)
	// 							(u16*) _3ds_bubblePal, // Load our 16 color tiles palette
	// 							(u8*) _3ds_bubbleBitmap // image data generated by GRIT
	// 							);
	// } else if (subtheme == 7) {

	// 	bubbleTexID = glLoadTileSet(tex().bubbleImage(), // pointer to glImage array
	// 							11, // sprite width
	// 							8, // sprite height
	// 							16, // bitmap width
	// 							8, // bitmap height
	// 							GL_RGB16, // texture type for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_16, // sizeX for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_8, // sizeY for glTexImage2D() in videoGL.h
	// 							TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
	// 							12, // Length of the palette to use (16 colors)
	// 							(u16*) purple_bubblePal, // Load our 16 color tiles palette
	// 							(u8*) purple_bubbleBitmap // image data generated by GRIT
	// 							);
	// } else if (subtheme == 6) {

	// 	bubbleTexID = glLoadTileSet(tex().bubbleImage(), // pointer to glImage array
	// 							11, // sprite width
	// 							8, // sprite height
	// 							16, // bitmap width
	// 							8, // bitmap height
	// 							GL_RGB16, // texture type for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_16, // sizeX for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_8, // sizeY for glTexImage2D() in videoGL.h
	// 							TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
	// 							12, // Length of the palette to use (16 colors)
	// 							(u16*) pink_bubblePal, // Load our 16 color tiles palette
	// 							(u8*) pink_bubbleBitmap // image data generated by GRIT
	// 							);
	// } else if (subtheme == 5) {

	// 	bubbleTexID = glLoadTileSet(tex().bubbleImage(), // pointer to glImage array
	// 							11, // sprite width
	// 							8, // sprite height
	// 							16, // bitmap width
	// 							8, // bitmap height
	// 							GL_RGB16, // texture type for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_16, // sizeX for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_8, // sizeY for glTexImage2D() in videoGL.h
	// 							TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
	// 							12, // Length of the palette to use (16 colors)
	// 							(u16*) yellow_bubblePal, // Load our 16 color tiles palette
	// 							(u8*) yellow_bubbleBitmap // image data generated by GRIT
	// 							);
	// } else if (subtheme == 4) {

	// 	bubbleTexID = glLoadTileSet(tex().bubbleImage(), // pointer to glImage array
	// 							11, // sprite width
	// 							8, // sprite height
	// 							16, // bitmap width
	// 							8, // bitmap height
	// 							GL_RGB16, // texture type for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_16, // sizeX for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_8, // sizeY for glTexImage2D() in videoGL.h
	// 							TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
	// 							12, // Length of the palette to use (16 colors)
	// 							(u16*) green_bubblePal, // Load our 16 color tiles palette
	// 							(u8*) green_bubbleBitmap // image data generated by GRIT
	// 							);
	// } else if (subtheme == 3) {

	// 	bubbleTexID = glLoadTileSet(tex().bubbleImage(), // pointer to glImage array
	// 							11, // sprite width
	// 							8, // sprite height
	// 							16, // bitmap width
	// 							8, // bitmap height
	// 							GL_RGB16, // texture type for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_16, // sizeX for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_8, // sizeY for glTexImage2D() in videoGL.h
	// 							TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
	// 							12, // Length of the palette to use (16 colors)
	// 							(u16*) blue_bubblePal, // Load our 16 color tiles palette
	// 							(u8*) blue_bubbleBitmap // image data generated by GRIT
	// 							);
	// } else if (subtheme == 2) {

	// 	bubbleTexID = glLoadTileSet(tex().bubbleImage(), // pointer to glImage array
	// 							11, // sprite width
	// 							8, // sprite height
	// 							16, // bitmap width
	// 							8, // bitmap height
	// 							GL_RGB16, // texture type for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_16, // sizeX for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_8, // sizeY for glTexImage2D() in videoGL.h
	// 							TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
	// 							12, // Length of the palette to use (16 colors)
	// 							(u16*) red_bubblePal, // Load our 16 color tiles palette
	// 							(u8*) red_bubbleBitmap // image data generated by GRIT
	// 							);
	// } else if (subtheme == 1) {

	// 	bubbleTexID = glLoadTileSet(tex().bubbleImage(), // pointer to glImage array
	// 							11, // sprite width
	// 							8, // sprite height
	// 							16, // bitmap width
	// 							8, // bitmap height
	// 							GL_RGB16, // texture type for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_16, // sizeX for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_8, // sizeY for glTexImage2D() in videoGL.h
	// 							TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
	// 							12, // Length of the palette to use (16 colors)
	// 							(u16*) org_bubblePal, // Load our 16 color tiles palette
	// 							(u8*) org_bubbleBitmap // image data generated by GRIT
	// 							);
	// } else {

	// 	bubbleTexID = glLoadTileSet(tex().bubbleImage(), // pointer to glImage array
	// 							11, // sprite width
	// 							8, // sprite height
	// 							16, // bitmap width
	// 							8, // bitmap height
	// 							GL_RGB16, // texture type for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_16, // sizeX for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_8, // sizeY for glTexImage2D() in videoGL.h
	// 							TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
	// 							12, // Length of the palette to use (16 colors)
	// 							(u16*) bubblePal, // Load our 16 color tiles palette
	// 							(u8*) bubbleBitmap // image data generated by GRIT
	// 							);
	// }

	// bipsTexID = glLoadTileSet(tex().bipsImage(), // pointer to glImage array
	// 						8, // sprite width
	// 						8, // sprite height
	// 						8, // bitmap width
	// 						32, // bitmap height
	// 						GL_RGB16, // texture type for glTexImage2D() in videoGL.h
	// 						TEXTURE_SIZE_8, // sizeX for glTexImage2D() in videoGL.h
	// 						TEXTURE_SIZE_32, // sizeY for glTexImage2D() in videoGL.h
	// 						TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
	// 						16, // Length of the palette to use (16 colors)
	// 						(u16*) bipsPal, // Load our 16 color tiles palette
	// 						(u8*) bipsBitmap // image data generated by GRIT
	// 						);

	// if (subtheme == 7) {
	// 	scrollwindowTexID = glLoadTileSet(tex().scrollwindowImage(), // pointer to glImage array
	// 							32, // sprite width
	// 							32, // sprite height
	// 							32, // bitmap width
	// 							32, // bitmap height
	// 							GL_RGB16, // texture type for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_32, // sizeX for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_32, // sizeY for glTexImage2D() in videoGL.h
	// 							TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
	// 							16, // Length of the palette to use (16 colors)
	// 							(u16*) purple_scroll_windowPal, // Load our 16 color tiles palette
	// 							(u8*) purple_scroll_windowBitmap // image data generated by GRIT
	// 							);
	// } else if (subtheme == 6) {
	// 	scrollwindowTexID = glLoadTileSet(tex().scrollwindowImage(), // pointer to glImage array
	// 							32, // sprite width
	// 							32, // sprite height
	// 							32, // bitmap width
	// 							32, // bitmap height
	// 							GL_RGB16, // texture type for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_32, // sizeX for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_32, // sizeY for glTexImage2D() in videoGL.h
	// 							TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
	// 							16, // Length of the palette to use (16 colors)
	// 							(u16*) pink_scroll_windowPal, // Load our 16 color tiles palette
	// 							(u8*) pink_scroll_windowBitmap // image data generated by GRIT
	// 							);
	// } else if (subtheme == 5) {
	// 	scrollwindowTexID = glLoadTileSet(tex().scrollwindowImage(), // pointer to glImage array
	// 							32, // sprite width
	// 							32, // sprite height
	// 							32, // bitmap width
	// 							32, // bitmap height
	// 							GL_RGB16, // texture type for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_32, // sizeX for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_32, // sizeY for glTexImage2D() in videoGL.h
	// 							TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
	// 							16, // Length of the palette to use (16 colors)
	// 							(u16*) yellow_scroll_windowPal, // Load our 16 color tiles palette
	// 							(u8*) yellow_scroll_windowBitmap // image data generated by GRIT
	// 							);
	// } else if (subtheme == 4) {
	// 	scrollwindowTexID = glLoadTileSet(tex().scrollwindowImage(), // pointer to glImage array
	// 							32, // sprite width
	// 							32, // sprite height
	// 							32, // bitmap width
	// 							32, // bitmap height
	// 							GL_RGB16, // texture type for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_32, // sizeX for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_32, // sizeY for glTexImage2D() in videoGL.h
	// 							TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
	// 							16, // Length of the palette to use (16 colors)
	// 							(u16*) green_scroll_windowPal, // Load our 16 color tiles palette
	// 							(u8*) green_scroll_windowBitmap // image data generated by GRIT
	// 							);
	// } else if (subtheme == 3) {
	// 	scrollwindowTexID = glLoadTileSet(tex().scrollwindowImage(), // pointer to glImage array
	// 							32, // sprite width
	// 							32, // sprite height
	// 							32, // bitmap width
	// 							32, // bitmap height
	// 							GL_RGB16, // texture type for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_32, // sizeX for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_32, // sizeY for glTexImage2D() in videoGL.h
	// 							TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
	// 							16, // Length of the palette to use (16 colors)
	// 							(u16*) blue_scroll_windowPal, // Load our 16 color tiles palette
	// 							(u8*) blue_scroll_windowBitmap // image data generated by GRIT
	// 							);
	// } else if (subtheme == 2) {
	// 	scrollwindowTexID = glLoadTileSet(tex().scrollwindowImage(), // pointer to glImage array
	// 							32, // sprite width
	// 							32, // sprite height
	// 							32, // bitmap width
	// 							32, // bitmap height
	// 							GL_RGB16, // texture type for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_32, // sizeX for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_32, // sizeY for glTexImage2D() in videoGL.h
	// 							TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
	// 							16, // Length of the palette to use (16 colors)
	// 							(u16*) red_scroll_windowPal, // Load our 16 color tiles palette
	// 							(u8*) red_scroll_windowBitmap // image data generated by GRIT
	// 							);
	// } else if (subtheme == 1) {
	// 	scrollwindowTexID = glLoadTileSet(tex().scrollwindowImage(), // pointer to glImage array
	// 							32, // sprite width
	// 							32, // sprite height
	// 							32, // bitmap width
	// 							32, // bitmap height
	// 							GL_RGB16, // texture type for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_32, // sizeX for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_32, // sizeY for glTexImage2D() in videoGL.h
	// 							TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
	// 							16, // Length of the palette to use (16 colors)
	// 							(u16*) org_scroll_windowPal, // Load our 16 color tiles palette
	// 							(u8*) org_scroll_windowBitmap // image data generated by GRIT
	// 							);
	// } else {
	// 	scrollwindowTexID = glLoadTileSet(tex().scrollwindowImage(), // pointer to glImage array
	// 							32, // sprite width
	// 							32, // sprite height
	// 							32, // bitmap width
	// 							32, // bitmap height
	// 							GL_RGB16, // texture type for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_32, // sizeX for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_32, // sizeY for glTexImage2D() in videoGL.h
	// 							TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
	// 							16, // Length of the palette to use (16 colors)
	// 							(u16*) scroll_windowPal, // Load our 16 color tiles palette
	// 							(u8*) scroll_windowBitmap // image data generated by GRIT
	// 							);
	// }

	// buttonarrowTexID = glLoadTileSet(tex().buttonarrowImage(), // pointer to glImage array
	// 						32, // sprite width
	// 						32, // sprite height
	// 						32, // bitmap width
	// 						64, // bitmap height
	// 						GL_RGB16, // texture type for glTexImage2D() in videoGL.h
	// 						TEXTURE_SIZE_32, // sizeX for glTexImage2D() in videoGL.h
	// 						TEXTURE_SIZE_64, // sizeY for glTexImage2D() in videoGL.h
	// 						TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
	// 						16, // Length of the palette to use (16 colors)
	// 						(u16*) button_arrowPals+((PersonalData->theme)*16), // Load our 16 color tiles palette
	// 						(u8*) button_arrowBitmap // image data generated by GRIT
	// 						);

	// launchdotTexID = glLoadTileSet(tex().launchdotImage(), // pointer to glImage array
	// 						16, // sprite width
	// 						16, // sprite height
	// 						16, // bitmap width
	// 						96, // bitmap height
	// 						GL_RGB16, // texture type for glTexImage2D() in videoGL.h
	// 						TEXTURE_SIZE_16, // sizeX for glTexImage2D() in videoGL.h
	// 						TEXTURE_SIZE_128, // sizeY for glTexImage2D() in videoGL.h
	// 						TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
	// 						16, // Length of the palette to use (16 colors)
	// 						(u16*) button_arrowPals+((PersonalData->theme)*16), // Load our 16 color tiles palette
	// 						(u8*) launch_dotBitmap // image data generated by GRIT
	// 						);

	// startTexID = glLoadTileSet(tex().startImage(), // pointer to glImage array
	// 						64, // sprite width
	// 						16, // sprite height
	// 						64, // bitmap width
	// 						128, // bitmap height
	// 						GL_RGB16, // texture type for glTexImage2D() in videoGL.h
	// 						TEXTURE_SIZE_64, // sizeX for glTexImage2D() in videoGL.h
	// 						TEXTURE_SIZE_128, // sizeY for glTexImage2D() in videoGL.h
	// 						TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
	// 						4, // Length of the palette to use (4 colors)
	// 						(u16*) start_textPals+((PersonalData->theme)*16), // Load our 16 color tiles palette
	// 						(u8*) start_textBitmap // image data generated by GRIT
	// 						);

	// if (theme == 1) {
	// 	startbrdTexID = glLoadTileSet(tex().startbrdImage(), // pointer to glImage array
	// 							32, // sprite width
	// 							64, // sprite height
	// 							32, // bitmap width
	// 							192, // bitmap height
	// 							GL_RGB16, // texture type for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_32, // sizeX for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_256, // sizeY for glTexImage2D() in videoGL.h
	// 							TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
	// 							6, // Length of the palette to use (6 colors)
	// 							(u16*) _3ds_cursorPal, // Load our 16 color tiles palette
	// 							(u8*) _3ds_cursorBitmap // image data generated by GRIT
	// 							);
	// } else {
	// 	startbrdTexID = glLoadTileSet(startbrdImage, // pointer to glImage array
	// 							32, // sprite width
	// 							80, // sprite height
	// 							32, // bitmap width
	// 							256, // bitmap height
	// 							GL_RGB16, // texture type for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_32, // sizeX for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_256, // sizeY for glTexImage2D() in videoGL.h
	// 							TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
	// 							16, // Length of the palette to use (16 colors)
	// 							(u16*) start_borderPals+((PersonalData->theme)*16), // Load our 16 color tiles palette
	// 							(u8*) start_borderBitmap // image data generated by GRIT
	// 							);
	// }

	// if (theme == 1) {
	// 	settingsTexID = glLoadTileSet(tex().settingsImage(), // pointer to glImage array
	// 							64, // sprite width
	// 							64, // sprite height
	// 							64, // bitmap width
	// 							128, // bitmap height
	// 							GL_RGB16, // texture type for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_64, // sizeX for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_128, // sizeY for glTexImage2D() in videoGL.h
	// 							TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
	// 							16, // Length of the palette to use (16 colors)
	// 							(u16*) org_icon_settingsPal, // Load our 16 color tiles palette
	// 							(u8*) org_icon_settingsBitmap // image data generated by GRIT
	// 							);

	// 	boxfullTexID = glLoadTileSet(boxfullImage, // pointer to glImage array
	// 							64, // sprite width
	// 							64, // sprite height
	// 							64, // bitmap width
	// 							64, // bitmap height
	// 							GL_RGB16, // texture type for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_64, // sizeX for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_64, // sizeY for glTexImage2D() in videoGL.h
	// 							TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
	// 							16, // Length of the palette to use (16 colors)
	// 							(u16*) _3ds_box_fullPal, // Load our 16 color tiles palette
	// 							(u8*) _3ds_box_fullBitmap // image data generated by GRIT
	// 							);

	// 	boxemptyTexID = glLoadTileSet(boxemptyImage, // pointer to glImage array
	// 							64, // sprite width
	// 							64, // sprite height
	// 							64, // bitmap width
	// 							64, // bitmap height
	// 							GL_RGB16, // texture type for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_64, // sizeX for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_64, // sizeY for glTexImage2D() in videoGL.h
	// 							TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
	// 							16, // Length of the palette to use (16 colors)
	// 							(u16*) _3ds_box_emptyPal, // Load our 16 color tiles palette
	// 							(u8*) _3ds_box_emptyBitmap // image data generated by GRIT
	// 							);

	// 	folderTexID = glLoadTileSet(tex().folderImage(), // pointer to glImage array
	// 							64, // sprite width
	// 							64, // sprite height
	// 							64, // bitmap width
	// 							64, // bitmap height
	// 							GL_RGB16, // texture type for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_64, // sizeX for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_64, // sizeY for glTexImage2D() in videoGL.h
	// 							TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
	// 							16, // Length of the palette to use (16 colors)
	// 							(u16*) _3ds_folderPal, // Load our 16 color tiles palette
	// 							(u8*) _3ds_folderBitmap // image data generated by GRIT
	// 							);
	// } else if (subtheme == 7) {
	// 	settingsTexID = glLoadTileSet(tex().settingsImage(), // pointer to glImage array
	// 							64, // sprite width
	// 							64, // sprite height
	// 							64, // bitmap width
	// 							128, // bitmap height
	// 							GL_RGB16, // texture type for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_64, // sizeX for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_128, // sizeY for glTexImage2D() in videoGL.h
	// 							TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
	// 							16, // Length of the palette to use (16 colors)
	// 							(u16*) org_icon_settingsPal, // Load our 16 color tiles palette
	// 							(u8*) org_icon_settingsBitmap // image data generated by GRIT
	// 							);

	// 	braceTexID = glLoadTileSet(tex().braceImage(), // pointer to glImage array
	// 							16, // sprite width
	// 							128, // sprite height
	// 							16, // bitmap width
	// 							128, // bitmap height
	// 							GL_RGB16, // texture type for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_16, // sizeX for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_128, // sizeY for glTexImage2D() in videoGL.h
	// 							TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
	// 							4, // Length of the palette to use (16 colors)
	// 							(u16*) purple_bracePal, // Load our 16 color tiles palette
	// 							(u8*) purple_braceBitmap // image data generated by GRIT
	// 							);

	// 	boxfullTexID = glLoadTileSet(boxfullImage, // pointer to glImage array
	// 							64, // sprite width
	// 							64, // sprite height
	// 							64, // bitmap width
	// 							128, // bitmap height
	// 							GL_RGB16, // texture type for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_64, // sizeX for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_128, // sizeY for glTexImage2D() in videoGL.h
	// 							TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
	// 							16, // Length of the palette to use (16 colors)
	// 							(u16*) purple_boxPal, // Load our 16 color tiles palette
	// 							(u8*) purple_boxBitmap // image data generated by GRIT
	// 							);

	// 	folderTexID = glLoadTileSet(tex().folderImage(), // pointer to glImage array
	// 							64, // sprite width
	// 							64, // sprite height
	// 							64, // bitmap width
	// 							64, // bitmap height
	// 							GL_RGB16, // texture type for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_64, // sizeX for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_64, // sizeY for glTexImage2D() in videoGL.h
	// 							TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
	// 							16, // Length of the palette to use (16 colors)
	// 							(u16*) org_folderPal, // Load our 16 color tiles palette
	// 							(u8*) org_folderBitmap // image data generated by GRIT
	// 							);
	// } else if (subtheme == 6) {
	// 	settingsTexID = glLoadTileSet(tex().settingsImage(), // pointer to glImage array
	// 							64, // sprite width
	// 							64, // sprite height
	// 							64, // bitmap width
	// 							128, // bitmap height
	// 							GL_RGB16, // texture type for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_64, // sizeX for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_128, // sizeY for glTexImage2D() in videoGL.h
	// 							TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
	// 							16, // Length of the palette to use (16 colors)
	// 							(u16*) org_icon_settingsPal, // Load our 16 color tiles palette
	// 							(u8*) org_icon_settingsBitmap // image data generated by GRIT
	// 							);

	// 	braceTexID = glLoadTileSet(tex().braceImage(), // pointer to glImage array
	// 							16, // sprite width
	// 							128, // sprite height
	// 							16, // bitmap width
	// 							128, // bitmap height
	// 							GL_RGB16, // texture type for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_16, // sizeX for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_128, // sizeY for glTexImage2D() in videoGL.h
	// 							TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
	// 							4, // Length of the palette to use (16 colors)
	// 							(u16*) pink_bracePal, // Load our 16 color tiles palette
	// 							(u8*) pink_braceBitmap // image data generated by GRIT
	// 							);

	// 	boxfullTexID = glLoadTileSet(boxfullImage, // pointer to glImage array
	// 							64, // sprite width
	// 							64, // sprite height
	// 							64, // bitmap width
	// 							128, // bitmap height
	// 							GL_RGB16, // texture type for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_64, // sizeX for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_128, // sizeY for glTexImage2D() in videoGL.h
	// 							TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
	// 							16, // Length of the palette to use (16 colors)
	// 							(u16*) pink_boxPal, // Load our 16 color tiles palette
	// 							(u8*) pink_boxBitmap // image data generated by GRIT
	// 							);

	// 	folderTexID = glLoadTileSet(tex().folderImage(), // pointer to glImage array
	// 							64, // sprite width
	// 							64, // sprite height
	// 							64, // bitmap width
	// 							64, // bitmap height
	// 							GL_RGB16, // texture type for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_64, // sizeX for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_64, // sizeY for glTexImage2D() in videoGL.h
	// 							TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
	// 							16, // Length of the palette to use (16 colors)
	// 							(u16*) org_folderPal, // Load our 16 color tiles palette
	// 							(u8*) org_folderBitmap // image data generated by GRIT
	// 							);
	// } else if (subtheme == 5) {
	// 	settingsTexID = glLoadTileSet(tex().settingsImage(), // pointer to glImage array
	// 							64, // sprite width
	// 							64, // sprite height
	// 							64, // bitmap width
	// 							128, // bitmap height
	// 							GL_RGB16, // texture type for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_64, // sizeX for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_128, // sizeY for glTexImage2D() in videoGL.h
	// 							TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
	// 							16, // Length of the palette to use (16 colors)
	// 							(u16*) org_icon_settingsPal, // Load our 16 color tiles palette
	// 							(u8*) org_icon_settingsBitmap // image data generated by GRIT
	// 							);

	// 	braceTexID = glLoadTileSet(tex().braceImage(), // pointer to glImage array
	// 							16, // sprite width
	// 							128, // sprite height
	// 							16, // bitmap width
	// 							128, // bitmap height
	// 							GL_RGB16, // texture type for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_16, // sizeX for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_128, // sizeY for glTexImage2D() in videoGL.h
	// 							TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
	// 							4, // Length of the palette to use (16 colors)
	// 							(u16*) yellow_bracePal, // Load our 16 color tiles palette
	// 							(u8*) yellow_braceBitmap // image data generated by GRIT
	// 							);

	// 	boxfullTexID = glLoadTileSet(boxfullImage, // pointer to glImage array
	// 							64, // sprite width
	// 							64, // sprite height
	// 							64, // bitmap width
	// 							128, // bitmap height
	// 							GL_RGB16, // texture type for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_64, // sizeX for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_128, // sizeY for glTexImage2D() in videoGL.h
	// 							TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
	// 							16, // Length of the palette to use (16 colors)
	// 							(u16*) yellow_boxPal, // Load our 16 color tiles palette
	// 							(u8*) yellow_boxBitmap // image data generated by GRIT
	// 							);

	// 	folderTexID = glLoadTileSet(tex().folderImage(), // pointer to glImage array
	// 							64, // sprite width
	// 							64, // sprite height
	// 							64, // bitmap width
	// 							64, // bitmap height
	// 							GL_RGB16, // texture type for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_64, // sizeX for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_64, // sizeY for glTexImage2D() in videoGL.h
	// 							TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
	// 							16, // Length of the palette to use (16 colors)
	// 							(u16*) org_folderPal, // Load our 16 color tiles palette
	// 							(u8*) org_folderBitmap // image data generated by GRIT
	// 							);
	// } else if (subtheme == 4) {
	// 	settingsTexID = glLoadTileSet(tex().settingsImage(), // pointer to glImage array
	// 							64, // sprite width
	// 							64, // sprite height
	// 							64, // bitmap width
	// 							128, // bitmap height
	// 							GL_RGB16, // texture type for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_64, // sizeX for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_128, // sizeY for glTexImage2D() in videoGL.h
	// 							TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
	// 							16, // Length of the palette to use (16 colors)
	// 							(u16*) org_icon_settingsPal, // Load our 16 color tiles palette
	// 							(u8*) org_icon_settingsBitmap // image data generated by GRIT
	// 							);

	// 	braceTexID = glLoadTileSet(tex().braceImage(), // pointer to glImage array
	// 							16, // sprite width
	// 							128, // sprite height
	// 							16, // bitmap width
	// 							128, // bitmap height
	// 							GL_RGB16, // texture type for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_16, // sizeX for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_128, // sizeY for glTexImage2D() in videoGL.h
	// 							TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
	// 							4, // Length of the palette to use (16 colors)
	// 							(u16*) green_bracePal, // Load our 16 color tiles palette
	// 							(u8*) green_braceBitmap // image data generated by GRIT
	// 							);

	// 	boxfullTexID = glLoadTileSet(boxfullImage, // pointer to glImage array
	// 							64, // sprite width
	// 							64, // sprite height
	// 							64, // bitmap width
	// 							128, // bitmap height
	// 							GL_RGB16, // texture type for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_64, // sizeX for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_128, // sizeY for glTexImage2D() in videoGL.h
	// 							TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
	// 							16, // Length of the palette to use (16 colors)
	// 							(u16*) green_boxPal, // Load our 16 color tiles palette
	// 							(u8*) green_boxBitmap // image data generated by GRIT
	// 							);

	// 	folderTexID = glLoadTileSet(tex().folderImage(), // pointer to glImage array
	// 							64, // sprite width
	// 							64, // sprite height
	// 							64, // bitmap width
	// 							64, // bitmap height
	// 							GL_RGB16, // texture type for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_64, // sizeX for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_64, // sizeY for glTexImage2D() in videoGL.h
	// 							TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
	// 							16, // Length of the palette to use (16 colors)
	// 							(u16*) org_folderPal, // Load our 16 color tiles palette
	// 							(u8*) org_folderBitmap // image data generated by GRIT
	// 							);
	// } else if (subtheme == 3) {
	// 	settingsTexID = glLoadTileSet(tex().settingsImage(), // pointer to glImage array
	// 							64, // sprite width
	// 							64, // sprite height
	// 							64, // bitmap width
	// 							128, // bitmap height
	// 							GL_RGB16, // texture type for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_64, // sizeX for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_128, // sizeY for glTexImage2D() in videoGL.h
	// 							TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
	// 							16, // Length of the palette to use (16 colors)
	// 							(u16*) org_icon_settingsPal, // Load our 16 color tiles palette
	// 							(u8*) org_icon_settingsBitmap // image data generated by GRIT
	// 							);

	// 	braceTexID = glLoadTileSet(tex().braceImage(), // pointer to glImage array
	// 							16, // sprite width
	// 							128, // sprite height
	// 							16, // bitmap width
	// 							128, // bitmap height
	// 							GL_RGB16, // texture type for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_16, // sizeX for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_128, // sizeY for glTexImage2D() in videoGL.h
	// 							TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
	// 							4, // Length of the palette to use (16 colors)
	// 							(u16*) blue_bracePal, // Load our 16 color tiles palette
	// 							(u8*) blue_braceBitmap // image data generated by GRIT
	// 							);

	// 	boxfullTexID = glLoadTileSet(boxfullImage, // pointer to glImage array
	// 							64, // sprite width
	// 							64, // sprite height
	// 							64, // bitmap width
	// 							128, // bitmap height
	// 							GL_RGB16, // texture type for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_64, // sizeX for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_128, // sizeY for glTexImage2D() in videoGL.h
	// 							TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
	// 							16, // Length of the palette to use (16 colors)
	// 							(u16*) blue_boxPal, // Load our 16 color tiles palette
	// 							(u8*) blue_boxBitmap // image data generated by GRIT
	// 							);

	// 	folderTexID = glLoadTileSet(tex().folderImage(), // pointer to glImage array
	// 							64, // sprite width
	// 							64, // sprite height
	// 							64, // bitmap width
	// 							64, // bitmap height
	// 							GL_RGB16, // texture type for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_64, // sizeX for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_64, // sizeY for glTexImage2D() in videoGL.h
	// 							TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
	// 							16, // Length of the palette to use (16 colors)
	// 							(u16*) org_folderPal, // Load our 16 color tiles palette
	// 							(u8*) org_folderBitmap // image data generated by GRIT
	// 							);
	// } else if (subtheme == 2) {
	// 	settingsTexID = glLoadTileSet(tex().settingsImage(), // pointer to glImage array
	// 							64, // sprite width
	// 							64, // sprite height
	// 							64, // bitmap width
	// 							128, // bitmap height
	// 							GL_RGB16, // texture type for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_64, // sizeX for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_128, // sizeY for glTexImage2D() in videoGL.h
	// 							TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
	// 							16, // Length of the palette to use (16 colors)
	// 							(u16*) org_icon_settingsPal, // Load our 16 color tiles palette
	// 							(u8*) org_icon_settingsBitmap // image data generated by GRIT
	// 							);

	// 	braceTexID = glLoadTileSet(tex().braceImage(), // pointer to glImage array
	// 							16, // sprite width
	// 							128, // sprite height
	// 							16, // bitmap width
	// 							128, // bitmap height
	// 							GL_RGB16, // texture type for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_16, // sizeX for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_128, // sizeY for glTexImage2D() in videoGL.h
	// 							TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
	// 							4, // Length of the palette to use (16 colors)
	// 							(u16*) red_bracePal, // Load our 16 color tiles palette
	// 							(u8*) red_braceBitmap // image data generated by GRIT
	// 							);

	// 	boxfullTexID = glLoadTileSet(boxfullImage, // pointer to glImage array
	// 							64, // sprite width
	// 							64, // sprite height
	// 							64, // bitmap width
	// 							128, // bitmap height
	// 							GL_RGB16, // texture type for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_64, // sizeX for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_128, // sizeY for glTexImage2D() in videoGL.h
	// 							TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
	// 							16, // Length of the palette to use (16 colors)
	// 							(u16*) red_boxPal, // Load our 16 color tiles palette
	// 							(u8*) red_boxBitmap // image data generated by GRIT
	// 							);

	// 	folderTexID = glLoadTileSet(tex().folderImage(), // pointer to glImage array
	// 							64, // sprite width
	// 							64, // sprite height
	// 							64, // bitmap width
	// 							64, // bitmap height
	// 							GL_RGB16, // texture type for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_64, // sizeX for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_64, // sizeY for glTexImage2D() in videoGL.h
	// 							TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
	// 							16, // Length of the palette to use (16 colors)
	// 							(u16*) org_folderPal, // Load our 16 color tiles palette
	// 							(u8*) org_folderBitmap // image data generated by GRIT
	// 							);
	// } else if (subtheme == 1) {
	// 	settingsTexID = glLoadTileSet(tex().settingsImage(), // pointer to glImage array
	// 							64, // sprite width
	// 							64, // sprite height
	// 							64, // bitmap width
	// 							128, // bitmap height
	// 							GL_RGB16, // texture type for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_64, // sizeX for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_128, // sizeY for glTexImage2D() in videoGL.h
	// 							TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
	// 							16, // Length of the palette to use (16 colors)
	// 							(u16*) org_icon_settingsPal, // Load our 16 color tiles palette
	// 							(u8*) org_icon_settingsBitmap // image data generated by GRIT
	// 							);

	// 	braceTexID = glLoadTileSet(tex().braceImage(), // pointer to glImage array
	// 							16, // sprite width
	// 							128, // sprite height
	// 							16, // bitmap width
	// 							128, // bitmap height
	// 							GL_RGB16, // texture type for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_16, // sizeX for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_128, // sizeY for glTexImage2D() in videoGL.h
	// 							TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
	// 							4, // Length of the palette to use (16 colors)
	// 							(u16*) org_bracePal, // Load our 16 color tiles palette
	// 							(u8*) org_braceBitmap // image data generated by GRIT
	// 							);

	// 	boxfullTexID = glLoadTileSet(boxfullImage, // pointer to glImage array
	// 							64, // sprite width
	// 							64, // sprite height
	// 							64, // bitmap width
	// 							128, // bitmap height
	// 							GL_RGB16, // texture type for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_64, // sizeX for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_128, // sizeY for glTexImage2D() in videoGL.h
	// 							TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
	// 							16, // Length of the palette to use (16 colors)
	// 							(u16*) org_boxPal, // Load our 16 color tiles palette
	// 							(u8*) org_boxBitmap // image data generated by GRIT
	// 							);

	// 	folderTexID = glLoadTileSet(tex().folderImage(), // pointer to glImage array
	// 							64, // sprite width
	// 							64, // sprite height
	// 							64, // bitmap width
	// 							64, // bitmap height
	// 							GL_RGB16, // texture type for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_64, // sizeX for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_64, // sizeY for glTexImage2D() in videoGL.h
	// 							TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
	// 							16, // Length of the palette to use (16 colors)
	// 							(u16*) org_folderPal, // Load our 16 color tiles palette
	// 							(u8*) org_folderBitmap // image data generated by GRIT
	// 							);
	// } else {
	// 	settingsTexID = glLoadTileSet(tex().settingsImage(), // pointer to glImage array
	// 							64, // sprite width
	// 							64, // sprite height
	// 							64, // bitmap width
	// 							128, // bitmap height
	// 							GL_RGB16, // texture type for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_64, // sizeX for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_128, // sizeY for glTexImage2D() in videoGL.h
	// 							TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
	// 							16, // Length of the palette to use (16 colors)
	// 							(u16*) icon_settingsPal, // Load our 16 color tiles palette
	// 							(u8*) icon_settingsBitmap // image data generated by GRIT
	// 							);

	// 	braceTexID = glLoadTileSet(tex().braceImage(), // pointer to glImage array
	// 							16, // sprite width
	// 							128, // sprite height
	// 							16, // bitmap width
	// 							128, // bitmap height
	// 							GL_RGB16, // texture type for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_16, // sizeX for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_128, // sizeY for glTexImage2D() in videoGL.h
	// 							TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
	// 							4, // Length of the palette to use (16 colors)
	// 							(u16*) bracePal, // Load our 16 color tiles palette
	// 							(u8*) braceBitmap // image data generated by GRIT
	// 							);

	// 	boxfullTexID = glLoadTileSet(boxfullImage, // pointer to glImage array
	// 							64, // sprite width
	// 							64, // sprite height
	// 							64, // bitmap width
	// 							128, // bitmap height
	// 							GL_RGB16, // texture type for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_64, // sizeX for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_128, // sizeY for glTexImage2D() in videoGL.h
	// 							TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
	// 							16, // Length of the palette to use (16 colors)
	// 							(u16*) boxPal, // Load our 16 color tiles palette
	// 							(u8*) boxBitmap // image data generated by GRIT
	// 							);

	// 	folderTexID = glLoadTileSet(tex().folderImage(), // pointer to glImage array
	// 							64, // sprite width
	// 							64, // sprite height
	// 							64, // bitmap width
	// 							64, // bitmap height
	// 							GL_RGB16, // texture type for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_64, // sizeX for glTexImage2D() in videoGL.h
	// 							TEXTURE_SIZE_64, // sizeY for glTexImage2D() in videoGL.h
	// 							TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
	// 							16, // Length of the palette to use (16 colors)
	// 							(u16*) folderPal, // Load our 16 color tiles palette
	// 							(u8*) folderBitmap // image data generated by GRIT
	// 							);
	// }
	
	// wirelessiconTexID = glLoadTileSet(wirelessIcons, // pointer to glImage array
	// 						32, // sprite width
	// 						32, // sprite height
	// 						32, // bitmap width
	// 						64, // bitmap height
	// 						GL_RGB16, // texture type for glTexImage2D() in videoGL.h
	// 						TEXTURE_SIZE_32, // sizeX for glTexImage2D() in videoGL.h
	// 						TEXTURE_SIZE_64, // sizeY for glTexImage2D() in videoGL.h
	// 						TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
	// 						7, // Length of the palette to use (7 colors)
	// 						(u16*) wirelessiconsPal, // Load our 16 color tiles palette
	// 						(u8*) wirelessiconsBitmap // image data generated by GRIT
	// 						);
	irqSet(IRQ_VBLANK, vBlankHandler);
	irqEnable(IRQ_VBLANK);
	//consoleDemoInit();


}
