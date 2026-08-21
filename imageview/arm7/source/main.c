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
#include "fpsAdjust.h"

void my_touchInit();
void my_installSystemFIFO(void);

u8 my_i2cReadRegister(u8 device, u8 reg);

static fpsa_t sActiveFpsa;

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
	void my_inputGetAndSend(void);
	my_inputGetAndSend();

	if (*(int*)0x02003004 == 2) {
		soundFadeIn();
	} else if (*(int*)0x02003004 == 1) {
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

	irqSet(IRQ_VBLANK, VblankHandler);
	irqEnable(IRQ_VBLANK);

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
		if (*(u32*)(0x2FFFD0C) == 0x43535046) {
			const u32 num = 76;
			const u32 den = 1;
			const int max = 78;

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
		}
		if (fifoCheckValue32(FIFO_USER_02)) {
			ReturntoDSiMenu();
		}
		swiWaitForVBlank();
	}
	return 0;
}

