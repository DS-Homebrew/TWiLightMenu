#include <nds/system.h>
#include <nds/bios.h>
#include "my_sdmmc.h"
#include <nds/arm9/dldi.h>
#include <nds/interrupts.h>
#include <nds/ipc.h>
#include <nds/fifocommon.h>
#include <nds/fifomessages.h>

#include <string.h>
#include <stddef.h>

static struct mmcdevice deviceSD;
static struct mmcdevice deviceNAND;

static bool useDLDI = false;

/*mmcdevice *getMMCDevice(int drive) {
	if (drive==0) return &deviceNAND;
	return &deviceSD;
}
*/

//---------------------------------------------------------------------------------
int my_geterror(struct mmcdevice *ctx) {
//---------------------------------------------------------------------------------
	//if (ctx->error == 0x4) return -1;
	//else return 0;
	return (ctx->error << 29) >> 31;
}


//---------------------------------------------------------------------------------
void my_setTarget(struct mmcdevice *ctx) {
//---------------------------------------------------------------------------------
	sdmmc_mask16(REG_SDPORTSEL,0x3,(u16)ctx->devicenumber);
	setckl(ctx->clk);
	if (ctx->SDOPT == 0) {
		sdmmc_mask16(REG_SDOPT, 0, 0x8000);
	} else {
		sdmmc_mask16(REG_SDOPT, 0x8000, 0);
	}

}


