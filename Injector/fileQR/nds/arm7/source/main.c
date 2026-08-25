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

/******************************************
 * Modified to have pxi checking for I2C. *
 * -Pk11, 2020                            *
 *****************************************/

#include "i2c_handler.h"

// #include <calico.h>
#include <nds.h>
// #include <maxmod7.h>

//---------------------------------------------------------------------------------
int main() {
//---------------------------------------------------------------------------------

	// Read settings from NVRAM
	envReadNvramSettings();

	// Set up extended keypad server (X/Y/hinge)
	keypadStartExtServer();

	// Configure and enable VBlank interrupt
	lcdSetIrqMask(DISPSTAT_IE_ALL, DISPSTAT_IE_VBLANK);
	irqEnable(IRQ_VBLANK);

	// Set up RTC
	rtcInit();
	rtcSyncTime();

	// Initialize power management
	pmInit();

	// Set up block device peripherals
	blkInit();

	// Set up touch screen driver
	touchInit();
	touchStartServer(80, MAIN_THREAD_PRIO);

	// // Set up sound and mic driver
	// soundStartServer(MAIN_THREAD_PRIO-0x10);
	// micStartServer(MAIN_THREAD_PRIO-0x18);

	// // Set up wireless manager
	// wlmgrStartServer(MAIN_THREAD_PRIO-8);

	// // Set up Maxmod
	// mmInstall(MAIN_THREAD_PRIO+1);

	// Set up server thread
	threadPrepare(&s_i2cPxiThread, i2cPxiThreadMain, NULL, &s_i2cPxiThreadStack[sizeof(s_i2cPxiThreadStack)], MAIN_THREAD_PRIO);
	threadStart(&s_i2cPxiThread);

	// Keep the ARM7 mostly idle
	while (pmMainLoop()) {
		threadWaitForVBlank();
	}

	return 0;
}
