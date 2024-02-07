#include "common/tonccpy.h"
#include "find.h"
#include "Save.h"
#include "EepromSave.h"

extern u32 romFileSize;

//todo: Moero!! Jaleco Collection (Japan) reports EEPROM_V124, but the signatures below don't work!

static const u8 sReadEepromDwordV111Sig[0x10] =
	{0xB0, 0xB5, 0xAA, 0xB0, 0x6F, 0x46, 0x79, 0x60, 0x39, 0x1C, 0x08, 0x80, 0x38, 0x1C, 0x01, 0x88};

static const u8 sReadEepromDwordV120Sig[0x10] =
	{0x70, 0xB5, 0xA2, 0xB0, 0x0D, 0x1C, 0x00, 0x04, 0x03, 0x0C, 0x03, 0x48, 0x00, 0x68, 0x80, 0x88};

static const u8 sProgramEepromDwordV111Sig[0x10] =
	{0x80, 0xB5, 0xAA, 0xB0, 0x6F, 0x46, 0x79, 0x60, 0x39, 0x1C, 0x08, 0x80, 0x38, 0x1C, 0x01, 0x88};

//changed in EEPROM_V124
static const u8 sProgramEepromDwordV120Sig[0x10] =
	{0x30, 0xB5, 0xA9, 0xB0, 0x0D, 0x1C, 0x00, 0x04, 0x04, 0x0C, 0x03, 0x48, 0x00, 0x68, 0x80, 0x88};

//changed in EEPROM_V126
static const u8 sProgramEepromDwordV124Sig[0x10] =
	{0xF0, 0xB5, 0xAC, 0xB0, 0x0D, 0x1C, 0x00, 0x04, 0x01, 0x0C, 0x12, 0x06, 0x17, 0x0E, 0x03, 0x48};

static const u8 sProgramEepromDwordV126Sig[0x10] =
	{0xF0, 0xB5, 0x47, 0x46, 0x80, 0xB4, 0xAC, 0xB0, 0x0E, 0x1C, 0x00, 0x04, 0x05, 0x0C, 0x12, 0x06};

//not in EEPROM_V111
//could be used to identify the eeprom size, but not strictly needed
/*static u16 identifyEeprom(u16 kbitSize)
{
	return 0;
}*/

const u8 patch_eeprom_1[]=
  {
    0x00,0x04, // LSL     R0, R0, #0x10
    0x0a,0x1c, // ADD     R2, R1, #0
    0x40,0x0b, // LSR     R0, R0, #0xD
    0xe0,0x21,0x09,0x05, // MOVL    R1, 0xE000000
    0x41,0x18, // ADD     R1, R0, R1
    0x07,0x31, // ADD     R1, #7
    0x00,0x23, // MOV     R3, #0
    //l1:
    0x08,0x78, // LDRB    R0, [R1]
    0x10,0x70, // STRB    R0, [R2]
    0x01,0x33, // ADD     R3, #1
    0x01,0x32, // ADD     R2, #1
    0x01,0x39, // SUB     R1, #1
    0x07,0x2b, // CMP     R3, #7
    0xf8,0xd9, // BLS     l1
    0x00,0x20, // MOV     R0, #0
    0x70,0x47  // BX      LR
};
const u8 patch_eeprom_2[]=
{
    0x00,0x04, // LSL     R0, R0, #0x10
    0x0a,0x1c, // ADD     R2, R1, #0
    0x40,0x0b, // LSR     R0, R0, #0xD
    0xe0,0x21,0x09,0x05, // MOVL    R1, 0xE000000
    0x41,0x18, // ADD     R1, R0, R1
    0x07,0x31, // ADD     R1, #7
    0x00,0x23, // MOV     R3, #0
    //l1:
    0x10,0x78, // LDRB    R0, [R2]
    0x08,0x70, // STRB    R0, [R1]
    0x01,0x33, // ADD     R3, #1
    0x01,0x32, // ADD     R2, #1
    0x01,0x39, // SUB     R1, #1
    0x07,0x2b, // CMP     R3, #7
    0xf8,0xd9, // BLS     l1
    0x00,0x20, // MOV     R0, #0
    0x70,0x47  // BX      LR
};

