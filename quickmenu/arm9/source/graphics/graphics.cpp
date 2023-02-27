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
#include <nds/arm9/dldi.h>
#include <maxmod9.h>
#include <gl2d.h>
#include "bios_decompress_callback.h"
#include "myDSiMode.h"
#include "common/flashcard.h"
#include "common/systemdetails.h"
#include "common/twlmenusettings.h"

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
#include "common/lodepng.h"
#include "fontHandler.h"
#include "../ndsheaderbanner.h"
#include "../errorScreen.h"

#define CONSOLE_SCREEN_WIDTH 32
#define CONSOLE_SCREEN_HEIGHT 24

extern bool useTwlCfg;

//extern bool widescreenEffects;

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

int frameOf60fps = 60;
int frameDelay = 0;
bool frameDelayEven = true; // For 24FPS
bool renderFrame = true;

bool showProgressBar = false;
int progressBarLength = 0;

extern int spawnedtitleboxes;

extern bool showCursor;
extern bool startMenu;
extern int cursorPosition;

extern bool pictochatFound;
extern bool dlplayFound;
extern bool gbaBiosFound;
extern bool cardEjected;

int boxArtType[2] = {0};

bool moveIconUp[7] = {false};
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

u16 bmpImageBuffer[256*192] = {0};
u16 topImageBuffer[256*192] = {0};

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
	*(vu16*)(0x0400006C + (0x1000 * screen)) = bright + mode;
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

