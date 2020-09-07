#ifndef DSI_NDMA_INCLUDE
#define DSI_NDMA_INCLUDE

#include <nds/ndstypes.h>

static inline
/*! \fn void ndmaCopyWords(uint8 channel, const void* src, void* dest, uint32 size)
    \brief copies from source to destination on one of the 4 available channels in words
    \param channel the dma channel to use (0 - 3).
    \param src the source to copy from
    \param dest the destination to copy to
    \param size the size in bytes of the data to copy.  Will be truncated to the nearest word (4 bytes)
*/
void ndmaCopyWords(uint8 ndmaSlot, const void* src, void* dest, uint32 size) {
	u32 addr[5] = {0x4004104, 0x4004108, 0x4004110, 0x4004114, 0x400411C};
	if (ndmaSlot > 0)
	for (int a = 0; a < 5; a++) {
		addr[a] += 0x1C*(ndmaSlot+1);
	}

	*(vu32*)addr[0] = (u32)src;
	*(vu32*)addr[1] = (u32)dest;

	*(vu32*)addr[2] = size/4;	

    *(vu32*)addr[3] = 0x1;

	*(vu32*)addr[4] = 0x90070000;

	while ((*(vu32*)(addr[4]) & BIT(31)) == 0x80000000);
}

static inline
/*! \fn void ndmaCopyWordsAsynch(uint8 channel, const void* src, void* dest, uint32 size)
\brief copies from source to destination on one of the 4 available channels in half words.  
This function returns immediately after starting the transfer.
\param channel the dma channel to use (0 - 3).
\param src the source to copy from
\param dest the destination to copy to
\param size the size in bytes of the data to copy.  Will be truncated to the nearest word (4 bytes)
*/
void ndmaCopyWordsAsynch(uint8 ndmaSlot, const void* src, void* dest, uint32 size) {
	u32 addr[5] = {0x4004104, 0x4004108, 0x4004110, 0x4004114, 0x400411C};
	if (ndmaSlot > 0)
	for (int a = 0; a < 5; a++) {
		addr[a] += 0x1C*(ndmaSlot+1);
	}

	*(vu32*)addr[0] = (u32)src;
	*(vu32*)addr[1] = (u32)dest;

	*(vu32*)addr[2] = size/4;	

    *(vu32*)addr[3] = 0x1;

	*(vu32*)addr[4] = 0x90070000;
}

static inline 
bool ndmaBusy(uint8 ndmaSlot) {
	switch (ndmaSlot) {
		case 0:
		default:
			return	(*(vu32*)(0x400411C) & BIT(31)) == 0x80000000;
		case 1:
			return	(*(vu32*)(0x4004130) & BIT(31)) == 0x80000000;
		case 2:
			return	(*(vu32*)(0x400414C) & BIT(31)) == 0x80000000;
		case 3:
			return	(*(vu32*)(0x4004160) & BIT(31)) == 0x80000000;
	}
}

static inline 
bool ndmaEnabled(void) {
	return (REG_SCFG_EXT & BIT(16));
}

#endif