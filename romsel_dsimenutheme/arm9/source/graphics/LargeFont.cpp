/******************************************************************************
 *******************************************************************************
	A simple font class for Easy GL2D DS created by:

	Relminator (Richard Eric M. Lope BSN RN)
	Http://Rel.Phatcode.Net

 *******************************************************************************
 ******************************************************************************/

#include <nds.h>
#include <nds/arm9/decompress.h>

#include <vector>
#include <array>
#include <stdio.h>
#include <gl2d.h>
#include "FontGraphic.h"
#include "LargeFont.h"
#include "unicode_font_lut.h"
#include <nds/debug.h>

#include "large_font.h"

#include "uvcoord_large_font.h"

glImage largeFontPrmImage[LARGE_FONT_PRM_NUM_IMAGES];
std::vector<std::array<glImage, LARGE_FONT_AUX_NUM_IMAGES>> largeFontAuxImages;
std::vector<int> textureIds;


int nextAuxFontBank = 1;
int LargeFont::initFont()
{
    consoleDemoInit();
    for (int i = 0; i < LARGE_FONT_NUM_AUX_TEX; i++)
    {
         texBankMap[i] = -1;
    }
    refreshFontBanks();
    return 0;
}

void LargeFont::refreshFontBanks()
{
    vramSetBankA(VRAM_A_TEXTURE);
    vramSetBankD(VRAM_D_TEXTURE);
    clearFontBanks(false);
    initPrimaryFontBank();
}

void LargeFont::clearFontBanks(bool clearState)
{
    // nextAuxFontBank = 0;
    // if (clearState)
    // {
    //     for (int i = 0; i < LARGE_FONT_NUM_AUX_BANKS; i++)
    //     {
    //         auxFontBankState[i] = -1;
    //     }

    //     for (int i = 0; i < LARGE_FONT_NUM_AUX_TEX; i++)
    //     {
    //         auxFontTexBankMap[i] = -1;
    //     }
    // }
    // // Deletes all the textures.
    glDeleteTextures(textureIds.size(), textureIds.data());
    textureIds.clear();
    for (int i = 0; i < LARGE_FONT_NUM_AUX_TEX + 1; i++)
    {
         texBankMap[i] = -1;
    }
    texBankMap[0] = 0;
}

int LargeFont::initPrimaryFontBank()
{
    glImage *sprite = largeFontPrmImage;
    int textureID = glLoadSpriteSet(
        sprite,
        LARGE_FONT_PRM_NUM_IMAGES,
        large_font_texcoords[0],
        GL_RGB16,
        TEXTURE_SIZE_512,
        TEXTURE_SIZE_128,
        TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT,
        16,
        (u16 *)large_fontPal[0],
        (const u8 *)large_fontBitmaps[0]);
    textureIds.push_back(textureID);
    return textureID;
}

int LargeFont::initAuxillaryFontBank(int fontTextureIndex)
{
    std::array<glImage, LARGE_FONT_AUX_NUM_IMAGES> _sprite;
    int auxIndex = largeFontAuxImages.size();
    largeFontAuxImages.push_back(std::move(_sprite));
    glImage *sprite = (glImage*) largeFontAuxImages[auxIndex].data();
    int textureID = glLoadSpriteSet(
        sprite,
        LARGE_FONT_PRM_NUM_IMAGES,
        large_font_texcoords[fontTextureIndex],
        GL_RGB16,
        TEXTURE_SIZE_128,
        TEXTURE_SIZE_64,
        TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT,
        16,
        (u16 *)large_fontPal[fontTextureIndex],
        (const u8 *)large_fontBitmaps[fontTextureIndex]);
    textureIds.push_back(textureID);
    texBankMap[fontTextureIndex] = auxIndex;

    //fontBankTexID[fontBankIndex] = textureID;
   // auxFontBankState[fontBankIndex] = fontTextureIndex;
    //auxFontTexBankMap[fontTextureIndex - 1] = fontBankIndex;
    return textureID;
}

int LargeFont::getFontTextureIndex(unsigned short int spriteIndex)
{
    // Anything falling within extended ASCII range (192 characters)
    // will be in the primary sheet.
    if (spriteIndex < LARGE_FONT_PRM_NUM_IMAGES)
    {
        return 0;
    }

    // Anything above FULLWIDTH_BEGIN_INDEX is a full width character
    // large texes 23-29
    if (spriteIndex > FULLWIDTH_BEGIN_INDEX) {
        int fontTexIndex;
        spriteIndex = spriteIndex - FULLWIDTH_BEGIN_INDEX;
        fontTexIndex = spriteIndex / LARGE_FONT_AUX_NUM_IMAGES; // truncated division works fine here.
        return fontTexIndex + LARGE_FONT_FULLWIDTH_BEGIN;
    }

    // Calculated the adjusted font sheet by subtracting the
    // sprite index by 192 and dividing by the number of images
    // in each auxillary to get a 0-based index.
    // Since auxillary textures are indexed at 1, add one.
    int fontTexIndex;
    spriteIndex = spriteIndex - LARGE_FONT_PRM_NUM_IMAGES;
    fontTexIndex = spriteIndex / LARGE_FONT_AUX_NUM_IMAGES; // truncated division works fine here.
    return fontTexIndex + 1;
}

