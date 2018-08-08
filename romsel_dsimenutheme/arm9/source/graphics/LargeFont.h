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


// There are 22 auxillary font textures 
// Auxillary font banks have indices starting at 1, since the primary font bank
// has index 0.
#define LARGE_FONT_NUM_AUX_BANKS 8

class LargeFont : public FontGraphic
{
  private:
   // glImage* fontSpritesheetBanks[2];
    // int fontSpritesheetBankId[LARGE_FONT_NUM_BANKS];
    // const unsigned int *texBitmaps[LARGE_FONT_NUM_TEXTURES];
    // const short unsigned int *texPalettes[LARGE_FONT_NUM_TEXTURES];

    // /**
    //  * Stores which font texture index is loaded into which
    //  * font bank.
    //  * 
    //  * The primary ASCII texture is always loaded into bank 0.
    //  */
    // int auxFontBankState[LARGE_FONT_NUM_AUX_BANKS];

    /**
     * Stores which font texture is loaded into which font
     * bank. -1 if not loaded.
     */
    int texBankMap[LARGE_FONT_NUM_AUX_TEX + 1];
    
    /**
     * Stores the texture IDs of loaded banks.
     */
   // int fontBankTexID[LARGE_FONT_NUM_AUX_BANKS + 1];

    glImage* getFontBankImage(int fontTexIndex);
    int adjustFontChar(unsigned short int fontChar, int fontTexIndex);
    int getFontTextureIndex(unsigned short int spriteIndex);
    int initAuxillaryFontBank(int fontTextureIndex);
    int initPrimaryFontBank();
    // int freeFontBank(int index);

  public:
    LargeFont(){};
    int initFont();
    void refreshFontBanks();
    void clearFontBanks(bool clearState);
    // todo: inline this virtual once both fonts are ported to dynamic paging.
    void setFontBankState(char *text1, char *text2, char *text3);
    void print(int x, int y, const char *text) override;
    int calcWidth(const char *text) override;
    int getCenteredX(const char *text) override;
    void printCentered(int y, const char *text) override;
};