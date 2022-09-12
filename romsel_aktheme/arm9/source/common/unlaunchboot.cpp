#include "unlaunchboot.h"
#include "common/fatHeader.h"
#include "tool/stringtool.h"
#include <nds.h>
#include "dsimenusettings.h"
#include "filecopy.h"

const char *unlaunchAutoLoadID = "AutoLoadInfo";
char hiyaNdsPath[14] = {'s','d','m','c',':','/','h','i','y','a','.','d','s','i'};

UnlaunchBoot::UnlaunchBoot(const std::string &fileName, u32 pubSavSize, u32 prvSavSize)
{
    _fileName = fileName;
    _pubSavSize = pubSavSize;
    _prvSavSize = prvSavSize;
}

void UnlaunchBoot::createSaveIfNotExists(const std::string &fileExt, const std::string &savExt, u32 saveSize)
{
    std::string saveName = replaceAll(_fileName, fileExt, savExt);
    if (access(saveName.c_str(), F_OK) == 0) {
        return;
	}

	if (saveSize > 0) {
		const u16 sectorSize = 0x200;

		//fit maximum sectors for the size
		const u16 maxSectors = saveSize / sectorSize;
		u16 sectorCount = 1;
		u16 secPerTrk = 1;
		u16 numHeads = 1;
		u16 sectorCountNext = 0;
		while (sectorCountNext <= maxSectors) {
			sectorCountNext = secPerTrk * (numHeads + 1) * (numHeads + 1);
			if (sectorCountNext <= maxSectors) {
				numHeads++;
				sectorCount = sectorCountNext;

				secPerTrk++;
				sectorCountNext = secPerTrk * numHeads * numHeads;
				if (sectorCountNext <= maxSectors) {
					sectorCount = sectorCountNext;
				}
			}
		}
		sectorCountNext = (secPerTrk + 1) * numHeads * numHeads;
		if (sectorCountNext <= maxSectors) {
			secPerTrk++;
			sectorCount = sectorCountNext;
		}

		u8 secPerCluster = (sectorCount > (8 << 10)) ? 8 : (sectorCount > (1 << 10) ? 4 : 1);

		u16 rootEntryCount = saveSize < 0x8C000 ? 0x20 : 0x200;

		#define ALIGN(v, a) (((v) % (a)) ? ((v) + (a) - ((v) % (a))) : (v))
		u16 totalClusters = ALIGN(sectorCount, secPerCluster) / secPerCluster;
		u32 fatBytes = (ALIGN(totalClusters, 2) / 2) * 3; // 2 sectors -> 3 byte
		u16 fatSize = ALIGN(fatBytes, sectorSize) / sectorSize;


		FATHeader h;
		memset(&h, 0, sizeof(FATHeader));

		h.BS_JmpBoot[0] = 0xE9;
		h.BS_JmpBoot[1] = 0;
		h.BS_JmpBoot[2] = 0;

		memcpy(h.BS_OEMName, "MSWIN4.1", 8);

		h.BPB_BytesPerSec = sectorSize;
		h.BPB_SecPerClus = secPerCluster;
		h.BPB_RsvdSecCnt = 0x0001;
		h.BPB_NumFATs = 0x02;
		h.BPB_RootEntCnt = rootEntryCount;
		h.BPB_TotSec16 = sectorCount;
		h.BPB_Media = 0xF8; // "hard drive"
		h.BPB_FATSz16 = fatSize;
		h.BPB_SecPerTrk = secPerTrk;
		h.BPB_NumHeads = numHeads;
		h.BS_DrvNum = 0x05;
		h.BS_BootSig = 0x29;
		h.BS_VolID = 0x12345678;
		memcpy(h.BS_VolLab, "VOLUMELABEL", 11);
		memcpy(h.BS_FilSysType,"FAT12   ", 8);
		h.BS_BootSign = 0xAA55;

		FILE *file = fopen(saveName.c_str(), "wb");
		if (file) {
			fwrite(&h, sizeof(FATHeader), 1, file); // Write header
			fseek(file, saveSize - 1, SEEK_SET); // Pad rest of the file
			fputc('\0', file);
			fclose(file);
		}
	}
}

UnlaunchBoot &UnlaunchBoot::onPrvSavCreated(std::function<void(void)> handler)
{
    _prvSavCreatedHandler = handler;
    return *this;
}

