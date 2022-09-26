/*
    NitroHax -- Cheat tool for the Nintendo DS
    Copyright (C) 2008  Michael "Chishm" Chisholm

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "read_card.h"

#include <nds.h>
#include <nds/arm9/cache.h>
#include <nds/dma.h>
#include <nds/card.h>
#include <string.h>

#include "encryption.h"
#include "common/tonccpy.h"

enum {
	ERR_NONE         = 0x00,
	ERR_STS_CLR_MEM  = 0x01,
	ERR_STS_LOAD_BIN = 0x02,
	ERR_STS_HOOK_BIN = 0x03,
	ERR_STS_START    = 0x04,
		// initCard error codes:
		ERR_LOAD_NORM = 0x11,
		ERR_LOAD_OTHR = 0x12,
		ERR_SEC_NORM  = 0x13,
		ERR_SEC_OTHR  = 0x14,
		ERR_LOGO_CRC  = 0x15,
		ERR_HEAD_CRC  = 0x16,
} ERROR_CODES;

typedef union
{
	char title[4];
	u32 key;
} GameCode;

bool cardInited = false;
static bool twlBlowfish = false;
static bool normalChip = false;	// As defined by GBAtek, normal chip secure area is accessed in blocks of 0x200, other chip in blocks of 0x1000
static u32 portFlags = 0;
static u32 headerData[0x1000/sizeof(u32)] = {0};
static u32 secureArea[CARD_SECURE_AREA_SIZE/sizeof(u32)] = {0};
sNDSHeaderExt ndsCardHeader;

static const u8 cardSeedBytes[] = {0xE8, 0x4D, 0x5A, 0xB1, 0x17, 0x8F, 0x99, 0xD5};

static u32 getRandomNumber(void) {
	return 4;	// chosen by fair dice roll.
				// guaranteed to be random.
}

static void decryptSecureArea (u32 gameCode, u32* secureArea, int iCardDevice)
{
	init_keycode (gameCode, 2, 8, iCardDevice);
	crypt_64bit_down (secureArea);

	init_keycode (gameCode, 3, 8, iCardDevice);

	for (int i = 0; i < 0x200; i+= 2) {
		crypt_64bit_down (secureArea + i);
	}
}

static struct {
	unsigned int iii;
	unsigned int jjj;
	unsigned int kkkkk;
	unsigned int llll;
	unsigned int mmm;
	unsigned int nnn;
} key1data;


static void initKey1Encryption (u8* cmdData, int iCardDevice) {
	key1data.iii = getRandomNumber() & 0x00000fff;
	key1data.jjj = getRandomNumber() & 0x00000fff;
	key1data.kkkkk = getRandomNumber() & 0x000fffff;
	key1data.llll = getRandomNumber() & 0x0000ffff;
	key1data.mmm = getRandomNumber() & 0x00000fff;
	key1data.nnn = getRandomNumber() & 0x00000fff;

    if (iCardDevice) //DSi
      cmdData[7]=0x3D;	// CARD_CMD_ACTIVATE_BF2
    else
      cmdData[7]=CARD_CMD_ACTIVATE_BF;

	cmdData[6] = (u8) (key1data.iii >> 4);
	cmdData[5] = (u8) ((key1data.iii << 4) | (key1data.jjj >> 8));
	cmdData[4] = (u8) key1data.jjj;
	cmdData[3] = (u8) (key1data.kkkkk >> 16);
	cmdData[2] = (u8) (key1data.kkkkk >> 8);
	cmdData[1] = (u8) key1data.kkkkk;
	cmdData[0] = (u8) getRandomNumber();
}

// Note: cmdData must be aligned on a word boundary
static void createEncryptedCommand (u8 command, u8* cmdData, u32 block)
{
	unsigned long iii, jjj;

	if (command != CARD_CMD_SECURE_READ) {
		block = key1data.llll;
	}

	if (command == CARD_CMD_ACTIVATE_SEC) {
		iii = key1data.mmm;
		jjj = key1data.nnn;
	} else {
		iii = key1data.iii;
		jjj = key1data.jjj;
	}

	cmdData[7] = (u8) (command | (block >> 12));
	cmdData[6] = (u8) (block >> 4);
	cmdData[5] = (u8) ((block << 4) | (iii >> 8));
	cmdData[4] = (u8) iii;
	cmdData[3] = (u8) (jjj >> 4);
	cmdData[2] = (u8) ((jjj << 4) | (key1data.kkkkk >> 16));
	cmdData[1] = (u8) (key1data.kkkkk >> 8);
	cmdData[0] = (u8) key1data.kkkkk;

	crypt_64bit_up ((u32*)cmdData);

	key1data.kkkkk += 1;
}

static void cardDelay (u16 readTimeout) {
	/* Using a while loop to check the timeout,
	   so we have to wait until one before overflow.
	   This also requires an extra 1 for the timer data.
	   See GBATek for the normal formula used for card timeout.
	*/
	TIMER_DATA(0) = 0 - (((readTimeout & 0x3FFF) + 3));
	TIMER_CR(0)   = TIMER_DIV_256 | TIMER_ENABLE;
	while (TIMER_DATA(0) != 0xFFFF);

	// Clear out the timer registers
	TIMER_CR(0)   = 0;
	TIMER_DATA(0) = 0;
}

