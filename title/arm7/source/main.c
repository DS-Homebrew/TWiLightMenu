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
#include "common/picoLoader7.h"
#include "fpsAdjust.h"

#define REG_SCFG_WL *(vu16*)0x4004020

void my_touchInit();
void my_installSystemFIFO(void);
void my_sdmmc_get_cid(int devicenumber, u32 *cid);

u8 my_i2cReadRegister(u8 device, u8 reg);
u8 my_i2cWriteRegister(u8 device, u8 reg, u8 data);

static fpsa_t sActiveFpsa;

#define BIT_SET(c, n) ((c) << (n))

#define SD_IRQ_STATUS (*(vu32*)0x400481C)

volatile int timeTilVolumeLevelRefresh = 0;
static int soundVolume = 127;
volatile int rebootTimer = 0;
volatile u32 status = 0;
static bool i2cBricked = false;

//static bool gotCartHeader = false;


//---------------------------------------------------------------------------------
void soundFadeOut() {
//---------------------------------------------------------------------------------
	soundVolume -= 5;
	if (soundVolume < 0) {
		soundVolume = 0;
	}
}

//---------------------------------------------------------------------------------
void ReturntoDSiMenu() {
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

typedef void (*pico_loader_7_func_t)(void);

static void resetDSPico() {
    memset((void*)0x40000B0, 0, 0x30);

    REG_IME = IME_DISABLE;
    REG_IE = 0;
    REG_IF = ~0;

    pload_header7_t* header7 = (pload_header7_t*)0x06000000;
    // header7->dldiDriver = (void*)0x037F8000;
    ((pico_loader_7_func_t)header7->entryPoint)();
}

//---------------------------------------------------------------------------------
void VblankHandler(void) {
//---------------------------------------------------------------------------------
	void my_inputGetAndSend(void);
	my_inputGetAndSend();

	resyncClock();
	if (fifoGetValue32(FIFO_USER_04) == 10) {
		my_i2cWriteRegister(0x4A, 0x71, 0x01);
		fifoSendValue32(FIFO_USER_04, 0);
	}
	if (fifoCheckValue32(FIFO_USER_01)) {
		soundFadeOut();
	} else {
		soundVolume = 127;
	}
	REG_MASTER_VOLUME = soundVolume;
}

static void vcountIrqLower()
{
    while (1)
    {
        if (sActiveFpsa.initial)
        {
            sActiveFpsa.initial = FALSE;
            break;
        }

        if (!sActiveFpsa.backJump)
            sActiveFpsa.cycleDelta += sActiveFpsa.targetCycles - ((u64)FPSA_CYCLES_PER_FRAME << 24);
        u32 linesToAdd = 0;
        while (sActiveFpsa.cycleDelta >= (s64)((u64)FPSA_CYCLES_PER_LINE << 23))
        {
            sActiveFpsa.cycleDelta -= (u64)FPSA_CYCLES_PER_LINE << 24;
            if (++linesToAdd == 5)
                break;
        }
        if (linesToAdd == 0)
        {
            sActiveFpsa.backJump = FALSE;
            break;
        }
        if (linesToAdd > 1)
        {
            sActiveFpsa.backJump = TRUE;
        }
        else
        {
            // don't set the backJump flag because the irq is not retriggered if the new vcount
            // is the same as the previous line
            sActiveFpsa.backJump = FALSE;
        }
        // ensure we won't accidentally run out of line time
        while (REG_DISPSTAT & DISP_IN_HBLANK)
            ;
        int curVCount = REG_VCOUNT;
        REG_VCOUNT = curVCount - (linesToAdd - 1);
        if (linesToAdd == 1)
            break;

        while (REG_VCOUNT >= curVCount)//FPSA_ADJUST_MAX_VCOUNT - 5)
            ;
        while (REG_VCOUNT < curVCount)//FPSA_ADJUST_MAX_VCOUNT - 5)
            ;
    }
    REG_IF = IRQ_VCOUNT;
}

static void vcountIrqHigher()
{
    if (sActiveFpsa.initial)
    {
        sActiveFpsa.initial = FALSE;
        return;
    }
    sActiveFpsa.cycleDelta += ((u64)FPSA_CYCLES_PER_FRAME << 24) - sActiveFpsa.targetCycles;
    u32 linesToSkip = 0;
    while (sActiveFpsa.cycleDelta >= (s64)((u64)FPSA_CYCLES_PER_LINE << 23))
    {
        sActiveFpsa.cycleDelta -= (u64)FPSA_CYCLES_PER_LINE << 24;
        if (++linesToSkip == sActiveFpsa.linesToSkipMax)
            break;
    }
    if (linesToSkip == 0)
        return;
    // ensure we won't accidentally run out of line time
    while (REG_DISPSTAT & DISP_IN_HBLANK)
        ;
    REG_VCOUNT = REG_VCOUNT + (linesToSkip + 1);
}

void fpsa_init(fpsa_t* fpsa)
{
    memset(fpsa, 0, sizeof(fpsa_t));
    fpsa->isStarted = FALSE;
    fpsa_setTargetFrameCycles(fpsa, (u64)FPSA_CYCLES_PER_FRAME << 24); // default to no adjustment
}

void fpsa_start(fpsa_t* fpsa)
{
    int irq = enterCriticalSection();
    do
    {
        if (fpsa->isStarted)
            break;
        if (fpsa->targetCycles == ((u64)FPSA_CYCLES_PER_FRAME << 24))
            break;
        irqDisable(IRQ_VCOUNT);
        fpsa->backJump = FALSE;
        fpsa->cycleDelta = 0;
        fpsa->initial = TRUE;
        fpsa->isFpsLower = fpsa->targetCycles >= ((u64)FPSA_CYCLES_PER_FRAME << 24);
        // prevent the irq from immediately happening
        while (REG_VCOUNT != FPSA_ADJUST_MAX_VCOUNT + 2)
            ;
        fpsa->isStarted = TRUE;
        if (fpsa->isFpsLower)
        {
            SetYtrigger(FPSA_ADJUST_MAX_VCOUNT - 5);
            irqSet(IRQ_VCOUNT, vcountIrqLower);
        }
        else
        {
            SetYtrigger(FPSA_ADJUST_MIN_VCOUNT);
            irqSet(IRQ_VCOUNT, vcountIrqHigher);
        }
        irqEnable(IRQ_VCOUNT);
    } while (0);
    leaveCriticalSection(irq);
}

void fpsa_stop(fpsa_t* fpsa)
{
    if (!fpsa->isStarted)
        return;
    fpsa->isStarted = FALSE;
    irqDisable(IRQ_VCOUNT);
}

void fpsa_setTargetFrameCycles(fpsa_t* fpsa, u64 cycles)
{
    fpsa->targetCycles = cycles;
}

void fpsa_setTargetFpsFraction(fpsa_t* fpsa, u32 num, u32 den)
{
    u64 cycles = (((double)FPSA_SYS_CLOCK * den * (1 << 24)) / num) + 0.5;
    fpsa_setTargetFrameCycles(fpsa, cycles);//((((u64)FPSA_SYS_CLOCK * (u64)den) << 24) + ((num + 1) >> 1)) / num);
	fpsa->linesToSkipMax = (num / den > 62) ? 55 : 5;
}


volatile bool exitflag = false;

//---------------------------------------------------------------------------------
void powerButtonCB() {
//---------------------------------------------------------------------------------
	exitflag = true;
}

extern u16 biosRead16(u32 addr);

void biosDump(void* dst, const void* src, u32 len)
{
	u16* _dst = (u16*)dst;

	for (u32 i = 0; i < len; i+=2)
	{
		_dst[i>>1] = biosRead16(((u32)src) + i);
	}
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

	*(u32*)0x02FFFDF0 = REG_SCFG_EXT;

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

	installSoundFIFO();
	my_installSystemFIFO();

	irqSet(IRQ_VBLANK, VblankHandler);
	irqEnable(IRQ_VBLANK);

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

		memset((void*)0x0CF80000, 0, 0x20);
		biosDump((void*)0x02F80020, (const void*)0x00000020, 0x7FE0);
	}

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

	if (REG_SNDEXTCNT == 0) {
		if (pmBacklight & 0xF0) { // DS Lite
			int backlightLevel = pmBacklight & 3; // Brightness
			*(int*)0x02003000 = backlightLevel;
		}
	}

	bool is3DS = false;

	if ((isDSiMode() || REG_SCFG_EXT != 0) && !i2cBricked) {
		fifoSendValue32(FIFO_USER_04, my_i2cReadRegister(0x4A, 0x71));

		// Check for 3DS
		u8 byteBak = my_i2cReadRegister(0x4A, 0x71);
		my_i2cWriteRegister(0x4A, 0x71, 0xD2);
		u8 byteNew = my_i2cReadRegister(0x4A, 0x71);
		fifoSendValue32(FIFO_USER_05, byteNew);
		is3DS = (byteNew != 0xD2);
		my_i2cWriteRegister(0x4A, 0x71, byteBak);
	}

	if (isDSiMode()) {
		*(u8*)(0x02FFFD00) = 0xFF;
		if (is3DS) {
			*(u8*)(0x02FFFD01) = (REG_SCFG_WL & BIT(0)) ? 0x13 : 0x12;
		} else {
			*(u8*)(0x02FFFD01) = i2cReadRegister(0x4A, 0x30);
		}
		*(u8*)(0x02FFFD02) = 0x77;
	}


	// Keep the ARM7 mostly idle
	while (!exitflag) {
		if ((REG_KEYINPUT & (KEY_SELECT | KEY_START | KEY_L | KEY_R)) == 0) {
			exitflag = true;
		}
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

		if (*(u32*)(0x2FFFD0C) == 0x54494D52) {
			if (rebootTimer == 60*2) {
				ReturntoDSiMenu();	// Reboot, if fat init code is stuck in a loop
				*(u32*)(0x2FFFD0C) = 0;
			}
			rebootTimer++;
		} else if (*(u32*)(0x2FFFD0C) == 0x454D4D43) {
			my_sdmmc_get_cid(true, (u32*)0x2FFD7BC);	// Get eMMC CID
			*(u32*)(0x2FFFD0C) = 0;
		} else if (*(u32*)(0x2FFFD0C) == 0x43535046) {
			const u32 num = 72000;
			const u32 den = 1001;
			const int max = (num / den > 62) ? 74 : 62;

			int vblankCount = 1;
			while (num * (vblankCount + 1) / den < max)
				vblankCount++;

			// safety
			if (num * vblankCount / den < max)
			{
				fpsa_init(&sActiveFpsa);
				fpsa_setTargetFpsFraction(&sActiveFpsa, num * vblankCount, den);
				fpsa_start(&sActiveFpsa);
			}

			*(u32*)(0x2FFFD0C) = 0;
		} else if (*(u32*)(0x2FFFD0C) == 0x4F434950) { // 'PICO'
			resetDSPico();
		}
		swiWaitForVBlank();
	}
	return 0;
}