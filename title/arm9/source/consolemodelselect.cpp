#include <nds.h>

#include "graphics/bios_decompress_callback.h"
#include "common/dsimenusettings.h"
#include "bootstrapsettings.h"
#include "graphics/graphics.h"

#define CONSOLE_SCREEN_WIDTH 32
#define CONSOLE_SCREEN_HEIGHT 24

extern bool fadeType;
extern bool controlTopBright;

void LoadConsoleBMP(int consoleModel) {
	FILE* file;
	switch (consoleModel) {
		case 0:
		default:
			file = fopen("nitro:/graphics/dsi.bmp", "rb");
			break;
		case 1:
			file = fopen("nitro:/graphics/devdsi.bmp", "rb");
			break;
		case 2:
			file = fopen("nitro:/graphics/o3ds.bmp", "rb");
			break;
		case 3:
			file = fopen("nitro:/graphics/n3ds.bmp", "rb");
			break;
	}

	if (file) {
		// Start loading
		fseek(file, 0xe, SEEK_SET);
		u8 pixelStart = (u8)fgetc(file) + 0xe;
		fseek(file, pixelStart, SEEK_SET);
		fread(bmpImageBuffer, 2, 0x1A000, file);
		u16* src = bmpImageBuffer;
		int x = 0;
		int y = 191;
		for (int i=0; i<256*192; i++) {
			if (x >= 256) {
				x = 0;
				y--;
			}
			u16 val = *(src++);
			BG_GFX[y*256+x] = ((val>>10)&0x1f) | ((val)&(0x1f<<5)) | (val&0x1f)<<10 | BIT(15);
			x++;
		}
	}

	fclose(file);
	
	switch (consoleModel) {
		case 0:
		default:
			file = fopen("nitro:/graphics/consoleseltext_dsi.bmp", "rb");
			break;
		case 1:
			file = fopen("nitro:/graphics/consoleseltext_devdsi.bmp", "rb");
			break;
		case 2:
			file = fopen("nitro:/graphics/consoleseltext_o3ds.bmp", "rb");
			break;
		case 3:
			file = fopen("nitro:/graphics/consoleseltext_n3ds.bmp", "rb");
			break;
	}

	if (file) {
		// Start loading
		fseek(file, 0xe, SEEK_SET);
		u8 pixelStart = (u8)fgetc(file) + 0xe;
		fseek(file, pixelStart, SEEK_SET);
		fread(bmpImageBuffer, 2, 0x1A000, file);
		u16* src = bmpImageBuffer;
		int x = 0;
		int y = 191;
		for (int i=0; i<256*192; i++) {
			if (x >= 256) {
				x = 0;
				y--;
			}
			u16 val = *(src++);
			BG_GFX_SUB[y*256+x] = ((val>>10)&0x1f) | ((val)&(0x1f<<5)) | (val&0x1f)<<10 | BIT(15);
			x++;
		}
	}

	fclose(file);
	
}

bool consoleModel_isSure(void) {
	int isSure_pressed = 0;

	controlTopBright = false;
	fadeType = false;
	for (int i = 0; i < 90; i++) {
		swiWaitForVBlank();
	}

	FILE* file = fopen("nitro:/graphics/consoleseltext_areyousure.bmp", "rb");

	if (file) {
		// Start loading
		fseek(file, 0xe, SEEK_SET);
		u8 pixelStart = (u8)fgetc(file) + 0xe;
		fseek(file, pixelStart, SEEK_SET);
		fread(bmpImageBuffer, 2, 0x1A000, file);
		u16* src = bmpImageBuffer;
		int x = 0;
		int y = 191;
		for (int i=0; i<256*192; i++) {
			if (x >= 256) {
				x = 0;
				y--;
			}
			u16 val = *(src++);
			BG_GFX_SUB[y*256+x] = ((val>>10)&0x1f) | ((val)&(0x1f<<5)) | (val&0x1f)<<10 | BIT(15);
			x++;
		}
	}

	fclose(file);

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

	if (ms().consoleModel < 1 || ms().consoleModel > 3) {
		ms().consoleModel = 1;
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
				ms().consoleModel = 3;
			}
			LoadConsoleBMP(ms().consoleModel);
		}
		if (pressed & KEY_RIGHT) {
			ms().consoleModel++;
			if (ms().consoleModel > 3) {
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