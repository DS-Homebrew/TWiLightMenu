/*---------------------------------------------------------------------------------

	default ARM7 core

		Copyright (C) 2005 - 2010
		Michael Noland (joat)
		Jason Rogers (dovoto)
		Dave Murphy (WinterMute)

	This software is provided 'as-is', without any express or implied
	warranty.  In no event will the authors be held liable for any
	damages arising from the use of this software.

	Permission is granted to anyone to use this software for any
	purpose, including commercial applications, and to alter it and
	redistribute it freely, subject to the following restrictions:

	1.	The origin of this software must not be misrepresented; you
		must not claim that you wrote the original software. If you use
		this software in a product, an acknowledgment in the product
		documentation would be appreciated but is not required.

	2.	Altered source versions must be plainly marked as such, and
		must not be misrepresented as being the original software.

	3.	This notice may not be removed or altered from any source
		distribution.

---------------------------------------------------------------------------------*/
#include <nds.h>
#include <string.h>
#include <maxmod7.h>

unsigned int * SCFG_EXT=(unsigned int*)0x4004008;

static int soundVolume = 127;
static int timeTilVolumeLevelRefresh = 0;

//static bool gotCartHeader = false;

//---------------------------------------------------------------------------------
void soundFadeOut() {
//---------------------------------------------------------------------------------
	soundVolume -= 3;
	if (soundVolume < 0) {
		soundVolume = 0;
	}
}

//---------------------------------------------------------------------------------
void ReturntoDSiMenu() {
//---------------------------------------------------------------------------------
	if (isDSiMode()) {
		i2cWriteRegister(0x4A, 0x70, 0x01);		// Bootflag = Warmboot/SkipHealthSafety
		i2cWriteRegister(0x4A, 0x11, 0x01);		// Reset to DSi Menu
	} else {
		u8 readCommand = readPowerManagement(0x10);
		readCommand |= BIT(0);
		writePowerManagement(0x10, readCommand);
	}
}

//---------------------------------------------------------------------------------
//void UpdateCardInfo(void) {
//---------------------------------------------------------------------------------
	//cardReadHeader((u8*)0x02000000);
//}

//---------------------------------------------------------------------------------
void VblankHandler(void) {
//---------------------------------------------------------------------------------
}

//---------------------------------------------------------------------------------
void VcountHandler() {
//---------------------------------------------------------------------------------
	inputGetAndSend();
}

volatile bool exitflag = false;

//---------------------------------------------------------------------------------
void powerButtonCB() {
//---------------------------------------------------------------------------------
	exitflag = true;
}

//---------------------------------------------------------------------------------
int main() {
//---------------------------------------------------------------------------------
    // nocashMessage("ARM7 main.c main");
	
	// clear sound registers
	dmaFillWords(0, (void*)0x04000400, 0x100);

	REG_SOUNDCNT |= SOUND_ENABLE;
	writePowerManagement(PM_CONTROL_REG, ( readPowerManagement(PM_CONTROL_REG) & ~PM_SOUND_MUTE ) | PM_SOUND_AMP );
	powerOn(POWER_SOUND);

	readUserSettings();
	ledBlink(0);

	irqInit();
	// Start the RTC tracking IRQ
	initClockIRQ();

	fifoInit();
	touchInit();

	mmInstall(FIFO_MAXMOD);

	SetYtrigger(80);

	installSoundFIFO();
	installSystemFIFO();
	
	fifoSendValue32(FIFO_USER_03, *SCFG_EXT);
	fifoSendValue32(FIFO_USER_08, *(u16*)(0x4004700)); //SNDEX_CNT
	fifoSendValue32(FIFO_USER_06, 1);

	irqSet(IRQ_VCOUNT, VcountHandler);
	irqSet(IRQ_VBLANK, VblankHandler);

	irqEnable( IRQ_VBLANK | IRQ_VCOUNT );

	setPowerButtonCB(powerButtonCB);
	
	

	// Keep the ARM7 mostly idle
	while (!exitflag) {
		if ( 0 == (REG_KEYINPUT & (KEY_SELECT | KEY_START | KEY_L | KEY_R))) {
			exitflag = true;
		}
		/*if (!gotCartHeader && fifoCheckValue32(FIFO_USER_04)) {
			UpdateCardInfo();
			fifoSendValue32(FIFO_USER_04, 0);
			gotCartHeader = true;
		}*/
		resyncClock();
		timeTilVolumeLevelRefresh++;
		if (timeTilVolumeLevelRefresh == 8) {
			if (isDSiMode()) {
				*(u8*)(0x023FF000) = i2cReadRegister(I2C_PM, I2CREGPM_VOL);
			}
			*(u8*)(0x023FF001) = (isDSiMode() ? i2cReadRegister(I2C_PM, I2CREGPM_BATTERY) : readPowerManagement(PM_BATTERY_REG));
			timeTilVolumeLevelRefresh = 0;
		}
		if (isDSiMode() && *(vu32*)(0x400481C) & BIT(4)) {
			*(u8*)(0x023FF002) = 2;
		} else if (isDSiMode() && *(vu32*)(0x400481C) & BIT(3)) {
			*(u8*)(0x023FF002) = 1;
		} else {
			*(u8*)(0x023FF002) = 0;
		}
		if(fifoCheckValue32(FIFO_USER_01)) {
			soundFadeOut();
		} else {
			soundVolume = 127;
		}
		REG_MASTER_VOLUME = soundVolume;
		if(fifoCheckValue32(FIFO_USER_02)) {
			ReturntoDSiMenu();
		}
		swiIntrWait(0, 1);
	}
	return 0;
}
