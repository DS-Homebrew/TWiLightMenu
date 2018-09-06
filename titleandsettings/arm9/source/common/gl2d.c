/******************************************************************************

	Easy GL2D

	Relminator 2010 
	Richard Eric M. Lope BSN RN

	http://rel.betterwebber.com

	A very small and simple DS rendering lib using the 3d core to render 2D stuff

******************************************************************************/


#include "gl2d.h"


/******************************************************************************

                          INLINE HARDWARE WRAPPERS

******************************************************************************/




/******************************************************************************
	
	Direct copy of glVertex3v16()
	I made this since VideoGL don't have a
	glVertex#i() wrappers

******************************************************************************/
static inline void gxVertex3i(v16 x, v16 y, v16 z)
{
	GFX_VERTEX16 = (y << 16) | (x & 0xFFFF);
	GFX_VERTEX16 = ((uint32)(uint16)z);
}



/******************************************************************************

	Again no gxVertex2i() in the videoGL header
	This is used for optimizing vertex calls

******************************************************************************/
static inline void gxVertex2i(v16 x, v16 y)
{
	GFX_VERTEX_XY = (y << 16) | (x & 0xFFFF);	
}



/******************************************************************************

	Almost a direct copy of TEXTURE_PACK except that
	UV coords are shifted left by 4 bits.
	U and V are shifted left by 4 bits
	since GFX_TEX_COORD expects 12.4 Fixed point values

******************************************************************************/
static inline void gxTexcoord2i(t16 u, t16 v)
{
	GFX_TEX_COORD = (v << 20) | ( (u << 4) & 0xFFFF );
}




/******************************************************************************

    I made this since the scale wrappers are either the 
	vectorized mode or does not permit you to scale only
	the axis you want to scale. Needed for sprite scaling.

******************************************************************************/
static inline void gxScalef32(s32 x, s32 y, s32 z)
{
	MATRIX_SCALE = x;
	MATRIX_SCALE = y;
	MATRIX_SCALE = z;
}




/******************************************************************************

    I this made for future naming conflicts.

******************************************************************************/
static inline void gxTranslate3f32( int32 x, int32 y, int32 z ) 
{
	MATRIX_TRANSLATE = x;
	MATRIX_TRANSLATE = y;
	MATRIX_TRANSLATE = z;
}




/******************************************************************************

                          IMPLEMENTATION

******************************************************************************/


/*
  Our static global variable used for
  Depth values since we cannot disable 
  depth testing in the DS hardware
  This value is incremented for every draw call
*/
static v16 g_depth = 0;
int gCurrentTexture = 0;

/******************************************************************************

    !!! PRIVATE !!!
	Set orthographic projection
	at 1:1 correspondence to screen coords
	glOrtho expects f32 values but if we use the
	standard f32 values, we need to rescale either
	every vert or the modelview matrix by the same
	amount to make it work.  That's gonna give us
	lots of overflows and headaches.  So we "scale down"
	and use an all integer value.

******************************************************************************/
void SetOrtho( void )		
{

	glMatrixMode( GL_PROJECTION );     // set matrixmode to projection
	glLoadIdentity();				 // reset
	glOrthof32( 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, -1 << 12, 1 << 12 );  // downscale projection matrix
	
}



/******************************************************************************

	Initializes GL in 2D mode 
	Also initializes GL in 3d mode so that we could combine 2D and 3D later
	Almost a direct copy from the DS example files

******************************************************************************/
void glScreen2D( void )
{
		
	// initialize gl
	glInit();
	
	//enable textures
	glEnable( GL_TEXTURE_2D );
	
	// enable antialiasing
	glEnable( GL_ANTIALIAS );
		
	// setup the rear plane
	glClearColor( 0, 0, 0, 31 ); // BG must be opaque for AA to work
	glClearPolyID( 63 ); // BG must have a unique polygon ID for AA to work
	
	glClearDepth( GL_MAX_DEPTH );

	//this should work the same as the normal gl call
	glViewport(0,0,255,191);
	
	
	//any floating point gl call is being converted to fixed prior to being implemented
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluPerspective( 70, 256.0 / 192.0, 1, 200 );
	
	gluLookAt(	0.0, 0.0, 1.0,		//camera possition 
				0.0, 0.0, 0.0,		//look at
				0.0, 1.0, 0.0);		//up

	glMaterialf( GL_AMBIENT, RGB15(31,31,31) );
	glMaterialf( GL_DIFFUSE, RGB15(31,31,31) );
	glMaterialf( GL_SPECULAR, BIT(15) | RGB15(31,31,31) );
	glMaterialf( GL_EMISSION, RGB15(31,31,31) );

	//ds uses a table for shinyness..this generates a half-ass one
	glMaterialShinyness();

	//not a real gl function and will likely change
	glPolyFmt( POLY_ALPHA(31) | POLY_CULL_BACK );
	
	
}



