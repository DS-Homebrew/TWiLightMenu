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
protected:
	glImage *fontSprite;
	char buffer[256];
	char buffer2[256];
	static unsigned short int getSpriteIndex(const u16 letter);

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
	// todo: remove this virtual once both fonts are ported to dynamic paging.
	virtual void print(int x, int y, const char *text);
	virtual int calcWidth(const char *text);
    void print(int x, int y, int value);
	virtual int getCenteredX(const char *text);
	virtual void printCentered(int y, const char *text);
	void printCentered(int y, int value);
};