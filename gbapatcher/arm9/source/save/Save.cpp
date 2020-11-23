#include <nds.h>
#include "EepromSave.h"
//#include "FlashSave.h"
#include "Save.h"

extern u32 saveSize;

//#define SAVE_TYPE_COUNT		18
#define SAVE_TYPE_COUNT		7

static const save_type_t sSaveTypes[SAVE_TYPE_COUNT] =
{
	{"EEPROM_V111", 12, SAVE_TYPE_EEPROM_V111, 512, eeprom_patchV111},
	{"EEPROM_V120", 12, SAVE_TYPE_EEPROM_V120, 8 * 1024, eeprom_patchV120},
	{"EEPROM_V121", 12, SAVE_TYPE_EEPROM_V121, 8 * 1024, eeprom_patchV120},
	{"EEPROM_V122", 12, SAVE_TYPE_EEPROM_V122, 8 * 1024, eeprom_patchV120},
	{"EEPROM_V124", 12, SAVE_TYPE_EEPROM_V124, 8 * 1024, eeprom_patchV124},
	{"EEPROM_V125", 12, SAVE_TYPE_EEPROM_V125, 8 * 1024, eeprom_patchV124},
	{"EEPROM_V126", 12, SAVE_TYPE_EEPROM_V126, 8 * 1024, eeprom_patchV126},

	/*{"FLASH_V120", 11, SAVE_TYPE_FLASH_V120, 64 * 1024, flash_patchV120},
	{"FLASH_V121", 11, SAVE_TYPE_FLASH_V121, 64 * 1024, flash_patchV120},
	{"FLASH_V123", 11, SAVE_TYPE_FLASH_V123, 64 * 1024, flash_patchV123},
	{"FLASH_V124", 11, SAVE_TYPE_FLASH_V124, 64 * 1024, flash_patchV123},
	{"FLASH_V125", 11, SAVE_TYPE_FLASH_V125, 64 * 1024, flash_patchV123},
	{"FLASH_V126", 11, SAVE_TYPE_FLASH_V126, 64 * 1024, flash_patchV126},
	{"FLASH512_V130", 14, SAVE_TYPE_FLASH512_V130, 64 * 1024, flash_patch512V130},
	{"FLASH512_V131", 14, SAVE_TYPE_FLASH512_V131, 64 * 1024, flash_patch512V130},
	{"FLASH512_V133", 14, SAVE_TYPE_FLASH512_V133, 64 * 1024, flash_patch512V130},
	{"FLASH1M_V102", 13, SAVE_TYPE_FLASH1M_V102, 128 * 1024, flash_patch1MV102},
	{"FLASH1M_V103", 13, SAVE_TYPE_FLASH1M_V103, 128 * 1024, flash_patch1MV103 },*/
};

const save_type_t* save_findTag()
{
	u32  curAddr = 0x08000000;
	while (curAddr < 0x0A000000)
	{
		u32      fst = *(u32*)curAddr;
		SaveType type = SAVE_TYPE_NONE;
		/*if (fst == 0x53414C46)
		{
			//FLAS
			type = SAVE_TYPE_FLASH;
		}
		else*/ if (fst == 0x4D415253)
		{
			//SRAM
			type = SAVE_TYPE_SRAM;
			saveSize = 32*1024;
			return NULL;
		}
		else if (fst == 0x52504545)
		{
			//EEPR
			type = SAVE_TYPE_EEPROM;
		}

		if (type != SAVE_TYPE_NONE)
		{
			for (int i = 0; i < SAVE_TYPE_COUNT; i++)
			{
				if ((sSaveTypes[i].type & SAVE_TYPE_TYPE_MASK) != type)
					continue;
				return &sSaveTypes[i];
			}
		}
		curAddr += 4;
	}
	return NULL;
}
