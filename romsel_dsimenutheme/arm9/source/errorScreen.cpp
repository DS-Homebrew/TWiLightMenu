#include <nds.h>
#include <stdio.h>
#include <maxmod9.h>

extern bool arm7SCFGLocked;

extern u16 bmpImageBuffer[256*192];
u16* sdRemovedImage = (u16*)0x026E0000;

extern u16 convertToDsBmp(u16 val);

void loadSdRemovedImage(void) {
	const char* filename = (arm7SCFGLocked ? "nitro:/graphics/sdRemovedSimple.bmp" : "nitro:/graphics/sdRemoved.bmp");
	FILE* file = fopen(filename, "rb");
	if (file) {
		// Start loading
		fseek(file, 0xe, SEEK_SET);
		u8 pixelStart = (u8)fgetc(file) + 0xe;
		fseek(file, pixelStart, SEEK_SET);
		fread(bmpImageBuffer, 2, 0x18000, file);
		u16* src = bmpImageBuffer;
		int x = 0;
		int y = 191;
		for (int i=0; i<256*192; i++) {
			if (x >= 256) {
				x = 0;
				y--;
			}
			u16 val = *(src++);
			sdRemovedImage[y*256+x] = convertToDsBmp(val);
			x++;
		}
	}
	fclose(file);
}

void checkSdEject(void) {
	if (*(u8*)(0x023FF002) == 0) return;
	
	// Show "SD removed" screen
	irqDisable(IRQ_VBLANK);
	mmEffectCancelAll();

	videoSetMode(MODE_3_2D | DISPLAY_BG3_ACTIVE);
	videoSetModeSub(MODE_3_2D | DISPLAY_BG3_ACTIVE);

	REG_BLDY = 0;

	dmaCopyWordsAsynch(0, sdRemovedImage, BG_GFX, 0x18000);
	dmaFillWords(0, BG_GFX_SUB, 0x18000);

	while(1) {
		swiWaitForVBlank();
	}
}