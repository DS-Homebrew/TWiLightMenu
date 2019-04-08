#include <nds.h>
#include <string>
#include "common/singleton.h"

#pragma once
#ifndef _THEMECONFIG_H_
#define _THEMECONFIG_H_

class ThemeConfig {
    private:
        int _startBorderRenderY;
        int _startBorderSpriteW;
        int _startBorderSpriteH;
        int _startTextRenderY;
        int _titleboxRenderY;
        
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

        // int _photoRenderY;
        // int _photoRenderX;

        bool _startTextUserPalette;
        bool _startBorderUserPalette;
        bool _buttonArrowUserPalette;
        bool _movingArrowUserPalette;
        bool _launchDotsUserPalette;
        bool _dialogBoxUserPalette;

        bool _renderPhoto;
        bool _playStartupJingle;
        int _startupJingleDelayAdjust;

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

        // int photoRenderY() const { return _photoRenderY; }
        // int photoRenderX() const { return _photoRenderX; }

        bool startTextUserPalette() const { return _startTextUserPalette; }
        bool startBorderUserPalette() const { return _startBorderUserPalette; }
        bool buttonArrowUserPalette() const { return _buttonArrowUserPalette; }
        bool movingArrowUserPalette() const { return _movingArrowUserPalette; }
        bool launchDotsUserPalette() const { return _launchDotsUserPalette; }
        bool dialogBoxUserPalette() const { return _dialogBoxUserPalette; }

        bool renderPhoto() const { return _renderPhoto; }

        bool playStartupJingle() const { return _playStartupJingle; }
        int startupJingleDelayAdjust() const { return _startupJingleDelayAdjust; }
};

typedef singleton<ThemeConfig> themeConfig_s;
inline ThemeConfig &tc() { return themeConfig_s::instance(); }

#endif