
#include "ThemeTextures.h"
#include "ThemeConfig.h"

#include <nds.h>
#include "common/dsimenusettings.h"
#include "common/systemdetails.h"

#include "paletteEffects.h"
#include "themefilenames.h"
#include "tool/colortool.h"
// Graphic files
#include "../include/startborderpal.h"

#include "color.h"
#include "errorScreen.h"
#include "tool/stringtool.h"
#include "uvcoord_date_time_font.h"
#include "uvcoord_top_font.h"
#include "common/lzss.h"
#include "common/tonccpy.h"
#include "graphics/lodepng.h"
#include "ndsheaderbanner.h"


extern bool useTwlCfg;

// #include <nds/arm9/decompress.h>
// extern u16 bmpImageBuffer[256*192];
extern s16 usernameRendered[11];
extern bool showColon;

static u16 _bmpImageBuffer[256 * 192] = {0};
static u16 _bgMainBuffer[256 * 192] = {0};
static u16 _bgSubBuffer[256 * 192] = {0};
static u16 _photoBuffer[208 * 156] = {0};

static void* boxArtCache = (void*)0x02500000;	// Size: 0x1B8000
static bool boxArtFound[40] = {false};
int boxArtType[40] = {0};	// 0: NDS, 1: FDS/GBA/GBC/GB, 2: NES/GEN/MD/SFC, 3: SNES

ThemeTextures::ThemeTextures()
    : previouslyDrawnBottomBg(-1), bubbleTexID(0), bipsTexID(0), scrollwindowTexID(0), buttonarrowTexID(0),
      movingarrowTexID(0), launchdotTexID(0), startTexID(0), startbrdTexID(0), settingsTexID(0), braceTexID(0),
      boxfullTexID(0), boxemptyTexID(0), folderTexID(0), cornerButtonTexID(0), smallCartTexID(0), progressTexID(0),
      dialogboxTexID(0), wirelessiconTexID(0), _cachedVolumeLevel(-1), _cachedBatteryLevel(-1) {
	// Overallocation, but thats fine,
	// 0: Top, 1: Bottom, 2: Bottom Bubble, 3: Moving, 4: MovingLeft, 5: MovingRight
	_backgroundTextures.reserve(6);
}

void ThemeTextures::loadBubbleImage(const Texture &tex, int sprW, int sprH) {
	_bubbleImage = std::move(loadTexture(&bubbleTexID, tex, 1, sprW, sprH, GL_RGB16));
}

void ThemeTextures::loadProgressImage(const Texture &tex) {
	// todo: 9 palette
	_progressImage = std::move(loadTexture(&progressTexID, tex, (16 / 16) * (128 / 16), 16, 16, GL_RGB16));
}

void ThemeTextures::loadDialogboxImage(const Texture &tex) {
	_dialogboxImage = std::move(loadTexture(&dialogboxTexID, tex, (256 / 16) * (256 / 16), 16, 16, GL_RGB16));
}

void ThemeTextures::loadBipsImage(const Texture &tex) {
	_bipsImage = std::move(loadTexture(&bipsTexID, tex, (8 / 8) * (32 / 8), 8, 8, GL_RGB16));
}

void ThemeTextures::loadScrollwindowImage(const Texture &tex) {
	_scrollwindowImage = std::move(loadTexture(&scrollwindowTexID, tex, (32 / 16) * (32 / 16), 32, 32, GL_RGB16));
}

void ThemeTextures::loadButtonarrowImage(const Texture &tex) {
	_buttonarrowImage = std::move(loadTexture(&buttonarrowTexID, tex, (32 / 32) * (128 / 32), 32, 32, GL_RGB16));
}

void ThemeTextures::loadMovingarrowImage(const Texture &tex) {
	_movingarrowImage = std::move(loadTexture(&movingarrowTexID, tex, (32 / 32) * (32 / 32), 32, 32, GL_RGB16));
}

void ThemeTextures::loadLaunchdotImage(const Texture &tex) {
	_launchdotImage = std::move(loadTexture(&launchdotTexID, tex, (16 / 16) * (96 / 16), 16, 16, GL_RGB16));
}

void ThemeTextures::loadStartImage(const Texture &tex) {
	_startImage = std::move(loadTexture(&startTexID, tex, (64 / 16) * (128 / 16), 64, 16, GL_RGB16));
}

void ThemeTextures::loadStartbrdImage(const Texture &tex, int sprH) {
	int arraysize = (tex.texWidth() / tc().startBorderSpriteW()) * (tex.texHeight() / sprH);
	_startbrdImage = std::move(loadTexture(&startbrdTexID, tex, arraysize, tc().startBorderSpriteW(), sprH, GL_RGB16));
}
void ThemeTextures::loadBraceImage(const Texture &tex) {
	// todo: confirm 4 palette
	_braceImage = std::move(loadTexture(&braceTexID, tex, (16 / 16) * (128 / 16), 16, 128, GL_RGB16));
}

void ThemeTextures::loadSettingsImage(const Texture &tex) {
	_settingsImage = std::move(loadTexture(&settingsTexID, tex, (64 / 16) * (128 / 64), 64, 64, GL_RGB16));
}

void ThemeTextures::loadManualImage(const Texture &tex) {
	_manualImage = std::move(loadTexture(&manualTexID, tex, (32 / 32) * (32 / 32), 32, 32, GL_RGB16));
}

void ThemeTextures::loadBoxfullImage(const Texture &tex) {
	//_boxfullImage = std::move(loadTexture(&boxfullTexID, tex, (64 / 16) * (128 / 64), 64, 64, (ms().theme==4 ? GL_RGB256 : GL_RGB16)));
	_boxfullImage = std::move(loadTexture(&boxfullTexID, tex, (64 / 16) * (128 / 64), 64, 64, GL_RGB16));
}

void ThemeTextures::loadBoxemptyImage(const Texture &tex) {
	//_boxemptyImage = std::move(loadTexture(&boxemptyTexID, tex, (64 / 16) * (64 / 16), 64, 64, (ms().theme==4 ? GL_RGB256 : GL_RGB16)));
	_boxemptyImage = std::move(loadTexture(&boxemptyTexID, tex, (64 / 16) * (64 / 16), 64, 64, GL_RGB16));
}

void ThemeTextures::loadFolderImage(const Texture &tex) {
	_folderImage = std::move(loadTexture(&folderTexID, tex, (64 / 16) * (64 / 16), 64, 64, GL_RGB16));
}

void ThemeTextures::loadCornerButtonImage(const Texture &tex, int arraysize, int sprW, int sprH) {
	_cornerButtonImage = std::move(loadTexture(&cornerButtonTexID, tex, arraysize, sprW, sprH, GL_RGB16));
}

