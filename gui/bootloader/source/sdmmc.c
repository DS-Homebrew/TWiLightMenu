#include <nds.h>
#include "sdmmc.h"


static struct mmcdevice deviceSD;

/*mmcdevice *getMMCDevice(int drive) {
    if(drive==0) return &deviceNAND;
    return &deviceSD;
}
*/
//---------------------------------------------------------------------------------
int __attribute__((noinline)) geterror(struct mmcdevice *ctx) {
//---------------------------------------------------------------------------------
    //if(ctx->error == 0x4) return -1;
    //else return 0;
    return (ctx->error << 29) >> 31;
}


//---------------------------------------------------------------------------------
void __attribute__((noinline)) setTarget(struct mmcdevice *ctx) {
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
void __attribute__((noinline)) sdmmc_send_command(struct mmcdevice *ctx, u32 cmd, u32 args) {
//---------------------------------------------------------------------------------
    int i;
    bool getSDRESP = (cmd << 15) >> 31;
    u16 flags = (cmd << 15) >> 31;
    const bool readdata = cmd & 0x20000;
    const bool writedata = cmd & 0x40000;

    if (readdata || writedata)
        flags |= TMIO_STAT0_DATAEND;

    ctx->error = 0;
    while (sdmmc_read16(REG_SDSTATUS1) & TMIO_STAT1_CMD_BUSY); //mmc working?
    sdmmc_write16(REG_SDIRMASK0,0);
    sdmmc_write16(REG_SDIRMASK1,0);
    sdmmc_write16(REG_SDSTATUS0,0);
    sdmmc_write16(REG_SDSTATUS1,0);

#ifdef DATA32_SUPPORT
    if (readdata)
        sdmmc_mask16(REG_SDDATACTL32, 0x1000, 0x800);
    if (writedata)
        sdmmc_mask16(REG_SDDATACTL32, 0x800, 0x1000);
#else
    sdmmc_mask16(REG_SDDATACTL32,0x1800,0);
#endif

    sdmmc_write16(REG_SDCMDARG0,args &0xFFFF);
    sdmmc_write16(REG_SDCMDARG1,args >> 16);
    sdmmc_write16(REG_SDCMD,cmd &0xFFFF);

    u32 size = ctx->size;
    u16 *dataPtr = (u16*)ctx->data;
#ifdef DATA32_SUPPORT
    u32 *dataPtr32 = (u32*)ctx->data;
#endif

    bool useBuf = ( 0 != dataPtr );
#ifdef DATA32_SUPPORT
    bool useBuf32 = (useBuf && (0 == (3 & ((u32)dataPtr))));
#endif

    u16 status0 = 0;
    while(true) {
        u16 status1 = sdmmc_read16(REG_SDSTATUS1);
        if (status1 & TMIO_STAT1_RXRDY) {
            if (readdata && useBuf) {
                sdmmc_mask16(REG_SDSTATUS1, TMIO_STAT1_RXRDY, 0);
                //sdmmc_write16(REG_SDSTATUS1,~TMIO_STAT1_RXRDY);
                if (size > 0x1FF) {
#ifdef DATA32_SUPPORT
                    if (useBuf32) {
                        for(i = 0; i<0x200; i+=4)
                            *dataPtr32++ = sdmmc_read32(REG_SDFIFO32);
                    } else {
#endif
                        for(i = 0; i<0x200; i+=2)
                            *dataPtr++ = sdmmc_read16(REG_SDFIFO);
#ifdef DATA32_SUPPORT
                    }
#endif
                    size -= 0x200;
                }
            }
        }

        if (status1 & TMIO_STAT1_TXRQ) {
            if (writedata && useBuf) {
                sdmmc_mask16(REG_SDSTATUS1, TMIO_STAT1_TXRQ, 0);
                //sdmmc_write16(REG_SDSTATUS1,~TMIO_STAT1_TXRQ);
                if (size > 0x1FF) {
#ifdef DATA32_SUPPORT
                    for (i = 0; i<0x200; i+=4)
                        sdmmc_write32(REG_SDFIFO32,*dataPtr32++);
#else
                    for (i = 0; i<0x200; i+=2)
                        sdmmc_write16(REG_SDFIFO,*dataPtr++);
#endif
                    size -= 0x200;
                }
            }
        }
        if (status1 & TMIO_MASK_GW) {
            ctx->error |= 4;
            break;
        }

        if (!(status1 & TMIO_STAT1_CMD_BUSY)) {
            status0 = sdmmc_read16(REG_SDSTATUS0);
            if (sdmmc_read16(REG_SDSTATUS0) & TMIO_STAT0_CMDRESPEND)
                ctx->error |= 0x1;
            if (status0 & TMIO_STAT0_DATAEND)
                ctx->error |= 0x2;

            if ((status0 & flags) == flags)
                break;
        }
    }
    ctx->stat0 = sdmmc_read16(REG_SDSTATUS0);
    ctx->stat1 = sdmmc_read16(REG_SDSTATUS1);
    sdmmc_write16(REG_SDSTATUS0,0);
    sdmmc_write16(REG_SDSTATUS1,0);

    if (getSDRESP != 0) {
        ctx->ret[0] = sdmmc_read16(REG_SDRESP0) | (sdmmc_read16(REG_SDRESP1) << 16);
        ctx->ret[1] = sdmmc_read16(REG_SDRESP2) | (sdmmc_read16(REG_SDRESP3) << 16);
        ctx->ret[2] = sdmmc_read16(REG_SDRESP4) | (sdmmc_read16(REG_SDRESP5) << 16);
        ctx->ret[3] = sdmmc_read16(REG_SDRESP6) | (sdmmc_read16(REG_SDRESP7) << 16);
    }
}


//---------------------------------------------------------------------------------
int sdmmc_cardinserted() {
//---------------------------------------------------------------------------------
	return 1; //sdmmc_cardready;
}

//---------------------------------------------------------------------------------
void sdmmc_controller_init() {
//---------------------------------------------------------------------------------
    deviceSD.isSDHC = 0;
    deviceSD.SDOPT = 0;
    deviceSD.res = 0;
    deviceSD.initarg = 0;
    deviceSD.clk = 0x80;
    deviceSD.devicenumber = 0;

    *(vu16*)(SDMMC_BASE + REG_SDDATACTL32) &= 0xF7FFu;
    *(vu16*)(SDMMC_BASE + REG_SDDATACTL32) &= 0xEFFFu;
    *(vu16*)(SDMMC_BASE + REG_SDDATACTL32) |= 0x402u;

    *(vu16*)(SDMMC_BASE + REG_SDDATACTL) = (*(vu16*)(SDMMC_BASE + 0xd8) & 0xFFDD) | 2;

#ifdef DATA32_SUPPORT
    *(vu16*)(SDMMC_BASE + REG_SDDATACTL32) &= 0xFFFFu;
    *(vu16*)(SDMMC_BASE + REG_SDDATACTL) &= 0xFFDFu;
    *(vu16*)(SDMMC_BASE + REG_SDBLKLEN32) = 512;
#else
    *(vu16*)(SDMMC_BASE + REG_SDDATACTL32) &= 0xFFFDu;
    *(vu16*)(SDMMC_BASE + REG_SDDATACTL) &= 0xFFDDu;
    *(vu16*)(SDMMC_BASE + REG_SDBLKLEN32) = 0;
#endif
    *(vu16*)(SDMMC_BASE + REG_SDBLKCOUNT32) = 1; //SDBLKCOUNT32
    *(vu16*)(SDMMC_BASE + REG_SDRESET) &= 0xFFFEu; //SDRESET
    *(vu16*)(SDMMC_BASE + REG_SDRESET) |= 1u; //SDRESET
    *(vu16*)(SDMMC_BASE + REG_SDIRMASK0) |= TMIO_MASK_ALL;
    *(vu16*)(SDMMC_BASE + REG_SDIRMASK1) |= TMIO_MASK_ALL>>16;
    *(vu16*)(SDMMC_BASE + 0x0fc) |= 0xDBu; //SDCTL_RESERVED7
    *(vu16*)(SDMMC_BASE + 0x0fe) |= 0xDBu; //SDCTL_RESERVED8
    *(vu16*)(SDMMC_BASE + REG_SDPORTSEL) &= 0xFFFCu; //SDPORTSEL
#ifdef DATA32_SUPPORT
    *(vu16*)(SDMMC_BASE + REG_SDCLKCTL) = 0x20;
    *(vu16*)(SDMMC_BASE + REG_SDOPT) = 0x40EE;
#else
    *(vu16*)(SDMMC_BASE + REG_SDCLKCTL) = 0x40; //Nintendo sets this to 0x20
    *(vu16*)(SDMMC_BASE + REG_SDOPT) = 0x40EB; //Nintendo sets this to 0x40EE
#endif
    *(vu16*)(SDMMC_BASE + REG_SDPORTSEL) &= 0xFFFCu; ////SDPORTSEL
    *(vu16*)(SDMMC_BASE + REG_SDBLKLEN) = 512; //SDBLKLEN
    *(vu16*)(SDMMC_BASE + REG_SDSTOP) = 0; //SDSTOP

    setTarget(&deviceSD);
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
int sdmmc_sdcard_init() {
//---------------------------------------------------------------------------------
    setTarget(&deviceSD);
    swiDelay(0xF000);
    sdmmc_send_command(&deviceSD,0,0);
    sdmmc_send_command(&deviceSD,0x10408,0x1AA);
    //u32 temp = (deviceSD.ret[0] == 0x1AA) << 0x1E;
    u32 temp = (deviceSD.error & 0x1) << 0x1E;

    //int count = 0;
    u32 temp2 = 0;
    do {
        do {
            sdmmc_send_command(&deviceSD,0x10437,deviceSD.initarg << 0x10);
            sdmmc_send_command(&deviceSD,0x10769,0x00FF8000 | temp);
            temp2 = 1;
        } while ( !(deviceSD.error & 1) );

    } while((deviceSD.ret[0] & 0x80000000) == 0);
    //do
    //{
    //  sdmmc_send_command(&deviceSD,0x10437,deviceSD.initarg << 0x10);
    //  sdmmc_send_command(&deviceSD,0x10769,0x00FF8000 | temp);
    //
    //}
    //while(!(deviceSD.ret[0] & 0x80000000));

    if(!((deviceSD.ret[0] >> 30) & 1) || !temp)
        temp2 = 0;

    deviceSD.isSDHC = temp2;
    //deviceSD.isSDHC = (deviceSD.ret[0] & 0x40000000);

    sdmmc_send_command(&deviceSD,0x10602,0);
    if (deviceSD.error & 0x4) return -1;

    sdmmc_send_command(&deviceSD,0x10403,0);
    if (deviceSD.error & 0x4) return -1;
    deviceSD.initarg = deviceSD.ret[0] >> 0x10;

    sdmmc_send_command(&deviceSD,0x10609,deviceSD.initarg << 0x10);
    if (deviceSD.error & 0x4) return -1;

    deviceSD.total_size = calcSDSize((u8*)&deviceSD.ret[0],-1);
    deviceSD.clk = 1;
    setckl(1);

    sdmmc_send_command(&deviceSD,0x10507,deviceSD.initarg << 0x10);
    if (deviceSD.error & 0x4) return -1;

    sdmmc_send_command(&deviceSD,0x10437,deviceSD.initarg << 0x10);
    if (deviceSD.error & 0x4) return -1;

    deviceSD.SDOPT = 1;
    sdmmc_send_command(&deviceSD,0x10446,0x2);
    if (deviceSD.error & 0x4) return -1;

    sdmmc_send_command(&deviceSD,0x1040D,deviceSD.initarg << 0x10);
    if (deviceSD.error & 0x4) return -1;

    sdmmc_send_command(&deviceSD,0x10410,0x200);
    if (deviceSD.error & 0x4) return -1;
    deviceSD.clk |= 0x200;

    return 0;

}


int __attribute__((noinline)) sdmmc_sdcard_readsectors(u32 sector_no, u32 numsectors, void *out) {
    if (deviceSD.isSDHC == 0)
        sector_no <<= 9;
    setTarget(&deviceSD);
    sdmmc_write16(REG_SDSTOP,0x100);

#ifdef DATA32_SUPPORT
    sdmmc_write16(REG_SDBLKCOUNT32,numsectors);
    sdmmc_write16(REG_SDBLKLEN32,0x200);
#endif

    sdmmc_write16(REG_SDBLKCOUNT,numsectors);
    deviceSD.data = out;
    deviceSD.size = numsectors << 9;
    sdmmc_send_command(&deviceSD,0x33C12,sector_no);
    return geterror(&deviceSD);
}