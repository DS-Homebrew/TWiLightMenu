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
#include "bckgrd1.h"
#include "icons.h"
#include "iconbox.h"

#include "../iconTitle.h"
#include "graphics.h"
#include "fontHandler.h"
#include "../ndsheaderbanner.h"

#define CONSOLE_SCREEN_WIDTH 32
#define CONSOLE_SCREEN_HEIGHT 24

extern bool whiteScreen;
extern bool fadeType;
extern bool fadeSpeed;
int fadeDelay = 0;

extern int colorRvalue;
extern int colorGvalue;
extern int colorBvalue;

int screenBrightness = 31;

extern int spawnedtitleboxes;

extern bool startMenu;

extern bool dsiWareList;
extern int theme;
extern int subtheme;
extern int cursorPosition;
extern int dsiWare_cursorPosition;
extern int startMenu_cursorPosition;
extern int pagenum;
extern int dsiWarePageNum;

int subBgTexID, iconboxTexID;

glImage subBgImage[(256 / 16) * (256 / 16)];
glImage iconboxImage[(64 / 16) * (64 / 16)];

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

void vBlankHandler()
{
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

			//drawBG(subBgImage);
			glBoxFilled(0, 0, 256, 192, 0x0AF2);	// R: 148, G: 189, B: 16
			
			glBoxFilled(31, 23, 72, 64, RGB15(0, 0, 0));
			glSprite(32, 24, GL_FLIP_NONE, iconboxImage);
			if (bnrRomType == 3) drawIconNES(36, 28);
			else if (bnrRomType == 2) drawIconGBC(36, 28);
			else if (bnrRomType == 1) drawIconGB(36, 28);
			else drawIcon(36, 28);
			if (whiteScreen) {
				glBoxFilled(0, 0, 256, 192, RGB15(31, 31, 31));
			} else {
				// Playback animated icons
				if(bnriconisDSi==true) {
					playBannerSequence();
				}
			}
			updateText(false);
			glColor(RGB15(31, 31, 31));
		//}
	}
	glEnd2D();
	GFX_FLUSH = 0;
}

