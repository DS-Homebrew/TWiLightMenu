#ifndef GRAPHIC_H
#define GRAPHIC_H

#include <3ds.h>
#include "pp2d/pp2d.h"

// Textures
extern size_t homeicontex;
extern size_t topbgtex;
extern size_t subbgtex;
extern size_t logotex;
extern size_t buttontex;

// Colors
#define TRANSPARENT RGBA8(0, 0, 0, 0)

#define BLACK RGBA8(0, 0, 0, 255)
#define WHITE RGBA8(255, 255, 255, 255)
#define GRAY RGBA8(127, 127, 127, 255)
#define BLUE RGBA8(0, 0, 255, 255)
#define GREEN RGBA8(0, 255, 0, 255)
#define RED RGBA8(255, 0, 0, 255)

#define TIME RGBA8(16, 0, 0, 223)
#define DSSPLASH RGBA8(61, 161, 191, 255)

void pp2d_draw_texture_scale_blend(size_t id, int x, int y, float scaleX, float scaleY, u32 color);
void pp2d_draw_texture_part_blend(size_t id, int x, int y, int xbegin, int ybegin, int width, int height, u32 color);
void pp2d_draw_texture_part_scale(size_t id, int x, int y, int xbegin, int ybegin, int width, int height, float scaleX, float scaleY);
void pp2d_draw_texture_part_scale_blend(size_t id, int x, int y, int xbegin, int ybegin, int width, int height, float scaleX, float scaleY, u32 color);
void pp2d_draw_texture_rotate_flip_blend(size_t id, int x, int y, float angle, flipType fliptype, u32 color);
#endif // GRAPHIC_H