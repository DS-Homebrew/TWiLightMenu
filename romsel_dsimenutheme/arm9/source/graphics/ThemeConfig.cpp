#include "ThemeConfig.h"
#include "ThemeTextures.h"
#include "themefilenames.h"
#include "common/twlmenusettings.h"
#include "common/singleton.h"

#include <nds.h>
#include <string>

// Magic numbers derived from default dark theme
ThemeConfig::ThemeConfig()
	: _startBorderRenderY(81), _startBorderSpriteW(32), _startBorderSpriteH(80), _startTextRenderY(143),
	_titleboxRenderY(85), _titleboxMaxLines(4), _titleboxTextY(30), _titleboxTextW(240), _titleboxTextLarge(true),
	_bubbleTipRenderY(80), _bubbleTipRenderX(122), _bubbleTipSpriteH(8), _bubbleTipSpriteW(11),
	_rotatingCubesRenderY(78), _shoulderLRenderY(172), _shoulderLRenderX(0), _shoulderRRenderY(172), _shoulderRRenderX(178),
	_volumeRenderY(4), _volumeRenderX(16), _batteryRenderY(5), _batteryRenderX(235), _usernameRenderY(3), _usernameRenderX(28),
	_usernameRenderXDS(4), _dateRenderY(5), _dateRenderX(162), _timeRenderY(5), _timeRenderX(200),
	// _photoRenderY(24), _photoRenderX(179),
	_bipsUserPalette(false), _boxUserPalette(false), _boxEmptyUserPalette(false), _boxFullUserPalette(false),
	_braceUserPalette(false), _bubbleUserPalette(false), _buttonArrowUserPalette(true), _cornerButtonUserPalette(false),
	_cursorUserPalette(false), _dialogBoxUserPalette(true), _folderUserPalette(false), _launchDotsUserPalette(true),
	_movingArrowUserPalette(true), _progressUserPalette(true), _scrollWindowUserPalette(false), _smallCartUserPalette(false),
	_startBorderUserPalette(true), _startTextUserPalette(true), _wirelessIconsUserPalette(false),
	_iconA26UserPalette(false), _iconCOLUserPalette(false), _iconGBUserPalette(false), _iconGBAUserPalette(false),
	_iconGBAModeUserPalette(false),	_iconGGUserPalette(false), _iconIMGUserPalette(false), _iconINTUserPalette(false),
	_iconM5UserPalette(false), _iconManualUserPalette(false), _iconMDUserPalette(false), _iconNESUserPalette(false),
	_iconNGPUserPalette(false), _iconPCEUserPalette(false), _iconPLGUserPalette(false), _iconSettingsUserPalette(false),
	_iconSGUserPalette(false), _iconSMSUserPalette(false), _iconSNESUserPalette(false), _iconUnknownUserPalette(false),
	_iconWSUserPalette(false),
	_usernameUserPalette(true), _progressBarUserPalette(true),
	_purpleBatteryAvailable(false), _renderPhoto(true), _darkLoading(false), _useAlphaBlend(true),
	_playStartupJingle(false), _startupJingleDelayAdjust(0), _progressBarColor(0x7C00),
	_fontPalette1(0x0000), _fontPalette2(0xDEF7), _fontPalette3(0xC631), _fontPalette4(0xA108),
	_fontPaletteTitlebox1(0x0000), _fontPaletteTitlebox2(0xDEF7), _fontPaletteTitlebox3(0xC631), _fontPaletteTitlebox4(0xA108),
	_fontPaletteDialog1(0x0000), _fontPaletteDialog2(0xDEF7), _fontPaletteDialog3(0xC631), _fontPaletteDialog4(0xA108),
	_fontPaletteOverlay1(0x0000), _fontPaletteOverlay2(0xDEF7), _fontPaletteOverlay3(0xC631), _fontPaletteOverlay4(0xA108),
	_fontPaletteUsername1(0x0000), _fontPaletteUsername2(0xDEF7), _fontPaletteUsername3(0xC631), _fontPaletteUsername4(0xA108),
	_fontPaletteDateTime1(0x0000), _fontPaletteDateTime2(0xDEF7), _fontPaletteDateTime3(0xC631), _fontPaletteDateTime4(0xA108)
{
	if (ms().theme == TWLSettings::ETheme3DS) {
		_startBorderUserPalette = false;
		_dialogBoxUserPalette = false;
	}

	if (ms().theme == TWLSettings::EThemeSaturn || ms().theme == TWLSettings::EThemeHBL) {
		_renderPhoto = false;
		_darkLoading = true;
	}
}

