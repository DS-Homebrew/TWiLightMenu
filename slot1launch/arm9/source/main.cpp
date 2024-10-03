/*
    NitroHax -- Cheat tool for the Nintendo DS
    Copyright (C) 2008  Michael "Chishm" Chisholm

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <nds.h>
#include <fat.h>
#include <nds/fifocommon.h>
#include <nds/arm9/dldi.h>

#include <stdio.h>
#include <string.h>
#include <list>

#include "twlClockExcludeMap.h"
#include "defaultSettings.h"
#include "common/inifile.h"
#include "common/tonccpy.h"
#include "nds_card.h"
#include "launch_engine.h"
#include "crc.h"

struct {
sNDSHeader header;
char padding[0x200 - sizeof(sNDSHeader)];
} ndsHeader;

bool consoleInited = false;
bool scfgUnlock = false;
int TWLMODE = 0;
bool TWLCLK = false;	// false == NTR, true == TWL
int TWLVRAM = 0;
bool TWLTOUCH = false;
bool soundFreq = false;
bool sleepMode = true;
bool runCardEngine = false;
bool EnableSD = false;
bool ignoreBlacklists = false;
int language = -1;

/**
 * Disable TWL clock speed for a specific game.
 */
bool setClockSpeed(int setting, char gameTid[], bool ignoreBlacklists) {
	if (!ignoreBlacklists) {
		// TODO: If the list gets large enough, switch to bsearch().
		for (unsigned int i = 0; i < sizeof(twlClockExcludeList)/sizeof(twlClockExcludeList[0]); i++) {
			if (memcmp(gameTid, twlClockExcludeList[i], 3) == 0) {
				// Found match
				return false;
			}
		}
	}

	return setting == -1 ? DEFAULT_BOOST_CPU : setting;
}

u8 cheatData[0x8000] = {0};

off_t getFileSize(const char *fileName)
{
	FILE* fp = fopen(fileName, "rb");
	off_t fsize = 0;
	if (fp) {
		fseek(fp, 0, SEEK_END);
		fsize = ftell(fp);			// Get source file's size
		fseek(fp, 0, SEEK_SET);
	}
	fclose(fp);

	return fsize;
}


bool consoleOn = false;

