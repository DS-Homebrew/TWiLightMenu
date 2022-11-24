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
        mm_sfxhand playStartup();
        mm_sfxhand playStop();
        mm_sfxhand playWrong();
        
        // Refill the stream buffers
        volatile void updateStream();

        void beginStream();
        void stopStream();
        void fadeOutStream();
        void cancelFadeOutStream();

        // Sets the number of samples of silence to
        // stream before continuing.
        void setStreamDelay(u32 stream_delay);
        
        u32 getStartupSoundLength() { return startup_sample_length; }
      
    private:
        mm_sound_effect snd_launch;
        mm_sound_effect snd_select;
        mm_sound_effect snd_stop;
        mm_sound_effect snd_wrong;
        mm_sound_effect snd_back;
        mm_sound_effect snd_switch;
        mm_stream stream;
        mm_ds_system sndSys;
        bool stream_is_playing;
        //mm_sound_effect snd_loading;
        mm_sound_effect mus_startup;
        FILE* stream_source;
        u32 startup_sample_length;
};

typedef singleton<SoundControl> soundCtl_s;
inline SoundControl &snd() { return soundCtl_s::instance(); }

#endif