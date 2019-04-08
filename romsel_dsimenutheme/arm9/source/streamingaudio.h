#pragma once
#ifndef __TWILIGHTMENU_SOUND_STREAM__
#define __TWILIGHTMENU_SOUND_STREAM__
#include <maxmod9.h>
#include <mm_types.h>
#include <nds.h>
#include <stdio.h>

#define STREAMING_BUF_LENGTH 96000                             // Size in samples (16 bits) => 256000B = 256KB * 2 = 512KB. // samples to keep cached
#define FILL_FACTOR 4
#define SAMPLES_PER_FILL (STREAMING_BUF_LENGTH >> FILL_FACTOR) // Samples to load into the fill buffer per fill.
#define TOTAL_FILLS (1 << FILL_FACTOR)                         // Fills before we stop accepting new data into the fill buffer

// The higher the fill factor the more frequent the fills will be requested.

#ifdef __cplusplus
extern "C" {
#endif

mm_word on_stream_request(mm_word length, mm_addr dest, mm_stream_formats format);

#ifdef __cplusplus
}
#endif
#endif