/*-----------------------------------------------------------------
 Copyright (C) 2005 - 2013
	Michael "Chishm" Chisholm
	Dave "WinterMute" Murphy
	Claudio "sverx"

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

------------------------------------------------------------------*/
#include <nds.h>
#include <stdio.h>
#include <fat.h>
#include <sys/stat.h>
#include <limits.h>

#include <string.h>
#include <unistd.h>
#include <gl2d.h>

#include "graphics/graphics.h"

#include "common/nds_loader_arm9.h"
#include "common/tonccpy.h"

#include "graphics/fontHandler.h"

bool fadeType = false;		// false = out, true = in
bool waitBeforeFade = false;

using namespace std;

//---------------------------------------------------------------------------------
void stop (void) {
//---------------------------------------------------------------------------------
	while (1) {
		swiWaitForVBlank();
	}
}

const char *unlaunchAutoLoadID = "AutoLoadInfo";

std::u16string utf8to16(std::string_view text) {
	std::u16string out;
	for (uint i=0;i<text.size();) {
		char16_t c = 0;
		if (!(text[i] & 0x80)) {
			c = text[i++];
		} else if ((text[i] & 0xE0) == 0xC0) {
			c  = (text[i++] & 0x1F) << 6;
			c |=  text[i++] & 0x3F;
		} else if ((text[i] & 0xF0) == 0xE0) {
			c  = (text[i++] & 0x0F) << 12;
			c |= (text[i++] & 0x3F) << 6;
			c |=  text[i++] & 0x3F;
		} else {
			i++; // out of range or something (This only does up to 0xFFFF since it goes to a U16 anyways)
		}
		out += c;
	}
	return out;
}

void unlaunchRomBoot(std::string_view rom) {
	std::u16string path(utf8to16(rom));
	if (path.substr(0, 3) == u"sd:") {
		path = u"sdmc:" + path.substr(3);
	}

	tonccpy((u8*)0x02000800, unlaunchAutoLoadID, 12);
	*(u16*)(0x0200080C) = 0x3F0;		// Unlaunch Length for CRC16 (fixed, must be 3F0h)
	*(u16*)(0x0200080E) = 0;			// Unlaunch CRC16 (empty)
	*(u32*)(0x02000810) = 0;			// Unlaunch Flags
	*(u32*)(0x02000810) |= BIT(0);		// Load the title at 2000838h
	*(u32*)(0x02000810) |= BIT(1);		// Use colors 2000814h
	*(u16*)(0x02000814) = 0x7FFF;		// Unlaunch Upper screen BG color (0..7FFFh)
	*(u16*)(0x02000816) = 0x7FFF;		// Unlaunch Lower screen BG color (0..7FFFh)
	toncset((u8*)0x02000818, 0, 0x20+0x208+0x1C0);		// Unlaunch Reserved (zero)
	for (uint i = 0; i < std::min(path.length(), 0x103u); i++) {
		((char16_t*)0x02000838)[i] = path[i];		// Unlaunch Device:/Path/Filename.ext (16bit Unicode,end by 0000h)
	}
	*(u16*)(0x0200080E) = swiCRC16(0xFFFF, (void*)0x02000810, 0x3F0);		// Unlaunch CRC16

	DC_FlushAll();						// Make reboot not fail
	fifoSendValue32(FIFO_USER_01, 1);	// Reboot into DSiWare title, booted via Unlaunch
	stop();
}

char filePath[PATH_MAX];

//---------------------------------------------------------------------------------
void doPause(int x, int y) {
//---------------------------------------------------------------------------------
	// iprintf("Press start...\n");
	printSmall(false, x, y, "Press start...");
	while (1) {
		scanKeys();
		if (keysDown() & KEY_START)
			break;
	}
	scanKeys();
}

