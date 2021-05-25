/*---------------------------------------------------------------------------------

	DSi "codec" Touchscreen/Sound Controller control for ARM7

	Copyright (C) 2017
		fincs

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

#include "my_codec.h"

//---------------------------------------------------------------------------------
static u8 my_readTSC(u8 reg) {
//---------------------------------------------------------------------------------

	while (REG_SPICNT & 0x80);

	REG_SPICNT = SPI_ENABLE | SPI_BAUD_4MHz | SPI_DEVICE_TOUCH | SPI_CONTINUOUS;
	REG_SPIDATA = 1 | (reg << 1);

	while (REG_SPICNT & 0x80);

	REG_SPICNT = SPI_ENABLE | SPI_BAUD_4MHz | SPI_DEVICE_TOUCH;
	REG_SPIDATA = 0;

	while (REG_SPICNT & 0x80);
	return REG_SPIDATA;
}

//---------------------------------------------------------------------------------
static void my_writeTSC(u8 reg, u8 value) {
//---------------------------------------------------------------------------------

	while (REG_SPICNT & 0x80);

	REG_SPICNT = SPI_ENABLE | SPI_BAUD_4MHz | SPI_DEVICE_TOUCH | SPI_CONTINUOUS;
	REG_SPIDATA = reg << 1;

	while (REG_SPICNT & 0x80);

	REG_SPICNT = SPI_ENABLE | SPI_BAUD_4MHz | SPI_DEVICE_TOUCH;
	REG_SPIDATA = value;
}

//---------------------------------------------------------------------------------
static void my_bankSwitchTSC(u8 bank) {
//---------------------------------------------------------------------------------

	static u8 curBank = 0x63;
	if (bank != curBank) {
		my_writeTSC(curBank == 0xFF ? 0x7F : 0x00, bank);
		curBank = bank;
	}
}

//---------------------------------------------------------------------------------
u8 my_cdcReadReg(u8 bank, u8 reg) {
//---------------------------------------------------------------------------------

	my_bankSwitchTSC(bank);
	return my_readTSC(reg);
}

//---------------------------------------------------------------------------------
void my_cdcReadRegArray(u8 bank, u8 reg, void* data, u8 size) {
//---------------------------------------------------------------------------------

	u8* out = (u8*)data;
	my_bankSwitchTSC(bank);

	while (REG_SPICNT & 0x80);

	REG_SPICNT = SPI_ENABLE | SPI_BAUD_4MHz | SPI_DEVICE_TOUCH | SPI_CONTINUOUS;
	REG_SPIDATA = 1 | (reg << 1);

	while (REG_SPICNT & 0x80);

	for (; size > 1; size--) {
		REG_SPIDATA = 0;
		while (REG_SPICNT & 0x80);
		*out++ = REG_SPIDATA;
	}

	REG_SPICNT = SPI_ENABLE | SPI_BAUD_4MHz | SPI_DEVICE_TOUCH;
	REG_SPIDATA = 0;

	while (REG_SPICNT & 0x80);

	*out++ = REG_SPIDATA;
}

//---------------------------------------------------------------------------------
void my_cdcWriteReg(u8 bank, u8 reg, u8 value) {
//---------------------------------------------------------------------------------

	my_bankSwitchTSC(bank);
	my_writeTSC(reg, value);
}

//---------------------------------------------------------------------------------
void my_cdcWriteRegMask(u8 bank, u8 reg, u8 mask, u8 value) {
//---------------------------------------------------------------------------------

	my_bankSwitchTSC(bank);
	my_writeTSC(reg, (my_readTSC(reg) &~ mask) | (value & mask));
}

//---------------------------------------------------------------------------------
void my_cdcWriteRegArray(u8 bank, u8 reg, const void* data, u8 size) {
//---------------------------------------------------------------------------------

	const u8* in = (u8*)data;
	my_bankSwitchTSC(bank);

	while (REG_SPICNT & 0x80);

	REG_SPICNT = SPI_ENABLE | SPI_BAUD_4MHz | SPI_DEVICE_TOUCH | SPI_CONTINUOUS;
	REG_SPIDATA = reg << 1;

	while (REG_SPICNT & 0x80);

	for (; size > 1; size--) {
		REG_SPIDATA = *in++;
		while (REG_SPICNT & 0x80);
	}

	REG_SPICNT = SPI_ENABLE | SPI_BAUD_4MHz | SPI_DEVICE_TOUCH;
	REG_SPIDATA = *in++;
}

//---------------------------------------------------------------------------------
void my_cdcTouchInit(void) {
//---------------------------------------------------------------------------------

	my_cdcWriteRegMask(CDC_TOUCHCNT, 0x0E, 0x80, 0<<7);
	my_cdcWriteRegMask(CDC_TOUCHCNT, 0x02, 0x18, 3<<3);
	my_cdcWriteReg    (CDC_TOUCHCNT, 0x0F, 0xA0);
	my_cdcWriteRegMask(CDC_TOUCHCNT, 0x0E, 0x38, 5<<3);
	my_cdcWriteRegMask(CDC_TOUCHCNT, 0x0E, 0x40, 0<<6);
	my_cdcWriteReg    (CDC_TOUCHCNT, 0x03, 0x8B);
	my_cdcWriteRegMask(CDC_TOUCHCNT, 0x05, 0x07, 4<<0);
	my_cdcWriteRegMask(CDC_TOUCHCNT, 0x04, 0x07, 6<<0);
	my_cdcWriteRegMask(CDC_TOUCHCNT, 0x04, 0x70, 4<<4);
	my_cdcWriteRegMask(CDC_TOUCHCNT, 0x12, 0x07, 0<<0);
	my_cdcWriteRegMask(CDC_TOUCHCNT, 0x0E, 0x80, 1<<7);
}

//---------------------------------------------------------------------------------
bool my_cdcTouchPenDown(void) {
//---------------------------------------------------------------------------------

	return (my_cdcReadReg(CDC_TOUCHCNT, 0x09) & 0xC0) != 0x40 && !(my_cdcReadReg(CDC_TOUCHCNT, 0x0E) & 0x02);
}

//---------------------------------------------------------------------------------
bool my_cdcTouchRead(touchPosition* pos) {
//---------------------------------------------------------------------------------

	u8 raw[4*2*5];
	u16 arrayX[5], arrayY[5], arrayZ1[5], arrayZ2[5];
	u32 sumX, sumY, sumZ1, sumZ2;
	int i;

	my_cdcReadRegArray(CDC_TOUCHDATA, 0x01, raw, sizeof(raw));

	for (i = 0; i < 5; i ++) {
		arrayX[i]  = (raw[i*2+ 0]<<8) | raw[i*2+ 1];
		arrayY[i]  = (raw[i*2+10]<<8) | raw[i*2+11];
		arrayZ1[i] = (raw[i*2+20]<<8) | raw[i*2+21];
		arrayZ2[i] = (raw[i*2+30]<<8) | raw[i*2+31];
		if ((arrayX[i] & 0xF000) || (arrayY[i] & 0xF000)) {
			pos->rawx = 0;
			pos->rawy = 0;
			return false;
		}
	}

	// TODO: For now we just average all values without removing inaccurate values
	sumX = 0;
	sumY = 0;
	sumZ1 = 0;
	sumZ2 = 0;
	for (i = 0; i < 5; i ++) {
		sumX += arrayX[i];
		sumY += arrayY[i];
		sumZ1 += arrayZ1[i];
		sumZ2 += arrayZ2[i];
	}

	pos->rawx = sumX / 5;
	pos->rawy = sumY / 5;
	pos->z1 = sumZ1 / 5;
	pos->z2 = sumZ2 / 5;
	return true;
}
