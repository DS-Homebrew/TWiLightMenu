#include <nds.h>
#include <fat.h>
#include <stdio.h>

extern u16 bmpImageBuffer[256*192];
extern u16 videoImageBuffer[40][256*144];

static char videoFrameFilename[256];

static FILE* videoFrameFile;

//static int currentFrame = 0;
static int frameDelay = 0;
static bool frameDelayEven = true;	// For 24FPS
static bool loadFrame = true;

void twlMenuVideo(void) {
	//dmaFillHalfWords(0, BG_GFX, 0x18000);

	for (int selectedFrame = 0; selectedFrame < 40; selectedFrame++) {
		if (selectedFrame < 10) {
			snprintf(videoFrameFilename, sizeof(videoFrameFilename), "nitro:/video/twlmenupp/frame0%i.bmp", selectedFrame);
		} else {
			snprintf(videoFrameFilename, sizeof(videoFrameFilename), "nitro:/video/twlmenupp/frame%i.bmp", selectedFrame);
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

		scanKeys();
		if ((keysHeld() & KEY_START) || (keysHeld() & KEY_SELECT)) return;
	}

	for (int i = 0; i < 40; i++) {
		while (1) {
			if (!loadFrame) {
				frameDelay++;
				loadFrame = (frameDelay == 2+frameDelayEven);
			}

			if (loadFrame) {
				dmaCopy((void*)videoImageBuffer[i], (u16*)BG_GFX+(256*22), 0x12000);

				//currentFrame++;
				//if (currentFrame > i) currentFrame = 0;
				frameDelay = 0;
				frameDelayEven = !frameDelayEven;
				loadFrame = false;
				break;
			}
			swiWaitForVBlank();
		}
		scanKeys();
		if ((keysHeld() & KEY_START) || (keysHeld() & KEY_SELECT)) return;
		swiWaitForVBlank();
	}

	for (int selectedFrame = 40; selectedFrame <= 43; selectedFrame++) {
		snprintf(videoFrameFilename, sizeof(videoFrameFilename), "nitro:/video/twlmenupp/frame%i.bmp", selectedFrame);
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
			dmaCopy((void*)videoImageBuffer[0], (u16*)BG_GFX+(256*22), 0x12000);
		}
		fclose(videoFrameFile);

		scanKeys();
		if ((keysHeld() & KEY_START) || (keysHeld() & KEY_SELECT)) return;
	}

	// Change TWL letters to user color
	snprintf(videoFrameFilename, sizeof(videoFrameFilename), "nitro:/graphics/TWL_%i.bmp", (int)PersonalData->theme);
	videoFrameFile = fopen(videoFrameFilename, "rb");

	if (videoFrameFile) {
		// Start loading
		fseek(videoFrameFile, 0xe, SEEK_SET);
		u8 pixelStart = (u8)fgetc(videoFrameFile) + 0xe;
		fseek(videoFrameFile, pixelStart, SEEK_SET);
		fread(bmpImageBuffer, 2, 0x800, videoFrameFile);
		u16* src = bmpImageBuffer;
		int x = 68;
		int y = 69;
		for (int i=0; i<62*14; i++) {
			if (x >= 130) {
				x = 68;
				y--;
			}
			u16 val = *(src++);
			if (val != 0x7C1F) {
				BG_GFX[(y+22)*256+x] = ((val>>10)&0x1f) | ((val)&(0x1f<<5)) | (val&0x1f)<<10 | BIT(15);
			}
			x++;
		}
	}
	fclose(videoFrameFile);

	for (int i = 0; i < 60 * 2; i++)
	{
		scanKeys();
		if ((keysHeld() & KEY_START) || (keysHeld() & KEY_SELECT)) return;
		swiWaitForVBlank();
	}
}