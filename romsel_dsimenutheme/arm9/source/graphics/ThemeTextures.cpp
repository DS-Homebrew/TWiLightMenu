
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
extern bool rocketVideo_topVisible;
extern bool rocketVideo_dualScreen;
extern bool rocketVideo_onTop;
extern bool rocketVideo_onBottom;
extern u8 rocketVideo_guiRunCount[SCREEN_HEIGHT];
extern u16 rocketVideo_guiRuns[SCREEN_HEIGHT][2 * ROCKET_VIDEO_GUI_RUNS];
extern int rocketVideo_guiRows;
extern bool rocketVideo_interlaced;
extern bool rocketVideo_hasAlpha;
extern bool rocketVideo_indexed;
extern u32 *rocketVideo_spanIndex;
extern u16 *rocketVideo_spanData;
extern void resetVideoAlphaTracking(void);
extern void invalidateVideoAlphaRows(int screen, int y0, int y1);
extern bool rocketVideo_weaveRefill;
extern u8 rocketVideo_bandHeight;
extern u8 rocketVideo_height;
extern u8 rocketVideo_fps;
extern int rocketVideo_videoFrames;
extern int rocketVideo_loopFrame;
extern int rocketVideo_currentFrame;
extern int rocketVideo_prevFrame;
extern bool rocketVideo_field;
extern int rocketVideo_videoYpos;
extern int rocketVideo_videoYposBottom;
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

// Top screen pixels drawn by GUI elements (username, date, time, battery...), which a video
// on the top screen keeps in front of it, and the rows changed since flushTopGui() last ran.
static u8 _topGuiMask[256 * 192] = {0};
static int _topGuiDirtyY0 = SCREEN_HEIGHT;
static int _topGuiDirtyY1 = 0;
static int _topGuiDirtyBeforeY0 = SCREEN_HEIGHT;	// The range as it was when beginBgSubModify() widened it
static int _topGuiDirtyBeforeY1 = 0;
bool boxArtColorDeband = false;

static u8* boxArtCache = NULL;	// Size: 0x1B8000
static bool boxArtFound[40] = {false};
uint boxArtWidth = 0, boxArtHeight = 0;

ThemeTextures::ThemeTextures()
    : bubbleTexID(0), bipsTexID(0), scrollwindowTexID(0), buttonarrowTexID(0),
      movingarrowTexID(0), launchdotTexID(0), startTexID(0), startbrdTexID(0), settingsTexID(0), manualTexID(0), braceTexID(0),
      boxfullTexID(0), boxemptyTexID(0), folderTexID(0), cornerButtonTexID(0), smallCartTexID(0), smallCartFallbackTexID(0), progressTexID(0),
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

// Whether an icon of a 32px wide texture has no opaque pixels
static bool isSmallCartIconEmpty(const Texture &tex, unsigned int icon) {
	if ((tex.type() & TextureType::Compressed) || (icon + 1) * 32 > tex.texHeight()) {
		return false;
	}
	const unsigned int rowSize = (tex.type() & TextureType::Paletted) ? 32 / 2 : 32 * sizeof(u16);
	const u8 *pixels = tex.bytes() + icon * 32 * rowSize;
	for (unsigned int i = 0; i < 32 * rowSize; i++) {
		if (pixels[i] != 0) {
			return false;
		}
	}
	return true;
}

