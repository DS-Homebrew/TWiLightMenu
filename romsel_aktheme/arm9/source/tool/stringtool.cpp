/*---------------------------------------------------------------------------------


Copyright (C) 2007 Acekard, www.acekard.com

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.


---------------------------------------------------------------------------------*/

#include "stringtool.h"
#include <cstdarg>
#include <cstdio>
#include <malloc.h>

std::string formatString( const char* fmt, ... )
{
    const char * f = fmt;
    va_list argList;
    va_start(argList, fmt);
    char * ptempStr = NULL;
    size_t max_len = vasiprintf( &ptempStr, f, argList);
    std::string str( ptempStr );
    str.resize( max_len );
    free( ptempStr );
    va_end(argList);
    return str;
}

std::string replaceAll(const std::string str, const std::string &from, const std::string &to)
{
	size_t start_pos = 0;
    std::string newStr = std::string(str);
	while ((start_pos = str.find(from, start_pos)) != std::string::npos)
	{
		newStr.replace(start_pos, from.length(), to);
		start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
	}
	return newStr;
}