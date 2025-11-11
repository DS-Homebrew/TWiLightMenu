#include <nds.h>
#include <string>
#include "common/inifile.h"
#include "common/singleton.h"

#pragma once
#ifndef _THEMECONFIG_H_
#define _THEMECONFIG_H_

class ThemeConfig {
private:
	int getInt(CIniFile &ini, const std::string &item, int defaultVal);

	int _startBorderRenderY;
	int _startBorderSpriteW;
	int _startBorderSpriteH;
	int _startTextRenderY;

	int _titleboxRenderY;
	int _titleboxMaxLines;
	int _titleboxTextY;
	int _titleboxTextW;
	bool _titleboxTextLarge;
	
	int _bubbleTipRenderY;
	int _bubbleTipRenderX;
	int _bubbleTipSpriteH;
	int _bubbleTipSpriteW;

	int _rotatingCubesRenderY;

	int _shoulderLRenderY;
	int _shoulderLRenderX;
	int _shoulderLTextY;
	int _shoulderLTextX;
	int _shoulderLTextAlign;
	
	int _shoulderRRenderY;
	int _shoulderRRenderX;
	int _shoulderRTextY;
	int _shoulderRTextX;
	int _shoulderRTextAlign;

	int _volumeRenderY;
	int _volumeRenderX;
	
	int _batteryRenderY;
	int _batteryRenderX;

	int _usernameRenderY;
	int _usernameRenderX;
	int _usernameRenderXDS;
	bool _usernameEdgeAlpha;

	int _dateRenderY;
	int _dateRenderX;
	int _timeRenderY;
	int _timeRenderX;

	// int _photoRenderY;
	// int _photoRenderX;

	bool _bipsUserPalette;
	bool _boxUserPalette;
	bool _boxEmptyUserPalette;
	bool _boxFullUserPalette;
	bool _braceUserPalette;
	bool _bubbleUserPalette;
	bool _buttonArrowUserPalette;
	bool _cornerButtonUserPalette;
	bool _cursorUserPalette;
	bool _dialogBoxUserPalette;
	bool _folderUserPalette;
	bool _launchDotsUserPalette;
	bool _movingArrowUserPalette;
	bool _progressUserPalette;
	bool _scrollWindowUserPalette;
	bool _smallCartUserPalette;
	bool _startBorderUserPalette;
	bool _startTextUserPalette;
	bool _wirelessIconsUserPalette;
	bool _iconA26UserPalette;
	bool _iconCPCUserPalette;
	bool _iconCOLUserPalette;
	bool _iconGBUserPalette;
	bool _iconGBAUserPalette;
	bool _iconGBAModeUserPalette;
	bool _iconGGUserPalette;
	bool _iconHBUserPalette;
	bool _iconIMGUserPalette;
	bool _iconINTUserPalette;
	bool _iconM5UserPalette;
	bool _iconManualUserPalette;
	bool _iconMDUserPalette;
	bool _iconMINIUserPalette;
	bool _iconMSXUserPalette;
	bool _iconNESUserPalette;
	bool _iconNGPUserPalette;
	bool _iconPCEUserPalette;
	bool _iconPLGUserPalette;
	bool _iconSettingsUserPalette;
	bool _iconSGUserPalette;
	bool _iconSMSUserPalette;
	bool _iconSNESUserPalette;
	bool _iconUnknownUserPalette;
	bool _iconVIDUserPalette;
	bool _iconWSUserPalette;
	bool _usernameUserPalette;
	bool _progressBarUserPalette;
	
	bool _purpleBatteryAvailable;
	bool _renderPhoto;
	bool _darkLoading;
	bool _useAlphaBlend;
	bool _playStopSound;
	bool _playStartupJingle;
	int _startupJingleDelayAdjust;
	u16 _progressBarColor;

	u16 _fontPalette1;
	u16 _fontPalette2;
	u16 _fontPalette3;
	u16 _fontPalette4;
	u16 _fontPaletteDisabled1;
	u16 _fontPaletteDisabled2;
	u16 _fontPaletteDisabled3;
	u16 _fontPaletteDisabled4;
	u16 _fontPaletteTitlebox1;
	u16 _fontPaletteTitlebox2;
	u16 _fontPaletteTitlebox3;
	u16 _fontPaletteTitlebox4;
	u16 _fontPaletteDialog1;
	u16 _fontPaletteDialog2;
	u16 _fontPaletteDialog3;
	u16 _fontPaletteDialog4;
	u16 _fontPaletteOverlay1;
	u16 _fontPaletteOverlay2;
	u16 _fontPaletteOverlay3;
	u16 _fontPaletteOverlay4;
	u16 _fontPaletteUsername1;
	u16 _fontPaletteUsername2;
	u16 _fontPaletteUsername3;
	u16 _fontPaletteUsername4;
	u16 _fontPaletteDateTime1;
	u16 _fontPaletteDateTime2;
	u16 _fontPaletteDateTime3;
	u16 _fontPaletteDateTime4;

public:
	ThemeConfig();
	virtual ~ThemeConfig() = default;

