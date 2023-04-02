#include <nds.h>

#include <stdio.h>
#include <fat.h>

#include <string.h>
#include <unistd.h>

#include "graphics/graphics.h"

#include "common/systemdetails.h"
#include "common/twlmenusettings.h"
#include "graphics/gif.hpp"

#include "myDSiMode.h"

bool fadeType = false;		// false = out, true = in
bool controlTopBright = true;
bool controlBottomBright = true;
bool useTwlCfg = false;

//---------------------------------------------------------------------------------
int main(int argc, char **argv) {
//---------------------------------------------------------------------------------
	if (!dsiFeatures()) {
		return 0; // Not a DSi!
	}

	fifoSendValue32(FIFO_PM, PM_REQ_SLEEP_DISABLE);		// Disable sleep mode to prevent unexpected crashes from exiting sleep mode
	defaultExceptionHandler();
	sys().initFilesystem(argv[0]);
	sys().initArm7RegStatuses();

	if (!sys().fatInitOk()) {
		SetBrightness(0, 0);
		SetBrightness(1, 0);
		consoleDemoInit();
		iprintf("FAT init failed!");
		while (1) {
			swiWaitForVBlank();
		}
	}

	keysSetRepeat(25, 25);

	useTwlCfg = (dsiFeatures() && (*(u8*)0x02000400 != 0) && (*(u8*)0x02000401 == 0) && (*(u8*)0x02000402 == 0) && (*(u8*)0x02000404 == 0) && (*(u8*)0x02000448 != 0));

	ms().loadSettings();

	graphicsInit();

	fadeType = true;	// Fade in from white

	// Load 3DS splash screen
	Gif intro("nitro:/graphics/intro.gif", true, true);
	Gif loop("nitro:/graphics/loop.gif", true, true);
	bgLoad();
	while (!intro.finished()) {
		intro.displayFrame();
		swiWaitForVBlank();
	}

	while (1) {
		loop.displayFrame();
		swiWaitForVBlank();
	}
}