/******************************************************************************

   Sets up OpenGL for 2d rendering
   Call this before drawing any of GL2D's
   drawing or sprite functions.

******************************************************************************/
void glBegin2D( void )
{

	
	// save 3d perpective projection matrix
	glMatrixMode( GL_PROJECTION );   
    glPushMatrix();
	
	// save 3d modelview matrix for safety
	glMatrixMode( GL_MODELVIEW );
    glPushMatrix();
	
	
	//what?!! No glDisable(GL_DEPTH_TEST)?!!!!!!
	glEnable( GL_BLEND );
	glEnable( GL_TEXTURE_2D );
	glDisable( GL_ANTIALIAS );		// disable AA
	glDisable( GL_OUTLINE );			// disable edge-marking

	glColor( 0x7FFF ); 				// max color
	
	glPolyFmt( POLY_ALPHA(31) | POLY_CULL_NONE );  // no culling

	SetOrtho();
	
	glMatrixMode( GL_TEXTURE );      	// reset texture matrix just in case we did some funky stuff with it
	glLoadIdentity();
	
	glMatrixMode( GL_MODELVIEW );		// reset modelview matrix. No need to scale up by << 12
	glLoadIdentity();

	gCurrentTexture = 0; // set current texture to 0 
	g_depth = 0; 	// set depth to 0. We need this var since we cannot disable depth testing

}



/******************************************************************************

	Issue this after drawing 2d so that we don't mess the matrix stack
	The compliment of glBegin2D

******************************************************************************/
void glEnd2D( void )
{

	// restore 3d matrices and set current matrix to modelview
	glMatrixMode( GL_PROJECTION );    
    glPopMatrix( 1 );
	glMatrixMode( GL_MODELVIEW );
    glPopMatrix( 1 );

}


/******************************************************************************

	Returns the active texture. 
	Needed to achieve some effects since libnds 1.5.0.
	
******************************************************************************/



/******************************************************************************

	Sets the active texture. 
	Needed to achieve some effects since libnds 1.5.0.
	
******************************************************************************/


/******************************************************************************

	Draws a pixel
	Parameters:
		x,y 	-> First coordinate of the line 
		color 	-> RGB15/ARGB16 color 
		
******************************************************************************/
void glPutPixel( int x, int y, int color )
{

	glBindTexture( 0, 0 );
	glColor( color);
	glBegin( GL_TRIANGLES );
		gxVertex3i( x, y, g_depth );
		gxVertex2i( x, y );
		gxVertex2i( x, y );
	glEnd();
	glColor( 0x7FFF );
	g_depth++;
	gCurrentTexture = 0;
	
}

/******************************************************************************

	Draws a line
	Parameters:
		x1,y1 	-> First coordinate of the line 
		x2,y2 	-> Second coordinate of the line 
		color 	-> RGB15/ARGB16 color 
		
******************************************************************************/
void glLine( int x1, int y1, int x2, int y2, int color )
{

	x2++;
	y2++;
	
	glBindTexture( 0, 0 );
	glColor( color);
	glBegin( GL_TRIANGLES );
		gxVertex3i( x1, y1, g_depth );
		gxVertex2i( x2, y2 );
		gxVertex2i( x2, y2 );
	glEnd();
	glColor( 0x7FFF );
	g_depth++;
	gCurrentTexture = 0;
	
}