	void loadConfig();

	int startBorderRenderY() const { return _startBorderRenderY; }
	int startBorderSpriteW() const { return _startBorderSpriteW; }
	int startBorderSpriteH() const { return _startBorderSpriteH; }
	int startTextRenderY() const { return _startTextRenderY; }

	int titleboxRenderY() const { return _titleboxRenderY; }
	int titleboxMaxLines() const { return _titleboxMaxLines; }
	int titleboxTextY() const { return _titleboxTextY; }
	int titleboxTextW() const { return _titleboxTextW; }
	bool titleboxTextLarge() const { return _titleboxTextLarge; }
	
	int bubbleTipRenderY() const { return _bubbleTipRenderY; }
	int bubbleTipRenderX() const { return _bubbleTipRenderX; }
	int bubbleTipSpriteH() const { return _bubbleTipSpriteH; }
	int bubbleTipSpriteW() const { return _bubbleTipSpriteW; }

	int rotatingCubesRenderY() const { return _rotatingCubesRenderY; }

	int shoulderLRenderY() const { return _shoulderLRenderY; }
	int shoulderLRenderX() const { return _shoulderLRenderX; }
	int shoulderLTextY() const { return _shoulderLTextY; }
	int shoulderLTextX() const { return _shoulderLTextX; }
	int shoulderLTextAlign() const { return _shoulderLTextAlign; }
	
	int shoulderRRenderY() const { return _shoulderRRenderY; }
	int shoulderRRenderX() const { return _shoulderRRenderX; }
	int shoulderRTextY() const { return _shoulderRTextY; }
	int shoulderRTextX() const { return _shoulderRTextX; }
	int shoulderRTextAlign() const { return _shoulderRTextAlign; }

	int volumeRenderY() const { return _volumeRenderY; }
	int volumeRenderX() const { return _volumeRenderX; }
	
	int batteryRenderY() const { return _batteryRenderY; }
	int batteryRenderX() const { return _batteryRenderX; }

	int usernameRenderY() const { return _usernameRenderY; }
	int usernameRenderX() const { return _usernameRenderX; }
	int usernameRenderXDS() const { return _usernameRenderXDS; }
	bool usernameEdgeAlpha() const { return _usernameEdgeAlpha; }

	int dateRenderY() const { return _dateRenderY; }
	int dateRenderX() const { return _dateRenderX; }
	int timeRenderY() const { return _timeRenderY; }
	int timeRenderX() const { return _timeRenderX; }

	// int photoRenderY() const { return _photoRenderY; }
	// int photoRenderX() const { return _photoRenderX; }

	bool bipsUserPalette() const { return _bipsUserPalette; }
	bool boxUserPalette() const { return _boxUserPalette; }
	bool boxEmptyUserPalette() const { return _boxEmptyUserPalette; }
	bool boxFullUserPalette() const { return _boxFullUserPalette; }
	bool braceUserPalette() const { return _braceUserPalette; }
	bool bubbleUserPalette() const { return _bubbleUserPalette; }
	bool buttonArrowUserPalette() const { return _buttonArrowUserPalette; }
	bool cornerButtonUserPalette() const { return _cornerButtonUserPalette; }
	bool cursorUserPalette() const { return _cursorUserPalette; }
	bool dialogBoxUserPalette() const { return _dialogBoxUserPalette; }
	bool folderUserPalette() const { return _folderUserPalette; }
	bool launchDotsUserPalette() const { return _launchDotsUserPalette; }
	bool movingArrowUserPalette() const { return _movingArrowUserPalette; }
	bool progressUserPalette() const { return _progressUserPalette; }
	bool scrollWindowUserPalette() const { return _scrollWindowUserPalette; }
	bool smallCartUserPalette() const { return _smallCartUserPalette; }
	bool startBorderUserPalette() const { return _startBorderUserPalette; }
	bool startTextUserPalette() const { return _startTextUserPalette; }
	bool wirelessIconsUserPalette() const { return _wirelessIconsUserPalette; }
	bool iconA26UserPalette() const { return _iconA26UserPalette; }
	bool iconCPCUserPalette() const { return _iconCPCUserPalette; }
	bool iconCOLUserPalette() const { return _iconCOLUserPalette; }
	bool iconGBUserPalette() const { return _iconGBUserPalette; }
	bool iconGBAUserPalette() const { return _iconGBAUserPalette; }
	bool iconGBAModeUserPalette() const { return _iconGBAModeUserPalette; }
	bool iconGGUserPalette() const { return _iconGGUserPalette; }
	bool iconHBUserPalette() const { return _iconHBUserPalette; }
	bool iconIMGUserPalette() const { return _iconIMGUserPalette; }
	bool iconINTUserPalette() const { return _iconINTUserPalette; }
	bool iconM5UserPalette() const { return _iconM5UserPalette; }
	bool iconManualUserPalette() const { return _iconManualUserPalette; }
	bool iconMDUserPalette() const { return _iconMDUserPalette; }
	bool iconMINIUserPalette() const { return _iconMINIUserPalette; }
	bool iconMSXUserPalette() const { return _iconMSXUserPalette; }
	bool iconNESUserPalette() const { return _iconNESUserPalette; }
	bool iconNGPUserPalette() const { return _iconNGPUserPalette; }
	bool iconPCEUserPalette() const { return _iconPCEUserPalette; }
	bool iconPLGUserPalette() const { return _iconPLGUserPalette; }
	bool iconSettingsUserPalette() const { return _iconSettingsUserPalette; }
	bool iconSGUserPalette() const { return _iconSGUserPalette; }
	bool iconSMSUserPalette() const { return _iconSMSUserPalette; }
	bool iconSNESUserPalette() const { return _iconSNESUserPalette; }
	bool iconUnknownUserPalette() const { return _iconUnknownUserPalette; }
	bool iconVIDUserPalette() const { return _iconVIDUserPalette; }
	bool iconWSUserPalette() const { return _iconWSUserPalette; }
	bool usernameUserPalette() const { return _usernameUserPalette; }
	bool progressBarUserPalette() const { return _progressBarUserPalette; }

