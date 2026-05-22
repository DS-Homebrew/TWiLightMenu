/*
    Copyright (C) 2024 lifehackerhansol
    Additional modifications Copyright (C) 2025-2026 coderkei & Rocket Robz

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef NO_PICO_LAUNCHER

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
#include "cheat.h"

constexpr std::align_val_t cache_align { 32 };

typedef void (*pico_loader_9_func_t)(void);

static pload_cheats_t* mCheats;

static bool prepareCheats(std::string romPath) {
    u32* cheatTablePtr;
    std::vector<cCheatDatItem> cheatEntries;
    // a pload_cheats_t table always has two u32s as the header
    u32 cheatTableSize = 8;
    CheatCodelist cheat;

    if (!cheat.parse(romPath)) goto cheat_failed;

    cheatEntries = cheat.getCheatsAlt();
    if (cheatEntries.empty()) goto cheat_failed;

    // calculate size of pload_cheats_t to allocate
    for (u32 i = 0; i < cheatEntries.size(); i++) {
        // Each pload_cheats_t table has a pload_cheat_t entry for each cheat entry
        // which has a u32 for length
        cheatTableSize += 4;
        // then the data, multiplied by 4 since each opcode is a u32
        cheatTableSize += ((cheatEntries[i]._cheat.size()) << 2);
    }

    // construct the cheat table
    mCheats = (pload_cheats_t*)memalign(4, cheatTableSize);
    mCheats->length = cheatTableSize;
    mCheats->numberOfCheats = cheatEntries.size();
    cheatTablePtr = (u32*)mCheats;

    // load cheat codes to table
    // skip the first two u32, which is the cheat table's header
    cheatTablePtr += 2;

    for (u32 i = 0; i < cheatEntries.size(); i++) {
        // pload_cheat_t->length
        *cheatTablePtr = cheatEntries[i]._cheat.size() << 2;

        cheatTablePtr++;
        // pload_cheat_opcode_t, arbitrary size
        // TODO bounds check
        for (u32 j = 0; j < cheatEntries[i]._cheat.size(); j++) {
            *cheatTablePtr = cheatEntries[i]._cheat[j];
            cheatTablePtr++;
        }
    }

    return true;

    cheat_failed:
    return false;
}

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

	const bool useCheats = prepareCheats(romPath);

	fseek(loader9, 0, SEEK_END);
	auto picoLoader9Size = ftell(loader9);
	fseek(loader9, 0, SEEK_SET);

	fseek(loader7, 0, SEEK_END);
	auto picoLoader7Size = ftell(loader7);
	fseek(loader7, 0, SEEK_SET);

	auto picoLoader7 = new(cache_align) u8[picoLoader7Size];
	fread(picoLoader7, 1, picoLoader7Size, loader7);
	fclose(loader7);

	const int apiVersion = ((pload_header7_t*)picoLoader7)->apiVersion;

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

	((pload_header7_t*)0x06840000)->bootDrive = PLOAD_BOOT_DRIVE_DLDI;
	((pload_header7_t*)0x06840000)->dldiDriver = (void*)io_dldi_data;
	tonccpy(&((pload_header7_t*)0x06840000)->loadParams, &sLoadParams, sizeof(pload_params_t));
	if (apiVersion >= 3 && useCheats) {
		((pload_header7_t*)0x06840000)->v3.cheats = mCheats;
	}

	DC_FlushAll();
	DC_InvalidateAll();
	IC_InvalidateAll();
	sysSetBusOwners(false, false);

	vramSetBankC(VRAM_C_ARM7_0x06000000);
	vramSetBankD(VRAM_D_ARM7_0x06020000);
	fifoSendValue32(FIFO_USER_02, 0x4F434950); // 'PICO'
	irqDisable(IRQ_ALL);

	((pico_loader_9_func_t)0x06800000)();
	return 0;
}

#endif