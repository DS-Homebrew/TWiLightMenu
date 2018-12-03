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
    }

    mm_sound_effect snd_launch;
    mm_sound_effect snd_select;
    mm_sound_effect snd_stop;
    mm_sound_effect snd_wrong;
    mm_sound_effect snd_back;
    mm_sound_effect snd_switch;

  private:
    bool music;
    int counter;
};

typedef singleton<SoundEffect>
    soundEffect_s;
inline SoundEffect &snd() { return soundEffect_s::instance(); }

#endif
