#pragma once
#ifndef __DSIMENUPP_THEME_TEXTURES__
#define __DSIMENUPP_THEME_TEXTURES__
#include <gl2d.h>
#include "common/singleton.h"
#include "FontGraphic.h"
#include "Texture.h"
#include <memory>
#include <string>
#include <algorithm>
#include <vector>

#define BG_BUFFER_PIXELCOUNT 256 * 192

extern bool boxArtColorDeband;

extern int boxArtType[40];

using std::unique_ptr;
using std::min;
using std::vector;

class ThemeTextures
{

public:
	ThemeTextures();
	virtual ~ThemeTextures() = default;

public:
	void loadHBTheme();
	void loadSaturnTheme();
	void loadDSiTheme();
	void load3DSTheme();

	void reloadPalDialogBox();

private:
	void loadVolumeTextures();
	void loadBatteryTextures();
	void loadUITextures();
	void loadIconTextures();

public:
	static unsigned short *beginBgSubModify();
	static void commitBgSubModify();
	static void commitBgSubModifyAsync();

	static unsigned short *beginBgMainModify();
	static void commitBgMainModify();
	static void commitBgMainModifyAsync();

	void drawTopBg();
	void drawTopBgAvoidingShoulders();

	void drawProfileName();
	void resetProfileName();
	void drawBottomBg(int bg);

	void loadBoxArtToMem(const char *filename, int num);
	void drawBoxArt(const char* filename);
	void drawBoxArtFromMem(int num);

	void drawVolumeImage(int volumeLevel);
	void drawVolumeImageMacro(int volumeLevel);
	void drawVolumeImageCached();
	void resetCachedVolumeLevel();

	void drawBatteryImage(int batteryLevel, bool drawDSiMode, bool isRegularDS);
	void drawBatteryImageMacro(int batteryLevel, bool drawDSiMode, bool isRegularDS);
	void drawBatteryImageCached();
	void resetCachedBatteryLevel();

	void drawShoulders(bool LShoulderActive, bool RShoulderActive);
	void drawDateTime(const char* date, int posX, int posY, bool isDate);
	void drawDateTimeMacro(const char* date, int posX, int posY, bool isDate);

	void clearTopScreen();
	static void videoSetup();
private:
	void applyUserPaletteToAllGrfTextures();
	void applyGrayscaleToAllGrfTextures();

	void loadBubbleImage(const Texture& tex, int sprW, int sprH);
	void loadProgressImage(const Texture& tex);
	void loadDialogboxImage(const Texture& tex);
	void loadBipsImage(const Texture& tex);
	void loadScrollwindowImage(const Texture& tex);
	void loadButtonarrowImage(const Texture& tex);
	void loadMovingarrowImage(const Texture& tex);
	void loadLaunchdotImage(const Texture& tex);
	void loadStartImage(const Texture& tex);
	void loadStartbrdImage(const Texture& tex, int sprH);
	void loadBraceImage(const Texture& tex);
	void loadSettingsImage(const Texture& tex);
	void loadManualImage(const Texture& tex);
	void loadBoxfullImage(const Texture& tex);
	void loadBoxemptyImage(const Texture& tex);
	void loadFolderImage(const Texture& tex);
	void loadCornerButtonImage(const Texture& tex, int arraysize,
											int sprW, int sprH);
	void loadSmallCartImage(const Texture& tex);
	void loadWirelessIcons(const Texture& tex);

	void loadBackgrounds();

	static int getVolumeLevel();
	static int getBatteryLevel();

private:

	/**
	 * Allocates space for and loads a glTexture into memory, returning a
	 * unique_ptr to the glImage array, and sets textureId to the ID of the
	 * loaded texture.
	 *
	 * arraySize is the size of the glImage array.
	 */
	unique_ptr<glImage[]> loadTexture(
						int *textureId, const Texture& texture,
						unsigned int arraySize,
						int sprW, int sprH, GL_TEXTURE_TYPE_ENUM texType);

public:
	const glImage *bubbleImage() { return _bubbleImage.get(); }
	const glImage *progressImage() { return _progressImage.get(); }
	const glImage *dialogboxImage() { return _dialogboxImage.get(); }
	const glImage *bipsImage() { return _bipsImage.get(); }
	const glImage *scrollwindowImage() { return _scrollwindowImage.get(); }
	const glImage *buttonarrowImage() { return _buttonarrowImage.get(); }
	const glImage *movingArrowImage() { return _movingarrowImage.get(); }
	const glImage *launchdotImage() { return _launchdotImage.get(); }
	const glImage *startImage() { return _startImage.get(); }
	const glImage *startbrdImage() { return _startbrdImage.get(); }
	const glImage *braceImage() { return _braceImage.get(); }
	const glImage *settingsImage() { return _settingsImage.get(); }
	const glImage *manualImage() { return _manualImage.get(); }
	const glImage *boxfullImage() { return _boxfullImage.get(); }
	const glImage *boxemptyImage() { return _boxemptyImage.get(); }
	const glImage *folderImage() { return _folderImage.get(); }
	const glImage *cornerButtonImage() { return _cornerButtonImage.get(); }
	const glImage *smallCartImage() { return _smallCartImage.get(); }
	const glImage *wirelessIcons() { return _wirelessIcons.get(); }


