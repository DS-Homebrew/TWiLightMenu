
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

// #include "common/ColorLut.h"
#include "color.h"
#include "errorScreen.h"
#include "fileBrowse.h"
#include "fileCopy.h"
#include "common/lzss.h"
#include "common/tonccpy.h"
#include "common/lodepng.h"
#include "language.h"
#include "ndsheaderbanner.h"
#include "ndma.h"


extern bool useTwlCfg;

//extern bool widescreenEffects;

extern u16* colorTable;
extern bool invertedColors;
extern bool noWhiteFade;
extern u32 rotatingCubesLoaded;
extern bool rocketVideo_playVideo;
extern u8 *rotatingCubesLocation;

// #include <nds/arm9/decompress.h>
extern bool showColon;

static u16 _bgMainBuffer[256 * 192] = {0};
static u16 _bgSubBuffer[256 * 192] = {0};
static u16* _photoBuffer = NULL;
static u16 _topBorderBuffer[256 * 192] = {0};
static u16* _bgSubBuffer2 = (u16*)_bgSubBuffer;
static u16* _photoBuffer2 = (u16*)_photoBuffer;
// DSi mode double-frame buffers
//static u16* _frameBuffer[2] = {(u16*)0x02F80000, (u16*)0x02F98000};
static u16* _frameBufferBot[2] = {NULL};

static bool topBorderBufferLoaded = false;
bool boxArtColorDeband = false;

static u8* boxArtCache = NULL;	// Size: 0x1B8000
static bool boxArtFound[40] = {false};
uint boxArtWidth = 0, boxArtHeight = 0;

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

	if (ms().showPhoto && tc().renderPhoto()) {
		_backgroundTextures.emplace_back(TFN_BG_TOPPHOTOBG, TFN_BG_TOPBG, ms().theme == TWLSettings::EThemeDSi ? TFN_FALLBACK_BG_TOPPHOTOBG : TFN_FALLBACK_BG_TOPBG);
	} else {
		_backgroundTextures.emplace_back(TFN_BG_TOPBG, TFN_FALLBACK_BG_TOPBG);
	}
		
	
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
		_backgroundTextures.emplace_back(TFN_BG_BOTTOMBG_MACRO, TFN_BG_BOTTOMBG, TFN_FALLBACK_BG_BOTTOMBG);
		_backgroundTextures.emplace_back(TFN_BG_BOTTOMBUBBLEBG_MACRO, TFN_BG_BOTTOMBUBBLEBG, TFN_FALLBACK_BG_BOTTOMBUBBLEBG_MACRO);
		if (ms().theme == TWLSettings::EThemeDSi) _backgroundTextures.emplace_back(TFN_BG_BOTTOMMOVINGBG_MACRO, TFN_BG_BOTTOMMOVINGBG, TFN_FALLBACK_BG_BOTTOMMOVINGBG);
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

	_bubbleTexture = std::make_unique<Texture>(TFN_GRF_BUBBLE, TFN_FALLBACK_GRF_BUBBLE);
	_settingsIconTexture = std::make_unique<Texture>(TFN_GRF_ICON_SETTINGS, TFN_FALLBACK_GRF_ICON_SETTINGS);

	_boxFullTexture = std::make_unique<Texture>(TFN_GRF_BOX_FULL, TFN_FALLBACK_GRF_BOX_FULL);
	_boxEmptyTexture = std::make_unique<Texture>(TFN_GRF_BOX_EMPTY, TFN_FALLBACK_GRF_BOX_EMPTY);
	_folderTexture = std::make_unique<Texture>(TFN_GRF_FOLDER, TFN_FALLBACK_GRF_FOLDER);
	_progressTexture = std::make_unique<Texture>(TFN_GRF_PROGRESS, TFN_FALLBACK_GRF_PROGRESS);

	_smallCartTexture = std::make_unique<Texture>(TFN_GRF_SMALL_CART, TFN_FALLBACK_GRF_SMALL_CART);
	_wirelessIconsTexture = std::make_unique<Texture>(TFN_GRF_WIRELESSICONS, TFN_FALLBACK_GRF_WIRELESSICONS);
	_startBorderTexture = std::make_unique<Texture>(TFN_GRF_CURSOR, TFN_FALLBACK_GRF_CURSOR);
	_dialogBoxTexture = std::make_unique<Texture>(TFN_GRF_DIALOGBOX, TFN_FALLBACK_GRF_DIALOGBOX);

	applyUserPaletteToAllGrfTextures();

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

	//iprintf("loadBipsImage(*_bipsTexture)\n");
	loadBipsImage(*_bipsTexture);

	//iprintf("loadBubbleImage(*_bubbleTexture, tc().bubbleTipSpriteW(), tc().bubbleTipSpriteH())\n");
	loadBubbleImage(*_bubbleTexture, tc().bubbleTipSpriteW(), tc().bubbleTipSpriteH());
	//iprintf("loadScrollwindowImage(*_scrollWindowTexture)\n");
	loadScrollwindowImage(*_scrollWindowTexture);
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
	if (dsiFeatures() && !sys().i2cBricked()) {
		_volume0Texture = std::make_unique<Texture>(TFN_VOLUME0, TFN_FALLBACK_VOLUME0);
		_volume1Texture = std::make_unique<Texture>(TFN_VOLUME1, TFN_FALLBACK_VOLUME1);
		_volume2Texture = std::make_unique<Texture>(TFN_VOLUME2, TFN_FALLBACK_VOLUME2);
		_volume3Texture = std::make_unique<Texture>(TFN_VOLUME3, TFN_FALLBACK_VOLUME3);
		_volume4Texture = std::make_unique<Texture>(TFN_VOLUME4, TFN_FALLBACK_VOLUME4);
	}
}

