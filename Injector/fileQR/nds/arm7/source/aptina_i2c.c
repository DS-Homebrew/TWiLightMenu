/*---------------------------------------------------------------------------------

	I2C control for the ARM7

	Copyright (C) 2011
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

/*********************************************************
 * Based on libnds's i2c.twl.c, but modifed to work with *
 * u16 addresses and data for the Aptina cameras.        *
 * -Pk11, 2020                                           *
 ********************************************************/

#include "aptina_i2c.h"

#include <nds/bios.h>

// https://github.com/devkitPro/calico/blob/d0cffb09d3fedc40bf1a6ce52aa22112d8814e99/source/nds/arm7/i2c.twl.c#L54-L58
MK_INLINE bool i2cGetResult() {
	i2cWaitBusy();
	return (REG_I2C_CNT >> 4) & 0x01;
}

u8 aptGetData(u8 flags) {
	REG_I2CCNT = 0xC0 | flags;
	i2cWaitBusy();
	return REG_I2CDATA;
}

u8 aptSetData(u8 data, u8 flags) {
	REG_I2CDATA = data;
	REG_I2CCNT  = 0xC0 | flags;
	return i2cGetResult();
}

u8 aptSelectDevice(u8 device, u8 flags) {
	i2cWaitBusy();
	REG_I2CDATA = device;
	REG_I2CCNT  = 0xC0 | flags;
	return i2cGetResult();
}

u8 aptSelectRegister(u8 reg, u8 flags) {
	REG_I2CDATA = reg;
	REG_I2CCNT  = 0xC0 | flags;
	return i2cGetResult();
}

u8 aptWriteRegister(u8 device, u16 reg, u16 data) {
	int i;

	for(i = 0; i < 8; i++) {
		if(aptSelectDevice(device, I2C_START) && aptSelectRegister(reg >> 8, I2C_NONE) &&
		   aptSelectRegister(reg & 0xFF, I2C_NONE)) {
			if(aptSetData(data >> 8, I2C_NONE) && aptSetData(data & 0xFF, I2C_STOP))
				return 1;
		}
		REG_I2CCNT = 0xC5;
	}

	return 0;
}

u16 aptReadRegister(u8 device, u16 reg) {
	int i;

	for(i = 0; i < 8; i++) {
		if(aptSelectDevice(device, I2C_START) && aptSelectRegister(reg >> 8, I2C_NONE) &&
		   aptSelectRegister(reg & 0xFF, I2C_STOP)) {
			if(aptSelectDevice(device | 1, I2C_START)) {
				return (aptGetData(I2C_READ | I2C_ACK) << 8) | aptGetData(I2C_STOP | I2C_READ);
			}
		}

		REG_I2CCNT = 0xC5;
	}

	return 0xFFFF;
}
