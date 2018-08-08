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
#include "SmallFont.h"
#include "unicode_font_lut.h"
#include <nds/debug.h>

#include "small_font.h"

#include "uvcoord_small_font.h"

glImage smallFontPrmImage[SMALL_FONT_PRM_NUM_IMAGES];
std::vector<std::array<glImage, SMALL_FONT_AUX_NUM_IMAGES>> smallFontAuxImages;

int SmallFont::initFont()
{
  //  consoleDemoInit();
    refreshFontBanks();
    return 0;
}

void SmallFont::refreshFontBanks()
{
    vramSetBankA(VRAM_A_TEXTURE);
    vramSetBankD(VRAM_D_TEXTURE);
    clearFontBanks();
    initPrimaryFontBank();
}

void SmallFont::clearFontBanks()
{
    glDeleteTextures(textureIds.size(), textureIds.data());
    textureIds.clear();
    for (int i = 0; i < SMALL_FONT_NUM_AUX_TEX + 1; i++)
    {
         texBankMap[i] = -1;
    }
    texBankMap[0] = 0;
}

int SmallFont::initPrimaryFontBank()
{
    glImage *sprite = smallFontPrmImage;
    int textureID = glLoadSpriteSet(
        sprite,
        SMALL_FONT_PRM_NUM_IMAGES,
        small_font_texcoords[0],
        GL_RGB16,
        TEXTURE_SIZE_256,
        TEXTURE_SIZE_128,
        TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT,
        16,
        (u16 *)small_fontPal[0],
        (const u8 *)small_fontBitmaps[0]);
    textureIds.push_back(textureID);
    return textureID;
}

int SmallFont::initAuxillaryFontBank(int fontTextureIndex)
{
    std::array<glImage, SMALL_FONT_AUX_NUM_IMAGES> _sprite;
    int auxIndex = smallFontAuxImages.size();
    smallFontAuxImages.push_back(std::move(_sprite));
    glImage *sprite = (glImage*) smallFontAuxImages[auxIndex].data();
    int textureID = glLoadSpriteSet(
        sprite,
        SMALL_FONT_PRM_NUM_IMAGES,
        small_font_texcoords[fontTextureIndex],
        GL_RGB16,
        TEXTURE_SIZE_128,
        TEXTURE_SIZE_64,
        TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT,
        16,
        (u16 *)small_fontPal[fontTextureIndex],
        (const u8 *)small_fontBitmaps[fontTextureIndex]);
    textureIds.push_back(textureID);
    texBankMap[fontTextureIndex] = auxIndex;
    return textureID;
}

int SmallFont::getFontTextureIndex(unsigned short int spriteIndex)
{
    // Anything falling within extended ASCII range (192 characters)
    // will be in the primary sheet.
    if (spriteIndex < SMALL_FONT_PRM_NUM_IMAGES)
    {
        return 0;
    }

    // Anything above FULLWIDTH_BEGIN_INDEX is a full width character
    // large texes 23-29
    if (spriteIndex > FULLWIDTH_BEGIN_INDEX) {
        int fontTexIndex;
        spriteIndex = spriteIndex - FULLWIDTH_BEGIN_INDEX;
        fontTexIndex = spriteIndex / SMALL_FONT_AUX_NUM_IMAGES; // truncated division works fine here.
        return fontTexIndex + SMALL_FONT_FULLWIDTH_BEGIN;
    }

    // Calculated the adjusted font sheet by subtracting the
    // sprite index by 192 and dividing by the number of images
    // in each auxillary to get a 0-based index.
    // Since auxillary textures are indexed at 1, add one.
    int fontTexIndex;
    spriteIndex = spriteIndex - SMALL_FONT_PRM_NUM_IMAGES;
    fontTexIndex = spriteIndex / SMALL_FONT_AUX_NUM_IMAGES; // truncated division works fine here.
    return fontTexIndex + 1;
}

int SmallFont::adjustFontChar(unsigned short int fontChar, int fontTexIndex)
{
    if (fontTexIndex == 0)
    {
        return fontChar;
    }
    else if (fontTexIndex >= SMALL_FONT_FULLWIDTH_BEGIN) 
    {
        // Subtract FULLWIDTH_BEGIN_INDEX characters.
        fontChar = fontChar - FULLWIDTH_BEGIN_INDEX;
        // font char relative to the fullwidth block start.
        fontChar = fontChar - (SMALL_FONT_AUX_NUM_IMAGES * (fontTexIndex - SMALL_FONT_FULLWIDTH_BEGIN));
        return fontChar;
    }
    else
    {
        // It is in one of the auxillary font banks!

        // Subtract the primary image offset.
        fontChar = fontChar - SMALL_FONT_PRM_NUM_IMAGES;
        // Subtract the previous auxillary counts to get an index
        // relative to the font texture of this character.
        fontChar = fontChar - (SMALL_FONT_AUX_NUM_IMAGES * (fontTexIndex - 1));
        return fontChar;
    }
}

/**
 * Gets the glImage pointer to the given index
 * if it does not load, load it.
 */
glImage *SmallFont::getFontBankImage(int fontTexIndex)
{    
    if (fontTexIndex == 0) {
        //printf("PRIMARY FONT BANK\n");
        return smallFontPrmImage;
    }

    if (texBankMap[fontTexIndex] != -1) {
        int glImageIndex = texBankMap[fontTexIndex];   
        return smallFontAuxImages[glImageIndex].data();
    } else {
    //    printf("AUX FONT NEEDS LOAD\n");
        initAuxillaryFontBank(fontTexIndex);
        return getFontBankImage(fontTexIndex);
    }
    // if (auxFontTexBankMap[fontTexIndex - 1] != -1)
    // {
    //  //   printf("AUX FONT BANK LOADED\n");
    //     int glImageIndex = auxFontTexBankMap[fontTexIndex - 1];
    //     return SmallFontAuxImageBanks[glImageIndex];
    // } else {
    //     
    // }
}

void SmallFont::print(int x, int y, const char *text)
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

int SmallFont::calcWidth(const char *text)
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

int SmallFont::getCenteredX(const char *text)
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

void SmallFont::printCentered(int y, const char *text)
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
