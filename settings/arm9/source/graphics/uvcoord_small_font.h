/*======================================================================

   small_font texture coordinates

======================================================================*/


#ifndef SMALL_FONT__H
#define SMALL_FONT__H


#define SMALL_FONT_BITMAP_WIDTH   512
#define SMALL_FONT_BITMAP_HEIGHT  128
#define SMALL_FONT_NUM_IMAGES   0xF0

#define TEXT_SY 16
		//	Format:
		//	U,V,Width,Height



extern const unsigned int small_font_texcoords[] = {
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
	0, 16, 4, TEXT_SY, // Space
	16, 16, 2, TEXT_SY, // !
	32, 16, 5, TEXT_SY, // "
	48, 16, 9, TEXT_SY, // #
	64, 16, 8, TEXT_SY, // $
	80, 16, 12, TEXT_SY, // %
	96, 16, 10, TEXT_SY, // &
	112, 16, 2, TEXT_SY, // '
	128, 16, 4, TEXT_SY, // (
	144, 16, 4, TEXT_SY, // )
	160, 16, 8, TEXT_SY, // *
	176, 16, 8, TEXT_SY, // +
	192, 16, 3, TEXT_SY, // ,
	208, 16, 5, TEXT_SY, // -
	224, 16, 3, TEXT_SY, // .
	240, 16, 5, TEXT_SY, // /
	256, 16, 8, TEXT_SY, // 0
	272, 16, 4, TEXT_SY, // 1
	288, 16, 8, TEXT_SY, // 2
	304, 16, 8, TEXT_SY, // 3
	320, 16, 9, TEXT_SY, // 4
	336, 16, 8, TEXT_SY, // 5
	352, 16, 8, TEXT_SY, // 6
	368, 16, 8, TEXT_SY, // 7
	384, 16, 8, TEXT_SY, // 8
	400, 16, 8, TEXT_SY, // 9
	416, 16, 2, TEXT_SY, // :
	432, 16, 2, TEXT_SY, // ;
	448, 16, 7, TEXT_SY, // <
	464, 16, 8, TEXT_SY, // =
	480, 16, 7, TEXT_SY, // >
	496, 16, 8, TEXT_SY, // ?
	0, 32, 12, TEXT_SY, // @
	16, 32, 9, TEXT_SY, // A
	32, 32, 8, TEXT_SY, // B
	48, 32, 8, TEXT_SY, // C
	64, 32, 9, TEXT_SY, // D
	80, 32, 8, TEXT_SY, // E
	96, 32, 7, TEXT_SY, // F
	112, 32, 9, TEXT_SY, // G
	128, 32, 8, TEXT_SY, // H
	144, 32, 3, TEXT_SY, // I
	160, 32, 5, TEXT_SY, // J
	176, 32, 8, TEXT_SY, // K
	192, 32, 7, TEXT_SY, // L
	208, 32, 10, TEXT_SY, // M
	224, 32, 8, TEXT_SY, // N
	240, 32, 9, TEXT_SY, // O
	256, 32, 8, TEXT_SY, // P
	272, 32, 9, TEXT_SY, // Q
	288, 32, 8, TEXT_SY, // R
	304, 32, 7, TEXT_SY, // S
	320, 32, 8, TEXT_SY, // T
	336, 32, 8, TEXT_SY, // U
	352, 32, 9, TEXT_SY, // V
	368, 32, 12, TEXT_SY, // W
	384, 32, 8, TEXT_SY, // X
	400, 32, 8, TEXT_SY, // Y
	416, 32, 7, TEXT_SY, // Z
	432, 32, 5, TEXT_SY, // [
	448, 32, 6, TEXT_SY, // "\"
	464, 32, 5, TEXT_SY, // ]
	480, 32, 6, TEXT_SY, // ^
	496, 32, 7, TEXT_SY, // _
	0, 48, 4, TEXT_SY, // `
	16, 48, 7, TEXT_SY, // a
	32, 48, 7, TEXT_SY, // b
	48, 48, 6, TEXT_SY, // c
	64, 48, 7, TEXT_SY, // d
	80, 48, 7, TEXT_SY, // e
	96, 48, 5, TEXT_SY, // f
	112, 48, 7, TEXT_SY, // g
	128, 48, 7, TEXT_SY, // h
	144, 48, 2, TEXT_SY, // i
	160, 48, 5, TEXT_SY, // j
	176, 48, 6, TEXT_SY, // k
	192, 48, 2, TEXT_SY, // l
	208, 48, 10, TEXT_SY, // m
	224, 48, 7, TEXT_SY, // n
	240, 48, 7, TEXT_SY, // o
	256, 48, 7, TEXT_SY, // p
	272, 48, 7, TEXT_SY, // q
	288, 48, 4, TEXT_SY, // r
	304, 48, 6, TEXT_SY, // s
	320, 48, 5, TEXT_SY, // t
	336, 48, 7, TEXT_SY, // u
	352, 48, 7, TEXT_SY, // v
	368, 48, 10, TEXT_SY, // w
	384, 48, 7, TEXT_SY, // x
	400, 48, 6, TEXT_SY, // y
	416, 48, 6, TEXT_SY, // z
	432, 48, 4, TEXT_SY, // {
	448, 48, 3, TEXT_SY, // |
	464, 48, 4, TEXT_SY, // }
	480, 48, 8, TEXT_SY, // ~
	496, 48, 0, TEXT_SY, // NULL
	0, 64, 9, TEXT_SY, // €
	16, 64, 0, TEXT_SY, // NULL
	32, 64, 2, TEXT_SY, // ‚
	48, 64, 7, TEXT_SY, // ƒ
	64, 64, 5, TEXT_SY, // „
	80, 64, 13, TEXT_SY, // …
	96, 64, 6, TEXT_SY, // †
	112, 64, 6, TEXT_SY, // ‡
	128, 64, 5, TEXT_SY, // ˆ
	144, 64, 12, TEXT_SY, // ‰
	160, 64, 8, TEXT_SY, // Š
	176, 64, 5, TEXT_SY, // ‹
	192, 64, 11, TEXT_SY, // Œ
	208, 64, 0, TEXT_SY, // NULL
	224, 64, 8, TEXT_SY, // Ž
	240, 64, 0, TEXT_SY, // NULL
	256, 64, 0, TEXT_SY, // NULL
	272, 64, 2, TEXT_SY, // ‘
	288, 64, 2, TEXT_SY, // ’
	304, 64, 5, TEXT_SY, // “
	320, 64, 5, TEXT_SY, // ”
	336, 64, 3, TEXT_SY, // •
	352, 64, 7, TEXT_SY, // –
	368, 64, 14, TEXT_SY, // —
	384, 64, 5, TEXT_SY, // ˜
	400, 64, 11, TEXT_SY, // ™
	416, 64, 6, TEXT_SY, // š
	432, 64, 5, TEXT_SY, // ›
	448, 64, 12, TEXT_SY, // œ
	464, 64, 0, TEXT_SY, // NULL
	480, 64, 6, TEXT_SY, // ž
	496, 64, 8, TEXT_SY, // Ÿ
	0, 80, 4, TEXT_SY, // Space
	16, 80, 2, TEXT_SY, // ¡
	32, 80, 6, TEXT_SY, // ¢
	48, 80, 8, TEXT_SY, // £
	64, 80, 6, TEXT_SY, // ¤
	80, 80, 8, TEXT_SY, // ¥
	96, 80, 3, TEXT_SY, // ¦
	112, 80, 7, TEXT_SY, // §
	128, 80, 5, TEXT_SY, // ¨
	144, 80, 9, TEXT_SY, // ©
	160, 80, 4, TEXT_SY, // ª
	176, 80, 7, TEXT_SY, // «
	192, 80, 7, TEXT_SY, // ¬
	208, 80, 0, TEXT_SY, // NULL
	224, 80, 9, TEXT_SY, // ®
	240, 80, 5, TEXT_SY, // ¯
	256, 80, 4, TEXT_SY, // °
	272, 80, 7, TEXT_SY, // ±
	288, 80, 3, TEXT_SY, // ²
	304, 80, 4, TEXT_SY, // ³
	320, 80, 3, TEXT_SY, // ´
	336, 80, 7, TEXT_SY, // µ
	352, 80, 7, TEXT_SY, // ¶
	368, 80, 3, TEXT_SY, // ·
	384, 80, 3, TEXT_SY, // ¸
	400, 80, 3, TEXT_SY, // ¹
	416, 80, 3, TEXT_SY, // º
	432, 80, 7, TEXT_SY, // »
	448, 80, 10, TEXT_SY, // ¼
	464, 80, 10, TEXT_SY, // ½
	480, 80, 10, TEXT_SY, // ¾
	496, 80, 8, TEXT_SY, // ¿
	0, 96, 9, TEXT_SY, // À
	16, 96, 9, TEXT_SY, // Á
	32, 96, 9, TEXT_SY, // Â
	48, 96, 9, TEXT_SY, // Ã
	64, 96, 9, TEXT_SY, // Ä
	80, 96, 9, TEXT_SY, // Å
	96, 96, 11, TEXT_SY, // Æ
	112, 96, 8, TEXT_SY, // Ç
	128, 96, 8, TEXT_SY, // È
	144, 96, 8, TEXT_SY, // É
	160, 96, 8, TEXT_SY, // Ê
	176, 96, 8, TEXT_SY, // Ë
	192, 96, 3, TEXT_SY, // Ì
	208, 96, 3, TEXT_SY, // Í
	224, 96, 6, TEXT_SY, // Î
	240, 96, 6, TEXT_SY, // Ï
	256, 96, 9, TEXT_SY, // Ð
	272, 96, 9, TEXT_SY, // Ñ
	288, 96, 9, TEXT_SY, // Ò
	304, 96, 9, TEXT_SY, // Ó
	320, 96, 9, TEXT_SY, // Ô
	336, 96, 9, TEXT_SY, // Õ
	352, 96, 9, TEXT_SY, // Ö
	368, 96, 7, TEXT_SY, // ×
	384, 96, 9, TEXT_SY, // Ø
	400, 96, 9, TEXT_SY, // Ù
	416, 96, 9, TEXT_SY, // Ú
	432, 96, 9, TEXT_SY, // Û
	448, 96, 9, TEXT_SY, // Ü
	464, 96, 8, TEXT_SY, // Ý
	480, 96, 8, TEXT_SY, // Þ
	496, 96, 8, TEXT_SY, // ß
	0, 112, 7, TEXT_SY, // à
	16, 112, 7, TEXT_SY, // á
	32, 112, 7, TEXT_SY, // â
	48, 112, 7, TEXT_SY, // ã
	64, 112, 7, TEXT_SY, // ä
	80, 112, 7, TEXT_SY, // å
	96, 112, 10, TEXT_SY, // æ
	112, 112, 6, TEXT_SY, // ç
	128, 112, 7, TEXT_SY, // è
	144, 112, 7, TEXT_SY, // é
	160, 112, 7, TEXT_SY, // ê
	176, 112, 7, TEXT_SY, // ë
	192, 112, 3, TEXT_SY, // ì
	208, 112, 3, TEXT_SY, // í
	224, 112, 6, TEXT_SY, // î
	240, 112, 5, TEXT_SY, // ï
	256, 112, 7, TEXT_SY, // ð
	272, 112, 7, TEXT_SY, // ñ
	288, 112, 7, TEXT_SY, // ò
	304, 112, 7, TEXT_SY, // ó
	320, 112, 7, TEXT_SY, // ô
	336, 112, 7, TEXT_SY, // õ
	352, 112, 7, TEXT_SY, // ö
	368, 112, 7, TEXT_SY, // ÷
	384, 112, 7, TEXT_SY, // ø
	400, 112, 7, TEXT_SY, // ù
	416, 112, 7, TEXT_SY, // ú
	432, 112, 7, TEXT_SY, // û
	448, 112, 7, TEXT_SY, // ü
	464, 112, 6, TEXT_SY, // ý
	480, 112, 7, TEXT_SY, // þ
	496, 112, 6, TEXT_SY, // ÿ
};


#endif
