#pragma once
#ifndef _COLOR_H_
#define _COLOR_H_

#include <nds.h>
#include <algorithm>
using std::min;
using std::max;

u16 rgb8ToRgb565(const u8 r, const u8 g, const u8 b) {
	const u16 green = (g >> 2) << 5;
	u16 color = r >> 3 | (b >> 3) << 10;
	if (green & BIT(5)) {
		color |= BIT(15);
	}
	for (int gBit = 6; gBit <= 10; gBit++) {
		if (green & BIT(gBit)) {
			color |= BIT(gBit-1);
		}
	}
	return color;
}


/**
 * Adapted from https://stackoverflow.com/questions/18937701/
 * applies alphablending with the given
 * RGB565 foreground, RGB565 background, and alpha from
 * 0 to 255 (0, 1.0).
 * The lower the alpha the more transparent, but
 * this function does not produce good results when blending 
 * less than 128 (50%) alpha due to overflow.
 */
u16 rgb8ToRgb565_alphablend(const u8 fg_r, const u8 fg_g, const u8 fg_b, const u8 bg_r, const u8 bg_g, const u8 bg_b, const u8 alpha, const u8 alphaG) {

  //  Alpha blend components
    u16 out_r = fg_r * alpha + bg_r * (255 - alpha);
    u16 out_g = fg_g * alphaG + bg_g * (255 - alphaG);
    u16 out_b = fg_b * alpha + bg_b * (255 - alpha);
    out_r = min((out_r + 1 + (out_r >> 8)) >> 8, 255);
    out_g = min((out_g + 1 + (out_g >> 8)) >> 8, 255);
    out_b = min((out_b + 1 + (out_b >> 8)) >> 8, 255);
		
	const u16 green = (out_g >> 2) << 5;
	u16 color = out_r >> 3 | (out_b >> 3) << 10;
	if (green & BIT(5)) {
		color |= BIT(15);
	}
	for (int gBit = 6; gBit <= 10; gBit++) {
		if (green & BIT(gBit)) {
			color |= BIT(gBit-1);
		}
	}
	return color;
}


#endif