#include <nds.h>
#include <nds/arm9/dldi.h>
#include <fat.h>
#include <sys/stat.h>

bool secondaryDevice = false;

/*bool sdFound(void) {
	if (access("sd:/", F_OK) == 0) {
		return true;
	} else {
		return false;
	}
}*/

bool flashcardFound(void) {
	if (access("fat:/", F_OK) == 0) {
		return true;
	} else {
		return false;
	}
}

bool bothSDandFlashcard(void) {
	if ((access("sd:/", F_OK) == 0) && (access("fat:/", F_OK) == 0)) {
		return true;
	} else {
		return false;
	}
}

void flashcardInit(void) {
	if (!flashcardFound() && REG_SCFG_MC != 0x11) {
		io_dldi_data = dldiLoadFromFile("sd:/_nds/test.dldi");
		fatMountSimple("fat", &io_dldi_data->ioInterface);
	}
}