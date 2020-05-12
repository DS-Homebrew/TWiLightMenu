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

#include <nds/debug.h>
#include <nds/system.h>

#include "hook.h"
#include "common.h"
#include "cardengine_arm7_bin.h"

extern unsigned long language;
extern bool gameSoftReset;

extern unsigned long cheat_engine_size;
extern unsigned long intr_orig_return_offset;

static const u32 handlerStartSig[5] = {
	0xe92d4000, 	// push {lr}
	0xe3a0c301, 	// mov  ip, #0x4000000
	0xe28cce21,		// add  ip, ip, #0x210
	0xe51c1008,		// ldr	r1, [ip, #-8]
	0xe3510000		// cmp	r1, #0
};

static const u32 handlerEndSig[4] = {
	0xe59f1008, 	// ldr  r1, [pc, #8]	(IRQ Vector table address)
	0xe7910100,		// ldr  r0, [r1, r0, lsl #2]
	0xe59fe004,		// ldr  lr, [pc, #4]	(IRQ return address)
	0xe12fff10		// bx   r0
};

static const int MAX_HANDLER_SIZE = 50;

static u32* hookInterruptHandler (u32* addr, size_t size) {
	u32* end = addr + size/sizeof(u32);
	int i;

	// Find the start of the handler
	while (addr < end) {
		if ((addr[0] == handlerStartSig[0]) && 
			(addr[1] == handlerStartSig[1]) && 
			(addr[2] == handlerStartSig[2]) && 
			(addr[3] == handlerStartSig[3]) && 
			(addr[4] == handlerStartSig[4])) 
		{
			break;
		}
		addr++;
	}

	if (addr >= end) {
		return NULL;
	}

	// Find the end of the handler
	for (i = 0; i < MAX_HANDLER_SIZE; i++) {
		if ((addr[i+0] == handlerEndSig[0]) && 
			(addr[i+1] == handlerEndSig[1]) && 
			(addr[i+2] == handlerEndSig[2]) && 
			(addr[i+3] == handlerEndSig[3])) 
		{
			break;
		}
	}

	if (i >= MAX_HANDLER_SIZE) {
		return NULL;
	}

	// Now find the IRQ vector table
	// Make addr point to the vector table address pointer within the IRQ handler
	addr = addr + i + sizeof(handlerEndSig)/sizeof(handlerEndSig[0]);

	// Use relative and absolute addresses to find the location of the table in RAM
	u32 tableAddr = addr[0];
	u32 returnAddr = addr[1];
	u32* actualReturnAddr = addr + 2;
	u32* actualTableAddr = actualReturnAddr + (tableAddr - returnAddr)/sizeof(u32);

	// The first entry in the table is for the Vblank handler, which is what we want
	return actualTableAddr;
	// 2     LCD V-Counter Match
}


int hookNdsRetail (const tNDSHeader* ndsHeader, u32* cardEngineLocation) {
	u32* hookLocation = hookInterruptHandler((u32*)ndsHeader->arm7destination, ndsHeader->arm7binarySize);

	// SDK 5
	if (!hookLocation && ndsHeader->unitCode != 0) {
		switch (ndsHeader->arm7binarySize) {
			case 0x0001D5A8:
				hookLocation = (u32*)0x239D280;		// DS WiFi Settings
				break;

			case 0x00022B40:
				hookLocation = (u32*)0x238DED8;
				break;

			case 0x00022BCC:
				hookLocation = (u32*)0x238DF60;
				break;

			case 0x00025664:
				hookLocation = (u32*)0x23A5330;		// DSi-Exclusive cart games
				break;

			case 0x000257DC:
				hookLocation = (u32*)0x23A54B8;		// DSi-Exclusive cart games
				break;

			case 0x00025860:
				hookLocation = (u32*)0x23A5538;		// DSi-Exclusive cart games
				break;

			case 0x00026DF4:
				hookLocation = (u32*)0x23A6AD4;		// DSi-Exclusive cart games
				break;

			case 0x00028F84:
				hookLocation = (u32*)0x2391918;
				break;

			case 0x0002909C:
				hookLocation = (u32*)0x2391A30;
				break;

			case 0x0002914C:
			case 0x00029164:
				hookLocation = (u32*)0x2391ADC;
				break;

			case 0x00029EE8:
				hookLocation = (u32*)0x2391F70;
				break;

			case 0x0002A2EC:
				hookLocation = (u32*)0x23921BC;
				break;

			case 0x0002A318:
				hookLocation = (u32*)0x23921D8;
				break;

			case 0x0002AF18:
				hookLocation = (u32*)0x239227C;
				break;

			case 0x0002B184:
				hookLocation = (u32*)0x23924CC;
				break;

			case 0x0002B24C:
				hookLocation = (u32*)0x2392578;
				break;

			case 0x0002C5B4:
				hookLocation = (u32*)0x2392E74;
				break;
		}
	}

	if (!hookLocation) {
		return ERR_HOOK;
	}

	u32* vblankHandler = hookLocation;

	cardEngineLocation[1] = *vblankHandler;
	cardEngineLocation[2] = language;
	cardEngineLocation[3] = gameSoftReset;

	u32* patches =  (u32*) cardEngineLocation[0];

	*vblankHandler = patches[0];

	nocashMessage("ERR_NONE");
	return ERR_NONE;
}
