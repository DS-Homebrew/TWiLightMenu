#ifndef _NO_MAIN_ALL

#include <nds.h>

#include <fat.h>
#include <limits.h>
#include <stdio.h>
#include <sys/stat.h>

#include <string.h>
#include <unistd.h>

#include "common/twlmenusettings.h"
#include "common/systemdetails.h"
#include "myDSiMode.h"

bool useTwlCfg = false;

// Ported from PAlib (obsolete)
static void SetBrightness(u8 screen, s8 bright) {
	u16 mode = 1 << 14;

	if (bright < 0) {
		mode = 2 << 14;
		bright = -bright;
	}
	if (bright > 31)
		bright = 31;
	*(vu16 *)(0x0400006C + (0x1000 * screen)) = bright + mode;
}

//---------------------------------------------------------------------------------
static void stop(void) {
//---------------------------------------------------------------------------------
	while (1) {
		swiWaitForVBlank();
	}
}

static int screenMode = CURRENT_SCREEN_MODE;

int main(int argc, char **argv) {
	// overwrite reboot stub identifier
	/*extern char *fake_heap_end;
	*fake_heap_end = 0;*/

	defaultExceptionHandler();
	fifoSendValue32(FIFO_PM, PM_REQ_SLEEP_DISABLE);		// Disable sleep mode to prevent unexpected crashes from exiting sleep mode

	sys().initFilesystem(argc==0 ? "sd:/_nds/TWiLightMenu/main.srldr" : argv[0]);
	sys().initArm7RegStatuses();

	if (!sys().fatInitOk()) {
		SetBrightness(0, 0);
		SetBrightness(1, 0);
		consoleDemoInit();
		iprintf("FAT init failed!");
		stop();
	}

	useTwlCfg = (dsiFeatures() && (*(u8*)0x02000400 != 0) && (*(u8*)0x02000401 == 0) && (*(u8*)0x02000402 == 0) && (*(u8*)0x02000404 == 0) && (*(u8*)0x02000448 != 0));

	sysSetCartOwner(BUS_OWNER_ARM9); // Allow arm9 to access GBA ROM

	ms();

	while (1) {
		switch (screenMode) {
			case 0:
				extern int titleMode(void);
				titleMode();
				break;
			case 1:
				extern int settingsMode(void);
				settingsMode();
				break;
			case 2:
				extern int dsClassicMenu(void);
				dsClassicMenu();
				break;
			case 3:
				extern int dsiMenuTheme(void);
				dsiMenuTheme();
				break;
			case 4:
				extern int r4Theme(void);
				r4Theme();
				break;
			case 5:
				extern int manualScreen(void);
				manualScreen();
				break;
			case 6:
				extern int imageViewer(void);
				imageViewer();
				break;
		}
	}

	return 0;
}

#endif