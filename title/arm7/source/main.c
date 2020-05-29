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

//unsigned int * SCFG_ROM=(unsigned int*)0x4004000;
//unsigned int * SCFG_CLK=(unsigned int*)0x4004004; 
unsigned int * SCFG_EXT=(unsigned int*)0x4004008;
unsigned int * SCFG_MC=(unsigned int*)0x4004010;
unsigned int * CPUID=(unsigned int*)0x4004D00;
unsigned int * CPUID2=(unsigned int*)0x4004D04;

static bool doFrameRateHackAgain = false;
static bool runFrameRateHack = false;
static bool isDSLite = false;

static u32 sRateCounter = 0;

void frameRateHack()
{
	doFrameRateHackAgain = !doFrameRateHackAgain;
	if (!doFrameRateHackAgain) return;

	sRateCounter += 484839276;
	if(sRateCounter >= 1116733440)
	{
		sRateCounter -= 1116733440;
		while(REG_DISPSTAT & DISP_IN_HBLANK);
		*(vu16*)0x04000006 = *(vu16*)0x04000006;//repeat line
	}
}

//---------------------------------------------------------------------------------
void ReturntoDSiMenu() {
//---------------------------------------------------------------------------------
	nocashMessage("ARM7 ReturnToDSiMenu");
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
	resyncClock();
	if(fifoGetValue32(FIFO_USER_01) == 10) {
		i2cWriteRegister(0x4A, 0x71, 0x01);
		fifoSendValue32(FIFO_USER_01, 0);
	}
}

//---------------------------------------------------------------------------------
void VcountHandler() {
//---------------------------------------------------------------------------------
	if (runFrameRateHack) {
		frameRateHack();
	}
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
    nocashMessage("ARM7 main.c main");

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

	touchInit();

	fifoInit();

	mmInstall(FIFO_MAXMOD);
	
	SetYtrigger(80);
	
	installSoundFIFO();
	installSystemFIFO();

	irqSet(IRQ_VCOUNT, VcountHandler);
	irqSet(IRQ_VBLANK, VblankHandler);

	irqEnable( IRQ_VBLANK | IRQ_VCOUNT );

	setPowerButtonCB(powerButtonCB);
	
	u8 readCommand = readPowerManagement(4);
	isDSLite = (readCommand & BIT(4) || readCommand & BIT(5) || readCommand & BIT(6) || readCommand & BIT(7));

	//fifoSendValue32(FIFO_USER_01, *SCFG_ROM);
	if (isDSiMode()) {
		fifoSendValue32(FIFO_USER_01, i2cReadRegister(0x4A, 0x71));
	}
	//fifoSendValue32(FIFO_USER_02, *SCFG_CLK);
	fifoSendValue32(FIFO_USER_03, *SCFG_EXT);
	fifoSendValue32(FIFO_USER_04, isDSLite);
	//fifoSendValue32(FIFO_USER_04, *CPUID2);
	//fifoSendValue32(FIFO_USER_05, *CPUID);
	fifoSendValue32(FIFO_USER_07, *(u16*)(0x4004700));
	if (isDSiMode()) {
		*(u8*)(0x023FFD00) = 0xFF;
		*(u8*)(0x023FFD01) = i2cReadRegister(0x4A, 0x30);
	}
	fifoSendValue32(FIFO_USER_06, 1);
	
	// Keep the ARM7 mostly idle
	while (!exitflag) {
		if ( 0 == (REG_KEYINPUT & (KEY_SELECT | KEY_START | KEY_L | KEY_R))) {
			exitflag = true;
		}
		if (isDSiMode() && *(u8*)(0x023FFD00) != 0xFF) {
			i2cWriteRegister(0x4A, 0x30, *(u8*)(0x023FFD00));
			*(u8*)(0x023FFD00) = 0xFF;
		}
		if (fifoCheckValue32(FIFO_USER_02)) {
			ReturntoDSiMenu();
		}
		if (fifoGetValue32(FIFO_USER_05) == 1) {
			SetYtrigger(202);
			REG_DISPSTAT |= DISP_YTRIGGER_IRQ;
			REG_DISPSTAT |= BIT(7);
			runFrameRateHack = true;
			fifoSendValue32(FIFO_USER_05, 0);
		}
		swiWaitForVBlank();
	}
	return 0;
}

