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
#include <fstream>
#include "common/gl2d.h"
#include "bios_decompress_callback.h"
#include "FontGraphic.h"
#include "inifile.h"
#include "flashcard.h"

// Graphic files
#include "iconbox.h"
#include "wirelessicons.h"

#include "../iconTitle.h"
#include "graphics.h"
#include "fontHandler.h"
#include "../ndsheaderbanner.h"
#include "../errorScreen.h"

#define CONSOLE_SCREEN_WIDTH 32
#define CONSOLE_SCREEN_HEIGHT 24

extern bool whiteScreen;
extern bool fadeType;
extern bool fadeSpeed;
extern bool controlTopBright;
extern bool controlBottomBright;
extern int colorMode;
extern int blfLevel;
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

int icon1TexID, icon2TexID, icon3TexID, iconboxTexID, wirelessiconTexID;

glImage icon1Image[(73 / 16) * (72 / 16)];
glImage icon2Image[(73 / 16) * (72 / 16)];
glImage icon3Image[(73 / 16) * (72 / 16)];
glImage iconboxImage[(64 / 16) * (64 / 16)];
glImage wirelessIcons[(32 / 32) * (64 / 32)];

int bottomBg;

u16 bmpImageBuffer[256*192];
static u16 topImage[2][256*192];
static u16 bottomImage[2][256*192];

static u16 startBorderColor = 0;
static u16 windowColorTop = 0;
static u16 windowColorBottom = 0;

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

u16 convertToDsBmp(u16 val) {
	if (colorMode == 1) {
		u16 newVal = ((val>>10)&31) | (val&31<<5) | (val&31)<<10 | BIT(15);

		u8 b,g,r,max,min;
		b = ((newVal)>>10)&31;
		g = ((newVal)>>5)&31;
		r = (newVal)&31;
		// Value decomposition of hsv
		max = (b > g) ? b : g;
		max = (max > r) ? max : r;

		// Desaturate
		min = (b < g) ? b : g;
		min = (min < r) ? min : r;
		max = (max + min) / 2;
		
		newVal = 32768|(max<<10)|(max<<5)|(max);

		b = ((newVal)>>10)&(31-6*blfLevel);
		g = ((newVal)>>5)&(31-3*blfLevel);
		r = (newVal)&31;

		return 32768|(b<<10)|(g<<5)|(r);
	} else {
		return ((val>>10)&31) | (val&(31-3*blfLevel)<<5) | (val&(31-6*blfLevel))<<10 | BIT(15);
	}
}

u16 convertVramColorToGrayscale(u16 val) {
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
}

void bottomBgLoad(bool startMenu) {
	dmaCopyWordsAsynch(1, bottomImage[startMenu], BG_GFX, 0x18000);
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
		if (controlBottomBright) SetBrightness(0, screenBrightness);
		if (controlTopBright) SetBrightness(1, screenBrightness);

		glColor(RGB15(31, 31-(3*blfLevel), 31-(6*blfLevel)));

		if (startMenu) {
			glBox(10+(startMenu_cursorPosition*82), 62, 81+(startMenu_cursorPosition*82), 132, startBorderColor);
		} else {
			glBoxFilled(31, 23, 217, 64, RGB15(0, 0, 0));
			glBoxFilled(73, 24, 216, 63, RGB15(31, 31, 31));
			glSprite(32, 24, GL_FLIP_NONE, iconboxImage);
			if (isDirectory) drawIconFolder(36, 28);
			else if (bnrRomType == 7) drawIconSNES(36, 28);
			else if (bnrRomType == 6) drawIconMD(36, 28);
			else if (bnrRomType == 5) drawIconGG(36, 28);
			else if (bnrRomType == 4) drawIconSMS(36, 28);
			else if (bnrRomType == 3) drawIconNES(36, 28);
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
			glBoxFilledGradient(16, 80, 240, 94, windowColorTop, windowColorBottom, windowColorBottom, windowColorTop);
			glBoxFilled(16, 96, 240, 128+(dialogboxHeight*8), RGB15(31, 31, 31));
		}
		if (whiteScreen) {
			glBoxFilled(0, 0, 256, 192, RGB15(31, 31, 31));
		}
		updateText(false);
	}
	glEnd2D();
	GFX_FLUSH = 0;
}

void topBgLoad(bool startMenu) {
	dmaCopyWordsAsynch(0, topImage[startMenu], (u16*)BG_GFX_SUB+(256*32), 0x18000);
}

