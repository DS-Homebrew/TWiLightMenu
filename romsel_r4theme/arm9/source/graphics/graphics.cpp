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

#define CONSOLE_SCREEN_WIDTH 32
#define CONSOLE_SCREEN_HEIGHT 24

extern bool whiteScreen;
extern bool fadeType;
extern bool fadeSpeed;
extern bool controlTopBright;
extern bool controlBottomBright;
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

static u16 bmpImageBuffer[256*192];

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

void bottomBgLoad(bool startMenu) {
	const char* settingsinipath	= "sd:/_nds/TWiLightMenu/settings.ini";
	std::string r4_theme = "sd:/";
	if (access(settingsinipath, F_OK) != 0 && flashcardFound()) {		// Fallback to .ini path on flashcard, if not found on SD card, or if SD access is disabled
		settingsinipath = "fat:/_nds/TWiLightMenu/settings.ini";
		r4_theme = "fat:/";
	}

	CIniFile settingsini( settingsinipath );
	r4_theme += "_nds/TWiLightMenu/r4menu/themes/";
	r4_theme += settingsini.GetString("SRLOADER", "R4_THEME", "") + "/";

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

	FILE* fileBottom = fopen(pathBottom, "rb");

	if (fileBottom) {
		// Start loading
		fseek(fileBottom, 0xe, SEEK_SET);
		u8 pixelStart = (u8)fgetc(fileBottom) + 0xe;
		fseek(fileBottom, pixelStart, SEEK_SET);
		fread(bmpImageBuffer, 2, 0x1A000, fileBottom);
		u16* src = bmpImageBuffer;
		int x = 0;
		int y = 191;
		for (int i=0; i<256*192; i++) {
			if (x >= 256) {
				x = 0;
				y--;
			}
			u16 val = *(src++);
			BG_GFX[y*256+x] = ((val>>10)&0x1f) | ((val)&(0x1f<<5)) | (val&0x1f)<<10 | BIT(15);
			x++;
		}
	}

	fclose(fileBottom);

	/*if (startMenu) {
		switch (subtheme) {
			case 0:
			default:
				dmaCopy(theme01_iconsTiles, bgGetGfxPtr(bottomBg), theme01_iconsTilesLen);
				dmaCopy(theme01_iconsPal, BG_PALETTE, theme01_iconsPalLen);
				dmaCopy(theme01_iconsMap, bgGetMapPtr(bottomBg), theme01_iconsMapLen);
				break;
			case 1:
				dmaCopy(theme02_iconsTiles, bgGetGfxPtr(bottomBg), theme02_iconsTilesLen);
				dmaCopy(theme02_iconsPal, BG_PALETTE, theme02_iconsPalLen);
				dmaCopy(theme02_iconsMap, bgGetMapPtr(bottomBg), theme02_iconsMapLen);
				break;
			case 2:
				dmaCopy(theme03_iconsTiles, bgGetGfxPtr(bottomBg), theme03_iconsTilesLen);
				dmaCopy(theme03_iconsPal, BG_PALETTE, theme03_iconsPalLen);
				dmaCopy(theme03_iconsMap, bgGetMapPtr(bottomBg), theme03_iconsMapLen);
				break;
			case 3:
				dmaCopy(theme04_iconsTiles, bgGetGfxPtr(bottomBg), theme04_iconsTilesLen);
				dmaCopy(theme04_iconsPal, BG_PALETTE, theme04_iconsPalLen);
				dmaCopy(theme04_iconsMap, bgGetMapPtr(bottomBg), theme04_iconsMapLen);
				break;
			case 4:
				dmaCopy(theme05_iconsTiles, bgGetGfxPtr(bottomBg), theme05_iconsTilesLen);
				dmaCopy(theme05_iconsPal, BG_PALETTE, theme05_iconsPalLen);
				dmaCopy(theme05_iconsMap, bgGetMapPtr(bottomBg), theme05_iconsMapLen);
				break;
			case 5:
				dmaCopy(theme06_iconsTiles, bgGetGfxPtr(bottomBg), theme06_iconsTilesLen);
				dmaCopy(theme06_iconsPal, BG_PALETTE, theme06_iconsPalLen);
				dmaCopy(theme06_iconsMap, bgGetMapPtr(bottomBg), theme06_iconsMapLen);
				break;
			case 6:
				dmaCopy(theme07_iconsTiles, bgGetGfxPtr(bottomBg), theme07_iconsTilesLen);
				dmaCopy(theme07_iconsPal, BG_PALETTE, theme07_iconsPalLen);
				dmaCopy(theme07_iconsMap, bgGetMapPtr(bottomBg), theme07_iconsMapLen);
				break;
			case 7:
				dmaCopy(theme08_iconsTiles, bgGetGfxPtr(bottomBg), theme08_iconsTilesLen);
				dmaCopy(theme08_iconsPal, BG_PALETTE, theme08_iconsPalLen);
				dmaCopy(theme08_iconsMap, bgGetMapPtr(bottomBg), theme08_iconsMapLen);
				break;
			case 8:
				dmaCopy(theme09_iconsTiles, bgGetGfxPtr(bottomBg), theme09_iconsTilesLen);
				dmaCopy(theme09_iconsPal, BG_PALETTE, theme09_iconsPalLen);
				dmaCopy(theme09_iconsMap, bgGetMapPtr(bottomBg), theme09_iconsMapLen);
				break;
			case 9:
				dmaCopy(theme10_iconsTiles, bgGetGfxPtr(bottomBg), theme10_iconsTilesLen);
				dmaCopy(theme10_iconsPal, BG_PALETTE, theme10_iconsPalLen);
				dmaCopy(theme10_iconsMap, bgGetMapPtr(bottomBg), theme10_iconsMapLen);
				break;
			case 10:
				dmaCopy(theme11_iconsTiles, bgGetGfxPtr(bottomBg), theme11_iconsTilesLen);
				dmaCopy(theme11_iconsPal, BG_PALETTE, theme11_iconsPalLen);
				dmaCopy(theme11_iconsMap, bgGetMapPtr(bottomBg), theme11_iconsMapLen);
				break;
			case 11:
				dmaCopy(theme12_iconsTiles, bgGetGfxPtr(bottomBg), theme12_iconsTilesLen);
				dmaCopy(theme12_iconsPal, BG_PALETTE, theme12_iconsPalLen);
				dmaCopy(theme12_iconsMap, bgGetMapPtr(bottomBg), theme12_iconsMapLen);
				break;
			case 12:
				dmaCopy(bluemoon_iconsTiles, bgGetGfxPtr(bottomBg), bluemoon_iconsTilesLen);
				dmaCopy(bluemoon_iconsPal, BG_PALETTE, bluemoon_iconsPalLen);
				dmaCopy(bluemoon_iconsMap, bgGetMapPtr(bottomBg), bluemoon_iconsMapLen);
				break;
		}
	} else if (subtheme == 12) {
		dmaCopy(bluemoon_bckgrd2Tiles, bgGetGfxPtr(bottomBg), bluemoon_bckgrd2TilesLen);
		dmaCopy(bluemoon_bckgrd2Pal, BG_PALETTE, bluemoon_bckgrd2PalLen);
		dmaCopy(bluemoon_bckgrd2Map, bgGetMapPtr(bottomBg), bluemoon_bckgrd2MapLen);
	}*/
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
			/*if (subtheme >= 0 && subtheme < 12) {
				glSprite(10, 62, GL_FLIP_NONE, icon1Image);
				glSprite(92, 62, GL_FLIP_NONE, icon2Image);
				glSprite(174, 62, GL_FLIP_NONE, icon3Image);
			}*/
			glBox(10+(startMenu_cursorPosition*82), 62, 81+(startMenu_cursorPosition*82), 132, RGB15(colorRvalue/8, colorGvalue/8, colorBvalue/8));
		} else {
			/*switch (subtheme) {
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
					break;
			}*/

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
		if (whiteScreen) {
			glBoxFilled(0, 0, 256, 192, RGB15(31, 31, 31));
		}
		updateText(false);
		glColor(RGB15(31, 31, 31));
	}
	glEnd2D();
	GFX_FLUSH = 0;
}

