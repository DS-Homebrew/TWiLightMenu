#ifndef FIND_H
#define FIND_H

#include <nds/ndstypes.h>
#include <nds/memory.h> // tNDSHeader

//extern int readType;

// COMMON
//u8* memsearch(const u8* start, u32 dataSize, const u8* find, u32 findSize);
u32* memsearch32(const u32* start, u32 dataSize, const u32* find, u32 findSize, bool forward);
u16* memsearch16(const u16* start, u32 dataSize, const u16* find, u32 findSize, bool forward);

inline u32* findOffset(const u32* start, u32 dataSize, const u32* find, u32 findLen) {
	return memsearch32(start, dataSize, find, findLen*sizeof(u32), true);
}
inline u32* findOffsetBackwards(const u32* start, u32 dataSize, const u32* find, u32 findLen) {
	return memsearch32(start, dataSize, find, findLen*sizeof(u32), false);
}
inline u16* findOffsetThumb(const u16* start, u32 dataSize, const u16* find, u32 findLen) {
	return memsearch16(start, dataSize, find, findLen*sizeof(u16), true);
}
inline u16* findOffsetBackwardsThumb(const u16* start, u32 dataSize, const u16* find, u32 findLen) {
	return memsearch16(start, dataSize, find, findLen*sizeof(u16), false);
}
#endif // FIND_H
