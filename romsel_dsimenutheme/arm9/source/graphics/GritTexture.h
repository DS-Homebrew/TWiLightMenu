#pragma once
#ifndef __TWILIGHTMENU_GRIT_TEXTURES__
#define __TWILIGHTMENU_GRIT_TEXTURES__

#include <memory>
#include <string>
#include "nds.h"

using std::unique_ptr;

struct GrfHeader
{
    u8  gfxAttr, mapAttr, mmapAttr, palAttr;
    u8  tileWidth, tileHeight;
    u8  metaWidth, metaHeight;
    u32 texWidth, texHeight;
};

class GritTexture
{
    typedef void (*PaletteEffect)(u16* palette, u8 paletteLength);
    private:
        unique_ptr<unsigned short []> _palette;
        unique_ptr<unsigned int []> _texture;
        u8 _paletteLength;
        u32 _graphicsLength;
        GrfHeader _header;
    public:
        GritTexture(std::string& filePath);
        ~GritTexture() { };
        void applyEffect(PaletteEffect effect);
        const u16 *palette() { return (u16*)_palette.get(); }
        const u8 *texture() { return (u8*)_texture.get(); }
    
};

#endif