int getFavoriteColor(void) {
	int favoriteColor = (int)(useTwlCfg ? *(u8*)0x02000444 : PersonalData->theme);
	if (favoriteColor < 0 || favoriteColor >= 16) favoriteColor = 0; // Invalid color found, so default to gray
	return favoriteColor;
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
	std::string bottomBGFile = "nitro:/graphics/bottombg.png";

	char temp[256];

	switch (ms().theme) {
		case TWLSettings::EThemeDSi: // DSi Theme
			sprintf(temp, "%s:/_nds/TwilightMenu/dsimenu/themes/%s/quickmenu/bottombg.png", sdFound() ? "sd" : "fat", ms().dsi_theme.c_str());
			break;
		case TWLSettings::ETheme3DS:
			sprintf(temp, "%s:/_nds/TwilightMenu/3dsmenu/themes/%s/quickmenu/bottombg.png", sdFound() ? "sd" : "fat", ms()._3ds_theme.c_str());
			break;
		case TWLSettings::EThemeR4:
			sprintf(temp, "%s:/_nds/TwilightMenu/r4menu/themes/%s/quickmenu/bottombg.png", sdFound() ? "sd" : "fat", ms().r4_theme.c_str());
			break;
		case TWLSettings::EThemeWood:
			// sprintf(temp, "%s:/_nds/TwilightMenu/akmenu/themes/%s/quickmenu/bottombg.png", sdFound() ? "sd" : "fat", ms().ak_theme.c_str());
			break;
		case TWLSettings::EThemeSaturn:
			sprintf(temp, "nitro:/graphics/bottombg_saturn.png");
			break;
		case TWLSettings::EThemeHBL:
		case TWLSettings::EThemeGBC:
			break;
	}

	if (access(temp, F_OK) == 0)
		bottomBGFile = std::string(temp);

	std::vector<unsigned char> image;
	uint imageWidth, imageHeight;
	lodepng::decode(image, imageWidth, imageHeight, bottomBGFile);
	if (imageWidth > 256 || imageHeight > 192)	return;

	for (uint i=0;i<image.size()/4;i++) {
		bmpImageBuffer[i] = image[i*4]>>3 | (image[(i*4)+1]>>3)<<5 | (image[(i*4)+2]>>3)<<10 | BIT(15);
		if (ms().colorMode == 1) {
			bmpImageBuffer[i] = convertVramColorToGrayscale(bmpImageBuffer[i]);
		}
	}

	// Start loading
	u16* src = bmpImageBuffer;
	int x = 0;
	int y = 0;
	for (int i=0; i<256*192; i++) {
		if (x >= 256) {
			x = 0;
			y++;
		}
		BG_GFX[y*256+x] = *(src++);
		x++;
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
	  glBegin2D();
	  {
		if (controlBottomBright) SetBrightness(0, screenBrightness);
		if (controlTopBright && !ms().macroMode) SetBrightness(1, screenBrightness);

		// glColor(RGB15(31, 31-(3*ms().blfLevel), 31-(6*ms().blfLevel)));
		glColor(RGB15(31, 31, 31));
		if (whiteScreen) {
			glBoxFilled(0, 0, 256, 192, RGB15(31, 31, 31));
			if (showProgressBar) {
				int barXpos = 31;
				int barYpos = 169;
				glBoxFilled(barXpos, barYpos, barXpos+192, barYpos+5, RGB15(23, 23, 23));
				if (progressBarLength > 0) {
					glBoxFilled(barXpos, barYpos, barXpos+progressBarLength, barYpos+5, RGB15(0, 0, 31));
				}
			}
		} else if (startMenu) {
			if (isDSiMode() && cardEjected) {
				//glSprite(33, iconYpos[0], GL_FLIP_NONE, &iconboxImage[(REG_SCFG_MC == 0x11) ? 1 : 0]);
				//glSprite(40, iconYpos[0]+6, GL_FLIP_NONE, &dscardIconImage[(REG_SCFG_MC == 0x11) ? 1 : 0]);
				glSprite(33, iconYpos[0], GL_FLIP_NONE, &iconboxImage[1]);
				glSprite(40, iconYpos[0]+6, GL_FLIP_NONE, &dscardIconImage[1]);
			} else {
				glSprite(33, iconYpos[0], GL_FLIP_NONE, &iconboxImage[0]);
				if ((isDSiMode() && !flashcardFound() && sys().arm7SCFGLocked()) || (io_dldi_data->ioInterface.features & FEATURE_SLOT_GBA)) {
					glSprite(40, iconYpos[0]+6, GL_FLIP_NONE, &dscardIconImage[0]);
				} else if (customIcon[1]) drawIcon(1, 40, iconYpos[0]+6);
				else if (bnrRomType[1] == 20) drawIconIMG(40, iconYpos[0]+6);
				else if (bnrRomType[1] == 19) drawIconVID(40, iconYpos[0]+6);
				else if (bnrRomType[1] == 18) drawIconCPC(40, iconYpos[0]+6);
				else if (bnrRomType[1] == 17) drawIconNGP(40, iconYpos[0]+6);
				else if (bnrRomType[1] == 16) drawIconWS(40, iconYpos[0]+6);
				else if (bnrRomType[1] == 15) drawIconSG(40, iconYpos[0]+6);
				else if (bnrRomType[1] == 14) drawIconM5(40, iconYpos[0]+6);
				else if (bnrRomType[1] == 13) drawIconCOL(40, iconYpos[0]+6);
				else if (bnrRomType[1] == 12) drawIconINT(40, iconYpos[0]+6);
				else if (bnrRomType[1] == 11) drawIconPCE(40, iconYpos[0]+6);
				else if (bnrRomType[1] == 10) drawIconA26(40, iconYpos[0]+6);
				else if (bnrRomType[1] == 9) drawIconPlg(40, iconYpos[0]+6);
				else if (bnrRomType[1] == 8) drawIconSNES(40, iconYpos[0]+6);
				else if (bnrRomType[1] == 7) drawIconMD(40, iconYpos[0]+6);
				else if (bnrRomType[1] == 6) drawIconGG(40, iconYpos[0]+6);
				else if (bnrRomType[1] == 5) drawIconSMS(40, iconYpos[0]+6);
				else if (bnrRomType[1] == 4) drawIconNES(40, iconYpos[0]+6);
				else if (bnrRomType[1] == 3) drawIconGBC(40, iconYpos[0]+6);
				else if (bnrRomType[1] == 2) drawIconGB(40, iconYpos[0]+6);
				else if (bnrRomType[1] == 1) glSprite(40, iconYpos[0]+6, GL_FLIP_NONE, gbaIconImage);
				else drawIcon(1, 40, iconYpos[0]+6);
			}
			if (bnrWirelessIcon[0] > 0) glSprite(207, iconYpos[0]+30, GL_FLIP_NONE, &wirelessIcons[(bnrWirelessIcon[0]-1) & 31]);
			// Playback animated icon
			if (bnriconisDSi[0]==true) {
				playBannerSequence(0);
			}
			glSprite(33, iconYpos[1], GL_FLIP_NONE, &pictodlpImage[1-pictochatFound]);
			glSprite(129, iconYpos[2], GL_FLIP_NONE, &pictodlpImage[3-dlplayFound]);
			glSprite(33, iconYpos[3], GL_FLIP_NONE, sdFound() ? &iconboxImage[0] : &iconboxImage[sys().isRegularDS() ? (((u8*)GBAROM)[0xB2] != 0x96) : 1]);
			int num = (io_dldi_data->ioInterface.features & FEATURE_SLOT_GBA) ? 1 : 0;
			if (!sdFound() && (io_dldi_data->ioInterface.features & FEATURE_SLOT_NDS)) drawIconGBA(40, iconYpos[3]+6);
			else if (customIcon[num]) drawIcon(num, 40, iconYpos[3]+6);
			else if (bnrRomType[num] == 20) drawIconIMG(40, iconYpos[3]+6);
			else if (bnrRomType[num] == 19) drawIconVID(40, iconYpos[3]+6);
			else if (bnrRomType[num] == 18) drawIconCPC(40, iconYpos[3]+6);
			else if (bnrRomType[num] == 17) drawIconNGP(40, iconYpos[3]+6);
			else if (bnrRomType[num] == 16) drawIconWS(40, iconYpos[3]+6);
			else if (bnrRomType[num] == 15) drawIconSG(40, iconYpos[3]+6);
			else if (bnrRomType[num] == 14) drawIconM5(40, iconYpos[3]+6);
			else if (bnrRomType[num] == 13) drawIconCOL(40, iconYpos[3]+6);
			else if (bnrRomType[num] == 12) drawIconINT(40, iconYpos[3]+6);
			else if (bnrRomType[num] == 11) drawIconPCE(40, iconYpos[3]+6);
			else if (bnrRomType[num] == 10) drawIconA26(40, iconYpos[3]+6);
			else if (bnrRomType[num] == 9) drawIconPlg(40, iconYpos[3]+6);
			else if (bnrRomType[num] == 8) drawIconSNES(40, iconYpos[3]+6);
			else if (bnrRomType[num] == 7) drawIconMD(40, iconYpos[3]+6);
			else if (bnrRomType[num] == 6) drawIconGG(40, iconYpos[3]+6);
			else if (bnrRomType[num] == 5) drawIconSMS(40, iconYpos[3]+6);
			else if (bnrRomType[num] == 4) drawIconNES(40, iconYpos[3]+6);
			else if (bnrRomType[num] == 3) drawIconGBC(40, iconYpos[3]+6);
			else if (bnrRomType[num] == 2) drawIconGB(40, iconYpos[3]+6);
			else if (bnrRomType[num] == 1) glSprite(40, iconYpos[3]+6, GL_FLIP_NONE, gbaIconImage);
			else drawIcon(num, 40, iconYpos[3]+6);
			if (sys().isDSLite() || (dsiFeatures() && ms().consoleModel < 2)) {
				glSprite(10, iconYpos[4], GL_FLIP_NONE, &cornerIcons[0]);
			}
			if (bnrWirelessIcon[1] > 0) glSprite(207, iconYpos[3]+30, GL_FLIP_NONE, &wirelessIcons[(bnrWirelessIcon[1]-1) & 31]);
			// Playback animated icon
			if (bnriconisDSi[1]==true) {
				playBannerSequence(1);
			}
			if (!ms().kioskMode) {
				glSprite(117, iconYpos[5], GL_FLIP_NONE, settingsIconImage);
			}
			glSprite(235, iconYpos[6], GL_FLIP_NONE, &cornerIcons[3]);

			// Draw cursor
			if (showCursor) {
				switch (cursorPosition) {
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
		/*if (showdialogbox) {
			glBoxFilled(15, 79, 241, 129+(dialogboxHeight*8), RGB15(0, 0, 0));
			glBoxFilledGradient(16, 80, 240, 94, RGB15(0, 0, 31), RGB15(0, 0, 15), RGB15(0, 0, 15), RGB15(0, 0, 31));
			glBoxFilled(16, 96, 240, 128+(dialogboxHeight*8), RGB15(31, 31, 31));
		}*/
	  }
	  glEnd2D();
	  GFX_FLUSH = 0;

		frameDelay = 0;
		frameDelayEven = !frameDelayEven;
		renderFrame = false;
	}

	if (!whiteScreen) {
		for (int i = 0; i < 7; i++) {
			if (moveIconUp[i]) {
				iconYpos[i] -= 6;
			}
		}
	}
}

void loadBoxArt(const char* filename, bool secondaryDevice) {
	if (ms().macroMode) return;

	if (access(filename, F_OK) != 0) {
		switch (boxArtType[secondaryDevice]) {
			case -1:
				return;
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
	if (imageWidth > 256 || imageHeight > 192)	return;

	for (uint i=0;i<image.size()/4;i++) {
		bmpImageBuffer[i] = image[i*4]>>3 | (image[(i*4)+1]>>3)<<5 | (image[(i*4)+2]>>3)<<10 | BIT(15);
		if (ms().colorMode == 1) {
			bmpImageBuffer[i] = convertVramColorToGrayscale(bmpImageBuffer[i]);
		}
	}

	// Re-load top BG (excluding top bar)
	u16* src = topImageBuffer+(256*16);
	int x = 0;
	int y = 16;
	for (int i=256*16; i<256*192; i++) {
		if (x >= 256) {
			x = 0;
			y++;
		}
		BG_GFX_SUB[y*256+x] = *(src++);
		x++;
	}

	imageXpos = (256-imageWidth)/2;
	imageYpos = (192-imageHeight)/2;
	src = bmpImageBuffer;
	for (uint y = 0; y < imageHeight; y++) {
		for (uint x = 0; x < imageWidth; x++) {
			BG_GFX_SUB[(y+imageYpos) * 256 + imageXpos + x] = *(src++);
		}
	}
}

void topBgLoad(void) {
	if (ms().macroMode) return;

	char filePath[256];
	sprintf(filePath, "nitro:/graphics/%s.png", sys().isDSPhat() ? "phat_topbg" : "topbg");

	char temp[256];

	switch (ms().theme) {
		case TWLSettings::EThemeDSi: // DSi Theme
			sprintf(temp, "%s:/_nds/TwilightMenu/dsimenu/themes/%s/quickmenu/%s.png", sdFound() ? "sd" : "fat", ms().dsi_theme.c_str(), sys().isDSPhat() ? "phat_topbg" : "topbg");
			break;
		case TWLSettings::ETheme3DS:
			sprintf(temp, "%s:/_nds/TwilightMenu/3dsmenu/themes/%s/quickmenu/%s.png", sdFound() ? "sd" : "fat", ms()._3ds_theme.c_str(), sys().isDSPhat() ? "phat_topbg" : "topbg");
			break;
		case TWLSettings::EThemeR4:
			sprintf(temp, "%s:/_nds/TwilightMenu/r4menu/themes/%s/quickmenu/%s.png", sdFound() ? "sd" : "fat", ms().r4_theme.c_str(), sys().isDSPhat() ? "phat_topbg" : "topbg");
			break;
		case TWLSettings::EThemeWood:
			// sprintf(temp, "%s:/_nds/TwilightMenu/akmenu/themes/%s/quickmenu/%s.png", sdFound() ? "sd" : "fat", ms().ak_theme.c_str(), sys().isDSPhat() ? "phat_topbg" : "topbg");
			break;
		case TWLSettings::EThemeSaturn:
			sprintf(temp, "nitro:/graphics/%s.png", sys().isDSPhat() ? "phat_topbg_saturn" : "topbg_saturn");
			break;
		case TWLSettings::EThemeHBL:
		case TWLSettings::EThemeGBC:
			break;
	}

	if (access(temp, F_OK) == 0)
		sprintf(filePath, "%s", temp);

	std::vector<unsigned char> image;
	uint imageWidth, imageHeight;
	lodepng::decode(image, imageWidth, imageHeight, std::string(filePath));
	if (imageWidth > 256 || imageHeight > 192)	return;

	for (uint i=0;i<image.size()/4;i++) {
		topImageBuffer[i] = image[i*4]>>3 | (image[(i*4)+1]>>3)<<5 | (image[(i*4)+2]>>3)<<10 | BIT(15);
		if (ms().colorMode == 1) {
			topImageBuffer[i] = convertVramColorToGrayscale(topImageBuffer[i]);
		}
	}

	// Start loading
	u16* src = topImageBuffer;
	int x = 0;
	int y = 0;
	for (int i=0; i<256*192; i++) {
		if (x >= 256) {
			x = 0;
			y++;
		}
		BG_GFX_SUB[y*256+x] = *(src++);
		x++;
	}
}

void topBarLoad(void) {
	if (ms().macroMode) return;

	char filePath[256];
	snprintf(filePath, sizeof(filePath), "nitro:/graphics/%s/%i.png", sys().isDSPhat() ? "phat_topbar" : "topbar", getFavoriteColor());
	FILE* file = fopen(filePath, "rb");

	if (file) {
		// Start loading
		std::vector<unsigned char> image;
		unsigned width, height;
		lodepng::decode(image, width, height, filePath);
		for (unsigned i=0;i<image.size()/4;i++) {
			bmpImageBuffer[i] = image[i*4]>>3 | (image[(i*4)+1]>>3)<<5 | (image[(i*4)+2]>>3)<<10 | BIT(15);
			if (ms().colorMode == 1) {
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
			BG_GFX_SUB[y*256+x] = *(src++);
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
	videoSetMode(MODE_5_3D);
	videoSetModeSub(MODE_5_2D);

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
	vramSetBankA(VRAM_A_MAIN_BG);
	vramSetBankB(VRAM_B_MAIN_BG);
	vramSetBankC(VRAM_C_SUB_BG);
	vramSetBankD(VRAM_D_TEXTURE);
	vramSetBankE(VRAM_E_TEX_PALETTE);
	vramSetBankF(VRAM_F_TEX_PALETTE_SLOT4);
	vramSetBankG(VRAM_G_TEX_PALETTE_SLOT5); // 16Kb of palette ram, and font textures take up 8*16 bytes.
	vramSetBankH(VRAM_H_SUB_BG_EXT_PALETTE);
	vramSetBankI(VRAM_I_SUB_SPRITE_EXT_PALETTE);

	lcdMainOnBottom();
	
	int bg3Main = bgInit(3, BgType_Bmp16, BgSize_B16_256x256, 0, 0);
	bgSetPriority(bg3Main, 3);

	int bg2Main = bgInit(2, BgType_Bmp8, BgSize_B8_256x256, 7, 0);
	bgSetPriority(bg2Main, 0);

	int bg3Sub = bgInitSub(3, BgType_Bmp16, BgSize_B16_256x256, 0, 0);
	bgSetPriority(bg3Sub, 3);

	// int bg2Sub = bgInitSub(2, BgType_Bmp8, BgSize_B8_256x256, 3, 0);
	// bgSetPriority(bg2Sub, 0);

	bgSetPriority(0, 1); // Set 3D to below text

	/*if (widescreenEffects) {
		// Add black bars to left and right sides
		s16 c = cosLerp(0) >> 4;
		REG_BG3PA_SUB = ( c * 315)>>8;
		REG_BG3X_SUB = -29 << 8;
	}*/

	swiWaitForVBlank();

	u16* newPalette = (u16*)cursorPals+(getFavoriteColor()*16);
	if (ms().colorMode == 1) {
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
	if (ms().colorMode == 1) {
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

	newPalette = (u16*)pictodlpPal;
	if (ms().colorMode == 1) {
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
	if (ms().colorMode == 1) {
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

	newPalette = (u16*)(sys().isDSPhat() ? iconPhat_gbaPal : icon_gbaPal);
	if (ms().colorMode == 1) {
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
							(u8*) (sys().isDSPhat() ? iconPhat_gbaBitmap : icon_gbaBitmap) // image data generated by GRIT
							);

	newPalette = (u16*)cornericonsPal;
	if (ms().colorMode == 1) {
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
	if (ms().colorMode == 1) {
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
	irqSet(IRQ_VCOUNT, frameRateHandler);
	irqEnable(IRQ_VCOUNT);
}
