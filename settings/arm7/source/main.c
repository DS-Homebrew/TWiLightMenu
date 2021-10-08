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

#define REG_SCFG_WL *(vu16*)0x4004020

void my_touchInit();
void my_installSystemFIFO(void);

//unsigned int * SCFG_ROM=(unsigned int*)0x4004000;
//unsigned int * SCFG_MC=(unsigned int*)0x4004010;
//unsigned int * CPUID=(unsigned int*)0x4004D00;
//unsigned int * CPUID2=(unsigned int*)0x4004D04;

static int soundVolume = 127;

static int rebootTimer = 0;

//---------------------------------------------------------------------------------
void soundFadeOut() {
//---------------------------------------------------------------------------------
	soundVolume -= 5;
	if (soundVolume < 0) {
		soundVolume = 0;
	}
}

void limitedModeMemoryPit(void) {
	// arm7 is master of WRAM A-C
	REG_MBK9=0x00FFFF0F;

	// WRAM-A fully mapped to arm7
	*((vu32*)REG_MBK1)=0x8D85898D;

	// WRAM-B fully mapped to arm7
	*((vu32*)REG_MBK2)=0x8D85898D;
	*((vu32*)REG_MBK3)=0x9195999D;

	// WRAM-C fully mapped to arm7
	*((vu32*)REG_MBK4)=0x8D85898D;
	*((vu32*)REG_MBK5)=0x9195999D;

	// WRAM mapped to the 0x3700000 - 0x37FFFFF area 
	// WRAM-A mapped to the 0x37C0000 - 0x37FFFFF area : 256k
	REG_MBK6=0x080037C0;
	// WRAM-B mapped to the 0x3780000 - 0x37BFFFF area : 256k // why?
	REG_MBK7=0x07C03780;
	// WRAM-C mapped to the 0x3740000 - 0x377FFFF area : 256k
	REG_MBK8=0x07803740;

	REG_SCFG_EXT &= ~(1UL << 31); // Lock SCFG
}

void limitedMode(void) {
	// arm7 is master of WRAM-A, arm9 of WRAM-B & C
	REG_MBK9=0x3000000F;

	// WRAM-A fully mapped to arm7
	*((vu32*)REG_MBK1)=0x8D898581; // same as dsiware

	// WRAM-B fully mapped to arm9 // inverted order
	*((vu32*)REG_MBK2)=0x8C888480;
	*((vu32*)REG_MBK3)=0x9C989490;

	// WRAM-C fully mapped to arm9 // inverted order
	*((vu32*)REG_MBK4)=0x8C888480;
	*((vu32*)REG_MBK5)=0x9C989490;

	// WRAM mapped to the 0x3700000 - 0x37FFFFF area 
	// WRAM-A mapped to the 0x37C0000 - 0x37FFFFF area : 256k
	REG_MBK6=0x080037C0; // same as dsiware
	// WRAM-B mapped to the 0x3740000 - 0x37BFFFF area : 512k // why? only 256k real memory is there
	REG_MBK7=0x07C03740; // same as dsiware
	// WRAM-C mapped to the 0x3700000 - 0x373FFFF area : 256k
	REG_MBK8=0x07403700; // same as dsiware

	REG_SCFG_EXT &= ~(1UL << 31); // Lock SCFG
}

//---------------------------------------------------------------------------------
void ReturntoDSiMenu() {
//---------------------------------------------------------------------------------
	i2cWriteRegister(0x4A, 0x70, 0x01);		// Bootflag = Warmboot/SkipHealthSafety
	i2cWriteRegister(0x4A, 0x11, 0x01);		// Reset to DSi Menu
}

//---------------------------------------------------------------------------------
void VblankHandler(void) {
//---------------------------------------------------------------------------------
	resyncClock();
	if(fifoCheckValue32(FIFO_USER_01)) {
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

	my_touchInit();

	fifoInit();
	
	mmInstall(FIFO_MAXMOD);
	
	SetYtrigger(80);
	
	installSoundFIFO();
	my_installSystemFIFO();

	irqSet(IRQ_VCOUNT, VcountHandler);
	irqSet(IRQ_VBLANK, VblankHandler);

	irqEnable( IRQ_VBLANK | IRQ_VCOUNT );

	setPowerButtonCB(powerButtonCB);
	
	//fifoSendValue32(FIFO_USER_01, *SCFG_ROM);
	//fifoSendValue32(FIFO_USER_02, REG_SCFG_CLK);
	fifoSendValue32(FIFO_USER_03, REG_SCFG_EXT);
	//fifoSendValue32(FIFO_USER_04, *CPUID2);
	//fifoSendValue32(FIFO_USER_05, *CPUID);
	fifoSendValue32(FIFO_USER_07, *(u16*)(0x4004700));
	if (isDSiMode()) {
		*(u8*)(0x023FFD00) = 0xFF;
	}
	fifoSendValue32(FIFO_USER_06, 1);
	
	// Keep the ARM7 mostly idle
	while (!exitflag) {
		if ( 0 == (REG_KEYINPUT & (KEY_SELECT | KEY_START | KEY_L | KEY_R))) {
			exitflag = true;
		}
		if (isDSiMode() && *(u8*)(0x023FFD00) != 0xFF) {
			i2cWriteRegister(0x4A, 0x30, *(u8*)(0x023FFD00));
			if (*(u8*)(0x023FFD00) == 0x13) {
				REG_SCFG_WL &= BIT(0);
			} else {
				REG_SCFG_WL |= BIT(0);
			}
			*(u8*)(0x023FFD00) = 0xFF;
		}
		if (fifoCheckValue32(FIFO_USER_02)) {
			ReturntoDSiMenu();
		}
		if (fifoGetValue32(FIFO_USER_08) == 2) {
			limitedModeMemoryPit();
			fifoSendValue32(FIFO_USER_08, 0);
		} else if (fifoGetValue32(FIFO_USER_08) == 1) {
			limitedMode();
			fifoSendValue32(FIFO_USER_08, 0);
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

