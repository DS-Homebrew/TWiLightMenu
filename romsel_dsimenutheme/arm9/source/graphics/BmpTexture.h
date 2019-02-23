#pragma once
#ifndef __TWILIGHTMENU_BMP_TEXTURES__
#define __TWILIGHTMENU_BMP_TEXTURES__

#include <memory>
#include <string>
#include "nds.h"

using std::unique_ptr;

class BmpTexture
{
    typedef void (*BitmapEffect)(u16* palette, u8 paletteLength);
    private:
        unique_ptr<u16[]> _texture;
        u32 _texHeight;
        u32 _texWidth;
        u32 _texLength;
    public:
        BmpTexture(const char* filePath);
        ~BmpTexture() { };
        void applyEffect(BitmapEffect effect);
        const u16 *texture() { return (u16*)_texture.get(); }
};

#endif