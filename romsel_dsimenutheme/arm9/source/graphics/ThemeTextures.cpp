#include "ThemeTextures.h"

#include <nds.h>

#include "common/dsimenusettings.h"
#include "common/systemdetails.h"

#include "paletteEffects.h"
#include "themefilenames.h"
#include "tool/colortool.h"
// Graphic files
#include "../include/startborderpal.h"

#include "tool/stringtool.h"
#include "uvcoord_date_time_font.h"
#include "uvcoord_top_font.h"

extern u16 usernameRendered[10];

void ThemeTextures::loadBubbleImage(const unsigned short *palette, const unsigned int *bitmap, int sprW, int sprH,
				    int texW) {
	_bubbleImage = std::move(loadTexture(&bubbleTexID, palette, bitmap, 1, 12, sprW, sprH, texW, 8));
}

void ThemeTextures::loadProgressImage(const unsigned short *palette, const unsigned int *bitmap) {
	_progressImage =
	    std::move(loadTexture(&progressTexID, palette, bitmap, (16 / 16) * (128 / 16), 9, 16, 16, 16, 128));
}

void ThemeTextures::loadDialogboxImage(const unsigned short *palette, const unsigned int *bitmap) {
	_dialogboxImage =
	    std::move(loadTexture(&dialogboxTexID, palette, bitmap, (256 / 16) * (256 / 16), 12, 16, 16, 256, 192));
}

void ThemeTextures::loadBipsImage(const unsigned short *palette, const unsigned int *bitmap) {
	_bipsImage = std::move(loadTexture(&bipsTexID, palette, bitmap, (8 / 8) * (32 / 8), 16, 8, 8, 8, 32));
}

void ThemeTextures::loadScrollwindowImage(const unsigned short *palette, const unsigned int *bitmap) {
	_scrollwindowImage =
	    std::move(loadTexture(&scrollwindowTexID, palette, bitmap, (32 / 16) * (32 / 16), 16, 32, 32, 32, 32));
}

void ThemeTextures::loadButtonarrowImage(const unsigned short *palette, const unsigned int *bitmap) {
	_buttonarrowImage =
	    std::move(loadTexture(&buttonarrowTexID, palette, bitmap, (32 / 32) * (128 / 32), 16, 32, 32, 32, 128));
}

void ThemeTextures::loadMovingarrowImage(const unsigned short *palette, const unsigned int *bitmap) {
	_movingarrowImage =
	    std::move(loadTexture(&movingarrowTexID, palette, bitmap, (32 / 32) * (32 / 32), 16, 32, 32, 32, 32));
}

void ThemeTextures::loadLaunchdotImage(const unsigned short *palette, const unsigned int *bitmap) {
	_launchdotImage =
	    std::move(loadTexture(&launchdotTexID, palette, bitmap, (16 / 16) * (96 / 16), 16, 16, 16, 16, 96));
}

void ThemeTextures::loadStartImage(const unsigned short *palette, const unsigned int *bitmap) {
	_startImage = std::move(loadTexture(&startTexID, palette, bitmap, (64 / 16) * (128 / 16), 4, 64, 16, 64, 128));
}

void ThemeTextures::loadStartbrdImage(const unsigned short *palette, const unsigned int *bitmap, int arraysize,
				      int palLength, int sprH, int texH) {
	_startbrdImage =
	    std::move(loadTexture(&startbrdTexID, palette, bitmap, arraysize, palLength, 32, sprH, 32, texH));
}
void ThemeTextures::loadBraceImage(const unsigned short *palette, const unsigned int *bitmap) {
	_braceImage = std::move(loadTexture(&braceTexID, palette, bitmap, (16 / 16) * (128 / 16), 4, 16, 128, 16, 128));
}

void ThemeTextures::loadSettingsImage(const unsigned short *palette, const unsigned int *bitmap) {
	_settingsImage =
	    std::move(loadTexture(&settingsTexID, palette, bitmap, (64 / 16) * (128 / 64), 16, 64, 64, 64, 128));
}

void ThemeTextures::loadBoxfullImage(const unsigned short *palette, const unsigned int *bitmap) {
	_boxfullImage =
	    std::move(loadTexture(&boxfullTexID, palette, bitmap, (64 / 16) * (128 / 64), 16, 64, 64, 64, 128));
}

void ThemeTextures::loadBoxemptyImage(const unsigned short *palette, const unsigned int *bitmap) {
	_boxemptyImage =
	    std::move(loadTexture(&boxemptyTexID, palette, bitmap, (64 / 16) * (64 / 16), 16, 64, 64, 64, 64));
}

