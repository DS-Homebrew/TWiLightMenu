#include "ThemeTextures.h"
#include <nds.h>

// Graphic files
#include "bottom.h"
#include "bottom_bubble.h"

//#include "org_bottom.h"
//#include "org_bottom_bubble.h"

#include "new_bottom.h"
#include "new_bottom_bubble.h"

#include "red_bottom.h"
#include "red_bottom_bubble.h"

#include "blue_bottom.h"
#include "blue_bottom_bubble.h"

#include "green_bottom.h"
#include "green_bottom_bubble.h"

#include "yellow_bottom.h"
#include "yellow_bottom_bubble.h"

#include "pink_bottom.h"
#include "pink_bottom_bubble.h"

#include "purple_bottom.h"
#include "purple_bottom_bubble.h"

#include "_3ds_bottom.h"
#include "_3ds_bottom_bubble.h"

#include "dialogbox.h"
#include "nintendo_dsi_menu.h"
#include "org_nintendo_dsi_menu.h"
#include "bubble.h"
#include "org_bubble.h"
#include "red_bubble.h"
#include "blue_bubble.h"
#include "green_bubble.h"
#include "yellow_bubble.h"
#include "pink_bubble.h"
#include "purple_bubble.h"
#include "_3ds_bubble.h"
#include "progress.h"
#include "bips.h"
#include "scroll_window.h"
#include "org_scroll_window.h"
#include "red_scroll_window.h"
#include "blue_scroll_window.h"
#include "green_scroll_window.h"
#include "yellow_scroll_window.h"
#include "pink_scroll_window.h"
#include "purple_scroll_window.h"
//#include "button_arrow.h"
#include "new_button_arrow.h"
#include "launch_dot.h"
#include "start_text.h"
//#include "start_border.h"
#include "new_start_border.h"
#include "../include/startborderpal.h"
#include "_3ds_cursor.h"
#include "brace.h"
#include "org_brace.h"
#include "red_brace.h"
#include "blue_brace.h"
#include "green_brace.h"
#include "yellow_brace.h"
#include "pink_brace.h"
#include "purple_brace.h"
#include "icon_settings.h"
#include "org_icon_settings.h"
#include "box.h"
#include "org_box.h"
#include "red_box.h"
#include "blue_box.h"
#include "green_box.h"
#include "yellow_box.h"
#include "pink_box.h"
#include "purple_box.h"
#include "_3ds_box_full.h"
#include "_3ds_box_empty.h"
#include "folder.h"
#include "org_folder.h"
#include "_3ds_folder.h"
#include "cornerbutton.h"
#include "_3ds_cornerbutton.h"
#include "wirelessicons.h"

#include "stringtool.h"

void ThemeTextures::loadBubbleImage(const unsigned short *palette, const unsigned int *bitmap,
                                    int sprW, int sprH, int texW)
{
  _bubbleImage = std::move(loadTexture(&bubbleTexID, palette, bitmap, 1, 12, sprW, sprH, texW, 8));
}

void ThemeTextures::loadProgressImage(const unsigned short *palette, const unsigned int *bitmap)
{
  _progressImage = std::move(loadTexture(&progressTexID, palette, bitmap, (16 / 16) * (128 / 16), 9, 16, 16, 16, 128));
}

void ThemeTextures::loadDialogboxImage(const unsigned short *palette, const unsigned int *bitmap)
{
  _dialogboxImage = std::move(loadTexture(&dialogboxTexID, palette, bitmap, (256 / 16) * (256 / 16), 4, 16, 16, 256, 192));
}

void ThemeTextures::loadBipsImage(const unsigned short *palette, const unsigned int *bitmap)
{
  _bipsImage = std::move(loadTexture(&bipsTexID, palette, bitmap, (8 / 8) * (32 / 8), 16, 8, 8, 8, 32));
}

void ThemeTextures::loadScrollwindowImage(const unsigned short *palette, const unsigned int *bitmap)
{
  _scrollwindowImage = std::move(loadTexture(&scrollwindowTexID, palette, bitmap, (32 / 16) * (32 / 16), 16, 32, 32, 32, 32));
}

