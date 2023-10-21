#include <nds/ndstypes.h>
#include <algorithm>
#include <cmath>

// based on https://gist.github.com/profi200/bfa7be60b3eecb8c43f59000f626c743

#define TARGET_GAMMA  (2.f)
#define DISPLAY_GAMMA (2.f)
#define DARKEN_SCREEN (0.5f)
#define LUMINANCE     (0.93f) // 0.99f for mGBA.

static constexpr u32 rgb8ToRgb5(u32 value8)
{
    u32 value5 = (value8 * 63 + 255) / (255 * 2);
    if (value5 > 31)
        return 31;
    return value5;
}

static constexpr u32 rgb8ToRgb6(u32 value8)
{
    return (value8 * 63 + 128) / 255;
}

u16 convertDSColorToPhat(const u16 i)
{
    double r = (double)(i & 0x1F) / 31;
    double g = (double)((i >> 5) & 0x1F) / 31;
    double b = (double)((i >> 10) & 0x1F) / 31;

    // Convert to linear gamma.
    r = std::pow(r, TARGET_GAMMA + DARKEN_SCREEN);
    g = std::pow(g, TARGET_GAMMA + DARKEN_SCREEN);
    b = std::pow(b, TARGET_GAMMA + DARKEN_SCREEN);

    // Luminance.
    r = std::clamp(r * LUMINANCE, 0.0, 1.0);
    g = std::clamp(g * LUMINANCE, 0.0, 1.0);
    b = std::clamp(b * LUMINANCE, 0.0, 1.0);

    /*
    *               Input
    *                [r]
    *                [g]
    *                [b]
    *
    * Correction    Output
    * [ r][gr][br]   [r]
    * [rg][ g][bg]   [g]
    * [rb][gb][ b]   [b]
    */
#if 1
    // libretro.
    // 0.800, 0.275, -0.075,
    // 0.135, 0.640,  0.225,
    // 0.195, 0.155,  0.650
    double newR = 0.8 * r + 0.275 * g + -0.075 * b;
    double newG = 0.135 * r + 0.64 * g + 0.225 * b;
    double newB = 0.195 * r + 0.155 * g + 0.65 * b;
    // Assuming no alpha channel in original calculation.
#else
    // mGBA.
    // 0.84, 0.18, 0.00,
    // 0.09, 0.67, 0.26,
    // 0.15, 0.10, 0.73
    double newR = 0.84 * r + 0.18 * g + 0.0 * b;
    double newG = 0.09 * r + 0.67 * g + 0.26 * b;
    double newB = 0.15 * r + 0.10 * g + 0.73 * b;
    // Assuming no alpha channel in original calculation.
#endif

    if (newR < 0)
        newR = 0;
    if (newG < 0)
        newG = 0;
    if (newB < 0)
        newB = 0;  

    // Convert to display gamma.
    newR = std::pow(newR, 1.0 / DISPLAY_GAMMA);
    newG = std::pow(newG, 1.0 / DISPLAY_GAMMA);
    newB = std::pow(newB, 1.0 / DISPLAY_GAMMA);

    // Denormalize, clamp and convert to RGB8.
    u32 outR = rgb8ToRgb5(std::clamp<int>(newR * 255, 0, 255));
    u32 outG = rgb8ToRgb5(std::clamp<int>(newG * 255, 0, 255));
    u32 outB = rgb8ToRgb5(std::clamp<int>(newB * 255, 0, 255));
    return outR | outG<<5 | outB<<10 | BIT(15);
}
