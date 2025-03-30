#include <nds.h>
#include <stdio.h>

#include "common/twlmenusettings.h"
#include "common/systemdetails.h"
#include "common/flashcard.h"
#include "common/lodepng.h"
//#include "autoboot.h"

extern u16* colorTable;

extern const char *unlaunchAutoLoadID;
extern char unlaunchDevicePath[256];
extern void unlaunchSetHiyaBoot();

u16* sdRemovedExtendedImage = NULL;
u16* sdRemovedImage = NULL;

static int timeTillChangeToNonExtendedImage = 0;
static bool showNonExtendedImage = false;

void loadSdRemovedImage(void) {
	sdRemovedExtendedImage = new u16[0x18000/sizeof(u16)];
	sdRemovedImage = new u16[0x18000/sizeof(u16)];

	uint imageWidth, imageHeight;
	std::vector<unsigned char> image;
	char sdRemovedError[40];
	char sdRemoved[40];
	sprintf(sdRemovedError, "nitro:/graphics/sdRemovedError_%i.png", ms().getGuiLanguage());
	sprintf(sdRemoved, "nitro:/graphics/sdRemoved_%i.png", ms().getGuiLanguage());
	if (access(sdRemovedError, F_OK) != 0 || access(sdRemoved, F_OK) != 0) {
		sprintf(sdRemovedError, "nitro:/graphics/sdRemovedError_1.png");
		sprintf(sdRemoved, "nitro:/graphics/sdRemoved_1.png");
	}

	lodepng::decode(image, imageWidth, imageHeight, sdRemovedError);

	for (uint i=0;i<image.size()/4;i++) {
		sdRemovedExtendedImage[i] = image[i*4]>>3 | (image[(i*4)+1]>>3)<<5 | (image[(i*4)+2]>>3)<<10 | BIT(15);
		if (colorTable) {
			sdRemovedExtendedImage[i] = colorTable[sdRemovedExtendedImage[i] % 0x8000];
		}
	}

	image.clear();
	lodepng::decode(image, imageWidth, imageHeight, sdRemoved);

	for (uint i=0;i<image.size()/4;i++) {
		sdRemovedImage[i] = image[i*4]>>3 | (image[(i*4)+1]>>3)<<5 | (image[(i*4)+2]>>3)<<10 | BIT(15);
		if (colorTable) {
			sdRemovedImage[i] = colorTable[sdRemovedImage[i] % 0x8000];
		}
	}
}

void checkSdEject(void) {
	if (!ms().sdRemoveDetect) return;

	if (sys().sdStatus() == SystemDetails::ESDStatus::SDOk || !isDSiMode() || !sdFound()) {
		timeTillChangeToNonExtendedImage++;
		if (timeTillChangeToNonExtendedImage > 10) {
			showNonExtendedImage = true;
			timeTillChangeToNonExtendedImage = 10;
		}
		return;
	}
	
	// Show "SD removed" screen
	videoSetMode(MODE_3_2D | DISPLAY_BG3_ACTIVE);
	videoSetModeSub(MODE_3_2D | DISPLAY_BG3_ACTIVE);

	REG_BLDY = 0;

	extern bool doubleBuffer;
	doubleBuffer = false;
	swiWaitForVBlank();

	dmaCopyWordsAsynch(0, (showNonExtendedImage ? sdRemovedImage : sdRemovedExtendedImage), BG_GFX, 0x18000);
	dmaFillWords(1, BG_GFX_SUB, 0x18000);

	while (1) {
		// Works here, but disabled for consistency with the other themes
		/*scanKeys();
		if (keysDown() & KEY_B) {
			if (ms().consoleModel < 2 && ms().launcherApp != -1) {
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
		if (*(u8*)(0x023FF002) == 2 && !sys().arm7SCFGLocked()) {
			if (ms().consoleModel < 2) {
				unlaunchSetHiyaBoot();
			}
			memcpy((u32*)0x02000300, autoboot_bin, 0x020);
			fifoSendValue32(FIFO_USER_02, 1);	// ReturntoDSiMenu
			swiWaitForVBlank();
		}*/
		swiWaitForVBlank();
	}
}