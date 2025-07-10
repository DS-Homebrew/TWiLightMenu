#include "streamingaudio.h"
#include "common/tonccpy.h"

/* Not a actual pointer, but the index to the current location of the 
 * buffers that are being streamed.
 */
volatile s32 streaming_buf_ptr = 0;

/**
 * Number of samples filled so far
 */
volatile s32 filled_samples = 0;

// Pointers to the stream buffers.
volatile s16* play_stream_buf = NULL;
volatile s16* fill_stream_buf = NULL;


// Toggle this to true to trigger a fill as soon as possible.
volatile bool fill_requested = false;

volatile u16 fade_counter = FADE_STEPS;
volatile bool fade_out = false;

volatile u32 sample_delay_count = 0;

#ifdef SOUND_DEBUG
char debug_buf[256] = {0};
#endif

void alloc_streaming_buf(void) {
	play_stream_buf = malloc(STREAMING_BUF_LENGTH*sizeof(s16));
	fill_stream_buf = malloc(STREAMING_BUF_LENGTH*sizeof(s16));
}

void free_streaming_buf(void) {
	free(play_stream_buf);
	free(fill_stream_buf);
}

void resetStreamSettings() {
	streaming_buf_ptr = 0;
	filled_samples = 0;


	fill_requested = false;

	fade_counter = FADE_STEPS;
	fade_out = false;

	sample_delay_count = 0;
}

/*
 * The maxmod stream request handler. 
 * 
 * This method is called automatically by maxmod at random times.
 * 
 * While streaming from RAM is a trivial matter, streaming from
 * a file is a bit more involved because SD access is much slower.
 * 
 * We can't fread in the stream request handler, or it will take
 * too long and the DS will crash. We can load it after as soon as
 * possible, but there is no guarantee that it will be finished 
 * before we need to stream again. Waiting until we've run out
 * of things to stream and then loading new samples is too slow.
 * We need it to be "just-in-time".
 * 
 * The buffers that play_stream_buf and fill_stream_buf must be
 * completely filled with audio data before the first call to
 * this method. This is set up by sound.cpp.
 * 
 * Samples are then streamed from play_stream_buf. Once a 
 * sample has been compied to dest (and thus streamed), it
 * is replaced by a sample from the same location at 
 * fill_stream_buf. Hence there is always new data at
 * play_stream_buf.
 * 
 * Every SAMPLES_PER_FILL, a fill request is made. This is
 * handled by sound.cpp, and copies enough data to replace
 * the number of samples that were streamed since the last
 * fill request into the fill_stream_buf. This ensures that 
 * there is always new data in the fill_stream_buf, and that
 * data is only overwritten once it has been copied into
 * play_stream_buf.
 * 
 * streaming_buf_ptr keeps track of where in the buffers
 * data will be read from. This is reset after STREAMING_BUF_LENGTH
 * samples have been streamed.
 * 
 * In combination, this creates two looping buffers that ensure
 * that new data is ready just in time.
 */
mm_word on_stream_request(mm_word length, mm_addr dest, mm_stream_formats format) {
   
    // Debug stuff
    // if (fill_requested) {
    //     nocashMessage("missed fill");
    // }

    int len = length;
	s16 *target = dest;
   
    // fill delay with silence
    for (; sample_delay_count && len; len--, sample_delay_count--) {
        *target++ = 0;
    }

    for (; len; len--) {
        // Loop the streaming_buf_ptr
        if (streaming_buf_ptr >= STREAMING_BUF_LENGTH) {
            streaming_buf_ptr = 0;
        }

        // Stream the next sample
		*target++ = (*(play_stream_buf + streaming_buf_ptr) >> (FADE_STEPS - fade_counter));

        // Copy the next sample that will be played the next time
        // the streaming_buf_ptr is at this location from
        // fill_stream_buf
        *(play_stream_buf + streaming_buf_ptr) = *(fill_stream_buf + streaming_buf_ptr);

        // Zero out fill_stream_buf at this location, preventing
        // glitched sound if fills don't keep up.
        *(fill_stream_buf + streaming_buf_ptr) = 0;

        // Increment the streaming buf pointer.
        streaming_buf_ptr++;
        
    }

    /*if (!sample_delay_count && fade_out && (fade_counter > 0)) {
	    // sprintf(debug_buf, "Fade i: %i", fade_counter);
        // nocashMessage(debug_buf);
        fade_counter--;
    }*/

    
    #ifdef SOUND_DEBUG
	sprintf(debug_buf, "Stream filled, pointer at %li, samples filed %li", streaming_buf_ptr, filled_samples);
    nocashMessage(debug_buf);
    #endif
    // Request a new fill from sound.cpp, refreshing the fill buffer.
    // Ensure that fills are requested only if the streaming buf ptr is more than
    // the filled samples.
    if (!fill_requested && abs(streaming_buf_ptr - filled_samples) >= SAMPLES_PER_FILL) {
        #ifdef SOUND_DEBUG
        nocashMessage("Fill requested!");
        #endif
        fill_requested = true;
    } 
    return length;
}