	const Texture *iconGBTexture() { return _iconGBTexture.get(); }
	const Texture *iconGBATexture() { return _iconGBATexture.get(); }
	const Texture *iconGBAModeTexture() { return _iconGBAModeTexture.get(); }
	const Texture *iconGGTexture() { return _iconGGTexture.get(); }
	const Texture *iconMDTexture() { return _iconMDTexture.get(); }
	const Texture *iconNESTexture() { return _iconNESTexture.get(); }
	const Texture *iconSGTexture() { return _iconSGTexture.get(); }
	const Texture *iconSMSTexture() { return _iconSMSTexture.get(); }
	const Texture *iconSNESTexture() { return _iconSNESTexture.get(); }
	const Texture *iconPLGTexture() { return _iconPLGTexture.get(); }
	const Texture *iconA26Texture() { return _iconA26Texture.get(); }
	const Texture *iconCOLTexture() { return _iconCOLTexture.get(); }
	const Texture *iconM5Texture() { return _iconM5Texture.get(); }
	const Texture *iconINTTexture() { return _iconINTTexture.get(); }
	const Texture *iconPCETexture() { return _iconPCETexture.get(); }
	const Texture *iconWSTexture() { return _iconWSTexture.get(); }
	const Texture *iconNGPTexture() { return _iconNGPTexture.get(); }
	const Texture *iconCPCTexture() { return _iconCPCTexture.get(); }
	const Texture *iconVIDTexture() { return _iconVIDTexture.get(); }
	const Texture *iconIMGTexture() { return _iconIMGTexture.get(); }
	const Texture *iconUnknownTexture() { return _iconUnknownTexture.get(); }

	const Texture *leftShoulderTexture() { return _leftShoulderTexture.get(); }
	const Texture *rightShoulderTexture() { return _rightShoulderTexture.get(); }
	const Texture *leftShoulderGreyedTexture() { return _leftShoulderGreyedTexture.get(); }
	const Texture *rightShoulderGreyedTexture() { return _rightShoulderGreyedTexture.get(); }

	FontGraphic *dateTimeFont() { return _dateTimeFont.get(); }
	FontGraphic *usernameFont() { extern FontGraphic *smallFont; return _usernameFont ? _usernameFont.get() : smallFont; }

	static u16* bmpImageBuffer();
	static u16* bgSubBuffer2();
	static u16* photoBuffer();
	static u16* photoBuffer2();
	static u16* frameBuffer(bool secondBuffer);
	static u16* frameBufferBot(bool secondBuffer);

	const Texture *volumeTexture(int texture) {
		switch(texture) {
			case 4:
				return _volume4Texture.get();
			case 3:
				return _volume3Texture.get();
			case 2:
				return _volume2Texture.get();
			case 1:
				return _volume1Texture.get();
			case 0:
			default:
				return _volume0Texture.get();
		}
	}


	const Texture *batteryTexture(int texture, bool dsiMode, bool regularDS) {
		if (dsiMode) {
			switch(texture) {
				case 8:
					return _batterychargeblinkTexture.get();
				case 7:
					return _batterychargeTexture.get();
				case 4:
				default:
					return _battery4Texture.get();
				case 3:
					return _battery3Texture.get();
				case 2:
					return _battery2Texture.get();
				case 1:
				case 0:
					return _battery1Texture.get();
				case -1:
					return _battery0Texture.get();
			}
		} else {
			switch (texture) {
				case 8:
					return _batterychargeblinkTexture.get();
				case 7:
					return _batterychargeTexture.get();
				default:
					return regularDS ? _batteryfullDSTexture.get() : _batteryfullTexture.get();
				case 1:
				case 0:
					return _batterylowTexture.get();
			}
		}
	}

private:
	vector<Texture> _backgroundTextures;

	unique_ptr<glImage[]> _progressImage;
	unique_ptr<glImage[]> _dialogboxImage;
	unique_ptr<glImage[]> _bipsImage;
	unique_ptr<glImage[]> _scrollwindowImage;
	unique_ptr<glImage[]> _buttonarrowImage;
	unique_ptr<glImage[]> _movingarrowImage;
	unique_ptr<glImage[]> _launchdotImage;
	unique_ptr<glImage[]> _startImage;
	unique_ptr<glImage[]> _startbrdImage;
	unique_ptr<glImage[]> _braceImage;
	unique_ptr<glImage[]> _settingsImage;
	unique_ptr<glImage[]> _manualImage;
	unique_ptr<glImage[]> _boxfullImage;
	unique_ptr<glImage[]> _boxemptyImage;
	unique_ptr<glImage[]> _folderImage;
	unique_ptr<glImage[]> _cornerButtonImage;
	unique_ptr<glImage[]> _smallCartImage;
	unique_ptr<glImage[]> _wirelessIcons;
	unique_ptr<glImage[]> _bubbleImage;

