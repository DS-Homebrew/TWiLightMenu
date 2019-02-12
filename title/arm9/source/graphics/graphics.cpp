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
#include <nds/dma.h>
#include <maxmod9.h>
#include "bios_decompress_callback.h"
#include "common/dsimenusettings.h"
#include "common/gl2d.h"
#include "graphics.h"

#define CONSOLE_SCREEN_WIDTH 32
#define CONSOLE_SCREEN_HEIGHT 24

extern bool fadeType;
bool controlTopBright = true;
int screenBrightness = 31;

bool rocketVideo_playVideo = false;
int rocketVideo_videoYpos = 0;
int rocketVideo_videoFrames = 0;
int rocketVideo_videoFps = 60;
int rocketVideo_currentFrame = -1;
int rocketVideo_frameDelay = 0;
bool rocketVideo_frameDelayEven = true;	// For 24FPS
bool rocketVideo_loadFrame = true;

bool twlMenuSplash = false;
extern void twlMenuVideo_loadTopGraphics(void);
extern void twlMenuVideo_topGraphicRender(void);

u16 bmpImageBuffer[256*192];
u16 videoImageBuffer[39][256*144];
void* dsiSplashLocation = (void*)0x02600000;

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
	if (controlTopBright) SetBrightness(0, screenBrightness);
	SetBrightness(1, screenBrightness);
	if (twlMenuSplash) {
		twlMenuVideo_topGraphicRender();
	}
	if (rocketVideo_playVideo) {
		if (!rocketVideo_loadFrame) {
			if (rocketVideo_videoFps != 60) {
				rocketVideo_frameDelay++;
				rocketVideo_loadFrame = (rocketVideo_frameDelay == 2+rocketVideo_frameDelayEven);
			} else {
				rocketVideo_loadFrame = true;
			}
		}

		if (rocketVideo_loadFrame) {
			rocketVideo_currentFrame++;

			if (rocketVideo_currentFrame > rocketVideo_videoFrames) {
				rocketVideo_playVideo = false;
				rocketVideo_currentFrame = -1;
				rocketVideo_frameDelay = 0;
				rocketVideo_frameDelayEven = true;
				rocketVideo_loadFrame = false;
			} else {
				if (rocketVideo_videoFps == 60) {
					dmaCopy(dsiSplashLocation+(0x12000*rocketVideo_currentFrame), (u16*)BG_GFX+(256*rocketVideo_videoYpos), 0x12000);
				} else {
					dmaCopy((void*)videoImageBuffer[rocketVideo_currentFrame % 39], (u16*)BG_GFX+(256*rocketVideo_videoYpos), 0x12000);
				}
				rocketVideo_frameDelay = 0;
				rocketVideo_frameDelayEven = !rocketVideo_frameDelayEven;
				rocketVideo_loadFrame = false;
			}
		}
	}
}

void LoadBMP(void) {
	dmaFillHalfWords(0, BG_GFX, 0x18000);

	FILE* file = fopen("nitro:/video/twlmenupp/dsi.bmp", "rb");

	if (file) {
		// Start loading
		fseek(file, 0xe, SEEK_SET);
		u8 pixelStart = (u8)fgetc(file) + 0xe;
		fseek(file, pixelStart, SEEK_SET);
		fread(bmpImageBuffer, 2, 0x14000, file);
		u16* src = bmpImageBuffer;
		int x = 0;
		int y = 143;
		for (int i=0; i<256*144; i++) {
			if (x >= 256) {
				x = 0;
				y--;
			}
			u16 val = *(src++);
			BG_GFX[(y+24)*256+x] = ((val>>10)&0x1f) | ((val)&(0x1f<<5)) | (val&0x1f)<<10 | BIT(15);
			x++;
		}
	}

	fclose(file);

	if (ms().useBootstrap || isDSiMode()) {		// Show nds-bootstrap logo, if nds-bootstrap is set to be used
		file = fopen("nitro:/graphics/logo_rocketrobzbootstrap.bmp", "rb");
	} else {
		file = fopen("nitro:/graphics/logo_rocketrobz.bmp", "rb");
	}

	if (file) {
		// Start loading
		fseek(file, 0xe, SEEK_SET);
		u8 pixelStart = (u8)fgetc(file) + 0xe;
		fseek(file, pixelStart, SEEK_SET);
		fread(bmpImageBuffer, 2, 0x18000, file);
		u16* src = bmpImageBuffer;
		int x = 0;
		int y = 191;
		for (int i=0; i<256*192; i++) {
			if (x >= 256) {
				x = 0;
				y--;
			}
			u16 val = *(src++);
			BG_GFX_SUB[y*256+x] = ((val>>10)&0x1f) | ((val)&(0x1f<<5)) | (val&0x1f)<<10 | BIT(15);
			x++;
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
	videoSetMode(MODE_5_3D | DISPLAY_BG3_ACTIVE);
	videoSetModeSub(MODE_3_2D | DISPLAY_BG3_ACTIVE);

	// Initialize gl2d
	glScreen2D();
	// Make gl2d render on transparent stage.
	glClearColor(31,31,31,0);
	glDisable(GL_CLEAR_BMP);

	// Clear the GL texture state
	glResetTextures();

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

	twlMenuVideo_loadTopGraphics();

	// Display TWiLightMenu++ logo
	LoadBMP();
}