	bool purpleBatteryAvailable() const { return _purpleBatteryAvailable; }
	bool renderPhoto() const { return _renderPhoto; }
	bool darkLoading() const { return _darkLoading; }
	bool useAlphaBlend() const { return _useAlphaBlend; }

	bool playStopSound() const { return _playStopSound; }
	bool playStartupJingle() const { return _playStartupJingle; }
	int startupJingleDelayAdjust() const { return _startupJingleDelayAdjust; }
	u16 progressBarColor() const { return _progressBarColor; }

	u16 fontPalette1() const { return _fontPalette1; }
	u16 fontPalette2() const { return _fontPalette2; }
	u16 fontPalette3() const { return _fontPalette3; }
	u16 fontPalette4() const { return _fontPalette4; }
	u16 fontPaletteDisabled1() const { return _fontPaletteDisabled1; }
	u16 fontPaletteDisabled2() const { return _fontPaletteDisabled2; }
	u16 fontPaletteDisabled3() const { return _fontPaletteDisabled3; }
	u16 fontPaletteDisabled4() const { return _fontPaletteDisabled4; }
	u16 fontPaletteTitlebox1() const { return _fontPaletteTitlebox1; }
	u16 fontPaletteTitlebox2() const { return _fontPaletteTitlebox2; }
	u16 fontPaletteTitlebox3() const { return _fontPaletteTitlebox3; }
	u16 fontPaletteTitlebox4() const { return _fontPaletteTitlebox4; }
	u16 fontPaletteDialog1() const { return _fontPaletteDialog1; }
	u16 fontPaletteDialog2() const { return _fontPaletteDialog2; }
	u16 fontPaletteDialog3() const { return _fontPaletteDialog3; }
	u16 fontPaletteDialog4() const { return _fontPaletteDialog4; }
	u16 fontPaletteOverlay1() const { return _fontPaletteOverlay1; }
	u16 fontPaletteOverlay2() const { return _fontPaletteOverlay2; }
	u16 fontPaletteOverlay3() const { return _fontPaletteOverlay3; }
	u16 fontPaletteOverlay4() const { return _fontPaletteOverlay4; }
	u16 fontPaletteUsername1() const { return _fontPaletteUsername1; }
	u16 fontPaletteUsername2() const { return _fontPaletteUsername2; }
	u16 fontPaletteUsername3() const { return _fontPaletteUsername3; }
	u16 fontPaletteUsername4() const { return _fontPaletteUsername4; }
	u16 fontPaletteDateTime1() const { return _fontPaletteDateTime1; }
	u16 fontPaletteDateTime2() const { return _fontPaletteDateTime2; }
	u16 fontPaletteDateTime3() const { return _fontPaletteDateTime3; }
	u16 fontPaletteDateTime4() const { return _fontPaletteDateTime4; }
};

typedef singleton<ThemeConfig> themeConfig_s;
inline ThemeConfig &tc() { return themeConfig_s::instance(); }

#endif