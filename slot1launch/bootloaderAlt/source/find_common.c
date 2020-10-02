//#include <string.h> // memcmp
#include <stddef.h> // NULL
#include <nds/ndstypes.h>
//#include <limits.h>
#include "find.h"

// (memcmp is slower)
//#define memcmp __builtin_memcmp

//#define TABLE_SIZE (UCHAR_MAX + 1) // 256

extern inline u32* findOffset(const u32* start, u32 dataLen, const u32* find, u32 findLen);
extern inline u32* findOffsetBackwards(const u32* start, u32 dataLen, const u32* find, u32 findLen);
extern inline u16* findOffsetThumb(const u16* start, u32 dataLen, const u16* find, u32 findLen);
extern inline u16* findOffsetBackwardsThumb(const u16* start, u32 dataLen, const u16* find, u32 findLen);

/*
*   Look for @find and return the position of it.
*   Brute Force algorithm
*/
u32* memsearch32(const u32* start, u32 dataSize, const u32* find, u32 findSize, bool forward) {
	u32 dataLen = dataSize/sizeof(u32);
	u32 findLen = findSize/sizeof(u32);

	const u32* end = forward ? (start + dataLen) : (start - dataLen);
	for (u32* addr = (u32*)start; addr != end; forward ? ++addr : --addr) {
		bool found = true;
		for (u32 j = 0; j < findLen; ++j) {
			if (addr[j] != find[j]) {
				found = false;
				break;
			}
		}
		if (found) {
			return (u32*)addr;
		}
	}
	return NULL;
}
u16* memsearch16(const u16* start, u32 dataSize, const u16* find, u32 findSize, bool forward) {
	u32 dataLen = dataSize/sizeof(u16);
	u32 findLen = findSize/sizeof(u16);

	const u16* end = forward ? (start + dataLen) : (start - dataLen);
	for (u16* addr = (u16*)start; addr != end; forward ? ++addr : --addr) {
		bool found = true;
		for (u32 j = 0; j < findLen; ++j) {
			if (addr[j] != find[j]) {
				found = false;
				break;
			}
		}
		if (found) {
			return (u16*)addr;
		}
	}
	return NULL;
}

/*
*   Boyer-Moore Horspool algorithm
*/
/*u8* memsearch(const u8* start, u32 dataSize, const u8* find, u32 findSize) {
	u32 dataLen = dataSize/sizeof(u8);
	u32 findLen = findSize/sizeof(u8);

	u32 table[TABLE_SIZE];

	// Preprocessing
	for (u32 i = 0; i < TABLE_SIZE; ++i) {
		table[i] = findLen;
	}
	for (u32 i = 0; i < findLen - 1; ++i) {
		table[find[i]] = findLen - i - 1;
	}

	// Searching
	u32 j = 0;
	while (j <= dataLen - findLen) {
		u8 c = start[j + findLen - 1];
		if (find[findLen - 1] == c && memcmp(find, start + j, findLen - 1) == 0) {
			return (u8*)start + j;
		}
		j += table[c];
	}

	return NULL;
}*/

/*
*   Quick Search algorithm
*/
/*u8* memsearch(const u8* start, u32 dataSize, const u8* find, u32 findSize) {
	u32 dataLen = dataSize/sizeof(u8);
	u32 findLen = findSize/sizeof(u8);

	u32 table[TABLE_SIZE];

	// Preprocessing
	for (u32 i = 0; i < TABLE_SIZE; ++i) {
		table[i] = findLen + 1;
	}
	for (u32 i = 0; i < findLen; ++i) {
		table[find[i]] = findLen - i;
	}

	// Searching
	u32 j = 0;
	while (j <= dataLen - findLen) {
		if (memcmp(find, start + j, findLen) == 0) {
			return (u8*)start + j;
		}
		j += table[start[j + findLen]];
	}

	return NULL;
}*/
