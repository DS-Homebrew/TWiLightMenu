#include <nds/system.h>
#include <nds/bios.h>
#include "my_sdmmc.h"
#include "tmio.h"
#include <nds/arm9/dldi.h>
#include <nds/interrupts.h>
#include <nds/ipc.h>
#include <nds/fifocommon.h>
#include <nds/fifomessages.h>

#include <string.h>
#include <stddef.h>

static inline u32 intLog2(u32 val)
{
	// The result is undefined if __builtin_clzl() is called with 0.
	return (val ? 31u - __builtin_clzl(val) : val);
}

#define NDMA_REGS_BASE  (0x4004100)
#define REG_NDMA_GCNT   *((vu32*)NDMA_REGS_BASE) // Global control.

// Note: The channel regs are offset by 4 (REG_NDMA_GCNT).
typedef struct
{
	vu32 sad;   // 0x00 Source address.
	vu32 dad;   // 0x04 Destination address.
	vu32 tcnt;  // 0x08 Total repeat length in words.
	vu32 wcnt;  // 0x0C Logical block size in words.
	vu32 bcnt;  // 0x10 Block transfer timing/interval.
	vu32 fdata; // 0x14 Fill data.
	vu32 cnt;   // 0x18 Control.
} NdmaCh;
static_assert(offsetof(NdmaCh, cnt) == 0x18, "Error: Member cnt of NdmaCh is not at offset 0x18!");

static inline NdmaCh* getNdmaChRegs(u8 c)
{
	return &((NdmaCh*)(NDMA_REGS_BASE + 4))[c];
}


// REG_NDMA_GCNT
#define NDMA_REG_READBACK    (1u) // Show internal state on REG_NDMAx_SAD/DAD/TCNT/WCNT. 3DS mode only.
#define NDMA_ROUND_ROBIN(n)  (1u<<31 | (intLog2(n) + 1)<<16) // DSP DMA/CPU cycles (power of 2). Maximum 16384.
#define NDMA_HIGHEST_PRIO    (0u)

// REG_NDMA_BCNT
// TODO: When is the delay happening during transfers?
// TODO: Does NDMA run at 67 MHz in 3DS mode? We will assume so for now.
#define NDMA_CYCLES(n)       (n)      // Maximum 65535. 0 means no delay/interval.
#define NDMA_PRESCALER_1     (0u)     // 67027964 Hz.
#define NDMA_PRESCALER_4     (1u<<16) // 16756991 Hz.
#define NDMA_PRESCALER_16    (2u<<16) // 4189247.75 Hz.
#define NDMA_PRESCALER_64    (3u<<16) // 1047311.9375 Hz.
#define NDMA_FASTEST         (NDMA_PRESCALER_1 | NDMA_CYCLES(0)) // Convenience macro.

// REG_NDMA_CNT
#define NDMA_DAD_INC         (0u)     // Destination address increment.
#define NDMA_DAD_DEC         (1u<<10) // Destination address decrement.
#define NDMA_DAD_FIX         (2u<<10) // Destination address fixed.
#define NDMA_DAD_RELOAD      (1u<<12) // Reload destination address on logical block end (REG_NDMAx_WCNT).
#define NDMA_SAD_INC         (0u)     // Source address increment.
#define NDMA_SAD_DEC         (1u<<13) // Source address decrement.
#define NDMA_SAD_FIX         (2u<<13) // Source address fixed.
#define NDMA_SAD_FILL        (3u<<13) // Source is REG_NDMAx_FDATA.
#define NDMA_SAD_RELOAD      (1u<<15) // Reload source address on logical block end (REG_NDMAx_WCNT).
#define NDMA_BURST_SHIFT     (16u)
#define NDMA_BURST(n)        (intLog2(n)<<NDMA_BURST_SHIFT) // Burst length is 2n words. Maximum 32768. Must be power of 2.
#define NDMA_TCNT_MODE       (0u)     // REG_NDMAx_TCNT mode.
#define NDMA_REPEAT_MODE     (1u<<29) // Repeat transfer infinitely.
#define NDMA_IRQ_EN          (1u<<30) // IRQ enable.
#define NDMA_EN              (1u<<31) // Channel enable/active.


enum
{
	NDMA_START_TIMER0       =  0u<<24,
	NDMA_START_TIMER1       =  1u<<24,
	NDMA_START_TIMER2       =  2u<<24,
	NDMA_START_TIMER3       =  3u<<24,
	NDMA_START_SPICARD      =  4u<<24,
	NDMA_START_SPICARD2     =  5u<<24,
	NDMA_START_VBLANK       =  6u<<24,
	NDMA_START_NDSWIFI      =  7u<<24,
	NDMA_START_SDMMC        =  8u<<24,
	NDMA_START_DSIWIFI      =  9u<<24,
	NDMA_START_AES_IN       = 10u<<24, // AES write fifo.
	NDMA_START_AES_OUT      = 11u<<24, // AES read fifo.
	NDMA_START_MIC          = 12u<<24,

	// This bit is technically not part of the startup modes.
	NDMA_START_IMMEDIATE    = 16u<<24
};


//---------------------------------------------------------------------------------
static inline u16 my_sdmmc_read16(u16 reg) {
//---------------------------------------------------------------------------------
	return *(vu16*)(0x04004800 + reg);
}

//---------------------------------------------------------------------------------
int my_sdmmc_cardinserted() {
//---------------------------------------------------------------------------------
	return 1; //my_sdmmc_cardready;
}

static bool useDLDI = false;
bool devCardInited = false;
bool devEMMCInited = false;

