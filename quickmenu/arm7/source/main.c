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

#define REG_SCFG_WL *(vu16*)0x4004020

void my_touchInit();
void my_installSystemFIFO(void);
void my_sdmmc_get_cid(int devicenumber, u32 *cid);

u8 my_i2cReadRegister(u8 device, u8 reg);
u8 my_i2cWriteRegister(u8 device, u8 reg, u8 data);

#define BIT_SET(c, n) ((c) << (n))

#define SD_IRQ_STATUS (*(vu32*)0x400481C)

volatile int timeTilVolumeLevelRefresh = 0;
static int soundVolume = 127;
volatile int rebootTimer = 0;
volatile u32 status = 0;
static int backlightLevel = 0;
static bool isDSPhat = false;
static bool hasRegulableBacklight = false;
static bool i2cBricked = false;

//static bool gotCartHeader = false;


//---------------------------------------------------------------------------------
void soundFadeOut(void) {
//---------------------------------------------------------------------------------
	soundVolume -= 3;
	if (soundVolume < 0) {
		soundVolume = 0;
	}
}

//---------------------------------------------------------------------------------
void ReturntoDSiMenu(void) {
//---------------------------------------------------------------------------------
	nocashMessage("ARM7 ReturnToDSiMenu");
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
void changeBacklightLevel(void) {
//---------------------------------------------------------------------------------
	if (REG_SNDEXTCNT == 0) {
		// if the backlight is regulable the range will be 0 - 3
		// if the backlight is regulable and the console is a phat the range will be 0 - 4 (with 4 being backlight off)
		// if the backlight is not regulable the only possible values will be 0 and 4 (with 4 being backlight off)
		backlightLevel += 1 + (3 * !hasRegulableBacklight);
		
		if (backlightLevel > (3 + isDSPhat)) {
			backlightLevel = 0;
		}
		if (hasRegulableBacklight) {
			u8 pmBacklight = readPowerManagement(PM_BACKLIGHT_LEVEL);
			writePowerManagement(PM_BACKLIGHT_LEVEL, (pmBacklight & ~3) | (backlightLevel & 0x3));
		}

		if(backlightLevel == 4)
			writePowerManagement(PM_CONTROL_REG, readPowerManagement(PM_CONTROL_REG) & ~0xC);
		else
			writePowerManagement(PM_CONTROL_REG, readPowerManagement(PM_CONTROL_REG) | 0xC);
		return;
	}

	// DSi
	u8 backlightLevel = my_i2cReadRegister(0x4A, 0x41);
	backlightLevel++;
	if (backlightLevel > 4) {
		backlightLevel = 0;
	}
	my_i2cWriteRegister(0x4A, 0x41, backlightLevel);
}

//---------------------------------------------------------------------------------
void VblankHandler(void) {
//---------------------------------------------------------------------------------
	resyncClock();
	if (fifoCheckValue32(FIFO_USER_01)) {
		soundFadeOut();
	} else {
		soundVolume = 127;
	}
	REG_MASTER_VOLUME = soundVolume;
}

//---------------------------------------------------------------------------------
void VcountHandler(void) {
//---------------------------------------------------------------------------------
	void my_inputGetAndSend(void);
	my_inputGetAndSend();
}

volatile bool exitflag = false;

//---------------------------------------------------------------------------------
void powerButtonCB(void) {
//---------------------------------------------------------------------------------
	exitflag = true;
}

TWL_CODE void set_ctr(u32* ctr){
	for (int i = 0; i < 4; i++) REG_AES_IV[i] = ctr[3-i];
}

// 10 11  22 23 24 25
TWL_CODE void aes(void* in, void* out, void* iv, u32 method){ //this is sort of a bodged together dsi aes function adapted from this 3ds function
	REG_AES_CNT = ( AES_CNT_MODE(method) |           //https://github.com/TiniVi/AHPCFW/blob/master/source/aes.c#L42
					AES_WRFIFO_FLUSH |				 //as long as the output changes when keyslot values change, it's good enough.
					AES_RDFIFO_FLUSH |
					AES_CNT_KEY_APPLY |
					AES_CNT_KEYSLOT(3) |
					AES_CNT_DMA_WRITE_SIZE(2) |
					AES_CNT_DMA_READ_SIZE(1)
					);

	if (iv != NULL) set_ctr((u32*)iv);
	REG_AES_BLKCNT = (1 << 16);
	REG_AES_CNT |= 0x80000000;

	for (int j = 0; j < 0x10; j+=4) REG_AES_WRFIFO = *((u32*)(in+j));
	while (((REG_AES_CNT >> 0x5) & 0x1F) < 0x4); //wait for every word to get processed
	for (int j = 0; j < 0x10; j+=4) *((u32*)(out+j)) = REG_AES_RDFIFO;
	//REG_AES_CNT &= ~0x80000000;
	//if (method & (AES_CTR_DECRYPT | AES_CTR_ENCRYPT)) add_ctr((u8*)iv);
}

TWL_CODE void getConsoleID(void) {
	// Fix duplicated line bug on 3DS
	while (REG_VCOUNT != 191);
	while (REG_VCOUNT == 191);

	u8 base[16]={0};
	u8 in[16]={0};
	u8 iv[16]={0};
	u8 *scratch=(u8*)0x02F00200;
	u8 *out=(u8*)0x02F00000;
	u8 *key3=(u8*)0x40044D0;

	aes(in, base, iv, 2);

	//write consecutive 0-255 values to any byte in key3 until we get the same aes output as "base" above - this reveals the hidden byte. this way we can uncover all 16 bytes of the key3 normalkey pretty easily.
	//greets to Martin Korth for this trick https://problemkaputt.de/gbatek.htm#dsiaesioports (Reading Write-Only Values)
	for (int i=0;i<16;i++){
		for (int j=0;j<256;j++){
			*(key3+i)=j & 0xFF;
			aes(in, scratch, iv, 2);
			if (memcmp(scratch, base, 16)==0){
				out[i]=j;
				//hit++;
				break;
			}
		}
	}
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

	irqEnable(IRQ_VBLANK | IRQ_VCOUNT);

	setPowerButtonCB(powerButtonCB);

	if (isDSiMode() && REG_SCFG_EXT == 0) {
		u32 wordBak = *(vu32*)0x037C0000;
		*(vu32*)0x037C0000 = 0x414C5253;
		if (*(vu32*)0x037C0000 == 0x414C5253 && *(vu32*)0x037C8000 != 0x414C5253) {
			*(u32*)0x02FFE1A0 = 0x080037C0;
		}
		*(vu32*)0x037C0000 = wordBak;
	}

	if (isDSiMode()) {
		getConsoleID();
	}

	if (isDSiMode() || REG_SCFG_EXT != 0) {
		const u8 i2cVer = my_i2cReadRegister(0x4A, 0);
		i2cBricked = (i2cVer == 0 || i2cVer == 0xFF);
	}

	u8 pmBacklight = readPowerManagement(PM_BACKLIGHT_LEVEL);

	hasRegulableBacklight = !!(pmBacklight & BIT(4) || pmBacklight & BIT(5) || pmBacklight & BIT(6) || pmBacklight & BIT(7));
	isDSPhat = isPhat();

	// 01: Fade Out
	// 02: Return
	// 03: status (Bit 0: hasRegulableBacklight, Bit 1: scfgEnabled, Bit 2: REG_SNDEXTCNT, Bit 3: isDSPhat, Bit 4: i2cBricked)


	// 03: Status: Init/Volume/Battery/SD
	// https://problemkaputt.de/gbatek.htm#dsii2cdevice4ahbptwlchip
	// Battery is 7 bits -- bits 0-7
	// Volume is 00h to 1Fh = 5 bits -- bits 8-12
	// SD status -- bits 13-14
	// Init status -- bits 15-18 (Bit 0 (15): hasRegulableBacklight, Bit 1 (16): scfgEnabled, Bit 2 (17): REG_SNDEXTCNT, Bit 3 (18): isDSPhat, Bit 4 (19): i2cBricked)

	u8 initStatus = (BIT_SET(!!(REG_SNDEXTCNT), SNDEXTCNT_BIT)
									| BIT_SET(!!(REG_SCFG_EXT), REGSCFG_BIT)
									| BIT_SET(hasRegulableBacklight, BACKLIGHT_BIT)
									| BIT_SET(isDSPhat, DSPHAT_BIT)
									| BIT_SET(i2cBricked, I2CBRICKED_BIT));

	status = (status & ~INIT_MASK) | ((initStatus << INIT_OFF) & INIT_MASK);
	fifoSendValue32(FIFO_USER_03, status);

	if (REG_SNDEXTCNT == 0) {
		if (hasRegulableBacklight)
			backlightLevel = pmBacklight & 3; // Brightness
		
		if((readPowerManagement(PM_CONTROL_REG) & 0xC) == 0) // DS Phat backlight off
			backlightLevel = 4;
	}


	// Keep the ARM7 mostly idle
	while (!exitflag) {
		if ((REG_KEYINPUT & (KEY_SELECT | KEY_START | KEY_L | KEY_R)) == 0) {
			exitflag = true;
		}
		if (REG_SNDEXTCNT == 0) {
			*(int*)0x02003000 = backlightLevel;
		}
		if (isDSiMode() && *(u8*)(0x02FFFD00) != 0xFF) {
			i2cWriteRegister(0x4A, 0x30, *(u8*)(0x02FFFD00));
			if (*(u8*)(0x02FFFD00) == 0x13) {
				REG_SCFG_WL &= BIT(0);
			} else {
				REG_SCFG_WL |= BIT(0);
			}
			*(u8*)(0x02FFFD00) = 0xFF;
		}
		/*if (!gotCartHeader && fifoCheckValue32(FIFO_USER_04)) {
			UpdateCardInfo();
			fifoSendValue32(FIFO_USER_04, 0);
			gotCartHeader = true;
		}*/


		timeTilVolumeLevelRefresh++;
		if (timeTilVolumeLevelRefresh == 8) {
			if ((isDSiMode() || REG_SCFG_EXT != 0) && !i2cBricked) { //vol
				status = (status & ~VOL_MASK) | ((my_i2cReadRegister(I2C_PM, I2CREGPM_VOL) << VOL_OFF) & VOL_MASK);
				status = (status & ~BAT_MASK) | ((my_i2cReadRegister(I2C_PM, I2CREGPM_BATTERY) << BAT_OFF) & BAT_MASK);
			} else {
				int battery = (readPowerManagement(PM_BATTERY_REG) & 1)?3:15;
				int backlight = readPowerManagement(PM_BACKLIGHT_LEVEL);
				if (backlight & (1<<6)) battery += (backlight & (1<<3))<<4;

				status = (status & ~BAT_MASK) | ((battery << BAT_OFF) & BAT_MASK);
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

		if (fifoCheckValue32(FIFO_USER_02)) {
			ReturntoDSiMenu();
		}

		if (fifoGetValue32(FIFO_USER_04) == 1) {
			changeBacklightLevel();
			fifoSendValue32(FIFO_USER_04, 0);
		}

		if (*(u32*)(0x2FFFD0C) == 0x54494D52) {
			if (rebootTimer == 60*2) {
				ReturntoDSiMenu();	// Reboot, if fat init code is stuck in a loop
				*(u32*)(0x2FFFD0C) = 0;
			}
			rebootTimer++;
		}

		if (*(u32*)(0x2FFFD0C) == 0x454D4D43) {
			my_sdmmc_get_cid(true, (u32*)0x2FFD7BC);	// Get eMMC CID
			*(u32*)(0x2FFFD0C) = 0;
		}
		swiWaitForVBlank();
	}
	return 0;
}