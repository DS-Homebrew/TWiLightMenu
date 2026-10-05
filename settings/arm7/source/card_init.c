/*
    NitroHax -- Cheat tool for the Nintendo DS
    Copyright (C) 2008 Michael "Chishm" Chisholm

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.

*/

#include "card_init.h"
#include "encryption.h"
#include <stdlib.h>
#include <string.h>

static struct {
	u32 iii, jjj, kkkkk, llll, mmm, nnn;
} key1data;

static void initKey1Command(u8* command) {
	key1data.iii = rand() & 0xFFF;
	key1data.jjj = rand() & 0xFFF;
	key1data.kkkkk = rand() & 0xFFFFF;
	key1data.llll = rand() & 0xFFFF;
	key1data.mmm = rand() & 0xFFF;
	key1data.nnn = rand() & 0xFFF;

	command[7] = CARD_CMD_ACTIVATE_BF;
	command[6] = key1data.iii >> 4;
	command[5] = (key1data.iii << 4) | (key1data.jjj >> 8);
	command[4] = key1data.jjj;
	command[3] = key1data.kkkkk >> 16;
	command[2] = key1data.kkkkk >> 8;
	command[1] = key1data.kkkkk;
	command[0] = rand();
}

static void encryptedCommand(u8 code, u8* command, u32 block) {
	u32 iii = key1data.iii;
	u32 jjj = key1data.jjj;
	if (code != CARD_CMD_SECURE_READ)
		block = key1data.llll;
	if (code == CARD_CMD_ACTIVATE_SEC) {
		iii = key1data.mmm;
		jjj = key1data.nnn;
	}

	command[7] = code | (block >> 12);
	command[6] = block >> 4;
	command[5] = (block << 4) | (iii >> 8);
	command[4] = iii;
	command[3] = jjj >> 4;
	command[2] = (jjj << 4) | (key1data.kkkkk >> 16);
	command[1] = key1data.kkkkk >> 8;
	command[0] = key1data.kkkkk;
	crypt_64bit_up((u32*)command);
	key1data.kkkkk++;
}

static void cardDelay(u16 timeout) {
	TIMER_DATA(0) = 0 - ((timeout & 0x3FFF) + 3);
	TIMER_CR(0) = TIMER_DIV_256 | TIMER_ENABLE;
	while (TIMER_DATA(0) != 0xFFFF);
	TIMER_CR(0) = 0;
	TIMER_DATA(0) = 0;
}

u32 initFlashcardArm7(void) {
	static const u8 seedBytes[] = {0xE8, 0x4D, 0x5A, 0xB1, 0x17, 0x8F, 0x99, 0xD5};
	static u32 headerData[0x200 / sizeof(u32)];
	tNDSHeader* header = (tNDSHeader*)headerData;
	u8 command[8] __attribute__((aligned(4)));

	cardReadHeader((u8*)headerData);
	if (header->headerCRC16 != swiCRC16(0xFFFF, headerData, 0x15E))
		return 0x16;

	u32 chipId = cardReadID(CARD_CLK_SLOW);
	while (REG_ROMCTRL & CARD_BUSY);
	bool normalChip = (chipId & BIT(31)) != 0;
	u32 gameCode;
	memcpy(&gameCode, header->gameCode, sizeof(gameCode));
	init_keycode(gameCode, 2, 8, NTR_CARD_KEY);

	u32 key1Flags = CARD_ACTIVATE | CARD_nRESET |
		(header->cardControl13 & (CARD_WR | CARD_CLK_SLOW)) |
		((header->cardControlBF & (CARD_CLK_SLOW | CARD_DELAY1(0x1FFF))) +
		((header->cardControlBF & CARD_DELAY2(0x3F)) >> 16));
	if (!normalChip)
		key1Flags |= CARD_SEC_LARGE;

	initKey1Command(command);
	cardPolledTransfer((header->cardControl13 & (CARD_WR | CARD_nRESET | CARD_CLK_SLOW)) |
		CARD_ACTIVATE, NULL, 0, command);
	encryptedCommand(CARD_CMD_ACTIVATE_SEC, command, 0);
	if (normalChip) {
		cardPolledTransfer(key1Flags, NULL, 0, command);
		cardDelay(header->readTimeout);
	}
	cardPolledTransfer(key1Flags, NULL, 0, command);

	REG_ROMCTRL = 0;
	REG_CARD_1B0 = seedBytes[header->deviceType & 7] |
		(key1data.nnn << 15) | (key1data.mmm << 27) | 0x6000;
	REG_CARD_1B4 = 0x879B9B05;
	REG_CARD_1B8 = key1data.mmm >> 5;
	REG_CARD_1BA = 0x5C;
	REG_ROMCTRL = CARD_nRESET | CARD_SEC_SEED | CARD_SEC_EN | CARD_SEC_DAT;
	key1Flags |= CARD_SEC_EN | CARD_SEC_DAT;

	encryptedCommand(CARD_CMD_SECURE_CHIPID, command, 0);
	if (normalChip) {
		cardPolledTransfer(key1Flags, NULL, 0, command);
		cardDelay(header->readTimeout);
	}
	cardPolledTransfer(key1Flags | CARD_BLK_SIZE(7), NULL, 0, command);

	u32 secureFlags = (header->cardControlBF &
		(CARD_CLK_SLOW | CARD_DELAY1(0x1FFF) | CARD_DELAY2(0x3F))) |
		CARD_ACTIVATE | CARD_nRESET | CARD_SEC_EN | CARD_SEC_DAT;
	for (int block = 4; block < 8; block++) {
		encryptedCommand(CARD_CMD_SECURE_READ, command, block);
		if (normalChip) {
			cardPolledTransfer(secureFlags, NULL, 0, command);
			cardDelay(header->readTimeout);
			for (int i = 0; i < 8; i++)
				cardPolledTransfer(secureFlags | CARD_BLK_SIZE(1), NULL, 0, command);
		} else {
			cardPolledTransfer(secureFlags | CARD_BLK_SIZE(4) | CARD_SEC_LARGE,
				NULL, 0, command);
		}
	}

	encryptedCommand(CARD_CMD_DATA_MODE, command, 0);
	if (normalChip) {
		cardPolledTransfer(key1Flags, NULL, 0, command);
		cardDelay(header->readTimeout);
	}
	cardPolledTransfer(key1Flags, NULL, 0, command);
	return 0;
}
