#include "iconHandler.h"
#include "ThemeTextures.h"
#include "common/twlmenusettings.h"
#include "common/logging.h"
#include <gl2d.h>
#include <ctype.h>
#include <nds.h>
#include <stdio.h>
#include <sys/stat.h>

bool initialized;

static int _iconTexID[NDS_ICON_BANK_COUNT];
static u16 _paletteCache[NDS_ICON_BANK_COUNT][16];

glImage _ndsIcon[NDS_ICON_BANK_COUNT][TWL_ICON_FRAMES];

static u8 clearTiles[(32 * 256) / 2] = {0};
static u16 blackPalette[16 * 8] = {0};

/**
 * Gets the current icon stored at the specified index.
 * If the index is out of bounds or the icon manager is not
 * initialized, returns null.
 */
const glImage *getIcon(int num) {
	if (BAD_ICON_IDX(num) || !initialized)
		return NULL;
	return _ndsIcon[num];
}

/**
 * Copied glLoadTileSet directly from gl2d.c
 *
 * With the added feature of specifitying the preallocated textureID.
 * The texture ID is intended to have already been initialized, as well
 * as palette already set.
 *
 * Do not call this directly, instead, use only loadIcon.
 */
void glLoadTileSetIntoSlot(int num, int tile_wid, int tile_hei, int bmp_wid, int bmp_hei, GL_TEXTURE_TYPE_ENUM type,
			   int sizeX, int sizeY, int param, int pallette_width, const u16 *_palette,
			   const uint8 *_texture, bool init) {
	int textureID;
	glImage *sprite;

	switch (num) {
	default:
		if (BAD_ICON_IDX(num))
			return;
		textureID = _iconTexID[num];
		sprite = _ndsIcon[num];
		break;
	}

	glBindTexture(0, textureID);
	glTexImage2D(0, 0, type, sizeX, sizeY, 0, param, _texture);
	glColorTableEXT(0, 0, pallette_width, 0, 0, _palette);

	int i = 0;
	int x, y;

	// init sprites texture coords and texture ID
	for (y = 0; y < (bmp_hei / tile_hei); y++) {
		for (x = 0; x < (bmp_wid / tile_wid); x++) {
			sprite[i].width = tile_wid;
			sprite[i].height = tile_hei;
			sprite[i].u_off = x * tile_wid;
			sprite[i].v_off = y * tile_hei;
			sprite[i].textureID = textureID;
			i++;
		}
	}
}

static inline GL_TEXTURE_SIZE_ENUM tex_height(int texHeight) {
	switch (texHeight) {
	case 8:
		return TEXTURE_SIZE_8;
	case 16:
		return TEXTURE_SIZE_16;
	case 32:
		return TEXTURE_SIZE_32;
	case 64:
		return TEXTURE_SIZE_64;
	case 128:
		return TEXTURE_SIZE_128;
	case 256:
		return TEXTURE_SIZE_256;
	case 512:
		return TEXTURE_SIZE_512;
	case 1024:
		return TEXTURE_SIZE_1024;
	default:
		return TEXTURE_SIZE_8;
	}
}

/**
 * Initializes an icon into one of 6 existing banks, overwritting
 * the previous data.
 * num must be in the range [0, 5], OR ONE OF
 * GBA_ICON, GBC_ICON, or NES_ICON
 *
 * If init is true, then the palettes will be copied into
 * texture memory before being bound with
 * glColorTableEXT.
 *
 * Otherwise, they will be replacing the existing palette
 * using glColorTableSubEXT at the same memory location.
 */
void glLoadIcon(int num, const u16 *_palette, const u8 *_tiles, int texHeight, bool init) {
	if (!BAD_ICON_IDX(num))
		swiCopy(_palette, _paletteCache[num], 4 * sizeof(u16) | COPY_MODE_COPY | COPY_MODE_WORD);

	glLoadTileSetIntoSlot(num,
			      32,		     // sprite width
			      32,		     // sprite height
			      32,		     // bitmap image width
			      texHeight,	     // bitmap image height
			      GL_RGB16,		     // texture type for glTexImage2D() in videoGL.h
			      TEXTURE_SIZE_32,       // sizeX for glTexImage2D() in videoGL.h
			      tex_height(texHeight), // sizeY for glTexImage2D() in videoGL.h
			      TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT,
			      16,		 // Length of the palette to use (16 colors)
			      (u16 *)_palette, // Image palette
			      (u8 *)_tiles,      // Raw image data
			      init);
}

void glLoadPalette(int num, const u16 *_palette) {
	if (!BAD_ICON_IDX(num))
		swiCopy(_palette, _paletteCache[num], 4 * sizeof(u16) | COPY_MODE_COPY | COPY_MODE_WORD);

	glReloadIconPalette(num);
}

/**
 * Reloads the palette in the given slot from
 * the palette cache.
 */
void glReloadIconPalette(int num) {

	int textureID;
	const u16 *cachedPalette;
	switch (num) {
	default:
		if (BAD_ICON_IDX(num))
			return;
		textureID = _iconTexID[num];
		cachedPalette = _paletteCache[num];
		break;
	}
	
	glBindTexture(0, textureID);
	glColorTableEXT(0, 0, 16, 0, 0, cachedPalette);
}

/**
 * Reloads all the palettes in the palette cache if
 * they have been corrupted.
 */
void reloadIconPalettes() {
	for (int i = 0; i < NDS_ICON_BANK_COUNT; i++) {
		glReloadIconPalette(i);
	}
}
/**
 * Loads an icon into one of 6 existing banks, overwritting
 * the previous data.
 * num must be in the range [0, 5], or else this function
 * does nothing.
 *
 * If init is true, then the palettes will be copied into
 * texture memory before being bound with
 * glColorTableEXT.
 *
 * Otherwise, they will be replacing the existing palette
 * using glColorTableSubEXT at the same memory location.
 */
void glLoadIcon(int num, const u16 *palette, const u8 *tiles, int texHeight) {
	glLoadIcon(num, palette, tiles, texHeight, false);
}

/**
 * Clears an icon from the bank.
 */
void glClearIcon(int num) { glLoadIcon(num, blackPalette, clearTiles, 256, true); }

/**
 * Allocates and initializes the VRAM locations for
 * icons. Must be called before the icon manager is used.
 */
void iconManagerInit() {
	logPrint("iconManagerInit()\n");

	tex().loadIconUnknownTexture();

	// Allocate texture memory for 6 textures.
	glGenTextures(NDS_ICON_BANK_COUNT, _iconTexID);

	// Initialize empty data for the 6 textures.
	for (int i = 0; i < NDS_ICON_BANK_COUNT; i++) {
		// Todo: Check if this is too much VRAM for NDS icons.
		glLoadIcon(i, tex().iconUnknownTexture()->palette(), tex().iconUnknownTexture()->bytes(),
			   TWL_TEX_HEIGHT, true);
	}

	// set initialized.
	initialized = true;
}
