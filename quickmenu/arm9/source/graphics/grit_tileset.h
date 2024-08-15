#ifndef GRIT_TILESET_H
#define GRIT_TILESET_H

#include <array>
#include <algorithm>
#include <gl2d.h>

template<size_t sprite_width, size_t sprite_height, size_t bitmap_width, size_t bitmap_height, size_t palette_length, size_t count>
struct GritBitmap {
	template<size_t BitmapArrayLen, size_t PaletteArrayLen>
	GritBitmap(const unsigned int(&bitmap_)[BitmapArrayLen], const unsigned short(&palette_)[PaletteArrayLen]) : bitmap(bitmap_), palette(palette_) {
		static_assert((sprite_width * sprite_height) * count == bitmap_width * bitmap_height);
		static_assert(BitmapArrayLen * 8 == bitmap_width * bitmap_height);
	}
	template<size_t PaletteArrayLen>
	GritBitmap(const unsigned short(&palette_)[PaletteArrayLen]) : palette(palette_) {
		static_assert((sprite_width * sprite_height) * count == bitmap_width * bitmap_height);
	}
	//grit data
	const unsigned int* bitmap;
	const unsigned short* palette;
};

template<size_t sprite_width, size_t sprite_height, size_t bitmap_width, size_t bitmap_height, size_t palette_length, size_t count>
struct GritTexture final : public GritBitmap<sprite_width, sprite_height, bitmap_width, bitmap_height, palette_length, count> {
	template<size_t BitmapArrayLen, size_t PaletteArrayLen>
	GritTexture(const unsigned int(&bitmap_)[BitmapArrayLen], const unsigned short(&palette_)[PaletteArrayLen]) : GritBitmap<sprite_width, sprite_height, bitmap_width, bitmap_height, palette_length, count>{bitmap_, palette_} {
	}
	template<size_t PaletteArrayLen>
	GritTexture(const unsigned short(&palette_)[PaletteArrayLen]) : GritBitmap<sprite_width, sprite_height, bitmap_width, bitmap_height, palette_length, count>{palette_} {
	}
	//gpu data
	int id;
	std::array<glImage, count> images;
};

template<size_t size>
constexpr auto mapTextureSize() {
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

template<size_t sprite_width, size_t sprite_height, size_t bitmap_width, size_t bitmap_height, size_t palette_length, size_t count>
inline auto LoadTilesetTexture(GritBitmap<sprite_width, sprite_height, bitmap_width, bitmap_height, palette_length, count>& tileset,
				glImage* images_array) {
	
	constexpr auto textureSize = std::make_pair(mapTextureSize<bitmap_width>(), mapTextureSize<bitmap_height>());
	
	return glLoadTileSet(images_array,
							sprite_width,
							sprite_height,
							bitmap_width,
							bitmap_height,
							GL_RGB16,
							textureSize.first,
							textureSize.second,
							TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT,
							palette_length,
							(u16*)tileset.palette,
							(const u8*) tileset.bitmap
							);
}

template<size_t sprite_width, size_t sprite_height, size_t bitmap_width, size_t bitmap_height, size_t palette_length, size_t count>
inline auto LoadTilesetTextureCachePalette(GritBitmap<sprite_width, sprite_height, bitmap_width, bitmap_height, palette_length, count>& tileset,
				glImage* images_array, u16* paletteCache) {
	
	swiCopy(tileset.palette, paletteCache, 4 * sizeof(u16) | COPY_MODE_COPY | COPY_MODE_WORD);

	constexpr auto textureSize = std::make_pair(mapTextureSize<bitmap_width>(), mapTextureSize<bitmap_height>());
	
	return glLoadTileSet(images_array,
							sprite_width,
							sprite_height,
							bitmap_width,
							bitmap_height,
							GL_RGB16,
							textureSize.first,
							textureSize.second,
							TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT,
							palette_length,
							(u16*)tileset.palette,
							(const u8*) tileset.bitmap
							);
}


template<size_t sprite_width, size_t sprite_height, size_t bitmap_width, size_t bitmap_height, size_t palette_length, size_t count>
inline auto LoadTileset(GritTexture<sprite_width, sprite_height, bitmap_width, bitmap_height, palette_length, count>& texture) {

	texture.id = LoadTilesetTexture(texture, texture.images.data());
}

template<size_t sprite_width, size_t sprite_height, size_t bitmap_width, size_t bitmap_height, size_t palette_length, size_t count>
inline auto RemapPalette(GritBitmap<sprite_width, sprite_height, bitmap_width, bitmap_height, palette_length, count>& tileset, u16* paletteTable) {
	std::for_each((u16*)tileset.palette, (u16*)tileset.palette + palette_length, [paletteTable](auto& color){ color = paletteTable[color]; });
}

#define GRIT_BITMAP(name, sprite_width, sprite_height, bitmap_width, bitmap_height, count) GritBitmap<sprite_width, sprite_height, bitmap_width, bitmap_height, name##PalLen/sizeof(*name##Pal), count> name{name##Bitmap, name##Pal}
#define GRIT_TEXTURE(name, sprite_width, sprite_height, bitmap_width, bitmap_height, count) GritTexture<sprite_width, sprite_height, bitmap_width, bitmap_height, name##PalLen/sizeof(*name##Pal), count> name{name##Bitmap, name##Pal}

#endif //GRIT_TILESET_H