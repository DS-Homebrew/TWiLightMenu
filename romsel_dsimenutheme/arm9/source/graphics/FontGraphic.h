/******************************************************************************
 *******************************************************************************
	A simple font class for Easy GL2D DS created by:

	Relminator (Richard Eric M. Lope BSN RN)
	Http://Rel.Phatcode.Net

 *******************************************************************************
 ******************************************************************************/
#pragma once
#include <gl2d.h>
#define FONT_SX 8
#define FONT_SY 10
#define UTF16_SIGNAL_BYTE  0x0F

class FontGraphic
{
private:
	glImage *fontSprite;
	const unsigned short int *mapping;
	unsigned int imageCount;
	char buffer[256];
	char buffer2[256];
	unsigned int getSpriteIndex(const u16 letter);

public:

	FontGraphic() { };
	int load(int textureID, glImage *_font_sprite,
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
			);
	void print(int x, int y, const char *text);
	int calcWidth(const char *text);
	void print(int x, int y, int value);
	int getCenteredX(const char *text);
	void printCentered(int y, const char *text);
	void printCentered(int y, int value);
};