void ThemeTextures::loadButtonarrowImage(const unsigned short *palette, const unsigned int *bitmap)
{
  _buttonarrowImage = std::move(loadTexture(&buttonarrowTexID, palette, bitmap, (32 / 32) * (64 / 32), 16, 32, 32, 32, 64));
}

void ThemeTextures::loadLaunchdotImage(const unsigned short *palette, const unsigned int *bitmap)
{
  _launchdotImage = std::move(loadTexture(&launchdotTexID, palette, bitmap, (16 / 16) * (96 / 16), 16, 16, 16, 16, 96));
}

void ThemeTextures::loadStartImage(const unsigned short *palette, const unsigned int *bitmap)
{
  _startImage = std::move(loadTexture(&startTexID, palette, bitmap, (64 / 16) * (128 / 16), 4, 64, 16, 64, 128));
}

void ThemeTextures::loadStartbrdImage(const unsigned short *palette, const unsigned int *bitmap, int arraysize, int palLength,
                                      int sprH, int texH)
{
  _startbrdImage = std::move(loadTexture(&startbrdTexID, palette, bitmap, arraysize, palLength, 32, sprH, 32, texH));
}
void ThemeTextures::loadBraceImage(const unsigned short *palette, const unsigned int *bitmap)
{
  _braceImage = std::move(loadTexture(&braceTexID, palette, bitmap, (16 / 16) * (128 / 16), 4, 16, 128, 16, 128));
}

void ThemeTextures::loadSettingsImage(const unsigned short *palette, const unsigned int *bitmap)
{
  _settingsImage = std::move(loadTexture(&settingsTexID, palette, bitmap, (64 / 16) * (128 / 64), 16, 64, 64, 64, 128));
}

void ThemeTextures::loadBoxfullImage(const unsigned short *palette, const unsigned int *bitmap)
{
  _boxfullImage = std::move(loadTexture(&boxfullTexID, palette, bitmap, (64 / 16) * (128 / 64), 16, 64, 64, 64, 128));
}

void ThemeTextures::loadBoxemptyImage(const unsigned short *palette, const unsigned int *bitmap)
{
  _boxemptyImage = std::move(loadTexture(&boxemptyTexID, palette, bitmap, (64 / 16) * (64 / 16), 16, 64, 64, 64, 64));
}

void ThemeTextures::loadFolderImage(const unsigned short *palette, const unsigned int *bitmap)
{
  _folderImage = std::move(loadTexture(&folderTexID, palette, bitmap, (64 / 16) * (64 / 16), 16, 64, 64, 64, 64));
}

void ThemeTextures::loadCornerButtonImage(const unsigned short *palette, const unsigned int *bitmap, int arraysize,
											int sprW, int sprH, int texW, int texH)
{
  _cornerButtonImage = std::move(loadTexture(&cornerButtonTexID, palette, bitmap, arraysize, 16, sprW, sprH, texW, texH));
}

void ThemeTextures::loadWirelessIcons(const unsigned short *palette, const unsigned int *bitmap)
{
  _wirelessIcons = std::move(loadTexture(&wirelessiconTexID, palette, bitmap, (32 / 32) * (64 / 32), 7, 32, 32, 32, 64));
}

