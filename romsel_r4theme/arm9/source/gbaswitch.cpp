#include "common/nds_loader_arm9.h"
#include "common/tonccpy.h"
#include "common/twlmenusettings.h"
#include "common/lodepng.h"
#include "gbaswitch.h"

extern u16 bmpImageBuffer[256*192];

void loadGbaBorder(const char* filename) {
	uint imageWidth, imageHeight;
	std::vector<unsigned char> image;
	lodepng::decode(image, imageWidth, imageHeight, filename);
	bool alternatePixel = false;
	bool alternatePixel2 = false;

	for (int b = 0; b < 2; b++) {
		for (uint i = 0; i < image.size()/4; i++) {
			const u8 oldR = image[(i*4)];
			const u8 oldG = image[(i*4)+1];
			const u8 oldB = image[(i*4)+2];
			u8 newR = oldR;
			u8 newG = oldG;
			u8 newB = oldB;
			if (alternatePixel) {
				if (oldR >= 4 && oldR < 0xFC) newR += 4;
				if (oldG >= 4 && oldG < 0xFC) newG += 4;
				if (oldB >= 4 && oldB < 0xFC) newB += 4;
			}
			if (alternatePixel2) {
				if (((oldR/2) % 2) == 1 && newR < 0xFE) newR += 2;
				if (((oldG/2) % 2) == 1 && newG < 0xFE) newG += 2;
				if (((oldB/2) % 2) == 1 && newB < 0xFE) newB += 2;
			}
			bmpImageBuffer[i] = newR>>3 | (newG>>3)<<5 | (newB>>3)<<10 | BIT(15);
			if ((i % 256) == 255) {
				alternatePixel = !alternatePixel;
				alternatePixel2 = !alternatePixel2;
			}
			alternatePixel = !alternatePixel;
			alternatePixel2 = !alternatePixel2;
		}
		alternatePixel = !alternatePixel;

		DC_FlushRange(bmpImageBuffer,SCREEN_WIDTH*SCREEN_HEIGHT*2);
		dmaCopy(bmpImageBuffer,(void*)BG_BMP_RAM(b==1 ? 8 : 0),SCREEN_WIDTH*SCREEN_HEIGHT*2);
	}
}

void gbaSwitch(void) {
	irqDisable(IRQ_VBLANK);

	videoSetMode(MODE_5_2D | DISPLAY_BG3_ACTIVE);
	videoSetModeSub(MODE_5_2D | DISPLAY_BG3_ACTIVE);

	vramSetBankA(VRAM_A_MAIN_BG_0x06000000);
	vramSetBankB(VRAM_B_MAIN_BG_0x06020000);
	vramSetBankC(VRAM_C_SUB_BG_0x06200000);
	vramSetBankD(VRAM_D_LCD);

	// for the main screen
	REG_BG3CNT = BG_BMP16_256x256 | BG_BMP_BASE(0) | BG_WRAP_OFF;
	REG_BG3PA = 1 << 8; //scale x
	REG_BG3PB = 0; //rotation x
	REG_BG3PC = 0; //rotation y
	REG_BG3PD = 1 << 8; //scale y
	REG_BG3X = 0; //translation x
	REG_BG3Y = 0; //translation y

	toncset((void*)BG_BMP_RAM(0),0,0x18000);
	toncset((void*)BG_BMP_RAM(8),0,0x18000);

	char borderPath[256];
	sprintf(borderPath, "/_nds/TWiLightMenu/gbaborders/%s", ms().gbaBorder.c_str());
	loadGbaBorder((access(borderPath, F_OK)==0) ? borderPath : "nitro:/graphics/gbaborder.png");

	// Switch to GBA mode
	runNdsFile ("/_nds/TWiLightMenu/gbaswitch.srldr", 0, NULL, false, true, false, true, false, false, false, -1);	
}