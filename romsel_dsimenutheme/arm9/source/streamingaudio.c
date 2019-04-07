#include "streamingaudio.h"
#include "common/tonccpy.h"


volatile s16 streaming_buf_main[STREAMING_BUF_LENGTH] = {0}; // 3600B
volatile s16 streaming_buf_swap[STREAMING_BUF_LENGTH] = {0}; // 3600B

volatile s16* curr_stream_buf = streaming_buf_main;
volatile s16* next_stream_buf = streaming_buf_swap;


volatile u32 streaming_buf_ptr = 0;
volatile s32 samples_left_until_next_fill = SAMPLES_PER_FILL;
volatile u32 filled_samples = 0;
// s16 streaming_buf_temp[STREAMING_BUF_LENGTH + 1] = {0}; // 3600B

bool fill_requested = false;
bool pop_fill_queue = false;

volatile u16 fill_count = 0;

#define SAMPLES_USED (STREAMING_BUF_LENGTH - samples_left)
#define REFILL_THRESHOLD STREAMING_BUF_LENGTH >> 2
static FILE* stream_source;
char debug_buf[256] = {0};

void set_streaming_source(FILE* source) {
    stream_source = source;
}
void swap_stream_buffers() {
    if (curr_stream_buf == streaming_buf_main) {
        curr_stream_buf = streaming_buf_swap;
        next_stream_buf = streaming_buf_main;
    } else {
        curr_stream_buf = streaming_buf_main;
        next_stream_buf = streaming_buf_swap;
    }
    streaming_buf_ptr = 0;
    fill_count = 0;
    filled_samples = 0;
    toncset32((void*)next_stream_buf, 0, sizeof(streaming_buf_main) << 2);
    nocashMessage("buffers swapped");
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
        // return 0;
    }
    // if (samples_left <= 0) {
    //     nocashMessage("no samples left");
    //     fill_requested = true;
    //     return 0;
    // }

	s16 *target = dest;
   
    int len = length;
     for (int i = 0 ; i < len; i++ )
	{
        if (streaming_buf_ptr >= STREAMING_BUF_LENGTH) {
            swap_stream_buffers();
        }
        // if (*music == 0) break;
		*target++ = *(curr_stream_buf + streaming_buf_ptr);
        streaming_buf_ptr++;
        
        if (samples_left_until_next_fill > 0) {
            samples_left_until_next_fill--;
        }
        // *target++ = *music++;
    }

	sprintf(debug_buf, "Stream filled, %li until next fill", samples_left_until_next_fill);
    nocashMessage(debug_buf);
    
    if (!fill_requested && samples_left_until_next_fill <= 0) {
        nocashMessage("Fill requested!");
        fill_requested = true;
    }
     // samples_left -= length;

    // sprintf(debug_buf, "Filled %i, remaining %li, used %li", length, samples_left, SAMPLES_USED);
    // nocashMessage(debug_buf);
    // // Since we fill four times as often as we need, there should never be stutter.
    // if (SAMPLES_USED > REFILL_THRESHOLD) {
    //     fill_requested = true;
    //     nocashMessage("fill requested");
    // }
    
    return length;
}