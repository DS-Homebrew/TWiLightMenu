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

#include <string.h>
#include <nds.h>

#include "load_bin.h"
#include "launch_engine.h"

#define LCDC_BANK_D (u16*)0x06860000

#define DSIMODE_OFFSET 4
#define LANGUAGE_OFFSET 8
#define SDACCESS_OFFSET 12
#define SCFGUNLOCK_OFFSET 16
#define TWLMODE_OFFSET 20
#define TWLCLOCK_OFFSET 24
#define SOUNDFREQ_OFFSET 28
#define RUNCARDENGINE_OFFSET 32

typedef signed int addr_t;
typedef unsigned char data_t;

/*static addr_t readAddr (data_t *mem, addr_t offset) {
	return ((addr_t*)mem)[offset/sizeof(addr_t)];
}*/

static void writeAddr (data_t *mem, addr_t offset, addr_t value) {
	((addr_t*)mem)[offset/sizeof(addr_t)] = value;
}

void vramcpy (void* dst, const void* src, int len)
{
	u16* dst16 = (u16*)dst;
	u16* src16 = (u16*)src;
	
	for ( ; len > 0; len -= 2) {
		*dst16++ = *src16++;
	}
}	

void runLaunchEngine (bool EnableSD, int language, bool isDSBrowser, bool scfgUnlock, bool TWLMODE, bool TWLCLK, bool TWLVRAM, bool soundFreq, bool runCardEngine)
{
	nocashMessage("runLaunchEngine");

	irqDisable(IRQ_ALL);

	// Direct CPU access to VRAM bank D
	VRAM_D_CR = VRAM_ENABLE | VRAM_D_LCD;

	// Clear VRAM
	memset (LCDC_BANK_D, 0x00, 128 * 1024);

	// Load the loader/patcher into the correct address
	vramcpy (LCDC_BANK_D, load_bin, load_bin_size);

	// Set the parameters for the loader
	writeAddr ((data_t*) LCDC_BANK_D, DSIMODE_OFFSET, isDSiMode());
	writeAddr ((data_t*) LCDC_BANK_D, LANGUAGE_OFFSET, language);
	writeAddr ((data_t*) LCDC_BANK_D, SDACCESS_OFFSET, EnableSD);
	writeAddr ((data_t*) LCDC_BANK_D, SCFGUNLOCK_OFFSET, scfgUnlock);
	writeAddr ((data_t*) LCDC_BANK_D, TWLMODE_OFFSET, TWLMODE);
	writeAddr ((data_t*) LCDC_BANK_D, TWLCLOCK_OFFSET, TWLCLK);
	writeAddr ((data_t*) LCDC_BANK_D, SOUNDFREQ_OFFSET, soundFreq);
	writeAddr ((data_t*) LCDC_BANK_D, RUNCARDENGINE_OFFSET, runCardEngine);

	nocashMessage("irqDisable(IRQ_ALL);");
	irqDisable(IRQ_ALL);

	if (isDSiMode()) {
		if (scfgUnlock || TWLMODE) {
			if (TWLVRAM) {
				REG_SCFG_EXT = (isDSBrowser ? 0x8300E000 : 0x83002000);
			} else {
				REG_SCFG_EXT = (isDSBrowser ? 0x8300C000 : 0x83000000);
			}
		} else {
			if (TWLVRAM) {
				REG_SCFG_EXT = (isDSBrowser ? 0x0300E000 : 0x03002000);
			} else {
				REG_SCFG_EXT = (isDSBrowser ? 0x0300C000 : 0x03000000);
			}
		}
	}
	
	// Give the VRAM to the ARM7
	nocashMessage("Give the VRAM to the ARM7");
	VRAM_D_CR = VRAM_ENABLE | VRAM_D_ARM7_0x06020000;
	
	// Reset into a passme loop
	nocashMessage("Reset into a passme loop");
	REG_EXMEMCNT |= ARM7_OWNS_ROM | ARM7_OWNS_CARD;
	
	*(vu32*)0x02FFFFFC = 0;

	// Return to passme loop
	*(vu32*)0x02FFFE04 = (u32)0xE59FF018; // ldr pc, 0x02FFFE24
	*(vu32*)0x02FFFE24 = (u32)0x02FFFE04;  // Set ARM9 Loop address --> resetARM9(0x02FFFE04);
	
	// Reset ARM7
	nocashMessage("resetARM7");
	resetARM7(0x06020000);	

	// swi soft reset
	nocashMessage("swiSoftReset");
	swiSoftReset();
}

