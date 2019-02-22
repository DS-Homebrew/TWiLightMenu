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
        u8 _texHeight;
        u8 _texWidth;
    public:
        BmpTexture(std::string& filePath, const u8 texHeight, const u8 texWidth);
        ~BmpTexture() { };
        void applyEffect(BitmapEffect effect);
        const u16 *texture() { return (u16*)_texture.get(); }
};

#endif