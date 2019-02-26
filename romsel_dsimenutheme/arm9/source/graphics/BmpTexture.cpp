#include "BmpTexture.h"

BmpTexture::BmpTexture(const std::string& filePath, const std::string& fallback)
    : _texHeight(0), _texWidth(0), _texLength(0)
{
    FILE *file = fopen(filePath.c_str(), "rb");
    
    if (!file) {
		file = fopen(fallback.c_str(), "rb");
	}

    if (file) {
        fseek(file, 0, SEEK_SET);
        u16 identifier = 0;
        u16 offset = 0;
        u32 texLength = 0;
        fread(&identifier, sizeof(u16), 1, file);

        // todo check if ID = 'B' 'M'

        fseek(file, 2 * sizeof(u32), SEEK_CUR);
        fread(&offset, sizeof(u32), 1, file);

        fseek(file, sizeof(u32), SEEK_CUR);

        fread(&_texWidth, sizeof(u32), 1, file);
        fread(&_texHeight, sizeof(u32), 1, file);

        fseek(file, 2 * sizeof(u32), SEEK_CUR);
        
        fread(&texLength, sizeof(u32), 1, file);

        _texLength = texLength >> 1;
        _texture = std::make_unique<u16[]>(_texLength);

        fseek(file, offset, SEEK_SET);
        fread(_texture.get(), sizeof(u16), _texLength, file);
        fclose(file);
    }

    fclose(file);

}

void BmpTexture::applyEffect(BmpTexture::BitmapEffect effect) { effect(_texture.get(), _texLength); }
