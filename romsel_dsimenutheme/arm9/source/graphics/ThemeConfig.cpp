#include "ThemeConfig.h"
#include "ThemeTextures.h"
#include "themefilenames.h"
#include "common/dsimenusettings.h"
#include "common/singleton.h"
#include "common/inifile.h"

#include <nds.h>
#include <string>

ThemeConfig::ThemeConfig() : ThemeConfig(false) {}

// Magic numbers derived from default dark theme
ThemeConfig::ThemeConfig(bool _3dsDefaults)
	: _startBorderRenderY(81), _startBorderSpriteW(32), _startBorderSpriteH(80), _startTextRenderY(143),
	_titleboxRenderY(85), _titleboxMaxLines(4), _titleboxTextY(30), _titleboxTextW(240), _titleboxTextLarge(true),
	_bubbleTipRenderY(80), _bubbleTipRenderX(122), _bubbleTipSpriteH(8), _bubbleTipSpriteW(11),
	_rotatingCubesRenderY(78), _shoulderLRenderY(172), _shoulderLRenderX(0), _shoulderRRenderY(172), _shoulderRRenderX(178),
	_volumeRenderY(4), _volumeRenderX(16),  _batteryRenderY(5), _batteryRenderX(235),
	// _photoRenderY(24), _photoRenderX(179),
	_startTextUserPalette(true), _startBorderUserPalette(true), _buttonArrowUserPalette(true),
	_movingArrowUserPalette(true), _launchDotsUserPalette(true), _dialogBoxUserPalette(true),
	_renderPhoto(true), _playStartupJingle(false), _startupJingleDelayAdjust(0),
	_fontPalette1(0x0000), _fontPalette2(0xDEF7), _fontPalette3(0xC631), _fontPalette4(0xA108)
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

	if (ms().theme == 4 || ms().theme == 5) {
		_renderPhoto = false;
	}
}


void ThemeConfig::loadConfig() {
	printf("tc().loadConfig()\n");
	CIniFile themeConfig(TFN_THEME_SETTINGS);
	_startBorderRenderY = themeConfig.GetInt("THEME", "StartBorderRenderY", _startBorderRenderY);
	_startBorderSpriteW = themeConfig.GetInt("THEME", "StartBorderSpriteW", _startBorderSpriteW);
	_startBorderSpriteH = themeConfig.GetInt("THEME", "StartBorderSpriteH", _startBorderSpriteH);
	_startTextRenderY = themeConfig.GetInt("THEME", "StartTextRenderY", _startTextRenderY);

	_bubbleTipRenderY = themeConfig.GetInt("THEME", "BubbleTipRenderY", _bubbleTipRenderY);
	_bubbleTipRenderX = themeConfig.GetInt("THEME", "BubbleTipRenderX", _bubbleTipRenderX);
	_bubbleTipSpriteW = themeConfig.GetInt("THEME", "BubbleTipSpriteW", _bubbleTipSpriteW);
	_bubbleTipSpriteH = themeConfig.GetInt("THEME", "BubbleTipSpriteH", _bubbleTipSpriteH);

	_titleboxRenderY = themeConfig.GetInt("THEME", "TitleboxRenderY", _titleboxRenderY);
	_titleboxMaxLines = themeConfig.GetInt("THEME", "TitleboxMaxLines", _titleboxMaxLines);
	_titleboxTextY = themeConfig.GetInt("THEME", "TitleboxTextY", _titleboxTextY);
	_titleboxTextW = themeConfig.GetInt("THEME", "TitleboxTextW", _titleboxTextW);
	_titleboxTextLarge = themeConfig.GetInt("THEME", "TitleboxTextLarge", _titleboxTextLarge);

	_volumeRenderX = themeConfig.GetInt("THEME", "VolumeRenderX", _volumeRenderX);
	_volumeRenderY = themeConfig.GetInt("THEME", "VolumeRenderY", _volumeRenderY);
	// _photoRenderX = themeConfig.GetInt("THEME", "PhotoRenderX", _photoRenderX);
	// _photoRenderY = themeConfig.GetInt("THEME", "PhotoRenderY", _photoRenderY);
	_shoulderLRenderY = themeConfig.GetInt("THEME", "ShoulderLRenderY", _shoulderLRenderY);
	_shoulderLRenderX = themeConfig.GetInt("THEME", "ShoulderLRenderX", _shoulderLRenderX);
	_shoulderRRenderY = themeConfig.GetInt("THEME", "ShoulderRRenderY", _shoulderRRenderY);
	_shoulderRRenderX = themeConfig.GetInt("THEME", "ShoulderRRenderX", _shoulderRRenderX);
	_batteryRenderY = themeConfig.GetInt("THEME", "BatteryRenderY", _batteryRenderY);
	_batteryRenderX = themeConfig.GetInt("THEME", "BatteryRenderX", _batteryRenderX);

	_startTextUserPalette = themeConfig.GetInt("THEME", "StartTextUserPalette", _startTextUserPalette);
	_startBorderUserPalette = themeConfig.GetInt("THEME", "StartBorderUserPalette", _startBorderUserPalette);
	_buttonArrowUserPalette = themeConfig.GetInt("THEME", "ButtonArrowUserPalette", _buttonArrowUserPalette);
	_movingArrowUserPalette = themeConfig.GetInt("THEME", "MovingArrowUserPalette", _movingArrowUserPalette);
	_launchDotsUserPalette = themeConfig.GetInt("THEME", "LaunchDotsUserPalette", _launchDotsUserPalette);
	_dialogBoxUserPalette = themeConfig.GetInt("THEME", "DialogBoxUserPalette", _dialogBoxUserPalette);
	_rotatingCubesRenderY = themeConfig.GetInt("THEME", "RotatingCubesRenderY", _rotatingCubesRenderY);
	_renderPhoto = themeConfig.GetInt("THEME", "RenderPhoto", _renderPhoto);

	_playStartupJingle = themeConfig.GetInt("THEME", "PlayStartupJingle", _playStartupJingle);
	_startupJingleDelayAdjust = themeConfig.GetInt("THEME", "StartupJingleDelayAdjust", _startupJingleDelayAdjust);

	_fontPalette1 = themeConfig.GetInt("THEME", "FontPalette1", _fontPalette1);
	_fontPalette2 = themeConfig.GetInt("THEME", "FontPalette2", _fontPalette2);
	_fontPalette3 = themeConfig.GetInt("THEME", "FontPalette3", _fontPalette3);
	_fontPalette4 = themeConfig.GetInt("THEME", "FontPalette4", _fontPalette4);
}
