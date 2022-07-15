#pragma once
#ifndef __TWILIGHTMENU_SOUND__
#define __TWILIGHTMENU_SOUND__
#include <nds.h>
#include <mm_types.h>
#include <maxmod9.h>
#include "common/singleton.h"
#include <cstdio>

/*
 * Handles playing sound effects and the streaming background music control.
 * See streamingaudio.c for a technical overview of how streaming works.
 */
class SoundControl {
    public:
        SoundControl();
        mm_sfxhand playLaunch();
        mm_sfxhand playSelect();
        mm_sfxhand playBack();
        mm_sfxhand playSwitch();
        mm_sfxhand playStop();
        mm_sfxhand playWrong();
        
        // Refill the stream buffers
        volatile void updateStream();

        void beginStream();
        void stopStream();
        void fadeOutStream();
        void cancelFadeOutStream();
    private:
        mm_sound_effect snd_launch;
        mm_sound_effect snd_select;
        mm_sound_effect snd_stop;
        mm_sound_effect snd_wrong;
        mm_sound_effect snd_back;
        mm_sound_effect snd_switch;
        mm_stream stream;
		mm_ds_system sys;
        bool stream_is_playing;
        FILE* stream_source;
};

typedef singleton<SoundControl> soundCtl_s;
inline SoundControl &snd() { return soundCtl_s::instance(); }

#endif