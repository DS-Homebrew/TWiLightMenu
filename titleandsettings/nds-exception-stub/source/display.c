/*---------------------------------------------------------------------------------

  Copyright (C) 2005 - 2017
  	Dave Murphy (WinterMute)

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any
  damages arising from the use of this software.

  Permission is granted to anyone to use this software for any
  purpose, including commercial applications, and to alter it and
  redistribute it freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you
     must not claim that you wrote the original software. If you use
     this software in a product, an acknowledgment in the product
     documentation would be appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and
     must not be misrepresented as being the original software.
  3. This notice may not be removed or altered from any source
     distribution.

---------------------------------------------------------------------------------*/
#include <nds.h>

#include "font_bin.h"

typedef struct con {
	int x,y;
} consoleVars;


static consoleVars topCON = {0,0} , btmCON = {0,0};

static consoleVars *currentCON = &topCON;


//---------------------------------------------------------------------------------
void setScreen(int screen) {
//---------------------------------------------------------------------------------
	currentCON = (screen == 0 ) ? &btmCON : &topCON;
}

//---------------------------------------------------------------------------------
// upcvt_4bit()
// Convert a 1-bit font to GBA 4-bit format.
//---------------------------------------------------------------------------------
void upcvt_4bit(void *dst, const u8 *src, u32 len) {
//---------------------------------------------------------------------------------
	u32 *out = dst;

	for(; len > 0; len--) {
		u32 dst_bits = 0;
		u32 src_bits = *src++;
		u32 x;

		for(x = 0; x < 8; x++) {
			dst_bits <<= 4;
			dst_bits |= src_bits & 1;
			src_bits >>= 1;
		}
	*out++ = dst_bits;
	}
}

//---------------------------------------------------------------------------------
static void newRow() {
//---------------------------------------------------------------------------------
	currentCON->y++;
	currentCON->x = 0;
	if (currentCON->y > 23) {
		u32 *src = (u32*)(BG_MAP_RAM(4) + 32);
		u32 *dst = (u32*)BG_MAP_RAM(4);
		memcpy(dst,src,64*24);
		currentCON->y = 23;
	}
}

//---------------------------------------------------------------------------------
static void writeChar(char c) {
//---------------------------------------------------------------------------------
	u16 *mapcell = BG_MAP_RAM(4) + currentCON->x + (currentCON->y * 32);
	switch(c) {

	case 10:
		newRow();
	case 13:
		currentCON->x = 0;
		break;
	default:
		*mapcell = c;
		currentCON->x++;
		break;
	}
	if (currentCON->x > 31) newRow();
}

//---------------------------------------------------------------------------------
static void writeString(char *str, int len) {
//---------------------------------------------------------------------------------
	while(len--) {
		writeChar(*str++);
	}
}

//---------------------------------------------------------------------------------
void setCursor(int row, int column) {
//---------------------------------------------------------------------------------
	currentCON->x = column;
	currentCON->y = row;
}

void getCursor(int *row, int*column) {
	*column = currentCON->x;
	*row = currentCON->y;
}

//---------------------------------------------------------------------------------
void initDisplay() {
//---------------------------------------------------------------------------------
	videoSetMode(MODE_0_2D | DISPLAY_BG0_ACTIVE);
	videoSetModeSub(MODE_0_2D | DISPLAY_BG0_ACTIVE);

	upcvt_4bit(BG_TILE_RAM(0),font_bin,font_bin_size);
	upcvt_4bit(BG_TILE_RAM_SUB(0),font_bin,font_bin_size);
	BGCTRL[0] = BG_MAP_BASE(4) | BG_TILE_BASE(0) | BG_COLOR_16 | BG_32x32;
	BGCTRL_SUB[0] = BG_MAP_BASE(4) | BG_TILE_BASE(0) | BG_COLOR_16 | BG_32x32;
	
	u16 *top = BG_MAP_RAM(4);
	u16 *btm = BG_MAP_RAM_SUB(4);
	int i;
	for ( i = 0; i < 32*32; i++ ) {
		*top++ = 0x20;
		*btm++ = 0x20;
	}
	BG_PALETTE[0]=RGB5(31,0,0);
	BG_PALETTE[1]=RGB5(31,31,31);
	BG_PALETTE_SUB[0]=RGB5(31,0,0);
	BG_PALETTE_SUB[1]=RGB5(31,31,31);

}

