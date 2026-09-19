#include <gl2d.h>
#include <nds.h> // REG_VCOUNT, for vramSafeToUnmap() below

#pragma once

// Half-width of the icon grid's draw window, in boxes either side of the
// cursor. The draw loop in graphics.cpp samples list entries
// [CURPOS - ICON_GRID_MAX_OFFSET, CURPOS + ICON_GRID_MAX_OFFSET].
#define ICON_GRID_MAX_OFFSET 3

// One texture bank per box the grid can draw in a single frame. This MUST stay
// >= the width of that window, or two entries drawn in the same frame alias
// onto one bank and the box further from the cursor renders the other entry's
// icon (and steals its palette in loadDeferredIconPalettes).
#define ICON_GRID_BANKS (2 * ICON_GRID_MAX_OFFSET + 1)

// Dedicated bank for the app being dragged in move mode (list index 40).
#define ICON_MOVING_BANK ICON_GRID_BANKS

#define NDS_ICON_BANK_COUNT (ICON_GRID_BANKS + 1)

// Maps a list position (0..39) to the bank holding its icon. Every load site
// and every draw site must agree on this one expression.
#define ICON_GRID_BANK(pos) ((pos) % ICON_GRID_BANKS)

#define TWL_ICON_FRAMES 8
#define TWL_TEX_HEIGHT 256

// The blanking interval is the only time it is safe to upload to VRAM. Every GL
// upload switches its bank to LCD mode for the length of the copy --
// glTexImage2D takes VRAM A-D (the 3D textures and both backgrounds) and
// glColorTableEXT takes E/F/G -- so one that runs during active display blanks
// them mid-scanout and shows up as a horizontal stripe. The vblank handler does
// not always start on line 192: an SD read on the main thread can hold the IRQ
// off well past it, and SD reads are exactly what queues these uploads while
// scrolling. So check this before each one rather than assuming.
// The margin matters as much as the lower bound: blanking is lines 192..262, and
// an upload started at 260 still finishes under the beam. Several banks can be
// queued at once, so check this before each one rather than only on entry.
#define VRAM_UNMAP_LAST_SAFE_LINE 250
static inline bool vramSafeToUnmap() {
	const int line = REG_VCOUNT;
	return line >= 192 && line <= VRAM_UNMAP_LAST_SAFE_LINE;
}

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
 * Loads an icon into one of the NDS_ICON_BANK_COUNT existing banks,
 * overwritting the previous data.
 * num must be in the range [0, NDS_ICON_BANK_COUNT - 1] else this
 * function does nothing.
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
 * Loads an icon's palette into one of the NDS_ICON_BANK_COUNT existing
 * banks, overwritting the previous data.
 * num must be in the range [0, NDS_ICON_BANK_COUNT - 1] else this
 * function does nothing.
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