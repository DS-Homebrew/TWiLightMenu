#include "ThemeConfig.h"
#include "ThemeTextures.h"
#include "themefilenames.h"
#include "common/twlmenusettings.h"
#include "common/singleton.h"

#include <nds.h>
#include <string>

ThemeConfig::ThemeConfig() : ThemeConfig(false) {}

// Magic numbers derived from default dark theme
ThemeConfig::ThemeConfig(bool _3dsDefaults)
	: _startBorderRenderY(81), _startBorderSpriteW(32), _startBorderSpriteH(80), _startTextRenderY(143),
	_titleboxRenderY(85), _titleboxMaxLines(4), _titleboxTextY(30), _titleboxTextW(240), _titleboxTextLarge(true),
	_bubbleTipRenderY(80), _bubbleTipRenderX(122), _bubbleTipSpriteH(8), _bubbleTipSpriteW(11),
	_rotatingCubesRenderY(78), _shoulderLRenderY(172), _shoulderLRenderX(0), _shoulderRRenderY(172), _shoulderRRenderX(178),
	_volumeRenderY(4), _volumeRenderX(16), _batteryRenderY(5), _batteryRenderX(235), _usernameRenderY(3), _usernameRenderX(28),
	_usernameRenderXDS(4), _dateRenderY(5), _dateRenderX(162), _timeRenderY(5), _timeRenderX(200),
	// _photoRenderY(24), _photoRenderX(179),
	_startTextUserPalette(true), _startBorderUserPalette(true), _buttonArrowUserPalette(true), _movingArrowUserPalette(true),
	_launchDotsUserPalette(true), _dialogBoxUserPalette(true), _usernameUserPalette(true),
	_purpleBatteryAvailable(false), _renderPhoto(true), _playStartupJingle(false), _startupJingleDelayAdjust(0),
	_fontPalette1(0x0000), _fontPalette2(0xDEF7), _fontPalette3(0xC631), _fontPalette4(0xA108),
	_fontPaletteTitlebox1(0x0000), _fontPaletteTitlebox2(0xDEF7), _fontPaletteTitlebox3(0xC631), _fontPaletteTitlebox4(0xA108),
	_fontPaletteDialog1(0x0000), _fontPaletteDialog2(0xDEF7), _fontPaletteDialog3(0xC631), _fontPaletteDialog4(0xA108),
	_fontPaletteOverlay1(0x0000), _fontPaletteOverlay2(0xDEF7), _fontPaletteOverlay3(0xC631), _fontPaletteOverlay4(0xA108),
	_fontPaletteName1(0x0000), _fontPaletteName2(0xDEF7), _fontPaletteName3(0xC631), _fontPaletteName4(0xA108),
	_fontPaletteDateTime1(0x0000), _fontPaletteDateTime2(0xDEF7), _fontPaletteDateTime3(0xC631), _fontPaletteDateTime4(0xA108)
{
	// hack to reassign 3ds defaults
	if (_3dsDefaults) {
		_startBorderRenderY = 92;
		_startBorderSpriteH = 64;
		_titleboxRenderY = 96;
		_bubbleTipRenderX = 125;
		_bubbleTipRenderY = 98;
		_bubbleTipSpriteH = 7;
		_bubbleTipSpriteW = 7;
		_renderPhoto = false;

		_titleboxMaxLines = 3;
		_titleboxTextY = 55;
		_titleboxTextW = 200;
		_titleboxTextLarge = false;
	}

	if (ms().theme == TWLSettings::EThemeSaturn || ms().theme == TWLSettings::EThemeHBL) {
		_renderPhoto = false;
	}
}

int ThemeConfig::getInt(CIniFile &ini, const std::string &item, int defaultVal) {
	if(ms().macroMode)
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

	_startTextUserPalette = getInt(themeConfig, "StartTextUserPalette", _startTextUserPalette);
	_startBorderUserPalette = getInt(themeConfig, "StartBorderUserPalette", _startBorderUserPalette);
	_buttonArrowUserPalette = getInt(themeConfig, "ButtonArrowUserPalette", _buttonArrowUserPalette);
	_movingArrowUserPalette = getInt(themeConfig, "MovingArrowUserPalette", _movingArrowUserPalette);
	_launchDotsUserPalette = getInt(themeConfig, "LaunchDotsUserPalette", _launchDotsUserPalette);
	_dialogBoxUserPalette = getInt(themeConfig, "DialogBoxUserPalette", _dialogBoxUserPalette);
	_usernameUserPalette = getInt(themeConfig, "UsernameUserPalette", _usernameUserPalette);
	_purpleBatteryAvailable = getInt(themeConfig, "PurpleBatteryAvailable", _purpleBatteryAvailable);
	_rotatingCubesRenderY = getInt(themeConfig, "RotatingCubesRenderY", _rotatingCubesRenderY);
	_renderPhoto = getInt(themeConfig, "RenderPhoto", _renderPhoto);

	_playStartupJingle = getInt(themeConfig, "PlayStartupJingle", _playStartupJingle);
	_startupJingleDelayAdjust = getInt(themeConfig, "StartupJingleDelayAdjust", _startupJingleDelayAdjust);

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
	_fontPaletteName1 = getInt(themeConfig, "FontPaletteName1", _fontPaletteName1);
	_fontPaletteName2 = getInt(themeConfig, "FontPaletteName2", _fontPaletteName2);
	_fontPaletteName3 = getInt(themeConfig, "FontPaletteName3", _fontPaletteName3);
	_fontPaletteName4 = getInt(themeConfig, "FontPaletteName4", _fontPaletteName4);
	_fontPaletteDateTime1 = getInt(themeConfig, "FontPaletteDateTime1", _fontPaletteDateTime1);
	_fontPaletteDateTime2 = getInt(themeConfig, "FontPaletteDateTime2", _fontPaletteDateTime2);
	_fontPaletteDateTime3 = getInt(themeConfig, "FontPaletteDateTime3", _fontPaletteDateTime3);
	_fontPaletteDateTime4 = getInt(themeConfig, "FontPaletteDateTime4", _fontPaletteDateTime4);
}