void ThemeTextures::loadBatteryTextures() {
	if (dsiFeatures() && !sys().i2cBricked()) {
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
		if (sys().hasRegulableBacklight()) {
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
		if (ms().showPhoto && tc().renderPhoto()) {
			_leftShoulderTexture = std::make_unique<Texture>(TFN_UI_LSHOULDER_PHOTO, TFN_UI_LSHOULDER, TFN_FALLBACK_UI_LSHOULDER);
			_rightShoulderTexture = std::make_unique<Texture>(TFN_UI_RSHOULDER_PHOTO, TFN_UI_RSHOULDER, TFN_FALLBACK_UI_RSHOULDER);
			_leftShoulderGreyedTexture = std::make_unique<Texture>(TFN_UI_LSHOULDER_PHOTO_GREYED, TFN_UI_LSHOULDER_GREYED, TFN_FALLBACK_UI_LSHOULDER_GREYED);
			_rightShoulderGreyedTexture = std::make_unique<Texture>(TFN_UI_RSHOULDER_PHOTO_GREYED, TFN_UI_RSHOULDER_GREYED, TFN_FALLBACK_UI_RSHOULDER_GREYED);
		} else {
			_leftShoulderTexture = std::make_unique<Texture>(TFN_UI_LSHOULDER, TFN_FALLBACK_UI_LSHOULDER);
			_rightShoulderTexture = std::make_unique<Texture>(TFN_UI_RSHOULDER, TFN_FALLBACK_UI_RSHOULDER);
			_leftShoulderGreyedTexture = std::make_unique<Texture>(TFN_UI_LSHOULDER_GREYED, TFN_FALLBACK_UI_LSHOULDER_GREYED);
			_rightShoulderGreyedTexture = std::make_unique<Texture>(TFN_UI_RSHOULDER_GREYED, TFN_FALLBACK_UI_RSHOULDER_GREYED);
		}
	}
}

void ThemeTextures::loadIconGBTexture() {
	static bool loaded = false;
	if (loaded) return;
	loaded = true;

	_iconGBTexture = std::make_unique<Texture>(TFN_GRF_ICON_GB, TFN_FALLBACK_GRF_ICON_GB);
	if (_iconGBTexture && tc().iconGBUserPalette()) {
		_iconGBTexture->applyUserPaletteFile(TFN_PALETTE_ICON_GB, effectDSiArrowButtonPalettes);
	}
	logPrint("Loaded iconGBTexture\n");
}
void ThemeTextures::loadIconGBATexture() {
	static bool loaded = false;
	if (loaded) return;
	loaded = true;

	_iconGBATexture = std::make_unique<Texture>(TFN_GRF_ICON_GBA, TFN_FALLBACK_GRF_ICON_GBA);
	if (_iconGBATexture && tc().iconGBAUserPalette()) {
		_iconGBATexture->applyUserPaletteFile(TFN_PALETTE_ICON_GBA, effectDSiArrowButtonPalettes);
	}
	logPrint("Loaded iconGBATexture\n");
	/* _iconGBAModeTexture = std::make_unique<Texture>(TFN_GRF_ICON_GBAMODE, TFN_FALLBACK_GRF_ICON_GBAMODE);
	if (_iconGBAModeTexture && tc().iconGBAModeUserPalette()) {
		_iconGBAModeTexture->applyUserPaletteFile(TFN_PALETTE_ICON_GBAMODE, effectDSiArrowButtonPalettes);
	} */
}
void ThemeTextures::loadIconGGTexture() {
	static bool loaded = false;
	if (loaded) return;
	loaded = true;

	_iconGGTexture = std::make_unique<Texture>(TFN_GRF_ICON_GG, TFN_FALLBACK_GRF_ICON_GG);
	if (_iconGGTexture && tc().iconGGUserPalette()) {
		_iconGGTexture->applyUserPaletteFile(TFN_PALETTE_ICON_GG, effectDSiArrowButtonPalettes);
	}
	logPrint("Loaded iconGGTexture\n");
}
void ThemeTextures::loadIconMDTexture() {
	static bool loaded = false;
	if (loaded) return;
	loaded = true;

	_iconMDTexture = std::make_unique<Texture>(TFN_GRF_ICON_MD, TFN_FALLBACK_GRF_ICON_MD);
	if (_iconMDTexture && tc().iconMDUserPalette()) {
		_iconMDTexture->applyUserPaletteFile(TFN_PALETTE_ICON_MD, effectDSiArrowButtonPalettes);
	}
	logPrint("Loaded iconMDTexture\n");
}
void ThemeTextures::loadIconNESTexture() {
	static bool loaded = false;
	if (loaded) return;
	loaded = true;

	_iconNESTexture = std::make_unique<Texture>(TFN_GRF_ICON_NES, TFN_FALLBACK_GRF_ICON_NES);
	if (_iconNESTexture && tc().iconNESUserPalette()) {
		_iconNESTexture->applyUserPaletteFile(TFN_PALETTE_ICON_NES, effectDSiArrowButtonPalettes);
	}
	logPrint("Loaded iconNESTexture\n");
}
void ThemeTextures::loadIconSGTexture() {
	static bool loaded = false;
	if (loaded) return;
	loaded = true;

	_iconSGTexture = std::make_unique<Texture>(TFN_GRF_ICON_SG, TFN_FALLBACK_GRF_ICON_SG);
	if (_iconSGTexture && tc().iconSGUserPalette()) {
		_iconSGTexture->applyUserPaletteFile(TFN_PALETTE_ICON_SG, effectDSiArrowButtonPalettes);
	}
	logPrint("Loaded iconSGTexture\n");
}
void ThemeTextures::loadIconSMSTexture() {
	static bool loaded = false;
	if (loaded) return;
	loaded = true;

	_iconSMSTexture = std::make_unique<Texture>(TFN_GRF_ICON_SMS, TFN_FALLBACK_GRF_ICON_SMS);
	if (_iconSMSTexture && tc().iconSMSUserPalette()) {
		_iconSMSTexture->applyUserPaletteFile(TFN_PALETTE_ICON_SMS, effectDSiArrowButtonPalettes);
	}
	logPrint("Loaded iconSMSTexture\n");
}
void ThemeTextures::loadIconSNESTexture() {
	static bool loaded = false;
	if (loaded) return;
	loaded = true;

	_iconSNESTexture = std::make_unique<Texture>(TFN_GRF_ICON_SNES, TFN_FALLBACK_GRF_ICON_SNES);
	if (_iconSNESTexture && tc().iconSNESUserPalette()) {
		_iconSNESTexture->applyUserPaletteFile(TFN_PALETTE_ICON_SNES, effectDSiArrowButtonPalettes);
	}
	logPrint("Loaded iconSNESTexture\n");
}
void ThemeTextures::loadIconPLGTexture() {
	static bool loaded = false;
	if (loaded) return;
	loaded = true;

	_iconPLGTexture = std::make_unique<Texture>(TFN_GRF_ICON_PLG, TFN_FALLBACK_GRF_ICON_PLG);
	if (_iconPLGTexture && tc().iconPLGUserPalette()) {
		_iconPLGTexture->applyUserPaletteFile(TFN_PALETTE_ICON_PLG, effectDSiArrowButtonPalettes);
	}
	logPrint("Loaded iconPLGTexture\n");
}
void ThemeTextures::loadIconA26Texture() {
	static bool loaded = false;
	if (loaded) return;
	loaded = true;

	_iconA26Texture = std::make_unique<Texture>(TFN_GRF_ICON_A26, TFN_FALLBACK_GRF_ICON_A26);
	if (_iconA26Texture && tc().iconA26UserPalette()) {
		_iconA26Texture->applyUserPaletteFile(TFN_PALETTE_ICON_A26, effectDSiArrowButtonPalettes);
	}
	logPrint("Loaded iconA26Texture\n");
}
void ThemeTextures::loadIconCOLTexture() {
	static bool loaded = false;
	if (loaded) return;
	loaded = true;

	_iconCOLTexture = std::make_unique<Texture>(TFN_GRF_ICON_COL, TFN_FALLBACK_GRF_ICON_COL);
	if (_iconCOLTexture && tc().iconCOLUserPalette()) {
		_iconCOLTexture->applyUserPaletteFile(TFN_PALETTE_ICON_COL, effectDSiArrowButtonPalettes);
	}
	logPrint("Loaded iconCOLTexture\n");
}
void ThemeTextures::loadIconM5Texture() {
	static bool loaded = false;
	if (loaded) return;
	loaded = true;

	_iconM5Texture = std::make_unique<Texture>(TFN_GRF_ICON_M5, TFN_FALLBACK_GRF_ICON_M5);
	if (_iconM5Texture && tc().iconM5UserPalette()) {
		_iconM5Texture->applyUserPaletteFile(TFN_PALETTE_ICON_M5, effectDSiArrowButtonPalettes);
	}
	logPrint("Loaded iconM5Texture\n");
}
void ThemeTextures::loadIconINTTexture() {
	static bool loaded = false;
	if (loaded) return;
	loaded = true;

	_iconINTTexture = std::make_unique<Texture>(TFN_GRF_ICON_INT, TFN_FALLBACK_GRF_ICON_INT);
	if (_iconINTTexture && tc().iconINTUserPalette()) {
		_iconINTTexture->applyUserPaletteFile(TFN_PALETTE_ICON_INT, effectDSiArrowButtonPalettes);
	}
	logPrint("Loaded iconINTTexture\n");
}
void ThemeTextures::loadIconPCETexture() {
	static bool loaded = false;
	if (loaded) return;
	loaded = true;

	_iconPCETexture = std::make_unique<Texture>(TFN_GRF_ICON_PCE, TFN_FALLBACK_GRF_ICON_PCE);
	if (_iconPCETexture && tc().iconPCEUserPalette()) {
		_iconPCETexture->applyUserPaletteFile(TFN_PALETTE_ICON_PCE, effectDSiArrowButtonPalettes);
	}
	logPrint("Loaded iconPCETexture\n");
}
void ThemeTextures::loadIconWSTexture() {
	static bool loaded = false;
	if (loaded) return;
	loaded = true;

	_iconWSTexture = std::make_unique<Texture>(TFN_GRF_ICON_WS, TFN_FALLBACK_GRF_ICON_WS);
	if (_iconWSTexture && tc().iconWSUserPalette()) {
		_iconWSTexture->applyUserPaletteFile(TFN_PALETTE_ICON_WS, effectDSiArrowButtonPalettes);
	}
	logPrint("Loaded iconWSTexture\n");
}
void ThemeTextures::loadIconNGPTexture() {
	static bool loaded = false;
	if (loaded) return;
	loaded = true;

	_iconNGPTexture = std::make_unique<Texture>(TFN_GRF_ICON_NGP, TFN_FALLBACK_GRF_ICON_NGP);
	if (_iconNGPTexture && tc().iconNGPUserPalette()) {
		_iconNGPTexture->applyUserPaletteFile(TFN_PALETTE_ICON_NGP, effectDSiArrowButtonPalettes);
	}
	logPrint("Loaded iconNGPTexture\n");
}
void ThemeTextures::loadIconCPCTexture() {
	static bool loaded = false;
	if (loaded) return;
	loaded = true;

	_iconCPCTexture = std::make_unique<Texture>(TFN_GRF_ICON_CPC, TFN_FALLBACK_GRF_ICON_CPC);
	if (_iconCPCTexture && tc().iconCPCUserPalette()) {
		_iconCPCTexture->applyUserPaletteFile(TFN_PALETTE_ICON_CPC, effectDSiArrowButtonPalettes);
	}
	logPrint("Loaded iconCPCTexture\n");
}
void ThemeTextures::loadIconVIDTexture() {
	static bool loaded = false;
	if (loaded) return;
	loaded = true;

	_iconVIDTexture = std::make_unique<Texture>(TFN_GRF_ICON_VID, TFN_FALLBACK_GRF_ICON_VID);
	if (_iconVIDTexture && tc().iconVIDUserPalette()) {
		_iconVIDTexture->applyUserPaletteFile(TFN_PALETTE_ICON_VID, effectDSiArrowButtonPalettes);
	}
	logPrint("Loaded iconVIDTexture\n");
}
void ThemeTextures::loadIconIMGTexture() {
	static bool loaded = false;
	if (loaded) return;
	loaded = true;

	_iconIMGTexture = std::make_unique<Texture>(TFN_GRF_ICON_IMG, TFN_FALLBACK_GRF_ICON_IMG);
	if (_iconIMGTexture && tc().iconIMGUserPalette()) {
		_iconIMGTexture->applyUserPaletteFile(TFN_PALETTE_ICON_IMG, effectDSiArrowButtonPalettes);
	}
	logPrint("Loaded iconIMGTexture\n");
}
void ThemeTextures::loadIconMSXTexture() {
	static bool loaded = false;
	if (loaded) return;
	loaded = true;

	_iconMSXTexture = std::make_unique<Texture>(TFN_GRF_ICON_MSX, TFN_FALLBACK_GRF_ICON_MSX);
	if (_iconMSXTexture && tc().iconMSXUserPalette()) {
		_iconMSXTexture->applyUserPaletteFile(TFN_PALETTE_ICON_MSX, effectDSiArrowButtonPalettes);
	}
	logPrint("Loaded iconMSXTexture\n");
}
void ThemeTextures::loadIconMINITexture() {
	static bool loaded = false;
	if (loaded) return;
	loaded = true;

	_iconMINITexture = std::make_unique<Texture>(TFN_GRF_ICON_MINI, TFN_FALLBACK_GRF_ICON_MINI);
	if (_iconMINITexture && tc().iconMINIUserPalette()) {
		_iconMINITexture->applyUserPaletteFile(TFN_PALETTE_ICON_MINI, effectDSiArrowButtonPalettes);
	}
	logPrint("Loaded iconMINITexture\n");
}
void ThemeTextures::loadIconHBTexture() {
	static bool loaded = false;
	if (loaded) return;
	loaded = true;

	_iconHBTexture = std::make_unique<Texture>(TFN_GRF_ICON_HB, TFN_FALLBACK_GRF_ICON_HB);
	if (_iconHBTexture && tc().iconHBUserPalette()) {
		_iconHBTexture->applyUserPaletteFile(TFN_PALETTE_ICON_HB, effectDSiArrowButtonPalettes);
	}
	logPrint("Loaded iconHBTexture\n");
}
void ThemeTextures::loadIconUnknownTexture() {
	static bool loaded = false;
	if (loaded) return;
	loaded = true;

	_iconUnknownTexture = std::make_unique<Texture>(TFN_GRF_ICON_UNK, TFN_FALLBACK_GRF_ICON_UNK);
	if (_iconUnknownTexture && tc().iconUnknownUserPalette()) {
		_iconUnknownTexture->applyUserPaletteFile(TFN_PALETTE_ICON_UNK, effectDSiArrowButtonPalettes);
	}
	logPrint("Loaded iconUnknownTexture\n");
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
	if (boxArtColorDeband && ndmaEnabled()) {
		DC_FlushRange(_bgSubBuffer2, sizeof(u16) * BG_BUFFER_PIXELCOUNT);
	}
	while (REG_VCOUNT != 191); // Fix screen tearing
	dmaCopyWordsAsynch(2, _bgSubBuffer, bgLoc, sizeof(u16) * BG_BUFFER_PIXELCOUNT);
	if (boxArtColorDeband) {
		if (ndmaEnabled()) {
			ndmaCopyWordsAsynch(2, _bgSubBuffer2, _frameBufferBot[1], sizeof(u16) * BG_BUFFER_PIXELCOUNT);
		} else {
			tonccpy(_frameBufferBot[1], _bgSubBuffer2, sizeof(u16) * BG_BUFFER_PIXELCOUNT);
		}
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

	commitBgMainModify();
}

void ThemeTextures::clearTopScreen() {
	beginBgSubModify();
	const u16 val = colorTable ? (colorTable[0x7FFF] | BIT(15)) : 0xFFFF;
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
		topBorderBufferLoaded = true;
	}

	// Load username
	int xPos = ((dsiFeatures() && !sys().i2cBricked()) ? tc().usernameRenderX() : tc().usernameRenderXDS());
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
			u16 val = 0;
			if (tc().usernameEdgeAlpha()) {
				val = px ? themealphablend(BG_PALETTE_SUB[px], bg, (px % 4) < 2 ? 128 : 224) : bg;
			} else {
				val = px ? (BG_PALETTE_SUB[px] | BIT(15)) : bg;
			}

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

void ThemeTextures::drawBoxArt(const char *filename, bool inMem) {
	if (inMem ? !boxArtFound[CURPOS] : access(filename, F_OK) != 0) return;

	beginBgSubModify();

	std::vector<unsigned char> image;
	uint imageXpos, imageYpos;
	if (inMem) {
		lodepng::decode(image, boxArtWidth, boxArtHeight, (unsigned char*)boxArtCache+(CURPOS*0xB000), 0xB000);
	} else {
		lodepng::decode(image, boxArtWidth, boxArtHeight, filename);
	}
	bool alternatePixel = false;
	if (boxArtWidth > 256 || boxArtHeight > 192) return;

	u16* bmpImageBuffer = new u16[256 * 192];
	u16* bmpImageBuffer2 = boxArtColorDeband ? new u16[256 * 192] : NULL;

	imageXpos = (256-boxArtWidth)/2;
	imageYpos = (192-boxArtHeight)/2;

	int photoXstart = imageXpos;
	int photoXend = imageXpos+boxArtWidth;
	int photoX = photoXstart;
	int photoY = imageYpos;

	for (uint i=0;i<image.size()/4;i++) {
		u8 pixelAdjustInfo = 0;
		if (boxArtColorDeband) {
			if (alternatePixel) {
				if (image[(i*4)] >= 0x4 && image[(i*4)] < 0xFC) {
					image[(i*4)] += 0x4;
					pixelAdjustInfo |= BIT(0);
				}
				if (image[(i*4)+1] >= 0x4 && image[(i*4)+1] < 0xFC) {
					image[(i*4)+1] += 0x4;
					pixelAdjustInfo |= BIT(1);
				}
				if (image[(i*4)+2] >= 0x4 && image[(i*4)+2] < 0xFC) {
					image[(i*4)+2] += 0x4;
					pixelAdjustInfo |= BIT(2);
				}
				if (image[(i*4)+3] >= 0x4 && image[(i*4)+3] < 0xFC) {
					image[(i*4)+3] += 0x4;
					pixelAdjustInfo |= BIT(3);
				}
			}
		}
		u16 color = image[i*4]>>3 | (image[(i*4)+1]>>3)<<5 | (image[(i*4)+2]>>3)<<10 | BIT(15);
		if (colorTable) {
			color = colorTable[color % 0x8000] | BIT(15);
		}
		if (image[(i*4)+3] == 255) {
			bmpImageBuffer[i] = color;
		} else {
			bmpImageBuffer[i] = alphablend(color, _bgSubBuffer[(photoY*256)+photoX], image[(i*4)+3]);
		}
		if (boxArtColorDeband) {
			if (alternatePixel) {
				if (pixelAdjustInfo & BIT(0)) {
					image[(i*4)] -= 0x4;
				}
				if (pixelAdjustInfo & BIT(1)) {
					image[(i*4)+1] -= 0x4;
				}
				if (pixelAdjustInfo & BIT(2)) {
					image[(i*4)+2] -= 0x4;
				}
				if (pixelAdjustInfo & BIT(3)) {
					image[(i*4)+3] -= 0x4;
				}
			} else {
				if (image[(i*4)] >= 0x4 && image[(i*4)] < 0xFC) {
					image[(i*4)] += 0x4;
				}
				if (image[(i*4)+1] >= 0x4 && image[(i*4)+1] < 0xFC) {
					image[(i*4)+1] += 0x4;
				}
				if (image[(i*4)+2] >= 0x4 && image[(i*4)+2] < 0xFC) {
					image[(i*4)+2] += 0x4;
				}
				if (image[(i*4)+3] >= 0x4 && image[(i*4)+3] < 0xFC) {
					image[(i*4)+3] += 0x4;
				}
			}
			color = image[i*4]>>3 | (image[(i*4)+1]>>3)<<5 | (image[(i*4)+2]>>3)<<10 | BIT(15);
			if (colorTable) {
				color = colorTable[color % 0x8000] | BIT(15);
			}
			if (image[(i*4)+3] == 255) {
				bmpImageBuffer2[i] = color;
			} else {
				bmpImageBuffer2[i] = alphablend(color, _bgSubBuffer2[(photoY*256)+photoX], image[(i*4)+3]);
			}
			if ((i % boxArtWidth) == boxArtWidth-1) alternatePixel = !alternatePixel;
			alternatePixel = !alternatePixel;
		}
		photoX++;
		if (photoX == photoXend) {
			photoX = photoXstart;
			photoY++;
		}
	}

	u16 *src = bmpImageBuffer;
	u16 *src2 = bmpImageBuffer2;
	for (uint y = 0; y < boxArtHeight; y++) {
		for (uint x = 0; x < boxArtWidth; x++) {
			_bgSubBuffer[(y+imageYpos) * 256 + imageXpos + x] = *(src++);
			if (boxArtColorDeband) {
				_bgSubBuffer2[(y+imageYpos) * 256 + imageXpos + x] = *(src2++);
			}
		}
	}
	commitBgSubModify();

	delete[] bmpImageBuffer;
	if (boxArtColorDeband) {
		delete[] bmpImageBuffer2;
	}
}

#define MAX_PHOTO_WIDTH 208
#define MAX_PHOTO_HEIGHT 156
#define PHOTO_OFFSET 24
// Redraw background and photo over the boxart bounds
void ThemeTextures::drawOverBoxArt(uint photoWidth, uint photoHeight) {
	if (boxArtWidth == 0 || boxArtHeight == 0) return;
	uint boxArtX = (SCREEN_WIDTH - boxArtWidth) / 2;
	uint boxArtY = (SCREEN_HEIGHT - boxArtHeight) / 2;

	beginBgSubModify();
	if (!ms().showPhoto || !tc().renderPhoto() || boxArtWidth > MAX_PHOTO_WIDTH || boxArtHeight > MAX_PHOTO_HEIGHT) {
		if (!topBorderBufferLoaded) {
			_backgroundTextures[0].copy(_topBorderBuffer, false);
			topBorderBufferLoaded = true;
		}
		for (uint y = 0; y < boxArtHeight; y++) {
			uint offset = boxArtX + (boxArtY + y) * SCREEN_WIDTH;
			tonccpy(_bgSubBuffer + offset, _topBorderBuffer + offset, sizeof(u16) * boxArtWidth);
			if (boxArtColorDeband) {
				tonccpy(_bgSubBuffer2 + offset, _topBorderBuffer + offset, sizeof(u16) * boxArtWidth);
			}
		}
	}
	
	if (ms().showPhoto && tc().renderPhoto()) {
		// fill black within boxart and photo bounds
		uint blackX = boxArtX > PHOTO_OFFSET ? boxArtX : PHOTO_OFFSET;
		uint blackY = boxArtY > PHOTO_OFFSET ? boxArtY : PHOTO_OFFSET;
		uint blackWidth = boxArtWidth < MAX_PHOTO_WIDTH ? boxArtWidth : MAX_PHOTO_WIDTH;
		uint blackHeight = boxArtHeight < MAX_PHOTO_HEIGHT ? boxArtHeight : MAX_PHOTO_HEIGHT;
		for (uint y = 0; y < blackHeight; y++) {
			uint offset = blackX + (blackY + y) * SCREEN_WIDTH;
			dmaFillHalfWords(0x8000, _bgSubBuffer + offset, sizeof(u16) * blackWidth);
			if (boxArtColorDeband) {
				dmaFillHalfWords(0x8000, _bgSubBuffer2 + offset, sizeof(u16) * blackWidth);
			}
		}
		// draw photo within boxart bounds
		uint photoX = PHOTO_OFFSET + (MAX_PHOTO_WIDTH - photoWidth) / 2;
		uint photoY = PHOTO_OFFSET + (MAX_PHOTO_HEIGHT - photoHeight) / 2;
		uint xOffset = boxArtX > photoX ? boxArtX - photoX : 0;
		uint yOffset = boxArtY > photoY ? boxArtY - photoY : 0;
		uint copyWidth = boxArtWidth < photoWidth ? boxArtWidth : photoWidth;
		uint copyHeight = boxArtHeight < photoHeight ? boxArtHeight : photoHeight;
		for (uint y = 0; y < copyHeight; y++) {
			uint offset = photoX + xOffset + (photoY + yOffset + y) * SCREEN_WIDTH;
			tonccpy(_bgSubBuffer + offset, _photoBuffer + xOffset + (yOffset + y) * photoWidth, sizeof(u16) * copyWidth);
			if (boxArtColorDeband) {
				tonccpy(_bgSubBuffer2 + offset, _photoBuffer2 + xOffset + (yOffset + y) * photoWidth, sizeof(u16) * copyWidth);
			}
		}
	}
	commitBgSubModify();
	boxArtWidth = boxArtHeight = 0;
}

// Redraw background over the rotating cubes bounds
void ThemeTextures::drawOverRotatingCubes() {
	// if (!rotatingCubesLoaded) return;

	extern u8 rocketVideo_height;
	extern int rocketVideo_videoYpos;

	beginBgSubModify();
	for (uint y = 0; y < rocketVideo_height; y++) {
		uint offset = (rocketVideo_videoYpos + y) * SCREEN_WIDTH;
		tonccpy(_bgSubBuffer + offset, _topBorderBuffer + offset, sizeof(u16) * SCREEN_WIDTH);
	}
	commitBgSubModify();
}

ITCM_CODE void ThemeTextures::drawVolumeImage(int volumeLevel) {
	if (!dsiFeatures() || sys().i2cBricked())
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
	if (!dsiFeatures() || sys().i2cBricked())
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
			topBorderBufferLoaded = true;
		}
		ms().macroMode ? drawVolumeImageMacro(volumeLevel) : drawVolumeImage(volumeLevel);
	}
}