void topBgLoad(bool startMenu) {
	const char* settingsinipath	= "sd:/_nds/TWiLightMenu/settings.ini";
	std::string r4_theme = "sd:/";
	if (access(settingsinipath, F_OK) != 0 && flashcardFound()) {		// Fallback to .ini path on flashcard, if not found on SD card, or if SD access is disabled
		settingsinipath = "fat:/_nds/TWiLightMenu/settings.ini";
		r4_theme = "fat:/";
	}

	CIniFile settingsini( settingsinipath );
	r4_theme += "_nds/TWiLightMenu/r4menu/themes/";
	r4_theme += settingsini.GetString("SRLOADER", "R4_THEME", "") + "/";

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

	FILE* fileTop = fopen(pathTop, "rb");

	if (fileTop) {
		// Start loading
		fseek(fileTop, 0xe, SEEK_SET);
		u8 pixelStart = (u8)fgetc(fileTop) + 0xe;
		fseek(fileTop, pixelStart, SEEK_SET);
		fread(bmpImageBuffer, 2, 0x1A000, fileTop);
		u16* src = bmpImageBuffer;
		int x = 0;
		int y = 191+32;
		for (int i=0; i<256*192; i++) {
			if (x >= 0+256) {
				x = 0;
				y--;
			}
			u16 val = *(src++);
			BG_GFX_SUB[y*256+x] = ((val>>10)&0x1f) | ((val)&(0x1f<<5)) | (val&0x1f)<<10 | BIT(15);
			x++;
		}
	}
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

	// Initialize the bottom background
	// bottomBg = bgInit(2, BgType_ExRotation, BgSize_ER_256x256, 0,1);

	swiWaitForVBlank();

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
							TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
							16, // Length of the palette to use (16 colors)
							(u16*) wirelessiconsPal, // Load our 16 color tiles palette
							(u8*) wirelessiconsBitmap // image data generated by GRIT
							);

	loadGBCIcon();
	loadNESIcon();

	irqSet(IRQ_VBLANK, vBlankHandler);
	irqEnable(IRQ_VBLANK);
}