void ThemeTextures::loadSmallCartImage(const Texture &tex) {
	const unsigned int arraySize = (32 / 16) * (256 / 32);
	_smallCartImage = std::move(loadTexture(&smallCartTexID, tex, arraySize, 32, 32, GL_RGB16));

	if (ms().theme != TWLSettings::ETheme3DS || tex.texWidth() != 32) {
		return;
	}

	// Older 3DS themes leave the icons added later (Pictochat, DS Download Play, Internet Browser) empty, so use the default theme's
	const unsigned int iconCount = 256 / 32;
	bool missingIcon[iconCount];
	bool anyMissingIcon = false;
	for (unsigned int i = 0; i < iconCount; i++) {
		missingIcon[i] = (i >= tex.texHeight() / 32) || isSmallCartIconEmpty(tex, i);
		anyMissingIcon |= missingIcon[i];
	}
	if (!anyMissingIcon) {
		return;
	}

	const Texture fallbackTex(TFN_FALLBACK_GRF_SMALL_CART, "");
	if (fallbackTex.texWidth() != 32) {
		return;
	}
	unique_ptr<glImage[]> fallbackImage = loadTexture(&smallCartFallbackTexID, fallbackTex, arraySize, 32, 32, GL_RGB16);
	for (unsigned int i = 0; i < iconCount && i < fallbackTex.texHeight() / 32; i++) {
		if (missingIcon[i] && !isSmallCartIconEmpty(fallbackTex, i)) {
			_smallCartImage[i] = fallbackImage[i];
		}
	}
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
	_dateTimeFont = std::make_unique<FontGraphic>(((access((TFN_FONT_DATE_TIME).c_str(), F_OK) == 0) ? TFN_FONT_DATE_TIME : TFN_FALLBACK_FONT_DATE_TIME).c_str(), false, false, false);
	if (access((TFN_FONT_USERNAME).c_str(), F_OK) == 0) {
		_usernameFont = std::make_unique<FontGraphic>((TFN_FONT_USERNAME).c_str(), false, false, false);
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
// While a video plays, its band belongs to the vblank handler, and the matching rows of the
// background buffers are kept free of it: snapshots skip them, so they hold the background
// plus whatever was drawn over it (clock, battery...). Transparent video pixels are restored
// from there, and a whole-screen commit would otherwise write a stale frame over the band.
static bool videoOwnsTopBand() {
	return rotatingCubesLoaded && rocketVideo_playVideo && rocketVideo_onTop && rocketVideo_topVisible && !boxArtColorDeband;
}

static bool videoOwnsBottomBand() {
	return rotatingCubesLoaded && rocketVideo_playVideo && rocketVideo_onBottom && !ms().macroMode;
}

static void markTopGuiRows(int y0, int y1) {
	if (y0 < 0) y0 = 0;
	if (y1 > SCREEN_HEIGHT) y1 = SCREEN_HEIGHT;
	if (y0 >= y1) return;
	if (y0 < _topGuiDirtyY0) _topGuiDirtyY0 = y0;
	if (y1 > _topGuiDirtyY1) _topGuiDirtyY1 = y1;
}

// beginBgSubModify() assumes the whole screen may change. Writers that only touch a few rows
// call this afterwards, so only those rows get rebuilt and restored behind the video.
static void narrowTopGuiRows(int y0, int y1) {
	_topGuiDirtyY0 = _topGuiDirtyBeforeY0;
	_topGuiDirtyY1 = _topGuiDirtyBeforeY1;
	markTopGuiRows(y0, y1);
}

// Rebuild the GUI run lists of the changed rows inside the top band, and have the video
// restore the background behind its transparent pixels on those rows.
static void flushTopGui() {
	int y0 = _topGuiDirtyY0;
	int y1 = _topGuiDirtyY1;
	_topGuiDirtyY0 = SCREEN_HEIGHT;
	_topGuiDirtyY1 = 0;
	if (!rotatingCubesLoaded || !rocketVideo_onTop) return;

	if (y0 < rocketVideo_videoYpos) y0 = rocketVideo_videoYpos;
	if (y1 > rocketVideo_videoYpos + rocketVideo_bandHeight) y1 = rocketVideo_videoYpos + rocketVideo_bandHeight;
	if (y0 >= y1) return;

	u16 runs[2 * ROCKET_VIDEO_GUI_RUNS];
	for (int y = y0; y < y1; y++) {
		const u8 *mask = _topGuiMask + (y * SCREEN_WIDTH);
		int count = 0;
		for (int x = 0; x < SCREEN_WIDTH;) {
			if (!mask[x]) {
				x++;
				continue;
			}
			const int start = x;
			while (x < SCREEN_WIDTH && mask[x]) x++;
			if (count == ROCKET_VIDEO_GUI_RUNS) {
				runs[(2 * count) - 1] = x; // Out of room: stretch the last run over the gap
			} else {
				runs[2 * count] = start;
				runs[(2 * count) + 1] = x;
				count++;
			}
		}

		const int oldIE = enterCriticalSection();
		rocketVideo_guiRows += (count != 0) - (rocketVideo_guiRunCount[y] != 0);
		if (count) {
			tonccpy(rocketVideo_guiRuns[y], runs, sizeof(u16) * 2 * count);
		}
		rocketVideo_guiRunCount[y] = count;
		leaveCriticalSection(oldIE);
	}
	invalidateVideoAlphaRows(0, y0, y1);
}

void ThemeTextures::clearTopGuiMask(int x, int y, int w, int h) {
	if (x < 0) {
		w += x;
		x = 0;
	}
	if (y < 0) {
		h += y;
		y = 0;
	}
	if (x + w > SCREEN_WIDTH) w = SCREEN_WIDTH - x;
	if (y + h > SCREEN_HEIGHT) h = SCREEN_HEIGHT - y;
	if (w <= 0 || h <= 0) return;
	for (int row = y; row < y + h; row++) {
		toncset(_topGuiMask + (row * SCREEN_WIDTH) + x, 0, w);
	}
	markTopGuiRows(y, y + h);
}

// Snapshot the screen into a background buffer, leaving the band rows as they are.
static void snapshotAroundBand(const u16 *src, u16 *dst, int bandY) {
	const u32 aboveBytes = sizeof(u16) * SCREEN_WIDTH * bandY;
	const u32 belowOffset = SCREEN_WIDTH * (bandY + rocketVideo_bandHeight);
	const u32 belowBytes = sizeof(u16) * (BG_BUFFER_PIXELCOUNT - belowOffset);
	if (aboveBytes) {
		dmaCopyWords(3, src, dst, aboveBytes);
	}
	if (belowBytes) {
		dmaCopyWords(3, src + belowOffset, dst + belowOffset, belowBytes);
	}
}

// Copy a background buffer to VRAM around the band. When asynchronous, the rows above the
// band still go synchronously, as a DMA channel only takes one transfer at a time.
static void commitAroundBand(const u16 *src, u16 *dst, int bandY, bool async) {
	const u32 aboveBytes = sizeof(u16) * SCREEN_WIDTH * bandY;
	const u32 belowOffset = SCREEN_WIDTH * (bandY + rocketVideo_bandHeight);
	const u32 belowBytes = sizeof(u16) * (BG_BUFFER_PIXELCOUNT - belowOffset);
	if (aboveBytes) {
		dmaCopyWords(2, src, dst, aboveBytes);
	}
	if (belowBytes) {
		if (async) {
			dmaCopyWordsAsynch(2, src + belowOffset, dst + belowOffset, belowBytes);
		} else {
			dmaCopyWords(2, src + belowOffset, dst + belowOffset, belowBytes);
		}
	}
}

u16 *ThemeTextures::beginBgSubModify() {
	if (ms().macroMode)
		return _bgSubBuffer;

	u16* bgLoc = BG_GFX_SUB;
	if (boxArtColorDeband) {
		bgLoc = _frameBufferBot[0];
	}
	_topGuiDirtyBeforeY0 = _topGuiDirtyY0;
	_topGuiDirtyBeforeY1 = _topGuiDirtyY1;
	markTopGuiRows(0, SCREEN_HEIGHT);
	if (videoOwnsTopBand()) {
		snapshotAroundBand(bgLoc, _bgSubBuffer, rocketVideo_videoYpos);
	} else {
		dmaCopyWords(3, bgLoc, _bgSubBuffer, sizeof(u16) * BG_BUFFER_PIXELCOUNT);
	}
	if (boxArtColorDeband) {
		dmaCopyWords(3, _frameBufferBot[1], _bgSubBuffer2, sizeof(u16) * BG_BUFFER_PIXELCOUNT);
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
	if (videoOwnsTopBand()) {
		commitAroundBand(_bgSubBuffer, bgLoc, rocketVideo_videoYpos, false);
	} else {
		dmaCopyWords(2, _bgSubBuffer, bgLoc, sizeof(u16) * BG_BUFFER_PIXELCOUNT);
	}
	if (boxArtColorDeband) {
		dmaCopyWords(2, _bgSubBuffer2, _frameBufferBot[1], sizeof(u16) * BG_BUFFER_PIXELCOUNT);
	}
	flushTopGui();
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
	if (videoOwnsTopBand()) {
		commitAroundBand(_bgSubBuffer, bgLoc, rocketVideo_videoYpos, true);
	} else {
		dmaCopyWordsAsynch(2, _bgSubBuffer, bgLoc, sizeof(u16) * BG_BUFFER_PIXELCOUNT);
	}
	if (boxArtColorDeband) {
		if (ndmaEnabled()) {
			ndmaCopyWordsAsynch(2, _bgSubBuffer2, _frameBufferBot[1], sizeof(u16) * BG_BUFFER_PIXELCOUNT);
		} else {
			tonccpy(_frameBufferBot[1], _bgSubBuffer2, sizeof(u16) * BG_BUFFER_PIXELCOUNT);
		}
	}
	flushTopGui();
}

u16 *ThemeTextures::beginBgMainModify() {
	u16* bgLoc = BG_GFX;
	/*if (boxArtColorDeband) {
		bgLoc = _frameBufferBot[0];
	}*/
	if (videoOwnsBottomBand()) {
		snapshotAroundBand(bgLoc, _bgMainBuffer, rocketVideo_videoYposBottom);
	} else {
		dmaCopyWords(3, bgLoc, _bgMainBuffer, sizeof(u16) * BG_BUFFER_PIXELCOUNT);
	}
	/*if (ndmaEnabled()) {
		dmaCopyWords(3, _frameBuffer[1], _bgMainBuffer, sizeof(u16) * BG_BUFFER_PIXELCOUNT);
	}*/
	return _bgMainBuffer;
}

void ThemeTextures::commitBgMainModify() {
	u16* bgLoc = BG_GFX;
	/*if (boxArtColorDeband) {
		bgLoc = _frameBufferBot[0];
	}*/
	DC_FlushRange(_bgMainBuffer, sizeof(u16) * BG_BUFFER_PIXELCOUNT);
	if (videoOwnsBottomBand()) {
		commitAroundBand(_bgMainBuffer, bgLoc, rocketVideo_videoYposBottom, false);
		invalidateVideoAlphaRows(1, 0, SCREEN_HEIGHT);
	} else {
		dmaCopyWords(2, _bgMainBuffer, bgLoc, sizeof(u16) * BG_BUFFER_PIXELCOUNT);
	}
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
	if (videoOwnsBottomBand()) {
		commitAroundBand(_bgMainBuffer, bgLoc, rocketVideo_videoYposBottom, true);
		invalidateVideoAlphaRows(1, 0, SCREEN_HEIGHT);
	} else {
		dmaCopyWordsAsynch(2, _bgMainBuffer, bgLoc, sizeof(u16) * BG_BUFFER_PIXELCOUNT);
	}
	/*if (boxArtColorDeband) {
		ndmaCopyWordsAsynch(2, _bgMainBuffer, _frameBuffer[1], sizeof(u16) * BG_BUFFER_PIXELCOUNT);
	}*/
}

void ThemeTextures::drawTopBg() {
	beginBgSubModify();

	_backgroundTextures[0].copy(_bgSubBuffer, false);
	toncset(_topGuiMask, 0, sizeof(_topGuiMask));

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
	toncset(_topGuiMask, 0, sizeof(_topGuiMask));
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
				_topGuiMask[(yPos + y) * 256 + (xPos + x)] = (px != 0);
				_bgSubBuffer[(yPos + y) * 256 + (xPos + x)] = val;
				if (boxArtColorDeband) {
					_bgSubBuffer2[(yPos + y) * 256 + (xPos + x)] = val;
				}
			}
		}
	}

	if (!ms().macroMode) {
		markTopGuiRows(yPos, yPos + usernameFont()->height());
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

bool ThemeTextures::drawBoxArt(const char *filename, bool inMem) {
	logPrint("drawBoxArt\n");

	logPrint("filename: ");
	logPrint(filename);

	if (inMem ? !boxArtFound[CURPOS] : access(filename, F_OK) != 0) {
		logPrint(" not found\n");
		return false;
	}
	logPrint("\n");

	if (extension(filename, {".bmp"})) {
		return drawBoxArtBmp(filename, inMem);
	}
	return drawBoxArtPng(filename, inMem);
}

bool ThemeTextures::drawBoxArtBmp(const char *filename, bool inMem) {
	logPrint("drawBoxArtBmp\n");

	FILE* file = inMem ? NULL : fopen(filename, "rb");

	uint imageXpos, imageYpos;
	u8* fileMem = inMem ? boxArtCache+(CURPOS*0xB000) : NULL;
	u8* image = NULL;
	u16* image16 = NULL;

	// Read width & height
	u32 width, height;
	if (!inMem) {
		fseek(file, 0x12, SEEK_SET);
		fread(&width, 1, sizeof(width), file);
		fread(&height, 1, sizeof(height), file);
	} else {
		tonccpy(&width, fileMem+0x12, sizeof(width));
		tonccpy(&height, fileMem+0x16, sizeof(height));
	}

	if (width > 256 || height > 192) {
		logPrint("Width and/or height is too high\n");
		if (!inMem) fclose(file);
		return false;
	}

	if (ms().theme == TWLSettings::ETheme3DS && rocketVideo_topVisible) {
		rocketVideo_topVisible = false;
		while (dmaBusy(1)); // Wait for frame to finish rendering
		drawOverRotatingCubes(); // Clear top screen cubes for 3DS theme
	}

	if (ms().theme == TWLSettings::ETheme3DS) {
		extern uint photoWidth, photoHeight;
		tex().drawOverBoxArt(photoWidth, photoHeight);
	}

	boxArtWidth = width;
	boxArtHeight = height;

	bool alternatePixel = false;
	bool alternatePixel2 = false;

	beginBgSubModify();

	u16* bmpImageBuffer = new u16[256 * 192];
	u16* bmpImageBuffer2 = boxArtColorDeband ? new u16[256 * 192] : NULL;

	imageXpos = (256-boxArtWidth)/2;
	imageYpos = (192-boxArtHeight)/2;

	if (!inMem) fseek(file, 0x1C, SEEK_SET);
	u8 bitsPerPixel = inMem ? fileMem[0x1C] : fgetc(file);
	int bits = (bitsPerPixel == 32) ? 4 : 3;
	if (!inMem) fseek(file, 0xE, SEEK_SET);
	u8 headerSize = inMem ? fileMem[0xE] : fgetc(file);
	bool rgb565 = false;
	if (!inMem) {
		if (headerSize == 0x38) {
			fseek(file, 0x2C, SEEK_CUR);
			rgb565 = fgetc(file) == 0x07;
			fseek(file, headerSize - 0x2E, SEEK_CUR);
		} else {
			fseek(file, headerSize - 1, SEEK_CUR);
		}
	} else {
		if (headerSize == 0x38) {
			rgb565 = fileMem[0x3B] == 0x07;
		}
		fileMem += 0xE + headerSize;
	}
	if (bitsPerPixel == 24 || bitsPerPixel == 32) { // 24-bit or 32-bit
		logPrint("BMP is 26/32-bit\n");
		image = new u8[(width * height)*bits];
		if (inMem) {
			tonccpy(image, fileMem, (width * height)*bits);
		} else {
			fread(image, bits, width * height, file);
		}
	} else if (bitsPerPixel == 16) { // 16-bit
		logPrint("BMP is 16-bit\n");
		image16 = new u16[width * height];
		if (inMem) {
			tonccpy(image16, fileMem, (width * height)*2);
		} else {
			fread(image16, 2, width * height, file);
		}
	} else if (bitsPerPixel == 8) { // 8-bit
		logPrint("BMP is 8-bit\n");
		u8* pixelBufferB = new u8[256];
		u8* pixelBufferG = new u8[256];
		u8* pixelBufferR = new u8[256];
		if (inMem) {
			for (int i = 0; i < 256; i++) {
				pixelBufferB[i] = *fileMem++;
				pixelBufferG[i] = *fileMem++;
				pixelBufferR[i] = *fileMem++;
				fileMem++;
			}
		} else {
			for (int i = 0; i < 256; i++) {
				u8 unk = 0;
				fread(&pixelBufferB[i], 1, 1, file);
				fread(&pixelBufferG[i], 1, 1, file);
				fread(&pixelBufferR[i], 1, 1, file);
				fread(&unk, 1, 1, file);
			}
		}

		image = new u8[(width * height)*3];
		u8 *image8 = new u8[width * height];
		if (inMem) {
			tonccpy(image8, fileMem, width * height);
		} else {
			fread(image8, 1, width * height, file);
		}

		for (u32 i = 0; i < width*height; i++) {
			image[(i*3)] = pixelBufferB[image8[i]];
			image[(i*3)+1] = pixelBufferG[image8[i]];
			image[(i*3)+2] = pixelBufferR[image8[i]];
		}
		delete[] pixelBufferB;
		delete[] pixelBufferG;
		delete[] pixelBufferR;
		delete[] image8;
	} else /* if (bitsPerPixel == 1) */ { // 1-bit (Not supported)
		logPrint("BMP is invalid\n");
		if (!inMem) fclose(file);
		return false;
	}
	if (!inMem) fclose(file);

	int x = 0;
	int y = boxArtHeight-1;
	if (bitsPerPixel == 16) {
		for (u32 i = 0; i < width*height; i++) {
			const u16 val = image16[i];
			u16 color = 0;
			if (rgb565) {
				color = ((val >> 11) & 0x1F) | ((val & (0x1F << 6)) >> 1) | ((val & 0x1F) << 10) | BIT(15);
			} else {
				color = ((val >> 10) & 0x1F) | ((val) & (0x1F << 5)) | ((val & 0x1F) << 10) | BIT(15);
			}
			if (colorTable) {
				color = colorTable[color % 0x8000] | BIT(15);
			}
			bmpImageBuffer[x+(y*boxArtWidth)] = color;
			if (boxArtColorDeband) {
				bmpImageBuffer2[x+(y*boxArtWidth)] = color;
			}
			x++;
			if (x == (int)boxArtWidth) {
				if ((x % 2) == 0) {
					alternatePixel = !alternatePixel;
					alternatePixel2 = !alternatePixel2;
				}
				x=0;
				y--;
			}
			alternatePixel = !alternatePixel;
			alternatePixel2 = !alternatePixel2;
		}
	} else for (int b = 0; b < boxArtColorDeband+1; b++) {
		for (u32 i = 0; i < width*height; i++) {
			const u8 oldR = image[(i*bits)+2];
			const u8 oldG = image[(i*bits)+1];
			const u8 oldB = image[(i*bits)];
			u8 newR = oldR;
			u8 newG = oldG;
			u8 newB = oldB;
			if (alternatePixel) {
				if (oldR >= 4 && oldR < 0xFC) newR += 4;
				if (oldG >= 4 && oldG < 0xFC) newG += 4;
				if (oldB >= 4 && oldB < 0xFC) newB += 4;
			}
			if (alternatePixel2 && boxArtColorDeband) {
				if (oldR >= 2 && newR < 0xFE) newR += 2;
				if (oldG >= 2 && newG < 0xFE) newG += 2;
				if (oldB >= 2 && newB < 0xFE) newB += 2;
			}
			u16 color = newR>>3 | (newG>>3)<<5 | (newB>>3)<<10 | BIT(15);
			if (colorTable) {
				color = colorTable[color % 0x8000] | BIT(15);
			}
			if (b == 0) {
				bmpImageBuffer[x+(y*boxArtWidth)] = color;
			} else if (boxArtColorDeband) {
				bmpImageBuffer2[x+(y*boxArtWidth)] = color;
			}
			x++;
			if (x == (int)boxArtWidth) {
				if ((x % 2) == 0) {
					alternatePixel = !alternatePixel;
					alternatePixel2 = !alternatePixel2;
				}
				x=0;
				y--;
			}
			alternatePixel = !alternatePixel;
			alternatePixel2 = !alternatePixel2;
		}
		alternatePixel = !alternatePixel;
		y = boxArtHeight-1;
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

	if (image) delete[] image;
	if (image16) delete[] image16;
	delete[] bmpImageBuffer;
	if (boxArtColorDeband) {
		delete[] bmpImageBuffer2;
	}

	return true;
}

bool ThemeTextures::drawBoxArtPng(const char *filename, bool inMem) {
	logPrint("drawBoxArtPng\n");

	std::vector<unsigned char> image;
	uint imageWidth, imageHeight;
	uint imageXpos, imageYpos;
	if (inMem) {
		lodepng::decode(image, imageWidth, imageHeight, (unsigned char*)boxArtCache+(CURPOS*0xB000), 0xB000);
	} else {
		lodepng::decode(image, imageWidth, imageHeight, filename);
	}
	bool alternatePixel = false;
	bool alternatePixel2 = false;
	if (imageWidth > 256 || imageHeight > 192) {
		logPrint("Width and/or height is too high\n");
		return false;
	}

	if (ms().theme == TWLSettings::ETheme3DS && rocketVideo_topVisible) {
		rocketVideo_topVisible = false;
		while (dmaBusy(1)); // Wait for frame to finish rendering
		drawOverRotatingCubes(); // Clear top screen cubes for 3DS theme
	}

	if (ms().theme == TWLSettings::ETheme3DS) {
		extern uint photoWidth, photoHeight;
		tex().drawOverBoxArt(photoWidth, photoHeight);
	}

	boxArtWidth = imageWidth;
	boxArtHeight = imageHeight;

	beginBgSubModify();

	u16* bmpImageBuffer = new u16[256 * 192];
	u16* bmpImageBuffer2 = boxArtColorDeband ? new u16[256 * 192] : NULL;

	imageXpos = (256-boxArtWidth)/2;
	imageYpos = (192-boxArtHeight)/2;

	int photoXstart = imageXpos;
	int photoXend = imageXpos+boxArtWidth;
	int photoX = photoXstart;
	int photoY = imageYpos;

	for (int b = 0; b < boxArtColorDeband+1; b++) {
		for (uint i=0;i<image.size()/4;i++) {
			const u8 oldR = image[(i*4)];
			const u8 oldG = image[(i*4)+1];
			const u8 oldB = image[(i*4)+2];
			const u8 oldAlpha = image[(i*4)+3];
			u8 newR = oldR;
			u8 newG = oldG;
			u8 newB = oldB;
			u8 newAlpha = oldAlpha;
			if (alternatePixel) {
				if (oldR >= 4 && oldR < 0xFC) newR += 4;
				if (oldG >= 4 && oldG < 0xFC) newG += 4;
				if (oldB >= 4 && oldB < 0xFC) newB += 4;
				if (oldAlpha >= 4 && oldAlpha < 0xFC) newAlpha += 4;
			}
			if (alternatePixel2 && boxArtColorDeband) {
				if (oldR >= 2 && newR < 0xFE) newR += 2;
				if (oldG >= 2 && newG < 0xFE) newG += 2;
				if (oldB >= 2 && newB < 0xFE) newB += 2;
				if (oldAlpha >= 2 && newAlpha < 0xFE) newAlpha += 2;
			}
			u16 color = newR>>3 | (newG>>3)<<5 | (newB>>3)<<10 | BIT(15);
			if (colorTable) {
				color = colorTable[color % 0x8000] | BIT(15);
			}
			if (b == 0) {
				if (oldAlpha == 255) {
					bmpImageBuffer[i] = color;
				} else {
					bmpImageBuffer[i] = alphablend(color, _bgSubBuffer[(photoY*256)+photoX], newAlpha);
				}
			} else if (boxArtColorDeband) {
				if (oldAlpha == 255) {
					bmpImageBuffer2[i] = color;
				} else {
					bmpImageBuffer2[i] = alphablend(color, _bgSubBuffer2[(photoY*256)+photoX], newAlpha);
				}
			}
			if ((i % boxArtWidth) == boxArtWidth-1) {
				alternatePixel = !alternatePixel;
				alternatePixel2 = !alternatePixel2;
			}
			alternatePixel = !alternatePixel;
			alternatePixel2 = !alternatePixel2;
			photoX++;
			if (photoX == photoXend) {
				photoX = photoXstart;
				photoY++;
			}
		}
		alternatePixel = !alternatePixel;
		photoY = imageYpos;
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

	return true;
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
		clearTopGuiMask(boxArtX, boxArtY, boxArtWidth, boxArtHeight);
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
		clearTopGuiMask(blackX, blackY, blackWidth, blackHeight);
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
}

// Redraw background over the rotating cubes bounds. Snapshots taken while the video played
// skipped the band, so those rows of _bgSubBuffer still hold the background along with
// anything drawn over it, such as the clock or battery.
void ThemeTextures::drawOverRotatingCubes() {
	if (!rotatingCubesLoaded || !rocketVideo_onTop) return;

	const u32 offset = (u32)rocketVideo_videoYpos * SCREEN_WIDTH;
	const u32 size = sizeof(u16) * SCREEN_WIDTH * rocketVideo_bandHeight;
	DC_FlushRange(_bgSubBuffer + offset, size);
	while (REG_VCOUNT != 191); // Fix screen tearing
	dmaCopyWords(2, _bgSubBuffer + offset, (u16*)BG_GFX_SUB + offset, size);
}

// Redraw the bottom screen background over the second video band.
// _bgMainBuffer always holds the pristine bottom background: the video writes straight
// to BG_GFX and never touches it, and the partial main-BG writers are macro mode only,
// where the video is never loaded.
void ThemeTextures::drawOverRotatingCubesBottom() {
	if (!rocketVideo_onBottom || ms().macroMode) return;

	const u32 offset = (u32)rocketVideo_videoYposBottom * SCREEN_WIDTH;
	const u32 size = sizeof(u16) * SCREEN_WIDTH * rocketVideo_bandHeight;
	DC_FlushRange(_bgMainBuffer + offset, size);
	dmaCopyWords(3, _bgMainBuffer + offset, (u16*)BG_GFX + offset, size);
}

ITCM_CODE void ThemeTextures::drawVolumeImage(int volumeLevel) {
	if (!dsiFeatures() || sys().i2cBricked())
		return;
	beginBgSubModify();

	const Texture *tex = volumeTexture(volumeLevel);
	const u16 *src = tex->texture();
	int startX = tc().volumeRenderX();
	int startY = tc().volumeRenderY();
	narrowTopGuiRows(startY, startY + tex->texHeight());
	for (uint y = 0; y < tex->texHeight(); y++) {
		for (uint x = 0; x < tex->texWidth(); x++) {
			u16 val = *(src++);
			_topGuiMask[(startY + y) * 256 + startX + x] = (val & BIT(15)) != 0;
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
	narrowTopGuiRows(tc().batteryRenderY(), tc().batteryRenderY() + tex->texHeight());
	for (uint y = tc().batteryRenderY(); y < tc().batteryRenderY() + tex->texHeight(); y++) {
		for (uint x = tc().batteryRenderX(); x < tc().batteryRenderX() + tex->texWidth(); x++) {
			u16 val = *(src++);
			_topGuiMask[y * 256 + x] = (val & BIT(15)) != 0;
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
				_topGuiMask[y * 256 + x] = 1;
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

			if (px) _topGuiMask[(posY + y) * SCREEN_WIDTH + (posX + x)] = 1;
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
				_topGuiMask[y * 256 + x] = 1;
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

			if (px) _topGuiMask[(posY + y) * SCREEN_WIDTH + (posX + x)] = 1;
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

	// Copy to background. The text also goes into _bgSubBuffer and the GUI mask; rows inside a
	// playing video's band are left to the video, which copies the text back over its frame.
	const bool videoPlaying = videoOwnsTopBand();
	for (int y = 0; y < dateTimeFont()->height() && posY + y < SCREEN_HEIGHT; y++) {
		if (posY + y < 0) continue;
		const bool rowInVideoBand = videoPlaying && posY + y >= rocketVideo_videoYpos
			&& posY + y < rocketVideo_videoYpos + rocketVideo_bandHeight;
		for (int x = 0; x < width && posX + x < SCREEN_WIDTH; x++) {
			if (posX + x < 0) continue;
			int px = FontGraphic::textBuf[1][y * 256 + x];
			u16 bg = _topBorderBuffer[(posY + y) * 256 + (posX + x)];
			u16 val = px ? themealphablend(BG_PALETTE[px], bg, (px % 4) < 2 ? 128 : 224) : bg;

			_topGuiMask[(posY + y) * 256 + (posX + x)] = (px != 0);
			_bgSubBuffer[(posY + y) * 256 + (posX + x)] = val;
			if (!rowInVideoBand) {
				BG_GFX_SUB[(posY + y) * 256 + (posX + x)] = val;
			}
			if (boxArtColorDeband) {
				_frameBufferBot[0][(posY + y) * 256 + (posX + x)] = val;
				_frameBufferBot[1][(posY + y) * 256 + (posX + x)] = val;
			}
		}
	}

	// Without a commit to do it, rebuild the video's GUI runs here, whether or not the band is
	// visible right now.
	markTopGuiRows(posY, posY + dateTimeFont()->height());
	flushTopGui();

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

u16 *ThemeTextures::bgMainBuffer() { return _bgMainBuffer; }
u16 *ThemeTextures::bgSubBuffer() { return _bgSubBuffer; }
u16 *ThemeTextures::bgSubBuffer2() { return _bgSubBuffer2; }
u16 *ThemeTextures::photoBuffer() { return _photoBuffer; }
u16 *ThemeTextures::photoBuffer2() { return _photoBuffer2; }
//u16 *ThemeTextures::frameBuffer(bool secondBuffer) { return _frameBuffer[secondBuffer]; }
u16 *ThemeTextures::frameBufferBot(bool secondBuffer) { return _frameBufferBot[secondBuffer]; }

// RVID v3-v5 header, as written by Vid2RVID (see RocketVideoPlayer's rvidHeaderInfo4).
// Naturally aligned, so the struct layout matches the file layout exactly.
struct RvidHeader {
	u32 magic;			// "RVID"
	u32 ver;
	u32 frames;
	u8 fps;				// >= 0x80: subtract 0x80 (NTSC), 0: console refresh rate
	u8 vRes;			// Rows per stored frame; the field height when interlaced
	u8 interlaced;
	u8 dualScreen;		// 1 = top and bottom screen, 2 = video is for GBA
	u16 sampleRate;
	u8 audioBitMode;
	u8 bmpMode;			// 0 = 8 BPP + palette, 1 = 16 BPP RGB555, 2 = 16 BPP RGB565 (bit 15 holds green)
	u32 compressedFrameSizeTableOffset;	// 0 when the frames are not compressed
	u32 soundLeftOffset;
	u32 soundRightOffset;
};
static_assert(sizeof(RvidHeader) == 0x20, "RVID header layout must match the file");

#define RVID_MAGIC				0x44495652	// "RVID"
#define RVID_FRAME_TABLE_OFFSET	0x200
#define ROTATING_CUBES_MAX_SIZE	0x700000	// Fixed video buffer: Slot-2 RAM pak, 3DS and dev units
// On a DSi, heap kept free once the video buffer is taken, for everything the menu loads after
// it (fonts, icons, box art decoding). Measured: about 1MB more is in use once the menu is idle.
#define VIDEO_RAM_RESERVE		0x200000

static u32 rotatingCubesBufferSize = ROTATING_CUBES_MAX_SIZE;
static bool rotatingCubesBufferAllocated = false;	// DSi: sized to the video and freed on unload

// Apply the screen color filter and/or force the alpha bit on a decoded 16 BPP frame.
// RGB555 frames keep their alpha bit, so pixels with bit 15 clear let the theme show
// through; RGB565 frames use bit 15 for green and are always opaque.
static void applyRotatingCubesColor(u16 *frame, u32 pixels, u8 bmpMode) {
	const bool keepAlpha = (bmpMode == 1);
	if (colorTable) {
		for (u32 i = 0; i < pixels; i++) {
			const u16 alpha = keepAlpha ? (frame[i] & BIT(15)) : BIT(15);
			frame[i] = colorTable[frame[i] % 0x8000] | alpha;
		}
	} else if (!keepAlpha) {
		for (u32 i = 0; i < pixels; i++) {
			frame[i] |= BIT(15);
		}
	}
}

// Transparent RGB555 videos are blitted from per-row run lists instead of pixel by pixel,
// which is far too slow for the vblank handler. They live in the video buffer after the
// frames: a u32 offset per sub-frame, then for each row a u16 count followed by that many
// x positions where the run flips between opaque and transparent. Runs start opaque at
// x = 0 and the last one ends at the screen edge. A row whose positions do not fit gets
// the count ALPHA_ROW_PER_PIXEL instead, and is blitted pixel by pixel.
#define ALPHA_ROW_PER_PIXEL 0xFFFF

struct AlphaSpanWriter {
	u32 *index;
	u16 *data;
	u16 *out;
	u16 *end;
	u32 rowsOwed;	// Headers of rows not written yet, which [out, end) always has room for
	bool ok;
	bool transparent;
};

// Lay the index and span data out after framesBytes of frames, for entries sub-frames of
// rows rows each.
static void initAlphaSpans(AlphaSpanWriter &w, u32 framesBytes, u32 entries, u8 rows) {
	const u32 bufferSize = rotatingCubesBufferSize;
	const u32 indexOffset = (framesBytes + 3) & ~3;
	const u32 dataOffset = indexOffset + (entries * 4);
	w.index = (u32*)(rotatingCubesLocation + indexOffset);
	w.data = w.out = (u16*)(rotatingCubesLocation + dataOffset);
	w.end = (u16*)(rotatingCubesLocation + bufferSize);
	w.rowsOwed = entries * rows;
	w.ok = (dataOffset < bufferSize) && ((u32)(w.end - w.out) >= w.rowsOwed);
	w.transparent = false;
}

static void addAlphaSpans(AlphaSpanWriter &w, u32 entry, const u16 *frame, u8 rows) {
	if (!w.ok) return;
	w.index[entry] = (u32)(w.out - w.data);
	u16 flipsAt[SCREEN_WIDTH];
	for (u8 y = 0; y < rows; y++) {
		u16 flips = 0;
		bool opaque = true;
		for (int x = 0; x < SCREEN_WIDTH; x++) {
			const bool pixelOpaque = (*frame++ & BIT(15));
			if (pixelOpaque != opaque) {
				flipsAt[flips++] = x;
				opaque = pixelOpaque;
			}
		}
		if (flips) w.transparent = true;

		w.rowsOwed--;
		if (flips && (u32)(w.end - w.out) < 1u + flips + w.rowsOwed) {
			*w.out++ = ALPHA_ROW_PER_PIXEL; // Too noisy to fit
			continue;
		}
		// Word by word: on a regular DS this may be the Slot-2 RAM pak's 16-bit bus
		*w.out++ = flips;
		for (u16 i = 0; i < flips; i++) {
			*w.out++ = flipsAt[i];
		}
	}
}

static void finishAlphaSpans(const AlphaSpanWriter &w) {
	rocketVideo_hasAlpha = (w.ok && w.transparent);
	rocketVideo_spanIndex = w.index;
	rocketVideo_spanData = w.data;
}

// Tables and scratch buffers for decoding v3-v5 sub-frames.
// 8 BPP sub-frames are stored as their palette indices followed by their 256-color palette,
// and turned into 16 BPP only when blitted; the others are stored as 16 BPP.
struct RvidDecoder {
	RvidHeader header;
	u32 entries;		// Sub-frames in the file
	u32 srcFrameBytes;	// Pixel bytes of a sub-frame in the file
	u32 dstFrameBytes;	// Bytes of a sub-frame in the video buffer
	u32 *offsets;
	u32 *sizes;			// NULL when the frames are not compressed
	// Frames bound for the Slot-2 RAM pak are decoded into main RAM and only then copied:
	// its 16-bit bus cannot take the byte-wide writes that LZ77 decompression performs.
	u8 *frameScratch;
	u8 *compressedScratch;
	u16 *palette;
};

static void closeRvidDecoder(RvidDecoder &d) {
	delete[] d.palette;
	delete[] d.compressedScratch;
	delete[] d.frameScratch;
	delete[] d.sizes;
	delete[] d.offsets;
	d.compressedScratch = d.frameScratch = NULL;
	d.palette = NULL;
	d.sizes = d.offsets = NULL;
}

static bool openRvidDecoder(RvidDecoder &d, FILE *file, const RvidHeader &header, u32 entries) {
	d.header = header;
	d.entries = entries;
	d.srcFrameBytes = (header.bmpMode == 0 ? 0x100 : 0x200) * header.vRes;
	d.dstFrameBytes = (header.bmpMode == 0) ? d.srcFrameBytes + 0x200 : 0x200 * header.vRes;
	d.offsets = new u32[entries];
	d.sizes = NULL;
	d.frameScratch = d.compressedScratch = NULL;
	d.palette = NULL;

	fseek(file, RVID_FRAME_TABLE_OFFSET, SEEK_SET);
	if (fread(d.offsets, 4, entries, file) != entries) {
		closeRvidDecoder(d);
		return false;
	}

	// From v4 on, the low 2 bits of an offset select one of up to four part files.
	// Theme videos are single file, so refuse rather than seek to the wrong place.
	if (header.ver >= 4) {
		for (u32 i = 0; i < entries; i++) {
			if (d.offsets[i] & 3) {
				closeRvidDecoder(d);
				return false;
			}
		}
	}

	if (header.compressedFrameSizeTableOffset != 0) {
		d.sizes = new u32[entries];
		fseek(file, header.compressedFrameSizeTableOffset, SEEK_SET);
		// The table holds a u16 per sub-frame for 8 BPP videos and a u32 for 16 BPP ones.
		bool sizesOk = true;
		if (header.bmpMode == 0) {
			u16 *sizes16 = new u16[entries];
			sizesOk = (fread(sizes16, 2, entries, file) == entries);
			for (u32 i = 0; i < entries; i++) {
				d.sizes[i] = sizes16[i];
			}
			delete[] sizes16;
		} else {
			sizesOk = (fread(d.sizes, 4, entries, file) == entries);
		}
		if (!sizesOk) {
			closeRvidDecoder(d);
			return false;
		}
		d.compressedScratch = new u8[d.srcFrameBytes];
	}

	d.frameScratch = new u8[d.dstFrameBytes];
	if (header.bmpMode == 0) {
		d.palette = new u16[256];
	}
	return true;
}

// LZ77 (type 0x10) decompression. Much faster than the BIOS call, which shortens loading.
// Returns false when the data would not fit maxSize.
ITCM_CODE static bool decompressLz77(const u8 *src, u8 *dst, u32 maxSize) {
	if (src[0] != 0x10) return false;
	const u32 size = src[1] | (src[2] << 8) | (src[3] << 16);
	if (size > maxSize) return false;
	src += 4;

	const u8 *const start = dst;
	const u8 *const end = dst + size;
	while (dst < end) {
		u8 flags = *src++;
		for (int i = 0; i < 8 && dst < end; i++, flags <<= 1) {
			if (flags & 0x80) {
				u32 len = (src[0] >> 4) + 3;
				const u32 disp = (((src[0] & 0xF) << 8) | src[1]) + 1;
				src += 2;
				if (disp > (u32)(dst - start)) return false;
				const u8 *from = dst - disp;
				if (len > (u32)(end - dst)) len = end - dst;
				while (len--) {
					*dst++ = *from++;
				}
			} else {
				*dst++ = *src++;
			}
		}
	}
	return true;
}

// Decode one sub-frame into out (d.dstFrameBytes long, byte-addressable memory) in its stored
// form, with the color filter applied.
static bool decodeRvidSubFrame(RvidDecoder &d, FILE *file, u32 entry, u8 *out) {
	const RvidHeader &header = d.header;
	fseek(file, d.offsets[entry], SEEK_SET);

	// The palette is stored ahead of the pixels and is never compressed.
	if (header.bmpMode == 0 && fread(d.palette, 2, 256, file) != 256) {
		return false;
	}

	u8 *decodeDst = out;
	const u32 size = d.sizes ? d.sizes[entry] : d.srcFrameBytes;
	if (size == d.srcFrameBytes) {
		if (fread(decodeDst, 1, d.srcFrameBytes, file) != d.srcFrameBytes) return false;
	} else if (size == 0 || size > d.srcFrameBytes) {
		return false; // Bogus size table
	} else {
		if (fread(d.compressedScratch, 1, size, file) != size) return false;
		if (!decompressLz77(d.compressedScratch, decodeDst, d.srcFrameBytes)) {
			decompress(d.compressedScratch, decodeDst, LZ77);
		}
	}

	if (header.bmpMode == 0) {
		if (colorTable) {
			for (int c = 0; c < 256; c++) {
				d.palette[c] = colorTable[d.palette[c] % 0x8000] | BIT(15);
			}
		} else {
			for (int c = 0; c < 256; c++) {
				d.palette[c] |= BIT(15);
			}
		}
		tonccpy(out + d.srcFrameBytes, d.palette, 0x200);
	} else {
		applyRotatingCubesColor((u16*)out, d.dstFrameBytes / 2, header.bmpMode);
	}
	return true;
}

// Decode a v3-v5 RVID into rotatingCubesLocation.
// Dual screen videos store two sub-frames per frame, interleaved top then bottom.
static bool loadRotatingCubesLatest(FILE *videoFrameFile, const RvidHeader &header) {
	const bool dualScreen = (header.dualScreen == 1);
	const bool interlaced = (header.interlaced != 0);
	const u32 screens = dualScreen ? 2 : 1;
	const u32 frameCount = (u32)rocketVideo_videoFrames + 1;
	const u32 entries = frameCount * screens;
	const u32 dstFrameBytes = (header.bmpMode == 0) ? (0x100 * header.vRes) + 0x200 : 0x200 * header.vRes;

	// A band that runs past the bottom of the screen would spill into the rows the
	// main engine shares with the 8 BPP text layer, so check it before anything else.
	const int bandHeight = header.vRes * (interlaced ? 2 : 1);
	const int yTop = tc().rotatingCubesRenderY();
	const int yBottom = tc().rotatingCubesRenderYBottom();
	const bool onBottomOnly = (!dualScreen && tc().rotatingCubesMainScreen() == 1);
	const bool onTop = !onBottomOnly;
	const bool onBottom = (dualScreen || onBottomOnly);
	if (bandHeight > SCREEN_HEIGHT
	 || (onTop && (yTop < 0 || yTop + bandHeight > SCREEN_HEIGHT))
	 || (onBottom && (yBottom < 0 || yBottom + bandHeight > SCREEN_HEIGHT))) {
		return false;
	}

	RvidDecoder decoder;
	if (!openRvidDecoder(decoder, videoFrameFile, header, entries)) {
		return false;
	}

	if ((u64)dstFrameBytes * entries > rotatingCubesBufferSize) {
		logPrint("RVID: too large to load\n");
		closeRvidDecoder(decoder);
		return false;
	}

	AlphaSpanWriter spans;
	initAlphaSpans(spans, entries * dstFrameBytes, entries, header.vRes);
	spans.ok = spans.ok && (header.bmpMode == 1);

	// Decode straight into the video buffer when it is main RAM. The Slot-2 RAM pak cannot
	// take the byte-wide writes, so frames bound for it go through main RAM first.
	const bool direct = (rotatingCubesLocation != (u8*)0x09000000);
	bool ok = true;
	for (u32 i = 0; i < entries && ok; i++) {
		u8 *dst = rotatingCubesLocation + (i * dstFrameBytes);
		u8 *out = direct ? dst : decoder.frameScratch;
		ok = decodeRvidSubFrame(decoder, videoFrameFile, i, out);
		if (!ok) break;
		if (header.bmpMode == 1) {
			addAlphaSpans(spans, i, (u16*)out, header.vRes);
		}
		if (!direct) {
			tonccpy(dst, decoder.frameScratch, dstFrameBytes);
		}
		DC_FlushRange(dst, dstFrameBytes);
	}
	closeRvidDecoder(decoder);
	if (!ok) return false;
	finishAlphaSpans(spans);

	rocketVideo_indexed = (header.bmpMode == 0);
	rocketVideo_dualScreen = dualScreen;
	rocketVideo_onTop = onTop;
	rocketVideo_onBottom = onBottom;
	rocketVideo_interlaced = interlaced;
	rocketVideo_bandHeight = bandHeight;
	rocketVideo_videoYpos = yTop;
	rocketVideo_videoYposBottom = yBottom;
	return true;
}

// Decode a pre-v3 RVID: frames laid end to end, 16 BPP single screen only.
// v1 and v2 share the first fields; only v2 carries the compression flag and a frame offset.
static bool loadRotatingCubesLegacy(FILE *videoFrameFile, const RvidHeader &header, u32 framesSize) {
	if (rocketVideo_height > 144 || framesSize > rotatingCubesBufferSize) {
		return false;
	}
	const bool onBottom = (tc().rotatingCubesMainScreen() == 1);
	const int yBottom = tc().rotatingCubesRenderYBottom();
	if (onBottom && (yBottom < 0 || yBottom + rocketVideo_height > SCREEN_HEIGHT)) {
		return false;
	}

	u32 framesOffset = RVID_FRAME_TABLE_OFFSET;
	if (header.ver == 2) {
		// v2 reuses the bytes this struct calls audioBitMode/bmpMode as a u16 "framesCompressed",
		// and the ones it calls compressedFrameSizeTableOffset as the offset of the first frame.
		const u16 framesCompressed = (u16)header.audioBitMode | ((u16)header.bmpMode << 8);
		if (framesCompressed || header.interlaced) {
			return false; // Never supported here, and loading it raw would show garbage
		}
		framesOffset = header.compressedFrameSizeTableOffset;
	}

	fseek(videoFrameFile, framesOffset, SEEK_SET);
	if (fread(rotatingCubesLocation, 1, framesSize, videoFrameFile) != framesSize) {
		return false;
	}

	applyRotatingCubesColor((u16*)rotatingCubesLocation, framesSize / 2, 1);
	DC_FlushRange(rotatingCubesLocation, framesSize);

	const u32 frameBytes = 0x200 * rocketVideo_height;
	const u32 entries = framesSize / frameBytes;
	AlphaSpanWriter spans;
	initAlphaSpans(spans, framesSize, entries, rocketVideo_height);
	for (u32 i = 0; i < entries; i++) {
		addAlphaSpans(spans, i, (u16*)(rotatingCubesLocation + (i * frameBytes)), rocketVideo_height);
	}
	finishAlphaSpans(spans);
	rocketVideo_indexed = false;
	rocketVideo_dualScreen = false;
	rocketVideo_onTop = !onBottom;
	rocketVideo_onBottom = onBottom;
	rocketVideo_interlaced = false;
	rocketVideo_bandHeight = rocketVideo_height;
	rocketVideo_videoYpos = tc().rotatingCubesRenderY();
	rocketVideo_videoYposBottom = yBottom;
	return true;
}

// Largest block the heap can hand out right now, to 64KB.
static u32 largestFreeBlock(void) {
	u32 lo = 0;
	u32 hi = 0x1000000 / 0x10000;
	while (lo < hi) {
		const u32 mid = (lo + hi + 1) / 2;
		u8 *block = new (std::nothrow) u8[mid * 0x10000];
		if (block) {
			delete[] block;
			lo = mid;
		} else {
			hi = mid - 1;
		}
	}
	return lo * 0x10000;
}

// Video buffer bytes a video needs: its stored frames, and for videos that may have alpha, the
// span index plus room for 16 run flips per row on average (noisier rows are blitted per pixel).
// 0 for a video this loader does not play.
static u32 rotatingCubesBytesNeeded(const RvidHeader &header) {
	u64 frameBytes = 0;
	u64 entries = header.frames;
	bool spans = false;
	if (header.ver >= 3 && header.ver <= 5 && header.dualScreen != 2) {
		entries *= (header.dualScreen == 1) ? 2 : 1;
		frameBytes = entries * ((header.bmpMode == 0) ? (0x100 * header.vRes) + 0x200 : 0x200 * header.vRes);
		spans = (header.bmpMode == 1);
	} else if (header.ver == 1 || header.ver == 2) {
		frameBytes = entries * 0x200 * header.vRes;
		spans = true;
	} else {
		return 0;
	}
	u64 total = (frameBytes + 3) & ~3ULL;
	if (spans) {
		total += (entries * 4) + (entries * header.vRes * 2 * 17);
	}
	total = (total + 3) & ~3ULL;
	return (total > 0x1000000) ? 0x1000000 : (u32)total;
}

void loadRotatingCubes() {
	std::string cubes = TFN_RVID_CUBES;
	FILE *videoFrameFile = fopen(cubes.c_str(), "rb");
	if (!videoFrameFile) return;

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

	if (!doRead) {
		fclose(videoFrameFile);
		return;
	}

	// Compatible with RVID v2-v5
	RvidHeader header = {0};
	if (fread(&header, 1, sizeof(header), videoFrameFile) != sizeof(header)
	 || header.magic != RVID_MAGIC || header.frames == 0 || header.vRes == 0) {
		fclose(videoFrameFile);
		return;
	}

	rocketVideo_videoFrames = (int)header.frames - 1;

	// On a DSi the buffer is sized to the video, up to what the heap can spare
	rotatingCubesBufferSize = ROTATING_CUBES_MAX_SIZE;
	if (dsiFeatures() && ms().consoleModel == 0) {
		const u32 needed = rotatingCubesBytesNeeded(header);
		const u32 largest = largestFreeBlock();
		const u32 limit = (largest > VIDEO_RAM_RESERVE) ? largest - VIDEO_RAM_RESERVE : 0;
		logPrint("RVID: needs %lu KB, %lu KB available\n", (unsigned long)(needed >> 10), (unsigned long)(limit >> 10));
		rotatingCubesLocation = (needed && needed <= limit) ? new (std::nothrow) u8[needed] : NULL;
		if (!rotatingCubesLocation) {
			if (needed) logPrint("RVID: too large to load\n");
			fclose(videoFrameFile);
			return;
		}
		rotatingCubesBufferSize = needed;
		rotatingCubesBufferAllocated = true;
	}

	rocketVideo_loopFrame = tc().rotatingCubesLoopFrame();
	if (rocketVideo_loopFrame < 0 || rocketVideo_loopFrame > rocketVideo_videoFrames) {
		rocketVideo_loopFrame = 0;
	}

	rocketVideo_fps = header.fps;
	if (rocketVideo_fps >= 0x80) {
		rocketVideo_fps -= 0x80;
	} else if (rocketVideo_fps == 0) {
		rocketVideo_fps = 60;
	}

	rocketVideo_height = header.vRes;

	const bool latestVer = (header.ver >= 3 && header.ver <= 5);
	bool loaded = false;
	if (latestVer) {
		if (header.dualScreen != 2) { // 2 means the video targets a GBA
			loaded = loadRotatingCubesLatest(videoFrameFile, header);
		}
	} else if (header.ver == 1 || header.ver == 2) {
		loaded = loadRotatingCubesLegacy(videoFrameFile, header,
			(0x200 * rocketVideo_height) * ((u32)rocketVideo_videoFrames + 1));
	}

	if (loaded) {
		rocketVideo_currentFrame = 0;
		rocketVideo_prevFrame = -1;
		rocketVideo_field = false;
		resetVideoAlphaTracking();
		rotatingCubesLoaded = true;
		rocketVideo_playVideo = true;
		rocketVideo_topVisible = true;
		rocketVideo_weaveRefill = true;
	} else if (rotatingCubesBufferAllocated) {
		delete[] rotatingCubesLocation;
		rotatingCubesLocation = NULL;
		rotatingCubesBufferAllocated = false;
	}
	fclose(videoFrameFile);
}
void ThemeTextures::unloadRotatingCubes() {
	rocketVideo_playVideo = false;
	while (dmaBusy(0) || dmaBusy(1)); // Wait for any in-flight frame to finish rendering
	drawOverRotatingCubesBottom(); // Restore the bottom screen behind the icons
	rotatingCubesLoaded = false;
	if (rotatingCubesBufferAllocated) {
		toncset32(rotatingCubesLocation, 0, rotatingCubesBufferSize/sizeof(u32)); // Clear video before freeing
		delete[] rotatingCubesLocation;
		rotatingCubesLocation = NULL;
		rotatingCubesBufferAllocated = false;
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
			// The video buffer is allocated by loadRotatingCubes(), once the video's size is known
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