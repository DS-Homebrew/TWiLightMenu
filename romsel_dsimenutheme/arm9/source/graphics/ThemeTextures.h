#pragma once
#ifndef __DSIMENUPP_THEME_TEXTURES__
#define __DSIMENUPP_THEME_TEXTURES__
#include "common/gl2d.h"
#include "common/singleton.h"
#include "GritTexture.h"
#include "BmpTexture.h"
#include <memory>
#include <string>


#define BG_BUFFER_PIXELCOUNT 256 * 192

using std::unique_ptr;

class ThemeTextures
{

public:
  ThemeTextures()
    : bubbleTexID(0), bipsTexID(0), scrollwindowTexID(0), buttonarrowTexID(0),
      movingarrowTexID(0), launchdotTexID(0), startTexID(0), startbrdTexID(0),
      settingsTexID(0), braceTexID(0), boxfullTexID(0), boxemptyTexID(0),
      folderTexID(0), cornerButtonTexID(0), smallCartTexID(0), progressTexID(0),
      dialogboxTexID(0), wirelessiconTexID(0),
      _cachedVolumeLevel(-1), _cachedBatteryLevel(-1)
  {
    _bgSubBuffer = std::make_unique<u16[]>(BG_BUFFER_PIXELCOUNT);
    _bmpImageBuffer = std::make_unique<u16[]>(BG_BUFFER_PIXELCOUNT);
    _bottomBgImage = std::make_unique<u16[]>(BG_BUFFER_PIXELCOUNT);
    _bottomBubbleBgImage = std::make_unique<u16[]>(BG_BUFFER_PIXELCOUNT);
    _bottomMovingBgImage = std::make_unique<u16[]>(BG_BUFFER_PIXELCOUNT);

  }

  ~ThemeTextures() {}

public:
  void loadDSiTheme();
  void load3DSTheme();

  void loadVolumeTextures();
  void loadBatteryTextures();
  void loadUiTextures();

  void reloadPalDialogBox();
  void reloadPal3dsCornerButton();

  static unsigned short convertToDsBmp(unsigned short val);
public:
  unsigned short *beginSubModify();
  void commitSubModify();  
  void commitSubModifyAsync();

  void drawTopBg();
  void drawTopBgAvoidingShoulders();

  void drawProfileName();
  void drawBottomBubbleBg();
  void drawBottomMovingBg();
  void drawBottomBg();

  void drawBoxArt(const char* filename);

  void drawVolumeImage(int volumeLevel);
  void drawVolumeImageCached();

  void drawBatteryImage(int batteryLevel, bool drawDSiMode, bool isRegularDS);
  void drawBatteryImageCached();

  void drawShoulders(bool showLShoulder, bool showRshoulder) ;
  void drawDateTime(const char* date, const int posX, const int posY, const int drawCount, int *hourWidthPointer);

  void clearTopScreen();

private:
  void loadBubbleImage(const unsigned short *palette, const unsigned int *bitmap, int sprW, int sprH, int texW);
  void loadProgressImage(const unsigned short *palette, const unsigned int *bitmap);
  void loadDialogboxImage(const unsigned short *palette, const unsigned int *bitmap);
  void loadBipsImage(const unsigned short *palette, const unsigned int *bitmap);
  void loadScrollwindowImage(const unsigned short *palette, const unsigned int *bitmap);
  void loadButtonarrowImage(const unsigned short *palette, const unsigned int *bitmap);
  void loadMovingarrowImage(const unsigned short *palette, const unsigned int *bitmap);
  void loadLaunchdotImage(const unsigned short *palette, const unsigned int *bitmap);
  void loadStartImage(const unsigned short *palette, const unsigned int *bitmap);
  void loadStartbrdImage(const unsigned short *palette, const unsigned int *bitmap, int arraysize, int palLength,
                         int sprH, int texH);
  void loadBraceImage(const unsigned short *palette, const unsigned int *bitmap);
  void loadSettingsImage(const unsigned short *palette, const unsigned int *bitmap);
  void loadBoxfullImage(const unsigned short *palette, const unsigned int *bitmap);
  void loadBoxemptyImage(const unsigned short *palette, const unsigned int *bitmap);
  void loadFolderImage(const unsigned short *palette, const unsigned int *bitmap);
  void loadCornerButtonImage(const unsigned short *palette, const unsigned int *bitmap, int arraysize,
											int sprW, int sprH, int texW, int texH);
  void loadSmallCartImage(const unsigned short *palette, const unsigned int *bitmap);
  void loadWirelessIcons(const unsigned short *palette, const unsigned int *bitmap);

