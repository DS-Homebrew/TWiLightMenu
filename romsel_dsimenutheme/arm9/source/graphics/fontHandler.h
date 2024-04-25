#pragma once

#include "FontGraphic.h"

void fontInit();
void esrbDescFontInit(bool dsFont);
void esrbDescFontDeinit();

void updateText(bool top);
void updateTextImg(u16* img, bool top);
void clearText(bool top);
void clearText();

void printSmall(bool top, int x, int y, std::string_view message, Alignment align = Alignment::left, FontPalette palette = FontPalette::regular);
void printSmall(bool top, int x, int y, std::u16string_view message, Alignment align = Alignment::left, FontPalette palette = FontPalette::regular);
void printLarge(bool top, int x, int y, std::string_view message, Alignment align = Alignment::left, FontPalette palette = FontPalette::regular);
void printLarge(bool top, int x, int y, std::u16string_view message, Alignment align = Alignment::left, FontPalette palette = FontPalette::regular);

int calcSmallFontWidth(std::string_view text);
int calcSmallFontWidth(std::u16string_view text);
int calcLargeFontWidth(std::string_view text);
int calcLargeFontWidth(std::u16string_view text);

int calcSmallFontHeight(std::string_view text);
int calcSmallFontHeight(std::u16string_view text);
int calcLargeFontHeight(std::string_view text);
int calcLargeFontHeight(std::u16string_view text);

u8 smallFontHeight(void);
u8 largeFontHeight(void);
