/******************************************************************************
 *******************************************************************************
	A simple font class for Easy GL2D DS created by:

	Relminator (Richard Eric M. Lope BSN RN)
	Http://Rel.Phatcode.Net

 *******************************************************************************
 ******************************************************************************/

#include <nds.h>
#include <stdio.h>
#include "common/gl2d.h"
#include "FontGraphic.h"

int FontGraphic::load(glImage *_font_sprite,
				  const unsigned int numframes,
				  const unsigned int *texcoords,
				  GL_TEXTURE_TYPE_ENUM type,
				  int sizeX,
				  int sizeY,
				  int param,
				  int pallette_width,
				  const u16 *palette,
				  const uint8 *texture
				  )

{
	fontSprite = _font_sprite;

	int textureID =
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

	return textureID;

}

void FontGraphic::print(int x, int y, const char *text)
{
	unsigned char font_char;

	while (*text)
	{
		font_char = (*(unsigned char*) text++) - 32;
		glSprite(x, y, GL_FLIP_NONE, &fontSprite[font_char]);
		x += fontSprite[font_char].width;
	}
}

int FontGraphic::calcWidth(const char *text)
{
	unsigned char font_char;
	int x = 0;

	while (*text)
	{
		font_char = (*(unsigned char*) text++) - 32;
		x += fontSprite[font_char].width;
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
	unsigned char fontChar;
	int total_width = 0;
	while (*text)
	{
		fontChar = (*(unsigned char*) text++) - 32;
		total_width += fontSprite[fontChar].width;
	}
	return (SCREEN_WIDTH - total_width) / 2;
}

void FontGraphic::printCentered(int y, const char *text)
{
	unsigned char fontChar;
	int x = getCenteredX(text);
	while (*text)
	{
		fontChar = (*(unsigned char*) text++) - 32;
		glSprite(x, y, GL_FLIP_NONE, &fontSprite[fontChar]);
		x += fontSprite[fontChar].width;
	}
}

void FontGraphic::printCentered(int y, int value)
{
	sprintf(buffer, "%i", value);
	printCentered(y, buffer);
}