void ThemeTextures::loadSmallCartImage(const Texture &tex) {
	_smallCartImage = std::move(loadTexture(&smallCartTexID, tex, (32 / 16) * (256 / 32), 32, 32, GL_RGB16));
}

void ThemeTextures::loadWirelessIcons(const Texture &tex) {
	_wirelessIcons = std::move(loadTexture(&wirelessiconTexID, tex, (32 / 32) * (64 / 32), 32, 32, GL_RGB16));
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
	return palette + ((useTwlCfg ? *(unsigned short*)0x02000444 : PersonalData->theme) * 16);
}

unique_ptr<glImage[]> ThemeTextures::loadTexture(int *textureId, const Texture &texture, unsigned int arraySize,
						 int sprW, int sprH, GL_TEXTURE_TYPE_ENUM texType) {

	// We need to delete the texture since the resource held by the unique pointer will be
	// immediately dropped when we assign it to the pointer.

	u32 texW = texture.texWidth();
	u32 texH = texture.texHeight();
	u8 paletteLength = texture.paletteLength();

	if (*textureId != 0) {
		nocashMessage("Existing texture found!?");
		glDeleteTextures(1, textureId);
	}

	// Do a heap allocation of arraySize glImage
	unique_ptr<glImage[]> texturePtr = std::make_unique<glImage[]>(arraySize);

	// Load the texture here.
	*textureId = glLoadTileSet(texturePtr.get(),   // pointer to glImage array
				   sprW,	       // sprite width
				   sprH,	       // sprite height
				   texW,	       // bitmap width
				   texH,	       // bitmap height
				   texType,	   // texture type for glTexImage2D() in videoGL.h
				   get_tex_size(texW), // sizeX for glTexImage2D() in videoGL.h
				   get_tex_size(texH), // sizeY for glTexImage2D() in videoGL.h
				   TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
				   paletteLength,	    // Length of the palette to use (16 colors)
				   (u16 *)texture.palette(), // Load our 16 color tiles palette
				   (u8 *)texture.texture()   // image data generated by GRIT
	);
	return texturePtr;
}

void ThemeTextures::reloadPalDialogBox() {
	if (ms().theme == 4 || ms().theme == 5) return;
	glBindTexture(0, dialogboxTexID);
	glColorSubTableEXT(0, 0, _dialogBoxTexture->paletteLength(), 0, 0, _dialogBoxTexture->palette());
	if (ms().theme != 1) {
		glBindTexture(0, cornerButtonTexID);
		glColorSubTableEXT(0, 0, 16, 0, 0, _cornerButtonTexture->palette());
	}
}

void ThemeTextures::loadBackgrounds() {
	// 0: Top, 1: Bottom, 2: Bottom Bubble, 3: Moving, 4: MovingLeft, 5: MovingRight

	// We reuse the _topBackgroundTexture as a buffer.
	_backgroundTextures.emplace_back(TFN_BG_TOPBG, TFN_FALLBACK_BG_TOPBG);
		
	
	if (ms().theme == 1 && !sys().isRegularDS()) {
		_backgroundTextures.emplace_back(TFN_BG_BOTTOMBG, TFN_FALLBACK_BG_BOTTOMBG);
		_backgroundTextures.emplace_back(TFN_BG_BOTTOMBUBBLEBG, TFN_FALLBACK_BG_BOTTOMBUBBLEBG);
		return;
	}

	if (ms().theme == 1 && sys().isRegularDS()) {
		_backgroundTextures.emplace_back(TFN_BG_BOTTOMBG_DS, TFN_FALLBACK_BG_BOTTOMBG_DS);
		_backgroundTextures.emplace_back(TFN_BG_BOTTOMBUBBLEBG_DS, TFN_FALLBACK_BG_BOTTOMBUBBLEBG_DS);
		return;
	}
	// DSi Theme
	_backgroundTextures.emplace_back(TFN_BG_BOTTOMBG, TFN_FALLBACK_BG_BOTTOMBG);
	_backgroundTextures.emplace_back(TFN_BG_BOTTOMBUBBLEBG, TFN_FALLBACK_BG_BOTTOMBUBBLEBG);
	if (ms().theme == 0) _backgroundTextures.emplace_back(TFN_BG_BOTTOMMOVINGBG, TFN_FALLBACK_BG_BOTTOMMOVINGBG);
	
}

void ThemeTextures::loadHBTheme() {	
	printf("tex().loadHBTheme()\n");

	// printf("tex().loadBackgrounds()\n");
	loadBackgrounds();
	// printf("tex().loadUITextures()\n");
	loadUITextures();

	// printf("tex().loadVolumeTextures()\n");
	loadVolumeTextures();
	// printf("tex().loadBatteryTextures()\n");
	loadBatteryTextures();
	// printf("tex().loadIconTextures()\n");
	loadIconTextures();

	_boxFullTexture = std::make_unique<Texture>(TFN_GRF_BOX_FULL, TFN_FALLBACK_GRF_BOX_FULL);
	_boxEmptyTexture = std::make_unique<Texture>(TFN_GRF_BOX_EMPTY, TFN_FALLBACK_GRF_BOX_EMPTY);
	_braceTexture = std::make_unique<Texture>(TFN_GRF_BRACE, TFN_FALLBACK_GRF_BRACE);
	_cornerButtonTexture = std::make_unique<Texture>(TFN_GRF_CORNERBUTTON, TFN_FALLBACK_GRF_CORNERBUTTON);

	_folderTexture = std::make_unique<Texture>(TFN_GRF_FOLDER, TFN_FALLBACK_GRF_FOLDER);
	_progressTexture = std::make_unique<Texture>(TFN_GRF_PROGRESS, TFN_FALLBACK_GRF_PROGRESS);

	_progressTexture = std::make_unique<Texture>(TFN_GRF_PROGRESS, TFN_FALLBACK_GRF_PROGRESS);
	_smallCartTexture = std::make_unique<Texture>(TFN_GRF_SMALL_CART, TFN_FALLBACK_GRF_SMALL_CART);
	_wirelessIconsTexture = std::make_unique<Texture>(TFN_GRF_WIRELESSICONS, TFN_FALLBACK_GRF_WIRELESSICONS);
	_settingsIconTexture = std::make_unique<Texture>(TFN_GRF_ICON_SETTINGS, TFN_FALLBACK_GRF_ICON_SETTINGS);
	_manualIconTexture = std::make_unique<Texture>(TFN_GRF_ICON_MANUAL, TFN_FALLBACK_GRF_ICON_MANUAL);

	if (ms().colorMode == 1) {
		// printf("tex().applyGrayscaleToAllGrfTextures()\n");
		applyGrayscaleToAllGrfTextures();
	}

	
	// printf("tex().loadWirelessIcons(*_wirelessIconsTexture)\n");
	loadWirelessIcons(*_wirelessIconsTexture);
	// printf("tex().loadSettingsImage(*_settingsIconTexture)\n");
	loadSettingsImage(*_settingsIconTexture);
	// printf("tex().loadBraceImage(*_braceTexture)\n");
	loadBraceImage(*_braceTexture);

	// printf("tex().loadBoxfullImage(*_boxFullTexture)\n");
	loadBoxfullImage(*_boxFullTexture);
	// printf("tex().loadBoxEmptyImage(*_boxFullTexture)\n");
	loadBoxemptyImage(*_boxEmptyTexture);

	// printf("tex().loadManualImage(*_manualIconTexture)\n");
	loadManualImage(*_manualIconTexture);
	// printf("tex().loadCornerButtonImage(*_cornerButtonTexture, (32 / 16) * (32 / 32), 32, 32)\n");
	loadCornerButtonImage(*_cornerButtonTexture, (32 / 16) * (32 / 32), 32, 32);
	// printf("tex().loadSmallCartImage(*_smallCartTexture)\n");
	loadSmallCartImage(*_smallCartTexture);
	// printf("tex().loadFolderImage(*_folderTexture)\n");
	loadFolderImage(*_folderTexture);
	
	// printf("tex().loadProgressImage(*_progressTexture)\n");
	loadProgressImage(*_progressTexture);
	// printf("tex().loadWirelessIcons(*_wirelessIconsTexture)\n");
	loadWirelessIcons(*_wirelessIconsTexture);
	
}

