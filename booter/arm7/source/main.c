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

#define SD_IRQ_STATUS (*(vu32*)0x400481C)

void my_installSystemFIFO(void);

//---------------------------------------------------------------------------------
void ReturntoDSiMenu() {
//---------------------------------------------------------------------------------
	// This will skip the power-off/sleep mode screen when returning to HOME Menu
	i2cWriteRegister(0x4A, 0x70, 0x01);		// Bootflag = Warmboot/SkipHealthSafety
	i2cWriteRegister(0x4A, 0x11, 0x01);		// Reset to DSi/3DS HOME Menu
}

//---------------------------------------------------------------------------------
void VblankHandler(void) {
//---------------------------------------------------------------------------------
	if (fifoCheckValue32(FIFO_USER_01)) {
		ReturntoDSiMenu();
	}
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
	if (isDSiMode()) {
		REG_SCFG_ROM = 0x101;
		REG_SCFG_CLK = (BIT(0) | BIT(1) | BIT(2) | BIT(7) | BIT(8));
		REG_SCFG_EXT = 0x93FFFB06;
		*(vu16*)(0x04004012) = 0x1988;
		*(vu16*)(0x04004014) = 0x264C;
		*(vu16*)(0x04004C02) = 0x4000;	// enable powerbutton irq (Fix for Unlaunch 1.3)

		if ((REG_SCFG_ROM & BIT(1)) || (REG_SCFG_ROM & BIT(9))) {
			ReturntoDSiMenu(); // Reboot if DS BIOS is set while in DSi mode
		}
	}

	*(vu16*)(0x04004700) |= BIT(13);	// Set 48khz sound/mic frequency

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
	
	fifoInit();
	
	SetYtrigger(80);
	
	my_installSystemFIFO();

	irqSet(IRQ_VCOUNT, VcountHandler);
	irqSet(IRQ_VBLANK, VblankHandler);

	irqEnable(IRQ_VBLANK | IRQ_VCOUNT);

	setPowerButtonCB(powerButtonCB);
	
	// Keep the ARM7 mostly idle
	while (!exitflag) {
		if ((REG_KEYINPUT & (KEY_SELECT | KEY_START | KEY_L | KEY_R)) == 0) {
			exitflag = true;
		}
		if (*(u32*)0x02FFFD0C == 0x54534453) { // 'SDST'
			fifoSendValue32(FIFO_USER_04, SD_IRQ_STATUS);
			*(u32*)0x02FFFD0C = 0;
		}
		// fifocheck();
		swiWaitForVBlank();
	}
	return 0;
}