UnlaunchBoot &UnlaunchBoot::onPubSavCreated(std::function<void(void)> handler)
{
    _pubSavCreatedHandler = handler;
    return *this;
}

bool UnlaunchBoot::doRenames(const std::string &fileExt)
{
    std::string pubPath = replaceAll(_fileName, fileExt, ".pub");
    std::string prvPath = replaceAll(_fileName, fileExt, ".prv");
    
    ms().dsiWareSrlPath = _fileName;
    ms().dsiWarePrvPath = prvPath;
    ms().dsiWarePubPath = pubPath;
    ms().saveSettings();

	if (ms().secondaryDevice) {
		fcopy(_fileName.c_str(), "sd:/_nds/TWiLightMenu/tempDSiWare.dsi");
		if (access(pubPath.c_str(), F_OK) == 0) {
			fcopy(pubPath.c_str(), "sd:/_nds/TWiLightMenu/tempDSiWare.pub");
		}
		if (access(prvPath.c_str(), F_OK) == 0) {
			fcopy(prvPath.c_str(), "sd:/_nds/TWiLightMenu/tempDSiWare.prv");
		}
		if (access(pubPath.c_str(), F_OK) == 0 || access(prvPath.c_str(), F_OK) == 0) {
			return true;	// Show a message if save(s) is on secondary drive
		}
	}

	return false;
}

bool UnlaunchBoot::prepare()
{
    std::string extension;
    size_t lastDotPos = _fileName.find_last_of('.');

    if (_fileName.npos != lastDotPos)
        extension = _fileName.substr(lastDotPos);
    else
        return false;

    createSaveIfNotExists(extension, ".prv", _prvSavSize);
    if (_prvSavCreatedHandler)
        _prvSavCreatedHandler();

    createSaveIfNotExists(extension, ".pub", _pubSavSize);
    if (_pubSavCreatedHandler)
        _pubSavCreatedHandler();

    
    return UnlaunchBoot::doRenames(extension);
}

void UnlaunchBoot::launch()
{
	char unlaunchDevicePath[256];
	if (strncmp(_fileName.c_str(), "cart:", 5) == 0) {
		sprintf(unlaunchDevicePath, "cart:");
	} else if (ms().secondaryDevice) {
		snprintf(unlaunchDevicePath, sizeof(unlaunchDevicePath), "sdmc:/_nds/TWiLightMenu/tempDSiWare.dsi");
	} else {
		snprintf(unlaunchDevicePath, sizeof(unlaunchDevicePath), "__%s", _fileName.c_str());
		unlaunchDevicePath[0] = 's';
		unlaunchDevicePath[1] = 'd';
		unlaunchDevicePath[2] = 'm';
		unlaunchDevicePath[3] = 'c';
	}

	memcpy((u8*)0x02000800, unlaunchAutoLoadID, 12);
	*(u16*)(0x0200080C) = 0x3F0;		// Unlaunch Length for CRC16 (fixed, must be 3F0h)
	*(u16*)(0x0200080E) = 0;			// Unlaunch CRC16 (empty)
	*(u32*)(0x02000810) = 0;			// Unlaunch Flags
	*(u32*)(0x02000810) |= BIT(0);		// Load the title at 2000838h
	*(u32*)(0x02000810) |= BIT(1);		// Use colors 2000814h
	*(u16*)(0x02000814) = 0x7FFF;		// Unlaunch Upper screen BG color (0..7FFFh)
	*(u16*)(0x02000816) = 0x7FFF;		// Unlaunch Lower screen BG color (0..7FFFh)
	memset((u8*)0x02000818, 0, 0x20+0x208+0x1C0);		// Unlaunch Reserved (zero)
	int i2 = 0;
	for (int i = 0; i < (int)sizeof(unlaunchDevicePath); i++) {
		*(u8*)(0x02000838+i2) = unlaunchDevicePath[i];		// Unlaunch Device:/Path/Filename.ext (16bit Unicode,end by 0000h)
		i2 += 2;
	}
	while (*(u16*)(0x0200080E) == 0) {	// Keep running, so that CRC16 isn't 0
		*(u16*)(0x0200080E) = swiCRC16(0xFFFF, (void*)0x02000810, 0x3F0);		// Unlaunch CRC16
	}

	DC_FlushAll();						// Make reboot not fail
    fifoSendValue32(FIFO_USER_02, 1);
}