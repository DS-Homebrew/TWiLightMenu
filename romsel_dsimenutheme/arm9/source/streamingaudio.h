#pragma once
#ifndef __TWILIGHTMENU_SOUND_STREAM__
#define __TWILIGHTMENU_SOUND_STREAM__
#include <maxmod9.h>
#include <mm_types.h>
#include <nds.h>
#include <stdio.h>

// #define SOUND_DEBUG

#define FADE_STEPS 7                                           // Number of fill requests to fade out across when fade out is requested.
#define STREAMING_BUF_LENGTH 96000                             // Size in samples (16 bits) => 96000 samples = 192KB * 2 buffers = 384KB RAM total.
#define FILL_FACTOR 4                                          // The higher the fill factor the more frequent the fills will be requested.
#define SAMPLES_PER_FILL (STREAMING_BUF_LENGTH >> FILL_FACTOR) // Samples to load into the fill buffer per fill.
#define TOTAL_FILLS (1 << FILL_FACTOR)                         // Fills before we stop accepting new data into the fill buffer


#ifdef __cplusplus
extern "C" {
#endif

void alloc_streaming_buf(void);
void free_streaming_buf(void);
void resetStreamSettings();
mm_word on_stream_request(mm_word length, mm_addr dest, mm_stream_formats format);

#ifdef __cplusplus
}
#endif
#endif