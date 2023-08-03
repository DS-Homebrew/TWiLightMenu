#include <gl2d.h>

#pragma once

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