void ThemeTextures::loadSaturnTheme() {	
	printf("tex().loadSaturnTheme()\n");

	loadBackgrounds();
	loadUITextures();

	loadVolumeTextures();
	loadBatteryTextures();
	loadIconTextures();

	_boxFullTexture = std::make_unique<Texture>(TFN_GRF_BOX_FULL, TFN_FALLBACK_GRF_BOX_FULL);
	_boxEmptyTexture = std::make_unique<Texture>(TFN_GRF_BOX_EMPTY, TFN_FALLBACK_GRF_BOX_EMPTY);
	_braceTexture = std::make_unique<Texture>(TFN_GRF_BRACE, TFN_FALLBACK_GRF_BRACE);
	_cornerButtonTexture = std::make_unique<Texture>(TFN_GRF_CORNERBUTTON, TFN_FALLBACK_GRF_CORNERBUTTON);

	_folderTexture = std::make_unique<Texture>(TFN_GRF_FOLDER, TFN_FALLBACK_GRF_FOLDER);

	_progressTexture = std::make_unique<Texture>(TFN_GRF_PROGRESS, TFN_FALLBACK_GRF_PROGRESS);
	_smallCartTexture = std::make_unique<Texture>(TFN_GRF_SMALL_CART, TFN_FALLBACK_GRF_SMALL_CART);
	_wirelessIconsTexture = std::make_unique<Texture>(TFN_GRF_WIRELESSICONS, TFN_FALLBACK_GRF_WIRELESSICONS);
	_settingsIconTexture = std::make_unique<Texture>(TFN_GRF_ICON_SETTINGS, TFN_FALLBACK_GRF_ICON_SETTINGS);
	_manualIconTexture = std::make_unique<Texture>(TFN_GRF_ICON_MANUAL, TFN_FALLBACK_GRF_ICON_MANUAL);

	if (ms().colorMode == 1) {
		applyGrayscaleToAllGrfTextures();
	}

	loadWirelessIcons(*_wirelessIconsTexture);
	loadSettingsImage(*_settingsIconTexture);
	loadBraceImage(*_braceTexture);

	loadBoxfullImage(*_boxFullTexture);
	loadBoxemptyImage(*_boxEmptyTexture);

	loadManualImage(*_manualIconTexture);
	loadCornerButtonImage(*_cornerButtonTexture, (32 / 16) * (32 / 32), 32, 32);
	loadSmallCartImage(*_smallCartTexture);
	loadFolderImage(*_folderTexture);

	loadProgressImage(*_progressTexture);
	loadWirelessIcons(*_wirelessIconsTexture);
}

void ThemeTextures::load3DSTheme() {
	printf("tex().load3DSTheme()\n");

	loadBackgrounds();
	loadUITextures();

	loadVolumeTextures();
	loadBatteryTextures();

	loadIconTextures();

	_bubbleTexture = std::make_unique<Texture>(TFN_GRF_BUBBLE, TFN_FALLBACK_GRF_BUBBLE);
	_settingsIconTexture = std::make_unique<Texture>(TFN_GRF_ICON_SETTINGS, TFN_FALLBACK_GRF_ICON_SETTINGS);

	_boxFullTexture = std::make_unique<Texture>(TFN_GRF_BOX_FULL, TFN_FALLBACK_GRF_BOX_FULL);
	_boxEmptyTexture = std::make_unique<Texture>(TFN_GRF_BOX_EMPTY, TFN_FALLBACK_GRF_BOX_EMPTY);
	_folderTexture = std::make_unique<Texture>(TFN_GRF_FOLDER, TFN_FALLBACK_GRF_FOLDER);
	_progressTexture = std::make_unique<Texture>(TFN_GRF_PROGRESS, TFN_FALLBACK_GRF_PROGRESS);

	_smallCartTexture = std::make_unique<Texture>(TFN_GRF_SMALL_CART, TFN_FALLBACK_GRF_SMALL_CART);
	_startBorderTexture = std::make_unique<Texture>(TFN_GRF_CURSOR, TFN_FALLBACK_GRF_CURSOR);
	_dialogBoxTexture = std::make_unique<Texture>(TFN_GRF_DIALOGBOX, TFN_FALLBACK_GRF_DIALOGBOX);

	if (ms().colorMode == 1) {
		applyGrayscaleToAllGrfTextures();
	}

	loadBubbleImage(*_bubbleTexture, tc().bubbleTipSpriteW(), tc().bubbleTipSpriteH());
	loadSettingsImage(*_settingsIconTexture);

	loadBoxfullImage(*_boxFullTexture);
	loadBoxemptyImage(*_boxEmptyTexture);
	loadFolderImage(*_folderTexture);

	loadSmallCartImage(*_smallCartTexture);
	loadStartbrdImage(*_startBorderTexture, tc().startBorderSpriteH());
	loadDialogboxImage(*_dialogBoxTexture);
	loadProgressImage(*_progressTexture);
	loadWirelessIcons(*_wirelessIconsTexture);
}

