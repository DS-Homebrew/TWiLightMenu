#include "gbaswitch.h"
#include "nds_loader_arm9.h"

//int gbaBorder = 1;

/*void loadGbaBorder(const char* filename) {
	FILE* file = fopen(filename, "rb");

	if (file) {
		// Start loading
		fseek(file, 0xe, SEEK_SET);
		u8 pixelStart = (u8)fgetc(file) + 0xe;
		fseek(file, pixelStart, SEEK_SET);
		for (int y=191; y>=0; y--) {
			u16 buffer[256];
			fread(buffer, 2, 0x100, file);
			u16* src = buffer;
			for (int i=0; i<256; i++) {
				u16 val = *(src++);
				VRAM_A[y*256+i] = ((val>>10)&0x1f) | ((val)&(0x1f<<5)) | (val&0x1f)<<10 | BIT(15);
				VRAM_B[y*256+i] = ((val>>10)&0x1f) | ((val)&(0x1f<<5)) | (val&0x1f)<<10 | BIT(15);
			}
		}
	}

	fclose(file);
}*/

void gbaSwitch(void) {
	irqDisable(IRQ_VBLANK);
	vramSetBankA(VRAM_A_MAIN_BG);
	vramSetBankB(VRAM_B_MAIN_BG);
	// Clear VRAM A and B to show black border for GBA mode
	for (u32 i = 0; i < 0x80000; i++) {
		*(u32*)(0x06000000+i) = 0;
		*(u32*)(0x06200000+i) = 0;
	}
	/*if (gbaBorder == 1) {
		loadGbaBorder("nitro:/gbaborder/nologo.bmp");
	}*/
	// Switch to GBA mode
	runNdsFile ("/_nds/TWiLightMenu/gbaswitch.srldr", 0, NULL, true, false, true, false, false);	
}