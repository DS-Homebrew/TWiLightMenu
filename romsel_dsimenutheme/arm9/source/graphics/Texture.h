#pragma once
#ifndef __TWILIGHTMENU_TEXTURES_CLASS__
#define __TWILIGHTMENU_TEXTURES_CLASS__

#define BMP_ID(a,b)         ((u16)( (a) | (b)<<8 ))
#define CHUNK_ID(a,b,c,d)   ((u32)( (a) | (b)<<8 | (c)<<16 | (d)<<24 ))
#define ID_RIFF             CHUNK_ID('R','I','F','F')
#define ID_GRF              CHUNK_ID('G','R','F',' ')
#define ID_BMP              BMP_ID('B', 'M')

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

enum TextureType {
    Unknown = 0,
    PalettedGrf = 1,
    RawBitmap = 2,
    CompressedGrf = 3
};

class Texture
{
    typedef void (*PaletteEffect)(u16* palette, u8 paletteLength);
    typedef void (*BitmapEffect)(u16* texture, u32 texLength);

    private:
        unique_ptr<unsigned short []> _palette;
        unique_ptr<unsigned short []> _texture;
        u8 _paletteLength;
        unsigned int _texLength; // in words
        unsigned int _texCmpLength;
        u32 _texHeight;
        u32 _texWidth;
        TextureType _type;
    
    public:
        Texture(const std::string& filePath, const std::string& fallback) noexcept;
        virtual ~Texture() = default;
        void applyPaletteEffect(PaletteEffect effect);
        void applyBitmapEffect(BitmapEffect effect);

        const u16 *palette() const { return _type == TextureType::PalettedGrf ? (u16*)_palette.get() : NULL; }
        const u16 *texture() const { return _texture.get(); }
        const u8 *bytes() const { return (u8*)_texture.get(); }

        u32 texHeight() const { return _texHeight; };
        u32 texWidth() const { return _texWidth; };
        u32 pixelCount() const { return _texHeight * _texWidth; };
        u8 paletteLength() const { return _type == TextureType::PalettedGrf ? _paletteLength : 0;};
        TextureType type() const { return _type; }

    private:
        TextureType findType(FILE* file);
        void loadBitmap(FILE* file) noexcept;
        void loadPaletted(FILE* file) noexcept;
        void loadCompressed(FILE* file) noexcept;
};


#endif