/******************************************************************************
    
	Draws a Box
	Parameters:
		x1,y1 	-> Top-left corner of the box 
		x2,y2 	-> Bottom-Right corner of the box 
		color 	-> RGB15/ARGB16 color 

******************************************************************************/
void glBox( int x1, int y1, int x2, int y2, int color )
{
	
	x2++;
	y2++;
	
	glBindTexture( 0, 0 );
	glColor( color );
	glBegin( GL_TRIANGLES );
	
		gxVertex3i( x1, y1, g_depth );
		gxVertex2i( x2, y1 );
		gxVertex2i( x2, y1 );

		gxVertex2i( x2, y1 );
		gxVertex2i( x2, y2 );
		gxVertex2i( x2, y2 );

		gxVertex2i( ++x2,y2 );  // bug fix for lower-right corner disappearing pixel
		gxVertex2i( x1,y2 );
		gxVertex2i( x1,y2 );

		gxVertex2i( x1,y2 );
		gxVertex2i( x1,y1 );
		gxVertex2i( x1,y1 );
		
	glEnd();
	glColor( 0x7FFF );
	g_depth++;
	gCurrentTexture = 0;

}

/******************************************************************************
   
	Draws a Filled Box
	Parameters:
		x1,y1 	-> Top-left corner of the box 
		x2,y2 	-> Bottom-Right corner of the box 
		color 	-> RGB15/ARGB16 color 

******************************************************************************/
void glBoxFilled( int x1, int y1, int x2, int y2, int color )
{

	x2++;
	y2++;
	
	glBindTexture( 0, 0 );
	glColor( color );
	glBegin( GL_QUADS );
		gxVertex3i( x1, y1, g_depth );		// use 3i for first vertex so that we increment HW depth
		gxVertex2i( x1, y2 );				// no need for 3 vertices as 2i would share last depth call
		gxVertex2i( x2, y2 );
		gxVertex2i( x2, y1 );
	glEnd();
	glColor( 0x7FFF );
	g_depth++;
	gCurrentTexture = 0;

}


/******************************************************************************
   
	Draws a filled box in gradient colors
   	Parameters:
		x1,y1 	-> Top-left corner of the box 
		x2,y2 	-> Bottom-Right corner of the box 
		color1 	-> RGB15/ARGB16 color of the first corner
		color2 	-> RGB15/ARGB16 color of the second corner
		color3 	-> RGB15/ARGB16 color of the third corner
		color4 	-> RGB15/ARGB16 color of the fourth corner


******************************************************************************/
void glBoxFilledGradient( int x1, int y1, int x2, int y2,
						  int color1, int color2, int color3, int color4
						)
{

	x2++;
	y2++;
	
	glBindTexture( 0,0 );
	glBegin( GL_QUADS );
		glColor( color1 ); gxVertex3i( x1, y1, g_depth );		// use 3i for first vertex so that we increment HW depth
		glColor( color2 ); gxVertex2i( x1, y2 );				// no need for 3 vertices as 2i would share last depth call
		glColor( color3 ); gxVertex2i( x2, y2 );
		glColor( color4 ); gxVertex2i( x2, y1 );
	glEnd();
	glColor( 0x7FFF );
	g_depth++;
	gCurrentTexture = 0;

}



/******************************************************************************
    
	Draws a Triangle
   	Parameters:
		x1,y1 	-> vertex 1 of the triangle 
		x2,y2 	-> vertex 2 of the triangle
		x3,y3 	-> vertex 3 of the triangle 
		color 	-> RGB15/ARGB16 color of the triangle

******************************************************************************/
void glTriangle( int x1, int y1, int x2, int y2, int x3, int y3, int color )
{
	
	glBindTexture( 0, 0 );
	glColor( color );
	glBegin( GL_TRIANGLES );
	
		gxVertex3i( x1, y1, g_depth );
		gxVertex2i( x2, y2 );
		gxVertex2i( x2, y2 );

		gxVertex2i( x2, y2 );
		gxVertex2i( x3, y3 );
		gxVertex2i( x3, y3 );

		gxVertex2i( x3, y3 );  
		gxVertex2i( x1, y1 );
		gxVertex2i( x1, y1 );
		
	glEnd();
	glColor( 0x7FFF );
	g_depth++;
	gCurrentTexture = 0;

}

/******************************************************************************
   
	Draws a filled triangle
   	Parameters:
		x1,y1 	-> vertex 1 of the triangle 
		x2,y2 	-> vertex 2 of the triangle
		x3,y3 	-> vertex 3 of the triangle 
		color 	-> RGB15/ARGB16 color of the triangle

******************************************************************************/
void glTriangleFilled( int x1, int y1, int x2, int y2, int x3, int y3, int color )
{

	glBindTexture( 0, 0 );
	glColor( color );
	glBegin( GL_TRIANGLES);
		gxVertex3i( x1, y1, g_depth );		// use 3i for first vertex so that we increment HW depth
		gxVertex2i( x2, y2 );				// no need for 3 vertices as 2i would share last depth call
		gxVertex2i( x3, y3 );
	glEnd();
	glColor( 0x7FFF );
	g_depth++;
	gCurrentTexture = 0;

}


