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
#include "common/inifile.h"
#include "common/dsimenusettings.h"
#include "top_bg.h"
#include "srloader_top_bg.h"
#include "dsisionx_top_bg.h"
#include "sub_bg.h"
#include "font6x8.h"
#include "graphics.h"
#include "fontHandler.h"

#include "soundeffect.h"

#define CONSOLE_SCREEN_WIDTH 32
#define CONSOLE_SCREEN_HEIGHT 24

//extern int appName;

extern bool renderScreens;
extern bool fadeType;
int screenBrightness = 31;

bool renderingTop = true;
int mainBgTexID, subBgTexID;
glImage topBgImage[(256 / 16) * (256 / 16)];
glImage subBgImage[(256 / 16) * (256 / 16)];

void vramcpy_ui (void* dest, const void* src, int size) 
{
	u16* destination = (u16*)dest;
	u16* source = (u16*)src;
	while (size > 0) {
		*destination++ = *source++;
		size-=2;
	}
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

void startRendering(bool top)
{
	if (top)
	{
		lcdMainOnBottom();
		vramSetBankC(VRAM_C_LCD);
		vramSetBankD(VRAM_D_SUB_SPRITE);
		REG_DISPCAPCNT = DCAP_BANK(2) | DCAP_ENABLE | DCAP_SIZE(3);
	}
	else
	{
		lcdMainOnTop();
		vramSetBankD(VRAM_D_LCD);
		vramSetBankC(VRAM_C_SUB_BG);
		REG_DISPCAPCNT = DCAP_BANK(3) | DCAP_ENABLE | DCAP_SIZE(3);
	}
}

bool isRenderingTop()
{
	return renderingTop;
}

void vBlankHandler()
{
	if(fadeType == true) {
		screenBrightness--;
		if (screenBrightness < 0) screenBrightness = 0;
	} else {
		screenBrightness++;
		if (screenBrightness > 31) screenBrightness = 31;
	}
	SetBrightness(0, screenBrightness);
	SetBrightness(1, screenBrightness);

	snd().tickBgMusic();

	if (renderScreens) {
		startRendering(renderingTop);
		glBegin2D();
		{
			if (renderingTop)
			{
				drawBG(topBgImage);
				glColor(RGB15(31, 31, 31));
				updateText(renderingTop);
			}
			else
			{
				drawBG(subBgImage);
				glColor(RGB15(31, 31, 31));
				updateText(renderingTop);
			}
		}
		glEnd2D();
		GFX_FLUSH = 0;
		renderingTop = !renderingTop;
	}
}

void graphicsInit()
{
	*(u16*)(0x0400006C) |= BIT(14);
	*(u16*)(0x0400006C) &= BIT(15);
	SetBrightness(0, 31);
	SetBrightness(1, 31);
	
	renderScreens = true;

	////////////////////////////////////////////////////////////
	videoSetMode(MODE_5_3D);
	videoSetModeSub(MODE_5_2D);

	// Initialize OAM to capture 3D scene
	initSubSprites();

	// The sub background holds the top image when 3D directed to bottom
	bgInitSub(3, BgType_Bmp16, BgSize_B16_256x256, 0, 0);

	// Initialize GL in 3D mode
	glScreen2D();

	// Set up enough texture memory for our textures
	// Bank A is just 128kb and we are using 194 kb of
	// sprites
	// vramSetBankA(VRAM_A_TEXTURE);
	vramSetBankB(VRAM_B_TEXTURE);
	vramSetBankF(VRAM_F_TEX_PALETTE); // Allocate VRAM bank for all the palettes
	vramSetBankE(VRAM_E_MAIN_BG);

	subBgTexID = glLoadTileSet(subBgImage, // pointer to glImage array
							16, // sprite width
							16, // sprite height
							256, // bitmap width
							256, // bitmap height
							GL_RGB16, // texture type for glTexImage2D() in videoGL.h
							TEXTURE_SIZE_256, // sizeX for glTexImage2D() in videoGL.h
							TEXTURE_SIZE_256, // sizeY for glTexImage2D() in videoGL.h
							GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T | TEXGEN_OFF, // param for glTexImage2D() in videoGL.h
							16, // Length of the palette to use (256 colors)
							(u16*) sub_bgPal, // Load our 256 color tiles palette
							(u8*) sub_bgBitmap // image data generated by GRIT
							);

	switch (ms().appName) {
		case 0:
		default:
			mainBgTexID = glLoadTileSet(topBgImage, // pointer to glImage array
									16, // sprite width
									16, // sprite height
									256, // bitmap width
									256, // bitmap height
									GL_RGB256, // texture type for glTexImage2D() in videoGL.h
									TEXTURE_SIZE_256, // sizeX for glTexImage2D() in videoGL.h
									TEXTURE_SIZE_256, // sizeY for glTexImage2D() in videoGL.h
									GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T | TEXGEN_OFF, // param for glTexImage2D() in videoGL.h
									256, // Length of the palette to use (256 colors)
									(u16*) top_bgPal, // Load our 256 color tiles palette
									(u8*) top_bgBitmap // image data generated by GRIT
									);
			break;
		case 1:
			mainBgTexID = glLoadTileSet(topBgImage, // pointer to glImage array
									16, // sprite width
									16, // sprite height
									256, // bitmap width
									256, // bitmap height
									GL_RGB256, // texture type for glTexImage2D() in videoGL.h
									TEXTURE_SIZE_256, // sizeX for glTexImage2D() in videoGL.h
									TEXTURE_SIZE_256, // sizeY for glTexImage2D() in videoGL.h
									GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T | TEXGEN_OFF, // param for glTexImage2D() in videoGL.h
									256, // Length of the palette to use (256 colors)
									(u16*) srloader_top_bgPal, // Load our 256 color tiles palette
									(u8*) srloader_top_bgBitmap // image data generated by GRIT
									);
			break;
		case 2:
			mainBgTexID = glLoadTileSet(topBgImage, // pointer to glImage array
									16, // sprite width
									16, // sprite height
									256, // bitmap width
									256, // bitmap height
									GL_RGB256, // texture type for glTexImage2D() in videoGL.h
									TEXTURE_SIZE_256, // sizeX for glTexImage2D() in videoGL.h
									TEXTURE_SIZE_256, // sizeY for glTexImage2D() in videoGL.h
									GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T | TEXGEN_OFF, // param for glTexImage2D() in videoGL.h
									256, // Length of the palette to use (256 colors)
									(u16*) dsisionx_top_bgPal, // Load our 256 color tiles palette
									(u8*) dsisionx_top_bgBitmap // image data generated by GRIT
									);
			break;
	}

	irqSet(IRQ_VBLANK, vBlankHandler);
	irqEnable(IRQ_VBLANK);
}