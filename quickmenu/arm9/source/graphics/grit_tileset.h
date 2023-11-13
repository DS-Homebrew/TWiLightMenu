#ifndef GRIT_TILESET_H
#define GRIT_TILESET_H

#include <array>
#include <algorithm>
#include <gl2d.h>

template<size_t sprite_width, size_t sprite_height, size_t bitmap_width, size_t bitmap_height, size_t pallette_length, size_t count>
struct StaticTileset {
	template<size_t BitmapArrayLen, size_t PaletteArrayLen>
	StaticTileset(const unsigned int(&bitmap_)[BitmapArrayLen], const unsigned short(&pallette_)[PaletteArrayLen]) : bitmap(bitmap_), pallette(pallette_) {
		static_assert((sprite_width * sprite_height) * count == bitmap_width * bitmap_height);
		static_assert(BitmapArrayLen * 8 == bitmap_width * bitmap_height);
	}
	//grit data
	const unsigned int* bitmap;
	const unsigned short* pallette;

	//gpu data
	int id;
	std::array<glImage, count> images;
};

template<size_t size>
static constexpr auto mapTextureSize() {
	if constexpr(size == 8)
		return TEXTURE_SIZE_8;
	else if constexpr(size == 16)
		return TEXTURE_SIZE_16;
	else if constexpr(size == 32)
		return TEXTURE_SIZE_32;
	else if constexpr(size == 64)
		return TEXTURE_SIZE_64;
	else if constexpr(size == 128)
		return TEXTURE_SIZE_128;
	else if constexpr(size == 256)
		return TEXTURE_SIZE_256;
	else if constexpr(size == 512)
		return TEXTURE_SIZE_512;
	else if constexpr(size == 1024)
		return TEXTURE_SIZE_1024;
}

template<size_t sprite_width, size_t sprite_height, size_t bitmap_width, size_t bitmap_height, size_t pallette_length, size_t count>
auto LoadTileset(StaticTileset<sprite_width, sprite_height, bitmap_width, bitmap_height, pallette_length, count>& texture,
				u16* pallette_transform = nullptr,
				u16* custom_pallette = nullptr) {
	
	constexpr auto textureSize = std::make_pair(mapTextureSize<bitmap_width>(), mapTextureSize<bitmap_height>());
	
	auto pallette = custom_pallette ? custom_pallette : (u16*)texture.pallette;
	
	if (pallette_transform) {
		std::for_each(pallette, pallette + pallette_length, [pallette_transform](auto& color){ color = pallette_transform[color]; });
	}

	texture.id = glLoadTileSet(texture.images.data(),
							sprite_width,
							sprite_height,
							bitmap_width,
							bitmap_height,
							GL_RGB16,
							textureSize.first,
							textureSize.second,
							TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT,
							pallette_length,
							pallette,
							(const u8*) texture.bitmap
							);
}

#define GRIT_TILESET(name, sprite_width, sprite_height, bitmap_width, bitmap_height, count) StaticTileset<sprite_width, sprite_height, bitmap_width, bitmap_height, name##PalLen/sizeof(*name##Pal), count> name{name##Bitmap, name##Pal}

#endif //GRIT_TILESET_H