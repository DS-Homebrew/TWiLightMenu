#pragma once
#ifndef _COLOR_H_
#define _COLOR_H_

#include <nds.h>
#include <algorithm>
using std::min;
using std::max;

inline u16 grayscale(u16 val) {
	u16 newVal = ((val >> 10) & 31) | (val & 31 << 5) | (val & 31) << 10 | BIT(15);

	u8 b, g, r, max, min;
	b = ((newVal) >> 10) & 31;
	g = ((newVal) >> 5) & 31;
	r = (newVal)&31;
	// Value decomposition of hsv
	max = (b > g) ? b : g;
	max = (max > r) ? max : r;

	// Desaturate
	min = (b < g) ? b : g;
	min = (min < r) ? min : r;
	max = (max + min) / 2;

	newVal = 32768 | (max << 10) | (max << 5) | (max);

	b = ((newVal) >> 10) & (31);
	g = ((newVal) >> 5) & (31);
	r = (newVal)&31;

	return 32768 | (b << 10) | (g << 5) | (r);
}


/**
 * Adapted from https://stackoverflow.com/questions/18937701/
 * applies alphablending with the given
 * RGB555 foreground, RGB555 background, and alpha from
 * 0 to 255 (0, 1.0).
 * The lower the alpha the more transparent, but
 * this function does not produce good results when blending 
 * less than 128 (50%) alpha due to overflow.
 */
inline u16 alphablend(const u16 fg, const u16 bg, const u8 alpha) {
	u16 fg_b, fg_g, fg_r, bg_b, bg_g, bg_r;
	fg_b = ((fg) >> 10) & 31;
	fg_g = ((fg) >> 5) & 31;
	fg_r = (fg)&31;

	bg_b = ((bg) >> 10) & 31;
	bg_g = ((bg) >> 5) & 31;
	bg_r = (bg)&31;

	// Alpha blend components
	u16 out_r = fg_r * alpha + bg_r * (255 - alpha);
	u16 out_g = fg_g * alpha + bg_g * (255 - alpha);
	u16 out_b = fg_b * alpha + bg_b * (255 - alpha);
	out_r = min((out_r + 1 + (out_r >> 8)) >> 8, 255);
	out_g = min((out_g + 1 + (out_g >> 8)) >> 8, 255);
	out_b = min((out_b + 1 + (out_b >> 8)) >> 8, 255);

	return (u16) (BIT(15) | (out_b << 10) | (out_g << 5) | out_r);
}



/**
 * Adapted from https://stackoverflow.com/questions/18937701/
 * applies alphablending with the given
 * RGB555 foreground, RGB555 background, and alpha from
 * 0 to 255 (0, 1.0).
 */
inline u16 blend(const u16 fg, const u16 bg) {

	u8 fg_b, fg_g, fg_r, bg_b, bg_g, bg_r;
	fg_b = ((fg) >> 10) & 31;
	fg_g = ((fg) >> 5) & 31;
	fg_r = (fg)&31;

	bg_b = ((bg) >> 10) & 31;
	bg_g = ((bg) >> 5) & 31;
	bg_r = (bg)&31;


	// Alpha blend components
	unsigned out_r = min((fg_r + bg_r) >> 1, 255);
	unsigned out_g = min((fg_g + bg_g) >> 1, 255);
	unsigned out_b = min((fg_b + bg_b) >> 1, 255);
	return (u16) ((out_b << 10) | (out_g << 5) | out_r | BIT(15));
}


#endif