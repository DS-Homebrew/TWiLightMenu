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
#include "theme01_bckgrd1.h"
#include "theme01_logo.h"
#include "theme01_icons.h"
#include "theme02_bckgrd1.h"
#include "theme02_logo.h"
#include "theme02_icons.h"
#include "theme03_bckgrd1.h"
#include "theme03_logo.h"
#include "theme03_icons.h"
#include "theme04_bckgrd1.h"
#include "theme04_logo.h"
#include "theme04_icons.h"
#include "theme05_bckgrd1.h"
#include "theme05_logo.h"
#include "theme05_icons.h"
#include "theme06_bckgrd1.h"
#include "theme06_logo.h"
#include "theme06_icons.h"
#include "theme07_bckgrd1.h"
#include "theme07_logo.h"
#include "theme07_icons.h"
#include "theme08_bckgrd1.h"
#include "theme08_logo.h"
#include "theme08_icons.h"
#include "theme09_bckgrd1.h"
#include "theme09_logo.h"
#include "theme09_icons.h"
#include "theme10_bckgrd1.h"
#include "theme10_logo.h"
#include "theme10_icons.h"
#include "theme11_bckgrd1.h"
#include "theme11_logo.h"
#include "theme11_icons.h"
#include "theme12_bckgrd1.h"
#include "theme12_logo.h"
#include "theme12_icons.h"
#include "bluemoon_bckgrd1.h"
#include "bluemoon_bckgrd2.h"
#include "bluemoon_icons.h"
#include "iconbox.h"
#include "wirelessicons.h"

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

bool showdialogbox = false;
int dialogboxHeight = 0;

int subBgTexID, blueMoonSubBgTexID, iconboxTexID, wirelessiconTexID;

glImage subBgImage[(256 / 16) * (256 / 16)];
glImage blueMoonSubBgImage[(256 / 16) * (256 / 16)];
glImage iconboxImage[(64 / 16) * (64 / 16)];
glImage wirelessIcons[(32 / 32) * (64 / 32)];

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

		if (startMenu) {
			drawBG(subBgImage);
			glBox(10+(startMenu_cursorPosition*82), 62, 81+(startMenu_cursorPosition*82), 132, RGB15(colorRvalue/3, colorGvalue/3, colorBvalue/3));
		} else {
			switch (subtheme) {
				case 0:
				default:
					glBoxFilled(0, 0, 256, 192, 0x6318);	// R: 192, G: 192, B: 192
					break;
				case 1:
					glBoxFilled(0, 0, 256, 192, 0x431C);	// R: 224, G: 192, B: 128
					break;
				case 2:
					glBoxFilled(0, 0, 256, 192, 0x2314);	// R: 160, G: 192, B: 64
					break;
				case 3:
					glBoxFilled(0, 0, 256, 192, 0x621C);	// R: 224, G: 128, B: 192
					break;
				case 4:
					glBoxFilled(0, 0, 256, 192, 0x0200);	// R: 0, G: 128, B: 0
					break;
				case 5:
					glBoxFilled(0, 0, 256, 192, 0x6210);	// R: 128, G: 128, B: 192
					break;
				case 6:
					glBoxFilled(0, 0, 256, 192, 0x6308);	// R: 64, G: 192, B: 192
					break;
				case 7:
					glBoxFilled(0, 0, 256, 192, 0x6104);	// R: 32, G: 64, B: 192
					break;
				case 8:
					glBoxFilled(0, 0, 256, 192, 0x6010);	// R: 128, G: 0, B: 192
					break;
				case 9:
					glBoxFilled(0, 0, 256, 192, 0x0298);	// R: 192, G: 160, B: 0
					break;
				case 10:
					glBoxFilled(0, 0, 256, 192, 0x0110);	// R: 128, G: 64, B: 0
					break;
				case 11:
					glBoxFilled(0, 0, 256, 192, 0x0098);	// R: 192, G: 32, B: 0
					break;
				case 12:
					drawBG(blueMoonSubBgImage);
					break;
			}

			glBoxFilled(31, 23, 217, 64, RGB15(0, 0, 0));
			glBoxFilled(73, 24, 216, 63, RGB15(31, 31, 31));
			glSprite(32, 24, GL_FLIP_NONE, iconboxImage);
			if (bnrRomType == 3) drawIconNES(36, 28);
			else if (bnrRomType == 2) drawIconGBC(36, 28);
			else if (bnrRomType == 1) drawIconGB(36, 28);
			else drawIcon(36, 28);
			if (bnrWirelessIcon > 0) glSprite(20, 12, GL_FLIP_NONE, &wirelessIcons[(bnrWirelessIcon-1) & 31]);
			// Playback animated icons
			if(bnriconisDSi==true) {
				playBannerSequence();
			}
		}
		if (showdialogbox) {
			glBoxFilled(15, 79, 241, 129+(dialogboxHeight*8), RGB15(0, 0, 0));
			glBoxFilledGradient(16, 80, 240, 94, RGB15(0, 0, 31), RGB15(0, 0, 15), RGB15(0, 0, 15), RGB15(0, 0, 31));
			glBoxFilled(16, 96, 240, 128+(dialogboxHeight*8), RGB15(31, 31, 31));
		}
		updateText(false);
		glColor(RGB15(31, 31, 31));
	}
	glEnd2D();
	GFX_FLUSH = 0;
}

