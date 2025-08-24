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

#define SD_IRQ_STATUS (*(vu32*)0x400481C)

void my_installSystemFIFO(void);
void my_sdmmc_get_cid(int devicenumber, u32 *cid);

static void DSiTouchscreenMode(void) {
	// Touchscreen
	cdcWriteReg(0, 0x01, 0x01);
	cdcWriteReg(0, 0x39, 0x66);
	cdcWriteReg(1, 0x20, 0x16);
	cdcWriteReg(0, 0x04, 0x00);
	cdcWriteReg(0, 0x12, 0x81);
	cdcWriteReg(0, 0x13, 0x82);
	cdcWriteReg(0, 0x51, 0x82);
	cdcWriteReg(0, 0x51, 0x00);
	cdcWriteReg(0, 0x04, 0x03);
	cdcWriteReg(0, 0x05, 0xA1);
	cdcWriteReg(0, 0x06, 0x15);
	cdcWriteReg(0, 0x0B, 0x87);
	cdcWriteReg(0, 0x0C, 0x83);
	cdcWriteReg(0, 0x12, 0x87);
	cdcWriteReg(0, 0x13, 0x83);
	cdcWriteReg(3, 0x10, 0x88);
	cdcWriteReg(4, 0x08, 0x7F);
	cdcWriteReg(4, 0x09, 0xE1);
	cdcWriteReg(4, 0x0A, 0x80);
	cdcWriteReg(4, 0x0B, 0x1F);
	cdcWriteReg(4, 0x0C, 0x7F);
	cdcWriteReg(4, 0x0D, 0xC1);
	cdcWriteReg(0, 0x41, 0x08);
	cdcWriteReg(0, 0x42, 0x08);
	cdcWriteReg(0, 0x3A, 0x00);
	cdcWriteReg(4, 0x08, 0x7F);
	cdcWriteReg(4, 0x09, 0xE1);
	cdcWriteReg(4, 0x0A, 0x80);
	cdcWriteReg(4, 0x0B, 0x1F);
	cdcWriteReg(4, 0x0C, 0x7F);
	cdcWriteReg(4, 0x0D, 0xC1);
	cdcWriteReg(1, 0x2F, 0x2B);
	cdcWriteReg(1, 0x30, 0x40);
	cdcWriteReg(1, 0x31, 0x40);
	cdcWriteReg(1, 0x32, 0x60);
	cdcWriteReg(0, 0x74, 0x82);
	cdcWriteReg(0, 0x74, 0x92);
	cdcWriteReg(0, 0x74, 0xD2);
	cdcWriteReg(1, 0x21, 0x20);
	cdcWriteReg(1, 0x22, 0xF0);
	cdcWriteReg(0, 0x3F, 0xD4);
	cdcWriteReg(1, 0x23, 0x44);
	cdcWriteReg(1, 0x1F, 0xD4);
	cdcWriteReg(1, 0x28, 0x4E);
	cdcWriteReg(1, 0x29, 0x4E);
	cdcWriteReg(1, 0x24, 0x9E);
	cdcWriteReg(1, 0x25, 0x9E);
	cdcWriteReg(1, 0x20, 0xD4);
	cdcWriteReg(1, 0x2A, 0x14);
	cdcWriteReg(1, 0x2B, 0x14);
	cdcWriteReg(1, 0x26, 0xA7);
	cdcWriteReg(1, 0x27, 0xA7);
	cdcWriteReg(0, 0x40, 0x00);
	cdcWriteReg(0, 0x3A, 0x60);

	// Finish up!
	cdcReadReg (CDC_TOUCHCNT, 0x02);
	cdcWriteReg(CDC_TOUCHCNT, 0x02, 0x00);
}

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
		*(vu16*)0x04004012 = 0x1988;
		*(vu16*)0x04004014 = 0x264C;
		*(vu32*)0x400481C = 0;				// Clear SD IRQ stat register
		*(vu32*)0x4004820 = 0;				// Clear SD IRQ mask register
		*(vu16*)0x04004C02 = 0x4000;	// enable powerbutton irq (Fix for Unlaunch 1.3)

		if ((REG_SCFG_ROM & BIT(1)) || (REG_SCFG_ROM & BIT(9))) {
			ReturntoDSiMenu(); // Reboot if DS BIOS is set while in DSi mode
		}
	}

	if (REG_SNDEXTCNT & SNDEXTCNT_ENABLE) {
		if (!(REG_SNDEXTCNT & BIT(13))) {
			*(vu16*)0x04004700 |= BIT(13);	// Set 48khz sound/mic frequency
		}
	} else if (isDSiMode()) {
		DSiTouchscreenMode();
		*(vu16*)0x4000500 = 0x807F;

		REG_SNDEXTCNT = SNDEXTCNT_FREQ_47KHZ | SNDEXTCNT_RATIO(8);
		cdcWriteReg(CDC_CONTROL, 6, 15);
		cdcWriteReg(CDC_CONTROL, 11, 0x85);
		cdcWriteReg(CDC_CONTROL, 18, 0x85);
		REG_SNDEXTCNT |= SNDEXTCNT_ENABLE;
	}

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
	
	// *(u8*)0x0280FFFF = i2cReadRegister(0x4A, 0x71);

	// Keep the ARM7 mostly idle
	while (!exitflag) {
		if ((REG_KEYINPUT & (KEY_SELECT | KEY_START | KEY_L | KEY_R)) == 0) {
			exitflag = true;
		}
		if (*(u32*)0x02FFFD0C == 0x54534453) { // 'SDST'
			fifoSendValue32(FIFO_USER_04, SD_IRQ_STATUS);
			*(u32*)0x02FFFD0C = 0;
		}
		if (*(u32*)0x02FFFD0C == 0x47444943) { // 'CIDG'
			my_sdmmc_get_cid(0, (u32*)0x02810000);
			*(u32*)0x02FFFD0C = 0;
		}
		// fifocheck();
		swiWaitForVBlank();
	}
	return 0;
}

