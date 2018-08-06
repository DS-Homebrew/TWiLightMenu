/******************************************************************************
 *******************************************************************************
	A simple font class for Easy GL2D DS created by:

	Relminator (Richard Eric M. Lope BSN RN)
	Http://Rel.Phatcode.Net

 *******************************************************************************
 ******************************************************************************/

#include <nds.h>
#include <nds/arm9/decompress.h>

#include <stdio.h>
#include <gl2d.h>
#include "FontGraphic.h"
#include "LargeFont.h"
#include "unicode_font_lut.h"
#include <nds/debug.h>

#include "large_font_0.h"
#include "large_font_1.h"

#include "uvcoord_large_font.h"

glImage largeFontImagesAscii[LARGE_FONT_NUM_IMAGES];
glImage largeFontImages_1[LARGE_FONT_NUM_IMAGES];
// glImage largeFontImages_2[LARGE_FONT_NUM_IMAGES];
// glImage largeFontImages_3[LARGE_FONT_NUM_IMAGES];
int LargeFont::initFont()
{

    // Initialize the font sprite banks.
    fontSpritesheetBanks[0] = largeFontImagesAscii;
    fontSpritesheetBanks[1] = largeFontImages_1;
    // fontSpritesheetBanks[2] = largeFontImages_2;
    // fontSpritesheetBanks[3] = largeFontImages_3;

    // Indicate none of the textures have been initialized..
    // -1 if uninitialized, otherwise the index of the font bank
    // the sprite is loaded in.
    for (int i = 0; i < LARGE_FONT_NUM_TEXTURES; i++)
    {
        fontSpritesheetStatus[i] = -1;
    }

    // Indicate none of the font banks are loaded.
    // -1 if unloaded, otherwise the index of the font sprite.
    for (int i = 0; i < LARGE_FONT_NUM_BANKS; i++)
    {
        spriteBankStatus[i] = -1;
    }

    texBitmaps[0] = large_font_0Bitmap;
    texBitmaps[1] = large_font_1Bitmap;

    // todo: see if we can share a single palette
    texPalettes[0] = large_font_0Pal;
    texPalettes[1] = large_font_1Pal;

   // nextFontBank = 2; // font bank 0 will always be the first texture

    // Initialize font sprite 0 into font bank 0.
     // Initialize font sprite 2 into font bank 1. (test occupied)
    refreshFontBanks();
    return 0;
}

void LargeFont::refreshFontBanks() {
    vramSetBankA(VRAM_A_TEXTURE);
    vramSetBankD(VRAM_D_TEXTURE);
    glDeleteTextures(2, fontSpritesheetBankId);
    initFontBank(0, 0);
    initFontBank(1, 1);
}

int LargeFont::initFontBank(int index, int into)
{
    glImage *sprite = fontSpritesheetBanks[into];
    int textureID = glLoadSpriteSet(
        sprite,
        LARGE_FONT_NUM_IMAGES,
        large_font_texcoords[index],
        GL_RGB16,
        TEXTURE_SIZE_512,
        TEXTURE_SIZE_256,
        TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT,
        16,
        (u16 *)texPalettes[index],
        (const u8 *)texBitmaps[index]);
    
    spriteBankStatus[into] = index;
    fontSpritesheetBankId[into] = textureID;
    fontSpritesheetStatus[index] = into;
    return textureID;
}

// Algorithm to get correct font sprite index
// floor(spriteIndex/characters per texture)

int LargeFont::getFontBankIndex(unsigned short int spriteIndex)
{
    int fontSheetIdx;
    char msgBuffer[256];
    fontSheetIdx = spriteIndex / LARGE_FONT_NUM_IMAGES; // truncated division works fine here.
    sprintf(msgBuffer, "Trying to load font sheet: %i", fontSheetIdx);
    nocashMessage(msgBuffer);
    sprintf(msgBuffer, "Size of font sheet: %i", sizeof(fontSpritesheetBanks[fontSheetIdx]));
    nocashMessage(msgBuffer);
    return fontSpritesheetStatus[fontSheetIdx];
}

