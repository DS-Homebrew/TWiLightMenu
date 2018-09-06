#include <nds.h>
#include <nds/arm9/dldi.h>
#include <fat.h>

void flashcardInit(void) {
	io_dldi_data = dldiLoadFromFile("sd:/_nds/test.dldi");
	fatMountSimple("fat", &io_dldi_data->ioInterface);
}