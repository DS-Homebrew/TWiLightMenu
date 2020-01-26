#include "graphics/ThemeTextures.h"
#include "graphics/lodepng.h"
#include "gbaswitch.h"
#include "nds_loader_arm9.h"

int gbaBorder = 1;

void loadGbaBorder(const char* filename) {
	uint imageWidth, imageHeight;
	std::vector<unsigned char> image;

	lodepng::decode(image, imageWidth, imageHeight, filename);

	for(uint i=0;i<image.size()/4;i++) {
		tex().bmpImageBuffer()[i] = image[i*4]>>3 | (image[(i*4)+1]>>3)<<5 | (image[(i*4)+2]>>3)<<10 | BIT(15);
	}

    DC_FlushRange(tex().bmpImageBuffer(),SCREEN_WIDTH*SCREEN_HEIGHT*2);
    dmaCopy(tex().bmpImageBuffer(),(void*)BG_BMP_RAM(0),SCREEN_WIDTH*SCREEN_HEIGHT*2);
    dmaCopy(tex().bmpImageBuffer(),(void*)BG_BMP_RAM(8),SCREEN_WIDTH*SCREEN_HEIGHT*2);
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

  memset((void*)BG_BMP_RAM(0),0,0x18000);
  memset((void*)BG_BMP_RAM(8),0,0x18000);

	if (gbaBorder == 1) {
		loadGbaBorder("nitro:/graphics/gbaborder.png");
	}
	// Switch to GBA mode
	runNdsFile ("/_nds/TWiLightMenu/gbaswitch.srldr", 0, NULL, true, false, true, false, false);	
}