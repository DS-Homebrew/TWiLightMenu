#include "streamingaudio.h"
#include "common/tonccpy.h"


s16 streaming_buf[STREAMING_BUF_LENGTH + 1] = {0}; // 3600B
s16 streaming_buf_temp[STREAMING_BUF_LENGTH + 1] = {0}; // 3600B

bool fill_requested = true;
u32 filled_samples = 0;
u32 used_samples = 0;

static FILE* stream_source;
char debug_buf[256] = {0};

void set_streaming_source(FILE* source) {
    stream_source = source;
}


/***********************************************************************************
 * on_stream_request
 *
 * Audio stream data request handler.
 ***********************************************************************************/
mm_word on_stream_request(mm_word length, mm_addr dest, mm_stream_formats format) {
	//----------------------------------------------------------------------------------

    if (fill_requested) {
        nocashMessage("missed fill");
    }

	const s16 *music = streaming_buf + used_samples;
	s16 *target = dest;
   
    int len = length;
    tonccpy(target, music, len << 1);
    
   
    used_samples += length;
    sprintf(debug_buf, "fill req %i, filled %i", length, len);
    nocashMessage(debug_buf);

    // Since we fill twice as often as we need, there should never be stutter.
    if (used_samples >= STREAMING_BUF_LENGTH >> 1) {
        fill_requested = true;
    }
    
    return length;
}