//---------------------------------------------------------------------------------
int main(int argc, char **argv) {
//---------------------------------------------------------------------------------
	fifoSendValue32(FIFO_PM, PM_REQ_SLEEP_DISABLE);

	REG_SCFG_CLK &= ~(BIT(1) | BIT(2));	// Disable DSP and Camera Interface clocks

	// Turn on screen backlights if they're disabled
	powerOn(PM_BACKLIGHT_TOP);
	powerOn(PM_BACKLIGHT_BOTTOM);
	
	scanKeys();
	if ((keysHeld() & KEY_RIGHT) && (keysHeld() & KEY_A)) {
		graphicsInit();
		fontInit();
		fadeType = true;

		clearText();
		int yPos = 4;
		printSmall(false, 4, yPos, "Please remove the SD Card and");
		yPos += 8;
		printSmall(false, 4, yPos, "insert the SD Card containing");
		yPos += 8;
		printSmall(false, 4, yPos, "TWiLight Menu++, then press A");
		yPos += 8;
		printSmall(false, 4, yPos, "to continue.");

		// Prevent accidential presses
		for (int i = 0; i < 60; i++) {
			swiWaitForVBlank();
		}

		while (1) {
			scanKeys();
			if (keysHeld() & KEY_A) break;
			swiWaitForVBlank();
		}

		fadeType = false;
		for (int i = 0; i < 24; i++) {
			swiWaitForVBlank();
		}
	}

	*(u32*)0x0CFFFD0C = 0x54534453; // 'SDST'
	while (*(u32*)0x0CFFFD0C != 0) { swiDelay(100); }

	u32 sdIrqStatus = fifoGetValue32(FIFO_USER_04);
	if ((sdIrqStatus & BIT(5)) != 0 && (sdIrqStatus & BIT(7)) == 0) {
		graphicsInit();
		fontInit();
		fadeType = true;

		clearText();
		int yPos = 4;
		printSmall(false, 4, yPos, "The SD card is write-locked.");
		yPos += 8*2;
		printSmall(false, 4, yPos, "Please turn off the POWER,");
		yPos += 8;
		printSmall(false, 4, yPos, "remove the SD card, move the");
		yPos += 8;
		printSmall(false, 4, yPos, "write-lock switch up, re-insert");
		yPos += 8;
		printSmall(false, 4, yPos, "the SD card, then try again.");

		stop();
	}

	//bool runGame = (strncmp((char*)0x02FFFE0C, "SLRN", 4) == 0);

	const char* srldrPath = (/*runGame ? "sd:/_nds/TWiLightMenu/resetgame.srldr" :*/ "sd:/_nds/TWiLightMenu/main.srldr");

	/* if (*(u32*)0x02800000 == 0x00000041 || *(u32*)0x02800000 == 0x00060041) { // If using hiyaCFW...
		unlaunchRomBoot(srldrPath); // Start via Unlaunch
	} */

	graphicsInit();
	fontInit();
	clearText();

	waitBeforeFade = true;

	printSmall(false, 4, 4, "Please wait...");
	printSmall(false, 4, 12, "This may take a while.");

	fadeType = true;

	extern const DISC_INTERFACE __my_io_dsisd;
	bool fatInited = false;
	for (int i = 0; i < 30; i++) { // Run 30 times in case ntrboot is used
		fatInited = fatMountSimple("sd", &__my_io_dsisd);
		if (fatInited) {
			break;
		} else {
			swiWaitForVBlank();
		}
	}

	*(u32*)0x0CFFFD0C = 0x47444943; // 'CIDG'
	while (*(u32*)0x0CFFFD0C != 0) { swiDelay(100); }

	// DC_FlushRange((void*)0x02810000, 16);

	if (!fatInited && *(u16*)0x0C81000E == 0 && *(u8*)0x0C81000C == 0) {
		waitBeforeFade = false;

		clearText();
		int yPos = 4;
		printSmall(false, 4, yPos, "There is no SD card inserted.");
		yPos += 8;
		printSmall(false, 4, yPos, "Please insert the SD card");
		yPos += 8;
		printSmall(false, 4, yPos, "containing TWiLight Menu++,");
		yPos += 8;
		printSmall(false, 4, yPos, "then try again.");
		yPos += 8*2;
		printSmall(false, 4, yPos, "Press B to return to menu.");

		goto pressB;
	} /* else if (*(u8*)0x0C80FFFF != 0x01) {
		const u16 manufID = *(u16*)0x0C81000E;
		const char oemID[3] = {*(char*)0x0C81000D, *(char*)0x0C81000C, 0};

		// Source: https://web.archive.org/web/20130728131543/https://wiki.linaro.org/WorkingGroups/KernelArchived/Projects/FlashCardSurvey?action=show&redirect=WorkingGroups%2FKernel%2FProjects%2FFlashCardSurvey
		// and user findings
		if (
			(manufID == 0x0001 && strcmp(oemID, "PA") == 0) // Panasonic
		||	(manufID == 0x0002 && strcmp(oemID, "TM") == 0) // Kingston/PNY
		||	(manufID == 0x0003 && (strcmp(oemID, "SD") == 0 || strcmp(oemID, "PT") == 0)) // SanDisk
		||	(manufID == 0x0009 && strcmp(oemID, "AP") == 0) // Lexar
		||	(manufID == 0x001B && strcmp(oemID, "SM") == 0) // PNY/Samsung
		||	(manufID == 0x001D && strcmp(oemID, "AD") == 0) // (Some?) A-Data/extrememory/Microcenter
		||	(manufID == 0x0027 && strcmp(oemID, "PH") == 0) // Sony
		||	(manufID == 0x0028 && strcmp(oemID, "BE") == 0) // Lexar/Polaroid
		||	(manufID == 0x0041 && strcmp(oemID, "42") == 0) // Kingston
		||	(manufID == 0x009F && strcmp(oemID, "TI") == 0) // Kingston
		||	(manufID == 0x00AD && strcmp(oemID, "LS") == 0) // Lexar
		){
			// SD card is compatible!
		} else {
			char manufIdText[40];
			sprintf(manufIdText, "Manufacturer ID: %04X", manufID);
			char oemIdText[16];
			sprintf(oemIdText, "OEM ID: %s", oemID);

			waitBeforeFade = false;

			clearText();
			int yPos = 4;
			printSmall(false, 4, yPos, manufIdText);
			yPos += 8;
			printSmall(false, 4, yPos, oemIdText);
			yPos += 8*2;
			if (
				(manufID == 0x0000 && strcmp(oemID, "  ") == 0) // Brandless
			){
				printSmall(false, 4, yPos, "The inserted SD card is not");
				yPos += 8;
				printSmall(false, 4, yPos, "compatible. Please switch to an");
				yPos += 8;
				printSmall(false, 4, yPos, "SD card of a well-known brand.");
				yPos += 8*2;
				printSmall(false, 4, yPos, "Press B to return to menu.");

				goto pressB;
			} else {
				printSmall(false, 4, yPos, "The inserted SD card has not");
				yPos += 8;
				printSmall(false, 4, yPos, "been tested, and is possible that");
				yPos += 8;
				printSmall(false, 4, yPos, "it will not work correctly.");
				yPos += 8*2;
				printSmall(false, 4, yPos, "If you encounter any issues,");
				yPos += 8;
				printSmall(false, 4, yPos, "please switch to an SD card");
				yPos += 8;
				printSmall(false, 4, yPos, "of a well-known brand.");
				yPos += 8*2;
				printSmall(false, 4, yPos, "A: Continue");
				yPos += 8;
				printSmall(false, 4, yPos, "B: Return to menu");

				while (1) {
					scanKeys();
					if (keysHeld() & KEY_A) break;
					else if (keysHeld() & KEY_B) goto pressedB;
					swiWaitForVBlank();
				}
			}
		}
	} */

	if (!fatInited) {
		waitBeforeFade = false;
		// fadeType = true;

		clearText();
		printSmall(false, 4, 4, "FAT init failed!");
		printSmall(false, 4, 28, "Press B to return to menu.");
	} else {
		// Test code to extract device list
		// Only works, if booted via HiyaCFW or launched from 3DS HOME Menu
		/*FILE* deviceList = fopen("sd:/_nds/TWiLightMenu/deviceList.bin", "wb");
		fwrite((void*)0x02800000, 1, 0x400, deviceList);
		fclose(deviceList);*/

		if (!waitBeforeFade) {
			fadeType = false;
			for (int i = 0; i < 24; i++) {
				swiWaitForVBlank();
			}
		}

		vector<char *> argarray;
		argarray.push_back((char*)srldrPath);

		int err = runNdsFile(argarray[0], argarray.size(), (const char**)&argarray[0], true, true, false, false, true, true, false, -1);
		bool twlmFound = (access("sd:/_nds/TWiLightMenu", F_OK) == 0);

		waitBeforeFade = false;
		fadeType = true;

		int returnTextPos = 28;
		clearText();
		if (!twlmFound) {
			int yPos = 4;
			printSmall(false, 4, yPos, "The TWiLight Menu++ files are");
			yPos += 8;
			printSmall(false, 4, yPos, "missing. In order to start");
			yPos += 8;
			printSmall(false, 4, yPos, "TWiLight Menu++, please add the");
			yPos += 8;
			printSmall(false, 4, yPos, "missing files.");
			yPos += 8*2;
			printSmall(false, 4, yPos, "If they already exist, and the");
			yPos += 8;
			printSmall(false, 4, yPos, "above message still appears, then");
			yPos += 8;
			printSmall(false, 4, yPos, "the SD Card is formatted in a");
			yPos += 8;
			printSmall(false, 4, yPos, "way that TWiLight Menu++ cannot");
			yPos += 8;
			printSmall(false, 4, yPos, "start. Please reformat your");
			yPos += 8;
			printSmall(false, 4, yPos, "SD Card, or try another SD Card.");
			yPos += 8*2;
			returnTextPos = yPos;
		} else if (err == 1) {
			printSmall(false, 4, 4, "sd:/_nds/TWiLightMenu/");
			printSmall(false, 4, 12, /*runGame ? "resetgame.srldr not found." :*/ "main.srldr not found.");
		} else {
			char errorText[16];
			snprintf(errorText, sizeof(errorText), "Error %i", err);
			printSmall(false, 4, 4, /*runGame ? "Unable to start resetgame.srldr" :*/ "Unable to start main.srldr");
			printSmall(false, 4, 12, errorText);
		}
		printSmall(false, 4, returnTextPos, "Press B to return to menu.");
	}

pressB:
	while (1) {
		scanKeys();
		if (keysHeld() & KEY_B) break;
		swiWaitForVBlank();
	}

//pressedB:
	fadeType = false;
	for (int i = 0; i < 24; i++) {
		swiWaitForVBlank();
	}

	fifoSendValue32(FIFO_USER_01, 1);	// Tell ARM7 to reboot into 3DS HOME Menu (power-off/sleep mode screen skipped)

	while (1) {
		swiWaitForVBlank();
	}
}