void topLogoLoad() {
	switch (subtheme) {
		case 0:
		default:
			swiDecompressLZSSVram ((void*)theme01_logoTiles, (void*)CHAR_BASE_BLOCK_SUB(4), 0, &decompressBiosCallback);
			vramcpy_ui (&BG_PALETTE_SUB[0], theme01_logoPal, theme01_logoPalLen);
			break;
		case 1:
			swiDecompressLZSSVram ((void*)theme02_logoTiles, (void*)CHAR_BASE_BLOCK_SUB(4), 0, &decompressBiosCallback);
			vramcpy_ui (&BG_PALETTE_SUB[0], theme02_logoPal, theme02_logoPalLen);
			break;
		case 2:
			swiDecompressLZSSVram ((void*)theme03_logoTiles, (void*)CHAR_BASE_BLOCK_SUB(4), 0, &decompressBiosCallback);
			vramcpy_ui (&BG_PALETTE_SUB[0], theme03_logoPal, theme03_logoPalLen);
			break;
		case 3:
			swiDecompressLZSSVram ((void*)theme04_logoTiles, (void*)CHAR_BASE_BLOCK_SUB(4), 0, &decompressBiosCallback);
			vramcpy_ui (&BG_PALETTE_SUB[0], theme04_logoPal, theme04_logoPalLen);
			break;
		case 4:
			swiDecompressLZSSVram ((void*)theme05_logoTiles, (void*)CHAR_BASE_BLOCK_SUB(4), 0, &decompressBiosCallback);
			vramcpy_ui (&BG_PALETTE_SUB[0], theme05_logoPal, theme05_logoPalLen);
			break;
		case 5:
			swiDecompressLZSSVram ((void*)theme06_logoTiles, (void*)CHAR_BASE_BLOCK_SUB(4), 0, &decompressBiosCallback);
			vramcpy_ui (&BG_PALETTE_SUB[0], theme06_logoPal, theme06_logoPalLen);
			break;
		case 6:
			swiDecompressLZSSVram ((void*)theme07_logoTiles, (void*)CHAR_BASE_BLOCK_SUB(4), 0, &decompressBiosCallback);
			vramcpy_ui (&BG_PALETTE_SUB[0], theme07_logoPal, theme07_logoPalLen);
			break;
		case 7:
			swiDecompressLZSSVram ((void*)theme08_logoTiles, (void*)CHAR_BASE_BLOCK_SUB(4), 0, &decompressBiosCallback);
			vramcpy_ui (&BG_PALETTE_SUB[0], theme08_logoPal, theme08_logoPalLen);
			break;
		case 8:
			swiDecompressLZSSVram ((void*)theme09_logoTiles, (void*)CHAR_BASE_BLOCK_SUB(4), 0, &decompressBiosCallback);
			vramcpy_ui (&BG_PALETTE_SUB[0], theme09_logoPal, theme09_logoPalLen);
			break;
		case 9:
			swiDecompressLZSSVram ((void*)theme10_logoTiles, (void*)CHAR_BASE_BLOCK_SUB(4), 0, &decompressBiosCallback);
			vramcpy_ui (&BG_PALETTE_SUB[0], theme10_logoPal, theme10_logoPalLen);
			break;
		case 10:
			swiDecompressLZSSVram ((void*)theme11_logoTiles, (void*)CHAR_BASE_BLOCK_SUB(4), 0, &decompressBiosCallback);
			vramcpy_ui (&BG_PALETTE_SUB[0], theme11_logoPal, theme11_logoPalLen);
			break;
		case 11:
			swiDecompressLZSSVram ((void*)theme12_logoTiles, (void*)CHAR_BASE_BLOCK_SUB(4), 0, &decompressBiosCallback);
			vramcpy_ui (&BG_PALETTE_SUB[0], theme12_logoPal, theme12_logoPalLen);
			break;
		case 12:
			swiDecompressLZSSVram ((void*)bluemoon_bckgrd1Tiles, (void*)CHAR_BASE_BLOCK_SUB(4), 0, &decompressBiosCallback);
			vramcpy_ui (&BG_PALETTE_SUB[0], bluemoon_bckgrd1Pal, bluemoon_bckgrd1PalLen);
			break;
	}
}