//---------------------------------------------------------------------------------
void my_sdmmc_send_command(struct mmcdevice *ctx, uint32_t cmd, uint32_t args) {
//---------------------------------------------------------------------------------
	const bool getSDRESP = (cmd << 15) >> 31;
	u16 flags = (cmd << 15) >> 31;
	const bool readdata = cmd & 0x20000;
	const bool writedata = cmd & 0x40000;

	if (readdata || writedata) {
		flags |= TMIO_STAT0_DATAEND;
	}

	ctx->error = 0;
	while ((sdmmc_read16(REG_SDSTATUS1) & TMIO_STAT1_CMD_BUSY)); //mmc working?
	sdmmc_write16(REG_SDIRMASK0,0);
	sdmmc_write16(REG_SDIRMASK1,0);
	sdmmc_write16(REG_SDSTATUS0,0);
	sdmmc_write16(REG_SDSTATUS1,0);
	sdmmc_mask16(REG_SDDATACTL32,0x1800,0x400); // Disable TX32RQ and RX32RDY IRQ. Clear fifo.
	sdmmc_write16(REG_SDCMDARG0,args &0xFFFF);
	sdmmc_write16(REG_SDCMDARG1,args >> 16);
	sdmmc_write16(REG_SDCMD,cmd &0xFFFF);

	u32 size = ctx->size;
	const u16 blkSize = sdmmc_read16(REG_SDBLKLEN32);
	u32 *rDataPtr32 = (u32*)ctx->rData;
	u8  *rDataPtr8  = ctx->rData;
	const u32 *tDataPtr32 = (u32*)ctx->tData;
	const u8  *tDataPtr8  = ctx->tData;

	bool rUseBuf = ( NULL != rDataPtr32 );
	bool tUseBuf = ( NULL != tDataPtr32 );

	u16 status0 = 0;
	while (1) {
		volatile u16 status1 = sdmmc_read16(REG_SDSTATUS1);
#ifdef DATA32_SUPPORT
		volatile u16 ctl32 = sdmmc_read16(REG_SDDATACTL32);
		if ((ctl32 & 0x100))
#else
		if ((status1 & TMIO_STAT1_RXRDY))
#endif
		{
			if (readdata) {
				if (rUseBuf) {
					sdmmc_mask16(REG_SDSTATUS1, TMIO_STAT1_RXRDY, 0);
					if (size >= blkSize) {
						#ifdef DATA32_SUPPORT
						if (!((u32)rDataPtr32 & 3)) {
							for (u32 i = 0; i < blkSize; i += 4) {
								*rDataPtr32++ = sdmmc_read32(REG_SDFIFO32);
							}
						} else {
							for (u32 i = 0; i < blkSize; i += 4) {
								u32 data = sdmmc_read32(REG_SDFIFO32);
								*rDataPtr8++ = data;
								*rDataPtr8++ = data >> 8;
								*rDataPtr8++ = data >> 16;
								*rDataPtr8++ = data >> 24;
							}
						}
						#else
						if (!((u32)rDataPtr16 & 1)) {
							for (u32 i = 0; i < blkSize; i += 4) {
								*rDataPtr16++ = sdmmc_read16(REG_SDFIFO);
							}
						} else {
							for (u32 i = 0; i < blkSize; i += 4) {
								u16 data = sdmmc_read16(REG_SDFIFO);
								*rDataPtr8++ = data;
								*rDataPtr8++ = data >> 8;
							}
						}
						#endif
						size -= blkSize;
					}
				}

				sdmmc_mask16(REG_SDDATACTL32, 0x800, 0);
			}
		}
#ifdef DATA32_SUPPORT
		if (!(ctl32 & 0x200))
#else
		if ((status1 & TMIO_STAT1_TXRQ))
#endif
		{
			if (writedata) {
				if (tUseBuf) {
					sdmmc_mask16(REG_SDSTATUS1, TMIO_STAT1_TXRQ, 0);
					if (size >= blkSize) {
						#ifdef DATA32_SUPPORT
						if (!((u32)tDataPtr32 & 3)) {
							for (u32 i = 0; i < blkSize; i += 4) {
								sdmmc_write32(REG_SDFIFO32, *tDataPtr32++);
							}
						} else {
							for (u32 i = 0; i < blkSize; i += 4) {
								u32 data = *tDataPtr8++;
								data |= (u32)*tDataPtr8++ << 8;
								data |= (u32)*tDataPtr8++ << 16;
								data |= (u32)*tDataPtr8++ << 24;
								sdmmc_write32(REG_SDFIFO32, data);
							}
						}
						#else
						if (!((u32)tDataPtr16 & 1)) {
							for (u32 i = 0; i < blkSize; i += 2) {
								sdmmc_write16(REG_SDFIFO, *tDataPtr16++);
							}
						} else {
							for (u32 i = 0; i < blkSize; i += 2) {
								u16 data = *tDataPtr8++;
								data |= (u16)(*tDataPtr8++ << 8);
								sdmmc_write16(REG_SDFIFO, data);
							}
						}
						#endif
						size -= blkSize;
					}
				}

				sdmmc_mask16(REG_SDDATACTL32, 0x1000, 0);
			}
		}
		if (status1 & TMIO_MASK_GW) {
			ctx->error |= 4;
			break;
		}

		if (!(status1 & TMIO_STAT1_CMD_BUSY)) {
			status0 = sdmmc_read16(REG_SDSTATUS0);
			if (sdmmc_read16(REG_SDSTATUS0) & TMIO_STAT0_CMDRESPEND) {
				ctx->error |= 0x1;
			}
			if (status0 & TMIO_STAT0_DATAEND) {
				ctx->error |= 0x2;
			}

			if ((status0 & flags) == flags)
				break;
		}
	}
	ctx->stat0 = sdmmc_read16(REG_SDSTATUS0);
	ctx->stat1 = sdmmc_read16(REG_SDSTATUS1);
	sdmmc_write16(REG_SDSTATUS0,0);
	sdmmc_write16(REG_SDSTATUS1,0);

	if (getSDRESP != 0) {
		ctx->ret[0] = (u32)(sdmmc_read16(REG_SDRESP0) | (sdmmc_read16(REG_SDRESP1) << 16));
		ctx->ret[1] = (u32)(sdmmc_read16(REG_SDRESP2) | (sdmmc_read16(REG_SDRESP3) << 16));
		ctx->ret[2] = (u32)(sdmmc_read16(REG_SDRESP4) | (sdmmc_read16(REG_SDRESP5) << 16));
		ctx->ret[3] = (u32)(sdmmc_read16(REG_SDRESP6) | (sdmmc_read16(REG_SDRESP7) << 16));
	}
}


