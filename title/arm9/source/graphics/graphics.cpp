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
#include "logo_rocketrobz.h"
#include "logo_rocketrobzbootstrap.h"
#include "font6x8.h"
#include "graphics.h"
#include "fontHandler.h"

#define CONSOLE_SCREEN_WIDTH 32
#define CONSOLE_SCREEN_HEIGHT 24

//extern int appName;

extern int screenmode;

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

void clearBrightness(void) {
	fadeType = true;
	screenBrightness = 0;
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
}

void LoadBMP(void) {
	FILE* file;
	switch (ms().appName) {
		case 0:
		default:
			file = fopen("nitro:/graphics/TWiLightMenu.bmp", "rb");
			break;
		case 1:
			file = fopen("nitro:/graphics/SRLoader.bmp", "rb");
			break;
		case 2:
			file = fopen("nitro:/graphics/DSiMenuPP.bmp", "rb");
			break;
	}

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
			BG_GFX[y*256+i] = ((val>>10)&0x1f) | ((val)&(0x1f<<5)) | (val&0x1f)<<10 | BIT(15);
		}
	}

	fclose(file);
}

void runGraphicIrq(void) {
	*(u16*)(0x0400006C) |= BIT(14);
	*(u16*)(0x0400006C) &= BIT(15);
	SetBrightness(0, 31);
	SetBrightness(1, 31);

	irqSet(IRQ_VBLANK, vBlankHandler);
	irqEnable(IRQ_VBLANK);
}

void loadTitleGraphics() {
	videoSetMode(MODE_3_2D | DISPLAY_BG3_ACTIVE);
	videoSetModeSub(MODE_0_2D | DISPLAY_BG0_ACTIVE);
	vramSetBankD(VRAM_D_MAIN_BG_0x06040000);
	vramSetBankC (VRAM_C_SUB_BG_0x06200000);
	REG_BG3CNT = BG_MAP_BASE(0) | BG_BMP16_256x256;
	REG_BG3X = 0;
	REG_BG3Y = 0;
	REG_BG3PA = 1<<8;
	REG_BG3PB = 0;
	REG_BG3PC = 0;
	REG_BG3PD = 1<<8;
	REG_BG0CNT_SUB = BG_MAP_BASE(0) | BG_COLOR_256 | BG_TILE_BASE(2);
	BG_PALETTE[0]=0;
	BG_PALETTE[255]=0xffff;
	u16* bgMapSub = (u16*)SCREEN_BASE_BLOCK_SUB(0);
	for (int i = 0; i < CONSOLE_SCREEN_WIDTH*CONSOLE_SCREEN_HEIGHT; i++) {
		bgMapSub[i] = (u16)i;
	}

	bool appNameChanged = false;

	scanKeys();

	if (keysHeld() & KEY_UP) {
		ms().appName = 0;
		appNameChanged = true;
	} else if (keysHeld() & KEY_DOWN) {
		ms().appName = 1;
		appNameChanged = true;
	} else if (keysHeld() & KEY_LEFT) {
		ms().appName = 2;
		appNameChanged = true;
	}
	
	if (appNameChanged) {
		ms().saveSettings();
	}

	// Display TWiLightMenu++ logo
	LoadBMP();
	if (isDSiMode()) {		// Show nds-bootstrap logo, if in DSi mode
		swiDecompressLZSSVram ((void*)logo_rocketrobzbootstrapTiles, (void*)CHAR_BASE_BLOCK_SUB(2), 0, &decompressBiosCallback);
		vramcpy_ui (&BG_PALETTE_SUB[0], logo_rocketrobzbootstrapPal, logo_rocketrobzbootstrapPalLen);
	} else {
		swiDecompressLZSSVram ((void*)logo_rocketrobzTiles, (void*)CHAR_BASE_BLOCK_SUB(2), 0, &decompressBiosCallback);
		vramcpy_ui (&BG_PALETTE_SUB[0], logo_rocketrobzPal, logo_rocketrobzPalLen);
	}
}