void topBgLoad() {
	switch (subtheme) {
		case 0:
		default:
			swiDecompressLZSSVram ((void*)theme01_bckgrd1Tiles, (void*)CHAR_BASE_BLOCK_SUB(4), 0, &decompressBiosCallback);
			vramcpy_ui (&BG_PALETTE_SUB[0], theme01_bckgrd1Pal, theme01_bckgrd1PalLen);
			break;
		case 1:
			swiDecompressLZSSVram ((void*)theme02_bckgrd1Tiles, (void*)CHAR_BASE_BLOCK_SUB(4), 0, &decompressBiosCallback);
			vramcpy_ui (&BG_PALETTE_SUB[0], theme02_bckgrd1Pal, theme02_bckgrd1PalLen);
			break;
		case 2:
			swiDecompressLZSSVram ((void*)theme03_bckgrd1Tiles, (void*)CHAR_BASE_BLOCK_SUB(4), 0, &decompressBiosCallback);
			vramcpy_ui (&BG_PALETTE_SUB[0], theme03_bckgrd1Pal, theme03_bckgrd1PalLen);
			break;
		case 3:
			swiDecompressLZSSVram ((void*)theme04_bckgrd1Tiles, (void*)CHAR_BASE_BLOCK_SUB(4), 0, &decompressBiosCallback);
			vramcpy_ui (&BG_PALETTE_SUB[0], theme04_bckgrd1Pal, theme04_bckgrd1PalLen);
			break;
		case 4:
			swiDecompressLZSSVram ((void*)theme05_bckgrd1Tiles, (void*)CHAR_BASE_BLOCK_SUB(4), 0, &decompressBiosCallback);
			vramcpy_ui (&BG_PALETTE_SUB[0], theme05_bckgrd1Pal, theme05_bckgrd1PalLen);
			break;
		case 5:
			swiDecompressLZSSVram ((void*)theme06_bckgrd1Tiles, (void*)CHAR_BASE_BLOCK_SUB(4), 0, &decompressBiosCallback);
			vramcpy_ui (&BG_PALETTE_SUB[0], theme06_bckgrd1Pal, theme06_bckgrd1PalLen);
			break;
		case 6:
			swiDecompressLZSSVram ((void*)theme07_bckgrd1Tiles, (void*)CHAR_BASE_BLOCK_SUB(4), 0, &decompressBiosCallback);
			vramcpy_ui (&BG_PALETTE_SUB[0], theme07_bckgrd1Pal, theme07_bckgrd1PalLen);
			break;
		case 7:
			swiDecompressLZSSVram ((void*)theme08_bckgrd1Tiles, (void*)CHAR_BASE_BLOCK_SUB(4), 0, &decompressBiosCallback);
			vramcpy_ui (&BG_PALETTE_SUB[0], theme08_bckgrd1Pal, theme08_bckgrd1PalLen);
			break;
		case 8:
			swiDecompressLZSSVram ((void*)theme09_bckgrd1Tiles, (void*)CHAR_BASE_BLOCK_SUB(4), 0, &decompressBiosCallback);
			vramcpy_ui (&BG_PALETTE_SUB[0], theme09_bckgrd1Pal, theme09_bckgrd1PalLen);
			break;
		case 9:
			swiDecompressLZSSVram ((void*)theme10_bckgrd1Tiles, (void*)CHAR_BASE_BLOCK_SUB(4), 0, &decompressBiosCallback);
			vramcpy_ui (&BG_PALETTE_SUB[0], theme10_bckgrd1Pal, theme10_bckgrd1PalLen);
			break;
		case 10:
			swiDecompressLZSSVram ((void*)theme11_bckgrd1Tiles, (void*)CHAR_BASE_BLOCK_SUB(4), 0, &decompressBiosCallback);
			vramcpy_ui (&BG_PALETTE_SUB[0], theme11_bckgrd1Pal, theme11_bckgrd1PalLen);
			break;
		case 11:
			swiDecompressLZSSVram ((void*)theme12_bckgrd1Tiles, (void*)CHAR_BASE_BLOCK_SUB(4), 0, &decompressBiosCallback);
			vramcpy_ui (&BG_PALETTE_SUB[0], theme12_bckgrd1Pal, theme12_bckgrd1PalLen);
			break;
		case 12:
			swiDecompressLZSSVram ((void*)bluemoon_bckgrd1Tiles, (void*)CHAR_BASE_BLOCK_SUB(4), 0, &decompressBiosCallback);
			vramcpy_ui (&BG_PALETTE_SUB[0], bluemoon_bckgrd1Pal, bluemoon_bckgrd1PalLen);
			break;
	}
}

