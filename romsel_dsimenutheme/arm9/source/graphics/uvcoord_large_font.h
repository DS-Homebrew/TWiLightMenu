/*======================================================================

   large_font texture coordinates

======================================================================*/


#ifndef LARGE_FONT__H
#define LARGE_FONT__H


#define LARGE_FONT_BITMAP_WIDTH   512
#define LARGE_FONT_BITMAP_HEIGHT  128
#define LARGE_FONT_NUM_IMAGES   128

#define TEXT_LY 15
		//	Format:
		//	U,V,Width,Height



extern const unsigned int large_font_texcoords[] = {
	/* 0, 0, 0, TEXT_LY, // 0
	16, 0, 0, TEXT_LY, // 1
	32, 0, 0, TEXT_LY, // 2
	48, 0, 0, TEXT_LY, // 3
	64, 0, 0, TEXT_LY, // 4
	80, 0, 0, TEXT_LY, // 5
	96, 0, 0, TEXT_LY, // 6
	112, 0, 0, TEXT_LY, // 7
	128, 0, 0, TEXT_LY, // 8
	144, 0, 0, TEXT_LY, // 9
	160, 0, 0, TEXT_LY, // 10
	176, 0, 0, TEXT_LY, // 11
	192, 0, 0, TEXT_LY, // 12
	208, 0, 0, TEXT_LY, // 13
	224, 0, 0, TEXT_LY, // 14
	240, 0, 0, TEXT_LY, // 15
	256, 0, 0, TEXT_LY, // 16
	272, 0, 0, TEXT_LY, // 17
	288, 0, 0, TEXT_LY, // 18
	304, 0, 0, TEXT_LY, // 19
	320, 0, 0, TEXT_LY, // 20
	336, 0, 0, TEXT_LY, // 21
	352, 0, 0, TEXT_LY, // 22
	368, 0, 0, TEXT_LY, // 23
	384, 0, 0, TEXT_LY, // 24
	400, 0, 0, TEXT_LY, // 25
	416, 0, 0, TEXT_LY, // 26
	432, 0, 0, TEXT_LY, // 27
	448, 0, 0, TEXT_LY, // 28
	464, 0, 0, TEXT_LY, // 29
	480, 0, 0, TEXT_LY, // 30
	496, 0, 0, TEXT_LY, // 31 */
	0, 16, 3, TEXT_LY, // Space
	16, 16, 3, TEXT_LY, // !
	32, 16, 6, TEXT_LY, // "
	48, 16, 10, TEXT_LY, // #
	64, 16, 9, TEXT_LY, // $
	80, 16, 12, TEXT_LY, // %
	96, 16, 0, TEXT_LY, // NULL
	112, 16, 3, TEXT_LY, // '
	128, 16, 4, TEXT_LY, // (
	144, 16, 4, TEXT_LY, // )
	160, 16, 3, TEXT_LY, // '
	178, 16, 9, TEXT_LY, // +
	192, 16, 5, TEXT_LY, // ,
	208, 16, 5, TEXT_LY, // -
	224, 16, 3, TEXT_LY, // .
	240, 16, 8, TEXT_LY, // /
	256, 16, 8, TEXT_LY, // 0
	272, 16, 8, TEXT_LY, // 1
	288, 16, 8, TEXT_LY, // 2
	304, 16, 8, TEXT_LY, // 3
	320, 16, 9, TEXT_LY, // 4
	336, 16, 8, TEXT_LY, // 5
	352, 16, 8, TEXT_LY, // 6
	368, 16, 8, TEXT_LY, // 7
	384, 16, 8, TEXT_LY, // 8
	400, 16, 8, TEXT_LY, // 9
	416, 16, 4, TEXT_LY, // :
	432, 16, 5, TEXT_LY, // ;
	448, 16, 9, TEXT_LY, // <
	464, 16, 8, TEXT_LY, // =
	480, 16, 9, TEXT_LY, // >
	496, 16, 8, TEXT_LY, // ?
	0, 32, 13, TEXT_LY, // @
	16, 32, 11, TEXT_LY, // A
	32, 32, 10, TEXT_LY, // B
	48, 32, 10, TEXT_LY, // C
	64, 32, 10, TEXT_LY, // D
	80, 32, 9, TEXT_LY, // E
	96, 32, 8, TEXT_LY, // F
	112, 32, 10, TEXT_LY, // G
	128, 32, 10, TEXT_LY, // H
	144, 32, 3, TEXT_LY, // I
	160, 32, 6, TEXT_LY, // J
	176, 32, 10, TEXT_LY, // K
	192, 32, 8, TEXT_LY, // L
	208, 32, 12, TEXT_LY, // M
	224, 32, 10, TEXT_LY, // N
	240, 32, 11, TEXT_LY, // O
	256, 32, 9, TEXT_LY, // P
	272, 32, 11, TEXT_LY, // Q
	288, 32, 9, TEXT_LY, // R
	304, 32, 9, TEXT_LY, // S
	320, 32, 9, TEXT_LY, // T
	336, 32, 10, TEXT_LY, // U
	352, 32, 11, TEXT_LY, // V
	368, 32, 14, TEXT_LY, // W
	384, 32, 10, TEXT_LY, // X
	400, 32, 10, TEXT_LY, // Y
	416, 32, 9, TEXT_LY, // Z
	432, 32, 5, TEXT_LY, // [
	448, 32, 8, TEXT_LY, // /
	464, 32, 5, TEXT_LY, // ]
	480, 32, 8, TEXT_LY, // ^
	496, 32, 10, TEXT_LY, // _
	0, 48, 3, TEXT_LY, // NULL
	16, 48, 7, TEXT_LY, // a
	32, 48, 8, TEXT_LY, // b
	48, 48, 7, TEXT_LY, // c
	64, 48, 8, TEXT_LY, // d
	80, 48, 8, TEXT_LY, // e
	96, 48, 7, TEXT_LY, // f
	112, 48, 9, TEXT_LY, // g
	128, 48, 8, TEXT_LY, // h
	144, 48, 4, TEXT_LY, // i
	160, 48, 6, TEXT_LY, // j
	176, 48, 7, TEXT_LY, // k
	192, 48, 3, TEXT_LY, // l
	208, 48, 13, TEXT_LY, // m
	224, 48, 8, TEXT_LY, // n
	240, 48, 8, TEXT_LY, // o
	256, 48, 8, TEXT_LY, // p
	272, 48, 8, TEXT_LY, // q
	288, 48, 6, TEXT_LY, // r
	304, 48, 6, TEXT_LY, // s
	320, 48, 6, TEXT_LY, // t
	336, 48, 8, TEXT_LY, // u
	352, 48, 9, TEXT_LY, // v
	368, 48, 13, TEXT_LY, // w
	384, 48, 9, TEXT_LY, // x
	400, 48, 9, TEXT_LY, // y
	416, 48, 6, TEXT_LY, // z
	432, 48, 6, TEXT_LY, // {
	448, 48, 3, TEXT_LY, // |
	464, 48, 5, TEXT_LY, // }
	480, 48, 8, TEXT_LY, // "
	496, 48, 0, TEXT_LY, // NULL
};


#endif
