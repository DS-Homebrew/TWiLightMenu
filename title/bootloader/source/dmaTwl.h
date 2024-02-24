#pragma once

typedef struct
{
    const void* src; // Source address; not used in fill mode
    void* dst; // Destination address
    u32 totalWordCount; // For auto-start mode without infinite repeat
    u32 wordCount; // Number of words to transfer per start trigger
    u32 blockInterval; // Sets prescaler and cycles of delay between physical blocks
    u32 fillData; // For fill mode
    u32 control;
} dma_twl_config_t;

#define REG_NDMAGCNT        (*(vu32*)0x04004100)

#define NDMAGCNT_YIELD_CYCLES_0         (0 << 16)
#define NDMAGCNT_YIELD_CYCLES_1         (1 << 16)
#define NDMAGCNT_YIELD_CYCLES_2         (2 << 16)
#define NDMAGCNT_YIELD_CYCLES_4         (3 << 16)
#define NDMAGCNT_YIELD_CYCLES_8         (4 << 16)
#define NDMAGCNT_YIELD_CYCLES_16        (5 << 16)
#define NDMAGCNT_YIELD_CYCLES_32        (6 << 16)
#define NDMAGCNT_YIELD_CYCLES_64        (7 << 16)
#define NDMAGCNT_YIELD_CYCLES_128       (8 << 16)
#define NDMAGCNT_YIELD_CYCLES_256       (9 << 16)
#define NDMAGCNT_YIELD_CYCLES_512       (10 << 16)
#define NDMAGCNT_YIELD_CYCLES_1024      (11 << 16)
#define NDMAGCNT_YIELD_CYCLES_2048      (12 << 16)
#define NDMAGCNT_YIELD_CYCLES_4096      (13 << 16)
#define NDMAGCNT_YIELD_CYCLES_8192      (14 << 16)
#define NDMAGCNT_YIELD_CYCLES_16384     (15 << 16)

#define NDMAGCNT_ARBITRATION_FIXED          (0 << 31)
#define NDMAGCNT_ARBITRATION_ROUND_ROBIN    (1 << 31)

#define REG_NDMA0SAD        (*(vu32*)0x04004104)
#define REG_NDMA0DAD        (*(vu32*)0x04004108)
#define REG_NDMA0TCNT       (*(vu32*)0x0400410C)
#define REG_NDMA0WCNT       (*(vu32*)0x04004110)
#define REG_NDMA0BCNT       (*(vu32*)0x04004114)
#define REG_NDMA0FDATA      (*(vu32*)0x04004118)
#define REG_NDMA0CNT        (*(vu32*)0x0400411C)

#define REG_NDMA1SAD        (*(vu32*)0x04004120)
#define REG_NDMA1DAD        (*(vu32*)0x04004124)
#define REG_NDMA1TCNT       (*(vu32*)0x04004128)
#define REG_NDMA1WCNT       (*(vu32*)0x0400412C)
#define REG_NDMA1BCNT       (*(vu32*)0x04004130)
#define REG_NDMA1FDATA      (*(vu32*)0x04004134)
#define REG_NDMA1CNT        (*(vu32*)0x04004138)

#define REG_NDMA2SAD        (*(vu32*)0x0400413C)
#define REG_NDMA2DAD        (*(vu32*)0x04004140)
#define REG_NDMA2TCNT       (*(vu32*)0x04004144)
#define REG_NDMA2WCNT       (*(vu32*)0x04004148)
#define REG_NDMA2BCNT       (*(vu32*)0x0400414C)
#define REG_NDMA2FDATA      (*(vu32*)0x04004150)
#define REG_NDMA2CNT        (*(vu32*)0x04004154)

#define REG_NDMA3SAD        (*(vu32*)0x04004158)
#define REG_NDMA3DAD        (*(vu32*)0x0400415C)
#define REG_NDMA3TCNT       (*(vu32*)0x04004160)
#define REG_NDMA3WCNT       (*(vu32*)0x04004164)
#define REG_NDMA3BCNT       (*(vu32*)0x04004168)
#define REG_NDMA3FDATA      (*(vu32*)0x0400416C)
#define REG_NDMA3CNT        (*(vu32*)0x04004170)

