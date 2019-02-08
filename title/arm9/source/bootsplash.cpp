#include <nds.h>
#include <fat.h>
#include <stdio.h>
#include <maxmod9.h>

#include "soundbank.h"
#include "soundbank_bin.h"

extern u16 bmpImageBuffer[256*192];
extern u16 videoImageBuffer[40][256*144];

extern bool fadeType;

static char videoFrameFilename[256];

static FILE* videoFrameFile;

//static int currentFrame = 0;
static int frameDelay = 0;
static bool frameDelayEven = true;	// For 24FPS
static bool loadFrame = true;
 
#include "bootsplash.h"

#define CONSOLE_SCREEN_WIDTH 32
#define CONSOLE_SCREEN_HEIGHT 24

void BootJingleDSi() {
	
	mmInitDefaultMem((mm_addr)soundbank_bin);

	mmLoadEffect( SFX_DSIBOOT );

	mm_sound_effect dsiboot = {
		{ SFX_DSIBOOT } ,			// id
		(int)(1.0f * (1<<10)),	// rate
		0,		// handle
		255,	// volume
		128,	// panning
	};
	
	mmEffectEx(&dsiboot);
}

void BootSplashDSi(void) {

	u16 whiteCol = 0xFFFF;
	for (int i = 0; i < 256*192; i++) {
		BG_GFX[i] = ((whiteCol>>10)&0x1f) | ((whiteCol)&(0x1f<<5)) | (whiteCol&0x1f)<<10 | BIT(15);
	}

	fadeType = true;

	for (int selectedFrame = 0; selectedFrame < 40; selectedFrame++) {
		if (selectedFrame < 10) {
			snprintf(videoFrameFilename, sizeof(videoFrameFilename), "nitro:/video/dsisplash/frame0%i.bmp", selectedFrame);
		} else {
			snprintf(videoFrameFilename, sizeof(videoFrameFilename), "nitro:/video/dsisplash/frame%i.bmp", selectedFrame);
		}
		videoFrameFile = fopen(videoFrameFilename, "rb");

		if (videoFrameFile) {
			// Start loading
			fseek(videoFrameFile, 0xe, SEEK_SET);
			u8 pixelStart = (u8)fgetc(videoFrameFile) + 0xe;
			fseek(videoFrameFile, pixelStart, SEEK_SET);
			fread(bmpImageBuffer, 2, 0x14000, videoFrameFile);
			u16* src = bmpImageBuffer;
			int x = 0;
			int y = 143;
			for (int i=0; i<256*144; i++) {
				if (x >= 256) {
					x = 0;
					y--;
				}
				u16 val = *(src++);
				videoImageBuffer[selectedFrame][y*256+x] = ((val>>10)&0x1f) | ((val)&(0x1f<<5)) | (val&0x1f)<<10 | BIT(15);
				x++;
			}
		}
		fclose(videoFrameFile);

		//scanKeys();
		//if ((keysHeld() & KEY_START) || (keysHeld() & KEY_SELECT)) return;
	}

	for (int i = 0; i < 40; i++) {
		while (1) {
			if (!loadFrame) {
				frameDelay++;
				loadFrame = (frameDelay == 2+frameDelayEven);
			}

			if (loadFrame) {
				dmaCopy((void*)videoImageBuffer[i], (u16*)BG_GFX+(256*12), 0x12000);

				//currentFrame++;
				//if (currentFrame > i) currentFrame = 0;
				frameDelay = 0;
				frameDelayEven = !frameDelayEven;
				loadFrame = false;
				break;
			}
			swiWaitForVBlank();
		}
		if (i == 9) BootJingleDSi();
		scanKeys();
		if ((keysHeld() & KEY_START) || (keysHeld() & KEY_SELECT)) return;
		swiWaitForVBlank();
	}

	for (int selectedFrame = 40; selectedFrame <= 42; selectedFrame++) {
		snprintf(videoFrameFilename, sizeof(videoFrameFilename), "nitro:/video/dsisplash/frame%i.bmp", selectedFrame);
		videoFrameFile = fopen(videoFrameFilename, "rb");

		if (videoFrameFile) {
			// Start loading
			fseek(videoFrameFile, 0xe, SEEK_SET);
			u8 pixelStart = (u8)fgetc(videoFrameFile) + 0xe;
			fseek(videoFrameFile, pixelStart, SEEK_SET);
			fread(bmpImageBuffer, 2, 0x14000, videoFrameFile);
			u16* src = bmpImageBuffer;
			int x = 0;
			int y = 143;
			for (int i=0; i<256*144; i++) {
				if (x >= 256) {
					x = 0;
					y--;
				}
				u16 val = *(src++);
				videoImageBuffer[0][y*256+x] = ((val>>10)&0x1f) | ((val)&(0x1f<<5)) | (val&0x1f)<<10 | BIT(15);
				x++;
			}
			dmaCopy((void*)videoImageBuffer[0], (u16*)BG_GFX+(256*12), 0x12000);
		}
		fclose(videoFrameFile);

		scanKeys();
		if ((keysHeld() & KEY_START) || (keysHeld() & KEY_SELECT)) return;
	}

	swiWaitForVBlank();
	for (int i = 0; i < 256*60; i++) {
		BG_GFX[i] = ((whiteCol>>10)&0x1f) | ((whiteCol)&(0x1f<<5)) | (whiteCol&0x1f)<<10 | BIT(15);
	}

	// Pause on frame 31 for a second		
	for (int i = 0; i < 80; i++) { swiWaitForVBlank(); }

	// Fade out
	fadeType = false;
	for (int i = 0; i < 30; i++) { swiWaitForVBlank(); }
}

void BootSplashInit(void) {

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
	BG_PALETTE_SUB[0]=0x7fff;
	u16* bgMapSub = (u16*)SCREEN_BASE_BLOCK_SUB(0);
	for (int i = 0; i < CONSOLE_SCREEN_WIDTH*CONSOLE_SCREEN_HEIGHT; i++) {
		bgMapSub[i] = (u16)i;
	}

	BootSplashDSi();
}

