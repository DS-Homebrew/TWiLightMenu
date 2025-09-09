#include <nds/asminc.h>

    .syntax  unified
    .arch    armv5te
    .cpu     arm946e-s

    .arm

#define CP15_CONFIG_REGION_ENABLE               (1 << 0)

#define CP15_REGION_SIZE_16MB                   (0x17 << 1)
#define CP15_REGION_SIZE_32MB                   (0x18 << 1)

// Region 6 is used as non cacheable main RAM by libnds

BEGIN_ASM_FUNC CP15_ExtendRegion6
    ldr     r0, =(0xC000000 | CP15_CONFIG_REGION_ENABLE | CP15_REGION_SIZE_32MB)
    mcr     p15, 0, r0, c6, c6, 0
    bx      lr
