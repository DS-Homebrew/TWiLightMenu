#ifndef FIND_H
#define FIND_H

#include <nds/ndstypes.h>

#ifdef __cplusplus
extern "C" {
#endif

// COMMON
//u8* memsearch(const u8* start, u32 dataSize, const u8* find, u32 findSize);
u32* memsearch32(const u32* start, u32 dataSize, const u32* find, u32 findSize, bool forward);
u16* memsearch16(const u16* start, u32 dataSize, const u16* find, u32 findSize, bool forward);
u8* memsearch8(const u8* start, u32 dataSize, const u8* find, u32 findSize, bool forward);

#ifdef __cplusplus
}
#endif

#endif // FIND_H
