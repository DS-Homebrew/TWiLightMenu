#include <nds.h>
#include <stdio.h>
#include <maxmod9.h>

#include "autoboot.h"
#include "graphics/lodepng.h"
#include "graphics/fontHandler.h"

extern bool sdRemoveDetect;

extern const char *unlaunchAutoLoadID;
extern char unlaunchDevicePath[256];
extern void unlaunchSetHiyaBoot();

extern bool arm7SCFGLocked;

extern int consoleModel;
extern int launcherApp;

extern u16 bmpImageBuffer[256*192];
u16* sdRemovedExtendedImage = (u16*)0x026C8000;
u16* sdRemovedImage = (u16*)0x026E0000;

extern u16 convertToDsBmp(u16 val);

static int timeTillChangeToNonExtendedImage = 0;
static bool showNonExtendedImage = false;

void loadSdRemovedImage(void) {
	//FILE* file = fopen((arm7SCFGLocked ? "nitro:/graphics/sdRemovedSimple.bmp" : "nitro:/graphics/sdRemoved.bmp"), "rb");
	std::vector<unsigned char> image;
	unsigned width, height;
	unsigned error = lodepng::decode(image, width, height, "nitro:/graphics/sdRemovedError.png");
	if(error)	printSmallCentered(false, 30, "Error");
	for(unsigned i = 0; i < image.size(); i = i * 4) {
  		sdRemovedExtendedImage[i] = image[i]>>3 | (image[i + 1]>>3)<<5 | (image[i + 2]>>3)<<10 | BIT(15);
	}

	lodepng::decode(image, width, height, "nitro:/graphics/sdRemoved.png");
	for(unsigned i = 0; i < image.size(); i = i * 4) {
  		sdRemovedImage[i] = image[i]>>3 | (image[i + 1]>>3)<<5 | (image[i + 2]>>3)<<10 | BIT(15);
	}
}

void checkSdEject(void) {
	if (!sdRemoveDetect) return;

	if (*(u8*)(0x023FF002) == 0 || !isDSiMode()) {
		timeTillChangeToNonExtendedImage++;
		if (timeTillChangeToNonExtendedImage > 10) {
			showNonExtendedImage = true;
			timeTillChangeToNonExtendedImage = 10;
		}
		return;
	}
	
	// Show "SD removed" screen
	mmEffectCancelAll();

	videoSetMode(MODE_3_2D | DISPLAY_BG3_ACTIVE);
	videoSetModeSub(MODE_3_2D | DISPLAY_BG3_ACTIVE);

	REG_BLDY = 0;

	dmaCopyWordsAsynch(0, (showNonExtendedImage ? sdRemovedImage : sdRemovedExtendedImage), BG_GFX, 0x18000);
	dmaFillWords(0, BG_GFX_SUB, 0x18000);

	while(1) {
		/*scanKeys();
		if (keysDown() & KEY_B) {
			if (consoleModel < 2 && launcherApp != -1) {
				memcpy((u8*)0x02000800, unlaunchAutoLoadID, 12);
				*(u16*)(0x0200080C) = 0x3F0;		// Unlaunch Length for CRC16 (fixed, must be 3F0h)
				*(u16*)(0x0200080E) = 0;			// Unlaunch CRC16 (empty)
				*(u32*)(0x02000810) = (BIT(0) | BIT(1));		// Load the title at 2000838h
																// Use colors 2000814h
				*(u16*)(0x02000814) = 0x7FFF;		// Unlaunch Upper screen BG color (0..7FFFh)
				*(u16*)(0x02000816) = 0x7FFF;		// Unlaunch Lower screen BG color (0..7FFFh)
				memset((u8*)0x02000818, 0, 0x20+0x208+0x1C0);		// Unlaunch Reserved (zero)
				int i2 = 0;
				for (int i = 0; i < (int)sizeof(unlaunchDevicePath); i++) {
					*(u8*)(0x02000838+i2) = unlaunchDevicePath[i];		// Unlaunch Device:/Path/Filename.ext (16bit Unicode,end by 0000h)
					i2 += 2;
				}
				while (*(u16*)(0x0200080E) == 0) {	// Keep running, so that CRC16 isn't 0
					*(u16*)(0x0200080E) = swiCRC16(0xFFFF, (void*)0x02000810, 0x3F0);		// Unlaunch CRC16
				}
			} 
			fifoSendValue32(FIFO_USER_02, 1);	// ReturntoDSiMenu
			swiWaitForVBlank();
		}
		if (*(u8*)(0x023FF002) == 2 && !arm7SCFGLocked) {
			if (consoleModel < 2) {
				unlaunchSetHiyaBoot();
			}
			memcpy((u32*)0x02000300, autoboot_bin, 0x020);
			fifoSendValue32(FIFO_USER_02, 1);	// ReturntoDSiMenu
			swiWaitForVBlank();
		}*/
		swiWaitForVBlank();
	}
}