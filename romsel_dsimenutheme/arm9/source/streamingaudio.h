#pragma once
#ifndef __TWILIGHTMENU_SOUND_STREAM__
#define __TWILIGHTMENU_SOUND_STREAM__
#include <maxmod9.h>
#include <mm_types.h>
#include <nds.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

mm_word on_stream_request(mm_word length, mm_addr dest, mm_stream_formats format);
#ifdef __cplusplus
}
#endif
#endif