static void switchToTwlBlowfish(void) {
	if (!cardInited || twlBlowfish || ndsCardHeader.unitCode == 0) return;

	// Used for dumping the DSi arm9i/7i binaries

	u32 portFlagsKey1, portFlagsSecRead;
	int secureBlockNumber;
	int i;
	u8 cmdData[8] __attribute__ ((aligned));
	GameCode* gameCode;

	if (isDSiMode()) { 
		// Reset card slot
		disableSlot1();
		for (int i = 0; i < 25; i++) { swiWaitForVBlank(); }
		enableSlot1();
		for (int i = 0; i < 15; i++) { swiWaitForVBlank(); }

		// Dummy command sent after card reset
		cardParamCommand (CARD_CMD_DUMMY, 0,
			CARD_ACTIVATE | CARD_nRESET | CARD_CLK_SLOW | CARD_BLK_SIZE(1) | CARD_DELAY1(0x1FFF) | CARD_DELAY2(0x3F),
			NULL, 0);
	} else {
		REG_ROMCTRL=0;
		REG_AUXSPICNT=0;
		//ioDelay2(167550);
		for (i = 0; i < 25; i++) { swiWaitForVBlank(); }
		REG_AUXSPICNT=CARD_CR1_ENABLE|CARD_CR1_IRQ;
		REG_ROMCTRL=CARD_nRESET|CARD_SEC_SEED;
		while (REG_ROMCTRL&CARD_BUSY) ;
		cardReset();
		while (REG_ROMCTRL&CARD_BUSY) ;
	}

	//int iCardDevice = 1;

	// Initialise blowfish encryption for KEY1 commands and decrypting the secure area
	gameCode = (GameCode*)ndsCardHeader.gameCode;
	init_keycode (gameCode->key, 1, 8, 1);

	// Port 40001A4h setting for normal reads (command B7)
	portFlags = ndsCardHeader.cardControl13 & ~CARD_BLK_SIZE(7);
	// Port 40001A4h setting for KEY1 commands   (usually 001808F8h)
	portFlagsKey1 = CARD_ACTIVATE | CARD_nRESET | (ndsCardHeader.cardControl13 & (CARD_WR|CARD_CLK_SLOW)) |
		((ndsCardHeader.cardControlBF & (CARD_CLK_SLOW|CARD_DELAY1(0x1FFF))) + ((ndsCardHeader.cardControlBF & CARD_DELAY2(0x3F)) >> 16));

	// Adjust card transfer method depending on the most significant bit of the chip ID
	if (!normalChip) {
		portFlagsKey1 |= CARD_SEC_LARGE;
	}

	// 3Ciiijjj xkkkkkxx - Activate KEY1 Encryption Mode
	initKey1Encryption (cmdData, 1);
	cardPolledTransfer((ndsCardHeader.cardControl13 & (CARD_WR|CARD_nRESET|CARD_CLK_SLOW)) | CARD_ACTIVATE, NULL, 0, cmdData);

	// 4llllmmm nnnkkkkk - Activate KEY2 Encryption Mode
	createEncryptedCommand (CARD_CMD_ACTIVATE_SEC, cmdData, 0);

	if (normalChip) {
		cardPolledTransfer(portFlagsKey1, NULL, 0, cmdData);
		cardDelay(ndsCardHeader.readTimeout);
	}
	cardPolledTransfer(portFlagsKey1, NULL, 0, cmdData);

	// Set the KEY2 encryption registers
	REG_ROMCTRL = 0;
	REG_CARD_1B0 = cardSeedBytes[ndsCardHeader.deviceType & 0x07] | (key1data.nnn << 15) | (key1data.mmm << 27) | 0x6000;
	REG_CARD_1B4 = 0x879b9b05;
	REG_CARD_1B8 = key1data.mmm >> 5;
	REG_CARD_1BA = 0x5c;
	REG_ROMCTRL = CARD_nRESET | CARD_SEC_SEED | CARD_SEC_EN | CARD_SEC_DAT;

	// Update the DS card flags to suit KEY2 encryption
	portFlagsKey1 |= CARD_SEC_EN | CARD_SEC_DAT;

	// 1lllliii jjjkkkkk - 2nd Get ROM Chip ID / Get KEY2 Stream
	createEncryptedCommand (CARD_CMD_SECURE_CHIPID, cmdData, 0);

	if (normalChip) {
		cardPolledTransfer(portFlagsKey1, NULL, 0, cmdData);
		cardDelay(ndsCardHeader.readTimeout);
	}
	cardPolledTransfer(portFlagsKey1 | CARD_BLK_SIZE(7), NULL, 0, cmdData);

	// 2bbbbiii jjjkkkkk - Get Secure Area Block
	portFlagsSecRead = (ndsCardHeader.cardControlBF & (CARD_CLK_SLOW|CARD_DELAY1(0x1FFF)|CARD_DELAY2(0x3F)))
		| CARD_ACTIVATE | CARD_nRESET | CARD_SEC_EN | CARD_SEC_DAT;

    int secureAreaOffset = 0;
	for (secureBlockNumber = 4; secureBlockNumber < 8; secureBlockNumber++) {
		createEncryptedCommand (CARD_CMD_SECURE_READ, cmdData, secureBlockNumber);

		if (normalChip) {
			cardPolledTransfer(portFlagsSecRead, NULL, 0, cmdData);
			cardDelay(ndsCardHeader.readTimeout);
			for (i = 8; i > 0; i--) {
				cardPolledTransfer(portFlagsSecRead | CARD_BLK_SIZE(1), secureArea + secureAreaOffset, 0x200, cmdData);
				secureAreaOffset += 0x200/sizeof(u32);
			}
		} else {
			cardPolledTransfer(portFlagsSecRead | CARD_BLK_SIZE(4) | CARD_SEC_LARGE, secureArea + secureAreaOffset, 0x1000, cmdData);
			secureAreaOffset += 0x1000/sizeof(u32);
		}
	}

	// Alllliii jjjkkkkk - Enter Main Data Mode
	createEncryptedCommand (CARD_CMD_DATA_MODE, cmdData, 0);

	if (normalChip) {
		cardPolledTransfer(portFlagsKey1, NULL, 0, cmdData);
		cardDelay(ndsCardHeader.readTimeout);
    }
	cardPolledTransfer(portFlagsKey1, NULL, 0, cmdData);

	// The 0x800 bytes are modcrypted, so this function isn't ran
	//decryptSecureArea (gameCode->key, secureArea, 1);

	twlBlowfish = true;
}