//---------------------------------------------------------------------------------
int my_sdmmc_cardinserted() {
//---------------------------------------------------------------------------------
	return 1; //my_sdmmc_cardready;
}


static bool my_sdmmc_controller_initialised = false;

//---------------------------------------------------------------------------------
void my_sdmmc_controller_init( bool force_init ) {
//---------------------------------------------------------------------------------

	if (!force_init && my_sdmmc_controller_initialised) return;

	deviceSD.isSDHC = 0;
	deviceSD.SDOPT = 0;
	deviceSD.res = 0;
	deviceSD.initarg = 0;
	deviceSD.clk = 0x80;
	deviceSD.devicenumber = 0;

	deviceNAND.isSDHC = 0;
	deviceNAND.SDOPT = 0;
	deviceNAND.res = 0;
	deviceNAND.initarg = 1;
	deviceNAND.clk = 0x80;
	deviceNAND.devicenumber = 1;

	*(vu16*)(SDMMC_BASE + REG_SDDATACTL32) &= 0xF7FFu;
	*(vu16*)(SDMMC_BASE + REG_SDDATACTL32) &= 0xEFFFu;
#ifdef DATA32_SUPPORT
	*(vu16*)(SDMMC_BASE + REG_SDDATACTL32) |= 0x402u;
#else
	*(vu16*)(SDMMC_BASE + REG_SDDATACTL32) |= 0x402u;
#endif
	*(vu16*)(SDMMC_BASE + REG_SDDATACTL) = (*(vu16*)(SDMMC_BASE + REG_SDDATACTL) & 0xFFDD) | 2;
#ifdef DATA32_SUPPORT
	*(vu16*)(SDMMC_BASE + REG_SDDATACTL32) &= 0xFFFFu;
	*(vu16*)(SDMMC_BASE + REG_SDDATACTL) &= 0xFFDFu;
	*(vu16*)(SDMMC_BASE + REG_SDBLKLEN32) = 512;
#else
	*(vu16*)(SDMMC_BASE + REG_SDDATACTL32) &= 0xFFFDu;
	*(vu16*)(SDMMC_BASE + REG_SDDATACTL) &= 0xFFDDu;
	*(vu16*)(SDMMC_BASE + REG_SDBLKLEN32) = 0;
#endif
	*(vu16*)(SDMMC_BASE + REG_SDBLKCOUNT32) = 1;
	*(vu16*)(SDMMC_BASE + REG_SDRESET) &= 0xFFFEu;
	*(vu16*)(SDMMC_BASE + REG_SDRESET) |= 1u;
	*(vu16*)(SDMMC_BASE + REG_SDIRMASK0) |= TMIO_MASK_ALL;
	*(vu16*)(SDMMC_BASE + REG_SDIRMASK1) |= TMIO_MASK_ALL>>16;
	*(vu16*)(SDMMC_BASE + 0x0fc) |= 0xDBu; //SDCTL_RESERVED7
	*(vu16*)(SDMMC_BASE + 0x0fe) |= 0xDBu; //SDCTL_RESERVED8
	*(vu16*)(SDMMC_BASE + REG_SDPORTSEL) &= 0xFFFCu;
#ifdef DATA32_SUPPORT
	*(vu16*)(SDMMC_BASE + REG_SDCLKCTL) = 0x20;
	*(vu16*)(SDMMC_BASE + REG_SDOPT) = 0x40EE;
#else
	*(vu16*)(SDMMC_BASE + REG_SDCLKCTL) = 0x40; //Nintendo sets this to 0x20
	*(vu16*)(SDMMC_BASE + REG_SDOPT) = 0x40EB; //Nintendo sets this to 0x40EE
#endif
	*(vu16*)(SDMMC_BASE + REG_SDPORTSEL) &= 0xFFFCu;
	*(vu16*)(SDMMC_BASE + REG_SDBLKLEN) = 512;
	*(vu16*)(SDMMC_BASE + REG_SDSTOP) = 0;

	my_sdmmc_controller_initialised = true;

	my_setTarget(&deviceSD);
}

