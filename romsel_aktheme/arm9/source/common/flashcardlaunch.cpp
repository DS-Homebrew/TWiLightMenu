
#include "flashcardlaunch.h"
#include "pergamesettings.h"

#define FC_PREFIX_SLASH "/"
#define FC_PREFIX_FAT "fat:/"
#define FC_PREFIX_FAT0 "fat0:/"
#define FC_PREFIX_FAT1 "fat1:/"

int loadGameOnFlashcard(const char *ndsPath, std::string filename, bool usePerGameSettings)
{
	bool runNds_boostCpu = false;
	bool runNds_boostVram = false;
	/*if (isDSiMode() && usePerGameSettings) {
		PerGameSettings gameConfig(filename);
		if (PerGameSettings.boostCpu == -1) {
			runNds_boostCpu = bs().boostCpu;
		} else {
			runNds_boostCpu = PerGameSettings::boostCpu;
		}
		if (PerGameSettings.boostVram == -1) {
			runNds_boostVram = bs().boostVram;
		} else {
			runNds_boostVram = PerGameSettings.boostVram;
		}
	}*/
    std::string launchPath;
    switch (ms().flashcard)
    {
    case DSiMenuPlusPlusSettings::EDSTTClone:
    case DSiMenuPlusPlusSettings::ER4Original:
    default:
    {
        LoaderConfig config("fat:/YSMenu.nds", "fat:/TTMenu/YSMenu.ini");
        launchPath = replaceAll(ndsPath, FC_PREFIX_FAT, FC_PREFIX_SLASH);
        config.option("YSMENU", "AUTO_BOOT", launchPath)
            .option("YSMENU", "DEFAULT_DMA", "true")
            .option("YSMENU", "DEFAULT_RESET", "false");
        return config.launch(0, NULL, true, true, runNds_boostCpu, runNds_boostVram);
    }
    case DSiMenuPlusPlusSettings::ER4iGoldClone:
    case DSiMenuPlusPlusSettings::EAcekardRPG:
    case DSiMenuPlusPlusSettings::EGatewayBlue:
    {
        LoaderConfig config("fat:/Wfwd.dat", "fat:/_wfwd/lastsave.ini");
        launchPath = replaceAll(ndsPath, FC_PREFIX_FAT, FC_PREFIX_FAT0);
        config.option("Save Info", "lastLoaded", launchPath);
        return config.launch(0, NULL, true, true, runNds_boostCpu, runNds_boostVram);
    }
    case DSiMenuPlusPlusSettings::EAcekard2i:
    {
        LoaderConfig config("fat:/Afwd.dat", "fat:/_afwd/lastsave.ini");
        launchPath = replaceAll(ndsPath, FC_PREFIX_FAT, FC_PREFIX_FAT0);
        config.option("Save Info", "lastLoaded", launchPath);
        return config.launch(0, NULL, true, true, runNds_boostCpu, runNds_boostVram);
    }
    case DSiMenuPlusPlusSettings::ESupercardDSTWO:
    {
        LoaderConfig config("fat:/_dstwo/autoboot.nds", "fat:/_dstwo/autoboot.ini");
        launchPath = replaceAll(ndsPath, FC_PREFIX_FAT, FC_PREFIX_FAT1);
        config.option("Dir Info", "fullName", launchPath);
        return config.launch(0, NULL, true, true, runNds_boostCpu, runNds_boostVram);
    }
    }
    return 100;
}