
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
//#include "utils.h"
#include "sector0.h"

// return 0 for valid NCSD header
int parse_ncsd(const uint8_t sector0[SECTOR_SIZE]) {
	const ncsd_header_t * h = (ncsd_header_t *)sector0;
	if (h->magic != 0x4453434e) {
		//printf("NCSD magic not found\n");
		return -1;
	}

	for (unsigned i = 0; i < NCSD_PARTITIONS; ++i) {
		unsigned fs_type = h->fs_types[i];
		if (fs_type == 0) {
			break;
		}
		//const char *s_fs_type;
		switch (fs_type) {
			case 1:
				//s_fs_type = "Normal";
				break;
			case 3:
				//s_fs_type = "FIRM";
				break;
			case 4:
				//s_fs_type = "AGB_FIRM save";
				break;
			default:
				//iprintf("invalid partition type %d\n", fs_type);
				return -2;
		}
	}
	return 0;
}
