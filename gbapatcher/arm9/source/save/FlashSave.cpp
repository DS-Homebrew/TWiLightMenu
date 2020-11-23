#include "tonccpy.h"
#include "find.h"
#include "Save.h"
#include "FlashSave.h"

struct flash_patchinfo_t
{
	u32* progSectorPtr;
	u32* progBytePtr;
	u32* eraseChipPtr;
	u32* eraseSectorPtr;
	u32* pollingSrPtr;
	u32* flMaxTimePtr;
	u32* flashPtr;
};

static flash_patchinfo_t sPatchInfo;

#define FLASH_V120_OFFSET_PROG_SECTOR		0x18
#define FLASH_V120_OFFSET_ERASE_CHIP		0x1C
#define FLASH_V120_OFFSET_ERASE_SECTOR		0x20
#define FLASH_V120_OFFSET_POLLING_SR		0x24
#define FLASH_V120_OFFSET_FL_MAXTIME		0x28
#define FLASH_V120_OFFSET_FLASH				0x2C
#define FLASH_V120_OFFSET_READ				0x84
#define FLASH_V120_OFFSET_VERIFY_SECTOR		0x90

#define FLASH_512V130_OFFSET_PROG_SECTOR	0x14
#define FLASH_512V130_OFFSET_ERASE_CHIP		0x18
#define FLASH_512V130_OFFSET_ERASE_SECTOR	0x1C
#define FLASH_512V130_OFFSET_POLLING_SR		0x20
#define FLASH_512V130_OFFSET_FL_MAXTIME		0x24
#define FLASH_512V130_OFFSET_FLASH			0x28

#define FLASH_1MV102_OFFSET_PROG_SECTOR		0x10
#define FLASH_1MV102_OFFSET_ERASE_CHIP		0x14
#define FLASH_1MV102_OFFSET_ERASE_SECTOR	0x18
#define FLASH_1MV102_OFFSET_POLLING_SR		0x1C
#define FLASH_1MV102_OFFSET_FL_MAXTIME		0x20
#define FLASH_1MV102_OFFSET_FLASH			0x24

#define FLASH_1MV103_OFFSET_PROG_BYTE		0x10
#define FLASH_1MV103_OFFSET_PROG_SECTOR		0x14
#define FLASH_1MV103_OFFSET_ERASE_CHIP		0x18
#define FLASH_1MV103_OFFSET_ERASE_SECTOR	0x1C
#define FLASH_1MV103_OFFSET_POLLING_SR		0x20
#define FLASH_1MV103_OFFSET_FL_MAXTIME		0x24
#define FLASH_1MV103_OFFSET_FLASH			0x28

struct flash_v120_sector
{
	u32 size;
	u8  shift;
	u16 count;
	u16 top;
};

struct flash_v120_type
{
	u32               romSize;
	flash_v120_sector sector;
	u16               agbWait[2];
	u8                makerID;
	u8                deviceID;
};

static flash_v120_type sFlashType;
static const u16       sMaxTime[] = {0xA, 0xFFBD, 0xC2, 0xA, 0xFFBD, 0xC2, 0x28, 0xFFBD, 0xC2, 0xC8, 0xFFBD, 0xC2};

static const u8 sIdentifyFlashV120Sig[0x10] =
	{0x80, 0xB5, 0x82, 0xB0, 0x6F, 0x46, 0x0E, 0x48, 0x0D, 0x49, 0x0A, 0x88, 0x0D, 0x4B, 0x11, 0x1C};

static const u8 sIdentifyFlashV123Sig[0x10] =
	{0x10, 0xB5, 0x07, 0x4A, 0x10, 0x88, 0x07, 0x49, 0x08, 0x40, 0x03, 0x21, 0x08, 0x43, 0x10, 0x80};

static const u8 sIdentifyFlash1MV103Sig[0x10] =
	{0x10, 0xB5, 0x07, 0x4A, 0x10, 0x88, 0x07, 0x49, 0x08, 0x40, 0x03, 0x21, 0x08, 0x43, 0x10, 0x80};

