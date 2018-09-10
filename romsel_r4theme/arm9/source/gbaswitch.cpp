#include "gbaswitch.h"
#include "nds_loader_arm9.h"

//#include "gbaframe.h"
//#include "gbaframe_srloader.h"

//int gbaBorder = 0;

void gbaSwitch(void) {
	vramSetBankA(VRAM_A_MAIN_BG);
	vramSetBankB(VRAM_B_MAIN_BG);
	/*if (gbaBorder == 2) {
		int bg3 = bgInit(3, BgType_Bmp8, BgSize_B8_256x256, 0,0);

		dmaCopy(gbaframe_srloaderBitmap, bgGetGfxPtr(bg3), 256*256);
		dmaCopy(gbaframe_srloaderPal, BG_PALETTE, 256*2);
	} else if (gbaBorder == 1) {
		int bg3 = bgInit(3, BgType_Bmp8, BgSize_B8_256x256, 0,0);

		dmaCopy(gbaframeBitmap, bgGetGfxPtr(bg3), 256*256);
		dmaCopy(gbaframePal, BG_PALETTE, 256*2);
	} else {*/
		// Clear VRAM A and B to show black border for GBA mode
		for (u32 i = 0; i < 0x80000; i++) {
			*(u32*)(0x06000000+i) = 0;
			*(u32*)(0x06200000+i) = 0;
		}
	//}
	// Switch to GBA mode
	runNdsFile ("/_nds/dsimenuplusplus/gbaswitch.srldr", 0, NULL, false, true, false, false);	
}