#include <nds.h>
#include <nds/arm9/dldi.h>
#include <maxmod9.h>

#include <stdio.h>
#include <fat.h>
#include <sys/stat.h>
#include <limits.h>

#include <algorithm>
#include <string.h>
#include <unistd.h>

#include "graphics/graphics.h"

#include "common/systemdetails.h"
#include "common/nds_loader_arm9.h"
#include "common/twlmenusettings.h"
#include "common/flashcard.h"
#include "errorScreen.h"

#include "graphics/fontHandler.h"

#include "myDSiMode.h"
#include "common/inifile.h"
#include "common/tonccpy.h"
#include "language.h"

#include "sound.h"

bool fadeType = false;		// false = out, true = in
bool fadeSpeed = true;		// false = slow (for DSi launch effect), true = fast
bool controlTopBright = true;
bool controlBottomBright = true;
bool supportsDoubleBuffer = false;

extern void ClearBrightness();
extern int imageType;

//---------------------------------------------------------------------------------
void stop (void) {
//---------------------------------------------------------------------------------
	while (1) {
		swiWaitForVBlank();
	}
}

void loadROMselect() {
	snd().playBack();
	fadeType = false;	// Fade out to white
	for (int i = 0; i < 25; i++) {
		swiWaitForVBlank();
	}
	std::vector<char *> argarray;

	switch (ms().theme) {
		case TWLSettings::EThemeDSi:
		case TWLSettings::EThemeHBL:
		case TWLSettings::EThemeSaturn:
		case TWLSettings::ETheme3DS:
			argarray.push_back((char*)(sys().isRunFromSD() ? "sd:/_nds/TWiLightMenu/dsimenu.srldr" : "fat:/_nds/TWiLightMenu/dsimenu.srldr"));
			break;
		case TWLSettings::EThemeR4:
		case TWLSettings::EThemeGBC:
			argarray.push_back((char*)(sys().isRunFromSD() ? "sd:/_nds/TWiLightMenu/r4menu.srldr" : "fat:/_nds/TWiLightMenu/r4menu.srldr"));
			break;
		case TWLSettings::EThemeWood:
			argarray.push_back((char*)(sys().isRunFromSD() ? "sd:/_nds/TWiLightMenu/akmenu.srldr" : "fat:/_nds/TWiLightMenu/akmenu.srldr"));
			break;
	}

	runNdsFile(argarray[0], argarray.size(), (const char**)&argarray[0], sys().isRunFromSD(), true, false, false, true, true, false, -1);

	fadeType = true;	// Fade in from white
}

bool extension(const std::string_view filename, const std::vector<std::string_view> extensions) {
	for (std::string_view extension : extensions) {
		if (strcasecmp(filename.substr(filename.size() - extension.size()).data(), extension.data()) == 0) {
			return true;
		}
	}

	return false;
}

void customSleep() {
	*(int*)0x02003004 = 1; // Fade out sound
	fadeType = false;
	for (int i = 0; i < 25; i++) {
		swiWaitForVBlank();
	}
	mmPause();
	if (!ms().macroMode) {
		powerOff(PM_BACKLIGHT_TOP);
	}
	powerOff(PM_BACKLIGHT_BOTTOM);
	irqDisable(IRQ_VBLANK & IRQ_VCOUNT);
	while (keysHeld() & KEY_LID) {
		scanKeys();
		swiWaitForVBlank();
	}
	irqEnable(IRQ_VBLANK & IRQ_VCOUNT);
	if (!ms().macroMode) {
		powerOn(PM_BACKLIGHT_TOP);
	}
	powerOn(PM_BACKLIGHT_BOTTOM);
	mmResume();
	fadeType = true;
	*(int*)0x02003004 = 2; // Fade in sound
}

void printText(void) {
	if (ms().macroMode) return;

	clearText(false);
	if (supportsDoubleBuffer) {
		printSmall(false, 0, 88, doubleBuffer ? STR_A_REGULAR_DITHERING : STR_A_TEMPORAL_DITHERING, Alignment::center);
	}
	printSmall(false, -88, 174, STR_BACK, Alignment::center);
	updateText(false);
}

//---------------------------------------------------------------------------------
int imageViewer(void) {
//---------------------------------------------------------------------------------
	keysSetRepeat(25, 25);

	ms().loadSettings();

	const char* imagePathChar = ms().romPath[ms().previousUsedDevice].c_str();

	if (strlen(imagePathChar) >= 2) {
		if (extension(imagePathChar, {".gif"})) {
			imageType = 0;
		} else if (extension(imagePathChar, {".bmp"})) {
			imageType = 1;
		} else if (extension(imagePathChar, {".png"})) {
			imageType = 2;
		}
	} else {
		imageType = 2;
	}

	graphicsInit();
	fontInit();

	langInit();

	imageLoad((strlen(imagePathChar) >= 2) ? imagePathChar : "nitro:/graphics/test.png");
	bgLoad();
	supportsDoubleBuffer = doubleBuffer;
	printText();

	snd();
	snd().beginStream();

	int pressed = 0;
	int held = 0;
	//int repeat = 0;
	touchPosition touch;

	fadeType = true;	// Fade in from white

	bool exitflag = false;

	while (1) {
		do {
			scanKeys();
			touchRead(&touch);
			pressed = keysDown();
			held = keysHeld();
			//repeat = keysDownRepeat();
			checkSdEject();
			// snd().updateStream();
			swiWaitForVBlank();
			exitflag = fifoCheckValue32(FIFO_USER_01);
		} while (!held && !exitflag);

		if ((pressed & KEY_LID) && ms().sleepMode) {
			customSleep();
		}

		if ((pressed & KEY_A) && supportsDoubleBuffer) {
			doubleBuffer = !doubleBuffer;
			printText();
			snd().playSwitch();
		}

		if ((pressed & KEY_B) || ((pressed & KEY_TOUCH) && touch.px >= 0 && touch.px < 80 && touch.py >= 169 && touch.py < 192)) {
			loadROMselect();
		}

		if (exitflag) {
			break;
		}
	}

	*(int*)0x02003004 = 1; // Fade out sound
	fadeType = false;
	for (int i = 0; i < 25; i++) {
		swiWaitForVBlank();
	}
	mmStop();
	*(int*)0x02003004 = 0;

	runNdsFile(sys().isRunFromSD() ? "sd:/boot.nds" : "fat:/boot.nds", 0, NULL, sys().isRunFromSD(), true, true, false, true, true, false, -1);

	return 0;
}
