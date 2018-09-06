#include "unlaunchboot.h"
#include "tool/stringtool.h"
#include <nds.h>
#include "dsimenusettings.h"

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

    if (access(BOOTTHIS_SRL, F_OK) == 0)
        return false;
    if (access(BOOTTHIS_PRV, F_OK) == 0)
        return false;
    if (access(BOOTTHIS_PUB, F_OK) == 0)
        return false;

    rename(_fileName.c_str(), BOOTTHIS_SRL);

    if (access(pubPath.c_str(), F_OK) == 0)
        rename(pubPath.c_str(), BOOTTHIS_PUB);

    if (access(prvPath.c_str(), F_OK) == 0)
        rename(prvPath.c_str(), BOOTTHIS_PRV);
    return true;
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
    fifoSendValue32(FIFO_USER_02, 1);
}