void topBgLoad() {
	//switch (PersonalData->theme) {
		//case 0:
		//default:
			swiDecompressLZSSVram ((void*)bckgrd1Tiles, (void*)CHAR_BASE_BLOCK_SUB(4), 0, &decompressBiosCallback);
			vramcpy_ui (&BG_PALETTE_SUB[0], bckgrd1Pal, bckgrd1PalLen);
			//break;
		/*case 1:
			swiDecompressLZSSVram ((void*)topbg_1brownTiles, (void*)CHAR_BASE_BLOCK_SUB(4), 0, &decompressBiosCallback);
			vramcpy_ui (&BG_PALETTE_SUB[0], topbg_1brownPal, topbg_1brownPalLen);
			break;
		case 2:
			swiDecompressLZSSVram ((void*)topbg_2redTiles, (void*)CHAR_BASE_BLOCK_SUB(4), 0, &decompressBiosCallback);
			vramcpy_ui (&BG_PALETTE_SUB[0], topbg_2redPal, topbg_2redPalLen);
			break;
		case 3:
			swiDecompressLZSSVram ((void*)topbg_3pinkTiles, (void*)CHAR_BASE_BLOCK_SUB(4), 0, &decompressBiosCallback);
			vramcpy_ui (&BG_PALETTE_SUB[0], topbg_3pinkPal, topbg_3pinkPalLen);
			break;
		case 4:
			swiDecompressLZSSVram ((void*)topbg_4orangeTiles, (void*)CHAR_BASE_BLOCK_SUB(4), 0, &decompressBiosCallback);
			vramcpy_ui (&BG_PALETTE_SUB[0], topbg_4orangePal, topbg_4orangePalLen);
			break;
		case 5:
			swiDecompressLZSSVram ((void*)topbg_5yellowTiles, (void*)CHAR_BASE_BLOCK_SUB(4), 0, &decompressBiosCallback);
			vramcpy_ui (&BG_PALETTE_SUB[0], topbg_5yellowPal, topbg_5yellowPalLen);
			break;
		case 6:
			swiDecompressLZSSVram ((void*)topbg_6yellowgreenTiles, (void*)CHAR_BASE_BLOCK_SUB(4), 0, &decompressBiosCallback);
			vramcpy_ui (&BG_PALETTE_SUB[0], topbg_6yellowgreenPal, topbg_6yellowgreenPalLen);
			break;
		case 7:
			swiDecompressLZSSVram ((void*)topbg_7green1Tiles, (void*)CHAR_BASE_BLOCK_SUB(4), 0, &decompressBiosCallback);
			vramcpy_ui (&BG_PALETTE_SUB[0], topbg_7green1Pal, topbg_7green1PalLen);
			break;
		case 8:
			swiDecompressLZSSVram ((void*)topbg_8green2Tiles, (void*)CHAR_BASE_BLOCK_SUB(4), 0, &decompressBiosCallback);
			vramcpy_ui (&BG_PALETTE_SUB[0], topbg_8green2Pal, topbg_8green2PalLen);
			break;
		case 9:
			swiDecompressLZSSVram ((void*)topbg_9lightgreenTiles, (void*)CHAR_BASE_BLOCK_SUB(4), 0, &decompressBiosCallback);
			vramcpy_ui (&BG_PALETTE_SUB[0], topbg_9lightgreenPal, topbg_9lightgreenPalLen);
			break;
		case 10:
			swiDecompressLZSSVram ((void*)topbg_10skyblueTiles, (void*)CHAR_BASE_BLOCK_SUB(4), 0, &decompressBiosCallback);
			vramcpy_ui (&BG_PALETTE_SUB[0], topbg_10skybluePal, topbg_10skybluePalLen);
			break;
		case 11:
			swiDecompressLZSSVram ((void*)topbg_11lightblueTiles, (void*)CHAR_BASE_BLOCK_SUB(4), 0, &decompressBiosCallback);
			vramcpy_ui (&BG_PALETTE_SUB[0], topbg_11lightbluePal, topbg_11lightbluePalLen);
			break;
		case 12:
			swiDecompressLZSSVram ((void*)topbg_12blueTiles, (void*)CHAR_BASE_BLOCK_SUB(4), 0, &decompressBiosCallback);
			vramcpy_ui (&BG_PALETTE_SUB[0], topbg_12bluePal, topbg_12bluePalLen);
			break;
		case 13:
			swiDecompressLZSSVram ((void*)topbg_13violetTiles, (void*)CHAR_BASE_BLOCK_SUB(4), 0, &decompressBiosCallback);
			vramcpy_ui (&BG_PALETTE_SUB[0], topbg_13violetPal, topbg_13violetPalLen);
			break;
		case 14:
			swiDecompressLZSSVram ((void*)topbg_14purpleTiles, (void*)CHAR_BASE_BLOCK_SUB(4), 0, &decompressBiosCallback);
			vramcpy_ui (&BG_PALETTE_SUB[0], topbg_14purplePal, topbg_14purplePalLen);
			break;
		case 15:
			swiDecompressLZSSVram ((void*)topbg_15fuchsiaTiles, (void*)CHAR_BASE_BLOCK_SUB(4), 0, &decompressBiosCallback);
			vramcpy_ui (&BG_PALETTE_SUB[0], topbg_15fuchsiaPal, topbg_15fuchsiaPalLen);
			break;
	}*/
}

void graphicsInit()
{
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
	
	*(u16*)(0x0400006C) |= BIT(14);
	*(u16*)(0x0400006C) &= BIT(15);
	SetBrightness(0, 31);
	SetBrightness(1, 31);

	consoleInit(NULL, 2, BgType_Text4bpp, BgSize_T_256x256, 15, 0, false, true);

	topBgLoad();

	/*if (subtheme == 1) {
		swiDecompressLZSSVram ((void*)org_topTiles, (void*)CHAR_BASE_BLOCK_SUB(4), 0, &decompressBiosCallback);
		vramcpy_ui (&BG_PALETTE_SUB[0], org_topPal, org_topPalLen);
	} else {
		swiDecompressLZSSVram ((void*)topTiles, (void*)CHAR_BASE_BLOCK_SUB(4), 0, &decompressBiosCallback);
		vramcpy_ui (&BG_PALETTE_SUB[0], topPal, topPalLen);
	}*/

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
							(u16*) iconsPal, // Load our 16 color tiles palette
							(u8*) iconsBitmap // image data generated by GRIT
							);

	iconboxTexID = glLoadTileSet(iconboxImage, // pointer to glImage array
							40, // sprite width
							40, // sprite height
							64, // bitmap width
							64, // bitmap height
							GL_RGB16, // texture type for glTexImage2D() in videoGL.h
							TEXTURE_SIZE_64, // sizeX for glTexImage2D() in videoGL.h
							TEXTURE_SIZE_64, // sizeY for glTexImage2D() in videoGL.h
							GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T | TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
							16, // Length of the palette to use (256 colors)
							(u16*) iconboxPal, // Load our 16 color tiles palette
							(u8*) iconboxBitmap // image data generated by GRIT
							);

	loadGBCIcon();
	loadNESIcon();

}
