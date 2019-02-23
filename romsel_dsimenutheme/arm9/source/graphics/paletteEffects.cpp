#include "paletteEffects.h"
#include "string.h"
#include "tool/colortool.h"

void effectDSiArrowButtonPalettes(u16* palette, u8 paletteLength)
{
    int offset = ((PersonalData->theme) * 16);
    memcpy(palette, arrowButtonPalettes + offset, paletteLength * sizeof(u16));
}

void effectDSiStartBorderPalettes(u16* palette, u8 paletteLength)
{
    int offset = ((PersonalData->theme) * 16);
    memcpy(palette, startBorderPalettes + offset, paletteLength * sizeof(u16));
}

void effectDSiStartTextPalettes(u16* palette, u8 paletteLength)
{
    int offset = ((PersonalData->theme) * 16);
    memcpy(palette, startTextPalettes + offset, paletteLength * sizeof(u16));
}

void effectGrayscalePalette(u16* palette, u8 paletteLength)
{
    for (int i = 0; i < paletteLength; i++) {
  		*(palette+i) = convertVramColorToGrayscale(*(palette+i));
  	}
}