/******************************************************************************
   
	Draws a filled Triangle in gradient colors
   	Parameters:
		x1,y1 	-> vertex 1 of the triangle 
		x2,y2 	-> vertex 2 of the triangle
		x3,y3 	-> vertex 3 of the triangle 
		color1 	-> RGB15/ARGB16 color of the triangle
		color2 	-> RGB15/ARGB16 color of the triangle
		color3 	-> RGB15/ARGB16 color of the triangle

******************************************************************************/
void glTriangleFilledGradient( int x1, int y1, int x2, int y2, int x3, int y3,
							   int color1, int color2, int color3
							 )
{

	glBindTexture( 0, 0 );
	glBegin( GL_TRIANGLES );
		glColor( color1 ); gxVertex3i( x1, y1, g_depth );		// use 3i for first vertex so that we increment HW depth
		glColor( color2 ); gxVertex2i( x2, y2 );				// no need for 3 vertices as 2i would share last depth call
		glColor( color3 ); gxVertex2i( x3, y3 );
	glEnd();
	glColor( 0x7FFF );
	g_depth++;
	gCurrentTexture = 0;

}



/******************************************************************************

    Draws a sprite
	Parameters:
		x 			-> x position of the sprite
		y 			-> y position of the sprite
		flipmode 	-> mode for flipping (see GL_FLIP_MODE enum)
		*spr 		-> pointer to a glImage

******************************************************************************/
void glSprite( int x, int y, int flipmode, const glImage *spr )
{
	int x1 = x;
	int y1 = y;
	int x2 = x + spr->width;
	int y2 = y + spr->height;

	int	u1 = spr->u_off + (( flipmode & GL_FLIP_H ) ? spr->width-1  : 0);
 	int	u2 = spr->u_off + (( flipmode & GL_FLIP_H ) ? 0			    : spr->width);
	int v1 = spr->v_off + (( flipmode & GL_FLIP_V ) ? spr->height-1 : 0);
 	int v2 = spr->v_off + (( flipmode & GL_FLIP_V ) ? 0 		    : spr->height);

	
 
    if ( spr->textureID != gCurrentTexture )
    {
        glBindTexture( GL_TEXTURE_2D, spr->textureID );
        gCurrentTexture = spr->textureID;
    }

	glBegin( GL_QUADS );
		
		gxTexcoord2i( u1, v1 ); gxVertex3i( x1, y1, g_depth );	
		gxTexcoord2i( u1, v2 ); gxVertex2i( x1, y2 );
		gxTexcoord2i( u2, v2 ); gxVertex2i( x2, y2 );
		gxTexcoord2i( u2, v1 ); gxVertex2i( x2, y1 );
		
	glEnd();
	
	g_depth++;

}


/******************************************************************************

    Draws a scaled sprite
	Parameters:
		x 			-> x position of the sprite
		y 			-> y position of the sprite
		scale    	-> 20.12 FP scale value (1 << 12 is normal) 
		flipmode 	-> mode for flipping (see GL_FLIP_MODE enum)
		*spr 		-> pointer to a glImage

******************************************************************************/
void glSpriteScale( int x, int y, s32 scale, int flipmode, const glImage *spr )
{
	int x1 = 0;
	int y1 = 0;
	int x2 = spr->width;
	int y2 = spr->height;

	int	u1 = spr->u_off + (( flipmode & GL_FLIP_H ) ? spr->width-1  : 0);
 	int	u2 = spr->u_off + (( flipmode & GL_FLIP_H ) ? 0		    	: spr->width-1);
	int v1 = spr->v_off + (( flipmode & GL_FLIP_V ) ? spr->height-1 : 0);
 	int v2 = spr->v_off + (( flipmode & GL_FLIP_V ) ? 0		    	: spr->height-1);
 	
 
    if ( spr->textureID != gCurrentTexture )
    {
        glBindTexture( GL_TEXTURE_2D, spr->textureID );
        gCurrentTexture = spr->textureID;
    }

	glPushMatrix();

		gxTranslate3f32( x, y, 0 );
		gxScalef32( scale, scale, 1 << 12 );

		glBegin( GL_QUADS );
			
			gxTexcoord2i( u1, v1 ); gxVertex3i( x1, y1, g_depth );	
			gxTexcoord2i( u1, v2 ); gxVertex2i( x1, y2 );
			gxTexcoord2i( u2, v2 ); gxVertex2i( x2, y2 );
			gxTexcoord2i( u2, v1 ); gxVertex2i( x2, y1 );
			
		glEnd();
	
	glPopMatrix( 1 );
	g_depth++;

}



