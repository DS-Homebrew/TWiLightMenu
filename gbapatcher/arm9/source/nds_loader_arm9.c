/*-----------------------------------------------------------------
 Copyright (C) 2005 - 2010
	Michael "Chishm" Chisholm
	Dave "WinterMute" Murphy

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <nds.h>
#include <nds/arm9/dldi.h>
#include <sys/stat.h>
#include <limits.h>

#include <unistd.h>

#include "tonccpy.h"
#include "load_bin.h"

#ifndef _NO_BOOTSTUB_
#include "bootstub_bin.h"
#endif

#include "nds_loader_arm9.h"
#define LCDC_BANK_C (u16*)0x06840000
#define STORED_FILE_CLUSTER (*(((u32*)LCDC_BANK_C) + 1))
#define INIT_DISC (*(((u32*)LCDC_BANK_C) + 2))
#define WANT_TO_PATCH_DLDI (*(((u32*)LCDC_BANK_C) + 3))


/*
	b	startUp
	
storedFileCluster:
	.word	0x0FFFFFFF		@ default BOOT.NDS
initDisc:
	.word	0x00000001		@ init the disc by default
wantToPatchDLDI:
	.word	0x00000001		@ by default patch the DLDI section of the loaded NDS
@ Used for passing arguments to the loaded app
argStart:
	.word	_end - _start
argSize:
	.word	0x00000000
dldiOffset:
	.word	_dldi_start - _start
dsiSD:
	.word	0
dsiMode:
	.word	0
*/

#define STORED_FILE_CLUSTER_OFFSET 4 
#define INIT_DISC_OFFSET 8
#define WANT_TO_PATCH_DLDI_OFFSET 12
#define ARG_START_OFFSET 16
#define ARG_SIZE_OFFSET 20
#define HAVE_DSISD_OFFSET 28
#define DSIMODE_OFFSET 32
#define CLEAR_MASTER_BRIGHT_OFFSET 36
#define DSMODE_SWITCH_OFFSET 40
#define LOADFROMRAM_OFFSET 44


int runNds (const void* loader, u32 loaderSize)
{
	irqDisable(IRQ_ALL);

	// Direct CPU access to VRAM bank C
	VRAM_C_CR = VRAM_ENABLE | VRAM_C_LCD;
	// Load the loader/patcher into the correct address
	tonccpy (LCDC_BANK_C, loader, loaderSize);

	// Set the parameters for the loader
	// STORED_FILE_CLUSTER = cluster;
	// INIT_DISC = initDisc;

	irqDisable(IRQ_ALL);
	
	// Give the VRAM to the ARM7
	VRAM_C_CR = VRAM_ENABLE | VRAM_C_ARM7_0x06000000;	
	// Reset into a passme loop
	REG_EXMEMCNT |= ARM7_OWNS_ROM | ARM7_OWNS_CARD;
	*((vu32*)0x02FFFFFC) = 0;
	*((vu32*)0x02FFFE04) = (u32)0xE59FF018;
	*((vu32*)0x02FFFE24) = (u32)0x02FFFE04;

	resetARM7(0x06000000);

	swiSoftReset(); 
	return true;
}

int runNdsFile (void)  {
	return runNds (load_bin, load_bin_size);
}
