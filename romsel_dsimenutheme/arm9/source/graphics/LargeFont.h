/******************************************************************************
 *******************************************************************************
	A simple font class for Easy GL2D DS created by:

	Relminator (Richard Eric M. Lope BSN RN)
	Http://Rel.Phatcode.Net

 *******************************************************************************
 ******************************************************************************/
#pragma once
#include <gl2d.h>
#include <array>
#include "FontGraphic.h"
#include "uvcoord_large_font.h"

#define LARGE_FONT_NUM_BANKS  4

class LargeFont : public FontGraphic
{
private:
	std::array<glImage*, LARGE_FONT_NUM_BANKS> fontSpriteBanks;
    int fontSpriteBankId[LARGE_FONT_NUM_BANKS]; 
	const unsigned int* texBitmaps[LARGE_FONT_NUM_TEXTURES];
    const short unsigned int* texPalettes[LARGE_FONT_NUM_TEXTURES];

	int loadedSprites[LARGE_FONT_NUM_TEXTURES];
    int spriteBankStatus[LARGE_FONT_NUM_BANKS];

    char nextFontBank;
    int initFontBank(int index, int into);
   // int freeFontBank(int index);

public:
	LargeFont() { };
    int initFont();

	// todo: inline this virtual once both fonts are ported to dynamic paging.
	void print(int x, int y, const char *text) override;
	int calcWidth(const char *text) override;
	int getCenteredX(const char *text) override;
	void printCentered(int y, const char *text) override;
};