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
#include "cursor.h"
#include "iconbox.h"
#include "wirelessicons.h"
#include "pictodlp.h"
#include "icon_dscard.h"
#include "icon_gba.h"
#include "iconPhat_gba.h"
#include "icon_gbamode.h"
#include "cornericons.h"
#include "icon_settings.h"

#include "cursorpal.h"

#include "../iconTitle.h"
#include "graphics.h"
#include "graphics/lodepng.h"
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

extern bool showCursor;
extern bool startMenu;
extern int startMenu_cursorPosition;

extern int launchType;
extern bool pictochatFound;
extern bool dlplayFound;
extern bool gbaBiosFound;
extern bool useGbarunner;
extern bool isDSPhat(void);
extern int theme;
extern int subtheme;
extern int consoleModel;

int boxArtType = 0;

//bool moveIconUp[7] = {false};
int iconYpos[7] = {25, 73, 73, 121, 175, 170, 175};

bool showdialogbox = false;
int dialogboxHeight = 0;

int cursorTexID, iconboxTexID, wirelessiconTexID, pictodlpTexID, dscardiconTexID, gbaiconTexID, cornericonTexID, settingsiconTexID;

glImage cursorImage[(32 / 32) * (128 / 32)];
glImage iconboxImage[(256 / 16) * (128 / 64)];
glImage wirelessIcons[(32 / 32) * (64 / 32)];
glImage pictodlpImage[(128 / 16) * (256 / 64)];
glImage dscardIconImage[(32 / 32) * (64 / 32)];
glImage gbaIconImage[(32 / 32) * (32 / 32)];
glImage cornerIcons[(32 / 32) * (128 / 32)];
glImage settingsIconImage[(32 / 32) * (32 / 32)];

u16 bmpImageBuffer[256*192];

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

