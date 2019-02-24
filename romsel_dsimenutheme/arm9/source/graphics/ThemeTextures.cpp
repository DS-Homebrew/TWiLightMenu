#include "ThemeTextures.h"

#include <nds.h>

#include "common/dsimenusettings.h"
#include "paletteEffects.h"
#include "themefilenames.h"
#include "tool/colortool.h"
// Graphic files
#include "../include/startborderpal.h"
#include "_3ds_box_empty.h"
#include "_3ds_box_full.h"
#include "_3ds_bubble.h"
#include "_3ds_cornerbutton.h"
#include "_3ds_cursor.h"
#include "_3ds_dialogbox.h"
#include "_3ds_folder.h"
#include "_3ds_small_cart.h"
#include "org_icon_settings.h"

#include "tool/stringtool.h"

static u16 loadedBottomImg[256 * 192];
static u16 loadedBottomBubbleImg[256 * 192];

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
	glColorSubTableEXT(0, 0, 16, 0, 0, (u16 *)(_3ds_cornerbuttonPal));
}

void ThemeTextures::drawBg() {
	DC_FlushRange(loadedBottomImg, 0x18000);
	dmaCopyWords(0, loadedBottomImg, BG_GFX, 0x18000);
}

void ThemeTextures::drawBubbleBg() {
	DC_FlushRange(loadedBottomBubbleImg, 0x18000);
	dmaCopyWords(0, loadedBottomBubbleImg, BG_GFX, 0x18000);
}

void ThemeTextures::loadBottomImage() {
	extern u16 bmpImageBuffer[256 * 192];
	extern u16 convertToDsBmp(u16 val);

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
			loadedBottomImg[y * 256 + x] = convertToDsBmp(val);
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
			loadedBottomBubbleImg[y * 256 + x] = convertToDsBmp(val);
			x++;
		}
	}
}

void ThemeTextures::loadCommonTextures() {

	loadVolumeTextures();
	loadBatteryTextures();
}

void ThemeTextures::setStringPaths(const std::string theme) {
	topBgPath = formatString("nitro:/graphics/%s_top.bmp", theme.c_str());
	bottomBgPath = formatString("nitro:/graphics/%s_bottom.bmp", theme.c_str());
	bottomBubbleBgPath = formatString("nitro:/graphics/%s_bottom_bubble.bmp", theme.c_str());
	shoulderRPath = formatString("nitro:/graphics/%s_Rshoulder.bmp", theme.c_str());
	shoulderRGreyPath = formatString("nitro:/graphics/%s_Rshoulder_greyed.bmp", theme.c_str());

	shoulderLPath = formatString("nitro:/graphics/%s_Lshoulder.bmp", theme.c_str());
	shoulderLGreyPath = formatString("nitro:/graphics/%s_Lshoulder_greyed.bmp", theme.c_str());
}

void ThemeTextures::load3DSTheme() {

	setStringPaths("3ds");

	loadBottomImage();

	loadBubbleImage(_3ds_bubblePal, _3ds_bubbleBitmap, 7, 7, 8);
	loadSettingsImage(org_icon_settingsPal, org_icon_settingsBitmap);

	loadBoxfullImage(_3ds_box_fullPal, _3ds_box_fullBitmap);
	loadBoxemptyImage(_3ds_box_emptyPal, _3ds_box_emptyBitmap);
	loadFolderImage(_3ds_folderPal, _3ds_folderBitmap);

	loadCornerButtonImage(_3ds_cornerbuttonPal, _3ds_cornerbuttonBitmap, (64 / 16) * (64 / 32), 64, 32, 64, 64);
	loadSmallCartImage(_3ds_small_cartPal, _3ds_small_cartBitmap);
	loadStartbrdImage(_3ds_cursorPal, _3ds_cursorBitmap, (32 / 32) * (192 / 64), 6, 64, 192);
	loadDialogboxImage(_3ds_dialogboxPal, _3ds_dialogboxBitmap);

	loadCommonTextures();
}

void ThemeTextures::loadDSiDarkTheme() {

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

	loadBottomImage();

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

	loadCommonTextures();
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