/******************************************************************************

    Draws an axis exclusive scaled sprite
	Parameters:
		x 			-> x position of the sprite
		y 			-> y position of the sprite
		scaleX   	-> 20.12 FP X axis scale value (1 << 12 is normal)
		scaleY   	-> 20.12 FP Y axis scale value (1 << 12 is normal)
		flipmode 	-> mode for flipping (see GL_FLIP_MODE enum)
		*spr 		-> pointer to a glImage

******************************************************************************/
void glSpriteScaleXY( int x, int y, s32 scaleX, s32 scaleY, int flipmode, const glImage *spr )
{
	int x1 = 0;
	int y1 = 0;
	int x2 = spr->width;
	int y2 = spr->height;

	int	u1 = spr->u_off + (( flipmode & GL_FLIP_H ) ? spr->width-1  : 0);
 	int	u2 = spr->u_off + (( flipmode & GL_FLIP_H ) ? 0		    	: spr->width-1);
	int v1 = spr->v_off + (( flipmode & GL_FLIP_V ) ? spr->height-1 : 0);
 	int v2 = spr->v_off + (( flipmode & GL_FLIP_V ) ? 0		    	: spr->height-1);
 	
 
    if ( spr->textureID != gCurrentTexture )
    {
        glBindTexture( GL_TEXTURE_2D, spr->textureID );
        gCurrentTexture = spr->textureID;
    }

	glPushMatrix();

		gxTranslate3f32( x, y, 0 );
		gxScalef32( scaleX, scaleY, 1 << 12 );

		glBegin( GL_QUADS );
			
			gxTexcoord2i( u1, v1 ); gxVertex3i( x1, y1, g_depth );	
			gxTexcoord2i( u1, v2 ); gxVertex2i( x1, y2 );
			gxTexcoord2i( u2, v2 ); gxVertex2i( x2, y2 );
			gxTexcoord2i( u2, v1 ); gxVertex2i( x2, y1 );
			
		glEnd();
	
	glPopMatrix( 1 );
	g_depth++;

}

/******************************************************************************

    Draws a center rotated sprite
	Parameters:
		x 			-> x position of the sprite center
		y 			-> y position of the sprite center
		angle    	-> angle to rotate the sprite (-32768 to 32767)
		flipmode 	-> mode for flipping (see GL_FLIP_MODE enum)
		*spr 		-> pointer to a glImage

******************************************************************************/
void glSpriteRotate( int x, int y, s32 angle, int flipmode, const glImage *spr )
{
	
	int s_half_x = ((spr->width) + (spr->width & 1)) / 2;
	int	s_half_y = ((spr->height) + (spr->height & 1))  / 2;
	
	int x1 =  -s_half_x;
	int y1 =  -s_half_y;
	
	int x2 =  s_half_x;
	int y2 =  s_half_y;
	
	
	int	u1 = spr->u_off + (( flipmode & GL_FLIP_H ) ? spr->width-1  : 0);
 	int	u2 = spr->u_off + (( flipmode & GL_FLIP_H ) ? 0		    	: spr->width-1);
	int v1 = spr->v_off + (( flipmode & GL_FLIP_V ) ? spr->height-1 : 0);
 	int v2 = spr->v_off + (( flipmode & GL_FLIP_V ) ? 0		    	: spr->height-1);
 	
    if ( spr->textureID != gCurrentTexture )
    {
        glBindTexture( GL_TEXTURE_2D, spr->textureID );
        gCurrentTexture = spr->textureID;
    }
	
	glPushMatrix();
	
		gxTranslate3f32( x, y, 0 );
		glRotateZi( angle );
		
		
		glBegin( GL_QUADS );
			
			gxTexcoord2i( u1, v1 ); gxVertex3i( x1, y1, g_depth );	
			gxTexcoord2i( u1, v2 ); gxVertex2i( x1, y2 );
			gxTexcoord2i( u2, v2 ); gxVertex2i( x2, y2 );
			gxTexcoord2i( u2, v1 ); gxVertex2i( x2, y1 );
			
		glEnd();
	
	glPopMatrix( 1 );
		
	g_depth++;

}


