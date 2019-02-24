#include "GritTexture.h"
#include "stdio.h"
#include "string.h"
#include <cstdbool>

bool verify_grif(FILE *file) {

	u32 magic[4] = {0};
	fseek(file, 0, SEEK_SET);
	fread(&magic, sizeof(u32), 4, file);

	// Needs to be in format [RIFF][u32 size][GRF ][HDR ]
	if (magic[0] != CHUNK_ID('R', 'I', 'F', 'F'))
		return false;
	if (magic[2] != CHUNK_ID('G', 'R', 'F', ' '))
		return false;
	if (magic[3] != CHUNK_ID('H', 'D', 'R', ' '))
		return false;

	return true;
}

int GritTexture::loadUnchecked(FILE *file) {

	fseek(file, 5 * sizeof(u32), SEEK_SET);
	fread(&_header, sizeof(GrfHeader), 1, file);

    u32 gfxId = 0;
    fread(&gfxId, sizeof(u32), 1, file);
    if (gfxId != CHUNK_ID('G', 'F', 'X', ' ')) return 1;

    // skip size of GFX data (since it includes textureLength u32 length).
	fseek(file, sizeof(u32), SEEK_CUR);

	u32 textureLengthInBytes = 0;
	fread(&textureLengthInBytes, sizeof(u32), 1, file);

	_textureLength = (textureLengthInBytes >> 8) / sizeof(unsigned int);
    
	_texture = std::make_unique<unsigned int[]>(_textureLength);
	fread(_texture.get(), sizeof(unsigned int), _textureLength, file);

    u32 palId = 0;
    fread(&palId, sizeof(u32), 1, file);
    if (palId != CHUNK_ID('P', 'A', 'L', ' ')) return 1;
	fseek(file, sizeof(u32), SEEK_CUR);

	u32 paletteLength = 0;
	fread(&paletteLength, sizeof(u32), 1, file);
	_paletteLength = (paletteLength >> 8) / sizeof(unsigned short);

	_palette = std::make_unique<unsigned short[]>(_paletteLength);
	fread(_palette.get(), sizeof(unsigned short), _paletteLength, file);

    return 0;
}

GritTexture::GritTexture(const std::string& filePath, const std::string& fallback) {
	FILE *file = fopen(filePath.c_str(), "rb");
	bool isValidGrif = false;

	if (file) {
		isValidGrif = verify_grif(file);
	}

	if (!file || !isValidGrif) {
		if (!isValidGrif) {
			fclose(file);
		}

		file = fopen(fallback.c_str(), "rb");
	}

    if (file) {
        // read in header        

        fseek(file, 5 * sizeof(u32), SEEK_SET);
        fread(&_header, sizeof(GrfHeader), 1, file);

        // Skip 'G' 'F' 'X'[datasize]
        // todo: verify
        fseek(file, 2 * sizeof(u32), SEEK_CUR);
        u32 textureLengthInBytes = 0;
        fread(&textureLengthInBytes, sizeof(u32), 1, file);

        
        _textureLength = (textureLengthInBytes >> 8) / sizeof(unsigned int);
    
        _texture = std::make_unique<unsigned int[]>(_textureLength);
        fread(_texture.get(), sizeof(unsigned int), _textureLength, file);


        // Skip 'P' 'A' 'L'[datasize]
        // todo: verify
        fseek(file, 2 * sizeof(u32), SEEK_CUR);
        u32 paletteLength = 0;
        fread(&paletteLength, sizeof(u32), 1, file);
        _paletteLength = (paletteLength >> 8) / sizeof(unsigned short);

        _palette = std::make_unique<unsigned short[]>(_paletteLength);
        fread(_palette.get(), sizeof(unsigned short), _paletteLength, file);
    }

	fclose(file);
}

void GritTexture::applyEffect(GritTexture::PaletteEffect effect) { effect(_palette.get(), _paletteLength); }
