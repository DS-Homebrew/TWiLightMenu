// A list of favorite colors.

#ifndef USERCOLOR_H
#define USERCOLOR_H

#include "nds/ndstypes.h"

extern bool useTwlCfg;

const u16 userColors[16] = {
    0xce0c,
    0x8137,
    0x8c1f,
    0xfe3f,
    0x825f,
    0x839e,
    0x83f5,
    0x83e0,
    0x9e80,
    0xc769,
    0xfae6,
    0xf960,
    0xc800,
    0xe811,
    0xf41a,
    0xc81f
};

static inline int getFavoriteColor(void) {
	int favoriteColor = (int)(useTwlCfg ? *(u8*)0x02000444 : PersonalData->theme);
	if (favoriteColor < 0 || favoriteColor >= 16) favoriteColor = 0; // Invalid color found, so default to gray
	return favoriteColor;
}

const u16 userColorsTopBar[16] = {
    0xffff,
    0xeb9f,
    0xeb3f,
    0xffbf,
    0xf3df,
    0xefff,
    0xffff,
    0xffff,
    0xffff,
    0xfbfd,
    0xffff,
    0xffbb,
    0xffbd,
    0xff7e,
    0xffff,
    0xfb9f
};

#endif