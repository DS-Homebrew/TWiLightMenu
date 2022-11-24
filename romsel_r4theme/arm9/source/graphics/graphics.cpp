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
#include "common/lodepng.h"
#include "bios_decompress_callback.h"
#include "FontGraphic.h"
#include "common/inifile.h"
#include "common/twlmenusettings.h"
#include "common/systemdetails.h"

// Graphic files
#include "icon_manual.h"
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
extern bool blackScreen;
extern bool fadeType;
extern bool fadeSpeed;
extern bool controlTopBright;
extern bool controlBottomBright;
int fadeDelay = 0;

extern int colorRvalue;
extern int colorGvalue;
extern int colorBvalue;

int screenBrightness = 0;
bool lcdSwapped = false;
static bool secondBuffer = false;
bool doubleBuffer = true;

int frameOf60fps = 60;
int frameDelay = 0;
bool frameDelayEven = true; // For 24FPS
bool renderFrame = true;

extern int spawnedtitleboxes;

extern bool startMenu;

extern int startMenu_cursorPosition;

bool manualIconNextImg = false;

bool showdialogbox = false;
int dialogboxHeight = 0;

int manualTexID, iconboxTexID, wirelessiconTexID;

glImage manualIcon[(32 / 32) * (64 / 32)];
glImage iconboxImage[(64 / 16) * (64 / 16)];
glImage wirelessIcons[(32 / 32) * (64 / 32)];

int bottomBg;

u16 bmpImageBuffer[256*192];
static u16 topImage[2][2][256*192];
static u16 bottomImage[2][2][256*192];

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

bool screenFadedIn(void) { return (screenBrightness == 0); }

bool screenFadedOut(void) { return (screenBrightness > 24); }

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

