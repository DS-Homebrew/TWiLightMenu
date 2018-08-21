/******************************************************************************
 *******************************************************************************
	A simple font class for Easy GL2D DS created by:

	Relminator (Richard Eric M. Lope BSN RN)
	Http://Rel.Phatcode.Net

 *******************************************************************************
 ******************************************************************************/

#include <nds.h>
#include <stdio.h>
#include <gl2d.h>
#include "FontGraphic.h"

int fontTextureID[2];


int FontGraphic::load(int textureID, glImage *_font_sprite,
				  const unsigned int numframes,
				  const unsigned int *texcoords,
				  GL_TEXTURE_TYPE_ENUM type,
				  int sizeX,
				  int sizeY,
				  int param,
				  int pallette_width,
				  const u16 *palette,
				  const uint8 *texture,
				  const unsigned short int *_mapping
				  )

{
	fontSprite = _font_sprite;
	imageCount = numframes;
	mapping = _mapping;
	if (fontTextureID[textureID]) glDeleteTextures(1, &fontTextureID[textureID]);
	fontTextureID[textureID] =
			glLoadSpriteSet(fontSprite,
							numframes,
							texcoords,
							type,
							sizeX,
							sizeY,
							param,
							pallette_width,
							palette,
							texture
							);

	return fontTextureID[textureID];

}

/**
 * Get the index in the UV coordinate array where the letter appears
 */
unsigned int FontGraphic::getSpriteIndex(const u16 letter) {
	unsigned int spriteIndex = 0;
	for (unsigned int i = 0; i < imageCount; i++) {
		if (mapping[i] == letter) {
			spriteIndex = i;
		}
	}
	return spriteIndex;
}

void FontGraphic::print(int x, int y, const char *text)
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
		
		glSprite(x, y, GL_FLIP_NONE, &fontSprite[fontChar]);
		x += fontSprite[fontChar].width;
	}
}

int FontGraphic::calcWidth(const char *text)
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

		x += fontSprite[fontChar].width;
	}
	return x;
}

void FontGraphic::print(int x, int y, int value)
{
	sprintf(buffer, "%i", value);
	print(x, y, buffer);
}

int FontGraphic::getCenteredX(const char *text)
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

		total_width += fontSprite[fontChar].width;
	}
	return (SCREEN_WIDTH - total_width) / 2;
}

void FontGraphic::printCentered(int y, const char *text)
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
		glSprite(x, y, GL_FLIP_NONE, &fontSprite[fontChar]);
		x += fontSprite[fontChar].width;
	}
}

void FontGraphic::printCentered(int y, int value)
{
	sprintf(buffer, "%i", value);
	printCentered(y, buffer);
}