/******************************************************************************

    Draws a center rotated and scaled sprite
	Parameters:
		x 			-> x position of the sprite center
		y 			-> y position of the sprite center
		angle    	-> is the angle to rotated (-32768 to 32767)
		scale    	-> 20.12 FP X axis scale value (1 << 12 is normal)
		flipmode 	-> mode for flipping (see GL_FLIP_MODE enum)
		*spr 		-> pointer to a glImage

******************************************************************************/
void glSpriteRotateScale( int x, int y, s32 angle, s32 scale, int flipmode, const glImage *spr)
{
	
	int s_half_x = ((spr->width) + (spr->width & 1)) / 2;
	int	s_half_y = ((spr->height) + (spr->height & 1))  / 2;
	
	int x1 =  -s_half_x;
	int y1 =  -s_half_y;
	
	int x2 =  s_half_x;
	int y2 =  s_half_y;
	
	int	u1 = spr->u_off + (( flipmode & GL_FLIP_H ) ? spr->width-1  : 0);
 	int	u2 = spr->u_off + (( flipmode & GL_FLIP_H ) ? 0		    	: spr->width-1);
	int v1 = spr->v_off + (( flipmode & GL_FLIP_V ) ? spr->height-1 : 0);
 	int v2 = spr->v_off + (( flipmode & GL_FLIP_V ) ? 0		    	: spr->height-1);
 	
    if ( spr->textureID != gCurrentTexture )
    {
        glBindTexture( GL_TEXTURE_2D, spr->textureID );
        gCurrentTexture = spr->textureID;
    }
	
	glPushMatrix();
	
		gxTranslate3f32( x, y, 0 );
		gxScalef32( scale, scale, 1 << 12 );
		glRotateZi( angle );
		
		
		glBegin( GL_QUADS );
			
			gxTexcoord2i( u1, v1 ); gxVertex3i( x1, y1, g_depth );	
			gxTexcoord2i( u1, v2 ); gxVertex2i( x1, y2 );
			gxTexcoord2i( u2, v2 ); gxVertex2i( x2, y2 );
			gxTexcoord2i( u2, v1 ); gxVertex2i( x2, y1 );
			
		glEnd();
	
	glPopMatrix( 1 );
		
	g_depth++;

}



/******************************************************************************

    Draws a center rotated and scaled sprite
	Parameters:
		x 			-> x position of the sprite center
		y 			-> y position of the sprite center
		angle    	-> is the angle to rotated (-32768 to 32767)
		scaleX   	-> 20.12 FP X axis scale value (1 << 12 is normal)
		scaleY   	-> 20.12 FP Y axis scale value (1 << 12 is normal)
		flipmode	-> mode for flipping (see GL_FLIP_MODE enum)
		*spr 		-> pointer to a glImage

******************************************************************************/
void glSpriteRotateScaleXY( int x, int y, s32 angle, s32 scaleX, s32 scaleY, int flipmode, const glImage *spr)
{
	
	int s_half_x = ((spr->width) + (spr->width & 1)) / 2;
	int	s_half_y = ((spr->height) + (spr->height & 1))  / 2;
	
	int x1 =  -s_half_x;
	int y1 =  -s_half_y;
	
	int x2 =  s_half_x;
	int y2 =  s_half_y;
	
	int	u1 = spr->u_off + (( flipmode & GL_FLIP_H ) ? spr->width-1  : 0);
 	int	u2 = spr->u_off + (( flipmode & GL_FLIP_H ) ? 0		    	: spr->width-1);
	int v1 = spr->v_off + (( flipmode & GL_FLIP_V ) ? spr->height-1 : 0);
 	int v2 = spr->v_off + (( flipmode & GL_FLIP_V ) ? 0		    	: spr->height-1);
 	
    if ( spr->textureID != gCurrentTexture )
    {
        glBindTexture( GL_TEXTURE_2D, spr->textureID );
        gCurrentTexture = spr->textureID;
    }
	
	glPushMatrix();
	
		gxTranslate3f32( x, y, 0 );
		gxScalef32( scaleX, scaleY, 1 << 12 );
		glRotateZi( angle );
		
		
		glBegin( GL_QUADS );
			
			gxTexcoord2i( u1, v1 ); gxVertex3i( x1, y1, g_depth );	
			gxTexcoord2i( u1, v2 ); gxVertex2i( x1, y2 );
			gxTexcoord2i( u2, v2 ); gxVertex2i( x2, y2 );
			gxTexcoord2i( u2, v1 ); gxVertex2i( x2, y1 );
			
		glEnd();
	
	glPopMatrix( 1 );
		
	g_depth++;

}


