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
#include <gl2d.h>
#include "bios_decompress_callback.h"
#include "FontGraphic.h"

// Graphic files
#include "top.h"
#include "org_top.h"
#include "_3ds_top.h"
#include "bottom.h"
#include "org_bottom.h"
#include "_3ds_bottom.h"
#include "dialogbox.h"
#include "shoulder.h"
#include "org_shoulder.h"
#include "nintendo_dsi_menu.h"
#include "org_nintendo_dsi_menu.h"
#include "bubble.h"
#include "org_bubble.h"
#include "_3ds_bubble.h"
#include "bubble_arrow.h"
#include "org_bubble_arrow.h"
#include "_3ds_bubble_arrow.h"
#include "bips.h"
#include "scroll_window.h"
#include "org_scroll_window.h"
#include "button_arrow.h"
#include "start_text.h"
#include "start_border.h"
#include "../include/startborderpal.h"
#include "_3ds_cursor.h"
#include "brace.h"
#include "org_brace.h"
#include "icon_settings.h"
#include "org_icon_settings.h"
#include "box.h"
#include "org_box.h"
#include "_3ds_box_full.h"
#include "_3ds_box_empty.h"
#include "folder.h"
#include "org_folder.h"
#include "_3ds_folder.h"
#include "wirelessicons.h"

#include "../iconTitle.h"
#include "graphics.h"
#include "fontHandler.h"
#include "../ndsheaderbanner.h"
#include "../perGameSettings.h"

#define CONSOLE_SCREEN_WIDTH 32
#define CONSOLE_SCREEN_HEIGHT 24

extern bool whiteScreen;
extern bool fadeType;
extern bool fadeSpeed;
int fadeDelay = 0;

extern bool music;
int musicTime = 0;

extern int colorRvalue;
extern int colorGvalue;
extern int colorBvalue;

extern bool showbubble;
extern bool showSTARTborder;

extern bool titleboxXmoveleft;
extern bool titleboxXmoveright;

extern bool applaunchprep;

int screenBrightness = 31;

int movetimer = 0;

int titleboxYmovepos = 0;

extern int spawnedtitleboxes;

extern bool startMenu;

extern bool flashcardUsed;

extern bool dsiWareList;
extern int theme;
extern int subtheme;
extern int cursorPosition;
extern int dsiWare_cursorPosition;
extern int startMenu_cursorPosition;
extern int pagenum;
extern int dsiWarePageNum;
int titleboxXpos;
int dsiWare_titleboxXpos;
int startMenu_titleboxXpos;
int titleboxYpos = 84;
int titlewindowXpos;
int dsiWare_titlewindowXpos;
int startMenu_titlewindowXpos;

int movecloseXpos = 0;

bool startBorderZoomOut = false;
int startBorderZoomAnimSeq[5] = {0, 1, 2, 1, 0};
int startBorderZoomAnimNum = 0;

bool showdialogbox = false;
float dbox_movespeed = 22;
float dbox_Ypos = -192;

int subBgTexID, mainBgTexID, shoulderTexID, ndsimenutextTexID, bubbleTexID, bubblearrowTexID, dialogboxTexID, wirelessiconTexID;
int bipsTexID, scrollwindowTexID, buttonarrowTexID, startTexID, startbrdTexID, settingsTexID, braceTexID, boxfullTexID, boxemptyTexID, folderTexID;

glImage subBgImage[(256 / 16) * (256 / 16)];
//glImage mainBgImage[(256 / 16) * (256 / 16)];
//glImage shoulderImage[(128 / 16) * (64 / 32)];
glImage ndsimenutextImage[(256 / 16) * (32 / 16)];
glImage bubbleImage[(256 / 16) * (128 / 16)];
glImage bubblearrowImage[(16 / 16) * (16 / 16)];
glImage dialogboxImage[(256 / 16) * (256 / 16)];
glImage bipsImage[(8 / 8) * (32 / 8)];
glImage scrollwindowImage[(32 / 16) * (32 / 16)];
glImage buttonarrowImage[(32 / 32) * (64 / 32)];
glImage startImage[(64 / 16) * (16 / 16)];
glImage startbrdImage[(32 / 32) * (256 / 80)];
glImage braceImage[(16 / 16) * (128 / 16)];
glImage settingsImage[(64 / 16) * (128 / 64)];
glImage boxfullImage[(64 / 16) * (128 / 64)];
glImage boxemptyImage[(64 / 16) * (64 / 16)];
glImage folderImage[(64 / 16) * (64 / 16)];
glImage wirelessIcons[(32 / 32) * (64 / 32)];

int bubbleYpos = 0;

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
			if (cursorPosition-2 == num) movecloseXpos = 1;
			else if (cursorPosition+2 == num) movecloseXpos = -2;
		} else if(movetimer == 2) {
			if (cursorPosition-2 == num) movecloseXpos = 1;
			else if (cursorPosition+2 == num) movecloseXpos = -2;
		} else if(movetimer == 3) {
			if (cursorPosition-2 == num) movecloseXpos = 2;
			else if (cursorPosition+2 == num) movecloseXpos = -3;
		} else if(movetimer == 4) {
			if (cursorPosition-2 == num) movecloseXpos = 2;
			else if (cursorPosition+2 == num) movecloseXpos = -3;
		} else if(movetimer == 5) {
			if (cursorPosition-2 == num) movecloseXpos = 3;
			else if (cursorPosition+2 == num) movecloseXpos = -4;
		} else if(movetimer == 6) {
			if (cursorPosition-2 == num) movecloseXpos = 4;
			else if (cursorPosition+2 == num) movecloseXpos = -5;
		} else if(movetimer == 7) {
			if (cursorPosition-2 == num) movecloseXpos = 5;
			else if (cursorPosition+2 == num) movecloseXpos = -6;
		} else if(movetimer == 8) {
			if (cursorPosition-2 == num) movecloseXpos = 6;
			else if (cursorPosition+2 == num) movecloseXpos = -7;
		}
	}
	if (titleboxXmoveright) {
		movecloseXpos = 0;
		if(movetimer == 1) {
			if (cursorPosition-2 == num) movecloseXpos = 2;
			else if (cursorPosition+2 == num) movecloseXpos = -1;
		} else if(movetimer == 2) {
			if (cursorPosition-2 == num) movecloseXpos = 2;
			else if (cursorPosition+2 == num) movecloseXpos = -1;
		} else if(movetimer == 3) {
			if (cursorPosition-2 == num) movecloseXpos = 3;
			else if (cursorPosition+2 == num) movecloseXpos = -2;
		} else if(movetimer == 4) {
			if (cursorPosition-2 == num) movecloseXpos = 3;
			else if (cursorPosition+2 == num) movecloseXpos = -2;
		} else if(movetimer == 5) {
			if (cursorPosition-2 == num) movecloseXpos = 4;
			else if (cursorPosition+2 == num) movecloseXpos = -3;
		} else if(movetimer == 6) {
			if (cursorPosition-2 == num) movecloseXpos = 5;
			else if (cursorPosition+2 == num) movecloseXpos = -4;
		} else if(movetimer == 7) {
			if (cursorPosition-2 == num) movecloseXpos = 6;
			else if (cursorPosition+2 == num) movecloseXpos = -5;
		} else if(movetimer == 8) {
			if (cursorPosition-2 == num) movecloseXpos = 7;
			else if (cursorPosition+2 == num) movecloseXpos = -6;
		}
	}
	if(!titleboxXmoveleft || !titleboxXmoveright) {
		if (cursorPosition-2 == num) movecloseXpos = 6;
		else if (cursorPosition+2 == num) movecloseXpos = -6;
		else movecloseXpos = 0;
	}
}

