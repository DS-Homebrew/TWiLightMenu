#include "sound.h"

#include "soundbank.h"
#include "soundbank_bin.h"

#include "graphics/themefilenames.h"
#include "common/dsimenusettings.h"
#include "streamingaudio.h"
#include "string.h"
// mm_sound_effect mus_menu;

extern s16 streaming_buf[2800];
extern mm_word last_length;

mm_word SOUNDBANK[MSL_BANKSIZE];

SoundControl::SoundControl() {

	mm_ds_system sys;
	sys.mod_count = MSL_NSONGS;
	sys.samp_count = MSL_NSAMPS;
	sys.mem_bank = SOUNDBANK;
	sys.fifo_channel = FIFO_MAXMOD;
	mmInit(&sys);

	mmSoundBankInMemory((mm_addr)soundbank_bin);

	// mmInitDefaultMem((mm_addr);
	mmLoadEffect(SFX_LAUNCH);
	mmLoadEffect(SFX_SELECT);
	mmLoadEffect(SFX_STOP);
	mmLoadEffect(SFX_WRONG);
	mmLoadEffect(SFX_BACK);
	mmLoadEffect(SFX_SWITCH);
	mmLoadEffect(SFX_STARTUP);
	// mmLoadEffect(SFX_MENU);

	snd_launch = {
	    {SFX_LAUNCH},	    // id
	    (int)(1.0f * (1 << 10)), // rate
	    0,			     // handle
	    255,		     // volume
	    128,		     // panning
	};
	snd_select = {
	    {SFX_SELECT},	    // id
	    (int)(1.0f * (1 << 10)), // rate
	    0,			     // handle
	    255,		     // volume
	    128,		     // panning
	};
	snd_stop = {
	    {SFX_STOP},		     // id
	    (int)(1.0f * (1 << 10)), // rate
	    0,			     // handle
	    255,		     // volume
	    128,		     // panning
	};
	snd_wrong = {
	    {SFX_WRONG},	     // id
	    (int)(1.0f * (1 << 10)), // rate
	    0,			     // handle
	    255,		     // volume
	    128,		     // panning
	};
	snd_back = {
	    {SFX_BACK},		     // id
	    (int)(1.0f * (1 << 10)), // rate
	    0,			     // handle
	    255,		     // volume
	    128,		     // panning
	};
	snd_switch = {
	    {SFX_SWITCH},	    // id
	    (int)(1.0f * (1 << 10)), // rate
	    0,			     // handle
	    255,		     // volume
	    128,		     // panning
	};
	mus_startup = {
	    {SFX_STARTUP},	   // id
	    (int)(1.0f * (1 << 10)), // rate
	    0,			     // handle
	    255,		     // volume
	    128,		     // panning
	};

	stream_source = fopen(std::string(TFN_SOUND_BG).c_str(), "rb");
	if (!stream_source) {
		nocashMessage("Opening soundbg from nitrofs");

		if (ms().dsiMusic == 2) {
			nocashMessage("Opening soundbg from nitrofs");
			stream_source = fopen(std::string(TFN_SHOP_SOUND_BG).c_str(), "rb");
		} else {
			nocashMessage("Opening default from nitrofs");
			nocashMessage(std::string(TFN_DEFAULT_SOUND_BG).c_str());
			stream_source = fopen("nitro:/sound/defaultbg.pcm.raw", "rb");
		}
	}
	// set_streaming_source(stream_source);

	mm_stream stream;
	stream.sampling_rate = 16000;	 // 16kHZ
	stream.buffer_length = 1200;	  // should be adequate
	stream.callback = on_stream_request;  // give stereo filling routine
	stream.format = MM_STREAM_16BIT_MONO; // select format
	stream.timer = MM_TIMER0;	     // use timer0
	stream.manual = true;	      // manual filling

	// open the stream
	mmStreamOpen(&stream);
	SetYtrigger(0);

	// mus_menu = {
	//     {SFX_MENU},		     // id
	//     (int)(1.0f * (1 << 10)), // rate
	//     0,			     // handle
	//     255,		     // volume
	//     128,		     // panning
	// };
}

mm_sfxhand SoundControl::playLaunch() { return mmEffectEx(&snd_launch); }
mm_sfxhand SoundControl::playSelect() { return mmEffectEx(&snd_select); }
mm_sfxhand SoundControl::playBack() { return mmEffectEx(&snd_back); }
mm_sfxhand SoundControl::playSwitch() { return mmEffectEx(&snd_switch); }
mm_sfxhand SoundControl::playStartup() { return mmEffectEx(&mus_startup); }
mm_sfxhand SoundControl::playStop() { return mmEffectEx(&snd_stop); }
mm_sfxhand SoundControl::playWrong() { return mmEffectEx(&snd_wrong); }


void SoundControl::updateStream() {
	
	// memmove(streaming_buf, streaming_buf + last_length, 1200 - last_length);

	// if (last_length >= 2800) {
	// 	fread(streaming_buf, sizeof(s16), 2800, stream_source);
	// 	last_length = 0;
	// }
	mmStreamUpdate(); // Update sound.
}