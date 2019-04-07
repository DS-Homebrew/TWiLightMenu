#pragma once
#ifndef __TWILIGHTMENU_SOUND_STREAM__
#define __TWILIGHTMENU_SOUND_STREAM__
#include <maxmod9.h>
#include <mm_types.h>
#include <nds.h>
#include <stdio.h>

#define STREAMING_BUF_LENGTH 25600 
#ifdef __cplusplus
extern "C" {
#endif

mm_word on_stream_request(mm_word length, mm_addr dest, mm_stream_formats format);
void set_streaming_source(FILE* source);
#ifdef __cplusplus
}
#endif
#endif