inline GL_TEXTURE_SIZE_ENUM get_tex_size(int texSize)
{
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

inline const unsigned short* apply_personal_theme(const unsigned short *palette)
{
  return palette + ((PersonalData->theme) * 16);
}

unique_ptr<glImage[]> ThemeTextures::loadTexture(int *textureId, const unsigned short *palette, const unsigned int *bitmap,
                                                 unsigned int arraySize, int paletteLength, int sprW, int sprH, int texW, int texH)
{

  // We need to delete the texture since the resource held by the unique pointer will be
  // immediately dropped when we assign it to the pointer.

  if (*textureId != 0)
  {
    nocashMessage("Existing texture found!?");
    glDeleteTextures(1, textureId);
  }

  // Do a heap allocation of arraySize glImage
  unique_ptr<glImage[]> texturePtr = std::make_unique<glImage[]>(arraySize);

  // Load the texture here.
  *textureId = glLoadTileSet(texturePtr.get(),                           // pointer to glImage array
                             sprW,                                       // sprite width
                             sprH,                                       // sprite height
                             texW,                                       // bitmap width
                             texH,                                       // bitmap height
                             GL_RGB16,                                   // texture type for glTexImage2D() in videoGL.h
                             get_tex_size(texW),                         // sizeX for glTexImage2D() in videoGL.h
                             get_tex_size(texH),                         // sizeY for glTexImage2D() in videoGL.h
                             TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
                             paletteLength,                              // Length of the palette to use (16 colors)
                             (u16 *)palette,                             // Load our 16 color tiles palette
                             (u8 *)bitmap                                // image data generated by GRIT
  );
  return texturePtr;
}

void ThemeTextures::reloadPalDialogBox()
{
  glBindTexture(0, dialogboxTexID);
  glColorSubTableEXT(0, 0, 4, 0, 0, (u16 *)dialogboxPal);
}
void ThemeTextures::drawBg(int bgId)
{
  dmaCopy(_bottomTiles, bgGetGfxPtr(bgId), _bottomTilesLen);
  dmaCopy(_bottomPalette, BG_PALETTE, _bottomPaletteLen);
  dmaCopy(_bottomMap, bgGetMapPtr(bgId), _bottomMapLen);
}

void ThemeTextures::drawBubbleBg(int bgId)
{
  dmaCopy(_bottom_bubbleTiles, bgGetGfxPtr(bgId), _bottom_bubbleTilesLen);
  dmaCopy(_bottom_bubblePalette, BG_PALETTE, _bottom_bubblePaletteLen);
  dmaCopy(_bottom_bubbleMap, bgGetMapPtr(bgId), _bottom_bubbleMapLen);
}

void ThemeTextures::loadCommonTextures()
{
  loadProgressImage(progressPal, progressBitmap);
  loadDialogboxImage(dialogboxPal, dialogboxBitmap);
  loadWirelessIcons(wirelessiconsPal, wirelessiconsBitmap);
}

void ThemeTextures::setStringPaths(const std::string theme)
{
  topBgPath = formatString("nitro:/graphics/%s_top.bmp", theme.c_str());
  shoulderRPath = formatString("nitro:/graphics/%s_Rshoulder.bmp", theme.c_str());
  shoulderRGreyPath = formatString("nitro:/graphics/%s_Rshoulder_greyed.bmp", theme.c_str());

  shoulderLPath = formatString("nitro:/graphics/%s_Lshoulder.bmp", theme.c_str());
  shoulderLGreyPath = formatString("nitro:/graphics/%s_Lshoulder_greyed.bmp", theme.c_str());
}

void ThemeTextures::load3DSTheme()
{

  setStringPaths("3ds");
  _bottomTiles = _3ds_bottomTiles;
  _bottomPalette = _3ds_bottomPal;
  _bottomMap = _3ds_bottomMap;

  _bottom_bubbleMap = _3ds_bottom_bubbleMap;
  _bottom_bubblePalette = _3ds_bottom_bubblePal;
  _bottom_bubbleTiles = _3ds_bottom_bubbleTiles;

  _bottomTilesLen = _3ds_bottomTilesLen;
  _bottomPaletteLen = _3ds_bottomPalLen;
  _bottomMapLen = _3ds_bottomMapLen;

  _bottom_bubbleMapLen = _3ds_bottom_bubbleMapLen;
  _bottom_bubblePaletteLen = _3ds_bottom_bubblePalLen;
  _bottom_bubbleTilesLen = _3ds_bottom_bubbleTilesLen;

  loadBubbleImage(_3ds_bubblePal, _3ds_bubbleBitmap, 7, 7, 8);
  loadSettingsImage(org_icon_settingsPal, org_icon_settingsBitmap);

  loadBoxfullImage(_3ds_box_fullPal, _3ds_box_fullBitmap);
  loadBoxemptyImage(_3ds_box_emptyPal, _3ds_box_emptyBitmap);
  loadFolderImage(_3ds_folderPal, _3ds_folderBitmap);

  loadCornerButtonImage(_3ds_cornerbuttonPal, _3ds_cornerbuttonBitmap, (64 / 16) * (64 / 32), 64, 32, 64, 64);

  loadStartbrdImage(_3ds_cursorPal, _3ds_cursorBitmap, (32 / 32) * (192 / 64), 6, 64, 192);

  // loadStartImage(apply_personal_theme(start_textPals), start_textBitmap);

  //loadBipsImage(bipsPal, bipsBitmap);
  //loadButtonarrowImage(apply_personal_theme(button_arrowPals), new_button_arrowBitmap);
  //loadLaunchdotImage(apply_personal_theme(button_arrowPals), launch_dotBitmap);

  loadCommonTextures();
}

void ThemeTextures::loadDSiDarkTheme()
{

  setStringPaths("dark");

  loadBubbleImage(bubblePal, bubbleBitmap, 11, 8, 16);
  loadScrollwindowImage(scroll_windowPal, scroll_windowBitmap);
  loadStartImage(apply_personal_theme(start_textPals), start_textBitmap);
  loadBipsImage(bipsPal, bipsBitmap);
  loadStartbrdImage(apply_personal_theme(start_borderPals), new_start_borderBitmap, (32 / 32) * (256 / 80), 16, 80, 256);
  loadButtonarrowImage(apply_personal_theme(button_arrowPals), new_button_arrowBitmap);
  loadLaunchdotImage(apply_personal_theme(button_arrowPals), launch_dotBitmap);
  loadWirelessIcons(wirelessiconsPal, wirelessiconsBitmap);
  loadSettingsImage(icon_settingsPal, icon_settingsBitmap);
  loadBraceImage(bracePal, braceBitmap);
  loadBoxfullImage(boxPal, boxBitmap);
  loadCornerButtonImage(cornerbuttonPal, cornerbuttonBitmap, (32 / 16) * (32 / 32), 32, 32, 32, 64);
  loadFolderImage(folderPal, folderBitmap);

  loadCommonTextures();

  _bottomTiles = bottomTiles;
  _bottomPalette = bottomPal;
  _bottomMap = bottomMap;

  _bottom_bubbleMap = bottom_bubbleMap;
  _bottom_bubblePalette = bottom_bubblePal;
  _bottom_bubbleTiles = bottom_bubbleTiles;

  _bottomTilesLen = bottomTilesLen;
  _bottomPaletteLen = bottomPalLen;
  _bottomMapLen = bottomMapLen;

  _bottom_bubbleMapLen = bottom_bubbleMapLen;
  _bottom_bubblePaletteLen = bottom_bubblePalLen;
  _bottom_bubbleTilesLen = bottom_bubbleTilesLen;
}

void ThemeTextures::loadDSiWhiteTheme()
{

  setStringPaths("org");

  _bottomTiles = new_bottomTiles;
  _bottomPalette = new_bottomPal;
  _bottomMap = new_bottomMap;

  _bottom_bubbleMap = new_bottom_bubbleMap;
  _bottom_bubblePalette = new_bottom_bubblePal;
  _bottom_bubbleTiles = new_bottom_bubbleTiles;

  _bottomTilesLen = new_bottomTilesLen;
  _bottomPaletteLen = new_bottomPalLen;
  _bottomMapLen = new_bottomMapLen;

  _bottom_bubbleMapLen = new_bottom_bubbleMapLen;
  _bottom_bubblePaletteLen = new_bottom_bubblePalLen;
  _bottom_bubbleTilesLen = new_bottom_bubbleTilesLen;

  loadBubbleImage(org_bubblePal, org_bubbleBitmap, 11, 8, 16);
  loadScrollwindowImage(org_scroll_windowPal, org_scroll_windowBitmap);
  loadSettingsImage(org_icon_settingsPal, org_icon_settingsBitmap);
  loadBraceImage(org_bracePal, org_braceBitmap);
  loadBoxfullImage(org_boxPal, org_boxBitmap);
  loadCornerButtonImage(cornerbuttonPal, cornerbuttonBitmap, (32 / 16) * (32 / 32), 32, 32, 32, 64);
  loadFolderImage(org_folderPal, org_folderBitmap);

  loadStartImage(apply_personal_theme(start_textPals), start_textBitmap);
  loadBipsImage(bipsPal, bipsBitmap);
  loadStartbrdImage(apply_personal_theme(start_borderPals), new_start_borderBitmap, (32 / 32) * (256 / 80), 16, 80, 256);
  loadButtonarrowImage(apply_personal_theme(button_arrowPals), new_button_arrowBitmap);
  loadLaunchdotImage(apply_personal_theme(button_arrowPals), launch_dotBitmap);

  loadCommonTextures();
}

void ThemeTextures::loadDSiRedTheme()
{

  setStringPaths("red");

  _bottomTiles = red_bottomTiles;
  _bottomPalette = red_bottomPal;
  _bottomMap = red_bottomMap;

  _bottom_bubbleMap = red_bottom_bubbleMap;
  _bottom_bubblePalette = red_bottom_bubblePal;
  _bottom_bubbleTiles = red_bottom_bubbleTiles;

  _bottomTilesLen = red_bottomTilesLen;
  _bottomPaletteLen = red_bottomPalLen;
  _bottomMapLen = red_bottomMapLen;

  _bottom_bubbleMapLen = red_bottom_bubbleMapLen;
  _bottom_bubblePaletteLen = red_bottom_bubblePalLen;
  _bottom_bubbleTilesLen = red_bottom_bubbleTilesLen;

  loadBubbleImage(red_bubblePal, red_bubbleBitmap, 11, 8, 16);
  loadScrollwindowImage(red_scroll_windowPal, red_scroll_windowBitmap);
  loadBraceImage(red_bracePal, red_braceBitmap);
  loadBoxfullImage(red_boxPal, red_boxBitmap);

  loadSettingsImage(org_icon_settingsPal, org_icon_settingsBitmap);
  loadCornerButtonImage(cornerbuttonPal, cornerbuttonBitmap, (32 / 16) * (32 / 32), 32, 32, 32, 64);
  loadFolderImage(org_folderPal, org_folderBitmap);

  loadStartImage(apply_personal_theme(start_textPals), start_textBitmap);
  loadBipsImage(bipsPal, bipsBitmap);
  loadStartbrdImage(apply_personal_theme(start_borderPals), new_start_borderBitmap, (32 / 32) * (256 / 80), 16, 80, 256);
  loadButtonarrowImage(apply_personal_theme(button_arrowPals), new_button_arrowBitmap);
  loadLaunchdotImage(apply_personal_theme(button_arrowPals), launch_dotBitmap);

  loadCommonTextures();
}

void ThemeTextures::loadDSiBlueTheme()
{

  setStringPaths("blue");

  _bottomTiles = blue_bottomTiles;
  _bottomPalette = blue_bottomPal;
  _bottomMap = blue_bottomMap;

  _bottom_bubbleMap = blue_bottom_bubbleMap;
  _bottom_bubblePalette = blue_bottom_bubblePal;
  _bottom_bubbleTiles = blue_bottom_bubbleTiles;

  _bottomTilesLen = blue_bottomTilesLen;
  _bottomPaletteLen = blue_bottomPalLen;
  _bottomMapLen = blue_bottomMapLen;

  _bottom_bubbleMapLen = blue_bottom_bubbleMapLen;
  _bottom_bubblePaletteLen = blue_bottom_bubblePalLen;
  _bottom_bubbleTilesLen = blue_bottom_bubbleTilesLen;

  loadBubbleImage(blue_bubblePal, blue_bubbleBitmap, 11, 8, 16);
  loadScrollwindowImage(blue_scroll_windowPal, blue_scroll_windowBitmap);
  loadBraceImage(blue_bracePal, blue_braceBitmap);
  loadBoxfullImage(blue_boxPal, blue_boxBitmap);

  loadSettingsImage(org_icon_settingsPal, org_icon_settingsBitmap);
  loadCornerButtonImage(cornerbuttonPal, cornerbuttonBitmap, (32 / 16) * (32 / 32), 32, 32, 32, 64);
  loadFolderImage(org_folderPal, org_folderBitmap);

  loadStartImage(apply_personal_theme(start_textPals), start_textBitmap);
  loadBipsImage(bipsPal, bipsBitmap);
  loadStartbrdImage(apply_personal_theme(start_borderPals), new_start_borderBitmap, (32 / 32) * (256 / 80), 16, 80, 256);
  loadButtonarrowImage(apply_personal_theme(button_arrowPals), new_button_arrowBitmap);
  loadLaunchdotImage(apply_personal_theme(button_arrowPals), launch_dotBitmap);

  loadCommonTextures();
}

void ThemeTextures::loadDSiGreenTheme()
{

  setStringPaths("green");

  _bottomTiles = green_bottomTiles;
  _bottomPalette = green_bottomPal;
  _bottomMap = green_bottomMap;

  _bottom_bubbleMap = green_bottom_bubbleMap;
  _bottom_bubblePalette = green_bottom_bubblePal;
  _bottom_bubbleTiles = green_bottom_bubbleTiles;

  _bottomTilesLen = green_bottomTilesLen;
  _bottomPaletteLen = green_bottomPalLen;
  _bottomMapLen = green_bottomMapLen;

  _bottom_bubbleMapLen = green_bottom_bubbleMapLen;
  _bottom_bubblePaletteLen = green_bottom_bubblePalLen;
  _bottom_bubbleTilesLen = green_bottom_bubbleTilesLen;

  loadBubbleImage(green_bubblePal, green_bubbleBitmap, 11, 8, 16);
  loadScrollwindowImage(green_scroll_windowPal, green_scroll_windowBitmap);
  loadBraceImage(green_bracePal, green_braceBitmap);
  loadBoxfullImage(green_boxPal, green_boxBitmap);

  loadSettingsImage(org_icon_settingsPal, org_icon_settingsBitmap);
  loadCornerButtonImage(cornerbuttonPal, cornerbuttonBitmap, (32 / 16) * (32 / 32), 32, 32, 32, 64);
  loadFolderImage(org_folderPal, org_folderBitmap);

  loadStartImage(apply_personal_theme(start_textPals), start_textBitmap);
  loadBipsImage(bipsPal, bipsBitmap);
  loadStartbrdImage(apply_personal_theme(start_borderPals), new_start_borderBitmap, (32 / 32) * (256 / 80), 16, 80, 256);

  loadButtonarrowImage(apply_personal_theme(button_arrowPals), new_button_arrowBitmap);
  loadStartbrdImage(apply_personal_theme(start_borderPals), new_start_borderBitmap, (32 / 32) * (256 / 80), 16, 80, 256);
  loadLaunchdotImage(apply_personal_theme(button_arrowPals), launch_dotBitmap);

  loadCommonTextures();
}

void ThemeTextures::loadDSiYellowTheme()
{
  setStringPaths("yellow");

  _bottomTiles = yellow_bottomTiles;
  _bottomPalette = yellow_bottomPal;
  _bottomMap = yellow_bottomMap;

  _bottom_bubbleMap = yellow_bottom_bubbleMap;
  _bottom_bubblePalette = yellow_bottom_bubblePal;
  _bottom_bubbleTiles = yellow_bottom_bubbleTiles;

  _bottomTilesLen = yellow_bottomTilesLen;
  _bottomPaletteLen = yellow_bottomPalLen;
  _bottomMapLen = yellow_bottomMapLen;

  _bottom_bubbleMapLen = yellow_bottom_bubbleMapLen;
  _bottom_bubblePaletteLen = yellow_bottom_bubblePalLen;
  _bottom_bubbleTilesLen = yellow_bottom_bubbleTilesLen;

  loadBubbleImage(yellow_bubblePal, yellow_bubbleBitmap, 11, 8, 16);
  loadScrollwindowImage(yellow_scroll_windowPal, yellow_scroll_windowBitmap);
  loadBraceImage(yellow_bracePal, yellow_braceBitmap);
  loadBoxfullImage(yellow_boxPal, yellow_boxBitmap);

  loadSettingsImage(org_icon_settingsPal, org_icon_settingsBitmap);
  loadCornerButtonImage(cornerbuttonPal, cornerbuttonBitmap, (32 / 16) * (32 / 32), 32, 32, 32, 64);
  loadFolderImage(org_folderPal, org_folderBitmap);

  loadStartImage(apply_personal_theme(start_textPals), start_textBitmap);
  loadBipsImage(bipsPal, bipsBitmap);
  loadStartbrdImage(apply_personal_theme(start_borderPals), new_start_borderBitmap, (32 / 32) * (256 / 80), 16, 80, 256);

  loadButtonarrowImage(apply_personal_theme(button_arrowPals), new_button_arrowBitmap);
  loadLaunchdotImage(apply_personal_theme(button_arrowPals), launch_dotBitmap);

  loadCommonTextures();
}

void ThemeTextures::loadDSiPurpleTheme()
{
  setStringPaths("purple");

  _bottomTiles = purple_bottomTiles;
  _bottomPalette = purple_bottomPal;
  _bottomMap = purple_bottomMap;

  _bottom_bubbleMap = purple_bottom_bubbleMap;
  _bottom_bubblePalette = purple_bottom_bubblePal;
  _bottom_bubbleTiles = purple_bottom_bubbleTiles;

  _bottomTilesLen = purple_bottomTilesLen;
  _bottomPaletteLen = purple_bottomPalLen;
  _bottomMapLen = purple_bottomMapLen;

  _bottom_bubbleMapLen = purple_bottom_bubbleMapLen;
  _bottom_bubblePaletteLen = purple_bottom_bubblePalLen;
  _bottom_bubbleTilesLen = purple_bottom_bubbleTilesLen;

  loadBubbleImage(purple_bubblePal, purple_bubbleBitmap, 11, 8, 16);
  loadScrollwindowImage(purple_scroll_windowPal, purple_scroll_windowBitmap);
  loadBraceImage(purple_bracePal, purple_braceBitmap);
  loadBoxfullImage(purple_boxPal, purple_boxBitmap);

  loadSettingsImage(org_icon_settingsPal, org_icon_settingsBitmap);
  loadCornerButtonImage(cornerbuttonPal, cornerbuttonBitmap, (32 / 16) * (32 / 32), 32, 32, 32, 64);
  loadFolderImage(org_folderPal, org_folderBitmap);

  loadStartImage(apply_personal_theme(start_textPals), start_textBitmap);
  loadBipsImage(bipsPal, bipsBitmap);
  loadStartbrdImage(apply_personal_theme(start_borderPals), new_start_borderBitmap, (32 / 32) * (256 / 80), 16, 80, 256);
  loadButtonarrowImage(apply_personal_theme(button_arrowPals), new_button_arrowBitmap);
  loadLaunchdotImage(apply_personal_theme(button_arrowPals), launch_dotBitmap);

  loadCommonTextures();
}

void ThemeTextures::loadDSiPinkTheme()
{
  setStringPaths("pink");

  _bottomTiles = pink_bottomTiles;
  _bottomPalette = pink_bottomPal;
  _bottomMap = pink_bottomMap;

  _bottom_bubbleMap = pink_bottom_bubbleMap;
  _bottom_bubblePalette = pink_bottom_bubblePal;
  _bottom_bubbleTiles = pink_bottom_bubbleTiles;

  _bottomTilesLen = pink_bottomTilesLen;
  _bottomPaletteLen = pink_bottomPalLen;
  _bottomMapLen = pink_bottomMapLen;

  _bottom_bubbleMapLen = pink_bottom_bubbleMapLen;
  _bottom_bubblePaletteLen = pink_bottom_bubblePalLen;
  _bottom_bubbleTilesLen = pink_bottom_bubbleTilesLen;

  loadBubbleImage(pink_bubblePal, pink_bubbleBitmap, 11, 8, 16);
  loadScrollwindowImage(pink_scroll_windowPal, pink_scroll_windowBitmap);
  loadBraceImage(pink_bracePal, pink_braceBitmap);
  loadBoxfullImage(pink_boxPal, pink_boxBitmap);

  loadSettingsImage(org_icon_settingsPal, org_icon_settingsBitmap);
  loadCornerButtonImage(cornerbuttonPal, cornerbuttonBitmap, (32 / 16) * (32 / 32), 32, 32, 32, 64);
  loadFolderImage(org_folderPal, org_folderBitmap);

  loadStartImage(apply_personal_theme(start_textPals), start_textBitmap);
  loadBipsImage(bipsPal, bipsBitmap);
  loadStartbrdImage(apply_personal_theme(start_borderPals), new_start_borderBitmap, (32 / 32) * (256 / 80), 16, 80, 256);
  loadButtonarrowImage(apply_personal_theme(button_arrowPals), new_button_arrowBitmap);
  loadLaunchdotImage(apply_personal_theme(button_arrowPals), launch_dotBitmap);

  loadCommonTextures();
}