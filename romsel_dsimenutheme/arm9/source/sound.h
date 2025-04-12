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
		void reloadSfxData();
		void unloadSfxData();
        mm_sfxhand playLaunch(u8 panning = 128);
        mm_sfxhand playSelect(u8 panning = 128);
        mm_sfxhand playBack(u8 panning = 128);
        mm_sfxhand playSwitch(u8 panning = 128);
        mm_sfxhand playStartup(u8 panning = 128);
        mm_sfxhand playStop(u8 panning = 128);
        mm_sfxhand playWrong(u8 panning = 128);
        
        // Refill the stream buffers
        volatile void updateStream();

        void loadStream(const bool prepMsg);
        void beginStream();
        void stopStream();
        void unloadStream();
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
        bool sfxDataLoaded;
        bool stream_is_playing;
        bool loopingPoint;
        //mm_sound_effect snd_loading;
        mm_sound_effect mus_startup;
        FILE* stream_start_source;
        FILE* stream_source;
        u32 startup_sample_length;
        u32 seekPos;
};

typedef singleton<SoundControl> soundCtl_s;
inline SoundControl &snd() { return soundCtl_s::instance(); }

#endif