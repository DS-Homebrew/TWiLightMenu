#include <nds/disc_io.h>
#include <nds/fifocommon.h>
#include <nds/fifomessages.h>
#include <nds/ipc.h>
#include <nds/bios.h>
#include <nds/system.h>
#include <nds/memory.h>
#include <nds/arm9/cache.h>
#include <nds/arm9/dldi.h>

vu32* sharedAddr = (vu32*)0x02FFFA00;

//---------------------------------------------------------------------------------
bool my_sdio_Startup() {
//---------------------------------------------------------------------------------
	*(vu32*)0x0CFFFA00 = 0x54534554;
	if (*(vu32*)0x0CFFFA00 == 0x54534554) {
		sharedAddr = (vu32*)0x0CFFFA00;
	}

	int result = 0;

	if (sharedAddr[1] == 0x49444C44) {
		sharedAddr[0] = (u32)io_dldi_data;
		sysSetCardOwner(BUS_OWNER_ARM7);
	} else {
		sharedAddr[3] = 0x56484453;
		IPC_SendSync(0);
		while (sharedAddr[3] == 0x56484453) {
			swiDelay(100);
		}
		result = sharedAddr[3];

		if (result==0) return false;
	}

	sharedAddr[3] = 0x54534453;
	IPC_SendSync(1);
	while (sharedAddr[3] == 0x54534453) {
		swiDelay(100);
	}

	result = sharedAddr[3];

	return result == 0;
}

//---------------------------------------------------------------------------------
bool my_sdio_IsInserted() {
//---------------------------------------------------------------------------------
	sharedAddr[3] = 0x4E494453;
	IPC_SendSync(3);
	while (sharedAddr[3] == 0x4E494453) {
		swiDelay(100);
	}
	int result = sharedAddr[3];

	return result == 1;
}

//---------------------------------------------------------------------------------
bool my_sdio_ReadSectors(sec_t sector, sec_t numSectors,void* buffer) {
//---------------------------------------------------------------------------------
	DC_FlushRange(buffer,numSectors * 512);

	sharedAddr[0] = sector;
	sharedAddr[1] = numSectors;
	sharedAddr[2] = (vu32)buffer;
	
	sharedAddr[3] = 0x44524453;
	IPC_SendSync(4);
	while (sharedAddr[3] == 0x44524453) {
		swiDelay(100);
	}

	int result = sharedAddr[3];
	
	return result == 0;
}

//---------------------------------------------------------------------------------
bool my_sdio_WriteSectors(sec_t sector, sec_t numSectors,const void* buffer) {
//---------------------------------------------------------------------------------
	DC_FlushRange(buffer,numSectors * 512);

	sharedAddr[0] = sector;
	sharedAddr[1] = numSectors;
	sharedAddr[2] = (vu32)buffer;
	
	sharedAddr[3] = 0x52574453;
	IPC_SendSync(5);
	while (sharedAddr[3] == 0x52574453) {
		swiDelay(100);
	}

	int result = sharedAddr[3];
	
	return result == 0;
}


//---------------------------------------------------------------------------------
bool my_sdio_ClearStatus() {
//---------------------------------------------------------------------------------
	return true;
}

//---------------------------------------------------------------------------------
bool my_sdio_Shutdown() {
//---------------------------------------------------------------------------------
	return true;
}

const DISC_INTERFACE __my_io_dsisd = {
	DEVICE_TYPE_DSI_SD,
	FEATURE_MEDIUM_CANREAD | FEATURE_MEDIUM_CANWRITE,
	(FN_MEDIUM_STARTUP)&my_sdio_Startup,
	(FN_MEDIUM_ISINSERTED)&my_sdio_IsInserted,
	(FN_MEDIUM_READSECTORS)&my_sdio_ReadSectors,
	(FN_MEDIUM_WRITESECTORS)&my_sdio_WriteSectors,
	(FN_MEDIUM_CLEARSTATUS)&my_sdio_ClearStatus,
	(FN_MEDIUM_SHUTDOWN)&my_sdio_Shutdown
};