void ThemeTextures::loadFolderImage(const unsigned short *palette, const unsigned int *bitmap) {
	_folderImage = std::move(loadTexture(&folderTexID, palette, bitmap, (64 / 16) * (64 / 16), 16, 64, 64, 64, 64));
}

void ThemeTextures::loadCornerButtonImage(const unsigned short *palette, const unsigned int *bitmap, int arraysize,
					  int sprW, int sprH, int texW, int texH) {
	_cornerButtonImage =
	    std::move(loadTexture(&cornerButtonTexID, palette, bitmap, arraysize, 16, sprW, sprH, texW, texH));
}

void ThemeTextures::loadSmallCartImage(const unsigned short *palette, const unsigned int *bitmap) {
	_smallCartImage =
	    std::move(loadTexture(&smallCartTexID, palette, bitmap, (32 / 16) * (256 / 32), 16, 32, 32, 32, 256));
}

void ThemeTextures::loadWirelessIcons(const unsigned short *palette, const unsigned int *bitmap) {
	_wirelessIcons =
	    std::move(loadTexture(&wirelessiconTexID, palette, bitmap, (32 / 32) * (64 / 32), 7, 32, 32, 32, 64));
}

inline GL_TEXTURE_SIZE_ENUM get_tex_size(int texSize) {
	if (texSize <= 8)
		return TEXTURE_SIZE_8;
	if (texSize <= 16)
		return TEXTURE_SIZE_16;
	if (texSize <= 32)
		return TEXTURE_SIZE_32;
	if (texSize <= 64)
		return TEXTURE_SIZE_64;
	if (texSize <= 128)
		return TEXTURE_SIZE_128;
	if (texSize <= 256)
		return TEXTURE_SIZE_256;
	if (texSize <= 512)
		return TEXTURE_SIZE_512;
	return TEXTURE_SIZE_1024;
}

inline const unsigned short *apply_personal_theme(const unsigned short *palette) {
	return palette + ((PersonalData->theme) * 16);
}

unique_ptr<glImage[]> ThemeTextures::loadTexture(int *textureId, const unsigned short *palette,
						 const unsigned int *bitmap, unsigned int arraySize, int paletteLength,
						 int sprW, int sprH, int texW, int texH) {

	// We need to delete the texture since the resource held by the unique pointer will be
	// immediately dropped when we assign it to the pointer.

	if (*textureId != 0) {
		nocashMessage("Existing texture found!?");
		glDeleteTextures(1, textureId);
	}

	// Do a heap allocation of arraySize glImage
	unique_ptr<glImage[]> texturePtr = std::make_unique<glImage[]>(arraySize);

	u16 *newPalette = (u16 *)palette;

	if (ms().colorMode == 1) {
		// Convert palette to grayscale
		for (int i2 = 0; i2 < paletteLength; i2++) {
			*(newPalette + i2) = convertVramColorToGrayscale(*(newPalette + i2));
		}
	}

	// Load the texture here.
	*textureId = glLoadTileSet(texturePtr.get(),   // pointer to glImage array
				   sprW,	       // sprite width
				   sprH,	       // sprite height
				   texW,	       // bitmap width
				   texH,	       // bitmap height
				   GL_RGB16,	   // texture type for glTexImage2D() in videoGL.h
				   get_tex_size(texW), // sizeX for glTexImage2D() in videoGL.h
				   get_tex_size(texH), // sizeY for glTexImage2D() in videoGL.h
				   TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
				   paletteLength,     // Length of the palette to use (16 colors)
				   (u16 *)newPalette, // Load our 16 color tiles palette
				   (u8 *)bitmap       // image data generated by GRIT
	);
	return texturePtr;
}

void ThemeTextures::reloadPalDialogBox() {
	glBindTexture(0, dialogboxTexID);
	glColorSubTableEXT(0, 0, 12, 0, 0, _dialogBoxTexture->palette());
	if (ms().theme != 1) {
		glBindTexture(0, cornerButtonTexID);
		glColorSubTableEXT(0, 0, 16, 0, 0, _cornerButtonTexture->palette());
	}
}

void ThemeTextures::reloadPal3dsCornerButton() {
	glBindTexture(0, cornerButtonTexID);
	glColorSubTableEXT(0, 0, 16, 0, 0, _cornerButtonTexture->palette());
}

