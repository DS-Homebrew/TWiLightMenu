#include <nds.h>
#include <stdio.h>
#include <maxmod9.h>

#include "common/systemdetails.h"
#include "common/dsimenusettings.h"
#include "graphics/ThemeTextures.h"
#include "autoboot.h"
#include "sound.h"

extern const char *unlaunchAutoLoadID;
extern char unlaunchDevicePath[256];
extern void unlaunchSetHiyaBoot();


extern bool rocketVideo_playVideo;
extern bool showdialogbox;
extern float dbox_Ypos;


vu16* sdRemovedExtendedImage = (vu16*)0x026C8000;
vu16* sdRemovedImage = (vu16*)0x026E0000;

extern u16 convertToDsBmp(u16 val);

static int timeTillChangeToNonExtendedImage = 0;
static bool showNonExtendedImage = false;

void loadSdRemovedImage(void) {
	//FILE* file = fopen((sys().arm7SCFGLocked() ? "nitro:/graphics/sdRemovedSimple.bmp" : "nitro:/graphics/sdRemoved.bmp"), "rb");
	FILE* file = fopen("nitro:/graphics/sdRemovedError.bmp", "rb");
	if (file) {
		// Start loading
		fseek(file, 0xe, SEEK_SET);
		u8 pixelStart = (u8)fgetc(file) + 0xe;
		fseek(file, pixelStart, SEEK_SET);
		fread(tex().bmpImageBuffer(), 2, 0x18000, file);
		u16* src = tex().bmpImageBuffer();
		int x = 0;
		int y = 191;
		for (int i=0; i<256*192; i++) {
			if (x >= 256) {
				x = 0;
				y--;
			}
			u16 val = *(src++);
			sdRemovedExtendedImage[y*256+x] = tex().convertToDsBmp(val);
			x++;
		}
	}
	fclose(file);

	file = fopen("nitro:/graphics/sdRemoved.bmp", "rb");
	if (file) {
		// Start loading
		fseek(file, 0xe, SEEK_SET);
		u8 pixelStart = (u8)fgetc(file) + 0xe;
		fseek(file, pixelStart, SEEK_SET);
		fread(tex().bmpImageBuffer(), 2, 0x18000, file);
		u16* src = tex().bmpImageBuffer();
		int x = 0;
		int y = 191;
		for (int i=0; i<256*192; i++) {
			if (x >= 256) {
				x = 0;
				y--;
			}
			u16 val = *(src++);
			sdRemovedImage[y*256+x] = tex().convertToDsBmp(val);
			x++;
		}
	}
	fclose(file);
}

void checkSdEject(void) {
	if (!ms().sdRemoveDetect) return;

	if (sys().sdStatus() == SystemDetails::ESDStatus::SDOk || !isDSiMode()) {
		timeTillChangeToNonExtendedImage++;
		if (timeTillChangeToNonExtendedImage > 10) {
			showNonExtendedImage = true;
			timeTillChangeToNonExtendedImage = 10;
		}
		return;
	}
	
	// Show "SD removed" screen
	rocketVideo_playVideo = false;
	
	if (showdialogbox) {
		showdialogbox = false;
		dbox_Ypos = 192;
	}
	snd().stopStream();
	mmEffectCancelAll();

	videoSetMode(MODE_3_2D | DISPLAY_BG3_ACTIVE);
	videoSetModeSub(MODE_3_2D | DISPLAY_BG3_ACTIVE);

	REG_BLDY = 0;

	dmaCopyWordsAsynch(0, (void*)(showNonExtendedImage ? sdRemovedImage : sdRemovedExtendedImage), BG_GFX, 0x18000);
	dmaFillWords(0, BG_GFX_SUB, 0x18000);

	while(1) {
		// Currently not working
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
		if (sys().sdStatus() == SystemDetails::ESDStatus::SDInserted && !sys().arm7SCFGLocked()) {
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