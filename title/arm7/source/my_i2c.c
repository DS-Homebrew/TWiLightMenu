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

#include <nds/arm7/i2c.h>
#include <nds/bios.h>

static u32 i2cCurrentDelay = 0;

//---------------------------------------------------------------------------------
void my_i2cDelay() {
//---------------------------------------------------------------------------------
	i2cWaitBusy();
	swiDelay(i2cCurrentDelay);
}

//---------------------------------------------------------------------------------
void my_i2cStop(u8 arg0) {
//---------------------------------------------------------------------------------
	if(i2cCurrentDelay) {
		REG_I2CCNT = (arg0 << 5) | 0xC0;
		my_i2cDelay();
		REG_I2CCNT = 0xC5;
	} else REG_I2CCNT = (arg0 << 5) | 0xC1;
}


//---------------------------------------------------------------------------------
u8 my_i2cGetResult() {
//---------------------------------------------------------------------------------
	i2cWaitBusy();
	return (REG_I2CCNT >> 4) & 0x01;
}

//---------------------------------------------------------------------------------
u8 my_i2cGetData() {
//---------------------------------------------------------------------------------
	i2cWaitBusy();
	return REG_I2CDATA;
}

//---------------------------------------------------------------------------------
void my_i2cSetDelay(u8 device) {
//---------------------------------------------------------------------------------
	if (device == I2C_PM ) {
		i2cCurrentDelay = 0x180;
	} else {
		i2cCurrentDelay = 0;
	}
}

//---------------------------------------------------------------------------------
u8 my_i2cSelectDevice(u8 device) {
//---------------------------------------------------------------------------------
	i2cWaitBusy();
	REG_I2CDATA = device;
	REG_I2CCNT = 0xC2;
	return my_i2cGetResult();
}

//---------------------------------------------------------------------------------
u8 my_i2cSelectRegister(u8 reg) {
//---------------------------------------------------------------------------------
	my_i2cDelay();
	REG_I2CDATA = reg;
	REG_I2CCNT = 0xC0;
	return my_i2cGetResult();
}

//---------------------------------------------------------------------------------
u8 my_i2cWriteRegister(u8 device, u8 reg, u8 data) {
//---------------------------------------------------------------------------------
	my_i2cSetDelay(device);
	int i;

	for(i = 0; i < 8; i++) {
		if((my_i2cSelectDevice(device) != 0) && (my_i2cSelectRegister(reg) != 0)) {
			my_i2cDelay();
			REG_I2CDATA = data;
			my_i2cStop(0);
			if(my_i2cGetResult() != 0) return 1;
        }
		REG_I2CCNT = 0xC5;
    }

    return 0;
}

//---------------------------------------------------------------------------------
u8 my_i2cReadRegister(u8 device, u8 reg) {
//---------------------------------------------------------------------------------
	my_i2cSetDelay(device);
	int i;

	for(i = 0; i < 8; i++) {
		
		if((my_i2cSelectDevice(device) != 0) && (my_i2cSelectRegister(reg) != 0)) {
			my_i2cDelay();
			if(my_i2cSelectDevice(device | 1)) {
				my_i2cDelay();
				my_i2cStop(1);
				return my_i2cGetData();
			}
		}

		REG_I2CCNT = 0xC5;
	}

	return 0xff;
}