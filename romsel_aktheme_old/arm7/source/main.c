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
#include "common/isPhatCheck.h"
#include "common/arm7status.h"

#define BIT_SET(c, n) ((c) << (n))

void my_touchInit();
void my_installSystemFIFO(void);

#define SD_IRQ_STATUS (*(vu32*)0x400481C)

volatile int soundVolume = 127;
volatile int timeTilVolumeLevelRefresh = 0;
volatile int status = 0;

volatile int rebootTimer = 0;
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
	resyncClock();
}

//---------------------------------------------------------------------------------
void VcountHandler() {
//---------------------------------------------------------------------------------
	void my_inputGetAndSend(void);
	my_inputGetAndSend();
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
	//nocashMessage("ARM7 main.c main");

	// Grab from DS header in GBA slot
	*(u16*)0x02FFFC36 = *(u16*)0x0800015E;	// Header CRC16
	*(u32*)0x02FFFC38 = *(u32*)0x0800000C;	// Game Code
	
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
	my_touchInit();

	SetYtrigger(80);
	
	installSoundFIFO();
	my_installSystemFIFO();

	irqSet(IRQ_VCOUNT, VcountHandler);
	irqSet(IRQ_VBLANK, VblankHandler);

	irqEnable( IRQ_VBLANK | IRQ_VCOUNT );

	setPowerButtonCB(powerButtonCB);

	u8 pmBacklight = readPowerManagement(PM_BACKLIGHT_LEVEL);

	// 01: Fade Out
	// 02: Return
	// 03: status (Bit 0: hasRegulableBacklight, Bit 1: scfgEnabled, Bit 2: REG_SNDEXTCNT, Bit 3: isDSPhat)


	// 03: Status: Init/Volume/Battery/SD
	// https://problemkaputt.de/gbatek.htm#dsii2cdevice4ahbptwlchip
	// Battery is 7 bits -- bits 0-7
	// Volume is 00h to 1Fh = 5 bits -- bits 8-12
	// SD status -- bits 13-14
	// Init status -- bits 15-18 (Bit 0 (15): hasRegulableBacklight, Bit 1 (16): scfgEnabled, Bit 2 (17): REG_SNDEXTCNT, Bit 3 (18): isDSPhat)

	u8 initStatus = (BIT_SET(!!(REG_SNDEXTCNT), SNDEXTCNT_BIT)
									| BIT_SET(!!(REG_SCFG_EXT), REGSCFG_BIT)
									| BIT_SET(!!(pmBacklight & BIT(4) || pmBacklight & BIT(5) || pmBacklight & BIT(6) || pmBacklight & BIT(7)), BACKLIGHT_BIT)
									| BIT_SET(isPhat(), DSPHAT_BIT));

	status = (status & ~INIT_MASK) | ((initStatus << INIT_OFF) & INIT_MASK);
	fifoSendValue32(FIFO_USER_03, status);


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

		timeTilVolumeLevelRefresh++;
		if (timeTilVolumeLevelRefresh == 8) {
			if (isDSiMode() || REG_SCFG_EXT != 0) { //vol
				status = (status & ~VOL_MASK) | ((i2cReadRegister(I2C_PM, I2CREGPM_VOL) << VOL_OFF) & VOL_MASK);
				status = (status & ~BAT_MASK) | ((i2cReadRegister(I2C_PM, I2CREGPM_BATTERY) << BAT_OFF) & BAT_MASK);				
			} else {
				status = (status & ~BAT_MASK) | ((readPowerManagement(PM_BATTERY_REG) << BAT_OFF) & BAT_MASK);
				// batteryLevel = readPowerManagement(PM_BATTERY_REG);
			}
			timeTilVolumeLevelRefresh = 0;
			fifoSendValue32(FIFO_USER_03, status);
		}

		if (isDSiMode()) {
			if (SD_IRQ_STATUS & BIT(4)) {
				status = (status & ~SD_MASK) | ((2 << SD_OFF) & SD_MASK);
				fifoSendValue32(FIFO_USER_03, status);
			} else if (SD_IRQ_STATUS & BIT(3)) {
				status = (status & ~SD_MASK) | ((1 << SD_OFF) & SD_MASK);
				fifoSendValue32(FIFO_USER_03, status);
			}
		}

		if (fifoCheckValue32(FIFO_USER_01)) {
			soundFadeOut();
		} else {
			soundVolume = 127;
		}
		REG_MASTER_VOLUME = soundVolume;
		if (fifoCheckValue32(FIFO_USER_02)) {
			ReturntoDSiMenu();
		}
		if (*(u32*)(0x2FFFD0C) == 0x54494D52) {
			if (rebootTimer == 60*2) {
				ReturntoDSiMenu();	// Reboot, if fat init code is stuck in a loop
				*(u32*)(0x2FFFD0C) = 0;
			}
			rebootTimer++;
		}
		swiWaitForVBlank();
	}
	return 0;
}