static const u8 sVerifyFlashV126Sig[0x10] =
	{0x70, 0xB5, 0xC0, 0xB0, 0x0D, 0x1C, 0x16, 0x1C, 0x00, 0x04, 0x04, 0x0C, 0x07, 0x4A, 0x10, 0x88};

static const u8 sReadFlash512V130Sig[0x10] =
	{0xF0, 0xB5, 0xA0, 0xB0, 0x0D, 0x1C, 0x16, 0x1C, 0x1F, 0x1C, 0x03, 0x04, 0x1C, 0x0C, 0x0F, 0x4A};

static const u8 sVerifyFlash512V130Sig[0x10] =
	{0x70, 0xB5, 0xC0, 0xB0, 0x0D, 0x1C, 0x16, 0x1C, 0x02, 0x04, 0x14, 0x0C, 0x0E, 0x48, 0x00, 0x68};

static const u8 sVerifyFlashSector512V130Sig[0x10] =
	{0x30, 0xB5, 0xC0, 0xB0, 0x0D, 0x1C, 0x03, 0x04, 0x1C, 0x0C, 0x0F, 0x4A, 0x10, 0x88, 0x0F, 0x49};

/*extern "C" u16 eraseFlashChip_impl()
{
	vram_cd_t* vramcd_uncached = (vram_cd_t*)(((u32)vram_cd) + UNCACHED_OFFSET);
	u8*        pSave = (u8*)MAIN_MEMORY_ADDRESS_SAVE_DATA;
	//disable irqs
	u32 irq = *(vu32*)0x04000208;
	*(vu32*)0x04000208 = 0;
	{
		CP15_SET_DATA_PROT(0x33333333);
		for (int i = 0; i < SAVE_DATA_SIZE; i++)
			*pSave++ = 0xFF;
		vramcd_uncached->save_work.save_state = SAVE_WORK_STATE_DIRTY;
		CP15_SET_DATA_PROT(pu_data_permissions);
	}
	//restore irqs
	*(vu32*)0x04000208 = irq;
	return 0;
}

GBA_INTERWORK_BRIDGE(eraseFlashChip)

extern "C" u16 eraseFlashSector_impl(u16 secNo)
{
	vram_cd_t* vramcd_uncached = (vram_cd_t*)(((u32)vram_cd) + UNCACHED_OFFSET);
	u8*        pSave = (u8*)(MAIN_MEMORY_ADDRESS_SAVE_DATA + (secNo << 12));
	//disable irqs
	u32 irq = *(vu32*)0x04000208;
	*(vu32*)0x04000208 = 0;
	{
		CP15_SET_DATA_PROT(0x33333333);
		for (int i = 0; i < (1 << 12); i++)
			*pSave++ = 0xFF;
		vramcd_uncached->save_work.save_state = SAVE_WORK_STATE_DIRTY;
		CP15_SET_DATA_PROT(pu_data_permissions);
	}
	//restore irqs
	*(vu32*)0x04000208 = irq;
	return 0;
}

GBA_INTERWORK_BRIDGE(eraseFlashSector)

extern "C" u16 programFlashSector512_impl(u16 secNo, u8* src)
{
	u8* pSave = (u8*)(0x0E000000 + (secNo << 12));
	for (int i = 0; i < (1 << 12); i++)
		*pSave++ = *src++;
	return 0;
}

GBA_INTERWORK_BRIDGE(programFlashSector512)

//todo: account for rom source
extern "C" u16 programFlashSector1M_impl(u16 secNo, u8* src)
{
	vram_cd_t* vramcd_uncached = (vram_cd_t*)(((u32)vram_cd) + UNCACHED_OFFSET);
	u8*        pSave = (u8*)(MAIN_MEMORY_ADDRESS_SAVE_DATA + (secNo << 12));
	//disable irqs
	u32 irq = *(vu32*)0x04000208;
	*(vu32*)0x04000208 = 0;
	{
		CP15_SET_DATA_PROT(0x33333333);
		for (int i = 0; i < (1 << 12); i++)
			*pSave++ = *src++;
		vramcd_uncached->save_work.save_state = SAVE_WORK_STATE_DIRTY;
		CP15_SET_DATA_PROT(pu_data_permissions);
	}
	//restore irqs
	*(vu32*)0x04000208 = irq;
	return 0;
}

GBA_INTERWORK_BRIDGE(programFlashSector1M)

extern "C" u16 programFlashByte1M_impl(u16 secNo, u32 offset, u8 data)
{
	vram_cd_t* vramcd_uncached = (vram_cd_t*)(((u32)vram_cd) + UNCACHED_OFFSET);
	u8*        pSave = (u8*)(MAIN_MEMORY_ADDRESS_SAVE_DATA + (secNo << 12) + offset);
	//disable irqs
	u32 irq = *(vu32*)0x04000208;
	*(vu32*)0x04000208 = 0;
	{
		CP15_SET_DATA_PROT(0x33333333);
		*pSave = data;
		vramcd_uncached->save_work.save_state = SAVE_WORK_STATE_DIRTY;
		CP15_SET_DATA_PROT(pu_data_permissions);
	}
	//restore irqs
	*(vu32*)0x04000208 = irq;
	return 0;
}

GBA_INTERWORK_BRIDGE(programFlashByte1M)

extern "C" u32 verifyFlashSector_impl(u16 secNo, u8* src)
{
	//reading from main memory is safe without changing permissions
	const u32 addr = secNo << 12;
	u8*       pSave = (u8*)(MAIN_MEMORY_ADDRESS_SAVE_DATA + addr);
	for (int i = 0; i < (1 << 12); i++)
		if (*pSave++ != *src++)
			return addr + i;
	return 0;
}

GBA_INTERWORK_BRIDGE(verifyFlashSector)

extern "C" u32 verifyFlash_impl(u16 secNo, u8* src, u32 size)
{
	//reading from main memory is safe without changing permissions
	const u32 addr = secNo << 12;
	u8*       pSave = (u8*)(MAIN_MEMORY_ADDRESS_SAVE_DATA + addr);
	for (int i = 0; i < size; i++)
		if (*pSave++ != *src++)
			return addr + i;
	return 0;
}

GBA_INTERWORK_BRIDGE(verifyFlash)

extern "C" void readFlash_impl(u16 secNo, u32 offset, u8* dst, u32 size)
{
	//reading from main memory is safe without changing permissions
	u8* pSave = (u8*)(MAIN_MEMORY_ADDRESS_SAVE_DATA + (secNo << 12) + offset);
	for (int i = 0; i < size; i++)
		*dst++ = *pSave++;
}

GBA_INTERWORK_BRIDGE(readFlash)

extern "C" u16 identifyFlash512_impl()
{
	*sPatchInfo.progSectorPtr = (u32)&programFlashSector512;
	*sPatchInfo.eraseChipPtr = (u32)&eraseFlashChip;
	*sPatchInfo.eraseSectorPtr = (u32)&eraseFlashSector;
	*sPatchInfo.pollingSrPtr = NULL;
	*sPatchInfo.flMaxTimePtr = (u32)sMaxTime;

	sFlashType.romSize = SAVE_DATA_SIZE;
	sFlashType.sector.size = 0x1000;
	MI_WriteByte(&sFlashType.sector.shift, 12);
	sFlashType.sector.count = sFlashType.sector.size >> sFlashType.sector.shift;
	sFlashType.sector.top = 0;
	sFlashType.agbWait[0] = 0;
	sFlashType.agbWait[1] = 3;
	MI_WriteByte(&sFlashType.makerID, 3);
	MI_WriteByte(&sFlashType.deviceID, 0);
	*sPatchInfo.flashPtr = (u32)&sFlashType;
	return 0;
}

GBA_INTERWORK_BRIDGE(identifyFlash512)

extern "C" u16 identifyFlash1M_impl()
{
	*sPatchInfo.progSectorPtr = (u32)&programFlashSector1M;
	if(sPatchInfo.progBytePtr)
		*sPatchInfo.progBytePtr = (u32)&programFlashByte1M;
	*sPatchInfo.eraseChipPtr = (u32)&eraseFlashChip;
	*sPatchInfo.eraseSectorPtr = (u32)&eraseFlashSector;
	*sPatchInfo.pollingSrPtr = NULL;
	*sPatchInfo.flMaxTimePtr = (u32)sMaxTime;

	sFlashType.romSize = SAVE_DATA_SIZE;
	sFlashType.sector.size = 0x2000;
	MI_WriteByte(&sFlashType.sector.shift, 12);
	sFlashType.sector.count = sFlashType.sector.size >> sFlashType.sector.shift;
	sFlashType.sector.top = 0;
	sFlashType.agbWait[0] = 0;
	sFlashType.agbWait[1] = 3;
	MI_WriteByte(&sFlashType.makerID, 3);
	MI_WriteByte(&sFlashType.deviceID, 0);
	*sPatchInfo.flashPtr = (u32)&sFlashType;
	return 0;
}

GBA_INTERWORK_BRIDGE(identifyFlash1M)

static bool loadDataV120(const save_type_t* type)
{
	//load flash data
	f_lseek(&vram_cd->fil, f_tell(&vram_cd->fil) + ((type->tagLength + 3) & ~3));
	UINT read;
	if (f_read(&vram_cd->fil, vram_cd->tmpSector, 0x94, &read) != FR_OK || read != 0x94)
		return false;

	sPatchInfo.progSectorPtr = *(u32**)(vram_cd->tmpSector + FLASH_V120_OFFSET_PROG_SECTOR);
	sPatchInfo.progBytePtr = NULL;
	sPatchInfo.eraseChipPtr = *(u32**)(vram_cd->tmpSector + FLASH_V120_OFFSET_ERASE_CHIP);
	sPatchInfo.eraseSectorPtr = *(u32**)(vram_cd->tmpSector + FLASH_V120_OFFSET_ERASE_SECTOR);
	sPatchInfo.pollingSrPtr = *(u32**)(vram_cd->tmpSector + FLASH_V120_OFFSET_POLLING_SR);
	sPatchInfo.flMaxTimePtr = *(u32**)(vram_cd->tmpSector + FLASH_V120_OFFSET_FL_MAXTIME);
	sPatchInfo.flashPtr = *(u32**)(vram_cd->tmpSector + FLASH_V120_OFFSET_FLASH);
	return true;
}

bool flash_patchV120(const save_type_t* type)
{
	if (!loadDataV120(type))
		return false;

	u32* pIdentify = gptc_findSignature(sIdentifyFlashV120Sig);
	if (!pIdentify)
		return false;

	save_injectJump(pIdentify, (void*)identifyFlash512);

	save_injectJump((u32*)((*(u32*)(vram_cd->tmpSector + FLASH_V120_OFFSET_READ) & ~1) - 0x08000000 +
		                MAIN_MEMORY_ADDRESS_ROM_DATA), (void*)readFlash);

	save_injectJump((u32*)((*(u32*)(vram_cd->tmpSector + FLASH_V120_OFFSET_VERIFY_SECTOR) & ~1) - 0x08000000 +
		                MAIN_MEMORY_ADDRESS_ROM_DATA), (void*)verifyFlashSector);
	return true;
}

bool flash_patchV123(const save_type_t* type)
{
	if (!loadDataV120(type))
		return false;

	u32* pIdentify = gptc_findSignature(sIdentifyFlashV123Sig);
	if (!pIdentify)
		return false;

	save_injectJump(pIdentify, (void*)identifyFlash512);

	save_injectJump((u32*)((*(u32*)(vram_cd->tmpSector + FLASH_V120_OFFSET_READ) & ~1) - 0x08000000 +
		                MAIN_MEMORY_ADDRESS_ROM_DATA), (void*)readFlash);

	save_injectJump((u32*)((*(u32*)(vram_cd->tmpSector + FLASH_V120_OFFSET_VERIFY_SECTOR) & ~1) - 0x08000000 +
		                MAIN_MEMORY_ADDRESS_ROM_DATA), (void*)verifyFlashSector);
	return true;
}

bool flash_patchV126(const save_type_t* type)
{
	if (!loadDataV120(type))
		return false;

	u32* pIdentify = gptc_findSignature(sIdentifyFlashV123Sig);
	if (!pIdentify)
		return false;

	save_injectJump(pIdentify, (void*)identifyFlash512);

	save_injectJump((u32*)((*(u32*)(vram_cd->tmpSector + FLASH_V120_OFFSET_READ) & ~1) - 0x08000000 +
		                MAIN_MEMORY_ADDRESS_ROM_DATA), (void*)readFlash);

	save_injectJump((u32*)((*(u32*)(vram_cd->tmpSector + FLASH_V120_OFFSET_VERIFY_SECTOR) & ~1) - 0x08000000 +
		                MAIN_MEMORY_ADDRESS_ROM_DATA), (void*)verifyFlashSector);

	u32* verify = gptc_findSignature(sVerifyFlashV126Sig);
	if (!verify)
		return false;
	save_injectJump(verify, (void*)verifyFlash);
	return true;
}

bool flash_patch512V130(const save_type_t* type)
{
	//load flash data
	f_lseek(&vram_cd->fil, f_tell(&vram_cd->fil) + ((type->tagLength + 3) & ~3));
	UINT read;
	if (f_read(&vram_cd->fil, vram_cd->tmpSector, 0x94, &read) != FR_OK || read != 0x94)
		return false;

	sPatchInfo.progSectorPtr = *(u32**)(vram_cd->tmpSector + FLASH_512V130_OFFSET_PROG_SECTOR);
	sPatchInfo.progBytePtr = NULL;
	sPatchInfo.eraseChipPtr = *(u32**)(vram_cd->tmpSector + FLASH_512V130_OFFSET_ERASE_CHIP);
	sPatchInfo.eraseSectorPtr = *(u32**)(vram_cd->tmpSector + FLASH_512V130_OFFSET_ERASE_SECTOR);
	sPatchInfo.pollingSrPtr = *(u32**)(vram_cd->tmpSector + FLASH_512V130_OFFSET_POLLING_SR);
	sPatchInfo.flMaxTimePtr = *(u32**)(vram_cd->tmpSector + FLASH_512V130_OFFSET_FL_MAXTIME);
	sPatchInfo.flashPtr = *(u32**)(vram_cd->tmpSector + FLASH_512V130_OFFSET_FLASH);

	u32* pIdentify = gptc_findSignature(sIdentifyFlashV123Sig);
	if (!pIdentify)
		return false;

	save_injectJump(pIdentify, (void*)identifyFlash512);

	u32* readFunc = gptc_findSignature(sReadFlash512V130Sig);
	if (!readFunc)
		return false;
	save_injectJump(readFunc, (void*)readFlash);

	u32* verifySector = gptc_findSignature(sVerifyFlashSector512V130Sig);
	if (!verifySector)
		return false;
	save_injectJump(verifySector, (void*)verifyFlashSector);

	u32* verify = gptc_findSignature(sVerifyFlash512V130Sig);
	if (!verify)
		return false;
	save_injectJump(verify, (void*)verifyFlash);
	return true;
}

bool flash_patch1MV102(const save_type_t* type)
{
	//load flash data
	f_lseek(&vram_cd->fil, f_tell(&vram_cd->fil) + ((type->tagLength + 3) & ~3));
	UINT read;
	if (f_read(&vram_cd->fil, vram_cd->tmpSector, 0x94, &read) != FR_OK || read != 0x94)
		return false;

	sPatchInfo.progSectorPtr = *(u32**)(vram_cd->tmpSector + FLASH_1MV102_OFFSET_PROG_SECTOR);
	sPatchInfo.progBytePtr = NULL;
	sPatchInfo.eraseChipPtr = *(u32**)(vram_cd->tmpSector + FLASH_1MV102_OFFSET_ERASE_CHIP);
	sPatchInfo.eraseSectorPtr = *(u32**)(vram_cd->tmpSector + FLASH_1MV102_OFFSET_ERASE_SECTOR);
	sPatchInfo.pollingSrPtr = *(u32**)(vram_cd->tmpSector + FLASH_1MV102_OFFSET_POLLING_SR);
	sPatchInfo.flMaxTimePtr = *(u32**)(vram_cd->tmpSector + FLASH_1MV102_OFFSET_FL_MAXTIME + (
		type->type == SAVE_TYPE_FLASH1M_V102 ? 0 : 4));
	sPatchInfo.flashPtr = *(u32**)(vram_cd->tmpSector + FLASH_1MV102_OFFSET_FLASH + (
		type->type == SAVE_TYPE_FLASH1M_V102 ? 0 : 4));

	u32* pIdentify = gptc_findSignature(type->type == SAVE_TYPE_FLASH1M_V102 ? sIdentifyFlashV123Sig : sIdentifyFlash1MV103Sig);
	if (!pIdentify)
		return false;

	save_injectJump(pIdentify, (void*)identifyFlash1M);

	u32* readFunc = gptc_findSignature(sReadFlash512V130Sig);
	if (!readFunc)
		return false;
	save_injectJump(readFunc, (void*)readFlash);

	u32* verifySector = gptc_findSignature(sVerifyFlashSector512V130Sig);
	if (!verifySector)
		return false;
	save_injectJump(verifySector, (void*)verifyFlashSector);

	u32* verify = gptc_findSignature(sVerifyFlash512V130Sig);
	if (!verify)
		return false;
	save_injectJump(verify, (void*)verifyFlash);
	return true;
}

bool flash_patch1MV103(const save_type_t* type)
{
	//load flash data
	f_lseek(&vram_cd->fil, f_tell(&vram_cd->fil) + ((type->tagLength + 3) & ~3));
	UINT read;
	if (f_read(&vram_cd->fil, vram_cd->tmpSector, 0x94, &read) != FR_OK || read != 0x94)
		return false;

	sPatchInfo.progSectorPtr = *(u32**)(vram_cd->tmpSector + FLASH_1MV103_OFFSET_PROG_SECTOR);
	sPatchInfo.progBytePtr = *(u32**)(vram_cd->tmpSector + FLASH_1MV103_OFFSET_PROG_BYTE);
	sPatchInfo.eraseChipPtr = *(u32**)(vram_cd->tmpSector + FLASH_1MV103_OFFSET_ERASE_CHIP);
	sPatchInfo.eraseSectorPtr = *(u32**)(vram_cd->tmpSector + FLASH_1MV103_OFFSET_ERASE_SECTOR);
	sPatchInfo.pollingSrPtr = *(u32**)(vram_cd->tmpSector + FLASH_1MV103_OFFSET_POLLING_SR);
	sPatchInfo.flMaxTimePtr = *(u32**)(vram_cd->tmpSector + FLASH_1MV103_OFFSET_FL_MAXTIME);
	sPatchInfo.flashPtr = *(u32**)(vram_cd->tmpSector + FLASH_1MV103_OFFSET_FLASH);

	u32* pIdentify = gptc_findSignature(sIdentifyFlash1MV103Sig);
	if (!pIdentify)
		return false;

	save_injectJump(pIdentify, (void*)identifyFlash1M);

	u32* readFunc = gptc_findSignature(sReadFlash512V130Sig);
	if (!readFunc)
		return false;
	save_injectJump(readFunc, (void*)readFlash);

	u32* verifySector = gptc_findSignature(sVerifyFlashSector512V130Sig);
	if (!verifySector)
		return false;
	save_injectJump(verifySector, (void*)verifyFlashSector);

	u32* verify = gptc_findSignature(sVerifyFlash512V130Sig);
	if (!verify)
		return false;
	save_injectJump(verify, (void*)verifyFlash);
	return true;
}*/