  void loadBottomImage();
  void setStringPaths(const std::string theme);

  void loadDateFont(const unsigned short *bitmap);

  static unsigned int getTopFontSpriteIndex(const u16 character);
  static unsigned int getDateTimeFontSpriteIndex(const u16 character);

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
  unique_ptr<glImage[]> loadTexture(int *textureId, const unsigned short *palette, const unsigned int *bitmap,
                                    unsigned int arraySize, int paletteLength, int sprW, int sprH, int texW, int texH);

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
  const glImage *boxfullImage() { return _boxfullImage.get(); }
  const glImage *boxemptyImage() { return _boxemptyImage.get(); }
  const glImage *folderImage() { return _folderImage.get(); }
  const glImage *cornerButtonImage() { return _cornerButtonImage.get(); }
  const glImage *smallCartImage() { return _smallCartImage.get(); }
  const glImage *wirelessIcons() { return _wirelessIcons.get(); }

  const BmpTexture *topBackgroundTexture() { return _topBackgroundTexture.get(); }
  const BmpTexture *bottomBackgroundTexture() { return _bottomBackgroundTexture.get(); }
  const BmpTexture *bottomBackgroundBubbleTexture() { return _bottomBackgroundBubbleTexture.get(); }
  const BmpTexture *dateTimeFontTexture() { return _dateTimeFontTexture.get(); }
  const BmpTexture *leftShoulderTexture() { return _leftShoulderTexture.get(); }
  const BmpTexture *rightShoulderTexture() { return _rightShoulderTexture.get(); }
  const BmpTexture *leftShoulderGreyedTexture() { return _leftShoulderGreyedTexture.get(); }
  const BmpTexture *rightShoulderGreyedTexture() { return _rightShoulderGreyedTexture.get(); }