void ThemeTextures::loadBottomImage() {

	{
		const u16 *src = _bottomBackgroundTexture->texture();
		int x = 0;
		int y = 191;
		for (int i = 0; i < 256 * 192; i++) {
			if (x >= 256) {
				x = 0;
				y--;
			}
			u16 val = *(src++);
			_bottomBgImage[y * 256 + x] = convertToDsBmp(val);
			x++;
		}
	}

	{
		const u16 *src = _bottomBackgroundBubbleTexture->texture();
		int x = 0;
		int y = 191;
		for (int i = 0; i < 256 * 192; i++) {
			if (x >= 256) {
				x = 0;
				y--;
			}
			u16 val = *(src++);
			_bottomBubbleBgImage[y * 256 + x] = convertToDsBmp(val);
			x++;
		}
	}
}

void ThemeTextures::load3DSTheme() {

	loadUiTextures();
	loadVolumeTextures();
	loadBatteryTextures();

	loadBottomImage();
	loadDateFont(_dateTimeFontTexture->texture());

	_bubbleTexture = std::make_unique<GritTexture>(TFN_GRF_BUBBLE, TFN_FALLBACK_GRF_BUBBLE);
	_bubbleTexture = std::make_unique<GritTexture>(TFN_GRF_BUBBLE, TFN_FALLBACK_GRF_BUBBLE);
	_settingsIconTexture = std::make_unique<GritTexture>(TFN_GRF_ICON_SETTINGS, TFN_FALLBACK_GRF_ICON_SETTINGS);

	_boxFullTexture = std::make_unique<GritTexture>(TFN_GRF_BOX_FULL, TFN_FALLBACK_GRF_BOX_FULL);
	_boxEmptyTexture = std::make_unique<GritTexture>(TFN_GRF_BOX_EMPTY, TFN_FALLBACK_GRF_BOX_EMPTY);
	_folderTexture = std::make_unique<GritTexture>(TFN_GRF_FOLDER, TFN_FALLBACK_GRF_FOLDER);
	_progressTexture = std::make_unique<GritTexture>(TFN_GRF_PROGRESS, TFN_FALLBACK_GRF_PROGRESS);

	_cornerButtonTexture = std::make_unique<GritTexture>(TFN_GRF_CORNERBUTTON, TFN_FALLBACK_GRF_CORNERBUTTON);
	_smallCartTexture = std::make_unique<GritTexture>(TFN_GRF_SMALL_CART, TFN_FALLBACK_GRF_SMALL_CART);
	_startBorderTexture = std::make_unique<GritTexture>(TFN_GRF_CURSOR, TFN_FALLBACK_GRF_CURSOR);
	_dialogBoxTexture = std::make_unique<GritTexture>(TFN_GRF_DIALOGBOX, TFN_FALLBACK_GRF_DIALOGBOX);

	loadBubbleImage(_bubbleTexture->palette(), (const unsigned int *)_bubbleTexture->texture(), 7, 7, 8);
	loadSettingsImage(_settingsIconTexture->palette(), (const unsigned int *)_settingsIconTexture->texture());

	loadBoxfullImage(_boxFullTexture->palette(), (const unsigned int *)_boxFullTexture->texture());
	loadBoxemptyImage(_boxEmptyTexture->palette(), (const unsigned int *)_boxEmptyTexture->texture());
	loadFolderImage(_folderTexture->palette(), (const unsigned int *)_folderTexture->texture());

	loadCornerButtonImage(_cornerButtonTexture->palette(), (const unsigned int *)_cornerButtonTexture->texture(),
			      (64 / 16) * (64 / 32), 64, 32, 64, 64);
	loadSmallCartImage(_smallCartTexture->palette(), (const unsigned int *)_smallCartTexture->texture());
	loadStartbrdImage(_startBorderTexture->palette(), (const unsigned int *)_startBorderTexture->texture(),
			  (32 / 32) * (192 / 64), 6, 64, 192);
	loadDialogboxImage(_dialogBoxTexture->palette(), (const unsigned int *)_dialogBoxTexture->texture());
	loadProgressImage(_progressTexture->palette(), (const unsigned int *)_progressTexture->texture());

}

