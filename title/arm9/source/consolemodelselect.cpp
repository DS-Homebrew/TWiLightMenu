#include <nds.h>

#include "graphics/graphics.h"
#include "common/dsimenusettings.h"

#define CONSOLE_SCREEN_WIDTH 32
#define CONSOLE_SCREEN_HEIGHT 24

extern bool fadeType;

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
}

void consoleModelSelect(void) {
	videoSetMode(MODE_3_2D | DISPLAY_BG3_ACTIVE);
	videoSetModeSub(MODE_0_2D | DISPLAY_BG0_ACTIVE);
	vramSetBankD(VRAM_D_MAIN_BG_0x06040000);
	vramSetBankC (VRAM_C_SUB_BG_0x06200000);
	REG_BG3CNT = BG_MAP_BASE(0) | BG_BMP16_256x256;
	REG_BG3X = 0;
	REG_BG3Y = 0;
	REG_BG3PA = 1<<8;
	REG_BG3PB = 0;
	REG_BG3PC = 0;
	REG_BG3PD = 1<<8;
	REG_BG0CNT_SUB = BG_MAP_BASE(0) | BG_COLOR_256 | BG_TILE_BASE(2);
	BG_PALETTE[0]=0;
	BG_PALETTE[255]=0xffff;
	u16* bgMapSub = (u16*)SCREEN_BASE_BLOCK_SUB(0);
	for (int i = 0; i < CONSOLE_SCREEN_WIDTH*CONSOLE_SCREEN_HEIGHT; i++) {
		bgMapSub[i] = (u16)i;
	}

	if (ms().consoleModel < 0 || ms().consoleModel > 3) {
		ms().consoleModel = 0;
	}

	LoadConsoleBMP(ms().consoleModel);

	int pressed = 0;
	touchPosition touch;

	fadeType = true;
	while (1) {
		// Power saving loop. Only poll the keys once per frame and sleep the CPU if there is nothing else to do
		do
		{
			scanKeys();
			pressed = keysDown();
			touchRead(&touch);
			swiWaitForVBlank();
		}
		while (!pressed);

		if (pressed & KEY_LEFT) {
			ms().consoleModel--;
			if (ms().consoleModel < 0) {
				ms().consoleModel = 3;
			}
			LoadConsoleBMP(ms().consoleModel);
		}
		if (pressed & KEY_RIGHT) {
			ms().consoleModel++;
			if (ms().consoleModel > 3) {
				ms().consoleModel = 0;
			}
			LoadConsoleBMP(ms().consoleModel);
		}

		if (pressed & KEY_A) {
			fadeType = false;
			for (int i = 0; i < 25; i++) {
				swiWaitForVBlank();
			}
			ms().saveSettings();
			break;
		}
	}
}