  const BmpTexture *volumeTexture(int texture) { 
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

  
  const BmpTexture *batteryTexture(int texture, bool dsiMode, bool regularDS) { 
    if (dsiMode) {
      switch(texture) {
        case 7:
          return _batterychargeTexture.get();
        case 4:
          return _battery4Texture.get();
        case 3:
          return _battery3Texture.get();
        case 2:
          return _battery2Texture.get();
        case 1:
          return _battery1Texture.get();
        case 0:
        default:
          return _battery1Texture.get();
      }
    } else {
      switch (texture)
      {
        case 1:
          return _batterylowTexture.get();
        case 0:
        default:
          return regularDS ? _batteryfullDSTexture.get() : _batteryfullTexture.get();
      }
    }
  }

private:
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
  unique_ptr<glImage[]> _boxfullImage;
  unique_ptr<glImage[]> _boxemptyImage;
  unique_ptr<glImage[]> _folderImage;
  unique_ptr<glImage[]> _cornerButtonImage;
  unique_ptr<glImage[]> _smallCartImage;
  unique_ptr<glImage[]> _wirelessIcons;
  unique_ptr<glImage[]> _bubbleImage;

  unique_ptr<GritTexture> _bipsTexture;
  unique_ptr<GritTexture> _boxTexture;
  unique_ptr<GritTexture> _braceTexture;
  unique_ptr<GritTexture> _bubbleTexture;
  unique_ptr<GritTexture> _buttonArrowTexture;
  unique_ptr<GritTexture> _cornerButtonTexture;
  unique_ptr<GritTexture> _dialogBoxTexture;
  unique_ptr<GritTexture> _folderTexture;
  unique_ptr<GritTexture> _launchDotTexture;
  unique_ptr<GritTexture> _movingArrowTexture;
  unique_ptr<GritTexture> _progressTexture;
  unique_ptr<GritTexture> _scrollWindowTexture;
  unique_ptr<GritTexture> _smallCartTexture;
  unique_ptr<GritTexture> _startBorderTexture;
  unique_ptr<GritTexture> _startTextTexture;
  unique_ptr<GritTexture> _wirelessIconsTexture;
  unique_ptr<GritTexture> _settingsIconTexture;

  unique_ptr<GritTexture> _boxFullTexture;
  unique_ptr<GritTexture> _boxEmptyTexture;

  unique_ptr<BmpTexture> _volume0Texture;
  unique_ptr<BmpTexture> _volume1Texture;
  unique_ptr<BmpTexture> _volume2Texture;
  unique_ptr<BmpTexture> _volume3Texture;
  unique_ptr<BmpTexture> _volume4Texture;

  unique_ptr<BmpTexture> _battery1Texture;
  unique_ptr<BmpTexture> _battery2Texture;
  unique_ptr<BmpTexture> _battery3Texture;
  unique_ptr<BmpTexture> _battery4Texture;
  unique_ptr<BmpTexture> _batterychargeTexture;
  unique_ptr<BmpTexture> _batteryfullTexture;
  unique_ptr<BmpTexture> _batteryfullDSTexture;
  unique_ptr<BmpTexture> _batterylowTexture;

  unique_ptr<BmpTexture> _topBackgroundTexture;
  unique_ptr<BmpTexture> _bottomBackgroundTexture;
  unique_ptr<BmpTexture> _bottomBackgroundBubbleTexture;
  unique_ptr<BmpTexture> _bottomBackgroundMovingTexture;

  unique_ptr<BmpTexture> _dateTimeFontTexture;
  unique_ptr<BmpTexture> _leftShoulderTexture;
  unique_ptr<BmpTexture> _rightShoulderTexture;
  unique_ptr<BmpTexture> _leftShoulderGreyedTexture;
  unique_ptr<BmpTexture> _rightShoulderGreyedTexture;

  unique_ptr<u16[]> _bgSubBuffer;
  unique_ptr<u16[]> _bmpImageBuffer;

  unique_ptr<u16[]> _bottomBgImage;
  unique_ptr<u16[]> _bottomBubbleBgImage;
  unique_ptr<u16[]> _bottomMovingBgImage;

  unique_ptr<u16[]> _dateFontImage;

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
};


//   xrrrrrgggggbbbbb according to http://problemkaputt.de/gbatek.htm#dsvideobgmodescontrol
#define MASK_RB 0b0111110000011111
#define MASK_G 0b0000001111100000
#define MASK_MUL_RB 0b0111110000011111000000
#define MASK_MUL_G 0b0000001111100000000000
#define MAX_ALPHA 64 // 6bits+1 with rounding

/**
 * Adapted from https://stackoverflow.com/questions/18937701/
 * applies alphablending with the given
 * RGB555 foreground, RGB555 background, and alpha from
 * 0 to 128 (0, 1.0).
 * The lower the alpha the more transparent, but
 * this function does not produce good results at the extremes
 * (near 0 or 128).
 */
inline u16 alphablend(u16 fg, u16 bg, u8 alpha) {

	// alpha for foreground multiplication
	// convert from 8bit to (6bit+1) with rounding
	// will be in [0..64] inclusive
	alpha = (alpha + 2) >> 2;
	// "beta" for background multiplication; (6bit+1);
	// will be in [0..64] inclusive
	u8 beta = MAX_ALPHA - alpha;
	// so (0..64)*alpha + (0..64)*beta always in 0..64

	return (u16)((((alpha * (u32)(fg & MASK_RB) + beta * (u32)(bg & MASK_RB)) & MASK_MUL_RB) |
		      ((alpha * (fg & MASK_G) + beta * (bg & MASK_G)) & MASK_MUL_G)) >>
		     5);
}


typedef singleton<ThemeTextures> themeTextures_s;
inline ThemeTextures &tex() { return themeTextures_s::instance(); }

#endif