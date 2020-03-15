#include <string.h>
#include <nds/system.h>
#include "module_params.h"
#include "patch.h"
#include "find.h"
#include "common.h"

//#define memcpy __builtin_memcpy // memcpy

void patchMpu(const tNDSHeader* ndsHeader, const module_params_t* moduleParams) {
	u32 patchMpuRegion = 0;
	u32 patchMpuSize = 0;

	// Find the mpu init
	u32* mpuStartOffset = findMpuStartOffset(ndsHeader, patchMpuRegion);
	u32* mpuDataOffset = findMpuDataOffset(moduleParams, patchMpuRegion, mpuStartOffset);
	if (mpuDataOffset) {
		// Change the region 1 configuration

		u32 mpuInitRegionNewData = PAGE_32M | 0x02000000 | 1;
		u32 mpuNewDataAccess     = 0;
		u32 mpuNewInstrAccess    = 0;
		int mpuAccessOffset      = 0;
		switch (patchMpuRegion) {
			case 0:
				mpuInitRegionNewData = PAGE_128M | 0x00000000 | 1;
				break;
			case 2:
				mpuNewDataAccess  = 0x15111111;
				mpuNewInstrAccess = 0x5111111;
				mpuAccessOffset   = 6;
				break;
			case 3:
				mpuInitRegionNewData = PAGE_8M | 0x03000000 | 1;
				mpuNewInstrAccess    = 0x5111111;
				mpuAccessOffset      = 5;
				break;
		}

		*mpuDataOffset = mpuInitRegionNewData;

		if (mpuAccessOffset) {
			if (mpuNewInstrAccess) {
				mpuDataOffset[mpuAccessOffset] = mpuNewInstrAccess;
			}
			if (mpuNewDataAccess) {
				mpuDataOffset[mpuAccessOffset + 1] = mpuNewDataAccess;
			}
		}
	}

	// Find the mpu cache init
	u32* mpuInitCacheOffset = findMpuInitCacheOffset(mpuStartOffset);
	if (mpuInitCacheOffset) {
		*mpuInitCacheOffset = 0xE3A00046;
	}

	// Patch out all further mpu reconfiguration
	// dbg_printf("patchMpuSize: ");
	// dbg_hexa(patchMpuSize);
	// dbg_printf("\n\n");
	const u32* mpuInitRegionSignature = getMpuInitRegionSignature(patchMpuRegion);
	u32* mpuInitOffset = mpuStartOffset;
	while (mpuInitOffset && patchMpuSize) {
		u32 patchSize = ndsHeader->arm9binarySize;
		if (patchMpuSize > 1) {
			patchSize = patchMpuSize;
		}
		mpuInitOffset = findOffset(
			//(u32*)((u32)mpuStartOffset + 4), patchSize,
			mpuInitOffset + 1, patchSize,
			mpuInitRegionSignature, 1
		);
		if (mpuInitOffset) {
			// dbg_printf("Mpu init: ");
			// dbg_hexa((u32)mpuInitOffset);
			// dbg_printf("\n\n");

			*mpuInitOffset = 0xE1A00000; // nop

			// Try to find it
			/*for (int i = 0; i < 0x100; i++) {
				mpuDataOffset += i;
				if ((*mpuDataOffset & 0xFFFFFF00) == 0x02000000) {
					*mpuDataOffset = PAGE_32M | 0x02000000 | 1;
					break;
				}
				if (i == 100) {
					*mpuStartOffset = 0xE1A00000;
				}
			}*/
		}
	}
}
