/*
    Copyright (C) 2024 lifehackerhansol
    Additional modifications Copyright (C) 2025 coderkei & Rocket Robz

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include <sys/stat.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <fat.h>
#include <nds/arm9/dldi.h>

#include <nds.h>

#include "common/picoLoader7.h"
#include "common/tonccpy.h"
#include "DSpicoLauncher.h"

constexpr std::align_val_t cache_align { 32 };

typedef void (*pico_loader_9_func_t)(void);

int picoLaunchRom(std::string romPath, std::string savePath) {
    const char picoLoader7Path[] = "fat:/_pico/picoLoader7.bin";
    const char picoLoader9Path[] = "fat:/_pico/picoLoader9.bin";

    if(access(picoLoader7Path, F_OK) != 0 || access(picoLoader9Path, F_OK) != 0) {
        return 1;
    }

    auto* loader9 = fopen(picoLoader9Path, "rb");

    if(!loader9){
        return 9;
    }
    auto* loader7 = fopen(picoLoader7Path, "rb");

    if(!loader7){
        fclose(loader9);
        return 7;
    }

    pload_params_t sLoadParams{};
    strcpy(sLoadParams.romPath, romPath.c_str());
    strcpy(sLoadParams.savePath, savePath.c_str());

    fseek(loader9, 0, SEEK_END);
    auto picoLoader9Size = ftell(loader9);
    fseek(loader9, 0, SEEK_SET);

    fseek(loader7, 0, SEEK_END);
    auto picoLoader7Size = ftell(loader7);
    fseek(loader7, 0, SEEK_SET);

    auto picoLoader7 = new(cache_align) u8[picoLoader7Size];
    fread(picoLoader7, 1, picoLoader7Size, loader7);
    fclose(loader7);

    auto picoLoader9 = new(cache_align) u8[picoLoader9Size];
    fread(picoLoader9, 1, picoLoader9Size, loader9);
    fclose(loader9);

    // soundDisable();
    irqDisable(IRQ_ALL & ~(IRQ_FIFO_EMPTY |IRQ_FIFO_NOT_EMPTY));

    vramSetBankA(VRAM_A_LCD);
    vramSetBankB(VRAM_B_LCD);
    vramSetBankC(VRAM_C_LCD);
    vramSetBankD(VRAM_D_LCD);

    tonccpy((void*)0x06800000, picoLoader9, (picoLoader9Size + 1) & ~1);
    tonccpy((void*)0x06840000, picoLoader7, (picoLoader7Size + 1) & ~1);
    delete[] picoLoader9;
    delete[] picoLoader7;

    //clear out ARM9 DMA channels
    for (int i = 0; i < 4; i++) {
        DMA_CR(i) = 0;
        DMA_SRC(i) = 0;
        DMA_DEST(i) = 0;
        TIMER_CR(i) = 0;
        TIMER_DATA(i) = 0;
    }

    DC_FlushAll();
    DC_InvalidateAll();
    IC_InvalidateAll();
    sysSetBusOwners(false, false);

    ((pload_header7_t*)0x06840000)->bootDrive = PLOAD_BOOT_DRIVE_DLDI;
    ((pload_header7_t*)0x06840000)->dldiDriver = (void*)io_dldi_data;
    tonccpy(&((pload_header7_t*)0x06840000)->loadParams, &sLoadParams, sizeof(pload_params_t));
    vramSetBankC(VRAM_C_ARM7_0x06000000);
    vramSetBankD(VRAM_D_ARM7_0x06020000);
    fifoSendValue32(FIFO_USER_02, 0x4F434950); // 'PICO'
    irqDisable(IRQ_ALL);

    ((pico_loader_9_func_t)0x06800000)();
    return 0;
}