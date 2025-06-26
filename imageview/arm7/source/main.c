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
#include <maxmod7.h>
#include "common/isPhatCheck.h"
#include "common/arm7status.h"

void my_touchInit();
void my_installSystemFIFO(void);

u8 my_i2cReadRegister(u8 device, u8 reg);

#define BIT_SET(c, n) ((c) << (n))

#define SD_IRQ_STATUS (*(vu32*)0x400481C)

static int soundVolume = 127;
volatile u32 status = 0;
static bool i2cBricked = false;

//---------------------------------------------------------------------------------
void soundFadeOut() {
//---------------------------------------------------------------------------------
	soundVolume -= 3;
	if (soundVolume < 0) {
		soundVolume = 0;
	}
}

//---------------------------------------------------------------------------------
void soundFadeIn() {
//---------------------------------------------------------------------------------
	soundVolume += 3;
	if (soundVolume > 127) {
		soundVolume = 127;
	}
}

//---------------------------------------------------------------------------------
void ReturntoDSiMenu() {
//---------------------------------------------------------------------------------
	if (isDSiMode() && !i2cBricked) {
		i2cWriteRegister(0x4A, 0x70, 0x01);		// Bootflag = Warmboot/SkipHealthSafety
		i2cWriteRegister(0x4A, 0x11, 0x01);		// Reset to DSi Menu
	} else {
		u8 readCommand = readPowerManagement(0x10);
		readCommand |= BIT(0);
		writePowerManagement(0x10, readCommand);
	}
}

//---------------------------------------------------------------------------------
void VblankHandler(void) {
//---------------------------------------------------------------------------------
	if (*(int*)0x02003004 == 2) {
		soundFadeIn();
	} else if (*(int*)0x02003004 == 1) {
		soundFadeOut();
	} else {
		soundVolume = 127;
	}
	REG_MASTER_VOLUME = soundVolume;
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
	if (exitflag) return;
	fifoSendValue32(FIFO_USER_01, 1);
	exitflag = true;
}

//---------------------------------------------------------------------------------
int main() {
//---------------------------------------------------------------------------------
	//nocashMessage("ARM7 main.c main");

	// Grab from DS header in GBA slot
	*(u16*)0x02FFFC36 = *(u16*)0x0800015E;	// Header CRC16
	*(u32*)0x02FFFC38 = *(u32*)0x0800000C;	// Game Code

	*(u32*)0x02FFFDF0 = REG_SCFG_EXT;

	// clear sound registers
	dmaFillWords(0, (void*)0x04000400, 0x100);

	REG_SOUNDCNT |= SOUND_ENABLE;
	writePowerManagement(PM_CONTROL_REG, ( readPowerManagement(PM_CONTROL_REG) & ~PM_SOUND_MUTE ) | PM_SOUND_AMP);
	powerOn(POWER_SOUND);

	readUserSettings();
	ledBlink(0);

	irqInit();
	// Start the RTC tracking IRQ
	initClockIRQ();

	my_touchInit();

	fifoInit();

	mmInstall(FIFO_MAXMOD);

	SetYtrigger(80);

	installSoundFIFO();
	my_installSystemFIFO();

	irqSet(IRQ_VCOUNT, VcountHandler);
	irqSet(IRQ_VBLANK, VblankHandler);

	irqEnable(IRQ_VBLANK | IRQ_VCOUNT);

	setPowerButtonCB(powerButtonCB);

	if (isDSiMode() || REG_SCFG_EXT != 0) {
		const u8 i2cVer = my_i2cReadRegister(0x4A, 0);
		i2cBricked = (i2cVer == 0 || i2cVer == 0xFF);
	}

	u8 pmBacklight = readPowerManagement(PM_BACKLIGHT_LEVEL);

	// 01: Fade Out
	// 02: Return
	// 03: status (Bit 0: hasRegulableBacklight, Bit 1: scfgSdmmcEnabled, Bit 2: REG_SNDEXTCNT, Bit 3: isDSPhat, Bit 4: i2cBricked)


	// 03: Status: Init/Volume/Battery/SD
	// https://problemkaputt.de/gbatek.htm#dsii2cdevice4ahbptwlchip
	// Battery is 7 bits -- bits 0-7
	// Volume is 00h to 1Fh = 5 bits -- bits 8-12
	// SD status -- bits 13-14
	// Init status -- bits 15-18 (Bit 0 (15): hasRegulableBacklight, Bit 1 (16): scfgSdmmcEnabled, Bit 2 (17): REG_SNDEXTCNT, Bit 3 (18): isDSPhat, Bit 4 (19): i2cBricked)

	*(vu32*)0x4004820 = 0x8B7F0305;

	u8 initStatus = (BIT_SET(!!(REG_SNDEXTCNT), SNDEXTCNT_BIT)
									| BIT_SET(*(vu32*)0x4004820, SCFGSDMMC_BIT)
									| BIT_SET(!!(pmBacklight & BIT(4) || pmBacklight & BIT(5) || pmBacklight & BIT(6) || pmBacklight & BIT(7)), BACKLIGHT_BIT)
									| BIT_SET(isPhat(), DSPHAT_BIT)
									| BIT_SET(i2cBricked, I2CBRICKED_BIT));

	*(vu32*)0x4004820 = 0;

	status = (status & ~INIT_MASK) | ((initStatus << INIT_OFF) & INIT_MASK);
	fifoSendValue32(FIFO_USER_03, status);

	// Keep the ARM7 mostly idle
	while (1) {
		if ( 0 == (REG_KEYINPUT & (KEY_SELECT | KEY_START | KEY_L | KEY_R))) {
			powerButtonCB();
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
		if (fifoCheckValue32(FIFO_USER_02)) {
			ReturntoDSiMenu();
		}
		swiWaitForVBlank();
	}
	return 0;
}

