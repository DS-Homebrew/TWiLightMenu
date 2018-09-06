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

#define LCDC_BANK_C (u16*)0x06840000

#define LANGUAGE_OFFSET 4
#define SDACCESS_OFFSET 8
#define TWLMODE_OFFSET 12
#define TWLCLOCK_OFFSET 16
#define RUNCARDENGINE_OFFSET 20

typedef signed int addr_t;
typedef unsigned char data_t;

static addr_t readAddr (data_t *mem, addr_t offset) {
	return ((addr_t*)mem)[offset/sizeof(addr_t)];
}

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

// Basic engine with no cheat related code.
void runLaunchEngine (bool EnableSD, int language, bool TWLMODE, bool TWLCLK, bool TWLVRAM, bool runCardEngine)
{

	nocashMessage("runLaunchEngine");

	irqDisable(IRQ_ALL);

	// Direct CPU access to VRAM bank C
	VRAM_C_CR = VRAM_ENABLE | VRAM_C_LCD;

	// Clear VRAM
	memset (LCDC_BANK_C, 0x00, 128 * 1024);

	// Load the loader/patcher into the correct address
	vramcpy (LCDC_BANK_C, load_bin, load_bin_size);

	// Set the parameters for the loader
	writeAddr ((data_t*) LCDC_BANK_C, LANGUAGE_OFFSET, language);
	writeAddr ((data_t*) LCDC_BANK_C, SDACCESS_OFFSET, EnableSD);
	writeAddr ((data_t*) LCDC_BANK_C, TWLMODE_OFFSET, TWLMODE);
	writeAddr ((data_t*) LCDC_BANK_C, TWLCLOCK_OFFSET, TWLCLK);
	writeAddr ((data_t*) LCDC_BANK_C, RUNCARDENGINE_OFFSET, runCardEngine);

	nocashMessage("irqDisable(IRQ_ALL);");

	irqDisable(IRQ_ALL);

	nocashMessage("Give the VRAM to the ARM7");
	// Give the VRAM to the ARM7
	VRAM_C_CR = VRAM_ENABLE | VRAM_C_ARM7_0x06000000;
	
	if (TWLMODE) {
		if (TWLVRAM) {
			REG_SCFG_EXT=0x83002000;
		} else {
			REG_SCFG_EXT=0x83000000;
		}
	} else {
		if (TWLVRAM) {
			REG_SCFG_EXT=0x03002000;
		} else {
			REG_SCFG_EXT=0x03000000;
		}
	}
	
	nocashMessage("Reset into a passme loop");
	// Reset into a passme loop
	REG_EXMEMCNT |= ARM7_OWNS_ROM | ARM7_OWNS_CARD;

	*((vu32*)0x02FFFFFC) = 0;
	*((vu32*)0x02FFFE04) = (u32)0xE59FF018;
	*((vu32*)0x02FFFE24) = (u32)0x02FFFE04;

	nocashMessage("resetARM7");

	resetARM7(0x06000000);	

	nocashMessage("swiSoftReset");

	swiSoftReset(); 
}

