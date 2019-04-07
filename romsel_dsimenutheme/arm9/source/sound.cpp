#include "sound.h"

#include "graphics/themefilenames.h"
#include "common/dsimenusettings.h"
#include "soundbank_bin.h"
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

extern s16 streaming_buf[STREAMING_BUF_LENGTH + 1];
extern s16 streaming_buf_temp[STREAMING_BUF_LENGTH + 1];

extern bool fill_requested;
extern u32 used_samples;
extern u32 filled_samples;
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
	
	// FILE* soundbank_file;
	
	// switch(ms().dsiMusic) {
	// 	case 2:
	// 		soundbank_file = fopen(std::string(TFN_SHOP_SOUND_EFFECTBANK).c_str(), "rb");
	// 		break;
	// 	case 3:
	// 		soundbank_file = fopen(std::string(TFN_SOUND_EFFECTBANK).c_str(), "rb");
	// 		if (stream_source) break; // fallthrough if stream_source fails.
	// 	case 1:
	// 	default:
	// 		soundbank_file = fopen(std::string(TFN_DEFAULT_SOUND_EFFECTBANK).c_str(), "rb");
	// 		break;
	// }
	
	// fread((void*)SFX_DATA, 1, sizeof(SFX_DATA), soundbank_file);

	// fclose(soundbank_file);

	mmInit(&sys);

	mmSoundBankInMemory((mm_addr)soundbank_bin);

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
	set_streaming_source(stream_source);

	stream.sampling_rate = 16000;	 // 11025HZ
	stream.buffer_length = 1600;	  // should be adequate
	stream.callback = on_stream_request;  // give stereo filling routine
	stream.format = MM_STREAM_16BIT_MONO; // select format
	stream.timer = MM_TIMER0;	     // use timer0
	stream.manual = false;	      // manual filling
	
	toncset16(streaming_buf, 0, STREAMING_BUF_LENGTH); // clear streaming buf
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
	
	// buffer updates happen every time half the buffer has been consumed
	if (fill_requested) {
		
		
		// memmove the unconsumed bytes to the front of the temp buffer
		u32 free_ptr = STREAMING_BUF_LENGTH - used_samples;
		toncset(streaming_buf_temp, 0, sizeof(streaming_buf_temp));
		tonccpy(streaming_buf_temp, streaming_buf + used_samples, free_ptr << 1);

		// Try to fill up the space that was used previously
		filled_samples = fread(streaming_buf_temp + free_ptr, 
						sizeof(s16), used_samples, stream_source);

		// If we couldn't fill completely (file has ended), loop.
		if (filled_samples < used_samples) {
			fseek(stream_source, 0, SEEK_SET);

			// Fill up only the space that was previously unfilled
			filled_samples += fread(streaming_buf_temp + free_ptr + filled_samples,
				 sizeof(s16), (used_samples - filled_samples), stream_source);
		}
		
		tonccpy(streaming_buf, streaming_buf_temp, sizeof(streaming_buf));
		
		// Reset fill state
		fill_requested = false;
		used_samples = 0;
	}
	// mmStreamUpdate();
}