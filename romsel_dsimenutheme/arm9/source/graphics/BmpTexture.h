#pragma once
#ifndef __TWILIGHTMENU_BMP_TEXTURES__
#define __TWILIGHTMENU_BMP_TEXTURES__

#include <memory>
#include <string>
#include "nds.h"

using std::unique_ptr;

class BmpTexture
{
    typedef void (*BitmapEffect)(u16* texture, u32 texLength);
    private:
        unique_ptr<u16[]> _texture;
        u32 _texHeight;
        u32 _texWidth;
        u32 _texLength;
    public:
        BmpTexture(const std::string& file, const std::string& fallbackPath) noexcept;
        virtual ~BmpTexture() = default;
        void applyEffect(BitmapEffect effect);
        const u16 *texture() const { return (u16*)_texture.get(); }
        u32 texHeight() const { return _texHeight; }
        u32 texWidth() const { return _texWidth; }
        u32 pixelCount() const { return _texHeight * _texWidth; }

};

#endif