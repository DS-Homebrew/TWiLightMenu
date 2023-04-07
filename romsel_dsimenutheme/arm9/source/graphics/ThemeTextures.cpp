
#include "ThemeTextures.h"
#include "ThemeConfig.h"

#include <nds.h>
#include <nds/arm9/dldi.h>
#include "common/twlmenusettings.h"
#include "common/systemdetails.h"
#include "common/logging.h"
#include "myDSiMode.h"

#include "paletteEffects.h"
#include "themefilenames.h"
#include "tool/colortool.h"
// Graphic files
#include "../include/startborderpal.h"

#include "color.h"
#include "errorScreen.h"
#include "fileBrowse.h"
#include "common/lzss.h"
#include "common/tonccpy.h"
#include "common/lodepng.h"
#include "ndsheaderbanner.h"
#include "ndma.h"


extern bool useTwlCfg;

//extern bool widescreenEffects;

extern u32 rotatingCubesLoaded;
extern bool rocketVideo_playVideo;
extern u8 *rotatingCubesLocation;

// #include <nds/arm9/decompress.h>
// extern u16 bmpImageBuffer[256*192];
extern bool showColon;

static u16 _bmpImageBuffer[256 * 192] = {0};
static u16* _bmpImageBuffer2 = (u16*)_bmpImageBuffer;
static u16 _bgMainBuffer[256 * 192] = {0};
static u16 _bgSubBuffer[256 * 192] = {0};
static u16 _photoBuffer[208 * 156] = {0};
static u16 _topBorderBuffer[256 * 192] = {0};
static u16* _bgSubBuffer2 = (u16*)_bgSubBuffer;
static u16* _photoBuffer2 = (u16*)_photoBuffer;
// DSi mode double-frame buffers
//static u16* _frameBuffer[2] = {(u16*)0x02F80000, (u16*)0x02F98000};
static u16* _frameBufferBot[2] = {(u16*)_bmpImageBuffer, (u16*)_bmpImageBuffer};

static bool topBorderBufferLoaded = false;
bool boxArtColorDeband = false;

static u8* boxArtCache = (u8*)NULL;	// Size: 0x1B8000
static bool boxArtFound[40] = {false};
int boxArtType[40] = {0};	// 0: NDS, 1: FDS/GBA/GBC/GB, 2: NES/GEN/MD/SFC, 3: SNES

ThemeTextures::ThemeTextures()
    : bubbleTexID(0), bipsTexID(0), scrollwindowTexID(0), buttonarrowTexID(0),
      movingarrowTexID(0), launchdotTexID(0), startTexID(0), startbrdTexID(0), settingsTexID(0), manualTexID(0), braceTexID(0),
      boxfullTexID(0), boxemptyTexID(0), folderTexID(0), cornerButtonTexID(0), smallCartTexID(0), progressTexID(0),
      dialogboxTexID(0), wirelessiconTexID(0), _cachedVolumeLevel(-1), _cachedBatteryLevel(-1), _profileNameLoaded(false) {
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
	return palette + (getFavoriteColor() * 16);
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
	if (ms().theme == TWLSettings::EThemeSaturn || ms().theme == TWLSettings::EThemeHBL) return;
	glBindTexture(0, dialogboxTexID);
	glColorSubTableEXT(0, 0, _dialogBoxTexture->paletteLength(), 0, 0, _dialogBoxTexture->palette());
	if (ms().theme != TWLSettings::ETheme3DS) {
		glBindTexture(0, cornerButtonTexID);
		glColorSubTableEXT(0, 0, 16, 0, 0, _cornerButtonTexture->palette());
	}
}

void ThemeTextures::loadBackgrounds() {
	// 0: Top, 1: Bottom, 2: Bottom Bubble, 3: Moving, 4: MovingLeft, 5: MovingRight

	// We reuse the _topBackgroundTexture as a buffer.
	_backgroundTextures.emplace_back(TFN_BG_TOPBG, TFN_FALLBACK_BG_TOPBG);
		
	
	if (ms().theme == TWLSettings::ETheme3DS && !sys().isRegularDS()) {
		_backgroundTextures.emplace_back(TFN_BG_BOTTOMBG, TFN_FALLBACK_BG_BOTTOMBG);
		_backgroundTextures.emplace_back(TFN_BG_BOTTOMBUBBLEBG, TFN_FALLBACK_BG_BOTTOMBUBBLEBG);
		return;
	}

	if (ms().theme == TWLSettings::ETheme3DS && sys().isRegularDS()) {
		_backgroundTextures.emplace_back(TFN_BG_BOTTOMBG_DS, TFN_FALLBACK_BG_BOTTOMBG_DS);
		_backgroundTextures.emplace_back(TFN_BG_BOTTOMBUBBLEBG_DS, TFN_FALLBACK_BG_BOTTOMBUBBLEBG_DS);
		return;
	}
	// DSi Theme
	if (ms().macroMode) {
		if (Texture::exists(TFN_BG_BOTTOMBG_MACRO)) {
			_backgroundTextures.emplace_back(TFN_BG_BOTTOMBG_MACRO, TFN_FALLBACK_BG_BOTTOMBG);
		} else {
			_backgroundTextures.emplace_back(TFN_BG_BOTTOMBG, TFN_FALLBACK_BG_BOTTOMBG);
		}

		if (Texture::exists(TFN_BG_BOTTOMBUBBLEBG_MACRO)) {
			_backgroundTextures.emplace_back(TFN_BG_BOTTOMBUBBLEBG_MACRO, TFN_FALLBACK_BG_BOTTOMBUBBLEBG_MACRO);
		} else {
			_backgroundTextures.emplace_back(TFN_BG_BOTTOMBUBBLEBG, TFN_FALLBACK_BG_BOTTOMBUBBLEBG);
		}

		if (ms().theme == TWLSettings::EThemeDSi && Texture::exists(TFN_BG_BOTTOMMOVINGBG_MACRO)) {
			_backgroundTextures.emplace_back(TFN_BG_BOTTOMMOVINGBG_MACRO, TFN_FALLBACK_BG_BOTTOMMOVINGBG);
		} else {
			_backgroundTextures.emplace_back(TFN_BG_BOTTOMMOVINGBG, TFN_FALLBACK_BG_BOTTOMMOVINGBG);
		}
	} else {
		_backgroundTextures.emplace_back(TFN_BG_BOTTOMBG, TFN_FALLBACK_BG_BOTTOMBG);
		_backgroundTextures.emplace_back(TFN_BG_BOTTOMBUBBLEBG, TFN_FALLBACK_BG_BOTTOMBUBBLEBG);
		if (ms().theme == TWLSettings::EThemeDSi) _backgroundTextures.emplace_back(TFN_BG_BOTTOMMOVINGBG, TFN_FALLBACK_BG_BOTTOMMOVINGBG);
	}
	
}

void ThemeTextures::loadHBTheme() {	
	logPrint("tex().loadHBTheme()\n");

	// iprintf("tex().loadBackgrounds()\n");
	loadBackgrounds();
	// iprintf("tex().loadUITextures()\n");
	loadUITextures();

	// iprintf("tex().loadVolumeTextures()\n");
	loadVolumeTextures();
	// iprintf("tex().loadBatteryTextures()\n");
	loadBatteryTextures();
	// iprintf("tex().loadIconTextures()\n");
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
		// iprintf("tex().applyGrayscaleToAllGrfTextures()\n");
		applyGrayscaleToAllGrfTextures();
	}

	
	// iprintf("tex().loadWirelessIcons(*_wirelessIconsTexture)\n");
	loadWirelessIcons(*_wirelessIconsTexture);
	// iprintf("tex().loadSettingsImage(*_settingsIconTexture)\n");
	loadSettingsImage(*_settingsIconTexture);
	// iprintf("tex().loadBraceImage(*_braceTexture)\n");
	loadBraceImage(*_braceTexture);

	// iprintf("tex().loadBoxfullImage(*_boxFullTexture)\n");
	loadBoxfullImage(*_boxFullTexture);
	// iprintf("tex().loadBoxEmptyImage(*_boxFullTexture)\n");
	loadBoxemptyImage(*_boxEmptyTexture);

	// iprintf("tex().loadManualImage(*_manualIconTexture)\n");
	loadManualImage(*_manualIconTexture);
	// iprintf("tex().loadCornerButtonImage(*_cornerButtonTexture, (32 / 16) * (32 / 32), 32, 32)\n");
	loadCornerButtonImage(*_cornerButtonTexture, (32 / 16) * (32 / 32), 32, 32);
	// iprintf("tex().loadSmallCartImage(*_smallCartTexture)\n");
	loadSmallCartImage(*_smallCartTexture);
	// iprintf("tex().loadFolderImage(*_folderTexture)\n");
	loadFolderImage(*_folderTexture);
	
	// iprintf("tex().loadProgressImage(*_progressTexture)\n");
	loadProgressImage(*_progressTexture);
	// iprintf("tex().loadWirelessIcons(*_wirelessIconsTexture)\n");
	loadWirelessIcons(*_wirelessIconsTexture);
	
}