//---------------------------------------------------------------------------------
static u32 calcSDSize(u8* csd, int type) {
//---------------------------------------------------------------------------------
	u32 result = 0;
	if (type == -1) type = csd[14] >> 6;
	switch (type) {
		case 0:
			{
				u32 block_len = csd[9] & 0xf;
				block_len = 1 << block_len;
				u32 mult = (csd[4] >> 7) | ((csd[5] & 3) << 1);
				mult = 1 << (mult + 2);
				result = csd[8] & 3;
				result = (result << 8) | csd[7];
				result = (result << 2) | (csd[6] >> 6);
				result = (result + 1) * mult * block_len / 512;
			}
			break;
		case 1:
			result = csd[7] & 0x3f;
			result = (result << 8) | csd[6];
			result = (result << 8) | csd[5];
			result = (result + 1) * 1024;
			break;
	}
	return result;
}

//---------------------------------------------------------------------------------
int my_sdmmc_sdcard_init() {
//---------------------------------------------------------------------------------
	// We need to send at least 74 clock pulses before talking to the card.
	my_setTarget(&deviceSD);
	// TMIO base clock is half of the CPU clock so 2 CPU cycles = 1 base clock pulse.
	// cycles = 2 * [TMIO clock divider] * 74
	swiDelay(2 * 128 * 74);

	// card reset
	my_sdmmc_send_command(&deviceSD,0,0);

	// CMD8 0x1AA
	my_sdmmc_send_command(&deviceSD,0x10408,0x1AA);
	u32 temp = (deviceSD.error & 0x1) << 0x1E;

	u32 temp2 = 0;
	do {
		do {
			// CMD55
			my_sdmmc_send_command(&deviceSD,0x10437,deviceSD.initarg << 0x10);
			// ACMD41
			my_sdmmc_send_command(&deviceSD,0x10769,0x00FF8000 | temp);
			temp2 = 1;
		} while ( !(deviceSD.error & 1) );

	} while ((deviceSD.ret[0] & 0x80000000) == 0);

	if (!((deviceSD.ret[0] >> 30) & 1) || !temp)
		temp2 = 0;

	deviceSD.isSDHC = temp2;

	my_sdmmc_send_command(&deviceSD,0x10602,0);
	if (deviceSD.error & 0x4) return -1;

	my_sdmmc_send_command(&deviceSD,0x10403,0);
	if (deviceSD.error & 0x4) return -1;
	deviceSD.initarg = deviceSD.ret[0] >> 0x10;

	my_sdmmc_send_command(&deviceSD,0x10609,deviceSD.initarg << 0x10);
	if (deviceSD.error & 0x4) return -1;

	deviceSD.total_size = calcSDSize((u8*)&deviceSD.ret[0],-1);
	deviceSD.clk = 1;
	setckl(1);

	my_sdmmc_send_command(&deviceSD,0x10507,deviceSD.initarg << 0x10);
	if (deviceSD.error & 0x4) return -1;

	// CMD55
	my_sdmmc_send_command(&deviceSD,0x10437,deviceSD.initarg << 0x10);
	if (deviceSD.error & 0x4) return -1;

	// ACMD42
	my_sdmmc_send_command(&deviceSD,0x1076A,0x0);
	if (deviceSD.error & 0x4) return -1;

	// CMD55
	my_sdmmc_send_command(&deviceSD,0x10437,deviceSD.initarg << 0x10);
	if (deviceSD.error & 0x4) return -7;

	deviceSD.SDOPT = 1;
	my_sdmmc_send_command(&deviceSD,0x10446,0x2);
	if (deviceSD.error & 0x4) return -8;

	my_sdmmc_send_command(&deviceSD,0x1040D,deviceSD.initarg << 0x10);
	if (deviceSD.error & 0x4) return -9;

	my_sdmmc_send_command(&deviceSD,0x10410,0x200);
	if (deviceSD.error & 0x4) return -10;
	deviceSD.clk |= 0x200;

	return 0;

}

