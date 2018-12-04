/*
    NitroHax -- Cheat tool for the Nintendo DS
    Copyright (C) 2008  Michael "Chishm" Chisholm

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <nds.h>
#include <maxmod9.h>

extern bool fadeType;

#include "graphics/graphics.h"

#include "soundbank.h"
#include "soundbank_bin.h"
 
#include "graphics/bios_decompress_callback.h"

#include "DSi01.h"
#include "DSi02.h"
#include "DSi03.h"
#include "DSi04.h"
#include "DSi05.h"
#include "DSi06.h"
#include "DSi07.h"
#include "DSi08.h"
#include "DSi09.h"
#include "DSi10.h"
#include "DSi11.h"
#include "DSi12.h"
#include "DSi13.h"
#include "DSi14.h"
#include "DSi15.h"
#include "DSi16.h"
#include "DSi17.h"
#include "DSi18.h"
#include "DSi19.h"
#include "DSi20.h"
#include "DSi21.h"
#include "DSi22.h"
#include "DSi23.h"
#include "DSi24.h"
#include "DSi25.h"
#include "DSi26.h"
#include "DSi27.h"
#include "DSi28.h"

#include "bootsplash.h"

#define CONSOLE_SCREEN_WIDTH 32
#define CONSOLE_SCREEN_HEIGHT 24

void BootJingleDSi() {
	
	mmInitDefaultMem((mm_addr)soundbank_bin);

	mmLoadEffect( SFX_DSIBOOT );

	mm_sound_effect dsiboot = {
		{ SFX_DSIBOOT } ,			// id
		(int)(1.0f * (1<<10)),	// rate
		0,		// handle
		255,	// volume
		128,	// panning
	};
	
	mmEffectEx(&dsiboot);
}

void BootSplashDSi(void) {

	swiDecompressLZSSVram ((void*)DSi01Tiles, (void*)CHAR_BASE_BLOCK(2), 0, &decompressBiosCallback);
	swiDecompressLZSSVram ((void*)DSi01Tiles, (void*)CHAR_BASE_BLOCK_SUB(2), 0, &decompressBiosCallback);
	vramcpy_ui (&BG_PALETTE[0], DSi01Pal, DSi01PalLen);
	vramcpy_ui (&BG_PALETTE_SUB[0], DSi01Pal, DSi01PalLen);

	for (int i = 0; i < 2; i++) { swiWaitForVBlank(); }
	clearBrightness();

	swiDecompressLZSSVram ((void*)DSi02Tiles, (void*)CHAR_BASE_BLOCK(2), 0, &decompressBiosCallback);
	vramcpy_ui (&BG_PALETTE[0], DSi02Pal, DSi02PalLen);

	for (int i = 0; i < 2; i++) { swiWaitForVBlank(); }
	
	swiDecompressLZSSVram ((void*)DSi03Tiles, (void*)CHAR_BASE_BLOCK(2), 0, &decompressBiosCallback);
	vramcpy_ui (&BG_PALETTE[0], DSi03Pal, DSi03PalLen);
	
	for (int i = 0; i < 2; i++) { swiWaitForVBlank(); }
	
	swiDecompressLZSSVram ((void*)DSi04Tiles, (void*)CHAR_BASE_BLOCK(2), 0, &decompressBiosCallback);
	vramcpy_ui (&BG_PALETTE[0], DSi04Pal, DSi04PalLen);

	BootJingleDSi();

	for (int i = 0; i < 2; i++) { swiWaitForVBlank(); }

	swiDecompressLZSSVram ((void*)DSi05Tiles, (void*)CHAR_BASE_BLOCK(2), 0, &decompressBiosCallback);
	vramcpy_ui (&BG_PALETTE[0], DSi05Pal, DSi05PalLen);

	for (int i = 0; i < 2; i++) { swiWaitForVBlank(); }
	
	swiDecompressLZSSVram ((void*)DSi06Tiles, (void*)CHAR_BASE_BLOCK(2), 0, &decompressBiosCallback);
	vramcpy_ui (&BG_PALETTE[0], DSi06Pal, DSi06PalLen);
		
	for (int i = 0; i < 2; i++) { swiWaitForVBlank(); }

	swiDecompressLZSSVram ((void*)DSi07Tiles, (void*)CHAR_BASE_BLOCK(2), 0, &decompressBiosCallback);
	vramcpy_ui (&BG_PALETTE[0], DSi07Pal, DSi07PalLen);

	for (int i = 0; i < 2; i++) { swiWaitForVBlank(); }

	swiDecompressLZSSVram ((void*)DSi08Tiles, (void*)CHAR_BASE_BLOCK(2), 0, &decompressBiosCallback);
	vramcpy_ui (&BG_PALETTE[0], DSi08Pal, DSi08PalLen);

	for (int i = 0; i < 2; i++) { swiWaitForVBlank(); }
	
	swiDecompressLZSSVram ((void*)DSi09Tiles, (void*)CHAR_BASE_BLOCK(2), 0, &decompressBiosCallback);
	vramcpy_ui (&BG_PALETTE[0], DSi09Pal, DSi09PalLen);

	for (int i = 0; i < 2; i++) { swiWaitForVBlank(); }

	swiDecompressLZSSVram ((void*)DSi10Tiles, (void*)CHAR_BASE_BLOCK(2), 0, &decompressBiosCallback);
	vramcpy_ui (&BG_PALETTE[0], DSi10Pal, DSi10PalLen);

	for (int i = 0; i < 2; i++) { swiWaitForVBlank(); }

	swiDecompressLZSSVram ((void*)DSi11Tiles, (void*)CHAR_BASE_BLOCK(2), 0, &decompressBiosCallback);
	vramcpy_ui (&BG_PALETTE[0], DSi11Pal, DSi11PalLen);

	for (int i = 0; i < 2; i++) { swiWaitForVBlank(); }

	swiDecompressLZSSVram ((void*)DSi12Tiles, (void*)CHAR_BASE_BLOCK(2), 0, &decompressBiosCallback);
	vramcpy_ui (&BG_PALETTE[0], DSi12Pal, DSi12PalLen);
	
	for (int i = 0; i < 2; i++) { swiWaitForVBlank(); }

	swiDecompressLZSSVram ((void*)DSi13Tiles, (void*)CHAR_BASE_BLOCK(2), 0, &decompressBiosCallback);
	vramcpy_ui (&BG_PALETTE[0], DSi13Pal, DSi13PalLen);

	for (int i = 0; i < 2; i++) { swiWaitForVBlank(); }

	swiDecompressLZSSVram ((void*)DSi14Tiles, (void*)CHAR_BASE_BLOCK(2), 0, &decompressBiosCallback);
	vramcpy_ui (&BG_PALETTE[0], DSi14Pal, DSi14PalLen);

	for (int i = 0; i < 2; i++) { swiWaitForVBlank(); }

	swiDecompressLZSSVram ((void*)DSi15Tiles, (void*)CHAR_BASE_BLOCK(2), 0, &decompressBiosCallback);
	vramcpy_ui (&BG_PALETTE[0], DSi15Pal, DSi15PalLen);

	for (int i = 0; i < 2; i++) { swiWaitForVBlank(); }

	swiDecompressLZSSVram ((void*)DSi16Tiles, (void*)CHAR_BASE_BLOCK(2), 0, &decompressBiosCallback);
	vramcpy_ui (&BG_PALETTE[0], DSi16Pal, DSi16PalLen);

	for (int i = 0; i < 2; i++) { swiWaitForVBlank(); }

	swiDecompressLZSSVram ((void*)DSi17Tiles, (void*)CHAR_BASE_BLOCK(2), 0, &decompressBiosCallback);
	vramcpy_ui (&BG_PALETTE[0], DSi17Pal, DSi17PalLen);

	for (int i = 0; i < 2; i++) { swiWaitForVBlank(); }
	
	swiDecompressLZSSVram ((void*)DSi18Tiles, (void*)CHAR_BASE_BLOCK(2), 0, &decompressBiosCallback);
	vramcpy_ui (&BG_PALETTE[0], DSi18Pal, DSi18PalLen);

	for (int i = 0; i < 2; i++) { swiWaitForVBlank(); }

	swiDecompressLZSSVram ((void*)DSi19Tiles, (void*)CHAR_BASE_BLOCK(2), 0, &decompressBiosCallback);
	vramcpy_ui (&BG_PALETTE[0], DSi19Pal, DSi19PalLen);

	for (int i = 0; i < 2; i++) { swiWaitForVBlank(); }

	swiDecompressLZSSVram ((void*)DSi20Tiles, (void*)CHAR_BASE_BLOCK(2), 0, &decompressBiosCallback);
	vramcpy_ui (&BG_PALETTE[0], DSi20Pal, DSi20PalLen);

	for (int i = 0; i < 2; i++) { swiWaitForVBlank(); }

	swiDecompressLZSSVram ((void*)DSi21Tiles, (void*)CHAR_BASE_BLOCK(2), 0, &decompressBiosCallback);
	vramcpy_ui (&BG_PALETTE[0], DSi21Pal, DSi21PalLen);

	for (int i = 0; i < 2; i++) { swiWaitForVBlank(); }

	swiDecompressLZSSVram ((void*)DSi22Tiles, (void*)CHAR_BASE_BLOCK(2), 0, &decompressBiosCallback);
	vramcpy_ui (&BG_PALETTE[0], DSi22Pal, DSi22PalLen);

	for (int i = 0; i < 2; i++) { swiWaitForVBlank(); }

	swiDecompressLZSSVram ((void*)DSi23Tiles, (void*)CHAR_BASE_BLOCK(2), 0, &decompressBiosCallback);
	vramcpy_ui (&BG_PALETTE[0], DSi23Pal, DSi23PalLen);

	for (int i = 0; i < 2; i++) { swiWaitForVBlank(); }

	swiDecompressLZSSVram ((void*)DSi24Tiles, (void*)CHAR_BASE_BLOCK(2), 0, &decompressBiosCallback);
	vramcpy_ui (&BG_PALETTE[0], DSi24Pal, DSi24PalLen);

	for (int i = 0; i < 2; i++) { swiWaitForVBlank(); }

	swiDecompressLZSSVram ((void*)DSi25Tiles, (void*)CHAR_BASE_BLOCK(2), 0, &decompressBiosCallback);
	vramcpy_ui (&BG_PALETTE[0], DSi25Pal, DSi25PalLen);

	for (int i = 0; i < 2; i++) { swiWaitForVBlank(); }

	swiDecompressLZSSVram ((void*)DSi26Tiles, (void*)CHAR_BASE_BLOCK(2), 0, &decompressBiosCallback);
	vramcpy_ui (&BG_PALETTE[0], DSi26Pal, DSi26PalLen);

	for (int i = 0; i < 2; i++) { swiWaitForVBlank(); }

	swiDecompressLZSSVram ((void*)DSi27Tiles, (void*)CHAR_BASE_BLOCK(2), 0, &decompressBiosCallback);
	vramcpy_ui (&BG_PALETTE[0], DSi27Pal, DSi27PalLen);

	for (int i = 0; i < 2; i++) { swiWaitForVBlank(); }

	swiDecompressLZSSVram ((void*)DSi28Tiles, (void*)CHAR_BASE_BLOCK(2), 0, &decompressBiosCallback);
	vramcpy_ui (&BG_PALETTE[0], DSi28Pal, DSi28PalLen);

	// Pause on frame 31 for a second		
	for (int i = 0; i < 80; i++) { swiWaitForVBlank(); }

	// Fade out
	fadeType = false;
	for (int i = 0; i < 30; i++) { swiWaitForVBlank(); }
}

void BootSplashInit(void) {

	videoSetMode(MODE_0_2D | DISPLAY_BG0_ACTIVE);
	videoSetModeSub(MODE_0_2D | DISPLAY_BG0_ACTIVE);
	vramSetBankA (VRAM_A_MAIN_BG_0x06000000);
	vramSetBankC (VRAM_C_SUB_BG_0x06200000);
	REG_BG0CNT = BG_MAP_BASE(0) | BG_COLOR_256 | BG_TILE_BASE(2);
	REG_BG0CNT_SUB = BG_MAP_BASE(0) | BG_COLOR_256 | BG_TILE_BASE(2);
	BG_PALETTE[0]=0;
	BG_PALETTE[255]=0xffff;
	u16* bgMapTop = (u16*)SCREEN_BASE_BLOCK(0);
	u16* bgMapSub = (u16*)SCREEN_BASE_BLOCK_SUB(0);
	for (int i = 0; i < CONSOLE_SCREEN_WIDTH*CONSOLE_SCREEN_HEIGHT; i++) {
		bgMapTop[i] = (u16)i;
		bgMapSub[i] = (u16)i;
	}

	BootSplashDSi();
}