void graphicsInit()
{	
	*(u16*)(0x0400006C) |= BIT(14);
	*(u16*)(0x0400006C) &= BIT(15);
	SetBrightness(0, 31);
	SetBrightness(1, 31);

	////////////////////////////////////////////////////////////
	videoSetMode(MODE_5_3D | DISPLAY_BG3_ACTIVE);
	videoSetModeSub(MODE_3_2D | DISPLAY_BG0_ACTIVE | DISPLAY_BG3_ACTIVE);

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
	REG_BG0CNT_SUB = BG_MAP_BASE(0) | BG_COLOR_256 | BG_TILE_BASE(2) | BG_PRIORITY(2);
	REG_BG1CNT_SUB = BG_MAP_BASE(2) | BG_COLOR_256 | BG_TILE_BASE(4) | BG_PRIORITY(1);
	u16* bgMapSub = (u16*)SCREEN_BASE_BLOCK_SUB(0);
	for (int i = 0; i < CONSOLE_SCREEN_WIDTH*CONSOLE_SCREEN_HEIGHT; i++) {
		bgMapSub[i] = (u16)i;
	}
	bgMapSub = (u16*)SCREEN_BASE_BLOCK_SUB(2);
	for (int i = 0; i < CONSOLE_SCREEN_WIDTH*CONSOLE_SCREEN_HEIGHT; i++) {
		bgMapSub[i] = (u16)i;
	}
	vramSetBankD(VRAM_D_MAIN_BG_0x06000000);
	vramSetBankE(VRAM_E_TEX_PALETTE);
	vramSetBankF(VRAM_F_TEX_PALETTE_SLOT4);
	vramSetBankG(VRAM_G_TEX_PALETTE_SLOT5); // 16Kb of palette ram, and font textures take up 8*16 bytes.
	vramSetBankH(VRAM_H_SUB_BG_EXT_PALETTE);
	vramSetBankI(VRAM_I_SUB_SPRITE_EXT_PALETTE);

	lcdMainOnBottom();
	
	consoleInit(NULL, 2, BgType_Text4bpp, BgSize_T_256x256, 0, 15, false, true);
	
	REG_BG3CNT = BG_MAP_BASE(0) | BG_BMP16_256x256 | BG_PRIORITY(0);
	REG_BG3X = 0;
	REG_BG3Y = 0;
	REG_BG3PA = 1<<8;
	REG_BG3PB = 0;
	REG_BG3PC = 0;
	REG_BG3PD = 1<<8;

	REG_BG3CNT_SUB = BG_MAP_BASE(1) | BG_BMP16_256x256 | BG_PRIORITY(0);
	REG_BG3X_SUB = 0;
	REG_BG3Y_SUB = 0;
	REG_BG3PA_SUB = 1<<8;
	REG_BG3PB_SUB = 0;
	REG_BG3PC_SUB = 0;
	REG_BG3PD_SUB = 1<<8;

	if (isDSiMode()) {
		loadSdRemovedImage();
	}

	BG_PALETTE_SUB[255] = RGB15(31, 31-(3*blfLevel), 31-(6*blfLevel));

	for (int startMenu = 0; startMenu < 2; startMenu++) {
		extern std::string r4_theme;

		char pathTop[256];
		if (startMenu) {
			std::ifstream file((r4_theme+"logo.bmp").c_str());
			if(file)
				snprintf(pathTop, sizeof(pathTop), (r4_theme+"logo.bmp").c_str());
			else
				snprintf(pathTop, sizeof(pathTop), "nitro:/themes/theme1/logo.bmp");
		} else {
			std::ifstream file((r4_theme+"bckgrd_1.bmp").c_str());
			if(file)
				snprintf(pathTop, sizeof(pathTop), (r4_theme+"bckgrd_1.bmp").c_str());
			else
				snprintf(pathTop, sizeof(pathTop), "nitro:/themes/theme1/bckgrd_1.bmp");
		}

		char pathBottom[256];
		if (startMenu) {
			std::ifstream file((r4_theme+"icons.bmp").c_str());
			if(file)
				snprintf(pathBottom, sizeof(pathBottom), (r4_theme+"icons.bmp").c_str());
			else
				snprintf(pathBottom, sizeof(pathBottom), "nitro:/themes/theme1/icons.bmp");
		} else {
			std::ifstream file((r4_theme+"bckgrd_2.bmp").c_str());
			if(file)
				snprintf(pathBottom, sizeof(pathBottom), (r4_theme+"bckgrd_2.bmp").c_str());
			else
				snprintf(pathBottom, sizeof(pathBottom), "nitro:/themes/theme1/bckgrd_2.bmp");
		}

		FILE* fileTop = fopen(pathTop, "rb");

		if (fileTop) {
			// Start loading
			fseek(fileTop, 0xe, SEEK_SET);
			u8 pixelStart = (u8)fgetc(fileTop) + 0xe;
			fseek(fileTop, pixelStart, SEEK_SET);
			fread(bmpImageBuffer, 2, 0x18000, fileTop);
			u16* src = bmpImageBuffer;
			int x = 0;
			int y = 191;
			for (int i=0; i<256*192; i++) {
				if (x >= 256) {
					x = 0;
					y--;
				}
				u16 val = *(src++);
				topImage[startMenu][y*256+x] = convertToDsBmp(val);
				x++;
			}
		}
		fclose(fileTop);

		FILE* fileBottom = fopen(pathBottom, "rb");

		if (fileBottom) {
			// Start loading
			fseek(fileBottom, 0xe, SEEK_SET);
			u8 pixelStart = (u8)fgetc(fileBottom) + 0xe;
			fseek(fileBottom, pixelStart, SEEK_SET);
			fread(bmpImageBuffer, 2, 0x18000, fileBottom);
			u16* src = bmpImageBuffer;
			int x = 0;
			int y = 191;
			for (int i=0; i<256*192; i++) {
				if (x >= 256) {
					x = 0;
					y--;
				}
				u16 val = *(src++);
				bottomImage[startMenu][y*256+x] = convertToDsBmp(val);
				x++;
			}
		}
		fclose(fileBottom);
	}

	// Initialize the bottom background
	// bottomBg = bgInit(2, BgType_ExRotation, BgSize_ER_256x256, 0,1);

	startBorderColor = RGB15(colorRvalue/8, colorGvalue/8, colorBvalue/8);
	windowColorTop = RGB15(0, 0, 31);
	windowColorBottom = RGB15(0, 0, 15);
	if (colorMode == 1) {
		startBorderColor = convertVramColorToGrayscale(startBorderColor);
		windowColorTop = convertVramColorToGrayscale(windowColorTop);
		windowColorBottom = convertVramColorToGrayscale(windowColorBottom);
	}

	/*if (subtheme >= 0 && subtheme < 12) {
		icon1TexID = glLoadTileSet(icon1Image, // pointer to glImage array
								73, // sprite width
								72, // sprite height
								128, // bitmap width
								72, // bitmap height
								GL_RGB256, // texture type for glTexImage2D() in videoGL.h
								TEXTURE_SIZE_128, // sizeX for glTexImage2D() in videoGL.h
								TEXTURE_SIZE_128, // sizeY for glTexImage2D() in videoGL.h
								TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
								256, // Length of the palette to use (16 colors)
								(u16*) icon1Pal, // Load our 16 color tiles palette
								(u8*) icon1Bitmap // image data generated by GRIT
								);
		icon2TexID = glLoadTileSet(icon2Image, // pointer to glImage array
								73, // sprite width
								72, // sprite height
								128, // bitmap width
								72, // bitmap height
								GL_RGB256, // texture type for glTexImage2D() in videoGL.h
								TEXTURE_SIZE_128, // sizeX for glTexImage2D() in videoGL.h
								TEXTURE_SIZE_128, // sizeY for glTexImage2D() in videoGL.h
								TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
								256, // Length of the palette to use (16 colors)
								(u16*) icon2Pal, // Load our 16 color tiles palette
								(u8*) icon2Bitmap // image data generated by GRIT
								);
		icon3TexID = glLoadTileSet(icon3Image, // pointer to glImage array
								73, // sprite width
								72, // sprite height
								128, // bitmap width
								72, // bitmap height
								GL_RGB256, // texture type for glTexImage2D() in videoGL.h
								TEXTURE_SIZE_128, // sizeX for glTexImage2D() in videoGL.h
								TEXTURE_SIZE_128, // sizeY for glTexImage2D() in videoGL.h
								TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
								256, // Length of the palette to use (16 colors)
								(u16*) icon3Pal, // Load our 16 color tiles palette
								(u8*) icon3Bitmap // image data generated by GRIT
								);
	}*/

	u16* newPalette = (u16*)iconboxPal;
	if (colorMode == 1) {
		// Convert palette to grayscale
		for (int i2 = 0; i2 < 3; i2++) {
			*(newPalette+i2) = convertVramColorToGrayscale(*(newPalette+i2));
		}
	}

	iconboxTexID = glLoadTileSet(iconboxImage, // pointer to glImage array
							40, // sprite width
							40, // sprite height
							64, // bitmap width
							64, // bitmap height
							GL_RGB16, // texture type for glTexImage2D() in videoGL.h
							TEXTURE_SIZE_64, // sizeX for glTexImage2D() in videoGL.h
							TEXTURE_SIZE_64, // sizeY for glTexImage2D() in videoGL.h
							TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
							16, // Length of the palette to use (16 colors)
							(u16*) newPalette, // Load our 16 color tiles palette
							(u8*) iconboxBitmap // image data generated by GRIT
							);

	newPalette = (u16*)wirelessiconsPal;
	if (colorMode == 1) {
		// Convert palette to grayscale
		for (int i2 = 0; i2 < 3; i2++) {
			*(newPalette+i2) = convertVramColorToGrayscale(*(newPalette+i2));
		}
	}

	wirelessiconTexID = glLoadTileSet(wirelessIcons, // pointer to glImage array
							32, // sprite width
							32, // sprite height
							32, // bitmap width
							64, // bitmap height
							GL_RGB16, // texture type for glTexImage2D() in videoGL.h
							TEXTURE_SIZE_32, // sizeX for glTexImage2D() in videoGL.h
							TEXTURE_SIZE_64, // sizeY for glTexImage2D() in videoGL.h
							TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
							16, // Length of the palette to use (16 colors)
							(u16*) newPalette, // Load our 16 color tiles palette
							(u8*) wirelessiconsBitmap // image data generated by GRIT
							);

	loadConsoleIcons();

	irqSet(IRQ_VBLANK, vBlankHandler);
	irqEnable(IRQ_VBLANK);
}
