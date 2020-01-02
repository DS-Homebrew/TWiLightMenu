#include <nds.h>

#include <iostream>
#include <string>

#include "graphics/bios_decompress_callback.h"
#include "common/dsimenusettings.h"
#include "bootstrapsettings.h"

#include "graphics/graphics.h"
#include "graphics/lodepng.h"

#define CONSOLE_SCREEN_WIDTH 32
#define CONSOLE_SCREEN_HEIGHT 24

extern bool fadeType;
extern bool controlTopBright;

void LoadConsoleBMP(void) {
	std::string fileName = "nitro:/graphics/dsivs3ds.png";

	std::vector<unsigned char> image;
	unsigned width, height;
	unsigned error = lodepng::decode(image, width, height, fileName);
	for(unsigned i = 0; i < image.size(); i = i * 4) {
		BG_GFX[i] = image[i]>>3 | (image[i + 1]>>3)<<5 | (image[i + 2]>>3)<<10 | BIT(15);
	}

	int language = (ms().getGuiLanguage());
	fileName = "nitro:/graphics/consoleseltext_" + (language == 2 ? std::string("-fr") : std::string("")) + ".png";

	lodepng::decode(image, width, height, fileName);
	for(unsigned i = 0; i < image.size(); i = i * 4) {
		BG_GFX_SUB[i] = image[i]>>3 | (image[i + 1]>>3)<<5 | (image[i + 2]>>3)<<10 | BIT(15);
	}
}

bool consoleModel_isSure(int consoleModel) {
	int isSure_pressed = 0;

	controlTopBright = false;
	fadeType = false;
	for (int i = 0; i < 90; i++) {
		swiWaitForVBlank();
	}

	std::string names[] = { "dsi", "3ds" };
	std::string fileName = "nitro:/graphics/" + names[consoleModel - 1] + ".png";

	std::vector<unsigned char> image;
	unsigned width, height;
	unsigned error = lodepng::decode(image, width, height, fileName);
	for(unsigned i = 0; i < image.size(); i = i * 4) {
		BG_GFX[i] = image[i]>>3 | (image[i + 1]>>3)<<5 | (image[i + 2]>>3)<<10 | BIT(15);
	}

	//Get the language for the splash screen
	int language = (ms().getGuiLanguage());
	fileName = "nitro:/graphics/consoleseltext_areyousure" + (language == 2 ? std::string("-fr") : std::string("")) + ".png";

	lodepng::decode(image, width, height, fileName);
	for(unsigned i = 0; i < image.size(); i = i * 4) {
  		BG_GFX_SUB[i] = image[i]>>3 | (image[i + 1]>>3)<<5 | (image[i + 2]>>3)<<10 | BIT(15);
	}

	fadeType = true;
	for (int i = 0; i < 25; i++) {
		swiWaitForVBlank();
	}
	while (1) {
		// Power saving loop. Only poll the keys once per frame and sleep the CPU if there is nothing else to do
		do
		{
			scanKeys();
			isSure_pressed = keysDown();
			swiWaitForVBlank();
		}
		while (!isSure_pressed);

		if (isSure_pressed & KEY_A) {
			controlTopBright = true;
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
			LoadConsoleBMP();
			fadeType = true;
			for (int i = 0; i < 25; i++) {
				swiWaitForVBlank();
			}
			return false;
		}
	}
}

void consoleModelSelect(void) {
	videoSetMode(MODE_3_2D | DISPLAY_BG3_ACTIVE);
	videoSetModeSub(MODE_3_2D | DISPLAY_BG3_ACTIVE);
	vramSetBankD(VRAM_D_MAIN_BG_0x06000000);
	vramSetBankC(VRAM_C_SUB_BG_0x06200000);
	REG_BG3CNT = BG_MAP_BASE(0) | BG_BMP16_256x256;
	REG_BG3X = 0;
	REG_BG3Y = 0;
	REG_BG3PA = 1<<8;
	REG_BG3PB = 0;
	REG_BG3PC = 0;
	REG_BG3PD = 1<<8;

	REG_BG3CNT_SUB = BG_MAP_BASE(0) | BG_BMP16_256x256 | BG_PRIORITY(0);
	REG_BG3X_SUB = 0;
	REG_BG3Y_SUB = 0;
	REG_BG3PA_SUB = 1<<8;
	REG_BG3PB_SUB = 0;
	REG_BG3PC_SUB = 0;
	REG_BG3PD_SUB = 1<<8;

	LoadConsoleBMP();

	int pressed = 0;
	//touchPosition touch;

	fadeType = true;
	for (int i = 0; i < 25; i++) {
		swiWaitForVBlank();
	}
	while (1) {
		// Power saving loop. Only poll the keys once per frame and sleep the CPU if there is nothing else to do
		do
		{
			scanKeys();
			pressed = keysDown();
			//touchRead(&touch);
			swiWaitForVBlank();
		}
		while (!pressed);

		if (pressed & KEY_LEFT || pressed & KEY_RIGHT) {
			ms().consoleModel = (pressed & KEY_LEFT ? 1 : 2);
			if (consoleModel_isSure(ms().consoleModel))
				break;
		}
	}
}