int ThemeConfig::getInt(CIniFile &ini, const std::string &item, int defaultVal) {
	if (ms().macroMode)
		return ini.GetInt("MACRO", item, ini.GetInt("THEME", item, defaultVal));

	return ini.GetInt("THEME", item, defaultVal);
}

void ThemeConfig::loadConfig() {
	//iprintf("tc().loadConfig()\n");
	int macroY = 0;
	int macroW = 0;

	CIniFile themeConfig(TFN_THEME_SETTINGS);
	_startBorderRenderY = getInt(themeConfig, "StartBorderRenderY", _startBorderRenderY);
	_startBorderSpriteW = getInt(themeConfig, "StartBorderSpriteW", _startBorderSpriteW);
	_startBorderSpriteH = getInt(themeConfig, "StartBorderSpriteH", _startBorderSpriteH);
	_startTextRenderY = getInt(themeConfig, "StartTextRenderY", _startTextRenderY);

	_bubbleTipRenderY = getInt(themeConfig, "BubbleTipRenderY", _bubbleTipRenderY);
	_bubbleTipRenderX = getInt(themeConfig, "BubbleTipRenderX", _bubbleTipRenderX);
	_bubbleTipSpriteW = getInt(themeConfig, "BubbleTipSpriteW", _bubbleTipSpriteW);
	_bubbleTipSpriteH = getInt(themeConfig, "BubbleTipSpriteH", _bubbleTipSpriteH);

	_titleboxRenderY = getInt(themeConfig, "TitleboxRenderY", _titleboxRenderY);
	_titleboxMaxLines = getInt(themeConfig, "TitleboxMaxLines", _titleboxMaxLines);
	macroY = getInt(themeConfig, "MacroTitleboxTextY", -1);
	macroW = getInt(themeConfig, "MacroTitleboxTextW", -1);
	_titleboxTextY = getInt(themeConfig, "TitleboxTextY", _titleboxTextY);
	_titleboxTextW = getInt(themeConfig, "TitleboxTextW", _titleboxTextW);
	if (ms().macroMode) {
		if (macroY != -1) _titleboxTextY = macroY;
		if (macroW != -1) _titleboxTextW = macroW;
	}
	_titleboxTextLarge = getInt(themeConfig, "TitleboxTextLarge", _titleboxTextLarge);

	_volumeRenderX = getInt(themeConfig, "VolumeRenderX", _volumeRenderX);
	_volumeRenderY = getInt(themeConfig, "VolumeRenderY", _volumeRenderY);
	// _photoRenderX = getInt(themeConfig, "PhotoRenderX", _photoRenderX);
	// _photoRenderY = getInt(themeConfig, "PhotoRenderY", _photoRenderY);
	_shoulderLRenderY = getInt(themeConfig, "ShoulderLRenderY", _shoulderLRenderY);
	_shoulderLRenderX = getInt(themeConfig, "ShoulderLRenderX", _shoulderLRenderX);
	_shoulderRRenderY = getInt(themeConfig, "ShoulderRRenderY", _shoulderRRenderY);
	_shoulderRRenderX = getInt(themeConfig, "ShoulderRRenderX", _shoulderRRenderX);
	_batteryRenderY = getInt(themeConfig, "BatteryRenderY", _batteryRenderY);
	_batteryRenderX = getInt(themeConfig, "BatteryRenderX", _batteryRenderX);
	_usernameRenderY = getInt(themeConfig, "UsernameRenderY", _usernameRenderY);
	_usernameRenderX = getInt(themeConfig, "UsernameRenderX", _usernameRenderX);
	_usernameRenderXDS = getInt(themeConfig, "UsernameRenderXDS", _usernameRenderXDS);
	_dateRenderY = getInt(themeConfig, "DateRenderY", _dateRenderY);
	_dateRenderX = getInt(themeConfig, "DateRenderX", _dateRenderX);
	_timeRenderY = getInt(themeConfig, "TimeRenderY", _timeRenderY);
	_timeRenderX = getInt(themeConfig, "TimeRenderX", _timeRenderX);

	_bipsUserPalette = getInt(themeConfig, "BipsUserPalette", _bipsUserPalette);
	_boxUserPalette = getInt(themeConfig, "BoxUserPalette", _boxUserPalette);
	_boxEmptyUserPalette = getInt(themeConfig, "BoxEmptyUserPalette", _boxEmptyUserPalette);
	_boxFullUserPalette = getInt(themeConfig, "BoxFullUserPalette", _boxFullUserPalette);
	_braceUserPalette = getInt(themeConfig, "BraceUserPalette", _braceUserPalette);
	_bubbleUserPalette = getInt(themeConfig, "BubbleUserPalette", _bubbleUserPalette);
	_buttonArrowUserPalette = getInt(themeConfig, "ButtonArrowUserPalette", _buttonArrowUserPalette);
	_cornerButtonUserPalette = getInt(themeConfig, "CornerButtonUserPalette", _cornerButtonUserPalette);
	_cursorUserPalette = getInt(themeConfig, "CursorUserPalette", _cursorUserPalette);
	_dialogBoxUserPalette = getInt(themeConfig, "DialogBoxUserPalette", _dialogBoxUserPalette);
	_folderUserPalette = getInt(themeConfig, "FolderUserPalette", _folderUserPalette);
	_launchDotsUserPalette = getInt(themeConfig, "LaunchDotsUserPalette", _launchDotsUserPalette);
	_movingArrowUserPalette = getInt(themeConfig, "MovingArrowUserPalette", _movingArrowUserPalette);
	_progressUserPalette = getInt(themeConfig, "ProgressUserPalette", _progressUserPalette);
	_scrollWindowUserPalette = getInt(themeConfig, "ScrollWindowUserPalette", _scrollWindowUserPalette);
	_smallCartUserPalette = getInt(themeConfig, "SmallCartUserPalette", _smallCartUserPalette);
	_startBorderUserPalette = getInt(themeConfig, "StartBorderUserPalette", _startBorderUserPalette);
	_startTextUserPalette = getInt(themeConfig, "StartTextUserPalette", _startTextUserPalette);
	_wirelessIconsUserPalette = getInt(themeConfig, "WirelessIconsUserPalette", _wirelessIconsUserPalette);

	_iconA26UserPalette = getInt(themeConfig, "IconA26UserPalette", _iconA26UserPalette);
	_iconCOLUserPalette = getInt(themeConfig, "IconCOLUserPalette", _iconCOLUserPalette);
	_iconGBUserPalette = getInt(themeConfig, "IconGBUserPalette", _iconGBUserPalette);
	_iconGBAUserPalette = getInt(themeConfig, "IconGBAUserPalette", _iconGBAUserPalette);
	_iconGBAModeUserPalette = getInt(themeConfig, "IconGBAModeUserPalette", _iconGBAModeUserPalette);
	_iconGGUserPalette = getInt(themeConfig, "IconGGUserPalette", _iconGGUserPalette);
	_iconIMGUserPalette = getInt(themeConfig, "IconIMGUserPalette", _iconIMGUserPalette);
	_iconINTUserPalette = getInt(themeConfig, "IconINTUserPalette", _iconINTUserPalette);
	_iconM5UserPalette = getInt(themeConfig, "IconM5UserPalette", _iconM5UserPalette);
	_iconManualUserPalette = getInt(themeConfig, "IconManualUserPalette", _iconManualUserPalette);
	_iconMDUserPalette = getInt(themeConfig, "IconMDUserPalette", _iconMDUserPalette);
	_iconNESUserPalette = getInt(themeConfig, "IconNESUserPalette", _iconNESUserPalette);
	_iconNGPUserPalette = getInt(themeConfig, "IconNGPUserPalette", _iconNGPUserPalette);
	_iconPCEUserPalette = getInt(themeConfig, "IconPCEUserPalette", _iconPCEUserPalette);
	_iconPLGUserPalette = getInt(themeConfig, "IconPLGUserPalette", _iconPLGUserPalette);
	_iconSettingsUserPalette = getInt(themeConfig, "IconSettingsUserPalette", _iconSettingsUserPalette);
	_iconSGUserPalette = getInt(themeConfig, "IconSGUserPalette", _iconSGUserPalette);
	_iconSMSUserPalette = getInt(themeConfig, "IconSMSUserPalette", _iconSMSUserPalette);
	_iconSNESUserPalette = getInt(themeConfig, "IconSNESUserPalette", _iconSNESUserPalette);
	_iconUnknownUserPalette = getInt(themeConfig, "IconUnknownUserPalette", _iconUnknownUserPalette);
	_iconWSUserPalette = getInt(themeConfig, "IconWSUserPalette", _iconWSUserPalette);

	_usernameUserPalette = getInt(themeConfig, "UsernameUserPalette", _usernameUserPalette);
	_progressBarUserPalette = getInt(themeConfig, "ProgressBarUserPalette", _progressBarUserPalette);

	_purpleBatteryAvailable = getInt(themeConfig, "PurpleBatteryAvailable", _purpleBatteryAvailable);
	_rotatingCubesRenderY = getInt(themeConfig, "RotatingCubesRenderY", _rotatingCubesRenderY);
	_renderPhoto = getInt(themeConfig, "RenderPhoto", _renderPhoto);
	_darkLoading = getInt(themeConfig, "DarkLoading", _darkLoading);
	_useAlphaBlend = getInt(themeConfig, "UseAlphaBlend", _useAlphaBlend);

	_playStartupJingle = getInt(themeConfig, "PlayStartupJingle", _playStartupJingle);
	_startupJingleDelayAdjust = getInt(themeConfig, "StartupJingleDelayAdjust", _startupJingleDelayAdjust);
	_progressBarColor = getInt(themeConfig, "ProgressBarColor", _progressBarColor);

	_fontPalette1 = getInt(themeConfig, "FontPalette1", _fontPalette1);
	_fontPalette2 = getInt(themeConfig, "FontPalette2", _fontPalette2);
	_fontPalette3 = getInt(themeConfig, "FontPalette3", _fontPalette3);
	_fontPalette4 = getInt(themeConfig, "FontPalette4", _fontPalette4);
	_fontPaletteTitlebox1 = getInt(themeConfig, "FontPaletteTitlebox1", _fontPalette1);
	_fontPaletteTitlebox2 = getInt(themeConfig, "FontPaletteTitlebox2", _fontPalette2);
	_fontPaletteTitlebox3 = getInt(themeConfig, "FontPaletteTitlebox3", _fontPalette3);
	_fontPaletteTitlebox4 = getInt(themeConfig, "FontPaletteTitlebox4", _fontPalette4);
	_fontPaletteDialog1 = getInt(themeConfig, "FontPaletteDialog1", _fontPalette1);
	_fontPaletteDialog2 = getInt(themeConfig, "FontPaletteDialog2", _fontPalette2);
	_fontPaletteDialog3 = getInt(themeConfig, "FontPaletteDialog3", _fontPalette3);
	_fontPaletteDialog4 = getInt(themeConfig, "FontPaletteDialog4", _fontPalette4);
	_fontPaletteOverlay1 = getInt(themeConfig, "FontPaletteOverlay1", _fontPalette1);
	_fontPaletteOverlay2 = getInt(themeConfig, "FontPaletteOverlay2", _fontPalette2);
	_fontPaletteOverlay3 = getInt(themeConfig, "FontPaletteOverlay3", _fontPalette3);
	_fontPaletteOverlay4 = getInt(themeConfig, "FontPaletteOverlay4", _fontPalette4);
	_fontPaletteUsername1 = getInt(themeConfig, "FontPaletteUsername1", _fontPalette1);
	_fontPaletteUsername2 = getInt(themeConfig, "FontPaletteUsername2", _fontPalette2);
	_fontPaletteUsername3 = getInt(themeConfig, "FontPaletteUsername3", _fontPalette3);
	_fontPaletteUsername4 = getInt(themeConfig, "FontPaletteUsername4", _fontPalette4);
	_fontPaletteDateTime1 = getInt(themeConfig, "FontPaletteDateTime1", _fontPalette1);
	_fontPaletteDateTime2 = getInt(themeConfig, "FontPaletteDateTime2", _fontPalette2);
	_fontPaletteDateTime3 = getInt(themeConfig, "FontPaletteDateTime3", _fontPalette3);
	_fontPaletteDateTime4 = getInt(themeConfig, "FontPaletteDateTime4", _fontPalette4);
}
