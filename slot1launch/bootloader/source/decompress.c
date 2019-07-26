/*
	Copyright (C) 2008 somebody
	Copyright (C) 2009 yellow wood goblin
	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.
	You should have received a copy of the GNU General Public License
	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/
#include <nds/ndstypes.h>
#include <nds/memory.h> // tNDSHeader
#include <stddef.h>
#include "module_params.h"

/*static void decompressLZ77Backwards(u8* addr, u32 size) {
	u32 len = *(u32*)(addr + size - 4) + size;
	
	if(len == size) {
		size -= 12;
	}
		
	len = *(u32*)(addr + size - 4) + size;
	
	//byte[] Result = new byte[len];
	//Array.Copy(Data, Result, Data.Length);

	u32 end = *(u32*)(addr + size - 8) & 0xFFFFFF;

	u8* result = addr;

	int Offs = (int)(size - (*(u32*)(addr + size - 8) >> 24));
	int dstoffs = (int)len;
	while (true) {
		u8 header = result[--Offs];
		for (int i = 0; i < 8; i++) {
			if ((header & 0x80) == 0) {
				result[--dstoffs] = result[--Offs];
			} else {
				u8 a = result[--Offs];
				u8 b = result[--Offs];
				int offs = (((a & 0xF) << 8) | b) + 2;//+ 1;
				int length = (a >> 4) + 2;
				do {
					result[dstoffs - 1] = result[dstoffs + offs];
					dstoffs--;
					length--;
				} while (length >= 0);
			}

			if (Offs <= size - end) {
				return;
			}

			header <<= 1;
		}
	}
}*/

//static u32 iUncompressedSize = 0;
static u32 iFixedAddr = 0;
static u32 iFixedData = 0;

static u32 decompressBinary(u8 *aMainMemory, u32 aCodeLength, u32 aMemOffset) {
	u8 *ADDR1 = NULL;
	u8 *ADDR1_END = NULL;
	u8 *ADDR2 = NULL;
	u8 *ADDR3 = NULL;

	u8 *pBuffer32 = (u8 *)(aMainMemory);
	u8 *pBuffer32End = (u8 *)(aMainMemory + aCodeLength);

	while (pBuffer32 < pBuffer32End) {
		if (0xDEC00621 == *(u32 *)pBuffer32 && 0x2106C0DE == *(u32 *)(pBuffer32 + 4)) {
			ADDR1 = (u8 *)(*(u32 *)(pBuffer32 - 8));
			iFixedAddr = (u32)(pBuffer32 - 8);
			iFixedData = *(u32 *)(pBuffer32 - 8);
			*(u32 *)(pBuffer32 - 8) = 0;
			break;
		}
		pBuffer32 += 4;
	}
	if (0 == ADDR1) {
		iFixedAddr = 0;
		return 0;
	}

	u32 A = *(u32 *)(ADDR1 + aMemOffset - 4);
	u32 B = *(u32 *)(ADDR1 + aMemOffset - 8);
	ADDR1_END = ADDR1 + A;
	ADDR2 = ADDR1 - (B >> 24);
	B &= ~0xff000000;
	ADDR3 = ADDR1 - B;
	u32 uncompressEnd = ((u32)ADDR1_END) - ((u32)aMainMemory);

	while (!(ADDR2 <= ADDR3)) {
		u32 marku8 = *(--ADDR2 + aMemOffset);
		//ADDR2-=1;
		int count = 8;
		while (true) {
			count--;
			if (count < 0)
				break;
			if (0 == (marku8 & 0x80)) {
				*(--ADDR1_END + aMemOffset) = *(--ADDR2 + aMemOffset);
			} else {
				int u8_r12 = *(--ADDR2 + aMemOffset);
				int u8_r7 = *(--ADDR2 + aMemOffset);
				u8_r7 |= (u8_r12 << 8);
				u8_r7 &= ~0xf000;
				u8_r7 += 2;
				u8_r12 += 0x20;
				do
				{
					u8 realu8 = *(ADDR1_END + aMemOffset + u8_r7);
					*(--ADDR1_END + aMemOffset) = realu8;
					u8_r12 -= 0x10;
				} while (u8_r12 >= 0);
			}
			marku8 <<= 1;
			if (ADDR2 <= ADDR3) {
				break;
			}
		}
	}
	return uncompressEnd;
}

void ensureBinaryDecompressed(const tNDSHeader* ndsHeader, module_params_t* moduleParams) {
	//const char* romTid = getRomTid(ndsHeader);

	if (
		moduleParams->compressed_static_end
		/*|| strcmp(romTid, "YQUJ") == 0 // Chrono Trigger (Japan)
		|| strcmp(romTid, "YQUE") == 0 // Chrono Trigger (USA)
		|| strcmp(romTid, "YQUP") == 0 // Chrono Trigger (Europe)*/
	) {
		// Compressed
		//dbg_printf("This rom is compressed\n");
		//decompressLZ77Backwards((u8*)ndsHeader->arm9destination, ndsHeader->arm9binarySize);
		decompressBinary((u8*)ndsHeader->arm9destination, ndsHeader->arm9binarySize, 0);
		moduleParams->compressed_static_end = 0;
	}/* else {
		// Not compressed
		dbg_printf("This rom is not compressed\n");
	}*/
}
