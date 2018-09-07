#include "ThemeTextures.h"
#include <nds.h>

// Graphic files
#include "bottom.h"
#include "bottom_bubble.h"

#include "org_bottom.h"
#include "org_bottom_bubble.h"

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
#include "button_arrow.h"
#include "launch_dot.h"
#include "start_text.h"
#include "start_border.h"
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
#include "wirelessicons.h"

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
  _bipsImage = std::move(loadTexture(&bipsTexID, palette + ((PersonalData->theme) * 16), bitmap, (8 / 8) * (32 / 8), 16, 8, 8, 8, 32));
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
  _launchdotImage = std::move(loadTexture(&launchdotTexID, palette + ((PersonalData->theme) * 16), bitmap, (16 / 16) * (96 / 16), 16, 16, 16, 16, 96));
}

void ThemeTextures::loadStartImage(const unsigned short *palette, const unsigned int *bitmap)
{
  _startImage = std::move(loadTexture(&startTexID, palette + ((PersonalData->theme) * 16), bitmap, (64 / 16) * (128 / 16), 4, 64, 16, 64, 128));
}

void ThemeTextures::loadStartbrdImage(const unsigned short *palette, const unsigned int *bitmap, int arraysize, int palLength,
                                      int sprH, int texH)
{
  _startbrdImage = std::move(loadTexture(&startbrdTexID, palette + ((PersonalData->theme) * 16), bitmap, arraysize, palLength, 32, sprH, 32, texH));
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

unique_ptr<glImage[]> ThemeTextures::loadTexture(int *textureId, const unsigned short *palette, const unsigned int *bitmap,
                                                 unsigned int arraySize, int paletteLength, int sprW, int sprH, int texW, int texH)
{

  // We need to delete the texture since the resource held by the unique pointer will be
  // immediately dropped when we assign it to the pointer.

  if (*textureId != 0) {
    nocashMessage("Existing texture found!?");
    glDeleteTextures(1, textureId);
  }

  unique_ptr<glImage[]> texturePtr = std::make_unique<glImage[]>(arraySize);

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

void ThemeTextures::loadCommonTextures()
{
  loadProgressImage(progressPal, progressBitmap);
  loadDialogboxImage(dialogboxPal, dialogboxBitmap);
  loadWirelessIcons(wirelessiconsPal, wirelessiconsBitmap);
}

void ThemeTextures::loadDSiDarkTheme()
{

  loadBubbleImage(bubblePal, bubbleBitmap, 11, 8, 16);
  loadScrollwindowImage(scroll_windowPal, scroll_windowBitmap);
  loadStartImage(start_textPal, start_textBitmap);
  loadBipsImage(bipsPal, bipsBitmap);
  loadButtonarrowImage(button_arrowPal, button_arrowBitmap);
  loadLaunchdotImage(button_arrowPal, launch_dotBitmap);
  loadWirelessIcons(wirelessiconsPal, wirelessiconsBitmap);
  loadSettingsImage(icon_settingsPal, icon_settingsBitmap);
  loadBraceImage(bracePal, braceBitmap);
  loadBoxfullImage(boxPal, boxBitmap);
  loadFolderImage(folderPal, folderBitmap);

  loadCommonTextures();
  loadStartbrdImage(start_borderPal, start_borderBitmap, (32 / 32) * (256 / 80), 16, 80, 256);
}

void ThemeTextures::load3DSTheme()
{

  loadBubbleImage(_3ds_bubblePal, _3ds_bubbleBitmap, 7, 7, 8);
  loadSettingsImage(org_icon_settingsPal, org_icon_settingsBitmap);
 
  loadBoxfullImage(_3ds_box_fullPal, _3ds_box_fullBitmap);
  loadBoxemptyImage(_3ds_box_emptyPal, _3ds_box_emptyBitmap);
  loadFolderImage(_3ds_folderPal, _3ds_folderBitmap);
 

  loadStartbrdImage(_3ds_cursorPal, _3ds_cursorBitmap, (32 / 32) * (192 / 64), 6, 64, 192);

  loadStartImage(start_textPal, start_textBitmap);
  
  //loadBipsImage(bipsPal, bipsBitmap);
  //loadButtonarrowImage(button_arrowPal, button_arrowBitmap);
  //loadLaunchdotImage(button_arrowPal, launch_dotBitmap);

  loadCommonTextures();
}



void ThemeTextures::loadDSiWhiteTheme()
{

  loadBubbleImage(org_bubblePal, org_bubbleBitmap, 11, 8, 16);
  loadScrollwindowImage(org_scroll_windowPal, org_scroll_windowBitmap);
  loadSettingsImage(org_icon_settingsPal, org_icon_settingsBitmap);
  loadBraceImage(org_bracePal, org_braceBitmap);
  loadBoxfullImage(org_boxPal, org_boxBitmap);
  loadFolderImage(org_folderPal, org_folderBitmap);

  loadStartImage(start_textPal, start_textBitmap);
  loadBipsImage(bipsPal, bipsBitmap);
  loadButtonarrowImage(button_arrowPal, button_arrowBitmap);
  loadStartbrdImage(start_borderPal, start_borderBitmap, (32 / 32) * (256 / 80), 16, 80, 256);
  loadLaunchdotImage(button_arrowPal, launch_dotBitmap);

  loadCommonTextures();
}


void ThemeTextures::loadDSiRedTheme()
{

  loadBubbleImage(red_bubblePal, red_bubbleBitmap, 11, 8, 16);
  loadScrollwindowImage(red_scroll_windowPal, red_scroll_windowBitmap);
  loadBraceImage(red_bracePal, red_braceBitmap);
  loadBoxfullImage(red_boxPal, red_boxBitmap);


  loadSettingsImage(org_icon_settingsPal, org_icon_settingsBitmap);
  loadFolderImage(org_folderPal, org_folderBitmap);

  loadStartImage(start_textPal, start_textBitmap);
  loadBipsImage(bipsPal, bipsBitmap);
  loadButtonarrowImage(button_arrowPal, button_arrowBitmap);
  loadStartbrdImage(start_borderPal, start_borderBitmap, (32 / 32) * (256 / 80), 16, 80, 256);
  loadLaunchdotImage(button_arrowPal, launch_dotBitmap);

  loadCommonTextures();
}

void ThemeTextures::loadDSiBlueTheme()
{

  loadBubbleImage(blue_bubblePal, blue_bubbleBitmap, 11, 8, 16);
  loadScrollwindowImage(blue_scroll_windowPal, blue_scroll_windowBitmap);
  loadBraceImage(blue_bracePal, blue_braceBitmap);
  loadBoxfullImage(blue_boxPal, blue_boxBitmap);

  loadSettingsImage(org_icon_settingsPal, org_icon_settingsBitmap);
  loadFolderImage(org_folderPal, org_folderBitmap);

  loadStartImage(start_textPal, start_textBitmap);
  loadBipsImage(bipsPal, bipsBitmap);
  loadButtonarrowImage(button_arrowPal, button_arrowBitmap);
  loadStartbrdImage(start_borderPal, start_borderBitmap, (32 / 32) * (256 / 80), 16, 80, 256);
  loadLaunchdotImage(button_arrowPal, launch_dotBitmap);

  loadCommonTextures();
}

void ThemeTextures::loadDSiGreenTheme()
{
  loadBubbleImage(green_bubblePal, green_bubbleBitmap, 11, 8, 16);
  loadScrollwindowImage(green_scroll_windowPal, green_scroll_windowBitmap);
  loadBraceImage(green_bracePal, green_braceBitmap);
  loadBoxfullImage(green_boxPal, green_boxBitmap);


  loadSettingsImage(org_icon_settingsPal, org_icon_settingsBitmap);
  loadFolderImage(org_folderPal, org_folderBitmap);

  loadStartImage(start_textPal, start_textBitmap);
  loadBipsImage(bipsPal, bipsBitmap);
  loadButtonarrowImage(button_arrowPal, button_arrowBitmap);
  loadStartbrdImage(start_borderPal, start_borderBitmap, (32 / 32) * (256 / 80), 16, 80, 256);
  loadLaunchdotImage(button_arrowPal, launch_dotBitmap);

  loadCommonTextures();
}


void ThemeTextures::loadDSiYellowTheme()
{
  loadBubbleImage(yellow_bubblePal, yellow_bubbleBitmap, 11, 8, 16);
  loadScrollwindowImage(yellow_scroll_windowPal, yellow_scroll_windowBitmap);
  loadBraceImage(yellow_bracePal, yellow_braceBitmap);
  loadBoxfullImage(yellow_boxPal, yellow_boxBitmap);


  loadSettingsImage(org_icon_settingsPal, org_icon_settingsBitmap);
  loadFolderImage(org_folderPal, org_folderBitmap);

  loadStartImage(start_textPal, start_textBitmap);
  loadBipsImage(bipsPal, bipsBitmap);
  loadButtonarrowImage(button_arrowPal, button_arrowBitmap);
  loadStartbrdImage(start_borderPal, start_borderBitmap, (32 / 32) * (256 / 80), 16, 80, 256);
  loadLaunchdotImage(button_arrowPal, launch_dotBitmap);

  loadCommonTextures();
}


void ThemeTextures::loadDSiPurpleTheme()
{
  loadBubbleImage(purple_bubblePal, purple_bubbleBitmap, 11, 8, 16);
  loadScrollwindowImage(purple_scroll_windowPal, purple_scroll_windowBitmap);
  loadBraceImage(purple_bracePal, purple_braceBitmap);
  loadBoxfullImage(purple_boxPal, purple_boxBitmap);


  loadSettingsImage(org_icon_settingsPal, org_icon_settingsBitmap);
  loadFolderImage(org_folderPal, org_folderBitmap);

  loadStartImage(start_textPal, start_textBitmap);
  loadBipsImage(bipsPal, bipsBitmap);
  loadButtonarrowImage(button_arrowPal, button_arrowBitmap);
  loadStartbrdImage(start_borderPal, start_borderBitmap, (32 / 32) * (256 / 80), 16, 80, 256);
  loadLaunchdotImage(button_arrowPal, launch_dotBitmap);

  loadCommonTextures();
}


void ThemeTextures::loadDSiPinkTheme()
{
  loadBubbleImage(pink_bubblePal, pink_bubbleBitmap, 11, 8, 16);
  loadScrollwindowImage(pink_scroll_windowPal, pink_scroll_windowBitmap);
  loadBraceImage(pink_bracePal, pink_braceBitmap);
  loadBoxfullImage(pink_boxPal, pink_boxBitmap);


  loadSettingsImage(org_icon_settingsPal, org_icon_settingsBitmap);
  loadFolderImage(org_folderPal, org_folderBitmap);

  loadStartImage(start_textPal, start_textBitmap);
  loadBipsImage(bipsPal, bipsBitmap);
  loadButtonarrowImage(button_arrowPal, button_arrowBitmap);
  loadStartbrdImage(start_borderPal, start_borderBitmap, (32 / 32) * (256 / 80), 16, 80, 256);
  
  loadLaunchdotImage(button_arrowPal, launch_dotBitmap);

  loadCommonTextures();
}