//---------------------------------------------------------------------------------
void my_sdmmcHandler() {
//---------------------------------------------------------------------------------
	int result = 0;
	int sdflag = 0;
	//int oldIME = enterCriticalSection();

	switch(*(u32*)0x02FFFA0C) {

	case 0x56484453: // SDMMC_HAVE_SD
		result = (my_sdmmc_read16(REG_SDSTATUS0) & BIT(5)) != 0;
		break;

	case 0x54534453: // SDMMC_SD_START
		sdflag = 1;
		/* Falls through. */
	case 0x5453414E: // SDMMC_NAND_START
		if (my_sdmmc_read16(REG_SDSTATUS0) == 0) {
			result = 1;
		} else {
			TMIO_init();
			if (*(u32*)0x02FFFD78 == 0x56524453) {
				result = SDMMC_importDevState(SDMMC_DEV_CARD, (u8*)0x02FFFA80);
			} else if (*(u32*)0x02FFFD7C == 0x5652444E) {
				result = SDMMC_importDevState(SDMMC_DEV_eMMC, (u8*)0x02FFFAC0);
			} else {
				result = SDMMC_init((sdflag == 1) ? SDMMC_DEV_CARD : SDMMC_DEV_eMMC);
				(sdflag == 1) ? (*(u32*)0x02FFFD78 = 0x56524453) : (*(u32*)0x02FFFD7C = 0x5652444E);
			}
		}
		if (sdflag == 1 && result != 0 && *(u32*)(0x2FFFA04) == 0x49444C44) {
			if (*(vu32*)(0x303C000) != 0x49444C44) {
				memcpy((void*)io_dldi_data, (void*)*(u32*)0x2FFFA00, 0x4000);
				dldiFixDriverAddresses((DLDI_INTERFACE*)io_dldi_data);
				*(vu32*)(0x303C000) = 0x49444C44;
			}
			useDLDI = io_dldi_data->ioInterface.startup();
			if (useDLDI) {
				result = 0;
			}
		}
		break;

	case 0x4E494453: // SDMMC_SD_IS_INSERTED
		result = useDLDI ? io_dldi_data->ioInterface.isInserted() : my_sdmmc_cardinserted();
		break;

	//case SDMMC_SD_STOP:
	//	break;

	//case 0x5A53414E: // SDMMC_NAND_SIZE
	//	result = deviceNAND.total_size;
	//	break;

	case 0x44524453: // SDMMC_SD_READ_SECTORS
		if (useDLDI) {
			result = io_dldi_data->ioInterface.readSectors(*(u32*)0x02FFFA00, *(u32*)0x02FFFA04, (void*)*(u32*)0x02FFFA08) ? 0 : 1;
		} else {
			u32 sector = *(u32*)0x02FFFA00;
			u32 count = *(u32*)0x02FFFA04;

			NdmaCh *const ndmaCh = getNdmaChRegs(0);
			ndmaCh->sad  = (u32)getTmioFifo(getTmioRegs(SDMMC_DEV_CARD)); // TODO: SDMMC dev to FIFO function.
			ndmaCh->dad  = *(u32*)0x02FFFA08;
			ndmaCh->wcnt = 512 / 4;
			ndmaCh->bcnt = NDMA_FASTEST;
			ndmaCh->cnt  = NDMA_EN | NDMA_START_SDMMC | NDMA_REPEAT_MODE |
						   NDMA_BURST(64 / 4) | NDMA_SAD_FIX | NDMA_DAD_INC;

			do
			{
				const u16 blockCount = (count > 0xFFFF ? 0xFFFF : count);
				if(SDMMC_readSectors(SDMMC_DEV_CARD, sector, NULL, blockCount) != SDMMC_ERR_NONE)
				{
					result = 1;
					break;
				}

				sector += blockCount;
				count -= blockCount;
			} while(count > 0);

			// Stop DMA.
			ndmaCh->cnt = 0;
			result = 0;
		}
		break;
	case 0x52574453: // SDMMC_SD_WRITE_SECTORS
		result =
			useDLDI ? (io_dldi_data->ioInterface.writeSectors(*(u32*)0x02FFFA00, *(u32*)0x02FFFA04, (void*)*(u32*)0x02FFFA08) ? 0 : 1)
					: SDMMC_writeSectors(SDMMC_DEV_CARD, *(u32*)0x02FFFA00, (void*)*(u32*)0x02FFFA08, *(u32*)0x02FFFA04);
		break;
	case 0x44454453:
		result = SDMMC_exportDevState(SDMMC_DEV_CARD, (u8*)0x02FFFA80);
		break;
	/* case 0x44494453:
		result = SDMMC_importDevState(SDMMC_DEV_CARD, (u8*)0x02FFFD80);
		break; */
	case 0x4452414E: // SDMMC_NAND_READ_SECTORS
		result = SDMMC_readSectors(SDMMC_DEV_eMMC, *(u32*)0x02FFFA00, (void*)*(u32*)0x02FFFA08, *(u32*)0x02FFFA04);
		break;
	case 0x5257414E: // SDMMC_NAND_WRITE_SECTORS
		result = SDMMC_writeSectors(SDMMC_DEV_eMMC, *(u32*)0x02FFFA00, (void*)*(u32*)0x02FFFA08, *(u32*)0x02FFFA04);
		break;
	case 0x4445414E:
		result = SDMMC_exportDevState(SDMMC_DEV_eMMC, (u8*)0x02FFFAC0);
		break;
	}

	//leaveCriticalSection(oldIME);

	//fifoSendValue32(FIFO_SDMMC, result);
	*(u32*)0x02FFFA0C = result;
}
