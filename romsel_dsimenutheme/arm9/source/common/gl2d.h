/*---------------------------------------------------------------------------------

	Easy GL2D

	Relminator 2010 
	Richard Eric M. Lope BSN RN

	http://rel.betterwebber.com

	A very small and simple DS rendering lib using the 3d core to render 2D stuff

---------------------------------------------------------------------------------*/

/*!
 \mainpage Easy GL2D Documentation


 
 \section api Library API
 - \ref gl2d.h "Reference"
 
 \section intro Official Site
 - <a href="http://rel.phatcode.net">Genso's Junkyard</a> 
 
 \section download Download
 - <a href="http://rel.phatcode.net/junk.php?id=117">Easy GL2D</a> 
 
 \section games Example Games
 - <a href="http://rel.phatcode.net/index.php?action=contents&item=Bubble_Fight_EX_DS">Bubble Fight EX</a> 
 - <a href="http://rel.phatcode.net/index.php?action=contents&item=Space-Impakto-DS">Space Impakto DS</a> 
 
 \section tools Misc. Tools
 - <a href="http://rel.phatcode.net/junk.php?id=106">Rel's Texture Packer</a> 
 
 \section external_links Useful links
 - <a href="http://www.devkitpro.org/">devkitPro</a>
 - <a href="http://www.forum.gbadev.org">GbaDev forums</a>
 
*/




/*! \file gl2d.h
	\brief A very small and simple DS rendering lib using the 3d core to render 2D stuff.
*/


#include <nds/arm9/videoGL.h>

#ifndef GL2D__H
#define GL2D__H


/*! \brief Enums selecting flipping mode.
 *
 *	These enums are bits for flipping the sprites. <Br>
 *	You can <b>"|"</b> (or) GL_FLIP_V and GL_FLIP_H to flip 
 *	both ways. <Br><Br>
 *  <ul>
 *	<li> Related functions:
 * 		<ol>
 *	 	<li>glSprite()
 * 		<li>glSpriteScale()
 *	 	<li>glSpriteRotate()
 * 		<li>glSpriteScaleXY(()
 *	 	<li>glSpriteRotateScale()
 * 		<li>glSpriteRotateScaleXY()
 *	 	<li>glSpriteOnQuad()
 * 		</ol>
 * 	</ul>	
 */
 
typedef enum
{

	GL_FLIP_NONE 	= (1 << 0), /*!< No flipping */
	GL_FLIP_V 		= (1 << 1), /*!< Sprite is rendered vertically flipped */
	GL_FLIP_H 		= (1 << 2), /*!< Sprite is rendered horizontally flipped */
	
} GL_FLIP_MODE;


/*! \brief Struct for our GL-Based Images<BR>
This is our struct to hold our image
	attributes. You don't need to worry about this 
	if you use the texture packer. */

/*! \brief Struct for out GL-Based Images.
 *
 *	This is our struct to hold our image attributes. <Br>
 *	You don't need to worry about this if you use the texture packer. <Br><Br>
 *  <ul>
 *	<li> Related functions:
 * 		<ol>
 *	 	<li>glSprite()
 * 		<li>glSpriteScale()
 *	 	<li>glSpriteRotate()
 * 		<li>glSpriteScaleXY(()
 *	 	<li>glSpriteRotateScale()
 * 		<li>glSpriteRotateScaleXY()
 *	 	<li>glSpriteOnQuad()
 * 		</ol>
 * 	</ul>	
 */

typedef struct 
{

	int		width;		/*!< Width of the Sprite */      			
	int 	height;		/*!< Height of the Sprite */	
	int 	u_off;		/*!< S texture offset */
	int 	v_off;		/*!< T texture offset */
	int		textureID;  /*!< Texture handle ( used in glDeleteTextures() ) <Br> 
	  						 The texture handle in VRAM (returned by glGenTextures()) <Br>
							 ie. This references the actual texture stored in VRAM */

} glImage;



