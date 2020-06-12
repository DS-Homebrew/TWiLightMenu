#include "sound.h"

#include "systemfilenames.h"
#include "streamingaudio.h"
#include "string.h"
#include "common/tonccpy.h"
#include <algorithm>

#define SFX_STARTUP		0

#define MSL_NSONGS		0
#define MSL_NSAMPS		1
#define MSL_BANKSIZE	1


extern volatile s16 fade_counter;
extern volatile bool fade_out;

extern volatile s16* play_stream_buf;
extern volatile s16* fill_stream_buf;

// Number of samples filled into the fill buffer so far.
extern volatile s32 filled_samples;

extern volatile bool fill_requested;
extern volatile long streamed_samples;
extern volatile s32 streaming_buf_ptr;

#define SAMPLES_USED (STREAMING_BUF_LENGTH - samples_left)
#define REFILL_THRESHOLD STREAMING_BUF_LENGTH >> 2

#ifdef SOUND_DEBUG
extern char debug_buf[256];
#endif

extern volatile u32 sample_delay_count;

// volatile char SFX_DATA[0x7D000] = {0};
mm_word SOUNDBANK[MSL_BANKSIZE] = {0};

SoundControl::SoundControl()
	: stream_is_playing(false), stream_source(NULL), startup_sample_length(0)
 {

	sys.mod_count = MSL_NSONGS;
	sys.samp_count = MSL_NSAMPS;
	sys.mem_bank = SOUNDBANK;
	sys.fifo_channel = FIFO_MAXMOD;

	// FILE* soundbank_file;
	// soundbank_file = fopen(std::string(SFN_SOUND_EFFECTBANK).c_str(), "rb");

	// fread((void*)SFX_DATA, 1, sizeof(SFX_DATA), soundbank_file);

	// fclose(soundbank_file);

	// // Since SFX_STARTUP is the first sample, it begins at 0x10 after the
	// // *maxmod* header. Subtract the size of the sample header,
	// // and divide by two to get length in samples.
	// // https://github.com/devkitPro/mmutil/blob/master/source/msl.c#L80
	
	// startup_sample_length = (((*(u32*)(SFX_DATA + 0x10)) - 20) >> 1);

	// sprintf(debug_buf, "Read sample length %li for startup", startup_sample_length);
    // nocashMessage(debug_buf);

	mmInit(&sys);

	// mmSoundBankInMemory((mm_addr)SFX_DATA);
	// mmLoadEffect(SFX_STARTUP);

	// mus_startup = {
	//     {SFX_STARTUP},	   // id
	//     (int)(1.0f * (1 << 10)), // rate
	//     0,			     // handle
	//     255,		     // volume
	//     128,		     // panning
	// };
	stream_source = fopen(std::string(SFN_SOUND_BG).c_str(), "rb");
	fseek((FILE*)stream_source, 0, SEEK_SET);

	stream.sampling_rate = 16000;	 		// 16000Hz
	stream.buffer_length = 400;	  			// should be adequate
	stream.callback = on_stream_request;    
	stream.format = MM_STREAM_16BIT_MONO;  // select format
	stream.timer = MM_TIMER0;	    	   // use timer0
	stream.manual = false;	      		   // auto filling
	
	// Prep the first section of the stream
	fread((void*)play_stream_buf, sizeof(s16), STREAMING_BUF_LENGTH, (FILE*)stream_source);

	// Fill the next section premptively
	fread((void*)fill_stream_buf, sizeof(s16), STREAMING_BUF_LENGTH, (FILE*)stream_source);

}

void SoundControl::beginStream() {
	// open the stream
	stream_is_playing = true;
	mmStreamOpen(&stream);
	SetYtrigger(0);
}

void SoundControl::stopStream() {
	stream_is_playing = false;
	mmStreamClose();
}

void SoundControl::fadeOutStream() {
	fade_out = true;
}

void SoundControl::cancelFadeOutStream() {
	fade_out = false;
	fade_counter = FADE_STEPS;
}

void SoundControl::setStreamDelay(u32 delay) {
	sample_delay_count = delay;
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
	if (!stream_is_playing) return;
	if (fill_requested && filled_samples < STREAMING_BUF_LENGTH) {
		// Reset the fill request
		fill_requested = false;
		int instance_filled = 0;

		// Either fill the max amount, or fill up the buffer as much as possible.
		int instance_to_fill = streamed_samples;
		streamed_samples = 0;

		// If we don't read enough samples, loop from the beginning of the file.
		instance_filled = fread((s16*)fill_stream_buf + filled_samples, sizeof(s16), instance_to_fill, (FILE*)stream_source);	


		if (instance_filled < instance_to_fill) {
			fseek((FILE*)stream_source, 0, SEEK_SET);
			instance_filled += fread((s16*)fill_stream_buf + filled_samples + instance_filled,
				 sizeof(s16), (instance_to_fill - instance_filled), (FILE*)stream_source);
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
	}

}
