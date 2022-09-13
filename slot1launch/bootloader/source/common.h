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

#ifndef _COMMON_H
#define _COMMON_H

#include <nds/dma.h>
#include <stdlib.h>

#define resetCpu() \
		__asm volatile("swi 0x000000")

// ERROR_CODES
//enum {
#define ERR_NONE 0x00
#define ERR_STS_CLR_MEM 0x01
#define ERR_STS_LOAD_BIN 0x02
#define ERR_STS_HOOK_BIN 0x03
#define ERR_STS_START 0x04
// initCard error codes:
#define ERR_LOAD_NORM 0x11
#define ERR_LOAD_OTHR 0x12
#define ERR_SEC_NORM 0x13
#define ERR_SEC_OTHR 0x14
#define ERR_LOGO_CRC 0x15
#define ERR_HEAD_CRC 0x16
// hookARM7Binary error codes:
#define ERR_NOCHEAT 0x21
#define ERR_HOOK 0x22
//};

//ARM9_STATE
//enum {
#define ARM9_BOOT 0
#define ARM9_START 1
#define ARM9_MEMCLR 2
#define ARM9_READY 3
#define ARM9_BOOTBIN 4
#define ARM9_DISPERR 5
#define ARM9_SETSCFG 6
//};

extern tNDSHeader* ndsHeader;
extern bool dsiModeConfirmed;
extern bool arm9_boostVram;
extern bool arm9_scfgUnlock;
extern bool arm9_extendedMemory;
extern bool arm9_isSdk5;
extern volatile int arm9_stateFlag;
extern volatile u32 arm9_errorCode;
extern volatile bool arm9_errorClearBG;

static inline void dmaFill(const void* src, void* dest, uint32 size) {
	DMA_SRC(3)  = (uint32)src;
	DMA_DEST(3) = (uint32)dest;
	DMA_CR(3)   = DMA_COPY_WORDS | DMA_SRC_FIX | (size>>2);
	while (DMA_CR(3) & DMA_BUSY);
}

static inline void copyLoop (u32* dest, const u32* src, size_t size) {
	do {
		*dest++ = *src++;
	} while (size -= 4);
}

#endif // _COMMON_H

