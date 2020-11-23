#pragma once

enum SaveType : u16
{
	SAVE_TYPE_NONE = 0,

	SAVE_TYPE_EEPROM = (1 << 14),
	SAVE_TYPE_EEPROM_V111,
	SAVE_TYPE_EEPROM_V120,
	SAVE_TYPE_EEPROM_V121,
	SAVE_TYPE_EEPROM_V122,
	SAVE_TYPE_EEPROM_V124,
	SAVE_TYPE_EEPROM_V125,
	SAVE_TYPE_EEPROM_V126,

	SAVE_TYPE_FLASH = (2 << 14),
	SAVE_TYPE_FLASH512 = SAVE_TYPE_FLASH | (0 << 13),
	SAVE_TYPE_FLASH_V120,
	SAVE_TYPE_FLASH_V121,
	SAVE_TYPE_FLASH_V123,
	SAVE_TYPE_FLASH_V124,
	SAVE_TYPE_FLASH_V125,
	SAVE_TYPE_FLASH_V126,
	SAVE_TYPE_FLASH512_V130,
	SAVE_TYPE_FLASH512_V131,
	SAVE_TYPE_FLASH512_V133,

	SAVE_TYPE_FLASH1M = SAVE_TYPE_FLASH | (1 << 13),
	SAVE_TYPE_FLASH1M_V102,
	SAVE_TYPE_FLASH1M_V103,

	SAVE_TYPE_SRAM = (3 << 14),
	SAVE_TYPE_SRAM_F_V100,
	SAVE_TYPE_SRAM_F_V102,
	SAVE_TYPE_SRAM_F_V103,
	SAVE_TYPE_SRAM_V110,
	SAVE_TYPE_SRAM_V111,
	SAVE_TYPE_SRAM_V112,
	SAVE_TYPE_SRAM_V113,

	SAVE_TYPE_TYPE_MASK = (3 << 14)
};

struct save_type_t
{
	char     tag[16];
	u16      tagLength;
	SaveType type;
	u32      size;
	bool (*  patchFunc)(const save_type_t* type);
};

#define CP15_SET_DATA_PROT(x)		do { asm volatile("mcr p15, 0, %0, c5, c0, 2" :: "r"((x))); } while(0)

#define GBA_INTERWORK_BRIDGE(func) \
	static u16  __attribute__((naked)) func() \
	{ \
		asm volatile("push {lr}"); \
		asm volatile("bl "#func"_impl"); \
		asm volatile("pop {lr}"); \
		asm volatile("bx lr"); \
	}

const save_type_t* save_findTag();
void               save_injectJump(u32* location, void* jumpTarget);
