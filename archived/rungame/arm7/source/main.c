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

#define REG_SCFG_WL *(vu16*)0x4004020

void my_installSystemFIFO(void);

//---------------------------------------------------------------------------------
void ReturntoDSiMenu() {
//---------------------------------------------------------------------------------
	// This will skip the power-off/sleep mode screen when returning to HOME Menu
	i2cWriteRegister(0x4A, 0x70, 0x01);		// Bootflag = Warmboot/SkipHealthSafety
	i2cWriteRegister(0x4A, 0x11, 0x01);		// Reset to DSi Menu
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

//---------------------------------------------------------------------------------
int main() {
//---------------------------------------------------------------------------------
	REG_SCFG_ROM = 0x101;
	REG_SCFG_CLK = (BIT(0) | BIT(1) | BIT(2) | BIT(7) | BIT(8));
	REG_SCFG_EXT = 0x93FFFB06;
	*(vu16*)(0x04004012) = 0x1988;
	*(vu16*)(0x04004014) = 0x264C;
	*(vu16*)(0x04004C02) = 0x4000;	// enable powerbutton irq (Fix for Unlaunch 1.3)

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
	
	SetYtrigger(80);
	
	my_installSystemFIFO();

	irqSet(IRQ_VCOUNT, VcountHandler);
	irqSet(IRQ_VBLANK, VblankHandler);

	irqEnable( IRQ_VBLANK | IRQ_VCOUNT );

	setPowerButtonCB(powerButtonCB);
	
	fifoSendValue32(FIFO_USER_03, REG_SCFG_EXT);
	fifoSendValue32(FIFO_USER_06, 1);
	
	*(u8*)(0x02FFFD00) = 0xFF;
	*(u8*)(0x02FFFD02) = 0x77;

	// Keep the ARM7 mostly idle
	while (!exitflag) {
		if (isDSiMode() && *(u8*)(0x02FFFD00) != 0xFF) {
			i2cWriteRegister(0x4A, 0x30, *(u8*)(0x02FFFD00));
			if (*(u8*)(0x02FFFD00) == 0x13) {
				REG_SCFG_WL |= BIT(0);
			} else {
				REG_SCFG_WL &= ~BIT(0);
			}
			*(u8*)(0x02FFFD00) = 0xFF;
		}
		if (isDSiMode() && *(u8*)(0x02FFFD02) != 0x77) {
			if (i2cReadRegister(0x4A, 0x63) != *(u8*)(0x02FFFD02)) {
				i2cWriteRegister(0x4A, 0x63, *(u8*)(0x02FFFD02)); // Change power LED color
			}
			*(u8*)(0x02FFFD02) = 0x77;
		}
		if (fifoCheckValue32(FIFO_USER_08)) {
			ReturntoDSiMenu();
		}
		// fifocheck();
		swiWaitForVBlank();
	}
	return 0;
}