void LargeFont::print(int x, int y, const char *text)
{
    unsigned short int fontChar;
    unsigned int fontBankIndex;
    unsigned char lowBits;
    unsigned char highBits;
    char msgBuffer[256];
    while (*text)
    {
        lowBits = *(unsigned char *)text++;
        if (lowBits != UTF16_SIGNAL_BYTE)
        { // check if the lower bits is the signal bits.
            fontChar = getSpriteIndex(lowBits);
        }
        else
        {
            lowBits = *(unsigned char *)text++;  // LSB
            highBits = *(unsigned char *)text++; // HSB
            u16 assembled = (u16)(lowBits | highBits << 8);
            fontChar = getSpriteIndex(assembled);
        }

        fontBankIndex = getFontBankIndex(fontChar);

        sprintf(msgBuffer, "FontBank: %i", fontBankIndex);
        nocashMessage(msgBuffer);
        sprintf(msgBuffer, "Character: %i", fontChar);
        nocashMessage(msgBuffer);

        fontChar = fontChar - (LARGE_FONT_NUM_IMAGES * fontBankIndex);
        
        sprintf(msgBuffer, "Adjusted: %i", fontChar);
        nocashMessage(msgBuffer);

        sprintf(msgBuffer, "Width: %i", fontSpritesheetBanks[fontBankIndex][fontChar].width);
        nocashMessage(msgBuffer);
       
        glSprite(x, y, GL_FLIP_NONE, &fontSpritesheetBanks[fontBankIndex][fontChar]);
        x += fontSpritesheetBanks[fontBankIndex][fontChar].width;
    }
}

int LargeFont::calcWidth(const char *text)
{
    unsigned short int fontChar;
    unsigned int fontBankIndex;
    unsigned char lowBits;
    unsigned char highBits;
    int x = 0;

    while (*text)
    {
        lowBits = *(unsigned char *)text++;
        if (lowBits != UTF16_SIGNAL_BYTE)
        {
            fontChar = getSpriteIndex(lowBits);
        }
        else
        {
            lowBits = *(unsigned char *)text++;
            highBits = *(unsigned char *)text++;
            fontChar = getSpriteIndex((u16)(lowBits | highBits << 8));
        }

        fontBankIndex = getFontBankIndex(fontChar);
        fontChar = fontChar - (LARGE_FONT_NUM_IMAGES * fontBankIndex);
        x += fontSpritesheetBanks[fontBankIndex][fontChar].width;
    }
    return x;
}

int LargeFont::getCenteredX(const char *text)
{
    unsigned short int fontChar;
    unsigned char lowBits;
    unsigned char highBits;
    unsigned int fontBankIndex;
    

    int total_width = 0;
    while (*text)
    {
        lowBits = *(unsigned char *)text++;
        if (lowBits != UTF16_SIGNAL_BYTE)
        {
            fontChar = getSpriteIndex(lowBits);
        }
        else
        {
            lowBits = *(unsigned char *)text++;
            highBits = *(unsigned char *)text++;
            fontChar = getSpriteIndex((u16)(lowBits | highBits << 8));
        }
        
    
        fontBankIndex = getFontBankIndex(fontChar);
        fontChar = fontChar - (LARGE_FONT_NUM_IMAGES * fontBankIndex);
        total_width += fontSpritesheetBanks[fontBankIndex][fontChar].width;
    }
    return (SCREEN_WIDTH - total_width) / 2;
}

void LargeFont::printCentered(int y, const char *text)
{
    unsigned short int fontChar;
    unsigned char lowBits;
    unsigned char highBits;
    unsigned int fontBankIndex;
    int x = getCenteredX(text);
    while (*text)
    {
        lowBits = *(unsigned char *)text++;
        if (lowBits != UTF16_SIGNAL_BYTE)
        {
            fontChar = getSpriteIndex(lowBits);
        }
        else
        {
            lowBits = *(unsigned char *)text++;
            highBits = *(unsigned char *)text++;
            fontChar = getSpriteIndex((u16)(lowBits | highBits << 8));
        }

        fontBankIndex = getFontBankIndex(fontChar);
        fontChar = fontChar - (LARGE_FONT_NUM_IMAGES * fontBankIndex);
        glSprite(x, y, GL_FLIP_NONE, &fontSpritesheetBanks[fontBankIndex][fontChar]);
        x += fontSpritesheetBanks[fontBankIndex][fontChar].width;
    }
}
