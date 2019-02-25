#pragma once
#ifndef __TWILIGHTMENU_GRIT_TEXTURES__
#define __TWILIGHTMENU_GRIT_TEXTURES__


#define CHUNK_ID(a,b,c,d)   ((u32)( (a) | (b)<<8 | (c)<<16 | (d)<<24 ))
#define ID_RIFF             CHUNK_ID('R','I','F','F')
#define ID_GRF              CHUNK_ID('G','R','F',' ')

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
        unsigned int _textureLength;
        GrfHeader _header;
        int loadUnchecked(FILE* file);
    public:
        GritTexture(const std::string& filePath, const std::string& fallback);
        ~GritTexture() { };
        void applyEffect(PaletteEffect effect);
        const u16 *palette() const { return (u16*)_palette.get(); }
        const u8 *texture() const { return (u8*)_texture.get(); }
};


#endif