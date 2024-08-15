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
#ifndef I2C_ARM7_INCLUDE
#define I2C_ARM7_INCLUDE

#ifndef ARM7
#error i2c header is for ARM7 only
#endif

#include <nds/ndstypes.h>

#define REG_I2CDATA	(*(vu8 *)0x4004500)
#define REG_I2CCNT	(*(vu8 *)0x4004501)

static inline void i2cWaitBusy() {
	while (REG_I2CCNT & 0x80);
}

enum i2cDevices {
	I2C_CAM0	= 0x7A,
	I2C_CAM1	= 0x78,
	I2C_UNK1	= 0xA0,
	I2C_UNK2	= 0xE0,
	I2C_PM		= 0x4A,
	I2C_UNK3	= 0x40,
	I2C_GPIO	= 0x90
};

// Registers for Power Management (I2C_PM)
#define I2CREGPM_BATUNK		0x00
#define I2CREGPM_PWRIF		0x10
#define I2CREGPM_PWRCNT		0x11
#define I2CREGPM_MMCPWR		0x12
#define I2CREGPM_BATTERY	0x20
#define I2CREGPM_CAMLED		0x31
#define I2CREGPM_VOL		0x40
#define I2CREGPM_RESETFLAG	0x70

u8 i2cWriteRegister(u8 device, u8 reg, u8 data);
u8 i2cReadRegister(u8 device, u8 reg);

#endif // I2C_ARM7_INCLUDE