#include "common/lodepng.h"
#include "common/tonccpy.h"
#include "gbaswitch.h"
#include "nds_loader_arm9.h"

static void SetBrightness(u8 screen, s8 bright) {
	u16 mode = 1 << 14;

	if (bright < 0) {
		mode = 2 << 14;
		bright = -bright;
	}
	if (bright > 31) {
		bright = 31;
	}
	*(vu16*)(0x0400006C + (0x1000 * screen)) = bright + mode;
}

extern u8 borderData[0x30000];
static u16 bmpImageBuffer[256*192] = {0};

void loadGbaBorder(void) {
	uint imageWidth, imageHeight;
	std::vector<unsigned char> image;
	lodepng::decode(image, imageWidth, imageHeight, borderData, sizeof(borderData));
	bool alternatePixel = false;

	for (uint i = 0; i < image.size()/4; i++) {
		image[(i*4)+3] = 0;
		if (alternatePixel) {
			if (image[(i*4)] >= 0x4 && image[(i*4)] < 0xFC) {
				image[(i*4)] += 0x4;
				image[(i*4)+3] |= BIT(0);
			}
			if (image[(i*4)+1] >= 0x4 && image[(i*4)+1] < 0xFC) {
				image[(i*4)+1] += 0x4;
				image[(i*4)+3] |= BIT(1);
			}
			if (image[(i*4)+2] >= 0x4 && image[(i*4)+2] < 0xFC) {
				image[(i*4)+2] += 0x4;
				image[(i*4)+3] |= BIT(2);
			}
		}
		bmpImageBuffer[i] = image[i*4]>>3 | (image[(i*4)+1]>>3)<<5 | (image[(i*4)+2]>>3)<<10 | BIT(15);
		if ((i % 256) == 255) alternatePixel = !alternatePixel;
		alternatePixel = !alternatePixel;
	}
	DC_FlushRange(bmpImageBuffer,SCREEN_WIDTH*SCREEN_HEIGHT*2);
	dmaCopy(bmpImageBuffer,(void*)BG_BMP_RAM(0),SCREEN_WIDTH*SCREEN_HEIGHT*2);

	alternatePixel = false;
	for (uint i = 0; i < image.size()/4; i++) {
		if (alternatePixel) {
			if (image[(i*4)+3] & BIT(0)) {
				image[(i*4)] -= 0x4;
			}
			if (image[(i*4)+3] & BIT(1)) {
				image[(i*4)+1] -= 0x4;
			}
			if (image[(i*4)+3] & BIT(2)) {
				image[(i*4)+2] -= 0x4;
			}
		} else {
			if (image[(i*4)] >= 0x4 && image[(i*4)] < 0xFC) {
				image[(i*4)] += 0x4;
			}
			if (image[(i*4)+1] >= 0x4 && image[(i*4)+1] < 0xFC) {
				image[(i*4)+1] += 0x4;
			}
			if (image[(i*4)+2] >= 0x4 && image[(i*4)+2] < 0xFC) {
				image[(i*4)+2] += 0x4;
			}
		}
		bmpImageBuffer[i] = image[i*4]>>3 | (image[(i*4)+1]>>3)<<5 | (image[(i*4)+2]>>3)<<10 | BIT(15);
		if ((i % 256) == 255) alternatePixel = !alternatePixel;
		alternatePixel = !alternatePixel;
	}
	DC_FlushRange(bmpImageBuffer,SCREEN_WIDTH*SCREEN_HEIGHT*2);
	dmaCopy(bmpImageBuffer,(void*)BG_BMP_RAM(8),SCREEN_WIDTH*SCREEN_HEIGHT*2);
}

void gbaSwitch(void) {
	irqDisable(IRQ_VBLANK);

	*(u16*)0x0400006C |= BIT(14);
	*(u16*)0x0400006C &= BIT(15);
	SetBrightness(0, 31);
	SetBrightness(1, 31);

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

	loadGbaBorder();

	// Switch to GBA mode
	runNdsFile ();	
}