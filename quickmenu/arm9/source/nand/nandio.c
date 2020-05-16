
#include <nds.h>
#include <nds/disc_io.h>
#include <malloc.h>
#include <stdio.h>
#include "crypto.h"
#include "sector0.h"
#include "common/tonccpy.h"
#include "f_xy.h"

//#define SECTOR_SIZE 512
#define CRYPT_BUF_LEN 64

static bool is3DS;

extern bool nand_Startup();

static u8* crypt_buf = 0;

static u32 fat_sig_fix_offset = 0;

static u32 sector_buf32[SECTOR_SIZE/sizeof(u32)];
static u8 *sector_buf = (u8*)sector_buf32;

void nandio_set_fat_sig_fix(u32 offset) {
	fat_sig_fix_offset = offset;
}

void getConsoleID(u8 *consoleID){
	*(u32*)(0x2FFFD0C) = 0x434F4944;
	while (*(u32*)(0x2FFFD0C) != 0);

	u8 *fifo=(u8*)0x02500000; //shared mem address that has our computed key3 stuff
	u8 key[16]; //key3 normalkey - keyslot 3 is used for DSi/twln NAND crypto
	u8 key_xy[16]; //key3_y ^ key3_x
	u8 key_x[16];////key3_x - contains a DSi console id (which just happens to be the LFCS on 3ds)
	u8 key_y[16] = {0x76, 0xDC, 0xB9, 0x0A, 0xD3, 0xC4, 0x4D, 0xBD, 0x1D, 0xDD, 0x2D, 0x20, 0x05, 0x00, 0xA0, 0xE1}; //key3_y NAND constant
	
	tonccpy(key, fifo, 16);  //receive the goods from arm7

	F_XY_reverse((uint32_t*)key, (uint32_t*)key_xy); //work backwards from the normalkey to get key_x that has the consoleID

	for(int i=0;i<16;i++){
		key_x[i] = key_xy[i] ^ key_y[i];             //''
	}

	tonccpy(&consoleID[0], &key_x[0], 4);             
	tonccpy(&consoleID[4], &key_x[0xC], 4);
}

bool nandio_startup() {
	if (!nand_Startup()) return false;

	nand_ReadSectors(0, 1, sector_buf);
	is3DS = parse_ncsd(sector_buf, 0) == 0;
	if (is3DS) return false;

	if (*(u32*)(0x2FFD7BC) == 0) {
		// Get eMMC CID
		*(u32*)(0x2FFFD0C) = 0x454D4D43;
		while (*(u32*)(0x2FFFD0C) != 0);
	}

	u8 consoleID[8];
	u8 consoleIDfixed[8];

	// Get ConsoleID
	getConsoleID(consoleID);
	for (int i = 0; i < 8; i++) {
		consoleIDfixed[i] = consoleID[7-i];
	}

	// iprintf("sector 0 is %s\n", is3DS ? "3DS" : "DSi");
	dsi_crypt_init((const u8*)consoleIDfixed, (const u8*)0x2FFD7BC, is3DS);
	dsi_nand_crypt(sector_buf, sector_buf, 0, SECTOR_SIZE / AES_BLOCK_SIZE);
	parse_mbr(sector_buf, is3DS, 0);

	mbr_t *mbr = (mbr_t*)sector_buf;
	nandio_set_fat_sig_fix(is3DS ? 0 : mbr->partitions[0].offset);

	if (crypt_buf == 0) {
		crypt_buf = (u8*)memalign(32, SECTOR_SIZE * CRYPT_BUF_LEN);
		//if (crypt_buf == 0) {
			//printf("nandio: failed to alloc buffer\n");
		//}
	}
	return crypt_buf != 0;
}

bool nandio_is_inserted() {
	return true;
}

// len is guaranteed <= CRYPT_BUF_LEN
static bool read_sectors(sec_t start, sec_t len, void *buffer) {
	if (nand_ReadSectors(start, len, crypt_buf)) {
		dsi_nand_crypt(buffer, crypt_buf, start * SECTOR_SIZE / AES_BLOCK_SIZE, len * SECTOR_SIZE / AES_BLOCK_SIZE);
		if (fat_sig_fix_offset &&
			start == fat_sig_fix_offset
			&& ((u8*)buffer)[0x36] == 0
			&& ((u8*)buffer)[0x37] == 0
			&& ((u8*)buffer)[0x38] == 0)
		{
			((u8*)buffer)[0x36] = 'F';
			((u8*)buffer)[0x37] = 'A';
			((u8*)buffer)[0x38] = 'T';
		}
		return true;
	} else {
		//printf("NANDIO: read error\n");
		return false;
	}
}

bool nandio_read_sectors(sec_t offset, sec_t len, void *buffer) {
	// iprintf("R: %u(0x%08x), %u\n", (unsigned)offset, (unsigned)offset, (unsigned)len);
	while (len >= CRYPT_BUF_LEN) {
		if (!read_sectors(offset, CRYPT_BUF_LEN, buffer)) {
			return false;
		}
		offset += CRYPT_BUF_LEN;
		len -= CRYPT_BUF_LEN;
		buffer = ((u8*)buffer) + SECTOR_SIZE * CRYPT_BUF_LEN;
	}
	if (len > 0) {
		return read_sectors(offset, len, buffer);
	} else {
		return true;
	}
}

bool nandio_write_sectors(sec_t offset, sec_t len, const void *buffer) {
	// lol, nope
	return false;
}

bool nandio_clear_status() {
	return true;
}

bool nandio_shutdown() {
	free(crypt_buf);
	crypt_buf = 0;
	return true;
}

const DISC_INTERFACE io_dsi_nand = {
	('N' << 24) | ('A' << 16) | ('N' << 8) | 'D',
	FEATURE_MEDIUM_CANREAD,
	nandio_startup,
	nandio_is_inserted,
	nandio_read_sectors,
	nandio_write_sectors,
	nandio_clear_status,
	nandio_shutdown
};
