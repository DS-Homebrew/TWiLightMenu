#include "streamingaudio.h"
#include "common/tonccpy.h"

// Private members

// The main stream buffer that is streamed to maxmod
static volatile s16 streaming_buf_main[STREAMING_BUF_LENGTH] = {0}; 

/* The swap buffer, which is filled with bytes from the file
 * before it is streamed to maxmod.
 */
static volatile s16 streaming_buf_swap[STREAMING_BUF_LENGTH] = {0}; 

/* Not a actual pointer, but the index to the current location of the 
 * buffers that are being streamed.
 */
volatile u32 streaming_buf_ptr = 0;

// Pointers to the stream buffers.
volatile s16* play_stream_buf = streaming_buf_main;
volatile s16* fill_stream_buf = streaming_buf_swap;


// Toggle this to true to trigger a fill as soon as possible.
volatile bool fill_requested = false;

// Number of samples that must be streamed until the next filll
volatile s32 samples_left_until_next_fill = 0;

volatile u16 fade_counter = FADE_STEPS;
volatile bool fade_out = false;

volatile u32 sample_delay_count = 0;
char debug_buf[256] = {0};


/*
 * The maxmod stream request handler. 
 * 
 * This method is called automatically by maxmod at random times.
 * 
 * While streaming from RAM is a trivial matter, streaming from
 * a file is a bit more involved because SD access much slower.
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
   
    for (; sample_delay_count && len; len--, sample_delay_count--) {
        *target++ = 0;
    }

    for (; len; len--)
	{
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
        
        // Count down the next time until we request a fill.
        if (samples_left_until_next_fill > 0) {
            samples_left_until_next_fill--;
        }
    }

    if (!sample_delay_count && fade_out && (fade_counter > 0)) {
	    sprintf(debug_buf, "Fade i: %i", fade_counter);
        nocashMessage(debug_buf);
        fade_counter--;
    }

	sprintf(debug_buf, "Stream filled, %li until next fill", samples_left_until_next_fill);
    nocashMessage(debug_buf);
    
    // Request a new fill from sound.cpp, refreshing the fill buffer.
    if (!fill_requested && samples_left_until_next_fill <= 0) {
        nocashMessage("Fill requested!");
        fill_requested = true;
    } 
    return length;
}