void ThemeTextures::loadDSiTheme() {	
	printf("tex().loadDSiTheme()\n");

	loadBackgrounds();
	loadUITextures();
		
	loadVolumeTextures();
	loadBatteryTextures();
	loadIconTextures();

	_bipsTexture = std::make_unique<Texture>(TFN_GRF_BIPS, TFN_FALLBACK_GRF_BIPS);
	_boxTexture = std::make_unique<Texture>(TFN_GRF_BOX, TFN_FALLBACK_GRF_BOX);
	_braceTexture = std::make_unique<Texture>(TFN_GRF_BRACE, TFN_FALLBACK_GRF_BRACE);
	_bubbleTexture = std::make_unique<Texture>(TFN_GRF_BUBBLE, TFN_FALLBACK_GRF_BUBBLE);
	_buttonArrowTexture = std::make_unique<Texture>(TFN_GRF_BUTTON_ARROW, TFN_FALLBACK_GRF_BUTTON_ARROW);
	_cornerButtonTexture = std::make_unique<Texture>(TFN_GRF_CORNERBUTTON, TFN_FALLBACK_GRF_CORNERBUTTON);

	_dialogBoxTexture = std::make_unique<Texture>(TFN_GRF_DIALOGBOX, TFN_FALLBACK_GRF_DIALOGBOX);

	_folderTexture = std::make_unique<Texture>(TFN_GRF_FOLDER, TFN_FALLBACK_GRF_FOLDER);
	_launchDotTexture = std::make_unique<Texture>(TFN_GRF_LAUNCH_DOT, TFN_FALLBACK_GRF_LAUNCH_DOT);
	_movingArrowTexture = std::make_unique<Texture>(TFN_GRF_MOVING_ARROW, TFN_FALLBACK_GRF_MOVING_ARROW);

	_progressTexture = std::make_unique<Texture>(TFN_GRF_PROGRESS, TFN_FALLBACK_GRF_PROGRESS);
	_scrollWindowTexture = std::make_unique<Texture>(TFN_GRF_SCROLL_WINDOW, TFN_FALLBACK_GRF_SCROLL_WINDOW);
	_smallCartTexture = std::make_unique<Texture>(TFN_GRF_SMALL_CART, TFN_FALLBACK_GRF_SMALL_CART);
	_startBorderTexture = std::make_unique<Texture>(TFN_GRF_START_BORDER, TFN_FALLBACK_GRF_START_BORDER);
	_startTextTexture = std::make_unique<Texture>(TFN_GRF_START_TEXT, TFN_FALLBACK_GRF_START_TEXT);
	_wirelessIconsTexture = std::make_unique<Texture>(TFN_GRF_WIRELESSICONS, TFN_FALLBACK_GRF_WIRELESSICONS);
	_settingsIconTexture = std::make_unique<Texture>(TFN_GRF_ICON_SETTINGS, TFN_FALLBACK_GRF_ICON_SETTINGS);
	//_manualIconTexture = std::make_unique<Texture>(TFN_GRF_ICON_MANUAL, TFN_FALLBACK_GRF_ICON_MANUAL);

	// Apply the DSi palette shifts
	if (tc().startTextUserPalette())
		_startTextTexture->applyPaletteEffect(effectDSiStartTextPalettes);
	if (tc().startBorderUserPalette())
		_startBorderTexture->applyPaletteEffect(effectDSiStartBorderPalettes);
	if (tc().buttonArrowUserPalette())
		_buttonArrowTexture->applyPaletteEffect(effectDSiArrowButtonPalettes);
	if (tc().movingArrowUserPalette())
		_movingArrowTexture->applyPaletteEffect(effectDSiArrowButtonPalettes);
	if (tc().launchDotsUserPalette())
		_launchDotTexture->applyPaletteEffect(effectDSiArrowButtonPalettes);
	if (tc().dialogBoxUserPalette())
		_dialogBoxTexture->applyPaletteEffect(effectDSiArrowButtonPalettes);

	if (ms().colorMode == 1) {
		applyGrayscaleToAllGrfTextures();
	}

	loadBipsImage(*_bipsTexture);

	loadBubbleImage(*_bubbleTexture, tc().bubbleTipSpriteW(), tc().bubbleTipSpriteH());
	loadScrollwindowImage(*_scrollWindowTexture);
	loadWirelessIcons(*_wirelessIconsTexture);
	loadSettingsImage(*_settingsIconTexture);
	loadBraceImage(*_braceTexture);

	loadStartImage(*_startTextTexture);
	loadStartbrdImage(*_startBorderTexture, tc().startBorderSpriteH());

	loadButtonarrowImage(*_buttonArrowTexture);
	loadMovingarrowImage(*_movingArrowTexture);
	loadLaunchdotImage(*_launchDotTexture);
	loadDialogboxImage(*_dialogBoxTexture);

	// careful here, it's boxTexture, not boxFulltexture.
	loadBoxfullImage(*_boxTexture);

	//loadManualImage(*_manualIconTexture);
	loadCornerButtonImage(*_cornerButtonTexture, (32 / 16) * (32 / 32), 32, 32);
	loadSmallCartImage(*_smallCartTexture);
	loadFolderImage(*_folderTexture);

	loadProgressImage(*_progressTexture);
	loadWirelessIcons(*_wirelessIconsTexture);
}

void ThemeTextures::loadVolumeTextures() {
	if (isDSiMode() || REG_SCFG_EXT != 0) {
		_volume0Texture = std::make_unique<Texture>(TFN_VOLUME0, TFN_FALLBACK_VOLUME0);
		_volume1Texture = std::make_unique<Texture>(TFN_VOLUME1, TFN_FALLBACK_VOLUME1);
		_volume2Texture = std::make_unique<Texture>(TFN_VOLUME2, TFN_FALLBACK_VOLUME2);
		_volume3Texture = std::make_unique<Texture>(TFN_VOLUME3, TFN_FALLBACK_VOLUME3);
		_volume4Texture = std::make_unique<Texture>(TFN_VOLUME4, TFN_FALLBACK_VOLUME4);
	}
}

