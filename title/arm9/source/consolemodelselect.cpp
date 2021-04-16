#include <algorithm>
#include <nds.h>

#include "bootstrapsettings.h"
#include "common/dsimenusettings.h"
#include "graphics/bios_decompress_callback.h"
#include "graphics/fontHandler.h"
#include "graphics/gif.hpp"
#include "graphics/graphics.h"
#include "language.h"

extern bool useTwlCfg;

extern bool fadeType;

void LoadConsoleImage(int consoleModel) {
	Gif gif(consoleModel == 1 ? "nitro:/graphics/devdsi.gif" : "nitro:/graphics/o3ds.gif", true, false);
	gif.displayFrame();
}

bool consoleModel_isSure(void) {
	int isSure_pressed = 0;

	while (1) {
		clearText();
		printLarge(false, 0, 31, STR_ARE_YOU_SURE, Alignment::center);
		printSmall(false, 0, 88 - ((calcSmallFontHeight(STR_SELECTING_WRONG) - smallFontHeight()) / 2), STR_SELECTING_WRONG, Alignment::center);
		printSmall(false, 0, 146, STR_B_NO_A_YES, Alignment::center);
		updateText(false);

		fadeType = true;

		// Power saving loop. Only poll the keys once per frame and sleep the CPU if there is nothing else to do
		do {
			scanKeys();
			isSure_pressed = keysDown();
			swiWaitForVBlank();
		}
		while (!isSure_pressed);

		if (isSure_pressed & KEY_A) {
			fadeType = false;
			for (int i = 0; i < 25; i++) {
				swiWaitForVBlank();
			}
			bs().consoleModel = ms().consoleModel;
			ms().saveSettings();
			bs().saveSettings();
			return true;
		}
		if (isSure_pressed & KEY_B) {
			fadeType = false;
			for (int i = 0; i < 25; i++) {
				swiWaitForVBlank();
			}
			return false;
		}
	}
}

void consoleModelSelect(void) {
	videoSetMode(MODE_5_2D);
	videoSetModeSub(MODE_5_2D);

	vramSetBankA(VRAM_A_MAIN_BG);
	vramSetBankC(VRAM_C_SUB_BG);
	
	bgInit(3, BgType_Bmp8, BgSize_B8_256x256, 0, 0);
	bgSetPriority(3, 3);
	bgInit(2, BgType_Bmp8, BgSize_B8_256x256, 3, 0);
	bgSetPriority(2, 2);
	bgInitSub(3, BgType_Bmp8, BgSize_B8_256x256, 0, 0);
	bgSetPriority(7, 3);
	bgInitSub(2, BgType_Bmp8, BgSize_B8_256x256, 3, 0);
	bgSetPriority(6, 2);

	fontInit();
	langInit();

	LoadConsoleImage(ms().consoleModel);

	int pressed = 0;
	// touchPosition touch;

	Gif bg("nitro:/graphics/consolesel_bg.gif", false, false);
	bg.displayFrame();

	while (1) {
		clearText();
		printSmall(false, 0, 31, STR_SELECT_CONSOLE, Alignment::center);
		std::string &consoleStr = ms().consoleModel == 1 ? STR_DSI_PANDA : STR_3DS_2DS;
		printLarge(false, 0, 88 - ((calcLargeFontHeight(consoleStr) - largeFontHeight()) / 2), consoleStr, Alignment::center);
		printSmall(false, 0, 146, STR_LR_CHOOSE_A_SELECT, Alignment::center);
		updateText(false);

		fadeType = true;

		// Power saving loop. Only poll the keys once per frame and sleep the CPU if there is nothing else to do
		do {
			scanKeys();
			pressed = keysDown();
			// touchRead(&touch);
			swiWaitForVBlank();
		}
		while (!pressed);

		if (pressed & KEY_LEFT) {
			ms().consoleModel--;
			if (ms().consoleModel < 1) {
				ms().consoleModel = 2;
			}
			LoadConsoleImage(ms().consoleModel);
		}
		if (pressed & KEY_RIGHT) {
			ms().consoleModel++;
			if (ms().consoleModel > 2) {
				ms().consoleModel = 1;
			}
			LoadConsoleImage(ms().consoleModel);
		}

		if (pressed & KEY_A) {
			fadeType = false;
			for (int i = 0; i < 25; i++) {
				swiWaitForVBlank();
			}
			if (consoleModel_isSure()) {
				break;
			}
		}
	}
}
