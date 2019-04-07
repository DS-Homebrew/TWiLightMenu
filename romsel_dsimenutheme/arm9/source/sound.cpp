#include "sound.h"

#include "graphics/themefilenames.h"
#include "common/dsimenusettings.h"
// #include "soundbank_bin.h"
#include "streamingaudio.h"
#include "string.h"
#include "common/tonccpy.h"

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


// mm_sound_effect mus_menu;
extern volatile s16* play_stream_buf;
extern volatile s16* fill_stream_buf;
extern volatile u16 fill_count;
extern volatile u32 filled_samples;
extern bool fill_requested;
extern u32 samples_left_until_next_fill;

#define SAMPLES_USED (STREAMING_BUF_LENGTH - samples_left)
#define REFILL_THRESHOLD STREAMING_BUF_LENGTH >> 2

extern char debug_buf[256];

volatile char SFX_DATA[0x7D000] = {0};
mm_word SOUNDBANK[MSL_BANKSIZE] = {0};

extern "C" {


	// mm_word handle_event(mm_word msg, mm_word param){
	// 	switch( msg )
	// 	{
	// 		case MMCB_SONGMESSAGE:
	// 			// Process song message
	// 			break;
	// 		case MMCB_SONGFINISHED:
	// 			// A song has finished playing
	// 			break; 
	// 	}
	// 	return 0;
	// }
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
	stream.buffer_length = 3200;	  // should be adequate
	stream.callback = on_stream_request;  // give stereo filling routine
	stream.format = MM_STREAM_16BIT_MONO; // select format
	stream.timer = MM_TIMER0;	     // use timer0
	stream.manual = false;	      // manual filling
	
	
	// Prep the first section of the stream
	fread((void*)play_stream_buf, sizeof(s16), STREAMING_BUF_LENGTH, stream_source);

	// Fill 2 sections for parallel fill
	fill_requested = true;
	updateStream();
	fill_requested = true;
	updateStream();
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
	mmStreamOpen(&stream);
	SetYtrigger(0);
}

void SoundControl::stopStream() {
	mmStreamClose();
}

// Updates the background music fill buffer
void SoundControl::updateStream() {
	
	//
	if (fill_requested && fill_count < TOTAL_FILLS) {
		nocashMessage("Filling...");
		long int instance_filled = 0;
		instance_filled = fread((s16*)fill_stream_buf + filled_samples, sizeof(s16), SAMPLES_PER_FILL, stream_source);
		if (instance_filled <  SAMPLES_PER_FILL) {
			fseek(stream_source, 0, SEEK_SET);
			instance_filled += fread((s16*)fill_stream_buf + filled_samples + instance_filled,
				 sizeof(s16), (SAMPLES_PER_FILL - instance_filled), stream_source);
		}
		filled_samples += instance_filled;
		sprintf(debug_buf, "Loaded  %li samples, currently %li filled, fill number %i", instance_filled, filled_samples, fill_count);
    	nocashMessage(debug_buf);
		fill_requested = false;
		samples_left_until_next_fill = SAMPLES_PER_FILL;
		fill_count++;
	} else if (fill_count >= TOTAL_FILLS) {
		filled_samples = 0;
		fill_count = 0;
	}

}