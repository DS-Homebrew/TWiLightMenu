#include "FontGraphic.h"

#pragma once

void fontInit();

void updateText(bool top);
void clearText(bool top);
void clearText();

void printSmall(bool top, int x, int y, std::string_view message, Alignment align = Alignment::left);
void printLarge(bool top, int x, int y, std::string_view message, Alignment align = Alignment::left);

int calcSmallFontWidth(std::string_view text);
int calcLargeFontWidth(std::string_view text);

u8 smallFontHeight(void);
u8 largeFontHeight(void);
