#include "Texture.h"

Texture::Texture(const std::string &filePath, const std::string &fallback)
    : _paletteLength(0), _texLength(0), _texCmpLength(0), _texHeight(0), _texWidth(0), _type(TextureType::Unknown) {
	FILE *file = fopen(filePath.c_str(), "rb");
	if (!file)
		file = fopen(fallback.c_str(), "rb");

	_type = findType(file);

	if (_type == TextureType::Unknown) {
		fclose(file);
		file = fopen(fallback.c_str(), "rb");
		_type = findType(file);
	}

	switch (_type) {
	case TextureType::PalettedGrf:
		loadPaletted(file);
		break;
	case TextureType::RawBitmap:
		loadBitmap(file);
		break;
	default:
		break;
	}

	fclose(file);
}

TextureType Texture::findType(FILE *file) {

	fseek(file, 0, SEEK_SET);
	u32 magic[12] = {0};
	fread(&magic, sizeof(u32), 12, file);

	if (((u16)magic[0] & 0xffff) == BMP_ID('B', 'M')) {
		return TextureType::RawBitmap;
	}

	if (magic[0] == CHUNK_ID('R', 'I', 'F', 'F') && magic[2] == CHUNK_ID('G', 'R', 'F', ' ') &&
	    magic[3] == CHUNK_ID('H', 'D', 'R', ' ') && magic[9] == CHUNK_ID('G', 'F', 'X', ' ')) {
		switch (magic[11] & 0xF0) {
		// may not be the case for general GRF, but for our case
		// We're going to assume all paletted textures are not compressed.
		case 0x00:
			return TextureType::PalettedGrf;
		// LZ77 Compressed
		case 0x10:
			return TextureType::CompressedGrf;
		default:
			return TextureType::Unknown;
		}
	}

	return TextureType::Unknown;
}

void Texture::loadBitmap(FILE *file) noexcept {
	// SKIP 'B' 'M' Idenifier
	fseek(file, sizeof(u16), SEEK_SET);

	u16 offset = 0;
	u32 texLength = 0;

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
}

void Texture::loadPaletted(FILE *file) noexcept {

	GrfHeader header;
	fseek(file, 5 * sizeof(u32), SEEK_SET);
	fread(&header, sizeof(GrfHeader), 1, file);

	_texHeight = header.texHeight;
	_texWidth = header.texWidth;

	// Skip 'G' 'F' 'X'[datasize]
	// todo: verify

	fseek(file, 2 * sizeof(u32), SEEK_CUR);
	u32 textureLengthInBytes = 0;
	fread(&textureLengthInBytes, sizeof(u32), 1, file);

	_texLength = (textureLengthInBytes >> 9); // palette length in ints sizeof(unsigned int);

	_texture = std::make_unique<unsigned short[]>(_texLength);
	fread(_texture.get(), sizeof(unsigned short), _texLength, file);

	// Skip 'P' 'A' 'L'[datasize]
	// todo: verify
	fseek(file, 2 * sizeof(u32), SEEK_CUR);
	u32 paletteLength = 0;
	fread(&paletteLength, sizeof(u32), 1, file);
	_paletteLength = (paletteLength >> 9); // palette length in shorts. / sizoef(unsighed shor)

	_palette = std::make_unique<unsigned short[]>(_paletteLength);
	fread(_palette.get(), sizeof(unsigned short), _paletteLength, file);
}

void Texture::applyPaletteEffect(Texture::PaletteEffect effect) {
	if (_type == TextureType::PalettedGrf) {
		effect(_palette.get(), _paletteLength);
	}
}

void Texture::applyBitmapEffect(Texture::BitmapEffect effect) {
	if (_type == TextureType::RawBitmap) {
		effect(_texture.get(), _texLength);
	}
}
