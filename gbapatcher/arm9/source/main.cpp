#include <nds.h>
#include <stdio.h>
#include <fat.h>
#include <sys/stat.h>
#include <limits.h>

#include <string.h>
#include <unistd.h>

#include "save/Save.h"
#include "gbaswitch.h"

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

	const save_type_t* saveType = save_findTag();
	if (saveType != NULL)
	{
		if (saveType->patchFunc != NULL)
			saveType->patchFunc(saveType);
	}

	gbaSwitch();

	return 0;
}