/******************************************************************************

    Draws a clean-streched sprite
	* Useful for lasers, effects, etc.
	Parameters:
		x 			-> x position of the sprite 
		y 			-> y position of the sprite 
		length_x	-> the length to stretch the sprite
		*spr 		-> pointer to a glImage


******************************************************************************/
void glSpriteStretchHorizontal(int x, int y, int length_x, const glImage *spr )
{
	int x1 = x;
	int y1 = y;
	int x2 = x + length_x;
	int y2 = y + spr->height;
	int su = ( spr->width/2 )-1;
	
	
	int	u1 = spr->u_off;
 	int	u2 = spr->u_off + spr->width;
	int v1 = spr->v_off;
 	int v2 = spr->v_off + spr->height;
 	
    
	if ( spr->textureID != gCurrentTexture )
    {
        glBindTexture( GL_TEXTURE_2D, spr->textureID );
        gCurrentTexture = spr->textureID;
    }
	
	// left
	int x2l = x + su;
	glBegin( GL_QUADS );
	
		gxTexcoord2i( u1, v1 );
		gxVertex3i( x1, y1, g_depth );
		
		gxTexcoord2i( u1, v2 );
		gxVertex2i( x1, y2 );
		
		gxTexcoord2i( u1 + su, v2);
		gxVertex2i( x2l, y2 );
		
		gxTexcoord2i( u1 + su, v1 );
		gxVertex2i( x2l, y1 );
		
	glEnd();
	
	// center
	int x1l = x + su;
	x2l = x2 - su - 1; 
	glBegin( GL_QUADS );
	
		gxTexcoord2i( u1 + su, v1 );
		gxVertex2i( x1l, y1 );
		
		gxTexcoord2i( u1 + su, v2 );
		gxVertex2i( x1l, y2 );
		
		gxTexcoord2i( u1 + su, v2 );
		gxVertex2i( x2l, y2 );
		
		gxTexcoord2i( u1 + su, v1 );
		gxVertex2i( x2l, y1 );
		
	glEnd();
	
	// right
	x1l = x2 - su-1;
	glBegin( GL_QUADS );
	
		gxTexcoord2i( u1 + su, v1 );
		gxVertex2i( x1l, y1 );
		
		gxTexcoord2i( u1 + su, v2 );
		gxVertex2i( x1l, y2 );
		
		gxTexcoord2i( u2, v2 );
		gxVertex2i( x2, y2 );
		
		gxTexcoord2i( u2, v1 );
		gxVertex2i( x2, y1 );
		
	glEnd();
	
	g_depth++;

}


/******************************************************************************

    Draws a sprite stretched on a quad
	Parameters:
		x1 			-> x1 position of the sprite
		y1 			-> y1 position of the sprite
		x2 			-> x2 position of the sprite
		y2 			-> y2 position of the sprite
		x3 			-> x3 position of the sprite
		y3 			-> y3 position of the sprite
		x4 			-> x4 position of the sprite
		y4 			-> y4 position of the sprite
		flipmode 	-> mode for flipping (see GL_FLIP_MODE enum)
		*spr 		-> pointer to a glImage
 
******************************************************************************/
void glSpriteOnQuad( int x1, int y1,
					 int x2, int y2,
					 int x3, int y3,
					 int x4, int y4,
					 int uoff, int voff,
					 int flipmode, const glImage *spr 
				   )
{
	
	int	u1 = spr->u_off + (( flipmode & GL_FLIP_H ) ? spr->width-1  : 0);
 	int	u2 = spr->u_off + (( flipmode & GL_FLIP_H ) ? 0		    	: spr->width-1);
	int v1 = spr->v_off + (( flipmode & GL_FLIP_V ) ? spr->height-1 : 0);
 	int v2 = spr->v_off + (( flipmode & GL_FLIP_V ) ? 0		    	: spr->height-1);
 	
 
    if ( spr->textureID != gCurrentTexture )
    {
        glBindTexture( GL_TEXTURE_2D, spr->textureID );
        gCurrentTexture = spr->textureID;
    }

	glBegin( GL_QUADS );
		
		gxTexcoord2i( u1+uoff, v1+voff ); gxVertex3i( x1, y1, g_depth );	
		gxTexcoord2i( u1+uoff, v2+voff ); gxVertex2i( x2, y2 );
		gxTexcoord2i( u2+uoff, v2+voff ); gxVertex2i( x3, y3 );
		gxTexcoord2i( u2+uoff, v1+voff ); gxVertex2i( x4, y4 );
		
	glEnd();
	
	g_depth++;

}


