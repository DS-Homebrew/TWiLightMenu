#include "streamingaudio.h"
#include "common/tonccpy.h"
#include "menumusic_bin.h"
u32 bin_pointer = 0;

s16 streaming_buf[2800] = {0}; // 3600B
volatile mm_word last_length = 2800;
char debug_buf[256] = {0};

/***********************************************************************************
 * on_stream_request
 *
 * Audio stream data request handler.
 ***********************************************************************************/
mm_word on_stream_request(mm_word length, mm_addr dest, mm_stream_formats format) {
	//----------------------------------------------------------------------------------
	const s16 *music =  menumusic_bin;
	s16 *target = dest;
    // clear buffer
    // if (last_length + length > 2800) {
    //     length = length - ((last_length + length) % 2800);
    // }
    
    sprintf(debug_buf, "fill req %i", length);
    nocashMessage(debug_buf);
    // int len = length;
	int len = length;
	for( ; len; len-- )
	{
		*target++ = music[bin_pointer % (menumusic_bin_size >> 1)];
        bin_pointer++;
        // *target++ = *music++;
    }
    last_length += length;
    return length;
}