void ThemeTextures::loadBatteryTextures() {
	if (isDSiMode() || REG_SCFG_EXT != 0) {
		_batterychargeTexture = std::make_unique<Texture>(TFN_BATTERY_CHARGE, TFN_FALLBACK_BATTERY_CHARGE);
		_batterychargeblinkTexture = std::make_unique<Texture>(TFN_BATTERY_CHARGE_BLINK, TFN_FALLBACK_BATTERY_CHARGE_BLINK);
		_battery0Texture = std::make_unique<Texture>(TFN_BATTERY0, TFN_FALLBACK_BATTERY0);
		_battery1Texture = std::make_unique<Texture>(TFN_BATTERY1, TFN_FALLBACK_BATTERY1);
		_battery2Texture = std::make_unique<Texture>(TFN_BATTERY2, TFN_FALLBACK_BATTERY2);
		_battery3Texture = std::make_unique<Texture>(TFN_BATTERY3, TFN_FALLBACK_BATTERY3);
		_battery4Texture = std::make_unique<Texture>(TFN_BATTERY4, TFN_FALLBACK_BATTERY4);
	} else {
		_batteryfullTexture = std::make_unique<Texture>(TFN_BATTERY_FULL, TFN_FALLBACK_BATTERY_FULL);
		_batteryfullDSTexture = std::make_unique<Texture>(TFN_BATTERY_FULLDS, TFN_FALLBACK_BATTERY_FULLDS);
		_batterylowTexture = std::make_unique<Texture>(TFN_BATTERY_LOW, TFN_FALLBACK_BATTERY_LOW);
	}
}

void ThemeTextures::loadUITextures() {
	_dateTimeFontTexture = std::make_unique<Texture>(TFN_UI_DATE_TIME_FONT, TFN_FALLBACK_UI_DATE_TIME_FONT);
	if (ms().theme != 5) {
		_leftShoulderTexture = std::make_unique<Texture>(TFN_UI_LSHOULDER, TFN_FALLBACK_UI_LSHOULDER);
		_rightShoulderTexture = std::make_unique<Texture>(TFN_UI_RSHOULDER, TFN_FALLBACK_UI_RSHOULDER);
		_leftShoulderGreyedTexture = std::make_unique<Texture>(TFN_UI_LSHOULDER_GREYED, TFN_FALLBACK_UI_LSHOULDER_GREYED);
		_rightShoulderGreyedTexture = std::make_unique<Texture>(TFN_UI_RSHOULDER_GREYED, TFN_FALLBACK_UI_RSHOULDER_GREYED);
	}
}

void ThemeTextures::loadIconTextures() {
	_iconGBTexture = std::make_unique<Texture>(TFN_GRF_ICON_GB, TFN_FALLBACK_GRF_ICON_GB);
	_iconGBATexture = std::make_unique<Texture>(TFN_GRF_ICON_GBA, TFN_FALLBACK_GRF_ICON_GBA);
	//_iconGBAModeTexture = std::make_unique<Texture>(TFN_GRF_ICON_GBAMODE, TFN_FALLBACK_GRF_ICON_GBAMODE);
	_iconGGTexture = std::make_unique<Texture>(TFN_GRF_ICON_GG, TFN_FALLBACK_GRF_ICON_GG);
	_iconMDTexture = std::make_unique<Texture>(TFN_GRF_ICON_MD, TFN_FALLBACK_GRF_ICON_MD);
	_iconNESTexture = std::make_unique<Texture>(TFN_GRF_ICON_NES, TFN_FALLBACK_GRF_ICON_NES);
	_iconSMSTexture = std::make_unique<Texture>(TFN_GRF_ICON_SMS, TFN_FALLBACK_GRF_ICON_SMS);
	_iconSNESTexture = std::make_unique<Texture>(TFN_GRF_ICON_SNES, TFN_FALLBACK_GRF_ICON_SNES);
	_iconPLGTexture = std::make_unique<Texture>(TFN_GRF_ICON_PLG, TFN_FALLBACK_GRF_ICON_PLG);
	_iconUnknownTexture = std::make_unique<Texture>(TFN_GRF_ICON_UNK, TFN_FALLBACK_GRF_ICON_UNK);

	// if (ms().colorMode == 1)
	// {
	// 	_iconGBTexture->applyPaletteEffect(effectGrayscalePalette);
	// 	_iconGBATexture->applyPaletteEffect(effectGrayscalePalette);
	// 	_iconGBAModeTexture->applyPaletteEffect(effectGrayscalePalette);
	// 	_iconGGTexture->applyPaletteEffect(effectGrayscalePalette);
	// 	_iconMDTexture->applyPaletteEffect(effectGrayscalePalette);
	// 	_iconNESTexture->applyPaletteEffect(effectGrayscalePalette);
	// 	_iconSMSTexture->applyPaletteEffect(effectGrayscalePalette);
	// 	_iconSNESTexture->applyPaletteEffect(effectGrayscalePalette);
	// 	_iconPLGTexture->applyPaletteEffect(effectGrayscalePalette);
	// 	_iconUnknownTexture->applyPaletteEffect(effectGrayscalePalette);
	// }
}
u16 *ThemeTextures::beginBgSubModify() {
	dmaCopyWords(0, BG_GFX_SUB, _bgSubBuffer, sizeof(u16) * BG_BUFFER_PIXELCOUNT);
	return _bgSubBuffer;
}

void ThemeTextures::commitBgSubModify() {
	DC_FlushRange(_bgSubBuffer, sizeof(u16) * BG_BUFFER_PIXELCOUNT);
	dmaCopyWords(2, _bgSubBuffer, BG_GFX_SUB, sizeof(u16) * BG_BUFFER_PIXELCOUNT);
}

void ThemeTextures::commitBgSubModifyAsync() {
	DC_FlushRange(_bgSubBuffer, sizeof(u16) * BG_BUFFER_PIXELCOUNT);
	dmaCopyWordsAsynch(2, _bgSubBuffer, BG_GFX_SUB, sizeof(u16) * BG_BUFFER_PIXELCOUNT);
}

u16 *ThemeTextures::beginBgMainModify() {
	dmaCopyWords(0, BG_GFX, _bgMainBuffer, sizeof(u16) * BG_BUFFER_PIXELCOUNT);
	return _bgMainBuffer;
}

void ThemeTextures::commitBgMainModify() {
	DC_FlushRange(_bgMainBuffer, sizeof(u16) * BG_BUFFER_PIXELCOUNT);
	dmaCopyWords(2, _bgMainBuffer, BG_GFX, sizeof(u16) * BG_BUFFER_PIXELCOUNT);
}

void ThemeTextures::commitBgMainModifyAsync() {
	DC_FlushRange(_bgMainBuffer, sizeof(u16) * BG_BUFFER_PIXELCOUNT);
	dmaCopyWordsAsynch(2, _bgMainBuffer, BG_GFX, sizeof(u16) * BG_BUFFER_PIXELCOUNT);
}

void ThemeTextures::drawTopBg() {
	beginBgSubModify();
	LZ77_Decompress((u8*)_backgroundTextures[0].texture(), (u8*)_bgSubBuffer);
	commitBgSubModify();
}

