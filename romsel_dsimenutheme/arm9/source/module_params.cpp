#include <nds.h>
#include <stdio.h>
#include "module_params.h"

u32 offsetOfModuleParams;
u32 buf[2];
char moduleParamsBuf[sizeof(module_params_t)];

static const u32 moduleParamsSignature[2] = {0xDEC00621, 0x2106C0DE};

u32 findModuleParamsOffset(const sNDSHeaderExt *ndsHeader, FILE *ndsFile)
{
	for (int i = 0; i < 2; i++) {
		if (i == 1) {
			// Run fallback method
			fseek(ndsFile, (ndsHeader->arm9romOffset), SEEK_SET);
			offsetOfModuleParams = ndsHeader->arm9romOffset;
		} else {
			// Get module params offset from before Auto Load List
			fseek(ndsFile, (ndsHeader->arm9romOffset + (ndsHeader->unknownRAM1 - ndsHeader->arm9destination) - 4), SEEK_SET);
			fread(buf, sizeof(u32), 1, ndsFile);
			if (buf[0] >= 0x03000000) {
				fseek(ndsFile, (ndsHeader->arm9romOffset + (ndsHeader->unknownRAM1 - ndsHeader->arm9destination) - 0x168), SEEK_SET);
				fread(buf, sizeof(u32), 1, ndsFile);
			}
			offsetOfModuleParams = buf[0]-ndsHeader->arm9destination+ndsHeader->arm9romOffset;
			fseek(ndsFile, offsetOfModuleParams, SEEK_SET);
		}

		// Offset of the module params
		u32 offset = 0;
		while (fread(buf, sizeof(u32), 2, ndsFile) == 2) {
			// The first word of the signature is on an even offset (relative to arm9romOffset)
			if (buf[0] == moduleParamsSignature[0] && buf[1] == moduleParamsSignature[1]) {
				return offsetOfModuleParams + offset - 0x1C;
				break;
			}
			// The first word of the signature is on an odd offset (relative to arm9romOffset)
			else if (buf[1] == moduleParamsSignature[0]) {
				// Read the next byte and check if the next byte is the paramSignature.
				if (fread(buf, sizeof(u32), 1, ndsFile) == 1 && buf[0] == moduleParamsSignature[1]) {
					// The offset of moduleParamsSignature is at offset + 1.
					offset += sizeof(u32);
					return offsetOfModuleParams + offset - 0x1C;
					break;
				}
			}
			offset += sizeof(u32) * 2;

			// Stop searching, if signature isn't found
			if (offset > (i ? ndsHeader->arm9binarySize : 0x80)) {
				break;
			}
		}
	}
	
	return (u32)NULL;
}

module_params_t *getModuleParams(const sNDSHeaderExt *ndsHeader, FILE *ndsFile)
{

	u32 moduleParamsOffset = findModuleParamsOffset(ndsHeader, ndsFile);
	fseek(ndsFile, moduleParamsOffset, SEEK_SET);
	fread(moduleParamsBuf, sizeof(module_params_t), 1, ndsFile);

	//module_params_t* moduleParams = (module_params_t*)((u32)moduleParamsOffset - 0x1C);
	return moduleParamsOffset ? (module_params_t *)(moduleParamsBuf) : NULL;
}