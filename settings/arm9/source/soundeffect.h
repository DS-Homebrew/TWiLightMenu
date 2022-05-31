#include "common/singleton.h"
#include <maxmod9.h>

#include "soundbank.h"
#include "soundbank_bin.h"

#pragma once
#ifndef _DSIMENUPP_SOUNDEFFECT_H_
#define _DSIMENUPP_SOUNDEFFECT_H_

class SoundEffect
{
  public:
    SoundEffect(): music(false) {
        music = false;
    }
    ~SoundEffect() {}

    void init()
    {
        mmInitDefaultMem((mm_addr)soundbank_bin);

        mmLoadEffect(SFX_LAUNCH);
        mmLoadEffect(SFX_SATURNLAUNCH);
        mmLoadEffect(SFX_SELECT);
        mmLoadEffect(SFX_SATURNSELECT);
        mmLoadEffect(SFX_WRONG);
        mmLoadEffect(SFX_BACK);
        mmLoadEffect(SFX_SATURNBACK);
        mmLoadEffect(SFX_SWITCH);

        snd_launch = {
            {SFX_LAUNCH},            // id
            (int)(1.0f * (1 << 10)), // rate
            0,                       // handle
            255,                     // volume
            128,                     // panning
        };
        snd_saturn_launch = {
            {SFX_SATURNLAUNCH},        // id
            (int)(1.0f * (1 << 10)), // rate
            0,                       // handle
            255,                     // volume
            128,                     // panning
        };
        snd_select = {
            {SFX_SELECT},            // id
            (int)(1.0f * (1 << 10)), // rate
            0,                       // handle
            255,                     // volume
            128,                     // panning
        };
        snd_saturn_select = {
            {SFX_SATURNSELECT},       // id
            (int)(1.0f * (1 << 10)), // rate
            0,                       // handle
            255,                     // volume
            128,                     // panning
        };
        snd_wrong = {
            {SFX_WRONG},             // id
            (int)(1.0f * (1 << 10)), // rate
            0,                       // handle
            255,                     // volume
            128,                     // panning
        };
        snd_back = {
            {SFX_BACK},              // id
            (int)(1.0f * (1 << 10)), // rate
            0,                       // handle
            255,                     // volume
            128,                     // panning
        };
        snd_saturn_back = {
            {SFX_SATURNBACK},         // id
            (int)(1.0f * (1 << 10)), // rate
            0,                       // handle
            255,                     // volume
            128,                     // panning
        };
        snd_switch = {
            {SFX_SWITCH},            // id
            (int)(1.0f * (1 << 10)), // rate
            0,                       // handle
            255,                     // volume
            128,                     // panning
        };
    }

    void playBgMusic(int settingsMusic)
    {
        if (music) return;
		music = true;

		if (settingsMusic == -1) {
			extern int currentTheme;
			switch (currentTheme) {
				case 0:
				case 2:
				case 3:
					settingsMusic = 1;
					break;
				case 4:
					settingsMusic = 0;	// Do not play music if using SEGA Saturn theme
					break;
				case 1:
				case 5:
					settingsMusic = 2;
					break;
			}
		}
		if (settingsMusic == 0) {
			return;
		}
		// Play settings music
		if (settingsMusic == 2) {
			mmLoad(MOD_SETTINGS3D);
			mmSetModuleVolume(1000);
			mmStart(MOD_SETTINGS3D, MM_PLAY_LOOP);
		} else {
			mmLoad(MOD_SETTINGS);
			mmSetModuleVolume(500);
			mmSetModuleTempo(1900);
			mmStart(MOD_SETTINGS, MM_PLAY_LOOP);
		}
    }

    void stopBgMusic()
    {
        if (!mmActive()) return;

		*(int*)0x02003004 = 1; // Fade out sound
		for (int i = 0; i < 25; i++)
			swiWaitForVBlank();
		mmStop();
		*(int*)0x02003004 = 0; // Cancel sound fade out
    }

    mm_sound_effect snd_launch;
    mm_sound_effect snd_saturn_launch;
    mm_sound_effect snd_select;
    mm_sound_effect snd_saturn_select;
    mm_sound_effect snd_stop;
    mm_sound_effect snd_wrong;
    mm_sound_effect snd_back;
    mm_sound_effect snd_saturn_back;
    mm_sound_effect snd_switch;

  private:
    bool music;
};

typedef singleton<SoundEffect>
    soundEffect_s;
inline SoundEffect &snd() { return soundEffect_s::instance(); }

#endif
