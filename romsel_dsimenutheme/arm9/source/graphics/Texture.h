#pragma once
#ifndef __TWILIGHTMENU_TEXTURES_CLASS__
#define __TWILIGHTMENU_TEXTURES_CLASS__

#define BMP_ID(a,b)         ((u16)( (a) | (b)<<8 ))
#define CHUNK_ID(a,b,c,d)   ((u32)( (a) | (b)<<8 | (c)<<16 | (d)<<24 ))

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

	// Bitfields
	Bitmap = 1,
	Paletted = 2,
	Compressed = 4,

	// Types
	CompressedGrf = 8 | Compressed,
	PalettedGrf = 8 | Paletted,
	Bmp = 16 | Bitmap,
	PalettedBmp = 16 | Paletted,
	Png = 32 | Bitmap,
};

class Texture
{
	typedef void (*PaletteEffect)(u16* palette, u8 paletteLength);
	typedef void (*BitmapEffect)(u16* texture, u32 texLength);

	constexpr const static char *extensions[] = {".grf", ".png", ".bmp"};

	private:
		unique_ptr<u16[]> _palette;
		unique_ptr<u16[]> _texture;
		u8 _paletteLength;
		u32 _texLength; // in words
		u32 _texCmpLength;
		u32 _texHeight;
		u32 _texWidth;
		TextureType _type;
	
	public:
		Texture(const std::string& filePath, const std::string& fallback) noexcept;
		Texture(const Texture &) = delete;
		Texture(Texture&&) = default;

		virtual ~Texture() = default;

		void applyPaletteEffect(PaletteEffect effect);
		void applyBitmapEffect(BitmapEffect effect);
		void applyUserPaletteFile(const std::string &filePath, PaletteEffect fallbackEffect);

		static u16 bmpToDS(u16 val);

		const u16 *palette() const { return _type & TextureType::Paletted ? (u16*)_palette.get() : NULL; }
		const u16 *texture() const { return _texture.get(); }
		const u8 *bytes() const { return (u8*)_texture.get(); }

		u32 texHeight() const { return _texHeight; };
		u32 texWidth() const { return _texWidth; };
		u32 pixelCount() const { return _texHeight * _texWidth; };
		u8 paletteLength() const { return _type & TextureType::Paletted ? _paletteLength : 0;};
		u32 texLength() const { return _texLength; };
		u32 texCmpLength() const { return _texCmpLength; };

		TextureType type() const { return _type; }

		static bool exists(const std::string &filePath);

		void copy(u16 *dst, bool vram) const;

	private:
		TextureType findType(FILE* file);
		void loadBitmap(FILE* file) noexcept;
		void loadPaletted(FILE* file) noexcept;
		void loadCompressed(FILE* file) noexcept;
		void loadPNG(const std::string &path) noexcept;
};


#endif