void ThemeTextures::drawBottomBg(int index) {

	// clamp index
	if (index < 1)
		index = 1;
	if (index > 3)
		index = 3;
	if (index > 2 && ms().theme == 1)
		index = 2;
	beginBgMainModify();

	if (previouslyDrawnBottomBg != index) {
		LZ77_Decompress((u8*)_backgroundTextures[index].texture(), (u8*)_bgMainBuffer);
		previouslyDrawnBottomBg = index;
	} else {
		DC_FlushRange(_backgroundTextures[index].texture(), 0x18000);
		dmaCopyWords(0, _backgroundTextures[index].texture(), BG_GFX, 0x18000);
		LZ77_Decompress((u8*)_backgroundTextures[index].texture(), (u8*)_bgMainBuffer);
	}

	if(ms().colorMode == 1) {
		for (u16 i = 0; i < BG_BUFFER_PIXELCOUNT; i++) {
			_bgMainBuffer[i] =
			    convertVramColorToGrayscale(_bgMainBuffer[i]);
		}
	}

	commitBgMainModify();
}

void ThemeTextures::clearTopScreen() {
	beginBgSubModify();
	u16 val = 0xFFFF;
	for (int i = 0; i < BG_BUFFER_PIXELCOUNT; i++) {
		_bgSubBuffer[i] = ((val >> 10) & 31) | (val & (31 - 3 * ms().blfLevel) << 5) |
				  (val & (31 - 6 * ms().blfLevel)) << 10 | BIT(15);
	}
	commitBgSubModify();
}