//---------------------------------------------------------------------------------
u32 vramDefault() {
//---------------------------------------------------------------------------------
	u32 tmp = VRAM_CR;
	vramSetBankA(VRAM_A_MAIN_BG);
	vramSetBankC(VRAM_C_SUB_BG);
	return tmp;
}

/*
 *  linux/lib/vsprintf.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

/* vsprintf.c -- Lars Wirzenius & Linus Torvalds. */
/*
 * Wirzenius wrote this portably, Torvalds fucked it up :-)
 */

#include <stdarg.h>
#include <string.h>
#include <unistd.h>

/* we use this so that we can do without the ctype library */
#define is_digit(c)	((c) >= '0' && (c) <= '9')

static int skip_atoi(const char **s)
{
	int i=0;

	while (is_digit(**s))
		i = i*10 + *((*s)++) - '0';
	return i;
}

#define ZEROPAD	1		/* pad with zero */
#define SIGN	2		/* unsigned/signed long */
#define PLUS	4		/* show plus */
#define SPACE	8		/* space if plus */
#define LEFT	16		/* left justified */
#define SPECIAL	32		/* 0x */
#define LARGE	64		/* use 'ABCDEF' instead of 'abcdef' */

#define do_div(n,base) ({ \
int __res; \
__res = ((unsigned long) n) % (unsigned) base; \
n = ((unsigned long) n) / (unsigned) base; \
__res; })

static char * number(char * str, long num, int base, int size, int precision
	,int type)
{
	char c,sign,tmp[66];
	const char *digits="0123456789abcdefghijklmnopqrstuvwxyz";
	int i;

	if (type & LARGE)
		digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	if (type & LEFT)
		type &= ~ZEROPAD;
	if (base < 2 || base > 36)
		return 0;
	c = (type & ZEROPAD) ? '0' : ' ';
	sign = 0;
	if (type & SIGN) {
		if (num < 0) {
			sign = '-';
			num = -num;
			size--;
		} else if (type & PLUS) {
			sign = '+';
			size--;
		} else if (type & SPACE) {
			sign = ' ';
			size--;
		}
	}
	if (type & SPECIAL) {
		if (base == 16)
			size -= 2;
		else if (base == 8)
			size--;
	}
	i = 0;
	if (num == 0)
		tmp[i++]='0';
	else while (num != 0)
		tmp[i++] = digits[do_div(num,base)];
	if (i > precision)
		precision = i;
	size -= precision;
	if (!(type&(ZEROPAD+LEFT)))
		while(size-->0)
			*str++ = ' ';
	if (sign)
		*str++ = sign;
	if (type & SPECIAL) {
		if (base==8)
			*str++ = '0';
		else if (base==16) {
			*str++ = '0';
			*str++ = digits[33];
		}
	}
	if (!(type & LEFT))
		while (size-- > 0)
			*str++ = c;
	while (i < precision--)
		*str++ = '0';
	while (i-- > 0)
		*str++ = tmp[i];
	while (size-- > 0)
		*str++ = ' ';
	return str;
}

