#pragma once

#include "FontGraphic.h"

void fontInit();
void esrbDescFontInit(bool dsFont);
void esrbDescFontDeinit();

void updateText(bool top);
void updateTopTextArea(int x, int y, int width, int height, u16 *restoreBuf = NULL);
void updateTextImg(u16* img, bool top);
void clearText(bool top);
void clearText();

void printSmall(bool top, int x, int y, std::string_view message, Alignment align = Alignment::left, FontPalette palette = FontPalette::regular);
void printSmall(bool top, int x, int y, std::u16string_view message, Alignment align = Alignment::left, FontPalette palette = FontPalette::regular);
void printSmallMonospaced(bool top, int x, int y, std::string_view message, Alignment align = Alignment::left, FontPalette palette = FontPalette::regular);
void printSmallMonospaced(bool top, int x, int y, std::u16string_view message, Alignment align = Alignment::left, FontPalette palette = FontPalette::regular);
void printTiny(bool top, int x, int y, std::string_view message, Alignment align = Alignment::left, FontPalette palette = FontPalette::regular);
void printTiny(bool top, int x, int y, std::u16string_view message, Alignment align = Alignment::left, FontPalette palette = FontPalette::regular);
void printTinyMonospaced(bool top, int x, int y, std::string_view message, Alignment align = Alignment::left, FontPalette palette = FontPalette::regular);
void printTinyMonospaced(bool top, int x, int y, std::u16string_view message, Alignment align = Alignment::left, FontPalette palette = FontPalette::regular);

int calcSmallFontWidth(std::string_view text);
int calcSmallFontWidth(std::u16string_view text);
int calcTinyFontWidth(std::string_view text);
int calcTinyFontWidth(std::u16string_view text);

int calcSmallFontHeight(std::string_view text);
int calcSmallFontHeight(std::u16string_view text);
int calcTinyFontHeight(std::string_view text);
int calcTinyFontHeight(std::u16string_view text);

u8 smallFontHeight(void);
u8 tinyFontHeight(void);
