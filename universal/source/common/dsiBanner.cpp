#include "common/dsiBanner.h"
#include "common/tonccpy.h"

#include <nds/bios.h>
#include <stdio.h>
#include <stdlib.h>

bool dsiBannerSeqStep(const u16 *seq, int *pos, u16 *delay, DsiBannerFrame *out) {
	if (*pos < 0 || *pos >= DSI_BANNER_SEQ_LEN)
		*pos = 0;

	const u16 token = seq[*pos];
	if (DSI_SEQ_FRAMES(token) == 0) {
		// The current entry is a terminator, so there is nothing to animate.
		out->cell = 0;
		out->pltt = 0;
		out->flipH = false;
		out->flipV = false;
		*pos = 0;
		*delay = 0;
		return false;
	}

	out->cell = DSI_SEQ_CELL(token);
	out->pltt = DSI_SEQ_PLTT(token);
	out->flipH = DSI_SEQ_FLIPH(token);
	out->flipV = DSI_SEQ_FLIPV(token);

	if (++(*delay) >= DSI_SEQ_FRAMES(token)) {
		// This entry has been shown for its full duration.
		const int next = *pos + 1;
		// Running off the end of the table is an implicit loop terminator.
		const u16 nextToken = (next < DSI_BANNER_SEQ_LEN) ? seq[next] : 0;

		if (DSI_SEQ_FRAMES(nextToken) == 0) {
			if (DSI_SEQ_CELL(nextToken) == 1) {
				// Stop, holding this (the last) frame. Leaving the delay at the
				// frame count keeps it expired without needing extra state.
				*delay = DSI_SEQ_FRAMES(token);
			} else {
				*delay = 0;
				*pos = 0;
			}
		} else {
			*delay = 0;
			*pos = next;
		}
	}

	return true;
}

bool dsiSubBannerLoad(const char *path, void *animeOut, u16 *crcOut) {
	FILE *fp = fopen(path, "rb");
	if (!fp)
		return false;

	bool loaded = false;
	u16 header[5] = {0}; // version, crc16_v1, crc16_v2, crc16_v3, crc16_anime

	if (fread(header, sizeof(u16), 5, fp) == 5
	 && header[0] == DSI_SUBBANNER_VER
	 && header[4] != 0) {
		u8 *buf = (u8 *)malloc(DSI_BANNER_ANIME_SIZE);
		if (buf) {
			if (fseek(fp, DSI_SUBBANNER_ANIME_OFF, SEEK_SET) == 0
			 && fread(buf, 1, DSI_BANNER_ANIME_SIZE, fp) == DSI_BANNER_ANIME_SIZE
			 && swiCRC16(0xFFFF, buf, DSI_BANNER_ANIME_SIZE) == header[4]) {
				tonccpy(animeOut, buf, DSI_BANNER_ANIME_SIZE);
				if (crcOut)
					*crcOut = header[4];
				loaded = true;
			}
			free(buf);
		}
	}

	fclose(fp);
	return loaded;
}