bool eeprom_patchV111(const save_type_t* type)
{
	u8* readFunc = memsearch8((u8*)0x08000000, romFileSize, sReadEepromDwordV111Sig, 0x10, true);
	if (!readFunc)
		return false;
	tonccpy(readFunc, &patch_eeprom_1, sizeof(patch_eeprom_1));

	u8* progFunc = memsearch8((u8*)0x08000000, romFileSize, sProgramEepromDwordV111Sig, 0x10, true);
	if (!progFunc)
		return false;
	tonccpy(progFunc, &patch_eeprom_2, sizeof(patch_eeprom_2));
	return true;
}

bool eeprom_patchV120(const save_type_t* type)
{
	u32* romPos = (u32*)0x08000000;
	u32 curRomSize = romFileSize;
	u32 startSig[4] = {0};
	startSig[0] = *(u32*)0x08000000;
	startSig[1] = *(u32*)0x08000004;
	startSig[2] = *(u32*)0x08000008;
	startSig[3] = *(u32*)0x0800000C;

	for (int i = 0; i < 3; i++) {

	if (i != 0) {
		while (romPos < romPos+romFileSize) {
			// Look for another ROM in 2-3 in 1 game packs
			romPos += 0x100000;
			curRomSize -= 0x100000;
			if (curRomSize <= 0) break;
			if (romPos[0] == startSig[0]
			&& romPos[1] == startSig[1]
			&& romPos[2] == startSig[2]
			&& romPos[3] == startSig[3]) {
				break;
			}
		}

		if (romPos >= romPos+romFileSize) break;
	}

	u8* readFunc = memsearch8((u8*)romPos, curRomSize, sReadEepromDwordV120Sig, 0x10, true);
	if (!readFunc)
		return false;
	tonccpy(readFunc, &patch_eeprom_1, sizeof(patch_eeprom_1));

	u8* progFunc = memsearch8((u8*)romPos, curRomSize, sProgramEepromDwordV120Sig, 0x10, true);
	if (!progFunc)
		return false;
	tonccpy(progFunc, &patch_eeprom_2, sizeof(patch_eeprom_2));

	}

	return true;
}

bool eeprom_patchV124(const save_type_t* type)
{
	u32* romPos = (u32*)0x08000000;
	u32 curRomSize = romFileSize;
	u32 startSig[4] = {0};
	startSig[0] = *(u32*)0x08000000;
	startSig[1] = *(u32*)0x08000004;
	startSig[2] = *(u32*)0x08000008;
	startSig[3] = *(u32*)0x0800000C;

	for (int i = 0; i < 2; i++) {

	if (i != 0) {
		while (romPos < romPos+romFileSize) {
			// Look for another ROM in 2 in 1 game packs
			romPos += 0x100000;
			curRomSize -= 0x100000;
			if (curRomSize <= 0) break;
			if (romPos[0] == startSig[0]
			&& romPos[1] == startSig[1]
			&& romPos[2] == startSig[2]
			&& romPos[3] == startSig[3]) {
				break;
			}
		}

		if (romPos >= romPos+romFileSize) break;
	}

	u8* readFunc = memsearch8((u8*)romPos, curRomSize, sReadEepromDwordV120Sig, 0x10, true);
	if (!readFunc)
		return false;
	tonccpy(readFunc, &patch_eeprom_1, sizeof(patch_eeprom_1));

	u8* progFunc = memsearch8((u8*)romPos, curRomSize, sProgramEepromDwordV124Sig, 0x10, true);
	if (!progFunc)
		return false;
	tonccpy(progFunc, &patch_eeprom_2, sizeof(patch_eeprom_2));

	}

	return true;
}

bool eeprom_patchV126(const save_type_t* type)
{
	u8* readFunc = memsearch8((u8*)0x08000000, romFileSize, sReadEepromDwordV120Sig, 0x10, true);
	if (!readFunc)
		return false;
	tonccpy(readFunc, &patch_eeprom_1, sizeof(patch_eeprom_1));

	u8* progFunc = memsearch8((u8*)0x08000000, romFileSize, sProgramEepromDwordV126Sig, 0x10, true);
	if (!progFunc)
		return false;
	tonccpy(progFunc, &patch_eeprom_2, sizeof(patch_eeprom_2));
	return true;
}
