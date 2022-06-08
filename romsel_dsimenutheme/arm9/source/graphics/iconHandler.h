#include <gl2d.h>

#pragma once

#define GBA_ICON 0xBA
#define GBC_ICON 0xBC
#define NES_ICON 0xE5
#define SG_ICON 0xE6
#define SMS_ICON 0xE7
#define GG_ICON 0xE8
#define MD_ICON 0xDD
#define SNES_ICON 0xE2
#define MSX_ICON 0xEE
#define PLG_ICON 0xD2
#define A26_ICON 0xA2
#define COL_ICON 0xC0
#define M5_ICON 0x55
#define INT_ICON 0x1E
#define PCE_ICON 0xCE
#define WS_ICON 0xA5
#define NGP_ICON 0xAF
#define CPC_ICON 0xCC
#define VID_ICON 0x1D
#define NDS_ICON_BANK_COUNT 7
#define TWL_ICON_FRAMES 8
#define TWL_TEX_HEIGHT 256

// Checks if the icon is a bad index
#define BAD_ICON_IDX(i) (i < 0 || i > (NDS_ICON_BANK_COUNT - 1))

/**
 * Gets the current icon stored at the specified index.
 * If the index is out of bounds or the icon manager is not
 * initialized, returns null.
 */
const glImage* getIcon(int num);

/**
 * Allocates and initializes the VRAM locations for
 * icons. Must be called before the icon manager is used.
 */
void iconManagerInit();

/**
 * Loads an icon into one of 6 existing banks, overwritting 
 * the previous data.
 * num must be in the range [0, 5] else this function
 * does nothing.
 * 
 * If init is true, then the palettes will be copied into
 * texture memory before being bound with
 * glColorTableEXT. 
 * 
 * Otherwise, they will be replacing the existing palette
 * using glColorTableSubEXT at the same memory location.
 * 
 * texHeight must be a power of two, or bad things will happen.
 */
void glLoadIcon(int num, const u16 *palette, const u8 *tiles, int texHeight = 32);

/**
 * Loads an icon's palette into one of 6 existing banks,
 * overwritting the previous data.
 * num must be in the range [0, 5] else this function
 * does nothing.
 */
void glLoadPalette(int num, const u16 *palette);


/**
 * Clears an icon in the bank.
 */
void glClearIcon(int num);

/**
 * Reloads the palette of the icon in the 
 * numth slot, if it has been corrupted.
 */
void glReloadIconPalette(int num);

/**
 * Reloads the palette of all the icons in a slot, if
 * they have been corrupted.
 */
void reloadIconPalettes();