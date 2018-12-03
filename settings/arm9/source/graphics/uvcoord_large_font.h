/*======================================================================

   large_font texture coordinates

======================================================================*/


#ifndef LARGE_FONT__H
#define LARGE_FONT__H


#define LARGE_FONT_BITMAP_WIDTH   512
#define LARGE_FONT_BITMAP_HEIGHT  160
#define LARGE_FONT_NUM_IMAGES   0xF0

#define TEXT_LY 20
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
	0, 20, 5, TEXT_LY, // Space
	16, 20, 3, TEXT_LY, // !
	32, 20, 6, TEXT_LY, // "
	48, 20, 10, TEXT_LY, // #
	64, 20, 9, TEXT_LY, // $
	80, 20, 13, TEXT_LY, // %
	96, 20, 11, TEXT_LY, // &
	112, 20, 3, TEXT_LY, // '
	128, 20, 5, TEXT_LY, // (
	144, 20, 5, TEXT_LY, // )
	160, 20, 9, TEXT_LY, // *
	176, 20, 9, TEXT_LY, // +
	192, 20, 4, TEXT_LY, // ,
	208, 20, 6, TEXT_LY, // -
	224, 20, 4, TEXT_LY, // .
	240, 20, 6, TEXT_LY, // /
	256, 20, 9, TEXT_LY, // 0
	272, 20, 5, TEXT_LY, // 1
	288, 20, 9, TEXT_LY, // 2
	304, 20, 9, TEXT_LY, // 3
	320, 20, 10, TEXT_LY, // 4
	336, 20, 9, TEXT_LY, // 5
	352, 20, 9, TEXT_LY, // 6
	368, 20, 9, TEXT_LY, // 7
	384, 20, 9, TEXT_LY, // 8
	400, 20, 9, TEXT_LY, // 9
	416, 20, 3, TEXT_LY, // :
	432, 20, 3, TEXT_LY, // ;
	448, 20, 8, TEXT_LY, // <
	464, 20, 9, TEXT_LY, // =
	480, 20, 8, TEXT_LY, // >
	496, 20, 9, TEXT_LY, // ?
	0, 40, 13, TEXT_LY, // @
	16, 40, 10, TEXT_LY, // A
	32, 40, 9, TEXT_LY, // B
	48, 40, 9, TEXT_LY, // C
	64, 40, 10, TEXT_LY, // D
	80, 40, 9, TEXT_LY, // E
	96, 40, 8, TEXT_LY, // F
	112, 40, 10, TEXT_LY, // G
	128, 40, 10, TEXT_LY, // H
	144, 40, 3, TEXT_LY, // I
	160, 40, 6, TEXT_LY, // J
	176, 40, 9, TEXT_LY, // K
	192, 40, 8, TEXT_LY, // L
	208, 40, 12, TEXT_LY, // M
	224, 40, 10, TEXT_LY, // N
	240, 40, 10, TEXT_LY, // O
	256, 40, 9, TEXT_LY, // P
	272, 40, 10, TEXT_LY, // Q
	288, 40, 9, TEXT_LY, // R
	304, 40, 9, TEXT_LY, // S
	320, 40, 9, TEXT_LY, // T
	336, 40, 10, TEXT_LY, // U
	352, 40, 10, TEXT_LY, // V
	368, 40, 13, TEXT_LY, // W
	384, 40, 9, TEXT_LY, // X
	400, 40, 9, TEXT_LY, // Y
	416, 40, 9, TEXT_LY, // Z
	432, 40, 5, TEXT_LY, // [
	448, 40, 6, TEXT_LY, // "\"
	464, 40, 5, TEXT_LY, // ]
	480, 40, 7, TEXT_LY, // ^
	496, 40, 8, TEXT_LY, // _
	0, 60, 5, TEXT_LY, // `
	16, 60, 8, TEXT_LY, // a
	32, 60, 8, TEXT_LY, // b
	48, 60, 7, TEXT_LY, // c
	64, 60, 8, TEXT_LY, // d
	80, 60, 8, TEXT_LY, // e
	96, 60, 6, TEXT_LY, // f
	112, 60, 8, TEXT_LY, // g
	128, 60, 8, TEXT_LY, // h
	144, 60, 3, TEXT_LY, // i
	160, 60, 5, TEXT_LY, // j
	176, 60, 7, TEXT_LY, // k
	192, 60, 3, TEXT_LY, // l
	208, 60, 11, TEXT_LY, // m
	224, 60, 8, TEXT_LY, // n
	240, 60, 8, TEXT_LY, // o
	256, 60, 8, TEXT_LY, // p
	272, 60, 8, TEXT_LY, // q
	288, 60, 5, TEXT_LY, // r
	304, 60, 7, TEXT_LY, // s
	320, 60, 6, TEXT_LY, // t
	336, 60, 8, TEXT_LY, // u
	352, 60, 8, TEXT_LY, // v
	368, 60, 11, TEXT_LY, // w
	384, 60, 8, TEXT_LY, // x
	400, 60, 8, TEXT_LY, // y
	416, 60, 7, TEXT_LY, // z
	432, 60, 5, TEXT_LY, // {
	448, 60, 3, TEXT_LY, // |
	464, 60, 5, TEXT_LY, // }
	480, 60, 9, TEXT_LY, // ~
	496, 60, 0, TEXT_LY, // NULL
	0, 80, 10, TEXT_LY, // €
	16, 80, 0, TEXT_LY, // NULL
	32, 80, 3, TEXT_LY, // ‚
	48, 80, 8, TEXT_LY, // ƒ
	64, 80, 6, TEXT_LY, // „
	80, 80, 14, TEXT_LY, // …
	96, 80, 7, TEXT_LY, // †
	112, 80, 7, TEXT_LY, // ‡
	128, 80, 6, TEXT_LY, // ˆ
	144, 80, 13, TEXT_LY, // ‰
	160, 80, 9, TEXT_LY, // Š
	176, 80, 6, TEXT_LY, // ‹
	192, 80, 12, TEXT_LY, // Œ
	208, 80, 0, TEXT_LY, // NULL
	224, 80, 9, TEXT_LY, // Ž
	240, 80, 0, TEXT_LY, // NULL
	256, 80, 0, TEXT_LY, // NULL
	272, 80, 3, TEXT_LY, // ‘
	288, 80, 3, TEXT_LY, // ’
	304, 80, 6, TEXT_LY, // “
	320, 80, 6, TEXT_LY, // ”
	336, 80, 4, TEXT_LY, // •
	352, 80, 8, TEXT_LY, // –
	368, 80, 15, TEXT_LY, // —
	384, 80, 6, TEXT_LY, // ˜
	400, 80, 12, TEXT_LY, // ™
	416, 80, 7, TEXT_LY, // š
	432, 80, 6, TEXT_LY, // ›
	448, 80, 13, TEXT_LY, // œ
	464, 80, 0, TEXT_LY, // NULL
	480, 80, 7, TEXT_LY, // ž
	496, 80, 9, TEXT_LY, // Ÿ
	0, 100, 5, TEXT_LY, // Space
	16, 100, 3, TEXT_LY, // ¡
	32, 100, 7, TEXT_LY, // ¢
	48, 100, 9, TEXT_LY, // £
	64, 100, 7, TEXT_LY, // ¤
	80, 100, 9, TEXT_LY, // ¥
	96, 100, 3, TEXT_LY, // ¦
	112, 100, 8, TEXT_LY, // §
	128, 100, 6, TEXT_LY, // ¨
	144, 100, 10, TEXT_LY, // ©
	160, 100, 5, TEXT_LY, // ª
	176, 100, 8, TEXT_LY, // «
	192, 100, 8, TEXT_LY, // ¬
	208, 100, 0, TEXT_LY, // NULL
	224, 100, 10, TEXT_LY, // ®
	240, 100, 6, TEXT_LY, // ¯
	256, 100, 5, TEXT_LY, // °
	272, 100, 8, TEXT_LY, // ±
	288, 100, 4, TEXT_LY, // ²
	304, 100, 5, TEXT_LY, // ³
	320, 100, 4, TEXT_LY, // ´
	336, 100, 8, TEXT_LY, // µ
	352, 100, 8, TEXT_LY, // ¶
	368, 100, 4, TEXT_LY, // ·
	384, 100, 4, TEXT_LY, // ¸
	400, 100, 4, TEXT_LY, // ¹
	416, 100, 4, TEXT_LY, // º
	432, 100, 8, TEXT_LY, // »
	448, 100, 11, TEXT_LY, // ¼
	464, 100, 11, TEXT_LY, // ½
	480, 100, 11, TEXT_LY, // ¾
	496, 100, 9, TEXT_LY, // ¿
	0, 120, 10, TEXT_LY, // À
	16, 120, 10, TEXT_LY, // Á
	32, 120, 10, TEXT_LY, // Â
	48, 120, 10, TEXT_LY, // Ã
	64, 120, 10, TEXT_LY, // Ä
	80, 120, 10, TEXT_LY, // Å
	96, 120, 13, TEXT_LY, // Æ
	112, 120, 9, TEXT_LY, // Ç
	128, 120, 9, TEXT_LY, // È
	144, 120, 9, TEXT_LY, // É
	160, 120, 9, TEXT_LY, // Ê
	176, 120, 9, TEXT_LY, // Ë
	192, 120, 4, TEXT_LY, // Ì
	208, 120, 4, TEXT_LY, // Í
	224, 120, 7, TEXT_LY, // Î
	240, 120, 7, TEXT_LY, // Ï
	256, 120, 10, TEXT_LY, // Ð
	272, 120, 10, TEXT_LY, // Ñ
	288, 120, 10, TEXT_LY, // Ò
	304, 120, 10, TEXT_LY, // Ó
	320, 120, 10, TEXT_LY, // Ô
	336, 120, 10, TEXT_LY, // Õ
	352, 120, 10, TEXT_LY, // Ö
	368, 120, 8, TEXT_LY, // ×
	384, 120, 10, TEXT_LY, // Ø
	400, 120, 10, TEXT_LY, // Ù
	416, 120, 10, TEXT_LY, // Ú
	432, 120, 10, TEXT_LY, // Û
	448, 120, 10, TEXT_LY, // Ü
	464, 120, 9, TEXT_LY, // Ý
	480, 120, 9, TEXT_LY, // Þ
	496, 120, 9, TEXT_LY, // ß
	0, 140, 8, TEXT_LY, // à
	16, 140, 8, TEXT_LY, // á
	32, 140, 8, TEXT_LY, // â
	48, 140, 8, TEXT_LY, // ã
	64, 140, 8, TEXT_LY, // ä
	80, 140, 8, TEXT_LY, // å
	96, 140, 12, TEXT_LY, // æ
	112, 140, 7, TEXT_LY, // ç
	128, 140, 8, TEXT_LY, // è
	144, 140, 8, TEXT_LY, // é
	160, 140, 8, TEXT_LY, // ê
	176, 140, 8, TEXT_LY, // ë
	192, 140, 4, TEXT_LY, // ì
	208, 140, 4, TEXT_LY, // í
	224, 140, 7, TEXT_LY, // î
	240, 140, 6, TEXT_LY, // ï
	256, 140, 8, TEXT_LY, // ð
	272, 140, 8, TEXT_LY, // ñ
	288, 140, 8, TEXT_LY, // ò
	304, 140, 8, TEXT_LY, // ó
	320, 140, 8, TEXT_LY, // ô
	336, 140, 8, TEXT_LY, // õ
	352, 140, 8, TEXT_LY, // ö
	368, 140, 8, TEXT_LY, // ÷
	384, 140, 8, TEXT_LY, // ø
	400, 140, 8, TEXT_LY, // ù
	416, 140, 8, TEXT_LY, // ú
	432, 140, 8, TEXT_LY, // û
	448, 140, 8, TEXT_LY, // ü
	464, 140, 8, TEXT_LY, // ý
	480, 140, 8, TEXT_LY, // þ
	496, 140, 8, TEXT_LY, // ÿ
};


#endif