ITCM_CODE void ThemeTextures::resetCachedVolumeLevel() {
	_cachedVolumeLevel = -1;
}

ITCM_CODE int ThemeTextures::getVolumeLevel(void) {
	if (!dsiFeatures() || sys().i2cBricked())
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
	const u8 batteryLevel = sys().batteryStatus();
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
			topBorderBufferLoaded = true;
		}
		ms().macroMode ? drawBatteryImageMacro(batteryLevel, dsiFeatures() && !sys().i2cBricked(), sys().isRegularDS()) : drawBatteryImage(batteryLevel, dsiFeatures() && !sys().i2cBricked(), sys().isRegularDS());
	}
}

ITCM_CODE void ThemeTextures::resetCachedBatteryLevel() {
	_cachedBatteryLevel = -1;
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
	toncset16(FontGraphic::textBuf[1], 0, SCREEN_WIDTH * smallFont()->height());
	smallFont()->print(0, 0, true, STR_NEXT, Alignment::left, RShoulderActive ? FontPalette::overlay : FontPalette::disabled);
	int width = smallFont()->calcWidth(STR_NEXT);
	// Copy text to background
	int align = tc().shoulderRTextAlign();
	int posX = tc().shoulderRTextX() - (align < 0 ? width : align == 0 ? width/2 : 0), posY = tc().shoulderRTextY();
	for (int y = 0; y < smallFont()->height() && posY + y < SCREEN_HEIGHT; y++) {
		if (posY + y < 0) continue;
		for (int x = 0; x < width && posX + x < SCREEN_WIDTH; x++) {
			if (posX + x < 0) continue;
			int px = FontGraphic::textBuf[1][y * SCREEN_WIDTH + x];
			u16 bg = _bgSubBuffer[(posY + y) * SCREEN_WIDTH + (posX + x)];
			u16 val = px ? themealphablend(BG_PALETTE[px], bg, (px % 4) < 2 ? 128 : 224) : bg;

			_bgSubBuffer[(posY + y) * SCREEN_WIDTH + (posX + x)] = val;
			if (boxArtColorDeband) {
				_bgSubBuffer2[(posY + y) * SCREEN_WIDTH + (posX + x)] = val;
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
	toncset16(FontGraphic::textBuf[1], 0, SCREEN_WIDTH * smallFont()->height());
	smallFont()->print(0, 0, true, STR_PREV, Alignment::left, LShoulderActive ? FontPalette::overlay : FontPalette::disabled);
	width = smallFont()->calcWidth(STR_PREV);
	// Copy text to background
	align = tc().shoulderLTextAlign();
	posX = tc().shoulderLTextX() - (align < 0 ? width : align == 0 ? width/2 : 0), posY = tc().shoulderLTextY();
	for (int y = 0; y < smallFont()->height() && posY + y < SCREEN_HEIGHT; y++) {
		if (posY + y < 0) continue;
		for (int x = 0; x < width && posX + x < SCREEN_WIDTH; x++) {
			if (posX + x < 0) continue;
			int px = FontGraphic::textBuf[1][y * SCREEN_WIDTH + x];
			u16 bg = _bgSubBuffer[(posY + y) * SCREEN_WIDTH + (posX + x)];
			u16 val = px ? themealphablend(BG_PALETTE[px], bg, (px % 4) < 2 ? 128 : 224) : bg;

			_bgSubBuffer[(posY + y) * SCREEN_WIDTH + (posX + x)] = val;
			if (boxArtColorDeband) {
				_bgSubBuffer2[(posY + y) * SCREEN_WIDTH + (posX + x)] = val;
			}
		}
	}

	commitBgSubModify();
}

ITCM_CODE void ThemeTextures::drawDateTime(const char *str, int posX, int posY, bool isDate) {
	if (!topBorderBufferLoaded) {
		_backgroundTextures[0].copy(_topBorderBuffer, false);
		topBorderBufferLoaded = true;
	}

	toncset16(FontGraphic::textBuf[1], 0, 256 * dateTimeFont()->height());
	dateTimeFont()->print(0, 0, true, str, Alignment::left, FontPalette::dateTime);
	int width = std::max(dateTimeFont()->calcWidth(str), isDate ? _previousDateWidth : _previousTimeWidth);

	// Copy to background
	for (int y = 0; y < dateTimeFont()->height() && posY + y < SCREEN_HEIGHT; y++) {
		if (posY + y < 0) continue;
		for (int x = 0; x < width && posX + x < SCREEN_WIDTH; x++) {
			if (posX + x < 0) continue;
			int px = FontGraphic::textBuf[1][y * 256 + x];
			u16 bg = _topBorderBuffer[(posY + y) * 256 + (posX + x)];
			u16 val = px ? themealphablend(BG_PALETTE[px], bg, (px % 4) < 2 ? 128 : 224) : bg;

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
		topBorderBufferLoaded = true;
	}

	toncset16(FontGraphic::textBuf[1], 0, 256 * dateTimeFont()->height());
	dateTimeFont()->print(0, 0, true, str, Alignment::left, FontPalette::dateTime);
	int width = std::max(dateTimeFont()->calcWidth(str), isDate ? _previousDateWidth : _previousTimeWidth);

	// Copy to background
	for (int y = 0; y < dateTimeFont()->height() && posY + y < SCREEN_HEIGHT; y++) {
		if (posY + y < 0) continue;
		for (int x = 0; x < width && posX + x < SCREEN_WIDTH; x++) {
			if (posX + x < 0) continue;
			int px = FontGraphic::textBuf[1][y * 256 + x];
			u16 bg = _topBorderBuffer[(posY + y) * 256 + (posX + x)];
			u16 val = px ? themealphablend(BG_PALETTE[px], bg, (px % 4) < 2 ? 128 : 224) : bg;

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

	if (_manualIconTexture && tc().iconManualUserPalette())
		_manualIconTexture->applyUserPaletteFile(TFN_PALETTE_ICON_MANUAL, effectDSiArrowButtonPalettes);
	if (_settingsIconTexture && tc().iconSettingsUserPalette())
		_settingsIconTexture->applyUserPaletteFile(TFN_PALETTE_ICON_SETTINGS, effectDSiArrowButtonPalettes);
}

u16 *ThemeTextures::bgSubBuffer2() { return _bgSubBuffer2; }
u16 *ThemeTextures::photoBuffer() { return _photoBuffer; }
u16 *ThemeTextures::photoBuffer2() { return _photoBuffer2; }
//u16 *ThemeTextures::frameBuffer(bool secondBuffer) { return _frameBuffer[secondBuffer]; }
u16 *ThemeTextures::frameBufferBot(bool secondBuffer) { return _frameBufferBot[secondBuffer]; }

void loadRotatingCubes() {
	std::string cubes = TFN_RVID_CUBES;
	FILE *videoFrameFile = fopen(cubes.c_str(), "rb");

	if (videoFrameFile) {
		bool doRead = false;
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
			// Compatible with RVID v2
			extern int rocketVideo_videoFrames;
			fseek(videoFrameFile, 0x8, SEEK_SET);
			fread((void*)&rocketVideo_videoFrames, sizeof(u32), 1, videoFrameFile);
			rocketVideo_videoFrames--;

			extern u8 rocketVideo_fps;
			fseek(videoFrameFile, 0xC, SEEK_SET);
			fread((void*)&rocketVideo_fps, sizeof(u8), 1, videoFrameFile);

			extern u8 rocketVideo_height;
			// fseek(videoFrameFile, 0xD, SEEK_SET);
			fread((void*)&rocketVideo_height, sizeof(u8), 1, videoFrameFile);

			u32 framesSize = (0x200*rocketVideo_height)*(rocketVideo_videoFrames+1);
			if (rocketVideo_height > 144 || framesSize > 0x700000) {
				fclose(videoFrameFile);
				return;
			}

			// Configured by tc().rotatingCubesRenderY()
			/* if (rocketVideo_height >= 58) {
				// Adjust video positioning
				extern int rocketVideo_videoYpos;
				for (int i = 58; i < rocketVideo_height; i += 2) {
					rocketVideo_videoYpos--;
				}
			} */

			u32 framesOffset = 0x200;
			fseek(videoFrameFile, 0x14, SEEK_SET);
			fread((void*)&framesOffset, sizeof(u32), 1, videoFrameFile);

			fseek(videoFrameFile, framesOffset, SEEK_SET);

			fread(rotatingCubesLocation, 1, framesSize, videoFrameFile);

			if (colorTable) {
				u16* rotatingCubesLocation16 = (u16*)rotatingCubesLocation;
				for (u32 i = 0; i < framesSize/2; i++) {
					rotatingCubesLocation16[i] = colorTable[rotatingCubesLocation16[i] % 0x8000] | BIT(15);
				}
			}

			rotatingCubesLoaded = true;
			rocketVideo_playVideo = true;
		}
		fclose(videoFrameFile);
	}
}
void ThemeTextures::unloadRotatingCubes() {
	if (dsiFeatures() && !ms().macroMode && ms().theme == TWLSettings::ETheme3DS && ms().consoleModel == 0) {
		toncset32(rotatingCubesLocation, 0, 0x700000/sizeof(u32)); // Clear video before freeing
		delete[] rotatingCubesLocation;
	}
}
void ThemeTextures::unloadPhotoBuffer() {
	if (!_photoBuffer) {
		return;
	}

	delete[] _photoBuffer;
	if (boxArtColorDeband) {
		delete[] _photoBuffer2;
	}

	_photoBuffer = NULL;
	_photoBuffer2 = NULL;
}
void ThemeTextures::reloadPhotoBuffer() {
	_photoBuffer = new u16[208 * 156];
	if (boxArtColorDeband) {
		_photoBuffer2 = new u16[208 * 156];
	}

	extern void reloadPhoto();
	reloadPhoto();
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

	char currentSettingPath[40];
	sprintf(currentSettingPath, "%s:/_nds/colorLut/currentSetting.txt", (sys().isRunFromSD() ? "sd" : "fat"));

	if (access(currentSettingPath, F_OK) == 0) {
		// Load color LUT
		char lutName[128] = {0};
		FILE* file = fopen(currentSettingPath, "rb");
		fread(lutName, 1, 128, file);
		fclose(file);

		char colorTablePath[256];
		sprintf(colorTablePath, "%s:/_nds/colorLut/%s.lut", (sys().isRunFromSD() ? "sd" : "fat"), lutName);

		if (getFileSize(colorTablePath) == 0x10000) {
			colorTable = new u16[0x10000/sizeof(u16)];

			FILE* file = fopen(colorTablePath, "rb");
			fread(colorTable, 1, 0x10000, file);
			fclose(file);

			const u16 color0 = colorTable[0] | BIT(15);
			const u16 color7FFF = colorTable[0x7FFF] | BIT(15);

			invertedColors =
			  (color0 >= 0xF000 && color0 <= 0xFFFF
			&& color7FFF >= 0x8000 && color7FFF <= 0x8FFF);
			if (!invertedColors) noWhiteFade = (color7FFF < 0xF000);
		}
	}

	REG_BLDCNT = BLEND_SRC_BG3 | (invertedColors ? BLEND_FADE_WHITE : BLEND_FADE_BLACK);

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

	_photoBuffer = new u16[208 * 156];

	boxArtColorDeband = (ms().boxArtColorDeband && !ms().macroMode && (sys().isRegularDS() ? sys().dsDebugRam() : ndmaEnabled()) && !rotatingCubesLoaded && ms().theme != TWLSettings::EThemeHBL);

	if (boxArtColorDeband) {
		_bgSubBuffer2 = new u16[256 * 192];
		_photoBuffer2 = new u16[208 * 156];
		_frameBufferBot[0] = new u16[256 * 192];
		_frameBufferBot[1] = new u16[256 * 192];
	}
}