void dsiWare_moveIconClose(int num) {
	if (titleboxXmoveleft) {
		movecloseXpos = 0;
		if(movetimer == 1) {
			if (dsiWare_cursorPosition-2 == num) movecloseXpos = 1;
			else if (dsiWare_cursorPosition+2 == num) movecloseXpos = -2;
		} else if(movetimer == 2) {
			if (dsiWare_cursorPosition-2 == num) movecloseXpos = 1;
			else if (dsiWare_cursorPosition+2 == num) movecloseXpos = -2;
		} else if(movetimer == 3) {
			if (dsiWare_cursorPosition-2 == num) movecloseXpos = 2;
			else if (dsiWare_cursorPosition+2 == num) movecloseXpos = -3;
		} else if(movetimer == 4) {
			if (dsiWare_cursorPosition-2 == num) movecloseXpos = 2;
			else if (dsiWare_cursorPosition+2 == num) movecloseXpos = -3;
		} else if(movetimer == 5) {
			if (dsiWare_cursorPosition-2 == num) movecloseXpos = 3;
			else if (dsiWare_cursorPosition+2 == num) movecloseXpos = -4;
		} else if(movetimer == 6) {
			if (dsiWare_cursorPosition-2 == num) movecloseXpos = 4;
			else if (dsiWare_cursorPosition+2 == num) movecloseXpos = -5;
		} else if(movetimer == 7) {
			if (dsiWare_cursorPosition-2 == num) movecloseXpos = 5;
			else if (dsiWare_cursorPosition+2 == num) movecloseXpos = -6;
		} else if(movetimer == 8) {
			if (dsiWare_cursorPosition-2 == num) movecloseXpos = 6;
			else if (dsiWare_cursorPosition+2 == num) movecloseXpos = -7;
		}
	}
	if (titleboxXmoveright) {
		movecloseXpos = 0;
		if(movetimer == 1) {
			if (dsiWare_cursorPosition-2 == num) movecloseXpos = 2;
			else if (dsiWare_cursorPosition+2 == num) movecloseXpos = -1;
		} else if(movetimer == 2) {
			if (dsiWare_cursorPosition-2 == num) movecloseXpos = 2;
			else if (dsiWare_cursorPosition+2 == num) movecloseXpos = -1;
		} else if(movetimer == 3) {
			if (dsiWare_cursorPosition-2 == num) movecloseXpos = 3;
			else if (dsiWare_cursorPosition+2 == num) movecloseXpos = -2;
		} else if(movetimer == 4) {
			if (dsiWare_cursorPosition-2 == num) movecloseXpos = 3;
			else if (dsiWare_cursorPosition+2 == num) movecloseXpos = -2;
		} else if(movetimer == 5) {
			if (dsiWare_cursorPosition-2 == num) movecloseXpos = 4;
			else if (dsiWare_cursorPosition+2 == num) movecloseXpos = -3;
		} else if(movetimer == 6) {
			if (dsiWare_cursorPosition-2 == num) movecloseXpos = 5;
			else if (dsiWare_cursorPosition+2 == num) movecloseXpos = -4;
		} else if(movetimer == 7) {
			if (dsiWare_cursorPosition-2 == num) movecloseXpos = 6;
			else if (dsiWare_cursorPosition+2 == num) movecloseXpos = -5;
		} else if(movetimer == 8) {
			if (dsiWare_cursorPosition-2 == num) movecloseXpos = 7;
			else if (dsiWare_cursorPosition+2 == num) movecloseXpos = -6;
		}
	}
	if(!titleboxXmoveleft || !titleboxXmoveright) {
		if (dsiWare_cursorPosition-2 == num) movecloseXpos = 6;
		else if (dsiWare_cursorPosition+2 == num) movecloseXpos = -6;
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

	swiWaitForVBlank();

	oamUpdate(&oamSub);
}

void drawBG(glImage *images)
{
	for (int y = 0; y < 256 / 16; y++)
	{
		for (int x = 0; x < 256 / 16; x++)
		{
			int i = y * 16 + x;
			glSprite(x * 16, y * 16, GL_FLIP_NONE, &images[i & 255]);
		}
	}
}
void drawBubble(glImage *images)
{
	for (int y = 0; y < 128 / 16; y++)
	{
		for (int x = 0; x < 256 / 16; x++)
		{
			int i = y * 16 + x;
			glSprite(x * 16, bubbleYpos+y * 16, GL_FLIP_NONE, &images[i & 255]);
		}
	}
}
void drawDbox()
{
	for (int y = 0; y < 192 / 16; y++)
	{
		for (int x = 0; x < 256 / 16; x++)
		{
			int i = y * 16 + x;
			glSprite(x * 16, dbox_Ypos+y * 16, GL_FLIP_NONE, &dialogboxImage[i & 255]);
		}
	}
}

void vBlankHandler()
{
	if (music) {
		musicTime++;
		if (musicTime == 60*50) {	// Length of music file in seconds (60*ss)
			mmEffectEx(&mus_menu);
			musicTime = 0;
		}
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
		SetBrightness(0, screenBrightness);
		SetBrightness(1, screenBrightness);

		/*if (renderingTop)
		{
			glBoxFilledGradient(0, -64, 256, 112,
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
			glColor(RGB15(31, 31, 31));
		}
		else
		{*/

		if (showdialogbox) {
			if (dbox_movespeed <= 1) {
				if (dbox_Ypos >= 0) {
					dbox_movespeed = 0;
					dbox_Ypos = 0;
				} else
					dbox_movespeed = 1;
			} else {
				dbox_movespeed -= 1.25;
			}
			dbox_Ypos += dbox_movespeed;
		} else {
			if (dbox_Ypos <= -192 || dbox_Ypos >= 192) {
				dbox_movespeed = 22;
				dbox_Ypos = -192;
			} else {
				dbox_movespeed += 1;
				dbox_Ypos += dbox_movespeed;
			}
		}

			drawBG(subBgImage);
			if (showbubble) drawBubble(bubbleImage);
			else if (theme==0) glSprite(0, 32, GL_FLIP_NONE, ndsimenutextImage);
			
			if (titleboxXmoveleft) {
				if(startMenu) {
					if (movetimer == 8) {
						if (showbubble) mmEffectEx(&snd_stop);
						startBorderZoomOut = true;
						startMenu_titlewindowXpos -= 1;
						movetimer++;
					} else if (movetimer < 8) {
						startMenu_titleboxXpos -= 8;
						if(movetimer==0 || movetimer==2 || movetimer==4 || movetimer==6 ) startMenu_titlewindowXpos -= 1;
						movetimer++;
					} else {
						titleboxXmoveleft = false;
						movetimer = 0;
					}
				} else if (dsiWareList) {
					if (movetimer == 8) {
						if (showbubble) mmEffectEx(&snd_stop);
						startBorderZoomOut = true;
						dsiWare_titlewindowXpos -= 1;
						movetimer++;
					} else if (movetimer < 8) {
						dsiWare_titleboxXpos -= 8;
						if(movetimer==0 || movetimer==2 || movetimer==4 || movetimer==6 ) dsiWare_titlewindowXpos -= 1;
						movetimer++;
					} else {
						titleboxXmoveleft = false;
						movetimer = 0;
					}
				} else {
					if (movetimer == 8) {
						if (showbubble) mmEffectEx(&snd_stop);
						startBorderZoomOut = true;
						titlewindowXpos -= 1;
						movetimer++;
					} else if (movetimer < 8) {
						titleboxXpos -= 8;
						if(movetimer==0 || movetimer==2 || movetimer==4 || movetimer==6 ) titlewindowXpos -= 1;
						movetimer++;
					} else {
						titleboxXmoveleft = false;
						movetimer = 0;
					}
				}
			} else if (titleboxXmoveright) {
				if(startMenu) {
					if (movetimer == 8) {
						if (showbubble) mmEffectEx(&snd_stop);
						startBorderZoomOut = true;
						startMenu_titlewindowXpos += 1;
						movetimer++;
					} else if (movetimer < 8) {
						startMenu_titleboxXpos += 8;
						if(movetimer==0 || movetimer==2 || movetimer==4 || movetimer==6 ) startMenu_titlewindowXpos += 1;
						movetimer++;
					} else {
						titleboxXmoveright = false;
						movetimer = 0;
					}
				} else if (dsiWareList) {
					if (movetimer == 8) {
						if (showbubble) mmEffectEx(&snd_stop);
						startBorderZoomOut = true;
						dsiWare_titlewindowXpos += 1;
						movetimer++;
					} else if (movetimer < 8) {
						dsiWare_titleboxXpos += 8;
						if(movetimer==0 || movetimer==2 || movetimer==4 || movetimer==6 ) dsiWare_titlewindowXpos += 1;
						movetimer++;
					} else {
						titleboxXmoveright = false;
						movetimer = 0;
					}
				} else {
					if (movetimer == 8) {
						if (showbubble) mmEffectEx(&snd_stop);
						startBorderZoomOut = true;
						titlewindowXpos += 1;
						movetimer++;
					} else if (movetimer < 8) {
						titleboxXpos += 8;
						if(movetimer==0 || movetimer==2 || movetimer==4 || movetimer==6 ) titlewindowXpos += 1;
						movetimer++;
					} else {
						titleboxXmoveright = false;
						movetimer = 0;
					}
				}
			}

			if (theme==0) {
				glColor(RGB15(31, 31, 31));
				int bipXpos = 27;
				if(dsiWareList) {
					glSprite(16+dsiWare_titlewindowXpos, 171, GL_FLIP_NONE, scrollwindowImage);
					for(int i = 0; i < 40; i++) {
						if (i < spawnedtitleboxes) glSprite(bipXpos, 178, GL_FLIP_NONE, bipsImage);
						else glSprite(bipXpos, 178, GL_FLIP_NONE, &bipsImage[1 & 31]);
						bipXpos += 5;
					}
					glColor(RGB15(colorRvalue/3, colorGvalue/3, colorBvalue/3));
					glSprite(16+dsiWare_titlewindowXpos, 171, GL_FLIP_NONE, &buttonarrowImage[1]);
				} else if(startMenu) {
					glSprite(16+startMenu_titlewindowXpos, 171, GL_FLIP_NONE, scrollwindowImage);
					for(int i = 0; i < 40; i++) {
						if (i == 0) glSprite(bipXpos, 178, GL_FLIP_NONE, &bipsImage[2 & 31]);
						else if (i == 1) glSprite(bipXpos, 178, GL_FLIP_NONE, bipsImage);
						else glSprite(bipXpos, 178, GL_FLIP_NONE, &bipsImage[1 & 31]);
						bipXpos += 5;
					}
					glColor(RGB15(colorRvalue/3, colorGvalue/3, colorBvalue/3));
					glSprite(16+startMenu_titlewindowXpos, 171, GL_FLIP_NONE, &buttonarrowImage[1]);
				} else {
					glSprite(16+titlewindowXpos, 171, GL_FLIP_NONE, scrollwindowImage);
					for(int i = 0; i < 40; i++) {
						if (i < spawnedtitleboxes) glSprite(bipXpos, 178, GL_FLIP_NONE, bipsImage);
						else glSprite(bipXpos, 178, GL_FLIP_NONE, &bipsImage[1 & 31]);
						bipXpos += 5;
					}
					glColor(RGB15(colorRvalue/3, colorGvalue/3, colorBvalue/3));
					glSprite(16+titlewindowXpos, 171, GL_FLIP_NONE, &buttonarrowImage[1]);
				}
				glSprite(0, 171, GL_FLIP_NONE, &buttonarrowImage[0]);
				glSprite(224, 171, GL_FLIP_H, &buttonarrowImage[0]);
				glColor(RGB15(31, 31, 31));
				if (startMenu) {
					glSprite(72-startMenu_titleboxXpos, 80, GL_FLIP_NONE, braceImage);
				} else if (dsiWareList) {
					glSprite(72-dsiWare_titleboxXpos, 80, GL_FLIP_NONE, braceImage);
				} else {
					glSprite(72-titleboxXpos, 80, GL_FLIP_NONE, braceImage);
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
					if (i == 0) {
						glSprite(spawnedboxXpos-startMenu_titleboxXpos+movecloseXpos, titleboxYpos-1, GL_FLIP_NONE, &settingsImage[1 & 63]);
					} else if (i == 1) {
						if (!flashcardUsed) {
							glSprite(spawnedboxXpos-startMenu_titleboxXpos+movecloseXpos, titleboxYpos, GL_FLIP_NONE, &settingsImage[0 & 63]);
						} else {
							if (theme == 1) glSprite(spawnedboxXpos-startMenu_titleboxXpos+movecloseXpos, titleboxYpos, GL_FLIP_NONE, boxfullImage);
							else glSprite(spawnedboxXpos-startMenu_titleboxXpos+movecloseXpos, titleboxYpos, GL_FLIP_NONE, &boxfullImage[0 & 63]);
							drawIconGBA(iconXpos-startMenu_titleboxXpos+movecloseXpos, titleboxYpos+12);
						}
					} else if (i == 2 && !flashcardUsed) {
						if (theme == 1) glSprite(spawnedboxXpos-startMenu_titleboxXpos+movecloseXpos, titleboxYpos, GL_FLIP_NONE, boxfullImage);
						else glSprite(spawnedboxXpos-startMenu_titleboxXpos+movecloseXpos, titleboxYpos, GL_FLIP_NONE, &boxfullImage[0 & 63]);
						drawIconGBA(iconXpos-startMenu_titleboxXpos+movecloseXpos, titleboxYpos+12);
					} else {
						if (theme == 1) {
							glSprite(spawnedboxXpos-startMenu_titleboxXpos, titleboxYpos, GL_FLIP_NONE, boxemptyImage);
						} else {
							glSprite(spawnedboxXpos-startMenu_titleboxXpos+movecloseXpos, titleboxYpos, GL_FLIP_NONE, &boxfullImage[1 & 63]);
						}
					}
					spawnedboxXpos += 64;
					iconXpos += 64;
				}
				if (theme == 0) glSprite(spawnedboxXpos+10-startMenu_titleboxXpos, 80, GL_FLIP_H, braceImage);
			} else if(dsiWareList) {
				for(int i = 0; i < 40; i++) {
					if (theme == 0) {
						dsiWare_moveIconClose(i);
					} else {
						movecloseXpos = 0;
					}
					if (i < spawnedtitleboxes) {
						if (isDirectory[i]) {
							if (theme == 1) glSprite(spawnedboxXpos-dsiWare_titleboxXpos+movecloseXpos, titleboxYpos, GL_FLIP_NONE, folderImage);
							else glSprite(spawnedboxXpos-dsiWare_titleboxXpos+movecloseXpos, titleboxYpos-3, GL_FLIP_NONE, folderImage);
						} else {
							if (theme == 1) glSprite(spawnedboxXpos-dsiWare_titleboxXpos, titleboxYpos, GL_FLIP_NONE, boxfullImage);
							else glSprite(spawnedboxXpos-dsiWare_titleboxXpos+movecloseXpos, titleboxYpos, GL_FLIP_NONE, &boxfullImage[0 & 63]);
							drawIcon(iconXpos-dsiWare_titleboxXpos+movecloseXpos, titleboxYpos+12, i);
						}
					} else {
						if (theme == 1) {
							glSprite(spawnedboxXpos-dsiWare_titleboxXpos, titleboxYpos, GL_FLIP_NONE, boxemptyImage);
						} else {
							glSprite(spawnedboxXpos-dsiWare_titleboxXpos+movecloseXpos, titleboxYpos, GL_FLIP_NONE, &boxfullImage[1 & 63]);
						}
					}
					spawnedboxXpos += 64;
					iconXpos += 64;
				}
				if (theme == 0) glSprite(spawnedboxXpos+10-dsiWare_titleboxXpos, 80, GL_FLIP_H, braceImage);
			} else {
				for(int i = 0; i < 40; i++) {
					if (theme == 0) {
						moveIconClose(i);
					} else {
						movecloseXpos = 0;
					}
					if (i < spawnedtitleboxes) {
						if (isDirectory[i]) {
							if (theme == 1) glSprite(spawnedboxXpos-titleboxXpos+movecloseXpos, titleboxYpos, GL_FLIP_NONE, folderImage);
							else glSprite(spawnedboxXpos-titleboxXpos+movecloseXpos, titleboxYpos-3, GL_FLIP_NONE, folderImage);
						} else {
							if (theme == 1) glSprite(spawnedboxXpos-titleboxXpos, titleboxYpos, GL_FLIP_NONE, boxfullImage);
							else glSprite(spawnedboxXpos-titleboxXpos+movecloseXpos, titleboxYpos, GL_FLIP_NONE, &boxfullImage[0 & 63]);
							if (bnrRomType[i] == 3) drawIconNES(iconXpos-titleboxXpos+movecloseXpos, titleboxYpos+12);
							else if (bnrRomType[i] == 2) drawIconGBC(iconXpos-titleboxXpos+movecloseXpos, titleboxYpos+12);
							else if (bnrRomType[i] == 1) drawIconGB(iconXpos-titleboxXpos+movecloseXpos, titleboxYpos+12);
							else drawIcon(iconXpos-titleboxXpos+movecloseXpos, titleboxYpos+12, i);
						}
					} else {
						if (theme == 1) {
							glSprite(spawnedboxXpos-titleboxXpos, titleboxYpos, GL_FLIP_NONE, boxemptyImage);
						} else {
							glSprite(spawnedboxXpos-titleboxXpos+movecloseXpos, titleboxYpos, GL_FLIP_NONE, &boxfullImage[1 & 63]);
						}
					}
					spawnedboxXpos += 64;
					iconXpos += 64;
				}
				if (theme == 0) glSprite(spawnedboxXpos+10-titleboxXpos, 80, GL_FLIP_H, braceImage);
			}
			if (applaunchprep && theme==0) {
				// Cover selected app
				for (int y = 0; y < 4; y++)
				{
					for (int x = 0; x < 4; x++)
					{
						glSprite(96+x*16, 84+y*16, GL_FLIP_NONE, &subBgImage[2 & 255]);
					}
				}
				if(startMenu) {
					if (startMenu_cursorPosition == 0) {
						glSprite(96, 83-titleboxYmovepos, GL_FLIP_NONE, &settingsImage[1 & 63]);
					} else if (startMenu_cursorPosition == 1) {
						if (!flashcardUsed) {
							glSprite(96, 83-titleboxYmovepos, GL_FLIP_NONE, &settingsImage[0 & 63]);
						} else {
							glSprite(96, 84-titleboxYmovepos, GL_FLIP_NONE, boxfullImage);
							drawIconGBA(112, 96-titleboxYmovepos);
						}
					} else if (startMenu_cursorPosition == 2) {
						glSprite(96, 84-titleboxYmovepos, GL_FLIP_NONE, boxfullImage);
						drawIconGBA(112, 96-titleboxYmovepos);
					}
				} else if (dsiWareList) {
					glSprite(96, 84-titleboxYmovepos, GL_FLIP_NONE, boxfullImage);
					drawIcon(112, 96-titleboxYmovepos, dsiWare_cursorPosition);
				} else {
					if (isDirectory[cursorPosition]) {
						glSprite(96, 87-titleboxYmovepos, GL_FLIP_NONE, folderImage);
					} else {
						glSprite(96, 84-titleboxYmovepos, GL_FLIP_NONE, boxfullImage);
						if (bnrRomType[cursorPosition] == 3) drawIconNES(112, 96-titleboxYmovepos);
						else if (bnrRomType[cursorPosition] == 2) drawIconGBC(112, 96-titleboxYmovepos);
						else if (bnrRomType[cursorPosition] == 1) drawIconGB(112, 96-titleboxYmovepos);
						else drawIcon(112, 96-titleboxYmovepos, cursorPosition);
					}
				}
				titleboxYmovepos += 5;
				if (titleboxYmovepos > 240) whiteScreen = true;
			}
			if (showSTARTborder) {
				//glColor(RGB15(colorRvalue/3, colorGvalue/3, colorBvalue/3));
				if (theme == 1) {
					glSprite(96, 92, GL_FLIP_NONE, startbrdImage);
					glSprite(96+32, 92, GL_FLIP_H, startbrdImage);
					glColor(RGB15(31, 31, 31));
					if (!startMenu) {
						if (dsiWareList) {
							if (bnrWirelessIcon[dsiWare_cursorPosition] > 0) glSprite(96, 92, GL_FLIP_NONE, &wirelessIcons[(bnrWirelessIcon[dsiWare_cursorPosition]-1) & 31]);
						} else {
							if (bnrWirelessIcon[cursorPosition] > 0) glSprite(96, 92, GL_FLIP_NONE, &wirelessIcons[(bnrWirelessIcon[cursorPosition]-1) & 31]);
						}
					}
				} else {
					glSprite(96, 80, GL_FLIP_NONE, &startbrdImage[startBorderZoomAnimSeq[startBorderZoomAnimNum] & 79]);
					glSprite(96+32, 80, GL_FLIP_H, &startbrdImage[startBorderZoomAnimSeq[startBorderZoomAnimNum] & 79]);
					glColor(RGB15(31, 31, 31));
					if (!startMenu) {
						if (dsiWareList) {
							if (bnrWirelessIcon[dsiWare_cursorPosition] > 0) glSprite(96, 80, GL_FLIP_NONE, &wirelessIcons[(bnrWirelessIcon[dsiWare_cursorPosition]-1) & 31]);
						} else {
							if (bnrWirelessIcon[cursorPosition] > 0) glSprite(96, 80, GL_FLIP_NONE, &wirelessIcons[(bnrWirelessIcon[cursorPosition]-1) & 31]);
						}
					}
				}
			}
			if (showbubble) glSprite(120, bubbleYpos+72, GL_FLIP_NONE, bubblearrowImage);	// Make the bubble look like it's over the START border
			if (showSTARTborder && theme == 0) glSprite(96, 143, GL_FLIP_NONE, startImage);
			if (dbox_Ypos != -192) {
				// Draw the dialog box.
				drawDbox();
				drawIcon(24, dbox_Ypos+20, cursorPosition);
			}
			if (whiteScreen) {
				glBoxFilled(0, 0, 256, 192, RGB15(31, 31, 31));
			} else {
				// Playback animated icons
				for (int i = 0; i < 40; i++) {
					if(bnriconisDSi[i]==true) {
						playBannerSequence(i);
					}
				}
			}
			updateText(false);
			glColor(RGB15(31, 31, 31));
		//}
	}
	glEnd2D();
	GFX_FLUSH = 0;
	if (startBorderZoomOut) {
		startBorderZoomAnimNum++;
		if(startBorderZoomAnimSeq[startBorderZoomAnimNum] == 0) {
			startBorderZoomAnimNum = 0;
			startBorderZoomOut = false;
		}
	} else {
		startBorderZoomAnimNum = 0;
	}
}

void topBgLoad() {
	if (theme == 1) {
		swiDecompressLZSSVram ((void*)_3ds_topTiles, (void*)CHAR_BASE_BLOCK_SUB(4), 0, &decompressBiosCallback);
		vramcpy_ui (&BG_PALETTE_SUB[0], _3ds_topPal, _3ds_topPalLen);
	} else if (subtheme == 1) {
		swiDecompressLZSSVram ((void*)org_topTiles, (void*)CHAR_BASE_BLOCK_SUB(4), 0, &decompressBiosCallback);
		vramcpy_ui (&BG_PALETTE_SUB[0], org_topPal, org_topPalLen);
	} else {
		swiDecompressLZSSVram ((void*)topTiles, (void*)CHAR_BASE_BLOCK_SUB(4), 0, &decompressBiosCallback);
		vramcpy_ui (&BG_PALETTE_SUB[0], topPal, topPalLen);
	}
}

void graphicsInit()
{
	titleboxXpos = cursorPosition*64;
	titlewindowXpos = cursorPosition*5;
	dsiWare_titleboxXpos = dsiWare_cursorPosition*64;
	dsiWare_titlewindowXpos = dsiWare_cursorPosition*5;
	startMenu_titleboxXpos = startMenu_cursorPosition*64;
	startMenu_titlewindowXpos = startMenu_cursorPosition*5;
	
	*(u16*)(0x0400006C) |= BIT(14);
	*(u16*)(0x0400006C) &= BIT(15);
	SetBrightness(0, 31);
	SetBrightness(1, 31);

	irqSet(IRQ_VBLANK, vBlankHandler);
	irqEnable(IRQ_VBLANK);
	////////////////////////////////////////////////////////////
	videoSetMode(MODE_5_3D);
	videoSetModeSub(MODE_0_2D | DISPLAY_BG0_ACTIVE | DISPLAY_BG2_ACTIVE);


	// Initialize gl2d
	glScreen2D();

	// Set up enough texture memory for our textures
	// Bank A is just 128kb and we are using 194 kb of
	// sprites
	vramSetBankA(VRAM_A_TEXTURE);
	vramSetBankB(VRAM_B_TEXTURE);

	vramSetBankF(VRAM_F_TEX_PALETTE); // Allocate VRAM bank for all the palettes

	vramSetBankE(VRAM_E_MAIN_BG);
	lcdMainOnBottom();

	vramSetBankC(VRAM_C_SUB_BG_0x06200000);
	//REG_BG0CNT_SUB = BG_MAP_BASE(0) | BG_COLOR_256 | BG_TILE_BASE(2) | BG_PRIORITY(2);
	REG_BG0CNT_SUB = BG_MAP_BASE(2) | BG_COLOR_256 | BG_TILE_BASE(4) | BG_PRIORITY(1);
	u16* bgMapSub = (u16*)SCREEN_BASE_BLOCK_SUB(2);
	for (int i = 0; i < CONSOLE_SCREEN_WIDTH*CONSOLE_SCREEN_HEIGHT; i++) {
		bgMapSub[i] = (u16)i;
	}

	consoleInit(NULL, 2, BgType_Text4bpp, BgSize_T_256x256, 15, 0, false, true);

	topBgLoad();

	/*if (subtheme == 1) {
		swiDecompressLZSSVram ((void*)org_topTiles, (void*)CHAR_BASE_BLOCK_SUB(4), 0, &decompressBiosCallback);
		vramcpy_ui (&BG_PALETTE_SUB[0], org_topPal, org_topPalLen);
	} else {
		swiDecompressLZSSVram ((void*)topTiles, (void*)CHAR_BASE_BLOCK_SUB(4), 0, &decompressBiosCallback);
		vramcpy_ui (&BG_PALETTE_SUB[0], topPal, topPalLen);
	}*/

	dialogboxTexID = glLoadTileSet(dialogboxImage, // pointer to glImage array
							16, // sprite width
							16, // sprite height
							256, // bitmap width
							192, // bitmap height
							GL_RGB16, // texture type for glTexImage2D() in videoGL.h
							TEXTURE_SIZE_256, // sizeX for glTexImage2D() in videoGL.h
							TEXTURE_SIZE_256, // sizeY for glTexImage2D() in videoGL.h
							GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T | TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
							16, // Length of the palette to use (16 colors)
							(u16*) dialogboxPal, // Load our 16 color tiles palette
							(u8*) dialogboxBitmap // image data generated by GRIT
							);

	if (theme == 1) {
		subBgTexID = glLoadTileSet(subBgImage, // pointer to glImage array
								16, // sprite width
								16, // sprite height
								256, // bitmap width
								256, // bitmap height
								GL_RGB16, // texture type for glTexImage2D() in videoGL.h
								TEXTURE_SIZE_256, // sizeX for glTexImage2D() in videoGL.h
								TEXTURE_SIZE_256, // sizeY for glTexImage2D() in videoGL.h
								GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T | TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
								16, // Length of the palette to use (256 colors)
								(u16*) _3ds_bottomPal, // Load our 16 color tiles palette
								(u8*) _3ds_bottomBitmap // image data generated by GRIT
								);

		titleboxYpos = 96;
		bubbleYpos = 16;
		bubbleTexID = glLoadTileSet(bubbleImage, // pointer to glImage array
								16, // sprite width
								16, // sprite height
								256, // bitmap width
								128, // bitmap height
								GL_RGB16, // texture type for glTexImage2D() in videoGL.h
								TEXTURE_SIZE_256, // sizeX for glTexImage2D() in videoGL.h
								TEXTURE_SIZE_128, // sizeY for glTexImage2D() in videoGL.h
								GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T | TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
								16, // Length of the palette to use (16 colors)
								(u16*) _3ds_bubblePal, // Load our 16 color tiles palette
								(u8*) _3ds_bubbleBitmap // image data generated by GRIT
								);

		bubblearrowTexID = glLoadTileSet(bubblearrowImage, // pointer to glImage array
								16, // sprite width
								16, // sprite height
								16, // bitmap width
								16, // bitmap height
								GL_RGB16, // texture type for glTexImage2D() in videoGL.h
								TEXTURE_SIZE_16, // sizeX for glTexImage2D() in videoGL.h
								TEXTURE_SIZE_16, // sizeY for glTexImage2D() in videoGL.h
								GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T | TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
								16, // Length of the palette to use (16 colors)
								(u16*) _3ds_bubble_arrowPal, // Load our 16 color tiles palette
								(u8*) _3ds_bubble_arrowBitmap // image data generated by GRIT
								);
	} else if (subtheme == 1) {
		subBgTexID = glLoadTileSet(subBgImage, // pointer to glImage array
								16, // sprite width
								16, // sprite height
								256, // bitmap width
								256, // bitmap height
								GL_RGB16, // texture type for glTexImage2D() in videoGL.h
								TEXTURE_SIZE_256, // sizeX for glTexImage2D() in videoGL.h
								TEXTURE_SIZE_256, // sizeY for glTexImage2D() in videoGL.h
								GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T | TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
								16, // Length of the palette to use (16 colors)
								(u16*) org_bottomPal, // Load our 16 color tiles palette
								(u8*) org_bottomBitmap // image data generated by GRIT
								);

		ndsimenutextTexID = glLoadTileSet(ndsimenutextImage, // pointer to glImage array
								256, // sprite width
								32, // sprite height
								256, // bitmap width
								32, // bitmap height
								GL_RGB16, // texture type for glTexImage2D() in videoGL.h
								TEXTURE_SIZE_256, // sizeX for glTexImage2D() in videoGL.h
								TEXTURE_SIZE_32, // sizeY for glTexImage2D() in videoGL.h
								GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T | TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
								16, // Length of the palette to use (16 colors)
								(u16*) org_nintendo_dsi_menuPal, // Load our 16 color tiles palette
								(u8*) org_nintendo_dsi_menuBitmap // image data generated by GRIT
								);

		bubbleTexID = glLoadTileSet(bubbleImage, // pointer to glImage array
								16, // sprite width
								16, // sprite height
								256, // bitmap width
								128, // bitmap height
								GL_RGB16, // texture type for glTexImage2D() in videoGL.h
								TEXTURE_SIZE_256, // sizeX for glTexImage2D() in videoGL.h
								TEXTURE_SIZE_128, // sizeY for glTexImage2D() in videoGL.h
								GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T | TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
								16, // Length of the palette to use (16 colors)
								(u16*) org_bubblePal, // Load our 16 color tiles palette
								(u8*) org_bubbleBitmap // image data generated by GRIT
								);

		bubblearrowTexID = glLoadTileSet(bubblearrowImage, // pointer to glImage array
								16, // sprite width
								16, // sprite height
								16, // bitmap width
								16, // bitmap height
								GL_RGB16, // texture type for glTexImage2D() in videoGL.h
								TEXTURE_SIZE_16, // sizeX for glTexImage2D() in videoGL.h
								TEXTURE_SIZE_16, // sizeY for glTexImage2D() in videoGL.h
								GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T | TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
								16, // Length of the palette to use (16 colors)
								(u16*) org_bubble_arrowPal, // Load our 16 color tiles palette
								(u8*) org_bubble_arrowBitmap // image data generated by GRIT
								);
	} else {
		subBgTexID = glLoadTileSet(subBgImage, // pointer to glImage array
								16, // sprite width
								16, // sprite height
								256, // bitmap width
								256, // bitmap height
								GL_RGB16, // texture type for glTexImage2D() in videoGL.h
								TEXTURE_SIZE_256, // sizeX for glTexImage2D() in videoGL.h
								TEXTURE_SIZE_256, // sizeY for glTexImage2D() in videoGL.h
								GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T | TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
								16, // Length of the palette to use (16 colors)
								(u16*) bottomPal, // Load our 16 color tiles palette
								(u8*) bottomBitmap // image data generated by GRIT
								);

		ndsimenutextTexID = glLoadTileSet(ndsimenutextImage, // pointer to glImage array
								256, // sprite width
								32, // sprite height
								256, // bitmap width
								32, // bitmap height
								GL_RGB16, // texture type for glTexImage2D() in videoGL.h
								TEXTURE_SIZE_256, // sizeX for glTexImage2D() in videoGL.h
								TEXTURE_SIZE_32, // sizeY for glTexImage2D() in videoGL.h
								GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T | TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
								16, // Length of the palette to use (16 colors)
								(u16*) nintendo_dsi_menuPal, // Load our 16 color tiles palette
								(u8*) nintendo_dsi_menuBitmap // image data generated by GRIT
								);

		bubbleTexID = glLoadTileSet(bubbleImage, // pointer to glImage array
								16, // sprite width
								16, // sprite height
								256, // bitmap width
								128, // bitmap height
								GL_RGB16, // texture type for glTexImage2D() in videoGL.h
								TEXTURE_SIZE_256, // sizeX for glTexImage2D() in videoGL.h
								TEXTURE_SIZE_128, // sizeY for glTexImage2D() in videoGL.h
								GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T | TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
								16, // Length of the palette to use (16 colors)
								(u16*) bubblePal, // Load our 16 color tiles palette
								(u8*) bubbleBitmap // image data generated by GRIT
								);

		bubblearrowTexID = glLoadTileSet(bubblearrowImage, // pointer to glImage array
								16, // sprite width
								16, // sprite height
								16, // bitmap width
								16, // bitmap height
								GL_RGB16, // texture type for glTexImage2D() in videoGL.h
								TEXTURE_SIZE_16, // sizeX for glTexImage2D() in videoGL.h
								TEXTURE_SIZE_16, // sizeY for glTexImage2D() in videoGL.h
								GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T | TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
								16, // Length of the palette to use (16 colors)
								(u16*) bubble_arrowPal, // Load our 16 color tiles palette
								(u8*) bubble_arrowBitmap // image data generated by GRIT
								);
	}

	bipsTexID = glLoadTileSet(bipsImage, // pointer to glImage array
							8, // sprite width
							8, // sprite height
							8, // bitmap width
							32, // bitmap height
							GL_RGB16, // texture type for glTexImage2D() in videoGL.h
							TEXTURE_SIZE_8, // sizeX for glTexImage2D() in videoGL.h
							TEXTURE_SIZE_32, // sizeY for glTexImage2D() in videoGL.h
							GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T | TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
							16, // Length of the palette to use (16 colors)
							(u16*) bipsPal, // Load our 16 color tiles palette
							(u8*) bipsBitmap // image data generated by GRIT
							);

	if (subtheme == 1) {
		scrollwindowTexID = glLoadTileSet(scrollwindowImage, // pointer to glImage array
								32, // sprite width
								32, // sprite height
								32, // bitmap width
								32, // bitmap height
								GL_RGB16, // texture type for glTexImage2D() in videoGL.h
								TEXTURE_SIZE_32, // sizeX for glTexImage2D() in videoGL.h
								TEXTURE_SIZE_32, // sizeY for glTexImage2D() in videoGL.h
								GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T | TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
								16, // Length of the palette to use (16 colors)
								(u16*) org_scroll_windowPal, // Load our 16 color tiles palette
								(u8*) org_scroll_windowBitmap // image data generated by GRIT
								);
	} else {
		scrollwindowTexID = glLoadTileSet(scrollwindowImage, // pointer to glImage array
								32, // sprite width
								32, // sprite height
								32, // bitmap width
								32, // bitmap height
								GL_RGB16, // texture type for glTexImage2D() in videoGL.h
								TEXTURE_SIZE_32, // sizeX for glTexImage2D() in videoGL.h
								TEXTURE_SIZE_32, // sizeY for glTexImage2D() in videoGL.h
								GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T | TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
								16, // Length of the palette to use (16 colors)
								(u16*) scroll_windowPal, // Load our 16 color tiles palette
								(u8*) scroll_windowBitmap // image data generated by GRIT
								);
	}

	buttonarrowTexID = glLoadTileSet(buttonarrowImage, // pointer to glImage array
							32, // sprite width
							32, // sprite height
							32, // bitmap width
							64, // bitmap height
							GL_RGB16, // texture type for glTexImage2D() in videoGL.h
							TEXTURE_SIZE_32, // sizeX for glTexImage2D() in videoGL.h
							TEXTURE_SIZE_64, // sizeY for glTexImage2D() in videoGL.h
							GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T | TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
							16, // Length of the palette to use (16 colors)
							(u16*) button_arrowPal, // Load our 16 color tiles palette
							(u8*) button_arrowBitmap // image data generated by GRIT
							);

	startTexID = glLoadTileSet(startImage, // pointer to glImage array
							64, // sprite width
							16, // sprite height
							64, // bitmap width
							16, // bitmap height
							GL_RGB16, // texture type for glTexImage2D() in videoGL.h
							TEXTURE_SIZE_64, // sizeX for glTexImage2D() in videoGL.h
							TEXTURE_SIZE_16, // sizeY for glTexImage2D() in videoGL.h
							GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T | TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
							16, // Length of the palette to use (16 colors)
							(u16*) start_textPal, // Load our 16 color tiles palette
							(u8*) start_textBitmap // image data generated by GRIT
							);

	if (theme == 1) {
		startbrdTexID = glLoadTileSet(startbrdImage, // pointer to glImage array
								32, // sprite width
								64, // sprite height
								32, // bitmap width
								64, // bitmap height
								GL_RGB16, // texture type for glTexImage2D() in videoGL.h
								TEXTURE_SIZE_32, // sizeX for glTexImage2D() in videoGL.h
								TEXTURE_SIZE_64, // sizeY for glTexImage2D() in videoGL.h
								GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T | TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
								16, // Length of the palette to use (16 colors)
								(u16*) _3ds_cursorPal, // Load our 16 color tiles palette
								(u8*) _3ds_cursorBitmap // image data generated by GRIT
								);
	} else {
		startbrdTexID = glLoadTileSet(startbrdImage, // pointer to glImage array
								32, // sprite width
								80, // sprite height
								32, // bitmap width
								256, // bitmap height
								GL_RGB16, // texture type for glTexImage2D() in videoGL.h
								TEXTURE_SIZE_32, // sizeX for glTexImage2D() in videoGL.h
								TEXTURE_SIZE_256, // sizeY for glTexImage2D() in videoGL.h
								GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T | TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
								16, // Length of the palette to use (16 colors)
								(u16*) start_borderPals+((PersonalData->theme)*16), // Load our 16 color tiles palette
								(u8*) start_borderBitmap // image data generated by GRIT
								);
	}

	if (theme == 1) {
		settingsTexID = glLoadTileSet(settingsImage, // pointer to glImage array
								64, // sprite width
								64, // sprite height
								64, // bitmap width
								128, // bitmap height
								GL_RGB16, // texture type for glTexImage2D() in videoGL.h
								TEXTURE_SIZE_64, // sizeX for glTexImage2D() in videoGL.h
								TEXTURE_SIZE_128, // sizeY for glTexImage2D() in videoGL.h
								GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T | TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
								16, // Length of the palette to use (16 colors)
								(u16*) org_icon_settingsPal, // Load our 16 color tiles palette
								(u8*) org_icon_settingsBitmap // image data generated by GRIT
								);

		boxfullTexID = glLoadTileSet(boxfullImage, // pointer to glImage array
								64, // sprite width
								64, // sprite height
								64, // bitmap width
								64, // bitmap height
								GL_RGB16, // texture type for glTexImage2D() in videoGL.h
								TEXTURE_SIZE_64, // sizeX for glTexImage2D() in videoGL.h
								TEXTURE_SIZE_64, // sizeY for glTexImage2D() in videoGL.h
								GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T | TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
								16, // Length of the palette to use (16 colors)
								(u16*) _3ds_box_fullPal, // Load our 16 color tiles palette
								(u8*) _3ds_box_fullBitmap // image data generated by GRIT
								);

		boxemptyTexID = glLoadTileSet(boxemptyImage, // pointer to glImage array
								64, // sprite width
								64, // sprite height
								64, // bitmap width
								64, // bitmap height
								GL_RGB16, // texture type for glTexImage2D() in videoGL.h
								TEXTURE_SIZE_64, // sizeX for glTexImage2D() in videoGL.h
								TEXTURE_SIZE_64, // sizeY for glTexImage2D() in videoGL.h
								GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T | TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
								16, // Length of the palette to use (16 colors)
								(u16*) _3ds_box_emptyPal, // Load our 16 color tiles palette
								(u8*) _3ds_box_emptyBitmap // image data generated by GRIT
								);

		folderTexID = glLoadTileSet(folderImage, // pointer to glImage array
								64, // sprite width
								64, // sprite height
								64, // bitmap width
								64, // bitmap height
								GL_RGB16, // texture type for glTexImage2D() in videoGL.h
								TEXTURE_SIZE_64, // sizeX for glTexImage2D() in videoGL.h
								TEXTURE_SIZE_64, // sizeY for glTexImage2D() in videoGL.h
								GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T | TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
								16, // Length of the palette to use (16 colors)
								(u16*) _3ds_folderPal, // Load our 16 color tiles palette
								(u8*) _3ds_folderBitmap // image data generated by GRIT
								);
	} else if (subtheme == 1) {
		settingsTexID = glLoadTileSet(settingsImage, // pointer to glImage array
								64, // sprite width
								64, // sprite height
								64, // bitmap width
								128, // bitmap height
								GL_RGB16, // texture type for glTexImage2D() in videoGL.h
								TEXTURE_SIZE_64, // sizeX for glTexImage2D() in videoGL.h
								TEXTURE_SIZE_128, // sizeY for glTexImage2D() in videoGL.h
								GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T | TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
								16, // Length of the palette to use (16 colors)
								(u16*) org_icon_settingsPal, // Load our 16 color tiles palette
								(u8*) org_icon_settingsBitmap // image data generated by GRIT
								);

		braceTexID = glLoadTileSet(braceImage, // pointer to glImage array
								16, // sprite width
								128, // sprite height
								16, // bitmap width
								128, // bitmap height
								GL_RGB16, // texture type for glTexImage2D() in videoGL.h
								TEXTURE_SIZE_16, // sizeX for glTexImage2D() in videoGL.h
								TEXTURE_SIZE_128, // sizeY for glTexImage2D() in videoGL.h
								GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T | TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
								16, // Length of the palette to use (16 colors)
								(u16*) org_bracePal, // Load our 16 color tiles palette
								(u8*) org_braceBitmap // image data generated by GRIT
								);

		boxfullTexID = glLoadTileSet(boxfullImage, // pointer to glImage array
								64, // sprite width
								64, // sprite height
								64, // bitmap width
								128, // bitmap height
								GL_RGB16, // texture type for glTexImage2D() in videoGL.h
								TEXTURE_SIZE_64, // sizeX for glTexImage2D() in videoGL.h
								TEXTURE_SIZE_128, // sizeY for glTexImage2D() in videoGL.h
								GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T | TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
								16, // Length of the palette to use (16 colors)
								(u16*) org_boxPal, // Load our 16 color tiles palette
								(u8*) org_boxBitmap // image data generated by GRIT
								);

		folderTexID = glLoadTileSet(folderImage, // pointer to glImage array
								64, // sprite width
								64, // sprite height
								64, // bitmap width
								64, // bitmap height
								GL_RGB16, // texture type for glTexImage2D() in videoGL.h
								TEXTURE_SIZE_64, // sizeX for glTexImage2D() in videoGL.h
								TEXTURE_SIZE_64, // sizeY for glTexImage2D() in videoGL.h
								GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T | TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
								16, // Length of the palette to use (16 colors)
								(u16*) org_folderPal, // Load our 16 color tiles palette
								(u8*) org_folderBitmap // image data generated by GRIT
								);
	} else {
		settingsTexID = glLoadTileSet(settingsImage, // pointer to glImage array
								64, // sprite width
								64, // sprite height
								64, // bitmap width
								128, // bitmap height
								GL_RGB16, // texture type for glTexImage2D() in videoGL.h
								TEXTURE_SIZE_64, // sizeX for glTexImage2D() in videoGL.h
								TEXTURE_SIZE_128, // sizeY for glTexImage2D() in videoGL.h
								GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T | TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
								16, // Length of the palette to use (16 colors)
								(u16*) icon_settingsPal, // Load our 16 color tiles palette
								(u8*) icon_settingsBitmap // image data generated by GRIT
								);

		braceTexID = glLoadTileSet(braceImage, // pointer to glImage array
								16, // sprite width
								128, // sprite height
								16, // bitmap width
								128, // bitmap height
								GL_RGB16, // texture type for glTexImage2D() in videoGL.h
								TEXTURE_SIZE_16, // sizeX for glTexImage2D() in videoGL.h
								TEXTURE_SIZE_128, // sizeY for glTexImage2D() in videoGL.h
								GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T | TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
								16, // Length of the palette to use (16 colors)
								(u16*) bracePal, // Load our 16 color tiles palette
								(u8*) braceBitmap // image data generated by GRIT
								);

		boxfullTexID = glLoadTileSet(boxfullImage, // pointer to glImage array
								64, // sprite width
								64, // sprite height
								64, // bitmap width
								128, // bitmap height
								GL_RGB16, // texture type for glTexImage2D() in videoGL.h
								TEXTURE_SIZE_64, // sizeX for glTexImage2D() in videoGL.h
								TEXTURE_SIZE_128, // sizeY for glTexImage2D() in videoGL.h
								GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T | TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
								16, // Length of the palette to use (16 colors)
								(u16*) boxPal, // Load our 16 color tiles palette
								(u8*) boxBitmap // image data generated by GRIT
								);

		folderTexID = glLoadTileSet(folderImage, // pointer to glImage array
								64, // sprite width
								64, // sprite height
								64, // bitmap width
								64, // bitmap height
								GL_RGB16, // texture type for glTexImage2D() in videoGL.h
								TEXTURE_SIZE_64, // sizeX for glTexImage2D() in videoGL.h
								TEXTURE_SIZE_64, // sizeY for glTexImage2D() in videoGL.h
								GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T | TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
								16, // Length of the palette to use (16 colors)
								(u16*) folderPal, // Load our 16 color tiles palette
								(u8*) folderBitmap // image data generated by GRIT
								);
	}
	
	wirelessiconTexID = glLoadTileSet(wirelessIcons, // pointer to glImage array
							32, // sprite width
							32, // sprite height
							32, // bitmap width
							64, // bitmap height
							GL_RGB16, // texture type for glTexImage2D() in videoGL.h
							TEXTURE_SIZE_32, // sizeX for glTexImage2D() in videoGL.h
							TEXTURE_SIZE_64, // sizeY for glTexImage2D() in videoGL.h
							GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T | TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
							16, // Length of the palette to use (16 colors)
							(u16*) wirelessiconsPal, // Load our 16 color tiles palette
							(u8*) wirelessiconsBitmap // image data generated by GRIT
							);

	loadGBCIcon();
	loadNESIcon();

	/*if (subtheme == 1) {
		mainBgTexID = glLoadTileSet(mainBgImage, // pointer to glImage array
									16, // sprite width
									16, // sprite height
									256, // bitmap width
									256, // bitmap height
									GL_RGB16, // texture type for glTexImage2D() in videoGL.h
									TEXTURE_SIZE_256, // sizeX for glTexImage2D() in videoGL.h
									TEXTURE_SIZE_256, // sizeY for glTexImage2D() in videoGL.h
									GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T | TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
									16, // Length of the palette to use (16 colors)
									(u16*) org_topPal, // Load our 16 color tiles palette
									(u8*) org_topBitmap // image data generated by GRIT
									);

		shoulderTexID = glLoadTileSet(shoulderImage, // pointer to glImage array
									128, // sprite width
									32, // sprite height
									128, // bitmap width
									64, // bitmap height
									GL_RGB16, // texture type for glTexImage2D() in videoGL.h
									TEXTURE_SIZE_128, // sizeX for glTexImage2D() in videoGL.h
									TEXTURE_SIZE_64, // sizeY for glTexImage2D() in videoGL.h
									GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T | TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
									16, // Length of the palette to use (16 colors)
									(u16*) org_shoulderPal, // Load our 16 color tiles palette
									(u8*) org_shoulderBitmap // image data generated by GRIT
									);
	} else {
		mainBgTexID = glLoadTileSet(mainBgImage, // pointer to glImage array
									16, // sprite width
									16, // sprite height
									256, // bitmap width
									256, // bitmap height
									GL_RGB16, // texture type for glTexImage2D() in videoGL.h
									TEXTURE_SIZE_256, // sizeX for glTexImage2D() in videoGL.h
									TEXTURE_SIZE_256, // sizeY for glTexImage2D() in videoGL.h
									GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T | TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
									16, // Length of the palette to use (16 colors)
									(u16*) topPal, // Load our 16 color tiles palette
									(u8*) topBitmap // image data generated by GRIT
									);

		shoulderTexID = glLoadTileSet(shoulderImage, // pointer to glImage array
									128, // sprite width
									32, // sprite height
									128, // bitmap width
									64, // bitmap height
									GL_RGB16, // texture type for glTexImage2D() in videoGL.h
									TEXTURE_SIZE_128, // sizeX for glTexImage2D() in videoGL.h
									TEXTURE_SIZE_64, // sizeY for glTexImage2D() in videoGL.h
									GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T | TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
									16, // Length of the palette to use (16 colors)
									(u16*) shoulderPal, // Load our 16 color tiles palette
									(u8*) shoulderBitmap // image data generated by GRIT
									);
	}*/

}