#define NDMABCNT_INTERVAL(x)    (x)
#define NDMABCNT_PRESCALER_1    (0 << 16)
#define NDMABCNT_PRESCALER_4    (1 << 16)
#define NDMABCNT_PRESCALER_16   (2 << 16)
#define NDMABCNT_PRESCALER_64   (3 << 16)

#define NDMACNT_DST_MODE_INCREMENT      (0 << 10)
#define NDMACNT_DST_MODE_DECREMENT      (1 << 10)
#define NDMACNT_DST_MODE_FIXED          (2 << 10)

#define NDMACNT_DST_RELOAD              (1 << 12)

#define NDMACNT_SRC_MODE_INCREMENT      (0 << 13)
#define NDMACNT_SRC_MODE_DECREMENT      (1 << 13)
#define NDMACNT_SRC_MODE_FIXED          (2 << 13)
#define NDMACNT_SRC_MODE_FILLDATA       (3 << 13)

#define NDMACNT_SRC_RELOAD              (1 << 15)

#define NDMACNT_PHYSICAL_COUNT_1        (0 << 16)
#define NDMACNT_PHYSICAL_COUNT_2        (1 << 16)
#define NDMACNT_PHYSICAL_COUNT_4        (2 << 16)
#define NDMACNT_PHYSICAL_COUNT_8        (3 << 16)
#define NDMACNT_PHYSICAL_COUNT_16       (4 << 16)
#define NDMACNT_PHYSICAL_COUNT_32       (5 << 16)
#define NDMACNT_PHYSICAL_COUNT_64       (6 << 16)
#define NDMACNT_PHYSICAL_COUNT_128      (7 << 16)
#define NDMACNT_PHYSICAL_COUNT_256      (8 << 16)
#define NDMACNT_PHYSICAL_COUNT_512      (9 << 16)
#define NDMACNT_PHYSICAL_COUNT_1024     (10 << 16)
#define NDMACNT_PHYSICAL_COUNT_2048     (11 << 16)
#define NDMACNT_PHYSICAL_COUNT_4096     (12 << 16)
#define NDMACNT_PHYSICAL_COUNT_8192     (13 << 16)
#define NDMACNT_PHYSICAL_COUNT_16384    (14 << 16)
#define NDMACNT_PHYSICAL_COUNT_32768    (15 << 16)

#define NDMACNT_MODE_TIMER_0            (0 << 24)
#define NDMACNT_MODE_TIMER_1            (1 << 24)
#define NDMACNT_MODE_TIMER_2            (2 << 24)
#define NDMACNT_MODE_TIMER_3            (3 << 24)
#define NDMACNT_MODE_DS_SLOTA_ROM_XFER  (4 << 24)
#define NDMACNT_MODE_DS_SLOTB_ROM_XFER  (5 << 24)
#define NDMACNT_MODE_VBLANK             (6 << 24)

#ifdef LIBTWL_ARM9

#define NDMACNT_MODE_HBLANK             (7 << 24)
#define NDMACNT_MODE_DISPLAY            (8 << 24)
#define NDMACNT_MODE_MMEM_DISP_FIFO     (9 << 24)
#define NDMACNT_MODE_GX_FIFO            (10 << 24)
#define NDMACNT_MODE_CAMERA             (11 << 24)

#endif

#ifdef LIBTWL_ARM7

#define NDMACNT_MODE_WIFI               (7 << 24)
#define NDMACNT_MODE_SDMMC              (8 << 24)
#define NDMACNT_MODE_SDIO               (9 << 24)
#define NDMACNT_MODE_AES_IN             (10 << 24)
#define NDMACNT_MODE_AES_OUT            (11 << 24)
#define NDMACNT_MODE_MIC                (12 << 24)

#endif

#define NDMACNT_MODE_IMMEDIATE      (1 << 28)
#define NDMACNT_REPEAT_INFINITELY   (1 << 29)
#define NDMACNT_IRQ                 (1 << 30)
#define NDMACNT_ENABLE              (1 << 31)

