#pragma once
#ifndef __DSIMENUPP_THEME_TEXTURES__
#define __DSIMENUPP_THEME_TEXTURES__
#include "common/gl2d.h"
#include "common/singleton.h"
#include <memory>
#include <string>

using std::unique_ptr;

class ThemeTextures
{

public:
  ThemeTextures()
  {
    bubbleTexID = 0;
    bipsTexID = 0;
    scrollwindowTexID = 0;
    buttonarrowTexID = 0;
    movingarrowTexID = 0;
    launchdotTexID = 0;
    startTexID = 0;
    startbrdTexID = 0;
    settingsTexID = 0;
    braceTexID = 0;
    boxfullTexID = 0;
    boxemptyTexID = 0;
    folderTexID = 0;
    cornerButtonTexID = 0;
    smallCartTexID = 0;

    progressTexID = 0;
    dialogboxTexID = 0;
    wirelessiconTexID = 0;
  }
  ~ThemeTextures() {}

public:
  void loadDSiDarkTheme();
  void loadDSiWhiteTheme();
  void loadDSiRedTheme();
  void loadDSiBlueTheme();
  void loadDSiGreenTheme();
  void loadDSiYellowTheme();
  void loadDSiPinkTheme();
  void loadDSiPurpleTheme();
  void load3DSTheme();

  void reloadPalDialogBox();

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
  void loadCommonTextures();

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

  void drawBubbleBg();
  void drawBg();

  std::string shoulderLPath;
  std::string shoulderLGreyPath;
  std::string shoulderRPath;
  std::string shoulderRGreyPath;
  std::string topBgPath;
  std::string bottomBgPath;
  std::string bottomBubbleBgPath;

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

  // unique_ptr<glImage[1]> bubbleImage[1];
  // glImage progressImage[(16 / 16) * (128 / 16)];
  // glImage dialogboxImage[(256 / 16) * (256 / 16)];
  // glImage bipsImage[(8 / 8) * (32 / 8)];
  // glImage scrollwindowImage[(32 / 16) * (32 / 16)];
  // glImage buttonarrowImage[(32 / 32) * (64 / 32)];
  // glImage launchdotImage[(16 / 16) * (96 / 16)];
  // glImage startImage[(64 / 16) * (128 / 16)];
  // glImage startbrdImage[(32 / 32) * (256 / 80)];

  // glImage _3dsstartbrdImage[(32 / 32) * (192 / 64)];
  // glImage braceImage[(16 / 16) * (128 / 16)];
  // glImage settingsImage[(64 / 16) * (128 / 64)];
  // glImage boxfullImage[(64 / 16) * (128 / 64)];
  // glImage boxemptyImage[(64 / 16) * (64 / 16)];
  // glImage folderImage[(64 / 16) * (64 / 16)];
  // glImage wirelessIcons[(32 / 32) * (64 / 32)];
};

typedef singleton<ThemeTextures> themeTextures_s;
inline ThemeTextures &tex() { return themeTextures_s::instance(); }

#endif