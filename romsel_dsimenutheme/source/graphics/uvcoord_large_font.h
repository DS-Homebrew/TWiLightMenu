/*======================================================================

   large_font texture coordinates

======================================================================*/


#ifndef LARGE_FONT__H
#define LARGE_FONT__H


#define LARGE_FONT_BITMAP_WIDTH   512
#define LARGE_FONT_BITMAP_HEIGHT  128
#define LARGE_FONT_NUM_IMAGES   128

#define TEXT_SY 15
		//	Format:
		//	U,V,Width,Height



extern const unsigned int large_font_texcoords[] = {
	/* 0, 0, 0, TEXT_SY, // 0
	16, 0, 0, TEXT_SY, // 1
	32, 0, 0, TEXT_SY, // 2
	48, 0, 0, TEXT_SY, // 3
	64, 0, 0, TEXT_SY, // 4
	80, 0, 0, TEXT_SY, // 5
	96, 0, 0, TEXT_SY, // 6
	112, 0, 0, TEXT_SY, // 7
	128, 0, 0, TEXT_SY, // 8
	144, 0, 0, TEXT_SY, // 9
	160, 0, 0, TEXT_SY, // 10
	176, 0, 0, TEXT_SY, // 11
	192, 0, 0, TEXT_SY, // 12
	208, 0, 0, TEXT_SY, // 13
	224, 0, 0, TEXT_SY, // 14
	240, 0, 0, TEXT_SY, // 15
	256, 0, 0, TEXT_SY, // 16
	272, 0, 0, TEXT_SY, // 17
	288, 0, 0, TEXT_SY, // 18
	304, 0, 0, TEXT_SY, // 19
	320, 0, 0, TEXT_SY, // 20
	336, 0, 0, TEXT_SY, // 21
	352, 0, 0, TEXT_SY, // 22
	368, 0, 0, TEXT_SY, // 23
	384, 0, 0, TEXT_SY, // 24
	400, 0, 0, TEXT_SY, // 25
	416, 0, 0, TEXT_SY, // 26
	432, 0, 0, TEXT_SY, // 27
	448, 0, 0, TEXT_SY, // 28
	464, 0, 0, TEXT_SY, // 29
	480, 0, 0, TEXT_SY, // 30
	496, 0, 0, TEXT_SY, // 31 */
	0, 16, 3, TEXT_SY, // Space
	16, 16, 3, TEXT_SY, // !
	32, 16, 6, TEXT_SY, // "
	48, 16, 10, TEXT_SY, // #
	64, 16, 9, TEXT_SY, // $
	80, 16, 12, TEXT_SY, // %
	96, 16, 0, TEXT_SY, // NULL
	112, 16, 3, TEXT_SY, // '
	128, 16, 4, TEXT_SY, // (
	144, 16, 4, TEXT_SY, // )
	160, 16, 3, TEXT_SY, // '
	178, 16, 9, TEXT_SY, // +
	192, 16, 5, TEXT_SY, // ,
	208, 16, 5, TEXT_SY, // -
	224, 16, 3, TEXT_SY, // .
	240, 16, 8, TEXT_SY, // /
	256, 16, 8, TEXT_SY, // 0
	272, 16, 8, TEXT_SY, // 1
	288, 16, 8, TEXT_SY, // 2
	304, 16, 8, TEXT_SY, // 3
	320, 16, 9, TEXT_SY, // 4
	336, 16, 8, TEXT_SY, // 5
	352, 16, 8, TEXT_SY, // 6
	368, 16, 8, TEXT_SY, // 7
	384, 16, 8, TEXT_SY, // 8
	400, 16, 8, TEXT_SY, // 9
	416, 16, 4, TEXT_SY, // :
	432, 16, 5, TEXT_SY, // ;
	448, 16, 9, TEXT_SY, // <
	464, 16, 8, TEXT_SY, // =
	480, 16, 9, TEXT_SY, // >
	496, 16, 8, TEXT_SY, // ?
	0, 32, 13, TEXT_SY, // @
	16, 32, 11, TEXT_SY, // A
	32, 32, 8, TEXT_SY, // B
	48, 32, 9, TEXT_SY, // C
	64, 32, 9, TEXT_SY, // D
	80, 32, 7, TEXT_SY, // E
	96, 32, 7, TEXT_SY, // F
	112, 32, 10, TEXT_SY, // G
	128, 32, 10, TEXT_SY, // H
	144, 32, 3, TEXT_SY, // I
	160, 32, 5, TEXT_SY, // J
	176, 32, 9, TEXT_SY, // K
	192, 32, 7, TEXT_SY, // L
	208, 32, 13, TEXT_SY, // M
	224, 32, 9, TEXT_SY, // N
	240, 32, 10, TEXT_SY, // O
	256, 32, 8, TEXT_SY, // P
	272, 32, 11, TEXT_SY, // Q
	288, 32, 9, TEXT_SY, // R
	304, 32, 7, TEXT_SY, // S
	320, 32, 9, TEXT_SY, // T
	336, 32, 10, TEXT_SY, // U
	352, 32, 11, TEXT_SY, // V
	368, 32, 16, TEXT_SY, // W
	384, 32, 10, TEXT_SY, // X
	400, 32, 9, TEXT_SY, // Y
	416, 32, 9, TEXT_SY, // X
	432, 32, 5, TEXT_SY, // [
	448, 32, 8, TEXT_SY, // /
	464, 32, 5, TEXT_SY, // ]
	480, 32, 8, TEXT_SY, // ^
	496, 32, 10, TEXT_SY, // _
	0, 48, 3, TEXT_SY, // NULL
	16, 48, 7, TEXT_SY, // a
	32, 48, 8, TEXT_SY, // b
	48, 48, 7, TEXT_SY, // c
	64, 48, 8, TEXT_SY, // d
	80, 48, 8, TEXT_SY, // e
	96, 48, 7, TEXT_SY, // f
	112, 48, 9, TEXT_SY, // g
	128, 48, 8, TEXT_SY, // h
	144, 48, 4, TEXT_SY, // i
	160, 48, 6, TEXT_SY, // j
	176, 48, 7, TEXT_SY, // k
	192, 48, 3, TEXT_SY, // l
	208, 48, 13, TEXT_SY, // m
	224, 48, 8, TEXT_SY, // n
	240, 48, 8, TEXT_SY, // o
	256, 48, 8, TEXT_SY, // p
	272, 48, 8, TEXT_SY, // q
	288, 48, 6, TEXT_SY, // r
	304, 48, 6, TEXT_SY, // s
	320, 48, 6, TEXT_SY, // t
	336, 48, 8, TEXT_SY, // u
	352, 48, 9, TEXT_SY, // v
	368, 48, 13, TEXT_SY, // w
	384, 48, 9, TEXT_SY, // x
	400, 48, 9, TEXT_SY, // y
	416, 48, 6, TEXT_SY, // z
	432, 48, 6, TEXT_SY, // {
	448, 48, 3, TEXT_SY, // |
	464, 48, 5, TEXT_SY, // }
	480, 48, 8, TEXT_SY, // "
	496, 48, 0, TEXT_SY, // NULL
};


#endif