//---------------------------------------------------------------------------------
int my_sdmmc_nand_init() {
//---------------------------------------------------------------------------------
	my_setTarget(&deviceNAND);
	swiDelay(0xF000);

	my_sdmmc_send_command(&deviceNAND,0,0);

	do {
		do {
			my_sdmmc_send_command(&deviceNAND,0x10701,0x100000);
		} while ( !(deviceNAND.error & 1) );
	}
	while ((deviceNAND.ret[0] & 0x80000000) == 0);

	my_sdmmc_send_command(&deviceNAND,0x10602,0x0);
	if ((deviceNAND.error & 0x4))return -1;

	my_sdmmc_send_command(&deviceNAND,0x10403,deviceNAND.initarg << 0x10);
	if ((deviceNAND.error & 0x4))return -1;

	my_sdmmc_send_command(&deviceNAND,0x10609,deviceNAND.initarg << 0x10);
	if ((deviceNAND.error & 0x4))return -1;

	deviceNAND.total_size = calcSDSize((uint8_t*)&deviceNAND.ret[0],0);
	deviceNAND.clk = 1;
	setckl(1);

	my_sdmmc_send_command(&deviceNAND,0x10407,deviceNAND.initarg << 0x10);
	if ((deviceNAND.error & 0x4))return -1;

	deviceNAND.SDOPT = 1;

	my_sdmmc_send_command(&deviceNAND,0x10506,0x3B70100);
	if ((deviceNAND.error & 0x4))return -1;

	my_sdmmc_send_command(&deviceNAND,0x10506,0x3B90100);
	if ((deviceNAND.error & 0x4))return -1;

	my_sdmmc_send_command(&deviceNAND,0x1040D,deviceNAND.initarg << 0x10);
	if ((deviceNAND.error & 0x4))return -1;

	my_sdmmc_send_command(&deviceNAND,0x10410,0x200);
	if ((deviceNAND.error & 0x4))return -1;

	deviceNAND.clk |= 0x200;

	my_setTarget(&deviceSD);

	return 0;
}

//---------------------------------------------------------------------------------
int my_sdmmc_readsectors(struct mmcdevice *device, u32 sector_no, u32 numsectors, void *out) {
//---------------------------------------------------------------------------------
	if (device->isSDHC == 0) sector_no <<= 9;
	my_setTarget(device);
	sdmmc_write16(REG_SDSTOP,0x100);

#ifdef DATA32_SUPPORT
	sdmmc_write16(REG_SDBLKCOUNT32,numsectors);
	sdmmc_write16(REG_SDBLKLEN32,0x200);
#endif

	sdmmc_write16(REG_SDBLKCOUNT,numsectors);
	device->rData = out;
	device->size = numsectors << 9;
	my_sdmmc_send_command(device,0x33C12,sector_no);
	my_setTarget(&deviceSD);
	return my_geterror(device);
}

//---------------------------------------------------------------------------------
int my_sdmmc_writesectors(struct mmcdevice *device, u32 sector_no, u32 numsectors, void *in) {
//---------------------------------------------------------------------------------
	if (device->isSDHC == 0)
		sector_no <<= 9;
	my_setTarget(device);
	sdmmc_write16(REG_SDSTOP,0x100);

#ifdef DATA32_SUPPORT
	sdmmc_write16(REG_SDBLKCOUNT32,numsectors);
	sdmmc_write16(REG_SDBLKLEN32,0x200);
#endif

	sdmmc_write16(REG_SDBLKCOUNT,numsectors);
	device->tData = in;
	device->size = numsectors << 9;
	my_sdmmc_send_command(device,0x52C19,sector_no);
	my_setTarget(&deviceSD);
	return my_geterror(device);
}

