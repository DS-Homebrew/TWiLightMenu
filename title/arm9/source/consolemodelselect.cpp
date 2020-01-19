#include <nds.h>

#include "graphics/bios_decompress_callback.h"
#include "common/dsimenusettings.h"
#include "bootstrapsettings.h"
#include "graphics/graphics.h"
#include "graphics/lodepng.h"

#define CONSOLE_SCREEN_WIDTH 32
#define CONSOLE_SCREEN_HEIGHT 24

extern u16 convertVramColorToGrayscale(u16 val);

extern bool fadeType;
extern bool controlTopBright;

void LoadConsoleBMP(int consoleModel) {
	uint imageWidth, imageHeight;
	std::vector<unsigned char> image[2];

	const char* filePath;
	int language = (ms().getGuiLanguage());

	switch (consoleModel) {
		case 0:
		default:
			break;
		case 1:
			filePath = "nitro:/graphics/devdsi.png";
			break;
		case 2:
			filePath = "nitro:/graphics/o3ds.png";
			break;
	}

	lodepng::decode(image[0], imageWidth, imageHeight, filePath);

	for(uint i=0;i<image[0].size()/4;i++) {
		bmpImageBuffer[i] = image[0][i*4]>>3 | (image[0][(i*4)+1]>>3)<<5 | (image[0][(i*4)+2]>>3)<<10 | BIT(15);
		if (ms().colorMode == 1) {
			bmpImageBuffer[i] = convertVramColorToGrayscale(bmpImageBuffer[i]);
		}
	}

	// Start loading
	dmaCopyWordsAsynch(0, bmpImageBuffer, (u16*)BG_GFX, 0x200*192);

	//if french
	if (language == 2){
		switch (consoleModel) {
			case 0:
			default:
				break;
			case 1:
				filePath = "nitro:/graphics/consoleseltext_devdsi-fr.png";
				break;
			case 2:
				filePath = "nitro:/graphics/consoleseltext_3ds-fr.png";
				break;
		}
	}
	else {
		switch (consoleModel) {
			case 0:
			default:
				break;
			case 1:
				filePath = "nitro:/graphics/consoleseltext_devdsi.png";
				break;
			case 2:
				filePath = "nitro:/graphics/consoleseltext-3ds.png";
				break;
		}
	}

	lodepng::decode(image[1], imageWidth, imageHeight, filePath);

	while(dmaBusy(0));
	for(uint i=0;i<image[1].size()/4;i++) {
		bmpImageBuffer[i] = image[1][i*4]>>3 | (image[1][(i*4)+1]>>3)<<5 | (image[1][(i*4)+2]>>3)<<10 | BIT(15);
		if (ms().colorMode == 1) {
			bmpImageBuffer[i] = convertVramColorToGrayscale(bmpImageBuffer[i]);
		}
	}

	// Start loading
	dmaCopyWordsAsynch(1, bmpImageBuffer, (u16*)BG_GFX, 0x200*192);
}

bool consoleModel_isSure(void) {
	int isSure_pressed = 0;

	controlTopBright = false;
	fadeType = false;
	for (int i = 0; i < 60; i++) {
		swiWaitForVBlank();
	}

	uint imageWidth, imageHeight;
	std::vector<unsigned char> image;

	//Get the language for the splash screen
	int language = (ms().getGuiLanguage());
	const char* filePath;

	//If not french, then fallback to english
	if(language == 2){
		filePath = "nitro:/graphics/consoleseltext_areyousure-fr.png";
	}
	else {
		filePath = "nitro:/graphics/consoleseltext_areyousure.png";
	}

	lodepng::decode(image, imageWidth, imageHeight, filePath);

	for(uint i=0;i<image.size()/4;i++) {
		bmpImageBuffer[i] = image[i*4]>>3 | (image[(i*4)+1]>>3)<<5 | (image[(i*4)+2]>>3)<<10 | BIT(15);
		if (ms().colorMode == 1) {
			bmpImageBuffer[i] = convertVramColorToGrayscale(bmpImageBuffer[i]);
		}
	}

	// Start loading
	dmaCopyWordsAsynch(1, bmpImageBuffer, (u16*)BG_GFX, 0x200*192);

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
			LoadConsoleBMP(ms().consoleModel);
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

	if (ms().consoleModel < 1 || ms().consoleModel > 2) {
		ms().consoleModel = 2;
	}

	LoadConsoleBMP(ms().consoleModel);

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

		if (pressed & KEY_LEFT) {
			ms().consoleModel--;
			if (ms().consoleModel < 1) {
				ms().consoleModel = 2;
			}
			LoadConsoleBMP(ms().consoleModel);
		}
		if (pressed & KEY_RIGHT) {
			ms().consoleModel++;
			if (ms().consoleModel > 2) {
				ms().consoleModel = 1;
			}
			LoadConsoleBMP(ms().consoleModel);
		}

		if (pressed & KEY_A) {
			if (consoleModel_isSure()) {
				break;
			}
		}
	}
}