void ThemeTextures::loadDSiTheme() {

	loadUiTextures();
	loadVolumeTextures();
	loadBatteryTextures();
	loadBottomImage();
	
	loadDateFont(_dateTimeFontTexture->texture());

	_bipsTexture = std::make_unique<GritTexture>(TFN_GRF_BIPS, TFN_FALLBACK_GRF_BIPS);
	_boxTexture = std::make_unique<GritTexture>(TFN_GRF_BOX, TFN_FALLBACK_GRF_BOX);
	_braceTexture = std::make_unique<GritTexture>(TFN_GRF_BRACE, TFN_FALLBACK_GRF_BRACE);
	_bubbleTexture = std::make_unique<GritTexture>(TFN_GRF_BUBBLE, TFN_FALLBACK_GRF_BUBBLE);
	_buttonArrowTexture = std::make_unique<GritTexture>(TFN_GRF_BUTTON_ARROW, TFN_FALLBACK_GRF_BUTTON_ARROW);
	_cornerButtonTexture = std::make_unique<GritTexture>(TFN_GRF_CORNERBUTTON, TFN_FALLBACK_GRF_CORNERBUTTON);

	_dialogBoxTexture = std::make_unique<GritTexture>(TFN_GRF_DIALOGBOX, TFN_FALLBACK_GRF_DIALOGBOX);

	_folderTexture = std::make_unique<GritTexture>(TFN_GRF_FOLDER, TFN_FALLBACK_GRF_FOLDER);
	_launchDotTexture = std::make_unique<GritTexture>(TFN_GRF_LAUNCH_DOT, TFN_FALLBACK_GRF_LAUNCH_DOT);
	_movingArrowTexture = std::make_unique<GritTexture>(TFN_GRF_MOVING_ARROW, TFN_FALLBACK_GRF_MOVING_ARROW);

	_progressTexture = std::make_unique<GritTexture>(TFN_GRF_PROGRESS, TFN_FALLBACK_GRF_PROGRESS);
	_scrollWindowTexture = std::make_unique<GritTexture>(TFN_GRF_SCROLL_WINDOW, TFN_FALLBACK_GRF_SCROLL_WINDOW);
	_smallCartTexture = std::make_unique<GritTexture>(TFN_GRF_SMALL_CART, TFN_FALLBACK_GRF_SMALL_CART);
	_startBorderTexture = std::make_unique<GritTexture>(TFN_GRF_START_BORDER, TFN_FALLBACK_GRF_START_BORDER);
	_startTextTexture = std::make_unique<GritTexture>(TFN_GRF_START_TEXT, TFN_FALLBACK_GRF_START_TEXT);
	_wirelessIconsTexture = std::make_unique<GritTexture>(TFN_GRF_WIRELESSICONS, TFN_FALLBACK_GRF_WIRELESSICONS);
	_settingsIconTexture = std::make_unique<GritTexture>(TFN_GRF_ICON_SETTINGS, TFN_FALLBACK_GRF_ICON_SETTINGS);

	loadBipsImage(_bipsTexture->palette(), (const unsigned int *)_bipsTexture->texture());

	loadBubbleImage(_bubbleTexture->palette(), (const unsigned int *)_bubbleTexture->texture(), 11, 8, 16);
	loadScrollwindowImage(_scrollWindowTexture->palette(), (const unsigned int *)_scrollWindowTexture->texture());
	loadWirelessIcons(_wirelessIconsTexture->palette(), (const unsigned int *)_wirelessIconsTexture->texture());
	loadSettingsImage(_settingsIconTexture->palette(), (const unsigned int *)_settingsIconTexture->texture());
	loadBraceImage(_braceTexture->palette(), (const unsigned int *)_braceTexture->texture());

	_startTextTexture->applyEffect(effectDSiStartTextPalettes);
	_startBorderTexture->applyEffect(effectDSiStartBorderPalettes);

	_buttonArrowTexture->applyEffect(effectDSiArrowButtonPalettes);
	_movingArrowTexture->applyEffect(effectDSiArrowButtonPalettes);
	_launchDotTexture->applyEffect(effectDSiArrowButtonPalettes);
	_dialogBoxTexture->applyEffect(effectDSiArrowButtonPalettes);

	loadStartImage(_startTextTexture->palette(), (const unsigned int *)_startTextTexture->texture());
	loadStartbrdImage(_startBorderTexture->palette(), (const unsigned int *)_startBorderTexture->texture(),
			  (32 / 32) * (256 / 80), 16, 80, 256);

	loadButtonarrowImage(_buttonArrowTexture->palette(), (const unsigned int *)_buttonArrowTexture->texture());
	loadMovingarrowImage(_movingArrowTexture->palette(), (const unsigned int *)_movingArrowTexture->texture());
	loadLaunchdotImage(_launchDotTexture->palette(), (const unsigned int *)_launchDotTexture->texture());
	loadDialogboxImage(_dialogBoxTexture->palette(), (const unsigned int *)_dialogBoxTexture->texture());

	loadBoxfullImage(_boxTexture->palette(), (const unsigned int *)_boxTexture->texture());

	loadCornerButtonImage(_cornerButtonTexture->palette(), (const unsigned int *)_cornerButtonTexture->texture(),
			      (32 / 16) * (32 / 32), 32, 32, 32, 64);
	loadSmallCartImage(_smallCartTexture->palette(), (const unsigned int *)_smallCartTexture->texture());
	loadFolderImage(_folderTexture->palette(), (const unsigned int *)_folderTexture->texture());

	loadProgressImage(_progressTexture->palette(), (const unsigned int *)_progressTexture->texture());
	loadWirelessIcons(_wirelessIconsTexture->palette(), (const unsigned int *)_wirelessIconsTexture->texture());
}

