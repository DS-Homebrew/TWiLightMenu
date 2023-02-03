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
		result =
			useDLDI ? (io_dldi_data->ioInterface.readSectors(*(u32*)0x02FFFA00, *(u32*)0x02FFFA04, (void*)*(u32*)0x02FFFA08) ? 0 : 1)
					: SDMMC_readSectors(SDMMC_DEV_CARD, *(u32*)0x02FFFA00, (void*)*(u32*)0x02FFFA08, *(u32*)0x02FFFA04);
		break;
	case 0x52574453: // SDMMC_SD_WRITE_SECTORS
		/* u32 sector = *(u32*)0x02FFFA00;
		u32 numSectors = *(u32*)0x02FFFA04;
		void* buffer = (void*)*(u32*)0x02FFFA08;
		for (int i = 0; i < numSectors; i++) {
		result =
			useDLDI ? (io_dldi_data->ioInterface.writeSectors(sector+i, 1, buffer+(i*512)) ? 0 : 1)
					: my_sdmmc_writesectors(&deviceSD, sector+i, 1, buffer+(i*512));
		} */
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
