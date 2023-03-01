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

std::u16string FontGraphic::utf8to16(std::string_view text) {
	std::u16string out;
	for (uint i=0;i<text.size();) {
		char16_t c = 0;
		if (!(text[i] & 0x80)) {
			c = text[i++];
		} else if ((text[i] & 0xE0) == 0xC0) {
			c  = (text[i++] & 0x1F) << 6;
			c |=  text[i++] & 0x3F;
		} else if ((text[i] & 0xF0) == 0xE0) {
			c  = (text[i++] & 0x0F) << 12;
			c |= (text[i++] & 0x3F) << 6;
			c |=  text[i++] & 0x3F;
		} else {
			i++; // out of range or something (This only does up to 0xFFFF since it goes to a U16 anyways)
		}
		out += c;
	}
	return out;
}

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
	long int left = 0;
	long int right = imageCount;
	long int mid = 0;

	while (left <= right) {
		mid = left + ((right - left) / 2);
		if (mapping[mid] == letter) {
			spriteIndex = mid;
			break;
		}

		if (mapping[mid] < letter) {
			left = mid + 1;
		} else {
			right = mid - 1;
		}
	}
	return spriteIndex;
}

char16_t FontGraphic::getCharacter(const char *&text) {
	// UTF-8 handling
	if ((*text & 0x80) == 0) {
		return getSpriteIndex(*text++);
	} else if ((*text & 0xE0) == 0xC0) {
		char16_t c = ((*text++ & 0x1F) << 6);
		if ((*text & 0xC0) == 0x80) c |= *text++ & 0x3F;

		return getSpriteIndex(c);
	} else if ((*text & 0xF0) == 0xE0) {
		char16_t c = (*text++ & 0xF) << 12;
		if ((*text & 0xC0) == 0x80) c |= (*text++ & 0x3F) << 6;
		if ((*text & 0xC0) == 0x80) c |=  *text++ & 0x3F;

		return getSpriteIndex(c);
	} else if ((*text & 0xF8) == 0xF0) {
		char16_t c = (*text++ & 0x7) << 18;
		if ((*text & 0xC0) == 0x80) c |= (*text++ & 0x3F) << 12;
		if ((*text & 0xC0) == 0x80) c |= (*text++ & 0x3F) << 6;
		if ((*text & 0xC0) == 0x80) c |=  *text++ & 0x3F;

		return getSpriteIndex(c);
	} else {
		// Character isn't valid, return ?
		text++;
		return getSpriteIndex('?');
	}
}

void FontGraphic::print(int x, int y, const char *text)
{
	while (*text) {
		char16_t fontChar = getCharacter(text);
		glSprite(x, y, GL_FLIP_NONE, &fontSprite[fontChar]);
		x += fontSprite[fontChar].width;
	}
}

int FontGraphic::calcWidth(const char *text)
{
	int x = 0;

	while (*text) {
		x += fontSprite[getCharacter(text)].width;
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
	int total_width = 0;
	while (*text) {
		total_width += fontSprite[getCharacter(text)].width;
	}
	return (SCREEN_WIDTH - total_width) / 2;
}

void FontGraphic::printCentered(int y, const char *text)
{
	int x = getCenteredX(text);
	while (*text) {
		char16_t fontChar = getCharacter(text);
		glSprite(x, y, GL_FLIP_NONE, &fontSprite[fontChar]);
		x += fontSprite[fontChar].width;
	}
}

void FontGraphic::printCentered(int y, int value)
{
	sprintf(buffer, "%i", value);
	printCentered(y, buffer);
}