int main() {
	fifoSendValue32(FIFO_PM, PM_REQ_SLEEP_DISABLE);

	if (isDSiMode()) {
		// If slot is powered off, tell Arm7 slot power on is required.
		if (REG_SCFG_MC == 0x11) { fifoSendValue32(FIFO_USER_02, 1); }
		if (REG_SCFG_MC == 0x10) { fifoSendValue32(FIFO_USER_02, 1); }

		if (fatInitDefault()) {
			CIniFile settingsini("/_nds/TWiLightMenu/settings.ini");

			ignoreBlacklists = settingsini.GetInt("SRLOADER","IGNORE_BLACKLISTS",false);
			// TWLTOUCH = settingsini.GetInt("SRLOADER","SLOT1_TOUCH_MODE",0);
			soundFreq = settingsini.GetInt("NDS-BOOTSTRAP","SOUND_FREQ",0);
			sleepMode = settingsini.GetInt("SRLOADER","SLEEP_MODE",1);
			runCardEngine = settingsini.GetInt("SRLOADER","SLOT1_CARDENGINE",1);
			EnableSD = settingsini.GetInt("SRLOADER","SLOT1_ENABLESD",0);

			// Tell Arm7 it's ready for card reset (if card reset is nessecery)
			fifoSendValue32(FIFO_USER_01, 1);
			// Waits for Arm7 to finish card reset (if nessecery)
			fifoWaitValue32(FIFO_USER_03);

			// Wait for card to stablize before continuing
			for (int i = 0; i < 30; i++) { swiWaitForVBlank(); }

			sysSetCardOwner (BUS_OWNER_ARM9);

			cardReadHeader((uint8*)&ndsHeader);

			char gameTid[5];
			tonccpy(gameTid, ndsHeader.header.gameCode, 4);
			char pergamefilepath[256];
			sprintf(pergamefilepath, "/_nds/TWiLightMenu/gamesettings/slot1/%s.ini", gameTid);

			CIniFile pergameini(pergamefilepath);
			TWLMODE = pergameini.GetInt("GAMESETTINGS","DSI_MODE",-1);
			TWLCLK = setClockSpeed(pergameini.GetInt("GAMESETTINGS","BOOST_CPU",-1), gameTid, ignoreBlacklists);
			TWLVRAM = pergameini.GetInt("GAMESETTINGS","BOOST_VRAM",-1);

			if (TWLMODE == -1) {
				TWLMODE = DEFAULT_DSI_MODE;
			}
			if (TWLVRAM == -1) {
				TWLVRAM = DEFAULT_BOOST_VRAM;
			}

			//if (settingsini.GetInt("SRLOADER","DEBUG",0) == 1) {
			//	consoleOn = true;
			//	consoleDemoInit();
			//}

			if (!TWLCLK && (ndsHeader.header.unitCode == 0 || !TWLMODE)) {
				//if (settingsini.GetInt("TWL-MODE","DEBUG",0) == 1) {
				//	printf("TWL_CLOCK ON\n");		
				//}
				fifoSendValue32(FIFO_USER_04, 1);
				// Disabled for now. Doesn't result in correct SCFG_CLK configuration during testing. Will go back to old method.
				// setCpuClock(false);
				REG_SCFG_CLK = 0x80;
				swiWaitForVBlank();
			}

			//if (EnableSD) {
				//if (settingsini.GetInt("SRLOADER","DEBUG",0) == 1) {
				//	printf("SD access ON\n");		
				//}
			//}

			scfgUnlock = settingsini.GetInt("SRLOADER","SLOT1_SCFG_UNLOCK",0);

			if (settingsini.GetInt("SRLOADER","RESET_SLOT1",1) == 1) {
				fifoSendValue32(FIFO_USER_02, 1);
				fifoSendValue32(FIFO_USER_07, 1);
			}

			language = settingsini.GetInt("NDS-BOOTSTRAP", "LANGUAGE", -1);

			scanKeys();
			if (keysHeld() & KEY_A) {
				runCardEngine = !runCardEngine;
			}

		} else {
			//fifoSendValue32(FIFO_USER_02, 1);
			//fifoSendValue32(FIFO_USER_07, 1);

			// Tell Arm7 it's ready for card reset (if card reset is nessecery)
			fifoSendValue32(FIFO_USER_01, 1);
			// Waits for Arm7 to finish card reset (if nessecery)
			fifoWaitValue32(FIFO_USER_03);

			// Wait for card to stablize before continuing
			for (int i = 0; i < 30; i++) { swiWaitForVBlank(); }

			sysSetCardOwner (BUS_OWNER_ARM9);

			cardReadHeader((uint8*)&ndsHeader);
		}

		if (ndsHeader.header.unitCode > 0 && TWLMODE) {
			runCardEngine = false;
		}
	} else {
		sysSetCardOwner (BUS_OWNER_ARM9);

		if (io_dldi_data->ioInterface.features & FEATURE_SLOT_NDS) {
			consoleDemoInit();
			consoleInited = true;
			iprintf ("Please remove your flashcard.\n");
			do {
				swiWaitForVBlank();
				cardReadHeader((uint8*)&ndsHeader);
			} while (*(u32*)&ndsHeader != 0xffffffff);
			for (int i = 0; i < 60; i++) {
				swiWaitForVBlank();
			}
		}

		cardReadHeader((uint8*)&ndsHeader);

		if (*(u32*)&ndsHeader == 0xffffffff) {
			if (!consoleInited) {
				consoleDemoInit();
				consoleInited = true;
			} else {
				consoleClear();
			}
			if (io_dldi_data->ioInterface.features & FEATURE_SLOT_GBA) {
				iprintf ("Please insert a DS game\n");
				iprintf ("or flashcard.\n");
			} else {
				iprintf ("Please insert a DS game.\n");
			}
			do {
				swiWaitForVBlank();
				cardReadHeader((uint8*)&ndsHeader);
			} while (*(u32*)&ndsHeader == 0xffffffff);

			// Wait for card to stablize before continuing
			for (int i = 0; i < 30; i++) { swiWaitForVBlank(); }
		} else if (io_dldi_data->ioInterface.features & FEATURE_SLOT_GBA) {
			// Check header CRC
			if (ndsHeader.header.headerCRC16 != swiCRC16(0xFFFF, (void*)&ndsHeader, 0x15E)) {
				consoleDemoInit();
				consoleInited = true;
				iprintf ("Please reboot the console with\n");
				iprintf ("Slot-1 empty, and try again.\n");
				while (1);
			}
		}
	}

	while (1) {
		if (REG_SCFG_MC == 0x11) {
		break; } else {
			if (runCardEngine) {
				cheatData[3] = 0xCF;
				off_t wideCheatSize = getFileSize("sd:/_nds/nds-bootstrap/wideCheatData.bin");
				if (wideCheatSize > 0) {
					FILE* wideCheatFile = fopen("sd:/_nds/nds-bootstrap/wideCheatData.bin", "rb");
					fread(cheatData, 1, wideCheatSize, wideCheatFile);
					fclose(wideCheatFile);
					cheatData[wideCheatSize+3] = 0xCF;
				}
				tonccpy((void*)0x023F0000, cheatData, 0x8000);
			}
			const auto& gameCode = ndsHeader.header.gameCode;
			runLaunchEngine ((memcmp(gameCode, "UBRP", 4) == 0 || memcmp(gameCode, "AMFE", 4) == 0 || memcmp(gameCode, "ALXX", 4) == 0 || memcmp(ndsHeader.header.gameTitle, "D!S!XTREME", 10) == 0), (memcmp(gameCode, "UBRP", 4) == 0));
		}
	}
	return 0;
}