#ifdef __cplusplus
extern "C" 
{
#endif


extern int gCurrentTexture;

/******************************************************************************
	
	Function Prototypes
	
******************************************************************************/



/*! \brief Initializes GL in 2D mode */
void glScreen2D( void );


/*! \brief Sets up OpenGL for 2d rendering. <Br> 
	Call this before drawing any of GL2D's drawing or sprite functions.
 */
void glBegin2D( void );


/*! \brief Issue this after drawing 2d so that we don't mess the matrix stack. <Br> 
	The compliment of glBegin2D().
 */
void glEnd2D( void );


/*! \brief Returns the active texture. Use with care. <Br> 
	Needed to achieve some effects since libnds 1.5.0.
 */
static inline int glGetActiveTexture()
{
	return gCurrentTexture;
}



/*! \brief Set the active texture. Use with care. <Br> 
	Needed to achieve some effects since libnds 1.5.0.
 */
static inline void glSetActiveTexture( int TextureID )
{

	glBindTexture(0, TextureID );
	gCurrentTexture = TextureID;
	
}



/*! \brief Draws a Pixel
\param x X position of the pixel.
\param y Y position of the pixel.
\param color RGB15/ARGB16 color.
*/
void glPutPixel( int x, int y, int color );


/*! \brief Draws a Line
\param x1,y1 Top-Left coordinate of the line.
\param x2,y2 Bottom-Right coordinate of the line.
\param color RGB15/ARGB16 color.
*/
void glLine( int x1, int y1, int x2, int y2, int color );


/*! \brief Draws a Box
\param x1,y1 Top-Left coordinate of the box.
\param x2,y2 Bottom-Right coordinate of the box.
\param color RGB15/ARGB16 color.
*/
void glBox( int x1, int y1, int x2, int y2, int color );


/*! \brief Draws a Filled Box
\param x1,y1 Top-Left coordinate of the box.
\param x2,y2 Bottom-Right coordinate of the box.
\param color RGB15/ARGB16 color.
*/
void glBoxFilled( int x1, int y1, int x2, int y2, int color );


/*! \brief Draws a Filled Box in gradient colors
\param x1,y1 Top-Left coordinate of the box.
\param x2,y2 Bottom-Right coordinate of the box.
\param color1 RGB15/ARGB16 color of the Top-Left corner.
\param color2 RGB15/ARGB16 color of the Bottom-Left corner.
\param color3 RGB15/ARGB16 color of the Bottom-Right corner.
\param color4 RGB15/ARGB16 color of the Top-Right corner.
*/
void glBoxFilledGradient( int x1, int y1, int x2, int y2,
						  int color1, int color2, int color3, int color4
						);


/*! \brief Draws a Triangle
\param x1,y1 Vertex 1 of the triangle.
\param x2,y2 Vertex 2 of the triangle.
\param x3,y3 Vertex 3 of the triangle.
\param color RGB15/ARGB16 color of the triangle.
*/
void glTriangle( int x1, int y1, int x2, int y2, int x3, int y3, int color );


/*! \brief Draws a Filled Triangle
\param x1,y1 Vertex 1 of the triangle.
\param x2,y2 Vertex 2 of the triangle.
\param x3,y3 Vertex 3 of the triangle.
\param color RGB15/ARGB16 color of the triangle.
*/
void glTriangleFilled( int x1, int y1, int x2, int y2, int x3, int y3, int color );



/*! \brief Draws a Triangle in gradient colors
\param x1,y1 Vertex 1 of the triangle.
\param x2,y2 Vertex 2 of the triangle.
\param x3,y3 Vertex 3 of the triangle.
\param color1 RGB15/ARGB16 color of the vertex 1.
\param color2 RGB15/ARGB16 color of the vertex 2.
\param color3 RGB15/ARGB16 color of the vertex 3.
*/
void glTriangleFilledGradient( int x1, int y1, int x2, int y2, int x3, int y3,
							   int color1, int color2, int color3
							 );


/*! \brief Draws a Sprite
\param x X position of the sprite.
\param y Y position of the sprite.
\param flipmode mode for flipping (see GL_FLIP_MODE enum).
\param *spr pointer to a glImage. 
*/
void glSprite( int x, int y, int flipmode, const glImage *spr );


/*! \brief Draws a Scaled Sprite
\param x X position of the sprite.
\param y Y position of the sprite.
\param scale 20.12 fixed-point scale value (1 << 12 is normal).
\param flipmode mode for flipping (see GL_FLIP_MODE enum).
\param *spr pointer to a glImage. 
*/
void glSpriteScale( int x, int y, s32 scale, int flipmode, const glImage *spr );


/*! \brief Draws an Axis Exclusive Scaled Sprite
\param x X position of the sprite.
\param y Y position of the sprite.
\param scaleX 20.12 fixed-point X-Axis scale value (1 << 12 is normal).
\param scaleY 20.12 fixed-point Y-Axis scale value (1 << 12 is normal).
\param flipmode mode for flipping (see GL_FLIP_MODE enum).
\param *spr pointer to a glImage. 
*/
void glSpriteScaleXY( int x, int y, s32 scaleX, s32 scaleY, int flipmode, const glImage *spr );


/*! \brief Draws a Center Rotated Sprite
\param x X position of the sprite center.
\param y Y position of the sprite center.
\param angle Binary Radian Angle(-32768 to 32767) to rotate the sprite.
\param flipmode mode for flipping (see GL_FLIP_MODE enum).
\param *spr pointer to a glImage. 
*/
void glSpriteRotate( int x, int y, s32 angle, int flipmode, const glImage *spr );


/*! \brief Draws a Center Rotated Scaled Sprite
\param x X position of the sprite center.
\param y Y position of the sprite center.
\param angle Binary Radian Angle(-32768 to 32767) to rotate the sprite.
\param scale 20.12 fixed-point scale value (1 << 12 is normal).
\param flipmode mode for flipping (see GL_FLIP_MODE enum).
\param *spr pointer to a glImage. 
*/
void glSpriteRotateScale( int x, int y, s32 angle, s32 scale, int flipmode, const glImage *spr);


/*! \brief Draws a Center Rotated Axis-Exclusive Scaled Sprite
\param x X position of the sprite center.
\param y Y position of the sprite center.
\param angle Binary Radian Angle(-32768 to 32767) to rotate the sprite.
\param scaleX 20.12 fixed-point X-Axis scale value (1 << 12 is normal).
\param scaleY 20.12 fixed-point Y-Axis scale value (1 << 12 is normal).
\param flipmode mode for flipping (see GL_FLIP_MODE enum).
\param *spr pointer to a glImage. 
*/
void glSpriteRotateScaleXY( int x, int y, s32 angle, s32 scaleX, s32 scaleY, int flipmode, const glImage *spr);


/*! \brief Draws a Horizontaly Stretched Sprite (Clean Stretching) <Br>
	Useful for "Laser Effects".
\param x X position of the sprite center.
\param y Y position of the sprite center.
\param length_x The length(in pixels) to stretch the sprite.
\param *spr pointer to a glImage. 
*/
void glSpriteStretchHorizontal(int x, int y, int length_x, const glImage *spr );


/*! \brief Draws a Horizontaly Stretched Sprite (Clean Stretching) <Br>
	Useful for "Shrearing Effects".
\param x1,y1 First corner of the sprite.
\param x2,y2 Second corner of the sprite.
\param x3,y3 Third corner of the sprite.
\param x4,y4 Fourth corner of the sprite.
\param uoff,voff texture offsets.
\param flipmode mode for flipping (see GL_FLIP_MODE enum).
\param *spr pointer to a glImage. 
*/
void glSpriteOnQuad( int x1, int y1,
					 int x2, int y2,
					 int x3, int y3,
					 int x4, int y4,
					 int uoff, int voff,
					 int flipmode, const glImage *spr 
				   );


/*! \brief Initializes our spriteset with Texture Packer generated UV coordinates <Br>
	Very safe and easy to use..
\param *sprite Pointer to an array of glImage.
\param numframes number of frames in a spriteset (auto-generated by Texture Packer).
\param *texcoords Texture Packer auto-generated array of UV coords.
\param type The format of the texture ( see glTexImage2d() ).
\param sizeX The horizontal size of the texture; valid sizes are enumerated in GL_TEXTURE_TYPE_ENUM ( see glTexImage2d() ).
\param sizeY The vertical size of the texture; valid sizes are enumerated in GL_TEXTURE_TYPE_ENUM ( see glTexImage2d() ).
\param param parameters for the texture ( see glTexImage2d() ). 
\param pallette_width Length of the palette. Valid values are <b>4, 16, 32, 256</b> (if <b>0</b>, then palette is removed from currently bound texture). 
\param *palette Pointer to the palette data to load (if NULL, then palette is removed from currently bound texture).
\param *texture Pointer to the texture data to load.
*/
int glLoadSpriteSet( glImage              *sprite,
                     const unsigned int   numframes, 
					 const unsigned int   *texcoords,
					 GL_TEXTURE_TYPE_ENUM type,
					 int 	              sizeX,
					 int 	              sizeY,
					 int 	              param,
					 int				  pallette_width,
					 const u16			  *palette,
					 const uint8          *texture	 
                   );


/*! \brief Initializes our Tileset (like glInitSpriteset()) but without the use of Texture Packer auto-generated files. <Br>
	Can only be used when tiles in a tilset are of the same dimensions.
\param *sprite Pointer to an array of glImage.
\param tile_wid Width of each tile in the texture.
\param tile_hei Height of each tile in the texture.
\param bmp_wid Width of of the texture or tileset.
\param bmp_hei height of of the texture or tileset.
\param type The format of the texture ( see glTexImage2d() ).
\param sizeX The horizontal size of the texture; valid sizes are enumerated in GL_TEXTURE_TYPE_ENUM ( see glTexImage2d() ).
\param sizeY The vertical size of the texture; valid sizes are enumerated in GL_TEXTURE_TYPE_ENUM ( see glTexImage2d() ).
\param param parameters for the texture ( see glTexImage2d() ). 
\param pallette_width Length of the palette. Valid values are <b>4, 16, 32, 256</b> (if <b>0</b>, then palette is removed from currently bound texture). 
\param *palette Pointer to the palette data to load (if NULL, then palette is removed from currently bound texture).
\param *texture Pointer to the texture data to load.
*/
int glLoadTileSet( glImage              *sprite,
				   int                  tile_wid,
				   int                  tile_hei,
                   int                  bmp_wid,
				   int                  bmp_hei,
				   GL_TEXTURE_TYPE_ENUM type,
				   int 	                sizeX,
				   int 	                sizeY,
				   int 	                param,
				   int					pallette_width,
				   const u16			*palette,
				   const uint8          *texture	 
                 );

				 
				 
#ifdef __cplusplus
}
#endif

#endif


