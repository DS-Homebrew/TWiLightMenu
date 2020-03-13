#include "paletteEffects.h"
#include "string.h"
#include "tool/colortool.h"
#include "common/systemdetails.h"
#include "common/tonccpy.h"

extern bool useTwlCfg;

void effectDSiArrowButtonPalettes(u16* palette, u8 paletteLength)
{
    int offset = ((useTwlCfg ? *(u8*)0x02000444 : PersonalData->theme) * 16);
    tonccpy(palette, (sys().isDSPhat() ? phat_arrowButtonPalettes : arrowButtonPalettes) + offset, paletteLength * sizeof(u16));
}

void effectDSiStartBorderPalettes(u16* palette, u8 paletteLength)
{
    int offset = ((useTwlCfg ? *(u8*)0x02000444 : PersonalData->theme) * 16);
    tonccpy(palette, (sys().isDSPhat() ? phat_startBorderPalettes : startBorderPalettes) + offset, paletteLength * sizeof(u16));
}

void effectDSiStartTextPalettes(u16* palette, u8 paletteLength)
{
    int offset = ((useTwlCfg ? *(u8*)0x02000444 : PersonalData->theme) * 16);
    tonccpy(palette, startTextPalettes + offset, paletteLength * sizeof(u16));
}

void effectGrayscalePalette(u16* palette, u8 paletteLength)
{
    for (int i = 0; i < paletteLength; i++) {
  		*(palette+i) = convertVramColorToGrayscale(*(palette+i));
  	}
}