//---------------------------------------------------------------------------------
void my_sdmmc_get_cid(int devicenumber, u32 *cid) {
//---------------------------------------------------------------------------------

	struct mmcdevice *device = (devicenumber == 1 ? &deviceNAND : &deviceSD);

	int oldIME = enterCriticalSection();

	my_setTarget(device);

	// use cmd7 to put sd card in standby mode
	// CMD7
	my_sdmmc_send_command(device, 0x10507, 0);

	// get sd card info
	// use cmd10 to read CID
	my_sdmmc_send_command(device, 0x1060A, device->initarg << 0x10);

	for (int i = 0; i < 4; ++i)
		cid[i] = device->ret[i];

	// put sd card back to transfer mode
	// CMD7
	my_sdmmc_send_command(device, 0x10507, device->initarg << 0x10);

	leaveCriticalSection(oldIME);
}

#ifdef SDMMC_USE_FIFO
//---------------------------------------------------------------------------------
void my_sdmmcMsgHandler(int bytes, void *user_data) {
//---------------------------------------------------------------------------------
    FifoMessage msg;
    int retval = 0;

    fifoGetDatamsg(FIFO_SDMMC, bytes, (u8*)&msg);

    int oldIME = enterCriticalSection();
    switch (msg.type) {

    case SDMMC_SD_READ_SECTORS:
        retval = my_sdmmc_readsectors(&deviceSD, msg.sdParams.startsector, msg.sdParams.numsectors, msg.sdParams.buffer);
        break;
    case SDMMC_SD_WRITE_SECTORS:
        retval = my_sdmmc_writesectors(&deviceSD, msg.sdParams.startsector, msg.sdParams.numsectors, msg.sdParams.buffer);
        break;
    case SDMMC_NAND_READ_SECTORS:
        retval = my_sdmmc_readsectors(&deviceNAND, msg.sdParams.startsector, msg.sdParams.numsectors, msg.sdParams.buffer);
        break;
    case SDMMC_NAND_WRITE_SECTORS:
        retval = my_sdmmc_writesectors(&deviceNAND, msg.sdParams.startsector, msg.sdParams.numsectors, msg.sdParams.buffer);
        break;
    }

    leaveCriticalSection(oldIME);

    fifoSendValue32(FIFO_SDMMC, retval);
}
#endif

//---------------------------------------------------------------------------------
int my_sdmmc_nand_startup() {
//---------------------------------------------------------------------------------
	my_sdmmc_controller_init(false);
	return my_sdmmc_nand_init();
}

//---------------------------------------------------------------------------------
int my_sdmmc_sd_startup() {
//---------------------------------------------------------------------------------
	my_sdmmc_controller_init(false);
	return my_sdmmc_sdcard_init();
}

