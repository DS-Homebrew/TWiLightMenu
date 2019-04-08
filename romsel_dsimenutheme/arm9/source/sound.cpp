#include "sound.h"

#include "graphics/themefilenames.h"
#include "common/dsimenusettings.h"
// #include "soundbank_bin.h"
#include "streamingaudio.h"
#include "string.h"
#include "common/tonccpy.h"
#include <algorithm>

#define SFX_WRONG	0
#define SFX_LAUNCH	1
#define SFX_STOP	2
#define SFX_SWITCH	3
#define SFX_STARTUP	4
#define SFX_SELECT	5
#define SFX_BACK	6

#define MSL_NSONGS	0
#define MSL_NSAMPS	7
#define MSL_BANKSIZE	7


volatile bool fill_lock = false;
// mm_sound_effect mus_menu;
extern volatile s16* play_stream_buf;
extern volatile s16* fill_stream_buf;
extern volatile u16 fill_count;
extern volatile u32 filled_samples;
extern volatile bool fill_requested;
extern volatile u32 samples_left_until_next_fill;
extern volatile u32 streaming_buf_ptr;

#define SAMPLES_USED (STREAMING_BUF_LENGTH - samples_left)
#define REFILL_THRESHOLD STREAMING_BUF_LENGTH >> 2

extern char debug_buf[256];

volatile char SFX_DATA[0x7D000] = {0};
mm_word SOUNDBANK[MSL_BANKSIZE] = {0};

extern "C" {
	void fillAudio() {
		if (fill_requested) {
			snd().updateStream();
		}
	}	
}


SoundControl::SoundControl() {

	sys.mod_count = MSL_NSONGS;
	sys.samp_count = MSL_NSAMPS;
	sys.mem_bank = SOUNDBANK;
	sys.fifo_channel = FIFO_MAXMOD;
	
	FILE* soundbank_file;
	
	switch(ms().dsiMusic) {
		case 2:
			soundbank_file = fopen(std::string(TFN_SHOP_SOUND_EFFECTBANK).c_str(), "rb");
			break;
		case 3:
			soundbank_file = fopen(std::string(TFN_SOUND_EFFECTBANK).c_str(), "rb");
			if (stream_source) break; // fallthrough if stream_source fails.
		case 1:
		default:
			soundbank_file = fopen(std::string(TFN_DEFAULT_SOUND_EFFECTBANK).c_str(), "rb");
			break;
	}
	
	fread((void*)SFX_DATA, 1, sizeof(SFX_DATA), soundbank_file);

	fclose(soundbank_file);

	mmInit(&sys);

	mmSoundBankInMemory((mm_addr)SFX_DATA);

 	// mmSetEventHandler(handle_event);

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


	switch(ms().dsiMusic) {
		case 2:
			stream_source = fopen(std::string(TFN_SHOP_SOUND_BG).c_str(), "rb");
			break;
		case 3:
			stream_source = fopen(std::string(TFN_SOUND_BG).c_str(), "rb");
			if (stream_source) break; // fallthrough if stream_source fails.
		case 1:
		default:
			stream_source = fopen(std::string(TFN_DEFAULT_SOUND_BG).c_str(), "rb");
			break;
	}
	

	fseek(stream_source, 0, SEEK_SET);

	stream.sampling_rate = 11025;	 // 11025HZ
	stream.buffer_length = 1600;	  // should be adequate
	stream.callback = on_stream_request;  // give stereo filling routine
	stream.format = MM_STREAM_16BIT_MONO; // select format
	stream.timer = MM_TIMER0;	     // use timer0
	stream.manual = false;	      // manual filling
	

	
	// Prep the first section of the stream
	fread((void*)play_stream_buf, sizeof(s16), STREAMING_BUF_LENGTH, stream_source);

	// Fill the next section premptively
	fread((void*)fill_stream_buf, sizeof(s16), STREAMING_BUF_LENGTH, stream_source);

	

	
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

void SoundControl::beginStream() {
	// open the stream
	stream_is_playing = true;
	// timerStart(1, ClockDivider_256, 512, fillAudio);
	mmStreamOpen(&stream);
	SetYtrigger(0);
}

void SoundControl::stopStream() {
	// timerStop(1);
	stream_is_playing = false;
	mmStreamClose();
}

#define SAMPLES_LEFT_TO_FILL (STREAMING_BUF_LENGTH - filled_samples)
#define SAMPLES_TO_FILL (streaming_buf_ptr - filled_samples)

// Updates the background music fill buffer
volatile void SoundControl::updateStream() {
	
	if (!stream_is_playing) return;
	if (fill_lock) return;
	if (fill_requested && filled_samples < STREAMING_BUF_LENGTH) {
		fill_lock = true;
		// timerPause(1);
		nocashMessage("Filling...");
		fill_requested = false;
		fill_count++;
		long unsigned int instance_filled = 0;
		
		long unsigned int instance_to_fill = std::min(SAMPLES_LEFT_TO_FILL, SAMPLES_TO_FILL);

		instance_filled = fread((s16*)fill_stream_buf + filled_samples, sizeof(s16), instance_to_fill, stream_source);
		if (instance_filled <  instance_to_fill) {
			fseek(stream_source, 0, SEEK_SET);
			instance_filled += fread((s16*)fill_stream_buf + filled_samples + instance_filled,
				 sizeof(s16), (instance_to_fill - instance_filled), stream_source);
		}
		filled_samples += instance_filled;
		samples_left_until_next_fill = SAMPLES_PER_FILL;
		sprintf(debug_buf, "Loaded  %li samples, currently %li filled, fill number %i", instance_filled, filled_samples, fill_count);
    	nocashMessage(debug_buf);
		fill_lock = false;
		// timerUnpause(1);
	
	} else if (filled_samples >= STREAMING_BUF_LENGTH) {
		filled_samples = 0;
		fill_count = 0;
	}

}