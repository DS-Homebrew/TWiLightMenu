#pragma once
#ifndef _COLOR_H_
#define _COLOR_H_

#include <nds/ndstypes.h>

u16 grayscale(u16 val);
u16 alphablend(const u16 fg, const u16 bg, const u8 alpha);
u16 themealphablend(const u16 fg, const u16 bg, const u8 alpha);
u16 blend(const u16 fg, const u16 bg);

#endif