#ifdef SDMMC_USE_FIFO
//---------------------------------------------------------------------------------
void my_sdmmcValueHandler(u32 value, void* user_data) {
//---------------------------------------------------------------------------------
    int result = 0;
    int sdflag = 0;
    int oldIME = enterCriticalSection();

    switch(value) {

    case SDMMC_HAVE_SD:
        result = sdmmc_read16(REG_SDSTATUS0);
        break;

    case SDMMC_SD_START:
        sdflag = 1;
        /* Falls through. */
    case SDMMC_NAND_START:
        if (sdmmc_read16(REG_SDSTATUS0) == 0) {
            result = 1;
        } else {
            result = (sdflag == 1) ? my_sdmmc_sd_startup() : my_sdmmc_nand_startup();
        }
        break;

    case SDMMC_SD_IS_INSERTED:
        result = my_sdmmc_cardinserted();
        break;

    case SDMMC_SD_STOP:
        my_sdmmc_controller_initialised = false;
        break;

    case SDMMC_NAND_SIZE:
        result = deviceNAND.total_size;
        break;
    }

    leaveCriticalSection(oldIME);

    fifoSendValue32(FIFO_SDMMC, result);
}
#else
//---------------------------------------------------------------------------------
void my_sdmmcHandler() {
//---------------------------------------------------------------------------------
	int result = 0;
	int sdflag = 0;
	int oldIME = enterCriticalSection();

	switch(IPC_GetSync()) {

	case 0: // 0x56484453: // SDMMC_HAVE_SD
		result = (sdmmc_read16(REG_SDSTATUS0) & BIT(5)) != 0;
		break;

	case 1: // 0x54534453: // SDMMC_SD_START
		sdflag = 1;
		/* Falls through. */
	case 2: // 0x5453414E: // SDMMC_NAND_START
		if (sdmmc_read16(REG_SDSTATUS0) == 0) {
			result = 1;
		} else {
			result = (sdflag == 1) ? my_sdmmc_sd_startup() : my_sdmmc_nand_startup();
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

	case 3: // 0x4E494453: // SDMMC_SD_IS_INSERTED
		result = useDLDI ? io_dldi_data->ioInterface.isInserted() : my_sdmmc_cardinserted();
		break;

	//case SDMMC_SD_STOP:
	//	break;

	//case 0x5A53414E: // SDMMC_NAND_SIZE
	//	result = deviceNAND.total_size;
	//	break;

	case 4: // 0x44524453: // SDMMC_SD_READ_SECTORS
		result =
			useDLDI ? (io_dldi_data->ioInterface.readSectors(*(u32*)0x02FFFA00, *(u32*)0x02FFFA04, (void*)*(u32*)0x02FFFA08) ? 0 : 1)
					: my_sdmmc_readsectors(&deviceSD, *(u32*)0x02FFFA00, *(u32*)0x02FFFA04, (void*)*(u32*)0x02FFFA08);
		break;
	case 5: // 0x52574453: // SDMMC_SD_WRITE_SECTORS
		u32 sector = *(u32*)0x02FFFA00;
		u32 numSectors = *(u32*)0x02FFFA04;
		void* buffer = (void*)*(u32*)0x02FFFA08;
		for (int i = 0; i < numSectors; i++) {
		result =
			useDLDI ? (io_dldi_data->ioInterface.writeSectors(sector+i, 1, buffer+(i*512)) ? 0 : 1)
					: my_sdmmc_writesectors(&deviceSD, sector+i, 1, buffer+(i*512));
		}
		break;
	case 6: // 0x4452414E: // SDMMC_NAND_READ_SECTORS
		result = my_sdmmc_readsectors(&deviceNAND, *(u32*)0x02FFFA00, *(u32*)0x02FFFA04, (void*)*(u32*)0x02FFFA08);
		break;
	case 7: // 0x5257414E: // SDMMC_NAND_WRITE_SECTORS
		result = my_sdmmc_writesectors(&deviceNAND, *(u32*)0x02FFFA00, *(u32*)0x02FFFA04, (void*)*(u32*)0x02FFFA08);
		break;
	}

	leaveCriticalSection(oldIME);

	//fifoSendValue32(FIFO_SDMMC, result);
	*(u32*)0x02FFFA0C = result;
}
#endif

//---------------------------------------------------------------------------------
int my_sdmmc_sdcard_readsectors(u32 sector_no, u32 numsectors, void *out) {
//---------------------------------------------------------------------------------
	return my_sdmmc_readsectors(&deviceSD, sector_no, numsectors, out);
}

//---------------------------------------------------------------------------------
int my_sdmmc_sdcard_writesectors(u32 sector_no, u32 numsectors, void *in) {
//---------------------------------------------------------------------------------
	return my_sdmmc_writesectors(&deviceSD, sector_no, numsectors, in);
}

//---------------------------------------------------------------------------------
int my_sdmmc_nand_readsectors(u32 sector_no, u32 numsectors, void *out) {
//---------------------------------------------------------------------------------
	return my_sdmmc_readsectors(&deviceNAND, sector_no, numsectors, out);
}

//---------------------------------------------------------------------------------
int my_sdmmc_nand_writesectors(u32 sector_no, u32 numsectors, void *in) {
//---------------------------------------------------------------------------------
	return my_sdmmc_writesectors(&deviceNAND, sector_no, numsectors, in);
}