void graphicsInit()
{
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

	switch (subtheme) {
		case 0:
		default:
			subBgTexID = glLoadTileSet(subBgImage, // pointer to glImage array
									16, // sprite width
									16, // sprite height
									256, // bitmap width
									256, // bitmap height
									GL_RGB256, // texture type for glTexImage2D() in videoGL.h
									TEXTURE_SIZE_256, // sizeX for glTexImage2D() in videoGL.h
									TEXTURE_SIZE_256, // sizeY for glTexImage2D() in videoGL.h
									GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T | TEXGEN_OFF, // param for glTexImage2D() in videoGL.h
									256, // Length of the palette to use (256 colors)
									(u16*) theme01_iconsPal, // Load our 16 color tiles palette
									(u8*) theme01_iconsBitmap // image data generated by GRIT
									);
			break;
		case 1:
			subBgTexID = glLoadTileSet(subBgImage, // pointer to glImage array
									16, // sprite width
									16, // sprite height
									256, // bitmap width
									256, // bitmap height
									GL_RGB256, // texture type for glTexImage2D() in videoGL.h
									TEXTURE_SIZE_256, // sizeX for glTexImage2D() in videoGL.h
									TEXTURE_SIZE_256, // sizeY for glTexImage2D() in videoGL.h
									GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T | TEXGEN_OFF, // param for glTexImage2D() in videoGL.h
									256, // Length of the palette to use (256 colors)
									(u16*) theme02_iconsPal, // Load our 16 color tiles palette
									(u8*) theme02_iconsBitmap // image data generated by GRIT
									);
			break;
		case 2:
			subBgTexID = glLoadTileSet(subBgImage, // pointer to glImage array
									16, // sprite width
									16, // sprite height
									256, // bitmap width
									256, // bitmap height
									GL_RGB256, // texture type for glTexImage2D() in videoGL.h
									TEXTURE_SIZE_256, // sizeX for glTexImage2D() in videoGL.h
									TEXTURE_SIZE_256, // sizeY for glTexImage2D() in videoGL.h
									GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T | TEXGEN_OFF, // param for glTexImage2D() in videoGL.h
									256, // Length of the palette to use (256 colors)
									(u16*) theme03_iconsPal, // Load our 16 color tiles palette
									(u8*) theme03_iconsBitmap // image data generated by GRIT
									);
			break;
		case 3:
			subBgTexID = glLoadTileSet(subBgImage, // pointer to glImage array
									16, // sprite width
									16, // sprite height
									256, // bitmap width
									256, // bitmap height
									GL_RGB256, // texture type for glTexImage2D() in videoGL.h
									TEXTURE_SIZE_256, // sizeX for glTexImage2D() in videoGL.h
									TEXTURE_SIZE_256, // sizeY for glTexImage2D() in videoGL.h
									GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T | TEXGEN_OFF, // param for glTexImage2D() in videoGL.h
									256, // Length of the palette to use (256 colors)
									(u16*) theme04_iconsPal, // Load our 16 color tiles palette
									(u8*) theme04_iconsBitmap // image data generated by GRIT
									);
			break;
		case 4:
			subBgTexID = glLoadTileSet(subBgImage, // pointer to glImage array
									16, // sprite width
									16, // sprite height
									256, // bitmap width
									256, // bitmap height
									GL_RGB256, // texture type for glTexImage2D() in videoGL.h
									TEXTURE_SIZE_256, // sizeX for glTexImage2D() in videoGL.h
									TEXTURE_SIZE_256, // sizeY for glTexImage2D() in videoGL.h
									GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T | TEXGEN_OFF, // param for glTexImage2D() in videoGL.h
									256, // Length of the palette to use (256 colors)
									(u16*) theme05_iconsPal, // Load our 16 color tiles palette
									(u8*) theme05_iconsBitmap // image data generated by GRIT
									);
			break;
		case 5:
			subBgTexID = glLoadTileSet(subBgImage, // pointer to glImage array
									16, // sprite width
									16, // sprite height
									256, // bitmap width
									256, // bitmap height
									GL_RGB256, // texture type for glTexImage2D() in videoGL.h
									TEXTURE_SIZE_256, // sizeX for glTexImage2D() in videoGL.h
									TEXTURE_SIZE_256, // sizeY for glTexImage2D() in videoGL.h
									GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T | TEXGEN_OFF, // param for glTexImage2D() in videoGL.h
									256, // Length of the palette to use (256 colors)
									(u16*) theme06_iconsPal, // Load our 16 color tiles palette
									(u8*) theme06_iconsBitmap // image data generated by GRIT
									);
			break;
		case 6:
			subBgTexID = glLoadTileSet(subBgImage, // pointer to glImage array
									16, // sprite width
									16, // sprite height
									256, // bitmap width
									256, // bitmap height
									GL_RGB256, // texture type for glTexImage2D() in videoGL.h
									TEXTURE_SIZE_256, // sizeX for glTexImage2D() in videoGL.h
									TEXTURE_SIZE_256, // sizeY for glTexImage2D() in videoGL.h
									GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T | TEXGEN_OFF, // param for glTexImage2D() in videoGL.h
									256, // Length of the palette to use (256 colors)
									(u16*) theme07_iconsPal, // Load our 16 color tiles palette
									(u8*) theme07_iconsBitmap // image data generated by GRIT
									);
			break;
		case 7:
			subBgTexID = glLoadTileSet(subBgImage, // pointer to glImage array
									16, // sprite width
									16, // sprite height
									256, // bitmap width
									256, // bitmap height
									GL_RGB256, // texture type for glTexImage2D() in videoGL.h
									TEXTURE_SIZE_256, // sizeX for glTexImage2D() in videoGL.h
									TEXTURE_SIZE_256, // sizeY for glTexImage2D() in videoGL.h
									GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T | TEXGEN_OFF, // param for glTexImage2D() in videoGL.h
									256, // Length of the palette to use (256 colors)
									(u16*) theme08_iconsPal, // Load our 16 color tiles palette
									(u8*) theme08_iconsBitmap // image data generated by GRIT
									);
			break;
		case 8:
			subBgTexID = glLoadTileSet(subBgImage, // pointer to glImage array
									16, // sprite width
									16, // sprite height
									256, // bitmap width
									256, // bitmap height
									GL_RGB256, // texture type for glTexImage2D() in videoGL.h
									TEXTURE_SIZE_256, // sizeX for glTexImage2D() in videoGL.h
									TEXTURE_SIZE_256, // sizeY for glTexImage2D() in videoGL.h
									GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T | TEXGEN_OFF, // param for glTexImage2D() in videoGL.h
									256, // Length of the palette to use (256 colors)
									(u16*) theme09_iconsPal, // Load our 16 color tiles palette
									(u8*) theme09_iconsBitmap // image data generated by GRIT
									);
			break;
		case 9:
			subBgTexID = glLoadTileSet(subBgImage, // pointer to glImage array
									16, // sprite width
									16, // sprite height
									256, // bitmap width
									256, // bitmap height
									GL_RGB256, // texture type for glTexImage2D() in videoGL.h
									TEXTURE_SIZE_256, // sizeX for glTexImage2D() in videoGL.h
									TEXTURE_SIZE_256, // sizeY for glTexImage2D() in videoGL.h
									GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T | TEXGEN_OFF, // param for glTexImage2D() in videoGL.h
									256, // Length of the palette to use (256 colors)
									(u16*) theme10_iconsPal, // Load our 16 color tiles palette
									(u8*) theme10_iconsBitmap // image data generated by GRIT
									);
			break;
		case 10:
			subBgTexID = glLoadTileSet(subBgImage, // pointer to glImage array
									16, // sprite width
									16, // sprite height
									256, // bitmap width
									256, // bitmap height
									GL_RGB256, // texture type for glTexImage2D() in videoGL.h
									TEXTURE_SIZE_256, // sizeX for glTexImage2D() in videoGL.h
									TEXTURE_SIZE_256, // sizeY for glTexImage2D() in videoGL.h
									GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T | TEXGEN_OFF, // param for glTexImage2D() in videoGL.h
									256, // Length of the palette to use (256 colors)
									(u16*) theme11_iconsPal, // Load our 16 color tiles palette
									(u8*) theme11_iconsBitmap // image data generated by GRIT
									);
			break;
		case 11:
			subBgTexID = glLoadTileSet(subBgImage, // pointer to glImage array
									16, // sprite width
									16, // sprite height
									256, // bitmap width
									256, // bitmap height
									GL_RGB256, // texture type for glTexImage2D() in videoGL.h
									TEXTURE_SIZE_256, // sizeX for glTexImage2D() in videoGL.h
									TEXTURE_SIZE_256, // sizeY for glTexImage2D() in videoGL.h
									GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T | TEXGEN_OFF, // param for glTexImage2D() in videoGL.h
									256, // Length of the palette to use (256 colors)
									(u16*) theme12_iconsPal, // Load our 16 color tiles palette
									(u8*) theme12_iconsBitmap // image data generated by GRIT
									);
			break;
		case 12:
			subBgTexID = glLoadTileSet(subBgImage, // pointer to glImage array
									16, // sprite width
									16, // sprite height
									256, // bitmap width
									256, // bitmap height
									GL_RGB256, // texture type for glTexImage2D() in videoGL.h
									TEXTURE_SIZE_256, // sizeX for glTexImage2D() in videoGL.h
									TEXTURE_SIZE_256, // sizeY for glTexImage2D() in videoGL.h
									GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T | TEXGEN_OFF, // param for glTexImage2D() in videoGL.h
									256, // Length of the palette to use (256 colors)
									(u16*) bluemoon_iconsPal, // Load our 16 color tiles palette
									(u8*) bluemoon_iconsBitmap // image data generated by GRIT
									);
			blueMoonSubBgTexID = glLoadTileSet(blueMoonSubBgImage, // pointer to glImage array
									16, // sprite width
									16, // sprite height
									256, // bitmap width
									256, // bitmap height
									GL_RGB256, // texture type for glTexImage2D() in videoGL.h
									TEXTURE_SIZE_256, // sizeX for glTexImage2D() in videoGL.h
									TEXTURE_SIZE_256, // sizeY for glTexImage2D() in videoGL.h
									GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T | TEXGEN_OFF, // param for glTexImage2D() in videoGL.h
									256, // Length of the palette to use (256 colors)
									(u16*) bluemoon_bckgrd2Pal, // Load our 16 color tiles palette
									(u8*) bluemoon_bckgrd2Bitmap // image data generated by GRIT
									);
			break;
	}

	iconboxTexID = glLoadTileSet(iconboxImage, // pointer to glImage array
							40, // sprite width
							40, // sprite height
							64, // bitmap width
							64, // bitmap height
							GL_RGB16, // texture type for glTexImage2D() in videoGL.h
							TEXTURE_SIZE_64, // sizeX for glTexImage2D() in videoGL.h
							TEXTURE_SIZE_64, // sizeY for glTexImage2D() in videoGL.h
							GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T | TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
							16, // Length of the palette to use (16 colors)
							(u16*) iconboxPal, // Load our 16 color tiles palette
							(u8*) iconboxBitmap // image data generated by GRIT
							);

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

}
