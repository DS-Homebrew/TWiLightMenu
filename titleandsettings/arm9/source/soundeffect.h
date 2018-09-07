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
        counter = 0;
    }
    ~SoundEffect() {}

    void init()
    {
        mmInitDefaultMem((mm_addr)soundbank_bin);

        mmLoadEffect(SFX_LAUNCH);
        mmLoadEffect(SFX_SELECT);
        mmLoadEffect(SFX_STOP);
        mmLoadEffect(SFX_WRONG);
        mmLoadEffect(SFX_BACK);
        mmLoadEffect(SFX_SWITCH);
        mmLoadEffect(SFX_SETTINGS);

        snd_launch = {
            {SFX_LAUNCH},            // id
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
        snd_stop = {
            {SFX_STOP},              // id
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
        snd_switch = {
            {SFX_SWITCH},            // id
            (int)(1.0f * (1 << 10)), // rate
            0,                       // handle
            255,                     // volume
            128,                     // panning
        };
        mus_settings = {
            {SFX_SETTINGS},          // id
            (int)(1.0f * (1 << 10)), // rate
            0,                       // handle
            255,                     // volume
            128,                     // panning
        };
    }

    void playBgMusic()
    {
        if (!music)
        {
            music = true;
            mmEffectEx(&mus_settings); // Play settings music
        }
    }

    void tickBgMusic()
    {
        if (music)
        {
            counter++;
            if (counter >= 60 * 25)
            { // Length of music file in seconds (60*ss)
                mmEffectEx(&mus_settings);
                counter = 0;
            }
        }
    }

    void stopBgMusic()
    {
        if (music) {
            mmEffectCancelAll();

        }
        music = false;
    }
    
    mm_sound_effect snd_launch;
    mm_sound_effect snd_select;
    mm_sound_effect snd_stop;
    mm_sound_effect snd_wrong;
    mm_sound_effect snd_back;
    mm_sound_effect snd_switch;
    mm_sound_effect mus_settings;

  private:
    bool music;
    int counter;
};

typedef singleton<SoundEffect>
    soundEffect_s;
inline SoundEffect &snd() { return soundEffect_s::instance(); }

#endif