void my_cardReset (bool properReset)
{
	int i;

	sysSetCardOwner (BUS_OWNER_ARM9);	// Allow arm9 to access NDS cart

	if (isDSiMode()) { 
		// Reset card slot
		disableSlot1();
		for (i = 0; i < 25; i++) { swiWaitForVBlank(); }
		enableSlot1();
		for (i = 0; i < 15; i++) { swiWaitForVBlank(); }

		if (!properReset) {
			// Dummy command sent after card reset
			cardParamCommand (CARD_CMD_DUMMY, 0,
				CARD_ACTIVATE | CARD_nRESET | CARD_CLK_SLOW | CARD_BLK_SIZE(1) | CARD_DELAY1(0x1FFF) | CARD_DELAY2(0x3F),
				NULL, 0);
		}
	}
	if (!isDSiMode() || properReset) {
		REG_ROMCTRL=0;
		REG_AUXSPICNT=0;
		//ioDelay2(167550);
		for (i = 0; i < 25; i++) { swiWaitForVBlank(); }
		REG_AUXSPICNT=CARD_CR1_ENABLE|CARD_CR1_IRQ;
		REG_ROMCTRL=CARD_nRESET|CARD_SEC_SEED;
		while (REG_ROMCTRL&CARD_BUSY) ;
		cardReset();
		while (REG_ROMCTRL&CARD_BUSY) ;
	}

	toncset(headerData, 0, 0x1000);
}

