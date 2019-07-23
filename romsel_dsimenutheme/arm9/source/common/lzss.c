#include <nds.h>
#include <string.h>
#include "common/lzss.h"
#include "common/tonccpy.h"

#define __itcm __attribute__((section(".itcm")))

void __itcm
LZ77_Decompress(const unsigned char *src, unsigned char *dst){

	unsigned char flags = 0;
	unsigned char mask = 0;
	unsigned int len;
	unsigned int disp;

	u32 size = *((u32*)src) >> 8;

	src += 4;

	while (size > 0) {
		if (mask == 0) {
			// read in the flags data
			// from bit 7 to bit 0:
			//     0: raw byte
			//     1: compressed block
			flags = *src++;
			mask = 0x80;
		}

		if (flags & mask) { // compressed block
			// disp: displacement
			// len:  length
			len = (((*src) & 0xF0) >> 4) + 3;
			disp = ((*src++) & 0x0F);
			disp = disp << 8 | (*src++);

			size -= len;

			// for len, copy data from the displacement
			// to the current buffer position
			tonccpy(dst, dst - disp - 1, len);
			dst += len;
		} else { // uncompressed block
			// copy a raw byte from the input to the output
			*dst++ = *src++;
			size--;
		}

		mask >>= 1;
	}
}