void ThemeTextures::loadVolumeTextures() {
	_volume0Texture = std::make_unique<BmpTexture>(TFN_VOLUME0, TFN_FALLBACK_VOLUME0);
	_volume1Texture = std::make_unique<BmpTexture>(TFN_VOLUME1, TFN_FALLBACK_VOLUME1);
	_volume2Texture = std::make_unique<BmpTexture>(TFN_VOLUME2, TFN_FALLBACK_VOLUME2);
	_volume3Texture = std::make_unique<BmpTexture>(TFN_VOLUME3, TFN_FALLBACK_VOLUME3);
	_volume4Texture = std::make_unique<BmpTexture>(TFN_VOLUME4, TFN_FALLBACK_VOLUME4);
}

void ThemeTextures::loadBatteryTextures() {
	_battery1Texture = std::make_unique<BmpTexture>(TFN_BATTERY1, TFN_FALLBACK_BATTERY1);
	_battery2Texture = std::make_unique<BmpTexture>(TFN_BATTERY2, TFN_FALLBACK_BATTERY2);
	_battery3Texture = std::make_unique<BmpTexture>(TFN_BATTERY3, TFN_FALLBACK_BATTERY3);
	_battery4Texture = std::make_unique<BmpTexture>(TFN_BATTERY4, TFN_FALLBACK_BATTERY4);

	_batterychargeTexture = std::make_unique<BmpTexture>(TFN_BATTERY_CHARGE, TFN_FALLBACK_BATTERY_CHARGE);
	_batteryfullTexture = std::make_unique<BmpTexture>(TFN_BATTERY_FULL, TFN_FALLBACK_BATTERY_FULL);
	_batteryfullDSTexture = std::make_unique<BmpTexture>(TFN_BATTERY_FULLDS, TFN_FALLBACK_BATTERY_FULLDS);
	_batterylowTexture = std::make_unique<BmpTexture>(TFN_BATTERY_LOW, TFN_FALLBACK_BATTERY_LOW);
}

void ThemeTextures::loadUiTextures() {

	_topBackgroundTexture = std::make_unique<BmpTexture>(TFN_UI_TOPBG, TFN_FALLBACK_UI_TOPBG);
	_bottomBackgroundTexture = std::make_unique<BmpTexture>(TFN_UI_BOTTOMBG, TFN_FALLBACK_UI_BOTTOMBG);
	_bottomBackgroundBubbleTexture =
	    std::make_unique<BmpTexture>(TFN_UI_BOTTOMBUBBLEBG, TFN_FALLBACK_UI_BOTTOMBUBBLEBG);
	_dateTimeFontTexture = std::make_unique<BmpTexture>(TFN_UI_DATE_TIME_FONT, TFN_FALLBACK_UI_DATE_TIME_FONT);

	_leftShoulderTexture = std::make_unique<BmpTexture>(TFN_UI_LSHOULDER, TFN_FALLBACK_UI_LSHOULDER);
	_rightShoulderTexture = std::make_unique<BmpTexture>(TFN_UI_RSHOULDER, TFN_FALLBACK_UI_RSHOULDER);
	_leftShoulderGreyedTexture =
	    std::make_unique<BmpTexture>(TFN_UI_LSHOULDER_GREYED, TFN_FALLBACK_UI_LSHOULDER_GREYED);
	_rightShoulderGreyedTexture =
	    std::make_unique<BmpTexture>(TFN_UI_RSHOULDER_GREYED, TFN_FALLBACK_UI_RSHOULDER_GREYED);
}

u16 *ThemeTextures::beginSubModify() {
	dmaCopyWords(0, BG_GFX_SUB, _bgSubBuffer.get(), sizeof(u16) * BG_BUFFER_PIXELCOUNT);
	return _bgSubBuffer.get();
}

void ThemeTextures::commitSubModify() {
	DC_FlushRange(_bgSubBuffer.get(), sizeof(u16) * BG_BUFFER_PIXELCOUNT);
	dmaCopyWords(2, _bgSubBuffer.get(), BG_GFX_SUB, sizeof(u16) * BG_BUFFER_PIXELCOUNT);
}