int cardInit (void)
{
	u32 portFlagsKey1, portFlagsSecRead;
	normalChip = false;	// As defined by GBAtek, normal chip secure area is accessed in blocks of 0x200, other chip in blocks of 0x1000
	int secureBlockNumber;
	int i;
	u8 cmdData[8] __attribute__ ((aligned));
	GameCode* gameCode;

	twlBlowfish = false;

	sysSetCardOwner (BUS_OWNER_ARM9);	// Allow arm9 to access NDS cart

	u32 iCardId=cardReadID(CARD_CLK_SLOW);	
	while (REG_ROMCTRL & CARD_BUSY);
	//u32 iCheapCard=iCardId&0x80000000;

	// Read the header
	cardParamCommand (CARD_CMD_HEADER_READ, 0,
		CARD_ACTIVATE | CARD_nRESET | CARD_CLK_SLOW | CARD_BLK_SIZE(1) | CARD_DELAY1(0x1FFF) | CARD_DELAY2(0x3F),
		(void*)headerData, 0x200/sizeof(u32));

	tonccpy(&ndsCardHeader, headerData, sizeof(sNDSHeaderExt));

	if ((ndsCardHeader.unitCode != 0) || (ndsCardHeader.dsi_flags != 0)) {
		// Extended header found
		cardParamCommand (CARD_CMD_HEADER_READ, 0,
			CARD_ACTIVATE | CARD_nRESET | CARD_CLK_SLOW | CARD_BLK_SIZE(4) | CARD_DELAY1(0x1FFF) | CARD_DELAY2(0x3F),
			(void*)headerData, 0x1000/sizeof(u32));
		if (ndsCardHeader.dsi1[0]==0xFFFFFFFF && ndsCardHeader.dsi1[1]==0xFFFFFFFF
		 && ndsCardHeader.dsi1[2]==0xFFFFFFFF && ndsCardHeader.dsi1[3]==0xFFFFFFFF) {
			toncset((u8*)headerData+0x200, 0, 0xE00);	// Clear out FFs
		}
		tonccpy(&ndsCardHeader, headerData, sizeof(sNDSHeaderExt));
	}

	// Check header CRC
	if (ndsCardHeader.headerCRC16 != swiCRC16(0xFFFF, (void*)&ndsCardHeader, 0x15E)) {
		toncset(&ndsCardHeader, 0, sizeof(sNDSHeaderExt));
		toncset(headerData, 0, 0x1000);
		cardInited = false;
		return ERR_HEAD_CRC;
	}

	/*
	// Check logo CRC
	if (ndsCardHeader.logoCRC16 != 0xCF56) {
		return ERR_LOGO_CRC;
	}
	*/

	// Initialise blowfish encryption for KEY1 commands and decrypting the secure area
	gameCode = (GameCode*)ndsCardHeader.gameCode;
	init_keycode (gameCode->key, 2, 8, 0);

	// Port 40001A4h setting for normal reads (command B7)
	portFlags = ndsCardHeader.cardControl13 & ~CARD_BLK_SIZE(7);
	// Port 40001A4h setting for KEY1 commands   (usually 001808F8h)
	portFlagsKey1 = CARD_ACTIVATE | CARD_nRESET | (ndsCardHeader.cardControl13 & (CARD_WR|CARD_CLK_SLOW)) |
		((ndsCardHeader.cardControlBF & (CARD_CLK_SLOW|CARD_DELAY1(0x1FFF))) + ((ndsCardHeader.cardControlBF & CARD_DELAY2(0x3F)) >> 16));

	// Adjust card transfer method depending on the most significant bit of the chip ID
	normalChip = (iCardId & 0x80000000) != 0;		// ROM chip ID MSB
	if (!normalChip) {
		portFlagsKey1 |= CARD_SEC_LARGE;
	}

	// 3Ciiijjj xkkkkkxx - Activate KEY1 Encryption Mode
	initKey1Encryption (cmdData, 0);
	cardPolledTransfer((ndsCardHeader.cardControl13 & (CARD_WR|CARD_nRESET|CARD_CLK_SLOW)) | CARD_ACTIVATE, NULL, 0, cmdData);

	// 4llllmmm nnnkkkkk - Activate KEY2 Encryption Mode
	createEncryptedCommand (CARD_CMD_ACTIVATE_SEC, cmdData, 0);

	if (normalChip) {
		cardPolledTransfer(portFlagsKey1, NULL, 0, cmdData);
		cardDelay(ndsCardHeader.readTimeout);
	}
	cardPolledTransfer(portFlagsKey1, NULL, 0, cmdData);

	// Set the KEY2 encryption registers
	REG_ROMCTRL = 0;
	REG_CARD_1B0 = cardSeedBytes[ndsCardHeader.deviceType & 0x07] | (key1data.nnn << 15) | (key1data.mmm << 27) | 0x6000;
	REG_CARD_1B4 = 0x879b9b05;
	REG_CARD_1B8 = key1data.mmm >> 5;
	REG_CARD_1BA = 0x5c;
	REG_ROMCTRL = CARD_nRESET | CARD_SEC_SEED | CARD_SEC_EN | CARD_SEC_DAT;

	// Update the DS card flags to suit KEY2 encryption
	portFlagsKey1 |= CARD_SEC_EN | CARD_SEC_DAT;

	// 1lllliii jjjkkkkk - 2nd Get ROM Chip ID / Get KEY2 Stream
	createEncryptedCommand (CARD_CMD_SECURE_CHIPID, cmdData, 0);

	if (normalChip) {
		cardPolledTransfer(portFlagsKey1, NULL, 0, cmdData);
		cardDelay(ndsCardHeader.readTimeout);
	}
	cardPolledTransfer(portFlagsKey1 | CARD_BLK_SIZE(7), NULL, 0, cmdData);

	// 2bbbbiii jjjkkkkk - Get Secure Area Block
	portFlagsSecRead = (ndsCardHeader.cardControlBF & (CARD_CLK_SLOW|CARD_DELAY1(0x1FFF)|CARD_DELAY2(0x3F)))
		| CARD_ACTIVATE | CARD_nRESET | CARD_SEC_EN | CARD_SEC_DAT;

    int secureAreaOffset = 0;
	for (secureBlockNumber = 4; secureBlockNumber < 8; secureBlockNumber++) {
		createEncryptedCommand (CARD_CMD_SECURE_READ, cmdData, secureBlockNumber);

		if (normalChip) {
			cardPolledTransfer(portFlagsSecRead, NULL, 0, cmdData);
			cardDelay(ndsCardHeader.readTimeout);
			for (i = 8; i > 0; i--) {
				cardPolledTransfer(portFlagsSecRead | CARD_BLK_SIZE(1), secureArea + secureAreaOffset, 0x200, cmdData);
				secureAreaOffset += 0x200/sizeof(u32);
			}
		} else {
			cardPolledTransfer(portFlagsSecRead | CARD_BLK_SIZE(4) | CARD_SEC_LARGE, secureArea + secureAreaOffset, 0x1000, cmdData);
			secureAreaOffset += 0x1000/sizeof(u32);
		}
	}

	// Alllliii jjjkkkkk - Enter Main Data Mode
	createEncryptedCommand (CARD_CMD_DATA_MODE, cmdData, 0);

	if (normalChip) {
		cardPolledTransfer(portFlagsKey1, NULL, 0, cmdData);
		cardDelay(ndsCardHeader.readTimeout);
    }
	cardPolledTransfer(portFlagsKey1, NULL, 0, cmdData);

    //CycloDS doesn't like the dsi secure area being decrypted
    if ((ndsCardHeader.arm9romOffset != 0x4000) || secureArea[0] || secureArea[1])
    {
		decryptSecureArea (gameCode->key, secureArea, 0);
	}

	if (secureArea[0] == 0x72636e65 /*'encr'*/ && secureArea[1] == 0x6a624f79 /*'yObj'*/) {
		// Secure area exists, so just clear the tag
		secureArea[0] = 0xe7ffdeff;
		secureArea[1] = 0xe7ffdeff;
	} else {
		//return normalChip ? ERR_SEC_NORM : ERR_SEC_OTHR;
	}

	cardInited = true;
	return ERR_NONE;
}

