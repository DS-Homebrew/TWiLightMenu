#include "unlaunchboot.h"
#include "tool/stringtool.h"
#include <nds.h>
#include "dsimenusettings.h"
#include "filecopy.h"

static const char *unlaunchAutoLoadID = "AutoLoadInfo";
//static char hiyaNdsPath[14] = {'s','d','m','c',':','/','h','i','y','a','.','d','s','i'};

UnlaunchBoot::UnlaunchBoot(const std::string &fileName, u32 pubSavSize, u32 prvSavSize)
{
    _fileName = fileName;
    _pubSavSize = pubSavSize;
    _prvSavSize = prvSavSize;
}

void UnlaunchBoot::createSaveIfNotExists(const std::string &fileExt, const std::string &savExt, u32 saveSize)
{
    std::string saveName = replaceAll(_fileName, fileExt, savExt);
    if (access(saveName.c_str(), F_OK) == 0 && saveSize <= 0)
        return;

    static const int BUFFER_SIZE = 0x1000;
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));

    FILE *pFile = fopen(saveName.c_str(), "wb");
    if (pFile)
    {
        for (int i = saveSize; i > 0; i -= BUFFER_SIZE)
        {
            fwrite(buffer, 1, sizeof(buffer), pFile);
        }
        fclose(pFile);
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
	if (ms().secondaryDevice) {
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

    fifoSendValue32(FIFO_USER_02, 1);
}