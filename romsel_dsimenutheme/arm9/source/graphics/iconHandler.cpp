#include "iconHandler.h"
#include <nds.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/stat.h>
#include <gl2d.h>


// Graphic files
#include "icon_unk.h"
#include "icon_gbamode.h"
#include "icon_gba.h"
#include "icon_gb.h"
#include "icon_nes.h"


bool initialized;

int _iconTexID[NDS_ICON_BANK_COUNT];

int _gbaTexID;
int _gbcTexID;
int _nesTexID;

glImage _ndsIcon[NDS_ICON_BANK_COUNT][TWL_ICON_FRAMES];
glImage _gbaIcon[1];
glImage _gbcIcon[(32 / 32) * (64 / 32)];
glImage _nesIcon[1];


extern bool useGbarunner;

/**
 * Gets the current icon stored at the specified index.
 * If the index is out of bounds or the icon manager is not
 * initialized, returns null.
 */
const glImage *getIcon(int num)
{
    if (num == GBA_ICON) return _gbaIcon;
    if (num == GBC_ICON) return _gbcIcon;
    if (num == NES_ICON) return _nesIcon;
    if (BAD_ICON_IDX(num) || !initialized) return NULL;
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
void glLoadTileSetIntoSlot(
    int num,
    int tile_wid,
    int tile_hei,
    int bmp_wid,
    int bmp_hei,
    GL_TEXTURE_TYPE_ENUM type,
    int sizeX,
    int sizeY,
    int param,
    int pallette_width,
    const u16 *_palette,
    const uint8 *_texture,
    bool init)
{
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
        default:
            if (BAD_ICON_IDX(num)) return;
            textureID = _iconTexID[num];
            sprite = _ndsIcon[num];
            break;
    }

    glBindTexture(0, textureID);
    glTexImage2D(0, 0, type, sizeX, sizeY, 0, param, _texture);

    if (!init)
    {
        glColorSubTableEXT(0, 0, pallette_width, 0, 0, _palette);
    }
    else
    {
        glColorTableEXT(0, 0, pallette_width, 0, 0, _palette);
    }

    int i = 0;
    int x, y;

    // init sprites texture coords and texture ID
    for (y = 0; y < (bmp_hei / tile_hei); y++)
    {
        for (x = 0; x < (bmp_wid / tile_wid); x++)
        {
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
void glLoadIcon(int num, const u16 *_palette, const u8 *_tiles, int texHeight, bool init)
{
    if (!init && !BAD_ICON_IDX(num)) {
        // Check if the NDS icon to be loaded is already loaded in the
        // slot. If so, only reload the palette if necessary.
        void* texPtr = glGetTexturePointer(_iconTexID[num]);
        if (texPtr && memcmp(_tiles, texPtr, sizeof(*_tiles)) == 0) {
            uint16 cmpPal[sizeof(*_palette)];
            glBindTexture(0, _iconTexID[num]);
            glGetColorTableEXT(0,0,0, cmpPal);
            if (memcmp(cmpPal, _palette, sizeof(*_palette)) != 0) {
                glColorSubTableEXT(0, 0, 4, 0, 0, (u16*) _palette);
            }
            return;
        }
    }

    glLoadTileSetIntoSlot(
        num,
        32,               // sprite width
        32,               // sprite height
        32,               // bitmap image width
        texHeight,              // bitmap image height
        GL_RGB16,         // texture type for glTexImage2D() in videoGL.h
        TEXTURE_SIZE_32,  // sizeX for glTexImage2D() in videoGL.h
        tex_height(texHeight), // sizeY for glTexImage2D() in videoGL.h
        TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT,
        16,              // Length of the palette to use (16 colors)
        (u16 *)_palette, // Image palette
        (u8 *)_tiles,    // Raw image data
        init
    );
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
void glLoadIcon(int num, const u16 *palette, const u8 *tiles, int texHeight)
{
    glLoadIcon(num, palette, tiles, texHeight, false);
}

/**
 * Allocates and initializes the VRAM locations for
 * icons. Must be called before the icon manager is used.
 */
void iconManagerInit()
{
    consoleDemoInit();
    // Allocate texture memory for 6 textures.
    glGenTextures(6, _iconTexID);
    
    // Allocate texture memory for GBA/GBC/NES icons.
    glGenTextures(1, &_gbaTexID);
    glGenTextures(1, &_gbcTexID);
    glGenTextures(1, &_nesTexID);

    // Initialize empty data for the 6 textures.
    for (int i = 0; i < 6; i++)
    {
        // Todo: Check if this is too much VRAM for NDS icons.
        glLoadIcon(i, (u16 *)icon_unkPal, (u8 *)icon_unkBitmap, TWL_TEX_HEIGHT, true);
    }

    // Load GB Icon
	glLoadIcon(GBC_ICON, (u16*) icon_gbPal,(u8*) icon_gbBitmap, 64, true);

	glLoadIcon(NES_ICON, (u16*) icon_nesPal,(u8*) icon_nesBitmap, 32, true);
    
    if (useGbarunner) {
        glLoadIcon(GBA_ICON, (u16*) icon_gbaPal,(u8*) icon_gbaBitmap, 32, true);
    } else {
        glLoadIcon(GBA_ICON, (u16*) icon_gbamodePal,(u8*) icon_gbamodeBitmap, 32, true);
    }

    // set initialized.
    initialized = true;
}

