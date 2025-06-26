#include <nds.h>
#include <stdio.h>
#include <maxmod9.h>

// #include "autoboot.h"
#include "common/twlmenusettings.h"
#include "common/systemdetails.h"
#include "common/flashcard.h"
#include "graphics/fontHandler.h"
#include "common/tonccpy.h"
#include "language.h"

#include "sound.h"

extern u16* colorTable;

extern bool sdRemoveDetect;
extern const char *unlaunchAutoLoadID;
extern char unlaunchDevicePath[256];
extern void unlaunchSetHiyaBoot();

extern bool arm7SCFGLocked;

extern int consoleModel;
extern int launcherApp;

static int timeTillChangeToNonExtendedImage = 0;
static bool showNonExtendedImage = false;

void checkSdEject(void) {
	if (!ms().sdRemoveDetect) return;

	if (sys().sdStatus() == SystemDetails::ESDStatus::SDOk || !isDSiMode() || !sdFound()) {
		if (!showNonExtendedImage) {
			timeTillChangeToNonExtendedImage++;
			if (timeTillChangeToNonExtendedImage > 10) {
				showNonExtendedImage = true;
			}
		}
		return;
	}

	// Show "SD removed" screen
	mmEffectCancelAll();
	// snd().stopStream();
	mmStop();

	clearText();

	if (showNonExtendedImage) {
		printLarge(false, 0, 45, STR_SD_WAS_REMOVED, Alignment::center);
		printLarge(false, 0, 75, STR_REINSERT_SD_CARD, Alignment::center);
	} else {
		printLarge(false, 0, 37, STR_ERROR_HAS_OCCURRED, Alignment::center);
		printSmall(false, 0, 67, STR_DISABLE_SD_REMOVAL_CHECK, Alignment::center);
	}
	updateText(false);

	irqDisable(IRQ_VBLANK);
	irqDisable(IRQ_HBLANK);

	videoSetMode(MODE_5_2D | DISPLAY_BG2_ACTIVE);
	//videoSetModeSub(MODE_5_2D | DISPLAY_BG2_ACTIVE);

	REG_BLDY = 0;

	while (dmaBusy(0)) { swiDelay(100); }

	// Change to white text palette
	u16 palette[] = {
		0x0000,
		0xB9CE,
		0xD6B5,
		0xFFFF,
	};
	if (colorTable) {
		for (int i = 0; i < 4; i++) {
			palette[i] = colorTable[palette[i] % 0x8000];
		}
	}
	//tonccpy(BG_PALETTE + 0xF8, palette, sizeof(palette));
	toncset16(BG_PALETTE, 0, 256);
	toncset16(BG_PALETTE_SUB, 0, 256);
	tonccpy(BG_PALETTE_SUB + 0xF8, palette, sizeof(palette));

	if (ms().macroMode) {
		lcdMainOnTop();
	}
	swiWaitForVBlank();
	while (1) {
		// Currently not working
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
		if (*(u8*)(0x02FFF002) == 2 && !arm7SCFGLocked) {
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