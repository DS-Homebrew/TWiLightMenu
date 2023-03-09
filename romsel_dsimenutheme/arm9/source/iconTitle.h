/*-----------------------------------------------------------------
 Copyright (C) 2005 - 2013
	Michael "Chishm" Chisholm
	Dave "WinterMute" Murphy
	Claudio "sverx"

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
#pragma once

#include <string_view>

void getGameInfo(bool isDir, const char* name, int num, bool fromArgv = false);
void iconUpdate(bool isDir, const char* name, int num);
void titleUpdate(bool isDir, std::string_view name, int num);
void drawRomIcon(int Xpos, int Ypos, int num, int romType);
void drawIcon(int Xpos, int Ypos, int num);
void drawIconGBA(int Xpos, int Ypos);
void drawSmallIconGBA(int Xpos, int Ypos);
void drawIconGB(int Xpos, int Ypos);
void drawIconGBC(int Xpos, int Ypos);
void drawIconNES(int Xpos, int Ypos);
void drawIconSG(int Xpos, int Ypos);
void drawIconSMS(int Xpos, int Ypos);
void drawIconGG(int Xpos, int Ypos);
void drawIconMD(int Xpos, int Ypos);
void drawIconSNES(int Xpos, int Ypos);
void drawIconPLG(int Xpos, int Ypos);
void drawIconA26(int Xpos, int Ypos);
void drawIconCOL(int Xpos, int Ypos);
void drawIconM5(int Xpos, int Ypos);
void drawIconINT(int Xpos, int Ypos);
void drawIconPCE(int Xpos, int Ypos);
void drawIconWS(int Xpos, int Ypos);
void drawIconNGP(int Xpos, int Ypos);
void drawIconCPC(int Xpos, int Ypos);
void drawIconVID(int Xpos, int Ypos);
void drawIconIMG(int Xpos, int Ypos);
void execDeferredIconUpdates();
void writeBannerText(std::string_view text);
void writeBannerText(std::u16string text);
