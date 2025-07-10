#include "sound.h"

#include "common/twlmenusettings.h"
#include "common/flashcard.h"
#include "graphics/fontHandler.h"
#include "language.h"
#include "streamingaudio.h"
#include "string.h"
#include "common/tonccpy.h"

#include "soundbank.h"
#include "soundbank_bin.h"

#include <algorithm>
#include <sys/stat.h>

/* extern volatile s16 fade_counter;
extern volatile bool fade_out;

extern volatile s16* play_stream_buf;
extern volatile s16* fill_stream_buf;

// Number of samples filled into the fill buffer so far.
extern volatile s32 filled_samples;

extern volatile bool fill_requested;
extern volatile s32 samples_left_until_next_fill;
extern volatile s32 streaming_buf_ptr;

#define SAMPLES_USED (STREAMING_BUF_LENGTH - samples_left)
#define REFILL_THRESHOLD STREAMING_BUF_LENGTH >> 2

#ifdef SOUND_DEBUG
extern char debug_buf[256];
#endif

extern volatile u32 sample_delay_count; */

SoundControl::SoundControl()
	: stream_is_playing(false), stream_source(NULL)
 {

	mmInitDefaultMem((mm_addr)soundbank_bin);

	mmLoadEffect(SFX_LAUNCH);
	mmLoadEffect(SFX_SELECT);
	mmLoadEffect(SFX_STOP);
	mmLoadEffect(SFX_WRONG);
	mmLoadEffect(SFX_BACK);
	mmLoadEffect(SFX_SWITCH);

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

	/* play_stream_buf = new s16[STREAMING_BUF_LENGTH];
	fill_stream_buf = new s16[STREAMING_BUF_LENGTH];


	stream.sampling_rate = 16000;	 		// 16000Hz
	stream.format = MM_STREAM_16BIT_MONO;  // select format

	u8 wavFormat = 0;
	u8 numChannels = 1;
	stream_source = fopen("nitro:/music.wav", "rb");
	fseek(stream_source, 0x14, SEEK_SET);
	fread(&wavFormat, sizeof(u8), 1, stream_source);
	fseek(stream_source, 0x16, SEEK_SET);
	fread(&numChannels, sizeof(u8), 1, stream_source);
	stream.format = numChannels == 2 ? MM_STREAM_8BIT_STEREO : MM_STREAM_16BIT_MONO;
	fseek(stream_source, 0x18, SEEK_SET);
	fread(&stream.sampling_rate, sizeof(u16), 1, stream_source);

	fseek(stream_source, 0x2C, SEEK_SET);

	stream.buffer_length = 0x1000;	  			// should be adequate
	stream.callback = on_stream_request;    
	stream.timer = MM_TIMER0;	    	   // use timer0
	stream.manual = false;	      		   // auto filling

	// Prep the first section of the stream
	fread((void*)play_stream_buf, sizeof(s16), STREAMING_BUF_LENGTH, stream_source);

	// Fill the next section premptively
	fread((void*)fill_stream_buf, sizeof(s16), STREAMING_BUF_LENGTH, stream_source); */
}

mm_sfxhand SoundControl::playLaunch() { return mmEffectEx(&snd_launch); }
mm_sfxhand SoundControl::playSelect() { return mmEffectEx(&snd_select); }
mm_sfxhand SoundControl::playBack() { return mmEffectEx(&snd_back); }
mm_sfxhand SoundControl::playSwitch() { return mmEffectEx(&snd_switch); }
mm_sfxhand SoundControl::playStop() { return mmEffectEx(&snd_stop); }
mm_sfxhand SoundControl::playWrong() { return mmEffectEx(&snd_wrong); }

void SoundControl::beginStream() {
	/* if (!stream_source) return;

	// open the stream
	stream_is_playing = true;
	mmStreamOpen(&stream);
	SetYtrigger(0); */

	mmLoad(MOD_MUSIC);
	mmStart(MOD_MUSIC, MM_PLAY_LOOP);
}

void SoundControl::stopStream() {
	/* if (!stream_source) return;

	stream_is_playing = false;
	mmStreamClose(); */

	// mmStop();
}

void SoundControl::fadeOutStream() {
	//fade_out = true; // Bugged
	//fifoSendValue32(FIFO_USER_01, 1); // Fade out on ARM7 side
}

void SoundControl::cancelFadeOutStream() {
	//fade_out = false;
	//fade_counter = FADE_STEPS;
}


// Samples remaining in the fill buffer.
#define SAMPLES_LEFT_TO_FILL (abs(STREAMING_BUF_LENGTH - filled_samples))

// Samples that were already streamed and need to be refilled into the buffer.
#define SAMPLES_TO_FILL (abs(streaming_buf_ptr - filled_samples))

// Updates the background music fill buffer
// Fill the amount of samples that were used up between the
// last fill request and this.

// Precondition Invariants:
// filled_samples <= STREAMING_BUF_LENGTH
// filled_samples <= streaming_buf_ptr

// Postcondition Invariants:
// filled_samples <= STREAMING_BUF_LENGTH
// filled_samples <= streaming_buf_ptr
// fill_requested == false
volatile void SoundControl::updateStream() {
	
	/* if (!stream_is_playing) return;
	if (fill_requested && filled_samples < STREAMING_BUF_LENGTH) {
			
		// Reset the fill request
		fill_requested = false;
		int instance_filled = 0;

		// Either fill the max amount, or fill up the buffer as much as possible.
		int instance_to_fill = std::min(SAMPLES_LEFT_TO_FILL, SAMPLES_TO_FILL);

		// If we don't read enough samples, loop from the beginning of the file.
		instance_filled = fread((s16*)fill_stream_buf + filled_samples, sizeof(s16), instance_to_fill, stream_source);		
		if (instance_filled < instance_to_fill) {
			fseek(stream_source, 0x2C, SEEK_SET);
			instance_filled += fread((s16*)fill_stream_buf + filled_samples + instance_filled,
				 sizeof(s16), (instance_to_fill - instance_filled), stream_source);
		}

		#ifdef SOUND_DEBUG
		sprintf(debug_buf, "FC: SAMPLES_LEFT_TO_FILL: %li, SAMPLES_TO_FILL: %li, instance_filled: %i, filled_samples %li, to_fill: %i", SAMPLES_LEFT_TO_FILL, SAMPLES_TO_FILL, instance_filled, filled_samples, instance_to_fill);
    	nocashMessage(debug_buf);
		#endif

		// maintain invariant 0 < filled_samples <= STREAMING_BUF_LENGTH
		filled_samples = std::min<s32>(filled_samples + instance_filled, STREAMING_BUF_LENGTH);

	
	} else if (fill_requested && filled_samples >= STREAMING_BUF_LENGTH) {
		// filled_samples == STREAMING_BUF_LENGTH is the only possible case
		// but we'll keep it at gte to be safe.
		filled_samples = 0;
		// fill_count = 0;
	} */

}