#ifdef __cplusplus
extern "C" {
#endif

/// @brief Configures twl ndma to use fixed arbitration.
///        In this mode ndma0 has the highest and ndma3 the lowest priority,
///        similar to the nitro dma channels. Note that ndma0 has a lower
///        priority than nitro dma channel 3. When ndma channels are active
///        the dsp and cpu can not access the bus.
static inline void dma_twlSetFixedArbitration(void)
{
    REG_NDMAGCNT = NDMAGCNT_ARBITRATION_FIXED;
}

/// @brief Configures twl ndma to use round robin arbitration.
///        In this mode nitro dma channels still have a higher priority,
///        but bus access is distributed between all ndma channels and
///        the dsp and cpu.
///        This is done in the order ndma0, ndma1, ndma2, ndma3, dsp/cpu.
///        Candidates that do not have any outstanding request are skipped,
///        and the dsp takes priority over the cpu (as usual). The amount
///        of cycles reserved for the dsp/cpu is configurable.
/// @param yieldCycles The number of cycles that will be yielded to the
///        dsp/cpu in the round robin schedule. When there is no request
///        outstanding the cycles will not be wasted. Should be one of
///        NDMAGCNT_YIELD_CYCLES_*.
static inline void dma_twlSetRoundRobinArbitration(u32 yieldCycles)
{
    REG_NDMAGCNT = NDMAGCNT_ARBITRATION_ROUND_ROBIN | yieldCycles;
}

static inline void dma_twlSetParams(int dma, const dma_twl_config_t* config)
{
    vu32* channel = &(&REG_NDMA0SAD)[7 * dma];
    channel[0] = (u32)config->src;
    channel[1] = (u32)config->dst;
    channel[2] = config->totalWordCount;
    channel[3] = config->wordCount;
    channel[4] = config->blockInterval;
    channel[5] = config->fillData;
    channel[6] = config->control;
}

static inline void dma_twlWait(int dma)
{
    vu32* cnt = &(&REG_NDMA0CNT)[7 * dma];
    while (*cnt & NDMACNT_ENABLE);
}

static inline void dma_twlCopy32Async(int dma, const void* src, void* dst, u32 length)
{
    vu32* channel = &(&REG_NDMA0SAD)[7 * dma];
    channel[0] = (u32)src; //SAD
    channel[1] = (u32)dst; //DAD
    channel[3] = length >> 2; //WCNT
    channel[4] = NDMABCNT_PRESCALER_1 | NDMABCNT_INTERVAL(0); //BCNT
    channel[6] = NDMACNT_DST_MODE_INCREMENT | NDMACNT_SRC_MODE_INCREMENT |
        NDMACNT_PHYSICAL_COUNT_1 | NDMACNT_MODE_IMMEDIATE | NDMACNT_ENABLE;
}

static inline void dma_twlCopy32(int dma, const void* src, void* dst, u32 length)
{
    dma_twlCopy32Async(dma, src, dst, length);
    dma_twlWait(dma);
}

static inline void dma_twlFill32Async(int dma, u32 value, void* dst, u32 length)
{
    vu32* channel = &(&REG_NDMA0SAD)[7 * dma];
    channel[1] = (u32)dst; //DAD
    channel[3] = length >> 2; //WCNT
    channel[4] = NDMABCNT_PRESCALER_1 | NDMABCNT_INTERVAL(0); //BCNT
    channel[5] = value; //FDATA
    channel[6] = NDMACNT_DST_MODE_INCREMENT | NDMACNT_SRC_MODE_FILLDATA |
        NDMACNT_PHYSICAL_COUNT_1 | NDMACNT_MODE_IMMEDIATE | NDMACNT_ENABLE;
}

static inline void dma_twlFill32(int dma, u32 value, void* dst, u32 length)
{
    dma_twlFill32Async(dma, value, dst, length);
    dma_twlWait(dma);
}

#ifdef __cplusplus
}
#endif