int LargeFont::adjustFontChar(unsigned short int fontChar, int fontTexIndex)
{
    if (fontTexIndex == 0)
    {
        return fontChar;
    }
    else if (fontTexIndex >= LARGE_FONT_FULLWIDTH_BEGIN) 
    {
        // Subtract FULLWIDTH_BEGIN_INDEX characters.
        fontChar = fontChar - FULLWIDTH_BEGIN_INDEX;
        // font char relative to the fullwidth block start.
        fontChar = fontChar - (LARGE_FONT_AUX_NUM_IMAGES * (fontTexIndex - LARGE_FONT_FULLWIDTH_BEGIN));
        return fontChar;
    }
    else
    {
        // It is in one of the auxillary font banks!

        // Subtract the primary image offset.
        fontChar = fontChar - LARGE_FONT_PRM_NUM_IMAGES;
        // Subtract the previous auxillary counts to get an index
        // relative to the font texture of this character.
        fontChar = fontChar - (LARGE_FONT_AUX_NUM_IMAGES * (fontTexIndex - 1));
        return fontChar;
    }
}

/**
 * Gets the glImage pointer to the given index
 * if it does not load, load it.
 */
glImage *LargeFont::getFontBankImage(int fontTexIndex)
{    
    if (fontTexIndex == 0) {
        //printf("PRIMARY FONT BANK\n");
        return largeFontPrmImage;
    }

    if (texBankMap[fontTexIndex] != -1) {
        int glImageIndex = texBankMap[fontTexIndex];   
        return largeFontAuxImages[glImageIndex].data();
    } else {
        printf("AUX FONT NEEDS LOAD\n");
        initAuxillaryFontBank(fontTexIndex);
        return getFontBankImage(fontTexIndex);
    }
    // if (auxFontTexBankMap[fontTexIndex - 1] != -1)
    // {
    //  //   printf("AUX FONT BANK LOADED\n");
    //     int glImageIndex = auxFontTexBankMap[fontTexIndex - 1];
    //     return largeFontAuxImageBanks[glImageIndex];
    // } else {
    //     
    // }
}

void LargeFont::print(int x, int y, const char *text)
{
    unsigned short int fontChar;
    unsigned int fontTexIndex;
    unsigned char lowBits;
    unsigned char highBits;
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

        fontTexIndex = getFontTextureIndex(fontChar);
        fontChar = adjustFontChar(fontChar, fontTexIndex);
        glImage *fontImage = getFontBankImage(fontTexIndex);
        glSprite(x, y, GL_FLIP_NONE, &fontImage[fontChar]);
        x += fontImage[fontChar].width;
    }
}

int LargeFont::calcWidth(const char *text)
{
    unsigned short int fontChar;
    unsigned int fontTexIndex;
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

        fontTexIndex = getFontTextureIndex(fontChar);
        fontChar = adjustFontChar(fontChar, fontTexIndex);
        glImage *fontImage = getFontBankImage(fontTexIndex);
        x += fontImage[fontChar].width;
    }
    return x;
}

int LargeFont::getCenteredX(const char *text)
{
    unsigned short int fontChar;
    unsigned char lowBits;
    unsigned char highBits;
    unsigned int fontTexIndex;

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

        fontTexIndex = getFontTextureIndex(fontChar);
        fontChar = adjustFontChar(fontChar, fontTexIndex);
        glImage *fontImage = getFontBankImage(fontTexIndex);
        total_width += fontImage[fontChar].width;
    }
    return (SCREEN_WIDTH - total_width) / 2;
}

void LargeFont::printCentered(int y, const char *text)
{
    unsigned short int fontChar;
    unsigned char lowBits;
    unsigned char highBits;
    unsigned int fontTexIndex;
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

        fontTexIndex = getFontTextureIndex(fontChar);
        fontChar = adjustFontChar(fontChar, fontTexIndex);
        glImage *fontImage = getFontBankImage(fontTexIndex);
        glSprite(x, y, GL_FLIP_NONE, &fontImage[fontChar]);
        x += fontImage[fontChar].width;
    }
}
