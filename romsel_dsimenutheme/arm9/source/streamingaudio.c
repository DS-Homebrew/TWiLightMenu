#include "streamingaudio.h"

int sine; // sine position
int lfo;  // LFO position

//-----------------------------------------------------------------------------
enum {
	//-----------------------------------------------------------------------------
	// waveform base frequency
	sine_freq = 500,

	// LFO frequency
	lfo_freq = 3,

	// LFO output shift amount
	lfo_shift = 4,

	// blue backdrop
	bg_colour = 13 << 10,

	// red cpu usage
	cpu_colour = 31
};

/***********************************************************************************
 * on_stream_request
 *
 * Audio stream data request handler.
 ***********************************************************************************/
mm_word on_stream_request(mm_word length, mm_addr dest, mm_stream_formats format) {
	//----------------------------------------------------------------------------------

	s16 *target = dest;
	
	//------------------------------------------------------------
	// synthensize a sine wave with an LFO applied to the pitch
	// the stereo data is interleaved
	//------------------------------------------------------------
	int len = length;
	for( ; len; len-- )
	{
		int sample = sinLerp(sine);
		
		// output sample for left
		*target++ = sample;
		
		// output inverted sample for right
		*target++ = -sample;
		
		sine += sine_freq + (sinLerp(lfo) >> lfo_shift);
		lfo = (lfo + lfo_freq);
	}
	
	return length;
}