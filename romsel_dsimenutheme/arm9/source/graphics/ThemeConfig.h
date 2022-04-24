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
	
	int _shoulderRRenderY;
	int _shoulderRRenderX;

	int _volumeRenderY;
	int _volumeRenderX;
	
	int _batteryRenderY;
	int _batteryRenderX;

	int _usernameRenderY;
	int _usernameRenderX;
	int _usernameRenderXDS;

	int _dateRenderY;
	int _dateRenderX;
	int _timeRenderY;
	int _timeRenderX;

	// int _photoRenderY;
	// int _photoRenderX;

	bool _startTextUserPalette;
	bool _startBorderUserPalette;
	bool _buttonArrowUserPalette;
	bool _movingArrowUserPalette;
	bool _launchDotsUserPalette;
	bool _dialogBoxUserPalette;
	bool _purpleBatteryAvailable;

	bool _renderPhoto;
	bool _playStartupJingle;
	int _startupJingleDelayAdjust;

	u16 _fontPalette1;
	u16 _fontPalette2;
	u16 _fontPalette3;
	u16 _fontPalette4;
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

public:
	ThemeConfig();
	ThemeConfig(bool _3dsDefaults);
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
	
	int shoulderRRenderY() const { return _shoulderRRenderY; }
	int shoulderRRenderX() const { return _shoulderRRenderX; }

	int volumeRenderY() const { return _volumeRenderY; }
	int volumeRenderX() const { return _volumeRenderX; }
	
	int batteryRenderY() const { return _batteryRenderY; }
	int batteryRenderX() const { return _batteryRenderX; }

	int usernameRenderY() const { return _usernameRenderY; }
	int usernameRenderX() const { return _usernameRenderX; }
	int usernameRenderXDS() const { return _usernameRenderXDS; }

	int dateRenderY() const { return _dateRenderY; }
	int dateRenderX() const { return _dateRenderX; }
	int timeRenderY() const { return _timeRenderY; }
	int timeRenderX() const { return _timeRenderX; }

	// int photoRenderY() const { return _photoRenderY; }
	// int photoRenderX() const { return _photoRenderX; }

	bool startTextUserPalette() const { return _startTextUserPalette; }
	bool startBorderUserPalette() const { return _startBorderUserPalette; }
	bool buttonArrowUserPalette() const { return _buttonArrowUserPalette; }
	bool movingArrowUserPalette() const { return _movingArrowUserPalette; }
	bool launchDotsUserPalette() const { return _launchDotsUserPalette; }
	bool dialogBoxUserPalette() const { return _dialogBoxUserPalette; }
	bool purpleBatteryAvailable() const { return _purpleBatteryAvailable; }

	bool renderPhoto() const { return _renderPhoto; }

	bool playStartupJingle() const { return _playStartupJingle; }
	int startupJingleDelayAdjust() const { return _startupJingleDelayAdjust; }

	u16 fontPalette1() const { return _fontPalette1; }
	u16 fontPalette2() const { return _fontPalette2; }
	u16 fontPalette3() const { return _fontPalette3; }
	u16 fontPalette4() const { return _fontPalette4; }
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
};

typedef singleton<ThemeConfig> themeConfig_s;
inline ThemeConfig &tc() { return themeConfig_s::instance(); }

#endif