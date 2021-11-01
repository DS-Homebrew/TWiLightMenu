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
        mm_sfxhand playDSiBoot();
        mm_sfxhand playSelect();
        
        // Refill the stream buffers
        volatile void updateStream();

        void beginStream();
        void stopStream();
        void fadeOutStream();
        void cancelFadeOutStream();

    private:
        mm_sound_effect snd_dsiboot;
        mm_sound_effect snd_select;
        mm_stream stream;
        bool stream_is_playing;
        FILE* stream_source;
};

typedef singleton<SoundControl> soundCtl_s;
inline SoundControl &snd() { return soundCtl_s::instance(); }

#endif