void ThemeTextures::drawProfileName() {
	// Load username
	char fontPath[64] = {0};
	FILE *file;
	int x = ((isDSiMode() || REG_SCFG_EXT != 0) ? 28 : 4);

	for (int c = 0; c < 10; c++) {
		unsigned int charIndex = getTopFontSpriteIndex(usernameRendered[c]);
		// 42 characters per line.
		unsigned int texIndex = charIndex / 42;
		sprintf(fontPath, "nitro:/graphics/top_font/small_font_%u.bmp", texIndex);

		file = fopen(fontPath, "rb");

		if (file) {
			beginBgSubModify();
			// Start loading
			fseek(file, 0xe, SEEK_SET);
			u8 pixelStart = (u8)fgetc(file) + 0xe;
			fseek(file, pixelStart, SEEK_SET);
			for (int y = 15; y >= 0; y--) {
				fread(_bmpImageBuffer, 2, 0x200, file);
				u16 *src = _bmpImageBuffer + (top_font_texcoords[0 + (4 * charIndex)]);

				for (u16 i = 0; i < top_font_texcoords[2 + (4 * charIndex)]; i++) {
					u16 val = *(src++);

					// Blend with pixel
					const u16 bg =
					    _bgSubBuffer[(y + 2) * 256 + (i + x)]; // grab the background pixel
					// Apply palette here.

					// Magic numbers were found by dumping val to stdout
					// on case default.
					switch (val) {
					// #ff00ff
					case 0xFC1F:
						break;
					// #404040
					case 0xA108:
						val = alphablend(bmpPal_topSmallFont[1 + ((useTwlCfg ? *(u8*)0x02000444 : PersonalData->theme) * 16)],
								 bg, 224U);
						break;
					// #808080
					case 0xC210:
						// blend the colors with the background to make it look better.
						// Fills in the
						// 1 for light
						val = alphablend(bmpPal_topSmallFont[1 + ((useTwlCfg ? *(u8*)0x02000444 : PersonalData->theme) * 16)],
								 bg, 224U);
						break;
					// #b8b8b8
					case 0xDEF7:
						// 6 looks good on lighter themes
						// 3 do an average blend twice
						//
						val = alphablend(bmpPal_topSmallFont[3 + ((useTwlCfg ? *(u8*)0x02000444 : PersonalData->theme) * 16)],
								 bg, 128U);
						break;
					default:
						break;
					}
					if (val != 0xFC1F && val != 0x7C1F) { // Do not render magneta pixel
						_bgSubBuffer[(y + 2) * 256 + (i + x)] = Texture::bmpToDS(val);
					}
				}
			}
			x += top_font_texcoords[2 + (4 * charIndex)];
			commitBgSubModify();
		}

		fclose(file);
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

TWL_CODE void ThemeTextures::loadBoxArtToMem(const char *filename, int num) {
	if (num < 0 || num > 39) {
		return;
	}

	FILE *file = fopen(filename, "rb");
	if (!file) {
		boxArtFound[num] = false;
		//filename = "nitro:/graphics/boxart_unknown.bmp";
		//file = fopen(filename, "rb");
		return;
	}

	boxArtFound[num] = true;

	fread((u8*)boxArtCache+(num*0xB000), 1, 0xB000, file);
	fclose(file);
}

void ThemeTextures::drawBoxArt(const char *filename) {
	if(access(filename, F_OK) != 0) {
		switch (boxArtType[CURPOS]) {
			case 0:
			default:
				filename = "nitro:/graphics/boxart_unknown.png";
				break;
			case 1:
				filename = "nitro:/graphics/boxart_unknown1.png";
				break;
			case 2:
				filename = "nitro:/graphics/boxart_unknown2.png";
				break;
			case 3:
				filename = "nitro:/graphics/boxart_unknown3.png";
				break;
		}
	}

	beginBgSubModify();

	std::vector<unsigned char> image;
	uint imageXpos, imageYpos, imageWidth, imageHeight;
	lodepng::decode(image, imageWidth, imageHeight, filename);
	if(imageWidth > 256 || imageHeight > 192)	return;

	for(uint i=0;i<image.size()/4;i++) {
		_bmpImageBuffer[i] = image[i*4]>>3 | (image[(i*4)+1]>>3)<<5 | (image[(i*4)+2]>>3)<<10 | BIT(15);
		if (ms().colorMode == 1) {
			_bmpImageBuffer[i] = convertVramColorToGrayscale(_bmpImageBuffer[i]);
		}
	}

	imageXpos = (256-imageWidth)/2;
	imageYpos = (192-imageHeight)/2;
	u16 *src = _bmpImageBuffer;
	for(uint y = 0; y < imageHeight; y++) {
		for(uint x = 0; x < imageWidth; x++) {
			_bgSubBuffer[(y+imageYpos) * 256 + imageXpos + x] = *(src++);
		}
	}
	commitBgSubModify();
}

void ThemeTextures::drawBoxArtFromMem(int num) {
	if (num < 0 || num > 39) {
		return;
	}

	if (!boxArtFound[num]) {
		drawBoxArt("nitro:/null.png");
		return;
	}

	uint imageXpos, imageYpos, imageWidth, imageHeight;

	// Start loading
	beginBgSubModify();
	std::vector<unsigned char> image;
	lodepng::decode(image, imageWidth, imageHeight, (unsigned char*)boxArtCache+(num*0xB000), 0xB000);
	if(imageWidth > 256 || imageHeight > 192)	return;

	for(uint i=0;i<image.size()/4;i++) {
		_bmpImageBuffer[i] = image[i*4]>>3 | (image[(i*4)+1]>>3)<<5 | (image[(i*4)+2]>>3)<<10 | BIT(15);
		if (ms().colorMode == 1) {
			_bmpImageBuffer[i] = convertVramColorToGrayscale(_bmpImageBuffer[i]);
		}
	}

	imageXpos = (256-imageWidth)/2;
	imageYpos = (192-imageHeight)/2;
	u16 *src = _bmpImageBuffer;
	for(uint y = 0; y < imageHeight;y++) {
		for(uint x = 0; x < imageWidth; x++) {
			_bgSubBuffer[(y+imageYpos) * 256 + imageXpos + x] = *(src++);
		}
	}
	commitBgSubModify();
}

void ThemeTextures::drawVolumeImage(int volumeLevel) {
	if (!isDSiMode() && REG_SCFG_EXT == 0)
		return;
	beginBgSubModify();

	const Texture *tex = volumeTexture(volumeLevel);
	const u16 *src = tex->texture();
	int startX = (ms().theme == 4 ? 40 : 4);
	int startY = (ms().theme == 4 ? 10 : 5);
	for (uint y = 0; y < tex->texHeight(); y++) {
		for (uint x = 0; x < tex->texWidth(); x++) {
			u16 val = *(src++);
			if (val >> 15) { // Do not render transparent pixel
				_bgSubBuffer[(startY + y) * 256 + startX + x] = val;
			}
		}
	}
	commitBgSubModify();
}

void ThemeTextures::drawVolumeImageCached() {
	int volumeLevel = getVolumeLevel();
	if (_cachedVolumeLevel != volumeLevel) {
		_cachedVolumeLevel = volumeLevel;
		drawVolumeImage(volumeLevel);
	}
}

int ThemeTextures::getVolumeLevel(void) {
	if (!isDSiMode() && REG_SCFG_EXT == 0)
		return -1;
	
	u8 volumeLevel = sys().volumeStatus();
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
	u8 batteryLevel =  sys().batteryStatus();
	if (!isDSiMode() && REG_SCFG_EXT == 0) {
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
	beginBgSubModify();
	const Texture *tex = batteryTexture(batteryLevel, drawDSiMode, isRegularDS);
	const u16 *src = tex->texture();
	for (uint y = tc().batteryRenderY(); y < tc().batteryRenderY() + tex->texHeight(); y++) {
		for (uint x = tc().batteryRenderX(); x < tc().batteryRenderX() + tex->texWidth(); x++) {
			u16 val = *(src++);
			if (val >> 15) { // Do not render transparent pixel
				_bgSubBuffer[y * 256 + x] = val;
			}
		}
	}
	commitBgSubModify();
}

void ThemeTextures::drawBatteryImageCached() {
	int batteryLevel = getBatteryLevel();
	if(batteryLevel == 0 && showColon)	batteryLevel--;
	else if(batteryLevel == 7 && showColon)	batteryLevel++;
	if (_cachedBatteryLevel != batteryLevel) {
		_cachedBatteryLevel = batteryLevel;
		drawBatteryImage(batteryLevel, (isDSiMode() || REG_SCFG_EXT != 0), sys().isRegularDS());
	}
}

#define TOPLINES 32 * 256
#define BOTTOMOFFSET ((tc().shoulderLRenderY() - 5) * 256)
#define BOTTOMLINES ((192 - (tc().shoulderLRenderY() - 5)) * 256)
// Load .bmp file without overwriting shoulder button images or username
void ThemeTextures::drawTopBgAvoidingShoulders() {

	// Copy current to _bmpImageBuffer
	dmaCopyWords(0, BG_GFX_SUB, _bmpImageBuffer, sizeof(u16) * BG_BUFFER_PIXELCOUNT);

	// Throw the entire top background into the sub buffer.
	LZ77_Decompress((u8*)_backgroundTextures[0].texture(), (u8*)_bgSubBuffer);

 	// Copy top 32 lines from the buffer into the sub.
	tonccpy(_bgSubBuffer, _bmpImageBuffer, sizeof(u16) * TOPLINES);
	
	// Copy bottom tc().shoulderLRenderY() + 5 lines into the sub
	// ((192 - 32) * 256)
	tonccpy(_bgSubBuffer + BOTTOMOFFSET, _bmpImageBuffer + BOTTOMOFFSET, sizeof(u16) * BOTTOMLINES);

	commitBgSubModify();
}

void ThemeTextures::drawShoulders(bool LShoulderActive, bool RShoulderActive) {
	beginBgSubModify();

	const Texture *rightTex = RShoulderActive ? _rightShoulderTexture.get() : _rightShoulderGreyedTexture.get();
	const u16 *rightSrc = rightTex->texture();

	const Texture *leftTex = LShoulderActive ? _leftShoulderTexture.get() : _leftShoulderGreyedTexture.get();
	const u16 *leftSrc = leftTex->texture();

	// Draw R Shoulder
	for (uint y = tc().shoulderRRenderY(); y < tc().shoulderRRenderY() + rightTex->texHeight(); y++) {
		for (uint x = tc().shoulderRRenderX(); x < tc().shoulderRRenderX() + rightTex->texWidth(); x++) {
			u16 val = *(rightSrc++);
			if (val >> 15) { // Do not render transparent pixel
				_bgSubBuffer[y * 256 + x] = val;
			}
		}
	}

	// Draw L Shoulder
	for (uint y = tc().shoulderLRenderY(); y < tc().shoulderLRenderY() + leftTex->texHeight(); y++) {
		for (uint x = tc().shoulderLRenderX(); x < tc().shoulderLRenderX() + leftTex->texWidth(); x++) {
			u16 val = *(leftSrc++);
			if (val >> 15) { // Do not render transparent pixel
				_bgSubBuffer[y * 256 + x] = val;
			}
		}
	}

	commitBgSubModify();
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

void ThemeTextures::drawDateTime(const char *str, int posX, int posY, const int drawCount, int *hourWidthPointer) {
	beginBgSubModify();

	const Texture *tex = dateTimeFontTexture();
	const u16 *bitmap = tex->texture();

	for (int c = 0; c < drawCount; c++) {
		unsigned int charIndex = getDateTimeFontSpriteIndex(str[c]);
		// Start date
		for (uint y = 0; y < tex->texHeight(); y++) {
			const u16 *src = bitmap + (y * 128) + (date_time_font_texcoords[4 * charIndex]);
			for (uint x = 0; x < date_time_font_texcoords[2 + (4 * charIndex)]; x++) {
				u16 val = *(src++);
				if (val >> 15) { // Do not render transparent pixel
					_bgSubBuffer[(posY + y) * 256 + (posX + x)] = val;
				}
			}
		}
		posX += date_time_font_texcoords[2 + (4 * charIndex)];
		if (hourWidthPointer != NULL) {
			if (c == 2)
				*hourWidthPointer = posX;
		}
	}

	commitBgSubModify();
}

void ThemeTextures::applyGrayscaleToAllGrfTextures() {

	if (_bipsTexture) {
		_bipsTexture->applyPaletteEffect(effectGrayscalePalette);
	}
	if (_boxTexture) {
		_boxTexture->applyPaletteEffect(effectGrayscalePalette);
	}
	if (_braceTexture) {
		_braceTexture->applyPaletteEffect(effectGrayscalePalette);
	}
	if (_bubbleTexture) {
		_bubbleTexture->applyPaletteEffect(effectGrayscalePalette);
	}
	if (_buttonArrowTexture) {
		_buttonArrowTexture->applyPaletteEffect(effectGrayscalePalette);
	}
	if (_cornerButtonTexture) {
		_cornerButtonTexture->applyPaletteEffect(effectGrayscalePalette);
	}
	if (_dialogBoxTexture) {
		_dialogBoxTexture->applyPaletteEffect(effectGrayscalePalette);
	}
	if (_folderTexture) {
		_folderTexture->applyPaletteEffect(effectGrayscalePalette);
	}
	if (_launchDotTexture) {
		_launchDotTexture->applyPaletteEffect(effectGrayscalePalette);
	}
	if (_movingArrowTexture) {
		_movingArrowTexture->applyPaletteEffect(effectGrayscalePalette);
	}
	if (_progressTexture) {
		_progressTexture->applyPaletteEffect(effectGrayscalePalette);
	}
	if (_scrollWindowTexture) {
		_scrollWindowTexture->applyPaletteEffect(effectGrayscalePalette);
	}
	if (_smallCartTexture) {
		_smallCartTexture->applyPaletteEffect(effectGrayscalePalette);
	}
	if (_startBorderTexture) {
		_startBorderTexture->applyPaletteEffect(effectGrayscalePalette);
	}
	if (_startTextTexture) {
		_startTextTexture->applyPaletteEffect(effectGrayscalePalette);
	}
	if (_wirelessIconsTexture) {
		_wirelessIconsTexture->applyPaletteEffect(effectGrayscalePalette);
	}
	if (_settingsIconTexture) {
		_settingsIconTexture->applyPaletteEffect(effectGrayscalePalette);
	}
	if (_manualIconTexture) {
		_manualIconTexture->applyPaletteEffect(effectGrayscalePalette);
	}

	if (_boxFullTexture) {
		_boxFullTexture->applyPaletteEffect(effectGrayscalePalette);
	}
	if (_boxEmptyTexture) {
		_boxEmptyTexture->applyPaletteEffect(effectGrayscalePalette);
	}

	if (_iconGBTexture) {
		_iconGBTexture->applyPaletteEffect(effectGrayscalePalette);
	}
	if (_iconGBATexture) {
		_iconGBATexture->applyPaletteEffect(effectGrayscalePalette);
	}
	if (_iconGBAModeTexture) {
		_iconGBAModeTexture->applyPaletteEffect(effectGrayscalePalette);
	}
	if (_iconGGTexture) {
		_iconGGTexture->applyPaletteEffect(effectGrayscalePalette);
	}
	if (_iconMDTexture) {
		_iconMDTexture->applyPaletteEffect(effectGrayscalePalette);
	}
	if (_iconNESTexture) {
		_iconNESTexture->applyPaletteEffect(effectGrayscalePalette);
	}
	if (_iconSMSTexture) {
		_iconSMSTexture->applyPaletteEffect(effectGrayscalePalette);
	}
	if (_iconSNESTexture) {
		_iconSNESTexture->applyPaletteEffect(effectGrayscalePalette);
	}
	if (_iconPLGTexture) {
		_iconPLGTexture->applyPaletteEffect(effectGrayscalePalette);
	}
	if (_iconUnknownTexture) {
		_iconUnknownTexture->applyPaletteEffect(effectGrayscalePalette);
	}
}

u16 *ThemeTextures::bmpImageBuffer() { return _bmpImageBuffer; }
u16 *ThemeTextures::photoBuffer() { return _photoBuffer; }

void ThemeTextures::videoSetup() {
	printf("tex().videoSetup()\n");
	//////////////////////////////////////////////////////////
	videoSetMode(MODE_5_3D | DISPLAY_BG3_ACTIVE);
	videoSetModeSub(MODE_3_2D | DISPLAY_BG3_ACTIVE);

	// Initialize gl2d
	glScreen2D();
	// Make gl2d render on transparent stage.
	glClearColor(31, 31, 31, 0);
	glDisable(GL_CLEAR_BMP);

	// Clear the GL texture state
	glResetTextures();

	// Set up enough texture memory for our textures
	// Bank A is just 128kb and we are using 194 kb of
	// sprites
	vramSetBankA(VRAM_A_TEXTURE);
	vramSetBankB(VRAM_B_MAIN_BG_0x06020000);
	vramSetBankC(VRAM_C_SUB_BG_0x06200000);
	vramSetBankD(VRAM_D_MAIN_BG_0x06000000);
	vramSetBankE(VRAM_E_TEX_PALETTE);
	vramSetBankF(VRAM_F_TEX_PALETTE_SLOT4);
	vramSetBankG(VRAM_G_MAIN_SPRITE);
	vramSetBankH(VRAM_H_SUB_BG_EXT_PALETTE);
	vramSetBankI(VRAM_I_SUB_SPRITE_EXT_PALETTE);

	//	vramSetBankH(VRAM_H_SUB_BG_EXT_PALETTE); // Not sure this does anything...
	lcdMainOnBottom();

	int bg3Main = bgInit(3, BgType_Bmp16, BgSize_B16_256x256, 0, 0);
	bgSetPriority(bg3Main, 3);

	int bg2Main = bgInit(2, BgType_Bmp8, BgSize_B8_256x256, 6, 0);
	nocashMessage(std::to_string(bg2Main).c_str());
	bgSetPriority(bg2Main, 0);

	int bg3Sub = bgInitSub(3, BgType_Bmp16, BgSize_B16_256x256, 0, 0);
	bgSetPriority(bg3Sub, 3);

	bgSetPriority(0, 1); // Set 3D to below text

	REG_BLDCNT = BLEND_SRC_BG3 | BLEND_FADE_BLACK;
}