	unique_ptr<Texture> _bipsTexture;
	unique_ptr<Texture> _boxTexture;
	unique_ptr<Texture> _braceTexture;
	unique_ptr<Texture> _bubbleTexture;
	unique_ptr<Texture> _buttonArrowTexture;
	unique_ptr<Texture> _cornerButtonTexture;
	unique_ptr<Texture> _dialogBoxTexture;
	unique_ptr<Texture> _folderTexture;
	unique_ptr<Texture> _launchDotTexture;
	unique_ptr<Texture> _movingArrowTexture;
	unique_ptr<Texture> _progressTexture;
	unique_ptr<Texture> _scrollWindowTexture;
	unique_ptr<Texture> _smallCartTexture;
	unique_ptr<Texture> _startBorderTexture;
	unique_ptr<Texture> _startTextTexture;
	unique_ptr<Texture> _wirelessIconsTexture;
	unique_ptr<Texture> _settingsIconTexture;
	unique_ptr<Texture> _manualIconTexture;

	unique_ptr<Texture> _boxFullTexture;
	unique_ptr<Texture> _boxEmptyTexture;

	unique_ptr<Texture> _iconGBTexture;
	unique_ptr<Texture> _iconGBATexture;
	unique_ptr<Texture> _iconGBAModeTexture;
	unique_ptr<Texture> _iconGGTexture;
	unique_ptr<Texture> _iconMDTexture;
	unique_ptr<Texture> _iconNESTexture;
	unique_ptr<Texture> _iconSGTexture;
	unique_ptr<Texture> _iconSMSTexture;
	unique_ptr<Texture> _iconSNESTexture;
	unique_ptr<Texture> _iconPLGTexture;
	unique_ptr<Texture> _iconA26Texture;
	unique_ptr<Texture> _iconCOLTexture;
	unique_ptr<Texture> _iconM5Texture;
	unique_ptr<Texture> _iconINTTexture;
	unique_ptr<Texture> _iconPCETexture;
	unique_ptr<Texture> _iconWSTexture;
	unique_ptr<Texture> _iconNGPTexture;
	unique_ptr<Texture> _iconCPCTexture;
	unique_ptr<Texture> _iconVIDTexture;
	unique_ptr<Texture> _iconIMGTexture;
	unique_ptr<Texture> _iconUnknownTexture;

	unique_ptr<Texture> _volume0Texture;
	unique_ptr<Texture> _volume1Texture;
	unique_ptr<Texture> _volume2Texture;
	unique_ptr<Texture> _volume3Texture;
	unique_ptr<Texture> _volume4Texture;

	unique_ptr<Texture> _battery0Texture;
	unique_ptr<Texture> _battery1Texture;
	unique_ptr<Texture> _battery2Texture;
	unique_ptr<Texture> _battery3Texture;
	unique_ptr<Texture> _battery4Texture;
	unique_ptr<Texture> _batterychargeTexture;
	unique_ptr<Texture> _batterychargeblinkTexture;
	unique_ptr<Texture> _batteryfullTexture;
	unique_ptr<Texture> _batteryfullDSTexture;
	unique_ptr<Texture> _batterylowTexture;

	unique_ptr<Texture> _leftShoulderTexture;
	unique_ptr<Texture> _rightShoulderTexture;
	unique_ptr<Texture> _leftShoulderGreyedTexture;
	unique_ptr<Texture> _rightShoulderGreyedTexture;

	unique_ptr<FontGraphic> _dateTimeFont;
	unique_ptr<FontGraphic> _usernameFont;

private:
	int bubbleTexID;
	int bipsTexID;
	int scrollwindowTexID;
	int buttonarrowTexID;
	int movingarrowTexID;
	int launchdotTexID;
	int startTexID;
	int startbrdTexID;
	int settingsTexID;
	int manualTexID;
	int braceTexID;
	int boxfullTexID;
	int boxemptyTexID;
	int folderTexID;
	int cornerButtonTexID;
	int smallCartTexID;

	int progressTexID;
	int dialogboxTexID;
	int wirelessiconTexID;

private:
	int _cachedVolumeLevel;
	int _cachedBatteryLevel;
	bool _profileNameLoaded;
	int _previousDateWidth = 0;
	int _previousTimeWidth = 0;
};


//   xbbbbbgggggrrrrr according to http://problemkaputt.de/gbatek.htm#dsvideobgmodescontrol


typedef singleton<ThemeTextures> themeTextures_s;
inline ThemeTextures &tex() { return themeTextures_s::instance(); }

#endif