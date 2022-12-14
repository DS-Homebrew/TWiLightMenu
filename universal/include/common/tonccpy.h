//# Stuff you may not have yet.

#ifndef TONCCPY_H
#define TONCCPY_H


#ifdef __cplusplus
extern "C" {
#endif

#include <nds/ndstypes.h>

typedef unsigned int uint;
#define BIT_MASK(len)       ( (1<<(len))-1 )
static inline u32 quad8(u16 x)   {   x |= x<<8; return x | x<<16;    }


//# Declarations and inlines.

void tonccpy(void *dst, const void *src, uint size);

void __toncset(void *dst, u32 fill, uint size);
static inline void toncset(void *dst, u8 src, uint size);
static inline void toncset16(void *dst, u16 src, uint size);
static inline void toncset32(void *dst, u32 src, uint size);


//! VRAM-safe memset, byte version. Size in bytes.
static inline void toncset(void *dst, u8 src, uint size)
{   __toncset(dst, quad8(src), size);               }

//! VRAM-safe memset, halfword version. Size in hwords.
static inline void toncset16(void *dst, u16 src, uint size)
{   __toncset(dst, src|src<<16, size*2);            }

//! VRAM-safe memset, word version. Size in words.
static inline void toncset32(void *dst, u32 src, uint size)
{   __toncset(dst, src, size*4);                    }

#ifdef __cplusplus
}
#endif
#endif
