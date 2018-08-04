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
glImage largeFontImages_2[LARGE_FONT_NUM_IMAGES];
glImage largeFontImages_3[LARGE_FONT_NUM_IMAGES];

int LargeFont::initFont() {

    // Initialize the font sprite banks.
    fontSpriteBanks[0] = largeFontImagesAscii;
    fontSpriteBanks[1] = largeFontImages_1;
    fontSpriteBanks[2] = largeFontImages_2;
    fontSpriteBanks[3] = largeFontImages_3;

    // Indicate none of the textures have been initialized..
    // -1 if uninitialized, otherwise the index of the font bank
    // the sprite is loaded in.
    for(int i = 0; i < LARGE_FONT_NUM_TEXTURES; i++) {
        loadedSprites[i] = -1;
    }

    // Indicate none of the font banks are loaded.
    // -1 if unloaded, otherwise the index of the font sprite.
    for(int i = 0; i < LARGE_FONT_NUM_BANKS; i++) {
        spriteBankStatus[i] = -1;
    }

    texBitmaps[0] = large_font_0Bitmap;
    texBitmaps[1] = large_font_1Bitmap;

    // todo: see if we can share a single palette
    texPalettes[0] = large_font_0Pal;
    texPalettes[1] = large_font_1Pal;

    nextFontBank = 1; // font bank 0 will always be the first texture 

    initFontBank(0, 0); // Initialize font sprite 0 into font bank 0.
    return 0;
}

int LargeFont::initFontBank(int index, int into) {
    // Do nothing if the sprite index is already loaded into the bank.
    if (loadedSprites[index] == into) return fontSpriteBankId[into]; 

    glImage* sprite = fontSpriteBanks[into];
    int textureID = glLoadSpriteSet(
            sprite,
			LARGE_FONT_NUM_IMAGES,
			large_font_texcoords[index],
			GL_RGB16,
			TEXTURE_SIZE_512,
			TEXTURE_SIZE_256,
			GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T | TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT,
			16,
			(u16*) texPalettes[index],
			(const u8*) texBitmaps[index]
	);
    int bankOccupiedIndex = spriteBankStatus[into];
    if (bankOccupiedIndex < 0) { // We are replacing a bank
        // todo: unload the old sprite with glDelete
        int previousTextureId = fontSpriteBankId[into];
        loadedSprites[bankOccupiedIndex] = -1;
    } 
    spriteBankStatus[into] = index;
    fontSpriteBankId[into] = textureID;
    loadedSprites[index] = into;
    // todo: port print
    return textureID;
}

void LargeFont::print(int x, int y, const char *text)
{
	unsigned short int fontChar;
	unsigned char lowBits;
	unsigned char highBits;
	while (*text)
	{
		lowBits = *(unsigned char*) text++;
		if (lowBits != UTF16_SIGNAL_BYTE) { // check if the lower bits is the signal bits.
			fontChar = getSpriteIndex(lowBits);
		} else {
			lowBits = *(unsigned char*) text++; // LSB
			highBits = *(unsigned char*) text++; // HSB
			u16 assembled = (u16)(lowBits | highBits << 8);
			
			fontChar = getSpriteIndex(assembled);
		}
		
		glSprite(x, y, GL_FLIP_NONE, &fontSpriteBanks[0][fontChar]);
		x += fontSpriteBanks[0][fontChar].width;
	}
}

int LargeFont::calcWidth(const char *text)
{
	unsigned short int fontChar;
	unsigned char lowBits;
	unsigned char highBits;
	int x = 0;

	while (*text)
	{
		lowBits = *(unsigned char*) text++;
		if (lowBits != UTF16_SIGNAL_BYTE) {
			fontChar = getSpriteIndex(lowBits);
		} else {
			lowBits = *(unsigned char*) text++;
			highBits = *(unsigned char*) text++;
			fontChar = getSpriteIndex((u16)(lowBits | highBits << 8));
		}

		x += fontSpriteBanks[0][fontChar].width;
	}
	return x;
}

int LargeFont::getCenteredX(const char *text)
{
	unsigned short int fontChar;
	unsigned char lowBits;
	unsigned char highBits;
	int total_width = 0;
	while (*text)
	{
		lowBits = *(unsigned char*) text++;
		if (lowBits != UTF16_SIGNAL_BYTE) {
			fontChar = getSpriteIndex(lowBits);
		} else {
			lowBits = *(unsigned char*) text++;
			highBits = *(unsigned char*) text++;
			fontChar = getSpriteIndex((u16)(lowBits | highBits << 8));
		}

		total_width += fontSpriteBanks[0][fontChar].width;
	}
	return (SCREEN_WIDTH - total_width) / 2;
}

void LargeFont::printCentered(int y, const char *text)
{
	unsigned short int fontChar;
	unsigned char lowBits;
	unsigned char highBits;	

	int x = getCenteredX(text);
	while (*text)
	{
		lowBits = *(unsigned char*) text++;
		if (lowBits != UTF16_SIGNAL_BYTE) {
			fontChar = getSpriteIndex(lowBits);
		} else {
			lowBits = *(unsigned char*) text++;
			highBits = *(unsigned char*) text++;
			fontChar = getSpriteIndex((u16)(lowBits | highBits << 8));
		}

		fontChar = getSpriteIndex(*(unsigned char*) text++);
		glSprite(x, y, GL_FLIP_NONE, &fontSpriteBanks[0][fontChar]);
		x += fontSpriteBanks[0][fontChar].width;
	}
}
