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

void my_installSystemFIFO(void);
u8 my_cdcReadReg(u8 bank, u8 reg);
void my_cdcWriteReg(u8 bank, u8 reg, u8 value);

//---------------------------------------------------------------------------------
void ReturntoDSiMenu() {
//---------------------------------------------------------------------------------
	// This will skip the power-off/sleep mode screen when returning to HOME Menu
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

static void DSiTouchscreenMode(void) {
	if (my_cdcReadReg(CDC_SOUND, 0x22) != 0xF0) return;

	// Touchscreen
	my_cdcWriteReg(0, 0x01, 0x01);
	my_cdcWriteReg(0, 0x39, 0x66);
	my_cdcWriteReg(1, 0x20, 0x16);
	my_cdcWriteReg(0, 0x04, 0x00);
	my_cdcWriteReg(0, 0x12, 0x81);
	my_cdcWriteReg(0, 0x13, 0x82);
	my_cdcWriteReg(0, 0x51, 0x82);
	my_cdcWriteReg(0, 0x51, 0x00);
	my_cdcWriteReg(0, 0x04, 0x03);
	my_cdcWriteReg(0, 0x05, 0xA1);
	my_cdcWriteReg(0, 0x06, 0x15);
	my_cdcWriteReg(0, 0x0B, 0x87);
	my_cdcWriteReg(0, 0x0C, 0x83);
	my_cdcWriteReg(0, 0x12, 0x87);
	my_cdcWriteReg(0, 0x13, 0x83);
	my_cdcWriteReg(3, 0x10, 0x88);
	my_cdcWriteReg(4, 0x08, 0x7F);
	my_cdcWriteReg(4, 0x09, 0xE1);
	my_cdcWriteReg(4, 0x0A, 0x80);
	my_cdcWriteReg(4, 0x0B, 0x1F);
	my_cdcWriteReg(4, 0x0C, 0x7F);
	my_cdcWriteReg(4, 0x0D, 0xC1);
	my_cdcWriteReg(0, 0x41, 0x08);
	my_cdcWriteReg(0, 0x42, 0x08);
	my_cdcWriteReg(0, 0x3A, 0x00);
	my_cdcWriteReg(4, 0x08, 0x7F);
	my_cdcWriteReg(4, 0x09, 0xE1);
	my_cdcWriteReg(4, 0x0A, 0x80);
	my_cdcWriteReg(4, 0x0B, 0x1F);
	my_cdcWriteReg(4, 0x0C, 0x7F);
	my_cdcWriteReg(4, 0x0D, 0xC1);
	my_cdcWriteReg(1, 0x2F, 0x2B);
	my_cdcWriteReg(1, 0x30, 0x40);
	my_cdcWriteReg(1, 0x31, 0x40);
	my_cdcWriteReg(1, 0x32, 0x60);
	my_cdcWriteReg(0, 0x74, 0x82);
	my_cdcWriteReg(0, 0x74, 0x92);
	my_cdcWriteReg(0, 0x74, 0xD2);
	my_cdcWriteReg(1, 0x21, 0x20);
	my_cdcWriteReg(1, 0x22, 0xF0);
	my_cdcWriteReg(0, 0x3F, 0xD4);
	my_cdcWriteReg(1, 0x23, 0x44);
	my_cdcWriteReg(1, 0x1F, 0xD4);
	my_cdcWriteReg(1, 0x28, 0x4E);
	my_cdcWriteReg(1, 0x29, 0x4E);
	my_cdcWriteReg(1, 0x24, 0x9E);
	my_cdcWriteReg(1, 0x25, 0x9E);
	my_cdcWriteReg(1, 0x20, 0xD4);
	my_cdcWriteReg(1, 0x2A, 0x14);
	my_cdcWriteReg(1, 0x2B, 0x14);
	my_cdcWriteReg(1, 0x26, 0xA7);
	my_cdcWriteReg(1, 0x27, 0xA7);
	my_cdcWriteReg(0, 0x40, 0x00);
	my_cdcWriteReg(0, 0x3A, 0x60);

	// Finish up!
	my_cdcReadReg (CDC_TOUCHCNT, 0x02);
	my_cdcWriteReg(CDC_TOUCHCNT, 0x02, 0x00);
}

//---------------------------------------------------------------------------------
int main() {
//---------------------------------------------------------------------------------
	{
		REG_SCFG_ROM = 0x101;
		REG_SCFG_CLK = (BIT(0) | BIT(1) | BIT(2) | BIT(7) | BIT(8));
		const bool sdAccess = (REG_SCFG_EXT & BIT(18));
		REG_SCFG_EXT = 0x93FFFB06;
		if (!sdAccess) {
			REG_SCFG_EXT &= ~BIT(18);
		}
	}
	*(vu16*)(0x04004012) = 0x1988;
	*(vu16*)(0x04004014) = 0x264C;
	*(vu16*)(0x04004C02) = 0x4000;	// enable powerbutton irq (Fix for Unlaunch 1.3)

	DSiTouchscreenMode(); // Fix disabled audio, if DSi touch mode is set

	if ((REG_SNDEXTCNT & SNDEXTCNT_ENABLE) && !(REG_SNDEXTCNT & BIT(13))) {
		*(vu16*)0x04004700 |= BIT(13);	// Set 48khz sound/mic frequency
	}

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

	SetYtrigger(80);

	my_installSystemFIFO();

	irqSet(IRQ_VCOUNT, VcountHandler);
	irqSet(IRQ_VBLANK, VblankHandler);

	irqEnable( IRQ_VBLANK | IRQ_VCOUNT );

	setPowerButtonCB(powerButtonCB);

	bool isRegularDS = true; 
	if (REG_SNDEXTCNT != 0) isRegularDS = false; // If sound frequency setting is found, then the console is not a DS Phat/Lite
	fifoSendValue32(FIFO_USER_07, isRegularDS);
	// Keep the ARM7 mostly idle
	while (!exitflag) {
		if (fifoCheckValue32(FIFO_USER_01)) {
			if (!isRegularDS) {
				ReturntoDSiMenu(); // reboot into System Menu (power-off/sleep mode screen skipped)
			} else exitflag = true; // poweroff if DS
		}
		swiWaitForVBlank();
	}
	return 0;
}