void cardRead (u32 src, void* dest, size_t len)
{
	size_t readSize;

	if (src >= 0 && src < 0x1000) {
		// Read header
		tonccpy (dest, (u8*)headerData + src, len);
		return;
	} else if ((src < CARD_SECURE_AREA_OFFSET) || !cardInited) {
		toncset (dest, 0, len);
		return;
	} else if (src < CARD_DATA_OFFSET) {
		// Read data from secure area
		readSize = src + len < CARD_DATA_OFFSET ? len : CARD_DATA_OFFSET - src;
		tonccpy (dest, (u8*)secureArea + src - CARD_SECURE_AREA_OFFSET, readSize);
		src += readSize;
		dest += readSize/sizeof(*dest);
		len -= readSize;
	} else if ((ndsCardHeader.unitCode != 0) && (src >= ndsCardHeader.arm9iromOffset) && (src < ndsCardHeader.arm9iromOffset+CARD_SECURE_AREA_SIZE)) {
		// Read data from secure area
		readSize = src + len < ndsCardHeader.arm9iromOffset ? len : ndsCardHeader.arm9iromOffset - src;
		tonccpy (dest, (u8*)secureArea + src - ndsCardHeader.arm9iromOffset, readSize);
		src += readSize;
		dest += readSize/sizeof(*dest);
		len -= readSize;
	}

	while (len > 0) {
		readSize = len < CARD_DATA_BLOCK_SIZE ? len : CARD_DATA_BLOCK_SIZE;
		cardParamCommand (CARD_CMD_DATA_READ, src,
			portFlags | CARD_ACTIVATE | CARD_nRESET | CARD_BLK_SIZE(1),
			dest, readSize/sizeof(u32));
		src += readSize;
		dest += readSize/sizeof(*dest);
		len -= readSize;
	}

	if (src > ndsCardHeader.romSize) {
		switchToTwlBlowfish();
	}
}