/******************************************************************************

    Initializes our spriteset with Texture Packer generated UV coordinates
	Very safe and easy to use.
	Parameters:
		*spr 		-> pointer to an array of glImage
		numframes 	-> number of frames in a spriteset (auto-generated by Texture Packer)
		*texcoords 	-> Texture Packer auto-generated array of UV coords
		type 		-> The format of the texture (see glTexImage2d)
		sizeX 		-> the horizontal size of the texture; valid sizes are enumerated in GL_TEXTURE_TYPE_ENUM (see glTexImage2d)
		sizeY 		-> the vertical size of the texture; valid sizes are enumerated in GL_TEXTURE_TYPE_ENUM (see glTexImage2d)
		param 		-> parameters for the texture (see glTexImage2d)

******************************************************************************/
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
                   )
{


	int textureID; 
	glGenTextures( 1, &textureID );
	glBindTexture( 0, textureID );
	glTexImage2D( 0, 0, type, sizeX, sizeY, 0, param, texture );
	glColorTableEXT( 0, 0, pallette_width, 0, 0, palette );
	
	int i;
	// init sprites texture coords and texture ID
	for ( i=0; i < numframes; i++)
	{
		int j = i * 4; // texcoords array is u_off, wid, hei
		sprite[i].textureID = textureID;
		sprite[i].u_off = texcoords[j];			// set x-coord
		sprite[i].v_off = texcoords[j+1];			// y-coord 
		sprite[i].width = texcoords[j+2];			// don't decrease because NDS 3d core does not draw last vertical texel
		sprite[i].height = texcoords[j+3];		    // ditto
	}

	return textureID;
}


/******************************************************************************

    Initializes our Tileset (like glInitSpriteset()) but without the use of
	Texture Packer auto-generated files.
	Very rigid and prone to human error.
	!!! Can ony be used for tilesets whose tiles are the same sizes!!!
	Parameters:
		*sprite 		-> pointer to an array of glImage
		tile_wid 	-> width of each tile in the texture
		tile_hei 	-> height of each tile in the texture
		bmp_wid 	-> width of of the texture in VRAM
		bmp_hei 	-> height of of the texture in VRAM
		type 		-> The format of the texture (see glTexImage2d)
		sizeX 		-> the horizontal size of the texture; valid sizes are enumerated in GL_TEXTURE_TYPE_ENUM (see glTexImage2d)
		sizeY 		-> the vertical size of the texture; valid sizes are enumerated in GL_TEXTURE_TYPE_ENUM (see glTexImage2d)
		param 		-> parameters for the texture (see glTexImage2d)

******************************************************************************/
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
                 )
{


	int textureID; 
	glGenTextures( 1, &textureID );
	glBindTexture( 0, textureID );
	glTexImage2D( 0, 0, type, sizeX, sizeY, 0, param, texture );
	glColorTableEXT( 0, 0, pallette_width, 0, 0, palette );

	int i=0;
	int x, y;
	
	// init sprites texture coords and texture ID
	for (y = 0; y < (bmp_hei/tile_hei); y++)
	{
		for (x = 0; x < (bmp_wid/tile_wid); x++) 
		{
			sprite[i].width 			= tile_wid;
			sprite[i].height 			= tile_hei;
			sprite[i].u_off				= x*tile_wid;
			sprite[i].v_off				= y*tile_hei; 
			sprite[i].textureID 		= textureID;
			i++;
		}
	}

	return textureID;
}



