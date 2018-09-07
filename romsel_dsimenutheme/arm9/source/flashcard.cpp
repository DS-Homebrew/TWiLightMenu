#include <nds.h>
#include <nds/arm9/dldi.h>
#include <fat.h>
#include <sys/stat.h>
#include <stdio.h>

#include "graphics/graphics.h"

sNDSHeader nds;

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

bool UpdateCardInfo(sNDSHeader* nds, char* gameid, char* gamename) {
	cardReadHeader((uint8*)nds);
	memcpy(gameid, nds->gameCode, 4);
	gameid[4] = 0x00;
	memcpy(gamename, nds->gameTitle, 12);
	gamename[12] = 0x00;
	return true;
}

void ShowGameInfo(const char gameid[], const char gamename[]) {
	iprintf("Game id: %s\nName:    %s", gameid, gamename);
}

void flashcardInit(void) {
	if (!flashcardFound() && REG_SCFG_MC != 0x11) {
		sysSetCardOwner (BUS_OWNER_ARM9);
		disableSlot1();
		for(int i = 0; i < 25; i++) { swiWaitForVBlank(); }
		enableSlot1();

		nds.gameCode[0] = 0;
		nds.gameTitle[0] = 0;
		char gamename[13];
		char gameid[5];

		UpdateCardInfo(&nds, &gameid[0], &gamename[0]);
		/* consoleDemoInit();
		ShowGameInfo(gameid, gamename);

		SetBrightness(0, 0);
		SetBrightness(1, 0);
	
		for (int i = 0; i < 60*2; i++) {
			swiWaitForVBlank();
		} */

		if (!memcmp(gamename, "TOP TF/SD DS", 12)) {
			io_dldi_data = dldiLoadFromFile("nitro:/dldi/ttio.dldi");
		} else if (!memcmp(gamename, "R4DSULTRA", 9)) {
			io_dldi_data = dldiLoadFromFile("nitro:/dldi/r4idsn_sd.dldi");
		} else if (!memcmp(gameid, "R4DS", 4)) {
			io_dldi_data = dldiLoadFromFile("nitro:/dldi/r4tfv2.dldi");
		}
		fatMountSimple("fat", &io_dldi_data->ioInterface);
	}
}