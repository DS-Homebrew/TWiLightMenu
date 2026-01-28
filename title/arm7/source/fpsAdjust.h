#pragma once

#define FPSA_SYS_CLOCK           33513982

#define FPSA_CYCLES_PER_PIXEL    6

#define FPSA_LCD_WIDTH           256
#define FPSA_LCD_HBLANK          99
#define FPSA_LCD_COLUMNS         (FPSA_LCD_WIDTH + FPSA_LCD_HBLANK)

#define FPSA_LCD_HEIGHT          192
#define FPSA_LCD_VBLANK          71
#define FPSA_LCD_LINES           (FPSA_LCD_HEIGHT + FPSA_LCD_VBLANK)

#define FPSA_CYCLES_PER_LINE     (FPSA_LCD_COLUMNS * FPSA_CYCLES_PER_PIXEL)
#define FPSA_CYCLES_PER_FRAME    (FPSA_LCD_COLUMNS * FPSA_LCD_LINES * FPSA_CYCLES_PER_PIXEL)

#define FPSA_ADJUST_MIN_VCOUNT   202
#define FPSA_ADJUST_MAX_VCOUNT   212

typedef struct
{
    u32 isStarted;
    u32 isFpsLower; //TRUE if vblank needs to be extended, FALSE if it needs to be shortened
    u32 backJump;
    u32 initial;
    u64 targetCycles; //target cycles in 0.40.24 format
    s64 cycleDelta;
	u32 linesToSkipMax;
} fpsa_t;

void fpsa_init(fpsa_t* fpsa);
void fpsa_start(fpsa_t* fpsa);
void fpsa_stop(fpsa_t* fpsa);

//Sets the target frame rate as clock cycles per frame in 0.40.24 format
void fpsa_setTargetFrameCycles(fpsa_t* fpsa, u64 cycles);

//Sets the target frame rate as a frames per second fraction
void fpsa_setTargetFpsFraction(fpsa_t* fpsa, u32 num, u32 den);

//Sets the target frame rate in integer frames per second
static inline void fpsa_setTargetFps(fpsa_t* fpsa, u32 fps)
{
    fpsa_setTargetFpsFraction(fpsa, fps, 1);
}