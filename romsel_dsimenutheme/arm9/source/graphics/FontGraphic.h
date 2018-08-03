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

class FontGraphic
{
private:
	glImage *fontSprite;
	u16 buffer[256];
	u16 buffer2[256];
	
public:

	FontGraphic() { };
	int load(glImage *_font_sprite,
			const unsigned int numframes,
			const unsigned int *texcoords,
			GL_TEXTURE_TYPE_ENUM type,
			int sizeX,
			int sizeY,
			int param,
			int pallette_width,
			const u16 *palette,
			const uint8 *texture
			);
	void print(int x, int y, const u16 *text);
	int calcWidth(const u16 *text);
	void print(int x, int y, int value);
	int getCenteredX(const u16 *text);
	void printCentered(int y, const u16 *text);
	void printCentered(int y, int value);
};