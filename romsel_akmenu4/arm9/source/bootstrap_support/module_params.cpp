#include <nds.h>
#include <stdio.h>
#include "module_params.h"
#include "tool/dbgtool.h"

u32 buf[2];
char moduleParamsBuf[sizeof(module_params_t)];

static const u32 moduleParamsSignature[2] = {0xDEC00621, 0x2106C0DE};

u32 findModuleParamsOffset(const sNDSHeaderExt *ndsHeader, FILE *ndsFile)
{
	dbg_printf("findModuleParamsOffset:\n");
	fseek(ndsFile, ndsHeader->arm9romOffset, SEEK_SET);

	// Offset of the module params in bytes
	int offset = 0;
	bool found = false;
	while (fread(buf, sizeof(u32), 2, ndsFile) == 2)
	{
		// The first word of the signature is on an even offset (relative to arm9romOffset)
		if (buf[0] == moduleParamsSignature[0] && buf[1] == moduleParamsSignature[1])
		{
			dbg_printf("FOUND BUF %X, SIG %X", buf[0], moduleParamsSignature[0]);
			found = true;
			break;
		}
		// The first word of the signature is on an odd offset (relative to arm9romOffset)
		else if (buf[1] == moduleParamsSignature[0])
		{
			// Read the next byte and check if the next byte is the paramSignature.
			if (fread(buf, sizeof(u32), 1, ndsFile) == 1 && buf[0] == moduleParamsSignature[1])
			{
				// The offset of moduleParamsSignature is at offset + 1.
				offset += sizeof(u32);
				found = true;
				break;
			}
		}
		offset += sizeof(u32) * 2;

		// Stop searching, if past the arm9 binary
		if (offset > ndsHeader->arm9binarySize) {
			break;
		}
	}

	if (!found)
		return NULL;

	dbg_printf("OFFSET %X\n", ndsHeader->arm9romOffset + offset);

	return ndsHeader->arm9romOffset + offset - 0x1C;
}

module_params_t *getModuleParams(const sNDSHeaderExt *ndsHeader, FILE *ndsFile)
{

	u32 moduleParamsOffset = findModuleParamsOffset(ndsHeader, ndsFile);
	fseek(ndsFile, moduleParamsOffset, SEEK_SET);
	fread(moduleParamsBuf, sizeof(module_params_t), 1, ndsFile);

	//module_params_t* moduleParams = (module_params_t*)((u32)moduleParamsOffset - 0x1C);
	return moduleParamsOffset ? (module_params_t *)(moduleParamsBuf) : NULL;
}