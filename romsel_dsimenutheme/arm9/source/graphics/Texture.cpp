#include "Texture.h"
#include "common/dsimenusettings.h"
#include "lodepng.h"

Texture::Texture(const std::string &filePath, const std::string &fallback)
	: _paletteLength(0), _texLength(0), _texCmpLength(0), _texHeight(0), _texWidth(0), _type(TextureType::Unknown) {
	FILE *file = fopen(filePath.c_str(), "rb");
	bool useFallback = false;

	if (!file) {
		file = fopen(fallback.c_str(), "rb");
		useFallback = true;
	}

	_type = findType(file);
	
	if (_type == TextureType::Unknown) {
		fclose(file);
		file = fopen(fallback.c_str(), "rb");
		_type = findType(file);
	}

	switch (_type) {
	case TextureType::PalettedGrf:
		nocashMessage("loading paletted");
		loadPaletted(file);
		break;
	case TextureType::BMP:
		nocashMessage("loading bmp");
		loadBitmap(file);
		break;
	case TextureType::PNG:
		nocashMessage("loading png");
		loadPNG(useFallback ? fallback : filePath);
		break;
	case TextureType::CompressedGrf:
		nocashMessage("loading compressed");
		loadCompressed(file);
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
		return TextureType::BMP;
	} else if (magic[0] == CHUNK_ID('\x89', 'P', 'N', 'G')) {
		return TextureType::PNG;
	} else if (magic[0] == CHUNK_ID('R', 'I', 'F', 'F') && magic[2] == CHUNK_ID('G', 'R', 'F', ' ') &&
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
	std::vector<u16> buffer(texLength);
	for(uint i = 0; i < _texHeight; i++) {
		fread(buffer.data(), 1, buffer.size(), file);
	}

	// Apply filtering and flip image right-side up
	for(uint y = 0; y < _texHeight; y++) {
		for(uint x = 0; x < _texWidth; x++) {
			if((buffer[((_texHeight - y) * _texWidth) + x] & 0x7FFF) != 0x71CF) { // Don't include 0xF1CF or 0x71CF
				_texture[(y * _texWidth) + x] = bmpToDS(buffer[((_texHeight - y) * _texWidth) + x]);
			}
		}
	}
}

void Texture::loadPNG(const std::string &path) {
	std::vector<unsigned char> buffer;
	unsigned width, height;
	lodepng::decode(buffer, width, height, path);
	_texWidth = width;
	_texHeight = height;
	_texLength = (_texWidth * _texHeight) / 2;

	// Convert to DS bitmap format
	_texture = std::make_unique<u16[]>(_texWidth * _texHeight);
	for(uint i=0;i<buffer.size()/4;i++) {
		if(buffer[(i * 4) + 3] == 0xFF) { // Only keep full opacity pixels
			_texture[i] = bmpToDS((buffer[i * 4] >> 3) << 10 | (buffer[(i * 4) + 1] >> 3) << 5 | buffer[(i * 4) + 2] >> 3);
		}
	}
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

	_texture = std::make_unique<u16[]>(_texLength);
	fread(_texture.get(), sizeof(u16), _texLength, file);

	// Skip 'P' 'A' 'L'[datasize]
	// todo: verify
	fseek(file, 2 * sizeof(u32), SEEK_CUR);
	u32 paletteLength = 0;
	fread(&paletteLength, sizeof(u32), 1, file);
	_paletteLength = (paletteLength >> 9); // palette length in shorts. / sizoef(unsighed shor)

	_palette = std::make_unique<u16[]>(_paletteLength);
	fread(_palette.get(), sizeof(u16), _paletteLength, file);
}


void Texture::loadCompressed(FILE *file) noexcept {

	GrfHeader header;
	fseek(file, 5 * sizeof(u32), SEEK_SET);
	fread(&header, sizeof(GrfHeader), 1, file);

	_texHeight = header.texHeight;
	_texWidth = header.texWidth;

	// Skip 'G' 'F' 'X'[datasize]
	// todo: verify

	fseek(file, sizeof(u32), SEEK_CUR);
	// read chunk length
	fread(&_texCmpLength, sizeof(u32), 1, file); // this includes size of the header word

	_texture = std::make_unique<u16[]>(_texCmpLength >> 1);
	fread(_texture.get(), sizeof(u8), _texCmpLength, file);
	
	_texLength = (*((u32*)_texture.get()) >> 9); // palette length in ints sizeof(unsigned int);  
}

void Texture::applyPaletteEffect(Texture::PaletteEffect effect) {
	if (_type == TextureType::PalettedGrf) {
		effect(_palette.get(), _paletteLength);
	}
}

void Texture::applyBitmapEffect(Texture::BitmapEffect effect) {
	if (_type == TextureType::BMP) {
		effect(_texture.get(), _texLength);
	}
}

u16 Texture::bmpToDS(u16 val) {
	int blfLevel = ms().blfLevel;
	if (ms().colorMode == 1) {
		u8 b = val & 31;
		u8 g = (val >> 5) & 31;
		u8 r = (val >> 10) & 31;

		// Value decomposition of hsv
		u8 max = std::max(std::max(b, g), r);
		u8 min = std::min(std::min(b, g), r);

		// Desaturate
		max = (max + min) / 2;

		return max | (max & (31 - 3 * blfLevel)) << 5 | (max & (31 - 6 * blfLevel)) << 10 | BIT(15);
	} else {
		return ((val >> 10) & 31) | ((val >> 5) & (31 - 3 * blfLevel) << 5) | (val & (31 - 6 * blfLevel)) << 10 | BIT(15);
	}
}
