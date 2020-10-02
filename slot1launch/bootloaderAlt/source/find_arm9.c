#include <stddef.h> // NULL
#include "patch.h"
#include "find.h"

//#define memset __builtin_memset

//
// Subroutine function signatures ARM9
//

// Mpu cache
static const u32 mpuInitRegion0Signature[1] = {0xEE060F10};
static const u32 mpuInitRegion0Data[1]      = {0x4000033};
static const u32 mpuInitRegion1Signature[1] = {0xEE060F11};
static const u32 mpuInitRegion1Data1[1]     = {0x200002D}; // SDK <= 3
static const u32 mpuInitRegion1Data4[1]     = {0x200002D}; // SDK >= 4
//static const u32 mpuInitRegion1DataAlt[1]   = {0x200002B};
static const u32 mpuInitRegion2Signature[1] = {0xEE060F12};
static const u32 mpuInitRegion2Data1[1]     = {0x27C0023}; // SDK != 3 (Previously: SDK <= 2)
static const u32 mpuInitRegion2Data3[1]     = {0x27E0021}; // SDK 3 (Previously: SDK >= 3)
static const u32 mpuInitRegion3Signature[1] = {0xEE060F13};
static const u32 mpuInitRegion3Data[1]      = {0x8000035};

// Mpu cache init
static const u32 mpuInitCache[1] = {0xE3A00042};

const u32* getMpuInitRegionSignature(u32 patchMpuRegion) {
	switch (patchMpuRegion) {
		case 0: return mpuInitRegion0Signature;
		case 1: return mpuInitRegion1Signature;
		case 2: return mpuInitRegion2Signature;
		case 3: return mpuInitRegion3Signature;
	}
	return mpuInitRegion1Signature;
}

u32* findMpuStartOffset(const tNDSHeader* ndsHeader, u32 patchMpuRegion) {
	// dbg_printf("findMpuStartOffset:\n");

	const u32* mpuInitRegionSignature = getMpuInitRegionSignature(patchMpuRegion);

	u32* mpuStartOffset = findOffset(
		(u32*)ndsHeader->arm9destination, ndsHeader->arm9binarySize,
		mpuInitRegionSignature, 1
	);
	if (mpuStartOffset) {
		// dbg_printf("Mpu init found: ");
	} else {
		// dbg_printf("Mpu init not found\n");
	}

	if (mpuStartOffset) {
		// dbg_hexa((u32)mpuStartOffset);
		// dbg_printf("\n");
	}

	// dbg_printf("\n");
	return mpuStartOffset;
}

u32* findMpuDataOffset(const module_params_t* moduleParams, u32 patchMpuRegion, const u32* mpuStartOffset) {
	if (!mpuStartOffset) {
		return NULL;
	}

	// dbg_printf("findMpuDataOffset:\n");

	const u32* mpuInitRegion1Data = mpuInitRegion1Data1;
	const u32* mpuInitRegion2Data = mpuInitRegion2Data1;
	if (moduleParams->sdk_version > 0x3000000 && moduleParams->sdk_version < 0x4000000) {
		mpuInitRegion2Data = mpuInitRegion2Data3;
	} else if (moduleParams->sdk_version > 0x4000000) {
		mpuInitRegion1Data = mpuInitRegion1Data4;
	}

	const u32* mpuInitRegionData = mpuInitRegion1Data;
	switch (patchMpuRegion) {
		case 0:
			mpuInitRegionData = mpuInitRegion0Data;
			break;
		case 1:
			mpuInitRegionData = mpuInitRegion1Data;
			break;
		case 2:
			mpuInitRegionData = mpuInitRegion2Data;
			break;
		case 3:
			mpuInitRegionData = mpuInitRegion3Data;
			break;
	}
	
	u32* mpuDataOffset = findOffset(
		mpuStartOffset, 0x100,
		mpuInitRegionData, 1
	);
	if (!mpuDataOffset) {
		// Try to find it
		for (int i = 0; i < 0x100; i++) {
			mpuDataOffset += i;
			if ((*mpuDataOffset & 0xFFFFFF00) == 0x02000000) {
				break;
			}
		}
	}
	if (mpuDataOffset) {
		// dbg_printf("Mpu data found: ");
	} else {
		// dbg_printf("Mpu data not found\n");
	}

	if (mpuDataOffset) {
		// dbg_hexa((u32)mpuDataOffset);
		// dbg_printf("\n");
	}

	// dbg_printf("\n");
	return mpuDataOffset;
}

u32* findMpuInitCacheOffset(const u32* mpuStartOffset) {
	// dbg_printf("findMpuInitCacheOffset:\n");

	u32* mpuInitCacheOffset = findOffset(
		mpuStartOffset, 0x100,
		mpuInitCache, 1
	);
	if (mpuInitCacheOffset) {
		// dbg_printf("Mpu init cache found: ");
	} else {
		// dbg_printf("Mpu init cache not found\n");
	}

	if (mpuInitCacheOffset) {
		// dbg_hexa((u32)mpuInitCacheOffset);
		// dbg_printf("\n");
	}

	// dbg_printf("\n");
	return mpuInitCacheOffset;
}