void ThemeTextures::commitSubModifyAsync() {
	DC_FlushRange(_bgSubBuffer.get(), sizeof(u16) * BG_BUFFER_PIXELCOUNT);
	dmaCopyWordsAsynch(2, _bgSubBuffer.get(), BG_GFX_SUB, sizeof(u16) * BG_BUFFER_PIXELCOUNT);
}

void ThemeTextures::drawTopBg() {
	beginSubModify();
	const u16 *src = _topBackgroundTexture->texture();
	int x = 0;
	int y = 191;
	for (int i = 0; i < 256 * 192; i++) {
		if (x >= 256) {
			x = 0;
			y--;
		}
		u16 val = *(src++);
		if (val != 0xFC1F) { // Do not render magneta pixel

			_bgSubBuffer[y * 256 + x] = convertToDsBmp(val);
		}
		x++;
	}
	commitSubModify();
}

void ThemeTextures::drawBottomBg() {
	DC_FlushRange(_bottomBgImage.get(), 0x18000);
	dmaCopyWords(0, _bottomBgImage.get(), BG_GFX, 0x18000);
}

void ThemeTextures::drawBottomBubbleBg() {
	DC_FlushRange(_bottomBubbleBgImage.get(), 0x18000);
	dmaCopyWords(0, _bottomBubbleBgImage.get(), BG_GFX, 0x18000);
}

void ThemeTextures::clearTopScreen() {
	beginSubModify();
	u16 val = 0xFFFF;
	for (int i = 0; i < 256 * 192; i++) {
		_bgSubBuffer[i] = ((val >> 10) & 31) | (val & (31 - 3 * ms().blfLevel) << 5) |
				  (val & (31 - 6 * ms().blfLevel)) << 10 | BIT(15);
	}
	commitSubModify();
}

void ThemeTextures::drawProfileName() {
	// Load username
	char fontPath[64];
	FILE *file;
	int x = (isDSiMode() ? 28 : 4);

	for (int c = 0; c < 10; c++) {
		unsigned int charIndex = getTopFontSpriteIndex(usernameRendered[c]);
		// 42 characters per line.
		unsigned int texIndex = charIndex / 42;
		sprintf(fontPath, "nitro:/graphics/top_font/small_font_%u.bmp", texIndex);

		file = fopen(fontPath, "rb");

		if (file) {
			beginSubModify();
			// Start loading
			fseek(file, 0xe, SEEK_SET);
			u8 pixelStart = (u8)fgetc(file) + 0xe;
			fseek(file, pixelStart, SEEK_SET);
			for (int y = 15; y >= 0; y--) {
				u16 buffer[512];
				fread(buffer, 2, 0x200, file);
				u16 *src = buffer + (top_font_texcoords[0 + (4 * charIndex)]);

				for (u16 i = 0; i < top_font_texcoords[2 + (4 * charIndex)]; i++) {
					u16 val = *(src++);
					u16 bg = _bgSubBuffer[(y + 2) * 256 + (i + x)]; // grab the background pixel
					// Apply palette here.

					// Magic numbers were found by dumping val to stdout
					// on case default.
					switch (val) {
					// #ff00ff
					case 0xFC1F:
						break;
					// #414141
					case 0xA108:
						val = bmpPal_topSmallFont[1 + ((PersonalData->theme) * 16)];
						break;
					case 0xC210:
						// blend the colors with the background to make it look better.
						val = alphablend(bmpPal_topSmallFont[2 + ((PersonalData->theme) * 16)],
								 bg, 48);
						break;
					case 0xDEF7:
						val = alphablend(bmpPal_topSmallFont[3 + ((PersonalData->theme) * 16)],
								 bg, 64);
					default:
						break;
					}
					if (val != 0xFC1F) { // Do not render magneta pixel
						_bgSubBuffer[(y + 2) * 256 + (i + x)] = convertToDsBmp(val);
					}
				}
			}
			x += top_font_texcoords[2 + (4 * charIndex)];
			commitSubModify();
		}

		fclose(file);
	}
}

unsigned short ThemeTextures::convertToDsBmp(unsigned short val) {
	if (ms().colorMode == 1) {
		u16 newVal = ((val >> 10) & 31) | (val & 31 << 5) | (val & 31) << 10 | BIT(15);

		u8 b, g, r, max, min;
		b = ((newVal) >> 10) & 31;
		g = ((newVal) >> 5) & 31;
		r = (newVal)&31;
		// Value decomposition of hsv
		max = (b > g) ? b : g;
		max = (max > r) ? max : r;

		// Desaturate
		min = (b < g) ? b : g;
		min = (min < r) ? min : r;
		max = (max + min) / 2;

		newVal = 32768 | (max << 10) | (max << 5) | (max);

		b = ((newVal) >> 10) & (31 - 6 * ms().blfLevel);
		g = ((newVal) >> 5) & (31 - 3 * ms().blfLevel);
		r = (newVal)&31;

		return 32768 | (b << 10) | (g << 5) | (r);
	} else {
		return ((val >> 10) & 31) | (val & (31 - 3 * ms().blfLevel) << 5) |
		       (val & (31 - 6 * ms().blfLevel)) << 10 | BIT(15);
	}
}

