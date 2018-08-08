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

class LargeFont : public FontGraphic
{
  private:
    /**
     * Stores which font texture is loaded into which font
     * bank. -1 if not loaded.
     */
    int texBankMap[LARGE_FONT_NUM_AUX_TEX + 1];
    
    /**
     * Stores the texture IDs of loaded banks.
     */

    std::vector<int> textureIds;
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
    void clearFontBanks();
    // todo: inline this virtual once both fonts are ported to dynamic paging.
    void setFontBankState(char *text1, char *text2, char *text3);
    void print(int x, int y, const char *text) override;
    int calcWidth(const char *text) override;
    int getCenteredX(const char *text) override;
    void printCentered(int y, const char *text) override;
};