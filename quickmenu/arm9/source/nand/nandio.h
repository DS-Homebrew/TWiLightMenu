#pragma once

#include <nds.h>
#include <nds/disc_io.h>

void nandio_set_fat_sig_fix(u32 offset);

extern const DISC_INTERFACE io_dsi_nand;