void ThemeTextures::loadSaturnTheme() {	
	logPrint("tex().loadSaturnTheme()\n");

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
	logPrint("tex().load3DSTheme()\n");

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

	applyUserPaletteToAllGrfTextures();

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
	logPrint("tex().loadDSiTheme()\n");

	//iprintf("loadBackgrounds()\n");
	loadBackgrounds();
	//iprintf("loadUITextures()\n");
	loadUITextures();

	//iprintf("loadVolumeTextures()\n");
	loadVolumeTextures();
	//iprintf("loadBatteryTextures()\n");
	loadBatteryTextures();
	//iprintf("loadIconTextures()\n");
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
	_manualIconTexture = std::make_unique<Texture>(TFN_GRF_ICON_MANUAL, TFN_FALLBACK_GRF_ICON_MANUAL);

	// Apply the DSi palette shifts
	applyUserPaletteToAllGrfTextures();

	if (ms().colorMode == 1) {
		applyGrayscaleToAllGrfTextures();
	}

	//iprintf("loadBipsImage(*_bipsTexture)\n");
	loadBipsImage(*_bipsTexture);

	//iprintf("loadBubbleImage(*_bubbleTexture, tc().bubbleTipSpriteW(), tc().bubbleTipSpriteH())\n");
	loadBubbleImage(*_bubbleTexture, tc().bubbleTipSpriteW(), tc().bubbleTipSpriteH());
	//iprintf("loadScrollwindowImage(*_scrollWindowTexture)\n");
	loadScrollwindowImage(*_scrollWindowTexture);
	//iprintf("loadWirelessIcons(*_wirelessIconsTexture)\n");
	loadWirelessIcons(*_wirelessIconsTexture);
	//iprintf("loadSettingsImage(*_settingsIconTexture)\n");
	loadSettingsImage(*_settingsIconTexture);
	//iprintf("loadBraceImage(*_braceTexture)\n");
	loadBraceImage(*_braceTexture);

	//iprintf("loadStartImage(*_startTextTexture)\n");
	loadStartImage(*_startTextTexture);
	//iprintf("loadStartbrdImage(*_startBorderTexture, tc().startBorderSpriteH())\n");
	loadStartbrdImage(*_startBorderTexture, tc().startBorderSpriteH());

	//iprintf("loadButtonarrowImage(*_buttonArrowTexture)\n");
	loadButtonarrowImage(*_buttonArrowTexture);
	//iprintf("loadMovingarrowImage(*_movingArrowTexture)\n");
	loadMovingarrowImage(*_movingArrowTexture);
	//iprintf("loadLaunchdotImage(*_launchDotTexture)\n");
	loadLaunchdotImage(*_launchDotTexture);
	//iprintf("loadDialogboxImage(*_dialogBoxTexture)\n");
	loadDialogboxImage(*_dialogBoxTexture);

	// careful here, it's boxTexture, not boxFulltexture.
	//iprintf("loadBoxfullImage(*_boxTexture)\n");
	loadBoxfullImage(*_boxTexture);

	//iprintf("loadManualImage(*_manualIconTexture)\n");
	loadManualImage(*_manualIconTexture);
	//iprintf("loadCornerButtonImage(*_cornerButtonTexture, (32 / 16) * (32 / 32), 32, 32)\n");
	loadCornerButtonImage(*_cornerButtonTexture, (32 / 16) * (32 / 32), 32, 32);
	//iprintf("loadSmallCartImage(*_smallCartTexture)\n");
	loadSmallCartImage(*_smallCartTexture);
	//iprintf("loadFolderImage(*_folderTexture)\n");
	loadFolderImage(*_folderTexture);

	//iprintf("loadProgressImage(*_progressTexture)\n");
	loadProgressImage(*_progressTexture);
	//iprintf("loadWirelessIcons(*_wirelessIconsTexture)\n");
	loadWirelessIcons(*_wirelessIconsTexture);
}

void ThemeTextures::loadVolumeTextures() {
	if (dsiFeatures()) {
		_volume0Texture = std::make_unique<Texture>(TFN_VOLUME0, TFN_FALLBACK_VOLUME0);
		_volume1Texture = std::make_unique<Texture>(TFN_VOLUME1, TFN_FALLBACK_VOLUME1);
		_volume2Texture = std::make_unique<Texture>(TFN_VOLUME2, TFN_FALLBACK_VOLUME2);
		_volume3Texture = std::make_unique<Texture>(TFN_VOLUME3, TFN_FALLBACK_VOLUME3);
		_volume4Texture = std::make_unique<Texture>(TFN_VOLUME4, TFN_FALLBACK_VOLUME4);
	}
}

void ThemeTextures::loadBatteryTextures() {
	if (dsiFeatures()) {
		_batterychargeTexture = std::make_unique<Texture>(TFN_BATTERY_CHARGE, TFN_FALLBACK_BATTERY_CHARGE);
		_batterychargeblinkTexture = std::make_unique<Texture>(TFN_BATTERY_CHARGE_BLINK, TFN_FALLBACK_BATTERY_CHARGE_BLINK);
		_battery0Texture = std::make_unique<Texture>(TFN_BATTERY0, TFN_FALLBACK_BATTERY0);
		if (ms().consoleModel < 2 && ms().powerLedColor && tc().purpleBatteryAvailable()) {
			_battery1Texture = std::make_unique<Texture>(TFN_BATTERY1_PURPLE, TFN_FALLBACK_BATTERY1_PURPLE);
			_battery2Texture = std::make_unique<Texture>(TFN_BATTERY2_PURPLE, TFN_FALLBACK_BATTERY2_PURPLE);
			_battery3Texture = std::make_unique<Texture>(TFN_BATTERY3_PURPLE, TFN_FALLBACK_BATTERY3_PURPLE);
			_battery4Texture = std::make_unique<Texture>(TFN_BATTERY4_PURPLE, TFN_FALLBACK_BATTERY4_PURPLE);
		} else {
			_battery1Texture = std::make_unique<Texture>(TFN_BATTERY1, TFN_FALLBACK_BATTERY1);
			_battery2Texture = std::make_unique<Texture>(TFN_BATTERY2, TFN_FALLBACK_BATTERY2);
			_battery3Texture = std::make_unique<Texture>(TFN_BATTERY3, TFN_FALLBACK_BATTERY3);
			_battery4Texture = std::make_unique<Texture>(TFN_BATTERY4, TFN_FALLBACK_BATTERY4);
		}
	} else {
		if (!sys().isDSPhat()) {
			_batterychargeTexture = std::make_unique<Texture>(TFN_BATTERY_CHARGE, TFN_FALLBACK_BATTERY_CHARGE);
			_batterychargeblinkTexture = std::make_unique<Texture>(TFN_BATTERY_CHARGE_BLINK, TFN_FALLBACK_BATTERY_CHARGE_BLINK);
		}
		_batteryfullTexture = std::make_unique<Texture>(TFN_BATTERY_FULL, TFN_FALLBACK_BATTERY_FULL);
		_batteryfullDSTexture = std::make_unique<Texture>(TFN_BATTERY_FULLDS, TFN_FALLBACK_BATTERY_FULLDS);
		_batterylowTexture = std::make_unique<Texture>(TFN_BATTERY_LOW, TFN_FALLBACK_BATTERY_LOW);
	}
}