int kvsprintf(char *buf, const char *fmt, va_list args)
{
	int len;
	unsigned long num;
	int i, base;
	char * str;
	const char *s;

	int flags;		/* flags to number() */

	int field_width;	/* width of output field */
	int precision;		/* min. # of digits for integers; max
				   number of chars for from string */
	int qualifier;		/* 'h', 'l', or 'L' for integer fields */

	for (str=buf ; *fmt ; ++fmt) {
		if (*fmt != '%') {
			*str++ = *fmt;
			continue;
		}

		/* process flags */
		flags = 0;
		repeat:
			++fmt;		/* this also skips first '%' */
			switch (*fmt) {
				case '-': flags |= LEFT; goto repeat;
				case '+': flags |= PLUS; goto repeat;
				case ' ': flags |= SPACE; goto repeat;
				case '#': flags |= SPECIAL; goto repeat;
				case '0': flags |= ZEROPAD; goto repeat;
				}

		/* get field width */
		field_width = -1;
		if (is_digit(*fmt))
			field_width = skip_atoi(&fmt);
		else if (*fmt == '*') {
			++fmt;
			/* it's the next argument */
			field_width = va_arg(args, int);
			if (field_width < 0) {
				field_width = -field_width;
				flags |= LEFT;
			}
		}

		/* get the precision */
		precision = -1;
		if (*fmt == '.') {
			++fmt;
			if (is_digit(*fmt))
				precision = skip_atoi(&fmt);
			else if (*fmt == '*') {
				++fmt;
				/* it's the next argument */
				precision = va_arg(args, int);
			}
			if (precision < 0)
				precision = 0;
		}

		/* get the conversion qualifier */
		qualifier = -1;
		if (*fmt == 'h' || *fmt == 'l' || *fmt == 'L') {
			qualifier = *fmt;
			++fmt;
		}

		/* default base */
		base = 10;

		switch (*fmt) {
		case 'c':
			if (!(flags & LEFT))
				while (--field_width > 0)
					*str++ = ' ';
			*str++ = (unsigned char) va_arg(args, int);
			while (--field_width > 0)
				*str++ = ' ';
			continue;

		case 's':
			s = va_arg(args, char *);
			if (!s)
				s = "<NULL>";

			len = strnlen(s, precision);

			if (!(flags & LEFT))
				while (len < field_width--)
					*str++ = ' ';
			for (i = 0; i < len; ++i)
				*str++ = *s++;
			while (len < field_width--)
				*str++ = ' ';
			continue;

		case 'p':
			if (field_width == -1) {
				field_width = 2*sizeof(void *);
				flags |= ZEROPAD;
			}
			str = number(str,
				(unsigned long) va_arg(args, void *), 16,
				field_width, precision, flags);
			continue;


		case 'n':
			if (qualifier == 'l') {
				long * ip = va_arg(args, long *);
				*ip = (str - buf);
			} else {
				int * ip = va_arg(args, int *);
				*ip = (str - buf);
			}
			continue;

		case '%':
			*str++ = '%';
			continue;

		/* integer number formats - set up the flags and "break" */
		case 'o':
			base = 8;
			break;

		case 'X':
			flags |= LARGE;
		case 'x':
			base = 16;
			break;

		case 'd':
		case 'i':
			flags |= SIGN;
		case 'u':
			break;

		default:
			*str++ = '%';
			if (*fmt)
				*str++ = *fmt;
			else
				--fmt;
			continue;
		}
		if (qualifier == 'l')
			num = va_arg(args, unsigned long);
		else if (qualifier == 'h') {
			num = (unsigned short) va_arg(args, int);
			if (flags & SIGN)
				num = (short) num;
		} else if (flags & SIGN)
			num = va_arg(args, int);
		else
			num = va_arg(args, unsigned int);
		str = number(str, num, base, field_width, precision, flags);
	}
	*str = '\0';
	return str-buf;
}

#define __DOUTBUFSIZE 256

char __outstr[__DOUTBUFSIZE];

//---------------------------------------------------------------------------------
void kprintf(const char *str, ...) {
//---------------------------------------------------------------------------------

	int len;

	va_list args;

	va_start(args, str);
	len=kvsprintf(__outstr,str,args);
	va_end(args);

	writeString(__outstr, len);
}

