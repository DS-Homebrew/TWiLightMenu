/*---------------------------------------------------------------------------------

	Copyright (C) 2005
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

#ifndef NDS_CARD_INCLUDE
#define NDS_CARD_INCLUDE


#include "ndstypes.h"

// Card bus
#define	REG_CARD_DATA_RD	(*(vu32*)0x04100010)

#define REG_AUXSPICNT	(*(vu16*)0x040001A0)
#define REG_AUXSPICNTH	(*(vu8*)0x040001A1)
#define REG_AUXSPIDATA	(*(vu8*)0x040001A2)
#define REG_ROMCTRL		(*(vu32*)0x040001A4)

#define REG_CARD_COMMAND	((vu8*)0x040001A8)

#define	REG_CARD_1B0	(*(vu32*)0x040001B0)
#define	REG_CARD_1B4	(*(vu32*)0x040001B4)
#define	REG_CARD_1B8	(*(vu16*)0x040001B8)
#define	REG_CARD_1BA	(*(vu16*)0x040001BA)

#define CARD_CR1_ENABLE  0x80  // in byte 1, i.e. 0x8000
#define CARD_CR1_IRQ     0x40  // in byte 1, i.e. 0x4000

// SPI EEPROM COMMANDS
#define SPI_EEPROM_WRSR   0x01
#define SPI_EEPROM_PP     0x02	// Page Program
#define SPI_EEPROM_READ   0x03
#define SPI_EEPROM_WRDI   0x04  // Write disable
#define SPI_EEPROM_RDSR   0x05  // Read status register
#define SPI_EEPROM_WREN   0x06  // Write enable
#define SPI_EEPROM_PW     0x0a	// Page Write
#define SPI_EEPROM_FAST   0x0b	// Fast Read
#define SPI_EEPROM_RDID   0x9f
#define SPI_EEPROM_RDP    0xab	// Release from deep power down
#define SPI_EEPROM_DPD    0xb9  // Deep power down

#define CARD_ACTIVATE     (1<<31)           // when writing, get the ball rolling
#define CARD_WR           (1<<30)           // Card write enable
#define CARD_nRESET       (1<<29)           // value on the /reset pin (1 = high out, not a reset state, 0 = low out = in reset)
#define CARD_SEC_LARGE    (1<<28)           // Use "other" secure area mode, which tranfers blocks of 0x1000 bytes at a time
#define CARD_CLK_SLOW     (1<<27)           // Transfer clock rate (0 = 6.7MHz, 1 = 4.2MHz)
#define CARD_BLK_SIZE(n)  (((n)&0x7)<<24)   // Transfer block size, (0 = None, 1..6 = (0x100 << n) bytes, 7 = 4 bytes)
#define CARD_SEC_CMD      (1<<22)           // The command transfer will be hardware encrypted (KEY2)
#define CARD_DELAY2(n)    (((n)&0x3F)<<16)  // Transfer delay length part 2
#define CARD_SEC_SEED     (1<<15)           // Apply encryption (KEY2) seed to hardware registers
#define CARD_SEC_EN       (1<<14)           // Security enable
#define CARD_SEC_DAT      (1<<13)           // The data transfer will be hardware encrypted (KEY2)
#define CARD_DELAY1(n)    ((n)&0x1FFF)      // Transfer delay length part 1

// 3 bits in b10..b8 indicate something
// read bits
#define CARD_BUSY         (1<<31)           // when reading, still expecting incomming data?
#define CARD_DATA_READY   (1<<23)           // when reading, CARD_DATA_RD or CARD_DATA has another word of data and is good to go

// Card commands
#define CARD_CMD_DUMMY          0x9F
#define CARD_CMD_HEADER_READ    0x00
#define CARD_CMD_HEADER_CHIPID  0x90
#define CARD_CMD_ACTIVATE_BF    0x3C  // Go into blowfish (KEY1) encryption mode
#define CARD_CMD_ACTIVATE_SEC   0x40  // Go into hardware (KEY2) encryption mode
#define CARD_CMD_SECURE_CHIPID  0x10
#define CARD_CMD_SECURE_READ    0x20
#define CARD_CMD_DISABLE_SEC    0x60  // Leave hardware (KEY2) encryption mode
#define CARD_CMD_DATA_MODE      0xA0
#define CARD_CMD_DATA_READ      0xB7
#define CARD_CMD_DATA_CHIPID    0xB8

//REG_AUXSPICNT
#define CARD_ENABLE			(1<<15)
#define CARD_SPI_ENABLE		(1<<13)
#define CARD_SPI_BUSY		(1<<7)
#define CARD_SPI_HOLD		(1<<6)

#define CARD_SPICNTH_ENABLE  (1<<7)  // in byte 1, i.e. 0x8000
#define CARD_SPICNTH_IRQ     (1<<6)  // in byte 1, i.e. 0x4000

#ifdef __cplusplus
extern "C" {
#endif


void enableSlot1();
void disableSlot1();

void cardWriteCommand(const u8 *command);
void cardPolledTransfer(u32 flags, u32 *destination, u32 length, const u8 *command);
void cardStartTransfer(const u8 *command, u32 *destination, int channel, u32 flags);
uint32 cardWriteAndRead(const u8 *command, u32 flags);
void cardParamCommand (u8 command, u32 parameter, u32 flags, u32 *destination, u32 length);

// These commands require the cart to not be initialized yet, which may mean the user
// needs to eject and reinsert the cart or they will return random data.
void cardReadHeader(u8 *header);
u32 cardReadID(u32 flags);
void cardReset();

//---------------------------------------------------------------------------------
static inline void eepromWaitBusy() {
//---------------------------------------------------------------------------------
	while (REG_AUXSPICNT & CARD_SPI_BUSY);
}

// Reads from the EEPROM
void cardReadEeprom(u32 address, u8 *data, u32 length, u32 addrtype);

// Writes to the EEPROM. TYPE 3 EEPROM must be erased first (I think?)
void cardWriteEeprom(u32 address, u8 *data, u32 length, u32 addrtype);

// Returns the ID of the EEPROM chip? Doesn't work well, most chips give ff,ff
// i = 0 or 1
u32 cardEepromReadID(); 

// Sends a command to the EEPROM
u8 cardEepromCommand(u8 command); 

/*
 * -1:no card or no EEPROM
 *  0:unknown                 PassMe?
 *  1:TYPE 1  4Kbit(512Byte)  EEPROM
 *  2:TYPE 2 64Kbit(8KByte)or 512kbit(64Kbyte)   EEPROM
 *  3:TYPE 3  2Mbit(256KByte) FLASH MEMORY (some rare 4Mbit and 8Mbit chips also)
 */
int cardEepromGetType(void);

// Returns the size in bytes of EEPROM
u32 cardEepromGetSize();

// Erases the entire chip. TYPE 3 chips MUST be erased before writing to them. (I think?)
void cardEepromChipErase(void);

// Erases a single sector of the TYPE 3 chip
void cardEepromSectorErase(u32 address);

#ifdef __cplusplus
}
#endif


#endif

