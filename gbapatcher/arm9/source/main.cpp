#include <nds.h>
#include <stdio.h>
#include <fat.h>
#include <sys/stat.h>
#include <limits.h>

#include <string>
#include <string.h>
#include <unistd.h>

#include "tonccpy.h"
#include "fileCopy.h"
#include "tool/stringtool.h"
#include "save/Save.h"
#include "gbaswitch.h"

u32 saveSize = 0;

//---------------------------------------------------------------------------------
void stop (void) {
//---------------------------------------------------------------------------------
	while (1) {
		swiWaitForVBlank();
	}
}

//---------------------------------------------------------------------------------
int main(int argc, char **argv) {
//---------------------------------------------------------------------------------

	// overwrite reboot stub identifier
	extern char *fake_heap_end;
	*fake_heap_end = 0;

	defaultExceptionHandler();

	if (!fatInitDefault()) {
		consoleDemoInit();
		printf("fatInitDefault failed!");
		stop();
	}

	sysSetCartOwner(BUS_OWNER_ARM9); // Allow arm9 to access GBA ROM

	const save_type_t* saveType = save_findTag(getFileSize(argv[1]));
	if (saveType != NULL && saveType->patchFunc != NULL)
	{
		saveType->patchFunc(saveType);
	}

	if (saveSize > 0) {
		std::string savepath = replaceAll(argv[1], ".gba", ".sav");
		if (getFileSize(savepath.c_str()) == 0) {
			toncset((void*)0x0A000000, 0, saveSize);
			FILE *pFile = fopen(savepath.c_str(), "wb");
			if (pFile) {
				fseek(pFile, saveSize - 1, SEEK_SET);
				fputc('\0', pFile);
				fclose(pFile);
			}
		}
	}

	gbaSwitch();

	return 0;
}
