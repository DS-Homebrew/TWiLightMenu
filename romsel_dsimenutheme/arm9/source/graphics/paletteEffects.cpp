#include "paletteEffects.h"
#include "string.h"
#include "common/ColorLut.h"
#include "tool/colortool.h"
#include "common/systemdetails.h"
#include "common/tonccpy.h"
#include "common/twlmenusettings.h"

extern bool useTwlCfg;

int getFavoriteColor(void) {
	int favoriteColor = (int)(useTwlCfg ? *(u8*)0x02000444 : PersonalData->theme);
	if (favoriteColor < 0 || favoriteColor >= 16) favoriteColor = 0; // Invalid color found, so default to gray
	return favoriteColor;
}

void effectDSiArrowButtonPalettes(u16* palette, u8 paletteLength)
{
	int offset = (getFavoriteColor() * 16);
	tonccpy(palette, ((ms().colorMode != 1 && (sys().isDSPhat() || ms().colorMode == 2)) ? phat_arrowButtonPalettes : arrowButtonPalettes) + offset, paletteLength * sizeof(u16));
	effectColorModePalette(palette, paletteLength);
}

void effectDSiStartBorderPalettes(u16* palette, u8 paletteLength)
{
	int offset = (getFavoriteColor() * 16);
	tonccpy(palette, ((ms().colorMode != 1 && (sys().isDSPhat() || ms().colorMode == 2)) ? phat_startBorderPalettes : startBorderPalettes) + offset, paletteLength * sizeof(u16));
	effectColorModePalette(palette, paletteLength);
}

void effectDSiStartTextPalettes(u16* palette, u8 paletteLength)
{
	int offset = (getFavoriteColor() * 16);
	tonccpy(palette, startTextPalettes + offset, paletteLength * sizeof(u16));
	effectColorModePalette(palette, paletteLength);
}

void effectGrayscalePalette(u16* palette, u16 paletteLength)
{
	for (int i = 0; i < paletteLength; i++) {
  		*(palette+i) = convertVramColorToGrayscale(*(palette+i));
  	}
}

void effectPhatPalette(u16* palette, u16 paletteLength)
{
	for (int i = 0; i < paletteLength; i++) {
  		*(palette+i) = convertDSColorToPhat(*(palette+i));
  	}
}

void effectColorModePalette(u16* palette, u16 paletteLength)
{
	if (ms().colorMode == 0) {
		return;
	}

	if (ms().colorMode == 2) {
		effectPhatPalette(palette, paletteLength);
	} else if (ms().colorMode == 1) {
		effectGrayscalePalette(palette, paletteLength);
	}
}


