
#include <nds.h>
#include <nds/disc_io.h>
#include <malloc.h>
#include <stdio.h>
#include "sector0.h"

extern bool nand_Startup();

static u32 sector_buf32[SECTOR_SIZE/sizeof(u32)];
static u8 *sector_buf = (u8*)sector_buf32;

bool nandio_startup() {
	if (!nand_Startup()) return false;

	nand_ReadSectors(0, 1, sector_buf);
	bool is3DS = parse_ncsd(sector_buf) == 0;
	if (is3DS) return false;

	return true;
}