void bottomBgLoad(void) {
	std::vector<unsigned char> image;
	unsigned width, height;
	lodepng::decode(image, width, height, "nitro:/graphics/bottombg.png");
	for(unsigned i = 0; i < image.size(); i = i * 4) {
		bmpImageBuffer[i] = image[i] >> 3 | (image[i + 1]>>3)<<5 | (image[i + 2] >> 3) << 10 | BIT(15);
		if (colorMode == 1) {
			bmpImageBuffer[i] = convertVramColorToGrayscale(bmpImageBuffer[i]);
		}
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
			if (isDSiMode() && launchType == 0) {
				glSprite(33, iconYpos[0], GL_FLIP_NONE, &iconboxImage[(REG_SCFG_MC == 0x11) ? 1 : 0]);
				glSprite(40, iconYpos[0]+6, GL_FLIP_NONE, &dscardIconImage[(REG_SCFG_MC == 0x11) ? 1 : 0]);
			} else {
				glSprite(33, iconYpos[0], GL_FLIP_NONE, &iconboxImage[0]);
				if (bnrRomType == 9) drawIconPlg(40, iconYpos[0]+6);
				else if (bnrRomType == 8) drawIconSNES(40, iconYpos[0]+6);
				else if (bnrRomType == 7) drawIconMD(40, iconYpos[0]+6);
				else if (bnrRomType == 6) drawIconGG(40, iconYpos[0]+6);
				else if (bnrRomType == 5) drawIconSMS(40, iconYpos[0]+6);
				else if (bnrRomType == 4) drawIconNES(40, iconYpos[0]+6);
				else if (bnrRomType == 3) drawIconGBC(40, iconYpos[0]+6);
				else if (bnrRomType == 2) drawIconGB(40, iconYpos[0]+6);
				else if (bnrRomType == 1) glSprite(40, iconYpos[0]+6, GL_FLIP_NONE, gbaIconImage);
				else drawIcon(40, iconYpos[0]+6);
			}
			if (bnrWirelessIcon > 0) glSprite(207, iconYpos[0]+30, GL_FLIP_NONE, &wirelessIcons[(bnrWirelessIcon-1) & 31]);
			// Playback animated icons
			if(bnriconisDSi==true) {
				playBannerSequence();
			}
			glSprite(33, iconYpos[1], GL_FLIP_NONE, &pictodlpImage[1-pictochatFound]);
			glSprite(129, iconYpos[2], GL_FLIP_NONE, &pictodlpImage[3-dlplayFound]);
			glSprite(33, iconYpos[3], GL_FLIP_NONE, &iconboxImage[1-gbaBiosFound]);
			glSprite(40, iconYpos[3]+6, GL_FLIP_NONE, gbaIconImage);
			if (isDSiMode() && consoleModel < 2) {
				glSprite(10, iconYpos[4], GL_FLIP_NONE, &cornerIcons[0]);
			}
			glSprite(117, iconYpos[5], GL_FLIP_NONE, settingsIconImage);
			glSprite(235, iconYpos[6], GL_FLIP_NONE, &cornerIcons[3]);

			// Draw cursor
			if (showCursor) {
				switch (startMenu_cursorPosition) {
					case 0:
					default:
						glSprite(31, 23, GL_FLIP_NONE, &cursorImage[0]);
						glSprite(213, 23, GL_FLIP_NONE, &cursorImage[1]);
						glSprite(31, 61, GL_FLIP_NONE, &cursorImage[2]);
						glSprite(213, 61, GL_FLIP_NONE, &cursorImage[3]);
						break;
					case 1:
						glSprite(31, 71, GL_FLIP_NONE, &cursorImage[0]);
						glSprite(117, 71, GL_FLIP_NONE, &cursorImage[1]);
						glSprite(31, 109, GL_FLIP_NONE, &cursorImage[2]);
						glSprite(117, 109, GL_FLIP_NONE, &cursorImage[3]);
						break;
					case 2:
						glSprite(127, 71, GL_FLIP_NONE, &cursorImage[0]);
						glSprite(213, 71, GL_FLIP_NONE, &cursorImage[1]);
						glSprite(127, 109, GL_FLIP_NONE, &cursorImage[2]);
						glSprite(213, 109, GL_FLIP_NONE, &cursorImage[3]);
						break;
					case 3:
						glSprite(31, 119, GL_FLIP_NONE, &cursorImage[0]);
						glSprite(213, 119, GL_FLIP_NONE, &cursorImage[1]);
						glSprite(31, 157, GL_FLIP_NONE, &cursorImage[2]);
						glSprite(213, 157, GL_FLIP_NONE, &cursorImage[3]);
						break;
					case 4:
						glSprite(0, 167, GL_FLIP_NONE, &cursorImage[0]);
						glSprite(20, 167, GL_FLIP_NONE, &cursorImage[1]);
						glSprite(0, 182, GL_FLIP_NONE, &cursorImage[2]);
						glSprite(20, 182, GL_FLIP_NONE, &cursorImage[3]);
						break;
					case 5:
						glSprite(112, 167, GL_FLIP_NONE, &cursorImage[0]);
						glSprite(132, 167, GL_FLIP_NONE, &cursorImage[1]);
						glSprite(112, 182, GL_FLIP_NONE, &cursorImage[2]);
						glSprite(132, 182, GL_FLIP_NONE, &cursorImage[3]);
						break;
					case 6:
						glSprite(225, 167, GL_FLIP_NONE, &cursorImage[0]);
						glSprite(245, 167, GL_FLIP_NONE, &cursorImage[1]);
						glSprite(225, 182, GL_FLIP_NONE, &cursorImage[2]);
						glSprite(245, 182, GL_FLIP_NONE, &cursorImage[3]);
						break;
				}
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
	}
	glEnd2D();
	GFX_FLUSH = 0;
}

void loadBoxArt(const char* filename) {
	if(access(filename, F_OK) != 0) {
		switch (boxArtType) {
			case 0:
			default:
				filename = "nitro:/graphics/boxart_unknown.png";
				break;
			case 1:
				filename = "nitro:/graphics/boxart_unknown1.png";
				break;
			case 2:
				filename = "nitro:/graphics/boxart_unknown2.png";
				break;
			case 3:
				filename = "nitro:/graphics/boxart_unknown3.png";
				break;
		}
	}

	std::vector<unsigned char> image;
	uint imageXpos, imageYpos, imageWidth, imageHeight;
	lodepng::decode(image, imageWidth, imageHeight, filename);
	if(imageWidth > 256 || imageHeight > 192)
		return;

	for(uint i=0;i<image.size()/4;i++) {
		bmpImageBuffer[i] = image[i*4]>>3 | (image[(i*4)+1]>>3)<<5 | (image[(i*4)+2]>>3)<<10 | BIT(15);
		if (colorMode == 1) {
			bmpImageBuffer[i] = convertVramColorToGrayscale(bmpImageBuffer[i]);
		}
	}

	imageXpos = (256-imageWidth)/2;
	imageYpos = (192-imageHeight)/2;
	u16 *src = bmpImageBuffer;
	for(uint y = 0; y < imageHeight; y++) {
		for(uint x = 0; x < imageWidth; x++) {
			BG_GFX_SUB[(y+imageYpos) * 256 + imageXpos + x] = *(src++);
		}
	}
}

void topBgLoad(void) {
	std::vector<unsigned char> image;
	unsigned width, height;
	lodepng::decode(image, width, height, "nitro:/graphics/" + (isDSPhat() ? std::string("phat_") : std::string("")) + "topbg.png");
	for(unsigned i = 0; i < image.size(); i = i * 4) {
		bmpImageBuffer[i] = image[i] >> 3 | (image[i + 1]>>3)<<5 | (image[i + 2] >> 3) << 10 | BIT(15);
		if (colorMode == 1) {
			bmpImageBuffer[i] = convertVramColorToGrayscale(bmpImageBuffer[i]);
		}
	}
}

void topBarLoad(void) {
	char filePath[256];
	snprintf(filePath, sizeof(filePath), "nitro:/graphics/%s/%i.png", isDSPhat() ? "phat_topbar" : "topbar", PersonalData->theme);
	FILE* file = fopen(filePath, "rb");

	if (file) {
		// Start loading
		std::vector<unsigned char> image;
		unsigned width, height;
		lodepng::decode(image, width, height, filePath);
		for(unsigned i = 0; i < image.size(); i = i * 4) {
			bmpImageBuffer[i] = image[i] >> 3 | (image[i + 1]>>3)<<5 | (image[i + 2] >> 3) << 10 | BIT(15);
			if (colorMode == 1) {
				bmpImageBuffer[i] = convertVramColorToGrayscale(bmpImageBuffer[i]);
			}
		}
		u16* src = bmpImageBuffer;
		int x = 0;
		int y = 0;
		for (int i = 0; i < 256 * 16; i++) {
			if (x >= 256) {
				x = 0;
				y++;
			}
			u16 val = *(src++);
			BG_GFX_SUB[y*256+x] = val;
			x++;
		}
	}

	fclose(file);
}

void graphicsInit()
{
	*(u16*)(0x0400006C) |= BIT(14);
	*(u16*)(0x0400006C) &= BIT(15);
	SetBrightness(0, 31);
	SetBrightness(1, 31);

	////////////////////////////////////////////////////////////
	videoSetMode(MODE_5_3D | DISPLAY_BG3_ACTIVE);
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

	lcdMainOnBottom();
	
	REG_BG3CNT = BG_MAP_BASE(0) | BG_BMP16_256x256 | BG_PRIORITY(0);
	REG_BG3X = 0;
	REG_BG3Y = 0;
	REG_BG3PA = 1<<8;
	REG_BG3PB = 0;
	REG_BG3PC = 0;
	REG_BG3PD = 1<<8;

	REG_BG3CNT_SUB = BG_MAP_BASE(0) | BG_BMP16_256x256 | BG_PRIORITY(0);
	REG_BG3X_SUB = 0;
	REG_BG3Y_SUB = 0;
	REG_BG3PA_SUB = 1<<8;
	REG_BG3PB_SUB = 0;
	REG_BG3PC_SUB = 0;
	REG_BG3PD_SUB = 1<<8;

	if (isDSiMode()) {
		loadSdRemovedImage();
	}

	swiWaitForVBlank();

	u16* newPalette = (u16*)cursorPals+(PersonalData->theme*16);
	if (colorMode == 1) {
		// Convert palette to grayscale
		for (int i2 = 0; i2 < 3; i2++) {
			*(newPalette+i2) = convertVramColorToGrayscale(*(newPalette+i2));
		}
	}

	cursorTexID = glLoadTileSet(cursorImage, // pointer to glImage array
							32, // sprite width
							32, // sprite height
							32, // bitmap width
							128, // bitmap height
							GL_RGB16, // texture type for glTexImage2D() in videoGL.h
							TEXTURE_SIZE_32, // sizeX for glTexImage2D() in videoGL.h
							TEXTURE_SIZE_128, // sizeY for glTexImage2D() in videoGL.h
							TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
							3, // Length of the palette to use (3 colors)
							(u16*) newPalette, // Load our 16 color tiles palette
							(u8*) cursorBitmap // image data generated by GRIT
							);

	newPalette = (u16*)iconboxPal;
	if (colorMode == 1) {
		// Convert palette to grayscale
		for (int i2 = 0; i2 < 12; i2++) {
			*(newPalette+i2) = convertVramColorToGrayscale(*(newPalette+i2));
		}
	}

	iconboxTexID = glLoadTileSet(iconboxImage, // pointer to glImage array
							256, // sprite width
							64, // sprite height
							256, // bitmap width
							128, // bitmap height
							GL_RGB16, // texture type for glTexImage2D() in videoGL.h
							TEXTURE_SIZE_256, // sizeX for glTexImage2D() in videoGL.h
							TEXTURE_SIZE_128, // sizeY for glTexImage2D() in videoGL.h
							TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
							12, // Length of the palette to use (12 colors)
							(u16*) newPalette, // Load our 16 color tiles palette
							(u8*) iconboxBitmap // image data generated by GRIT
							);

	newPalette = (u16*)wirelessiconsPal;
	if (colorMode == 1) {
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

	newPalette = (u16*)pictodlpPal;
	if (colorMode == 1) {
		// Convert palette to grayscale
		for (int i2 = 0; i2 < 12; i2++) {
			*(newPalette+i2) = convertVramColorToGrayscale(*(newPalette+i2));
		}
	}

	pictodlpTexID = glLoadTileSet(pictodlpImage, // pointer to glImage array
							128, // sprite width
							64, // sprite height
							128, // bitmap width
							256, // bitmap height
							GL_RGB16, // texture type for glTexImage2D() in videoGL.h
							TEXTURE_SIZE_128, // sizeX for glTexImage2D() in videoGL.h
							TEXTURE_SIZE_256, // sizeY for glTexImage2D() in videoGL.h
							TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
							12, // Length of the palette to use (12 colors)
							(u16*) newPalette, // Load our 16 color tiles palette
							(u8*) pictodlpBitmap // image data generated by GRIT
							);

	newPalette = (u16*)icon_dscardPal;
	if (colorMode == 1) {
		// Convert palette to grayscale
		for (int i2 = 0; i2 < 16; i2++) {
			*(newPalette+i2) = convertVramColorToGrayscale(*(newPalette+i2));
		}
	}

	dscardiconTexID = glLoadTileSet(dscardIconImage, // pointer to glImage array
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
							(u8*) icon_dscardBitmap // image data generated by GRIT
							);

	newPalette = (u16*)(useGbarunner ? (isDSPhat() ? iconPhat_gbaPal : icon_gbaPal) : icon_gbamodePal);
	if (colorMode == 1) {
		// Convert palette to grayscale
		for (int i2 = 0; i2 < 16; i2++) {
			*(newPalette+i2) = convertVramColorToGrayscale(*(newPalette+i2));
		}
	}

	gbaiconTexID = glLoadTileSet(gbaIconImage, // pointer to glImage array
							32, // sprite width
							32, // sprite height
							32, // bitmap width
							32, // bitmap height
							GL_RGB16, // texture type for glTexImage2D() in videoGL.h
							TEXTURE_SIZE_32, // sizeX for glTexImage2D() in videoGL.h
							TEXTURE_SIZE_32, // sizeY for glTexImage2D() in videoGL.h
							TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
							16, // Length of the palette to use (16 colors)
							(u16*) newPalette, // Load our 16 color tiles palette
							(u8*) (useGbarunner ? (isDSPhat() ? iconPhat_gbaBitmap : icon_gbaBitmap) : icon_gbamodeBitmap) // image data generated by GRIT
							);

	newPalette = (u16*)cornericonsPal;
	if (colorMode == 1) {
		// Convert palette to grayscale
		for (int i2 = 0; i2 < 16; i2++) {
			*(newPalette+i2) = convertVramColorToGrayscale(*(newPalette+i2));
		}
	}

	cornericonTexID = glLoadTileSet(cornerIcons, // pointer to glImage array
							32, // sprite width
							32, // sprite height
							32, // bitmap width
							128, // bitmap height
							GL_RGB16, // texture type for glTexImage2D() in videoGL.h
							TEXTURE_SIZE_32, // sizeX for glTexImage2D() in videoGL.h
							TEXTURE_SIZE_128, // sizeY for glTexImage2D() in videoGL.h
							TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
							16, // Length of the palette to use (16 colors)
							(u16*) newPalette, // Load our 16 color tiles palette
							(u8*) cornericonsBitmap // image data generated by GRIT
							);

	newPalette = (u16*)icon_settingsPal;
	if (colorMode == 1) {
		// Convert palette to grayscale
		for (int i2 = 0; i2 < 16; i2++) {
			*(newPalette+i2) = convertVramColorToGrayscale(*(newPalette+i2));
		}
	}

	settingsiconTexID = glLoadTileSet(settingsIconImage, // pointer to glImage array
							32, // sprite width
							32, // sprite height
							32, // bitmap width
							32, // bitmap height
							GL_RGB16, // texture type for glTexImage2D() in videoGL.h
							TEXTURE_SIZE_32, // sizeX for glTexImage2D() in videoGL.h
							TEXTURE_SIZE_32, // sizeY for glTexImage2D() in videoGL.h
							TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
							16, // Length of the palette to use (16 colors)
							(u16*) newPalette, // Load our 16 color tiles palette
							(u8*) icon_settingsBitmap // image data generated by GRIT
							);

	loadConsoleIcons();

	irqSet(IRQ_VBLANK, vBlankHandler);
	irqEnable(IRQ_VBLANK);
}
