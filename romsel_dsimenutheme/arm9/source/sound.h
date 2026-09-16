#pragma once
#ifndef __TWILIGHTMENU_SOUND__
#define __TWILIGHTMENU_SOUND__
#include <nds.h>
#include <mm_types.h>
#include <maxmod9.h>
#include "common/singleton.h"
#include <cstdio>

// Sound effect banks from older themes only contain the first 7 samples.
// Banks that also carry the lid sounds have 9.
#define MSL_NSAMPS_BASE	7
#define MSL_NSAMPS_LID	9

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
        mm_sfxhand playLidClose(u8 panning = 128);
        mm_sfxhand playLidOpen(u8 panning = 128);

        // False for sound effect banks predating the lid sounds.
        bool sfxHasLidSounds() const { return sfxSampleCount >= MSL_NSAMPS_LID; }
        
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
        void readSfxBank();
        void loadSfxEffects();

        mm_sound_effect snd_launch;
        mm_sound_effect snd_select;
        mm_sound_effect snd_stop;
        mm_sound_effect snd_wrong;
        mm_sound_effect snd_back;
        mm_sound_effect snd_switch;
        mm_sound_effect snd_lidClose;
        mm_sound_effect snd_lidOpen;
        mm_stream stream;
		mm_ds_system sndSys;
        bool sfxDataLoaded;
        u16 sfxSampleCount;
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