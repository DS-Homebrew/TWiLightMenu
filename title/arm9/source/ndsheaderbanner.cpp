#include <nds.h>
#include <stdio.h>
#include <unistd.h>

#include "ndsheaderbanner.h"

static u32 arm9Sig[3][4];

bool checkDsiBinaries(FILE* ndsFile) {
	sNDSHeaderExt ndsHeader;

	fseek(ndsFile, 0, SEEK_SET);
	fread(&ndsHeader, 1, sizeof(ndsHeader), ndsFile);

	if (ndsHeader.unitCode == 0) {
		return true;
	}

	if (ndsHeader.arm9iromOffset < 0x8000 || ndsHeader.arm9iromOffset >= 0x20000000
	 || ndsHeader.arm7iromOffset < 0x8000 || ndsHeader.arm7iromOffset >= 0x20000000) {
		return false;
	}

	for (int i = 0; i < 3; i++) {
		arm9Sig[i][0] = 0;
		arm9Sig[i][1] = 0;
		arm9Sig[i][2] = 0;
		arm9Sig[i][3] = 0;
	}

	fseek(ndsFile, 0x8000, SEEK_SET);
	fread(arm9Sig[0], sizeof(u32), 4, ndsFile);
	fseek(ndsFile, ndsHeader.arm9iromOffset, SEEK_SET);
	fread(arm9Sig[1], sizeof(u32), 4, ndsFile);
	fseek(ndsFile, ndsHeader.arm7iromOffset, SEEK_SET);
	fread(arm9Sig[2], sizeof(u32), 4, ndsFile);
	for (int i = 1; i < 3; i++) {
		if (arm9Sig[i][0] == arm9Sig[0][0]
		 && arm9Sig[i][1] == arm9Sig[0][1]
		 && arm9Sig[i][2] == arm9Sig[0][2]
		 && arm9Sig[i][3] == arm9Sig[0][3]) {
			return false;
		}
		if (arm9Sig[i][0] == 0
		 && arm9Sig[i][1] == 0
		 && arm9Sig[i][2] == 0
		 && arm9Sig[i][3] == 0) {
			return false;
		}
		if (arm9Sig[i][0] == 0xFFFFFFFF
		 && arm9Sig[i][1] == 0xFFFFFFFF
		 && arm9Sig[i][2] == 0xFFFFFFFF
		 && arm9Sig[i][3] == 0xFFFFFFFF) {
			return false;
		}
	}

	return true;
}