/**
 * Get the index in the UV coordinate array where the letter appears
 */
unsigned int ThemeTextures::getTopFontSpriteIndex(const u16 letter) {
	unsigned int spriteIndex = 0;
	long int left = 0;
	long int right = TOP_FONT_NUM_IMAGES;
	long int mid = 0;

	while (left <= right) {
		mid = left + ((right - left) / 2);
		if (top_utf16_lookup_table[mid] == letter) {
			spriteIndex = mid;
			break;
		}

		if (top_utf16_lookup_table[mid] < letter) {
			left = mid + 1;
		} else {
			right = mid - 1;
		}
	}
	return spriteIndex;
}

void ThemeTextures::drawBoxArt(const char *filename) {
	FILE *file = fopen(filename, "rb");
	if (!file)
		file = fopen("nitro:/graphics/boxart_unknown.bmp", "rb");

	if (file) {
		// Start loading
		beginSubModify();
		fseek(file, 0xe, SEEK_SET);
		u8 pixelStart = (u8)fgetc(file) + 0xe;
		fseek(file, pixelStart, SEEK_SET);
		fread(_bmpImageBuffer.get(), 2, 0x7800, file);
		u16 *src = _bmpImageBuffer.get();
		int x = 64;
		int y = 40 + 114;
		for (int i = 0; i < 128 * 115; i++) {
			if (x >= 64 + 128) {
				x = 64;
				y--;
			}
			u16 val = *(src++);
			_bgSubBuffer[y * 256 + x] = convertToDsBmp(val);
			x++;
		}
		commitSubModify();
	}
	fclose(file);
}

void ThemeTextures::drawVolumeImage(int volumeLevel) {
	beginSubModify();

	const u16 *src = volumeTexture(volumeLevel)->texture();
	int x = 4;
	int y = 5 + 11;
	for (int i = 0; i < 18 * 12; i++) {
		if (x >= 4 + 18) {
			x = 4;
			y--;
		}
		u16 val = *(src++);
		if (val != 0x7C1F) { // Do not render magneta pixel
			_bgSubBuffer[y * 256 + x] = convertToDsBmp(val);
		}
		x++;
	}
	commitSubModify();
}

void ThemeTextures::drawVolumeImageCached() {
	int volumeLevel = getVolumeLevel();
	if (_cachedVolumeLevel != volumeLevel) {
		_cachedVolumeLevel = volumeLevel;
		drawVolumeImage(volumeLevel);
	}
}

int ThemeTextures::getVolumeLevel(void) {
	if (!isDSiMode())
		return -1;
	u8 volumeLevel = *(u8 *)(0x023FF000);
	if (volumeLevel == 0)
		return 0;
	if (volumeLevel > 0x00 && volumeLevel < 0x07)
		return 1;
	if (volumeLevel >= 0x07 && volumeLevel < 0x11)
		return 2;
	if (volumeLevel >= 0x11 && volumeLevel < 0x1C)
		return 3;
	if (volumeLevel >= 0x1C && volumeLevel < 0x20)
		return 4;
	return -1;
}

int ThemeTextures::getBatteryLevel(void) {
	u8 batteryLevel = *(u8 *)(0x023FF001);

	if (!isDSiMode()) {
		if (batteryLevel & BIT(0))
			return 1;
		return 0;
	}

	// DSi Mode
	if (batteryLevel & BIT(7))
		return 7;
	if (batteryLevel == 0xF)
		return 4;
	if (batteryLevel == 0xB)
		return 3;
	if (batteryLevel == 0x7)
		return 2;
	if (batteryLevel == 0x3 || batteryLevel == 0x1)
		return 1;
	return 0;
}

void ThemeTextures::drawBatteryImage(int batteryLevel, bool drawDSiMode, bool isRegularDS) {
	// Start loading
	beginSubModify();
	const u16 *src = batteryTexture(batteryLevel, drawDSiMode, isRegularDS)->texture();
	int x = 235;
	int y = 5 + 10;
	for (int i = 0; i < 18 * 11; i++) {
		if (x >= 235 + 18) {
			x = 235;
			y--;
		}
		u16 val = *(src++);
		if (val != 0x7C1F) { // Do not render magneta pixel
			_bgSubBuffer[y * 256 + x] = convertToDsBmp(val);
		}
		x++;
	}
	commitSubModify();
}

