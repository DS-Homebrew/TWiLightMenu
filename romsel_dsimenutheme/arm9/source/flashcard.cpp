#include <nds.h>
#include <nds/arm9/dldi.h>
#include <fat.h>
#include <sys/stat.h>
#include <stdio.h>

#include "graphics/graphics.h"

sNDSHeader nds;

bool previousUsedDevice = false;	// true == secondary
bool secondaryDevice = false;

int flashcard;
/* Flashcard value
	0: DSTT/R4i Gold/R4i-SDHC/R4 SDHC Dual-Core/R4 SDHC Upgrade/SC DSONE
	1: R4DS (Original Non-SDHC version)/ M3 Simply
	2: R4iDSN/R4i Gold RTS/R4 Ultra
	3: Acekard 2(i)/Galaxy Eagle/M3DS Real
	4: Acekard RPG
	5: Ace 3DS+/Gateway Blue Card/R4iTT
	6: SuperCard DSTWO
*/

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
		// Reset Slot-1 to allow reading title name and ID
		sysSetCardOwner (BUS_OWNER_ARM9);
		disableSlot1();
		for(int i = 0; i < 25; i++) { swiWaitForVBlank(); }
		enableSlot1();
		for(int i = 0; i < 15; i++) { swiWaitForVBlank(); }

		nds.gameCode[0] = 0;
		nds.gameTitle[0] = 0;
		char gamename[13];
		char gameid[5];

		UpdateCardInfo(&nds, &gameid[0], &gamename[0]);
		/* consoleDemoInit();
		iprintf("REG_SCFG_MC: %x\n", REG_SCFG_MC);
		ShowGameInfo(gameid, gamename);

		SetBrightness(0, 0);
		SetBrightness(1, 0);

		for (int i = 0; i < 60*2; i++) {
			swiWaitForVBlank();
		} */

		sysSetCardOwner (BUS_OWNER_ARM7);	// 3DS fix

		// Read a DLDI driver specific to the cart
		if (!memcmp(gamename, "D!S!XTREME", 12) && !memcmp(gameid, "AYIE", 4)) {
			flashcard = 0;
			io_dldi_data = dldiLoadFromFile("nitro:/dldi/dsx.dldi");
			fatMountSimple("fat", &io_dldi_data->ioInterface);
		} else if (!memcmp(gamename, "TOP TF/SD DS", 12)) {
			flashcard = 0;
			io_dldi_data = dldiLoadFromFile("nitro:/dldi/ttio.dldi");
			fatMountSimple("fat", &io_dldi_data->ioInterface);
		} else if (!memcmp(gamename, "R4DSULTRA", 9)) {
			flashcard = 2;
			io_dldi_data = dldiLoadFromFile("nitro:/dldi/r4idsn_sd.dldi");
			fatMountSimple("fat", &io_dldi_data->ioInterface);
		} else if (!memcmp(gameid, "ASMA", 4) || !memcmp(gameid, "R4DS", 4)) {
			flashcard = 1;
			io_dldi_data = dldiLoadFromFile("nitro:/dldi/r4tfv2.dldi");
			fatMountSimple("fat", &io_dldi_data->ioInterface);
		}
	}
}