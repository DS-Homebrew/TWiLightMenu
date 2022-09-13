/*
    dbgtool.h
    Copyright (C) 2007 Acekard, www.acekard.com
    Copyright (C) 2007-2009 somebody
    Copyright (C) 2009 yellow wood goblin

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _DBG_TOOL_H_
#define _DBG_TOOL_H_

#include <nds.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

// #define DEBUG

#ifdef DEBUG
static inline int dbg_printf( const char* format, ... ) {
	va_list args;
	va_start( args, format );
	int ret = vprintf( format, args );
	va_end(args);
	return ret;
}
#else
#define dbg_printf(...)
#define nocashMessage(...)
#endif//DEBUG

#ifdef DEBUG
static inline void _cwl(const char *file,int line) {
	const char *seek=file;

	while (*seek != 0) {
		if (*seek == '/') file = seek;
		seek++;
	}
	dbg_printf("%s(%d)\n",file,line);
}
#define cwl(); _cwl( __FILE__, __LINE__ );
#else
#define cwl()
#endif//DEBUG

#ifdef DEBUG
static inline void wait_press_b() {
	dbg_printf("\npress B to continue.\n");
	scanKeys();
	u16 keys_up = 0;
	while (0 == (keys_up & KEY_B)) {
		scanKeys();
		keys_up = keysUp();
	}
}
#else
#define wait_press_b()
#endif//DEBUG


#endif//_DBG_TOOL_H_