void ThemeTextures::drawBatteryImageCached() {
	int batteryLevel = getBatteryLevel();
	if (_cachedBatteryLevel != batteryLevel) {
		_cachedBatteryLevel = batteryLevel;
		drawBatteryImage(batteryLevel, isDSiMode(), sys().isRegularDS());
	}
}

// Load .bmp file without overwriting shoulder button images or username
void ThemeTextures::drawTopBgAvoidingShoulders() {
	beginSubModify();

	const u16 *src = _topBackgroundTexture->texture();
	int x = 0;
	int y = 191;
	for (int i = 0; i < 256 * 192; i++) {
		if (x >= 256) {
			x = 0;
			y--;
		}
		u16 val = *(src++);
		if (y >= 32 && y <= 167 && val != 0xFC1F) {
			_bgSubBuffer[y * 256 + x] = convertToDsBmp(val);
		}
		x++;
	}
	commitSubModify();
}

void ThemeTextures::drawShoulders(bool showLshoulder, bool showRshoulder) {

	beginSubModify();
	const u16 *rightSrc = showRshoulder ? _rightShoulderTexture->texture() : _rightShoulderGreyedTexture->texture();

	const u16 *leftSrc = showLshoulder ? _leftShoulderTexture->texture() : _leftShoulderGreyedTexture->texture();
	for (int y = 19; y >= 0; y--) {
		// Draw R Shoulders
		for (int i = 0; i < 78; i++) {
			u16 val = *(rightSrc++);
			if (val != 0xFC1F) { // Do not render magneta pixel
				_bgSubBuffer[(y + 172) * 256 + (i + 178)] = convertToDsBmp(val);
			}
		}
	}

	for (int y = 19; y >= 0; y--) {
		// Draw L Shoulders
		for (int i = 0; i < 78; i++) {
			u16 val = *(leftSrc++);
			if (val != 0xFC1F) { // Do not render magneta pixel
				_bgSubBuffer[(y + 172) * 256 + i] = convertToDsBmp(val);
			}
		}
	}
	commitSubModify();
}

void ThemeTextures::loadDateFont(const unsigned short *bitmap) {
	_dateFontImage = std::make_unique<u16[]>(128 * 16);

	int x = 0;
	int y = 15;
	for (int i = 0; i < 128 * 16; i++) {
		if (x >= 128) {
			x = 0;
			y--;
		}
		u16 val = *(bitmap++);
		if (val != 0x7C1F) { // Do not render magneta pixel
			_dateFontImage[y * 128 + x] = convertToDsBmp(val);
		} else {
			_dateFontImage[y * 128 + x] = 0x7C1F;
		}
		x++;
	}
}

unsigned int ThemeTextures::getDateTimeFontSpriteIndex(const u16 letter) {
	unsigned int spriteIndex = 0;
	long int left = 0;
	long int right = DATE_TIME_FONT_NUM_IMAGES;
	long int mid = 0;

	while (left <= right) {
		mid = left + ((right - left) / 2);
		if (date_time_utf16_lookup_table[mid] == letter) {
			spriteIndex = mid;
			break;
		}

		if (date_time_utf16_lookup_table[mid] < letter) {
			left = mid + 1;
		} else {
			right = mid - 1;
		}
	}
	return spriteIndex;
}

void ThemeTextures::drawDateTime(const char* str, const int posX, const int posY, const int drawCount, int *hourWidthPointer)
{
	int x = posX;

	beginSubModify();
	for (int c = 0; c < drawCount; c++) {
		int imgY = posY;

		unsigned int charIndex = getDateTimeFontSpriteIndex(str[c]);
		// Start date
		for (int y = 14; y >= 6; y--) {
			for (u16 i = 0; i < date_time_font_texcoords[2 + (4 * charIndex)]; i++) {
				if (_dateFontImage[(imgY * 128) + (date_time_font_texcoords[0 + (4 * charIndex)] + i)] !=
				    0x7C1F) { // Do not render magneta pixel
					_bgSubBuffer[y * 256 + (i + x)] =
					    _dateFontImage[(imgY * 128) +
							  (date_time_font_texcoords[0 + (4 * charIndex)] + i)];
				}
			}
			imgY--;
		}
		x += date_time_font_texcoords[2 + (4 * charIndex)];
		if (hourWidthPointer != NULL)
		{
			if (c == 2)
				*hourWidthPointer = x;
		}
	}
	commitSubModify();
}