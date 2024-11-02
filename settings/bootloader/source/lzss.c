#include <nds/ndstypes.h>
#include <string.h>

void LZ77_Decompress(u8* source, u8* destination) {
	u32 leng = (source[1] | (source[2] << 8) | (source[3] << 16));
	int Offs = 4;
	int dstoffs = 0;
	while (true) {
		u8 header = source[Offs++];
		for (int i = 0; i < 8; i++) {
			if ((header & 0x80) == 0) destination[dstoffs++] = source[Offs++];
			else
			{
				u8 a = source[Offs++];
				u8 b = source[Offs++];
				int offs = (((a & 0xF) << 8) | b) + 1;
				int length = (a >> 4) + 3;
				for (int j = 0; j < length; j++) {
					destination[dstoffs] = destination[dstoffs - offs];
					dstoffs++;
				}
			}
			if (dstoffs >= (int)leng) return;
			header <<= 1;
		}
	}
}
