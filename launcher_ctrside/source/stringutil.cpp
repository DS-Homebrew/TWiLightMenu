/*  This file is part of Checkpoint
>	Copyright (C) 2017 Bernardo Giordano
>
>   This program is free software: you can redistribute it and/or modify
>   it under the terms of the GNU General Public License as published by
>   the Free Software Foundation, either version 3 of the License, or
>   (at your option) any later version.
>
>   This program is distributed in the hope that it will be useful,
>   but WITHOUT ANY WARRANTY; without even the implied warranty of
>   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
>   GNU General Public License for more details.
>
>   You should have received a copy of the GNU General Public License
>   along with this program.  If not, see <http://www.gnu.org/licenses/>.
>   See LICENSE for information.
*/

#include "stringutil.h"

std::u16string u8tou16(const char* src)
{
    char16_t tmp[256] = {0};
    utf8_to_utf16((uint16_t *)tmp, (uint8_t *)src, 256);
    return std::u16string(tmp);
}

std::string u16tou8(std::u16string src)
{
	static std::wstring_convert<std::codecvt_utf8_utf16<char16_t>,char16_t> convert;
	std::string dst = convert.to_bytes(src);
	return dst;
}

std::u16string removeForbiddenCharacters(std::u16string src)
{
	static const std::u16string illegalChars = u8tou16(".,!\\/:?*\"<>|");
	for (size_t i = 0; i < src.length(); i++)
	{
		if (illegalChars.find(src[i]) != std::string::npos)
		{
			src[i] = ' ';
		}
	}
	
	size_t i;
	for (i = src.length() - 1; i > 0 && src[i] == L' '; i--);
	src.erase(i + 1, src.length() - i);

	return src;
}