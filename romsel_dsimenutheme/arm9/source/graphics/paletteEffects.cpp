#include "paletteEffects.h"
#include "string.h"
// #include "common/ColorLut.h"
#include "tool/colortool.h"
#include "common/systemdetails.h"
#include "common/tonccpy.h"

extern bool useTwlCfg;
extern u16* colorTable;

int getFavoriteColor(void) {
	int favoriteColor = (int)(useTwlCfg ? *(u8*)0x02000444 : PersonalData->theme);
	if (favoriteColor < 0 || favoriteColor >= 16) favoriteColor = 0; // Invalid color found, so default to gray
	return favoriteColor;
}

void effectDSiArrowButtonPalettes(u16* palette, u8 paletteLength)
{
	int offset = (getFavoriteColor() * 16);
	tonccpy(palette, (sys().isDSPhat() ? phat_arrowButtonPalettes : arrowButtonPalettes) + offset, paletteLength * sizeof(u16));
	effectColorModePalette(palette, paletteLength);
}

void effectDSiStartBorderPalettes(u16* palette, u8 paletteLength)
{
	int offset = (getFavoriteColor() * 16);
	tonccpy(palette, (sys().isDSPhat() ? phat_startBorderPalettes : startBorderPalettes) + offset, paletteLength * sizeof(u16));
	effectColorModePalette(palette, paletteLength);
}

void effectDSiStartTextPalettes(u16* palette, u8 paletteLength)
{
	int offset = (getFavoriteColor() * 16);
	tonccpy(palette, startTextPalettes + offset, paletteLength * sizeof(u16));
	effectColorModePalette(palette, paletteLength);
}

void effectColorModePalette(u16* palette, u16 paletteLength)
{
	if (!colorTable) {
		return;
	}

	for (int i = 0; i < paletteLength; i++) {
  		*(palette+i) = colorTable[*(palette+i) % 0x8000];
  	}
}

void effectColorModeBmpPalette(u16* palette, u16 paletteLength)
{
	if (!colorTable) {
		return;
	}

	for (int i = 0; i < paletteLength; i++) {
  		*(palette+i) = colorTable[*(palette+i) % 0x8000] | BIT(15);
  	}
}

