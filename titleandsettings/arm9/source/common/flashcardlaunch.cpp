
#include "flashcardlaunch.h"

#define FC_PREFIX_SLASH "/"
#define FC_PREFIX_FAT "fat:/"
#define FC_PREFIX_FAT0 "fat0:/"
#define FC_PREFIX_FAT1 "fat1:/"

int loadGameOnFlashcard(const char *filename)
{
    std::string launchPath;
    switch (ms().flashcard)
    {
    case DSiMenuPlusPlusSettings::EDSTTClone:
    case DSiMenuPlusPlusSettings::ER4Original:
    default:
    {
        LoaderConfig config("fat:/YSMenu.nds", "fat:/TTMenu/YSMenu.ini");
        launchPath = replaceAll(filename, FC_PREFIX_FAT, FC_PREFIX_SLASH);
        config.option("YSMENU", "AUTO_BOOT", launchPath)
            .option("YSMENU", "DEFAULT_DMA", "true")
            .option("YSMENU", "DEFAULT_RESET", "false");
        return config.launch();
    }
    case DSiMenuPlusPlusSettings::ER4iGoldClone:
    case DSiMenuPlusPlusSettings::EAcekardRPG:
    case DSiMenuPlusPlusSettings::EGatewayBlue:
    {
        LoaderConfig config("fat:/Wfwd.dat", "fat:/_wfwd/lastsave.ini");
        launchPath = replaceAll(filename, FC_PREFIX_FAT, FC_PREFIX_FAT0);
        config.option("Save Info", "lastLoaded", launchPath);
        return config.launch();
    }
    case DSiMenuPlusPlusSettings::EAcekard2i:
    {
        LoaderConfig config("fat:/Afwd.dat", "fat:/_afwd/lastsave.ini");
        launchPath = replaceAll(filename, FC_PREFIX_FAT, FC_PREFIX_FAT0);
        config.option("Save Info", "lastLoaded", launchPath);
        return config.launch();
    }
    case DSiMenuPlusPlusSettings::ESupercardDSTWO:
    {
        LoaderConfig config("fat:/_dstwo/autoboot.nds", "fat:/_dstwo/autoboot.ini");
        launchPath = replaceAll(filename, FC_PREFIX_FAT, FC_PREFIX_FAT1);
        config.option("Dir Info", "fullName", launchPath);
        return config.launch();
    }
    }
    return 100;
}