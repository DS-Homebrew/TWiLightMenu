#include <nds.h>
#include <nds/arm9/dldi.h>
#include <fat.h>
#include <sys/stat.h>
#include <stdio.h>

#include "common/flashcard.h"
#include "common/bootstrappaths.h"
#include "common/inifile.h"
#include "common/systemdetails.h"
#include "read_card.h"

/*TWL_CODE bool UpdateCardInfo(char* gameid, char* gamename) {
	memcpy(gameid, ndsCardHeader.gameCode, 4);
	gameid[4] = 0x00;
	memcpy(gamename, ndsCardHeader.gameTitle, 12);
	gamename[12] = 0x00;
	return true;
}

TWL_CODE void ShowGameInfo(const char gameid[], const char gamename[]) {
	iprintf("Game id: %s\nName:    %s", gameid, gamename);
}*/

TWL_CODE void twl_flashcardInit(void) {
	if (REG_SCFG_MC != 0x11 && !sys().arm7SCFGLocked()) {
		// Reset Slot-1 to allow reading title name and ID
		my_cardReset(false);

		CIniFile settingsini( DSIMENUPP_INI );

		if (settingsini.GetInt("SRLOADER", "SECONDARY_ACCESS", 0) == false) {
			disableSlot1();
			return;
		}

		// Read title name and ID
		cardInit();

		/*char gamename[13];
		char gameid[5];

		UpdateCardInfo(&gameid[0], &gamename[0]);

		consoleDemoInit();
		iprintf("REG_SCFG_MC: %x\n", REG_SCFG_MC);
		ShowGameInfo(gameid, gamename);

		for (int i = 0; i < 60*5; i++) {
			swiWaitForVBlank();
		}*/

		sysSetCardOwner (BUS_OWNER_ARM7);	// 3DS fix

		// Read a DLDI driver specific to the cart
		/*if (!memcmp(ndsCardHeader.gameCode, "ASMA", 4)) {
			io_dldi_data = dldiLoadFromFile("nitro:/dldi/r4tf.dldi");
			fatMountSimple("fat", &io_dldi_data->ioInterface);
		} else if (!memcmp(ndsCardHeader.gameTitle, "TOP TF/SD DS", 12) || !memcmp(ndsCardHeader.gameCode, "A76E", 4) || ((u32)ndsCardHeader.gameCode == 0xB003C24)) {
			io_dldi_data = dldiLoadFromFile("nitro:/dldi/ttio.dldi");
			fatMountSimple("fat", &io_dldi_data->ioInterface);
		} else if (!memcmp(ndsCardHeader.gameTitle, "PASS", 4) && !memcmp(ndsCardHeader.gameCode, "ASME", 4)) {
			io_dldi_data = dldiLoadFromFile("nitro:/dldi/CycloEvo.dldi");
			fatMountSimple("fat", &io_dldi_data->ioInterface);
		} else if (!memcmp(ndsCardHeader.gameTitle, "D!S!XTREME", 12) && !memcmp(ndsCardHeader.gameCode, "AYIE", 4)) {
			io_dldi_data = dldiLoadFromFile("nitro:/dldi/dsx.dldi");
			fatMountSimple("fat", &io_dldi_data->ioInterface);
		} else*/ if (!memcmp(ndsCardHeader.gameTitle, "QMATETRIAL", 9) || !memcmp(ndsCardHeader.gameTitle, "R4DSULTRA", 9) // R4iDSN/R4 Ultra
				|| !memcmp(ndsCardHeader.gameCode, "ACEK", 4) || !memcmp(ndsCardHeader.gameCode, "YCEP", 4) || !memcmp(ndsCardHeader.gameCode, "AHZH", 4) || !memcmp(ndsCardHeader.gameCode, "CHPJ", 4) || !memcmp(ndsCardHeader.gameCode, "ADLP", 4)) { // Acekard 2(i)
			io_dldi_data = dldiLoadFromFile("nitro:/dldi/ak2_sd.dldi");
			fatMountSimple("fat", &io_dldi_data->ioInterface);
		} /*else if (!memcmp(ndsCardHeader.gameCode, "ALXX", 4)) {
			io_dldi_data = dldiLoadFromFile("nitro:/dldi/dstwo.dldi");
			fatMountSimple("fat", &io_dldi_data->ioInterface);
		} else if (!memcmp(ndsCardHeader.gameCode, "VCKF", 4)) {
			io_dldi_data = dldiLoadFromFile("nitro:/dldi/CycloIEvo.dldi");
			fatMountSimple("fat", &io_dldi_data->ioInterface);
		} */

		flashcardFoundReset();
		if (!flashcardFound()) {
			disableSlot1();
		}
	}
}

void flashcardInit(void) {
	if (isDSiMode() && !flashcardFound()) {
		twl_flashcardInit();
	}
}