void frameRateHandler(void) {
	frameOf60fps++;
	if (frameOf60fps > 60) frameOf60fps = 1;

	if (!renderFrame) {
		frameDelay++;
		switch (ms().fps) {
			case 11:
				renderFrame = (frameDelay == 5+frameDelayEven);
				break;
			case 24:
			//case 25:
				renderFrame = (frameDelay == 2+frameDelayEven);
				break;
			case 48:
				renderFrame = (frameOf60fps != 3
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
				renderFrame = (frameOf60fps != 3
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
				renderFrame = (frameDelay == 60/ms().fps);
				break;
		}
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
		for (int x = 0; x < 4; x++) {
			oamSub.oamMemory[id].attribute[0] = ATTR0_BMP | ATTR0_SQUARE | (64 * y);
			oamSub.oamMemory[id].attribute[1] = ATTR1_SIZE_64 | (64 * x);
			oamSub.oamMemory[id].attribute[2] = ATTR2_ALPHA(1) | (8 * 32 * y) | (8 * x);
			++id;
		}

	swiWaitForVBlank();

	oamUpdate(&oamSub);
}

u16 convertToDsBmp(u16 val) {
	if (ms().colorMode == 1) {
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

		b = ((newVal)>>10)&31;
		g = ((newVal)>>5)&31;
		r = (newVal)&31;

		return 32768|(b<<10)|(g<<5)|(r);
	} else {
		return ((val>>10)&31) | (val&(31<<5)) | ((val&31)<<10) | BIT(15);
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
	if (doubleBuffer) {
		extern bool startMenu;
		dmaCopyHalfWordsAsynch(0, topImage[startMenu][secondBuffer], (u16*)BG_GFX_SUB+(256*32), 0x18000);
		dmaCopyHalfWordsAsynch(1, bottomImage[startMenu][secondBuffer], BG_GFX, 0x18000);
		secondBuffer = !secondBuffer;
	}

	glBegin2D();
	{
		if (fadeType == true) {
			if (!fadeDelay) {
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
			if (!fadeDelay) {
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
		if (renderFrame) {
			if (ms().macroMode) {
				SetBrightness(0, lcdSwapped ? (ms().theme==6 ? -screenBrightness : screenBrightness) : (ms().theme==6 ? -31 : 31));
				SetBrightness(1, !lcdSwapped ? (ms().theme==6 ? -screenBrightness : screenBrightness) : (ms().theme==6 ? -31 : 31));
			} else {
				if (controlBottomBright) SetBrightness(0, ms().theme==6 ? -screenBrightness : screenBrightness);
				if (controlTopBright) SetBrightness(1, ms().theme==6 ? -screenBrightness : screenBrightness);
			}
		}

		// glColor(RGB15(31, 31-(3*blfLevel), 31-(6*blfLevel)));
		glColor(RGB15(31, 31, 31));

	  if (ms().theme != TWLSettings::EThemeGBC) {
		if (startMenu) {
			glBox(10+(startMenu_cursorPosition*82), 62, 81+(startMenu_cursorPosition*82), 132, startBorderColor);
			glSprite(232, 2, GL_FLIP_NONE, &manualIcon[manualIconNextImg]);
		} else {
			glBoxFilled(35, 23, 217, 64, RGB15(0, 0, 0));
			glBoxFilled(77, 24, 216, 63, RGB15(31, 31, 31));
			glSprite(36, 24, GL_FLIP_NONE, iconboxImage);
			if (isDirectory) drawIconFolder(40, 28);
			else if (customIcon) drawIcon(40, 28);
			else if (bnrRomType == 20) drawIconIMG(40, 28);
			else if (bnrRomType == 19) drawIconVID(40, 28);
			else if (bnrRomType == 18) drawIconCPC(40, 28);
			else if (bnrRomType == 17) drawIconNGP(40, 28);
			else if (bnrRomType == 16) drawIconWS(40, 28);
			else if (bnrRomType == 15) drawIconSG(40, 28);
			else if (bnrRomType == 14) drawIconM5(40, 28);
			else if (bnrRomType == 13) drawIconCOL(40, 28);
			else if (bnrRomType == 12) drawIconINT(40, 28);
			else if (bnrRomType == 11) drawIconPCE(40, 28);
			else if (bnrRomType == 10) drawIconA26(40, 28);
			else if (bnrRomType == 9) drawIconPlg(40, 28);
			else if (bnrRomType == 8) drawIconSNES(40, 28);
			else if (bnrRomType == 7) drawIconMD(40, 28);
			else if (bnrRomType == 6) drawIconGG(40, 28);
			else if (bnrRomType == 5) drawIconSMS(40, 28);
			else if (bnrRomType == 4) drawIconNES(40, 28);
			else if (bnrRomType == 3) drawIconGBC(40, 28);
			else if (bnrRomType == 2) drawIconGB(40, 28);
			else if (bnrRomType == 1) drawIconGBA(40, 28);
			else drawIcon(40, 28);
			if (bnrWirelessIcon > 0) glSprite(24, 12, GL_FLIP_NONE, &wirelessIcons[(bnrWirelessIcon-1) & 31]);
			// Playback animated icons
			if (bnriconisDSi==true) {
				playBannerSequence();
			}
		}
	  }
		if (showdialogbox) {
			glBoxFilled(15, 71, 241, 121+(dialogboxHeight*12), RGB15(0, 0, 0));
			glBoxFilledGradient(16, 72, 240, 86, windowColorTop, windowColorBottom, windowColorBottom, windowColorTop);
			glBoxFilled(16, 88, 240, 120+(dialogboxHeight*12), RGB15(31, 31, 31));
		}
		if (whiteScreen) {
			glBoxFilled(0, 0, 256, 192, RGB15(31, 31, 31));
		} else if (blackScreen) {
			glBoxFilled(0, 0, 256, 192, RGB15(0, 0, 0));
		}
		updateText(false);
	}
	glEnd2D();
	GFX_FLUSH = 0;

	frameDelay = 0;
	frameDelayEven = !frameDelayEven;
	renderFrame = false;

	manualIconNextImg = !manualIconNextImg;
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

	if (ms().macroMode && ms().theme == TWLSettings::EThemeGBC) {
		lcdMainOnTop();
		lcdSwapped = false;
	} else {
		lcdMainOnBottom();
		lcdSwapped = true;
	}

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

	// Make screens black
	dmaFillWords(0, BG_GFX, 0x18000);
	dmaFillWords(0, (u16*)BG_GFX_SUB+(256*32), 0x18000);
	SetBrightness(0, 0);
	SetBrightness(1, 0);
}

void graphicsLoad()
{	
	if (isDSiMode()) {
		loadSdRemovedImage();
	}

	// BG_PALETTE_SUB[255] = RGB15(31, 31-(3*blfLevel), 31-(6*blfLevel));
	BG_PALETTE_SUB[255] = RGB15(31, 31, 31);

	uint imageWidth, imageHeight;
	std::vector<unsigned char> image;
	bool alternatePixel = false;

	if (ms().theme == TWLSettings::EThemeGBC) {
		lodepng::decode(image, imageWidth, imageHeight, "nitro:/graphics/gbcborder.png");

		for (uint i=0; i<image.size()/4; i++) {
			topImage[0][0][i] = image[i*4]>>3 | (image[(i*4)+1]>>3)<<5 | (image[(i*4)+2]>>3)<<10 | BIT(15);
			if (ms().colorMode == 1) {
				topImage[0][0][i] = convertVramColorToGrayscale(topImage[0][0][i]);
			}
			topImage[0][1][i] = topImage[0][0][i];
			topImage[1][0][i] = topImage[0][0][i];
			topImage[1][1][i] = topImage[0][0][i];
		}

		FILE* fileTop = fopen("nitro:/themes/gbnp/bg.bmp", "rb");

		if (fileTop) {
			// Start loading
			fseek(fileTop, 0xe, SEEK_SET);
			u8 pixelStart = (u8)fgetc(fileTop) + 0xe;
			fseek(fileTop, pixelStart, SEEK_SET);
			fread(bmpImageBuffer, 1, 0xB40A, fileTop);
			u16* src = bmpImageBuffer;
			int x = 48;
			int y = 167;
			for (int i=0; i<160*144; i++) {
				if (x >= 48+160) {
					x = 48;
					y--;
				}
				u16 val = *(src++);
				topImage[0][0][y*256+x] = convertToDsBmp(val);
				topImage[0][1][y*256+x] = topImage[0][0][y*256+x];
				topImage[1][0][y*256+x] = topImage[0][0][y*256+x];
				topImage[1][1][y*256+x] = topImage[0][0][y*256+x];
				x++;
			}
		}
		fclose(fileTop);
	} else
	for (int startMenu = 0; startMenu < 2; startMenu++) {
		image.clear();
		std::string themePath = std::string(sys().isRunFromSD() ? "sd:" : "fat:") + "/_nds/TwilightMenu/r4menu/themes/" + ms().r4_theme;
		std::string pathTop;
		if (startMenu) {
			FILE* file = fopen((themePath + "/logo.png").c_str(), "rb");
			if (file)
				pathTop = themePath + "/logo.png";
			else
				pathTop = "nitro:/themes/theme1/logo.png";
			fclose(file);
		} else {
			FILE* file = fopen((themePath + "/bckgrd_1.png").c_str(), "rb");
			if (file)
				pathTop = themePath + "/bckgrd_1.png";
			else
				pathTop = "nitro:/themes/theme1/bckgrd_1.png";
			fclose(file);
		}

		std::string pathBottom;
		if (startMenu) {
			FILE* file = fopen((themePath + "/icons.png").c_str(), "rb");
			if (file)
				pathBottom = themePath + "/icons.png";
			else
				pathBottom = "nitro:/themes/theme1/icons.png";
			fclose(file);
		} else {
			FILE* file = fopen((themePath + "/bckgrd_2.png").c_str(), "rb");
			if (file)
				pathBottom = themePath + "/bckgrd_2.png";
			else
				pathBottom = "nitro:/themes/theme1/bckgrd_2.png";
			fclose(file);
		}

		lodepng::decode(image, imageWidth, imageHeight, pathTop);

		for (unsigned i=0;i<image.size()/4;i++) {
			image[(i*4)+3] = 0;
			if (alternatePixel) {
				if (image[(i*4)] >= 0x4) {
					image[(i*4)] -= 0x4;
					image[(i*4)+3] |= BIT(0);
				}
				if (image[(i*4)+1] >= 0x4) {
					image[(i*4)+1] -= 0x4;
					image[(i*4)+3] |= BIT(1);
				}
				if (image[(i*4)+2] >= 0x4) {
					image[(i*4)+2] -= 0x4;
					image[(i*4)+3] |= BIT(2);
				}
			}
			topImage[startMenu][0][i] = image[i*4]>>3 | (image[(i*4)+1]>>3)<<5 | (image[(i*4)+2]>>3)<<10 | BIT(15);
			if (ms().colorMode == 1) {
				topImage[startMenu][0][i] = convertVramColorToGrayscale(topImage[startMenu][0][i]);
			}
			if (alternatePixel) {
				if (image[(i*4)+3] & BIT(0)) {
					image[(i*4)] += 0x4;
				}
				if (image[(i*4)+3] & BIT(1)) {
					image[(i*4)+1] += 0x4;
				}
				if (image[(i*4)+3] & BIT(2)) {
					image[(i*4)+2] += 0x4;
				}
			} else {
				if (image[(i*4)] >= 0x4) {
					image[(i*4)] -= 0x4;
				}
				if (image[(i*4)+1] >= 0x4) {
					image[(i*4)+1] -= 0x4;
				}
				if (image[(i*4)+2] >= 0x4) {
					image[(i*4)+2] -= 0x4;
				}
			}
			topImage[startMenu][1][i] = image[i*4]>>3 | (image[(i*4)+1]>>3)<<5 | (image[(i*4)+2]>>3)<<10 | BIT(15);
			if (ms().colorMode == 1) {
				topImage[startMenu][1][i] = convertVramColorToGrayscale(topImage[startMenu][1][i]);
			}
			if ((i % 256) == 255) alternatePixel = !alternatePixel;
			alternatePixel = !alternatePixel;
		}

		image.clear();
		lodepng::decode(image, imageWidth, imageHeight, pathBottom);

		for (unsigned i=0;i<image.size()/4;i++) {
			image[(i*4)+3] = 0;
			if (alternatePixel) {
				if (image[(i*4)] >= 0x4) {
					image[(i*4)] -= 0x4;
					image[(i*4)+3] |= BIT(0);
				}
				if (image[(i*4)+1] >= 0x4) {
					image[(i*4)+1] -= 0x4;
					image[(i*4)+3] |= BIT(1);
				}
				if (image[(i*4)+2] >= 0x4) {
					image[(i*4)+2] -= 0x4;
					image[(i*4)+3] |= BIT(2);
				}
			}
			bottomImage[startMenu][0][i] = image[i*4]>>3 | (image[(i*4)+1]>>3)<<5 | (image[(i*4)+2]>>3)<<10 | BIT(15);
			if (ms().colorMode == 1) {
				bottomImage[startMenu][0][i] = convertVramColorToGrayscale(bottomImage[startMenu][0][i]);
			}
			if (alternatePixel) {
				if (image[(i*4)+3] & BIT(0)) {
					image[(i*4)] += 0x4;
				}
				if (image[(i*4)+3] & BIT(1)) {
					image[(i*4)+1] += 0x4;
				}
				if (image[(i*4)+3] & BIT(2)) {
					image[(i*4)+2] += 0x4;
				}
			} else {
				if (image[(i*4)] >= 0x4) {
					image[(i*4)] -= 0x4;
				}
				if (image[(i*4)+1] >= 0x4) {
					image[(i*4)+1] -= 0x4;
				}
				if (image[(i*4)+2] >= 0x4) {
					image[(i*4)+2] -= 0x4;
				}
			}
			bottomImage[startMenu][1][i] = image[i*4]>>3 | (image[(i*4)+1]>>3)<<5 | (image[(i*4)+2]>>3)<<10 | BIT(15);
			if (ms().colorMode == 1) {
				bottomImage[startMenu][1][i] = convertVramColorToGrayscale(bottomImage[startMenu][1][i]);
			}
			if ((i % 256) == 255) alternatePixel = !alternatePixel;
			alternatePixel = !alternatePixel;
		}
	}

	// Initialize the bottom background
	// bottomBg = bgInit(2, BgType_ExRotation, BgSize_ER_256x256, 0,1);

	startBorderColor = RGB15(colorRvalue/8, colorGvalue/8, colorBvalue/8);
	windowColorTop = RGB15(0, 0, 31);
	windowColorBottom = RGB15(0, 0, 15);
	if (ms().colorMode == 1) {
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

	u16* newPalette = (u16*)icon_manualPal;
	if (ms().colorMode == 1) {
		// Convert palette to grayscale
		for (int i2 = 0; i2 < 16; i2++) {
			*(newPalette+i2) = convertVramColorToGrayscale(*(newPalette+i2));
		}
	}

	manualTexID = glLoadTileSet(manualIcon, // pointer to glImage array
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
							(u8*) icon_manualBitmap // image data generated by GRIT
							);

	newPalette = (u16*)iconboxPal;
	if (ms().colorMode == 1) {
		// Convert palette to grayscale
		for (int i2 = 0; i2 < 16; i2++) {
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
	if (ms().colorMode == 1) {
		// Convert palette to grayscale
		for (int i2 = 0; i2 < 16; i2++) {
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
	irqSet(IRQ_VCOUNT, frameRateHandler);
	irqEnable(IRQ_VCOUNT);
}
