#include "gbaswitch.h"
#include "nds_loader_arm9.h"

void gbaSwitch(void)
{
    irqDisable(IRQ_VBLANK);
    vramSetBankA(VRAM_A_MAIN_BG);
    vramSetBankB(VRAM_B_MAIN_BG);
    // Clear VRAM A and B to show black border for GBA mode
	dmaFillHalfWords(0, (u16*)0x06000000, 0x80000);
	dmaFillHalfWords(0, (u16*)0x06200000, 0x80000);
    /*if (gbaBorder == 1) {
		loadGbaBorder("nitro:/gbaborder/nologo.bmp");
	}*/
    // Switch to GBA mode
    runNdsFile("/_nds/TWiLightMenu/gbaswitch.srldr", 0, NULL, true, false, true, false, false);
}