void ThemeTextures::loadUITextures() {
	_dateTimeFont = std::make_unique<FontGraphic>(((access((TFN_FONT_DATE_TIME).c_str(), F_OK) == 0) ? TFN_FONT_DATE_TIME : TFN_FALLBACK_FONT_DATE_TIME).c_str(), false);
	if (access((TFN_FONT_USERNAME).c_str(), F_OK) == 0) {
		_usernameFont = std::make_unique<FontGraphic>((TFN_FONT_USERNAME).c_str(), false);
	}

	if (ms().theme != TWLSettings::EThemeHBL) {
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
	_iconSGTexture = std::make_unique<Texture>(TFN_GRF_ICON_SG, TFN_FALLBACK_GRF_ICON_SG);
	_iconSMSTexture = std::make_unique<Texture>(TFN_GRF_ICON_SMS, TFN_FALLBACK_GRF_ICON_SMS);
	_iconSNESTexture = std::make_unique<Texture>(TFN_GRF_ICON_SNES, TFN_FALLBACK_GRF_ICON_SNES);
	_iconPLGTexture = std::make_unique<Texture>(TFN_GRF_ICON_PLG, TFN_FALLBACK_GRF_ICON_PLG);
	_iconA26Texture = std::make_unique<Texture>(TFN_GRF_ICON_A26, TFN_FALLBACK_GRF_ICON_A26);
	_iconCOLTexture = std::make_unique<Texture>(TFN_GRF_ICON_COL, TFN_FALLBACK_GRF_ICON_COL);
	_iconM5Texture = std::make_unique<Texture>(TFN_GRF_ICON_M5, TFN_FALLBACK_GRF_ICON_M5);
	_iconINTTexture = std::make_unique<Texture>(TFN_GRF_ICON_INT, TFN_FALLBACK_GRF_ICON_INT);
	_iconPCETexture = std::make_unique<Texture>(TFN_GRF_ICON_PCE, TFN_FALLBACK_GRF_ICON_PCE);
	_iconWSTexture = std::make_unique<Texture>(TFN_GRF_ICON_WS, TFN_FALLBACK_GRF_ICON_WS);
	_iconNGPTexture = std::make_unique<Texture>(TFN_GRF_ICON_NGP, TFN_FALLBACK_GRF_ICON_NGP);
	_iconCPCTexture = std::make_unique<Texture>(TFN_GRF_ICON_CPC, TFN_FALLBACK_GRF_ICON_CPC);
	_iconVIDTexture = std::make_unique<Texture>(TFN_GRF_ICON_VID, TFN_FALLBACK_GRF_ICON_VID);
	_iconIMGTexture = std::make_unique<Texture>(TFN_GRF_ICON_IMG, TFN_FALLBACK_GRF_ICON_IMG);
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
	if (ms().macroMode)
		return _bgSubBuffer;

	u16* bgLoc = BG_GFX_SUB;
	if (boxArtColorDeband) {
		bgLoc = _frameBufferBot[0];
	}
	dmaCopyWords(0, bgLoc, _bgSubBuffer, sizeof(u16) * BG_BUFFER_PIXELCOUNT);
	if (boxArtColorDeband) {
		dmaCopyWords(0, _frameBufferBot[1], _bgSubBuffer2, sizeof(u16) * BG_BUFFER_PIXELCOUNT);
	}
	return _bgSubBuffer;
}

void ThemeTextures::commitBgSubModify() {
	if (ms().macroMode)
		return;

	u16* bgLoc = BG_GFX_SUB;
	if (boxArtColorDeband) {
		bgLoc = _frameBufferBot[0];
	}
	DC_FlushRange(_bgSubBuffer, sizeof(u16) * BG_BUFFER_PIXELCOUNT);
	if (boxArtColorDeband) {
		DC_FlushRange(_bgSubBuffer2, sizeof(u16) * BG_BUFFER_PIXELCOUNT);
	}
	while (REG_VCOUNT != 191); // Fix screen tearing
	dmaCopyWords(2, _bgSubBuffer, bgLoc, sizeof(u16) * BG_BUFFER_PIXELCOUNT);
	if (boxArtColorDeband) {
		dmaCopyWords(2, _bgSubBuffer2, _frameBufferBot[1], sizeof(u16) * BG_BUFFER_PIXELCOUNT);
	}
}

void ThemeTextures::commitBgSubModifyAsync() {
	if (ms().macroMode)
		return;

	u16* bgLoc = BG_GFX_SUB;
	if (boxArtColorDeband) {
		bgLoc = _frameBufferBot[0];
	}
	DC_FlushRange(_bgSubBuffer, sizeof(u16) * BG_BUFFER_PIXELCOUNT);
	if (boxArtColorDeband) {
		DC_FlushRange(_bgSubBuffer2, sizeof(u16) * BG_BUFFER_PIXELCOUNT);
	}
	while (REG_VCOUNT != 191); // Fix screen tearing
	dmaCopyWordsAsynch(2, _bgSubBuffer, bgLoc, sizeof(u16) * BG_BUFFER_PIXELCOUNT);
	if (boxArtColorDeband) {
		ndmaCopyWordsAsynch(2, _bgSubBuffer2, _frameBufferBot[1], sizeof(u16) * BG_BUFFER_PIXELCOUNT);
	}
}

u16 *ThemeTextures::beginBgMainModify() {
	u16* bgLoc = BG_GFX;
	/*if (boxArtColorDeband) {
		bgLoc = _frameBufferBot[0];
	}*/
	dmaCopyWords(0, bgLoc, _bgMainBuffer, sizeof(u16) * BG_BUFFER_PIXELCOUNT);
	/*if (ndmaEnabled()) {
		dmaCopyWords(0, _frameBuffer[1], _bgMainBuffer, sizeof(u16) * BG_BUFFER_PIXELCOUNT);
	}*/
	return _bgMainBuffer;
}

void ThemeTextures::commitBgMainModify() {
	u16* bgLoc = BG_GFX;
	/*if (boxArtColorDeband) {
		bgLoc = _frameBufferBot[0];
	}*/
	DC_FlushRange(_bgMainBuffer, sizeof(u16) * BG_BUFFER_PIXELCOUNT);
	dmaCopyWords(2, _bgMainBuffer, bgLoc, sizeof(u16) * BG_BUFFER_PIXELCOUNT);
	/*if (ndmaEnabled()) {
		dmaCopyWords(2, _bgMainBuffer, _frameBuffer[1], sizeof(u16) * BG_BUFFER_PIXELCOUNT);
	}*/
}

void ThemeTextures::commitBgMainModifyAsync() {
	u16* bgLoc = BG_GFX;
	/*if (boxArtColorDeband) {
		bgLoc = _frameBufferBot[0];
	}*/
	DC_FlushRange(_bgMainBuffer, sizeof(u16) * BG_BUFFER_PIXELCOUNT);
	dmaCopyWordsAsynch(2, _bgMainBuffer, bgLoc, sizeof(u16) * BG_BUFFER_PIXELCOUNT);
	/*if (boxArtColorDeband) {
		ndmaCopyWordsAsynch(2, _bgMainBuffer, _frameBuffer[1], sizeof(u16) * BG_BUFFER_PIXELCOUNT);
	}*/
}

void ThemeTextures::drawTopBg() {
	beginBgSubModify();

	_backgroundTextures[0].copy(_bgSubBuffer, false);

	if (ms().colorMode == 1) {
		for (u16 i = 0; i < BG_BUFFER_PIXELCOUNT; i++) {
			_bgSubBuffer[i] = convertVramColorToGrayscale(_bgSubBuffer[i]);
		}
	}

	if (boxArtColorDeband) {
		tonccpy((u8*)_bgSubBuffer2, (u8*)_bgSubBuffer, 0x18000);
	}
	commitBgSubModify();
}

void ThemeTextures::drawBottomBg(int index) {

	// clamp index
	if (index < 1)
		index = 1;
	if (index > 3)
		index = 3;
	if (index > 2 && ms().theme == TWLSettings::ETheme3DS)
		index = 2;
	beginBgMainModify();

	_backgroundTextures[index].copy(_bgMainBuffer, false);

	if (ms().colorMode == 1) {
		for (u16 i = 0; i < BG_BUFFER_PIXELCOUNT; i++) {
			_bgMainBuffer[i] = convertVramColorToGrayscale(_bgMainBuffer[i]);
		}
	}

	commitBgMainModify();
}

void ThemeTextures::clearTopScreen() {
	beginBgSubModify();
	u16 val = 0xFFFF;
	for (int i = 0; i < BG_BUFFER_PIXELCOUNT; i++) {
		_bgSubBuffer[i] = val;
		if (boxArtColorDeband) {
			_bgSubBuffer2[i] = val;
		}
	}
	commitBgSubModify();
}

void ThemeTextures::drawProfileName() {
	if (_profileNameLoaded || ms().theme == TWLSettings::EThemeSaturn || ms().theme == TWLSettings::EThemeHBL) return;

	if (!topBorderBufferLoaded) {
		_backgroundTextures[ms().macroMode].copy(_topBorderBuffer, false);
		if (ms().colorMode == 1) {
			for (u16 i = 0; i < BG_BUFFER_PIXELCOUNT; i++) {
				_topBorderBuffer[i] =
					convertVramColorToGrayscale(_topBorderBuffer[i]);
			}
		}
		topBorderBufferLoaded = true;
	}

	// Load username
	int xPos = (dsiFeatures() ? tc().usernameRenderX() : tc().usernameRenderXDS());
	int yPos = tc().usernameRenderY();
	char16_t username[11] = {0};
	tonccpy(username, useTwlCfg ? (s16 *)0x02000448 : PersonalData->name, 10 * sizeof(char16_t));

	toncset16(FontGraphic::textBuf[1], 0, 256 * usernameFont()->height());
	usernameFont()->print(0, 0, true, username, Alignment::left, FontPalette::name);
	int width = usernameFont()->calcWidth(username);

	// Copy to background
	for (int y = 0; y < usernameFont()->height() && yPos + y < SCREEN_HEIGHT; y++) {
		if (yPos + y < 0) continue;
		for (int x = 0; x < width && xPos + x < SCREEN_WIDTH; x++) {
			if (xPos + x < 0) continue;
			int px = FontGraphic::textBuf[1][y * 256 + x];
			u16 bg = _topBorderBuffer[(yPos + y) * 256 + (xPos + x)];
			u16 val = px ? alphablend(BG_PALETTE[px], bg, (px % 4) < 2 ? 128 : 224) : bg;

			if (ms().macroMode) {
				_bgMainBuffer[(yPos + y) * 256 + (xPos + x)] = val;
			} else {
				_bgSubBuffer[(yPos + y) * 256 + (xPos + x)] = val;
				if (boxArtColorDeband) {
					_bgSubBuffer2[(yPos + y) * 256 + (xPos + x)] = val;
				}
			}
		}
	}

	ms().macroMode ? commitBgMainModify() : commitBgSubModify();
	_profileNameLoaded = true;
}


ITCM_CODE void ThemeTextures::resetProfileName() {
	_profileNameLoaded = false;
}

void ThemeTextures::loadBoxArtToMem(const char *filename, int num) {
	if (num < 0 || num > 39) {
		return;
	}

	extern off_t getFileSize(const char *fileName);
	off_t filesize = getFileSize(filename);

	if (filesize == 0 || filesize > 0xB000) {
		boxArtFound[num] = false;
		//filename = "nitro:/graphics/boxart_unknown.bmp";
		//file = fopen(filename, "rb");
		return;
	}

	boxArtFound[num] = true;

	FILE *file = fopen(filename, "rb");
	fread(boxArtCache+(num*0xB000), 1, 0xB000, file);
	fclose(file);
}

void ThemeTextures::drawBoxArt(const char *filename) {
	bool found = true;

	if (access(filename, F_OK) != 0) {
		switch (boxArtType[CURPOS]) {
			case -1:
				return;
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
		found = false;
	}

	beginBgSubModify();

	std::vector<unsigned char> image;
	uint imageXpos, imageYpos, imageWidth, imageHeight;
	lodepng::decode(image, imageWidth, imageHeight, filename);
	bool alternatePixel = false;
	if (imageWidth > 256 || imageHeight > 192)	return;

	imageXpos = (256-imageWidth)/2;
	imageYpos = (192-imageHeight)/2;

	if (!found) {
		int photoXstart = imageXpos-24;
		int photoXend = (imageXpos+imageWidth)-24;
		int photoY = imageYpos-24;
		if (!tc().renderPhoto()) {
			photoXstart = imageXpos;
			photoXend = imageXpos+imageWidth;
			photoY = imageYpos;
		}
		int photoX = photoXstart;
		for (uint i=0;i<image.size()/4;i++) {
			u16 color = image[i*4]>>3 | (image[(i*4)+1]>>3)<<5 | (image[(i*4)+2]>>3)<<10 | BIT(15);
			u16 imgSrc = _photoBuffer[(photoY*208)+photoX];
			u16 imgSrc2 = _photoBuffer2[(photoY*208)+photoX];
			if (!tc().renderPhoto()) {
				imgSrc = _bgSubBuffer[(photoY*256)+photoX];
				imgSrc2 = imgSrc;
			}
			if (image[(i*4)+3] == 0) {
				_bmpImageBuffer[i] = color;
				if (boxArtColorDeband) _bmpImageBuffer2[i] = color;
			} else {
				_bmpImageBuffer[i] = alphablend(color, imgSrc, image[(i*4)+3]);
				if (boxArtColorDeband) _bmpImageBuffer2[i] = alphablend(color, imgSrc2, image[(i*4)+3]);
			}
			photoX++;
			if (photoX == photoXend) {
				photoX = photoXstart;
				photoY++;
			}
		}
	} else
	for (uint i=0;i<image.size()/4;i++) {
		if (boxArtColorDeband) {
			image[(i*4)+3] = 0;
			if (alternatePixel) {
				if (image[(i*4)] >= 0x4) {
					image[(i*4)] -= 0x4;
					image[(i*4)+3] |= BIT(0);
				}
				if (image[(i*4)+1] >= 0x4) {
					image[(i*4)+1] -= 0x4;
					image[(i*4)+3] |= BIT(1);
				}
				if (image[(i*4)+2] >= 0x4) {
					image[(i*4)+2] -= 0x4;
					image[(i*4)+3] |= BIT(2);
				}
			}
		}
		_bmpImageBuffer[i] = image[i*4]>>3 | (image[(i*4)+1]>>3)<<5 | (image[(i*4)+2]>>3)<<10 | BIT(15);
		if (ms().colorMode == 1) {
			_bmpImageBuffer[i] = convertVramColorToGrayscale(_bmpImageBuffer[i]);
		}
		if (boxArtColorDeband) {
			if (alternatePixel) {
				if (image[(i*4)+3] & BIT(0)) {
					image[(i*4)] += 0x4;
				}
				if (image[(i*4)+3] & BIT(1)) {
					image[(i*4)+1] += 0x4;
				}
				if (image[(i*4)+3] & BIT(2)) {
					image[(i*4)+2] += 0x4;
				}
			} else {
				if (image[(i*4)] >= 0x4) {
					image[(i*4)] -= 0x4;
				}
				if (image[(i*4)+1] >= 0x4) {
					image[(i*4)+1] -= 0x4;
				}
				if (image[(i*4)+2] >= 0x4) {
					image[(i*4)+2] -= 0x4;
				}
			}
			_bmpImageBuffer2[i] = image[i*4]>>3 | (image[(i*4)+1]>>3)<<5 | (image[(i*4)+2]>>3)<<10 | BIT(15);
			if (ms().colorMode == 1) {
				_bmpImageBuffer2[i] = convertVramColorToGrayscale(_bmpImageBuffer[i]);
			}
			if ((i % imageWidth) == imageWidth-1) alternatePixel = !alternatePixel;
			alternatePixel = !alternatePixel;
		}
	}

	u16 *src = _bmpImageBuffer;
	u16 *src2 = _bmpImageBuffer2;
	for (uint y = 0; y < imageHeight; y++) {
		for (uint x = 0; x < imageWidth; x++) {
			_bgSubBuffer[(y+imageYpos) * 256 + imageXpos + x] = *(src++);
			if (boxArtColorDeband) {
				_bgSubBuffer2[(y+imageYpos) * 256 + imageXpos + x] = *(src2++);
			}
		}
	}
	commitBgSubModify();
}

void ThemeTextures::drawBoxArtFromMem(int num) {
	if (num < 0 || num > 39) {
		return;
	}

	if (!boxArtFound[num]) {
		if (boxArtType[CURPOS] != -1) {
			drawBoxArt("nitro:/null.png");
		}
		return;
	}

	uint imageXpos, imageYpos, imageWidth, imageHeight;

	// Start loading
	beginBgSubModify();
	std::vector<unsigned char> image;
	lodepng::decode(image, imageWidth, imageHeight, (unsigned char*)boxArtCache+(num*0xB000), 0xB000);
	bool alternatePixel = false;
	if (imageWidth > 256 || imageHeight > 192)	return;

	for (uint i=0;i<image.size()/4;i++) {
		if (boxArtColorDeband) {
			image[(i*4)+3] = 0;
			if (alternatePixel) {
				if (image[(i*4)] >= 0x4) {
					image[(i*4)] -= 0x4;
					image[(i*4)+3] |= BIT(0);
				}
				if (image[(i*4)+1] >= 0x4) {
					image[(i*4)+1] -= 0x4;
					image[(i*4)+3] |= BIT(1);
				}
				if (image[(i*4)+2] >= 0x4) {
					image[(i*4)+2] -= 0x4;
					image[(i*4)+3] |= BIT(2);
				}
			}
		}
		_bmpImageBuffer[i] = image[i*4]>>3 | (image[(i*4)+1]>>3)<<5 | (image[(i*4)+2]>>3)<<10 | BIT(15);
		if (ms().colorMode == 1) {
			_bmpImageBuffer[i] = convertVramColorToGrayscale(_bmpImageBuffer[i]);
		}
		if (boxArtColorDeband) {
			if (alternatePixel) {
				if (image[(i*4)+3] & BIT(0)) {
					image[(i*4)] += 0x4;
				}
				if (image[(i*4)+3] & BIT(1)) {
					image[(i*4)+1] += 0x4;
				}
				if (image[(i*4)+3] & BIT(2)) {
					image[(i*4)+2] += 0x4;
				}
			} else {
				if (image[(i*4)] >= 0x4) {
					image[(i*4)] -= 0x4;
				}
				if (image[(i*4)+1] >= 0x4) {
					image[(i*4)+1] -= 0x4;
				}
				if (image[(i*4)+2] >= 0x4) {
					image[(i*4)+2] -= 0x4;
				}
			}
			_bmpImageBuffer2[i] = image[i*4]>>3 | (image[(i*4)+1]>>3)<<5 | (image[(i*4)+2]>>3)<<10 | BIT(15);
			if (ms().colorMode == 1) {
				_bmpImageBuffer2[i] = convertVramColorToGrayscale(_bmpImageBuffer[i]);
			}
			if ((i % imageWidth) == imageWidth-1) alternatePixel = !alternatePixel;
			alternatePixel = !alternatePixel;
		}
	}

	imageXpos = (256-imageWidth)/2;
	imageYpos = (192-imageHeight)/2;
	u16 *src = _bmpImageBuffer;
	u16 *src2 = _bmpImageBuffer2;
	for (uint y = 0; y < imageHeight;y++) {
		for (uint x = 0; x < imageWidth; x++) {
			_bgSubBuffer[(y+imageYpos) * 256 + imageXpos + x] = *(src++);
			if (boxArtColorDeband) {
				_bgSubBuffer2[(y+imageYpos) * 256 + imageXpos + x] = *(src2++);
			}
		}
	}
	commitBgSubModify();
}

ITCM_CODE void ThemeTextures::drawVolumeImage(int volumeLevel) {
	if (!dsiFeatures())
		return;
	beginBgSubModify();

	const Texture *tex = volumeTexture(volumeLevel);
	const u16 *src = tex->texture();
	int startX = tc().volumeRenderX();
	int startY = tc().volumeRenderY();
	for (uint y = 0; y < tex->texHeight(); y++) {
		for (uint x = 0; x < tex->texWidth(); x++) {
			u16 val = *(src++);
			if (!(val & BIT(15))) // If transparent, restore background image
					val = _topBorderBuffer[(startY + y) * 256 + startX + x];

			_bgSubBuffer[(startY + y) * 256 + startX + x] = val;
			if (boxArtColorDeband) {
				_bgSubBuffer2[(startY + y) * 256 + startX + x] = val;
			}
		}
	}
	commitBgSubModify();
}

ITCM_CODE void ThemeTextures::drawVolumeImageMacro(int volumeLevel) {
	if (!dsiFeatures())
		return;
	beginBgMainModify();

	const Texture *tex = volumeTexture(volumeLevel);
	const u16 *src = tex->texture();
	int startX = tc().volumeRenderX();
	int startY = tc().volumeRenderY();
	for (uint y = 0; y < tex->texHeight(); y++) {
		for (uint x = 0; x < tex->texWidth(); x++) {
			u16 val = *(src++);
			if (!(val & BIT(15))) // If transparent, restore background image
					val = _topBorderBuffer[(startY + y) * 256 + startX + x];

			_bgMainBuffer[(startY + y) * 256 + startX + x] = val;
		}
	}
	commitBgMainModify();
}

ITCM_CODE void ThemeTextures::drawVolumeImageCached() {
	if (ms().macroMode && ms().theme == TWLSettings::EThemeSaturn) return;

	int volumeLevel = getVolumeLevel();
	if (_cachedVolumeLevel != volumeLevel) {
		_cachedVolumeLevel = volumeLevel;
		if (!topBorderBufferLoaded) {
			_backgroundTextures[ms().macroMode].copy(_topBorderBuffer, false);
			if (ms().colorMode == 1) {
				for (u16 i = 0; i < BG_BUFFER_PIXELCOUNT; i++) {
					_topBorderBuffer[i] =
						convertVramColorToGrayscale(_topBorderBuffer[i]);
				}
			}
			topBorderBufferLoaded = true;
		}
		ms().macroMode ? drawVolumeImageMacro(volumeLevel) : drawVolumeImage(volumeLevel);
	}
}

ITCM_CODE void ThemeTextures::resetCachedVolumeLevel() {
	_cachedVolumeLevel = -1;
}

ITCM_CODE int ThemeTextures::getVolumeLevel(void) {
	if (!dsiFeatures())
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

ITCM_CODE int ThemeTextures::getBatteryLevel(void) {
	u8 batteryLevel = sys().batteryStatus();
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

ITCM_CODE void ThemeTextures::drawBatteryImage(int batteryLevel, bool drawDSiMode, bool isRegularDS) {
	// Start loading
	beginBgSubModify();
	const Texture *tex = batteryTexture(batteryLevel, drawDSiMode, isRegularDS);
	const u16 *src = tex->texture();
	for (uint y = tc().batteryRenderY(); y < tc().batteryRenderY() + tex->texHeight(); y++) {
		for (uint x = tc().batteryRenderX(); x < tc().batteryRenderX() + tex->texWidth(); x++) {
			u16 val = *(src++);
			if (!(val & BIT(15))) // If transparent, restore background image
				val = _topBorderBuffer[y * 256 + x];

			_bgSubBuffer[y * 256 + x] = val;
			if (boxArtColorDeband) {
				_bgSubBuffer2[y * 256 + x] = val;
			}
		}
	}
	commitBgSubModify();
}

ITCM_CODE void ThemeTextures::drawBatteryImageMacro(int batteryLevel, bool drawDSiMode, bool isRegularDS) {
	// Start loading
	beginBgMainModify();
	const Texture *tex = batteryTexture(batteryLevel, drawDSiMode, isRegularDS);
	const u16 *src = tex->texture();
	for (uint y = tc().batteryRenderY(); y < tc().batteryRenderY() + tex->texHeight(); y++) {
		for (uint x = tc().batteryRenderX(); x < tc().batteryRenderX() + tex->texWidth(); x++) {
			u16 val = *(src++);
			if (!(val & BIT(15))) // If transparent, restore background image
					val = _topBorderBuffer[y * 256 + x];

			_bgMainBuffer[y * 256 + x] = val;
		}
	}
	commitBgMainModify();
}

ITCM_CODE void ThemeTextures::drawBatteryImageCached() {
	if (ms().macroMode && ms().theme == TWLSettings::EThemeSaturn) return;

	int batteryLevel = getBatteryLevel();
	if (batteryLevel == 0 && showColon)	batteryLevel--;
	else if (batteryLevel == 7 && showColon)	batteryLevel++;
	if (_cachedBatteryLevel != batteryLevel) {
		_cachedBatteryLevel = batteryLevel;
		if (!topBorderBufferLoaded) {
			_backgroundTextures[ms().macroMode].copy(_topBorderBuffer, false);
			if (ms().colorMode == 1) {
				for (u16 i = 0; i < BG_BUFFER_PIXELCOUNT; i++) {
					_topBorderBuffer[i] =
						convertVramColorToGrayscale(_topBorderBuffer[i]);
				}
			}
			topBorderBufferLoaded = true;
		}
		ms().macroMode ? drawBatteryImageMacro(batteryLevel, dsiFeatures(), sys().isRegularDS()) : drawBatteryImage(batteryLevel, dsiFeatures(), sys().isRegularDS());
	}
}

ITCM_CODE void ThemeTextures::resetCachedBatteryLevel() {
	_cachedBatteryLevel = -1;
}

#define TOPLINES 32 * 256
#define BOTTOMOFFSET ((tc().shoulderLRenderY() - 5) * 256)
#define BOTTOMLINES ((192 - (tc().shoulderLRenderY() - 5)) * 256)
// Load .bmp file without overwriting shoulder button images or username
void ThemeTextures::drawTopBgAvoidingShoulders() {

	// Copy current to _bmpImageBuffer
	if (boxArtColorDeband) {
		dmaCopyWords(0, _frameBufferBot[0], _bmpImageBuffer, sizeof(u16) * BG_BUFFER_PIXELCOUNT);
		dmaCopyWords(0, _frameBufferBot[1], _bmpImageBuffer2, sizeof(u16) * BG_BUFFER_PIXELCOUNT);
	} else {
		dmaCopyWords(0, BG_GFX_SUB, _bmpImageBuffer, sizeof(u16) * BG_BUFFER_PIXELCOUNT);
	}

	// Throw the entire top background into the sub buffer.
	_backgroundTextures[0].copy(_bgSubBuffer, false);
	if (boxArtColorDeband) {
		tonccpy((u8*)_bgSubBuffer2, (u8*)_bgSubBuffer, 0x18000);
	}

 	if (ms().colorMode == 1) {
		for (u16 i = 0; i < BG_BUFFER_PIXELCOUNT; i++) {
			_bgSubBuffer[i] = convertVramColorToGrayscale(_bgSubBuffer[i]);
			if (boxArtColorDeband)
				_bgSubBuffer2[i] = _bgSubBuffer[i];
		}
	}

	// Copy top 32 lines from the buffer into the sub.
	tonccpy(_bgSubBuffer, _bmpImageBuffer, sizeof(u16) * TOPLINES);
	if (boxArtColorDeband) {
		tonccpy(_bgSubBuffer2, _bmpImageBuffer2, sizeof(u16) * TOPLINES);
	}
	
	// Copy bottom tc().shoulderLRenderY() + 5 lines into the sub
	// ((192 - 32) * 256)
	tonccpy(_bgSubBuffer + BOTTOMOFFSET, _bmpImageBuffer + BOTTOMOFFSET, sizeof(u16) * BOTTOMLINES);
	if (boxArtColorDeband) {
		tonccpy(_bgSubBuffer2 + BOTTOMOFFSET, _bmpImageBuffer2 + BOTTOMOFFSET, sizeof(u16) * BOTTOMLINES);
	}

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
				if (boxArtColorDeband) {
					_bgSubBuffer2[y * 256 + x] = val;
				}
			}
		}
	}

	// Draw L Shoulder
	for (uint y = tc().shoulderLRenderY(); y < tc().shoulderLRenderY() + leftTex->texHeight(); y++) {
		for (uint x = tc().shoulderLRenderX(); x < tc().shoulderLRenderX() + leftTex->texWidth(); x++) {
			u16 val = *(leftSrc++);
			if (val >> 15) { // Do not render transparent pixel
				_bgSubBuffer[y * 256 + x] = val;
				if (boxArtColorDeband) {
					_bgSubBuffer2[y * 256 + x] = val;
				}
			}
		}
	}

	commitBgSubModify();
}

ITCM_CODE void ThemeTextures::drawDateTime(const char *str, int posX, int posY, bool isDate) {
	if (!topBorderBufferLoaded) {
		_backgroundTextures[0].copy(_topBorderBuffer, false);
		if (ms().colorMode == 1) {
			for (u16 i = 0; i < BG_BUFFER_PIXELCOUNT; i++) {
				_topBorderBuffer[i] =
					convertVramColorToGrayscale(_topBorderBuffer[i]);
			}
		}
		topBorderBufferLoaded = true;
	}

	toncset16(FontGraphic::textBuf[1], 0, 256 * dateTimeFont()->height());
	dateTimeFont()->print(0, 0, true, str, Alignment::left, FontPalette::dateTime);
	int width = max(dateTimeFont()->calcWidth(str), isDate ? _previousDateWidth : _previousTimeWidth);

	// Copy to background
	for (int y = 0; y < dateTimeFont()->height() && posY + y < SCREEN_HEIGHT; y++) {
		if (posY + y < 0) continue;
		for (int x = 0; x < width && posX + x < SCREEN_WIDTH; x++) {
			if (posX + x < 0) continue;
			int px = FontGraphic::textBuf[1][y * 256 + x];
			u16 bg = _topBorderBuffer[(posY + y) * 256 + (posX + x)];
			u16 val = px ? alphablend(BG_PALETTE[px], bg, (px % 4) < 2 ? 128 : 224) : bg;

			BG_GFX_SUB[(posY + y) * 256 + (posX + x)] = val;
			if (boxArtColorDeband) {
				_frameBufferBot[0][(posY + y) * 256 + (posX + x)] = val;
				_frameBufferBot[1][(posY + y) * 256 + (posX + x)] = val;
			}
		}
	}

	if (isDate) {
		_previousDateWidth = dateTimeFont()->calcWidth(str);
	} else {
		_previousTimeWidth = dateTimeFont()->calcWidth(str);
	}
}

ITCM_CODE void ThemeTextures::drawDateTimeMacro(const char *str, int posX, int posY, bool isDate) {
	if (ms().theme == TWLSettings::EThemeSaturn) return;

	if (!topBorderBufferLoaded) {
		_backgroundTextures[1].copy(_topBorderBuffer, false);
		if (ms().colorMode == 1) {
			for (u16 i = 0; i < BG_BUFFER_PIXELCOUNT; i++) {
				_topBorderBuffer[i] =
					convertVramColorToGrayscale(_topBorderBuffer[i]);
			}
		}
		topBorderBufferLoaded = true;
	}

	toncset16(FontGraphic::textBuf[1], 0, 256 * dateTimeFont()->height());
	dateTimeFont()->print(0, 0, true, str, Alignment::left, FontPalette::dateTime);
	int width = max(dateTimeFont()->calcWidth(str), isDate ? _previousDateWidth : _previousTimeWidth);

	// Copy to background
	for (int y = 0; y < dateTimeFont()->height() && posY + y < SCREEN_HEIGHT; y++) {
		if (posY + y < 0) continue;
		for (int x = 0; x < width && posX + x < SCREEN_WIDTH; x++) {
			if (posX + x < 0) continue;
			int px = FontGraphic::textBuf[1][y * 256 + x];
			u16 bg = _topBorderBuffer[(posY + y) * 256 + (posX + x)];
			u16 val = px ? alphablend(BG_PALETTE[px], bg, (px % 4) < 2 ? 128 : 224) : bg;

			BG_GFX[(posY + y) * 256 + (posX + x)] = val;
		}
	}

	if (isDate) {
		_previousDateWidth = dateTimeFont()->calcWidth(str);
	} else {
		_previousTimeWidth = dateTimeFont()->calcWidth(str);
	}
}

void ThemeTextures::applyUserPaletteToAllGrfTextures() {
	if (_bipsTexture && tc().bipsUserPalette())
		_bipsTexture->applyUserPaletteFile(TFN_PALETTE_BIPS, effectDSiArrowButtonPalettes);
	if (_boxTexture && tc().boxUserPalette())
		_boxTexture->applyUserPaletteFile(TFN_PALETTE_BOX, effectDSiArrowButtonPalettes);
	if (_braceTexture && tc().braceUserPalette())
		_braceTexture->applyUserPaletteFile(TFN_PALETTE_BRACE, effectDSiArrowButtonPalettes);
	if (_bubbleTexture && tc().bubbleUserPalette())
		_bubbleTexture->applyUserPaletteFile(TFN_PALETTE_BUBBLE, effectDSiArrowButtonPalettes);
	if (_buttonArrowTexture && tc().buttonArrowUserPalette())
		_buttonArrowTexture->applyUserPaletteFile(TFN_PALETTE_BUTTON_ARROW, effectDSiArrowButtonPalettes);
	if (_cornerButtonTexture && tc().cornerButtonUserPalette())
		_cornerButtonTexture->applyUserPaletteFile(TFN_PALETTE_CORNERBUTTON, effectDSiArrowButtonPalettes);
	if (_dialogBoxTexture && tc().dialogBoxUserPalette())
		_dialogBoxTexture->applyUserPaletteFile(TFN_PALETTE_DIALOGBOX, effectDSiArrowButtonPalettes);
	if (_folderTexture && tc().folderUserPalette())
		_folderTexture->applyUserPaletteFile(TFN_PALETTE_FOLDER, effectDSiArrowButtonPalettes);
	if (_launchDotTexture && tc().launchDotsUserPalette())
		_launchDotTexture->applyUserPaletteFile(TFN_PALETTE_LAUNCH_DOT, effectDSiArrowButtonPalettes);
	if (_movingArrowTexture && tc().movingArrowUserPalette())
		_movingArrowTexture->applyUserPaletteFile(TFN_PALETTE_MOVING_ARROW, effectDSiArrowButtonPalettes);
	if (_progressTexture && tc().progressUserPalette())
		_progressTexture->applyUserPaletteFile(TFN_PALETTE_PROGRESS, effectDSiArrowButtonPalettes);
	if (_scrollWindowTexture && tc().scrollWindowUserPalette())
		_scrollWindowTexture->applyUserPaletteFile(TFN_PALETTE_SCROLL_WINDOW, effectDSiArrowButtonPalettes);
	if (_smallCartTexture && tc().smallCartUserPalette())
		_smallCartTexture->applyUserPaletteFile(TFN_PALETTE_SMALL_CART, effectDSiArrowButtonPalettes);
	if (_startBorderTexture && (tc().startBorderUserPalette() || tc().cursorUserPalette())) // same texture variable, different images in dsi/3ds themes
		_startBorderTexture->applyUserPaletteFile(TFN_PALETTE_START_BORDER, effectDSiStartBorderPalettes);
	if (_startTextTexture && tc().startTextUserPalette())
		_startTextTexture->applyUserPaletteFile(TFN_PALETTE_START_TEXT, effectDSiStartTextPalettes);
	if (_wirelessIconsTexture && tc().wirelessIconsUserPalette())
		_wirelessIconsTexture->applyUserPaletteFile(TFN_PALETTE_WIRELESSICONS, effectDSiArrowButtonPalettes);
	
	if (_boxEmptyTexture && tc().boxUserPalette())
		_boxEmptyTexture->applyUserPaletteFile(TFN_PALETTE_BOX_EMPTY, effectDSiArrowButtonPalettes);
	if (_boxFullTexture && tc().boxUserPalette())
		_boxFullTexture->applyUserPaletteFile(TFN_PALETTE_BOX_EMPTY, effectDSiArrowButtonPalettes);

	if (_iconA26Texture && tc().iconA26UserPalette())
		_iconA26Texture->applyUserPaletteFile(TFN_PALETTE_ICON_A26, effectDSiArrowButtonPalettes);
	if (_iconCOLTexture && tc().iconCOLUserPalette())
		_iconCOLTexture->applyUserPaletteFile(TFN_PALETTE_ICON_COL, effectDSiArrowButtonPalettes);
	if (_iconGBTexture && tc().iconGBUserPalette())
		_iconGBTexture->applyUserPaletteFile(TFN_PALETTE_ICON_GB, effectDSiArrowButtonPalettes);
	if (_iconGBATexture && tc().iconGBAUserPalette())
		_iconGBATexture->applyUserPaletteFile(TFN_PALETTE_ICON_GBA, effectDSiArrowButtonPalettes);
	if (_iconGBAModeTexture && tc().iconGBAModeUserPalette())
		_iconGBAModeTexture->applyUserPaletteFile(TFN_PALETTE_ICON_GBAMODE, effectDSiArrowButtonPalettes);
	if (_iconGGTexture && tc().iconGGUserPalette())
		_iconGGTexture->applyUserPaletteFile(TFN_PALETTE_ICON_GG, effectDSiArrowButtonPalettes);
	if (_iconIMGTexture && tc().iconIMGUserPalette())
		_iconIMGTexture->applyUserPaletteFile(TFN_PALETTE_ICON_IMG, effectDSiArrowButtonPalettes);
	if (_iconINTTexture && tc().iconINTUserPalette())
		_iconINTTexture->applyUserPaletteFile(TFN_PALETTE_ICON_INT, effectDSiArrowButtonPalettes);
	if (_iconM5Texture && tc().iconM5UserPalette())
		_iconM5Texture->applyUserPaletteFile(TFN_PALETTE_ICON_M5, effectDSiArrowButtonPalettes);
	if (_manualIconTexture && tc().iconManualUserPalette())
		_manualIconTexture->applyUserPaletteFile(TFN_PALETTE_ICON_MANUAL, effectDSiArrowButtonPalettes);
	if (_iconMDTexture && tc().iconMDUserPalette())
		_iconMDTexture->applyUserPaletteFile(TFN_PALETTE_ICON_MD, effectDSiArrowButtonPalettes);
	if (_iconNESTexture && tc().iconNESUserPalette())
		_iconNESTexture->applyUserPaletteFile(TFN_PALETTE_ICON_NES, effectDSiArrowButtonPalettes);
	if (_iconNGPTexture && tc().iconNGPUserPalette())
		_iconNGPTexture->applyUserPaletteFile(TFN_PALETTE_ICON_NGP, effectDSiArrowButtonPalettes);
	if (_iconPCETexture && tc().iconPCEUserPalette())
		_iconPCETexture->applyUserPaletteFile(TFN_PALETTE_ICON_PCE, effectDSiArrowButtonPalettes);
	if (_iconPLGTexture && tc().iconPLGUserPalette())
		_iconPLGTexture->applyUserPaletteFile(TFN_PALETTE_ICON_PLG, effectDSiArrowButtonPalettes);
	if (_settingsIconTexture && tc().iconSettingsUserPalette())
		_settingsIconTexture->applyUserPaletteFile(TFN_PALETTE_ICON_SETTINGS, effectDSiArrowButtonPalettes);
	if (_iconSGTexture && tc().iconSGUserPalette())
		_iconSGTexture->applyUserPaletteFile(TFN_PALETTE_ICON_SG, effectDSiArrowButtonPalettes);
	if (_iconSMSTexture && tc().iconSMSUserPalette())
		_iconSMSTexture->applyUserPaletteFile(TFN_PALETTE_ICON_SMS, effectDSiArrowButtonPalettes);
	if (_iconSNESTexture && tc().iconSNESUserPalette())
		_iconSNESTexture->applyUserPaletteFile(TFN_PALETTE_ICON_SNES, effectDSiArrowButtonPalettes);
	if (_iconUnknownTexture && tc().iconUnknownUserPalette())
		_iconUnknownTexture->applyUserPaletteFile(TFN_PALETTE_ICON_UNK, effectDSiArrowButtonPalettes);
	if (_iconWSTexture && tc().iconWSUserPalette())
		_iconWSTexture->applyUserPaletteFile(TFN_PALETTE_ICON_WS, effectDSiArrowButtonPalettes);
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

	if (_iconA26Texture) {
		_iconA26Texture->applyPaletteEffect(effectGrayscalePalette);
	}
	if (_iconCOLTexture) {
		_iconCOLTexture->applyPaletteEffect(effectGrayscalePalette);
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
	if (_iconIMGTexture) {
		_iconIMGTexture->applyPaletteEffect(effectGrayscalePalette);
	}
	if (_iconINTTexture) {
		_iconINTTexture->applyPaletteEffect(effectGrayscalePalette);
	}
	if (_iconM5Texture) {
		_iconM5Texture->applyPaletteEffect(effectGrayscalePalette);
	}
	if (_iconMDTexture) {
		_iconMDTexture->applyPaletteEffect(effectGrayscalePalette);
	}
	if (_iconNESTexture) {
		_iconNESTexture->applyPaletteEffect(effectGrayscalePalette);
	}
	if (_iconNGPTexture) {
		_iconNGPTexture->applyPaletteEffect(effectGrayscalePalette);
	}
	if (_iconPCETexture) {
		_iconPCETexture->applyPaletteEffect(effectGrayscalePalette);
	}
	if (_iconPLGTexture) {
		_iconPLGTexture->applyPaletteEffect(effectGrayscalePalette);
	}
	if (_iconSGTexture) {
		_iconSGTexture->applyPaletteEffect(effectGrayscalePalette);
	}
	if (_iconSMSTexture) {
		_iconSMSTexture->applyPaletteEffect(effectGrayscalePalette);
	}
	if (_iconSNESTexture) {
		_iconSNESTexture->applyPaletteEffect(effectGrayscalePalette);
	}
	if (_iconUnknownTexture) {
		_iconUnknownTexture->applyPaletteEffect(effectGrayscalePalette);
	}
	if (_iconWSTexture) {
		_iconWSTexture->applyPaletteEffect(effectGrayscalePalette);
	}
}

u16 *ThemeTextures::bmpImageBuffer() { return _bmpImageBuffer; }
u16 *ThemeTextures::bgSubBuffer2() { return _bgSubBuffer2; }
u16 *ThemeTextures::photoBuffer() { return _photoBuffer; }
u16 *ThemeTextures::photoBuffer2() { return _photoBuffer2; }
//u16 *ThemeTextures::frameBuffer(bool secondBuffer) { return _frameBuffer[secondBuffer]; }
u16 *ThemeTextures::frameBufferBot(bool secondBuffer) { return _frameBufferBot[secondBuffer]; }

void loadRotatingCubes() {
	bool rvidCompressed = false;
	std::string cubes = TFN_RVID_CUBES;
	FILE *videoFrameFile = fopen(cubes.c_str(), "rb");

	/*if (!videoFrameFile && isDSiMode()) {
		// Fallback to uncompressed RVID
		rvidCompressed = false;
		cubes = TFN_RVID_CUBES;
		if (ms().colorMode == 1) {
			cubes = TFN_RVID_CUBES_BW;
		}
		videoFrameFile = fopen(cubes.c_str(), "rb");
	}*/

	// if (!videoFrameFile) {
	// 	videoFrameFile = fopen(std::string(TFN_FALLBACK_RVID_CUBES).c_str(), "rb");
	// }
	// FILE* videoFrameFile;

	/*for (u8 selectedFrame = 0; selectedFrame <= rocketVideo_videoFrames; selectedFrame++) {
		if (selectedFrame < 0x10) {
			snprintf(videoFrameFilename, sizeof(videoFrameFilename),
	"nitro:/video/3dsRotatingCubes/0x0%x.bmp", (int)selectedFrame); } else { snprintf(videoFrameFilename,
	sizeof(videoFrameFilename), "nitro:/video/3dsRotatingCubes/0x%x.bmp", (int)selectedFrame);
		}
		videoFrameFile = fopen(videoFrameFilename, "rb");

		if (videoFrameFile) {
			// Start loading
			fseek(videoFrameFile, 0xe, SEEK_SET);
			u8 pixelStart = (u8)fgetc(videoFrameFile) + 0xe;
			fseek(videoFrameFile, pixelStart, SEEK_SET);
			fread(bmpImageBuffer, 2, 0x7000, videoFrameFile);
			u16* src = bmpImageBuffer;
			int x = 0;
			int y = 55;
			for (int i=0; i<256*56; i++) {
				if (x >= 256) {
					x = 0;
					y--;
				}
				u16 val = *(src++);
				renderedImageBuffer[y*256+x] = Texture::bmpToDS(val);
				x++;
			}
		}
		fclose(videoFrameFile);
		memcpy(rotatingCubesLocation+(selectedFrame*0x7000), renderedImageBuffer, 0x7000);
	}*/

	if (videoFrameFile) {
		bool doRead = false;
		if (!rvidCompressed) {
			fseek(videoFrameFile, 0x200, SEEK_SET);
		}

		if (dsiFeatures()) {
			doRead = true;
		} else if (sys().isRegularDS() && (io_dldi_data->ioInterface.features & FEATURE_SLOT_NDS)) {
			sysSetCartOwner(BUS_OWNER_ARM9); // Allow arm9 to access GBA ROM (or in this case, the DS Memory
							 // Expansion Pak)
			if (*(u16*)(0x020000C0) == 0) {
				*(vu16*)(0x08240000) = 1;
			}
			if ((*(u16*)(0x020000C0) != 0 && *(u16*)(0x020000C0) != 0x5A45) || *(vu16*)(0x08240000) == 1) {
				// Set to load video into DS Memory Expansion Pak
				rotatingCubesLocation = (u8*)0x09000000;
				doRead = true;
			}
		}

		if (doRead) {
			if (rvidCompressed) {
				fread((void*)0x02D80000, 1, 0x200000, videoFrameFile);
				LZ77_Decompress((u8*)0x02D80000, (u8*)rotatingCubesLocation);
			} else {
				fread(rotatingCubesLocation, 1, 0x700000, videoFrameFile);
			}
			if (ms().colorMode == 1) {
				u16* rotatingCubesLocation16 = (u16*)rotatingCubesLocation;
				for (int i = 0; i < 0x700000/2; i++) {
					if (rotatingCubesLocation16[i] != 0) {
						rotatingCubesLocation16[i] = convertVramColorToGrayscale(rotatingCubesLocation16[i]);
					}
				}
			}
			rotatingCubesLoaded = true;
			rocketVideo_playVideo = true;
		}
	}
}
void ThemeTextures::videoSetup() {
	logPrint("tex().videoSetup()\n");
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

	/*if (widescreenEffects) {
		// Add black bars to left and right sides
		s16 c = cosLerp(0) >> 4;
		REG_BG3PA_SUB = ( c * 315)>>8;
		REG_BG3X_SUB = -29 << 8;
	}*/

	REG_BLDCNT = BLEND_SRC_BG3 | BLEND_FADE_BLACK;

	if (dsiFeatures()) {
		_bmpImageBuffer2 = new u16[256 * 192];
		_bgSubBuffer2 = new u16[256 * 192];
		_photoBuffer2 = new u16[208 * 156];
		_frameBufferBot[0] = new u16[256 * 192];
		_frameBufferBot[1] = new u16[256 * 192];
	}

	if (dsiFeatures() && !ms().macroMode && ms().theme != TWLSettings::EThemeHBL) {
		if (ms().consoleModel > 0) {
			rotatingCubesLocation = (u8*)0x0D700000;
			boxArtCache = (u8*)0x0D540000;
		} else {
			if (ms().theme == TWLSettings::ETheme3DS) {
				rotatingCubesLocation = new u8[0x700000];
			}
			if (ms().showBoxArt == 2) {
				boxArtCache = new u8[0x1B8000];
			}
		}
	}

	if (ms().theme == TWLSettings::ETheme3DS && !ms().macroMode) {
		loadRotatingCubes();
	}

	boxArtColorDeband = (ms().boxArtColorDeband && !ms().macroMode && ndmaEnabled() && !rotatingCubesLoaded && ms().theme != TWLSettings::EThemeHBL);
}