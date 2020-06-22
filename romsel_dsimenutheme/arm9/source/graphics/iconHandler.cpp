#include "iconHandler.h"
#include "ThemeTextures.h"
#include "common/dsimenusettings.h"
#include <gl2d.h>
#include "tool/colortool.h"
#include <ctype.h>
#include <nds.h>
#include <stdio.h>
#include <sys/stat.h>

bool initialized;

static int _iconTexID[NDS_ICON_BANK_COUNT];
static u16 _paletteCache[NDS_ICON_BANK_COUNT][16];

int _gbaTexID;
int _gbcTexID;
int _nesTexID;
int _smsTexID;
int _ggTexID;
int _mdTexID;
int _snesTexID;
//int _msxTexID;
//int _colTexID;
int _plgTexID;

glImage _ndsIcon[NDS_ICON_BANK_COUNT][TWL_ICON_FRAMES];
glImage _gbaIcon[(32 / 32) * (64 / 32)];
glImage _gbcIcon[(32 / 32) * (64 / 32)];
glImage _nesIcon[1];
glImage _smsIcon[1];
glImage _ggIcon[1];
glImage _mdIcon[1];
glImage _snesIcon[1];
// glImage _msxIcon[1];
// glImage _colIcon[1];
glImage _plgIcon[1];

static u8 clearTiles[(32 * 256) / 2] = {0};
static u16 blackPalette[16 * 8] = {0};

/**
 * Gets the current icon stored at the specified index.
 * If the index is out of bounds or the icon manager is not
 * initialized, returns null.
 */
const glImage *getIcon(int num) {
	if (num == GBA_ICON)
		return _gbaIcon;
	if (num == GBC_ICON)
		return _gbcIcon;
	if (num == NES_ICON)
		return _nesIcon;
	if (num == SMS_ICON)
		return _smsIcon;
	if (num == GG_ICON)
		return _ggIcon;
	if (num == MD_ICON)
		return _mdIcon;
	if (num == SNES_ICON)
		return _snesIcon;
	/*if (num == MSX_ICON)
	    return _msxIcon;
	if (num == COL_ICON)
	    return _colIcon;*/
	if (num == PLG_ICON)
		return _plgIcon;
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
	case GBA_ICON:
		textureID = _gbaTexID;
		sprite = _gbaIcon;
		break;
	case GBC_ICON:
		textureID = _gbcTexID;
		sprite = _gbcIcon;
		break;
	case NES_ICON:
		textureID = _nesTexID;
		sprite = _nesIcon;
		break;
	case SMS_ICON:
		textureID = _smsTexID;
		sprite = _smsIcon;
		break;
	case GG_ICON:
		textureID = _ggTexID;
		sprite = _ggIcon;
		break;
	case MD_ICON:
		textureID = _mdTexID;
		sprite = _mdIcon;
		break;
	case SNES_ICON:
		textureID = _snesTexID;
		sprite = _snesIcon;
		break;
	case PLG_ICON:
		textureID = _plgTexID;
		sprite = _plgIcon;
		break;
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
	u16 *newPalette = (u16 *)_palette;

	if (ms().colorMode == 1) {
		for (int i2 = 0; i2 < 16; i2++) {
			*(newPalette + i2) = convertVramColorToGrayscale(*(newPalette + i2));
		}
	}

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
			      (u16 *)newPalette, // Image palette
			      (u8 *)_tiles,      // Raw image data
			      init);
}

/**
 * Reloads the palette in the given slot from
 * the palette cache.
 */
void glReloadIconPalette(int num) {

	int textureID;
	const u16 *cachedPalette;
	switch (num) {
	case GBA_ICON:
		textureID = _gbaTexID;
		cachedPalette = tex().iconGBATexture()->palette();
		break;
	case GBC_ICON:
		textureID = _gbcTexID;
		cachedPalette = tex().iconGBTexture()->palette();
		break;
	case NES_ICON:
		textureID = _nesTexID;
		cachedPalette = tex().iconNESTexture()->palette();
		break;
	case SMS_ICON:
		textureID = _smsTexID;
		cachedPalette = tex().iconSMSTexture()->palette();
		break;
	case GG_ICON:
		textureID = _ggTexID;
		cachedPalette = tex().iconGGTexture()->palette();
		break;
	case MD_ICON:
		textureID = _mdTexID;
		cachedPalette = tex().iconMDTexture()->palette();
		break;
	case SNES_ICON:
		textureID = _snesTexID;
		cachedPalette = tex().iconSNESTexture()->palette();
		break;
	case PLG_ICON:
		textureID = _plgTexID;
		cachedPalette = tex().iconPLGTexture()->palette();
		break;
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
	glReloadIconPalette(GBA_ICON);
	glReloadIconPalette(GBC_ICON);
	glReloadIconPalette(NES_ICON);
	glReloadIconPalette(SMS_ICON);
	glReloadIconPalette(GG_ICON);
	glReloadIconPalette(MD_ICON);
	glReloadIconPalette(SNES_ICON);
	glReloadIconPalette(PLG_ICON);

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

	// Allocate texture memory for 6 textures.
	glGenTextures(NDS_ICON_BANK_COUNT, _iconTexID);

	// Allocate texture memory for GBA/GBC/NES/SMS/GG/MD/SNES icons.
	glGenTextures(1, &_gbaTexID);
	glGenTextures(1, &_gbcTexID);
	glGenTextures(1, &_nesTexID);
	glGenTextures(1, &_smsTexID);
	glGenTextures(1, &_ggTexID);
	glGenTextures(1, &_mdTexID);
	glGenTextures(1, &_snesTexID);
	glGenTextures(1, &_plgTexID);

	// Initialize empty data for the 6 textures.
	for (int i = 0; i < NDS_ICON_BANK_COUNT; i++) {
		// Todo: Check if this is too much VRAM for NDS icons.
		glLoadIcon(i, tex().iconUnknownTexture()->palette(), tex().iconUnknownTexture()->bytes(),
			   TWL_TEX_HEIGHT, true);
	}

	// Load GB Icon
	glLoadIcon(GBC_ICON, tex().iconGBTexture()->palette(), tex().iconGBTexture()->bytes(), 64, true);

	glLoadIcon(NES_ICON, tex().iconNESTexture()->palette(), tex().iconNESTexture()->bytes(), 32, true);

	glLoadIcon(SMS_ICON, tex().iconSMSTexture()->palette(), tex().iconSMSTexture()->bytes(), 32, true);

	glLoadIcon(GG_ICON, tex().iconGGTexture()->palette(), tex().iconGGTexture()->bytes(), 32, true);

	glLoadIcon(MD_ICON, tex().iconMDTexture()->palette(), tex().iconMDTexture()->bytes(), 32, true);

	glLoadIcon(SNES_ICON, tex().iconSNESTexture()->palette(), tex().iconSNESTexture()->bytes(), 32, true);
	
	glLoadIcon(PLG_ICON, tex().iconPLGTexture()->palette(), tex().iconPLGTexture()->bytes(), 32, true);

	glLoadIcon(GBA_ICON, tex().iconGBATexture()->palette(), tex().iconGBATexture()->bytes(), 64, true);

	// set initialized.
	initialized = true;
}
