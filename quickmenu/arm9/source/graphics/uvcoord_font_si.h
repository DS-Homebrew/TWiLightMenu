/*-----------------------------------------------------------------
 Copyright (C) 2015
	Matthew Scholefield

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

------------------------------------------------------------------*/

#ifndef FONT_SI__H
#define FONT_SI__H


#define FONT_SI_BITMAP_WIDTH   512
#define FONT_SI_BITMAP_HEIGHT  32
#define FONT_SI_NUM_IMAGES   32

#define S_TEXT_SYS 10

//	Format:
//	U,V,Width,Height

extern const unsigned int font_si_texcoords[] = {
	0, 0, 3, S_TEXT_SYS, // Space
	4, 0, 3, S_TEXT_SYS, // !
	6, 0, 4, S_TEXT_SYS, // "
	10, 0, 9, S_TEXT_SYS, // #
	18, 0, 6, S_TEXT_SYS, // $
	24, 0, 8, S_TEXT_SYS, // %
	32, 0, 7, S_TEXT_SYS, // &
	39, 0, 3, S_TEXT_SYS, // '
	42, 0, 4, S_TEXT_SYS, // (
	46, 0, 4, S_TEXT_SYS, // )
	50, 0, 9, S_TEXT_SYS, // *
	59, 0, 6, S_TEXT_SYS, // +
	65, 0, 4, S_TEXT_SYS, // ,
	69, 0, 6, S_TEXT_SYS, // -
	75, 0, 4, S_TEXT_SYS, // .
	79, 0, 6, S_TEXT_SYS, // /
	85, 0, 6, S_TEXT_SYS, // 0
	91, 0, 4, S_TEXT_SYS, // 1
	95, 0, 6, S_TEXT_SYS, // 2
	101, 0, 6, S_TEXT_SYS, // 3
	107, 0, 7, S_TEXT_SYS, // 4
	// Unchanged from this point
	40, 0, 8, S_TEXT_SYS, // 5
	48, 0, 8, S_TEXT_SYS, // 6
	56, 0, 8, S_TEXT_SYS, // 7
	0, 0, 8, S_TEXT_SYS, // 8
	8, 0, 8, S_TEXT_SYS, // 9
	16, 24, 4, S_TEXT_SYS, // :
	24, 24, 4, S_TEXT_SYS, // ;
	32, 24, 6, S_TEXT_SYS, // <
	40, 24, 8, S_TEXT_SYS, // =
	48, 24, 6, S_TEXT_SYS, // >
	56, 24, 6, S_TEXT_SYS, // ?
	0, 32, 8, S_TEXT_SYS, // @
	8, 32, 8, S_TEXT_SYS, // A
	16, 32, 8, S_TEXT_SYS, // B
	24, 32, 7, S_TEXT_SYS, // C
	32, 32, 8, S_TEXT_SYS, // D
	40, 32, 7, S_TEXT_SYS, // E
	48, 32, 7, S_TEXT_SYS, // F
	56, 32, 8, S_TEXT_SYS, // G
	0, 40, 8, S_TEXT_SYS, // H
	8, 40, 7, S_TEXT_SYS, // I
	16, 40, 7, S_TEXT_SYS, // J
	24, 40, 8, S_TEXT_SYS, // K
	32, 40, 7, S_TEXT_SYS, // L
	40, 40, 8, S_TEXT_SYS, // M
	48, 40, 8, S_TEXT_SYS, // N
	56, 40, 8, S_TEXT_SYS, // O
	0, 48, 7, S_TEXT_SYS, // P
	8, 48, 8, S_TEXT_SYS, // Q
	16, 48, 7, S_TEXT_SYS, // R
	24, 48, 8, S_TEXT_SYS, // S
	32, 48, 7, S_TEXT_SYS, // T
	40, 48, 8, S_TEXT_SYS, // U
	48, 48, 8, S_TEXT_SYS, // V
	56, 48, 8, S_TEXT_SYS, // W
	0, 56, 8, S_TEXT_SYS, // X
	8, 56, 7, S_TEXT_SYS, // Y
	16, 56, 8, S_TEXT_SYS, // Z
	24, 56, 6, S_TEXT_SYS, // [
	32, 56, 8, S_TEXT_SYS, // \ And,
	40, 56, 6, S_TEXT_SYS, // ]
	48, 56, 5, S_TEXT_SYS, // ^
	56, 56, 7, S_TEXT_SYS, // _
	0, 64, 4, S_TEXT_SYS, // `
	8, 64, 8, S_TEXT_SYS, // a
	16, 64, 8, S_TEXT_SYS, // b
	24, 64, 8, S_TEXT_SYS, // c
	32, 64, 7, S_TEXT_SYS, // d
	40, 64, 8, S_TEXT_SYS, // e
	48, 64, 8, S_TEXT_SYS, // f
	56, 64, 8, S_TEXT_SYS, // g
	0, 72, 8, S_TEXT_SYS, // h
	8, 72, 3, S_TEXT_SYS, // i
	16, 72, 6, S_TEXT_SYS, // j
	24, 72, 7, S_TEXT_SYS, // k
	32, 72, 8, S_TEXT_SYS, // l
	40, 72, 8, S_TEXT_SYS, // m
	48, 72, 8, S_TEXT_SYS, // n
	56, 72, 8, S_TEXT_SYS, // o
	0, 80, 7, S_TEXT_SYS, // p
	8, 80, 7, S_TEXT_SYS, // q
	16, 80, 7, S_TEXT_SYS, // r
	24, 80, 8, S_TEXT_SYS, // s
	32, 80, 8, S_TEXT_SYS, // t
	40, 80, 7, S_TEXT_SYS, // u
	48, 80, 7, S_TEXT_SYS, // v
	56, 80, 8, S_TEXT_SYS, // w
	0, 88, 7, S_TEXT_SYS, // x
	8, 88, 8, S_TEXT_SYS, // y
	16, 88, 8, S_TEXT_SYS, // z
	24, 88, 6, S_TEXT_SYS, // {
	32, 88, 3, S_TEXT_SYS, // |
	40, 88, 6, S_TEXT_SYS, // }
	48, 88, 8, S_TEXT_SYS, // ~
	56, 88, 8, S_TEXT_SYS,
	0, 96, 8, S_TEXT_SYS,
	8, 96, 8, S_TEXT_SYS,
	16, 96, 8, S_TEXT_SYS,
	24, 96, 8, S_TEXT_SYS,
	32, 96, 8, S_TEXT_SYS,
	40, 96, 8, S_TEXT_SYS,
	48, 96, 8, S_TEXT_SYS,
	56, 96, 8, S_TEXT_SYS,
	0, 104, 8, S_TEXT_SYS,
	8, 104, 8, S_TEXT_SYS,
	16, 104, 8, S_TEXT_SYS,
	24, 104, 8, S_TEXT_SYS,
	32, 104, 8, S_TEXT_SYS,
	40, 104, 8, S_TEXT_SYS,
	48, 104, 8, S_TEXT_SYS,
	56, 104, 8, S_TEXT_SYS,
	0, 112, 8, S_TEXT_SYS,
	8, 112, 8, S_TEXT_SYS,
	16, 112, 8, S_TEXT_SYS,
	24, 112, 8, S_TEXT_SYS,
	32, 112, 8, S_TEXT_SYS,
	40, 112, 8, S_TEXT_SYS,
	48, 112, 8, S_TEXT_SYS,
	56, 112, 8, S_TEXT_SYS,
	0, 120, 8, S_TEXT_SYS,
	8, 120, 8, S_TEXT_SYS,
	16, 120, 8, S_TEXT_SYS,
	24, 120, 8, S_TEXT_SYS,
	32, 120, 8, S_TEXT_SYS,
	40, 120, 8, S_TEXT_SYS,
	48, 120, 8, S_TEXT_SYS,
	56, 120, 8, S_TEXT_SYS,
};


#endif
